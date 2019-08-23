# Flexible Packet Parser P4 compiler backend

Backend for P4 v16 compiler. This backend was created by modifiyng the existing EBPF sample backend.

## Installation

Follow the instructions in p4c/README.md - download dependencies. Backend installation steps:
```
git clone --quiet --recursive https://github.com/p4lang/p4c.git &&
cd p4c &&
git checkout d47142709a6c1f9ceb3f7a779cc5c220f0712f05 &&
mkdir extensions &&
cd extensions &&
ln -s ../../fpp-p4c-backend p4exporter
cd .. &&
mkdir -p build &&
cd build &&
cmake -DCMAKE_BUILD_TYPE=DEBUG -DENABLE_GC=OFF -DENABLE_PROTOBUF_STATIC=OFF -DENABLE_DOCS=OFF -DENABLE_BMV2=OFF .. &&
make -j 4 &&
make -j 4 install &&
```

### Vagrant
Easiest way to install is to use vagrant virtual machine.
```
vagrant up
```

Wait until installation is done.

```
vagrant ssh
```

