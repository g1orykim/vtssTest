/*

 Vitesse Switch API software.

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

#include "main.h"
#include "cli.h"
#include "cli_api.h"
#include "control_api.h"
#if defined(CYGPKG_FS_RAM) && defined(VTSS_SW_OPTION_ICFG)
#include "os_file_api.h"
#endif  /* CYGPKG_FS_RAM && VTSS_SW_OPTION_ICFG */
#ifdef VTSS_SW_OPTION_DEBUG
#include "vtss_api_if_api.h" /* For vtss_api_if_reg_access_cnt_get() */
#endif

#if defined(CYGPKG_FS_RAM) && defined(VTSS_SW_OPTION_ICFG)
#include <unistd.h>
#include <dirent.h>
#include <fcntl.h>
#include <sys/stat.h>
#endif

static void cli_cmd_sys_wait(cli_req_t *req)
{
    cli_printf("Wait %d usecs\n", req->int_values[0]);
    cyg_scheduler_lock();
    HAL_DELAY_US(req->int_values[0]);
    cyg_scheduler_unlock();
    cli_printf("Wait done\n");
}

cli_cmd_tab_entry(
    NULL,
    "Debug Wait <integer>",
    "Debug Wait N usecs",
    100,
    CLI_CMD_TYPE_STATUS,
    VTSS_MODULE_ID_DEBUG,
    cli_cmd_sys_wait,
    NULL,
    NULL,
    CLI_CMD_FLAG_NONE
);

static void cli_cmd_sys_load(cli_req_t *req)
{
    cyg_uint32 average_point1s, average_1s, average_10s;
    if(control_sys_get_cpuload(&average_point1s, &average_1s, &average_10s)) {
        cli_printf("Load average(100ms, 1s, 10s): %3d%%, %3d%%, %3d%%\n",
                   average_point1s, average_1s, average_10s);
    } else {
        cli_printf("No performance data available in system\n");
    }
}

cli_cmd_tab_entry (
  NULL,
  "System Load",
  "Show current CPU load: 100ms, 1s and 10s running average (in percent, zero is idle)",
  100,
  CLI_CMD_TYPE_STATUS,
  VTSS_MODULE_ID_SYSTEM,
  cli_cmd_sys_load,
  NULL,
  NULL,
  CLI_CMD_FLAG_NONE
);

#ifdef CYGPKG_PROFILE_GPROF

#include <cyg/profile/profile.h>

static int32_t cli_profile_keyword_parse(char *cmd, char *cmd2, char *stx,
                                         char *cmd_org, cli_req_t *req)
{
    char *found = cli_parse_find(cmd, stx);
    req->parm_parsed = 1;
    if (found != NULL) {
        if (!strncmp(found, "resume", 6))
            req->enable = TRUE;
        else if (!strncmp(found, "off", 3))
            req->disable = TRUE;
        else if (!strncmp(found, "suspend", 7))
            req->clear = TRUE;
    }
    return (found == NULL ? 1 : 0);
}

static int32_t cli_profile_resolution_parse(char *cmd, char *cmd2, char *stx,
                                            char *cmd_org, cli_req_t *req)
{
    int32_t error = 0;
    ulong ul;

    req->parm_parsed = 1;
    error = cli_parse_ulong(cmd, &ul, 10, 5000);
    req->int_values[0] = (int)ul;

    return error;
}

static int32_t cli_profile_bucket_parse(char *cmd, char *cmd2, char *stx,
                                        char *cmd_org, cli_req_t *req)
{
    int32_t error = 0;
    ulong ul;

    req->parm_parsed = 1;
    error = cli_parse_ulong(cmd, &ul, 16, 1024);
    req->int_values[1] = (int)ul;

    return error;
}

static void cli_cmd_profile_on(cli_req_t *req)
{
    extern char _stext[], _etext[];
    if(!req->int_values[0])
        req->int_values[0] = 100; /* default: 100 microsec resolution */
    if(!req->int_values[1])
        req->int_values[1] = 64; /* default: 64 byte bucket */

    profile_on(_stext, _etext, req->int_values[1], req->int_values[0]);
}

static void cli_cmd_profile_ctl(cli_req_t *req)
{
    if(req->enable) {
        CPRINTF("Resuming profiling.\n");
        profile_resume();
    } else if(req->disable) {
        profile_off();
        CPRINTF("Profiling disabled (and cleared).\n");
    } else if(req->clear) {
        profile_suspend();
        CPRINTF("Profiling suspended.\n");
    } else {
        CPRINTF("Please select resume/suspend/disable.\n");
    }
}

