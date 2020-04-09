#!/bin/sh
#This script sets up the codebase so that it can be configured.
autoreconf -fis
intltoolize --automake
echo "Now type './configure' to prepare LordsAWar! for compilation."
