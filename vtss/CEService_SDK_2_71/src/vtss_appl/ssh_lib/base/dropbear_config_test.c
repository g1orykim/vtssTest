#ifdef DROPBEAR_ECOS
#include "dropbear_config_ecos.h"
#else
#include "config.h"
#endif /* DROPBEAR_ECOS */

#if 0
#include <crypt.h>
#include <fcntl.h>
#include <inttypes.h>
#include <ioctl.h>
#include <lastlog.h>
#include <libgen.h>
#include <libutil.h>
#include <limits.h>
#include <memory.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netinet/tcp.h>
#include <pam/pam_appl.h>
#include <paths.h>
#include <pty.h>
#include <security/pam_appl.h>
#include <shadow.h>
#include <stdint.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <stropts.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <termios.h>
#include <unistd.h>
#include <util.h>
#include <utmpx.h>
#include <utmp.h>
#include <sys/time.h>
#include <time.h>
#include <sys/types.h>
#endif

void dropbear_config_test(void)
{
#if 0
    lastlog();
    pututline();
    pututxline();
    gai_strerror();
    login();
    openpty();
#endif

#if 0
    basename();
    clearenv();
    daemon();
    dup2();
    endutent();
    endutxent();
    freeaddrinfo();
    gai_strerror();
    getaddrinfo();
    getnameinfo();
    getspnam();
    getusershell();
    getutent();
    getutid();
    getutline();
    getutxent();
    getutxid();
    getutxline();
    logout();
    logwtmp();
    memset();
    putenv();
    pututline();
    pututxline();
    select();
    setutent();
    setutxent();
    socket();
    strdup();
    strlcat();
    strlcpy();
    updwtmp();
    utmpname();
    utmpxname();
    _getpty();
#endif
}
xxx
