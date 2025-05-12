#ifndef ZIGBEE_SCANNER_HELPERS_H
#define ZIGBEE_SCANNER_HELPERS_H

#include "block_definitions.h"

// Global variables
NetworkStats networkStats[MAX_NETWORKS];
NetworkHistory previousScan[MAX_NETWORKS];
int previousNetworksCount = 0;

// Functions
String getSignalLevel(uint8_t strength) {
    int level = (strength * 5) / 0xFF;
    switch(level) {
        case 0: return "[----]";
        case 1: return "[*---]";
        case 2: return "[**--]";
        case 3: return "[***-]";
        case 4: 
        case 5: return "[****]";
        default: return "[----]";
    }
}

String getLoadLevel(uint8_t load) {
    if (load >= 80) return "(!HIGH!)";
    if (load >= 60) return "(HIGH)";
    if (load >= 40) return "(Med)";
    return "(Low)";
}

const char* getSignalQuality(uint16_t rawSignal) {
    uint8_t signalStrength = (rawSignal >> 8) & 0xFF;
    if (signalStrength >= 0x80) return "Excellent";
    if (signalStrength >= 0x60) return "Good";
    if (signalStrength >= 0x40) return "Fair";
    return "Poor";
}

bool isCoordinator(zigbee_scan_result_t *network) {
    return network->router_capacity && 
           network->end_device_capacity && 
           network->permit_joining;
}

#endif // ZIGBEE_SCANNER_HELPERS_H