///////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2000-2003 Intel Corporation
// All rights reserved. 
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// * Redistributions of source code must retain the above copyright notice,
// this list of conditions and the following disclaimer.
// * Redistributions in binary form must reproduce the above copyright notice,
// this list of conditions and the following disclaimer in the documentation
// and/or other materials provided with the distribution.
// * Neither name of Intel Corporation nor the names of its contributors
// may be used to endorse or promote products derived from this software
// without specific prior written permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL INTEL OR
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
// OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
///////////////////////////////////////////////////////////////////////////

#include "config.h"

#ifdef INCLUDE_DEVICE_APIS
#if EXCLUDE_SSDP == 0

#ifdef VTSS_SW_OPTION_IP2
#include "ip2_api.h"
#include "misc_api.h"
#endif /* VTSS_SW_OPTION_IP2 */
#ifdef LIBUPNP_ECOS
#include "vtss_upnp.h"
#include "sock.h"
#endif /* LIBUPNP_ECOS */

#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "ssdplib.h"
#include "upnpapi.h"
#include "ThreadPool.h"
#include "httpparser.h"
#include "httpreadwrite.h"
#include "statcodes.h"
#include "unixutil.h"


#ifdef WIN32
	#include <ws2tcpip.h>
	#include <winsock2.h>
#endif /* WIN32 */

#define MSGTYPE_SHUTDOWN	0
#define MSGTYPE_ADVERTISEMENT	1
#define MSGTYPE_REPLY		2

/************************************************************************
* Function : advertiseAndReplyThread
*
* Parameters:
*	IN void *data: Structure containing the search request
*
* Description:
*	This function is a wrapper function to reply the search request
*	coming from the control point.
*
* Returns: void *
*	always return NULL
***************************************************************************/
void *
advertiseAndReplyThread( IN void *data )
{
    SsdpSearchReply *arg = ( SsdpSearchReply * ) data;
#if 0   
    static int i =0;
   
    printf("advertiseAndReplyThread  - %d\n",++i);
#endif     
    AdvertiseAndReply( 0, arg->handle,
                       arg->event.RequestType,
                       &arg->dest_addr,
                       arg->event.DeviceType,
                       arg->event.UDN,
                       arg->event.ServiceType, arg->MaxAge );
    upnp_callout_free( arg );

    return NULL;
}

