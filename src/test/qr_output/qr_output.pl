use strict;
use warnings;
use autodie;

use POSIX qw(ceil);

if (@ARGV != 1) {
    die "Usage: $0 <output_file>\n";
}

my $output_file = $ARGV[0];

my @bit_array;
my $img_size;

# read input lines from magick
while (<STDIN>) {
    chomp;
    if (/^# ImageMagick pixel enumeration: (\d+),(\d+),/) {
		# extract size of image
        print "Size: $1x$2\n";
		if ($1 != $1) {
			die "Error: Image is not square\n";
		}
		$img_size = $1;
		next;
    }
    if (/\bwhite\b/i) {
        push @bit_array, 0;
    } elsif (/\bblack\b/i) {
        push @bit_array, 1;
    } else {
        die "Error: Line does not contain 'white' or 'black': $_\n";
    }
}

# convert the bit array to a binary string
my $binary_data = '';
while (@bit_array) {
    my $byte = 0;
    for my $bit (0..7) {
        $byte = ($byte >> 1) | ((shift @bit_array // 0) << 7);
    }
    $binary_data .= pack('C', $byte);
}

# confirm that the binary data is the correct size
if (length($binary_data) != ceil(($img_size * $img_size) / 8)) {
	die "Error: Binary data is not the correct size\n";
}

# write the image size, space, and binary data to a file
open my $fh, '>>', $output_file;
print $fh $img_size, " ", $binary_data;
close $fh;

print "Binary data written to $output_file\n";

