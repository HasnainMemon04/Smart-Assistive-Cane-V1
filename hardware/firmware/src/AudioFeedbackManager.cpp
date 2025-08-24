#include "AudioFeedbackManager.h"
#include "SDCardManager.h"

// Global instance definition
AudioFeedbackManager audioManager;

AudioFeedbackManager::AudioFeedbackManager() {
    audioInitialized = false;
    sdCardAvailable = false;
    lastPlayTime = 0;
    queueHead = 0;
    queueTail = 0;
    queueSize = 0;
    mappingCount = 0;
}

AudioFeedbackManager::~AudioFeedbackManager() {
    deinitializeI2S();
}

bool AudioFeedbackManager::initialize() {
    Serial.println("[AudioManager] Initializing audio system...");
    
    // Check SD card availability
    if (!SDCardManager::isInitialized() {
        Serial.println("[AudioManager] SD card not available");
        sdCardAvailable = false;
        return false;
    }
    
    sdCardAvailable = true;
    Serial.println("[AudioManager] SD card available");
    
    // Initialize I2S for MAX98357A
    if (!initializeI2S()) {
        Serial.println("[AudioManager] Failed to initialize I2S");
        return false;
    }
    
    // Initialize serial statement mappings
    initializeSerialMappings();
    
    // Enable amplifier
    enableAmplifier();
    
    audioInitialized = true;
    Serial.println("[AudioManager] Audio system initialized successfully");
    
    return true;
}

bool AudioFeedbackManager::isAudioReady() const {
    return audioInitialized && sdCardAvailable;
}

void AudioFeedbackManager::testAudioSystem() {
    if (!isAudioReady()) {
        Serial.println("[AudioManager] Audio system not ready for testing");
        return;
    }
    
    Serial.println("[AudioManager] Testing audio system...");
    
    // Test with a simple audio file
    if (fileExists("/audio/critical/system_ready.wav")) {
        playAudioFile("/audio/critical/system_ready.wav");
    } else if (fileExists("/audio/digits/num1.wav")) {
        playAudioFile("/audio/digits/num1.wav");
    } else {
        Serial.println("[AudioManager] No test audio files found");
    }
}

bool AudioFeedbackManager::initializeI2S() {
    // Configure I2S
    i2s_config_t i2s_config = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),
        .sample_rate = AUDIO_SAMPLE_RATE,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
        .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
        .communication_format = I2S_COMM_FORMAT_STAND_I2S,
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
        .dma_buf_count = 8,
        .dma_buf_len = 1024,
        .use_apll = false,
        .tx_desc_auto_clear = true,
        .fixed_mclk = 0
    };
    
    // Pin configuration
    i2s_pin_config_t pin_config = {
        .bck_io_num = I2S_BCLK_PIN,
        .ws_io_num = I2S_LRCLK_PIN,
        .data_out_num = I2S_DIN_PIN,
        .data_in_num = I2S_PIN_NO_CHANGE
    };
    
    // Install and start I2S driver
    esp_err_t err = i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
    if (err != ESP_OK) {
        Serial.printf("[AudioManager] Failed to install I2S driver: %s\n", esp_err_to_name(err));
        return false;
    }
    
    err = i2s_set_pin(I2S_NUM_0, &pin_config);
    if (err != ESP_OK) {
        Serial.printf("[AudioManager] Failed to set I2S pins: %s\n", esp_err_to_name(err));
        i2s_driver_uninstall(I2S_NUM_0);
        return false;
    }
    
    Serial.println("[AudioManager] I2S initialized successfully");
    return true;
}

void AudioFeedbackManager::deinitializeI2S() {
    if (audioInitialized) {
        disableAmplifier();
        i2s_driver_uninstall(I2S_NUM_0);
        audioInitialized = false;
    }
}

void AudioFeedbackManager::enableAmplifier() {
    pinMode(I2S_SD_PIN, OUTPUT);
    digitalWrite(I2S_SD_PIN, HIGH); // Enable amplifier (active low, so HIGH enables)
    delay(10); // Small delay for amplifier to stabilize
}

void AudioFeedbackManager::disableAmplifier() {
    digitalWrite(I2S_SD_PIN, LOW); // Disable amplifier
}

