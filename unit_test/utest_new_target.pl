#!/usr/bin/perl -w
#

#==============================================================================
# utest_new_target.pl - run utest.pl to create a new set of target gpd files
#
# 14-May-2003 Terry Haran, tharan@nsidc.org, 303-492-1847
# National Snow & Ice Data Center, University of Colorado, Boulder
#==============================================================================
#
# $Header: /tmp_mnt/FILES/mapx/unit_test/utest_new_target.pl,v 1.3 2003-05-16 23:46:35 haran Exp $
#

#
# make stderr and stdout unbuffered so nothing gets lost
#
$| = 1;
select(STDERR);
select(STDOUT);

use Getopt::Std;

$script = "UTEST_NEW_TARGET";
$script = $script;

$Usage = "\n
USAGE: utest_new_target.pl [-v] tagout

   input: tagout - Specifies the tagout string to be used with the -o option
                   to utest.pl.
                 
   options:
       -v - verbose
";
        
#
# check out the arguments
#
my $verbose = 0;
my %opts;

if (!getopts('vi:o:c', \%opts)) {
    print STDERR "Incorrect usage\n";
    die($Usage);
    exit 1;
}
if ($opts{v}) {
    $verbose = $opts{v};
}
if (@ARGV != 1) {
    die($Usage);
}

my $verbose_string = $verbose ? "-v" : "";
my $tagout = $ARGV[0];
my $exit_value = 0;
my @tagsin = ("other", "snyder", "tilecalc");

#
#  Run utest.pl for each tagin (subdirectory)
#
my $tagin;
foreach $tagin (@tagsin) {
    my $command =
	"./utest.pl $verbose_string -o $tagout -c $tagin/$tagin*";
    my $this_exit_value = system($command);
    if ($this_exit_value) {
	$exit_value = $this_exit_value;
    }
}

exit($exit_value);