/************************************************************************
* Function : ssdp_handle_device_request
*
* Parameters:
*	IN http_message_t* hmsg: SSDP search request from the control point
*	IN struct sockaddr_in* dest_addr: The address info of control point
*
* Description:
*	This function handles the search request. It do the sanity checks of
*	the request and then schedules a thread to send a random time reply (
*	random within maximum time given by the control point to reply).
*
* Returns: void *
*	1 if successful else appropriate error
***************************************************************************/
#ifdef INCLUDE_DEVICE_APIS
void
ssdp_handle_device_request( IN http_message_t * hmsg,
                            IN struct sockaddr_in *dest_addr )
{
#define MX_FUDGE_FACTOR 10

    int handle;
    struct Handle_Info *dev_info = NULL;
    memptr hdr_value;
    int mx;
    char save_char;
    SsdpEvent event;
    int ret_code;
    SsdpSearchReply *threadArg = NULL;
    ThreadPoolJob job;
    int replyTime;
    int maxAge;
#ifdef LIBUPNP_ECOS    
    upnp_conf_t conf;
#endif    

    // check man hdr
    if( httpmsg_find_hdr( hmsg, HDR_MAN, &hdr_value ) == NULL ||
        memptr_cmp( &hdr_value, "\"ssdp:discover\"" ) != 0 ) {
        return;                 // bad or missing hdr
    }
    // MX header
    if( httpmsg_find_hdr( hmsg, HDR_MX, &hdr_value ) == NULL ||
        ( mx = raw_to_int( &hdr_value, 10 ) ) < 0 ) {
        return;
    }
    // ST header
    if( httpmsg_find_hdr( hmsg, HDR_ST, &hdr_value ) == NULL ) {
        return;
    }
    save_char = hdr_value.buf[hdr_value.length];
    hdr_value.buf[hdr_value.length] = '\0';
    ret_code = ssdp_request_type( hdr_value.buf, &event );
    hdr_value.buf[hdr_value.length] = save_char;    // restore
    if( ret_code == -1 ) {
        return;                 // bad ST header
    }

    HandleLock();
    // device info
    if( GetDeviceHandleInfo( &handle, &dev_info ) != HND_DEVICE ) {
        HandleUnlock();
        return;                 // no info found
    }
    maxAge = dev_info->MaxAge;
    
    HandleUnlock();

#ifdef LIBUPNP_ECOS    
    upnp_mgmt_conf_get(&conf);
    maxAge = conf.adv_interval;
#endif        


    UpnpPrintf( UPNP_PACKET, API, __FILE__, __LINE__,
        "ssdp_handle_device_request with Cmd %d SEARCH\n",
        event.Cmd );
    UpnpPrintf( UPNP_PACKET, API, __FILE__, __LINE__,
        "MAX-AGE     =  %d\n", maxAge );
    UpnpPrintf( UPNP_PACKET, API, __FILE__, __LINE__,
        "MX     =  %d\n", event.Mx );
    UpnpPrintf( UPNP_PACKET, API, __FILE__, __LINE__,
        "DeviceType   =  %s\n", event.DeviceType );
    UpnpPrintf( UPNP_PACKET, API, __FILE__, __LINE__,
        "DeviceUuid   =  %s\n", event.UDN );
    UpnpPrintf( UPNP_PACKET, API, __FILE__, __LINE__,
        "ServiceType =  %s\n", event.ServiceType );

    threadArg =
        ( SsdpSearchReply * )upnp_callout_malloc( sizeof( SsdpSearchReply ) );

    if( threadArg == NULL ) {
        return;
    }
    threadArg->handle = handle;
    threadArg->dest_addr = ( *dest_addr );
    threadArg->event = event;
    threadArg->MaxAge = maxAge;

    TPJobInit( &job, advertiseAndReplyThread, threadArg );
    TPJobSetFreeFunction( &job, ( free_routine ) upnp_callout_free );

    //Subtract a percentage from the mx
    //to allow for network and processing delays
    // (i.e. if search is for 30 seconds, 
    //       respond withing 0 - 27 seconds)

    if( mx >= 2 ) {
        mx -= MAXVAL( 1, mx / MX_FUDGE_FACTOR );
    }

    if( mx < 1 ) {
        mx = 1;
    }

    replyTime = rand() % mx;

    TimerThreadSchedule( &gTimerThread, replyTime, REL_SEC, &job,
                         SHORT_TERM, NULL );
}
#endif

#ifdef LIBUPNP_ECOS
extern SOCKET xxx_socket;
#endif

#ifdef VTSS_SW_OPTION_IP2
static void _NewRequestHandler(int ReplySock, i8 *ReqBuf, vtss_ipv4_t intf_adr, void *DestAddr, int ttl)
{
    int             ret;
    struct in_addr  addr;

    if (!ReqBuf || !DestAddr) {
        return;
    }

    memset((void *)&addr, 0x0, sizeof(struct in_addr));
    addr.s_addr = htonl(intf_adr);

    ret = setsockopt(ReplySock, IPPROTO_IP, IP_MULTICAST_IF,
                     (char *)&addr, sizeof(struct in_addr));
    ret = setsockopt(ReplySock, IPPROTO_IP, IP_MULTICAST_TTL,
                     (char *)&ttl, sizeof(int));

    UpnpPrintf(UPNP_INFO, SSDP, __FILE__, __LINE__,
               ">>> SSDP SEND >>>\n%s\n",
               ReqBuf);
#if defined(LIBUPNP_ECOS) && (VTSS_BACKWARD_COMPATIBLE == 1)
    if (DestAddr->sin_port == 0x6c07) {
        ret = sendto(ReplySock, ReqBuf, strlen(ReqBuf), 0,
                     (struct sockaddr *)DestAddr, sizeof(struct sockaddr_in));
    } else {
        ret = setsockopt(xxx_socket, IPPROTO_IP, IP_TTL,
                         (char *)&ttl, sizeof(int));
        ret = sendto(xxx_socket, ReqBuf, strlen(ReqBuf), 0,
                     (struct sockaddr *)DestAddr, sizeof(struct sockaddr_in));
    }
#else
    ret = sendto(ReplySock, ReqBuf, strlen(ReqBuf), 0,
                 (struct sockaddr *)DestAddr, sizeof(struct sockaddr_in));
#endif /* defined(LIBUPNP_ECOS) && (VTSS_BACKWARD_COMPATIBLE == 1) */
}
#endif /* VTSS_SW_OPTION_IP2 */

