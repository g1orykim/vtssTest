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
#include "util.h"
#ifdef INCLUDE_CLIENT_APIS
#if EXCLUDE_SSDP == 0

#ifdef VTSS_SW_OPTION_IP2
#include "ip2_api.h"
#endif /* VTSS_SW_OPTION_IP2 */
#ifdef LIBUPNP_ECOS
#include "vtss_upnp.h"
#endif /* LIBUPNP_ECOS */

#include "ssdplib.h"
#include "upnpapi.h"
#include <stdio.h>
#include "ThreadPool.h"

#include "httpparser.h"
#include "httpreadwrite.h"
#include "statcodes.h"

#include "unixutil.h"

#ifdef WIN32
	#include <ws2tcpip.h>
	#include <winsock2.h>
	#include <string.h>
#endif /* WIN32 */


/************************************************************************
* Function : send_search_result
*
* Parameters:
*	IN void *data: Search reply from the device
*
* Description:
*	This function sends a callback to the control point application with 
*	a SEARCH result
*
* Returns: void
*
***************************************************************************/
void
send_search_result( IN void *data )
{
    ResultData *temp = ( ResultData * ) data;

    temp->ctrlpt_callback( UPNP_DISCOVERY_SEARCH_RESULT,
                           &temp->param, temp->cookie );
    upnp_callout_free( temp );
}

