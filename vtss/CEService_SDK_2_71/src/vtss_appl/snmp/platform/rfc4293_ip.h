/*
 * Note: this file originally auto-generated by mib2c using
 *        $
 */
#ifndef IPADDRESSTABLE_H
#define IPADDRESSTABLE_H

/*
 * function declarations
 */
void            init_ip(void);
//void            init_ipAddressTable(void);
FindVarMethod   var_ipAddressTable;
FindVarMethod   var_ipAddressTable;
WriteMethod     write_ipAddressIfIndex;
WriteMethod     write_ipAddressType;
WriteMethod     write_ipAddressStatus;
WriteMethod     write_ipAddressRowStatus;
WriteMethod     write_ipAddressStorageType;

//void            init_ipTrafficStats(void);
FindVarMethod   var_ipTrafficStats;
FindVarMethod   var_ipSystemStatsTable;
FindVarMethod   var_ipIfStatsTable;

//void            init_icmpStatsTable(void);
FindVarMethod   var_icmpStatsTable;
FindVarMethod   var_icmpStatsTable;

//void            init_icmpMsgStatsTable(void);
FindVarMethod   var_icmpMsgStatsTable;
FindVarMethod   var_icmpMsgStatsTable;

//void init_ipNetToPhysicalTable(void);
FindVarMethod var_ipNetToPhysicalTable;
FindVarMethod var_ipNetToPhysicalTable;
WriteMethod write_ipNetToPhysicalPhysAddress;
WriteMethod write_ipNetToPhysicalType;
WriteMethod write_ipNetToPhysicalRowStatus;

//void init_ipDefaultRouterTable(void);
FindVarMethod var_ipDefaultRouterTable;
FindVarMethod var_ipDefaultRouterTable;

//void init_ipv4InterfaceTable(void);
FindVarMethod var_ipv4InterfaceTable;
FindVarMethod var_ipv4InterfaceTable;
WriteMethod write_ipv4InterfaceEnableStatus;

//void init_ipv6InterfaceTable(void);
FindVarMethod var_ipv6InterfaceTable;
FindVarMethod var_ipv6InterfaceTable;
WriteMethod write_ipv6InterfaceEnableStatus;
WriteMethod write_ipv6InterfaceForwarding;

//void init_ipv6ScopeZoneIndexTable(void);
FindVarMethod var_ipv6ScopeZoneIndexTable;
FindVarMethod var_ipv6ScopeZoneIndexTable;

#endif                          /* IPADDRESSTABLE_H */