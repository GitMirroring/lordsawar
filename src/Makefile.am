#   Copyright (C) 2008 Barry deFreese
#   Copyright (C) 2009, 2010, 2011, 2015, 2017, 2021 Ben Asselstine
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
SUBDIRS = editor gui . gls ghs utils
MAINTAINERCLEANFILES= Makefile.in

bin_PROGRAMS = lordsawar
noinst_LTLIBRARIES = liblordsawar.la liblordsawarnet.la liblordsawargfx.la \
		     liblordsawargamelist.la liblordsawargamehost.la

liblordsawar_la_SOURCES = action.cpp action.h AI_Analysis.cpp AI_Analysis.h \
        AI_Allocation.cpp AI_Allocation.h AI_Diplomacy.cpp AI_Diplomacy.h \
        ai_dummy.cpp ai_dummy.h ai_fast.cpp ai_fast.h \
        ai_smart.cpp ai_smart.h AICityInfo.cpp AICityInfo.h \
	armybase.cpp armybase.h armyproto.cpp armyproto.h armyprodbase.cpp \
        armyprodbase.h army.cpp army.h armysetlist.cpp armysetlist.h \
        armyset.cpp armyset.h armyprotobase.cpp armyprotobase.h \
        bridge.cpp bridge.h bridgelist.cpp bridgelist.h \
        city.cpp city.h citylist.cpp citylist.h set.h set.cpp setlist.h \
	citysetlist.cpp citysetlist.h cityset.cpp cityset.h \
        Configuration.cpp Configuration.h counter.cpp counter.h \
        CreateScenario.cpp CreateScenario.h \
	CreateScenarioRandomize.cpp CreateScenarioRandomize.h \
	fight.cpp fight.h File.cpp File.h FogMap.cpp FogMap.h \
        GameMap.cpp GameMap.h GameScenario.cpp GameScenario.h \
	GameScenarioOptions.cpp GameScenarioOptions.h \
	hero.cpp hero.h heroproto.cpp heroproto.h \
        herotemplates.cpp herotemplates.h history.cpp history.h \
	hero-strategy.cpp hero-strategy.h \
        Immovable.cpp Immovable.h Item.cpp Item.h Sage.cpp Sage.h \
	ItemProto.cpp ItemProto.h stacktile.cpp stacktile.h \
        stackreflist.cpp stackreflist.h Commentator.cpp Commentator.h \
        Itemlist.cpp Itemlist.h Location.cpp Location.h \
	LocationBox.cpp LocationBox.h LocationList.h \
        MapGenerator.cpp MapGenerator.h \
        maptile.cpp maptile.h Movable.cpp Movable.h \
        MoveResult.cpp MoveResult.h Named.cpp Named.h \
        NamedLocation.cpp NamedLocation.h network_player.cpp network_player.h \
	network-action.cpp network-action.h \
        network-history.cpp network-history.h \
        NextTurn.cpp NextTurn.h NextTurnHotseat.cpp NextTurnHotseat.h \
	NextTurnNetworked.cpp NextTurnNetworked.h \
        OwnerId.cpp OwnerId.h \
        path.cpp path.h PathCalculator.cpp PathCalculator.h \
        RoadPathCalculator.cpp RoadPathCalculator.h \
        player.cpp player.h playerlist.cpp playerlist.h \
        port.cpp port.h portlist.cpp portlist.h \
        Positioned.cpp Positioned.h QCityOccupy.cpp QCityOccupy.h \
        QCityRaze.cpp QCityRaze.h QCitySack.cpp QCitySack.h \
        QEnemyArmies.cpp QEnemyArmies.h QEnemyArmytype.cpp QEnemyArmytype.h \
	QKillHero.cpp QKillHero.h QPillageGold.cpp QPillageGold.h \
        Quest.cpp Quest.h QuestsManager.cpp QuestsManager.h \
        real_player.cpp real_player.h Renamable.h \
        reward.h reward.cpp rewardlist.h rewardlist.cpp \
	road.cpp road.h roadlist.cpp roadlist.h \
	stone.cpp stone.h stonelist.cpp stonelist.h \
        ruin.cpp ruin.h ruinlist.cpp ruinlist.h \
	tartan.cpp tartan.h \
        shield.cpp shield.h shieldset.cpp shieldset.h \
        shieldsetlist.cpp shieldsetlist.h shieldstyle.cpp shieldstyle.h \
        signpost.cpp signpost.h signpostlist.cpp signpostlist.h \
	stack.cpp stack.h stacklist.cpp stacklist.h  \
        temple.cpp temple.h templelist.cpp templelist.h \
        Threat.cpp Threat.h Threatlist.cpp Threatlist.h \
        Tile.cpp Tile.h tileset.cpp tileset.h tilesetlist.cpp tilesetlist.h \
        tilestyle.cpp tilestyle.h tilestyleset.cpp tilestyleset.h \
        timing.cpp timing.h UniquelyIdentified.cpp UniquelyIdentified.h \
        vectoredunit.cpp vectoredunit.h \
        vectoredunitlist.cpp vectoredunitlist.h xmlhelper.cpp xmlhelper.h \
        tarhelper.cpp tarhelper.h \
        rectangle.h vector.h ucompose.hpp boxcompose.h \
	SightMap.cpp SightMap.h SmallTile.cpp SmallTile.h \
	Triumphs.cpp Triumphs.h Backpack.cpp Backpack.h \
	MapBackpack.cpp MapBackpack.h prodslot.cpp prodslot.h \
	prodslotlist.cpp prodslotlist.h namelist.cpp namelist.h \
	PixMask.cpp PixMask.h \
	PixMaskCache.h ImageCache.cpp ImageCache.h \
	file-compat.cpp file-compat.h \
	rnd.cpp rnd.h game-actionlist.cpp game-actionlist.h \
	turn-actionlist.cpp turn-actionlist.h \
	ScenarioMedia.cpp ScenarioMedia.h \
	tarfile.cpp tarfile.h \
	TarFileMaskedImage.cpp TarFileMaskedImage.h \
	TarFileImage.cpp TarFileImage.h \
	snd.cpp snd.h \
	scenario-list.cpp scenario-list.h \
	scenario-details.cpp scenario-details.h \
	keeper.cpp keeper.h \
	character.cpp character.h