static cli_parm_t profile_cli_parm_table[] = {
    {
        "suspend|resume|off",
        "suspend, resume or disable profiling",
        CLI_PARM_FLAG_NONE,
        cli_profile_keyword_parse,
        cli_cmd_profile_ctl
    },
    {
        "<resolution>",
        "sample resolution: 10-5000 microseconds",
        CLI_PARM_FLAG_SET,
        cli_profile_resolution_parse,
        cli_cmd_profile_on
    },
    {
        "<bucket>",
        "sample bucket size: 16-1024 instructions",
        CLI_PARM_FLAG_SET,
        cli_profile_bucket_parse,
        cli_cmd_profile_on
    },
    {
        NULL,
        NULL,
        0,
        0,
        NULL
    },
};

cli_cmd_tab_entry (
  NULL,
  "debug profile on [<resolution>] [<bucket>]",
  "Start profiling",
  100,
  CLI_CMD_TYPE_STATUS,
  VTSS_MODULE_ID_SYSTEM,
  cli_cmd_profile_on,
  NULL,
  profile_cli_parm_table,
  CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
  NULL,
  "debug profile [suspend|resume|off]",
  "Suspend/Resume or disable profiling",
  100,
  CLI_CMD_TYPE_STATUS,
  VTSS_MODULE_ID_SYSTEM,
  cli_cmd_profile_ctl,
  NULL,
  profile_cli_parm_table,
  CLI_CMD_FLAG_NONE
);

#endif

#if defined(CYGPKG_FS_RAM) && defined(VTSS_SW_OPTION_ICFG)
static void cli_cmd_file_sync(cli_req_t *req)
{
    os_file_fs2flash();
}

static void cli_cmd_file_restore(cli_req_t *req)
{
    os_file_flash2fs();
}

static void cli_cmd_file_ls(cli_req_t *req)
{
    char * name = "/";
    int err;
    DIR *dirp;

    CPRINTF("Reading directory %s\n",name);

    dirp = opendir( name );
    if(dirp == NULL) {
        CPRINTF("opendir: %s\n", strerror(errno));
        return;
    }

    for(;;) {
        struct dirent *entry = readdir( dirp );
        char fullname[PATH_MAX];
        struct stat sbuf;

        if( entry == NULL )
            break;

        if (name[0]) {
            strcpy(fullname, name );
            if (!(name[0] == '/' && name[1] == 0 ))
                strcat(fullname, "/" );
        } else {
            fullname[0] = 0;
        }
        strcat(fullname, entry->d_name );
        err = stat( fullname, &sbuf );
        if( err < 0 ) {
            if( errno == ENOSYS )
                CPRINTF("%s: <no status available>\n", fullname);
            else {
                CPRINTF("%s: stat: %s\n", fullname, strerror(errno));
            }
        } else {
            CPRINTF("%14s [mode %08x ino %08x nlink %d mtime %s size %ld]\n",
                    entry->d_name,
                    sbuf.st_mode, sbuf.st_ino, sbuf.st_nlink, misc_time2str(sbuf.st_mtime),
                    (unsigned long) sbuf.st_size);
        }
    }

    err = closedir( dirp );
    if( err < 0 ) {
        CPRINTF("closedir: %s\n", strerror(errno));
    }
}

#define IOSIZE  1024
static void cli_cmd_file_write(cli_req_t *req)
{
    char buf[IOSIZE];
    int fd;
    ssize_t wrote;
    int i;
    int err;
    size_t size = req->int_values[0];
    const char *name = req->parm;

    CPRINTF("Create file %s size %zd\n", name, size);

    err = access( name, F_OK );
    if (err < 0 && errno != EACCES) {
        CPRINTF("%s: %s\n", name, strerror(errno));
        return;
    }

    for( i = 0; i < IOSIZE; i++ ) {
        buf[i] = i%256;
    }

    fd = open(name, O_WRONLY|O_CREAT|O_TRUNC);
    if (fd < 0 ) {
        CPRINTF("%s: %s\n", name, strerror(errno));
        return;
    }

    while( size > 0 ) {
        ssize_t len = size;
        if (len > IOSIZE)
            len = IOSIZE;
        wrote = write( fd, buf, len );
        if( wrote != len ) {
            CPRINTF("%s: Requested %ld, wrote %ld\n", name, len, wrote);
            break;
        }
        size -= wrote;
    }

    err = close( fd );
    if( err < 0 ) {
        CPRINTF("%s: %s\n", name, strerror(errno));
    }
}

