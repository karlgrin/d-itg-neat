# d-itg-neat 
This project is a derivative work of the code from the [D-ITG](http://traffic.comics.unina.it/software/ITG/) traffic generator with the aim to support the [NEAT](https://www.neat-project.org/)  transport system. 

## Requirements
* `cmake`
* `libuv`
* `ldns`
* `ljansson`
* `libmnl`
* `libsctp-dev`

| OS               | Install Dependencies                                                                 |
| :--------------- | :----------------------------------------------------------------------------------- |
| Ubuntu 15.04 or higher   | `apt-get install cmake libuv1-dev libldns-dev libjansson-dev libmnl-dev libsctp-dev` |

## Installation

Get the source files
```sh
$ git clone --recursive https://github.com/karlgrin/d-itg-neat.git
$ cd d-itg-neat 
$ mkdir bin
```

Build NEAT
```sh
$ cd d-itg-neat/NEAT
$ mkdir build && cd build
$ cmake ..
$ cmake --build .
```

Build D-ITG
```sh
$ cd d-itg-neat/src
$ make -B
```

Add libneat to local libraries
```sh
$ cd d-itg-neat
$ sudo cp NEAT/build/libneat.so /usr/local/lib
$ sudo ldconfig
```

## Quick start

In this example ITGSend will generate one NEAT flow with constant payload size (100 bytes) and constant packet rate (10 pps) for 15 seconds (15000 ms) with the neat properties set in custom.json

Open two console windows
```sh
$ cd d-itg-neat/bin
```
Start server
```sh
$ ./ITGRecv -NO "-P ../custom.json"
```

Start client
```sh
$ ./ITGSend -T NEAT -a 127.0.0.1 -c 100 -C 10 -t 15000 -NO "-P ../custom.json"
```

