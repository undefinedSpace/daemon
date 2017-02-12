# undefinedSpace (C++ daemon)

undefinedSpace - It's a system for changes monitoring of files on the hard drive, helpful if you do not know which file was changed a couple of days ago.

undefinedSpace - complex of programs that consists of 3 main blocks: Daemon, API (PHP, JS), Web UI (soon).

## How to install

First you need the make, gcc and some other tools/libs, on Debian or Ubuntu you can install all what you need via APT:

    apt-get install gcc make zlib1g-dev

Next we need build this program, for this from folder with source code you just need execute:

    make

As a result, you should see the **file_status** binary file.

## How to use

To use this program you need write 3 arguments:

    ./file_status path api-address

* path - absolute path on your filesystem
* api-server - ip or dns address of your api
