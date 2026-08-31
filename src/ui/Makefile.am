#   Copyright (C) 2007 Ole Laursen
#   Copyright (C) 2007, 2008, 2009, 2011, 2015 Ben Asselstine
#   Copyright (C) 2008 Barry deFreese
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
MAINTAINERCLEANFILES= Makefile.in

noinst_LTLIBRARIES = liblwgui.la

liblwgui_la_SOURCES = \
    driver.cpp driver.h \
    fight-order-dialog.cpp fight-order-dialog.h \
    hero-levels-dialog.cpp hero-levels-dialog.h \
    army-bonus-dialog.cpp army-bonus-dialog.h \
    item-bonus-dialog.cpp item-bonus-dialog.h \
    city-info-tip.cpp city-info-tip.h \
    army-info-tip.cpp army-info-tip.h \
    stack-info-tip.cpp stack-info-tip.h \
    army-gains-level-dialog.cpp army-gains-level-dialog.h \
    buy-production-dialog.cpp buy-production-dialog.h \
    city-window.cpp city-window.h \
    destination-dialog.cpp destination-dialog.h \
    fight-window.cpp fight-window.h \
    game-preferences-dialog.cpp game-preferences-dialog.h \
    characters-dialog.cpp characters-dialog.h \
    game-options-dialog.cpp game-options-dialog.h \
    game-window.cpp game-window.h \
    hero-dialog.cpp hero-dialog.h \
    hero-offer-dialog.cpp hero-offer-dialog.h \
    surrender-dialog.cpp surrender-dialog.h \
    surrender-refused-dialog.cpp surrender-refused-dialog.h \
    sage-dialog.cpp sage-dialog.h \
    ruin-rewarded-dialog.cpp ruin-rewarded-dialog.h \
    ruin-report-dialog.cpp ruin-report-dialog.h \
    item-report-dialog.cpp item-report-dialog.h \
    history-report-dialog.cpp history-report-dialog.h \
    report-dialog.cpp report-dialog.h \
    triumphs-dialog.cpp triumphs-dialog.h \
    diplomacy-dialog.cpp diplomacy-dialog.h \
    diplomacy-report-dialog.cpp diplomacy-report-dialog.h \
    load-scenario-dialog.cpp load-scenario-dialog.h \
    quest-assigned-dialog.cpp quest-assigned-dialog.h \
    quest-completed-dialog.cpp quest-completed-dialog.h \
    bar-chart.cpp bar-chart.h \
    line-chart.cpp line-chart.h \
    main.cpp main.h \
    quest-report-dialog.cpp quest-report-dialog.h \
    preferences-dialog.cpp preferences-dialog.h \
    main-preferences-dialog.cpp main-preferences-dialog.h \
    stack-info-dialog.cpp stack-info-dialog.h \
    splash-window.cpp splash-window.h \
    image-helpers.cpp image-helpers.h \
    input-helpers.h \
    timed-message-dialog.cpp timed-message-dialog.h \
    game-lobby-dialog.cpp game-lobby-dialog.h \
    quick-help-window.cpp quick-help-window.h \
    network-game-selector-dialog.cpp network-game-selector-dialog.h \
    new-network-game-dialog.cpp new-network-game-dialog.h \
    new-profile-dialog.cpp new-profile-dialog.h \
    new-network-game-download-window.cpp new-network-game-download-window.h \
    new-random-map-dialog.cpp new-random-map-dialog.h \
    use-item-dialog.cpp use-item-dialog.h \
    use-item-on-player-dialog.cpp use-item-on-player-dialog.h \
    use-item-on-city-dialog.cpp use-item-on-city-dialog.h \
    game-button-box.cpp game-button-box.h \
    status-box.cpp status-box.h \
    stack-tile-box.cpp stack-tile-box.h \
    stack-army-button.cpp stack-army-button.h \
    lw-dialog.cpp lw-dialog.h \
    builder-cache.cpp builder-cache.h \
    tartan-progress-bar.cpp tartan-progress-bar.h \
    font-size.cpp font-size.h \
    load-progress-window.cpp load-progress-window.h

liblwgui_la_LIBADD = @LTLIBOBJS@

AM_CPPFLAGS = \
   $(GTKMM_CFLAGS) \
   $(XMLPP_CFLAGS) \
   $(GSTREAMER_CFLAGS) \
   -I$(top_srcdir)/src

AM_CXXFLAGS = -Wall -Wshadow -Wextra -pedantic -Wno-deprecated-declarations \
	      -Wno-deprecated-copy -std=c++11
