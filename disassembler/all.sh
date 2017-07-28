#! /usr/bin/env sh

(./disassembler/disassembler.lua addresses=true entry=1,2,4,5,6,7,8,9,10,11,12,13,14,15,16) < /dev/stdin > /dev/stdout