#ifdef VTSS_SW_OPTION_IP2
void replaceLocationdByIpAddr( vtss_ipv4_t intf_adr, membuffer *mBuf)
{
    char *ssdp_data, *host_str_start, *host_str_end, ip_str[40];
    size_t str_len;
    int index;

    if (!mBuf) {
        return;
    }

    ssdp_data = mBuf->buf;
    host_str_start = strstr( ssdp_data, "LOCATION: http://")+strlen("LOCATION: http://");
    host_str_end = strstr(host_str_start, ":");
    str_len = host_str_end - host_str_start;
    index = host_str_start - ssdp_data;
    
    membuffer_delete( mBuf, index, str_len );
    
    membuffer_insert( mBuf, misc_ipv4_txt(intf_adr, ip_str), strlen(ip_str), index );
}
#endif

/************************************************************************
* Function : NewRequestHandler
*
* Parameters:
*		IN struct sockaddr_in * DestAddr: Ip address, to send the reply.
*		IN int NumPacket: Number of packet to be sent.
*		IN char **RqPacket:Number of packet to be sent.
*		IN membuffer *mBuf: the packet related memory buffer.
*
* Description:
*	This function works as a request handler which passes the HTTP
*	request string to multicast channel then
*
* Returns: void *
*	1 if successful else appropriate error
***************************************************************************/
static int
NewRequestHandler( IN struct sockaddr_in *DestAddr,
                   IN int NumPacket,
                   IN char **RqPacket, IN membuffer *mBuf )
{
    char errorBuffer[ERROR_BUFFER_LEN];
    int ReplySock;
    int NumCopy;
    int Index;
    int ttl = 4; // a/c to UPNP Spec
#ifdef VTSS_SW_OPTION_IP2
    vtss_vid_t          intf_ifid;  /* With IP2, VID is used as the IFID */
    BOOL                intf_up;
    vtss_ipv4_t         intf_adr;
    vtss_if_status_t    *ops, ipst[UPNP_IP_INTF_MAX_OPST];
    u32                 ops_idx;
    u32                 ops_cnt;
#else
    unsigned long replyAddr = inet_addr( LOCAL_HOST );
#endif /* VTSS_SW_OPTION_IP2 */

#ifdef LIBUPNP_ECOS
    upnp_conf_t conf;
    
    upnp_mgmt_conf_get(&conf);
    ttl = conf.ttl;
#endif     

    ReplySock = socket( AF_INET, SOCK_DGRAM, 0 );
    if ( ReplySock == -1 ) {
        strerror_r(errno, errorBuffer, ERROR_BUFFER_LEN);
        UpnpPrintf( UPNP_INFO, SSDP, __FILE__, __LINE__,
            "SSDP_LIB: New Request Handler:"
            "Error in socket(): %s\n", errorBuffer );

        return UPNP_E_OUTOF_SOCKET;
    }
#ifdef VTSS_SW_OPTION_IP2
    for (Index = 0; Index < NumPacket; Index++) {
        for (NumCopy = 0; NumCopy < NUM_COPY; NumCopy++) {
            /* FIXME!SGETZ: Should be based on per-interface-ifid */
            intf_ifid = 0;
            while (UPNP_IP_INTF_IFID_GET_NEXT(intf_ifid)) {
                ops_cnt = 0;
                memset(ipst, 0x0, sizeof(ipst));
                if (!UPNP_IP_INTF_OPST_GET(intf_ifid, ipst, ops_cnt)) {
                    continue;
                }

                intf_adr = 0;
                intf_up = FALSE;
                if (!(ops_cnt > UPNP_IP_INTF_MAX_OPST)) {
                    for (ops_idx = 0; ops_idx < ops_cnt; ops_idx++) {
                        ops = &ipst[ops_idx];

                        if ((ops->type == VTSS_IF_STATUS_TYPE_LINK) &&
                            UPNP_IP_INTF_OPST_UP(ops)) {
                            intf_up = TRUE;
                        }
                        if (ops->type == VTSS_IF_STATUS_TYPE_IPV4) {
                            intf_adr = UPNP_IP_INTF_OPST_ADR4(ops);
                        }
                    }
                }
                if (!intf_up || !intf_adr) {
                    continue;
                }

                replaceLocationdByIpAddr(intf_adr, &mBuf[Index]);
                _NewRequestHandler(ReplySock, *(RqPacket + Index), intf_adr, DestAddr, ttl);
            }

            imillisleep( SSDP_PAUSE );
        }
    }
#else
    setsockopt( ReplySock, IPPROTO_IP, IP_MULTICAST_IF,
        (char *)&replyAddr, sizeof (replyAddr) );
    setsockopt( ReplySock, IPPROTO_IP, IP_MULTICAST_TTL,
        (char *)&ttl, sizeof (int) );

    for( Index = 0; Index < NumPacket; Index++ ) {
        int rc;
        // The reason to keep this loop is purely historical/documentation,
        // according to section 9.2 of HTTPU spec:
        // 
        // "If a multicast resource would send a response(s) to any copy of the 
        //  request, it SHOULD send its response(s) to each copy of the request 
        //  it receives. It MUST NOT repeat its response(s) per copy of the 
        //  request."
        //  
        // http://www.upnp.org/download/draft-goland-http-udp-04.txt
        //
        // So, NUM_COPY has been changed from 2 to 1.
        NumCopy = 0;
        while( NumCopy < NUM_COPY ) {
            UpnpPrintf( UPNP_INFO, SSDP, __FILE__, __LINE__,
                ">>> SSDP SEND >>>\n%s\n",
                *( RqPacket + Index ) );
#ifdef LIBUPNP_ECOS          
#if (VTSS_BACKWARD_COMPATIBLE == 1)          
            if (DestAddr->sin_port == 0x6c07) {                      
#endif
#endif                
                rc = sendto( ReplySock, *( RqPacket + Index ),
                         strlen( *( RqPacket + Index ) ),
                         0, ( struct sockaddr * )DestAddr, sizeof(struct sockaddr_in));
#ifdef LIBUPNP_ECOS                         
#if (VTSS_BACKWARD_COMPATIBLE == 1)
            } else {
                setsockopt( xxx_socket, IPPROTO_IP, IP_TTL,
                    (char *)&ttl, sizeof (int) );             
                rc = sendto( xxx_socket, *( RqPacket + Index ),
                         strlen( *( RqPacket + Index ) ),
                         0, ( struct sockaddr * )DestAddr, sizeof(struct sockaddr_in));                         
            }
#endif
#endif             
            imillisleep( SSDP_PAUSE );
            ++NumCopy;
        }
    }
#endif /* VTSS_SW_OPTION_IP2 */

    shutdown( ReplySock, SD_BOTH );
    UpnpCloseSocket( ReplySock );

    return UPNP_E_SUCCESS;
}

