#!/bin/bash
make clean
make dep
sudo make
gdb bootimg

