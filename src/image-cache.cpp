//  Copyright (C) 2003, 2004, 2005, 2006, 2007 Ulf Lorenz
//  Copyright (C) 2004, 2005, 2006 Andrea Paternesi
//  Copyright (C) 2006, 2007, 2008, 2009, 2010, 2011, 2014, 2015, 2016, 2020,
//  2021, 2026 Ben Asselstine
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
//  Foundation, Inc., 31 Milk Street #960789, Boston, MA 02196, USA.

#include <sigc++/functors/mem_fun.h>
#include <iostream>
#include "image-cache.h"
#include "image-helpers.h"
#include "player-list.h"
#include "stack.h"
#include "player.h"
#include "tile-set-list.h"
#include "game-map.h"
#include "army-set-list.h"
#include "shield.h"
#include "city-set-list.h"
#include "shield-set-list.h"
#include "configuration.h"
#include "city.h"
#include "tile.h"
#include "ruin.h"
#include "temple.h"
#include "road.h"
#include "bridge.h"
#include "fog-map.h"
#include "shield-set.h"
#include "scenario-media.h"
#include "tar-file-masked-image.h"
#include "tar-file-image.h"

ImageCache* ImageCache::s_instance = 0;

ImageCache* ImageCache::instance()
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
    medalcache (c.medalcache), boxcache (c.boxcache)
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
    medalcache((sigc::ptr_fun(&MedalPixMaskCacheItem::generate))),
    boxcache((sigc::ptr_fun(&BoxPixMaskCacheItem::generate)))
{
    d_hero_newlevel[0] =
      new TarFileMaskedImage (TarFileMaskedImage::VERTICAL_MASK,
                              PixMask::DIMENSION_HEIGHT_IS_MARKED);
    d_hero_newlevel[1] =
      new TarFileMaskedImage (TarFileMaskedImage::VERTICAL_MASK,
                              PixMask::DIMENSION_HEIGHT_IS_MARKED);

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
  std::vector<PixMask*> diplomacy;
  diplomacy = disassemble_row(File::getVariousFile("diplomacy-small.png"),
                              DIPLOMACY_TYPES, broken);
  if (broken)
    return false;
  for (unsigned int i = 0; i < DIPLOMACY_TYPES ; i++)
    d_diplomacy[0][i] = diplomacy[i];

  diplomacy = disassemble_row(File::getVariousFile("diplomacy-large.png"),
                              DIPLOMACY_TYPES, broken);
  if (broken)
    return false;
  for (unsigned int i = 0; i < DIPLOMACY_TYPES ; i++)
    d_diplomacy[1][i] = diplomacy[i];
  return true;
}

bool ImageCache::loadCursorImages()
{
  bool broken = false;

  // load the cursor pictures
  std::vector<PixMask*> cursor;
  cursor = disassemble_row(File::getVariousFile("cursors.png"),
                           CURSOR_TYPES, broken);
  if (broken)
    return false;
  for (unsigned int i = 0; i < CURSOR_TYPES ; i++)
    d_cursor[i] = cursor[i];
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
  boxcache.reset();

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

  unsigned int num_players = Playerlist::instance()->countPlayersAlive();
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

  if (boxcache.size () >= 4)
    {
      d_cachesize -= boxcache.discardHalf();
      if (d_cachesize < maxcache)
        return;
    }
}

PixMask* ImageCache::getSelectorPic(guint32 type, guint32 frame,
                                    const Player *p)
{
  return getSelectorPic(type, frame, p,
                        GameMap::instance()->getTilesetId());
}

PixMask* ImageCache::getSelectorPic(guint32 type, guint32 frame,
                                    const Player *p, guint32 tileset)
{
  guint32 added = 0;
  SelectorPixMaskCacheItem i;
  i.tileset = tileset;
  i.type = type;
  i.frame = frame;
  i.shieldset = GameMap::instance ()->getShieldsetId ();
  i.shield_id = (guint32) p->get_shield ();
  i.armyset = p->getArmyset ();
  PixMask *s = selectorcache.get(i, added);
  d_cachesize += added;
  if (added)
    checkPictures();
  return s;
}

PixMask* ImageCache::getArmyPic(Army *a, bool greyed)
{
  return getArmyPic(a->getOwner()->getArmyset(), a->getTypeId(),
		    a->getOwner()->get_shield (), NULL, true, greyed);
}

PixMask* ImageCache::getDialogArmyPic(Army *a, bool greyed)
{
  return getArmyPic(a->getOwner()->getArmyset(), a->getTypeId(),
		    a->getOwner()->get_shield (), NULL, false, greyed);
}

PixMask* ImageCache::getArmyPic(guint32 armyset, guint32 army_id,
                                Shield::Color shield, const bool *medals,
                                bool map, bool greyed)
{
  guint added = 0;
  ArmyPixMaskCacheItem i;
  i.armyset = armyset;
  i.army_id = army_id;
  i.shieldset = GameMap::instance ()->getShieldsetId ();
  i.shield_id = (guint32) shield;
  for (guint32 j = 0; j < MEDAL_TYPES; j++)
    if (medals)
      i.medals[j] = medals[j];
    else
      i.medals[j] = false;
  i.map = map;
  i.greyed = greyed;
  PixMask *s = armycache.get(i, added);
  if (!s)
    {
      guint32 size = Armysetlist::instance()->get(i.armyset)->getTileSize();
      s = getDefaultTileStylePic(DEFAULT_TILESTYLE_TYPES-1, size);
    }
  d_cachesize += added;
  if (added)
    checkPictures();
  return s;
}

PixMask* ImageCache::getFlagPic(guint32 stack_size, Shield::Color shield, guint32 tileset)
{
  guint32 added = 0;
  FlagPixMaskCacheItem i;
  i.tileset = tileset;
  i.size = stack_size;
  i.shieldset = GameMap::instance ()->getShieldsetId ();
  i.shield_id = (guint32) shield;
  PixMask *s = flagcache.get(i, added);
  d_cachesize += added;
  if (added)
    checkPictures();
  return s;
}

PixMask* ImageCache::getFlagPic(guint32 stack_size, Shield::Color shield)
{
  return getFlagPic(stack_size, shield,
		    GameMap::instance()->getTilesetId());
}

PixMask* ImageCache::getFlagPic(const Stack* s)
{
  return getFlagPic(s->size (), s->getOwner ()->get_shield (),
                    GameMap::instance()->getTilesetId());
}

PixMask* ImageCache::getFlagPic(const Stack* s, guint32 tileset)
{
  return getFlagPic(s->size(), s->getOwner()->get_shield (), tileset);
}

