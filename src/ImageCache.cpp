// Copyright (C) 2003, 2004, 2005, 2006, 2007 Ulf Lorenz
// Copyright (C) 2004, 2005, 2006 Andrea Paternesi
// Copyright (C) 2006, 2007, 2008, 2009, 2010, 2011, 2014, 2015, 2016, 2020,
// 2021 Ben Asselstine
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU Library General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 
//  02110-1301, USA.
#include <sigc++/functors/mem_fun.h>
#include <iostream>
#include "ImageCache.h"
#include "gui/image-helpers.h"
#include "playerlist.h"
#include "stack.h"
#include "player.h"
#include "tilesetlist.h"
#include "GameMap.h"
#include "armysetlist.h"
#include "shield.h"
#include "citysetlist.h"
#include "shieldsetlist.h"
#include "Configuration.h"
#include "city.h"
#include "Tile.h"
#include "ruin.h"
#include "temple.h"
#include "road.h"
#include "bridge.h"
#include "FogMap.h"
#include "shieldset.h"
#include "ScenarioMedia.h"
#include "TarFileMaskedImage.h"
#include "TarFileImage.h"

ImageCache* ImageCache::s_instance = 0;

ImageCache* ImageCache::getInstance()
{
  if (!s_instance)
    s_instance = new ImageCache();

  return s_instance;
}

void ImageCache::deleteInstance()
{
  if (!s_instance)
    return;

  delete s_instance;
  s_instance = NULL;
}

ImageCache::ImageCache(const ImageCache &c)
 : d_cachesize(c.d_cachesize), selectorcache(c.selectorcache),
    armycache(c.armycache), flagcache(c.flagcache),
    circledarmycache(c.circledarmycache), circledshipcache(c.circledshipcache),
    circledstandardcache(c.circledstandardcache),
    tilecache(c.tilecache), citycache(c.citycache), towercache(c.towercache),
    templecache(c.templecache), ruincache(c.ruincache),
    diplomacycache(c.diplomacycache), roadcache(c.roadcache),
    fogcache(c.fogcache), bridgecache(c.bridgecache),
    cursorcache(c.cursorcache), shieldcache(c.shieldcache),
    prodshieldcache(c.prodshieldcache), movebonuscache(c.movebonuscache),
    shipcache(c.shipcache), plantedstandardcache(c.plantedstandardcache),
    portcache(c.portcache), signpostcache(c.signpostcache),
    bagcache(c.bagcache), explosioncache(c.explosioncache),
    newlevelcache(c.newlevelcache),
    defaulttilestylecache(c.defaulttilestylecache), tartancache(c.tartancache),
    emptytartancache(c.emptytartancache), statuscache(c.statuscache),
    gamebuttoncache(c.gamebuttoncache), dialogcache(c.dialogcache),
    medalcache(c.medalcache)
{
  for (guint32 i = 0; i < 2; i++)
    for (guint32 j = 0; j < DIPLOMACY_TYPES; j++)
      d_diplomacy[i][j] = c.d_diplomacy[i][j]->copy ();
  for (guint32 i = 0; i < CURSOR_TYPES; i++)
    d_cursor[i] = c.d_cursor[i]->copy ();
  for (guint32 i = 0; i < PRODUCTION_SHIELD_TYPES; i++)
    d_prodshield[i] = c.d_prodshield[i]->copy ();

  d_hero_newlevel[0] = new TarFileMaskedImage (*c.d_hero_newlevel[0]);
  d_hero_newlevel[1] = new TarFileMaskedImage (*c.d_hero_newlevel[1]);

  for (guint32 i = 0; i < DEFAULT_TILESTYLE_TYPES; i++)
    d_default_tilestyles[i] = c.d_default_tilestyles[i]->copy ();

  d_smallruinedcity = c.d_smallruinedcity->copy ();
  d_smallhero = c.d_smallhero->copy ();
  d_smallbag = c.d_smallbag->copy ();
  d_smallinactivehero = c.d_smallinactivehero->copy ();
  d_small_ruin_unexplored = c.d_small_ruin_unexplored->copy ();
  d_small_stronghold_unexplored = c.d_small_stronghold_unexplored->copy ();
  d_small_ruin_unexplored = c.d_small_ruin_unexplored->copy ();
  d_small_ruin_explored = c.d_small_ruin_explored->copy ();
  d_small_temple = c.d_small_temple->copy ();

  for (guint32 i = 0; i < NUM_WAYPOINTS; i++)
    d_waypoint[i] = c.d_waypoint[i]->copy ();
  for (guint32 i = 0; i < NUM_GAME_BUTTON_IMAGES; i++)
    d_gamebuttons[i] = c.d_gamebuttons[i]->copy ();

  d_next_turn = new TarFileImage (*c.d_next_turn);
  d_city_defeated = new TarFileImage (*c.d_city_defeated);
  d_winning = new TarFileImage (*c.d_winning);
  d_hero[0] = new TarFileImage (*c.d_hero[0]);
  d_hero[1] = new TarFileImage (*c.d_hero[1]);
  d_ruin_success = new TarFileImage (*c.d_ruin_success);
  d_ruin_defeat = new TarFileImage (*c.d_ruin_defeat);
  d_parley_offered = new TarFileImage (*c.d_parley_offered);
  d_parley_refused = new TarFileImage (*c.d_parley_refused);
  d_medal[0] = new TarFileImage (*c.d_medal[0]);
  d_medal[1] = new TarFileImage (*c.d_medal[1]);
  d_commentator = new TarFileImage (*c.d_commentator);
}

ImageCache::ImageCache()
 : d_cachesize(0),
    selectorcache((sigc::ptr_fun(&SelectorPixMaskCacheItem::generate))),
    armycache((sigc::ptr_fun(&ArmyPixMaskCacheItem::generate))),
    flagcache((sigc::ptr_fun(&FlagPixMaskCacheItem::generate))),
    circledarmycache((sigc::ptr_fun(&CircledArmyPixMaskCacheItem::generate))),
    circledshipcache((sigc::ptr_fun(&CircledShipPixMaskCacheItem::generate))),
    circledstandardcache((sigc::ptr_fun(&CircledStandardPixMaskCacheItem::generate))),
    tilecache((sigc::ptr_fun(&TilePixMaskCacheItem::generate))),
    citycache((sigc::ptr_fun(&CityPixMaskCacheItem::generate))),
    towercache((sigc::ptr_fun(&TowerPixMaskCacheItem::generate))),
    templecache((sigc::ptr_fun(&TemplePixMaskCacheItem::generate))),
    ruincache((sigc::ptr_fun(&RuinPixMaskCacheItem::generate))),
    diplomacycache((sigc::ptr_fun(&DiplomacyPixMaskCacheItem::generate))),
    roadcache((sigc::ptr_fun(&RoadPixMaskCacheItem::generate))),
    fogcache((sigc::ptr_fun(&FogPixMaskCacheItem::generate))),
    bridgecache((sigc::ptr_fun(&BridgePixMaskCacheItem::generate))),
    cursorcache((sigc::ptr_fun(&CursorPixMaskCacheItem::generate))),
    shieldcache((sigc::ptr_fun(&ShieldPixMaskCacheItem::generate))),
    prodshieldcache((sigc::ptr_fun(&ProdShieldPixMaskCacheItem::generate))),
    movebonuscache((sigc::ptr_fun(&MoveBonusPixMaskCacheItem::generate))),
    shipcache((sigc::ptr_fun(&ShipPixMaskCacheItem::generate))),
    plantedstandardcache((sigc::ptr_fun(&PlantedStandardPixMaskCacheItem::generate))),
    portcache((sigc::ptr_fun(&PortPixMaskCacheItem::generate))),
    signpostcache((sigc::ptr_fun(&SignpostPixMaskCacheItem::generate))),
    bagcache((sigc::ptr_fun(&BagPixMaskCacheItem::generate))),
    explosioncache((sigc::ptr_fun(&ExplosionPixMaskCacheItem::generate))),
    newlevelcache((sigc::ptr_fun(&NewLevelPixMaskCacheItem::generate))),
    defaulttilestylecache((sigc::ptr_fun(&DefaultTileStylePixMaskCacheItem::generate))),
    tartancache((sigc::ptr_fun(&TartanPixMaskCacheItem::generate))),
    emptytartancache((sigc::ptr_fun(&EmptyTartanPixMaskCacheItem::generate))),
    statuscache((sigc::ptr_fun(&StatusPixMaskCacheItem::generate))),
    gamebuttoncache((sigc::ptr_fun(&GameButtonPixMaskCacheItem::generate))),
    dialogcache((sigc::ptr_fun(&DialogPixMaskCacheItem::generate))),
    medalcache((sigc::ptr_fun(&MedalPixMaskCacheItem::generate)))
{
    d_hero_newlevel[0] =
      new TarFileMaskedImage (TarFileMaskedImage::HORIZONTAL_MASK,
                              PixMask::DIMENSION_ANY);
    d_hero_newlevel[1] =
      new TarFileMaskedImage (TarFileMaskedImage::HORIZONTAL_MASK,
                              PixMask::DIMENSION_ANY);

    loadDiplomacyImages();
    loadCursorImages();
    loadProdShieldImages();
    d_smallruinedcity = loadMiscImage("smallruinedcity.png");
    d_smallhero = loadMiscImage("hero.png");
    d_smallbag = loadMiscImage("bag.png");
    d_smallinactivehero = loadMiscImage("hero-inactive.png");
    d_small_ruin_unexplored = loadMiscImage("smallunexploredruin.png");
    d_small_stronghold_unexplored =
      loadMiscImage("smallunexploredstronghold.png");
    d_small_ruin_explored = loadMiscImage("smallexploredruin.png");
    d_small_temple = loadMiscImage("smalltemple.png");
    loadNewLevelImages();
    loadDefaultTileStyleImages();
    loadWaypointImages(); //only for game.  not for editors.
    loadGameButtonImages(); //only for game.  not for editors.

    d_next_turn = new TarFileImage (1, PixMask::DIMENSION_ANY);
    d_next_turn->loadFromFile
      (ScenarioMedia::getDefaultNextTurnImageFilename ());
    d_next_turn->instantiateImages ();

    d_city_defeated = new TarFileImage (1, PixMask::DIMENSION_ANY);
    d_city_defeated->loadFromFile
      (ScenarioMedia::getDefaultCityDefeatedImageFilename ());
    d_city_defeated->instantiateImages ();

    d_winning = new TarFileImage (1, PixMask::DIMENSION_ANY);
    d_winning->loadFromFile
      (ScenarioMedia::getDefaultWinningImageFilename ());
    d_winning->instantiateImages ();

    d_hero[0] = new TarFileImage (1, PixMask::DIMENSION_ANY);
    d_hero[0]->loadFromFile
      (ScenarioMedia::getDefaultMaleHeroImageFilename ());
    d_hero[0]->instantiateImages ();

    d_hero[1] = new TarFileImage (1, PixMask::DIMENSION_ANY);
    d_hero[1]->loadFromFile
      (ScenarioMedia::getDefaultFemaleHeroImageFilename ());
    d_hero[1]->instantiateImages ();

    d_ruin_success = new TarFileImage (1, PixMask::DIMENSION_ANY);
    d_ruin_success->loadFromFile
      (ScenarioMedia::getDefaultRuinSuccessImageFilename ());
    d_ruin_success->instantiateImages ();

    d_ruin_defeat = new TarFileImage (1, PixMask::DIMENSION_ANY);
    d_ruin_defeat->loadFromFile
      (ScenarioMedia::getDefaultRuinDefeatImageFilename ());
    d_ruin_defeat->instantiateImages ();

    d_parley_offered = new TarFileImage (1, PixMask::DIMENSION_ANY);
    d_parley_offered->loadFromFile
      (ScenarioMedia::getDefaultParleyOfferedImageFilename ());
    d_parley_offered->instantiateImages ();

    d_parley_refused = new TarFileImage (1, PixMask::DIMENSION_ANY);
    d_parley_refused->loadFromFile
      (ScenarioMedia::getDefaultParleyRefusedImageFilename ());
    d_parley_refused->instantiateImages ();

    d_medal[0] =
      new TarFileImage (MEDAL_TYPES,
                        PixMask::DIMENSION_WIDTH_IS_MULTIPLE_OF_HEIGHT);
    d_medal[0]->loadFromFile
      (ScenarioMedia::getDefaultSmallMedalsImageFilename ());
    d_medal[0]->instantiateImages ();

    d_medal[1] =
      new TarFileImage (MEDAL_TYPES,
                        PixMask::DIMENSION_WIDTH_IS_MULTIPLE_OF_HEIGHT);
    d_medal[1]->loadFromFile
      (ScenarioMedia::getDefaultBigMedalsImageFilename ());
    d_medal[1]->instantiateImages ();

    d_commentator = new TarFileImage (1, PixMask::DIMENSION_ANY);
    d_commentator->loadFromFile
      (ScenarioMedia::getDefaultCommentatorImageFilename ());
    d_commentator->instantiateImages ();
}

bool ImageCache::loadDiplomacyImages()
{
  bool broken = false;
  int ts = 30;
  std::vector<PixMask*> diplomacy;
  diplomacy = disassemble_row(File::getVariousFile("diplomacy-small.png"),
                              DIPLOMACY_TYPES, broken);
  if (broken)
    return false;
  for (unsigned int i = 0; i < DIPLOMACY_TYPES ; i++)
    {
      if (diplomacy[i]->get_width() != ts)
	PixMask::scale(diplomacy[i], ts, ts);
      d_diplomacy[0][i] = diplomacy[i];
    }

  ts = 50;
  diplomacy = disassemble_row(File::getVariousFile("diplomacy-large.png"),
                              DIPLOMACY_TYPES, broken);
  if (broken)
    return false;
  for (unsigned int i = 0; i < DIPLOMACY_TYPES ; i++)
    {
      if (diplomacy[i]->get_width() != ts)
	PixMask::scale(diplomacy[i], ts, ts);
      d_diplomacy[1][i] = diplomacy[i];
    }
  return true;
}

