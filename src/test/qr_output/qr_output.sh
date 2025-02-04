#!/bin/bash
# generates the required test data for qr_output.c
# requires qrencode, imagemagick, and perl

set -e -u -o pipefail
cd -- "$(dirname -- "${BASH_SOURCE[0]}")"

MAGICK=magick
if ! type "$MAGICK" &>/dev/null; then MAGICK=convert; fi
if ! type "$MAGICK" &>/dev/null; then
	echo "imagemagick not found" >&2
	exit 1
fi

PERL=perl
if ! type "$PERL" &>/dev/null; then
	echo "perl not found" >&2
	exit 1
fi

QRENCODE=qrencode
if ! type "$QRENCODE" &>/dev/null; then
	echo "qrencode not found" >&2
	exit 1
fi

bin_file=qr_output.bin
if [[ -e "$bin_file" ]]; then
	rm "$bin_file" # remove the old file
fi
for i in {1..40}; do
	"$QRENCODE" -s 1 -m 0 -o - "test string..." -l L -v "$i" | "$MAGICK" - txt:- | "$PERL" qr_output.pl "$bin_file"
done
