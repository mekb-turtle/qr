<div align="center">
    <h3 align="center">QR code generator</h3>
    <img alt="Version" src="https://img.shields.io/github/v/release/mekb-turtle/qr?style=flat&logoColor=f5c2e7&labelColor=1e1e2e&color=f5c2e7" />
    <img alt="Stars" src="https://img.shields.io/github/stars/mekb-turtle/qr?style=flat&logoColor=f5c2e7&labelColor=1e1e2e&color=f5c2e7" />
    <img alt="License" src="https://img.shields.io/github/license/mekb-turtle/qr?style=flat&logoColor=f5c2e7&labelColor=1e1e2e&color=f5c2e7" />
</div>

---
<br/>

<img alt="Preview" src="assets/preview.png"/>

## About The Project
This is a simple QR code generator written in C. It reads a string from arguments and outputs an image or text.

## Features
- Mode detection (numeric, alphanumeric, binary, kanji)
- Auto version selection
- Custom fg/bg color
- Supported output formats:
  - Image: PNG, BMP, Targa, HDR, JPEG, [farbfeld](https://tools.suckless.org/farbfeld/)
  - Text (using Unicode block characters)
  - HTML

## Future plans
- Micro QR codes (μQR)

## Dependencies
- `iconv` (glibc)

## CLI Usage
See `qr --help` for more information.

## Library Usage
Example program:
```c
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <qr/qr.h>

#define eprintf(...) fprintf(stderr, __VA_ARGS__)

int main() {
	struct qr_alloc alloc = QR_ALLOC(malloc, realloc, free);

	struct qr qr;
	memset(&qr, 0, sizeof(qr));

	const char *error = NULL;

	if (!qr_encode_utf8(&qr, alloc, "Hello world!", QR_MODE_AUTO, QR_VERSION_AUTO, QR_ECL_MEDIUM, &error)) {
		eprintf("Failed to encode data for QR code: %s\n", error);
		return 1;
	}

	if (!qr_prepare_data(&qr, &error)) {
		eprintf("Failed to prepare QR code data: %s\n", error);
		return 1;
	}

	if (!qr_render(&qr, &error)) {
		eprintf("Failed to render QR code: %s\n", error);
		return 1;
	}

	struct qr_pos pos;
	for (pos.y = 0; pos.y < qr.output.size; pos.y++) {
		for (pos.x = 0; pos.x < qr.output.size; pos.x++) {
			printf("%s", qr_bitmap_read(qr.output, pos) ? "##" : "  ");
		}
		printf("\n");
	}

	qr_close(&qr);

	return 0;
}
```
After running `meson install`, compile with 
```sh
gcc main.c -o main -Wl,-rpath,/usr/local/lib -lQR -I/usr/local/include
```

<br />

## Installing
### Debian-based
```bash
sudo apt install build-essential meson
```
Then follow the instructions below

### Other distros
Find [dependencies listed above](#dependencies) in your package manager or elsewhere:

```bash
git clone https://github.com/mekb-turtle/qr.git
cd qr
git checkout "$(git describe --tags --abbrev=0)" # checkout to latest tag, omit for latest commit
meson setup build
meson test -C build
meson install -C build
```