bool ImageCache::loadCursorImages()
{
  bool broken = false;
  int ts = 16;

  // load the cursor pictures
  std::vector<PixMask*> cursor;
  cursor = disassemble_row(File::getVariousFile("cursors.png"),
                           CURSOR_TYPES, broken);
  if (broken)
    return false;
  for (unsigned int i = 0; i < CURSOR_TYPES ; i++)
    {
      if (cursor[i]->get_width() != ts)
	PixMask::scale(cursor[i], ts, ts);
      d_cursor[i] = cursor[i];
    }
  return true;
}

bool ImageCache::loadProdShieldImages()
{
  bool broken = false;
  //load the production shieldset
  std::vector<PixMask*> prodshield;
  prodshield = disassemble_row
    (File::getVariousFile("prodshieldset.png"), PRODUCTION_SHIELD_TYPES,
     broken);
  if (broken)
    return false;
  for (unsigned int i = 0; i < PRODUCTION_SHIELD_TYPES; i++)
    d_prodshield[i] = prodshield[i];
  prodshield.clear();
  return true;
}

bool ImageCache::loadNewLevelImages()
{
  Glib::ustring filename =
    ScenarioMedia::getDefaultHeroNewLevelMaleImageFilename();
  bool broken = d_hero_newlevel[0]->loadFromFile (filename);
  if (!broken)
    {
      d_hero_newlevel[0]->instantiateImages ();

      filename = ScenarioMedia::getDefaultHeroNewLevelFemaleImageFilename();
      broken = d_hero_newlevel[1]->loadFromFile (filename);
      if (!broken)
        d_hero_newlevel[1]->instantiateImages ();
    }

  return broken;
}

bool ImageCache::loadDefaultTileStyleImages()
{
  bool broken = false;
  std::vector<PixMask*> images =
    disassemble_row(File::getVariousFile("tilestyles.png"),
                    DEFAULT_TILESTYLE_TYPES, broken);
  if (broken)
    return false;
  for (unsigned int i = 0; i < DEFAULT_TILESTYLE_TYPES; i++)
    d_default_tilestyles[i] = images[i];
  return true;
}

bool ImageCache::loadWaypointImages()
{
  bool broken = false;
  std::vector<PixMask*> images = disassemble_row
    (File::getVariousFile("waypoints.png"), NUM_WAYPOINTS, broken);
  if (broken)
    return false;
  for (unsigned int i = 0; i < NUM_WAYPOINTS; i++)
    d_waypoint[i] = images[i];
  return true;
}

bool ImageCache::loadGameButtonImages()
{
  bool broken = false;
  std::vector<PixMask*> images = disassemble_row
    (File::getVariousFile("buttons.png"), NUM_GAME_BUTTON_IMAGES, broken);
  if (broken)
    return false;
  int w = 0, h = 0;
  Gtk::IconSize::lookup(Gtk::IconSize(Gtk::ICON_SIZE_BUTTON), w, h);
  for (unsigned int i = 0; i < NUM_GAME_BUTTON_IMAGES; i++)
    PixMask::scale(images[i], w, h);
  for (unsigned int i = 0; i < NUM_GAME_BUTTON_IMAGES; i++)
    d_gamebuttons[i] = images[i];

  images.clear();
  return true;
}

PixMask* ImageCache::loadMiscImage(Glib::ustring pngfile)
{
  bool broken = false;
  return PixMask::create(File::getVariousFile(pngfile), broken);
}

ImageCache::~ImageCache()
{
  delete d_hero_newlevel[0];
  delete d_hero_newlevel[1];

  for (unsigned int i = 0; i < DIPLOMACY_TYPES;i++)
    {
      delete d_diplomacy[0][i];
      delete d_diplomacy[1][i];
    }

  for (unsigned int i = 0; i < CURSOR_TYPES;i++)
    delete d_cursor[i];

  for (unsigned int i = 0; i < PRODUCTION_SHIELD_TYPES; i++)
    delete d_prodshield[i];

  for (unsigned int i = 0; i < DEFAULT_TILESTYLE_TYPES; i++)
    delete d_default_tilestyles[i];

  delete d_smallruinedcity;
  delete d_smallhero;
  delete d_smallbag;
  delete d_smallinactivehero;
  delete d_small_temple;
  delete d_small_ruin_unexplored;
  delete d_small_stronghold_unexplored;
  delete d_small_ruin_explored;

  for (unsigned int i = 0; i < NUM_WAYPOINTS; i++)
    delete d_waypoint[i];

  for (unsigned int i = 0; i < NUM_GAME_BUTTON_IMAGES; i++)
    delete d_gamebuttons[i];

  delete d_next_turn;
  delete d_city_defeated;
  delete d_winning;
  delete d_hero[0];
  delete d_hero[1];
  delete d_ruin_success;
  delete d_ruin_defeat;
  delete d_parley_offered;
  delete d_parley_refused;
  delete d_medal[0];
  delete d_medal[1];
  delete d_commentator;
  reset();
}

void ImageCache::reset()
{
  selectorcache.reset();
  flagcache.reset();
  armycache.reset();
  circledarmycache.reset();
  circledshipcache.reset();
  circledstandardcache.reset();
  tilecache.reset();
  citycache.reset();
  towercache.reset();
  templecache.reset();
  ruincache.reset();
  diplomacycache.reset();
  roadcache.reset();
  fogcache.reset();
  bridgecache.reset();
  cursorcache.reset();
  shieldcache.reset();
  prodshieldcache.reset();
  movebonuscache.reset();
  shipcache.reset();
  plantedstandardcache.reset();
  portcache.reset();
  signpostcache.reset();
  bagcache.reset();
  explosioncache.reset();
  newlevelcache.reset();
  defaulttilestylecache.reset();
  tartancache.reset();
  emptytartancache.reset();
  statuscache.reset();
  gamebuttoncache.reset();
  dialogcache.reset();
  medalcache.reset();

  d_cachesize = 0;
  return;
}

void ImageCache::checkPictures()
{
  guint32 maxcache = Configuration::s_cacheSize;
  if (maxcache < MINIMUM_CACHE_SIZE)
    maxcache = MINIMUM_CACHE_SIZE;

  if (d_cachesize < maxcache)
    return;

  // Now the cache size has been exceeded. We try to guarantee the values
  // given above and reduce the number of images. Let us start with the
  // cities

  unsigned int num_players = Playerlist::getInstance()->countPlayersAlive();
  if (armycache.size() >= 15 * num_players)
    {
      d_cachesize -= armycache.discardHalf();
      if (d_cachesize < maxcache)
        return;
    }

  if (circledarmycache.size() >= 15 * num_players)
    {
      d_cachesize -= circledarmycache.discardHalf();
      if (d_cachesize < maxcache)
        return;
    }

  if (circledshipcache.size() >= num_players)
    {
      d_cachesize -= circledshipcache.discardHalf();
      if (d_cachesize < maxcache)
        return;
    }

  if (circledstandardcache.size() >= num_players)
    {
      d_cachesize -= circledstandardcache.discardHalf();
      if (d_cachesize < maxcache)
        return;
    }

  if (flagcache.size() >= num_players * MAX_STACK_SIZE)
    {
      d_cachesize -= flagcache.discardHalf();
      if (d_cachesize < maxcache)
        return;
    }

  if (citycache.size() >= num_players)
    {
      d_cachesize -= citycache.discardHalf();
      if (d_cachesize < maxcache)
        return;
    }

  if (roadcache.size() >= ROAD_TYPES / 2)
    {
      d_cachesize -= roadcache.discardHalf();
      if (d_cachesize < maxcache)
        return;
    }

  if (fogcache.size() >= FOG_TYPES / 2)
    {
      d_cachesize -= fogcache.discardHalf();
      if (d_cachesize < maxcache)
        return;
    }

  if (bridgecache.size() >= BRIDGE_TYPES)
    {
      d_cachesize -= bridgecache.discardHalf();
      if (d_cachesize < maxcache)
        return;
    }

  if (shipcache.size() >= MAX_PLAYERS)
    {
      d_cachesize -= shipcache.discardHalf();
      if (d_cachesize < maxcache)
        return;
    }

  if (plantedstandardcache.size() >= num_players)
    {
      d_cachesize -= plantedstandardcache.discardHalf();
      if (d_cachesize < maxcache)
        return;
    }

  if (portcache.size() > 1)
    {
      d_cachesize -= portcache.discardHalf();
      if (d_cachesize < maxcache)
        return;
    }

  if (towercache.size() >= num_players)
    {
      d_cachesize -= towercache.discardHalf();
      if (d_cachesize < maxcache)
        return;
    }

  if (templecache.size() >= TEMPLE_TYPES)
    {
      d_cachesize -= templecache.discardHalf();
      if (d_cachesize < maxcache)
        return;
    }

  if (ruincache.size() >= RUIN_TYPES)
    {
      d_cachesize -= ruincache.discardHalf();
      if (d_cachesize < maxcache)
        return;
    }

  if (signpostcache.size() > 1)
    {
      d_cachesize -= signpostcache.discardHalf();
      if (d_cachesize < maxcache)
        return;
    }

  if (bagcache.size() > 1)
    {
      d_cachesize -= bagcache.discardHalf();
      if (d_cachesize < maxcache)
        return;
    }

  if (selectorcache.size() >= num_players * MAX_STACK_SIZE)
    {
      d_cachesize -= selectorcache.discardHalf();
      if (d_cachesize < maxcache)
        return;
    }

  if (tilecache.size() >= 15*15)
    {
      d_cachesize -= tilecache.discardHalf();
      if (d_cachesize < maxcache)
        return;
    }

  if (diplomacycache.size() >= DIPLOMACY_TYPES)
    {
      d_cachesize -= diplomacycache.discardHalf();
      if (d_cachesize < maxcache)
        return;
    }

  if (cursorcache.size() >= CURSOR_TYPES)
    {
      d_cachesize -= cursorcache.discardHalf();
      if (d_cachesize < maxcache)
        return;
    }

  if (shieldcache.size() >= num_players * 3)
    {
      d_cachesize -= shieldcache.discardHalf();
      if (d_cachesize < maxcache)
        return;
    }

  if (prodshieldcache.size() >= PRODUCTION_SHIELD_TYPES)
    {
      d_cachesize -= prodshieldcache.discardHalf();
      if (d_cachesize < maxcache)
        return;
    }

  if (movebonuscache.size() >= 8) //around half of 17 different combinations
    {
      d_cachesize -= movebonuscache.discardHalf();
      if (d_cachesize < maxcache)
        return;
    }

  if (explosioncache.size() > 1)
    {
      d_cachesize -= explosioncache.discardHalf();
      if (d_cachesize < maxcache)
        return;
    }

  if (newlevelcache.size() > 2)
    {
      d_cachesize -= newlevelcache.discardHalf();
      if (d_cachesize < maxcache)
        return;
    }

  if (defaulttilestylecache.size() >= DEFAULT_TILESTYLE_TYPES)
    {
      d_cachesize -= defaulttilestylecache.discardHalf();
      if (d_cachesize < maxcache)
        return;
    }

  if (tartancache.size() >= MAX_PLAYERS + 1)
    {
      d_cachesize -= tartancache.discardHalf();
      if (d_cachesize < maxcache)
        return;
    }

  if (emptytartancache.size() >= MAX_PLAYERS + 1)
    {
      d_cachesize -= emptytartancache.discardHalf();
      if (d_cachesize < maxcache)
        return;
    }

  if (statuscache.size() >= 6)
    {
      d_cachesize -= statuscache.discardHalf();
      if (d_cachesize < maxcache)
        return;
    }

  if (gamebuttoncache.size() >= 12)
    {
      d_cachesize -= gamebuttoncache.discardHalf();
      if (d_cachesize < maxcache)
        return;
    }

  if (dialogcache.size() >= 10)
    {
      d_cachesize -= dialogcache.discardHalf();
      if (d_cachesize < maxcache)
        return;
    }

  if (medalcache.size() >= 6)
    {
      d_cachesize -= medalcache.discardHalf();
      if (d_cachesize < maxcache)
        return;
    }
}

PixMask* ImageCache::getSelectorPic(guint32 type, guint32 frame,
                                    const Player *p)
{
  return getSelectorPic(type, frame, p,
                        GameMap::getInstance()->getTilesetId());
}

PixMask* ImageCache::getSelectorPic(guint32 type, guint32 frame,
                                    const Player *p, guint32 tileset)
{
  guint32 added = 0;
  SelectorPixMaskCacheItem i;
  i.tileset = tileset;
  i.type = type;
  i.frame = frame;
  i.player_id = p->getId();
  PixMask *s = selectorcache.get(i, added);
  d_cachesize += added;
  if (added)
    checkPictures();
  return s;
}

PixMask* ImageCache::getArmyPic(Army *a, bool greyed)
{
  return getArmyPic(a->getOwner()->getArmyset(), a->getTypeId(),
		    a->getOwner(), NULL, true, 0, greyed);
}

PixMask* ImageCache::getDialogArmyPic(Army *a, guint32 font_size, bool greyed)
{
  return getArmyPic(a->getOwner()->getArmyset(), a->getTypeId(),
		    a->getOwner(), NULL, false, font_size, greyed);
}