static void cli_cmd_file_delete(cli_req_t *req)
{
    const char *name = req->parm;

    CPRINTF("Delete file %s\n", name);

    if(unlink(name) < 0) {
        CPRINTF("%s: %s\n", name, strerror(errno));
    }
}

static int32_t cli_file_name_parse (char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    int32_t error = 0;

    req->parm_parsed = 1;
    error = cli_parse_raw(cmd_org, req);

    return error;
}

static int32_t cli_file_size_parse(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    int32_t error = 0;
    ulong ul;

    req->parm_parsed = 1;
    error = cli_parse_ulong(cmd, &ul, 0, 32 * 1024 * 1024);
    req->int_values[0] = (int)ul;

    return error;
}

static cli_parm_t file_cli_parm_table[] = {
    {
        "<file_name>",
        "Firmware file name",
        CLI_PARM_FLAG_NONE,
        cli_file_name_parse,
        NULL
    },
    {
        "<size>",
        "File size: 0 to 3 megabytes",
        CLI_PARM_FLAG_SET,
        cli_file_size_parse,
        NULL
    },
    {
        NULL,
        NULL,
        0,
        0,
        NULL
    },
};

cli_cmd_tab_entry (
  NULL,
  "debug file sync",
  "Synchronize FS to flash",
  100,
  CLI_CMD_TYPE_STATUS,
  VTSS_MODULE_ID_SYSTEM,
  cli_cmd_file_sync,
  NULL,
  file_cli_parm_table,
  CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
  NULL,
  "debug file restore",
  "Synchronize flash to FS",
  100,
  CLI_CMD_TYPE_STATUS,
  VTSS_MODULE_ID_SYSTEM,
  cli_cmd_file_restore,
  NULL,
  file_cli_parm_table,
  CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
  NULL,
  "debug file ls",
  "list / directory",
  100,
  CLI_CMD_TYPE_STATUS,
  VTSS_MODULE_ID_SYSTEM,
  cli_cmd_file_ls,
  NULL,
  file_cli_parm_table,
  CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
  NULL,
  "debug file write <file_name> <size>",
  "create file",
  100,
  CLI_CMD_TYPE_STATUS,
  VTSS_MODULE_ID_SYSTEM,
  cli_cmd_file_write,
  NULL,
  file_cli_parm_table,
  CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
  NULL,
  "debug file delete <file_name>",
  "delete file",
  100,
  CLI_CMD_TYPE_STATUS,
  VTSS_MODULE_ID_SYSTEM,
  cli_cmd_file_delete,
  NULL,
  file_cli_parm_table,
  CLI_CMD_FLAG_NONE
);
#endif  /* CYGPKG_FS_RAM && VTSS_SW_OPTION_ICFG */

#ifdef CYGPKG_HAL_ARM_ARM9_ARM926EJ

static inline volatile cyg_uint32 rd_domac(void)
{
    cyg_uint32 reg = 0;
    asm volatile("mrc p15, 0, %[oreg], c3, c0, 0" : [oreg] "=r" (reg));
    return reg;
}

/*lint -esym(522, wr_domac) */
static inline void wr_domac(cyg_uint32 domac)
{
    asm volatile ("mcr p15, 0, %[ireg], c3, c0, 0" : : [ireg] "r"(domac));
}

static void cli_cmd_memprotect(cli_req_t *req)
{
    if(req->set) {
        if(req->enable) {
            CPRINTF("Enable protection.\n");
            wr_domac(0x1);      /* Client */
        } else {
            CPRINTF("Disable protection.\n");
            wr_domac(0x3);      /* Manager */
        }
    } else {
        CPRINTF("Protection is %s.\n", (rd_domac() & 0x3) == 0x3 ? "off" : "on");
    }
}

static cli_parm_t protect_cli_parm_table[] = {
    {
        "enable|disable",
        "enable : Enable protection\n"
        "disable: Disable protection",
        CLI_PARM_FLAG_NO_TXT | CLI_PARM_FLAG_SET,
        cli_parm_parse_keyword,
        cli_cmd_memprotect
    },
    {
        NULL,
        NULL,
        0,
        0,
        NULL
    },
};

