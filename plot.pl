#!/usr/bin/perl -w
#tworzy plik png z wykresami

sub plot_cp_blocksize {
open GNUPLOT,"| gnuplot" || die "can't open gnuplot";
	my ($datfile,$pngfile,$title) = @_;
	print GNUPLOT <<KONIEC;
	set terminal png
	set output "$pngfile"
	unset key
	set title "$title"
	set xlabel "rozmiar bloku"
	set ylabel "CP"
	plot "$datfile" using 1:4 with linespoints
KONIEC
close GNUPLOT;
}

sub plot_predictor_order {
open GNUPLOT,"| gnuplot" || die "can't open gnuplot";
	my ($datfile,$pngfile) = @_;
	print GNUPLOT <<KONIEC;
	set terminal png
	set output "$pngfile"
	unset key
	set title "wykres rzêdu predyktora LPC"
	set xlabel "numer ramki"
	set ylabel "rz±d predyktora"
	plot "$datfile" using :1 with impulses
KONIEC
close GNUPLOT;
}

sub plot_cp_order {
open GNUPLOT,"| gnuplot" || die "can't open gnuplot";
	my ($datfile,$pngfile,$title) = @_;
	print GNUPLOT <<KONIEC;
	set terminal png
	set output "$pngfile"
	unset key
	set title "$title"
	set xlabel "rz±d predyktora"
	set ylabel "CP"
	plot "$datfile" using 1:4 with linespoints
KONIEC
close GNUPLOT;
}

sub plot_residual_distrib {
open GNUPLOT,"| gnuplot" || die "can't open gnuplot";
	my ($datfile,$pngfile) = @_;
	print GNUPLOT <<KONIEC;
	set terminal png
	set output "$pngfile"
	unset key
	set title "rozk³ad sygna³u rezydualnego"
	set xlabel "warto¶æ sygna³u"
	set ylabel "ilo¶æ wyst±pieñ"
	plot "$datfile" using 1:2 with points
KONIEC
close GNUPLOT;
}

sub plot_signal {
open GNUPLOT,"| gnuplot" || die "can't open gnuplot";
	my ($orgdat,$pngfile,$title) = @_;
	print GNUPLOT <<KONIEC;
	set terminal png
	set output "$pngfile"
	unset key
	set title "$title"
	set xlabel "numer ramki"
	set ylabel "warto¶æ sygna³u"
	plot "$orgdat" using 1:2 with lines
KONIEC
close GNUPLOT;
}

sub plot_org_pred_signal {
open GNUPLOT,"| gnuplot" || die "can't open gnuplot";
	my ($orgdat,$preddat,$pngfile) = @_;
	print GNUPLOT <<KONIEC;
	set terminal png
	set output "$pngfile"
	set key below
	set title "Sygna³ oryginalny oraz na wyj¶ciu predyktora"
	set xlabel "numer próbki"
	set ylabel "warto¶æ sygna³u"
	plot "$orgdat" using 1:2 title "oryginalny" with lines, "$preddat" using 1:2 with lines title "po predykcji"
KONIEC
close GNUPLOT;
}

sub plot_org_manip_signal {
open GNUPLOT,"| gnuplot" || die "can't open gnuplot";
	my ($orgdat,$manipdat,$pngfile) = @_;
	print GNUPLOT <<KONIEC;
	set terminal png
	set output "$pngfile"
	set key below
	set title "Sygna³ oryginalny oraz po dekorelacji miêdzykana³owej"
	set xlabel "numer próbki"
	set ylabel "warto¶æ sygna³u"
	plot "$orgdat" using 1:2 title "oryginalny" with lines, "$manipdat" using 1:2 with lines title "po manipulacji"
KONIEC
close GNUPLOT;
}

#plot_blocksize @ARGV;
#plot_org_pred_signal @ARGV;
1;
