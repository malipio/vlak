#!/usr/bin/perl
# test LPC - najlepszy rzad predyktora
require "tests_base.pl";
my @ORDERS = (2,3,4,5,6,8,10,12,15,20,32);
sub do_order_test {
	my ($infile,$vlakopts) = @_;
	my $outfile = outdir($infile).'/'.outfile($infile).'.vlak.tmp';
	print STDERR "testing best lpc order for $infile\n";
	open LPCORD,">".outdir($infile).'/'.'orders.dat';
	print LPCORD '# ',"opts: -p 1 -m $MANIP -b 4608\n";
	foreach $order (@ORDERS) {
		my $sec = run_cmd(
		"./vlak -p 1 -m $MANIP -b 4608 -l $order,$order \"$infile\" \"$outfile\"");
		my ($cr,$cp) = get_crcp($outfile,$infile);
		printf LPCORD "%d\t%d\t%.4f\t%.4f\n",$order,$sec,$cr,$cp;
		unlink $outfile;
	}
	close LPCORD;
}

sub do_adaptive_order_test {
	my ($infile,$vlakopts) = @_;
	my $outfile = outdir($infile).'/'.outfile($infile).'.vlak.tmp';
	print STDERR "testing best adaptive lpc order for $infile\n";
	open LPCORD,">".outdir($infile).'/'.'max_orders.dat';
	print LPCORD '# ',"opts: -p 1 -m $MANIP -b 4608\n";
	foreach $order (@ORDERS) {
		my $sec = run_cmd(
		"./vlak -p 1 -m $MANIP -b 4608 -l 1,$order \"$infile\" \"$outfile\"");
		my ($cr,$cp) = get_crcp($outfile,$infile);
		printf LPCORD "%d\t%d\t%.4f\t%.4f\n",$order,$sec,$cr,$cp;
		unlink $outfile;
	}
	close LPCORD;
}

foreach $testfile (@TESTFILES) {
	do_order_test $testfile,get_vlak_opts();
	do_adaptive_order_test $testfile,get_vlak_opts();
}