bool AudioFeedbackManager::fileExists(const String& path) {
    if (!sdCardAvailable) return false;
    
    File file = SD.open(path);
    if (file) {
        file.close();
        return true;
    }
    return false;
}

bool AudioFeedbackManager::playAudioFile(const String& filePath) {
    if (!isAudioReady()) {
        Serial.println("[AudioManager] Audio system not ready");
        return false;
    }
    
    // Check minimum interval between plays
    unsigned long currentTime = millis();
    if (currentTime - lastPlayTime < MIN_PLAY_INTERVAL) {
        Serial.println("[AudioManager] Audio play rate limited");
        return false;
    }
    
    if (!fileExists(filePath)) {
        Serial.printf("[AudioManager] Audio file not found: %s\n", filePath.c_str());
        return false;
    }
    
    bool success = loadAndPlayWAV(filePath);
    if (success) {
        lastPlayTime = currentTime;
        Serial.printf("[AudioManager] Played: %s\n", filePath.c_str());
    }
    
    return success;
}

bool AudioFeedbackManager::loadAndPlayWAV(const String& filePath) {
    File audioFile = SD.open(filePath);
    if (!audioFile) {
        Serial.printf("[AudioManager] Failed to open: %s\n", filePath.c_str());
        return false;
    }
    
    uint32_t dataSize;
    uint32_t sampleRate;
    
    if (!parseWAVHeader(audioFile, dataSize, sampleRate)) {
        Serial.println("[AudioManager] Invalid WAV file format");
        audioFile.close();
        return false;
    }
    
    // Adjust I2S sample rate if needed
    if (sampleRate != AUDIO_SAMPLE_RATE) {
        i2s_set_sample_rates(I2S_NUM_0, sampleRate);
    }
    
    playAudioData(audioFile, dataSize);
    
    // Reset to default sample rate
    if (sampleRate != AUDIO_SAMPLE_RATE) {
        i2s_set_sample_rates(I2S_NUM_0, AUDIO_SAMPLE_RATE);
    }
    
    audioFile.close();
    return true;
}

bool AudioFeedbackManager::parseWAVHeader(File& file, uint32_t& dataSize, uint32_t& sampleRate) {
    // Simple WAV header parsing
    file.seek(0);
    
    char header[44];
    if (file.read((uint8_t*)header, 44) != 44) {
        return false;
    }
    
    // Check RIFF header
    if (strncmp(header, "RIFF", 4) != 0) {
        return false;
    }
    
    // Check WAVE format
    if (strncmp(header + 8, "WAVE", 4) != 0) {
        return false;
    }
    
    // Extract sample rate (bytes 24-27)
    sampleRate = *((uint32_t*)(header + 24));
    
    // Find data chunk
    file.seek(36);
    char chunkHeader[8];
    while (file.available()) {
        if (file.read((uint8_t*)chunkHeader, 8) != 8) {
            return false;
        }
        
        if (strncmp(chunkHeader, "data", 4) == 0) {
            dataSize = *((uint32_t*)(chunkHeader + 4));
            return true;
        }
        
        // Skip this chunk
        uint32_t chunkSize = *((uint32_t*)(chunkHeader + 4));
        file.seek(file.position() + chunkSize);
    }
    
    return false;
}

void AudioFeedbackManager::playAudioData(File& file, uint32_t dataSize) {
    const size_t bufferSize = 1024;
    uint8_t buffer[bufferSize];
    size_t bytesWritten;
    
    uint32_t totalBytesRead = 0;
    
    while (totalBytesRead < dataSize && file.available()) {
        size_t bytesToRead = min(bufferSize, dataSize - totalBytesRead);
        size_t bytesRead = file.read(buffer, bytesToRead);
        
        if (bytesRead == 0) break;
        
        // Write to I2S
        esp_err_t err = i2s_write(I2S_NUM_0, buffer, bytesRead, &bytesWritten, portMAX_DELAY);
        if (err != ESP_OK) {
            Serial.printf("[AudioManager] I2S write error: %s\n", esp_err_to_name(err));
            break;
        }
        
        totalBytesRead += bytesRead;
    }
    
    // Wait for I2S to finish playing
    delay(100);
}

