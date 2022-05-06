//==========================================================================
//
//      ./agent/current/include/mib_module_config.h
//
//
//==========================================================================
// ####ECOSGPLCOPYRIGHTBEGIN####                                            
// -------------------------------------------                              
// This file is part of eCos, the Embedded Configurable Operating System.   
// Copyright (C) 1998, 1999, 2000, 2001, 2002 Free Software Foundation, Inc.
//
// eCos is free software; you can redistribute it and/or modify it under    
// the terms of the GNU General Public License as published by the Free     
// Software Foundation; either version 2 or (at your option) any later      
// version.                                                                 
//
// eCos is distributed in the hope that it will be useful, but WITHOUT      
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or    
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License    
// for more details.                                                        
//
// You should have received a copy of the GNU General Public License        
// along with eCos; if not, write to the Free Software Foundation, Inc.,    
// 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.            
//
// As a special exception, if other files instantiate templates or use      
// macros or inline functions from this file, or you compile this file      
// and link it with other works to produce a work based on this file,       
// this file does not by itself cause the resulting work to be covered by   
// the GNU General Public License. However the source code for this file    
// must still be made available in accordance with section (3) of the GNU   
// General Public License v2.                                               
//
// This exception does not invalidate any other reasons why a work based    
// on this file might be covered by the GNU General Public License.         
// -------------------------------------------                              
// ####ECOSGPLCOPYRIGHTEND####                                              
//####UCDSNMPCOPYRIGHTBEGIN####
//
// -------------------------------------------
//
// Portions of this software may have been derived from the UCD-SNMP
// project,  <http://ucd-snmp.ucdavis.edu/>  from the University of
// California at Davis, which was originally based on the Carnegie Mellon
// University SNMP implementation.  Portions of this software are therefore
// covered by the appropriate copyright disclaimers included herein.
//
// The release used was version 4.1.2 of May 2000.  "ucd-snmp-4.1.2"
// -------------------------------------------
//
//####UCDSNMPCOPYRIGHTEND####
//==========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):    hmt
// Contributors: hmt
// Date:         2000-05-30
// Purpose:      Port of UCD-SNMP distribution to eCos.
// Description:  
//              
//
//####DESCRIPTIONEND####
//
//==========================================================================
/********************************************************************
       Copyright 1989, 1991, 1992 by Carnegie Mellon University

			  Derivative Work -
Copyright 1996, 1998, 1999, 2000 The Regents of the University of California

			 All Rights Reserved

Permission to use, copy, modify and distribute this software and its
documentation for any purpose and without fee is hereby granted,
provided that the above copyright notice appears in all copies and
that both that copyright notice and this permission notice appear in
supporting documentation, and that the name of CMU and The Regents of
the University of California not be used in advertising or publicity
pertaining to distribution of the software without specific written
permission.

CMU AND THE REGENTS OF THE UNIVERSITY OF CALIFORNIA DISCLAIM ALL
WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING ALL IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS.  IN NO EVENT SHALL CMU OR
THE REGENTS OF THE UNIVERSITY OF CALIFORNIA BE LIABLE FOR ANY SPECIAL,
INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING
FROM THE LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF
CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
*********************************************************************/
/* This file is automatically generated by configure.  Do not modify by hand. */
/* Define if compiling with the mibII module files.  */
#define USING_MIBII_MODULE 1
 
/* Define if compiling with the ucd_snmp module files.  */
#define USING_UCD_SNMP_MODULE 1
 
/* Define if compiling with the snmpv3mibs module files.  */
//#define USING_SNMPV3MIBS_MODULE 1
 
/* Define if compiling with the mibII/system_mib module files.  */
#define USING_MIBII_SYSTEM_MIB_MODULE 1
 
/* Define if compiling with the mibII/sysORTable module files.  */
#define USING_MIBII_SYSORTABLE_MODULE 1
 
/* Define if compiling with the mibII/at module files.  */
//#define USING_MIBII_AT_MODULE 1
 
