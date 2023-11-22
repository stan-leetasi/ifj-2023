#!/bin/bash

# zbalí projekt

archive_name="xkruli03.zip"
rm -f ${archive_name}
zip ${archive_name} Makefile rozdeleni dokumentace.pdf ./*.c ./*.h 