/************************************************************************
* Function : ssdp_handle_ctrlpt_msg
*
* Parameters:
*	IN http_message_t* hmsg: SSDP message from the device
*	IN struct sockaddr_in* dest_addr: Address of the device
*	IN xboolean timeout: timeout kept by the control point while
*		sending search message
*	IN void* cookie: Cookie stored by the control point application. 
*		This cookie will be returned to the control point
*		in the callback 
*
* Description:
*	This function handles the ssdp messages from the devices. These 
*	messages includes the search replies, advertisement of device coming 
*	alive and bye byes.
*
* Returns: void
*
***************************************************************************/
void
ssdp_handle_ctrlpt_msg( IN http_message_t * hmsg,
                        IN struct sockaddr_in *dest_addr,
                        IN xboolean timeout,    // only in search reply

                        IN void *cookie )   // only in search reply
{
    int handle;
    struct Handle_Info *ctrlpt_info = NULL;
    memptr hdr_value;
    xboolean is_byebye;         // byebye or alive
    struct Upnp_Discovery param;
    SsdpEvent event;
    xboolean nt_found,
      usn_found,
      st_found;
    char save_char;
    Upnp_EventType event_type;
    Upnp_FunPtr ctrlpt_callback;
    void *ctrlpt_cookie;
    ListNode *node = NULL;
    SsdpSearchArg *searchArg = NULL;
    int matched = 0;
    ResultData *threadData;
    ThreadPoolJob job;

    // we are assuming that there can be only one client supported at a time

    HandleReadLock();

    if( GetClientHandleInfo( &handle, &ctrlpt_info ) != HND_CLIENT ) {
        HandleUnlock();
        return;
    }
    // copy
    ctrlpt_callback = ctrlpt_info->Callback;
    ctrlpt_cookie = ctrlpt_info->Cookie;
    HandleUnlock();

    // search timeout
    if( timeout ) {
        ctrlpt_callback( UPNP_DISCOVERY_SEARCH_TIMEOUT, NULL, cookie );
        return;
    }

    param.ErrCode = UPNP_E_SUCCESS;

    // MAX-AGE
    param.Expires = -1;         // assume error
    if( httpmsg_find_hdr( hmsg, HDR_CACHE_CONTROL, &hdr_value ) != NULL ) {
        if( matchstr( hdr_value.buf, hdr_value.length,
                      "%imax-age = %d%0", &param.Expires ) != PARSE_OK )
            return;
    }

    // DATE
    param.Date[0] = '\0';
    if( httpmsg_find_hdr( hmsg, HDR_DATE, &hdr_value ) != NULL ) {
        linecopylen( param.Date, hdr_value.buf, hdr_value.length );
    }

    // dest addr
    memcpy(&param.DestAddr, dest_addr, sizeof(struct sockaddr_in) );

    // EXT
    param.Ext[0] = '\0';
    if( httpmsg_find_hdr( hmsg, HDR_EXT, &hdr_value ) != NULL ) {
        linecopylen( param.Ext, hdr_value.buf, hdr_value.length );
    }
    // LOCATION
    param.Location[0] = '\0';
    if( httpmsg_find_hdr( hmsg, HDR_LOCATION, &hdr_value ) != NULL ) {
        linecopylen( param.Location, hdr_value.buf, hdr_value.length );
    }
    // SERVER / USER-AGENT
    param.Os[0] = '\0';
    if( httpmsg_find_hdr( hmsg, HDR_SERVER, &hdr_value ) != NULL ||
        httpmsg_find_hdr( hmsg, HDR_USER_AGENT, &hdr_value ) != NULL ) {
        linecopylen( param.Os, hdr_value.buf, hdr_value.length );
    }
    // clear everything
    param.DeviceId[0] = '\0';
    param.DeviceType[0] = '\0';
    param.ServiceType[0] = '\0';

    param.ServiceVer[0] = '\0'; // not used; version is in ServiceType

    event.UDN[0] = '\0';
    event.DeviceType[0] = '\0';
    event.ServiceType[0] = '\0';

    nt_found = FALSE;

    if( httpmsg_find_hdr( hmsg, HDR_NT, &hdr_value ) != NULL ) {
        save_char = hdr_value.buf[hdr_value.length];
        hdr_value.buf[hdr_value.length] = '\0';

        nt_found = ( ssdp_request_type( hdr_value.buf, &event ) == 0 );

        hdr_value.buf[hdr_value.length] = save_char;
    }

    usn_found = FALSE;
    if( httpmsg_find_hdr( hmsg, HDR_USN, &hdr_value ) != NULL ) {
        save_char = hdr_value.buf[hdr_value.length];
        hdr_value.buf[hdr_value.length] = '\0';

        usn_found = ( unique_service_name( hdr_value.buf, &event ) == 0 );

        hdr_value.buf[hdr_value.length] = save_char;
    }

    if( nt_found || usn_found ) {
        strcpy( param.DeviceId, event.UDN );
        strcpy( param.DeviceType, event.DeviceType );
        strcpy( param.ServiceType, event.ServiceType );
    }

    // ADVERT. OR BYEBYE
    if( hmsg->is_request ) {
        // use NTS hdr to determine advert., or byebye
        //
        if( httpmsg_find_hdr( hmsg, HDR_NTS, &hdr_value ) == NULL ) {
            return;             // error; NTS header not found
        }
        if( memptr_cmp( &hdr_value, "ssdp:alive" ) == 0 ) {
            is_byebye = FALSE;
        } else if( memptr_cmp( &hdr_value, "ssdp:byebye" ) == 0 ) {
            is_byebye = TRUE;
        } else {
            return;             // bad value
        }

        if( is_byebye ) {
            // check device byebye
            if( !nt_found || !usn_found ) {
                return;         // bad byebye
            }
            event_type = UPNP_DISCOVERY_ADVERTISEMENT_BYEBYE;
        } else {
            // check advertisement      
            // .Expires is valid if positive. This is for testing
            //  only. Expires should be greater than 1800 (30 mins)
            if( !nt_found ||
                !usn_found ||
                strlen( param.Location ) == 0 || param.Expires <= 0 ) {
                return;         // bad advertisement
            }

            event_type = UPNP_DISCOVERY_ADVERTISEMENT_ALIVE;
        }

        // call callback
        ctrlpt_callback( event_type, &param, ctrlpt_cookie );

    } else                      // reply (to a SEARCH)
    {
        // only checking to see if there is a valid ST header
        st_found = FALSE;
        if( httpmsg_find_hdr( hmsg, HDR_ST, &hdr_value ) != NULL ) {
            save_char = hdr_value.buf[hdr_value.length];
            hdr_value.buf[hdr_value.length] = '\0';
            st_found = ssdp_request_type( hdr_value.buf, &event ) == 0;
            hdr_value.buf[hdr_value.length] = save_char;
        }
        if( hmsg->status_code != HTTP_OK ||
            param.Expires <= 0 ||
            strlen( param.Location ) == 0 || !usn_found || !st_found ) {
            return;             // bad reply
        }
        // check each current search
        HandleLock();
        if( GetClientHandleInfo( &handle, &ctrlpt_info ) != HND_CLIENT ) {
            HandleUnlock();
            return;
        }
        node = ListHead( &ctrlpt_info->SsdpSearchList );

        // temporary add null termination
        //save_char = hdr_value.buf[ hdr_value.length ];
        //hdr_value.buf[ hdr_value.length ] = '\0';

        while( node != NULL ) {
            searchArg = node->item;
            matched = 0;
            // check for match of ST header and search target
            switch ( searchArg->requestType ) {
                case SSDP_ALL:
                    {
                        matched = 1;
                        break;
                    }
                case SSDP_ROOTDEVICE:
                    {
                        matched = ( event.RequestType == SSDP_ROOTDEVICE );
                        break;
                    }
                case SSDP_DEVICEUDN:
                    {
                        matched = !( strncmp( searchArg->searchTarget,
                                              hdr_value.buf,
                                              hdr_value.length ) );
                        break;
                    }
                case SSDP_DEVICETYPE:
                    {
                        int m = min( hdr_value.length,
                                     strlen( searchArg->searchTarget ) );

                        matched = !( strncmp( searchArg->searchTarget,
                                              hdr_value.buf, m ) );
                        break;
                    }
                case SSDP_SERVICE:
                    {
                        int m = min( hdr_value.length,
                                     strlen( searchArg->searchTarget ) );

                        matched = !( strncmp( searchArg->searchTarget,
                                              hdr_value.buf, m ) );
                        break;
                    }
                default:
                    {
                        matched = 0;
                        break;
                    }
            }

            if( matched ) {
                // schedule call back
                threadData =
                    ( ResultData * )upnp_callout_malloc( sizeof( ResultData ) );
                if( threadData != NULL ) {
                    threadData->param = param;
                    threadData->cookie = searchArg->cookie;
                    threadData->ctrlpt_callback = ctrlpt_callback;
                    TPJobInit( &job, ( start_routine ) send_search_result,
                               threadData );
                    TPJobSetPriority( &job, MED_PRIORITY );
                    TPJobSetFreeFunction( &job, ( free_routine ) upnp_callout_free );
                    ThreadPoolAdd( &gRecvThreadPool, &job, NULL );
                }
            }
            node = ListNext( &ctrlpt_info->SsdpSearchList, node );
        }

        HandleUnlock();
        //ctrlpt_callback( UPNP_DISCOVERY_SEARCH_RESULT, &param, cookie );
    }
}

