#!/bin/bash
# Copyright (C) Ben Asselstine 2017, 2020
#
# The idea here is that we're going to replay an Xnee file to generate
# the correct screenshots for the manual.
# We need the manual.xnl file, the manual.map file, the
# main-screen-overlay.png file and to be run from the help/ directory.
# We also need xdotool, imagemagick, Xnee and cnee installed.
#
# We take a screenshot 14.5 seconds in, and then finally we
# generate main_screen.png
# (the .5 comes from the create-manual-screenshot program)
#
# And we also take the rest of the screenshots too, and size them.
#
# Why am I doing this!  Who still reads manuals anyway?
#
# For whatever reason, sometimes the replay doesn't quite happen right.
# I guess it's the timing of it all.
# Or maybe it's how we handle mouse input in the game.


grab="1"
if [ "x$grab" == "x1" ]; then
  echo "press enter when you're ready to leave the computer alone while it does this"
  read answer
  xdotool mousemove 0 0
  gnome-terminal --window --command="bash -c 'cnee --synchronised-replay --replay --file manual.xnl --recall-window-position & bash'"
  count=0
  sleep 36
  echo "image-01"
  image01=`./create-manual-screenshot.sh`
  sleep 2
  echo "image-02"
  image02=`./create-manual-screenshot.sh`
  sleep 3
  echo "image-03"
  image03=`./create-manual-screenshot.sh`
  sleep 2
  echo "image-04"
  image04=`./create-manual-screenshot.sh`
  sleep 1
  echo "image-05"
  image05=`./create-manual-screenshot.sh`
  sleep 0.5
  echo "image-06"
  image06=`./create-manual-screenshot.sh`
  sleep 0.5
  echo "image-07"
  image07=`./create-manual-screenshot.sh`
  sleep 0.5
  echo "image-08"
  image08=`./create-manual-screenshot.sh`
  sleep 9
  echo "image-09"
  image09=`./create-manual-screenshot.sh`
  sleep 0.5
  echo "image-10"
  image10=`./create-manual-screenshot.sh`
  #sleep 0.25
  echo "image-11"
  image11=`./create-manual-screenshot.sh`
  echo "image-12"
  image12=`./create-manual-screenshot.sh`
  sleep 1
  sleep 0.25
  sleep 0.5
  echo "image-13"
  image13=`./create-manual-screenshot.sh`
  sleep 7 
  echo "image-14"
  image14=`./create-manual-screenshot.sh`
  sleep 2 
  echo "image-15"
  image15=`./create-manual-screenshot.sh`
  sleep 4 
  echo "image-16"
  image16=`./create-manual-screenshot.sh`
  sleep 2 
  echo "image-17"
  image17=`./create-manual-screenshot.sh`
  sleep 2 
  echo "image-18"
  image18=`./create-manual-screenshot.sh`
  sleep 2 
  echo "image-19"
  image19=`./create-manual-screenshot.sh`
  sleep 4 
  echo "image-20"
  image20=`./create-manual-screenshot.sh`
  sleep 3 
  echo "image-21"
  image21=`./create-manual-screenshot.sh`
  sleep 1 
  echo "image-22"
  image22=`./create-manual-screenshot.sh`
  sleep 2 
  echo "image-23"
  image23=`./create-manual-screenshot.sh`
  sleep 25 
  echo "image-24"
  image24=`./create-manual-screenshot.sh`
  sleep 8
  echo "image-25"
  image25=`./create-manual-screenshot.sh`
  sleep 12
  echo "image-26"
  image26=`./create-manual-screenshot.sh`
  sleep 3
  echo "image-27"
  image27=`./create-manual-screenshot.sh`
  sleep 3
  sleep 0.4
  echo "image-28"
  image28=`./create-manual-screenshot.sh`
  sleep 0.15
  sleep 3
  echo "image-29"
  image29=`./create-manual-screenshot.sh`
  sleep 6.5
  echo "image-29b"
  image29b=`./create-manual-screenshot.sh`
  sleep 10
  echo "image-30"
  image30=`./create-manual-screenshot.sh`
  sleep 19
  echo "image-31"
  image31=`./create-manual-screenshot.sh`
  sleep 8
  echo "image-32"
  image32=`./create-manual-screenshot.sh`
  sleep 14
  echo "image-33"
  image33=`./create-manual-screenshot.sh`
  sleep 3
  echo "image-34"
  image34=`./create-manual-screenshot.sh`
  echo "image-35"
  image35=`./create-manual-screenshot.sh`
  echo "image-36"
  image36=`./create-manual-screenshot.sh`
  sleep 12
  sleep 7
  echo "image-37"
  image37=`./create-manual-screenshot.sh`
  sleep 4
  echo "image-38"
  image38=`./create-manual-screenshot.sh`
  
  cp $image01 image-01.png
  cp $image02 image-02.png
  cp $image03 image-03.png
  cp $image04 image-04.png
  cp $image05 image-05.png
  cp $image06 image-06.png
  cp $image07 image-07.png
  cp $image08 image-08.png
  cp $image09 image-09.png
  cp $image10 image-10.png
  cp $image11 image-11.png
  cp $image12 image-12.png
  cp $image13 image-13.png
  cp $image14 image-14.png
  cp $image15 image-15.png
  cp $image16 image-16.png
  cp $image17 image-17.png
  cp $image18 image-18.png
  cp $image19 image-19.png
  cp $image20 image-20.png
  cp $image21 image-21.png
  cp $image22 image-22.png
  cp $image23 image-23.png
  cp $image24 image-24.png
  cp $image25 image-25.png
  cp $image26 image-26.png
  cp $image27 image-27.png
  cp $image28 image-28.png
  cp $image29 image-29.png
  cp $image29b image-29b.png
  cp $image30 image-30.png
  cp $image31 image-31.png
  cp $image32 image-32.png
  cp $image33 image-33.png
  cp $image34 image-34.png
  cp $image35 image-35.png
  cp $image36 image-36.png
  cp $image37 image-37.png
  cp $image38 image-38.png

  rm $image01 $image02 $image03 $image04 $image05 $image06 $image07 \
  $image08 $image09 $image10 $image11 $image12 $image13 $image14 \
  $image15 $image16 $image17 $image18 $image19 $image20 $image21 \
  $image22 $image23 $image24 $image25 $image26 $image27 $image28 \
  $image29 $image29b $image30 $image31 $image32 $image33 $image34 \
  $image35 $image36 $image37 $image38

  #killall cnee