/************************************************************************
* Function : CreateServiceRequestPacket
*
* Parameters:
*	IN int msg_type : type of the message ( Search Reply, Advertisement
*		or Shutdown )
*	IN char * nt : ssdp type
*	IN char * usn : unique service name ( go in the HTTP Header)
*	IN char * location :Location URL.
*	IN int  duration :Service duration in sec.
*	OUT char** packet :Output buffer filled with HTTP statement.
*	OUT membuffer* mBuf : the packet related memory buffer.
*
* Description:
*	This function creates a HTTP request packet.  Depending
*	on the input parameter it either creates a service advertisement
*	request or service shutdown request etc.
*
* Returns: void
*
***************************************************************************/
void
CreateServicePacket( IN int msg_type,
                     IN char *nt,
                     IN char *usn,
                     IN char *location,
                     IN int duration,
                     OUT char **packet, OUT membuffer *mBuf )
{
    int ret_code;
    char *nts;
    membuffer buf;
    size_t    buf_len;

    //Notf=0 means service shutdown, 
    //Notf=1 means service advertisement, Notf =2 means reply   

    membuffer_init( &buf );
    buf.size_inc = 30;

    *packet = NULL;

    if( msg_type == MSGTYPE_REPLY ) {
        ret_code = http_MakeMessage(
            &buf, 1, 1,
#ifdef LIBUPNP_ECOS           
            "R" "sdc" "D" "sc" "ssc" "S" /*James "Xc"*/ "ssc" "sscc",
#else
            "R" "sdc" "D" "sc" "ssc" "S" "Xc" "ssc" "sscc",
#endif
            HTTP_OK,
            "CACHE-CONTROL: max-age=", duration,
	    "EXT:",
            "LOCATION: ", location,
#ifdef LIBUPNP_ECOS            
            /* James X_USER_AGENT, */
#else
            X_USER_AGENT,
#endif            
            "ST: ", nt,
            "USN: ", usn);
        if( ret_code != 0 ) {
            return;
        }
    } else if( msg_type == MSGTYPE_ADVERTISEMENT ||
               msg_type == MSGTYPE_SHUTDOWN ) {
        if( msg_type == MSGTYPE_ADVERTISEMENT ) {
            nts = "ssdp:alive";
        } else                  // shutdown
        {
            nts = "ssdp:byebye";
        }

        // NOTE: The CACHE-CONTROL and LOCATION headers are not present in
        //  a shutdown msg, but are present here for MS WinMe interop.
        
        ret_code = http_MakeMessage(
            &buf, 1, 1,
#ifdef LIBUPNP_ECOS            
            "Q" "sssdc" "sdc" "ssc" "ssc" "ssc" "S" /*James "Xc"*/ "sscc",
#else
            "Q" "sssdc" "sdc" "ssc" "ssc" "ssc" "S" "Xc" "sscc",
#endif            
            HTTPMETHOD_NOTIFY, "*", (size_t)1,
            "HOST: ", SSDP_IP, ":", SSDP_PORT,
            "CACHE-CONTROL: max-age=", duration,
            "LOCATION: ", location,
            "NT: ", nt,
            "NTS: ", nts,
#ifdef LIBUPNP_ECOS             
           /* James X_USER_AGENT, */
#else
            X_USER_AGENT,            
#endif           
            "USN: ", usn );
        if( ret_code != 0 ) {
            return;
        }

    } else {
        assert( 0 );            // unknown msg
    }

    buf_len = buf.length;
    *packet = membuffer_detach( &buf); // return msg

    if (mBuf) {
        membuffer_init( mBuf );
        membuffer_attach ( mBuf, *packet, buf_len);
    } else {
        membuffer_destroy( &buf );
    }

    return;
}

