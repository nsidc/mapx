#!/usr/bin/perl -w
#

#==============================================================================
# utest.pl - run mapx unit test for one or more mpp files
#
# 11-Apr-2003 Terry Haran, tharan@nsidc.org, 303-492-1847
# National Snow & Ice Data Center, University of Colorado, Boulder
#==============================================================================
#
# $Header: /tmp_mnt/FILES/mapx/unit_test/utest.pl,v 1.12 2003-04-23 20:39:45 haran Exp $
#

#
# make stderr and stdout unbuffered so nothing gets lost
#
$| = 1;
select(STDERR);
select(STDOUT);

$script = "UTEST";
$script = $script;

$Usage = "\n
USAGE: utest.pl [-v] mppfile1 [mppfile2...]

   input:
       mppfile - Map Projection Parameters file to be used as input to
                 macct and xytest to test a particular map projection.
                 One test of macct to be performed must consist of a group
                 of six lines in mppfile of the form:
                   # macct
                   # <points> points,  <bad_points> bad points
                   # average error = <average_error> km
                   # std dev error = <std_dev_error> km
                   # maximum error = <max_error> km
                   # max error was at <max_err_lat> <max_err_lon>
                 Each test of xytest to be performed must consist of a group
                 of three lines in mppfile (following the macct group, if any)
                 of the form:
                   # forward
                   # lat,lon = <lat_in> <lon_in>
                   # x,y = <x_expected> <y_expected> <comment>
                 or
                   # inverse
                   # x,y = <x_in> <y_in>
                   # lat,lon = <lat_expected> <lon_expected> <comment>

   options:
       -v - verbose
";
        
#
# check out the arguments
#
if (@ARGV < 1) {
    die($Usage);
}
my $verbose = 0;
if ($ARGV[0] eq "-v") {
    shift(@ARGV);
    $verbose = 1;
}
my @mppfiles = @ARGV;
my $mppfile;
my $tmpfile = "utest_temp.txt";
my $exit_value = 0;

