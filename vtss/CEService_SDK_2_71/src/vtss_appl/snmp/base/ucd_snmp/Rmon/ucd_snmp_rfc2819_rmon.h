/**************************************************************
 * Copyright (C) 2001 Alex Rozin, Optical Access
 *
 *                     All Rights Reserved
 *
 * Permission to use, copy, modify and distribute this software and its
 * documentation for any purpose and without fee is hereby granted,
 * provided that the above copyright notice appear in all copies and that
 * both that copyright notice and this permission notice appear in
 * supporting documentation.
 *
 * ALEX ROZIN DISCLAIM ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
 * ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
 * ALEX ROZIN BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
 * ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
 * WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
 * ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 * SOFTWARE.
 ******************************************************************/

#ifndef UCD_SNMP_RFC_2819_RMON_H
#define UCD_SNMP_RFC_2819_RMON_H

/*
 * Function declarations
 */
#if RFC2819_SUPPORTED_STATISTICS

/* statistics ----------------------------------------------------------*/
void            ucd_snmp_init_rmon_statisticsMIB(void);
FindVarMethod   var_statistics;
FindVarMethod   var_etherStatsTable;
WriteMethod     write_etherStatsDataSource;
WriteMethod     write_etherStatsOwner;
WriteMethod     write_etherStatsStatus;
#endif /* RFC2819_SUPPORTED_STATISTICS */

#if RFC2819_SUPPORTED_HISTORY
/* history ----------------------------------------------------------*/
void            ucd_snmp_init_rmon_historyMIB(void);
FindVarMethod   var_history;
FindVarMethod   var_historyControlTable;
FindVarMethod   var_etherHistoryTable;
WriteMethod     write_historyControlDataSource;
WriteMethod     write_historyControlBucketsRequested;
WriteMethod     write_historyControlInterval;
WriteMethod     write_historyControlOwner;
WriteMethod     write_historyControlStatus;
#endif /* RFC2819_SUPPORTED_HISTORY */

#if RFC2819_SUPPORTED_AlARM
/* alram ----------------------------------------------------------*/
void            ucd_snmp_init_rmon_alarmMIB(void);
FindVarMethod   var_alarm;
FindVarMethod   var_alarmTable;
WriteMethod     write_alarmInterval;
WriteMethod     write_alarmVariable;
WriteMethod     write_alarmSampleType;
WriteMethod     write_alarmStartupAlarm;
WriteMethod     write_alarmRisingThreshold;
WriteMethod     write_alarmFallingThreshold;
WriteMethod     write_alarmRisingEventIndex;
WriteMethod     write_alarmFallingEventIndex;
WriteMethod     write_alarmOwner;
WriteMethod     write_alarmStatus;
#endif /* RFC2819_SUPPORTED_AlARM */

#if RFC2819_SUPPORTED_EVENT
/* event ----------------------------------------------------------*/
void            ucd_snmp_init_rmon_eventMIB(void);
FindVarMethod   var_event;
FindVarMethod   var_eventTable;
FindVarMethod   var_logTable;
WriteMethod     write_eventDescription;
WriteMethod     write_eventType;
WriteMethod     write_eventCommunity;
WriteMethod     write_eventOwner;
WriteMethod     write_eventStatus;
#endif /* RFC2819_SUPPORTED_EVENT */

#endif /* UCD_SNMP_RFC_2819_RMON_H */

