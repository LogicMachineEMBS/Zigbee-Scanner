#ifndef ZIGBEE_SCANNER_ANALYSIS_H
#define ZIGBEE_SCANNER_ANALYSIS_H

#include "block_definitions.h"
#include "block_helpers.h"

// Channel statistics update function
void updateChannelStats(zigbee_scan_result_t *scan_result, uint16_t networksFound) {
    for(int i = 0; i < networksFound; i++) {
        uint8_t channel = scan_result[i].logic_channel;
        if(channel >= MIN_CHANNEL && channel <= MAX_CHANNEL) {
            // Additional statistics may be added here in the future
        }
    }
}

// Network statistics update function
void updateNetworkStats(int idx, zigbee_scan_result_t *network) {
    uint8_t* raw = (uint8_t*)&network->short_pan_id;
    uint16_t rawSignal = (raw[0] << 8) | raw[1];
    uint8_t signalStrength = (rawSignal >> 8) & 0xFF;
    uint8_t networkLoad = (raw[4] * 100) / 255;

    // Save the value to history
    networkStats[idx].signalHistory[networkStats[idx].signalHistoryIndex] = signalStrength;
    networkStats[idx].signalHistoryIndex = (networkStats[idx].signalHistoryIndex + 1) % SIGNAL_HISTORY_SIZE;
    if (networkStats[idx].signalHistoryCount < SIGNAL_HISTORY_SIZE) {
        networkStats[idx].signalHistoryCount++;
    }

    // Update min/max/avg based on history
    networkStats[idx].minSignalStrength = 255;
    networkStats[idx].maxSignalStrength = 0;
    uint32_t sum = 0;

    for (int i = 0; i < networkStats[idx].signalHistoryCount; i++) {
        uint8_t value = networkStats[idx].signalHistory[i];
        networkStats[idx].minSignalStrength = min(networkStats[idx].minSignalStrength, value);
        networkStats[idx].maxSignalStrength = max(networkStats[idx].maxSignalStrength, value);
        sum += value;
    }

    networkStats[idx].avgSignalStrength = sum / networkStats[idx].signalHistoryCount;

    // Trend detection
    if(networkStats[idx].lastSignalStrength > 0) {
        if(abs(signalStrength - networkStats[idx].lastSignalStrength) > 10) {
            networkStats[idx].signalTrend = (signalStrength > networkStats[idx].lastSignalStrength) ? 1 : 2;
        } else {
            networkStats[idx].signalTrend = 0;
        }
    }
    
    networkStats[idx].lastSignalStrength = signalStrength;
    networkStats[idx].maxNetworkLoad = max(networkLoad, networkStats[idx].maxNetworkLoad);
    networkStats[idx].isCoordinator = isCoordinator(network);
    networkStats[idx].uptime += 5;
    networkStats[idx].totalPackets++;

    // Error simulation
    if(random(100) < 5) networkStats[idx].failedPackets++;
    if(random(100) < 8) networkStats[idx].retriesCount++;
}

// Network analysis function
String getNetworkAnalysis(int idx) {
    String result = "";
    NetworkStats *stats = &networkStats[idx];
    
    // Signal stability analysis
    float signalVariation = stats->maxSignalStrength - stats->minSignalStrength;
    if(signalVariation > 50) {
        result += "(!!) High signal variation\n";
        result += String("     Signal range: ") + String(stats->minSignalStrength) + 
                 " to " + String(stats->maxSignalStrength) + "\n";
    }
    
    // Packet loss analysis
    if(stats->totalPackets > 0) {
        float lossRate = (float)stats->failedPackets / stats->totalPackets * 100;
        if(lossRate > 20) {
            result += "(!!) High packet loss rate\n";
            result += String("     Loss rate: ") + String(lossRate, 1) + "%\n";
        } else if(lossRate > 10) {
            result += "(!) Moderate packet loss\n";
            result += String("     Loss rate: ") + String(lossRate, 1) + "%\n";
        }
    }
    
    return result;
}

// Saving scan results
void saveNetworkHistory(zigbee_scan_result_t *scan_result, uint16_t networksFound) {
    previousNetworksCount = networksFound;
    for(int i = 0; i < networksFound && i < MAX_NETWORKS; i++) {
        uint8_t* raw = (uint8_t*)&scan_result[i];
        uint16_t rawSignal = (raw[0] << 8) | raw[1];
        
        previousScan[i].panId = scan_result[i].short_pan_id;
        previousScan[i].channel = scan_result[i].logic_channel;
        previousScan[i].signalStrength = (rawSignal >> 8) & 0xFF;
        previousScan[i].networkLoad = (raw[4] * 100) / 255;
        previousScan[i].wasPresent = true;
    }
}

#endif // ZIGBEE_SCANNER_ANALYSIS_H