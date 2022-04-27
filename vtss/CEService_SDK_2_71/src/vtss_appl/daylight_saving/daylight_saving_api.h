/*

 Vitesse Switch API software.

 Copyright (c) 2002-2012 Vitesse Semiconductor Corporation "Vitesse". All
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

 $Id$
 $Revision$

*/

#ifndef _VTSS_DAYLIGHT_SAVING_API_H_
#define _VTSS_DAYLIGHT_SAVING_API_H_

/* time daylight saving error codes (vtss_rc) */
typedef enum {
    TIME_DST_ERROR_GEN = MODULE_ERROR_START(VTSS_MODULE_ID_DAYLIGHT_SAVING),    /* Generic error code */
    TIME_DST_ERROR_PARM,                                                        /* Illegal parameter */
    TIME_DST_ERROR_STACK_STATE,                                                 /* Illegal MASTER/SLAVE state */
} time_dst_error_t;

/* daylight saving time mode */
typedef enum {
    TIME_DST_DISABLED,
    TIME_DST_RECURRING,
    TIME_DST_NON_RECURRING
} time_dst_mode_t;

/* daylight saving time */
typedef struct {
    uchar   week;
    uchar   day;
    uchar   month;
    uchar   date;
    ushort  year;
    uchar   hour;
    uchar   minute;
} time_dst_cfg_t;