/************************************************************************
* Function : process_reply
*
* Parameters:
*	IN char* request_buf: the response came from the device
*	IN int buf_len: The length of the response buffer
*	IN struct sockaddr_in* dest_addr: The address of the device
*	IN void *cookie : cookie passed by the control point application
*		at the time of sending search message
*
* Description:
*	This function processes reply recevied from a search
*
* Returns: void
*
***************************************************************************/
#ifndef WIN32
#warning There are currently no uses of the function 'process_reply()' in the code.
#warning 'process_reply()' is a candidate for removal.
#else
#pragma message("There are currently no uses of the function 'process_reply()' in the code.")
#pragma message("'process_reply()' is a candidate for removal.")
#endif
static UPNP_INLINE void
process_reply( IN char *request_buf,
               IN int buf_len,
               IN struct sockaddr_in *dest_addr,
               IN void *cookie )
{
    http_parser_t parser;

    parser_response_init( &parser, HTTPMETHOD_MSEARCH );

    // parse
    if( parser_append( &parser, request_buf, buf_len ) != PARSE_SUCCESS ) {
        httpmsg_destroy( &parser.msg );
        return;
    }
    // handle reply
    ssdp_handle_ctrlpt_msg( &parser.msg, dest_addr, FALSE, cookie );

    // done
    httpmsg_destroy( &parser.msg );
}

