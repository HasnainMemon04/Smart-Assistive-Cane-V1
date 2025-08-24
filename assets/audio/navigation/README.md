# Navigation Audio Files

This directory contains navigation and guidance audio files for the Smart Assistive Cane.

## Purpose

Navigation audio files include:
- Direction announcements (left, right, forward, back)
- Distance measurements
- Obstacle detection alerts
- GPS navigation instructions
- RFID zone announcements
- Path guidance feedback

## File Format

Audio files should be in WAV format, 16-bit, 22kHz for optimal ESP32 compatibility.

## Usage

These files are loaded by the AudioFeedbackManager and played during navigation assistance and obstacle detection.