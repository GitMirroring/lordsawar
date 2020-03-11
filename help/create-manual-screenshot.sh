#!/bin/bash
#
# Copyright (C) Ben Asselstine 2017, 2020
#
#   This program is free software; you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation; either version 3 of the License, or
#   (at your option) any later version.
# 
#   This program is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU Library General Public License for more details.
# 
#   You should have received a copy of the GNU General Public License
#   along with this program; if not, write to the Free Software
#   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 
#   02110-1301, USA.

# The approach here is to bring lordsawar to the foreground and then
# take a screenshot of it.
# the import program from imagemagick takes a screenshot of the active
# window, and then we dump the name of the file it created.
#
# it takes time to bring the window to the foreground, so we wait half
# a second before taking the screenshot.
#
# unfortunately the top bar needs to be taken into account, and it may
# be a different size on your system.
# also the size of the window can be different depending on your fonts.
# so you'll have to play with these values:
# -crop 882x646+0+33 
# my bar is 33 pixels tall, and the lordsawar window *with a unit selected*
# is 882 pixels wide and 646 pixels tall.
#
# if i could find a way to automatically find the size of the top bar, i'd
# do that.
tmpfile=`mktemp lwtest.XXXXXX --suffix .png -p ~/.cache/lordsawar`
xdotool=`which xdotool 2>/dev/null`
if [ ! -x "$xdotool" ]; then
        echo "$0: error: we need xdotool to run this script."
        exit 1
fi
screenshot=`which import 2>/dev/null`
if [ ! -x "$screenshot" ]; then
        echo "$0: error: we need imagemagick to run this script."
        exit 1
fi

id=`$xdotool search 'LordsAWar!' 2>/dev/null | tail -n1`
if [ "x$id" == "x" ]; then
        echo "$0: error: lordsawar not running!"
        exit 1
fi
ids=`$xdotool search 'LordsAWar!' 2>/dev/null`
for f in $ids; do
  $xdotool windowactivate $f 2>/dev/null
done
sleep 0.5
$screenshot -window root -border -frame $tmpfile -crop 882x646+0+33 2>/dev/null
echo $tmpfile