/************************************************************************
* Function : CreateClientRequestPacket
*
* Parameters:
*	IN char * RqstBuf:Output string in HTTP format.
*	IN char *SearchTarget:Search Target
*	IN int Mx dest_addr: Number of seconds to wait to 
*		collect all the responses
*
* Description:
*	This function creates a HTTP search request packet 
* 	depending on the input parameter.
*
* Returns: void
*
***************************************************************************/
static void
CreateClientRequestPacket( IN char *RqstBuf,
                           IN int Mx,
                           IN char *SearchTarget )
{
    char TempBuf[COMMAND_LEN];

    strcpy( RqstBuf, "M-SEARCH * HTTP/1.1\r\n" );

    sprintf( TempBuf, "HOST: %s:%d\r\n", SSDP_IP, SSDP_PORT );
    strcat( RqstBuf, TempBuf );
    strcat( RqstBuf, "MAN: \"ssdp:discover\"\r\n" );

    if( Mx > 0 ) {
        sprintf( TempBuf, "MX: %d\r\n", Mx );
        strcat( RqstBuf, TempBuf );
    }

    if( SearchTarget != NULL ) {
        sprintf( TempBuf, "ST: %s\r\n", SearchTarget );
        strcat( RqstBuf, TempBuf );
    }
    strcat( RqstBuf, "\r\n" );

}

/************************************************************************
* Function : searchExpired
*
* Parameters:
*		IN void * arg:
*
* Description:
*	This function 
*
* Returns: void
*
***************************************************************************/
void
searchExpired( void *arg )
{

    int *id = ( int * )arg;
    int handle = -1;
    struct Handle_Info *ctrlpt_info = NULL;

    //remove search Target from list and call client back
    ListNode *node = NULL;
    SsdpSearchArg *item;
    Upnp_FunPtr ctrlpt_callback;
    void *cookie = NULL;
    int found = 0;

    HandleLock();

    //remove search target from search list

    if( GetClientHandleInfo( &handle, &ctrlpt_info ) != HND_CLIENT ) {
        upnp_callout_free( id );
        HandleUnlock();
        return;
    }

    ctrlpt_callback = ctrlpt_info->Callback;

    node = ListHead( &ctrlpt_info->SsdpSearchList );

    while( node != NULL ) {
        item = ( SsdpSearchArg * ) node->item;
        if( item->timeoutEventId == ( *id ) ) {
            upnp_callout_free( item->searchTarget );
            cookie = item->cookie;
            found = 1;
            item->searchTarget = NULL;
            upnp_callout_free( item );
            ListDelNode( &ctrlpt_info->SsdpSearchList, node, 0 );
            break;
        }
        node = ListNext( &ctrlpt_info->SsdpSearchList, node );
    }
    HandleUnlock();

    if( found ) {
        ctrlpt_callback( UPNP_DISCOVERY_SEARCH_TIMEOUT, NULL, cookie );
    }

    upnp_callout_free( id );
}

