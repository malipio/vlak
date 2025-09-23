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
	"Wp�yw rozmiaru bloku na efektywno�� kompresji dla predyktora FIR");
	
	plot_cp_blocksize($dir.'block_test_lpc.dat',$dir.'block_test_lpc.png',
	"Wp�yw rozmiaru bloku na efektywno�� kompresji dla predyktora LPC");
	
	plot_cp_blocksize($dir.'block_test_wlet.dat',$dir.'block_test_wlet.png',
	"Wp�yw rozmiaru bloku na efektywno�� kompresji dla predyktora Wavelet");
	
	# wykresy dla test_order
	plot_cp_order($dir.'orders.dat',$dir.'orders.png',
	'Wp�yw statycznego rz�du predyktora LPC na efektywno�� kompresji');

	plot_cp_order($dir.'max_orders.dat',$dir.'max_orders.png',
	'Wp�yw maksymalnego rz�du adaptacyjnego predyktora LPC na efektywno�� kompresji');
}
