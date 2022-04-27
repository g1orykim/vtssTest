/*

 Vitesse Switch Software.

 Copyright (c) 2002-2013 Vitesse Semiconductor Corporation "Vitesse". All
 Rights Reserved.

 Unpublished rights reserved under the copyright laws of the United States of
 America, other countries and international treaties. Permission to use, copy,
 store and modify, the software and its source code is granted. Permission to
 integrate into other products, disclose, transmit and distribute the software
 in an absolute machine readable format (e.g. HEX file) is also granted.  The
 source code of the software may not be disclosed, transmitted or distributed
 without the written permission of Vitesse. The software and its source code
 may only be used in products utilizing the Vitesse switch products.

 This copyright notice must appear in any copy, modification, disclosure,
 transmission or distribution of the software. Vitesse retains all ownership,
 copyright, trade secret and proprietary rights in the software.

 THIS SOFTWARE HAS BEEN PROVIDED "AS IS," WITHOUT EXPRESS OR IMPLIED WARRANTY
 INCLUDING, WITHOUT LIMITATION, IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 FOR A PARTICULAR USE AND NON-INFRINGEMENT.

*/
// NB: This file is *only* used in "mockup" environment!
// But if new variables are introduced in the "live" version,
// then provide default settings here as well!
var configArchLuton28 = 0;
var configArchJaguar_1 = 0;
var configArchLuton26 = 0;
var configArchServal = 1;
var configBuildSMB = 1;
var configBuildCE = 0;
var configPortMin = 1;
var configNormalPortMax = 24;
var configRgmiiWifi = 0;
var configStackable = 1;
var configVlanIdMin = 1;
var configVlanIdMax = 4095;
var configVlanEntryCnt = 64;
var configPvlanIdMin = 1;
var configPvlanIdMax = 26;
var configSidMin = 1;
var configSidMax = 16;
var configAclPktRateMax = 1024;
var configAclBitRateMax = 1000000;
var configAclBitRateGranularity = 100;
var configAclRateLimitIdMax = 15;
var configAceMax = 128;
var configSwitchName = "MockUp";
var configSwitchDescription = "24x1G + 2x5G Stackable Ethernet Switch";
var configPsecLimitLimitMax = 100;
var configPolicyMax = 8;
var configPolicyBitmaskMax = 255;
var configAceMax = 128;
var configPortType = 0;
var configAuthServerCnt = 5;
var configAuthHostLen = 255;
var configAuthKeyLen = 63;
var configAuthRadiusAuthPortDef = 1812;
var configAuthRadiusAcctPortDef = 1813;
var configAuthTacacsPortDef = 49;
var configAuthTimeoutDef = 5;
var configAuthTimeoutMin = 1;
var configAuthTimeoutMax = 1000;
var configAuthRetransmitDef = 3;
var configAuthRetransmitMin = 1;
var configAuthRetransmitMax = 1000;
var configAuthDeadtimeDef = 0;
var configAuthDeadtimeMin = 0;
var configAuthDeadtimeMax = 1440;
var configHasIngressFiltering = 1;
var configQosClassMax = 4;
var configQosBitRateMin = 100;
var configQosBitRateMax = 1000000;
var configQosBitRateDef = 500;
var configQosDplMax = 3;
var configQCLMax = 23;
var configQCEMax = 256;
var configQosDscpNames = [
    '0  (BE)', '1', '2',        '3','4','5','6','7',
    '8  (CS1)','9', '10 (AF11)','11','12 (AF12)','13','14 (AF13)','15',
    '16 (CS2)','17','18 (AF21)','19','20 (AF22)','21','22 (AF23)','23',
    '24 (CS3)','25','26 (AF31)','27','28 (AF32)','29','30 (AF33)','31',
    '32 (CS4)','33','34 (AF41)','35','36 (AF42)','37','38 (AF43)','39',
    '40 (CS5)','41','42',       '43','44',       '45','46 (EF)',  '47',
    '48 (CS6)','49','50',       '51','52',       '53','54',       '55',
    '56 (CS7)','57','58',       '59','60',       '61','62',       '63'
];
var configHasStpEnhancements = 1;
var configAccessMgmtEntryCnt = 16;
var configVoiceVlanOuiEntryCnt = 16;
var configIgmpsFilteringMax = 5;
var configIgmpsVLANsMax = 64;
var configMldsnpFilteringMax = 5;
var configMldsnpVLANsMax = 64;
var configAccessMgmtMax = 16;
var configHasCDP = "1";
var configIPDNSSupport = 1;
var configIPv6Support = 1;

