#ifndef __ntp_proto_h
#define __ntp_proto_h

#ifdef NTP_ECOS
#include "ntp_config_ecos.h"
#else
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#endif

#define NTP_MAXFREQ	500e-6
 
#endif /* __ntp_proto_h */
