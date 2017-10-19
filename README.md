# Flexible Packet Parser P4 compiler backend

Backend for P4 v16 compiler. This backend was created by modifiyng the existing EBPF sample backend.

## Installation

Clone P4 compiler github repository

```
git clone --recursive https://github.com/p4lang/p4c
cd p4c/
git checkout 36ee62aeaa1d31cca57a613f9b02e367fb67c270
```

make symbolic link to this folder in p4c/extensions/

```
ln -s PATH-TO-FOLDER  extensions/fpp
```

Follow the instructions in p4c/README.md in order to build compiler.
