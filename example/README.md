# Flexible packet parser example

This folder contains example usage of generated packet parser.

# Build

Compile example by the following commands:

```
./bootstrap.sh
./configure
make
```

Run example by:

```
./example
```

# Directory structure

.
├── bootstrap.sh
├── configure.ac
├── Makefile.am
├── example.c           Main file.
├── p4-16               Folder with P4 parser.
│   ├── headers.p4      P4 headers definition.
│   └── parser.p4       P4 parser program.
├── parser.c            Generated parser by fpp backend from `p4-16/*.p4` files.
├── parser.h            Generated parser by fpp backend from `p4-16/*.p4` files.
├── pcaps               Folder with example pcaps.
    └── tunnel.pcap     Pcap with tunelled traffic.

