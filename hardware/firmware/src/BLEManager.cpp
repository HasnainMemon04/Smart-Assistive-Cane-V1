#include "BLEManager.h"
#include "ToF.h"

// Static member initialization
BLEServer* BLEManager::pServer = nullptr;
BLECharacteristic* BLEManager::pChr = nullptr;  // RX characteristic
BLECharacteristic* BLEManager::pTxChr = nullptr;  // TX characteristic
bool BLEManager::clientConnected = false;
uint32_t BLEManager::connectedAt = 0;
uint32_t BLEManager::lastSentStepCount = 0;  // Initialize step tracking

// FreeRTOS components
QueueHandle_t BLEManager::bleQueue = nullptr;
TaskHandle_t BLEManager::bleTaskHandle = nullptr;
SemaphoreHandle_t BLEManager::bleMutex = nullptr;

// Performance counters
static uint32_t droppedPackets = 0;
static uint32_t totalPackets = 0;
static uint32_t lastStatsTime = 0;

// BLE Server Callbacks
class BLEManager::CaneServerCallbacks : public BLEServerCallbacks {
    void onConnect(BLEServer* s) {
        if (xSemaphoreTake(bleMutex, pdMS_TO_TICKS(100)) == pdTRUE) {
            clientConnected = true;
            connectedAt = millis();
            xSemaphoreGive(bleMutex);
        }
        Serial.println("📱 BLE client connected - High-speed mode enabled");
    }

    void onDisconnect(BLEServer* s) {
        if (xSemaphoreTake(bleMutex, pdMS_TO_TICKS(100)) == pdTRUE) {
            clientConnected = false;
            // Flush remaining packets when disconnected
            flushQueue();
            xSemaphoreGive(bleMutex);
        }
        Serial.println("📱 BLE client disconnected - Queue flushed");
        
        // Restart advertising to allow reconnection
        BLEAdvertising* pAdv = s->getAdvertising();
        pAdv->start();
        Serial.println("🔄 BLE advertising restarted for reconnection");
    }
};

// BLE Characteristic Callbacks for receiving commands
class CaneCharacteristicCallbacks : public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic* pCharacteristic) {
        String command = pCharacteristic->getValue().c_str();
        command.trim();
        if (command.length() > 0) {
            Serial.printf("📱 Received BLE command: %s\n", command.c_str());
            BLEManager::processCommand(command);
        }
    }
};

// Helper functions - optimized for high-speed operation
float BLEManager::rf(float a, float b) { 
    return a + (esp_random() / (float)UINT32_MAX) * (b - a); 
}

int BLEManager::ri(int a, int b) { 
    return a + (esp_random() % (b - a + 1)); 
}

// Immediate transmission (blocking) - for critical data only
void BLEManager::sendLineImmediate(const char* fmt, ...) {
    if (!clientConnected || !pChr) return;
    
    char ln[64];
    va_list args;
    va_start(args, fmt);
    int n = vsnprintf(ln, sizeof(ln), fmt, args);
    va_end(args);
    
    if (n > 0 && n < 62) {
        ln[n] = '\n';
        ln[n+1] = '\0';
        
        pChr->setValue((uint8_t*)ln, n+1);
        pChr->notify();
        // No delay - immediate transmission
    }
}

void BLEManager::sendLargeData(const char* data) {
    if (!clientConnected || !pChr) return;
    
    int dataLen = strlen(data);
    const int chunkSize = 60; // Leave room for newline
    
    Serial.printf("🔧 [DEBUG] sendLargeData: Sending %d bytes in chunks of %d\n", dataLen, chunkSize);
    
    for (int i = 0; i < dataLen; i += chunkSize) {
        char chunk[64];
        int remainingBytes = dataLen - i;
        int currentChunkSize = (remainingBytes > chunkSize) ? chunkSize : remainingBytes;
        
        strncpy(chunk, data + i, currentChunkSize);
        chunk[currentChunkSize] = '\n';
        chunk[currentChunkSize + 1] = '\0';
        
        Serial.printf("🔧 [DEBUG] Sending chunk %d: %d bytes\n", (i/chunkSize) + 1, currentChunkSize + 1);
        
        pChr->setValue((uint8_t*)chunk, currentChunkSize + 1);
        pChr->notify();
        
        // Small delay between chunks to ensure reliable transmission
        delay(10);
    }
    
    Serial.println("🔧 [DEBUG] Large data transmission completed");
}

// Queue-based transmission (non-blocking) - for regular sensor data
void BLEManager::queueBLEMessage(const char* fmt, ...) {
    if (!bleQueue) return;
    
    BLEPacket packet;
    va_list args;
    va_start(args, fmt);
    int n = vsnprintf(packet.data, sizeof(packet.data) - 1, fmt, args);
    va_end(args);
    
    if (n > 0 && n < 62) {
        packet.data[n] = '\n';
        packet.data[n+1] = '\0';
        packet.length = n + 1;
        packet.timestamp = millis();
        
        totalPackets++;
        
        // Non-blocking queue send
        if (xQueueSend(bleQueue, &packet, 0) != pdTRUE) {
            droppedPackets++;
            // Queue full - drop oldest packet and try again
            BLEPacket dummy;
            xQueueReceive(bleQueue, &dummy, 0);
            xQueueSend(bleQueue, &packet, 0);
        }
    }
}

