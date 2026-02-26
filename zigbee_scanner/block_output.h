#ifndef ZIGBEE_SCANNER_OUTPUT_H
#define ZIGBEE_SCANNER_OUTPUT_H

#include "block_definitions.h"
#include "block_helpers.h"
#include "block_analysis.h"

// Output functions
void printSummaryTable(zigbee_scan_result_t *scan_result, uint16_t networksFound) {
    Serial.println("\n=== NETWORK SCAN SUMMARY ===");
    Serial.println("+------------------+----+------+---------+--------+----------+---------+");
    Serial.println("| PAN ID (dec/hex) | CH | Join | Routers | EndDev | Security | Status  |");
    Serial.println("+------------------+----+------+---------+--------+----------+---------+");

    for (int i = 0; i < networksFound; ++i) {
        uint8_t* raw = (uint8_t*)&scan_result[i];
        if (!raw) {
            Serial.printf("Error accessing network data for index %d\n", i);
            continue;
        }

        uint16_t rawSignal = (raw[0] << 8) | raw[1];
        uint8_t signalStrength = (rawSignal >> 8) & 0xFF;

        String status = "New";
        for(int j = 0; j < previousNetworksCount; j++) {
            if(previousScan[j].panId == scan_result[i].short_pan_id) {
                if(previousScan[j].signalStrength < signalStrength) {
                    status = "Better";
                } else if(previousScan[j].signalStrength > signalStrength) {
                    status = "(!) Down";
                } else {
                    status = "Stable";
                }
                break;
            }
        }

        char panIdStr[20];
        snprintf(panIdStr, sizeof(panIdStr), "%5d/0x%04x", 
            scan_result[i].short_pan_id, 
            scan_result[i].short_pan_id);

        Serial.printf("| %-16s | %2d | %-4s | %-7s | %-6s | %-8s | %-7s |\n",
            panIdStr,
            scan_result[i].logic_channel,
            scan_result[i].permit_joining ? "Yes" : "No",
            scan_result[i].router_capacity ? "Yes" : "No",
            scan_result[i].end_device_capacity ? "Yes" : "No",
            (raw[5] & 0x7F) ? "Secured" : "Open",
            status.c_str()
        );
    }
    
    Serial.println("+------------------+----+------+---------+--------+----------+---------+");
}

void printNetworkDiagnostics(zigbee_scan_result_t *scan_result, uint16_t networksFound) {
    Serial.println("\n=== NETWORK DIAGNOSTICS ===");
    
    // Update channel statistics
    updateChannelStats(scan_result, networksFound);

    // Network analysis
    Serial.println("\nNetwork Analysis:");
    for(int i = 0; i < networksFound; i++) {
        updateNetworkStats(i, &scan_result[i]);
        
        Serial.printf("\nNetwork 0x%04x (PAN ID: %d):\n",
            scan_result[i].short_pan_id,
            scan_result[i].short_pan_id);

        uint8_t* raw = (uint8_t*)&scan_result[i];
        uint16_t rawSignal = (raw[0] << 8) | raw[1];
        uint8_t signalStrength = (rawSignal >> 8) & 0xFF;
        uint8_t networkLoad = (raw[4] * 100) / 255;
        
        Serial.printf("├─ Type: %s\n", networkStats[i].isCoordinator ? "Coordinator" : "Router/End Device");
        Serial.printf("├─ Uptime: %d sec\n", networkStats[i].uptime);

        if(networkStats[i].totalPackets > 0) {
            float packetLoss = (float)networkStats[i].failedPackets / networkStats[i].totalPackets * 100;
            float retryRate = (float)networkStats[i].retriesCount / networkStats[i].totalPackets * 100;
            
            Serial.printf("├─ Packet Loss: %.1f%%", packetLoss);
            if(packetLoss > 20) Serial.println(" (!!!)");
            else if(packetLoss > 10) Serial.println(" (!)");
            else Serial.println(" (OK)");
            
            Serial.printf("├─ Retry Rate: %.1f%%\n", retryRate);
        }

        // Problem analysis
        String analysis = getNetworkAnalysis(i);
        if(analysis.length() > 0) {
            Serial.printf("└─ Issues Found:\n%s", analysis.c_str());
        } else {
            Serial.println("└─ No issues detected");
        }
    }
}

void printScannedNetworks(uint16_t networksFound) {
    Serial.printf("\nScan completed. Networks found: %d\n", networksFound);
    
    if (networksFound == 0 || networksFound == 255) {
        Serial.println("No networks found or scan error");
        return;
    }

    zigbee_scan_result_t *scan_result = Zigbee.getScanResult();
    if (!scan_result) {
        Serial.println("Error: Unable to get scan results");
        return;
    }

    Serial.println("Scan data received successfully");
    Serial.println("-------------------------------");

    if (networksFound > 0) {
        printSummaryTable(scan_result, networksFound);
        printNetworkDiagnostics(scan_result, networksFound);
        saveNetworkHistory(scan_result, networksFound);
    }
    
    Zigbee.scanDelete();
}

#endif // ZIGBEE_SCANNER_OUTPUT_H