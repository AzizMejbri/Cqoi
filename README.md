
### QOI Tool - Fast Image Converter & Viewer
https://img.shields.io/badge/Format-QOI-green
https://img.shields.io/badge/Support-PPM_P6-blue
https://img.shields.io/badge/Language-C-00599C
https://img.shields.io/badge/Graphics-SDL3-FF6B00

A high-performance command-line tool for encoding, decoding, and viewing images in the Quite OK Image (QOI) format and PPM P6 format. Built in C with SDL3 for maximum performance and minimal dependencies.

## ✨ Features
🚀 Blazing Fast - Native C implementation with optimized encoding/decoding

🔄 Bidirectional Conversion:

PPM P6 → QOI encoding

QOI → PPM P6 decoding

👁️ Built-in Viewer - View images directly in SDL3 window

📦 Lossless Compression - QOI format preserves exact pixel data

🔧 Auto-detection - Automatically detects file format by extension

🎨 True Color - Full 24-bit RGB support

## 📋 Requirements
# Dependencies
- C Compiler (GCC, Clang, or MSVC)

- SDL3 (Simple DirectMedia Layer 3)

- argp (for command-line parsing - included on most Linux systems)

Installation on Ubuntu/Debian
```bash
sudo apt-get update
sudo apt-get install build-essential libsdl3-dev
```

Installation on ArchLinux
```bash
sudo pacman -S sdl3 
```

# 🛠️ Building from Source
```bash
# Clone the repository
git clone https://github.com/AzizMejbri/qoi-tool.git
cd qoi-tool

# Build the project
make release

```
🚀 Usage
Basic Commands
```bash
# Encode PPM to QOI
./qoi-tool encode -i input.ppm -o output.qoi

# Decode QOI to PPM
./qoi-tool decode -i input.qoi -o output.ppm

# Display an image (auto-detects format)
./qoi-tool display -i image.qoi
./qoi-tool display -i image.ppm

# Display with specific format
./qoi-tool display -i image.unknown -f qoi
```

Command Line Options
```text
Usage: qoi-tool encode|decode|display [OPTION...]

  -i, --input=FILE     Input file (required)
  -o, --output=FILE    Output file (optional, default stdout)
  -f, --format=FORMAT  Format for display: p6, qoi, or auto (default: auto)

Subcommands:
  encode     Convert PPM P6 to QOI format
  decode     Convert QOI to PPM P6 format
  display    View image in a window
```

Examples
```bash
# Convert a PPM image to QOI with compression
./qoi-tool encode -i photo.ppm -o photo.qoi

# Convert QOI back to PPM
./qoi-tool decode -i photo.qoi -o photo.ppm

# View any supported image
./qoi-tool display -i image.qoi

# Pipe support (output to stdout)
./qoi-tool encode -i image.ppm | gzip > image.qoi.gz


# Batch processing with shell
for file in *.ppm; do
    ./qoi-tool encode -i "$file" -o "${file%.ppm}.qoi"
done
```

## 📊 Performance
QOI format offers excellent performance characteristics:

Fast Encoding/Decoding 

Streamable - No need to parse entire file before display

Simple Implementation - Single-file decoder fits in ~200 lines of C

No Dependencies - For core encoding/decoding operations

## 🏗️ Project Structure
. <br>
├── cli.c &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;           # CLI interface and argument parsing <br>
├── cli.h<br>
├── decode.c &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;       # QOI → PPM P6 decoding<br>
├── decode.h<br>
├── encode.c &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;       # PPM P6 → QOI encoding<br>
├── encode.h<br>
├── main.c<br>
├── Makefile    &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;    # Build configuration<br>
├── out<br>
│   ├── debug<br>
│   └── release<br>
├── README.md &nbsp;&nbsp;# This file<br>
├── types.h &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;         # Common type definitions<br>
├── viewer.c &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;        # SDL3-based image viewer<br>
└── viewer.h<br>


## 🔧 Technical Details
# PPM P6 Format Support
Supports binary PPM (P6) format with 24-bit RGB

No support for PBM, PGM, or ASCII PPM (P1-P3)

Max dimensions limited by available memory

# QOI Implementation
Implements the QOI specification

Supports all QOI features: RGB/RGBA, run-length encoding, index, diff

## 🙏 Acknowledgments
Dominic Szablewski for creating the QOI format

SDL Community for the excellent SDL3 library


