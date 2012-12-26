wol (Wake on LAN)
=================

Main motivation is to remotely wake up a PC prior to trying to access it via remote desktop.

Supports host name resolution (as well as regular IP address) and defaults to port 3389.  This will allow it to wake your PC from outside your local firewall (assuming your router has been configured to allow remote desktop connections).

Usage:  wol -b BROADCAST_TARGET [-p BROADCAST_PORT] MAC_ADDRESS

Tested on:
*  Windows
*  Linux
*  OSX

Prerequisites:
*  CMake:  sudo apt-get install cmake
*  Boost:  sudo apt-get install libboost-all-dev

Build Instructions:
```
mkdir git
cd git
git clone git@github.com:GordonSmith/wol.git
mkdir build
cd build
mkdir wol
cd wol
cmake ../../wol
cmake --build . --config Release
```