var configEvcIdMax = 128;
var configEceIdMax = 128;
var configEvcPolicerIdMax = 128;
var configEvcCirMin = 0;
var configEvcCirMax = 10000000;
var configEvcCbsMin = 0;
var configEvcCbsMax = 100000;
var configEvcEirMin = 0;
var configEvcEirMax = 10000000;
var configEvcEbsMin = 0;
var configEvcEbsMax = 100000;
var configPortFrameSizeMin = 1518;
var configPortFrameSizeMax = 9600;
var if_switch_interval = 1000;
var if_llag_start = 501;
var if_llag_cnt = 26;
var if_glag_start = -1;
var if_glag_cnt = 0;
var configLldpmedPoliciesMin = 1;
var configLldpmedPoliciesMax = 5;

var configPerfMonLMInstanceMax = 64;
var configPerfMonDMInstanceMax = 64;
var configPerfMonEVCInstanceMax = 64;
var configPerfMonECEInstanceMax = 64;

var rfc2544_profile_cnt =    16;
var rfc2544_report_cnt  =    10;
var rfc2544_dwell_min   =     1;
var rfc2544_dwell_max   =    10;
var rfc2544_dwell_def   =     2;
var rfc2544_tp_dur_min  =     1;
var rfc2544_tp_dur_max  =  1800;
var rfc2544_tp_dur_def  =    60;
var rfc2544_tp_min_min  =     1;
var rfc2544_tp_min_max  =  1000;
var rfc2544_tp_min_def  =   800;
var rfc2544_tp_max_min  =     1;
var rfc2544_tp_max_max  =  1000;
var rfc2544_tp_max_def  =  1000;
var rfc2544_tp_step_min =     1;
var rfc2544_tp_step_max =  1000;
var rfc2544_tp_step_def =     2;
var rfc2544_tp_pass_min =     0;
var rfc2544_tp_pass_max =   100;
var rfc2544_tp_pass_def =     0;
var rfc2544_la_dur_min  =    10;
var rfc2544_la_dur_max  =  1800;
var rfc2544_la_dur_def  =   120;
var rfc2544_la_dmm_min  =     1;
var rfc2544_la_dmm_max  =    60;
var rfc2544_la_dmm_def  =    10;
var rfc2544_la_pass_min =     0;
var rfc2544_la_pass_max =   100;
var rfc2544_la_pass_def =     0;
var rfc2544_fl_dur_min  =     1;
var rfc2544_fl_dur_max  =  1800;
var rfc2544_fl_dur_def  =    60;
var rfc2544_fl_min_min  =     1;
var rfc2544_fl_min_max  =  1000;
var rfc2544_fl_min_def  =   800;
var rfc2544_fl_max_min  =     1;
var rfc2544_fl_max_max  =  1000;
var rfc2544_fl_max_def  =  1000;
var rfc2544_fl_step_min =     1;
var rfc2544_fl_step_max =  1000;
var rfc2544_fl_step_def =     5;
var rfc2544_bb_dur_min  =   100;
var rfc2544_bb_dur_max  = 10000;
var rfc2544_bb_dur_def  =  2000;
var rfc2544_bb_cnt_min  =     1;
var rfc2544_bb_cnt_max  =   100;
var rfc2544_bb_cnt_def  =    50;

function configPortName(portno, long)
{
    var portname = String(portno);
    if(long) {
        portname = "Port " + portname;
    }
    return portname;
}

function configIndexName(index, long)
{
    var indexname = String(index);
    if(long) {
        indexname = "ID " + indexname;
    }
    return indexname;
}

function isNtpSupported()
{
 return true;
}

