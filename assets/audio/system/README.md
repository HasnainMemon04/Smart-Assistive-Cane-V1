# System Audio Files

This directory contains system-level audio files for the Smart Assistive Cane.

## Purpose

System audio files include:
- Startup/boot sounds
- Shutdown notifications
- Connection status (Bluetooth, WiFi)
- System mode changes
- Configuration confirmations
- Error beeps and alerts
- Battery status announcements

## File Format

Audio files should be in WAV format, 16-bit, 22kHz for optimal ESP32 compatibility.

## Usage

These files are loaded by the AudioFeedbackManager and played during system operations and status changes.