PixMask* ImageCache::getArmyPic(guint32 armyset, guint32 army_id,
                                const Player* p, const bool *medals,
                                bool map, guint32 font_size, bool greyed)
{
  guint added = 0;
  ArmyPixMaskCacheItem i;
  i.armyset = armyset;
  i.army_id = army_id;
  i.player_id = p->getId();
  for (guint32 j = 0; j < MEDAL_TYPES; j++)
    if (medals)
      i.medals[j] = medals[j];
    else
      i.medals[j] = false;
  i.map = map;
  i.font_size = font_size;
  i.greyed = greyed;
  PixMask *s = armycache.get(i, added);
  if (!s)
    {
      guint32 size = Armysetlist::getInstance()->get(i.armyset)->getTileSize();
      s = getDefaultTileStylePic(DEFAULT_TILESTYLE_TYPES-1, size);
    }
  d_cachesize += added;
  if (added)
    checkPictures();
  return s;
}

PixMask* ImageCache::getFlagPic(guint32 stack_size, const Player *p, guint32 tileset)
{
  guint32 added = 0;
  FlagPixMaskCacheItem i;
  i.tileset = tileset;
  i.size = stack_size;
  i.player_id = p->getId();
  PixMask *s = flagcache.get(i, added);
  d_cachesize += added;
  if (added)
    checkPictures();
  return s;
}

PixMask* ImageCache::getFlagPic(guint32 stack_size, const Player *p)
{
  return getFlagPic(stack_size, p,
		    GameMap::getInstance()->getTilesetId());
}

PixMask* ImageCache::getFlagPic(const Stack* s)
{
  return getFlagPic(s, GameMap::getInstance()->getTilesetId());
}

PixMask* ImageCache::getFlagPic(const Stack* s, guint32 tileset)
{
  return getFlagPic(s->size(), s->getOwner(), tileset);
}

PixMask* ImageCache::getCircledArmyPic(Army *a, bool greyed,
                                       guint32 circle_color_id,
                                       bool show_army, guint32 font_size)
{
  return getCircledArmyPic(a->getOwner()->getArmyset(), a->getTypeId(),
		    a->getOwner(), NULL, greyed, circle_color_id, show_army,
                    font_size);
}

PixMask* ImageCache::getCircledArmyPic(guint32 armyset, guint32 army_id,
                                             const Player* p,
                                             const bool *medals, bool greyed,
                                             guint32 circle_color_id,
                                             bool show_army, guint32 font_size)
{
  guint added = 0;
  CircledArmyPixMaskCacheItem i;
  i.armyset = armyset;
  i.army_id = army_id;
  i.player_id = p->getId();
  for (guint32 j = 0; j < MEDAL_TYPES; j++)
    if (medals)
      i.medals[j] = medals[j];
    else
      i.medals[j] = false;
  i.greyed = greyed;
  i.circle_color_id = circle_color_id;
  i.show_army = show_army;
  i.font_size = font_size;
  PixMask *s = circledarmycache.get(i, added);
  d_cachesize += added;
  if (added)
    checkPictures();
  return s;
}

PixMask* ImageCache::getCircledShipPic(guint32 armyset, const Player* p,
                                       bool greyed, guint32 circle_color_id,
                                       guint32 font_size)
{
  guint added = 0;
  CircledShipPixMaskCacheItem i;
  i.armyset = armyset;
  i.player_id = p->getId();
  i.greyed = greyed;
  i.circle_color_id = circle_color_id;
  i.font_size = font_size;
  PixMask *s = circledshipcache.get(i, added);
  d_cachesize += added;
  if (added)
    checkPictures();
  return s;
}

PixMask* ImageCache::getCircledStandardPic(guint32 armyset, const Player* p,
                                           bool greyed,
                                           guint32 circle_color_id,
                                           guint32 font_size)
{
  guint added = 0;
  CircledStandardPixMaskCacheItem i;
  i.armyset = armyset;
  i.player_id = p->getId();
  i.greyed = greyed;
  i.circle_color_id = circle_color_id;
  i.font_size = font_size;
  PixMask *s = circledstandardcache.get(i, added);
  d_cachesize += added;
  if (added)
    checkPictures();
  return s;
}

PixMask* ImageCache::getTilePic(int tile_style_id, int fog_type_id, bool has_bag, int bag_player_id, bool has_standard, int standard_player_id, int stack_size, int stack_player_id, int army_type_id, bool has_tower, bool has_ship, Maptile::Building building_type, int building_subtype, Vector<int> building_tile, int building_player_id, guint32 tilesize, bool has_grid, int stone_type)
{
  guint32 tileset = GameMap::getInstance()->getTilesetId();
  guint32 cityset = GameMap::getInstance()->getCitysetId();
  guint32 shieldset = GameMap::getInstance()->getShieldsetId();
  return getTilePic(tile_style_id, fog_type_id, has_bag, bag_player_id, has_standard, standard_player_id, stack_size, stack_player_id, army_type_id, has_tower, has_ship, building_type, building_subtype, building_tile, building_player_id, tilesize, has_grid, tileset, cityset, shieldset, stone_type);
}

PixMask* ImageCache::getTilePic(int tile_style_id, int fog_type_id, bool has_bag, int bag_player_id, bool has_standard, int standard_player_id, int stack_size, int stack_player_id, int army_type_id, bool has_tower, bool has_ship, Maptile::Building building_type, int building_subtype, Vector<int> building_tile, int building_player_id, guint32 tilesize, bool has_grid, guint32 tileset, guint32 cityset, guint32 shieldset, int stone_type)
{
  guint added = 0;
  TilePixMaskCacheItem i;
  i.tile_style_id = tile_style_id;
  i.fog_type_id = fog_type_id;
  i.has_bag = has_bag;
  i.bag_player_id = bag_player_id;
  i.has_standard = has_standard;
  i.standard_player_id = standard_player_id;
  i.stack_size = stack_size; //flag size
  i.stack_player_id = stack_player_id;
  i.army_type_id = army_type_id;
  i.has_tower = has_tower;
  i.has_ship = has_ship;
  i.building_type = building_type;
  i.building_subtype = building_subtype;
  i.building_tile = building_tile;
  i.building_player_id = building_player_id;
  i.tilesize = tilesize;
  i.has_grid = has_grid;
  i.tileset = tileset;
  i.cityset = cityset;
  i.shieldset = shieldset;
  i.stone_type = stone_type;
  PixMask *s = tilecache.get(i, added);
  d_cachesize += added;
  if (added)
    checkPictures();
  return s;
}

PixMask* ImageCache::getCityPic(const City* city, guint32 cityset)
{
  if (!city)
    return NULL;
  int type;
  if (city->isBurnt() == true)
    type = -1;
  else
    type = 0;
  return getCityPic(type, city->getOwner(), cityset);
}

PixMask* ImageCache::getCityPic(const City* city)
{
  guint32 cityset = GameMap::getInstance()->getCitysetId();
  return getCityPic(city, cityset);
}

PixMask* ImageCache::getCityPic(int type, const Player* p, guint32 cityset)
{
  guint added = 0;
  CityPixMaskCacheItem i;
  i.cityset = cityset;
  i.type = type;
  i.player_id = p->getId();
  PixMask *s = citycache.get(i, added);
  d_cachesize += added;
  if (added)
    checkPictures();
  return s;
}

PixMask* ImageCache::getTowerPic(const Player* p)
{
  guint32 cityset = GameMap::getInstance()->getCitysetId();
  return getTowerPic(p, cityset);
}

PixMask* ImageCache::getTowerPic(const Player* p, guint32 cityset)
{
  guint added = 0;
  TowerPixMaskCacheItem i;
  i.cityset = cityset;
  i.player_id = p->getId();
  PixMask *s = towercache.get(i, added);
  d_cachesize += added;
  if (added)
    checkPictures();
  return s;
}

PixMask* ImageCache::getTemplePic(Temple *t)
{
  guint32 cityset = GameMap::getInstance()->getCitysetId();
  return getTemplePic(t->getType(), cityset);
}

PixMask* ImageCache::getTemplePic(int type)
{
  guint32 cityset = GameMap::getInstance()->getCitysetId();
  return getTemplePic(type, cityset);
}

PixMask* ImageCache::getTemplePic(int type, guint32 cityset)
{
  guint added = 0;
  TemplePixMaskCacheItem i;
  i.cityset = cityset;
  i.type = type;
  PixMask *s = templecache.get(i, added);
  d_cachesize += added;
  if (added)
    checkPictures();
  return s;
}

PixMask* ImageCache::getRuinPic(Ruin *ruin)
{
  guint32 cityset = GameMap::getInstance()->getCitysetId();
  return getRuinPic(ruin->getType(), cityset);
}
PixMask* ImageCache::getRuinPic(int type)
{
  guint32 cityset = GameMap::getInstance()->getCitysetId();
  return getRuinPic(type, cityset);
}

PixMask* ImageCache::getRuinPic(int type, guint32 cityset)
{
  guint added = 0;
  RuinPixMaskCacheItem i;
  i.cityset = cityset;
  i.type = type;
  PixMask *s = ruincache.get(i, added);
  d_cachesize += added;
  if (added)
    checkPictures();
  return s;
}

PixMask* ImageCache::getDiplomacyPic(int type, Player::DiplomaticState state,
                                     guint32 font_size)
{
  guint added = 0;
  DiplomacyPixMaskCacheItem i;
  i.type = type;
  i.state = state;
  i.font_size = font_size;
  PixMask *s = diplomacycache.get(i, added);
  d_cachesize += added;
  if (added)
    checkPictures();
  return s;
}

PixMask* ImageCache::getRoadPic(Road *r)
{
  return ImageCache::getRoadPic(r->getType());
}

PixMask* ImageCache::getRoadPic(int type)
{
  return getRoadPic(type, GameMap::getInstance()->getTilesetId());
}

PixMask* ImageCache::getRoadPic(int type, guint32 tileset)
{
  guint added = 0;
  RoadPixMaskCacheItem i;
  i.type = type;
  i.tileset = tileset;
  PixMask *s = roadcache.get(i, added);
  d_cachesize += added;
  if (added)
    checkPictures();
  return s;
}

PixMask* ImageCache::getFogPic(int type)
{
  return getFogPic(type, GameMap::getInstance()->getTilesetId());
}

PixMask* ImageCache::getFogPic(int type, guint32 tileset)
{
  guint added = 0;
  FogPixMaskCacheItem i;
  i.type = type;
  i.tileset = tileset;
  PixMask *s = fogcache.get(i, added);
  d_cachesize += added;
  if (added)
    checkPictures();
  return s;
}

PixMask* ImageCache::getBridgePic(Bridge *b)
{
  return getBridgePic(b->getType());
}

PixMask* ImageCache::getBridgePic(int type)
{
  return getBridgePic(type, GameMap::getInstance()->getTilesetId());
}

PixMask* ImageCache::getBridgePic(int type, guint32 tileset)
{
  guint added = 0;
  BridgePixMaskCacheItem i;
  i.type = type;
  i.tileset = tileset;
  PixMask *s = bridgecache.get(i, added);
  d_cachesize += added;
  if (added)
    checkPictures();
  return s;
}

PixMask* ImageCache::getCursorPic(int type, guint32 font_size)
{
  guint added = 0;
  CursorPixMaskCacheItem i;
  i.type = type;
  i.font_size = font_size;
  PixMask *s = cursorcache.get(i, added);
  d_cachesize += added;
  if (added)
    checkPictures();
  return s;
}

PixMask* ImageCache::getShieldPic(guint32 type, Player *p, bool map,
                                  guint32 font_size)
{
  guint32 shieldset = GameMap::getInstance()->getShieldsetId();
  return getShieldPic(shieldset, type, p->getId(), map, font_size);
}

PixMask* ImageCache::getShieldPic(guint32 shieldset, guint32 type,
                                        guint32 color, bool map,
                                        guint32 font_size)
{
  guint added = 0;
  ShieldPixMaskCacheItem i;
  i.type = type;
  i.shieldset = shieldset;
  i.color = color;
  i.map = map;
  i.font_size = font_size;
  PixMask *s = shieldcache.get(i, added);
  d_cachesize += added;
  if (added)
    checkPictures();
  return s;
}

PixMask* ImageCache::getStatusPic(guint32 type, guint32 font_size)
{
  guint added = 0;
  StatusPixMaskCacheItem i;
  i.type = type;
  i.font_size = font_size;
  PixMask *s = statuscache.get(i, added);
  d_cachesize += added;
  if (added)
    checkPictures();
  return s;
}

PixMask* ImageCache::getGameButtonPic(guint32 type, guint32 font_size)
{
  guint added = 0;
  GameButtonPixMaskCacheItem i;
  i.type = type;
  i.font_size = font_size;
  PixMask *s = gamebuttoncache.get(i, added);
  d_cachesize += added;
  if (added)
    checkPictures();
  return s;
}

PixMask* ImageCache::getProdShieldPic(guint32 type, bool prod)
{
  guint added = 0;
  ProdShieldPixMaskCacheItem i;
  i.type = type;
  i.prod = prod;
  PixMask *s = prodshieldcache.get(i, added);
  d_cachesize += added;
  if (added)
    checkPictures();
  return s;
}


PixMask* ImageCache::getShipPic(const Player* p)
{
  guint added = 0;
  ShipPixMaskCacheItem i;
  i.player_id = p->getId();
  i.armyset = p->getArmyset();
  PixMask *s = shipcache.get(i, added);
  if (!s)
    {
      guint32 size = Armysetlist::getInstance()->get(i.armyset)->getTileSize();
      s = getDefaultTileStylePic(DEFAULT_TILESTYLE_TYPES-1, size);
    }
  d_cachesize += added;
  if (added)
    checkPictures();
  return s;
}

