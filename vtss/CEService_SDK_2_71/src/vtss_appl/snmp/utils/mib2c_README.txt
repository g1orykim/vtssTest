How to install NET-SNMP on Ubuntu
---------------------------------
1. Install NET-SNMP
    sudo apt-get install snmp

2. Install MIB
    sudo apt-get install snmp-mibs-downloader


How to run mib2c on Ubuntu
--------------------------
1. Add MIB search path
   vi /etc/snmp/snmp.conf
       mibdirs : +/usr/share/mibs/iana:/usr/share/mibs/ietf:/usr/share/mibs/netsnmp --> Add new path here
       mibs +ALL --> Add this new line

2. Run mib2c
    cd ~
    mkdir mib2c
    "copy vtss_mib2c_ucd_snmp.conf"
    mib2c -c vtss_mib2c_ucd_snmp.conf {OID Name | OID}

3. mib2c output - Four files will be created after run mib2c
   xxx.c/h -> Porting part
   ucd_snmp_xxx.c/h -> For UCD-SNMP core engine
   
   
What to do after the code generated
-----------------------------------
1. Copy output files to working directory
   Copy xxx.c/h to "vtss_appl/snmp/platform"
   Copy ucd_snmp_xxx.c/h to "vtss_appl/snmp/base/ucd_snmp"

2. Initialize API
   Add include file ¡§xxx.h¡¨ in vtss_snmp.c
   Add initial function in vtss_snmp_mibs_init()

3. Add the generated files for compiler
   Modify the make file in <TOP>/build/make/module_snmp.in
   After this step, we should compiler the new MIBs files without no error.

4. Revise the "FIXME" parts to make it as a completed code.

5. Don¡¦t forget to modify the filter file for release purpose
  \webstax2\build\release\modules_files_hash_table.pl