// Process commands received from the app
void BLEManager::processCommand(const String& command) {
    // Forward the command to the main serial command processor
    extern void processSerialCommand(const String& command);
    processSerialCommand(command);
    
    // Send acknowledgment back to app
    sendLineImmediate("CMD_ACK:%s", command.c_str());
}

// FreeRTOS task for BLE transmission
void BLEManager::bleTransmissionTask(void* parameter) {
    BLEPacket packet;
    TickType_t lastTransmission = 0;
    const TickType_t minInterval = pdMS_TO_TICKS(10); // Minimum 10ms between transmissions
    
    Serial.println("🚀 BLE transmission task started on Core 0");
    
    while (true) {
        // Wait for packet in queue
        if (xQueueReceive(bleQueue, &packet, portMAX_DELAY) == pdTRUE) {
            // Check if client is still connected
            if (xSemaphoreTake(bleMutex, pdMS_TO_TICKS(10)) == pdTRUE) {
                bool connected = clientConnected;
                xSemaphoreGive(bleMutex);
                
                if (connected && transmitPacket(packet)) {
                    // Throttle transmission rate to prevent BLE congestion
                    TickType_t now = xTaskGetTickCount();
                    TickType_t elapsed = now - lastTransmission;
                    if (elapsed < minInterval) {
                        vTaskDelay(minInterval - elapsed);
                    }
                    lastTransmission = xTaskGetTickCount();
                } else {
                    // Client disconnected or transmission failed
                    vTaskDelay(pdMS_TO_TICKS(100));
                }
            }
        }
    }
}

// Internal packet transmission
bool BLEManager::transmitPacket(const BLEPacket& packet) {
    if (!pChr || !clientConnected) return false;
    
    try {
        pChr->setValue((uint8_t*)packet.data, packet.length);
        pChr->notify();
        return true;
    } catch (...) {
        Serial.println("⚠️ BLE transmission error");
        return false;
    }
}

// Flush queue (called on disconnect)
void BLEManager::flushQueue() {
    if (!bleQueue) return;
    
    BLEPacket dummy;
    while (xQueueReceive(bleQueue, &dummy, 0) == pdTRUE) {
        // Empty the queue
    }
    Serial.println("🗑️ BLE queue flushed");
}

void BLEManager::init() {
    Serial.println("🔧 Initializing high-speed BLE system...");
    
    // Create FreeRTOS components
    bleQueue = xQueueCreate(BLE_QUEUE_SIZE, sizeof(BLEPacket));
    bleMutex = xSemaphoreCreateMutex();
    
    if (!bleQueue || !bleMutex) {
        Serial.println("❌ Failed to create FreeRTOS components");
        return;
    }
    
    // Initialize BLE
    BLEDevice::init("SmartCane-ESP32-HS"); // HS = High Speed
    pServer = BLEDevice::createServer();
    pServer->setCallbacks(new CaneServerCallbacks());
    
    BLEService* pService = pServer->createService(SVC_UUID);
    
    // RX Characteristic (ESP32 sends data to app)
    pChr = pService->createCharacteristic(
        CHR_UUID,
        BLECharacteristic::PROPERTY_READ | 
        BLECharacteristic::PROPERTY_NOTIFY
    );
    pChr->addDescriptor(new BLE2902());
    
    // TX Characteristic (ESP32 receives commands from app)
    pTxChr = pService->createCharacteristic(
        CHR_TX_UUID,
        BLECharacteristic::PROPERTY_WRITE |
        BLECharacteristic::PROPERTY_WRITE_NR
    );
    pTxChr->setCallbacks(new CaneCharacteristicCallbacks());
    
    Serial.println("✅ BLE characteristics configured:");
    Serial.println("   📤 RX (ESP32→App): " CHR_UUID);
    Serial.println("   📥 TX (App→ESP32): " CHR_TX_UUID);
    
    pService->start();
    
    BLEAdvertising* pAdv = pServer->getAdvertising();
    pAdv->addServiceUUID(SVC_UUID);
    pAdv->setScanResponse(true);
    pAdv->start();
    
    // Create BLE transmission task on Core 0
    xTaskCreatePinnedToCore(
        bleTransmissionTask,
        "BLE_TX_Task",
        BLE_TASK_STACK_SIZE,
        nullptr,
        BLE_TASK_PRIORITY,
        &bleTaskHandle,
        BLE_TASK_CORE
    );
    
    Serial.println("✅ High-speed BLE system initialized");
    Serial.println("📊 Sensor loop will run at maximum speed");
}

