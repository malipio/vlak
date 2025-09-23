#!/usr/bin/perl -w
require "tests_base.pl";

my @BLOCKSIZES = (512,1024,1152,2048,4096,4608,5000,6000,8192,10000,16384,20000,32768);
sub do_blocksize_test {
	my ($infile,$datfile,$vlakopts) = @_;
	my $outfile = outdir($infile).'/'.outfile($infile).'.vlak.tmp';
	print STDERR "testing blocksize for $infile","\n";

	open BLKTST,'>'.outdir($infile).'/'.$datfile;
	print BLKTST '# opts: ',$vlakopts,"\n";
	foreach $blockSize (@BLOCKSIZES) {
		my $cmd_txt = "./vlak -b $blockSize $vlakopts \"$infile\" \"$outfile\"";
		my $sec = run_cmd($cmd_txt);
		my ($cr,$cp) = get_crcp($outfile,$infile);
		printf BLKTST "%d\t%d\t%.4f\t%.4f\n",$blockSize,$sec,$cr,$cp;
		unlink $outfile;
	}
	close BLKTST;
}

foreach $testfile (@TESTFILES) {
	do_blocksize_test $testfile, 'block_test_lpc.dat',"-p 1 -m 2 -l 8,8";
	do_blocksize_test $testfile, 'block_test_fir.dat',"-p 2 -m 2";
#	do_blocksize_test $testfile, 'block_test_wlet.dat',"-p 3 -m 2";
}
