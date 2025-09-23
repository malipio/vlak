#!/usr/bin/perl -w
# (c) Piotr Malinowski
# taki glupi skrypcik do sprawdzania zaangazowania :P

my %excluded_files = ( "architektura.xmi" => 1, "riff.txt" => 1, "x.wav" => 1 );
my %accounting;
my %acc_per_file;

sub count_lines {
	my ($filename) = @_;
	my $tmp = $/;
	my $cnt = 0;
	open(SRC,"<$filename") || return 0;
	$/ = "\n";
	while(<SRC>) { $cnt++; }
	close(SRC);
	$/ = $tmp;
	return $cnt;
}

sub read_log {
	my ($log) = @_;
	if(!($log =~ /^Working file:\s*([^\s]*)$/m)) { return; }
	my $filename = $1;
	my @ln;
	my %files_to_check; # author => file
	my $rev;
#	print "[$filename]","\n";
	if( exists $excluded_files{$filename}) { print "EXCLUDED: $filename\n"; return; } # wylaczamy to z liczenia
	$acc_per_file{$filename} = 0; # init
	@ln = split(/\n/,$log);
	foreach $line (@ln) {
		if($line =~ /^revision ([\d\.]+)/) {
			#print "rev: $1\n";
			$rev = $1;
		}
		if($line =~ /^date:([^\n]*)$/) {
	  		#print $1,"\n";
			my $author;
			my $lcnt;
			if($line =~ /author:\s([^;]+)/)
			{
				$author = $1;
				#print $author,"\n";
				if($line =~ /lines:\s*(\+\d+)\s*(\-\d+)/)
				{
					$lcnt = $1+$2;
					#print $lcnt,"\n";
					$accounting{$author} += $lcnt;
					$acc_per_file{$filename} += $lcnt;
				} else {
				# taka sytuacja powinna zajsc tylko raz dla kazdego z plikow
				#	print "AUTOR: $author / PLIK: $filename / REV: $rev","\n";
					if(not($rev eq "1.1")) { die "not initial revision!" };
					$files_to_check{$author} = $filename;
				}
			}			
		}
	}

	# robimy korekte o initial revisions ktore nie sa uwzgledniane (bo nie ma lines: + -)
	foreach $k (keys %files_to_check) {
		# aktualna ilosc linii
		my $cnt_lines = count_lines($files_to_check{$k});
		# wiemy ile linii dopisano od czasu initial revision (%acc_per_file)
		# wiemy ile jest teraz linii w pliku, obliczamy roznice
		# co nam daje ilosc linii jaka powinna byc na poczatku!
		my $init_lines = $cnt_lines - $acc_per_file{$files_to_check{$k}};
#		print "KOREKTA: ",$files_to_check{$k}," author: ",
#			$k," lines: +",$init_lines,"\n";
		$accounting{$k} += $init_lines;
		# dla porzadku...
		$acc_per_file{$files_to_check{$k}} += $init_lines;
	}
	
}

$/="=============================================================================";
while(<STDIN>) {
	chomp;
	read_log $_;
}

print "SUMMARY:\n";
foreach $k (keys %accounting) {
	print $k," = ",$accounting{$k},"\n";
}

#print "SUMMARY PER FILE:\n";
#foreach $k (keys %acc_per_file) {
#	print $k," = ",$acc_per_file{$k},"\n";
#}
