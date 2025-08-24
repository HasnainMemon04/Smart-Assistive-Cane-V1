# Smart Cane Audio System

This directory contains the audio files for the Smart Cane's MAX98357A I2S audio feedback system.

## Directory Structure

```
audio/
├── serial/          # Serial statement audio files (serial_XXX.wav)
├── digits/          # Number and unit pronunciation files
├── critical/        # Critical alerts and system messages
├── environmental/   # Environmental feedback sounds
├── navigation/      # Navigation guidance sounds
└── README.md        # This file
```

## Audio File Requirements

### Format Specifications
- **Format**: WAV (uncompressed)
- **Sample Rate**: 16kHz (configurable in AudioFeedbackManager.h)
- **Bit Depth**: 16-bit
- **Channels**: Mono (single channel)
- **Encoding**: PCM

### File Naming Conventions

#### Serial Statement Files (`/serial/`)
- Format: `serial_XXX.wav` where XXX is a 3-digit number (001-999)
- Example: `serial_001.wav`, `serial_025.wav`, `serial_207.wav`

#### Digit Files (`/digits/`)
- Numbers 0-9: `num0.wav` to `num9.wav`
- Tens: `num10.wav`, `num20.wav`, `num30.wav`, etc.
- Special: `hundred.wav`, `point.wav` (for decimal point)
- Units: `centimeters.wav`, `celsius.wav`, `lux.wav`, `meters.wav`, etc.

#### Critical Alert Files (`/critical/`)
- `system_ready.wav` - System initialization complete
- `obstacle_ahead.wav` - Immediate obstacle warning
- `distance_sensor_failed.wav` - ToF sensor malfunction
- `low_battery.wav` - Battery level warning
- `emergency_alert.wav` - Emergency situation
- `gps_no_signal.wav` - GPS signal lost

#### Environmental Files (`/environmental/`)
- `dark_environment.wav` - Low light conditions
- `dim_light.wav` - Moderate lighting
- `bright_light.wav` - High light conditions
- `wet_surface.wav` - Wet surface detection
- `uneven_ground.wav` - Uneven terrain warning

#### Navigation Files (`/navigation/`)
- `turn_left.wav` - Left turn instruction
- `turn_right.wav` - Right turn instruction
- `go_straight.wav` - Continue straight
- `stairs_ahead.wav` - Stairs detection
- `destination_reached.wav` - Navigation complete
- `satellites_connected.wav` - GPS satellites available

## Serial Statement Mappings

The AudioFeedbackManager includes comprehensive mappings between serial print statements and their corresponding audio files. Key mappings include:

### System Status (serial_207.wav - serial_221.wav)
- System status display
- Sensor readings announcements
- Connectivity status
- Hardware diagnostics

### Commands and Help (serial_222.wav - serial_240.wav)
- Available commands listing
- Button control instructions
- Feature explanations

### Sensor Operations (serial_001.wav - serial_050.wav)
- WiFi connectivity
- BLE status
- GPS functionality
- Sensor calibration
- Feedback mode changes

### Diagnostics (serial_202.wav - serial_206.wav)
- System diagnostic procedures
- Sensor connectivity checks
- Hardware validation

## Audio File Creation Guidelines

### Recording Tips
1. **Clear Speech**: Use clear, slow pronunciation
2. **Consistent Volume**: Maintain uniform audio levels
3. **Minimal Background Noise**: Record in quiet environment
4. **Professional Voice**: Consider using text-to-speech or professional recording

### Processing Steps
1. Record or generate audio at high quality
2. Convert to 16kHz, 16-bit, mono WAV format
3. Normalize audio levels
4. Remove silence from beginning and end
5. Test playback on the device

### Recommended Tools
- **Audacity** (free, cross-platform)
- **FFmpeg** (command-line conversion)
- **Text-to-Speech services** (Google TTS, Amazon Polly, etc.)

### FFmpeg Conversion Example
```bash
ffmpeg -i input.wav -ar 16000 -ac 1 -sample_fmt s16 output.wav
```

## Implementation Details

### AudioFeedbackManager Class
The `AudioFeedbackManager` class handles:
- I2S initialization for MAX98357A amplifier
- WAV file parsing and playback
- Serial statement to audio file mapping
- Audio queue management
- Volume and timing control

### Hardware Configuration
- **I2S BCLK**: Pin 26
- **I2S LRCLK**: Pin 25
- **I2S DIN**: Pin 22
- **Amplifier Enable**: Pin 21

### Usage Examples

```cpp
// Initialize audio system
audioManager.initialize();

// Play specific audio file
audioManager.playAudioFile("/audio/critical/system_ready.wav");

// Announce serial statement
audioManager.announceSerialStatement("System status");

// Announce sensor readings
audioManager.announceDistanceReading(45.2);
audioManager.announceTemperature(23.5);
```

## File Size Considerations

### Storage Requirements
- Each second of 16kHz mono audio ≈ 32KB
- Target length: 1-3 seconds per file
- Total system: Approximately 500-1000 audio files
- Estimated storage: 50-100MB

### SD Card Organization
Store audio files on the SD card in the `/audio/` directory structure. Ensure the SD card has sufficient space and is properly formatted (FAT32 recommended).

## Testing and Validation

### Audio System Test
```cpp
// Test audio system functionality
audioManager.testAudioSystem();

// Check system status
if (audioManager.isAudioReady()) {
    Serial.println("Audio system ready");
} else {
    Serial.println("Audio system failed");
}
```

### Serial Commands
- `audiotest` - Test audio system
- `audiostatus` - Display audio system status
- `announce` - Announce current sensor readings

## Troubleshooting

### Common Issues
1. **No Audio Output**
   - Check I2S pin connections
   - Verify amplifier enable pin
   - Confirm SD card mounting

2. **Distorted Audio**
   - Check audio file format
   - Verify sample rate settings
   - Adjust volume levels

3. **Missing Audio Files**
   - Verify file paths and naming
   - Check SD card file system
   - Confirm file format compatibility

### Debug Output
Enable serial debugging to monitor audio system operations:
```
[AudioManager] Initializing audio system...
[AudioManager] SD card available
[AudioManager] I2S initialized successfully
[AudioManager] Audio system initialized successfully
[AudioManager] Played: /audio/critical/system_ready.wav
```

## Future Enhancements

### Planned Features
- Dynamic volume adjustment
- Audio compression support
- Multi-language support
- Voice customization
- Audio streaming capabilities

### Expansion Possibilities
- Bluetooth audio output
- Voice recognition input
- Custom audio generation
- Real-time audio processing

---

**Note**: This audio system is designed for accessibility and provides comprehensive audio feedback for visually impaired users. All audio content should be clear, informative, and helpful for navigation and system interaction.