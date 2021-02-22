#!/bin/bash
# Copyright (C) 2015 Ben Asselstine
# this script is licensed under the terms of the GNU GPL version 3, or later.

if [ ! -f configure.ac ]; then
  echo We need you to run this script in the top level directory of the lordsawar source tree.
  exit 1
fi

if [ ! -f src/.libs/lordsawar.exe ]; then
  echo We need you to cross-compile before running this script.  See the WINDOWS file.
  exit 1
fi

if [ -d lordsawar-windows ]; then
  echo We need you to remove the lordawar-windows directory if you want us to run again.
  exit 1
fi

bundledlls=`which mingw-bundledlls`
if [ "x$bundledlls" == "x" ]; then
  echo We need mingw-bundledlls.  You can get it from: https://github.com/mpreisler/mingw-bundledlls 
  exit 1
fi

zip=`which zip`
if [ "x$zip" == "x" ]; then
  echo We need zip.  sudo yum install zip
  exit 1
fi

if [ ! -d /usr/local/share/lordsawar ]; then
  echo "We need you to run \"make install\" before running this script."
  exit 1
fi

mkdir lordsawar-windows
cd lordsawar-windows
echo Please wait while we collect icons from GNOME...
../copy-gnome-icons.sh
cp ../src/.libs/lordsawar.exe ./
echo Please wait while we collect DLLs...
$bundledlls ./lordsawar.exe --copy
cp -r /usr/local/share/lordsawar/* ./
mingw-strip ./lordsawar.exe
cd ..
rm lordsawar-windows.zip 2>/dev/null >/dev/null
$zip -9 -r lordsawar-windows.zip lordsawar-windows
rm -rf lordsawar-windows