fi

# create army_bonus.png
infile="image-29.png"
convert +repage -crop 553x362+165+143 $infile army_bonus.png

# army_unit_info.png
infile="image-24.png"
tmpfile=`mktemp lwtest.XXXXXX  --suffix=.png -p /tmp`
convert +repage -crop 295x228+0+418 $infile $tmpfile
#needs a cursor on top of it.
composite -compose srcover -geometry +159+162 mouse-cursor.png $tmpfile army_unit_info.png
rm $tmpfile

# buy_production.png
infile="image-27.png"
convert +repage -crop 446x393+218+127 $infile buy_production.png

# city_window.png
infile="image-26.png"
convert +repage -crop 700x513+91+67 $infile city_window.png

# conquer_city.png
infile="image-25.png"
convert +repage -crop 489x449+197+99 $infile conquer_city.png

# fight_city_one_vs_two.png
infile="image-03.png"
tmpfile=`mktemp lwtest.XXXXXX  --suffix=.png -p /tmp`
convert +repage -crop 200x200+204+149 $infile $tmpfile
convert +repage -crop 16x16+128+0 ../dat/various/cursors.png sword-cursor.png
composite -compose srcover -geometry +104+113 sword-cursor.png $tmpfile fight_city_one_vs_two.png
rm $tmpfile
rm sword-cursor.png

# fighting_city_one_vs_two.png
infile="image-06.png"
convert +repage -crop 234x194+325+226 $infile fighting_city_one_vs_two.png

# fighting_one_vs_one.png
infile="image-12.png"
convert +repage -crop 143x194+370+226 $infile fighting_one_vs_one.png

# fight_one_vs_one.png
infile="image-09.png"
tmpfile=`mktemp lwtest.XXXXXX  --suffix=.png -p /tmp`
convert +repage -crop 200x200+255+214 $infile $tmpfile
convert +repage -crop 16x16+128+0 ../dat/various/cursors.png sword-cursor.png
composite -compose srcover -geometry +130+127 sword-cursor.png $tmpfile fight_one_vs_one.png
rm sword-cursor.png
rm $tmpfile

# fight_order.png
infile="image-28.png"
convert +repage -crop 424x465+229+91 $infile fight_order.png

