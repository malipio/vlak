#!/usr/bin/perl -w
# podstawowe funkcje do obslugi testowania
use File::Basename;

$TESTDIR = 'tests/';
@TESTFILES = ($TESTDIR.'baranek/baranek.wav',
	$TESTDIR.'Gladiator/GladiatorOST-NowWeAreFree.wav',
	$TESTDIR.'Izrael/Izrael-SeeI&I.wav',
	$TESTDIR.'LedZeppelin/LedZeppelin-D\'yerMak\'er.wav',
	$TESTDIR.'myslo-live/myslo.wav',
	$TESTDIR.'SweetNoise/SweetNoise-9 1.wav',
	$TESTDIR.'Vypsana/vf_palenie_titonia.wav',
	$TESTDIR.'turbo/turbo.wav',
	$TESTDIR.'ironmaiden/sotc.wav');
@PREDICTORS = (1,2,3);
@MANIPULATORS = (0,2);

# aktualne opcje
$PRED = $PREDICTORS[0];
$MANIP = $MANIPULATORS[1];

sub get_vlak_opts {
	return "-p $PRED -m $MANIP";
}

sub get_crcp {
	my ($vlakfile,$wavfile) = @_;
	my $CR = (-s $wavfile)/(-s $vlakfile);
	my $CP = (1- 1/$CR)*100;
	return ($CR,$CP);
}

sub run_cmd {
	my $start = (times)[2]; #time;
	my $retval = system @_;
	die "non zero exit status of executed command" if $retval != 0;
	my $stop = (times)[2];
#	my ($sec,$min,$nic) = localtime($stop-$start);
	my ($sec,$min) =($stop-$start,0);
	return $sec+60*$min;
}


sub outfile {
	my ($i) = @_;
	return basename($i,'.wav');
}

sub outdir {
	my ($i) = @_;
	return dirname($i);
}
# konczymy zwroceniem prawdy
1;