#ifdef VTSS_SW_OPTION_IP2
static BOOL _SearchByTarget(i8 *ReqBuf, fd_set *wrSet, vtss_ipv4_t intf_adr, void *destAddr)
{
    i32             ret;
    i8              errorBuffer[ERROR_BUFFER_LEN];
    struct in_addr  addr;

    if (!ReqBuf || !wrSet || !destAddr) {
        return FALSE;
    }

    memset((void *)&addr, 0x0, sizeof(struct in_addr));
    addr.s_addr = htonl(intf_adr);

    FD_ZERO(wrSet);
    FD_SET(gSsdpReqSocket, wrSet);
    ret = setsockopt(gSsdpReqSocket, IPPROTO_IP, IP_MULTICAST_IF,
                     (char *)&addr, sizeof(struct in_addr));

    ret = select(gSsdpReqSocket + 1, NULL, wrSet, NULL, NULL);
    if (ret == -1) {
        strerror_r(errno, errorBuffer, ERROR_BUFFER_LEN);
        UpnpPrintf(UPNP_INFO, SSDP, __FILE__, __LINE__,
                   "SSDP_LIB: Error in select(): %s\n",
                   errorBuffer);

        return FALSE;
    } else if (FD_ISSET(gSsdpReqSocket, wrSet)) {
        sendto(gSsdpReqSocket, ReqBuf, strlen(ReqBuf), 0,
               (struct sockaddr *)destAddr, sizeof(struct sockaddr_in));
    }

    return TRUE;
}
#endif /* VTSS_SW_OPTION_IP2 */

