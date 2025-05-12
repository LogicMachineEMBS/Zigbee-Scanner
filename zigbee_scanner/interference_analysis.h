#ifndef ZIGBEE_SCANNER_INTERFERENCE_ANALYSIS_H
#define ZIGBEE_SCANNER_INTERFERENCE_ANALYSIS_H

#include "block_definitions.h"

// Structure for interference analysis
struct InterferenceAnalysis {
    uint8_t wifiOverlap;      // Degree of overlap with WiFi (0–100%)
    uint8_t noiseLevel;       // Noise level on the channel
    bool isConstant;          // Constant or periodic interference
    uint8_t interferenceType; // Type of interference (WiFi, Bluetooth, others)
    uint32_t lastDetected;    // Time of last detection
};

// Interference history for each channel
struct InterferenceHistory {
    uint8_t noiseHistory[10];    // Last 10 measurements
    uint8_t historyIndex;        // Current index in history
    bool isInitialized;          // Initialization flag
    uint32_t lastUpdateTime;     // Time of last update
};

// Global variables for interference history
InterferenceHistory channelHistory[16]; // For channels 11–26
InterferenceAnalysis currentAnalysis[16];

// WiFi interference analysis function
String analyzeWiFiInterference(uint8_t channel) {
    String result = "";
    
    // WiFi channels and their center frequencies
    const uint8_t wifiChannels[] = {1, 6, 11};  // 2.4 GHz
    const uint16_t wifiFreq[] = {2412, 2437, 2462}; // MHz
    
    // Zigbee channel to frequency
    uint16_t zigbeeFreq = 2405 + ((channel - 11) * 5); // MHz
    
    for(int i = 0; i < 3; i++) {
        int freqDiff = abs(zigbeeFreq - wifiFreq[i]);
        if(freqDiff < 20) {
            result += String("   (!!) High interference risk from WiFi channel ") + String(wifiChannels[i]);
            result += " (overlap > 80%)\n";
        } else if(freqDiff < 30) {
            result += String("   (!) Moderate interference from WiFi channel ") + String(wifiChannels[i]);
            result += " (overlap 40-60%)\n";
        }
    }
    
    return result;
}

// Function for tracking interference history
void updateInterferenceHistory(uint8_t channel, uint8_t noiseLevel) {
    uint8_t idx = channel - 11;
    if(idx >= 16) return;
    
    InterferenceHistory *history = &channelHistory[idx];
    
    // Initialize if necessary
    if(!history->isInitialized) {
        memset(history->noiseHistory, 0, sizeof(history->noiseHistory));
        history->historyIndex = 0;
        history->isInitialized = true;
        history->lastUpdateTime = millis();
    }
    
    // Update history
    history->noiseHistory[history->historyIndex] = noiseLevel;
    history->historyIndex = (history->historyIndex + 1) % 10;
    history->lastUpdateTime = millis();
    
    // Analyze the nature of the interference
    bool isConstant = true;
    uint8_t baseLevel = history->noiseHistory[0];
    uint8_t maxDiff = 0;
    
    for(int i = 1; i < 10; i++) {
        uint8_t diff = abs(history->noiseHistory[i] - baseLevel);
        if(diff > maxDiff) maxDiff = diff;
        if(diff > 10) {
            isConstant = false;
        }
    }
    
    // Update current analysis
    currentAnalysis[idx].isConstant = isConstant;
    currentAnalysis[idx].noiseLevel = noiseLevel;
    currentAnalysis[idx].lastDetected = millis();
}

// Function to get recommendations
String getChannelRecommendations(uint8_t channel) {
    String recommendations = "\n   Channel Recommendations:\n";
    uint8_t idx = channel - 11;
    
    if(currentAnalysis[idx].wifiOverlap > 70) {
        recommendations += "   - High WiFi interference detected:\n";
        recommendations += "     * Consider using channels 15, 20, or 25\n";
        recommendations += "     * Increase distance from WiFi routers\n";
    }
    
    if(currentAnalysis[idx].isConstant) {
        recommendations += "   - Constant interference detected:\n";
        recommendations += "     * Check for nearby constant RF sources\n";
        recommendations += "     * Consider physical barriers or repositioning\n";
    } else if(currentAnalysis[idx].noiseLevel > 0) {
        recommendations += "   - Variable interference detected:\n";
        recommendations += "     * Monitor peak interference times\n";
        recommendations += "     * Consider scheduling critical transmissions during low-interference periods\n";
    }
    
    return recommendations;
}

// Main interference analysis function for the channel
String analyzeChannelInterference(uint8_t channel, uint8_t signalStrength) {
    String result = "";
    uint8_t idx = channel - 11;
    
    // Analyze WiFi interference
    String wifiAnalysis = analyzeWiFiInterference(channel);
    if(wifiAnalysis.length() > 0) {
        result += "\nWiFi Interference Analysis:\n" + wifiAnalysis;
    }
    
    // Update history
    updateInterferenceHistory(channel, 255 - signalStrength);
    
    // Add recommendations if problems are detected
    if(currentAnalysis[idx].noiseLevel > 0 || wifiAnalysis.length() > 0) {
        result += getChannelRecommendations(channel);
    }
    
    return result;
}

#endif // ZIGBEE_SCANNER_INTERFERENCE_ANALYSIS_H