/************************************************************************
* Function : DeviceAdvertisement
*
* Parameters:
*	IN char * DevType : type of the device
*	IN int RootDev: flag to indicate if the device is root device
*	IN char * nt : ssdp type
*	IN char * usn : unique service name
*	IN char * location :Location URL.
*	IN int  duration :Service duration in sec.
*
* Description:
*	This function creates the device advertisement request based on 
*	the input parameter, and send it to the multicast channel.
*
* Returns: int
*	UPNP_E_SUCCESS if successful else appropriate error
***************************************************************************/
int
DeviceAdvertisement( IN char *DevType,
                     int RootDev,
                     char *Udn,
                     IN char *Location,
                     IN int Duration )
{
    struct sockaddr_in DestAddr;

    //char Mil_Nt[LINE_SIZE]
    char Mil_Usn[LINE_SIZE];
    char *msgs[3];
    int ret_code;
    membuffer mBuf[3];

    UpnpPrintf( UPNP_INFO, SSDP, __FILE__, __LINE__,
        "In function SendDeviceAdvertisemenrt\n" );

    DestAddr.sin_family = AF_INET;
    DestAddr.sin_addr.s_addr = inet_addr( SSDP_IP );
    DestAddr.sin_port = htons( SSDP_PORT );

    msgs[0] = NULL;
    msgs[1] = NULL;
    msgs[2] = NULL;

    //If deviceis a root device , here we need to 
    //send 3 advertisement or reply
    if( RootDev ) {
        sprintf( Mil_Usn, "%s::upnp:rootdevice", Udn );
        CreateServicePacket( MSGTYPE_ADVERTISEMENT, "upnp:rootdevice",
                             Mil_Usn, Location, Duration, &msgs[0], &mBuf[0] );

    }
    // both root and sub-devices need to send these two messages
    //

    CreateServicePacket( MSGTYPE_ADVERTISEMENT, Udn, Udn,
                         Location, Duration, &msgs[1], &mBuf[1] );

    sprintf( Mil_Usn, "%s::%s", Udn, DevType );
    CreateServicePacket( MSGTYPE_ADVERTISEMENT, DevType, Mil_Usn,
                         Location, Duration, &msgs[2], &mBuf[2] );

//    printf("msgs[0] = %p, msgs[1] = %p, msgs[2] = %p\n",
//            msgs[0], msgs[1], msgs[2]);
    // check error
    if( ( RootDev && msgs[0] == NULL ) ||
        msgs[1] == NULL || msgs[2] == NULL ) {
        upnp_callout_free( msgs[0] );
        upnp_callout_free( msgs[1] );
        upnp_callout_free( msgs[2] );
        printf("UPNP_E_OUTOF_MEMORY");
        return UPNP_E_OUTOF_MEMORY;
    }
    // send packets
    if( RootDev ) {
        // send 3 msg types
        ret_code = NewRequestHandler( &DestAddr, 3, &msgs[0], &mBuf[0] );
    } else                      // sub-device
    {
        // send 2 msg types
        ret_code = NewRequestHandler( &DestAddr, 2, &msgs[1], &mBuf[1]);
    }

    // free msgs
    upnp_callout_free( msgs[0] );
    upnp_callout_free( msgs[1] );
    upnp_callout_free( msgs[2] );

    return ret_code;
}

