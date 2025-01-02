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