# fought_city_one_vs_two.png
infile="image-08.png"
convert +repage -crop 200x200+240+185 $infile fought_city_one_vs_two.png

# fought_one_vs_one.png
infile="image-13.png"
convert +repage -crop 200x200+213+175 $infile fought_one_vs_one.png

# create main_screen.png
infile="image-30.png"
tmpfile=`mktemp lwtest.XXXXXX  --suffix=.png -p /tmp`
convert $infile -resize 510 $tmpfile
step1=`mktemp lwtest.XXXXXX  --suffix=.png -p /tmp`
composite -size 700x391 -compose srcover $tmpfile -geometry +99+02 main-screen-overlay.png $step1
composite -compose srcover main-screen-overlay.png $step1 main_screen.png
rm $tmpfile
rm $step1

# main_screen_bag.png
infile="image-29b.png"
convert +repage -crop 72x69+484+391 $infile main_screen_bag.png

# main_screen_buttons.png
infile="image-09.png"
tmpfile=`mktemp lwtest.XXXXXX  --suffix=.png -p /tmp`
convert +repage -crop 201x124+665+407 $infile $tmpfile
convert +repage -crop 20x20+60+0 ../dat/various/buttons.png button.png
convert button.png -resize 24 button2.png
tmpfile2=`mktemp lwtest.XXXXXX  --suffix=.png -p /tmp`
composite -compose srcover -geometry +16+11 button2.png $tmpfile $tmpfile2
rm button.png
convert +repage -crop 20x20+0+0 ../dat/various/buttons.png button.png
convert button.png -resize 24 button2.png
tmpfile3=`mktemp lwtest.XXXXXX  --suffix=.png -p /tmp`
composite -compose srcover -geometry +63+50 button2.png $tmpfile2 $tmpfile3
rm button.png
convert +repage -crop 20x20+180+0 ../dat/various/buttons.png button.png
convert button.png -resize 24 button2.png
tmpfile4=`mktemp lwtest.XXXXXX  --suffix=.png -p /tmp`
composite -compose srcover -geometry +159+50 button2.png $tmpfile3 $tmpfile4
rm button.png
convert +repage -crop 20x20+80+0 ../dat/various/buttons.png button.png
convert button.png -resize 24 button2.png
composite -compose srcover -geometry +16+90 button2.png $tmpfile4 main_screen_buttons.png
rm button.png
rm button2.png
rm $tmpfile
rm $tmpfile2
rm $tmpfile3

# main_screen_center_on_stack_button.png
convert +repage -crop 52x44+1+40 main_screen_buttons.png button.png
convert button.png -resize 200 main_screen_center_on_stack_button.png

# main_screen_defend_stack_button.png
convert +repage -crop 52x44+97+40 main_screen_buttons.png button.png
convert button.png -resize 200 main_screen_defend_stack_button.png

# main_screen_deselect_stack_button.png
convert +repage -crop 52x44+146+0 main_screen_buttons.png button.png
convert button.png -resize 200 main_screen_deselect_stack_button.png

# main_screen_diplomacy_button.png
convert +repage -crop 52x44+49+40 main_screen_buttons.png button.png
convert button.png -resize 200 main_screen_diplomacy_button.png

# main_screen_diplomacy_proposed_button.png
convert +repage -crop 52x44+49+40 main_screen_buttons.png d.png
convert +repage -crop 20x20+160+0 ../dat/various/buttons.png button.png
convert button.png -resize 24 button2.png
composite -compose srcover -geometry +14+10 button2.png d.png e.png
convert e.png -resize 200 main_screen_diplomacy_proposed_button.png
rm d.png
rm e.png
rm button2.png
rm button.png

# main_screen_end_turn_button.png
convert +repage -crop 52x44+49+80 main_screen_buttons.png button.png
convert button.png -resize 200 main_screen_end_turn_button.png
rm button.png

# main_screen_move_all_stacks_button.png
convert +repage -crop 52x44+1+80 main_screen_buttons.png button.png
convert button.png -resize 200 main_screen_move_all_stacks_button.png
rm button.png

# main_screen_move_stack_button.png
convert +repage -crop 52x44+0+0 main_screen_buttons.png button.png
convert button.png -resize 200 main_screen_move_stack_button.png
rm button.png

# main_screen_park_stack_button.png
convert +repage -crop 52x44+97+0 main_screen_buttons.png button.png
convert button.png -resize 200 main_screen_park_stack_button.png
rm button.png