PixMask* ImageCache::getPlantedStandardPic(const Player* p)
{
  guint added = 0;
  PlantedStandardPixMaskCacheItem i;
  i.player_id = p->getId();
  i.armyset = p->getArmyset();
  PixMask *s = plantedstandardcache.get(i, added);
  if (!s)
    {
      guint32 size = Armysetlist::getInstance()->get(i.armyset)->getTileSize();
      s = getDefaultTileStylePic(DEFAULT_TILESTYLE_TYPES-1, size);
    }
  d_cachesize += added;
  if (added)
    checkPictures();
  return s;
}

PixMask* ImageCache::getPortPic()
{
  return getPortPic(GameMap::getInstance()->getCitysetId());
}

PixMask* ImageCache::getPortPic(guint32 cityset)
{
  guint added = 0;
  PortPixMaskCacheItem i;
  i.cityset = cityset;
  PixMask *s = portcache.get(i, added);
  d_cachesize += added;
  if (added)
    checkPictures();
  return s;
}

PixMask* ImageCache::getSignpostPic()
{
  return getSignpostPic(GameMap::getInstance()->getCitysetId());
}

PixMask* ImageCache::getSignpostPic(guint32 cityset)
{
  guint added = 0;
  SignpostPixMaskCacheItem i;
  i.cityset = cityset;
  PixMask *s = signpostcache.get(i, added);
  d_cachesize += added;
  if (added)
    checkPictures();
  return s;
}

PixMask* ImageCache::getBagPic()
{
  guint32 armyset = Playerlist::getActiveplayer()->getArmyset();
  return getBagPic(armyset);
}

PixMask* ImageCache::getBagPic(guint32 armyset)
{
  guint added = 0;
  BagPixMaskCacheItem i;
  i.armyset = armyset;
  PixMask *s = bagcache.get(i, added);
  if (!s)
    {
      guint32 size = Armysetlist::getInstance()->get(i.armyset)->getTileSize();
      s = getDefaultTileStylePic(DEFAULT_TILESTYLE_TYPES-1, size);
    }
  d_cachesize += added;
  if (added)
    checkPictures();
  return s;
}

PixMask* ImageCache::getExplosionPic()
{
  return getExplosionPic(GameMap::getInstance()->getTilesetId());
}

PixMask* ImageCache::getExplosionPic(guint32 tileset)
{
  guint added = 0;
  ExplosionPixMaskCacheItem i;
  i.tileset = tileset;
  PixMask *s = explosioncache.get(i, added);
  d_cachesize += added;
  if (added)
    checkPictures();
  return s;
}

PixMask* ImageCache::getNewLevelPic(const Player* p, guint32 gender,
                                    guint32 font_size)
{
  guint added = 0;
  NewLevelPixMaskCacheItem i;
  i.player_id = p->getId();
  i.gender = gender;
  i.font_size = font_size;
  PixMask *s = newlevelcache.get(i, added);
  d_cachesize += added;
  if (added)
    checkPictures();
  return s;
}

PixMask* ImageCache::getDefaultTileStylePic(guint32 type, guint32 size)
{
  guint added = 0;
  DefaultTileStylePixMaskCacheItem i;
  i.tilestyle_type = type;
  i.tilesize = size;
  PixMask *s = defaulttilestylecache.get(i, added);
  d_cachesize += added;
  if (added)
    checkPictures();
  return s;
}

PixMask* ImageCache::getTartanPic(const Player *p, guint32 width, Shieldset *shieldset, guint32 font_size)
{
  guint added = 0;
  TartanPixMaskCacheItem i;
  i.player_id = p->getId();
  i.width = width;
  i.shieldset = shieldset->getId();
  i.font_size = font_size;
  PixMask *s = tartancache.get(i, added);
  d_cachesize += added;
  if (added)
    checkPictures();
  return s;
}

PixMask* ImageCache::getEmptyTartanPic(const Player *p, guint32 width, Shieldset *shieldset, guint32 font_size)
{
  guint added = 0;
  EmptyTartanPixMaskCacheItem i;
  i.player_id = p->getId();
  i.width = width;
  i.shieldset = shieldset->getId();
  i.font_size = font_size;
  PixMask *s = emptytartancache.get(i, added);
  d_cachesize += added;
  if (added)
    checkPictures();
  return s;
}

PixMask* ImageCache::getDialogPic(guint32 type, guint32 font_size)
{
  guint added = 0;
  DialogPixMaskCacheItem i;
  i.type = type;
  i.font_size = font_size;
  PixMask *s = dialogcache.get(i, added);
  d_cachesize += added;
  if (added)
    checkPictures();
  return s;
}

PixMask* ImageCache::getDiplomacyImage(int type, Player::DiplomaticState state)
{
  return d_diplomacy[type][state];
}

PixMask* ImageCache::getMoveBonusPic(guint32 tileset_id, guint32 bonus, guint32 font_size)
{
  guint added = 0;
  MoveBonusPixMaskCacheItem i;
  i.bonus = bonus;
  i.tileset = tileset_id;
  i.font_size = font_size;

  PixMask *s = movebonuscache.get(i, added);
  d_cachesize += added;
  if (added)
    checkPictures();
  return s;
}

PixMask* ImageCache::getDefaultTileStyleImage(guint32 type)
{
  return d_default_tilestyles[type];
}

PixMask* ImageCache::getCursorImage(int type)
{
  return d_cursor[type];
}

PixMask *ImageCache::getProdShieldImage(guint32 type)
{
  return d_prodshield[type];
}

PixMask* ImageCache::getMedalImage(bool large, int type)
{
  ScenarioMedia *sm = ScenarioMedia::getInstance ();
  if (sm->getMedalImage (large)->getName() != "")
    return sm->getMedalImage(large)->getImage (type);
  else
    return getMedalImage (large)->getImage (type);
}

TarFileMaskedImage *ImageCache::getHeroNewLevelMaskedImage (bool female)
{
  TarFileMaskedImage *mim =
    ScenarioMedia::getInstance()->getHeroNewLevelMaskedImage(female);
  if (mim->getImage ())
    return mim;
  return d_hero_newlevel[female ? 1 : 0];
}

PixMask* ImageCache::getMedalPic(bool large, guint32 type,
                                        guint32 font_size)
{
  guint added = 0;
  MedalPixMaskCacheItem i;
  i.large = large;
  i.type = type;
  i.font_size = font_size;
  PixMask *s = medalcache.get(i, added);
  d_cachesize += added;
  if (added)
    checkPictures();
  return s;
}

PixMask* ImageCache::getGameButtonImage(guint32 type)
{
  return d_gamebuttons[type];
}

PixMask* ImageCache::getWaypointImage(guint32 type)
{
  return d_waypoint[type];
}

PixMask* ImageCache::getSmallRuinedCityImage()
{
  return d_smallruinedcity;
}

PixMask* ImageCache::getSmallBagImage()
{
  return d_smallbag;
}

PixMask* ImageCache::getSmallHeroImage(bool active)
{
  if (active)
    return d_smallhero;
  else
    return d_smallinactivehero;
}

PixMask* ImageCache::getSmallRuinExploredImage()
{
  return d_small_ruin_explored;
}

PixMask* ImageCache::getSmallRuinUnexploredImage()
{
  return d_small_ruin_unexplored;
}

PixMask* ImageCache::getSmallStrongholdUnexploredImage()
{
  return d_small_stronghold_unexplored;
}

PixMask* ImageCache::getSmallTempleImage()
{
  return d_small_temple;
}

PixMask* ImageCache::greyOut(PixMask* image)
{
  int width = image->get_width();
  int height = image->get_height();
  PixMask* result = image->copy ();

  Glib::RefPtr<Gdk::Pixbuf> d = image->to_pixbuf ();
  guint8 *data = d->get_pixels ();
  guint8 *copy = (guint8*)  malloc (height * width * 4 * sizeof(guint8));
  for (int i = 0; i < height * width * 4; i++)
    copy[i] = data[i];
  for (int i = 0; i < width; i++)
    for (int j = 0; j < height; j++)
      {
	const int base = (j * 4) + (i * height * 4);

	if (data[base+3] != 0)
	  {
	    guint32 max = 0;
	    if (copy[base+0] > max)
	      max = copy[base+0];
	    else if (copy[base+1] > max)
	      max = copy[base+1];
	    else if (copy[base+2] > max)
	      max = copy[base+2];
	    int x =  i % 2;
	    int y = j % 2;
	    if ((x == 0 && y == 0) || (x == 1 && y == 1))
	      max = 88;
	    copy[base+0] = max;
	    copy[base+1] = max;
	    copy[base+2] = max;
	  }
      }
  d.reset ();
  Glib::RefPtr<Gdk::Pixbuf> greyed_out =
    Gdk::Pixbuf::create_from_data(copy, Gdk::COLORSPACE_RGB, true, 8,
				  width, height, width * 4);

  result->draw_pixbuf(greyed_out, 0, 0, 0, 0, width, height);
  free(copy);

  return result;
}
void ImageCache::draw_circle(Cairo::RefPtr<Cairo::Context> cr, double width_percent, int width, int height, Gdk::RGBA color, bool colored, bool mask)
{
  if (width_percent > 100)
    width_percent = 0;
  else if (width_percent < 0)
    width_percent = 0;
  width_percent /= 100.0;
  //i want 2 o'clock as a starting point, and 8pm as an ending point.

  double dred = BEVELED_CIRCLE_DARK.get_red();
  double dgreen = BEVELED_CIRCLE_DARK.get_green();
  double dblue = BEVELED_CIRCLE_DARK.get_blue();
  double lred = BEVELED_CIRCLE_LIGHT.get_red();
  double lgreen = BEVELED_CIRCLE_LIGHT.get_green();
  double lblue = BEVELED_CIRCLE_LIGHT.get_blue();

  double radius = (double)width * width_percent / 2.0;
  double line_width = radius * 0.2;

  if (mask)
    cr->set_line_width(line_width + 2.0);
  else
    cr->set_line_width(line_width + 4.0);
  cr->set_source_rgb(((lred - dred) / 2.0) + lred,
                     ((lgreen -dgreen) / 2.0) + lgreen,
                     ((lblue - dblue) / 2.0) + lblue);
  cr->arc((double)width/2.0, (double)height/2.0, radius - (line_width / 2.0), 0, 2 *M_PI);
  cr->stroke();
  if (mask)
    return;

  cr->set_line_width(1.0);
  cr->set_source_rgb(dred, dgreen, dblue);
  cr->arc((double)width/2.0, (double)height/2.0, radius, (2 * M_PI) * (2.0/12.0), (2 *M_PI) * (8.0/12.0));
  cr->stroke();
  cr->set_source_rgb(lred, lgreen, lblue);
  cr->arc((double)width/2.0, (double)height/2.0, radius, (2 * M_PI) * (8.0/12.0), (2 *M_PI) * (2.0/12.0));
  cr->stroke();

  radius -= line_width;
  cr->set_source_rgb(lred, lgreen, lblue);
  cr->arc((double)width/2.0, (double)height/2.0, radius, (2 * M_PI) * (2.0/12.0), (2 *M_PI) * (8.0/12.0));
  cr->stroke();

  cr->set_source_rgb(dred, dgreen, dblue);
  cr->arc((double)width/2.0, (double)height/2.0, radius, (2 * M_PI) * (8.0/12.0), (2 *M_PI) * (2.0/12.0));
  cr->stroke();

  if (colored)
    {
      cr->set_line_width(line_width);
      double red = color.get_red();
      double green = color.get_green();
      double blue = color.get_blue();
      cr->set_source_rgb(red, green, blue);
      cr->arc((double)width/2.0, (double)height/2.0, radius + (line_width / 2.0), 0, 2 *M_PI);
      cr->stroke();
    }
}

PixMask* ImageCache::circled(PixMask* image, Gdk::RGBA color, bool colored, double width_percent)
{
  PixMask *copy = image->copy();
  int width = image->get_width();
  int height = image->get_height();
  //here we draw a colored circle on top of the army's image
  Cairo::RefPtr<Cairo::Context> cr = Cairo::Context::create(copy->get_pixmap());

  draw_circle(cr, width_percent, width, height, color, colored, false);

  //here we draw a white circle on a copy of the image's mask.
  Cairo::RefPtr<Cairo::Surface> mask = copy->get_mask();

  cr = Cairo::Context::create(mask);
  draw_circle(cr, width_percent, width, height, Gdk::RGBA("white"), colored, true);
  PixMask *result = PixMask::create(copy->get_pixmap(), mask);
  //draw the army on top again, to make it look like the circle is behind.
  result->draw_pixbuf(image->to_pixbuf(), 0, 0, 0, 0, width, height);
  delete copy;
  return result;
}

TarFileImage* ImageCache::getNextTurnImage ()
{
  TarFileImage *im = ScenarioMedia::getInstance()->getNextTurnImage();
  if (im->getImage ())
    return im;
  return d_next_turn;
}

TarFileImage* ImageCache::getCityDefeatedImage ()
{
  TarFileImage *im = ScenarioMedia::getInstance()->getCityDefeatedImage();
  if (im->getImage ())
    return im;
  return d_city_defeated;
}

TarFileImage * ImageCache::getWinningImage ()
{
  TarFileImage *im = ScenarioMedia::getInstance()->getWinningImage();
  if (im->getImage ())
    return im;
  return d_winning;
}

TarFileImage* ImageCache::getHeroOfferedImage (Hero::Gender gender)
{
  switch (gender)
    {
    case Hero::NONE:
    case Hero::MALE:
        {
          TarFileImage *im =
            ScenarioMedia::getInstance()->getHeroOfferedImage(false);
          if (im->getImage ())
            return im;
          return d_hero[0];
        }
      break;
    case Hero::FEMALE:
        {
          TarFileImage *im =
            ScenarioMedia::getInstance()->getHeroOfferedImage(true);
          if (im->getImage ())
            return im;
          return d_hero[1];
        }
      break;
    }
  return NULL;
}

