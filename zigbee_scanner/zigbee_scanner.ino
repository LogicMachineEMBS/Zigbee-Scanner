/*
 * Zigbee Network Scanner with adaptive scanning
 *
 * Done by EMBS, creators of LogicMachine
 */



/*
 * Zigbee Network Scanner with Diagnostics
 * 
 * This code provides comprehensive Zigbee network scanning and diagnostics capabilities.
 * It displays both a summary table of all networks and detailed diagnostics information
 * including signal strength, channel utilization, and network stability metrics.
 *
 * Symbol and Indicator Legend:
 * ---------------------------
 * Signal Level Indicators:
 * [****] - Excellent signal (80-100%)
 * [***-] - Good signal (60-79%)
 * [**--] - Medium signal (40-59%)
 * [*---] - Weak signal (20-39%)
 * [----] - Very weak signal (0-19%)
 *
 * Status Indicators:
 * (!!) CONGESTED  - More than 2 networks on channel
 * (!) BUSY       - 2 networks on channel
 * OK            - Channel clear
 *
 * Network Load Indicators:
 * (!HIGH!)      - Load >= 80%
 * (HIGH)        - Load >= 60%
 * (Med)         - Load >= 40%
 * (Low)         - Load < 40%
 */




#include "block_definitions.h"
#include "block_helpers.h"
#include "block_analysis.h"
#include "block_output.h"

// Global state variables for scanning control
unsigned long lastScanTime = 0;
const unsigned long SCAN_INTERVAL_NORMAL = 30000;  // 1 minute between scans when networks are found
const unsigned long SCAN_INTERVAL_SEARCH = 5000;   // 5 seconds between scans during search
bool scanInProgress = false;
bool networksFound = false;  // Flag to track network presence

void initializeStats() {
    for(int i = 0; i < MAX_NETWORKS; i++) {
        networkStats[i].totalPackets = 0;
        networkStats[i].failedPackets = 0;
        networkStats[i].retriesCount = 0;
        networkStats[i].signalTrend = 0;
        networkStats[i].lastSignalStrength = 0;
        networkStats[i].minSignalStrength = 0;
        networkStats[i].maxSignalStrength = 0;
        networkStats[i].avgSignalStrength = 0;
        networkStats[i].uptime = 0;
        networkStats[i].maxNetworkLoad = 0;
        networkStats[i].isCoordinator = false;
        networkStats[i].signalHistoryIndex = 0;
        networkStats[i].signalHistoryCount = 0;
    }
    previousNetworksCount = 0;
}

void setup() {
    Serial.begin(115200);
    while (!Serial) {
        ; // Waiting for Serial port to connect
    }
    
    delay(1000); // Give time for initialization
    Serial.println("\nZigbee Network Scanner Starting...");
    
    if (!Zigbee.begin(role)) {
        Serial.println("Zigbee initialization failed!");
        Serial.println("Rebooting...");
        ESP.restart();
    }

    initializeStats();
    Serial.println("Setup complete, starting network scan...");
    Zigbee.scanNetworks();
    scanInProgress = true;
    lastScanTime = millis();
}

void loop() {
    static unsigned long lastPrintTime = 0;
    static unsigned long scanStartTime = 0;
    unsigned long currentTime = millis();
    unsigned long currentScanInterval = networksFound ? SCAN_INTERVAL_NORMAL : SCAN_INTERVAL_SEARCH;

    if (scanInProgress) {
        int16_t scanStatus = Zigbee.scanComplete();
        
        if (scanStatus == ZB_SCAN_RUNNING) {
            // Still scanning
            if (currentTime - scanStartTime > 30000) { // 30-second timeout
                Serial.println("\nScan timeout - restarting scan");
                Zigbee.scanDelete();
                scanInProgress = false;
                lastScanTime = currentTime;
            }
        }
        else if (scanStatus == ZB_SCAN_FAILED) {
            Serial.println("\nScan failed! Waiting before retry...");
            scanInProgress = false;
            lastScanTime = currentTime;
        }
        else if (scanStatus > 0) {  // At least one network found
            Serial.printf("\nScan completed with status: %d\n", scanStatus);
            networksFound = true;  // Set the flag indicating networks are found
            printScannedNetworks(scanStatus);
            scanInProgress = false;
            lastScanTime = currentTime;
        }
        else if (scanStatus == 0) {  // No networks found
            Serial.println("\nNo networks found, will scan again soon...");
            scanInProgress = false;
            lastScanTime = currentTime;
            networksFound = false;
        }
    } else {
        // Check if it's time to start a new scan
        if (currentTime - lastScanTime >= currentScanInterval) {
            Serial.println("\nInitiating new scan cycle...");
            if (!networksFound) {
                Serial.println("(Fast scanning mode active)");
            }
            Zigbee.scanNetworks();
            Serial.println("Scan started");
            scanInProgress = true;
            scanStartTime = currentTime;
        } else if (currentTime - lastPrintTime >= 20000) { // Every 20 seconds
            int remainingTime = (currentScanInterval - (currentTime - lastScanTime)) / 1000;
            if (networksFound) {
                Serial.printf("\nWaiting for next scan: %d seconds", remainingTime);
            } else {
                Serial.printf("\nSearching for networks... Next scan in %d seconds", remainingTime);
            }
            lastPrintTime = currentTime;
        }
    }

    delay(100);
}