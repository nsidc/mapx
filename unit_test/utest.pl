#!/usr/bin/perl -w
#

#==============================================================================
# mapx_utest.pl - run mapx unit test for a single mpp file
#
# 11-Apr-2003 Terry Haran, tharan@nsidc.org, 303-492-1847
# National Snow & Ice Data Center, University of Colorado, Boulder
#==============================================================================
#
# $Id: utest.pl,v 1.7 2003-04-17 20:20:04 haran Exp $
#

#
# make stderr and stdout unbuffered so nothing gets lost
#
$| = 1;
select(STDERR);
select(STDOUT);

$script = "MAPX_UTEST";
$script = $script;

$Usage = "\n
USAGE: mapx_utest.pl [-v] mppfile1 [mppfile2...]

       mppfile - Map Projection Parameters file to be used as input to
                 xytest to test a particular map projection.
                 Each test to be performed must consist of a group
                 of three lines in mppfile of the form:
                   ; forward
                   ; lat,lon = <lat_in> <lon_in>
                   ; x,y = <x_expected> <y_expected>
                 or
                   ; inverse
                   ; x,y = <x_in> <y_in>
                   ; lat,lon = <lat_expected> <lon_expected>
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
my $tmpfile = "mapx_utest_temp.txt";
my $exit_value = 0;

#
# Loop through each mpp file
#
foreach $mppfile (@mppfiles) {

    my $ok = 1;

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
    
    my $got_one_result = 0;
    my $i = 0;
    my @mpp_lines;
    while(1) {
	
	#
	# Find the next forward or inverse line
	#
	my $got_forward = 0;
	my $got_inverse = 0;
	while($i < scalar(@mpp)) {
	    my $line = $mpp[$i++];
	    if (substr($line, 0, 1) ne ";" && substr($line, 0, 1) ne "#") {
		push(@mpp_lines, $line);
	    }
	    chomp $line;
	    $got_forward = ($line =~ /(\;|\#)\s+forward/);
	    $got_inverse = ($line =~ /(\;|\#)\s+inverse/);
	    if ($got_forward || $got_inverse) {
		last;
	    }
	}
	if (!$got_forward && !$got_inverse) {
	    if (!$got_one_result) {
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
	    $ok = ($x_expected eq $x_actual and
		   $y_expected eq $y_actual);
	    if ($verbose || !$ok) {
		print STDERR ("**********************************************\n");
		print STDERR ("$mppfile forward:\n");
		print STDERR @mpp_lines;
		print STDERR ("  lat,lon in:       $lat_in $lon_in\n");
		print STDERR ("  x,y expected:     $x_expected $y_expected" .
			      " $xy_comment\n");
		print STDERR ("  x,y actual:       $x_actual $y_actual\n");
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
	    $ok = ($lat_expected eq $lat_actual and
		   $lon_expected eq $lon_actual);
	    if ($verbose || !$ok) {
		print STDERR ("**********************************************\n");
		print STDERR ("$mppfile inverse:\n");
		print STDERR @mpp_lines;
		print STDERR ("  x,y in:           $x_in $y_in\n");
		print STDERR ("  lat,lon expected: $lat_expected $lon_expected".
			      " $latlon_comment\n");
		print STDERR ("  lat,lon actual:   $lat_actual $lon_actual\n");
	    }
	}
	if (!$ok) {
	    print STDERR ("  Expected and actual results differ\n");
	}
	if ($verbose && $ok) {
	    print STDERR ("  Expected results match actual results\n");
	}
	system("rm -f $tmpfile");
	$got_one_result = 1;
    }
    if (!$ok) {
	$exit_value = 1;
    }
}
exit($exit_value);