void AudioFeedbackManager::announceSerialStatement(const String& statement) {
    if (!isAudioReady()) return;
    
    String audioFile = mapSerialStatementToAudio(statement);
    if (audioFile.length() > 0) {
        playAudioFile(audioFile);
    } else {
        Serial.printf("[AudioManager] No audio mapping for: %s\n", statement.c_str());
    }
}

String AudioFeedbackManager::mapSerialStatementToAudio(const String& statement) {
    String lowerStatement = statement;
    lowerStatement.toLowerCase();
    
    // Search through serial mappings
    for (int i = 0; i < mappingCount; i++) {
        String mappingStatement = serialMappings[i].originalStatement;
        mappingStatement.toLowerCase();
        
        if (lowerStatement.indexOf(mappingStatement) >= 0 || 
            mappingStatement.indexOf(lowerStatement) >= 0) {
            return String(SERIAL_AUDIO_PATH) + "/" + serialMappings[i].audioFile;
        }
    }
    
    return ""; // No mapping found
}

bool AudioFeedbackManager::playSerialStatementByNumber(int serialNumber) {
    String audioFile = String(SERIAL_AUDIO_PATH) + "/serial_" + 
                      String(serialNumber, DEC).c_str() + ".wav";
    return playAudioFile(audioFile);
}

void AudioFeedbackManager::announceDistanceReading(float distance) {
    if (!isAudioReady()) return;
    
    if (distance < 30.0) {
        playAudioFile("/audio/critical/obstacle_ahead.wav");
        delay(500);
    }
    
    // Announce the distance
    announceFloat(distance, 0);
    delay(300);
    announceUnit("centimeters");
}

void AudioFeedbackManager::announceTemperature(float temperature) {
    if (!isAudioReady()) return;
    
    announceFloat(temperature, 1);
    delay(300);
    announceUnit("celsius");
}

void AudioFeedbackManager::announceLightLevel(float lux, const String& environment) {
    if (!isAudioReady()) return;
    
    announceFloat(lux, 0);
    delay(300);
    announceUnit("lux");
    delay(500);
    
    // Announce environment type
    if (environment == "Dark") {
        playAudioFile("/audio/environmental/dark_environment.wav");
    } else if (environment == "Dim") {
        playAudioFile("/audio/environmental/dim_light.wav");
    } else if (environment == "Bright") {
        playAudioFile("/audio/environmental/bright_light.wav");
    }
}

void AudioFeedbackManager::announceGPSStatus(const SensorData& data) {
    if (!isAudioReady()) return;
    
    if (data.gpsSatellites > 0) {
        announceNumber(data.gpsSatellites);
        delay(300);
        playAudioFile("/audio/navigation/satellites_connected.wav");
    } else {
        playAudioFile("/audio/critical/gps_no_signal.wav");
    }
}

void AudioFeedbackManager::announceSystemReady() {
    if (!isAudioReady()) return;
    
    playAudioFile("/audio/critical/system_ready.wav");
}

void AudioFeedbackManager::announceNumber(int number) {
    if (!isAudioReady()) return;
    
    if (number < 0 || number > 999) return;
    
    if (number < 10) {
        String audioFile = String(DIGITS_AUDIO_PATH) + "/num" + String(number) + ".wav";
        playAudioFile(audioFile);
    } else if (number < 100) {
        int tens = number / 10;
        int ones = number % 10;
        
        String tensFile = String(DIGITS_AUDIO_PATH) + "/num" + String(tens * 10) + ".wav";
        playAudioFile(tensFile);
        delay(200);
        
        if (ones > 0) {
            String onesFile = String(DIGITS_AUDIO_PATH) + "/num" + String(ones) + ".wav";
            playAudioFile(onesFile);
        }
    } else {
        int hundreds = number / 100;
        int remainder = number % 100;
        
        String hundredsFile = String(DIGITS_AUDIO_PATH) + "/num" + String(hundreds) + ".wav";
        playAudioFile(hundredsFile);
        delay(200);
        playAudioFile(String(DIGITS_AUDIO_PATH) + "/hundred.wav");
        delay(200);
        
        if (remainder > 0) {
            announceNumber(remainder);
        }
    }
}