TarFileImage *ImageCache::getRuinSuccessImage()
{
  TarFileImage *im = ScenarioMedia::getInstance()->getRuinSuccessImage();
  if (im->getImage ())
    return im;
  return d_ruin_success;
}

TarFileImage *ImageCache::getRuinDefeatImage()
{
  TarFileImage *im = ScenarioMedia::getInstance()->getRuinDefeatImage();
  if (im->getImage ())
    return im;
  return d_ruin_defeat;
}

TarFileImage * ImageCache::getParleyOfferedImage ()
{
  TarFileImage *im = ScenarioMedia::getInstance()->getParleyOfferedImage();
  if (im->getImage ())
    return im;
  return d_parley_offered;
}

TarFileImage* ImageCache::getParleyRefusedImage ()
{
  TarFileImage *im = ScenarioMedia::getInstance()->getParleyRefusedImage();
  if (im->getImage ())
    return im;
  return d_parley_refused;
}

TarFileImage* ImageCache::getMedalImage (bool large)
{
  TarFileImage *im = ScenarioMedia::getInstance()->getMedalImage(large);
  if (im->getImage ())
    return im;
  return d_medal[large ? 1 : 0];
}

TarFileImage* ImageCache::getCommentatorImage ()
{
  TarFileImage *im = ScenarioMedia::getInstance()->getCommentatorImage();
  if (im->getImage ())
    return im;
  return d_commentator;
}

void ImageCache::add_underline (PixMask **p, Gdk::RGBA color, guint32 font_size)
{
  int height = (*p)->get_height () +
    (font_size * TURN_INDICATOR_FONT_SIZE_MULTIPLE);
  int width = (*p)->get_width ();
    Glib::RefPtr<Gdk::Pixbuf> pixbuf
    = Gdk::Pixbuf::create(Gdk::COLORSPACE_RGB, true, 8, width, height);
  pixbuf->fill(0x00000000);
  PixMask *box = PixMask::create (pixbuf);
  (*p)->blit (box->get_pixmap(), 0, 0);
  Cairo::RefPtr<Cairo::Context> cr = Cairo::Context::create(box->get_pixmap());
  cr->set_line_width(1.0);
  cr->set_source_rgba(color.get_red (), color.get_green(), color.get_blue (),
                      color.get_alpha ());
  cr->rectangle(0, (*p)->get_height (), width, height - (*p)->get_height ());
  cr->fill ();
  //cr->stroke();
  delete *p;
  *p = box;
}

int ImageCache::calculate_width_from_adjusted_height (PixMask *p, double new_height)
{
  return p->get_width () * (new_height / p->get_height ());
}

PixMask *SelectorPixMaskCacheItem::generate(const SelectorPixMaskCacheItem &i)
{
  // armyset selectors override the tileset ones
  // we can't have a neutral selector, but just in case we change it to white
  Player *p = Playerlist::getInstance()->getPlayer(i.player_id);
  Shield::Color c = Shield::Color (p->getId ());
  if (c == Shield::NEUTRAL)
    c = Shield::WHITE;
  Armyset *as = Armysetlist::getInstance ()->get (p->getArmyset ());
  Tileset *ts = Tilesetlist::getInstance()->get(i.tileset);
  if (i.type == 0)
    {
      if (as->getSelector(true, c)->getNumberOfFrames () > 0)
        return as->getSelector(true, c)->applyMask (i.frame, p);
      else
        return ts->getSelector(true)->applyMask (i.frame, p);
    }
  else
    {
      if (as->getSelector(false, c)->getNumberOfFrames () > 0)
        return as->getSelector(false, c)->applyMask (i.frame, p);
      else
        return ts->getSelector(false)->applyMask (i.frame, p);
    }
}

int SelectorPixMaskCacheItem::comp(const SelectorPixMaskCacheItem &item) const
{
  return
    (tileset < item.tileset) ? -1 :
    (tileset> item.tileset)?  1 :
    (type < item.type) ? -1 :
    (type > item.type) ?  1 :
    (frame < item.frame) ? -1 :
    (frame > item.frame) ?  1 :
    (player_id < item.player_id) ? -1 :
    (player_id > item.player_id) ?  1 :
    0;
}

PixMask *FlagPixMaskCacheItem::generate(const FlagPixMaskCacheItem &i)
{
  Tileset *ts = Tilesetlist::getInstance()->get(i.tileset);

  //when size is zero
  Player *p = Playerlist::getInstance()->getPlayer(i.player_id);
  /**
   * if you're here and i.size is zero it means:
   * bigmap tried to lookup the number of stacks belonging to a player on
   * a tile, and it got nothing
   * so the ownership stuff has been messed up
   */
  // size of stack starts at 1, but we need the index, which starts at 0
  return ts->getFlags ()->applyMask (i.size - 1, p);
}

int FlagPixMaskCacheItem::comp(const FlagPixMaskCacheItem &item) const
{
  return
    (tileset < item.tileset) ? -1 :
    (tileset > item.tileset)?  1 :
    (size < item.size) ? -1 :
    (size > item.size) ?  1 :
    (player_id < item.player_id) ? -1 :
    (player_id > item.player_id) ?  1 :
    0;
}

PixMask *ArmyPixMaskCacheItem::generate(const ArmyPixMaskCacheItem &i)
{
  PixMask *s;
  const ArmyProto * basearmy =
    Armysetlist::getInstance()->getArmy(i.armyset, i.army_id);

  // copy the pixmap including player colors
  Player *p = Playerlist::getInstance()->getPlayer(i.player_id);
  Shield::Color c = Shield::Color(i.player_id);
  PixMask *colored = basearmy->getMaskedImage (c)->applyMask (p);
  if (i.greyed)
    {
      PixMask *greyed_out = ImageCache::greyOut(colored);
      s = greyed_out;
      delete colored;
    }
  else
    s = colored;

  for(int j = 0; j < 3; j++)
    {
      if (i.medals[j])
        ImageCache::getInstance()->getMedalImage(false, j)->blit(s->get_pixmap());
    }
  if (i.map == false)
    {
      int dialogsize = i.font_size * DIALOG_ARMY_PIC_FONTSIZE_MULTIPLE;
      PixMask::scale (s, dialogsize, dialogsize);
    }
  return s;
}

int ArmyPixMaskCacheItem::comp(const ArmyPixMaskCacheItem &item) const
{
  return
    (armyset < item.armyset) ? -1 :
    (armyset > item.armyset)?  1 :
    (army_id < item.army_id) ? -1 :
    (army_id > item.army_id) ?  1 :
    (player_id < item.player_id) ? -1 :
    (player_id > item.player_id) ?  1 :
    (memcmp(medals,item.medals,sizeof(medals)) < 0) ? -1 :
    (memcmp(medals,item.medals,sizeof(medals)) > 0) ? 1 :
    (map < item.map) ? -1 :
    (map > item.map) ?  1 :
    (font_size < item.font_size) ? -1 :
    (font_size > item.font_size) ?  1 :
    (greyed < item.greyed) ? -1 :
    (greyed > item.greyed) ?  1 :
    0;
}

PixMask *CircledArmyPixMaskCacheItem::generate(const CircledArmyPixMaskCacheItem &i)
{
  PixMask *s;
  if (i.show_army)
    {
      Player *p = Playerlist::getInstance()->getPlayer(i.player_id);
      PixMask *pre_circle =
        ImageCache::getInstance()->getArmyPic(i.armyset, i.army_id, p,
                                              i.medals, false, i.font_size,
                                              i.greyed);
      s = ImageCache::circled(pre_circle, p->getColor(),
                              i.circle_color_id != Shield::NEUTRAL);
    }
  else
    {
      guint32 size = Armysetlist::getInstance()->get(i.armyset)->getTileSize();
      Glib::RefPtr<Gdk::Pixbuf> pixbuf
        = Gdk::Pixbuf::create(Gdk::COLORSPACE_RGB, true, 8, size, size);
      pixbuf->fill(0x00000000);
      PixMask *empty = PixMask::create(pixbuf);
      s = ImageCache::circled
        (empty, Shield::get_default_colors_for_no(i.circle_color_id)[0],
         i.circle_color_id != Shield::NEUTRAL);
      delete empty;
    }
  int dialogsize = i.font_size * DIALOG_ARMY_PIC_FONTSIZE_MULTIPLE;
  PixMask::scale (s, dialogsize, dialogsize);
  return s;
}

int CircledArmyPixMaskCacheItem::comp(const CircledArmyPixMaskCacheItem &item) const
{
  return
    (armyset < item.armyset) ? -1 :
    (armyset > item.armyset)?  1 :
    (army_id < item.army_id) ? -1 :
    (army_id > item.army_id) ?  1 :
    (player_id < item.player_id) ? -1 :
    (player_id > item.player_id) ?  1 :
    (memcmp(medals,item.medals,sizeof(medals)) < 0) ? -1 :
    (memcmp(medals,item.medals,sizeof(medals)) > 0) ? 1 :
    (greyed < item.greyed) ?  -1 :
    (greyed > item.greyed) ?  1 :
    (circle_color_id < item.circle_color_id) ?  -1 :
    (circle_color_id > item.circle_color_id) ?  1 :
    (show_army < item.show_army) ?  -1 :
    (show_army > item.show_army) ?  1 :
    (font_size < item.font_size) ?  -1 :
    (font_size > item.font_size) ?  1 :
    0;
}

PixMask *CircledShipPixMaskCacheItem::generate(const CircledShipPixMaskCacheItem &i)
{
  PixMask *s;
  Player *p = Playerlist::getInstance()->getPlayer(i.player_id);
  PixMask *pre_circle = ImageCache::getInstance ()->getShipPic (p);
  s = ImageCache::circled(pre_circle, p->getColor(),
                          i.circle_color_id != Shield::NEUTRAL);
  int dialogsize = i.font_size * DIALOG_ARMY_PIC_FONTSIZE_MULTIPLE;
  PixMask::scale (s, dialogsize, dialogsize);
  return s;
}

int CircledShipPixMaskCacheItem::comp(const CircledShipPixMaskCacheItem &item) const
{
  return
    (armyset < item.armyset) ? -1 :
    (armyset > item.armyset)?  1 :
    (player_id < item.player_id) ? -1 :
    (player_id > item.player_id) ?  1 :
    (greyed < item.greyed) ?  -1 :
    (greyed > item.greyed) ?  1 :
    (circle_color_id < item.circle_color_id) ?  -1 :
    (circle_color_id > item.circle_color_id) ?  1 :
    (font_size < item.font_size) ?  -1 :
    (font_size > item.font_size) ?  1 :
    0;
}

PixMask *CircledStandardPixMaskCacheItem::generate(const CircledStandardPixMaskCacheItem &i)
{
  PixMask *s;
  Player *p = Playerlist::getInstance()->getPlayer(i.player_id);
  PixMask *pre_circle = ImageCache::getInstance ()->getPlantedStandardPic(p);
  s = ImageCache::circled(pre_circle, p->getColor(),
                          i.circle_color_id != Shield::NEUTRAL);
  int dialogsize = i.font_size * DIALOG_ARMY_PIC_FONTSIZE_MULTIPLE;
  PixMask::scale (s, dialogsize, dialogsize);
  return s;
}

int CircledStandardPixMaskCacheItem::comp(const CircledStandardPixMaskCacheItem &item) const
{
  return
    (armyset < item.armyset) ? -1 :
    (armyset > item.armyset)?  1 :
    (player_id < item.player_id) ? -1 :
    (player_id > item.player_id) ?  1 :
    (greyed < item.greyed) ?  -1 :
    (greyed > item.greyed) ?  1 :
    (circle_color_id < item.circle_color_id) ?  -1 :
    (circle_color_id > item.circle_color_id) ?  1 :
    (font_size < item.font_size) ?  -1 :
    (font_size > item.font_size) ?  1 :
    0;
}

