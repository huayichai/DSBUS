# DSBUS

`DSBUS` is a repository for practicing data structures.

`DS` stands for data structures and `BUS` stands for the kindergarten bus, which means beginner.


## Build 
We recommend developing DSBus on Ubuntu 20.04, Ubuntu 22.04.
```bash
mkdir build
cd build
cmake ..
make -j8
```
If you want to compile the DSBUS in release mode, pass in the following flag to cmake: Release mode:
```bash
cmake -DCMAKE_BUILD_TYPE=Release ..
make -j8
```

## Testing
```bash
cd build
make xxx_test -8
./test/xxx_test
```