#!/usr/bin/perl -w
require "tests_base.pl";
# using MANIPULATORS from tests_base

sub do_manip_test {
	my ($infile,$datfile,$vlakopts) = @_;
	my $outfile = outdir($infile).'/'.outfile($infile).'.vlak.tmp';
	print STDERR "testing manipulators for $infile","\n";

	open MTST,'>'.outdir($infile).'/'.$datfile;
	print MTST '# opts: ',$vlakopts,"\n";
	foreach $manip (@MANIPULATORS) {
		my $cmd_txt = "./vlak -m $manip $vlakopts \"$infile\" \"$outfile\"";
		my $sec = run_cmd($cmd_txt);
		my ($cr,$cp) = get_crcp($outfile,$infile);
		printf MTST "%d\t%d\t%.4f\t%.4f\n",$manip,$sec,$cr,$cp;
		unlink $outfile;
	}
	close MTST;
}

foreach $testfile (@TESTFILES) {
	do_manip_test $testfile, 'manip_test.dat',"-p 1 -l 8,8 -b 4608";
	do_manip_test $testfile, 'manip_test_fir.dat',"-p 2 -b 4608";
}
