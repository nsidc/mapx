#!/usr/bin/perl -w
#

#==============================================================================
# utest.pl - run mapx unit test for one or more mpp files
#
# 11-Apr-2003 Terry Haran, tharan@nsidc.org, 303-492-1847
# National Snow & Ice Data Center, University of Colorado, Boulder
#==============================================================================
#
# $Header: /tmp_mnt/FILES/mapx/unit_test/utest.pl,v 1.20 2003-05-14 17:01:45 haran Exp $
#

#
# make stderr and stdout unbuffered so nothing gets lost
#
$| = 1;
select(STDERR);
select(STDOUT);

use Getopt::Std;

$script = "UTEST";
$script = $script;

$Usage = "\n
USAGE: utest.pl [-v] [-i tagin] [-o tagout] [-c] file1 [file2...filen]

   input:
       filen - Either a Map Projection Parameters file (mppfile) or a Grid
               Parameters Definition file (gpdfile):
               mppfile:
                 Can contain comments that specify input and expected values
                 for macct and xytest to test a particular map projection.
                 One test of macct to be performed must consist of a group
                 of six lines in mppfile of the form:
                   # macct
                   # <points> points,  <bad_points> bad points
                   # average error = <average_error> km
                   # std dev error = <std_dev_error> km
                   # maximum error = <max_error> km
                   # max error was at <max_err_lat> <max_err_lon>
                 Each test of xytest to be performed must consist of a group
                 of three lines in mppfile of the form:
                   # xytest forward
                   # lat,lon = <lat_in> <lon_in>
                   # x,y = <x_expected> <y_expected> <comment>
                 or
                   # xytest inverse
                   # x,y = <x_in> <y_in>
                   # lat,lon = <lat_expected> <lon_expected> <comment>
               gpdfile: 
                 Can contain the same comments as above for mppfile (if gpdfile
                 contains map information in the new mapx format) as well as
                 comments that specify input and expected values for gacct
                 and crtest to test a particular grid for a particular map
                 projection. One test of gacct to be performed must consist
                 of a group of six lines in gpdfile of the form:
                   # gacct
                   # <points> points,  <bad_points> bad points
                   # average error = <average_error> pixels
                   # std dev error = <std_dev_error> pixels
                   # maximum error = <max_error> pixels
                   # max error was at col: <c> row: <r> lat: <lat> lon: <lon>
                 Each test of crtest to be performed must consist of a group
                 of three lines in gpdfile of the form:
                   # crtest forward
                   # lat,lon = <lat_in> <lon_in>
                   # col,row = <col_expected> <row_expected> <comment>
                 or
                   # crtest inverse
                   # col,row = <col_in> <row_in>
                   # lat,lon = <lat_expected> <lon_expected> <comment>
                 
   options:
       -v - verbose

       -i tagin - Specifies a tagin string. The default value of tagin is
                  the leading characters in the input file up to but not
                  including the first slash (/) or underbar (_).
                  If -o is not specified, then the tagin value is not used.

       -o tagout - Specifies a tagout string that should be used in creating an
                   output filename. All occurrences of the tagin value in
                   each input filename are replaced with the tagout value in
                   each output filename. The expected values in each input
                   file will be replaced with computed actual values in the
                   corresponding output file. Also, all occurrences of the
                   tagin value in the input file will be replaced with the
                   tagout value in the output file. If the -o option is not
                   specified, then no output file will be produced.

       -c - When creating an output file, append the string: 
              .vs <exp1> <exp2> in <tagin>
                where
                  <exp1> and <exp2> are the expected values from the input file
            to any comment on any xytest or crtest expected line. If both
            <exp1> and <exp2> are equal to \"dummy\", then the comment is not
            appended to. If -o is not specified, then the -c option is ignored.
";
        
#
# check out the arguments
#
my $verbose = 0;
my $tagin = "";
my $got_tagin = 0;
my $tagout = "";
my $append_comment = 0;
my %opts;

if (!getopts('vi:o:c', \%opts)) {
    print STDERR "Incorrect usage\n";
    die($Usage);
    exit 1;
}
if ($opts{v}) {
    $verbose = $opts{v};
}
if ($opts{i}) {
    $tagin = $opts{i};
    $got_tagin = 1;
}
if ($opts{o}) {
    $tagout = $opts{o};
}
if ($opts{c}) {
    $append_comment = 1;
}
if (@ARGV < 1) {
    die($Usage);
}