/************************************************************************
* Function : SendReply
*
* Parameters:
*	IN struct sockaddr_in * DestAddr:destination IP address.
*	IN char *DevType: Device type
*	IN int RootDev: 1 means root device 0 means embedded device.
*	IN char * Udn: Device UDN
*	IN char * Location: Location of Device description document.
*	IN int  Duration :Life time of this device.
*	IN int ByType:
*
* Description:
*	This function creates the reply packet based on the input parameter, 
*	and send it to the client addesss given in its input parameter DestAddr.
*
* Returns: int
*	UPNP_E_SUCCESS if successful else appropriate error
***************************************************************************/
int
SendReply( IN struct sockaddr_in *DestAddr,
           IN char *DevType,
           IN int RootDev,
           IN char *Udn,
           IN char *Location,
           IN int Duration,
           IN int ByType )
{
    int ret_code;
    char *msgs[2];
    int num_msgs;
    char Mil_Usn[LINE_SIZE];
    int i;
    membuffer mBuf[2];

    msgs[0] = NULL;
    msgs[1] = NULL;

    if( RootDev ) {
        // one msg for root device
        num_msgs = 1;

        sprintf( Mil_Usn, "%s::upnp:rootdevice", Udn );
        CreateServicePacket( MSGTYPE_REPLY, "upnp:rootdevice",
                             Mil_Usn, Location, Duration, &msgs[0], &mBuf[0] );
    } else {
        // two msgs for embedded devices
        num_msgs = 1;

        //NK: FIX for extra response when someone searches by udn
        if( !ByType ) {
            CreateServicePacket( MSGTYPE_REPLY, Udn, Udn, Location,
                                 Duration, &msgs[0], &mBuf[0] );
        } else {
            sprintf( Mil_Usn, "%s::%s", Udn, DevType );
            CreateServicePacket( MSGTYPE_REPLY, DevType, Mil_Usn,
                                 Location, Duration, &msgs[0], &mBuf[0] );
        }
    }

    // check error
    for( i = 0; i < num_msgs; i++ ) {
        if( msgs[i] == NULL ) {
            upnp_callout_free( msgs[0] );
            return UPNP_E_OUTOF_MEMORY;
        }
    }

    // send msgs
    ret_code = NewRequestHandler( DestAddr, num_msgs, msgs, mBuf );
    for( i = 0; i < num_msgs; i++ ) {
        if( msgs[i] != NULL )
            upnp_callout_free( msgs[i] );
    }

    return ret_code;
}

/************************************************************************
* Function : DeviceReply
*
* Parameters:
*	IN struct sockaddr_in * DestAddr:destination IP address.
*	IN char *DevType: Device type
*	IN int RootDev: 1 means root device 0 means embedded device.
*	IN char * Udn: Device UDN
*	IN char * Location: Location of Device description document.
*	IN int  Duration :Life time of this device.
* Description:
*	This function creates the reply packet based on the input parameter, 
*	and send it to the client address given in its input parameter DestAddr.
*
* Returns: int
*	UPNP_E_SUCCESS if successful else appropriate error
***************************************************************************/
int
DeviceReply( IN struct sockaddr_in *DestAddr,
             IN char *DevType,
             IN int RootDev,
             IN char *Udn,
             IN char *Location,
             IN int Duration )
{
    char *szReq[3],
      Mil_Nt[LINE_SIZE],
      Mil_Usn[LINE_SIZE];
    int RetVal;
    membuffer mBuf[3];

    szReq[0] = NULL;
    szReq[1] = NULL;
    szReq[2] = NULL;

    // create 2 or 3 msgs

    if( RootDev ) {
        // 3 replies for root device
        strcpy( Mil_Nt, "upnp:rootdevice" );
        sprintf( Mil_Usn, "%s::upnp:rootdevice", Udn );
        CreateServicePacket( MSGTYPE_REPLY, Mil_Nt, Mil_Usn,
                             Location, Duration, &szReq[0], &mBuf[0] );
    }

    sprintf( Mil_Nt, "%s", Udn );
    sprintf( Mil_Usn, "%s", Udn );
    CreateServicePacket( MSGTYPE_REPLY, Mil_Nt, Mil_Usn,
                         Location, Duration, &szReq[1], &mBuf[1] );

    sprintf( Mil_Nt, "%s", DevType );
    sprintf( Mil_Usn, "%s::%s", Udn, DevType );
    CreateServicePacket( MSGTYPE_REPLY, Mil_Nt, Mil_Usn,
                         Location, Duration, &szReq[2], &mBuf[2] );

    // check error

    if( ( RootDev && szReq[0] == NULL ) ||
        szReq[1] == NULL || szReq[2] == NULL ) {
        upnp_callout_free( szReq[0] );
        upnp_callout_free( szReq[1] );
        upnp_callout_free( szReq[2] );
        return UPNP_E_OUTOF_MEMORY;
    }
    // send replies
    if( RootDev ) {
        RetVal = NewRequestHandler( DestAddr, 3, szReq, mBuf );
    } else {
        RetVal = NewRequestHandler( DestAddr, 2, &szReq[1], &mBuf[1] );
    }

    // free
    upnp_callout_free( szReq[0] );
    upnp_callout_free( szReq[1] );
    upnp_callout_free( szReq[2] );

    return RetVal;
}