cli_cmd_tab_entry (
  NULL,
  "debug memory protect [enable|disable]",
  "Enable/Disable low memory read-only",
  100,
  CLI_CMD_TYPE_STATUS,
  VTSS_MODULE_ID_SYSTEM,
  cli_cmd_memprotect,
  NULL,
  protect_cli_parm_table,
  CLI_CMD_FLAG_NONE
);

#endif

#ifdef CYG_HAL_IRQCOUNT_SUPPORT

static int32_t cli_irqcount_keyword_parse(char *cmd, char *cmd2, char *stx,
                                          char *cmd_org, cli_req_t *req)
{
    char *found = cli_parse_find(cmd, stx);
    req->parm_parsed = 1;
    if (found != NULL) {
        if (found[0] == 'c')
            req->clear = TRUE;
    }
    return (found == NULL ? 1 : 0);
}

static void cli_cmd_irqcount(cli_req_t *req)
{
    cyg_vector_t i;

    if(!req->clear)
        cli_table_header("IRQ  Vector      IRQ Count       ");
    for(i = CYGNUM_HAL_ISR_MIN; i <= CYGNUM_HAL_ISR_MAX; i++) {
        if(req->clear)
            hal_irqcount_clear(i);
        else {
            cyg_uint64 count = hal_irqcount_read(i);
            if(count) {
                const char *name;
#ifdef CYG_HAL_NAMES_SUPPORT
                name = hal_interrupt_name[i];
#else
                name = NULL;
#endif
                if(!name)
                    name = "-";
                CPRINTF("%3d  %-10.10s  %14llu\n", i, name, count);
            }
        }
    }
}

static cli_parm_t irqcount_cli_parm_table[] = {
    {
        "clear",
        "Clear interrupt counters",
        CLI_PARM_FLAG_SET,
        cli_irqcount_keyword_parse,
        cli_cmd_irqcount
    },
    {
        NULL,
    },
};

cli_cmd_tab_entry (
  NULL,
  "Debug IRQ [clear]",
  "Display interrupt statistics",
  CLI_CMD_SORT_KEY_DEFAULT,
  CLI_CMD_TYPE_STATUS,
  VTSS_MODULE_ID_SYSTEM,
  cli_cmd_irqcount,
  NULL,
  irqcount_cli_parm_table,
  CLI_CMD_FLAG_NONE
);

#endif // CYG_HAL_IRQCOUNT_SUPPORT

#if defined(VTSS_SW_OPTION_DEBUG)
static cyg_handle_t access_statistics_alarm_handle;
static cyg_alarm    access_statistics_alarm;
static struct {
  struct {
    u64 last;
    u64 sec1;
    u64 sec10;
  } c[2]; // Index 0 == chip #0, index 1 == chip #1 (if any).
} access_statistics[2]; // Index 0 == Reads, index 1 == Writes

/******************************************************************************/
// access_statistics_alarm_func()
/******************************************************************************/
static void access_statistics_alarm_func(cyg_handle_t halarm, cyg_addrword_t data)
{
    int rw, c;
    u64 total[2][2];
    vtss_api_if_reg_access_cnt_get(total[0], total[1]);
    cyg_scheduler_lock();
    for (rw = 0; rw < 2; rw++) { // rw = r or w
        for (c = 0; c < 2 ; c++) { // c = chip_no
            access_statistics[rw].c[c].sec1  = total[rw][c] - access_statistics[rw].c[c].last;
            access_statistics[rw].c[c].sec10 = access_statistics[rw].c[c].sec1 + (access_statistics[rw].c[c].sec10 * 9ULL) / 10ULL;
            access_statistics[rw].c[c].last = total[rw][c];
        }
    }
    cyg_scheduler_unlock();
}

/******************************************************************************/
// control_access_statistics_start()
/******************************************************************************/
void control_access_statistics_start(void)
{
  cyg_handle_t counter;

  // Create a timer that fires every second.
  cyg_clock_to_counter(cyg_real_time_clock(), &counter);
  cyg_alarm_create(counter,
                   access_statistics_alarm_func,
                   0,
                   &access_statistics_alarm_handle,
                   &access_statistics_alarm);

  cyg_alarm_initialize(access_statistics_alarm_handle, cyg_current_time() + ECOS_TICKS_PER_SEC , ECOS_TICKS_PER_SEC);
  cyg_alarm_enable(access_statistics_alarm_handle);
}