void AudioFeedbackManager::announceFloat(float value, int decimals) {
    if (!isAudioReady()) return;
    
    int intPart = (int)value;
    announceNumber(intPart);
    
    if (decimals > 0) {
        delay(200);
        playAudioFile(String(DIGITS_AUDIO_PATH) + "/point.wav");
        delay(200);
        
        float fracPart = value - intPart;
        int fracInt = (int)(fracPart * pow(10, decimals));
        announceNumber(fracInt);
    }
}

void AudioFeedbackManager::announceUnit(const String& unit) {
    if (!isAudioReady()) return;
    
    String audioFile = String(DIGITS_AUDIO_PATH) + "/" + unit + ".wav";
    playAudioFile(audioFile);
}

// Critical alerts
void AudioFeedbackManager::playObstacleAlert() {
    playAudioFile("/audio/critical/obstacle_ahead.wav");
}

void AudioFeedbackManager::playDistanceSensorFailed() {
    playAudioFile("/audio/critical/distance_sensor_failed.wav");
}

void AudioFeedbackManager::playLowBattery() {
    playAudioFile("/audio/critical/low_battery.wav");
}

void AudioFeedbackManager::playEmergencyAlert() {
    playAudioFile("/audio/critical/emergency_alert.wav");
}

// Environmental feedback
void AudioFeedbackManager::playWetSurface() {
    playAudioFile("/audio/environmental/wet_surface.wav");
}

void AudioFeedbackManager::playUnevenGround() {
    playAudioFile("/audio/environmental/uneven_ground.wav");
}

void AudioFeedbackManager::playStairsAhead() {
    playAudioFile("/audio/navigation/stairs_ahead.wav");
}

// Navigation feedback
void AudioFeedbackManager::playTurnLeft() {
    playAudioFile("/audio/navigation/turn_left.wav");
}

void AudioFeedbackManager::playTurnRight() {
    playAudioFile("/audio/navigation/turn_right.wav");
}

void AudioFeedbackManager::playGoStraight() {
    playAudioFile("/audio/navigation/go_straight.wav");
}

void AudioFeedbackManager::playDestinationReached() {
    playAudioFile("/audio/navigation/destination_reached.wav");
}

void AudioFeedbackManager::announceSignificantChanges(const SensorData& data) {
    // This method can be used to announce significant changes in sensor data
    // Implementation depends on specific requirements
}

void AudioFeedbackManager::processSensorChanges(const SensorData& current, const SensorData& previous) {
    // Process sensor changes and provide appropriate audio feedback
    // Implementation depends on specific requirements
}

