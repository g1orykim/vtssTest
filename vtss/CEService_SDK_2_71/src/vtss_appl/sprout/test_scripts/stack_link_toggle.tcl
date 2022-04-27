########################################-*- mode: TCL; tcl-indent-level: 2 -*-
#
# Vitesse Switch Software.
#
# Copyright (c) 2002-2012 Vitesse Semiconductor Corporation "Vitesse". All
# Rights Reserved.
#
# Unpublished rights reserved under the copyright laws of the United States of
# America, other countries and international treaties. Permission to use, copy,
# store and modify, the software and its source code is granted. Permission to
# integrate into other products, disclose, transmit and distribute the software
# in an absolute machine readable format (e.g. HEX file) is also granted.  The
# source code of the software may not be disclosed, transmitted or distributed
# without the written permission of Vitesse. The software and its source code
# may only be used in products utilizing the Vitesse switch products.
#
# This copyright notice must appear in any copy, modification, disclosure,
# transmission or distribution of the software. Vitesse retains all ownership,
# copyright, trade secret and proprietary rights in the software.
#
# THIS SOFTWARE HAS BEEN PROVIDED "AS IS," WITHOUT EXPRESS OR IMPLIED WARRANTY
# INCLUDING, WITHOUT LIMITATION, IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
# FOR A PARTICULAR USE AND NON-INFRINGEMENT.
#
##############################################################################

# Script for toggling stack links of 48 port Jaguar1 switch
# 
# Usage: expect stack_link_toggle.tcl <host> <socket>

package require Expect

# Get host name and socket to telnet to
if {$argc != 2} {
  error "Incorrect number of arguments"
}

set host [lindex $argv 0]
set sock [lindex $argv 1]


# Login (if needed)
proc login { args } {
  send "\r"
  expect {
    -regexp ">$" { 
      puts "\nAlready logged in => Skipping login procedure"
      return 
    }
    "Username:" { }
    "Password:" { 
      send "\r"
      expect "Username:"
    }
  }
  puts "\nLogging in ..."
  send "admin\r\r"
  expect_prompt
}; # login


# Return random value in interval [1;max]
proc rand_int { max } {
  return [expr 1+int($max*rand())]
}


proc expect_prompt { args } {
  expect {
    -regexp ">$" { return }
    timeout      { puts "Timeout waiting for prompt\n" }
  }
}

# Toggle port link
# 
# Return value:
#   0: Link disabled
#   1: Link enabled
proc stack_port_link_toggle { port } {
  switch $port {
    49      { set chip 1 }
    51      { set chip 0 }
    default { error "Unsupport port: $port" }
  }

  send "debug chip $chip\r"
  expect_prompt
  send "debug sym read DEV10G\[2\]:PCS_XAUI_CONFIGURATION:PCS_XAUI_SD_CFG\r"
  expect {
    "0x00000011" {
      set up 1
    }
    "0x00000001" {
      set up 0
    }
  }

  if {$up} {
    puts "\nPort $port going down ..."
    send "debug sym write DEV10G\[2\]:PCS_XAUI_CONFIGURATION:PCS_XAUI_SD_CFG 0x1\r"
    set rc 0
  } else {
    puts "\nPort $port going up ..."
    send "debug sym write DEV10G\[2\]:PCS_XAUI_CONFIGURATION:PCS_XAUI_SD_CFG 0x11\r"
    set rc 1
  }
  expect_prompt

  return $rc
}; # stack_port_link_toggle

puts "Telnetting to $host $sock ..."
spawn telnet $host $sock
expect "Escape character is"
puts "We're in!" 
sleep 1
login

while {1} {
  if {[rand_int 10] <= 3} {
    stack_port_link_toggle 49
  } 
  if {[rand_int 10] <= 3} {
    stack_port_link_toggle 51
  } 
  set secs [rand_int 3]
  puts "Sleeping $secs seconds ..."
  sleep $secs
}

puts "Goodbye" 
exit


##############################################################################
#                                                                            #
#  End of file.                                                              #
#                                                                            #
##############################################################################
