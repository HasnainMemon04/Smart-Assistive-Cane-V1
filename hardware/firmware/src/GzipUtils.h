#pragma once
#ifndef GZIP_UTILS_H
#define GZIP_UTILS_H

#include <Arduino.h>
#include "SDCardManager.h"

// Gzip compression utilities
class GzipUtils {
public:
    // Compress a string and save to SD card
    static bool compressAndSave(const String& content, const char* outputPath);
    
    // Check if a gzipped version exists
    static bool gzipExists(const char* originalPath);
    
    // Get the gzipped file path
    static String getGzipPath(const char* originalPath);
    

    
private:
    // Simple gzip compression implementation
    static size_t compress(const uint8_t* input, size_t inputSize, uint8_t* output, size_t outputSize);
    
    // CRC32 calculation for gzip
    static uint32_t crc32(const uint8_t* data, size_t length);
    
    // Gzip header and footer
    static const uint8_t GZIP_HEADER[];
    static const size_t GZIP_HEADER_SIZE;
};

#endif // GZIP_UTILS_H