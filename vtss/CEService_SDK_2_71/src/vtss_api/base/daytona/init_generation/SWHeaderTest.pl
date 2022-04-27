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
# Test File to read in config info

# pass in working directory and block name from excel
my $workdir = @ARGV[0];
my $blockname = @ARGV[1];
my $outdir = @ARGV[2];

# build up file names from this
my $importfile = $workdir . $blockname . ".txt";
my $hfilename = $outdir . "vtss_daytona_reg_init_" . $blockname . ".h";
my $cfilename = $outdir . "vtss_daytona_reg_init_" . $blockname . ".inc";

# open output files to blow away old data
open (HFH, ">", $hfilename) or die "Cannot open $hfilename for writing";
open (CFH, ">", $cfilename) or die "Cannot open $cfilename for writing";

# print Vitesse Copyright header
print CFH "/*\n\n Vitesse Switch Software.\n\n";
print CFH " Copyright (c) 2002-2013 Vitesse Semiconductor Corporation \"Vitesse\". All\n";
print CFH " Rights Reserved.\n\n";
print CFH " Unpublished rights reserved under the copyright laws of the United States of\n";
print CFH " America, other countries and international treaties. Permission to use, copy,\n";
print CFH " store and modify, the software and its source code is granted. Permission to\n";
print CFH " integrate into other products, disclose, transmit and distribute the software\n";
print CFH " in an absolute machine readable format (e.g. HEX file) is also granted.  The\n";
print CFH " source code of the software may not be disclosed, transmitted or distributed\n";
print CFH " without the written permission of Vitesse. The software and its source code\n";
print CFH " may only be used in products utilizing the Vitesse switch products.\n\n";
print CFH " This copyright notice must appear in any copy, modification, disclosure,\n";
print CFH " transmission or distribution of the software. Vitesse retains all ownership,\n";
print CFH " copyright, trade secret and proprietary rights in the software.\n\n";
print CFH " THIS SOFTWARE HAS BEEN PROVIDED \"AS IS,\" WITHOUT EXPRESS OR IMPLIED WARRANTY\n";
print CFH " INCLUDING, WITHOUT LIMITATION, IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS\n";
print CFH " FOR A PARTICULAR USE AND NON-INFRINGEMENT.\n\n";
print CFH "*/\n";

print HFH "/*\n\n Vitesse Switch Software.\n\n";
print HFH " Copyright (c) 2002-2013 Vitesse Semiconductor Corporation \"Vitesse\". All\n";
print HFH " Rights Reserved.\n\n";
print HFH " Unpublished rights reserved under the copyright laws of the United States of\n";
print HFH " America, other countries and international treaties. Permission to use, copy,\n";
print HFH " store and modify, the software and its source code is granted. Permission to\n";
print HFH " integrate into other products, disclose, transmit and distribute the software\n";
print HFH " in an absolute machine readable format (e.g. HEX file) is also granted.  The\n";
print HFH " source code of the software may not be disclosed, transmitted or distributed\n";
print HFH " without the written permission of Vitesse. The software and its source code\n";
print HFH " may only be used in products utilizing the Vitesse switch products.\n\n";
print HFH " This copyright notice must appear in any copy, modification, disclosure,\n";
print HFH " transmission or distribution of the software. Vitesse retains all ownership,\n";
print HFH " copyright, trade secret and proprietary rights in the software.\n\n";
print HFH " THIS SOFTWARE HAS BEEN PROVIDED \"AS IS,\" WITHOUT EXPRESS OR IMPLIED WARRANTY\n";
print HFH " INCLUDING, WITHOUT LIMITATION, IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS\n";
print HFH " FOR A PARTICULAR USE AND NON-INFRINGEMENT.\n\n";
print HFH "*/\n";

# print out include protection
print HFH "#ifndef _VTSS_DAYTONA_REG_INIT_" . uc($blockname) . "_H" . "\n";
print HFH "#define _VTSS_DAYTONA_REG_INIT_" . uc($blockname) . "_H" . "\n\n";

close HFH;
close CFH;

# change based on the format of the config spreadhseet
my $nondatacols = 2;
my $nondatarows = 6;

# var for debug printing
my $debug = 0;

# initialize local variables
my @blocks = ();
my @reggrps = ();
my @regs = ();
my @fields = ();
my @usesws = ();
my $line = 0;
my $blocktable = "static const static_cfg_t *" . lc($blockname) . "_config_table[BM_" . uc($blockname) . "_LAST] = {\n";
my $blocktable0 = "static const static_cfg_t *" . lc($blockname) . "_0_config_table[BM_" . uc($blockname) . "_LAST] = {\n";
my $blocktable1 = "static const static_cfg_t *" . lc($blockname) . "_1_config_table[BM_" . uc($blockname) . "_LAST] = {\n";
my $blockmode = "typedef enum {\n";
#my $block = "";
(-e $importfile) or die "File $importfile does not exist!";

