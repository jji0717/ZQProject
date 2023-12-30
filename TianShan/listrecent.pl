#!/usr/bin/perl -w
use File::stat;
use lib '.';
use Getopt::Long;
use Win32;
use Cwd;
use env;
use File::Path;


&GetOptions(
                'help'		=>	\$ins_help,
		'path=s'		=>\$ins_path,
                'elapsed=s'	=> 	\$ins_elapsed,
               );

my $path;
my $elapsed;

if ($ins_help) {
    print <<EOF;
    Usage:
        list <options>
        list recently changed files in specified folder
    Options:
        --path
            specify the file path, no default value, must specified.
        --elapsed
            specify the elapsed time in second, default is 30s;
        --help
            display this screen
EOF
    exit 0;
}


if ($ins_path) {
    $path = $ins_path;
}
else
{
    print "please input the file path\n";
    exit 0;
}

if ($ins_elapsed) {
    $elapsed = $ins_elapsed;
}
else
{
    $elapsed = 30;
}


my $filename;
opendir(DIR, "$path") || die "specified $path is not existed";
@filename = readdir(DIR);
close DIR;
my $ntime=time();
my $in_count=0;
foreach $filename (@filename)
{
    $filename=trim($filename);
    $filename=$path."\\"."$filename";
    my @array = stat("$filename") or die("fail to open $filename");
    my $mtime = "$array[0][9]";
    my $diff=$ntime-$mtime;
    if ($diff < $elapsed) {
        print "$filename have been changed in $elapsed second\n";
        $in_count++;
    }
    
    
}

if ($in_count<1) {
    print "No file changed in $elapsed second at specified folder\n";
}



sub trim

{

my @out = @_;

   for (@out)

            {

            s/^\s+//;

      s/\s+$//;

      s/^\n+$//;

   }

   return wantarray ? @out : $out[0];
}