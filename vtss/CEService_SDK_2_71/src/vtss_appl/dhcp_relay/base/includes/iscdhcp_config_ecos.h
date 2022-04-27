/* iscdhcp_config_ecos.h

   Platform parameters definition for eCos. */

/*
 * Copyright(c) 2004-2008 by Internet Systems Consortium, Inc.("ISC")
 * Copyright(c) 1997-2003 by Internet Software Consortium
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND ISC DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS.  IN NO EVENT SHALL ISC BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT
 * OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 *   Internet Systems Consortium, Inc.
 *   950 Charter Street
 *   Redwood City, CA 94063
 *   <info@isc.org>
 *   http://www.isc.org/
 *
 * This software has been written for Internet Systems Consortium
 * by Ted Lemon in cooperation with Vixie Enterprises and Nominum, Inc.
 * To learn more about Internet Systems Consortium, see
 * ``http://www.isc.org/''.  To learn more about Vixie Enterprises,
 * see ``http://www.vix.com''.   To learn more about Nominum, Inc., see
 * ``http://www.nominum.com''.
 */

#ifndef _ISCDHCP_CONFIG_ECOS_H_
#define _ISCDHCP_CONFIG_ECOS_H_


/* Define to compile debug-only DHCP software. */
#undef DEBUG

/* Define to queue multiple DHCPACK replies per fsync. */
#undef DELAYED_ACK

/* Define to BIG_ENDIAN for MSB (Motorola or SPARC CPUs) or LITTLE_ENDIAN for
   LSB (Intel CPUs). */
#define DHCP_BYTE_ORDER LITTLE_ENDIAN

/* Define to 1 to include DHCPv6 support. */
#undef DHCPv6

/* Define to any value to chroot() prior to loading config. */
#undef EARLY_CHROOT

/* Define to include execute() config language support. */
#undef ENABLE_EXECUTE

/* Define to include Failover Protocol support. */
#undef FAILOVER_PROTOCOL

/* Define to 1 if you have the /dev/random file. */
#undef HAVE_DEV_RANDOM

/* Define to 1 if you have the <ifaddrs.h> header file. */
#define HAVE_IFADDRS_H 1

/* Define to 1 if you have the <inttypes.h> header file. */
#undef HAVE_INTTYPES_H

/* Define to 1 if you have the <linux/types.h> header file. */
#undef HAVE_LINUX_TYPES_H

/* Define to 1 if you have the <memory.h> header file. */
#undef HAVE_MEMORY_H

/* Define to 1 if you have the <net/if6.h> header file. */
#undef HAVE_NET_IF6_H

/* Define to 1 if you have the <net/if_dl.h> header file. */
#define HAVE_NET_IF_DL_H 1

/* Define to 1 if you have the <regex.h> header file. */
#undef HAVE_REGEX_H

/* Define to 1 if the sockaddr structure has a length field. */
#define HAVE_SA_LEN 1

/* Define to 1 if you have the <stdint.h> header file. */
#undef HAVE_STDINT_H

/* Define to 1 if you have the <stdlib.h> header file. */
#define HAVE_STDLIB_H 1

/* Define to 1 if you have the <strings.h> header file. */
#undef HAVE_STRINGS_H

/* Define to 1 if you have the <string.h> header file. */
#define HAVE_STRING_H 1

/* Define to 1 if you have the <sys/socket.h> header file. */
#define HAVE_SYS_SOCKET_H 1

/* Define to 1 if you have the <sys/stat.h> header file. */
#define HAVE_SYS_STAT_H 1

/* Define to 1 if you have the <sys/types.h> header file. */
#define HAVE_SYS_TYPES_H 1

/* Define to 1 if you have the <unistd.h> header file. */
#define HAVE_UNISTD_H 1

/* Define to 1 if the system has 'struct if_laddrconf'. */
#undef ISC_PLATFORM_HAVEIF_LADDRCONF

/* Define to 1 if the system has 'struct if_laddrreq'. */
#define ISC_PLATFORM_HAVEIF_LADDRREQ 1

/* Define to 1 if the system has 'struct lifnum'. */
#undef ISC_PLATFORM_HAVELIFNUM

/* Define to 1 if the inet_aton() function is missing. */
#undef NEED_INET_ATON

/* Name of package */
#undef PACKAGE

/* Define to the address where bug reports for this package should be sent. */
#undef PACKAGE_BUGREPORT

/* Define to the full name of this package. */
#undef PACKAGE_NAME

/* Define to the full name and version of this package. */
#undef PACKAGE_STRING

/* Define to the one symbol short name of this package. */
#undef PACKAGE_TARNAME

/* Define to the version of this package. */
#undef PACKAGE_VERSION

/* Define to any value to include Ari's PARANOIA patch. */
#undef PARANOIA

/* The size of `struct iaddr *', as computed by sizeof. */
#undef SIZEOF_STRUCT_IADDR_P

/* Define to 1 if you have the ANSI C header files. */
#define STDC_HEADERS 1

/* Define to include server activity tracing support. */
#undef TRACING

/* Define to 1 to use the Berkeley Packet Filter interface code. */
#undef USE_BPF

/* Define to 1 to use DLPI interface code. */
#undef USE_DLPI

/* Define to 1 to use the Linux Packet Filter interface code. */
#undef USE_LPF

/* Version number of package */
#undef VERSION

/* File for dhclient6 leases. */
#undef _PATH_DHCLIENT6_DB

/* File for dhclient6 process information. */
#undef _PATH_DHCLIENT6_PID

/* File for dhclient leases. */
#undef _PATH_DHCLIENT_DB

/* File for dhclient process information. */
#undef _PATH_DHCLIENT_PID

/* File for dhcpd6 leases. */
#undef _PATH_DHCPD6_DB

/* File for dhcpd6 process information. */
#undef _PATH_DHCPD6_PID

/* File for dhcpd leases. */
#undef _PATH_DHCPD_DB

/* File for dhcpd process information. */
#undef _PATH_DHCPD_PID

/* File for dhcrelay process information. */
#undef _PATH_DHCRELAY_PID


/*-----------------------------------------------*/
//#define ISCDHCP_ECOS
#define NO_H_ERRNO

#define LOG_EMERG       0
#define LOG_ALERT       1
#define LOG_CRIT        2
#define LOG_ERR         3
#define LOG_WARNING     4
#define LOG_NOTICE      5
#define LOG_INFO        6
#define LOG_DEBUG       7

#if !defined(isascii)	/* XXX - could be a function */
# define isascii(c) (!(c & 0200))
#endif

#define USE_SOCKET
#define USE_SOCKET_SEND
#define USE_SOCKET_RECEIVE

#define GET_HOST_ID_MISSING
#define _PATH_DHCRELAY_PID      ""

#undef  HAVE_GET_HW_ADDR

#endif /* _ISCDHCP_CONFIG_ECOS_H_ */
