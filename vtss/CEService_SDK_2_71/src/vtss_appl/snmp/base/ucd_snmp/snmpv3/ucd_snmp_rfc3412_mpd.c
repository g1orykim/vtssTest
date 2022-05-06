/* -----------------------------------------------------------------------------

 Portions of this software may have been derived from the UCD-SNMP
 project,  <http://www.net-snmp.org/>  from the University of
 California at Davis, which was originally based on the Carnegie Mellon
 University SNMP implementation.  Portions of this software are therefore
 covered by the appropriate copyright disclaimers included herein.

 The release used was version 5.0.11.2 of June 2008.  "net-snmp-5.0.11.2"

 -------------------------------------------------------------------------------
*/

/*
 * Note: this file originally auto-generated by mib2c using
 *        : mib2c.old-api.conf
 */

#include <main.h>
#include <cyg/infra/cyg_type.h>
#include <sys/param.h>

#include <ucd-snmp/config.h>
#if HAVE_STDLIB_H
#include <stdlib.h>
#endif                          /* HAVE_STDLIB_H */
#if HAVE_STRING_H
#include <string.h>
#else
#include <strings.h>
#endif                          /* HAVE_STRING_H */

#include <ucd-snmp/mibincl.h>   /* Standard set of SNMP includes */
#include <ucd-snmp/mibgroup/util_funcs.h>       /* utility function declarations */

#include "vtss_snmp_api.h"
#include "ucd_snmp_rfc3412_mpd.h"

/*
 * +++ Start (Internal implementation declarations)
 */
#include "rfc1213_mib2.h"
#include "ucd_snmp_rfc1213_mib2.h"
/*
 * --- End (Internal implementation declarations)
 */

/*
 * snmpMPDStats_variables_oid:
 *   this is the top level oid that we want to register under.  This
 *   is essentially a prefix, with the suffix appearing in the
 *   variable below.
 */

oid             snmpMPDStats_variables_oid[] =
{ 1, 3, 6, 1, 6, 3, 11, 2, 1 };

/*
 * variable2 snmpMPDStats_variables:
 *   this variable defines function callbacks and type return information
 *   for the lagMIBObjects mib section
 */
struct variable2 snmpMPDStats_variables[] = {
    /*
     * magic number        , variable type , ro/rw , callback fn  , L, oidsuffix
     */
#define   SNMPUNKNOWNSECURITYMODELS  0
    {SNMPUNKNOWNSECURITYMODELS, ASN_COUNTER, RONLY, var_snmpMPDStats, 1, {1}},
#define   SNMPINVALIDMSGS            1
    {SNMPINVALIDMSGS, ASN_COUNTER, RONLY, var_snmpMPDStats, 1, {2}},
#define   SNMPUNKNOWNPDUHANDLERS     2
    {SNMPUNKNOWNPDUHANDLERS, ASN_COUNTER, RONLY, var_snmpMPDStats, 1, {3}},
};


/*
 * Initializes the lagMIBObjects module
 */
void
ucd_snmp_init_snmpMPDStats(void)
{
    DEBUGMSGTL(("snmpMPDStats", "Initializing\n"));

    /*
     * register ourselves with the agent to handle our mib tree
     */
    REGISTER_MIB("snmpMPDStats", snmpMPDStats_variables, variable2,
                 snmpMPDStats_variables_oid);
}

u_char         *
var_snmpMPDStats(struct variable *vp,
                 oid *name,
                 size_t *length,
                 int exact, size_t *var_len, WriteMethod **write_method)
{
    /*
     * variables we may use later
     * static long long_ret;
     * static u_long ulong_ret;
     * static u_char string[SPRINT_MAX_LEN];
     * static oid objid[MAX_OID_LEN];
     * static struct counter64 c64;
     */
    static long     long_ret;
    int             tmagic;

    *write_method = 0;          /* assume it isnt writable for the time being */
    *var_len = sizeof(long_ret);        /* assume an integer and change later if not */

    if (header_generic(vp, name, length, exact, var_len, write_method)) {
        return 0;
    }

    /*
     * this is where we do the value assignments for the mib results.
     */
    tmagic = vp->magic;
    if ((tmagic >= 0)
        && (tmagic <= (STAT_MPD_STATS_END - STAT_MPD_STATS_START))) {
        long_ret = snmp_get_statistic(tmagic + STAT_MPD_STATS_START);
        return (unsigned char *) &long_ret;
    }
    return 0;
}