# main_screen_select_next_stack_button.png
convert +repage -crop 52x44+49+0 main_screen_buttons.png button.png
convert button.png -resize 200 main_screen_select_next_stack_button.png
rm button.png

# main_screen_stack_search_button.png
convert +repage -crop 52x44+145+40 main_screen_buttons.png button.png
convert button.png -resize 200 main_screen_stack_search_button.png
rm button.png

# main_screen_city.png
infile="image-16.png"
convert +repage -crop 138x133+262+328 $infile main_screen_city.png

# main_screen_city_razed.png
infile="image-20.png"
convert +repage -crop 138x133+257+334 $infile main_screen_city_razed.png

# main_screen_forest.png
infile="image-15.png"
convert +repage -crop 122x122+327+315 $infile main_screen_forest.png

# main_screen_hills.png
infile="image-17.png"
convert +repage -crop 122x122+206+235 $infile main_screen_hills.png

# main_screen_mountains.png
infile="image-14.png"
convert +repage -crop 122x122+426+291 $infile main_screen_mountains.png

# main_screen_move_bonus_flying.png
infile="image-30.png"
convert +repage -crop 96x96+465+550 $infile button.png
convert button.png -resize 200 main_screen_move_bonus_flying.png

# main_screen_move_bonus_forest.png
infile="image-35.png"
convert +repage -crop 96x96+465+550 $infile button.png
convert button.png -resize 200 main_screen_move_bonus_forest.png
rm button.png

# main_screen_move_bonus_hills.png
infile="image-36.png"
convert +repage -crop 96x96+465+550 $infile button.png
convert button.png -resize 200 main_screen_move_bonus_hills.png
rm button.png

# main_screen_move_bonus_hills_forest.png
infile="image-34.png"
convert +repage -crop 96x96+465+550 $infile button.png
convert button.png -resize 200 main_screen_move_bonus_hills_forest.png
rm button.png

# main_screen_move_bonus_water.png
infile="image-33.png"
convert +repage -crop 96x96+465+550 $infile button.png
convert button.png -resize 200 main_screen_move_bonus_water.png
rm button.png

# main_screen_port.png
infile="image-29b.png"
convert +repage -crop 76x69+34+221 $infile main_screen_port.png

# main_screen_road.png
infile="image-18.png"
convert +repage -crop 76x69+147+341 $infile main_screen_road.png

# main_screen_ruin.png
infile="image-18.png"
convert +repage -crop 76x69+349+421 $infile main_screen_ruin.png

# main_screen_selected_stack_flags.png
infile="image-31.png"
convert +repage -crop 208x121+260+257 $infile main_screen_selected_stack_flags.png

# main_screen_selected_stack_one.png
infile="image-32.png"
convert +repage -crop 76x69+309+265 $infile main_screen_selected_stack_one.png

# main_screen_signpost.png
infile="image-18.png"
convert +repage -crop 76x69+464+261 $infile main_screen_signpost.png

# main_screen_stack_tray_4_units.png
infile="image-38.png"
convert +repage -crop 551x117+0+529 $infile main_screen_stack_tray_4_units.png

# main_screen_stack_tray_5_units.png
infile="image-37.png"
convert +repage -crop 551x117+0+529 $infile main_screen_stack_tray_5_units.png

# main_screen_stack_water_defend.png
infile="image-14.png"
convert +repage -crop 117x94+383+90 $infile main_screen_stack_water_defend.png

# main_screen_stat_icons.png
infile="image-02.png"
convert +repage -crop 449x120+0+526 $infile main_screen_stat_icons.png

# main_screen_swamp.png
infile="image-16.png"
convert +repage -crop 122x122+127+115 $infile main_screen_swamp.png

# main_screen_temple.png
infile="image-01.png"
convert +repage -crop 76x69+428+383 $infile main_screen_temple.png

# main_screen_unselected_stack.png
infile="image-32.png"
convert +repage -crop 76x69+465+142 $infile main_screen_unselected_stack.png

# main_screen_water.png
infile="image-19.png"
convert +repage -crop 122x122+326+315 $infile main_screen_water.png

echo "copy files into place [y/n]?"
read ans