void BLEManager::cleanup() {
    if (bleTaskHandle) {
        vTaskDelete(bleTaskHandle);
        bleTaskHandle = nullptr;
    }
    
    if (bleQueue) {
        vQueueDelete(bleQueue);
        bleQueue = nullptr;
    }
    
    if (bleMutex) {
        vSemaphoreDelete(bleMutex);
        bleMutex = nullptr;
    }
}

// Legacy function - now uses queue system
void BLEManager::sendBLEData(const SensorData& s) {
    if (!clientConnected) return;

    // Queue all sensor data without blocking
    queueBLEMessage("TEMP:%.1f", s.temperature);
    queueBLEMessage("HUM:%.1f", s.humidity);
    queueBLEMessage("OBST:%d", (int)s.tofDistance);
    
    // Send battery level (simulated for now)
    static int batteryLevel = 4200;
    batteryLevel = constrain(batteryLevel - 1, 3200, 4200);
    queueBLEMessage("BATTmV:%d", batteryLevel);
    
    queueBLEMessage("RFID:%d", s.currentRoom);
    queueBLEMessage("STEP:%d", (int)s.dailySteps);
    queueBLEMessage("LUX:%d", (int)s.lightLux);
    
    // Fall detection from IMU data
    bool fallDetected = (abs(s.imuPitch) > 45 || abs(s.imuRoll) > 45);
    queueBLEMessage("FALL:%d", fallDetected ? 1 : 0);
    
    // GPS coordinates
    if (s.gpsLat != 0 && s.gpsLon != 0) {
        queueBLEMessage("GPS:%.2f,%.2f", s.gpsLat, s.gpsLon);
    }
}

// Optimized version - batches multiple readings
// Send step data only when count changes
void BLEManager::sendStepUpdateIfChanged(uint32_t currentStepCount) {
    if (!clientConnected) return;
    
    if (currentStepCount != lastSentStepCount) {
        queueBLEMessage("STEP:%d", (int)currentStepCount);
        lastSentStepCount = currentStepCount;
        Serial.printf("📊 Step update sent: %d\n", (int)currentStepCount);
    }
}

void BLEManager::sendBLEDataFast(const SensorData& s) {
    if (!clientConnected) return;
    
    // Batch critical sensor data into fewer packets
    queueBLEMessage("SENSORS:%.1f,%.1f,%d,%d", 
                   s.temperature, s.humidity, (int)s.tofDistance, (int)s.lightLux);
    
    // Send motion data without step count (steps sent separately when changed)
    queueBLEMessage("MOTION:%.1f,%.1f,%.1f", 
                   s.imuPitch, s.imuRoll, s.imuYaw);
    
    // Send ToF mode and radar data if in radar mode
    queueBLEMessage("TOFMODE:%d", ToF_isRadarMode() ? 1 : 0);
    
    if (ToF_isRadarMode()) {
        uint16_t* scanData = ToF_getScanData();
        if (scanData) {
            // Send radar data in chunks of 10 angles per message to fit BLE packet size
            for (int chunk = 0; chunk < 19; chunk++) { // 19 chunks of 10 angles each (190 total, but we only have 181)
                int startAngle = chunk * 10;
                int endAngle = min(startAngle + 9, 180); // Don't exceed 180°
                
                if (startAngle <= 180) {
                    queueBLEMessage("RADAR%d:%d,%d,%d,%d,%d,%d,%d,%d,%d,%d", 
                                   chunk,
                                   scanData[startAngle], scanData[startAngle+1], scanData[startAngle+2], 
                                   scanData[startAngle+3], scanData[startAngle+4], scanData[startAngle+5],
                                   scanData[startAngle+6], scanData[startAngle+7], scanData[startAngle+8],
                                   (endAngle >= startAngle+9) ? scanData[startAngle+9] : 0);
                }
            }
        }
    }
    
    if (s.gpsLat != 0 && s.gpsLon != 0) {
        queueBLEMessage("GPS:%.6f,%.6f", s.gpsLat, s.gpsLon);
    }
}

bool BLEManager::isConnected() {
    if (xSemaphoreTake(bleMutex, pdMS_TO_TICKS(10)) == pdTRUE) {
        bool connected = clientConnected;
        xSemaphoreGive(bleMutex);
        return connected;
    }
    return false;
}

uint32_t BLEManager::getQueuedPackets() {
    return bleQueue ? uxQueueMessagesWaiting(bleQueue) : 0;
}

uint32_t BLEManager::getDroppedPackets() {
    return droppedPackets;
}

void BLEManager::sendRadarLiveData(int angle, int distance) {
    if (!clientConnected) return;
    queueBLEMessage("RADAR_LIVE:%d,%d", angle, distance);
}

void BLEManager::printStats() {
  Serial.printf("Queue: %d/%d, Total: %lu, Dropped: %lu\n", 
    uxQueueMessagesWaiting(bleQueue), BLE_QUEUE_SIZE, totalPackets, droppedPackets);
}