/************************************************************************
* Function : SearchByTarget
*
* Parameters:
*	IN int Mx:Number of seconds to wait, to collect all the	responses.
*	IN char *St: Search target.
*	IN void *Cookie: cookie provided by control point application.
*		This cokie will be returned to application in the callback.
*
* Description:
*	This function creates and send the search request for a specific URL.
*
* Returns: int
*	1 if successful else appropriate error
***************************************************************************/
int
SearchByTarget( IN int Mx,
                IN char *St,
                IN void *Cookie )
{
    int *id = NULL;
    char *ReqBuf;
    struct sockaddr_in destAddr;
    fd_set wrSet;
    SsdpSearchArg *newArg = NULL;
    int timeTillRead = 0;
    int handle;
    struct Handle_Info *ctrlpt_info = NULL;
    enum SsdpSearchType requestType;
#ifdef VTSS_SW_OPTION_IP2
    vtss_vid_t          intf_ifid;  /* With IP2, VID is used as the IFID */
    BOOL                intf_up, has_err;
    vtss_ipv4_t         intf_adr;
    vtss_if_status_t    *ops, ipst[UPNP_IP_INTF_MAX_OPST];
    u32                 ops_idx;
    u32                 ops_cnt;
    i32                 NumCopy;
#else
    char errorBuffer[ERROR_BUFFER_LEN];
    int ret = 0;
    unsigned long addr = inet_addr( LOCAL_HOST );
#endif /* VTSS_SW_OPTION_IP2 */

    //ThreadData *ThData;
    ThreadPoolJob job;

    requestType = ssdp_request_type1( St );
    if( requestType == SSDP_SERROR ) {
        return UPNP_E_INVALID_PARAM;
    }

    ReqBuf = (char *)upnp_callout_malloc( BUFSIZE );
    if( ReqBuf == NULL ) {
        return UPNP_E_OUTOF_MEMORY;
    }

    UpnpPrintf(UPNP_INFO, SSDP, __FILE__, __LINE__, ">>> SSDP SEND >>>\n");

    timeTillRead = Mx;

    if( timeTillRead < MIN_SEARCH_TIME ) {
        timeTillRead = MIN_SEARCH_TIME;
    } else if( timeTillRead > MAX_SEARCH_TIME ) {
        timeTillRead = MAX_SEARCH_TIME;
    }

    CreateClientRequestPacket( ReqBuf, timeTillRead, St );

    // add search criteria to list
    HandleLock();
    if( GetClientHandleInfo( &handle, &ctrlpt_info ) != HND_CLIENT ) {
        HandleUnlock();
        upnp_callout_free( ReqBuf );
        return UPNP_E_INTERNAL_ERROR;
    }

    newArg = ( SsdpSearchArg * )upnp_callout_malloc( sizeof( SsdpSearchArg ) );

#if 1 /* CP: avoid use NULL */
    if( newArg == NULL ) {
        upnp_callout_free( ReqBuf );
        ReqBuf = NULL;
        return UPNP_E_OUTOF_MEMORY;
    }
#endif

    newArg->searchTarget = upnp_callout_strdup( St );
    newArg->cookie = Cookie;
    newArg->requestType = requestType;

    id = ( int * )upnp_callout_malloc( sizeof( int ) );

#if 1 /* CP: avoid use NULL */
    if( id == NULL ) {
        upnp_callout_free( ReqBuf );
        ReqBuf = NULL;
        upnp_callout_free( newArg );
        newArg = NULL;
        return UPNP_E_OUTOF_MEMORY;
    }
#endif

    TPJobInit( &job, ( start_routine ) searchExpired, id );
    TPJobSetPriority( &job, MED_PRIORITY );
    TPJobSetFreeFunction( &job, ( free_routine ) upnp_callout_free );

    // Schedule a timeout event to remove search Arg
    TimerThreadSchedule( &gTimerThread, timeTillRead,
                         REL_SEC, &job, SHORT_TERM, id );
    newArg->timeoutEventId = ( *id );

    ListAddTail( &ctrlpt_info->SsdpSearchList, newArg );
    HandleUnlock();

    memset( ( char * )&destAddr, 0, sizeof( struct sockaddr_in ) );
    destAddr.sin_family = AF_INET;
    destAddr.sin_addr.s_addr = inet_addr( SSDP_IP );
    destAddr.sin_port = htons( SSDP_PORT );

#ifdef VTSS_SW_OPTION_IP2
    for (NumCopy = 0; NumCopy < NUM_SSDP_COPY; NumCopy++) {
        /* FIXME!SGETZ: Should be based on per-interface-ifid */
        has_err = FALSE;
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

                    if (UPNP_IP_INTF_OPST_UP(ops)) {
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

            if (!_SearchByTarget(ReqBuf, &wrSet, intf_adr, &destAddr)) {
                has_err = TRUE;
                break;
            }
        }

        if (has_err) {
            shutdown(gSsdpReqSocket, SD_BOTH);
            UpnpCloseSocket(gSsdpReqSocket);
            upnp_callout_free(ReqBuf);

            return UPNP_E_INTERNAL_ERROR;
        }

        imillisleep( SSDP_PAUSE );
    }
#else
    FD_ZERO( &wrSet );
    FD_SET( gSsdpReqSocket, &wrSet );
    ret = setsockopt( gSsdpReqSocket, IPPROTO_IP, IP_MULTICAST_IF,
        (char *)&addr, sizeof (addr) );

    ret = select( gSsdpReqSocket + 1, NULL, &wrSet, NULL, NULL );
    if( ret == -1 ) {
        strerror_r(errno, errorBuffer, ERROR_BUFFER_LEN);
        UpnpPrintf( UPNP_INFO, SSDP, __FILE__, __LINE__,
            "SSDP_LIB: Error in select(): %s\n",
            errorBuffer );
	shutdown( gSsdpReqSocket, SD_BOTH );
        UpnpCloseSocket( gSsdpReqSocket );
        upnp_callout_free( ReqBuf );

        return UPNP_E_INTERNAL_ERROR;
    } else if( FD_ISSET( gSsdpReqSocket, &wrSet ) ) {
        int NumCopy = 0;
        while( NumCopy < NUM_SSDP_COPY ) {
            sendto( gSsdpReqSocket, ReqBuf, strlen( ReqBuf ), 0,
                (struct sockaddr *)&destAddr, sizeof(struct sockaddr_in));
            NumCopy++;
            imillisleep( SSDP_PAUSE );
        }
    }
#endif /* VTSS_SW_OPTION_IP2 */

    upnp_callout_free( ReqBuf );
    return 1;
}

#endif // EXCLUDE_SSDP
#endif // INCLUDE_CLIENT_APIS