/************************************************************************
* Function : ServiceAdvertisement
*
* Parameters:
*	IN char * Udn: Device UDN
*	IN char *ServType: Service Type.
*	IN char * Location: Location of Device description document.
*	IN int  Duration :Life time of this device.
* Description:
*	This function creates the advertisement packet based
*	on the input parameter, and send it to the multicast channel.
*
* Returns: int
*	UPNP_E_SUCCESS if successful else appropriate error
***************************************************************************/
int
ServiceAdvertisement( IN char *Udn,
                      IN char *ServType,
                      IN char *Location,
                      IN int Duration )
{
    char Mil_Usn[LINE_SIZE];
    char *szReq[1];
    struct sockaddr_in DestAddr;
    int RetVal;
    membuffer mBuf[1];

    DestAddr.sin_family = AF_INET;
    DestAddr.sin_addr.s_addr = inet_addr( SSDP_IP );
    DestAddr.sin_port = htons( SSDP_PORT );

    sprintf( Mil_Usn, "%s::%s", Udn, ServType );

    //CreateServiceRequestPacket(1,szReq[0],Mil_Nt,Mil_Usn,
    //Server,Location,Duration);
    CreateServicePacket( MSGTYPE_ADVERTISEMENT, ServType, Mil_Usn,
                         Location, Duration, &szReq[0], &mBuf[0] );
    if( szReq[0] == NULL ) {
        return UPNP_E_OUTOF_MEMORY;
    }

    RetVal = NewRequestHandler( &DestAddr, 1, szReq, mBuf );

    upnp_callout_free( szReq[0] );
    return RetVal;
}

/************************************************************************
* Function : ServiceReply
*
* Parameters:
*	IN struct sockaddr_in *DestAddr:
*	IN char * Udn: Device UDN
*	IN char *ServType: Service Type.
*	IN char * Location: Location of Device description document.
*	IN int  Duration :Life time of this device.
* Description:
*	This function creates the advertisement packet based 
*	on the input parameter, and send it to the multicast channel.
*
* Returns: int
*	UPNP_E_SUCCESS if successful else appropriate error
***************************************************************************/
int
ServiceReply( IN struct sockaddr_in *DestAddr,
              IN char *ServType,
              IN char *Udn,
              IN char *Location,
              IN int Duration )
{
    char Mil_Usn[LINE_SIZE];
    char *szReq[1];
    int RetVal;
    membuffer mBuf[1];

    szReq[0] = NULL;

    sprintf( Mil_Usn, "%s::%s", Udn, ServType );

    CreateServicePacket( MSGTYPE_REPLY, ServType, Mil_Usn,
                         Location, Duration, &szReq[0], &mBuf[0] );
    if( szReq[0] == NULL ) {
        return UPNP_E_OUTOF_MEMORY;
    }

    RetVal = NewRequestHandler( DestAddr, 1, szReq, mBuf );

    upnp_callout_free( szReq[0] );
    return RetVal;
}

