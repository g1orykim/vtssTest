/*
 
 Vitesse Switch Software.
 
 Copyright (c) 2002-2011 Vitesse Semiconductor Corporation "Vitesse". All
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
var timezone_groups = Array(
 "Australia",
 "Europe",
 "New Zealand",
 "USA & Canada",
 "Atlantic",
 "Asia (UTC+1)",
 "Asia (UTC+2)",
 "Asia (UTC+3:30)",
 "Asia (UTC+4)",
 "Asia (UTC+4:30)",
 "Asia (UTC+5)",
 "Asia (UTC+5:30)",
 "Asia (UTC+6)",
 "Asia (UTC+7)",
 "Asia (UTC+8)",
 "Asia (UTC+9)",
 "Central and South America",
 "Unknown");

var timezones = Array(
 /* Australia */
 Array(
  "Australia/Melbourne@EST-10EST,M10.1.0,M4.1.0/3|Melbourne,Canberra,Sydney",
  "Australia/Perth@WST-8|Perth",
  "Australia/Brisbane@EST-10|Brisbane",
  "Australia/Adelaide@CST-9:30CST,M10.1.0,M4.1.0/3|Adelaide",
  "Australia/Darwin@CST-9:30|Darwin",
  "Australia/Hobart@EST-10EST,M10.1.0,M4.1.0/3|Hobart"
 ),
 /* Europe */
 Array(
  "Europe/Amsterdam@CET-1CEST,M3.5.0,M10.5.0/3|Amsterdam,Netherlands",
  "Europe/Athens@EET-2EEST,M3.5.0/3,M10.5.0/4|Athens,Greece",
  "Europe/Berlin@CET-1CEST,M3.5.0,M10.5.0/3|Berlin,Germany",
  "Europe/Brussels@CET-1CEST,M3.5.0,M10.5.0/3|Brussels,Belgium",
  "Europe/Bratislava@CET-1CEST,M3.5.0,M10.5.0/3|Bratislava,Slovakia",
  "Europe/Budapest@CET-1CEST,M3.5.0,M10.5.0/3|Budapest,Hungary",
  "Europe/Copenhagen@CET-1CEST,M3.5.0,M10.5.0/3|Copenhagen,Denmark",
  "Europe/Dublin@GMT0IST,M3.5.0/1,M10.5.0|Dublin,Ireland",
  "Europe/Helsinki@EET-2EEST,M3.5.0/3,M10.5.0/4|Helsinki,Finland",
  "Europe/Kiev@EET-2EEST,M3.5.0/3,M10.5.0/4|Kyiv,Ukraine",
  "Europe/Lisbon@WET0WEST,M3.5.0/1,M10.5.0|Lisbon,Portugal",
  "Europe/London@GMT0BST,M3.5.0/1,M10.5.0|London,GreatBritain",
  "Europe/Madrid@CET-1CEST,M3.5.0,M10.5.0/3|Madrid,Spain",
  "Europe/Oslo@CET-1CEST,M3.5.0,M10.5.0/3|Oslo,Norway",
  "Europe/Paris@CET-1CEST,M3.5.0,M10.5.0/3|Paris,France",
  "Europe/Prague@CET-1CEST,M3.5.0,M10.5.0/3|Prague,CzechRepublic",
  "Europe/Rome@CET-1CEST,M3.5.0,M10.5.0/3|Roma,Italy",
  "Europe/Moscow@MSK-3MSD,M3.5.0,M10.5.0/3|Moscow,Russia",
  "Europe/Stockholm@CET-1CEST,M3.5.0,M10.5.0/3|Stockholm,Sweden",
  "Europe/Zurich@CET-1CEST,M3.5.0,M10.5.0/3|Zurich,Switzerland"
 ),
 /* New Zealand */
 Array(
  "Pacific/Auckland@NZST-12NZDT,M9.5.0,M4.1.0/3|Auckland, Wellington"
 ),
 /* USA & Canada */
 Array(
  "Pacific/Honolulu@HST10|Hawaii Time",
  "America/Anchorage@AKST9AKDT,M3.2.0,M11.1.0|Alaska Time",
  "America/Los_Angeles@PST8PDT,M3.2.0,M11.1.0|Pacific Time",
  "America/Denver@MST7MDT,M3.2.0,M11.1.0|Mountain Time",
  "America/Phoenix@MST7|Mountain Time (Arizona, no DST)",
  "America/Chicago@CST6CDT,M3.2.0,M11.1.0|Central Time",
  "America/New_York@EST5EDT,M3.2.0,M11.1.0|Eastern Time",
  "America/Halifax@AST4ADT,M3.2.0,M11.1.0|Atlantic Time"
 ),
 /* Atlantic */
 Array(
  "Atlantic/Bermuda@AST4ADT,M3.2.0,M11.1.0|Bermuda"
 ),
 /* Asia (UTC+1) */
 Array(
  "Asia/Anadyr@ANAT-12ANAST,M3.5.0,M10.5.0/3|Anadyr"
 ),
 /* Asia (UTC+2) */
 Array(
  "Asia/Amman@EET-2EEST,M3.5.4/0,M10.5.5/1|Amman",
  "Asia/Beirut@EET-2EEST,M3.5.0/0,M10.5.0/0|Beirut",
  "Asia/Damascus@EET-2EEST,M3.5.5/0,M11.1.5/0|Damascus",
  "Asia/Gaza@EET-2EEST,J91/0,M9.2.4|Gaza",
  "Asia/Jerusalem@GMT-2|Jerusalem",
  "Asia/Nicosia@EET-2EEST,M3.5.0/3,M10.5.0/4|Nicosia",
  "Asia (UTC+3)",
  "Asia/Aden@AST-3|Aden",
  "Asia/Baghdad@AST-3ADT,J91/3,J274/4|Baghdad",
  "Asia/Bahrain@AST-3|Bahrain",
  "Asia/Kuwait@AST-3|Kuwait",
  "Asia/Qatar@AST-3|Qatar",
  "Asia/Riyadh@AST-3|Riyadh"
 ),
 /* Asia (UTC+3:30) */
 Array(
  "Asia/Tehran@IRST-3:30|Tehran"
 ),
 /* Asia (UTC+4) */
 Array(
  "Asia/Baku@AZT-4AZST,M3.5.0/4,M10.5.0/5|Baku",
  "Asia/Dubai@GST-4|Dubai",
  "Asia/Muscat@GST-4|Muscat",
  "Asia/Tbilisi@GET-4|Tbilisi",
  "Asia/Yerevan@AMT-4AMST,M3.5.0,M10.5.0/3|Yerevan"
 ),
 /* Asia (UTC+4:30) */
 Array(
  "Asia/Kabul@AFT-4:30|Kabul"
 ),
 /* Asia (UTC+5) */
 Array(
  "Asia/Aqtobe@AQTT-5|Aqtobe",
  "Asia/Ashgabat@TMT-5|Ashgabat",
  "Asia/Dushanbe@TJT-5|Dushanbe",
  "Asia/Karachi@PKT-5|Karachi",
  "Asia/Oral@ORAT-5|Oral",
  "Asia/Samarkand@UZT-5|Samarkand",
  "Asia/Tashkent@UZT-5|Tashkent",
  "Asia/Yekaterinburg@YEKT-5YEKST,M3.5.0,M10.5.0/3|Yekaterinburg"
 ),
 /* Asia (UTC+5:30) */
 Array(
  "Asia/Calcutta@IST-5:30|Calcutta",
  "Asia/Colombo@IST-5:30|Colombo"
 ),
 /* Asia (UTC+6) */
 Array(
  "Asia/Almaty@ALMT-6|Almaty",
  "Asia/Bishkek@KGT-6|Bishkek",
  "Asia/Dhaka@BDT-6|Dhaka",
  "Asia/Novosibirsk@NOVT-6NOVST,M3.5.0,M10.5.0/3|Novosibirsk",
  "Asia/Omsk@OMST-6OMSST,M3.5.0,M10.5.0/3|Omsk",
  "Asia/Qyzylorda@QYZT-6|Qyzylorda",
  "Asia/Thimphu@BTT-6|Thimphu"
 ),
 /* Asia (UTC+7) */
 Array(
  "Asia/Jakarta@WIT-7|Jakarta",
  "Asia/Bangkok@ICT-7|Bangkok",
  "Asia/Vientiane@ICT-7|Vientiane",
  "Asia/Phnom_Penh@ICT-7|Phnom Penh"
 ),
 /* Asia (UTC+8) */
 Array(
  "Asia/Chongqing@CST-8|Chongqing",
  "Asia/Hong_Kong@HKT-8|Hong Kong",
  "Asia/Shanghai@CST-8|Shanghai",
  "Asia/Singapore@SGT-8|Singapore",
  "Asia/Urumqi@CST-8|Urumqi",
  "Asia/Taipei@CST-8|Taiwan",
  "Asia/Ulaanbaatar@ULAT-8|Ulaanbaatar"
 ),
 /* Asia (UTC+9) */
 Array(
  "Asia/Dili@TLT-9|Dili",
  "Asia/Jayapura@EIT-9|Jayapura",
  "Asia/Pyongyang@KST-9|Pyongyang",
  "Asia/Seoul@KST-9|Seoul",
  "Asia/Tokyo@JST-9|Tokyo",
  "Asia/Yakutsk@YAKT-9YAKST,M3.5.0,M10.5.0/3|Yakutsk"
 ),
 /* Central and South America */
 Array(
  "America/Sao_Paulo@BRT3BRST,M10.2.0/0,M2.3.0/0|Sao Paulo,Brazil",
  "America/Argentina/Buenos_Aires@ART3|Argentina",
  "America/Guatemala@CST6|Central America (no DST)"
 ),
 /* Unknown */
 Array(
  "-@UTC+0|Unknown"
 )
);