PixMask* ImageCache::getCircledArmyPic(Army *a, bool greyed,
                                       guint32 circle_color_id,
                                       bool show_army, bool dark)
{
  return getCircledArmyPic(a->getOwner()->getArmyset(), a->getTypeId(),
		    a->getOwner()->get_shield (), NULL, greyed, circle_color_id, show_army, dark);
}

PixMask* ImageCache::getEmptyCircledArmyPic (bool dark)
{
  Player *p = Playerlist::getActiveplayer ();
  guint added = 0;
  CircledArmyPixMaskCacheItem i;
  i.armyset = p->getArmyset();
  i.shieldset = GameMap::instance ()->getShieldsetId ();
  i.army_id = 0;
  i.shield_id = 0;
  for (guint32 j = 0; j < MEDAL_TYPES; j++)
    i.medals[j] = false;
  i.greyed = false;
  i.circle_color_id = Shield::NEUTRAL;
  i.show_army = false;
  i.dark = dark;
  PixMask *s = circledarmycache.get(i, added);
  d_cachesize += added;
  if (added)
    checkPictures();
  return s;
}

PixMask* ImageCache::getCircledArmyPic(guint32 armyset, guint32 army_id,
                                             Shield::Color shield,
                                             const bool *medals, bool greyed,
                                             guint32 circle_color_id,
                                             bool show_army, bool dark)
{
  guint added = 0;
  CircledArmyPixMaskCacheItem i;
  i.armyset = armyset;
  i.army_id = army_id;
  i.shield_id = (guint32) shield;
  i.shieldset = GameMap::instance ()->getShieldsetId ();
  for (guint32 j = 0; j < MEDAL_TYPES; j++)
    if (medals)
      i.medals[j] = medals[j];
    else
      i.medals[j] = false;
  i.greyed = greyed;
  i.circle_color_id = circle_color_id;
  i.show_army = show_army;
  i.dark = dark;
  PixMask *s = circledarmycache.get(i, added);
  d_cachesize += added;
  if (added)
    checkPictures();
  return s;
}

PixMask* ImageCache::getCircledShipPic(guint32 armyset, Shield::Color shield,
                                       bool greyed, guint32 circle_color_id,
                                       bool dark)
{
  guint added = 0;
  CircledShipPixMaskCacheItem i;
  i.armyset = armyset;
  i.shieldset = GameMap::instance ()->getShieldsetId ();
  i.shield_id = (guint32) shield;
  i.greyed = greyed;
  i.circle_color_id = circle_color_id;
  i.dark = dark;
  PixMask *s = circledshipcache.get(i, added);
  d_cachesize += added;
  if (added)
    checkPictures();
  return s;
}

PixMask* ImageCache::getCircledStandardPic(guint32 armyset,
                                           Shield::Color shield,
                                           bool greyed,
                                           guint32 circle_color_id,
                                           bool dark)
{
  guint added = 0;
  CircledStandardPixMaskCacheItem i;
  i.armyset = armyset;
  i.shieldset = GameMap::instance ()->getShieldsetId ();
  i.shield_id = (guint32) shield;
  i.greyed = greyed;
  i.circle_color_id = circle_color_id;
  i.dark = dark;
  PixMask *s = circledstandardcache.get(i, added);
  d_cachesize += added;
  if (added)
    checkPictures();
  return s;
}

PixMask* ImageCache::getTilePic(int tile_style_id, int fog_type_id, bool has_bag, int bag_player_id, bool has_standard, int standard_player_id, int stack_size, int stack_player_id, int army_type_id, bool has_tower, bool has_ship, Maptile::Building building_type, int building_subtype, Vector<int> building_tile, int building_player_id, guint32 tilesize, bool has_grid, int stone_type)
{
  guint32 tileset = GameMap::instance()->getTilesetId();
  guint32 cityset = GameMap::instance()->getCitysetId();
  guint32 shieldset = GameMap::instance()->getShieldsetId();
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
  //building type for cities, -1 razed, 0 normal, others are their type
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
  guint32 cityset = GameMap::instance()->getCitysetId();
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
  guint32 cityset = GameMap::instance()->getCitysetId();
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
  guint32 cityset = GameMap::instance()->getCitysetId();
  return getTemplePic(t->getType(), cityset);
}

