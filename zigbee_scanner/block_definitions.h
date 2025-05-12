#ifndef ZIGBEE_SCANNER_DEFINITIONS_H
#define ZIGBEE_SCANNER_DEFINITIONS_H

#include "Zigbee.h"
//#include "Zigbee/src/Zigbee.h"

// Constants
#define MAX_NETWORKS 10
#define MIN_CHANNEL 11
#define MAX_CHANNEL 26
#define SIGNAL_HISTORY_SIZE 10

// Data structures
struct NetworkStats {
    uint32_t totalPackets;
    uint32_t failedPackets;
    uint32_t retriesCount;
    uint32_t lastResponseTime;
    uint32_t uptime;              
    uint8_t lastSignalStrength;
    uint8_t minSignalStrength;
    uint8_t maxSignalStrength;
    uint8_t avgSignalStrength;    
    uint8_t signalTrend;          // 0 - stable, 1 - improving, 2 - degrading
    uint8_t maxNetworkLoad;       
    bool isCoordinator;           
    uint8_t signalHistory[SIGNAL_HISTORY_SIZE];
    uint8_t signalHistoryIndex;
    uint8_t signalHistoryCount;
};

struct NetworkHistory {
    uint16_t panId;
    uint8_t channel;
    uint8_t signalStrength;
    uint8_t networkLoad;
    bool wasPresent;
};

// External variable declarations
extern NetworkStats networkStats[MAX_NETWORKS];
extern NetworkHistory previousScan[MAX_NETWORKS];
extern int previousNetworksCount;

// Operating mode
#ifdef ZIGBEE_MODE_ZCZR
zigbee_role_t role = ZIGBEE_ROUTER;
#else
zigbee_role_t role = ZIGBEE_END_DEVICE;
#endif

// Function prototypes
bool isCoordinator(zigbee_scan_result_t *network);
String getChannelStatus(uint8_t channel, zigbee_scan_result_t *results, uint16_t count);
void printSummaryTable(zigbee_scan_result_t *scan_result, uint16_t networksFound);
void printNetworkDiagnostics(zigbee_scan_result_t *scan_result, uint16_t networksFound);
void printScannedNetworks(uint16_t networksFound);
void updateChannelStats(zigbee_scan_result_t *scan_result, uint16_t networksFound);
void updateNetworkStats(int idx, zigbee_scan_result_t *network);
void saveNetworkHistory(zigbee_scan_result_t *scan_result, uint16_t networksFound);
String getNetworkAnalysis(int idx);
String analyzeChannelInterference(uint8_t channel, uint8_t signalStrength);

#endif // ZIGBEE_SCANNER_DEFINITIONS_H