PixMask *TilePixMaskCacheItem::generate(const TilePixMaskCacheItem &i)
{
  PixMask *s;
  Tileset *t = Tilesetlist::getInstance()->get(i.tileset);
  guint32 uts = t->getUnscaledTileSize();
  if (i.fog_type_id == FogMap::ALL)
    s = t->getFog()->getImage(i.fog_type_id - 1)->copy();
  else
    {
      TileStyle *tilestyle = t->getTileStyle(i.tile_style_id);
      s = tilestyle->getImage()->copy();
      const Player *player;
      Cairo::RefPtr<Cairo::Surface> pixmap = s->get_pixmap();

      switch (i.building_type)
        {
        case Maptile::CITY:
            {
              player = Playerlist::getInstance()->getPlayer(i.building_player_id);
              ImageCache::getInstance()->getCityPic(i.building_subtype, player, i.cityset)->blit(i.building_tile, uts, pixmap);
            }
          break;
        case Maptile::RUIN:
          ImageCache::getInstance()->getRuinPic(i.building_subtype, i.cityset)->blit(i.building_tile, uts, pixmap);
          break;
        case Maptile::TEMPLE:
          ImageCache::getInstance()->getTemplePic(i.building_subtype, i.cityset)->blit(i.building_tile, uts, pixmap);
          break;
        case Maptile::SIGNPOST:
          ImageCache::getInstance()->getSignpostPic(i.cityset)->blit(i.building_tile, uts, pixmap);
          break;
        case Maptile::ROAD:
          ImageCache::getInstance()->getRoadPic(i.building_subtype)->blit(i.building_tile, uts, pixmap);
          if (i.stone_type != -1)
            {
              Tileset *ts = Tilesetlist::getInstance()->get(i.tileset);
              PixMask *p = ts->getStone()->getImage(i.stone_type);
              if (p)
                p->blit(i.building_tile, uts, pixmap);
            }
          break;
        case Maptile::STONE:
            {
              Tileset *ts = Tilesetlist::getInstance()->get(i.tileset);
              PixMask *p = ts->getStone()->getImage(i.stone_type);
              if (p)
                p->blit(i.building_tile, uts, pixmap);
            }
          break;
        case Maptile::PORT:
          ImageCache::getInstance()->getPortPic(i.cityset)->blit(i.building_tile, uts, pixmap);
          break;
        case Maptile::BRIDGE:
          ImageCache::getInstance()->getBridgePic(i.building_subtype)->blit(i.building_tile, uts, pixmap);
          break;
        case Maptile::NONE: default:
          break;
        }

      if (i.has_bag)
        {
          PixMask *pic = ImageCache::getInstance()->getBagPic();
          pic->blit(pixmap);
        }

      if (i.has_standard)
        {
          player = Playerlist::getInstance()->getPlayer(i.standard_player_id) ;
          ImageCache::getInstance()->getPlantedStandardPic(player)->blit(pixmap);
        }

      if (i.stack_player_id > -1)
        {
          player = Playerlist::getInstance()->getPlayer(i.stack_player_id);
          if (i.has_tower)
            ImageCache::getInstance()->getTowerPic(player)->blit(pixmap);
          else
            {
              if (i.stack_size > -1)
                ImageCache::getInstance()->getFlagPic(i.stack_size, player)->blit(pixmap);
              if (i.has_ship)
                ImageCache::getInstance()->getShipPic(player)->blit(pixmap);
              else
                ImageCache::getInstance()->getArmyPic(player->getArmyset(), i.army_type_id, player, NULL, true, 0)->blit(pixmap);
            }
        }
      if (i.has_grid)
        {
          Cairo::RefPtr<Cairo::Context> context = s->get_gc();
          context->set_source_rgba(GRID_BOX_COLOR.get_red(), GRID_BOX_COLOR.get_blue(), GRID_BOX_COLOR.get_green(), GRID_BOX_COLOR.get_alpha());
          context->move_to(0, 0);
          context->rel_line_to(uts, 0);
          context->rel_line_to(0, uts);
          context->rel_line_to(-uts, 0);
          context->rel_line_to(0, -uts);
          context->set_line_width(1.0);
          context->stroke();
        }

      if (i.fog_type_id)
        t->getFog()->getImage(i.fog_type_id - 1)->blit(pixmap);
    }
  int ts = t->getTileSize();
  if (s->get_width () != ts)
    s->scale (s, ts, ts);
  return s;
}

int TilePixMaskCacheItem::comp(const TilePixMaskCacheItem &item) const
{
  return
    (tile_style_id < item.tile_style_id) ? -1 :
    (tile_style_id > item.tile_style_id) ?  1 :
    (fog_type_id < item.fog_type_id) ? -1 :
    (fog_type_id > item.fog_type_id) ?  1 :
    (has_bag < item.has_bag) ? -1 :
    (has_bag > item.has_bag) ?  1 :
    (bag_player_id < item.bag_player_id) ? -1 :
    (bag_player_id > item.bag_player_id) ?  1 :
    (has_standard < item.has_standard) ? -1 :
    (has_standard > item.has_standard) ?  1 :
    (standard_player_id < item.standard_player_id) ?  -1 :
    (standard_player_id > item.standard_player_id) ?  1 :
    (stack_size < item.stack_size) ?  -1 :
    (stack_size > item.stack_size) ?  1 :
    (stack_player_id < item.stack_player_id) ?  -1 :
    (stack_player_id > item.stack_player_id) ?  1 :
    (army_type_id < item.army_type_id) ?  -1 :
    (army_type_id > item.army_type_id) ?  1 :
    (has_tower < item.has_tower) ? -1 :
    (has_tower > item.has_tower) ?  1 :
    (has_ship < item.has_ship) ? -1 :
    (has_ship > item.has_ship) ?  1 :
    (building_type < item.building_type) ?  -1 :
    (building_type > item.building_type) ?  1 :
    (building_subtype < item.building_subtype) ?  -1 :
    (building_subtype > item.building_subtype) ?  1 :
    (building_tile < item.building_tile) ?  -1 :
    (building_tile > item.building_tile) ?  1 :
    (building_player_id < item.building_player_id) ?  -1 :
    (building_player_id > item.building_player_id) ?  1 :
    (tilesize < item.tilesize) ?  -1 :
    (tilesize > item.tilesize) ?  1 :
    (has_grid < item.has_grid) ? -1 :
    (has_grid > item.has_grid) ?  1 :
    (tileset < item.tileset) ? -1 :
    (tileset > item.tileset) ?  1 :
    (cityset < item.cityset) ? -1 :
    (cityset > item.cityset) ?  1 :
    (shieldset < item.shieldset) ? -1 :
    (shieldset > item.shieldset) ?  1 :
    (stone_type < item.stone_type) ? -1 :
    (stone_type > item.stone_type) ?  1 :
    0;
}

PixMask *CityPixMaskCacheItem::generate(const CityPixMaskCacheItem &i)
{
  Cityset *cs = Citysetlist::getInstance()->get(i.cityset);
  Player *p = Playerlist::getInstance()->getPlayer(i.player_id);
  if (i.type == -1)
    return cs->getRazedCity()->getImage(p->getId())->copy();
  else
    return cs->getCity()->getImage(p->getId())->copy();
}

int CityPixMaskCacheItem::comp(const CityPixMaskCacheItem &item) const
{
  return
    (cityset < item.cityset) ? -1 :
    (cityset > item.cityset)?  1 :
    (type < item.type) ? -1 :
    (type > item.type) ?  1 :
    (player_id < item.player_id) ? -1 :
    (player_id > item.player_id) ?  1 :
    0;
}

PixMask *TowerPixMaskCacheItem::generate(const TowerPixMaskCacheItem &i)
{
  Cityset *cs = Citysetlist::getInstance()->get(i.cityset);
  return cs->getTower()->getImage(i.player_id)->copy();
}

int TowerPixMaskCacheItem::comp(const TowerPixMaskCacheItem &item) const
{
  return
    (cityset < item.cityset) ? -1 :
    (cityset > item.cityset)?  1 :
    (player_id < item.player_id) ? -1 :
    (player_id > item.player_id) ?  1 :
    0;
}

PixMask *TemplePixMaskCacheItem::generate(const TemplePixMaskCacheItem &i)
{
  Cityset *cs = Citysetlist::getInstance()->get(i.cityset);
  return cs->getTemple()->getImage(i.type)->copy();
}

int TemplePixMaskCacheItem::comp(const TemplePixMaskCacheItem &item) const
{
  return
    (cityset < item.cityset) ? -1 :
    (cityset > item.cityset)?  1 :
    (type < item.type) ? -1 :
    (type > item.type) ?  1 :
    0;
}

PixMask *RuinPixMaskCacheItem::generate(const RuinPixMaskCacheItem &i)
{
  Cityset *cs = Citysetlist::getInstance()->get(i.cityset);
  return cs->getRuin()->getImage(i.type)->copy();
}

int RuinPixMaskCacheItem::comp(const RuinPixMaskCacheItem &item) const
{
  return
    (cityset < item.cityset) ? -1 :
    (cityset > item.cityset)?  1 :
    (type < item.type) ? -1 :
    (type > item.type) ?  1 :
    0;
}

PixMask *DiplomacyPixMaskCacheItem::generate(const DiplomacyPixMaskCacheItem &i)
{
  PixMask *p =
    ImageCache::getInstance()->getDiplomacyImage
    (i.type, Player::DiplomaticState(i.state - Player::AT_PEACE))->copy();
  double ratio = 1;
  switch (i.type)
    {
    case 0:
      ratio = DIALOG_DIPLOMACY_TYPE_0_PIC_FONTSIZE_MULTIPLE;
      break;
    case 1:
      ratio = DIALOG_DIPLOMACY_TYPE_1_PIC_FONTSIZE_MULTIPLE;
      break;
    }

  double new_height = i.font_size * ratio;
  int new_width =
    ImageCache::calculate_width_from_adjusted_height (p, new_height);
  PixMask::scale (p, new_width, new_height);
  return p;
}

int DiplomacyPixMaskCacheItem::comp(const DiplomacyPixMaskCacheItem &item) const
{
  return
    (type < item.type) ? -1 :
    (type > item.type) ?  1 :
    (state < item.state) ? -1 :
    (state > item.state) ? 1 :
    (font_size < item.font_size) ? -1 :
    (font_size > item.font_size) ? 1 :
    0;
}

PixMask *RoadPixMaskCacheItem::generate(const RoadPixMaskCacheItem &i)
{
  Tileset *ts = Tilesetlist::getInstance()->get(i.tileset);
  return ts->getRoad()->getImage(i.type)->copy();
}

int RoadPixMaskCacheItem::comp(const RoadPixMaskCacheItem &item) const
{
  return
    (tileset < item.tileset) ? -1 :
    (tileset > item.tileset)?  1 :
    (type < item.type) ? -1 :
    (type > item.type) ?  1 :
    0;
}

PixMask *FogPixMaskCacheItem::generate(const FogPixMaskCacheItem &i)
{
  Tileset *ts = Tilesetlist::getInstance()->get(i.tileset);
  return ts->getFog()->getImage(i.type - 1)->copy();
}

int FogPixMaskCacheItem::comp(const FogPixMaskCacheItem &item) const
{
  return
    (tileset < item.tileset) ? -1 :
    (tileset > item.tileset)?  1 :
    (type < item.type) ? -1 :
    (type > item.type) ?  1 :
    0;
}

PixMask *BridgePixMaskCacheItem::generate(const BridgePixMaskCacheItem &i)
{
  Tileset *ts = Tilesetlist::getInstance()->get(i.tileset);
  return ts->getBridge()->getImage(i.type)->copy();
}

int BridgePixMaskCacheItem::comp(const BridgePixMaskCacheItem &item) const
{
  return
    (tileset < item.tileset) ? -1 :
    (tileset > item.tileset)?  1 :
    (type < item.type) ? -1 :
    (type > item.type) ?  1 :
    0;
}

PixMask *CursorPixMaskCacheItem::generate(const CursorPixMaskCacheItem &i)
{
  PixMask *p =
    ImageCache::getInstance()->getCursorImage(i.type)->copy();
  double ratio = DIALOG_CURSOR_PIC_FONTSIZE_MULTIPLE;
  double new_height = i.font_size * ratio;
  int new_width =
    ImageCache::calculate_width_from_adjusted_height (p, new_height);
  PixMask::scale (p, new_width, new_height);
  return p;
}

int CursorPixMaskCacheItem::comp(const CursorPixMaskCacheItem &item) const
{
  return
    (type < item.type) ? -1 :
    (type > item.type) ?  1 :
    (font_size < item.font_size) ? -1 :
    (font_size > item.font_size) ?  1 :
    0;
}

PixMask *ShieldPixMaskCacheItem::generate(const ShieldPixMaskCacheItem &i)
{
  ShieldStyle *sh = Shieldsetlist::getInstance()->getShield(i.shieldset,
                                                            i.type, i.color);
  PixMask *p =sh->getMaskedImage ()->applyMask
    (Shieldsetlist::getInstance ()->getColors (i.shieldset, i.color));
  if (i.map)
    return p;
  //okay now we size things accordingly.
  double height_ratio = 1.0;
  switch (i.type)
    {
    case 0:
      height_ratio = DIALOG_SMALL_SHIELD_PIC_FONTSIZE_MULTIPLE;
      break;
    case 1:
      height_ratio = DIALOG_MEDIUM_SHIELD_PIC_FONTSIZE_MULTIPLE;
      break;
    case 2:
      height_ratio = DIALOG_LARGE_SHIELD_PIC_FONTSIZE_MULTIPLE;
      break;
    }
  int new_height = i.font_size * height_ratio;
  int new_width =
    ImageCache::calculate_width_from_adjusted_height (p, new_height);
  PixMask::scale (p, new_width, new_height);
  return p;
}

int ShieldPixMaskCacheItem::comp(const ShieldPixMaskCacheItem &item) const
{
  return
    (shieldset < item.shieldset) ? -1 :
    (shieldset > item.shieldset) ?  1 :
    (type < item.type) ? -1 :
    (type > item.type) ?  1 :
    (color < item.color) ? -1 :
    (color > item.color) ?  1 :
    (map < item.map) ? -1 :
    (map > item.map) ?  1 :
    (font_size < item.font_size) ? -1 :
    (font_size > item.font_size) ?  1 :
    0;
}

PixMask *ProdShieldPixMaskCacheItem::generate(const ProdShieldPixMaskCacheItem &i)
{
  switch (i.type)
    {
    case 0: //home city
      if (i.prod) //production
        return ImageCache::getInstance()->getProdShieldImage(1)->copy();
      else //no production
        return ImageCache::getInstance()->getProdShieldImage(0)->copy();
      break;
    case 1: //away city
      if (i.prod) //production
        return ImageCache::getInstance()->getProdShieldImage(3)->copy();
      else //no production
        return ImageCache::getInstance()->getProdShieldImage(2)->copy();
      break;
    case 2: //destination city
      if (i.prod) //production
        return ImageCache::getInstance()->getProdShieldImage(5)->copy();
      else //no production
        return ImageCache::getInstance()->getProdShieldImage(4)->copy();
      break;
    case 3: //source city
      return ImageCache::getInstance()->getProdShieldImage(6)->copy();
      break;
    case 4: //invalid
      return ImageCache::getInstance()->getProdShieldImage(7)->copy();
      break;
    }
  return NULL;
}