if [ "$ans" == "y" ]; then
  cp -v army_bonus.png figures/
  cp -v army_unit_info.png figures/
  cp -v buy_production.png figures/
  cp -v city_window.png figures/
  cp -v conquer_city.png figures/
  cp -v fight_city_one_vs_two.png figures/
  cp -v fighting_city_one_vs_two.png figures/
  cp -v fighting_one_vs_one.png figures/
  cp -v fight_one_vs_one.png figures/
  cp -v fight_order.png figures/
  cp -v fought_city_one_vs_two.png figures/
  cp -v fought_one_vs_one.png figures/
  cp -v main_screen_bag.png figures/
  cp -v main_screen_buttons.png figures/
  cp -v main_screen_center_on_stack_button.png figures/
  cp -v main_screen_city.png figures/
  cp -v main_screen_city_razed.png figures/
  cp -v main_screen_defend_stack_button.png figures/
  cp -v main_screen_deselect_stack_button.png figures/
  cp -v main_screen_diplomacy_button.png figures/
  cp -v main_screen_diplomacy_proposed_button.png figures/
  cp -v main_screen_end_turn_button.png figures/
  cp -v main_screen_forest.png figures/
  cp -v main_screen_hills.png figures/
  cp -v main_screen_mountains.png figures/
  cp -v main_screen_move_all_stacks_button.png figures/
  cp -v main_screen_move_bonus_flying.png figures/
  cp -v main_screen_move_bonus_forest.png figures/
  cp -v main_screen_move_bonus_hills_forest.png figures/
  cp -v main_screen_move_bonus_hills.png figures/
  cp -v main_screen_move_bonus_water.png figures/
  cp -v main_screen_move_stack_button.png figures/
  cp -v main_screen_park_stack_button.png figures/
  cp -v main_screen.png figures/
  cp -v main_screen_port.png figures/
  cp -v main_screen_road.png figures/
  cp -v main_screen_ruin.png figures/
  cp -v main_screen_selected_stack_flags.png figures/
  cp -v main_screen_selected_stack_one.png figures/
  cp -v main_screen_select_next_stack_button.png figures/
  cp -v main_screen_signpost.png figures/
  cp -v main_screen_stack_search_button.png figures/
  cp -v main_screen_stack_tray_4_units.png figures/
  cp -v main_screen_stack_tray_5_units.png figures/
  cp -v main_screen_stack_water_defend.png figures/
  cp -v main_screen_stat_icons.png figures/
  cp -v main_screen_swamp.png figures/
  cp -v main_screen_temple.png figures/
  cp -v main_screen_unselected_stack.png figures/
  cp -v main_screen_water.png figures/
  rm army_bonus.png army_unit_info.png buy_production.png city_window.png conquer_city.png fight_city_one_vs_two.png fighting_city_one_vs_two.png fighting_one_vs_one.png fight_one_vs_one.png fight_order.png fought_city_one_vs_two.png fought_one_vs_one.png main_screen_bag.png main_screen_buttons.png main_screen_center_on_stack_button.png main_screen_city.png main_screen_city_razed.png main_screen_defend_stack_button.png main_screen_deselect_stack_button.png main_screen_diplomacy_button.png main_screen_diplomacy_proposed_button.png main_screen_end_turn_button.png main_screen_forest.png main_screen_hills.png main_screen_mountains.png main_screen_move_all_stacks_button.png main_screen_move_bonus_flying.png main_screen_move_bonus_forest.png main_screen_move_bonus_hills_forest.png main_screen_move_bonus_hills.png main_screen_move_bonus_water.png main_screen_move_stack_button.png main_screen_park_stack_button.png main_screen.png main_screen_port.png main_screen_road.png main_screen_ruin.png main_screen_selected_stack_flags.png main_screen_selected_stack_one.png main_screen_select_next_stack_button.png main_screen_signpost.png main_screen_stack_search_button.png main_screen_stack_tray_4_units.png main_screen_stack_tray_5_units.png main_screen_stack_water_defend.png main_screen_stat_icons.png main_screen_swamp.png main_screen_temple.png main_screen_unselected_stack.png main_screen_water.png
fi

rm image-01.png image-02.png image-03.png image-04.png image-05.png \
  image-06.png image-07.png image-08.png image-09.png image-10.png \
  image-11.png image-12.png image-13.png image-14.png image-15.png \
  image-16.png image-17.png image-18.png image-19.png image-20.png \
  image-21.png image-22.png image-23.png image-24.png image-25.png \
  image-26.png image-27.png image-28.png image-29.png image-30.png \
  image-31.png image-32.png image-33.png image-34.png image-35.png \
  image-36.png image-37.png image-38.png image-29b.png
