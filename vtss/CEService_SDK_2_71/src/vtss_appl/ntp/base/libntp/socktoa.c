/*
 * socktoa - return a numeric host name from a sockaddr_storage structure
 */

#ifdef NTP_ECOS
#include "ntp_config_ecos.h"
#else
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#endif

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <arpa/inet.h>

#ifdef ISC_PLATFORM_NEEDNTOP
#include <isc/net.h>
#endif

#include <stdio.h>

#include "ntp_fp.h"
#include "lib_strbuf.h"
#include "ntp_stdlib.h"
#include "ntp.h"

char *
socktoa(
	struct sockaddr_storage* sock
	)
{
	register char *buffer;

	LIB_GETBUF(buffer);

	if (sock == NULL)
		strcpy(buffer, "null");
	else
	{

		switch(sock->ss_family) {

		default:
		case AF_INET : 
			inet_ntop(AF_INET, (const void *)&GET_INADDR(*sock), buffer,
			    LIB_BUFLENGTH);
			break;

		case AF_INET6 :
			inet_ntop(AF_INET6, (const void *)&GET_INADDR6(*sock), buffer,
			    LIB_BUFLENGTH);
#if 0
		default:
			strcpy(buffer, "unknown");
#endif
		}
	}
  	return buffer;
}
