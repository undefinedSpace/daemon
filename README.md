# undefinedSpace (C++ daemon)

undefinedSpace - It's a system for changes monitoring of files on the hard drive, helpful if you do not know which file was changed a couple of days ago.

undefinedSpace - complex of programs that consists of 3 main blocks: [Daemon](https://github.com/undefinedSpace/daemon), API ([PHP](https://github.com/undefinedSpace/api-php), [JS](https://github.com/undefinedSpace/nodejs-api)), Web UI (soon).

## How to install

First you need the make, gcc and some other tools/libs, on Debian or Ubuntu you can install all what you need via APT:

    apt-get install cmake gcc make zlib1g-dev

Next we need build this program, for this from folder with source code you just need execute:

    mkdir build
    cd build
    cmake ..
    make

As a result, you should see the **daemon** binary file into the source directory.

## How to use

To use this program you need to give 2 or more arguments:

    ./daemon <path 1> ... [path n] <api-address[:port]>

* path 1, ..., path n - absolute paths in your filesystem
* api-server - ip or dns address of your api