// Initialize the comprehensive serial statement mappings
void AudioFeedbackManager::initializeSerialMappings() {
    mappingCount = 0;
    
    // Add comprehensive mappings based on audio_index.txt
    // System Status and Connectivity
    serialMappings[mappingCount++] = {"wifi connected", "serial_001.wav", 1};
    serialMappings[mappingCount++] = {"wifi connection failed", "serial_002.wav", 2};
    serialMappings[mappingCount++] = {"wifi status", "serial_003.wav", 3};
    serialMappings[mappingCount++] = {"connected to", "serial_004.wav", 4};
    serialMappings[mappingCount++] = {"signal strength", "serial_005.wav", 5};
    serialMappings[mappingCount++] = {"wifi functionality disabled", "serial_006.wav", 6};
    serialMappings[mappingCount++] = {"switching to offline mode", "serial_007.wav", 7};
    serialMappings[mappingCount++] = {"cloud functionality disabled", "serial_008.wav", 8};
    serialMappings[mappingCount++] = {"cloud connection status", "serial_009.wav", 9};
    serialMappings[mappingCount++] = {"status: offline mode", "serial_011.wav", 11};
    serialMappings[mappingCount++] = {"resetting cloud connection", "serial_012.wav", 12};
    
    // Light Sensor Calibration
    serialMappings[mappingCount++] = {"light sensor calibration reset", "serial_014.wav", 14};
    serialMappings[mappingCount++] = {"addlocation command received", "serial_016.wav", 16};
    serialMappings[mappingCount++] = {"light sensor calibration", "serial_017.wav", 17};
    serialMappings[mappingCount++] = {"calibration complete", "serial_018.wav", 18};
    serialMappings[mappingCount++] = {"calibration in progress", "serial_019.wav", 19};
    serialMappings[mappingCount++] = {"calibration failed", "serial_020.wav", 20};
    
    // Vibration Motor Testing
    serialMappings[mappingCount++] = {"testing vibration motors", "serial_021.wav", 21};
    serialMappings[mappingCount++] = {"vibration test complete", "serial_022.wav", 22};
    serialMappings[mappingCount++] = {"vibration motor 1 test", "serial_023.wav", 23};
    serialMappings[mappingCount++] = {"vibration motor 2 test", "serial_024.wav", 24};
    
    // BLE Status
    serialMappings[mappingCount++] = {"ble connected", "serial_025.wav", 25};
    serialMappings[mappingCount++] = {"ble disconnected", "serial_026.wav", 26};
    serialMappings[mappingCount++] = {"ble status", "serial_027.wav", 27};
    serialMappings[mappingCount++] = {"ble device name", "serial_028.wav", 28};
    serialMappings[mappingCount++] = {"ble mac address", "serial_029.wav", 29};
    
    // ToF Mode
    serialMappings[mappingCount++] = {"tof mode", "serial_030.wav", 30};
    serialMappings[mappingCount++] = {"radar mode", "serial_031.wav", 31};
    serialMappings[mappingCount++] = {"simple mode", "serial_032.wav", 32};
    serialMappings[mappingCount++] = {"distance reading", "serial_033.wav", 33};
    
    // GPS Functionality
    serialMappings[mappingCount++] = {"gps status", "serial_034.wav", 34};
    serialMappings[mappingCount++] = {"gps connected", "serial_035.wav", 35};
    serialMappings[mappingCount++] = {"gps disconnected", "serial_036.wav", 36};
    serialMappings[mappingCount++] = {"satellites", "serial_037.wav", 37};
    serialMappings[mappingCount++] = {"gps coordinates", "serial_038.wav", 38};
    
    // Sensor Reset
    serialMappings[mappingCount++] = {"sensor reset", "serial_039.wav", 39};
    serialMappings[mappingCount++] = {"all sensors reset", "serial_040.wav", 40};
    serialMappings[mappingCount++] = {"imu reset", "serial_041.wav", 41};
    serialMappings[mappingCount++] = {"tof reset", "serial_042.wav", 42};
    
    // Feedback Mode
    serialMappings[mappingCount++] = {"feedback mode", "serial_043.wav", 43};
    serialMappings[mappingCount++] = {"buzzer mode", "serial_044.wav", 44};
    serialMappings[mappingCount++] = {"vibration mode", "serial_045.wav", 45};
    serialMappings[mappingCount++] = {"both mode", "serial_046.wav", 46};
    
    // System Status
    serialMappings[mappingCount++] = {"smart cane system status", "serial_207.wav", 207};
    serialMappings[mappingCount++] = {"smart assistive cane v1", "serial_208.wav", 208};
    serialMappings[mappingCount++] = {"uptime", "serial_209.wav", 209};
    serialMappings[mappingCount++] = {"free heap", "serial_210.wav", 210};
    serialMappings[mappingCount++] = {"ble connected", "serial_211.wav", 211};
    serialMappings[mappingCount++] = {"audio system", "serial_212.wav", 212};
    serialMappings[mappingCount++] = {"feedback mode", "serial_213.wav", 213};
    serialMappings[mappingCount++] = {"tof mode", "serial_214.wav", 214};
    serialMappings[mappingCount++] = {"current room", "serial_215.wav", 215};
    serialMappings[mappingCount++] = {"daily steps", "serial_216.wav", 216};
    serialMappings[mappingCount++] = {"temperature", "serial_217.wav", 217};
    serialMappings[mappingCount++] = {"humidity", "serial_218.wav", 218};
    serialMappings[mappingCount++] = {"light", "serial_219.wav", 219};
    serialMappings[mappingCount++] = {"distance", "serial_220.wav", 220};
    serialMappings[mappingCount++] = {"gps", "serial_221.wav", 221};
    
    // Commands and Help
    serialMappings[mappingCount++] = {"available commands", "serial_222.wav", 222};
    serialMappings[mappingCount++] = {"wifi", "serial_223.wav", 223};
    serialMappings[mappingCount++] = {"reconnect", "serial_224.wav", 224};
    serialMappings[mappingCount++] = {"resetwifi", "serial_225.wav", 225};
    serialMappings[mappingCount++] = {"offline", "serial_226.wav", 226};
    serialMappings[mappingCount++] = {"resetcal", "serial_227.wav", 227};
    serialMappings[mappingCount++] = {"vibrate", "serial_228.wav", 228};
    serialMappings[mappingCount++] = {"rooms", "serial_229.wav", 229};
    serialMappings[mappingCount++] = {"clearrooms", "serial_230.wav", 230};
    serialMappings[mappingCount++] = {"autoreg", "serial_231.wav", 231};
    serialMappings[mappingCount++] = {"addlocation", "serial_232.wav", 232};
    serialMappings[mappingCount++] = {"addroom", "serial_233.wav", 233};
    
    // Button Controls
    serialMappings[mappingCount++] = {"button controls", "serial_234.wav", 234};
    serialMappings[mappingCount++] = {"hold btn1", "serial_235.wav", 235};
    serialMappings[mappingCount++] = {"hold btn2", "serial_236.wav", 236};
    serialMappings[mappingCount++] = {"hold btn3", "serial_237.wav", 237};
    serialMappings[mappingCount++] = {"btn3 short press", "serial_238.wav", 238};
    serialMappings[mappingCount++] = {"btn3 hold 3s", "serial_239.wav", 239};
    serialMappings[mappingCount++] = {"btn3 hold 10s", "serial_240.wav", 240};
    
    // ToF Sensor
    serialMappings[mappingCount++] = {"tof sensor reset", "serial_243.wav", 243};
    serialMappings[mappingCount++] = {"tof sensor reset complete", "serial_244.wav", 244};
    serialMappings[mappingCount++] = {"failed to initialize sensor", "serial_245.wav", 245};
    serialMappings[mappingCount++] = {"vl53l1x smart cane system initialized", "serial_246.wav", 246};
    serialMappings[mappingCount++] = {"modes: simple / radar", "serial_248.wav", 248};
    serialMappings[mappingCount++] = {"tof sensor appears stuck", "serial_249.wav", 249};
    
    // Radar Mode
    serialMappings[mappingCount++] = {"switching to radar mode", "serial_269.wav", 269};
    serialMappings[mappingCount++] = {"switching to simple mode", "serial_270.wav", 270};
    serialMappings[mappingCount++] = {"real-time radar task started", "serial_271.wav", 271};
    serialMappings[mappingCount++] = {"radar sweep completed", "serial_272.wav", 272};
    serialMappings[mappingCount++] = {"real-time radar scanning started", "serial_273.wav", 273};
    serialMappings[mappingCount++] = {"radar scan results", "serial_274.wav", 274};
    
    // Diagnostics
    serialMappings[mappingCount++] = {"running full system diagnostic", "serial_202.wav", 202};
    serialMappings[mappingCount++] = {"diagnostic complete", "serial_203.wav", 203};
    serialMappings[mappingCount++] = {"checking sensor connectivity", "serial_204.wav", 204};
    serialMappings[mappingCount++] = {"all sensors connected successfully", "serial_205.wav", 205};
    serialMappings[mappingCount++] = {"some sensors failed connectivity check", "serial_206.wav", 206};
    
    // Audio System
    serialMappings[mappingCount++] = {"testing audio system", "serial_001.wav", 1};
    serialMappings[mappingCount++] = {"audio system status", "serial_002.wav", 2};
    serialMappings[mappingCount++] = {"announcing current sensor readings", "serial_003.wav", 3};
    serialMappings[mappingCount++] = {"system status", "serial_207.wav", 207};
    serialMappings[mappingCount++] = {"rebooting system", "serial_004.wav", 4};
    serialMappings[mappingCount++] = {"help displayed", "serial_222.wav", 222};
    serialMappings[mappingCount++] = {"smart cane esp32-s3-n16r8 starting", "serial_208.wav", 208};
    
    Serial.printf("[AudioManager] Initialized %d serial statement mappings\n", mappingCount);
}