/* Define if compiling with the mibII/interfaces module files.  */
#define USING_MIBII_INTERFACES_MODULE 1
 
/* Define if compiling with the mibII/snmp_mib module files.  */
#define USING_MIBII_SNMP_MIB_MODULE 1
 
/* Define if compiling with the mibII/tcp module files.  */
#define USING_MIBII_TCP_MODULE 1
 
/* Define if compiling with the mibII/icmp module files.  */
#define USING_MIBII_ICMP_MODULE 1
 
/* Define if compiling with the mibII/ip module files.  */
#define USING_MIBII_IP_MODULE 1
 
/* Define if compiling with the mibII/udp module files.  */
#define USING_MIBII_UDP_MODULE 1
 
/* Define if compiling with the mibII/vacm_vars module files.  */
#ifdef CYGPKG_SNMPAGENT_V3_SUPPORT
#ifdef CYGPKG_SNMPLIB_FILESYSTEM_SUPPORT
#define USING_MIBII_VACM_VARS_MODULE 1
#endif
#endif
 
/* Define if compiling with the ucd-snmp/memory module files.  */
//#define USING_UCD_SNMP_MEMORY_MODULE 1
 
/* Define if compiling with the ucd-snmp/vmstat module files.  */
//#define USING_UCD_SNMP_VMSTAT_MODULE 1
 
/* Define if compiling with the ucd-snmp/proc module files.  */
//#define USING_UCD_SNMP_PROC_MODULE 1
 
/* Define if compiling with the ucd-snmp/versioninfo module files.  */
//#define USING_UCD_SNMP_VERSIONINFO_MODULE 1
 
/* Define if compiling with the ucd-snmp/pass module files.  */
//#define USING_UCD_SNMP_PASS_MODULE 1
 
/* Define if compiling with the ucd-snmp/pass_persist module files.  */
//#define USING_UCD_SNMP_PASS_PERSIST_MODULE 1
 
/* Define if compiling with the ucd-snmp/disk module files.  */
//#define USING_UCD_SNMP_DISK_MODULE 1
 
/* Define if compiling with the ucd-snmp/loadave module files.  */
//#define USING_UCD_SNMP_LOADAVE_MODULE 1
 
/* Define if compiling with the ucd-snmp/extensible module files.  */
//#define USING_UCD_SNMP_EXTENSIBLE_MODULE 1
 
/* Define if compiling with the ucd-snmp/errormib module files.  */
//#define USING_UCD_SNMP_ERRORMIB_MODULE 1
 
/* Define if compiling with the ucd-snmp/registry module files.  */
//#define USING_UCD_SNMP_REGISTRY_MODULE 1
 
/* Define if compiling with the ucd-snmp/file module files.  */
//#define USING_UCD_SNMP_FILE_MODULE 1
 
#ifdef CYGPKG_SNMPAGENT_V3_SUPPORT
/* Define if compiling with the snmpv3/snmpEngine module files.  */
#define USING_SNMPV3_SNMPENGINE_MODULE 1
 
/* Define if compiling with the snmpv3/snmpMPDStats module files.  */
//#define USING_SNMPV3_SNMPMPDSTATS_MODULE 1
 
/* Define if compiling with the snmpv3/usmStats module files.  */
#define USING_SNMPV3_USMSTATS_MODULE 1
 
/* Define if compiling with the snmpv3/usmUser module files.  */
/* +++ peter, 2007/12, do this from EstaX-34 management module
#define USING_SNMPV3_USMUSER_MODULE 1 */
#endif
 
/* Define if compiling with the util_funcs module files.  */
#define USING_UTIL_FUNCS_MODULE 1
 
/* Define if compiling with the mibII/var_route module files.  */
//#define USING_MIBII_VAR_ROUTE_MODULE 1
 
/* Define if compiling with the mibII/route_write module files.  */
//#define USING_MIBII_ROUTE_WRITE_MODULE 1
 