/* daylight saving time configuration */
typedef struct {
    time_dst_mode_t     dst_mode;
    time_dst_cfg_t      dst_start_time;
    time_dst_cfg_t      dst_end_time;
    u32                 dst_offset;             /* 1 - 1440 minutes */
    char                tz_acronym[20];
    int                 tz_offset;              /* +- 720 minutes   */
    int                 tz;
    /* system timezone: (total minutes)*10 + id
         0: "None",
     -7200: "(GMT-12:00) International Date Line West",
     -6600: "(GMT-11:00) Midway Island, Samoa",
     -6000: "(GMT-10:00) Hawaii",
     -5400: "(GMT-09:00) Alaska",
     -4800: "(GMT-08:00) Pacific Time (US and Canada)",
     -4801: "(GMT-08:00) Tijuana, Baja California",
     -4200: "(GMT-07:00) Arizona",
     -4201: "(GMT-07:00) Chihuahua, La Paz, Mazatlan - New",
     -4202: "(GMT-07:00) Chihuahua, La Paz, Mazatlan - Old",
     -4203: "(GMT-07:00) Mountain Time (US and Canada)",
     -3600: "(GMT-06:00) Central America",
     -3601: "(GMT-06:00) Central Time (US and Canada)",
     -3602: "(GMT-06:00) Guadalajara, Mexico City, Monterrey - New",
     -3603: "(GMT-06:00) Guadalajara, Mexico City, Monterrey - Old",
     -3604: "(GMT-06:00) Saskatchewan",
     -3000: "(GMT-05:00) Bogota, Lima, Quito, Rio Branco",
     -3001: "(GMT-05:00) Eastern Time (US and Canada)",
     -3002: "(GMT-05:00) Indiana (East)",
     -2700: "(GMT-04:30) Caracas",
     -2400: "(GMT-04:00) Atlantic Time (Canada)",
     -2401: "(GMT-04:00) La Paz","(GMT-04:00) Manaus",
     -2402: "(GMT-04:00) Santiago","(GMT-03:30) Newfoundland",
     -1800: "(GMT-03:00) Brasilia","(GMT-03:00) Buenos Aires",
     -1801: "(GMT-03:00) Georgetown","(GMT-03:00) Greenland",
     -1802: "(GMT-03:00) Montevideo","(GMT-02:00) Mid-Atlantic",
      -600: "(GMT-01:00) Azores",
      -601: "(GMT-01:00) Cape Verde Is.",
         1: "(GMT) Casablanca",
         2: "(GMT) Greenwich Mean Time : Dublin, Edinburgh, Lisbon, London",
         3: "(GMT) Monrovia, Reykjavik",
       600: "(GMT+01:00) Amsterdam, Berlin, Bern, Rome, Stockholm, Vienna",
       601: "(GMT+01:00) Belgrade, Bratislava, Budapest, Ljubljana, Prague",
       602: "(GMT+01:00) Brussels, Copenhagen, Madrid, Paris",
       603: "(GMT+01:00) Sarajevo, Skopje, Warsaw, Zagreb",
       604: "(GMT+01:00) West Central Africa",
      1200: "(GMT+02:00) Amman",
      1201: "(GMT+02:00) Athens, Bucharest, Istanbul",
      1202: "(GMT+02:00) Beirut",
      1203: "(GMT+02:00) Cairo",
      1204: "(GMT+02:00) Harare, Pretoria",
      1205: "(GMT+02:00) Helsinki, Kyiv, Riga, Sofia, Tallinn, Vilnius",
      1206: "(GMT+02:00) Jerusalem",
      1207: "(GMT+02:00) Minsk",
      1208: "(GMT+02:00) Windhoek",
      1800: "(GMT+03:00) Baghdad",
      1801: "(GMT+03:00) Kuwait, Riyadh",
      1802: "(GMT+03:00) Moscow, St. Petersburg, Volgograd",
      1803: "(GMT+03:00) Nairobi",
      1804: "(GMT+03:00) Tbilisi",
      2100: "(GMT+03:30) Tehran",
      2400: "(GMT+04:00) Abu Dhabi, Muscat",
      2401: "(GMT+04:00) Baku",
      2402: "(GMT+04:00) Caucasus Standard Time",
      2403: "(GMT+04:00) Port Louis",
      2404: "(GMT+04:00) Yerevan",
      2700: "(GMT+04:30) Kabul",
      3000: "(GMT+05:00) Ekaterinburg",
      3001: "(GMT+05:00) Islamabad, Karachi",
      3002: "(GMT+05:00) Tashkent",
      3300: "(GMT+05:30) Chennai, Kolkata, Mumbai, New Delhi",
      3301: "(GMT+05:30) Sri Jayawardenepura",
      3455: "(GMT+05:45) Kathmandu",
      3600: "(GMT+06:00) Almaty, Novosibirsk",
      3601: "(GMT+06:00) Astana, Dhaka",
      3600: "(GMT+06:30) Yangon (Rangoon)",
      4200: "(GMT+07:00) Bangkok, Hanoi, Jakarta",
      4201: "(GMT+07:00) Krasnoyarsk",
      4800: "(GMT+08:00) Beijing, Chongqing, Hong Kong, Urumqi",
      4801: "(GMT+08:00) Irkutsk, Ulaan Bataar",
      4802: "(GMT+08:00) Kuala Lumpur, Singapore",
      4803: "(GMT+08:00) Perth",
      4804: "(GMT+08:00) Taipei",
      5400: "(GMT+09:00) Osaka, Sapporo, Tokyo",
      5401: "(GMT+09:00) Seoul",
      5402: "(GMT+09:00) Yakutsk",
      5700: "(GMT+09:30) Adelaide",
      5701: "(GMT+09:30) Darwin",
      6000: "(GMT+10:00) Brisbane",
      6001: "(GMT+10:00) Canberra, Melbourne, Sydney",
      6002: "(GMT+10:00) Guam, Port Moresby",
      6003: "(GMT+10:00) Hobart",
      6004: "(GMT+10:00) Vladivostok",
      6600: "(GMT+11:00) Magadan, Solomon Is., New Caledonia",
      7200: "(GMT+12:00) Auckland, Wellington",
      7201: "(GMT+12:00) Fiji, Kamchatka, Marshall Is.",
      7800: "(GMT+13:00) Nuku'alofa"
    */
} time_conf_t;

/* Get daylight saving time offset */
u32 time_dst_get_offset(void);

/* Get daylight saving configuration */
vtss_rc time_dst_get_config(time_conf_t *conf);

/* Set daylight saving configuration */
vtss_rc time_dst_set_config(time_conf_t *conf);

/* Update time zone offset */
vtss_rc time_dst_update_tz_offset(int tz_off);

/* Initialize module */
vtss_rc time_dst_init(vtss_init_data_t *data);

#endif /* _VTSS_DAYLIGHT_SAVING_API_H_ */

/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/

