#!/usr/bin/perl -w
require "tests_base.pl";

sub do_crct_test {
	my ($infile,$vlakopts) = @_;
	my $outfile = outdir($infile).'/'.outfile($infile).'.tmp';
	print STDERR "testing compression ratio, time for $infile","\n";

	open CRCT,'>'.outdir($infile).'/'.'crct_test.dat';

	#VLAK
	for $pred (@PREDICTORS) {
		my $bsize = ($pred == 3) ? 4096 : 4608;
		print CRCT '# ',"vlak -p $pred -m $MANIP -b $bsize\n";
		my $sec = run_cmd(
			"./vlak -p $pred -m $MANIP -b $bsize \"$infile\" \"$outfile\"");
		my ($cr,$cp) = get_crcp($outfile,$infile);
		my $dec_sec = run_cmd("./vlak -d \"$outfile\" /dev/null");
		printf CRCT "%d\t%.4f\t%.4f\t%d\n",$sec,$cr,$cp,$dec_sec;
		unlink $outfile;
	}

	#flac
	foreach $preset ("-0","-5","-8") {
		print CRCT '# ',"flac $preset","\n";
		my $sec = run_cmd(
			"flac $preset -o \"$outfile\" \"$infile\"");
		my ($cr,$cp) = get_crcp($outfile,$infile);
		my $dec_sec = run_cmd(
			"flac -d -f -o /dev/null \"$outfile\"");
			
		printf CRCT "%d\t%.4f\t%.4f\t%d\n",$sec,$cr,$cp,$dec_sec;
		unlink $outfile;
	}

	#shorten
	foreach $settings ("","-b 4608 -p 8") {
		print CRCT '# ',"shorten $settings","\n";
		my $sec = run_cmd(
			"shorten $settings \"$infile\" \"$outfile\"");
		my ($cr,$cp) = get_crcp($outfile,$infile);
		my $dec_sec = run_cmd(
			"shorten -x \"$outfile\" /dev/null");
		printf CRCT "%d\t%.4f\t%.4f\t%d\n",$sec,$cr,$cp,$dec_sec;
		unlink $outfile;
	}

	#monkeys audio
	foreach $preset ("-c2000","-c3000") {
		print CRCT '# ',"mac $preset","\n";
		my $sec = run_cmd(
			"mac \"$infile\" \"$outfile.ape\" $preset");;
		my ($cr,$cp) = get_crcp($outfile.'.ape',$infile);
		my $dec_sec = run_cmd(
			"mac \"$outfile.ape\" /dev/null -d");
		printf CRCT "%d\t%.4f\t%.4f\t%d\n",$sec,$cr,$cp,$dec_sec;
		unlink $outfile.'.ape';
	}

	#gzip&bzip2
	foreach $compr ('gzip','bzip2') {
		print CRCT '# ',"$compr","\n";
		my $sec = run_cmd("$compr -c \"$infile\" > \"$outfile\"");
		my ($cr,$cp) = get_crcp($outfile,$infile);
		my $dec_sec = run_cmd("$compr -c -d \"$outfile\" > /dev/null");
		printf CRCT "%d\t%.4f\t%.4f\t%d\n",$sec,$cr,$cp,$dec_sec;
		unlink $outfile;
	}
	
	close CRCT;
}

foreach $testfile (@TESTFILES) {
	do_crct_test $testfile, get_vlak_opts();
}
