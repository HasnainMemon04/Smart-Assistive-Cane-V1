# Environmental Audio Files

This directory contains environmental feedback audio files for the Smart Assistive Cane.

## Purpose

Environmental audio files include:
- Weather condition announcements
- Temperature readings
- Humidity level notifications
- Light level feedback
- Air quality alerts

## File Format

Audio files should be in WAV format, 16-bit, 22kHz for optimal ESP32 compatibility.

## Usage

These files are loaded by the AudioFeedbackManager and played when environmental sensor readings are announced to the user.