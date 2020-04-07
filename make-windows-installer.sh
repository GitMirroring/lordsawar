#!/bin/bash
# Copyright (C) 2015 Ben Asselstine
# this script is licensed under the terms of the GNU GPL version 3, or later.
#
# make a windows installer using NSIS and the zip file that gets generated 
# by running make-windows-zip.sh

#let's make sure the lordsawar-windows.zip file is in place.
if [ !  -x ./make-windows-zip.sh ]; then
  if [ !  -x ./make-windows-zip.sh.in ]; then
    echo "Error: You need to run ./configure first."
  else
    echo "Error: run this script from the top level of the lordsawar src tree."
  fi
  exit 1
fi

if [ ! -f ./lordsawar-windows.zip ]; then
  ./make-windows-zip.sh
fi
if [ ! -f ./lordsawar-windows.zip ]; then
  echo "Error: can't make the zip file."
  exit 1
fi

#okay, do we have NSIS?
makensis=`which makensis 2>/dev/null`
if [ "x$makensis" == "x" ]; then
  echo "Error: We need the makensis command.  sudo dnf install mingw32-nsis"
  exit 1
fi
mktemp=`which mktemp 2>/dev/null`
if [ "x$mktemp" == "x" ]; then
  echo "Error: We need the mktemp command.  sudo dnf install coreutils"
  exit 1
fi
find=`which find 2>/dev/null`
if [ "x$find" == "x" ]; then
  echo "Error: We need the mktemp command.  sudo dnf install findutils"
  exit 1
fi
unzip=`which unzip 2>/dev/null`
if [ "x$unzip" == "x" ]; then
  echo "Error: We need the mktemp command.  sudo dnf install unzip"
  exit 1
fi


#unzip the zip file and enumerate the contents
tmpdir=`mktemp -d /tmp/lordsawar-windows.XXXXX`
unzip -q -d $tmpdir lordsawar-windows.zip
if [ "x$?" != "x0" ]; then
  echo "Error: we had trouble extracting the zip file."
  if [ -d $tmpdir ]; then
    rm -rf $tmpdir
  fi
  exit 1
fi

orig=`pwd`
cd $tmpdir
dirlist=`mktemp /tmp/lordsawar-windows.XXXXXX`
find -maxdepth 2 -type d  > $dirlist
filelist=`mktemp /tmp/lordsawar-windows.XXXXXX`
find -maxdepth 2 -type f  > $filelist
cd $orig

nsiscript=`mktemp /tmp/lordsawar-windows.XXXXXX`
 
echo "Name \"LordsAWar!\"" >> $nsiscript
echo "OutFile \"lordsawar-setup.exe\"" >> $nsiscript
echo "InstallDir \"\$PROGRAMFILES32\\LordsAWar\"" >> $nsiscript
echo "!include \"MUI.nsh\"" >> $nsiscript
echo "!insertmacro MUI_PAGE_WELCOME" >> $nsiscript
echo "!insertmacro MUI_PAGE_DIRECTORY" >> $nsiscript
echo "!insertmacro MUI_PAGE_INSTFILES" >> $nsiscript
echo "!define MUI_FINISHPAGE_NOAUTOCLOSE" >> $nsiscript
echo "!define MUI_FINISHPAGE_RUN" >> $nsiscript
echo "!define MUI_FINISHPAGE_RUN_NOTCHECKED" >> $nsiscript
echo "!define MUI_FINISHPAGE_RUN_TEXT \"Start playing LordsAWar! right now\"" >> $nsiscript
echo "!define MUI_FINISHPAGE_RUN_FUNCTION \"LaunchLink\"" >> $nsiscript
echo "!insertmacro MUI_PAGE_FINISH" >> $nsiscript
echo "!insertmacro MUI_LANGUAGE \"English\"" >> $nsiscript
echo "Section \"install\"" >> $nsiscript
echo "  SetOutPath \"\$INSTDIR\"" >> $nsiscript

while IFS='' read -r line || [[ -n $line ]]; do
  echo -n "  File " >> $nsiscript
  echo $line | sed -e 's/^.\/lordsawar-windows\///g' >> $nsiscript
done < "$filelist"
echo "  CreateShortcut \"\$DESKTOP\\lordsawar.lnk\" \"\$INSTDIR\\lordsawar.exe\" \"\" \"\$INSTDIR\\various\\castle_icon.ico\"" >> $nsiscript

while IFS='' read -r line || [[ -n $line ]]; do
  if [ "x$line" == "x." ]; then
    continue
  fi
  if [ "x$line" == "x./lordsawar-windows" ]; then
    continue
  fi
  dir=`echo $line | sed -e 's/^.\/lordsawar-windows\///g'`
  echo "  File /r $dir" >> $nsiscript
done < "$dirlist"
echo "  WriteUninstaller \"\$INSTDIR\\Uninstall.exe\"" >> $nsiscript
echo "SectionEnd" >> $nsiscript

echo "Section \"Uninstall\"" >> $nsiscript
echo "  RMDir /r \"\$INSTDIR\*.*\"" >> $nsiscript
echo "  RMDir \"\$INSTDIR\"" >> $nsiscript
echo "  delete \"\$DESKTOP\\lordsawar.lnk\"" >> $nsiscript
echo "  delete \"\$INSTDIR\\Uninstall.exe\"" >> $nsiscript
echo "SectionEnd" >> $nsiscript

echo "Function LaunchLink" >> $nsiscript
echo "  ExecShell \"open\" \"\$Desktop\\lordsawar.lnk\"" >> $nsiscript
echo "FunctionEnd" >> $nsiscript
echo "Generated this .nsi script:"
cat $nsiscript
echo "------"
echo "Please wait while we generate lordsawar-setup.exe..."

cd $tmpdir/lordsawar-windows
makensis -NOCD $nsiscript
if [ -f $tmpdir/lordsawar-windows/lordsawar-setup.exe ]; then
  cp $tmpdir/lordsawar-windows/lordsawar-setup.exe $orig
else
  echo "Error: something went wrong when creating the installer."
fi
cd $orig
# cleanup
rm $dirlist
rm $filelist
if [ -d $tmpdir ]; then
  rm -rf $tmpdir
fi
rm $nsiscript
echo "Done."
