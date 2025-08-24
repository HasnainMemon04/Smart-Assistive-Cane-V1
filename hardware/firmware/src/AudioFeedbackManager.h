#ifndef AUDIOFEEDBACKMANAGER_H
#define AUDIOFEEDBACKMANAGER_H

#include <Arduino.h>
#include <SD.h>
#include <driver/i2s.h>
#include "SensorData.h"

// MAX98357A I2S Audio Amplifier Pin Definitions
#define I2S_BCLK_PIN    11  // Bit Clock
#define I2S_LRCLK_PIN   13  // Left/Right Clock (Word Select)
#define I2S_DIN_PIN     15  // Data Input
#define I2S_SD_PIN      22  // Shutdown (active low)

// Audio file paths
#define AUDIO_BASE_PATH "/audio"
#define SERIAL_AUDIO_PATH "/audio/serial_statements"
#define DIGITS_AUDIO_PATH "/audio/digits"
#define CRITICAL_AUDIO_PATH "/audio/critical"
#define ENVIRONMENTAL_AUDIO_PATH "/audio/environmental"
#define NAVIGATION_AUDIO_PATH "/audio/navigation"

// Audio sample rate and format
#define AUDIO_SAMPLE_RATE 16000
#define AUDIO_BITS_PER_SAMPLE 16
#define AUDIO_CHANNELS 1

class AudioFeedbackManager {
public:
    AudioFeedbackManager();
    ~AudioFeedbackManager();
    
    // Initialization and status
    bool initialize();
    bool isAudioReady() const;
    void testAudioSystem();
    
    // Core audio playback functions
    bool playAudioFile(const String& filePath);
    bool playSerialStatement(const String& serialStatement);
    bool playSerialStatementByNumber(int serialNumber);
    
    // Serial statement mapping and announcement
    void announceSerialStatement(const String& statement);
    String mapSerialStatementToAudio(const String& statement);
    
    // Sensor-specific announcements
    void announceDistanceReading(float distance);
    void announceTemperature(float temperature);
    void announceLightLevel(float lux, const String& environment);
    void announceGPSStatus(const SensorData& data);
    void announceSystemReady();
    void announceSignificantChanges(const SensorData& data);
    void processSensorChanges(const SensorData& current, const SensorData& previous);
    
    // Number and unit announcements
    void announceNumber(int number);
    void announceFloat(float value, int decimals = 1);
    void announceUnit(const String& unit);
    
    // Critical alerts
    void playObstacleAlert();
    void playDistanceSensorFailed();
    void playLowBattery();
    void playEmergencyAlert();
    
    // Environmental feedback
    void playWetSurface();
    void playUnevenGround();
    void playStairsAhead();
    
    // Navigation feedback
    void playTurnLeft();
    void playTurnRight();
    void playGoStraight();
    void playDestinationReached();
    
private:
    bool audioInitialized;
    bool sdCardAvailable;
    
    // I2S configuration
    bool initializeI2S();
    void deinitializeI2S();
    
    // Audio file handling
    bool loadAndPlayWAV(const String& filePath);
    bool parseWAVHeader(File& file, uint32_t& dataSize, uint32_t& sampleRate);
    void playAudioData(File& file, uint32_t dataSize);
    
    // Serial statement mapping
    struct SerialMapping {
        String originalStatement;
        String audioFile;
        int serialNumber;
    };
    
    // Audio file mapping functions
    String getSerialAudioFile(const String& statement);
    String getDigitAudioFile(int digit);
    String getUnitAudioFile(const String& unit);
    
    // Timing and queue management
    unsigned long lastPlayTime;
    static const unsigned long MIN_PLAY_INTERVAL = 500; // Minimum 500ms between audio plays
    
    // Audio queue for sequential playback
    struct AudioQueueItem {
        String filePath;
        unsigned long delay;
    };
    
    static const int MAX_QUEUE_SIZE = 10;
    AudioQueueItem audioQueue[MAX_QUEUE_SIZE];
    int queueHead;
    int queueTail;
    int queueSize;
    
    void addToQueue(const String& filePath, unsigned long delay = 0);
    void processQueue();
    void clearQueue();
    
    // Helper functions
    bool fileExists(const String& path);
    void enableAmplifier();
    void disableAmplifier();
    
    // Serial statement to audio file mapping table
    void initializeSerialMappings();
    static const int MAX_SERIAL_MAPPINGS = 400;
    SerialMapping serialMappings[MAX_SERIAL_MAPPINGS];
    int mappingCount;
};

// Global instance declaration
extern AudioFeedbackManager audioManager;

#endif // AUDIOFEEDBACKMANAGER_H