int ProdShieldPixMaskCacheItem::comp(const ProdShieldPixMaskCacheItem &item) const
{
  return
    (type < item.type) ? -1 :
    (type > item.type) ?  1 :
    (prod < item.prod) ? -1 :
    (prod > item.prod) ?  1 :
    0;
}

std::vector<PixMask *> MoveBonusPixMaskCacheItem::getMoveBonusImages (Tileset *t, guint32 bonus, int &width, int &height,
                                                                      double wfrac)
{
  std::vector<PixMask *> im;

  if ((bonus & Tile::FOREST) == Tile::FOREST)
    {
      PixMask *p = t->getForestMoveBonus()->getImage ();
      if (p)
        im.push_back (p);
    }
  if ((bonus & Tile::HILLS) == Tile::HILLS)
    {
      PixMask *p = t->getHillsMoveBonus()->getImage ();
      if (p)
        im.push_back (p);
    }
  if ((bonus & Tile::MOUNTAIN) == Tile::MOUNTAIN)
    {
      PixMask *p = t->getMountainsMoveBonus()->getImage ();
      if (p)
        im.push_back (p);
    }
  if ((bonus & Tile::SWAMP) == Tile::SWAMP)
    {
      PixMask *p = t->getSwampMoveBonus()->getImage ();
      if (p)
        im.push_back (p);
    }

  width = 0;
  for (guint32 j = 0; j < im.size (); j++)
    width += (im[j]->get_width () * wfrac);

  height = 0;
  for (guint32 j = 0; j < im.size (); j++)
    if (im[j]->get_height () > height)
      height = im[j]->get_height ();

  if (width == 0 && height == 0)
    {
      width = 32;
      height = 20;
    }
  return im;
}

PixMask* MoveBonusPixMaskCacheItem::generateTwo (Tileset *t, guint32 bonus)
{
  // take the leftmost two thirds of the first, and  the rightmost two thirds
  // of the second
  int width, height;
  std::vector<PixMask *> im = getMoveBonusImages (t, bonus, width, height,
                                                  2.0/3.0);
 
  Glib::RefPtr<Gdk::Pixbuf> empty_pic =
    Gdk::Pixbuf::create(Gdk::COLORSPACE_RGB, true, 8, width, height);
  empty_pic->fill(0x00000000);
  PixMask *p = PixMask::create (empty_pic);
  if (im.size () != 2)
    return p;

  std::vector<PixMask*> parts;
  parts.push_back (im[0]->cropLeftTwoThirds ());
  parts.push_back (im[1]->cropRightTwoThirds ());

  guint32 x = 0;
  for (auto part : parts)
    {
      part->blit (p->get_pixmap (), x, 0);
      x += part->get_unscaled_width ();
    }

  for (auto part : parts)
    delete part;

  return p;
}

PixMask* MoveBonusPixMaskCacheItem::generateThree (Tileset *t, guint32 bonus)
{
  // take the leftmost half of the first, the center half of the second, and
  // the rightmost half of the third
  int width, height;
  std::vector<PixMask *> im = getMoveBonusImages (t, bonus, width, height,
                                                  1.0/2.0);

  Glib::RefPtr<Gdk::Pixbuf> empty_pic =
    Gdk::Pixbuf::create(Gdk::COLORSPACE_RGB, true, 8, width, height);
  empty_pic->fill(0x00000000);
  PixMask *p = PixMask::create (empty_pic);
  if (im.size () != 3)
    return p;

  std::vector<PixMask*> parts;
  parts.push_back(im[0]->cropLeftHalf ());
  parts.push_back(im[1]->cropCenterHalf ());
  parts.push_back(im[2]->cropRightHalf ());

  guint32 x = 0;
  for (auto part : parts)
    {
      part->blit (p->get_pixmap (), x, 0);
      x += part->get_unscaled_width ();
    }
  for (auto part : parts)
    delete part;

  return p;
}

PixMask* MoveBonusPixMaskCacheItem::generateFour (Tileset *t, guint32 bonus)
{
  //take the center half of all four
  int width, height;
  std::vector<PixMask *> im = getMoveBonusImages (t, bonus, width, height,
                                                  1.0/2.0);

  Glib::RefPtr<Gdk::Pixbuf> empty_pic =
    Gdk::Pixbuf::create(Gdk::COLORSPACE_RGB, true, 8, width, height);
  empty_pic->fill(0x00000000);
  PixMask *p = PixMask::create (empty_pic);
  if (im.size () != 4)
    return p;

  std::vector<PixMask*> parts;
  parts.push_back (im[0]->cropCenterHalf ());
  parts.push_back (im[1]->cropCenterHalf ());
  parts.push_back (im[2]->cropCenterHalf ());
  parts.push_back (im[3]->cropCenterHalf ());

  guint32 x = 0;
  for (auto part : parts)
    {
      part->blit (p->get_pixmap (), x, 0);
      x += part->get_unscaled_width ();
    }

  for (auto part : parts)
    delete part;

  return p;
}

PixMask *MoveBonusPixMaskCacheItem::getMoveBonusPic(Tileset *t, guint32 bonus, guint32 font_size,
                                                    double ratio)
{
  bool all = bonus == Tile::isFlying ();
  bool water = (bonus & Tile::WATER) == Tile::WATER;
  bool forest = (bonus & Tile::FOREST) == Tile::FOREST;
  bool hills = (bonus & Tile::HILLS) == Tile::HILLS;
  bool mountains = (bonus & Tile::MOUNTAIN) == Tile::MOUNTAIN;
  bool swamp = (bonus & Tile::SWAMP) == Tile::SWAMP;

  PixMask *p = NULL;
  if (all)
    {
      if (t->getAllMoveBonus()->getImage ())
        p = t->getAllMoveBonus()->getImage()->copy();
    }
  else if (water)
    {
      if (t->getWaterMoveBonus()->getImage ())
        p = t->getWaterMoveBonus()->getImage()->copy();
    }
  else if (forest || hills || mountains || swamp)
    {
      guint32 count = forest + hills + mountains + swamp;
      switch (count)
        {
        case 1:
          if (forest)
            {
              if (t->getForestMoveBonus()->getImage ())
                p = t->getForestMoveBonus()->getImage()->copy();
            }
          else if (hills)
            {
              if (t->getHillsMoveBonus()->getImage())
                p = t->getHillsMoveBonus()->getImage()->copy();
            }
          else if (mountains)
            {
              if (t->getMountainsMoveBonus()->getImage())
                p = t->getMountainsMoveBonus()->getImage()->copy();
            }
          else if (swamp)
            {
              if (t->getSwampMoveBonus()->getImage())
                p = t->getSwampMoveBonus()->getImage()->copy();
            }
          break;
        case 2:
          p = generateTwo (t, bonus);
          break;
        case 3:
          p = generateThree (t, bonus);
          break;
        case 4:
          p = generateFour (t, bonus);
          break;
        }
    }
  if (p == NULL)
    {
      Glib::RefPtr<Gdk::Pixbuf> empty_pic =
        Gdk::Pixbuf::create(Gdk::COLORSPACE_RGB, true, 8, 32, 20);
      empty_pic->fill(0x00000000);
      return PixMask::create (empty_pic);
    }

  //finally, scale it
  double new_height = font_size * ratio;
  int new_width =
    ImageCache::calculate_width_from_adjusted_height (p, new_height);
  PixMask::scale (p, new_width, new_height);

  return p;
}

PixMask *MoveBonusPixMaskCacheItem::generate(const MoveBonusPixMaskCacheItem &i)
{
  Tileset *t = Tilesetlist::getInstance()->get(i.tileset);
  return MoveBonusPixMaskCacheItem::getMoveBonusPic
    (t, i.bonus, i.font_size, DIALOG_MOVE_BONUS_PIC_FONTSIZE_MULTIPLE);
}

int MoveBonusPixMaskCacheItem::comp(const MoveBonusPixMaskCacheItem &item) const
{
  return
    (bonus < item.bonus) ? -1 :
    (bonus > item.bonus) ?  1 :
    (tileset < item.tileset) ? -1 :
    (tileset > item.tileset) ?  1 :
    (font_size < item.font_size) ? -1 :
    (font_size > item.font_size) ?  1 :
    0;
}

PixMask *ShipPixMaskCacheItem::generate(const ShipPixMaskCacheItem &i)
{
  // copy the pixmap including player colors
  if (i.player_id != MAX_PLAYERS)
    {
      TarFileMaskedImage * mim =
        Armysetlist::getInstance()->getShipPic(i.armyset);
      return mim->applyMask (i.player_id,
                             Playerlist::getInstance()->getPlayer(i.player_id));
    }
  else //we can put a neutral ship in the water in the editor
    {
      TarFileMaskedImage * mim =
        Armysetlist::getInstance()->getShipPic(i.armyset);

      return mim->getImage ()->copy ();
    }
}

int ShipPixMaskCacheItem::comp(const ShipPixMaskCacheItem &item) const
{
  return
    (player_id < item.player_id) ? -1 :
    (player_id > item.player_id) ?  1 :
    (armyset < item.armyset) ? -1 :
    (armyset > item.armyset) ?  1 :
    0;
}

PixMask *PlantedStandardPixMaskCacheItem::generate(const PlantedStandardPixMaskCacheItem &i)
{
  if (i.player_id != MAX_PLAYERS)
    {
      TarFileMaskedImage *mim =
        Armysetlist::getInstance()->getStandardPic (i.armyset);

      return mim->applyMask (i.player_id,
                             Playerlist::getInstance()->getPlayer(i.player_id));
    }
  else //we currently can't plant a neutral standard but just in case
    {
      TarFileMaskedImage * mim =
        Armysetlist::getInstance()->getStandardPic(i.armyset);

      return mim->getImage ()->copy ();
    }
}

int PlantedStandardPixMaskCacheItem::comp(const PlantedStandardPixMaskCacheItem &item) const
{
  return
    (player_id < item.player_id) ? -1 :
    (player_id > item.player_id) ?  1 :
    (armyset < item.armyset) ? -1 :
    (armyset > item.armyset) ?  1 :
    0;
}

PixMask *PortPixMaskCacheItem::generate(const PortPixMaskCacheItem &i)
{
  return
    Citysetlist::getInstance()->get(i.cityset)->getPort()->getImage()->copy();
}

int PortPixMaskCacheItem::comp(const PortPixMaskCacheItem &item) const
{
  return
    (cityset < item.cityset) ? -1 :
    (cityset > item.cityset) ?  1 :
    0;
}

PixMask *SignpostPixMaskCacheItem::generate(const SignpostPixMaskCacheItem &i)
{
  return Citysetlist::getInstance()->get(i.cityset)->getSignpost ()->getImage()->copy();
}

int SignpostPixMaskCacheItem::comp(const SignpostPixMaskCacheItem &item) const
{
  return
    (cityset < item.cityset) ? -1 :
    (cityset > item.cityset) ?  1 :
    0;
}

PixMask *BagPixMaskCacheItem::generate(const BagPixMaskCacheItem &i)
{
  return Armysetlist::getInstance()->getBag(i.armyset)->getImage ()->copy();
}

int BagPixMaskCacheItem::comp(const BagPixMaskCacheItem &item) const
{
  return
    (armyset < item.armyset) ? -1 :
    (armyset > item.armyset) ?  1 :
    0;
}

PixMask *ExplosionPixMaskCacheItem::generate(const ExplosionPixMaskCacheItem &i)
{
  return Tilesetlist::getInstance()->get(i.tileset)->getExplosion()->getImage()->copy();
}

int ExplosionPixMaskCacheItem::comp(const ExplosionPixMaskCacheItem &item) const
{
  return
    (tileset < item.tileset) ? -1 :
    (tileset > item.tileset) ?  1 :
    0;
}

PixMask *NewLevelPixMaskCacheItem::generate(const NewLevelPixMaskCacheItem &i)
{
  bool female = i.gender == Hero::FEMALE;
  TarFileMaskedImage *mim = 
    ImageCache::getInstance()->getHeroNewLevelMaskedImage (female);
  PixMask *p =
    mim->applyMask (Playerlist::getInstance()->getPlayer(i.player_id));

  double ratio = DIALOG_NEW_LEVEL_PIC_FONTSIZE_MULTIPLE;
  double new_height = i.font_size * ratio;
  int new_width =
    ImageCache::calculate_width_from_adjusted_height (p, new_height);
  PixMask::scale (p, new_width, new_height);
  return p;
}

int NewLevelPixMaskCacheItem::comp(const NewLevelPixMaskCacheItem &item) const
{
  return
    (player_id < item.player_id) ? -1 :
    (player_id > item.player_id) ?  1 :
    (gender < item.gender) ? -1 :
    (gender > item.gender) ?  1 :
    (font_size < item.font_size) ? -1 :
    (font_size > item.font_size) ?  1 :
    0;
}

PixMask *DefaultTileStylePixMaskCacheItem::generate(const DefaultTileStylePixMaskCacheItem &i)
{
  PixMask *t =
    ImageCache::getInstance()->getDefaultTileStyleImage(i.tilestyle_type);
  PixMask *s = t->copy();
  PixMask::scale(s, i.tilesize, i.tilesize);
  return s;
}

int DefaultTileStylePixMaskCacheItem::comp(const DefaultTileStylePixMaskCacheItem &item) const
{
  return
    (tilestyle_type < item.tilestyle_type) ? -1 :
    (tilestyle_type > item.tilestyle_type) ?  1 :
    (tilesize < item.tilesize) ? -1 :
    (tilesize > item.tilesize) ?  1 :
    0;
}

