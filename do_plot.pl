#!/usr/bin/perl -w
require "./tests_base.pl";
require "./plot.pl";

if(scalar @ARGV != 0) {
	$cmd = shift @ARGV;
	eval $cmd.'@ARGV;';
	exit;
}

foreach $testfile (@TESTFILES) {
	print $testfile,"\n";
	my $dir = outdir($testfile).'/';
	# wykresy dla test_blocksize
	plot_cp_blocksize($dir.'block_test_fir.dat',$dir.'block_test_fir.png',
	"Wp造w rozmiaru bloku na efektywno嗆 kompresji dla predyktora FIR");
	
	plot_cp_blocksize($dir.'block_test_lpc.dat',$dir.'block_test_lpc.png',
	"Wp造w rozmiaru bloku na efektywno嗆 kompresji dla predyktora LPC");
	
	plot_cp_blocksize($dir.'block_test_wlet.dat',$dir.'block_test_wlet.png',
	"Wp造w rozmiaru bloku na efektywno嗆 kompresji dla predyktora Wavelet");
	
	# wykresy dla test_order
	plot_cp_order($dir.'orders.dat',$dir.'orders.png',
	'Wp造w statycznego rz璠u predyktora LPC na efektywno嗆 kompresji');

	plot_cp_order($dir.'max_orders.dat',$dir.'max_orders.png',
	'Wp造w maksymalnego rz璠u adaptacyjnego predyktora LPC na efektywno嗆 kompresji');
}