/******************************************************************************/
// control_cli_cmd_access_statistics()
/******************************************************************************/
static void control_cli_cmd_access_statistics(cli_req_t *req)
{
    u64 total[2][2];
    int rw, c, chip_count;
    vtss_api_if_reg_access_cnt_get(total[0], total[1]);

    if (req->clear) {
        cyg_scheduler_lock();
        for (rw = 0; rw < 2; rw++) {
            for (c = 0; c < 2; c++) {
                access_statistics[rw].c[c].last  = total[rw][c];
                access_statistics[rw].c[c].sec1  = 0;
                access_statistics[rw].c[c].sec10 = 0;
            }
        }
        cyg_scheduler_unlock();
        return;
    }

    chip_count = vtss_api_if_chip_count();
    cyg_scheduler_lock();

    if (chip_count > 1) {
        cli_table_header("Chip  R/W  Total             1sec      10sec      ");
    } else {
        cli_table_header(      "R/W  Total             1sec      10sec      ");
    }

    for (c = 0; c < chip_count; c++) {
        for (rw = 0; rw < 2; rw ++) {
            if (chip_count > 1) {
                if (rw == 0) {
                    cli_printf("%4d  ", c);
                } else {
                    cli_printf("      ");
                }
            }
            cli_printf(" %c   %16llu  %8llu  %9llu\n", rw == 0 ? 'R' : 'W', total[rw][c], access_statistics[rw].c[c].sec1, access_statistics[rw].c[c].sec10);
        }
    }

    cyg_scheduler_unlock();
}

#endif /* defined(VTSS_SW_OPTION_DEBUG) */

/******************************************************************************/
// control_cli_cmd_debug_heap()
/******************************************************************************/
/*lint -esym(459, heap_usage_old)         */
/*lint -esym(459, heap_usage_tot_max_old) */
/*lint -esym(459, old_total)              */
static void control_cli_cmd_debug_heap(cli_req_t *req)
{
#if defined(VTSS_SW_OPTION_DEBUG)
    extern heap_usage_t heap_usage_cur[VTSS_MODULE_ID_NONE + 1];
    static heap_usage_t heap_usage_old[VTSS_MODULE_ID_NONE + 1];
           heap_usage_t heap_usage_new[VTSS_MODULE_ID_NONE + 1];
    static heap_usage_t old_total;
           heap_usage_t new_total;
    extern u32          heap_usage_tot_max_cur;
    static u32          heap_usage_tot_max_old;
           u32          heap_usage_tot_max_new;
    vtss_module_id_t    modid;
    int                 i;

// Required in both 32- and 64-bit version, hence a macro
#define PLUS_MINUS(_n_, _o_) ((_n_) < (_o_) ? '-' : (_n_) > (_o_) ? '+' : ' ')

    cyg_scheduler_lock();
    memcpy(heap_usage_new, heap_usage_cur, sizeof(heap_usage_new));
    heap_usage_tot_max_new = heap_usage_tot_max_cur;
    cyg_scheduler_unlock();

    memset(&new_total, 0, sizeof(new_total));

    cli_table_header("Name                 ID   Current     Allocs      Frees       Alive       Max         Total           ");
    for (modid = 0; modid <= VTSS_MODULE_ID_NONE; modid++) {
        heap_usage_t *m = &heap_usage_new[modid];

        new_total.usage  += m->usage;
        new_total.max    += m->max;
        new_total.total  += m->total;
        new_total.allocs += m->allocs;
        new_total.frees  += m->frees;

        if (req->vid_spec == CLI_SPEC_VAL && modid != req->value) {
            continue;
        }

        if (req->vid_spec == CLI_SPEC_VAL || m->total != 0) {
            cli_printf("%-19s  %3d  %10u%c %10u%c %10u%c %10u%c %10u%c %14llu%c\n",
                       vtss_module_names[modid],
                       modid,
                       m->usage,
                       PLUS_MINUS(m->usage,  heap_usage_old[modid].usage),
                       m->allocs,
                       PLUS_MINUS(m->allocs, heap_usage_old[modid].allocs),
                       m->frees,
                       PLUS_MINUS(m->frees,  heap_usage_old[modid].frees),
                       m->allocs - m->frees,
                       PLUS_MINUS(m->allocs - m->frees, heap_usage_old[modid].allocs - heap_usage_old[modid].frees),
                       m->max,
                       PLUS_MINUS(m->max,    heap_usage_old[modid].max),
                       m->total,
                       PLUS_MINUS(m->total, heap_usage_old[modid].total));
        }
    }

    for (i = 0; i < 100; i++) {
        cli_printf("-");
    }

    cli_printf("\nTotal                     %10u%c %10u%c %10u%c %10u%c %10u%c %14llu%c\n\n",
               new_total.usage,
               PLUS_MINUS(new_total.usage,  old_total.usage),
               new_total.allocs,
               PLUS_MINUS(new_total.allocs, old_total.allocs),
               new_total.frees,
               PLUS_MINUS(new_total.frees,  old_total.frees),
               new_total.allocs - new_total.frees,
               PLUS_MINUS(new_total.allocs - new_total.frees, old_total.allocs - old_total.frees),
               new_total.max,
               PLUS_MINUS(new_total.max,    old_total.max),
               new_total.total,
               PLUS_MINUS(new_total.total,  old_total.total));

    cli_printf("Max. allocated: %u%c\n", heap_usage_tot_max_new, PLUS_MINUS(heap_usage_tot_max_new, heap_usage_tot_max_old));
    cli_printf("A '+' after a number indicates it has increased, and a '-' that it has decreased since last printout.\n\n");

    memcpy(heap_usage_old, heap_usage_new, sizeof(heap_usage_old));
    heap_usage_tot_max_old = heap_usage_tot_max_new;
    old_total              = new_total;
#undef PLUS_MINUS
#endif /* defined defined(VTSS_SW_OPTION_DEBUG) */

#if CYGINT_ISO_MALLINFO
    {
        struct mallinfo mem_info;

        mem_info = mallinfo();

        cli_printf("OS-Total=0x%08x (%d KBytes) OS-Free=0x%08x (%d KBytes) OS-Max=0x%08x (%d KBytes)\n",
                   mem_info.arena, mem_info.arena / 1024,
                   mem_info.fordblks, mem_info.fordblks / 1024,
                   mem_info.maxfree, mem_info.maxfree / 1024);
    }
#endif /* CYGINT_ISO_MALLINFO */
}