void TartanPixMaskCacheItem::calculateWidth(guint32 iwidth, PixMask *left, PixMask *center, PixMask *right, guint32 &width, guint32 &centers, bool &include_right)
{
  //calculate the width, ugh.
  for (width = left->get_width(); width < iwidth - right->get_width();
       width += center->get_width())
    centers++;
  width += right->get_width();
  include_right = true;
  for (guint32 j = 0; j < centers; j++)
    {
      if (width > iwidth)
        {
          width -= center->get_width();
          if (centers)
            centers--;
        }
      else
        break;
    }
  if (width > iwidth)
    {
      width -= right->get_width();
      include_right = false;
    }
}

PixMask *TartanPixMaskCacheItem::generate(const TartanPixMaskCacheItem &i)
{
  //okay, here's where we fashion the new image.
  //we take the leftmost tartan image for this player
  //and then we repeat the center tartan image a bunch of times
  //and then finally we cap it off with the rightmost tartan image
  //the images are all masked in the player's color.

  std::vector<Gdk::RGBA> colors =
    Shieldsetlist::getInstance()->getColors(i.shieldset, i.player_id);
  TarFileMaskedImage *mim =
    Shieldsetlist::getInstance()->getTartan(i.shieldset, i.player_id,
                                            Tartan::LEFT);
  Player *player = Playerlist::getInstance ()->getPlayer (i.player_id);
  PixMask *left = mim->applyMask (colors);
  double ratio = DIALOG_TARTAN_PIC_FONTSIZE_MULTIPLE;
  double new_height = i.font_size * ratio;
  int new_width =
    ImageCache::calculate_width_from_adjusted_height (left, new_height);
  PixMask::scale (left, new_width, new_height);

  mim = Shieldsetlist::getInstance()->getTartan(i.shieldset, i.player_id,
                                                Tartan::CENTER);
  PixMask *center = mim->applyMask (player);
  new_width =
    ImageCache::calculate_width_from_adjusted_height (center, new_height);
  PixMask::scale (center, new_width, new_height);
  mim =Shieldsetlist::getInstance()->getTartan(i.shieldset, i.player_id,
                                               Tartan::RIGHT);
  PixMask *right = mim->applyMask (player);
  new_width =
    ImageCache::calculate_width_from_adjusted_height (right, new_height);
  PixMask::scale (right, new_width, new_height);
  //okay, so we have our left, right and center images, now we need to
  //concatenate them together

  guint32 w = 0, num_centers = 0;
  bool include_right = false;
  calculateWidth(i.width, left, center, right, w, num_centers, include_right);
  //okay w is the actual width of the image, which is the same or less than
  //the width we asked for.  it has NUM_CENTERS center pieces and it may or may
  //not have an ending piece (if we can fit it.)
  //we always have the leftmost piece though because we have to show something.
  Glib::RefPtr<Gdk::Pixbuf> pixbuf
    = Gdk::Pixbuf::create(Gdk::COLORSPACE_RGB, true, 8, w, left->get_height());
  pixbuf->fill(0x00000000);
  PixMask *tartan = PixMask::create(pixbuf);

  //blit the left image
  guint32 l = 0;
  left->blit (tartan->get_pixmap(), l, 0);
  l += left->get_width();

  //blit the center images
  for (guint32 j = 0; j < num_centers; j++)
    {
      center->blit (tartan->get_pixmap(), l, 0);
      l += center->get_width();
    }

  //blit the right image
  if (include_right)
    right->blit (tartan->get_pixmap(), l, 0);
  delete left;
  delete center;
  delete right;
  return tartan;
}

int TartanPixMaskCacheItem::comp(const TartanPixMaskCacheItem &item) const
{
  return
    (player_id < item.player_id) ? -1 :
    (player_id > item.player_id) ?  1 :
    (width < item.width) ? -1 :
    (width > item.width) ?  1 :
    (shieldset < item.shieldset) ? -1 :
    (shieldset > item.shieldset) ?  1 :
    (font_size < item.font_size) ? -1 :
    (font_size > item.font_size) ?  1 :
    0;
}

PixMask *EmptyTartanPixMaskCacheItem::generate(const EmptyTartanPixMaskCacheItem &i)
{
  //okay, here's where we fashion the new image.
  //we take the leftmost tartan image for this player
  //and then we repeat the center tartan image a bunch of times
  //and then finally we cap it off with the rightmost tartan image
  //the images are all masked in the player's color.

  //the empty tartan pictures are the same as the regular tartan pictures
  //except they're not colored in the player's color.

  TarFileMaskedImage *mim =
    Shieldsetlist::getInstance()->getTartan(i.shieldset, i.player_id,
                                            Tartan::LEFT);
  PixMask *left = mim->getImage ()->copy();
  double ratio = DIALOG_TARTAN_PIC_FONTSIZE_MULTIPLE;
  double new_height = i.font_size * ratio;
  int new_width =
    ImageCache::calculate_width_from_adjusted_height (left, new_height);
  PixMask::scale (left, new_width, new_height);

  mim = Shieldsetlist::getInstance()->getTartan(i.shieldset, i.player_id,
                                                Tartan::CENTER);
  PixMask *center = mim->getImage ()->copy();
  new_width =
    ImageCache::calculate_width_from_adjusted_height (center, new_height);
  PixMask::scale (center, new_width, new_height);

  mim = Shieldsetlist::getInstance()->getTartan(i.shieldset, i.player_id,
                                                Tartan::RIGHT);
  PixMask *right = mim->getImage ()->copy();
  new_width =
    ImageCache::calculate_width_from_adjusted_height (right, new_height);
  PixMask::scale (right, new_width, new_height);

  //okay, so we have our left, right and center images, now we need to
  //concatenate them together

  guint32 w = 0, num_centers = 0;
  bool include_right = false;
  TartanPixMaskCacheItem::calculateWidth(i.width, left, center, right, w, num_centers, include_right);
  //okay w is the actual width of the image, which is the same or less than
  //the width we asked for.  it has NUM_CENTERS center pieces and it may or may
  //not have an ending piece (if we can fit it.)
  //we always have the leftmost piece though because we have to show something.
  Glib::RefPtr<Gdk::Pixbuf> pixbuf
    = Gdk::Pixbuf::create(Gdk::COLORSPACE_RGB, true, 8, w, left->get_height());
  pixbuf->fill(0x00000000);
  PixMask *tartan = PixMask::create(pixbuf);

  //blit the left image
  guint32 l = 0;
  left->blit (tartan->get_pixmap(), l, 0);
  l += left->get_width();

  //blit the center images
  for (guint32 j = 0; j < num_centers; j++)
    {
      center->blit (tartan->get_pixmap(), l, 0);
      l += center->get_width();
    }

  //blit the right image
  if (include_right)
    right->blit (tartan->get_pixmap(), l, 0);
  delete left;
  delete center;
  delete right;
  return tartan;
}

int EmptyTartanPixMaskCacheItem::comp(const EmptyTartanPixMaskCacheItem &item) const
{
  return
    (player_id < item.player_id) ? -1 :
    (player_id > item.player_id) ?  1 :
    (width < item.width) ? -1 :
    (width > item.width) ?  1 :
    (shieldset < item.shieldset) ? -1 :
    (shieldset > item.shieldset) ?  1 :
    (font_size < item.font_size) ? -1 :
    (font_size > item.font_size) ?  1 :
    0;
}

PixMask *StatusPixMaskCacheItem::generate(const StatusPixMaskCacheItem &i)
{
  Glib::ustring file = "";
  switch (i.type)
    {
    case ImageCache::STATUS_CITY:
      file = File::getVariousFile ("smallcity.png");
      break;
    case ImageCache::STATUS_TREASURY:
      file = File::getVariousFile ("smalltreasury.png");
      break;
    case ImageCache::STATUS_INCOME:
      file = File::getVariousFile ("smallincome.png");
      break;
    case ImageCache::STATUS_UPKEEP:
      file = File::getVariousFile ("smallupkeep.png");
      break;
    case ImageCache::STATUS_DEFENSE:
      file = File::getVariousFile ("smalldefense.png");
      break;
    }

  bool broken = false;
  PixMask *p = PixMask::create (file, broken);
  if (!broken)
    {
      double ratio = DIALOG_STATUS_PIC_FONTSIZE_MULTIPLE;
      if (i.type == ImageCache::STATUS_DEFENSE)
        ratio = DIALOG_DEFENSE_PIC_FONTSIZE_MULTIPLE;
      double new_height = i.font_size * ratio;
      int new_width =
        ImageCache::calculate_width_from_adjusted_height (p, new_height);
      PixMask::scale (p, new_width, new_height);
    }
  return p;
}

int StatusPixMaskCacheItem::comp(const StatusPixMaskCacheItem &item) const
{
  return
    (type < item.type) ? -1 :
    (type > item.type) ?  1 :
    (font_size < item.font_size) ? -1 :
    (font_size > item.font_size) ?  1 :
    0;
}

PixMask *GameButtonPixMaskCacheItem::generate(const GameButtonPixMaskCacheItem &i)
{
  PixMask *p =
    ImageCache::getInstance ()->getGameButtonImage (i.type)->copy ();

  if (p)
    {
      double ratio = DIALOG_GAME_BUTTON_PIC_FONTSIZE_MULTIPLE;
      double new_height = i.font_size * ratio;
      int new_width =
        ImageCache::calculate_width_from_adjusted_height (p, new_height);
      PixMask::scale (p, new_width, new_height);
    }
  return p;
}

int GameButtonPixMaskCacheItem::comp(const GameButtonPixMaskCacheItem &item) const
{
  return
    (type < item.type) ? -1 :
    (type > item.type) ?  1 :
    (font_size < item.font_size) ? -1 :
    (font_size > item.font_size) ?  1 :
    0;
}

PixMask *DialogPixMaskCacheItem::generate(const DialogPixMaskCacheItem &i)
{
  PixMask *p = NULL;
  double ratio = 1;
  ImageCache *ic = ImageCache::getInstance ();
  switch (i.type)
    {
    case ImageCache::DIALOG_NEXT_TURN:
      p = ic->getNextTurnImage ()->getImage ()->copy ();
      ratio = DIALOG_NEXT_TURN_PIC_FONT_SIZE_MULTIPLE;
      break;
    case ImageCache::DIALOG_NEW_HERO_MALE:
      p = ic->getHeroOfferedImage(Hero::MALE)->getImage ()->copy ();
      ratio = DIALOG_NEW_HERO_PIC_FONT_SIZE_MULTIPLE;
      break;
    case ImageCache::DIALOG_NEW_HERO_FEMALE:
      p = ic->getHeroOfferedImage(Hero::FEMALE)->getImage ()->copy ();
      ratio = DIALOG_NEW_HERO_PIC_FONT_SIZE_MULTIPLE;
      break;
    case ImageCache::DIALOG_CONQUERED_CITY:
      p = ic->getCityDefeatedImage ()->getImage ()->copy ();
      ratio = DIALOG_CONQUERED_CITY_PIC_FONT_SIZE_MULTIPLE;
      break;
    case ImageCache::DIALOG_WINNING:
      p = ic->getWinningImage()->getImage ()->copy ();
      ratio = DIALOG_WINNING_PIC_FONT_SIZE_MULTIPLE;
      break;
    case ImageCache::DIALOG_RUIN_SUCCESS:
      p = ic->getRuinSuccessImage()->getImage ()->copy ();
      ratio = DIALOG_RUIN_PIC_FONT_SIZE_MULTIPLE;
      break;
    case ImageCache::DIALOG_RUIN_DEFEAT:
      p = ic->getRuinDefeatImage()->getImage ()->copy ();
      ratio = DIALOG_RUIN_PIC_FONT_SIZE_MULTIPLE;
      break;
    case ImageCache::DIALOG_PARLEY_OFFERED:
      p = ic->getParleyOfferedImage()->getImage ()->copy ();
      ratio = DIALOG_PARLEY_PIC_FONT_SIZE_MULTIPLE;
      break;
    case ImageCache::DIALOG_PARLEY_REFUSED:
      p = ic->getParleyRefusedImage()->getImage ()->copy ();
      ratio = DIALOG_PARLEY_PIC_FONT_SIZE_MULTIPLE;
      break;
    case ImageCache::DIALOG_COMMENTATOR:
      p = ic->getCommentatorImage()->getImage ()->copy ();
      ratio = DIALOG_COMMENTATOR_PIC_FONT_SIZE_MULTIPLE;
      break;
    }
  if (p)
    {
      double new_height = i.font_size * ratio;
      int new_width =
        ImageCache::calculate_width_from_adjusted_height (p, new_height);
      PixMask::scale (p, new_width, new_height);
    }
  return p;
}

int DialogPixMaskCacheItem::comp(const DialogPixMaskCacheItem &item) const
{
  return
    (type < item.type) ? -1 :
    (type > item.type) ?  1 :
    (font_size < item.font_size) ? -1 :
    (font_size > item.font_size) ?  1 :
    0;
}

PixMask *MedalPixMaskCacheItem::generate(const MedalPixMaskCacheItem &i)
{
  PixMask *p =
    ImageCache::getInstance ()->getMedalImage (i.large, i.type)->copy ();

  if (i.large == false)
    return p;

  if (p)
    {
      double ratio = DIALOG_MEDAL_PIC_FONTSIZE_MULTIPLE;
      double new_height = i.font_size * ratio;
      int new_width =
        ImageCache::calculate_width_from_adjusted_height (p, new_height);
      PixMask::scale (p, new_width, new_height);
    }
  return p;
}

int MedalPixMaskCacheItem::comp(const MedalPixMaskCacheItem &item) const
{
  return
    (large < item.large) ? -1 :
    (large > item.large) ?  1 :
    (type < item.type) ? -1 :
    (type > item.type) ?  1 :
    (font_size < item.font_size) ? -1 :
    (font_size > item.font_size) ?  1 :
    0;
}
