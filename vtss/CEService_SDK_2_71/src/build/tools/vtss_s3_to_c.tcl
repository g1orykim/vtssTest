#
# Vitesse Switch Software.
#
# Copyright (c) 2002-2011 Vitesse Semiconductor Corporation "Vitesse". All
# Rights Reserved.
#
# Unpublished rights reserved under the copyright laws of the United States of
# America, other countries and international treaties. Permission to use, copy,
# store and modify, the software and its source code is granted. Permission to
# integrate into other products, disclose, transmit and distribute the software
# in an absolute machine readable format (e.g. HEX file) is also granted.  The
# source code of the software may not be disclosed, transmitted or distributed
# without the written permission of Vitesse. The software and its source code
# may only be used in products utilizing the Vitesse switch products.
#
# This copyright notice must appear in any copy, modification, disclosure,
# transmission or distribution of the software. Vitesse retains all ownership,
# copyright, trade secret and proprietary rights in the software.
#
# THIS SOFTWARE HAS BEEN PROVIDED "AS IS," WITHOUT EXPRESS OR IMPLIED WARRANTY
# INCLUDING, WITHOUT LIMITATION, IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
# FOR A PARTICULAR USE AND NON-INFRINGEMENT.
#
#!/usr/bin/tclsh
# This TCL file converts Motorola S3 formatted EDC firmare (from Camarillo) into to a c-header file which can be compiled into the API
# 1. Start the script
# 2. Give the S3 input file when the script asks for it
# 3. The scripts converts the input file to the file:'vtss_phy_10g_edc_fw.h'
# 4. Copy this file to vtss_api/base
# 5. Compile the API with the macro:VTSS_FEATURE_EDC_FW_LOAD defined

puts "Name of input file (S3 formatted):"
gets stdin s3file_name
set s3file [open $s3file_name r]
set cfile [open vtss_phy_10g_edc_fw.h w]
set first 1
set addr_old 0
puts "Converting input file to a c-file...."
puts $cfile "#if defined(VTSS_FEATURE_EDC_FW_LOAD)"
puts $cfile "/*"
puts $cfile "  This c-file is auto-generated from S3 file:'$s3file_name' which is an Vitesse S3 encoded EDC firmware file."
puts $cfile "  If the API Macro:'VTSS_FEATURE_EDC_FW_LOAD' is defined then the API loads the FW into the iCPU (VSC848x 10G Phys) at chip initilization."
puts $cfile "*/"
puts $cfile ""
puts $cfile "u16 edc_fw_arr\[\] = {"

foreach line [split [read $s3file] \n] {
    if {[regexp {(..)(..)(.{0,8})(.{0,4})(.{0,4})(.{0,4})(.{0,4})(.{0,4})(.{0,4})(.{0,4})(.{0,4})} $line match S_type linesize addr d0 d1 d2 d3 d4 d5 d6 d7] != 1} {
        if {$S_type != "S3"} {
            continue
        }
        puts "The format of the input file is not supported. Pleae contact Vitesse support."
        exit
    }

    if {$S_type == "S3"} {
        puts $cfile "  0x$d0, 0x$d1, 0x$d2, 0x$d3, 0x$d4, 0x$d5, 0x$d6, 0x$d7,"
    } else {
        continue
    }

    if {!$first} {
        if {[expr 0x$addr-0x$addr_old] != 16} {
            puts "The address range (was:$addr_old, is now:$addr) is not supported. Pleae contact Vitesse support."
            exit
        } 
    } else {
        set first 0
        if {$addr != "BFB00000"} {
            puts "The start address is not right should be '0xBFB00000' is '0x$addr'. Pleae contact Vitesse support."
            exit
        }
    }
    set addr_old $addr
}
puts $cfile "};"
puts $cfile "#endif /*VTSS_FEATURE_EDC_FW_LOAD*/"
close $cfile 
puts "Done. Now copy the generated c-header file:'vtss_phy_10g_edc_fw.h' to directory: 'vtss_api/base'"
puts "The MACRO:VTSS_FEATURE_EDC_FW_LOAD must be defined to get the FW loaded during the initilization of the 10G Phy"