#
# Loop through each mpp file
#
foreach $mppfile (@mppfiles) {

    my $forinv_ok = 0;
    my $macct_ok = 0;

    #
    # Read the entire mpp file
    #
    if (!open(MPP, "$mppfile")) {
	print STDERR ("$script: ERROR: $mppfile:\n" .
		      "Can't open $mppfile for reading\n");
	next;
    }
    my @mpp = (<MPP>);
    close(MPP);

    #
    # Run macct to get the macct actual lines
    #

    my @macct_act_lines = `macct $mppfile 2>&1`;
    my $i;
    for ($i = 0; $i < scalar(@macct_act_lines); $i++) {
	chomp $macct_act_lines[$i];
	$macct_act_lines[$i] =~ s/\s*$//;
    }

    my $header_printed        = 0;
    my $got_one_macct_result  = 0;
    my $got_one_forinv_result = 0;
    $i = 0;
    my @mpp_lines;
    while(1) {
	
	#
	# Find the next macct, forward, or inverse line
	#
	my $got_macct   = 0;
	my $got_forward = 0;
	my $got_inverse = 0;
	while($i < scalar(@mpp)) {
	    my $line = $mpp[$i++];
	    chomp $line;
	    if (substr($line, 0, 1) ne ";" && substr($line, 0, 1) ne "#") {
		push(@mpp_lines, $line);
	    }
	    $got_macct   = ($line =~ /(\;|\#)\s*macct/);
	    $got_forward = ($line =~ /(\;|\#)\s*forward/);
	    $got_inverse = ($line =~ /(\;|\#)\s*inverse/);
	    if ($got_macct || $got_forward || $got_inverse) {
		last;
	    }
	}
	if (!$got_one_macct_result) {
	    my $macct_ok = 0;
	    my @macct_exp_lines;
	    if ($got_macct) {

		#
		# Compare macct expected and actual results
		#
		if (scalar(@macct_act_lines) == 5) {
		    my $j;
		    my $got_bad_one = 0;
		    for ($j = 0; $j < 5; $j++) {
			if ($i < scalar(@mpp)) {
			    my $line = $mpp[$i++];
			    chomp $line;
			    $line =~ s/^(\;|\#)\s*//;
			    $line =~ s/\s*$//;
			    $macct_exp_lines[$j] = $line;
			    if ($line ne $macct_act_lines[$j]) {
				$got_bad_one = 1;
			    }
			} else {
			    last;
			}
		    }
		    $macct_ok = !$got_bad_one;
		}
	    }
	    if ($verbose || !$macct_ok) {
		&print_header($mppfile, @mpp_lines);
		$header_printed = 1;
		&print_header2;
		print STDERR ("  macct expected results:\n");
		my $j;
		for ($j = 0; $j < scalar(@macct_exp_lines); $j++) {
		    print STDERR "    $macct_exp_lines[$j]\n";
		}
		print STDERR ("  macct actual results:\n");
		for ($j = 0; $j < scalar(@macct_act_lines); $j++) {
		    print STDERR "    $macct_act_lines[$j]\n";
		}
		if (!$macct_ok) {
		    print STDERR ("  Expected and actual results differ\n");
		}
		if ($verbose && $macct_ok) {
		    print STDERR ("  Expected results match actual results\n");
		}
	    }
	    $got_one_macct_result = 1;
	    if ($got_macct) {
		next;
	    }
	}
	if (!$got_forward && !$got_inverse) {
	    if (!$got_one_forinv_result) {
		print STDERR ("$script: ERROR: $mppfile:\n" .
			      "Can't find forward or inverse in $mppfile\n");
	    }
	    last;
	}

	my $tmpfile_string;
	my $target;
	my $junk;
	my $lat_in;
	my $lon_in;
	my $x_in;
	my $y_in;
	my $x_expected;
	my $y_expected;
	my $lat_expected;
	my $lon_expected;
	if ($got_forward) {

	    #
	    # Find the next input lat,lon line
	    #
	    my $got_latlon_in = 0;
	    while($i < scalar(@mpp)) {
		my $line = $mpp[$i++];
		chomp $line;
		($junk, $lat_in, $lon_in) =
		    ($line =~ /(\;|\#)\s+lat\,lon\s+\=\s*(\S+)\s+(\S+)/);
		if (defined($lat_in) && defined($lon_in)) {
		    $got_latlon_in = 1;
		    last;
		}
	    }
	    if (!$got_latlon_in) {
		print STDERR ("$script: ERROR: $mppfile:\n" .
			      "Can't find input lat,lon in $mppfile\n");
		last;
	    }
	    
	    #
	    # Find the next expected x,y line
	    #
	    my $got_xy_expected = 0;
	    while($i < scalar(@mpp)) {
		my $line = $mpp[$i++];
		chomp $line;
		($junk, $x_expected, $y_expected, $xy_comment) =
		    ($line =~ /(\;|\#)\s+x\,y\s+\=\s*(\S+)\s+(\S+)\s*(.*)/);
		if (defined($x_expected) && defined($y_expected)) {
		    $got_xy_expected = 1;
		    if (!defined($xy_comment)) {
			$xy_comment = "";
		    }
		    last;
		}
	    }
	    if (!$got_xy_expected) {
		print STDERR ("$script: ERROR: $mppfile:\n" .
			      "Can't find expected x,y in $mppfile\n");
		last;
	    }
	    $tmpfile_string = "$lat_in $lon_in\n\n\n\n";
	    $target = "x,y \=";

	} else {
        
	    #
	    # Find the next input x,y line
	    #
	    my $got_xy_in = 0;
	    while($i < scalar(@mpp)) {
		my $line = $mpp[$i++];
		chomp $line;
		($junk, $x_in, $y_in) =
		    ($line =~ /(\;|\#)\s+x\,y\s+\=\s*(\S+)\s+(\S+)/);
		if (defined($x_in) && defined($y_in)) {
		    $got_xy_in = 1;
		    last;
		}
	    }
	    if (!$got_xy_in) {
		print STDERR ("$script: ERROR: $mppfile:\n" .
			      "Can't find input x,y in $mppfile\n");
		last;
	    }
	    
	    #
	    # Find the next expected lat,lon line
	    #
	    my $got_latlon_expected = 0;
	    while($i < scalar(@mpp)) {
		my $line = $mpp[$i++];
		chomp $line;
		($junk, $lat_expected, $lon_expected, $latlon_comment) =
		    ($line =~
		     /(\;|\#)\s+lat\,lon\s+\=\s*(\S+)\s+(\S+)\s*(.*)/);
		if (defined($lat_expected) && defined($lon_expected)) {
		    $got_latlon_expected = 1;
		    if (!defined($latlon_comment)) {
			$latlon_comment = "";
		    }
		    last;
		}
	    }
	    if (!$got_latlon_expected) {
		print STDERR ("$script: ERROR: $mppfile:\n" .
			      "Can't find expected lat,lon in $mppfile\n");
		last;
	    }
	    $tmpfile_string = "\n$x_in $y_in\n\n\n";
	    $target = "lat,lon \=";
	}

        #
        # Create a temporary file to hold the input to xytest
        #
	if (!open(TEMP, ">$tmpfile")) {
	    print STDERR ("$script: ERROR: $mppfile:\n" .
			  "Can't open $tmpfile for writing\n");
	    last;
	}
	print TEMP $tmpfile_string;
	close(TEMP);
	
	#
	# Run xytest and capture the screen output
	#
	my $command = "xytest $mppfile <$tmpfile 2>&1";
	my @xytest = `$command`;
	if (!defined(@xytest)) {
	    print STDERR ("$script: ERROR: $mppfile:\n" .
			  "Can't run $command\n");
	    last;
	}
	if (grep(/error/, @xytest)) {
	    print STDERR (@xytest);
	    print STDERR ("$script: ERROR: $mppfile:\n" .
			  "Error detected running $command\n");
	    last;
	}

	#
	# Find expected x,y or lat,lon
	#
	my @actual = grep(/$target/, @xytest);
	if (!defined(@actual)) {
	    print STDERR (@xytest);
	    print STDERR ("$script: ERROR: $mppfile:\n" .
			  "Can't find $target in output from $command\n");
	    last;
	}

	if ($got_forward) {

	    #
	    # Compare expected and actual x,y
	    #
	    my ($x_actual, $y_actual) =
		($actual[0] =~ /$target\s+(\S+)\s+(\S+)/);
	    if (!defined($x_actual) || !defined($y_actual)) {
		print STDERR (@xytest);
		print STDERR ("$script: ERROR: $mppfile:\n" .
			      "Can't parse $target in output from $command\n");
		last;
	    }
	    $forinv_ok = ($x_expected eq $x_actual and
		   $y_expected eq $y_actual);
	    if ($verbose || !$forinv_ok) {
		if (!$header_printed) {
		    &print_header($mppfile, @mpp_lines);
		    $header_printed = 1;
		}
		&print_header2;
		print STDERR ("  xytest forward results:\n");
		print STDERR ("    lat,lon in:       $lat_in $lon_in\n");
		print STDERR ("    x,y expected:     $x_expected $y_expected\n" .
			      "                  $xy_comment\n");
		print STDERR ("    x,y actual:       $x_actual $y_actual\n");
	    }

	} else {
	
	    #
	    # Compare expected and actual lat,lon
	    #
	    my ($lat_actual, $lon_actual) =
		($actual[0] =~ /$target\s+(\S+)\s+(\S+)/);
	    if (!defined($lat_actual) || !defined($lon_actual)) {
		print STDERR (@xytest);
		print STDERR ("$script: ERROR: $mppfile:\n" .
			      "Can't parse $target in output from $command\n");
		last;
	    }
	    $forinv_ok = ($lat_expected eq $lat_actual and
		   $lon_expected eq $lon_actual);
	    if ($verbose || !$forinv_ok) {
		if (!$header_printed) {
		    &print_header($mppfile, @mpp_lines);
		    $header_printed = 1;
		}
		&print_header2;
		print STDERR ("  xytest inverse results:\n");
		print STDERR ("    x,y in:           $x_in $y_in\n");
		print STDERR ("    lat,lon expected: $lat_expected $lon_expected\n" .
			      "                  $latlon_comment\n");
		print STDERR ("    lat,lon actual:   $lat_actual $lon_actual\n");
	    }
	}
	if (!$forinv_ok) {
	    print STDERR ("  Expected and actual results differ\n");
	}
	if ($verbose && $forinv_ok) {
	    print STDERR ("  Expected results match actual results\n");
	}
	system("rm -f $tmpfile");
	$got_one_forinv_result = 1;
    }
    if ($header_printed) {
    print STDERR
	("****************************************************************\n");
    }
    if (!$forinv_ok || !$macct_ok) {
	$exit_value = 1;
    }
}
exit($exit_value);

sub print_header {
    my ($mppfile, @mpp_lines) = @_;
    print STDERR
	("****************************************************************\n");
    print STDERR
	("mppfile: $mppfile\n");
    my $i;
    for ($i = 0; $i < scalar(@mpp_lines); $i++) {
	print STDERR "  $mpp_lines[$i]\n";
    }
}

sub print_header2 {
    my ($mppfile, @mpp_lines) = @_;
    print STDERR
	("  **************************************************************\n");
}