PixMask* ImageCache::getTemplePic(int type)
{
  guint32 cityset = GameMap::instance()->getCitysetId();
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
  guint32 cityset = GameMap::instance()->getCitysetId();
  return getRuinPic(ruin->getType(), cityset);
}
PixMask* ImageCache::getRuinPic(int type)
{
  guint32 cityset = GameMap::instance()->getCitysetId();
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

PixMask* ImageCache::getDiplomacyPic(int type, Player::DiplomaticState state)
{
  guint added = 0;
  DiplomacyPixMaskCacheItem i;
  i.type = type;
  i.state = state;
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
  return getRoadPic(type, GameMap::instance()->getTilesetId());
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
  return getFogPic(type, GameMap::instance()->getTilesetId());
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
  return getBridgePic(type, GameMap::instance()->getTilesetId());
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

PixMask* ImageCache::getCursorPic(int type)
{
  guint added = 0;
  CursorPixMaskCacheItem i;
  i.type = type;
  PixMask *s = cursorcache.get(i, added);
  d_cachesize += added;
  if (added)
    checkPictures();
  return s;
}

PixMask* ImageCache::getShieldPic(guint32 type, Player *p, bool map)
{
  guint32 shieldset = GameMap::instance()->getShieldsetId();
  return getShieldPic(shieldset, type, p->get_shield (), map);
}

PixMask* ImageCache::getShieldPic(guint32 shieldset, guint32 type,
                                        guint32 color, bool map)
{
  guint added = 0;
  ShieldPixMaskCacheItem i;
  i.type = type;
  i.shieldset = shieldset;
  i.color = color;
  i.map = map;
  PixMask *s = shieldcache.get(i, added);
  d_cachesize += added;
  if (added)
    checkPictures();
  return s;
}

PixMask* ImageCache::getStatusPic(guint32 type)
{
  guint added = 0;
  StatusPixMaskCacheItem i;
  i.type = type;
  PixMask *s = statuscache.get(i, added);
  d_cachesize += added;
  if (added)
    checkPictures();
  return s;
}

PixMask* ImageCache::getGameButtonPic(guint32 type)
{
  guint added = 0;
  GameButtonPixMaskCacheItem i;
  i.type = type;
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

PixMask* ImageCache::getShipPic(guint32 armyset, Shield::Color shield)
{
  guint added = 0;
  ShipPixMaskCacheItem i;
  i.shield_id = (guint32) shield;
  i.armyset = armyset;
  i.shieldset = GameMap::instance ()->getShieldsetId ();
  PixMask *s = shipcache.get(i, added);
  if (!s)
    {
      guint32 size = Armysetlist::instance()->get(i.armyset)->getTileSize();
      s = getDefaultTileStylePic(DEFAULT_TILESTYLE_TYPES-1, size);
    }
  d_cachesize += added;
  if (added)
    checkPictures();
  return s;
}

PixMask* ImageCache::getPlantedStandardPic(guint32 armyset, Shield::Color shield)
{
  guint added = 0;
  PlantedStandardPixMaskCacheItem i;
  i.shieldset = GameMap::instance ()->getShieldsetId ();
  i.shield_id = (guint32) shield;
  i.armyset = armyset;
  PixMask *s = plantedstandardcache.get(i, added);
  if (!s)
    {
      guint32 size = Armysetlist::instance()->get(i.armyset)->getTileSize();
      s = getDefaultTileStylePic(DEFAULT_TILESTYLE_TYPES-1, size);
    }
  d_cachesize += added;
  if (added)
    checkPictures();
  return s;
}

PixMask* ImageCache::getPortPic()
{
  return getPortPic(GameMap::instance()->getCitysetId());
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
  return getSignpostPic(GameMap::instance()->getCitysetId());
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
      guint32 size = Armysetlist::instance()->get(i.armyset)->getTileSize();
      s = getDefaultTileStylePic(DEFAULT_TILESTYLE_TYPES-1, size);
    }
  d_cachesize += added;
  if (added)
    checkPictures();
  return s;
}

PixMask* ImageCache::getExplosionPic()
{
  return getExplosionPic(GameMap::instance()->getTilesetId());
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

PixMask* ImageCache::getNewLevelPic(Shield::Color shield, guint32 gender)
{
  guint added = 0;
  NewLevelPixMaskCacheItem i;
  i.shieldset = GameMap::instance ()->getShieldsetId ();
  i.shield_id = (guint32) shield;
  i.gender = gender;
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

PixMask* ImageCache::getTartanPic(Shieldset *shieldset, Shield::Color shield,
                                  guint32 width)
{
  guint added = 0;
  TartanPixMaskCacheItem i;
  i.shield = shield;
  i.width = width;
  i.shieldset = shieldset->getId();
  PixMask *s = tartancache.get(i, added);
  d_cachesize += added;
  if (added)
    checkPictures();
  return s;
}

PixMask* ImageCache::getEmptyTartanPic(Shieldset *shieldset,
                                       Shield::Color shield, guint32 width)
{
  guint added = 0;
  EmptyTartanPixMaskCacheItem i;
  i.shield = shield;
  i.width = width;
  i.shieldset = shieldset->getId();
  PixMask *s = emptytartancache.get(i, added);
  d_cachesize += added;
  if (added)
    checkPictures();
  return s;
}

PixMask* ImageCache::getDialogPic(guint32 type)
{
  guint added = 0;
  DialogPixMaskCacheItem i;
  i.type = type;
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

PixMask* ImageCache::getMoveBonusPic(guint32 tileset_id, guint32 bonus)
{
  guint added = 0;
  MoveBonusPixMaskCacheItem i;
  i.bonus = bonus;
  i.tileset = tileset_id;

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
  ScenarioMedia *sm = ScenarioMedia::instance ();
  if (sm->getMedalImage (large)->getName() != "")
    return sm->getMedalImage(large)->getImage (type);
  else
    return getMedalImage (large)->getImage (type);
}

TarFileMaskedImage *ImageCache::getHeroNewLevelMaskedImage (bool female)
{
  TarFileMaskedImage *mim =
    ScenarioMedia::instance()->getHeroNewLevelMaskedImage(female);
  if (mim->getImage ())
    return mim;
  return d_hero_newlevel[female ? 1 : 0];
}

PixMask* ImageCache::getMedalPic(bool large, guint32 type)
{
  guint added = 0;
  MedalPixMaskCacheItem i;
  i.large = large;
  i.type = type;
  PixMask *s = medalcache.get(i, added);
  d_cachesize += added;
  if (added)
    checkPictures();
  return s;
}

PixMask* ImageCache::getBoxPic (guint32 size, Gdk::RGBA color, bool rounded,
                                bool dashed, int line_width, bool offset)
{
  guint added = 0;
  BoxPixMaskCacheItem i;
  i.size = size;
  i.color = color;
  i.rounded = rounded;
  i.dashed = dashed;
  i.line_width = line_width;
  i.offset = offset;
  PixMask *s = boxcache.get(i, added);
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
    Gdk::Pixbuf::create_from_data(copy, Gdk::Colorspace::RGB, true, 8,
				  width, height, width * 4);

  result->draw_pixbuf(greyed_out, 0, 0, 0, 0, width, height);
  free(copy);

  return result;
}

void ImageCache::draw_circle(Cairo::RefPtr<Cairo::Context> cr, double width_percent, int width, int height, Gdk::RGBA color, bool dark, bool colored, bool mask)
{
  if (width_percent > 100)
    width_percent = 0;
  else if (width_percent < 0)
    width_percent = 0;
  width_percent /= 100.0;
  //i want 2 o'clock as a starting point, and 8pm as an ending point.

  Gdk::RGBA l, d;
  get_bevel_colors (dark, l, d);
  double dred = d.get_red();
  double dgreen = d.get_green();
  double dblue = d.get_blue();
  double lred = l.get_red();
  double lgreen = l.get_green();
  double lblue =  l.get_blue();

  double radius = (double)width * width_percent / 2.0;
  double line_width = radius * 0.2;

  cr->set_line_width(line_width + 2.0);
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

void ImageCache::draw_circle_outline(Cairo::RefPtr<Cairo::Context> cr, double width_percent, int width, int height, bool dark)
{
  if (width_percent > 100)
    width_percent = 0;
  else if (width_percent < 0)
    width_percent = 0;
  width_percent /= 100.0;
  //i want 2 o'clock as a starting point, and 8pm as an ending point.

  Gdk::RGBA l, d;
  get_bevel_colors (dark, l, d);
  double ired = d.get_red();
  double igreen = d.get_green();
  double iblue = d.get_blue();
  double ored = l.get_red();
  double ogreen = l.get_green();
  double oblue = l.get_blue();

  double outer_radius = (double)width * width_percent / 2.0;
  double inner_radius = (double)width * width_percent / 2.0 / 1.5;

  cr->set_line_width(1.0);
  cr->set_source_rgba(ired, igreen, iblue, 0.8);
  cr->arc((double)width/2.0, (double)height/2.0, outer_radius, 0, 2 * M_PI);
  cr->stroke();

  cr->set_source_rgba(ored, ogreen, oblue, 0.8);
  cr->arc((double)width/2.0, (double)height/2.0, inner_radius, 0, 2 * M_PI);

  cr->stroke();

}

PixMask* ImageCache::circled(PixMask* image, Gdk::RGBA color, bool dark, bool colored, double width_percent)
{
  PixMask *copy = image->copy ();
  int width = image->get_width ();
  int height = image->get_height ();
  //here we draw a colored circle on top of the army's image
  Cairo::RefPtr<Cairo::Context> cr =
    Cairo::Context::create (copy->get_pixmap ());

  draw_circle (cr, width_percent, width, height, color, dark, colored, false);

  //here we draw a white circle on a copy of the image's mask.
  Cairo::RefPtr<Cairo::Surface> mask = copy->get_mask ();

  cr = Cairo::Context::create (mask);
  auto white = Gdk::RGBA ("white");
  draw_circle (cr, width_percent, width, height, white, dark, colored,
               true);
  PixMask *result = PixMask::create(copy->get_pixmap (), mask);
  //draw the army on top again, to make it look like the circle is behind.
  result->draw_pixbuf (image->to_pixbuf (), 0, 0, 0, 0, width, height);
  delete copy;
  return result;
}

PixMask* ImageCache::circle_outline (PixMask* image, bool dark, double width_percent)
{
  PixMask *copy = image->copy ();
  int width = image->get_width ();
  int height = image->get_height ();
  //here we draw a colored circle on top of the army's image
  Cairo::RefPtr<Cairo::Context> cr =
    Cairo::Context::create (copy->get_pixmap ());

  draw_circle_outline (cr, width_percent, width, height, dark);

  copy->draw_pixbuf (image->to_pixbuf (), 0, 0, 0, 0, width, height);
  return copy;
}
TarFileImage* ImageCache::getNextTurnImage ()
{
  TarFileImage *im = ScenarioMedia::instance()->getNextTurnImage();
  if (im->getImage ())
    return im;
  return d_next_turn;
}

TarFileImage* ImageCache::getCityDefeatedImage ()
{
  TarFileImage *im = ScenarioMedia::instance()->getCityDefeatedImage();
  if (im->getImage ())
    return im;
  return d_city_defeated;
}

TarFileImage * ImageCache::getWinningImage ()
{
  TarFileImage *im = ScenarioMedia::instance()->getWinningImage();
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
            ScenarioMedia::instance()->getHeroOfferedImage(false);
          if (im->getImage ())
            return im;
          return d_hero[0];
        }
      break;
    case Hero::FEMALE:
        {
          TarFileImage *im =
            ScenarioMedia::instance()->getHeroOfferedImage(true);
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
  TarFileImage *im = ScenarioMedia::instance()->getRuinSuccessImage();
  if (im->getImage ())
    return im;
  return d_ruin_success;
}

TarFileImage *ImageCache::getRuinDefeatImage()
{
  TarFileImage *im = ScenarioMedia::instance()->getRuinDefeatImage();
  if (im->getImage ())
    return im;
  return d_ruin_defeat;
}

TarFileImage * ImageCache::getParleyOfferedImage ()
{
  TarFileImage *im = ScenarioMedia::instance()->getParleyOfferedImage();
  if (im->getImage ())
    return im;
  return d_parley_offered;
}

TarFileImage* ImageCache::getParleyRefusedImage ()
{
  TarFileImage *im = ScenarioMedia::instance()->getParleyRefusedImage();
  if (im->getImage ())
    return im;
  return d_parley_refused;
}

TarFileImage* ImageCache::getMedalImage (bool large)
{
  TarFileImage *im = ScenarioMedia::instance()->getMedalImage(large);
  if (im->getImage ())
    return im;
  return d_medal[large ? 1 : 0];
}

TarFileImage* ImageCache::getCommentatorImage ()
{
  TarFileImage *im = ScenarioMedia::instance()->getCommentatorImage();
  if (im->getImage ())
    return im;
  return d_commentator;
}

int ImageCache::calculate_width_from_adjusted_height (PixMask *p, double new_height)
{
  return p->get_width () * (new_height / p->get_height ());
}

PixMask *SelectorPixMaskCacheItem::generate(const SelectorPixMaskCacheItem &i)
{
  // armyset selectors override the tileset ones
  // we can't have a neutral selector, but just in case we change it to white
  Shield::Color c = Shield::Color (i.shield_id);
  if (c == Shield::NEUTRAL)
    c = Shield::WHITE;
  Armyset *as = Armysetlist::instance ()->get (i.armyset);
  Tileset *ts = Tilesetlist::instance()->get(i.tileset);
  if (i.type == 0)
    {
      if (as->getSelector(true, c)->getNumberOfFrames () > 0)
        return as->getSelector(true, c)->applyMask (i.frame, i.shield_id);
      else
        return ts->getSelector(true)->applyMask (i.frame, i.shield_id);
    }
  else
    {
      if (as->getSelector(false, c)->getNumberOfFrames () > 0)
        return as->getSelector(false, c)->applyMask (i.frame, i.shield_id);
      else
        return ts->getSelector(false)->applyMask (i.frame, i.shield_id);
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
    (armyset < item.armyset) ? -1 :
    (armyset > item.armyset) ?  1 :
    (shieldset < item.shieldset) ? -1 :
    (shieldset > item.shieldset) ?  1 :
    (shield_id < item.shield_id) ? -1 :
    (shield_id > item.shield_id) ?  1 :
    0;
}

PixMask *FlagPixMaskCacheItem::generate(const FlagPixMaskCacheItem &i)
{
  Tileset *ts = Tilesetlist::instance()->get(i.tileset);

  /**
   * if you're here and i.size is zero it means:
   * bigmap tried to lookup the number of stacks belonging to a player on
   * a tile, and it got nothing
   * so the ownership stuff has been messed up
   */
  // size of stack starts at 1, but we need the index, which starts at 0
  Shield::Color shield = Shield::Color (i.shield_id);
  return ts->getFlags (shield)->applyMask (i.size - 1, i.shield_id);
}

int FlagPixMaskCacheItem::comp(const FlagPixMaskCacheItem &item) const
{
  return
    (tileset < item.tileset) ? -1 :
    (tileset > item.tileset)?  1 :
    (size < item.size) ? -1 :
    (size > item.size) ?  1 :
    (shieldset < item.shieldset) ? -1 :
    (shieldset > item.shieldset) ?  1 :
    (shield_id < item.shield_id) ? -1 :
    (shield_id > item.shield_id) ?  1 :
    0;
}

PixMask *ArmyPixMaskCacheItem::generate(const ArmyPixMaskCacheItem &i)
{
  PixMask *s;
  const ArmyProto * basearmy =
    Armysetlist::instance()->getArmy(i.armyset, i.army_id);

  // copy the pixmap including player colors
  PixMask *colored =
    basearmy->getMaskedImage (Shield::Color (i.shield_id))->applyMask (i.shield_id);
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
        ImageCache::instance()->getMedalImage(false, j)->blit(s->get_pixmap());
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
    (shieldset < item.shieldset) ? -1 :
    (shieldset > item.shieldset) ?  1 :
    (shield_id < item.shield_id) ? -1 :
    (shield_id > item.shield_id) ?  1 :
    (memcmp(medals,item.medals,sizeof(medals)) < 0) ? -1 :
    (memcmp(medals,item.medals,sizeof(medals)) > 0) ? 1 :
    (map < item.map) ? -1 :
    (map > item.map) ?  1 :
    (greyed < item.greyed) ? -1 :
    (greyed > item.greyed) ?  1 :
    0;
}

PixMask *CircledArmyPixMaskCacheItem::generate(const CircledArmyPixMaskCacheItem &i)
{
  PixMask *s;
  if (i.show_army)
    {
      PixMask *pre_circle =
        ImageCache::instance ()->getArmyPic (i.armyset, i.army_id,
                                             Shield::Color (i.shield_id),
                                             i.medals, false, i.greyed);
      if (i.shield_id != i.circle_color_id)
        s = ImageCache::circle_outline (pre_circle, i.dark);
      else
        {
          auto ssl = Shieldsetlist::instance ()->get (i.shieldset);
          auto color = ssl->getColor (i.shield_id);
          s = ImageCache::circled (pre_circle, color, i.dark,
                                   i.circle_color_id != Shield::NEUTRAL);
        }
    }
  else
    {
      guint32 size = Armysetlist::instance ()->get (i.armyset)->getTileSize ();
      Glib::RefPtr<Gdk::Pixbuf> empty_pic =
        Gdk::Pixbuf::create (Gdk::Colorspace::RGB, true, 8, size, size);
      empty_pic->fill (0x00000000);
      auto pre_circle = PixMask::create (empty_pic);
      s = ImageCache::circle_outline (pre_circle, i.dark);
      delete pre_circle;
    }
  return s;
}

int CircledArmyPixMaskCacheItem::comp(const CircledArmyPixMaskCacheItem &item) const
{
  return
    (armyset < item.armyset) ? -1 :
    (armyset > item.armyset)?  1 :
    (army_id < item.army_id) ? -1 :
    (army_id > item.army_id) ?  1 :
    (shield_id < item.shield_id) ? -1 :
    (shield_id > item.shield_id) ?  1 :
    (shieldset < item.shieldset) ? -1 :
    (shieldset > item.shieldset) ?  1 :
    (memcmp(medals,item.medals,sizeof(medals)) < 0) ? -1 :
    (memcmp(medals,item.medals,sizeof(medals)) > 0) ? 1 :
    (greyed < item.greyed) ?  -1 :
    (greyed > item.greyed) ?  1 :
    (circle_color_id < item.circle_color_id) ?  -1 :
    (circle_color_id > item.circle_color_id) ?  1 :
    (show_army < item.show_army) ?  -1 :
    (show_army > item.show_army) ?  1 :
    (dark < item.dark) ?  -1 :
    (dark > item.dark) ?  1 :
    0;
}

PixMask *CircledShipPixMaskCacheItem::generate(const CircledShipPixMaskCacheItem &i)
{
  PixMask *s;
  PixMask *pre_circle =
    ImageCache::instance ()->getShipPic (i.armyset,
                                         Shield::Color (i.shield_id));
  s = ImageCache::circle_outline (pre_circle, i.dark);
  return s;
}

int CircledShipPixMaskCacheItem::comp(const CircledShipPixMaskCacheItem &item) const
{
  return
    (armyset < item.armyset) ? -1 :
    (armyset > item.armyset)?  1 :
    (shieldset < item.shieldset) ? -1 :
    (shieldset > item.shieldset)?  1 :
    (shield_id < item.shield_id) ? -1 :
    (shield_id > item.shield_id) ?  1 :
    (greyed < item.greyed) ?  -1 :
    (greyed > item.greyed) ?  1 :
    (circle_color_id < item.circle_color_id) ?  -1 :
    (circle_color_id > item.circle_color_id) ?  1 :
    (dark < item.dark) ?  -1 :
    (dark > item.dark) ?  1 :
    0;
}

PixMask *CircledStandardPixMaskCacheItem::generate(const CircledStandardPixMaskCacheItem &i)
{
  PixMask *s;
  PixMask *pre_circle =
    ImageCache::instance ()->getPlantedStandardPic
    (i.armyset, Shield::Color (i.shield_id));
          
  s = ImageCache::circle_outline (pre_circle, i.dark);
  return s;
}

int CircledStandardPixMaskCacheItem::comp(const CircledStandardPixMaskCacheItem &item) const
{
  return
    (armyset < item.armyset) ? -1 :
    (armyset > item.armyset)?  1 :
    (shieldset < item.shieldset) ? -1 :
    (shieldset > item.shieldset)?  1 :
    (shield_id < item.shield_id) ? -1 :
    (shield_id > item.shield_id) ?  1 :
    (greyed < item.greyed) ?  -1 :
    (greyed > item.greyed) ?  1 :
    (circle_color_id < item.circle_color_id) ?  -1 :
    (circle_color_id > item.circle_color_id) ?  1 :
    (dark < item.dark) ?  -1 :
    (dark > item.dark) ?  1 :
    0;
}

PixMask *TilePixMaskCacheItem::generate(const TilePixMaskCacheItem &i)
{
  PixMask *s;
  Tileset *t = Tilesetlist::instance()->get(i.tileset);
  guint32 uts = t->getTileSize();
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
              player = Playerlist::instance()->get (i.building_player_id);
              int span =
                Citysetlist::instance ()->get (i.cityset)->getCityTileWidth ();
              ImageCache::instance ()->getCityPic
                (i.building_subtype, player, i.cityset)->blit
                (i.building_tile, span, uts, pixmap);
            }
          break;
        case Maptile::RUIN:
            {
              int span =
                Citysetlist::instance ()->get (i.cityset)->getRuinTileWidth ();
              ImageCache::instance ()->getRuinPic
                (i.building_subtype, i.cityset)->blit
                (i.building_tile, span, uts, pixmap);
            }
          break;
        case Maptile::TEMPLE:
            {
              int span =
                Citysetlist::instance ()->get
                (i.cityset)->getTempleTileWidth ();
              ImageCache::instance ()->getTemplePic
                (i.building_subtype, i.cityset)->blit
                (i.building_tile, span, uts, pixmap);
            }
          break;
        case Maptile::SIGNPOST:
          ImageCache::instance()->getSignpostPic(i.cityset)->blit(i.building_tile, uts, pixmap);
          break;
        case Maptile::ROAD:
          ImageCache::instance()->getRoadPic(i.building_subtype)->blit(i.building_tile, uts, pixmap);
          if (i.stone_type != -1)
            {
              Tileset *ts = Tilesetlist::instance()->get(i.tileset);
              PixMask *p = ts->getStone()->getImage(i.stone_type);
              if (p)
                p->blit(i.building_tile, uts, pixmap);
            }
          break;
        case Maptile::STONE:
            {
              Tileset *ts = Tilesetlist::instance()->get(i.tileset);
              PixMask *p = ts->getStone()->getImage(i.stone_type);
              if (p)
                p->blit(i.building_tile, uts, pixmap);
            }
          break;
        case Maptile::PORT:
          ImageCache::instance()->getPortPic(i.cityset)->blit(i.building_tile, uts, pixmap);
          break;
        case Maptile::BRIDGE:
          ImageCache::instance()->getBridgePic(i.building_subtype)->blit(i.building_tile, uts, pixmap);
          break;
        case Maptile::NONE: default:
          break;
        }

      if (i.has_bag)
        {
          PixMask *pic = ImageCache::instance()->getBagPic();
          pic->blit(pixmap);
        }

      if (i.has_standard)
        {
          player = Playerlist::instance()->get (i.standard_player_id) ;
          ImageCache::instance()->getPlantedStandardPic
            (player->getArmyset (), player->get_shield ())->blit(pixmap);
        }

      if (i.stack_player_id > -1)
        {
          player = Playerlist::instance()->get (i.stack_player_id);
          if (i.has_tower)
            ImageCache::instance()->getTowerPic(player)->blit(pixmap);
          else
            {
              if (i.stack_size > -1)
                ImageCache::instance()->getFlagPic(i.stack_size, player->get_shield ())->blit(pixmap);
              if (i.has_ship)
                ImageCache::instance()->getShipPic
                  (player->getArmyset (), player->get_shield ())->blit(pixmap);
              else
                ImageCache::instance()->getArmyPic(player->getArmyset(), i.army_type_id, player->get_shield (), NULL, true, 0)->blit(pixmap);
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
  Cityset *cs = Citysetlist::instance()->get(i.cityset);
  Player *p = Playerlist::instance()->get (i.player_id);
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
  Cityset *cs = Citysetlist::instance()->get(i.cityset);
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
  Cityset *cs = Citysetlist::instance()->get(i.cityset);
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
  Cityset *cs = Citysetlist::instance()->get(i.cityset);
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
    ImageCache::instance()->getDiplomacyImage
    (i.type, Player::DiplomaticState(i.state - Player::AT_PEACE))->copy();
  return p;
}

int DiplomacyPixMaskCacheItem::comp(const DiplomacyPixMaskCacheItem &item) const
{
  return
    (type < item.type) ? -1 :
    (type > item.type) ?  1 :
    (state < item.state) ? -1 :
    (state > item.state) ? 1 :
    0;
}

PixMask *RoadPixMaskCacheItem::generate(const RoadPixMaskCacheItem &i)
{
  Tileset *ts = Tilesetlist::instance()->get(i.tileset);
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
  Tileset *ts = Tilesetlist::instance()->get(i.tileset);
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
  Tileset *ts = Tilesetlist::instance()->get(i.tileset);
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
    ImageCache::instance()->getCursorImage(i.type)->copy();
  return p;
}

int CursorPixMaskCacheItem::comp(const CursorPixMaskCacheItem &item) const
{
  return
    (type < item.type) ? -1 :
    (type > item.type) ?  1 :
    0;
}

PixMask *ShieldPixMaskCacheItem::generate(const ShieldPixMaskCacheItem &i)
{
  ShieldStyle *sh = Shieldsetlist::instance()->getShield(i.shieldset,
                                                            i.type, i.color);
  auto colors = Shieldsetlist::instance ()->getColors (i.shieldset, i.color);
  PixMask *p = sh->getMaskedImage ()->applyMask (colors);
  if (i.map)
    return p;
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
    0;
}

PixMask *ProdShieldPixMaskCacheItem::generate(const ProdShieldPixMaskCacheItem &i)
{
  switch (i.type)
    {
    case 0: //home city
      if (i.prod) //production
        return ImageCache::instance()->getProdShieldImage(1)->copy();
      else //no production
        return ImageCache::instance()->getProdShieldImage(0)->copy();
      break;
    case 1: //away city
      if (i.prod) //production
        return ImageCache::instance()->getProdShieldImage(3)->copy();
      else //no production
        return ImageCache::instance()->getProdShieldImage(2)->copy();
      break;
    case 2: //destination city
      if (i.prod) //production
        return ImageCache::instance()->getProdShieldImage(5)->copy();
      else //no production
        return ImageCache::instance()->getProdShieldImage(4)->copy();
      break;
    case 3: //source city
      return ImageCache::instance()->getProdShieldImage(6)->copy();
      break;
    case 4: //invalid
      return ImageCache::instance()->getProdShieldImage(7)->copy();
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
    Gdk::Pixbuf::create(Gdk::Colorspace::RGB, true, 8, width, height);
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
      x += part->get_width ();
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
    Gdk::Pixbuf::create(Gdk::Colorspace::RGB, true, 8, width, height);
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
      x += part->get_width ();
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
    Gdk::Pixbuf::create(Gdk::Colorspace::RGB, true, 8, width, height);
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
      x += part->get_width ();
    }

  for (auto part : parts)
    delete part;

  return p;
}

PixMask *MoveBonusPixMaskCacheItem::getMoveBonusPic(Tileset *t, guint32 bonus)
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
        Gdk::Pixbuf::create(Gdk::Colorspace::RGB, true, 8, 32, 20);
      empty_pic->fill(0x00000000);
      return PixMask::create (empty_pic);
    }

  return p;
}

PixMask *MoveBonusPixMaskCacheItem::generate(const MoveBonusPixMaskCacheItem &i)
{
  Tileset *t = Tilesetlist::instance()->get(i.tileset);
  return MoveBonusPixMaskCacheItem::getMoveBonusPic
    (t, i.bonus);
}

int MoveBonusPixMaskCacheItem::comp(const MoveBonusPixMaskCacheItem &item) const
{
  return
    (bonus < item.bonus) ? -1 :
    (bonus > item.bonus) ?  1 :
    (tileset < item.tileset) ? -1 :
    (tileset > item.tileset) ?  1 :
    0;
}

PixMask *ShipPixMaskCacheItem::generate(const ShipPixMaskCacheItem &i)
{
  // copy the pixmap including player colors
  if (i.shield_id != Shield::NEUTRAL)
    {
      TarFileMaskedImage * mim =
        Armysetlist::instance()->getShipPic(i.armyset);
      return mim->applyMask (i.shield_id);
    }
  else //we can put a neutral ship in the water in the editor
    {
      TarFileMaskedImage * mim =
        Armysetlist::instance()->getShipPic(i.armyset);

      return mim->getImage ()->copy ();
    }
}

int ShipPixMaskCacheItem::comp(const ShipPixMaskCacheItem &item) const
{
  return
    (shieldset < item.shieldset) ? -1 :
    (shieldset > item.shieldset) ?  1 :
    (shield_id < item.shield_id) ? -1 :
    (shield_id > item.shield_id) ?  1 :
    (armyset < item.armyset) ? -1 :
    (armyset > item.armyset) ?  1 :
    0;
}

PixMask *PlantedStandardPixMaskCacheItem::generate(const PlantedStandardPixMaskCacheItem &i)
{
  if (i.shield_id != Shield::NEUTRAL)
    {
      TarFileMaskedImage *mim =
        Armysetlist::instance()->getStandardPic (i.armyset);

      return mim->applyMask (i.shield_id);
    }
  else //we currently can't plant a neutral standard but just in case
    {
      TarFileMaskedImage * mim =
        Armysetlist::instance()->getStandardPic(i.armyset);

      return mim->getImage ()->copy ();
    }
}

int PlantedStandardPixMaskCacheItem::comp(const PlantedStandardPixMaskCacheItem &item) const
{
  return
    (shieldset < item.shieldset) ? -1 :
    (shieldset > item.shieldset) ?  1 :
    (shield_id < item.shield_id) ? -1 :
    (shield_id > item.shield_id) ?  1 :
    (armyset < item.armyset) ? -1 :
    (armyset > item.armyset) ?  1 :
    0;
}

PixMask *PortPixMaskCacheItem::generate(const PortPixMaskCacheItem &i)
{
  return
    Citysetlist::instance()->get(i.cityset)->getPort()->getImage()->copy();
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
  return Citysetlist::instance()->get(i.cityset)->getSignpost ()->getImage()->copy();
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
  return Armysetlist::instance()->getBag(i.armyset)->getImage ()->copy();
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
  return Tilesetlist::instance()->get(i.tileset)->getExplosion()->getImage()->copy();
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
    ImageCache::instance()->getHeroNewLevelMaskedImage (female);
  PixMask *p = mim->applyMask (i.shield_id);

  return p;
}

int NewLevelPixMaskCacheItem::comp(const NewLevelPixMaskCacheItem &item) const
{
  return
    (shieldset < item.shieldset) ? -1 :
    (shieldset > item.shieldset) ?  1 :
    (shield_id < item.shield_id) ? -1 :
    (shield_id > item.shield_id) ?  1 :
    (gender < item.gender) ? -1 :
    (gender > item.gender) ?  1 :
    0;
}

PixMask *DefaultTileStylePixMaskCacheItem::generate(const DefaultTileStylePixMaskCacheItem &i)
{
  PixMask *t =
    ImageCache::instance()->getDefaultTileStyleImage(i.tilestyle_type);

  return t->scale (i.tilesize, i.tilesize);
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

PixMask *TartanPixMaskCacheItem::generate(const TartanPixMaskCacheItem &i)
{
  auto s = Shieldsetlist::instance ()->get (i.shieldset);

  return Shield::get_progress_bar_completed (s, Shield::Color (i.shield),
                                             i.width);
}

int TartanPixMaskCacheItem::comp(const TartanPixMaskCacheItem &item) const
{
  return
    (shield < item.shield) ? -1 :
    (shield > item.shield) ?  1 :
    (width < item.width) ? -1 :
    (width > item.width) ?  1 :
    (shieldset < item.shieldset) ? -1 :
    (shieldset > item.shieldset) ?  1 :
    0;
}

PixMask *EmptyTartanPixMaskCacheItem::generate(const EmptyTartanPixMaskCacheItem &i)
{
  auto s = Shieldsetlist::instance ()->get (i.shieldset);
  return Shield::get_progress_bar_uncompleted (s, Shield::Color (i.shield),
                                               i.width);
}

int EmptyTartanPixMaskCacheItem::comp(const EmptyTartanPixMaskCacheItem &item) const
{
  return
    (shield < item.shield) ? -1 :
    (shield > item.shield) ?  1 :
    (width < item.width) ? -1 :
    (width > item.width) ?  1 :
    (shieldset < item.shieldset) ? -1 :
    (shieldset > item.shieldset) ?  1 :
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
  return p;
}

int StatusPixMaskCacheItem::comp(const StatusPixMaskCacheItem &item) const
{
  return
    (type < item.type) ? -1 :
    (type > item.type) ?  1 :
    0;
}

PixMask *GameButtonPixMaskCacheItem::generate(const GameButtonPixMaskCacheItem &i)
{
  PixMask *p =
    ImageCache::instance ()->getGameButtonImage (i.type)->copy ();

  return p;
}

int GameButtonPixMaskCacheItem::comp(const GameButtonPixMaskCacheItem &item) const
{
  return
    (type < item.type) ? -1 :
    (type > item.type) ?  1 :
    0;
}

PixMask *DialogPixMaskCacheItem::generate(const DialogPixMaskCacheItem &i)
{
  PixMask *p = NULL;
  ImageCache *ic = ImageCache::instance ();
  switch (i.type)
    {
    case ImageCache::DIALOG_NEXT_TURN:
      p = ic->getNextTurnImage ()->getImage ()->copy ();
      break;
    case ImageCache::DIALOG_NEW_HERO_MALE:
      p = ic->getHeroOfferedImage(Hero::MALE)->getImage ()->copy ();
      break;
    case ImageCache::DIALOG_NEW_HERO_FEMALE:
      p = ic->getHeroOfferedImage(Hero::FEMALE)->getImage ()->copy ();
      break;
    case ImageCache::DIALOG_CONQUERED_CITY:
      p = ic->getCityDefeatedImage ()->getImage ()->copy ();
      break;
    case ImageCache::DIALOG_WINNING:
      p = ic->getWinningImage()->getImage ()->copy ();
      break;
    case ImageCache::DIALOG_RUIN_SUCCESS:
      p = ic->getRuinSuccessImage()->getImage ()->copy ();
      break;
    case ImageCache::DIALOG_RUIN_DEFEAT:
      p = ic->getRuinDefeatImage()->getImage ()->copy ();
      break;
    case ImageCache::DIALOG_PARLEY_OFFERED:
      p = ic->getParleyOfferedImage()->getImage ()->copy ();
      break;
    case ImageCache::DIALOG_PARLEY_REFUSED:
      p = ic->getParleyRefusedImage()->getImage ()->copy ();
      break;
    case ImageCache::DIALOG_COMMENTATOR:
      p = ic->getCommentatorImage()->getImage ()->copy ();
      break;
    }
  return p;
}

int DialogPixMaskCacheItem::comp(const DialogPixMaskCacheItem &item) const
{
  return
    (type < item.type) ? -1 :
    (type > item.type) ?  1 :
    0;
}

PixMask *MedalPixMaskCacheItem::generate(const MedalPixMaskCacheItem &i)
{
  PixMask *p =
    ImageCache::instance ()->getMedalImage (i.large, i.type)->copy ();

  if (i.large == false)
    return p;

  return p;
}

int MedalPixMaskCacheItem::comp(const MedalPixMaskCacheItem &item) const
{
  return
    (large < item.large) ? -1 :
    (large > item.large) ?  1 :
    (type < item.type) ? -1 :
    (type > item.type) ?  1 :
    0;
}

PixMask *BoxPixMaskCacheItem::generate(const BoxPixMaskCacheItem &i)
{
  auto p = PixMask::create (i.size);
  auto cr = Cairo::Context::create (p->get_pixmap ());
  if (!i.rounded)
    ImageCache::draw_rectangle (cr, i.size, i.size, i.color, i.dashed,
                                i.line_width, i.offset);
  else
    ImageCache::draw_rounded_rectangle (cr, i.size, i.size, i.color,
                                        i.dashed, i.line_width,
                                        i.offset);

  return p;
}

int BoxPixMaskCacheItem::comp(const BoxPixMaskCacheItem &item) const
{
  return
    (size < item.size) ? -1 :
    (size > item.size) ?  1 :
    (color.to_string () < item.color.to_string ()) ? -1 :
    (color.to_string () > item.color.to_string ()) ?  1 :
    (rounded < item.rounded) ? -1 :
    (rounded > item.rounded) ?  1 :
    (dashed < item.dashed) ? -1 :
    (dashed > item.dashed) ?  1 :
    (line_width < item.line_width) ? -1 :
    (line_width > item.line_width) ?  1 :
    (offset < item.offset) ? -1 :
    (offset > item.offset) ?  1 :
    0;
}

void ImageCache::draw_rectangle (Cairo::RefPtr<Cairo::Context> cr,
                                 int width, int height, Gdk::RGBA color,
                                 bool dashed, int line_width, bool offset)
{
  cr->set_line_width (line_width);
  double red = color.get_red ();
  double green = color.get_green ();
  double blue = color.get_blue ();
  cr->set_source_rgb (red, green, blue);

  double x = 0;
  double y = 0;
  double w = width;
  double h = height;
  if (offset)
    {
      x += line_width;
      y += line_width;
      w -= (line_width * 2);
      h -= (line_width * 2);
    }
  if (dashed)
    {
      std::vector<double> dashes;
      dashes.push_back (width / 7);
      dashes.push_back (width / 7);
      cr->set_dash (dashes, 0);
    }

  cr->rectangle (x, y, w, h);
  cr->stroke ();
  cr->unset_dash ();
}

void ImageCache::draw_rounded_rectangle (Cairo::RefPtr<Cairo::Context> cr,
                                         int width, int height, Gdk::RGBA color,
                                         bool dashed, int line_width, bool offset)
{
  double w = width;
  double h = height;
  double r = w / 8;
  const double pi = M_PI;

  double x = 0;
  double y = 0;
  cr->set_line_width (line_width);

  if (offset)
    {
      x += line_width;
      y += line_width;
      w -= (line_width * 2);
      h -= (line_width * 2);
    }
  double red = color.get_red ();
  double green = color.get_green ();
  double blue = color.get_blue ();
  cr->set_source_rgb (red, green, blue);

  if (dashed)
    {
      std::vector<double> dashes;
      dashes.push_back (width / 7);
      dashes.push_back (width / 7);
      cr->set_dash (dashes, 0);
    }

  cr->begin_new_sub_path ();

  // top-left corner
  cr->arc (x + r, y + r, r, pi, 3 * pi / 2);

  // top edge
  cr->line_to (x + w - r, y);

  // top-right corner
  cr->arc (x + w - r, y + r, r, 3 * pi / 2, 0);

  // right edge
  cr->line_to (x + w, y + h - r);

  // bottom-right corner
  cr->arc (x + w - r, y + h - r, r, 0, pi / 2);

  // bottom edge
  cr->line_to (x + r, y + h);

  // bottom-left corner
  cr->arc (x + r, y + h - r, r, pi / 2, pi);

  // left edge
  cr->line_to (x, y + r);

  cr->close_path ();
  cr->stroke ();
  cr->unset_dash ();
}

void ImageCache::get_bevel_colors (bool darkmode, Gdk::RGBA &light, Gdk::RGBA &dark)
{
  switch (darkmode)
    {
    case true:
      light = BEVELED_CIRCLE_LIGHT;
      dark = BEVELED_CIRCLE_DARK;
      break;
    case false:
      light = DARK_MODE_BEVELED_CIRCLE_LIGHT;
      dark = DARK_MODE_BEVELED_CIRCLE_DARK;
      break;
    }
}

Vector<int> ImageCache::get_hotspot (CursorType c)
{
  Vector<int> hotspot = Vector<int>(4, 4);
  switch (c)
    {
    case POINTER:
      hotspot = Vector<int>(0, 0);
      break;

    case MAGNIFYING_GLASS:
      hotspot = Vector<int>(8, 5);
      break;

    case SHIP:
      hotspot = Vector<int>(7, 7);
      break;

    case ROOK:
      hotspot = Vector<int>(7, 7);
      break;

    case HAND:
      hotspot = Vector<int>(7, 7);
      break;

    case TARGET:
      hotspot = Vector<int>(7, 7);
      break;

    case FEET:
      hotspot = Vector<int>(6, 6);
      break;

    case RUIN:
      hotspot = Vector<int>(6, 6);
      break;

    case SWORD:
      hotspot = Vector<int>(7, 7);
      break;

    case QUESTION:
      hotspot = Vector<int>(7, 7);
      break;

    case HEART:
      hotspot = Vector<int>(7, 7);
      break;

    case GOTO_ARROW:
      hotspot = Vector<int>(0, 0);
      break;

    case CLOSED_HAND:
      hotspot = Vector<int>(7, 7);
      break;

    case HAND_POINTER:
      hotspot = Vector<int>(4, 1);
      break;
    }
  return hotspot;
}
