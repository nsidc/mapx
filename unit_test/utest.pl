#!/usr/bin/perl -w
#

#==============================================================================
# mapx_utest.pl - run mapx unit test for a single mpp file
#
# 11-Apr-2003 Terry Haran, tharan@nsidc.org, 303-492-1847
# National Snow & Ice Data Center, University of Colorado, Boulder
#==============================================================================
#
# $Id: utest.pl,v 1.2 2003-04-11 23:31:11 haran Exp $
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
                   # lat,lon = <lat_in> <lon_in>
                   # x,y = <x_expected> <y_expected>
                   # lat,lon = <lat_expected> <lon_expected>
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
    while(1) {
    
        #
        # Find the next input lat,lon line
        #
	my ($lat_in, $lon_in);
	my $got_latlon_in = 0;
	while($i < scalar(@mpp)) {
	    my $line = $mpp[$i++];
	    chomp $line;
	    ($lat_in, $lon_in) =
		($line =~ /\#\s+lat\,lon\s+\=\s*(\S+)\s+(\S+)/);
	    if (defined($lat_in) && defined($lon_in)) {
		$got_latlon_in = 1;
		last;
	    }
	}
	if (!$got_latlon_in) {
	    if (!$got_one_result) {
		print STDERR ("$script: ERROR: $mppfile:\n" .
			      "Can't find input lat,lon in $mppfile\n");
	    }
	    last;
	}

        #
        # Find the next expected x,y line
        #
	my ($x_expected, $y_expected);
	my $got_xy_expected = 0;
	while($i < scalar(@mpp)) {
	    my $line = $mpp[$i++];
	    chomp $line;
	    ($x_expected, $y_expected) =
		($line =~ /\#\s+x\,y\s+\=\s*(\S+)\s+(\S+)/);
	    if (defined($x_expected) && defined($y_expected)) {
		$got_xy_expected = 1;
		last;
	    }
	}
	if (!$got_xy_expected) {
	    print STDERR ("$script: ERROR: $mppfile:\n" .
			  "Can't find expected x,y in $mppfile\n");
	    last;
	}
        
        #
        # Find the next expected lat,lon line
        #
	my ($lat_expected, $lon_expected);
	my $got_latlon_expected = 0;
	while($i < scalar(@mpp)) {
	    my $line = $mpp[$i++];
	    chomp $line;
	    ($lat_expected, $lon_expected) =
		($line =~ /\#\s+lat\,lon\s+\=\s*(\S+)\s+(\S+)/);
	    if (defined($lat_expected) && defined($lon_expected)) {
		$got_latlon_expected = 1;
		last;
	    }
	}
	if (!$got_latlon_expected) {
	    print STDERR ("$script: ERROR: $mppfile:\n" .
			  "Can't find expected lat,lon in $mppfile\n");
	    last;
	}

        #
        # Create a temporary file to hold the input to xytest
        #
	if (!open(TEMP, ">$tmpfile")) {
	    print STDERR ("$script: ERROR: $mppfile:\n" .
			  "Can't open $tmpfile for writing\n");
	    last;
	}
	print TEMP "$lat_in $lon_in\n\n\n\n";
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
	# Find expected x,y
	#
	my $target = "x,y =";
	my @xy_actual = grep(/$target/, @xytest);
	if (!defined(@xy_actual)) {
	    print STDERR (@xytest);
	    print STDERR ("$script: ERROR: $mppfile:\n" .
			  "Can't find $target in output from $command\n");
	    last;
	}
	my ($x_actual, $y_actual) =
	    ($xy_actual[0] =~ /$target\s+(\S+)\s+(\S+)/);
	if (!defined($x_actual) || !defined($y_actual)) {
	    print STDERR (@xytest);
	    print STDERR ("$script: ERROR: $mppfile:\n" .
		"Can't parse $target in output from $command\n");
	    last;
	}
	
	#
	# Find expected lat,lon
	#
	$target = "lat,lon =";
	my @latlon_actual = grep(/$target/, @xytest);
	if (!defined(@latlon_actual)) {
	    print STDERR (@xytest);
	    print STDERR ("$script: ERROR: $mppfile:\n" .
			  "Can't find $target in output from $command\n");
	    last;
	}
	my ($lat_actual, $lon_actual) =
	    ($latlon_actual[0] =~ /$target\s+(\S+)\s+(\S+)/);
	if (!defined($lat_actual) || !defined($lon_actual)) {
	    print STDERR (@xytest);
	    print STDERR ("$script: ERROR: $mppfile:\n" .
			  "Can't parse $target in output from $command\n");
	    last;
	}
	
	#
	# Compare expected and actual results
	#
	$ok = ($x_expected eq $x_actual and
	       $y_expected eq $y_actual and
	       $lat_expected eq $lat_actual and
	       $lon_expected eq $lon_actual);
	if ($verbose || !$ok) {
	    print STDERR ("**********************************************\n");
	    print STDERR ("$mppfile:\n");
	    print STDERR ("  lat,lon in:       $lat_in $lon_in\n");
	    print STDERR ("  x,y expected:     $x_expected $y_expected\n");
	    print STDERR ("  x,y actual:       $x_actual $y_actual\n");
	    print STDERR ("  lat,lon expected: $lat_expected $lon_expected\n");
	    print STDERR ("  lat,lon actual:   $lat_actual $lon_actual\n");
	}
	if (!$ok) {
	    print STDERR ("$script: ERROR: $mppfile:\n" .
			  "Expected and actual results differ\n");
	}
	if ($verbose && $ok) {
	    print STDERR ("$script: MESSAGE: $mppfile:\n" .
			  "Expected results match actual results\n");
	}
	system("rm -f $tmpfile");
	$got_one_result = 1;
    }
    if (!$ok) {
	$exit_value = 1;
    }
}
exit($exit_value);