/************************************************************************
* Function : ServiceShutdown
*
* Parameters:
*	IN char * Udn: Device UDN
*	IN char *ServType: Service Type.
*	IN char * Location: Location of Device description document.
*	IN int  Duration :Service duration in sec.
* Description:
*	This function creates a HTTP service shutdown request packet 
*	and sent it to the multicast channel through RequestHandler.
*
* Returns: int
*	UPNP_E_SUCCESS if successful else appropriate error
***************************************************************************/
int
ServiceShutdown( IN char *Udn,
                 IN char *ServType,
                 IN char *Location,
                 IN int Duration )
{
    char Mil_Usn[LINE_SIZE];
    char *szReq[1];
    struct sockaddr_in DestAddr;
    int RetVal;
    membuffer mBuf[1];

    DestAddr.sin_family = AF_INET;
    DestAddr.sin_addr.s_addr = inet_addr( SSDP_IP );
    DestAddr.sin_port = htons( SSDP_PORT );

    //sprintf(Mil_Nt,"%s",ServType);
    sprintf( Mil_Usn, "%s::%s", Udn, ServType );
    //CreateServiceRequestPacket(0,szReq[0],Mil_Nt,Mil_Usn,
    //Server,Location,Duration);
    CreateServicePacket( MSGTYPE_SHUTDOWN, ServType, Mil_Usn,
                         Location, Duration, &szReq[0], &mBuf[0] );
    if( szReq[0] == NULL ) {
        return UPNP_E_OUTOF_MEMORY;
    }
    RetVal = NewRequestHandler( &DestAddr, 1, szReq, mBuf );

    upnp_callout_free( szReq[0] );
    return RetVal;
}

/************************************************************************
* Function : DeviceShutdown
*
* Parameters:
*	IN char *DevType: Device Type.
*	IN int RootDev:1 means root device.
*	IN char * Udn: Device UDN
*	IN char * Location: Location URL
*	IN int  Duration :Device duration in sec.
*
* Description:
*	This function creates a HTTP device shutdown request packet 
*	and sent it to the multicast channel through RequestHandler.
*
* Returns: int
*	UPNP_E_SUCCESS if successful else appropriate error
***************************************************************************/
int
DeviceShutdown( IN char *DevType,
                IN int RootDev,
                IN char *Udn,
                IN char *_Server,
                IN char *Location,
                IN int Duration )
{
    struct sockaddr_in DestAddr;
    char *msgs[3];
    char Mil_Usn[LINE_SIZE];
    int ret_code;
    membuffer mBuf[3];

    msgs[0] = NULL;
    msgs[1] = NULL;
    msgs[2] = NULL;

    DestAddr.sin_family = AF_INET;
    DestAddr.sin_addr.s_addr = inet_addr( SSDP_IP );
    DestAddr.sin_port = htons( SSDP_PORT );

    // root device has one extra msg
    if( RootDev ) {
        sprintf( Mil_Usn, "%s::upnp:rootdevice", Udn );
        CreateServicePacket( MSGTYPE_SHUTDOWN, "upnp:rootdevice",
                             Mil_Usn, Location, Duration, &msgs[0], &mBuf[0] );
    }

    UpnpPrintf( UPNP_INFO, SSDP, __FILE__, __LINE__,
        "In function DeviceShutdown\n" );
    // both root and sub-devices need to send these two messages
    CreateServicePacket( MSGTYPE_SHUTDOWN, Udn, Udn,
                         Location, Duration, &msgs[1], &mBuf[1] );

    sprintf( Mil_Usn, "%s::%s", Udn, DevType );
    CreateServicePacket( MSGTYPE_SHUTDOWN, DevType, Mil_Usn,
                         Location, Duration, &msgs[2], &mBuf[2] );

    // check error
    if( ( RootDev && msgs[0] == NULL ) ||
        msgs[1] == NULL || msgs[2] == NULL ) {
        upnp_callout_free( msgs[0] );
        upnp_callout_free( msgs[1] );
        upnp_callout_free( msgs[2] );
        return UPNP_E_OUTOF_MEMORY;
    }
    // send packets
    if( RootDev ) {
        // send 3 msg types
        ret_code = NewRequestHandler( &DestAddr, 3, &msgs[0], &mBuf[0] );
    } else                      // sub-device
    {
        // send 2 msg types
        ret_code = NewRequestHandler( &DestAddr, 2, &msgs[1], &mBuf[1] );
    }

    // free msgs
    upnp_callout_free( msgs[0] );
    upnp_callout_free( msgs[1] );
    upnp_callout_free( msgs[2] );

    return ret_code;
}

#endif // EXCLUDE_SSDP
#endif // INCLUDE_DEVICE_APIS