#if defined(VTSS_SW_OPTION_DEBUG)
static int32_t cli_parm_parse_module_id(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    req->parm_parsed = 1;
    req->vid_spec = CLI_SPEC_VAL; // Hack to indicate that user has specified a module_id
    return cli_parse_ulong(cmd, &req->value, 0, VTSS_MODULE_ID_NONE);
}
#endif /* defined(VTSS_SW_OPTION_DEBUG) */

#if defined(VTSS_SW_OPTION_DEBUG)
/******************************************************************************/
/******************************************************************************/
static cli_parm_t control_cli_debug_parm_table[] = {
    {
        "clear",
        "Clear register access counters",
        CLI_PARM_FLAG_SET,
        cli_parm_parse_keyword,
        control_cli_cmd_access_statistics
    },
    {
        "<module_id>",
        "Module ID",
        CLI_PARM_FLAG_SET,
        cli_parm_parse_module_id,
        control_cli_cmd_debug_heap
    },
    {
        NULL,
    },
};
#endif /* defined(VTSS_SW_OPTION_DEBUG) */

#if defined(VTSS_SW_OPTION_DEBUG)
cli_cmd_tab_entry (
    NULL,
    "Debug Accesses [clear]",
    "Display number of register accesses within the last one and ten seconds.",
    CLI_CMD_SORT_KEY_DEFAULT,
    CLI_CMD_TYPE_STATUS,
    VTSS_MODULE_ID_SYSTEM,
    control_cli_cmd_access_statistics,
    NULL,
    control_cli_debug_parm_table,
    CLI_CMD_FLAG_NONE
);
#endif /* defined(VTSS_SW_OPTION_DEBUG) */

#if defined(VTSS_SW_OPTION_DEBUG)
cli_cmd_tab_entry (
    "Debug Heap [<module_id>]",
    NULL,
    "Show per module heap allocation and statistics",
    CLI_CMD_SORT_KEY_DEFAULT,
    CLI_CMD_TYPE_STATUS,
    VTSS_MODULE_ID_DEBUG,
    control_cli_cmd_debug_heap,
    NULL,
    control_cli_debug_parm_table,
    CLI_CMD_FLAG_NONE
);
#else
cli_cmd_tab_entry (
    "Debug Heap",
    NULL,
    "Show heap allocation",
    CLI_CMD_SORT_KEY_DEFAULT,
    CLI_CMD_TYPE_STATUS,
    VTSS_MODULE_ID_DEBUG,
    control_cli_cmd_debug_heap,
    NULL,
    NULL,
    CLI_CMD_FLAG_NONE
);
#endif /* defined(VTSS_SW_OPTION_DEBUG) */

