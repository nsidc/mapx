#!/usr/local/bin/perl
#
# Generates lat/lon arrays for input list of gpd files
#
# 2011-10-05 M. J. Brodzik brodzik@nsidc.org 303-492-8263
# National Snow & Ice Data Center, University of Colorado, Boulder
# Copyright 2011 Regents of the University of Colorado
#
#-----------------------------------------------------------------------
use Env;

my $usage = '

do_gridloc.pl [gpds]

Input:
     [gpds] - list of gpds to generate gridloc output, for e.g. "EASE2_N25km",
              (do not include the .gpd extension)

Options: n/a

Output:
     A pair of flat-binary, double-precision (lat/lon) files for each input gpd,
     in the current directory

gridloc assumes that environment variable $PATHMPP is set to the path(s) where
.gpd/.mpp files are stored on this system

';

if ( -1 == $#ARGV ) {
    warn $usage;
    exit 0;
}

my $gridloc_cmd = $ENV{HOME} . '/bin/gridloc -D ';

my $gpd;
my $cmd;
my @cmd_out;
my $msg;

for ( @ARGV ) {

    chomp;
    $gpd = $_;
    printf STDERR "Next gpd is: $gpd\n";

    $cmd = "$gridloc_cmd -p -o ${gpd}.lats ${gpd}.gpd";
    @cmd_out = `$cmd 2>&1`;
    if ( 0 != $? ) {
	printf STDERR "Error for $gpd latitudes\n";
	printf STDERR "Output from: $cmd\n";
	printf STDERR "@cmd_out";
	printf STDERR "End of output.\n";
	next;
    }

    $cmd = "$gridloc_cmd -m -o ${gpd}.lons ${gpd}.gpd";
    @cmd_out = `$cmd 2>&1`;
    if ( 0 != $? ) {
	printf STDERR "Error for $gpd longitudes\n";
	printf STDERR "Output from: $cmd\n";
	printf STDERR "@cmd_out";
	printf STDERR "End of output.\n";
	next;
    }

}

1;