my @files_in = @ARGV;
my $file_in;
my $tmpfile = "utest_temp.txt";
my $exit_value = 0;
my $i;

#
# Loop through each input file
#
foreach $file_in (@files_in) {

    my $test_ok = 1;
    my $got_result = 0;

    #
    # Read the entire input file
    #
    if (!open(FILE_IN, "$file_in")) {
	print STDERR ("$script: ERROR: $file_in:\n" .
		      "Can't open $file_in for reading\n");
	next;
    }
    my @lines_in = (<FILE_IN>);
    close(FILE_IN);

    #
    # If creating an output file, then 
    # generate the output filename and open the output file
    #
    my $file_out;
    if ($tagout) {

	#
	#  If no tagin value was specifed on the command line, then
	#  use the leading characters up to the first slash or underbar in the
	#  input filename for tagin.
	#
	if (!$got_tagin) {
	    ($tagin) = ($file_in =~ /^([^\/^_]*)(\/|_)/);
	}
	if (!defined($tagin)) {
	    $tagin = "";
	}
	if (!$tagin) {
	    print STDERR ("$script: ERROR: $file_in:\n" .
			  "Can't determine tagin\n");
	    next;
	}
	$file_out = $file_in;
	$file_out =~ s/$tagin/$tagout/g;

	#
	#  If fileout contains a leading directory,
	#  create the directory if it doesn't exist.
	#
	my ($dir_out) = ($file_out =~ /(.*)\/[^\/]*$/);
	if (defined($dir_out) && $dir_out ne "" && (!(-e $dir_out))) {
	    system("mkdir -p $dir_out");
	}
	if (!open(FILE_OUT, ">$file_out")) {
	    print STDERR ("$script: ERROR: $file_in:\n" .
			  "Can't open $file_out for writing\n");
	    next;
	}
    }

    my $header_printed = 0;
    $i = 0;
    my @lines_in_non_comment;
	
    while($i < scalar(@lines_in)) {
	
	#
	#  Process each input line.
	#  Save the non-comments in a separate array.
	#
	my $line_in = $lines_in[$i++];
	chomp $line_in;
	if (substr($line_in, 0, 1) ne ";" &&
	    substr($line_in, 0, 1) ne "#") {
	    push(@lines_in_non_comment, $line_in);
	}
	
	&write_output("FILE_OUT", $tagin, $tagout, $line_in);
	
	my $got_macct          = 0;
	my $got_gacct          = 0;
	my $got_xytest_forward = 0;
	my $got_xytest_inverse = 0;	
	my $got_crtest_forward = 0;
	my $got_crtest_inverse = 0;
	
	my $command;
	my @lines_act;
	
	#
	# Find the next macct, gacct, xytest, or crtest
	#
	$got_macct          = ($line_in =~ /(\;|\#)\s*macct/);
	$got_gacct          = ($line_in =~ /(\;|\#)\s*gacct/);
	$got_xytest_forward = ($line_in =~ /(\;|\#)\s*xytest\s+forward/);
	$got_xytest_inverse = ($line_in =~ /(\;|\#)\s*xytest\s+inverse/);
	$got_crtest_forward = ($line_in =~ /(\;|\#)\s*crtest\s+forward/);
	$got_crtest_inverse = ($line_in =~ /(\;|\#)\s*crtest\s+inverse/);

	#
	#  Assume that just "forward" or "inverse" means
	#  "xytest forward" or "xytest inverse"
	if (!$got_xytest_forward) {
	    $got_xytest_forward = ($line_in =~ /(\;|\#)\s*forward/);
	}
	if (!$got_xytest_inverse) {
	    $got_xytest_inverse = ($line_in =~ /(\;|\#)\s*inverse/);
	}
	
	if ($got_macct || $got_gacct) {
	    
	    #
	    # Compare macct or gacct expected and actual results
	    #
	    $command = $got_macct ? "macct" : "gacct";
	    my @lines_act = &get_command_output($command, $file_in, "");
	    my @lines_exp;
	    my $acct_ok = 0;
	    if (scalar(@lines_act) == 5) {
		my $j;
		my $got_bad_one = 0;
		for ($j = 0; $j < 5; $j++) {
		    
		    #
		    #  Write actual results to output file as necessary
		    #
		    if ($tagout) {
			print FILE_OUT "#   $lines_act[$j]\n";
		    }
		    if ($i < scalar(@lines_in)) {
			$line_in = $lines_in[$i++];
			chomp $line_in;
			$line_in =~ s/^(\;|\#)\s*//;
			$line_in =~ s/\s*$//;
			$lines_exp[$j] = $line_in;
			if ($lines_exp[$j] ne $lines_act[$j]) {
			    $got_bad_one = 1;
			}
		    } else {
			last;
		    }
		}
		$acct_ok = !$got_bad_one;
	    }
	    if ($verbose || !$acct_ok) {
		if (!$header_printed) {
		    &print_header($file_in, @lines_in_non_comment);
		    $header_printed = 1;
		}
		&print_header2;
		print STDERR ("  $command expected results:\n");
		my $j;
		for ($j = 0; $j < scalar(@lines_exp); $j++) {
		    print STDERR "    $lines_exp[$j]\n";
		}
		print STDERR ("  $command actual results:\n");
		for ($j = 0; $j < scalar(@lines_act); $j++) {
		    print STDERR "    $lines_act[$j]\n";
		}
		if (!$acct_ok) {
		    print STDERR ("  Expected and actual results differ\n");
		}
	    }
	    if ($verbose && $acct_ok) {
		print STDERR ("  Expected results match actual results\n");
	    }
	    $got_result = 1;
	    if (!$acct_ok) {
		$test_ok = 0;
	    }
	    next;
	}
	
	my $target_in;
	my $target_out;
	my $direction;
	my $blank_string;
	
	if ($got_xytest_forward) {
	    $command = "xytest";
	    $target_in = "lat\,lon";
	    $target_out = "x\,y";
	    $direction = "forward";
	    $blank_string = "                  ";
	} elsif ($got_xytest_inverse) {
	    $command = "xytest";
	    $target_in = "x\,y";
	    $target_out = "lat\,lon";
	    $direction = "inverse";
	    $blank_string = "                      ";
	} elsif ($got_crtest_forward) {
	    $command = "crtest";
	    $target_in = "lat\,lon";
	    $target_out = "col\,row";
	    $direction = "forward";
	    $blank_string = "                      ";
	} elsif ($got_crtest_inverse) {
	    $command = "crtest";
	    $target_in = "col\,row";
	    $target_out = "lat\,lon";
	    $direction = "inverse";
	    $blank_string = "                      ";
	} else {
	    next;
	}
	
	my $in1;
	my $in2;
	my $exp1;
	my $exp2;
	my $exp1_exp2;
	my $act1;
	my $act2;
	my $comment;
	my $tmpfile_string;
	my $junk;
	my $target;
	
	#
	# Find the next input line
	#
	my $got_in = 0;
	while($i < scalar(@lines_in)) {
	    $line_in = $lines_in[$i++];
	    chomp $line_in;
	    &write_output("FILE_OUT", $tagin, $tagout, $line_in);
	    ($junk, $in1, $in2) =
		($line_in =~ /(\;|\#)\s+$target_in\s+\=\s*(\S+)\s+(\S+)/);
	    if (defined($in1) && defined($in2)) {
		$got_in = 1;
		last;
	    }
	}
	if (!$got_in) {
	    print STDERR ("$script: ERROR: $file_in:\n" .
			  "Can't find input $target_in in $file_in\n");
	    last;
	}
	
	#
	# Find the next expected line
	#
	my $got_expected = 0;
	while($i < scalar(@lines_in)) {
	    $line_in = $lines_in[$i++];
	    chomp $line_in;
	    ($junk, $exp1, $exp2, $comment) =
		($line_in =~ /(\;|\#)\s+$target_out\s+\=\s*(\S+)\s+(\S+)\s*(.*)/);
	    if (defined($exp1) && defined($exp2)) {
		$got_expected = 1;
		if (!defined($comment)) {
		    $comment = "";
		}
		($junk, $exp1_exp2) =
		    ($line_in =~ /(\;|\#)\s+$target_out\s+\=\s*(\S+\s+\S+)/);
		if (!defined($exp1_exp2)) {
		    $exp1_exp2 = "$exp1 $exp2";
		}
		last;
	    }
	}
	if (!$got_expected) {
	    print STDERR ("$script: ERROR: $file_in:\n" .
			  "Can't find expected $target_out in $file_in\n");
	    last;
	}
	if ($direction eq "forward") {
	    $tmpfile_string = "$in1 $in2\n\n\n\n";
	} else {
	    $tmpfile_string = "\n$in1 $in2\n\n\n";
	}
	$target = "$target_out \=";
	
	
	#
	# Create a temporary file to hold the input to xytest or crtest
	#
	if (!open(TEMP, ">$tmpfile")) {
	    print STDERR ("$script: ERROR: $file_in:\n" .
			  "Can't open $tmpfile for writing\n");
	    last;
	}
	print TEMP $tmpfile_string;
	close(TEMP);
	
	#
	# Run xytest or crtest and capture the screen output
	#
	@lines_act = &get_command_output($command, $file_in, "<$tmpfile");
	if (!@lines_act) {
	    last;
	}
	if (grep(/error/, @lines_act)) {
	    print STDERR (@lines_act);
	    print STDERR ("$script: ERROR: $file_in:\n" .
			  "Error detected running $command\n");
	    last;
	}
	
	#
	# Find the actual values
	#
	my @actual = grep(/$target/, @lines_act);
	if (!@actual) {
	    print STDERR (@lines_act);
	    print STDERR ("$script: ERROR: $file_in:\n" .
			  "Can't find $target in output from $command\n");
	    last;
	}

	#
	#  Get actual results
	#
	($act1, $act2) = ($actual[0] =~ /$target\s+(\S+)\s+(\S+)/);
	if (!defined($act1) || !defined($act2)) {
	    print STDERR (@lines_act);
	    print STDERR ("$script: ERROR: $file_in:\n" .
			  "Can't parse $target in output from $command\n");
	    last;
	}
	
	#
	#  Compare expected and actual values
	#
	my $forinv_ok = ($exp1 eq $act1 and $exp2 eq $act2);
	if ($verbose || !$forinv_ok) {
	    if (!$header_printed) {
		&print_header($file_in, @lines_in_non_comment);
		$header_printed = 1;
	    }
	    &print_header2;
	    print STDERR ("  $command $direction results:\n");
	    print STDERR ("    $target_in in:       $in1 $in2\n");
	    print STDERR ("    $target_out expected:     $exp1_exp2\n" .
			  "$blank_string$comment\n");
	    print STDERR ("    $target_out actual:       $act1 $act2\n");
	}
	if (!$forinv_ok) {
	    print STDERR ("  Expected and actual results differ\n");
	}
	if ($verbose && $forinv_ok) {
	    print STDERR ("  Expected results match actual results\n");
	}
	system("rm -f $tmpfile");

	#
	#  Write actual results to output file as necessary
	#
	if ($tagout) {

	    #
	    #  Append to comment as necessary
	    #
	    if ($append_comment && $exp1 ne "dummy" && $exp2 ne "dummy") {
		my $extra_blank = $comment ? "" : " ";
		$comment .= $extra_blank . ".vs $exp1_exp2 in $tagin";
	    }
	    print FILE_OUT "#   $target_out = $act1 $act2 $comment\n";
	}

	$got_result = 1;
	if (!$forinv_ok) {
	    $test_ok = 0;
	}
    }
    if (!$got_result) {
	if (!$header_printed) {
	    &print_header($file_in, @lines_in_non_comment);
	    $header_printed = 1;
	}
	print STDERR ("$script: ERROR: $file_in:\n" .
		      "No test specified");
    }
    if ($header_printed) {
	print STDERR
	    ("****************************************************************\n");
    }
    if (!$test_ok || !$got_result) {
	$exit_value = 1;
    }
}
exit($exit_value);

sub print_header {
    my ($file_in, @lines) = @_;
    print STDERR
	("****************************************************************\n");
    print STDERR
	("file: $file_in\n");
    my $i;
    for ($i = 0; $i < scalar(@lines); $i++) {
	print STDERR "  $lines[$i]\n";
    }
}

sub print_header2 {
    print STDERR
	("  **************************************************************\n");
}

sub get_command_output {
    my ($command, $file_in, $input_file) = @_;
    my $command_line = "../$command $file_in $input_file 2>&1";
    my @command_output = `$command_line`;
    if (!@command_output) {
	print STDERR ("$script: ERROR: $file_in:\n" .
		      "Can't run $command_line\n");
    } else {
	my $i;
	for ($i = 0; $i < scalar(@command_output); $i++) {
	    chomp $command_output[$i];
	    $command_output[$i] =~ s/\s*$//;
	}
    }
    return(@command_output);
}

sub write_output {

    my ($handle, $tagin, $tagout, $line_in) = @_;
    #
    #  Write an output line as necessary, substituting
    #  tagin with tagout.
    #
    if ($tagout) {
	my $line_out = $line_in;
	$line_out =~ s/$tagin/$tagout/g;
	print $handle "$line_out\n";
    }
}
