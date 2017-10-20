# Flexible packet parser example

This folder contains example usage of generated packet parser. Example program reads packet

## Dependencies

In order to compile and use this program, you will need [libpcap development version](http://www.tcpdump.org/) installed.

## Build

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

## Directory structure

```
.
├── bootstrap.sh
├── configure.ac
├── Makefile.am
├── example.c           Example program.
├── p4-16               Folder with P4 parser.
│   ├── headers.p4      P4 parser headers definition.
│   └── parser.p4       P4 parser program.
├── parser.c            Generated parser by fpp backend from `p4-16/*.p4` files.
├── parser.h            Generated parser by fpp backend from `p4-16/*.p4` files.
├── pcaps               Folder with example pcaps.
    └── tunnel.pcap     Pcap with tunelled traffic.
```

