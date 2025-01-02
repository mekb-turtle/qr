#!/bin/bash
# generates the required test data for qr_output.c
# requires the qrcode package from npm
# requires imagemagick, bc, nodejs

set -e -u -o pipefail
cd -- "$(dirname -- "${BASH_SOURCE[0]}")"
bin_file=qr_output.bin
if [[ -e "$bin_file" ]]; then
	rm "$bin_file" # remove the old file
fi
for i in {1..40}; do
	qrcode -s 1 -q 0 -o /dev/stdout "test string..." -e L -v "$i" | magick - txt:- | perl qr_output.pl "$bin_file"
done
