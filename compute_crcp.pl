#!/usr/bin/perl
# skrypcik obliczajacy Compression Ratio oraz Compression Percentage
# args: plik.vlak orgplik.wav
die "$0 plik.vlak orgplik.wav\n" if(scalar @ARGV != 2);
require "tests_base.pl";
$vlakfile = $ARGV[0];
$wavfile = $ARGV[1];
printf "CR= %.3f\nCP= %.2f\%\n",get_crcp($vlakfile,$wavfile);

