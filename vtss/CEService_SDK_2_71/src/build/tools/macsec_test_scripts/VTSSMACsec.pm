#
# Vitesse Switch Software.
#
# Copyright (c) 2002-2013 Vitesse Semiconductor Corporation "Vitesse". All
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
package VTSSMACsec;

require Exporter;
use Carp;
use Data::Dumper;
use JSON::RPC::Client;
use Crypt::Rijndael;

@ISA = qw(Exporter);
@EXPORT = qw(vtss_macsec_init_set
             vtss_macsec_init_get
             vtss_macsec_secy_conf_add
             vtss_macsec_secy_conf_get
             vtss_macsec_secy_controlled_set
             vtss_macsec_secy_controlled_get
             vtss_macsec_secy_port_status_get
             vtss_macsec_rx_sc_add
             vtss_macsec_rx_sc_get_all
             vtss_macsec_rx_sc_status_get
             vtss_macsec_tx_sc_set
             vtss_macsec_tx_sc_status_get
             vtss_macsec_rx_sa_set
             vtss_macsec_rx_sa_get
             vtss_macsec_rx_sa_activate
             vtss_macsec_rx_sa_disable
             vtss_macsec_rx_sa_lowest_pn_update
             vtss_macsec_rx_sa_status_get
             vtss_macsec_tx_sa_set
             vtss_macsec_tx_sa_get
             vtss_macsec_tx_sa_activate
             vtss_macsec_tx_sa_disable
             vtss_macsec_tx_sa_status_get
             vtss_macsec_controlled_counters_get
             vtss_macsec_uncontrolled_counters_get
             vtss_macsec_common_counters_get
             vtss_macsec_secy_cap_get
             vtss_macsec_secy_counters_get
             vtss_macsec_counters_update
             vtss_macsec_counters_clear
             vtss_macsec_rx_sc_counters_get
             vtss_macsec_tx_sc_counters_get
             vtss_macsec_tx_sa_counters_get
             vtss_macsec_rx_sa_counters_get
             vtss_macsec_control_frame_match_conf_set
             vtss_macsec_control_frame_match_conf_del
             vtss_macsec_control_frame_match_conf_get
             vtss_macsec_pattern_set
             vtss_macsec_pattern_del
             vtss_macsec_pattern_get
             vtss_macsec_default_action_set
             vtss_macsec_default_action_get
             vtss_macsec_bypass_mode_set
             vtss_macsec_bypass_mode_get
             vtss_macsec_bypass_tag_set
             vtss_macsec_bypass_tag_get
             vtss_macsec_frame_capture_set
             vtss_macsec_frame_get
             vtss_macsec_tx_sa_del
             vtss_macsec_rx_sa_del
             vtss_macsec_rx_sc_del
             vtss_macsec_tx_sc_del
             vtss_macsec_secy_conf_del
             vtss_macsec_event_poll
             vtss_macsec_event_enable_set
             vtss_macsec_event_enable_get
             vtss_macsec_event_seq_threshold_set
             vtss_macsec_event_seq_threshold_get
             as_number
             as_bool
             as_string
             sak_update_hash_key);

sub as_number {
    my ($num) = @_;
    return $num + 0;
}

sub as_bool {
    my ($bool) = @_;
    $bool = as_number($bool);

    if ($bool) {
        return JSON::XS::true;
    } else {
        return JSON::XS::false;
    }
}

sub as_string {
    my ($s) = @_;
    return $s."";
}

sub sak_update_hash_key {
    my ($sak) = @_;
    my $key = pack "H*", $sak->{'buf'};
    my $cipher = Crypt::Rijndael->new($key, Crypt::Rijndael::MODE_CBC());
    my @zeroes = (0) x 16;
    my $input = pack("C*", @zeroes);
    my $crypted = $cipher->encrypt($input);
    my @out = unpack "H*", $crypted;
    $sak->{'h_buf'} = @out[0];
    return $sak;
}


my $is_initliazed = 0;
my $client;
my $url = $ENV{'URI_RPC_SERVER'};

sub init {
    if (!$is_initliazed) {
        $is_initliazed = 1;
        $client = new JSON::RPC::Client;
        $client->version('1.0');
    }
}

sub call {
    init();

    my $method = shift(@_);
    my $callobj =  { method => $method,
                     params => [@_] };
    my $res = $client->call($url, $callobj);

    if($res) {
        if ($res->is_error) {
            print "\n\nGot an error from the RCP server\n";
            print "Callobject:\n";
            print Dumper $callobj;
            print "Error: ", $res->error_message, "\n";
            Carp::cluck "Back trace";
            print "\n";
        } else {
            return $res->result;
        }
    } else {
        print $client->status_line;
        print "\n";
        Carp::confess "Error reaching the RPC server: ".$url;
    }

    die;
    return undef;
}