# open file to read from 
open (FH, "<", $importfile);

print "\nCreating SW Files for block " . $blockname . "\n";
stdouthot(); # make stdout hot

my %block_alias = (
    'cfg_if_gcb' => 'devcpu_gcb',
    'wis' => 'ewis',
    'pcs' => 'pcs_10gbase_r',
    'mac' => 'mac10g',
    'ptp' => 'ip_1588',
    'otu' => 'otn_wrapper',
    'efec' => 'efeci4',
    'xaui' => 'pcs_xaui',
    'sfi' => 'sfi4',
    'tfi' => 'tfi_5'
    );


while (<FH>) {
	my @data  = split("\t", $_);
	# remove any extra tabs at the end of the line
	while (($data[$#data] eq "\n" | $data[$#data] eq "") && $#data >= 1) { pop @data; }
	$dl = @data;
	#debprint "(header test) line " . $line . "data length is" . $dl . "\n";
	if ($line == 0) { ######### Line 0 - Block Name
		# pop first few columns off of data
		for (my $j=0; $j<$nondatacols; $j++) { shift @data; };
		# add block names to list
		#debprint "(header test) in case 1 (line" . $line . "), data is " . @data . "\n";
		foreach $block (@data) { 
			$block =~ s/\n//g; # remove end of line
			$block =~ s/_xx|_XX|_yy|_YY//g; # remove block number, don't care as they should be the same
            # convert blockname to the one used in the register definitions generated from the CML
            my $newblock = $block_alias{$block};
            if ($newblock ne "") {$block = $newblock; }
			push(@blocks, $block);
		}
	} elsif ($line == 1) { ######### Line 1 - Register Group Name
		# pop first few columns off of data
		for (my $j=0; $j<$nondatacols; $j++) { shift @data; };
		# add register names to list
		#debprint "(header test) in case 0.5 (line" . $line . "), data is " . @data . "\n";
		foreach $reggrp (@data) { push(@reggrps, $reggrp); }
	} elsif ($line == 2) { ######### Line 2 - Register Name
		# pop first few columns off of data
		for (my $j=0; $j<$nondatacols; $j++) { shift @data; };
		# add register names to list
		#debprint "(header test) in case 1 (line" . $line . "), data is " . @data . "\n";
		foreach $reg (@data) { 
			$reg =~ s/_yy|_YY//; # remove block number, don't care as they should be the same
			push(@regs, $reg);
		}
	} elsif ($line == 3) { ######### Line 3 - Field Name
		# pop first few columns off of data
		for (my $j=0; $j<$nondatacols; $j++) { shift @data; };
		# add field names to list
		#debprint "(header test) in case 2 (line" . $line . "), data is " . @data . "\n";
		foreach $field (@data) { push(@fields, $field); }
	} elsif ($line == 4) { ######### Line 4 - Use for Validation
		# this is the "use for validation" row, ignore for SW
	} elsif ($line == 5) { ######### Line 5 - Use for Software
		# pop first few columns off of data
		for (my $j=0; $j<$nondatacols; $j++) { shift @data; };
		# add use for sw names to list
		#debprint "(header test) in case 2 (line" . $line . "), data is " . @data . "\n";
		foreach $tmp (@data) { push(@usesws, $tmp); }
	} else {
		# add values from a particular mode to the list
		# first col has mode name
		my $mode = shift @data;
		# remove unused mode names (which are in the first few columns)
		for (my $j=1; $j<$nondatacols; $j++) { shift @data; };
		# check that we still have some data, this filters out empty lines
		$dl = @data;
		if ($dl >= 1 && $data[0] ne "\n") { 
			# now call fucntion to process this line
			$data = join("\t",@data);
			#print "(header test) in case 2 (block $block) (line" . $line . "), data is " . @data . "\n";
			ProcessMode($block, $mode, $data, $line);
            $blockmode = $blockmode . "    BM_" . uc($blockname) . "_" . uc($mode) . ",\n";
		}	
	}
	print ".";
	$line++;
}

open (CFH, ">>", $cfilename) or die "Cannot open $cfilename for writing";
if ( $blockname =~ /gcb/ ) {
    print CFH $blocktable0 . "};\n\n";
    print CFH $blocktable1 . "};\n\n";
} else {
    print CFH $blocktable . "};\n\n";
}
close CFH;

open (HFH, ">>", $hfilename) or die "Cannot open $hfilename for writing";
    print HFH "\n\n" . $blockmode . "    BM_" . uc($blockname) . "_LAST\n} block_" . $blockname . "_mode_t;\n\n";
    print HFH "#endif /* _VTSS_DAYTONA_REG_INIT_" . uc($blockname) . "_H */" . "\n";
close HFH;

print "\nComplete!\n";

# close the file before we exit
close FH;

sub ProcessMode {
	my $block = shift;
	my $mode = shift;
	my $data = shift;
	my $line = shift;
	chomp $block;
	chomp $mode;
	
	# local variables
	my $structvar = "";
	
	# open output file for processing (appending each mode)
	open (HFH, ">>", $hfilename) or die "Cannot open $hfilename for writing";
	open (CFH, ">>", $cfilename) or die "Cannot open $cfilename for writing";
	
	my @data = split("\t", $data);  # split data to be processed individually
	my $dl = @data;                 # get length (which is 1 less than this)
	$dl--;
	my $i = 0;                      # index for this array
	
	# print out header for this mode
	print HFH "// Settings for mode $mode\n\n";
	
	my $currentfields = 0;  # informs us if there are already values stored
	my $firstfield = 1;     # informs us if this is the first field in the reg
	my $lastfield = 0;      # informs us if this is the last field in the reg
	my $lastreg = 0;        # informs us if we are at the end of the registers
	my $numregs = 0;        # number of registers in a mode

	foreach $setting (@data) {
		my $block = $blocks[$i];
		my $reggrp = $reggrps[$i];
		my $reg = $regs[$i];
		my $field = $fields[$i];
		my $usesw = $usesws[$i];
		my $regindex = -1;
        chomp $block;
		chomp $reggrp;
		chomp $reg;
		chomp $field;
        chomp $usesw;
        chomp $setting;

		# first check to see if this register is of use for SW, if not then just continue
		if ( $usesw ne "x" ) { $i++; next; }

		# need to change some stuff around if the register is replicated.  if so:
		# name will appear as <name>:<index>
		# seperate into two variables to use below
		if ( $reg =~ /(.*):(\d+)/ ) {
			$reg = $1;
			$regindex = $2;
		}
		
		# make sure we grab the default value, not the randomized values
		$setting = ProcessData($setting);
		
		# create string with mode prefix
		my $fieldname = join("_", "VTSS", uc($block), uc($reggrp), uc($reg), uc($field), $mode);
		my $maskname = join("_", "VTSS_M", uc($block), uc($reggrp), uc($reg), uc($field));
		my $macroname = join("_", "VTSS_F", uc($block), uc($reggrp), uc($reg), uc($field));
		
		###############
		# Header File
		# write the data and name to the header file do one field at a time
		# need to replace channel number if this is a GCB worksheet (and write out 2 copies)
		if ( $block =~ /gcb/ ) {
			$printfield0 = $fieldname;  $printfield0 =~ s/XX/0/g;
			$printfield1 = $fieldname;  $printfield1 =~ s/XX/1/g;
			$macro0 = $macroname;  $macro0 =~ s/XX/0/g;
			$macro1 = $macroname;  $macro1 =~ s/XX/1/g;
			printf (HFH "#define  %-90s %s(0x%x)\n", $printfield0, $macro0, $setting);
			printf (HFH "#define  %-90s %s(0x%x)\n", $printfield1, $macro1, $setting);
		} else {
			printf (HFH "#define  %-90s %s(0x%x)\n", $fieldname, $macroname, $setting);
		}

		###############
		# Source File
		# figure out where we are in the register
		if ( $i == 0 ) { # first field
			$firstfield = 1;
		} 
		if ($i == $dl) { # last field
			$lastfield = 1;
			$lastreg = 1;
		}
		#now figure out if we have hit the end of a register
        $reg1 = $regs[$i+1];
        chomp $reg1;
		if ($reg ne $reg1) {
			$lastfield = 1;
		}

		#print "thisreg \'$regs[$i]\', next \'$regs[$i+1]\'\n";
		#print "first $firstfield, last $lastfield, curr $currentfields, i $i, dl $dl\n";
		# now deal with building the string
		# if we are at the beginning of a register then first add header
		if ( $firstfield == 1 || $currentfields == 0 ) {
			# add replication as second arg if available
            if ( $block =~ /gcb/ ) {}
            else
            {
				if ( $regindex != -1 ) {
					$regargs = "(0, " . $regindex . ")";
				} else {
                    $regargs = "(0)";
				}
            }
			$structvar = $structvar . "{DAYTONA_BLOCK_" . uc($block) . ", ";
			$structvar = $structvar . "VTSS_" . uc($block) . "_" . uc($reggrp) . "_" . uc($reg) . uc($regargs) . ", (";
            $maskvar = ", (";
			$firstfield = 0;	# reset
			$numregs++;  # at the end of a register entry, so incrment
		}
		# now add on this field
		$structvar = $structvar . $fieldname;
		$maskvar = $maskvar . $maskname;
		$currentfields++;
		# then deal with end of string
		if ( $lastfield == 0 ) {
			# if we are in the middle of a register, add a spacer between this and the next
			$structvar = $structvar . " | ";
			$maskvar = $maskvar . " | ";
		} else {
			# if we are at the last register, then do not add an extra comma
			if ( $lastreg == 1 ) {
				# end of this mode, close up the struct and move on
				$structvar = $structvar . ")" . $maskvar . ")}";
			} else {
				# end of register but more regs to go, move to next line and set that up
				$structvar = $structvar . ")" . $maskvar . ")},\n\t\t\t\t";
			}
			$lastfield = 0;     # reset
			$currentfields = 0; # reset
			$firstfield = 1;
		}

		# increment index to next field
		$i++;
	}
	
	# Now that we are done with this mode, write full struct to c file


	print CFH "// register structures for mode $mode\n\n";
	# first need to replace channel number if this is a GCB worksheet (and write out 2 copies)

	if ( $blockname =~ /gcb/ ) {
		if ( $blockname eq 'gcb' ) {
                #Subtract the number of registers with the TOP_CLK_MUX registers for GCB block
	        $numregs = $numregs - 2;
        }
        $structbegin0 = "";
		if ($line == 6)     {$structbegin0 = "#define " . uc($blockname) . "_TBL_SIZE   " . $numregs . "\n\n ";}
		$structbegin0 = $structbegin0 . "static static_cfg_t " . lc($blockname) . "_0_" . lc($mode) . "_tbl[" . $numregs . "] = { ";   
		$structbegin1 = "static static_cfg_t " . lc($blockname) . "_1_" . lc($mode) . "_tbl[" . $numregs . "] = { ";   
		$structend = " };\n";
        $blocktable0 = $blocktable0 . "    " . lc($blockname) . "_0_" . lc($mode) . "_tbl,\n";
        $blocktable1 = $blocktable1 . "    " . lc($blockname) . "_1_" . lc($mode) . "_tbl,\n";
		$structvar0 = $structvar; $structvar0 =~ s/XX/0/g;
		$structvar1 = $structvar; $structvar1 =~ s/XX/1/g;
                # Only keep the TOP_CLK_MUX register configurations for corresponding channel - start
		if ( $blockname eq 'gcb' ) {
			my @line = split (/\n/, $structvar0);
			my @arr_var0; 
			my @arr_var1; 
			$structvar0 = "";
			foreach $l (@line) {
				if ($l !~ m/VTSS_DEVCPU_GCB_CHIP_MUX_TOP_CLK_MUX_1/) {
				push (@arr_var0, "$l");
				}
				$structvar0 = join "\n", @arr_var0 ;
			}
			@line = split (/\n/, $structvar1);
			$structvar1 = "";
			foreach $l (@line) {
				if ($l !~ m/VTSS_DEVCPU_GCB_CHIP_MUX_TOP_CLK_MUX_0/) {
				push (@arr_var1, "$l");
				}
				$structvar1 = join "\n", @arr_var1 ;
			}
        	}
                # Only keep the TOP_CLK_MUX register configurations for corresponding channel - end 
		print CFH $structbegin0 . $structvar0 . $structend . "\n\n";
		print CFH $structbegin1 . $structvar1 . $structend . "\n\n";

		$structvar = "";
		print HFH "\n\n";
	} else {
        $structbegin = "";
		if ($line == 6)     {$structbegin = "#define " . uc($blockname) . "_TBL_SIZE   " . $numregs . "\n ";}
		$structbegin = $structbegin . "static static_cfg_t " . lc($blockname) . "_" . lc($mode) . "_tbl[" . $numregs . "] = { ";   
		$structend = " };\n";
        $blocktable = $blocktable . "    " . lc($blockname) . "_" . lc($mode) . "_tbl,\n";
		print CFH $structbegin . $structvar . $structend . "\n\n";
		$structvar = "";
		print HFH "\n\n";
	}
			
	
	
	# close files 
	close HFH;
	close CFH;
}

# takes a single data value from the spreadsheet and pulls out the default value
# and removes the randomized values
# data will be one of 2 forms:
#  1) default only
#  2) <default> | <random>
sub ProcessData {
	my $data = shift;
	
	if ( $data =~ /(.*)\|(.*)/ ) {
		return $1;
	} else {
		return $data;
	}
}
	
# calls print only if the global variable debugp is set
sub debprint {
  my $string = shift;
  if ( $debug == 1 ) { print $string; }
}

# makes stdout "hot", or unbuffered, so we can show progress
sub stdouthot { 
	my $ofh = select STDOUT;
	  $| = 1;
	  select $ofh;
}
