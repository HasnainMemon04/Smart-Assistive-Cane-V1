# Critical Audio Files

This directory contains critical audio feedback files for the Smart Assistive Cane.

## Purpose

Critical audio files include:
- Emergency alerts
- Safety warnings
- System failure notifications
- Battery low alerts

## File Format

Audio files should be in WAV format, 16-bit, 22kHz for optimal ESP32 compatibility.

## Usage

These files are loaded by the AudioFeedbackManager and played during critical system events.