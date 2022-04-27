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

#include "vtss_upnp_api.h"
#include "msg_api.h"
#include "sample_util.h"
#include "upnp_device_main.h"
#include <pthread.h>
#include <stdio.h>

#define DESC_URL_SIZE 200
#define DEFAULT_WEB_DIR "./web"
#define VTSS_DEBUG  0

/*
   The amount of time (in seconds) before advertisements
   will expire
 */
int default_advr_expire = 100;

/*
   Device handle supplied by UPnP SDK
 */
UpnpDevice_Handle device_handle = -1;

/******************************************************************************
 * linux_print
 *
 * Description:
 *       Prints a string to standard out.
 *
 * Parameters:
 *    None
 *
 *****************************************************************************/
static void
linux_print( const char *string )
{
    printf( "%s", string );
}

/******************************************************************************
 * DeviceCallbackEventHandler
 *
 * Description:
 *       The callback handler registered with the SDK while registering
 *       root device.
 *****************************************************************************/
static void
DeviceCallbackEventHandler( Upnp_EventType EventType,
                            void *Event,
                            void *Cookie )
{
    if (EventType) {
        ;
    }
    if (Event) {
        ;
    }
    if (Cookie) {
        ;
    }
}
/******************************************************************************
 * upnp_device_start_in
 *
 * Description:
 *      Initializes the UPnP Sdk, registers the device, and sends out
 *      advertisements.
 *
 * Parameters:
 *
 *   ip_address - ip address to initialize the sdk (may be NULL)
 *                if null, then the first non null loopback address is used.
 *   port       - port number to initialize the sdk (may be 0)
 *                if zero, then a random number is used.
 *   desc_doc_name - name of description document.
 *                   may be NULL. Default is devicedesc.xml
 *   web_dir_path  - path of web directory.
 *                   may be NULL. Default is ./web (for Linux) or ../device/web
 *                   for windows.
 *   pfun          - print function to use.
 *
 *****************************************************************************/
int upnp_device_start_in( char *ip_address,
                          unsigned short port,
                          char *desc_doc_name,
                          char *web_dir_path,
                          print_string pfun )
{
    int ret = UPNP_E_SUCCESS;
    char desc_doc_url[DESC_URL_SIZE];
    upnp_conf_t conf;
    vtss_rc   rc;


    rc = upnp_mgmt_conf_get(&conf);
    if (rc != VTSS_OK) {
        return UPNP_E_INIT_FAILED;
    }
    default_advr_expire = conf.adv_interval;

#if (VTSS_DEBUG ==  1)
    SampleUtil_Initialize( pfun );
#endif

    SampleUtil_Print(
        "Initializing UPnP Sdk with\n"
        "\tipaddress = %s port = %u\n",
        ip_address, port );

    SampleUtil_Print( "UpnpInit -- %d\n", ret );
    if ( ( ret = UpnpInit( ip_address, port ) ) != UPNP_E_SUCCESS ) {
        SampleUtil_Print( "Error with UpnpInit -- %d\n", ret );
        UpnpFinish();
        return ret;
    }

    if ( ip_address == NULL ) {
        ip_address = UpnpGetServerIpAddress();
    }

    port = UpnpGetServerPort();

    SampleUtil_Print(
        "UPnP Initialized\n"
        "\tipaddress= %s port = %u\n",
        ip_address, port );

    if ( desc_doc_name == NULL ) {
        desc_doc_name = "devicedesc.xml";
    }

    if ( web_dir_path == NULL ) {
        web_dir_path = DEFAULT_WEB_DIR;
    }

    snprintf( desc_doc_url, DESC_URL_SIZE, "http://%s:%d/%s", ip_address,
              port, desc_doc_name );

    SampleUtil_Print( "Specifying the webserver root directory -- %s\n",
                      web_dir_path );
    SampleUtil_Print(
        "Registering the RootDevice\n"
        "\t with desc_doc_url: %s\n",
        desc_doc_url );
    if ( ( ret = UpnpRegisterRootDevice( desc_doc_url,
                                         (Upnp_FunPtr)DeviceCallbackEventHandler,
                                         &device_handle, &device_handle ) )
         != UPNP_E_SUCCESS ) {
        SampleUtil_Print( "Error registering the rootdevice : %d\n", ret );
        UpnpFinish();
        return ret;
    } else {
        SampleUtil_Print(
            "RootDevice Registered\n"
            "Initializing State Table\n");
        SampleUtil_Print("State Table Initialized\n");

        if ( ( ret =
                   UpnpSendAdvertisement( device_handle, default_advr_expire ) )
             != UPNP_E_SUCCESS ) {

            SampleUtil_Print( "Error sending advertisements : %d\n", ret );
            UpnpFinish();
            return ret;
        }

        SampleUtil_Print("Advertisements Sent\n");
    }
    return UPNP_E_SUCCESS;
}

#if 0
/******************************************************************************
 * main
 *
 * Description:
 *       Main entry point for device application.
 *       Initializes and registers with the sdk.
 *       Initializes the state stables of the service.
 *       Starts the command loop.
 *
 * Parameters:
 *    int argc  - count of arguments
 *    char ** argv -arguments. The application
 *                  accepts the following optional arguments:
 *
 *                  -ip ipaddress
 *                  -port port
 *          -desc desc_doc_name
 *              -webdir web_dir_path"
 *          -help
 *
 *
 *****************************************************************************/
void *upnp_device_main( void *args )
{

    char        *desc_doc_name = NULL,
                 *web_dir_path = NULL;
    char        ip_address[16];
    int         ret;
    upnp_conf_t conf;

    if (args) {
        ;
    }
    while (vtss_upnp_get_ip(ip_address) != 0) {
        /* waiting for IP address ready */
        isleep(1);
        if (upnp_mgmt_conf_get(&conf) != VTSS_OK) {
            return 0;
        }

        if (!msg_switch_is_master() || conf.mode == UPNP_MGMT_DISABLED) {
            /* no need to wait because it is no longer a master */
            return 0;
        }

    }

#if (VTSS_DEBUG ==  1)
    SampleUtil_Initialize( linux_print );
#endif
    ret = upnp_device_start(ip_address, UPNP_UDP_PORT, desc_doc_name, web_dir_path, linux_print);
    if (ret != UPNP_E_SUCCESS) {
        SampleUtil_Print("upnp_device_main: upnp_device_start fails");
    }

    if (upnp_mgmt_conf_get(&conf) != VTSS_OK) {
        SampleUtil_Print("upnp_mgmt_conf_get failed\n");
    }
    if (!msg_switch_is_master() || conf.mode == UPNP_MGMT_DISABLED) {
        /* This is to overcome the issue that the initial thread and this
         * thread run async. It is likely that the device is change to
         * SALVE but this thread does not learn it.
         */
        upnp_device_stop();
    }
    return 0;
}
#endif

void upnp_device_start(void)
{
    char        *desc_doc_name = NULL,
                 *web_dir_path = NULL;
    char        ip_address[16];
    int         ret;

    (void) vtss_upnp_get_ip(ip_address);
#if (VTSS_DEBUG ==  1)
    SampleUtil_Initialize( linux_print );
#endif
    ret = upnp_device_start_in(ip_address, UPNP_UDP_PORT, desc_doc_name, web_dir_path, linux_print);
    if (ret != UPNP_E_SUCCESS) {
        SampleUtil_Print("upnp_device_main: upnp_device_start_in fails");
    }
}


void upnp_device_stop(void)
{
    UpnpUnRegisterRootDevice( device_handle );
    UpnpFinish();
    SampleUtil_Finish();
}