sub vtss_macsec_init_set { return call("macsec.init_set", @_); }
sub vtss_macsec_init_get { return call("macsec.init_get", @_); }
sub vtss_macsec_secy_conf_add { return call("macsec.secy_conf_add", @_); }
sub vtss_macsec_secy_conf_get { return call("macsec.secy_conf_get", @_); }
sub vtss_macsec_secy_controlled_set { return call("macsec.secy_controlled_set", @_); }
sub vtss_macsec_secy_controlled_get { return call("macsec.secy_controlled_get", @_); }
sub vtss_macsec_secy_port_status_get { return call( "macsec.secy_port_status_get" , @_); }
sub vtss_macsec_rx_sc_add { return call( "macsec.rx_sc_add" , @_); }
sub vtss_macsec_rx_sc_get_all { return call( "macsec.rx_sc_get_all" , @_); }
sub vtss_macsec_rx_sc_status_get { return call( "macsec.rx_sc_status_get" , @_); }
sub vtss_macsec_tx_sc_set { return call( "macsec.tx_sc_set" , @_); }
sub vtss_macsec_tx_sc_status_get { return call( "macsec.tx_sc_status_get" , @_); }
sub vtss_macsec_rx_sa_set { return call("macsec.rx_sa_set" , @_); }
sub vtss_macsec_rx_sa_get { return call("macsec.rx_sa_get" , @_); }
sub vtss_macsec_rx_sa_activate { return call("macsec.rx_sa_activate" , @_); }
sub vtss_macsec_rx_sa_disable { return call("macsec.rx_sa_disable" , @_); }
sub vtss_macsec_rx_sa_lowest_pn_update { return call("macsec.rx_sa_lowest_pn_update" , @_); }
sub vtss_macsec_rx_sa_status_get { return call( "macsec.rx_sa_status_get" , @_); }
sub vtss_macsec_tx_sa_set { return call( "macsec.tx_sa_set" , @_); }
sub vtss_macsec_tx_sa_get { return call( "macsec.tx_sa_get" , @_ ); }
sub vtss_macsec_tx_sa_activate { return call("macsec.tx_sa_activate" , @_); }
sub vtss_macsec_tx_sa_disable { return call( "macsec.tx_sa_disable" , @_); }
sub vtss_macsec_tx_sa_status_get { return call( "macsec.tx_sa_status_get" , @_); }
sub vtss_macsec_controlled_counters_get { return call("macsec.controlled_counters_get" , @_); }
sub vtss_macsec_uncontrolled_counters_get { return call("macsec.uncontrolled_counters_get" , @_); }
sub vtss_macsec_common_counters_get { return call("macsec.common_counters_get" , @_); }
sub vtss_macsec_secy_cap_get { return call("macsec.secy_cap_get" , @_); }
sub vtss_macsec_secy_counters_get { return call("macsec.secy_counters_get" , @_); }
sub vtss_macsec_counters_update { return call("macsec.counters_update" , @_); }
sub vtss_macsec_counters_clear { return call("macsec.counters_clear" , @_); }
sub vtss_macsec_rx_sc_counters_get { return call("macsec.rx_sc_counters_get" , @_); }
sub vtss_macsec_tx_sc_counters_get { return call("macsec.tx_sc_counters_get" , @_); }
sub vtss_macsec_tx_sa_counters_get { return call("macsec.tx_sa_counters_get" , @_); }
sub vtss_macsec_rx_sa_counters_get { return call("macsec.rx_sa_counters_get" , @_); }
sub vtss_macsec_control_frame_match_conf_set { return call("macsec.control_frame_match_conf_set" , @_); }
sub vtss_macsec_control_frame_match_conf_del { return call("macsec.control_frame_match_conf_del" , @_); }
sub vtss_macsec_control_frame_match_conf_get { return call("macsec.control_frame_match_conf_get" , @_); }
sub vtss_macsec_pattern_set { return call("macsec.pattern_set" , @_); }
sub vtss_macsec_pattern_del { return call("macsec.pattern_del" , @_); }
sub vtss_macsec_pattern_get { return call("macsec.pattern_get" , @_); }
sub vtss_macsec_default_action_set { return call("macsec.default_action_set" , @_); }
sub vtss_macsec_default_action_get { return call("macsec.default_action_get" , @_); }
sub vtss_macsec_bypass_mode_set { return call("macsec.bypass_mode_set" , @_); }
sub vtss_macsec_bypass_mode_get { return call("macsec.bypass_mode_get" , @_); }
sub vtss_macsec_bypass_tag_set { return call("macsec.bypass_tag_set" , @_); }
sub vtss_macsec_bypass_tag_get { return call("macsec.bypass_tag_get" , @_); }
sub vtss_macsec_frame_capture_set { return call("macsec.frame_capture_set" , @_); }
sub vtss_macsec_frame_get { return call("macsec.frame_get" , @_); }
sub vtss_macsec_tx_sa_del { return call("macsec.tx_sa_del" , @_); }
sub vtss_macsec_rx_sa_del { return call("macsec.rx_sa_del" , @_); }
sub vtss_macsec_rx_sc_del { return call( "macsec.rx_sc_del" , @_); }
sub vtss_macsec_tx_sc_del { return call( "macsec.tx_sc_del" , @_); }
sub vtss_macsec_secy_conf_del { return call("macsec.secy_conf_del" , @_); }
sub vtss_macsec_event_enable_set{ return call("macsec.event_enable_set" , @_); }
sub vtss_macsec_event_enable_get{ return call("macsec.event_enable_get" , @_); }
sub vtss_macsec_event_poll{ return call("macsec.event_poll" , @_); }
sub vtss_macsec_event_seq_threshold_set{ return call("macsec.event_seq_threshold_set" , @_); }
sub vtss_macsec_event_seq_threshold_get{ return call("macsec.event_seq_threshold_get" , @_); }


1;