liblordsawarnet_la_SOURCES = \
	game-client.cpp game-client.h \
        game-client-decoder.cpp game-client-decoder.h \
	game-server.cpp game-server.h game-station.cpp game-station.h \
	network-connection.cpp chat-client.cpp chat-client.h \
        network-connection.h network-common.h \
        network-server.cpp network-server.h \
	connection-manager.cpp connection-manager.h \
	profile.cpp profile.h \
       	profilelist.cpp profilelist.h 

liblordsawargamelist_la_SOURCES= \
        recently-played-game.cpp recently-played-game.h \
        recently-played-game-list.cpp recently-played-game-list.h \
	network-gls-common.h \
	gamelist-client.cpp gamelist-client.h \
	advertised-game.cpp advertised-game.h \
	gamelist.cpp gamelist.h \
	hosted-game.cpp hosted-game.h

liblordsawargamehost_la_SOURCES= \
	network-ghs-common.h \
	gamehost-client.cpp gamehost-client.h 

liblordsawargfx_la_SOURCES = \
	bigmap.cpp bigmap.h citymap.cpp citymap.h  \
        historymap.cpp historymap.h heromap.cpp heromap.h \
	heroesmap.cpp heroesmap.h itemmap.cpp itemmap.h \
	ruinmap.cpp ruinmap.h questmap.cpp questmap.h \
	vectormap.cpp vectormap.h overviewmap.cpp overviewmap.h \
        smallmap.cpp smallmap.h \
	MapRenderer.cpp MapRenderer.h  \
	input-events.h map-tip-position.h  \
	select-city-map.cpp select-city-map.h


lordsawar_SOURCES = armymap.cpp armymap.h \
	callback-enums.h defs.h game-parameters.h \
	sidebar-stats.h game.cpp game.h gamebigmap.cpp gamebigmap.h \
	main.cpp 

if WINDOWS_OS

lordsawar_SOURCES += ${top_srcdir}/dat/lordsawar.rc
.rc.o:
	i686-w64-mingw32-windres -I .. -i $< $@
SUFFIXES=.rc

endif

localedir = $(datadir)/locale
DEFS = -DLOCALEDIR=\"$(localedir)\" @DEFS@

liblordsawargamelist_la_LIBADD = liblordsawar.la liblordsawarnet.la

liblordsawargamehost_la_LIBADD = liblordsawar.la liblordsawarnet.la \
				 liblordsawargamelist.la

liblordsawar_la_LIBADD = gui/liblwgui.la \
    @LTLIBOBJS@

lordsawar_LDFLAGS =
lordsawar_LDADD = liblordsawar.la \
    liblordsawarnet.la \
    liblordsawargfx.la \
    liblordsawargamelist.la \
    liblordsawargamehost.la \
    editor/libeditor.la \
    editor/libshieldsetwindow.la \
    editor/libarmysetwindow.la \
    editor/libcitysetwindow.la \
    editor/libtilesetwindow.la \
    editor/libeditutil.la \
    $(GSTREAMER_LIBS) \
    $(GTKMM_LIBS) \
    $(XMLPP_LIBS) \
    $(XSLT_LIBS) \
    $(ARCHIVE_LIBS) \
    $(LIBSIGC_LIBS) \
    -lz \
    gui/liblwgui.la

lordsawar_DEPENDENCIES = \
  editor/libeditor.la \
  editor/libshieldsetwindow.la \
  editor/libarmysetwindow.la \
  editor/libcitysetwindow.la \
  editor/libtilesetwindow.la \
  editor/libeditutil.la \
  liblordsawar.la \
  liblordsawargfx.la \
  liblordsawargamelist.la \
  liblordsawargamehost.la \
  liblordsawarnet.la \
  gui/liblwgui.la

AM_CXXFLAGS = -Wall -Wshadow -Wextra -pedantic -Wno-deprecated-declarations \
	      -Wno-deprecated-copy -std=c++11
AM_CPPFLAGS = $(GSTREAMER_CFLAGS) \
    $(GTKMM_CFLAGS) \
    $(XMLPP_CFLAGS) \
    $(XSLT_CFLAGS) \
    $(ARCHIVE_CFLAGS) \
    $(LIBSIGC_CFLAGS) \
    -DLORDSAWAR_DATADIR='"$(LORDSAWAR_DATADIR)"' \
    -I$(top_srcdir)
