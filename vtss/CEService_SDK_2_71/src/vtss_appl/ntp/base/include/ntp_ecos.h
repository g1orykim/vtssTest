/* Ecos NTP Features (2009/03/26)
   ---------------------------------------
    * Support NTPv4 client function only with previously backward compatible.
    * Support maximum 5 servers.
    * Support IPv6.
    * Support time zone offset.
    * Stackable supported: The NTP module supported stackable architecture. This
    *                      module is only operating in the master. Due to whole
    *                      system only need a system time. So the NTP time isn¡¦t
    *                      needed on the slaves.
*/

#ifndef _NTP_ECOS_H_
#define _NTP_ECOS_H_

#include <sys/socket.h>

#undef HAVE_CONFIG_H
#undef DISABLE_IPV6

// Align a value to a natural multiple (4)
#define _ALIGN(n) (((n)+3)&~3)
# define MAXHOSTNAMELEN 64
# define HAVE_NO_NICE 1

#define TIME_BASEDIFF           ((((cyg_uint32)70*365 + 17) * 24*3600))
#define TIME_NTP_TO_LOCAL(t)	((t)-TIME_BASEDIFF)
#define TIME_LOCAL_TO_NTP(t)	((t)+TIME_BASEDIFF)
#define NTP_SERVER_MAX_NUM 5

/* ntp ip type */
typedef enum {
    NTP_IPV4,
    NTP_IPV6,    
} ntp_ip_type_t;

typedef struct {
    unsigned char           ip_addr_string[46];     /* IP address or Hostname */
    struct sockaddr_storage ntp_sockaddr;
} ntp_ip_dns_mapping;

extern ntp_ip_dns_mapping ntp_mapping[NTP_SERVER_MAX_NUM];

extern void *base_ntp_client( void *args );
extern int ntp_set_ntp_interval(int interval_min, int interval_max);
extern int ntp_set_ntp_server(int ip_type, char *ip_addr_string);
extern int ntp_unset_ntp_server(int ip_type, char *ip_addr_string);
extern bool find_hostname_by_addr(struct sockaddr_storage *addr, u_char *hostname);
extern void ntp_timer_reset(void);
extern void ntp_timer_init(void);
#endif /* _NTP_ECOS_H_ */

