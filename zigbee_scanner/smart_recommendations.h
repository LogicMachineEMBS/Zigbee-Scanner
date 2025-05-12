#ifndef ZIGBEE_SCANNER_SMART_RECOMMENDATIONS_H
#define ZIGBEE_SCANNER_SMART_RECOMMENDATIONS_H

#include "block_definitions.h"
#include "block_helpers.h"

struct NetworkRecommendation {
    bool hasIssues;
    String signalRecommendation;
    String loadRecommendation;
    String channelRecommendation;
    String securityRecommendation;
};

// Analysis and recommendations for a single network
NetworkRecommendation analyzeNetwork(zigbee_scan_result_t *network, NetworkStats *stats, uint8_t networkIndex) {
    NetworkRecommendation rec = {false, "", "", "", ""};
    uint8_t* raw = (uint8_t*)&network->short_pan_id;
    uint16_t rawSignal = (raw[0] << 8) | raw[1];
    uint8_t signalStrength = (rawSignal >> 8) & 0xFF;
    uint8_t networkLoad = (raw[4] * 100) / 255;

    // Signal analysis
    if(signalStrength < 0x40) {
        rec.hasIssues = true;
        float avgStrength = stats->avgSignalStrength;
        if(avgStrength > signalStrength * 1.5) {
            rec.signalRecommendation = "Signal strength is significantly below average. Consider:\n"
                                     "- Checking for physical obstacles\n"
                                     "- Reducing distance to coordinator/router\n"
                                     "- Checking antenna orientation";
        }
    }

    // Stability analysis
    if(stats->maxSignalStrength - stats->minSignalStrength > 50) {
        rec.hasIssues = true;
        rec.signalRecommendation += "High signal variation detected. Possible causes:\n"
                                   "- Moving obstacles in signal path\n"
                                   "- Interference from nearby devices\n"
                                   "- Environmental factors (weather, temperature)";
    }

    // Load analysis
    if(networkLoad > 75) {
        rec.hasIssues = true;
        rec.loadRecommendation = "High network load detected. Consider:\n"
                                "- Distributing devices across multiple networks\n"
                                "- Optimizing data transmission intervals\n"
                                "- Reducing unnecessary traffic";
    }

    // Security recommendations
    if(!(raw[5] & 0x7F)) {
        rec.hasIssues = true;
        rec.securityRecommendation = "Network security is not enabled! Recommended actions:\n"
                                    "- Enable network encryption\n"
                                    "- Set up access control\n"
                                    "- Review device authentication settings";
    }

    return rec;
}

// Smart recommendation generation for the entire network
void generateSmartRecommendations(zigbee_scan_result_t *scan_result, uint16_t networksFound) {
    Serial.println("\n=== SMART RECOMMENDATIONS ===");

    // General network overview analysis
    bool hasOverloadedChannels = false;
    bool hasInterference = false;
    bool hasSecurityIssues = false;
    bool hasSignalIssues = false;

    // Count devices per channel
    uint8_t channelCounts[16] = {0}; // For channels 11–26
    
    for(int i = 0; i < networksFound; i++) {
        uint8_t channel = scan_result[i].logic_channel - 11;
        if(channel < 16) channelCounts[channel]++;
        
        NetworkRecommendation rec = analyzeNetwork(&scan_result[i], &networkStats[i], i);
        
        if(rec.hasIssues) {
            Serial.printf("\nRecommendations for Network 0x%04x:\n", scan_result[i].short_pan_id);
            
            if(rec.signalRecommendation.length() > 0) {
                Serial.println(rec.signalRecommendation);
                hasSignalIssues = true;
            }
            
            if(rec.loadRecommendation.length() > 0) {
                Serial.println(rec.loadRecommendation);
                hasOverloadedChannels = true;
            }
            
            if(rec.securityRecommendation.length() > 0) {
                Serial.println(rec.securityRecommendation);
                hasSecurityIssues = true;
            }
        }
    }

    // General optimization recommendations
    Serial.println("\nNetwork-Wide Recommendations:");

    // Check channel distribution
    int usedChannels = 0;
    int overloadedChannels = 0;
    for(int i = 0; i < 16; i++) {
        if(channelCounts[i] > 0) usedChannels++;
        if(channelCounts[i] > 2) overloadedChannels++;
    }

    if(overloadedChannels > 0) {
        Serial.println("\n1. Channel Distribution Issues:");
        Serial.println("   - Some channels are overloaded while others are unused");
        Serial.println("   - Consider redistributing networks across available channels");
        
        // Suggest specific channels to move to
        for(int i = 0; i < 16; i++) {
            if(channelCounts[i] == 0 && 
               !(i + 11 >= 11 && i + 11 <= 14) &&   // Not WiFi channel 1
               !(i + 11 >= 16 && i + 11 <= 19) &&   // Not WiFi channel 6
               !(i + 11 >= 21 && i + 11 <= 24)) {   // Not WiFi channel 11
                Serial.printf("   - Channel %d is free and recommended\n", i + 11);
            }
        }
    }

    // Performance recommendations
    if(hasSignalIssues) {
        Serial.println("\n2. Network Performance Optimization:");
        Serial.println("   - Consider network topology optimization");
        Serial.println("   - Review device placement and distances");
        Serial.println("   - Monitor environmental factors affecting signal");
    }

    // Security recommendations
    if(hasSecurityIssues) {
        Serial.println("\n3. Security Recommendations:");
        Serial.println("   - Implement network-wide security policy");
        Serial.println("   - Regular security audits recommended");
        Serial.println("   - Consider upgrading device firmware");
    }

    // Long-term recommendations
    Serial.println("\nLong-term Recommendations:");
    Serial.println("1. Regular network monitoring and maintenance");
    Serial.println("2. Plan for network scalability");
    Serial.println("3. Document network configuration and changes");
    Serial.println("4. Create backup plans for critical nodes");

    Serial.println(); // Empty line for readability
}

#endif // ZIGBEE_SCANNER_SMART_RECOMMENDATIONS_H