#!/bin/bash
#Copyright (C) 2014, 2021 Ben Asselstine
#This script is licensed under the terms of the GNU GPL version 3 or later.
#
# Pares down the gnome icons for distribution in the windows version of
# LordsAWar!.   There are just too many megabytes of them to include with
# this game, so this script tries to take a useful subset of them.
#
# This script copies the icon files, removes cache files, removes the icons 
# that we don't want, and then finally it updates the INI files appropriately.
# The INI files are called index.theme.
#
# This script does it's work in /tmp/, and then drops a newly created 
# share/ directory in the current working directory.
#
# We've removed the stock icons in the glade files but still the default icons
# on the title bar are required (e.g. minimize, maxmimize, close.)

#process these directories of icons in /usr/share/icons/
dirs="Adwaita"

if [ -d share ]; then
  echo "There is already a share/ directory here!"
  exit 1
fi
#remove these sizes
adwaitabadsizes="scalable scalable-up-to-32 8x8 22x22 24x24 32x32 48x48 64x64 96x96 256x256"

remove_ini_section()
{
  inifile=$1
  name=$2
  count="0"
  tmpfile=`mktemp /tmp/bar.XXXXXX`
  while IFS='' read -r line || [[ -n $line ]]; do
    echo $line | grep "^\[$name.*\]$" 2>/dev/null >/dev/null
    ret="$?"
    if [ "x$count" != "x0" ]; then
      count=`expr $count + 1`
    fi
    if [ "x$ret" == "x0" ]; then
      count="1"
    fi
    if [ "$count" == "0" ]; then
      echo $line >> $tmpfile
    fi
    if [ "x$line" == "x" ]; then
      count="0"
    fi
  done < "$inifile"
  cp $tmpfile $inifile
  rm $tmpfile
}

fixup_ini () 
{
  inifile=$1
  shift
  badsizes=$*
  for b in $badsizes; do
    remove_ini_section $inifile $b
  done
  for b in $badsizes; do
    sed -i -e "s/$b\/[A-Za-z0-9\/]*,*//g" $inifile
  done
}

tmpdir=`mktemp -d /tmp/foo.XXXXXX`

mkdir -p $tmpdir/share/icons

for d in $dirs; do
  cp -r /usr/share/icons/$d $tmpdir/share/icons/
  rm $tmpdir/share/icons/$d/icon-theme.cache
  badsizes=""
  if [ "$d" == "Adwaita" ]; then
    badsizes=$adwaitabadsizes
    rm -rf $tmpdir/share/icons/$d/cursors
  fi
  for delsize in $badsizes; do
    if [ -d $tmpdir/share/icons/$d/$delsize ]; then
      rm -rf $tmpdir/share/icons/$d/$delsize
    fi
  done
  fixup_ini $tmpdir/share/icons/$d/index.theme $badsizes
done

mkdir -p $tmpdir/share/glib-2.0/schemas
cp /usr/share/glib-2.0/schemas/gschemas.compiled $tmpdir/share/glib-2.0/schemas

mv $tmpdir/share ./
rmdir $tmpdir

