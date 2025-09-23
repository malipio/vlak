#!/usr/bin/perl
# skrypt wypisujacy ile trwalo wykonanie danego polecenia
use POSIX;
require "tests_base.pl";
my $start = (times)[2];
$sec = run_cmd(@ARGV); # zakladamy ze sie nie przekrceci
my $stop = (times)[2];
sysconf
printf "%d\tclock= %d\n",$sec,($stop-$start);#/CLK_TCK;
print sysconf(_SC_CLK_TCK),"\n";
