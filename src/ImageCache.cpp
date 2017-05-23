// Copyright (C) 2003, 2004, 2005, 2006, 2007 Ulf Lorenz
// Copyright (C) 2004, 2005, 2006 Andrea Paternesi
// Copyright (C) 2006-2011, 2014-2016 Ben Asselstine
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

ImageCache::ImageCache()
 : d_cachesize(0),
    selectorcache((sigc::ptr_fun(&SelectorPixMaskCacheItem::generate))),
    armycache((sigc::ptr_fun(&ArmyPixMaskCacheItem::generate))),
    flagcache((sigc::ptr_fun(&FlagPixMaskCacheItem::generate))),
    circledarmycache((sigc::ptr_fun(&CircledArmyPixMaskCacheItem::generate))),
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
    emptytartancache((sigc::ptr_fun(&EmptyTartanPixMaskCacheItem::generate)))
{
    loadDiplomacyImages();
    loadCursorImages();
    loadProdShieldImages();
    loadMoveBonusImages();
    loadMedalImages(ScenarioMedia::getDefaultSmallMedalsImageFilename(),
                    ScenarioMedia::getDefaultBigMedalsImageFilename());
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
    d_nextturn = NULL;
    d_citydefeated = NULL;
    d_winning = NULL;
    d_malehero = NULL;
    d_femalehero = NULL;
    d_ruinsuccess = NULL;
    d_ruindefeat = NULL;
    d_parleyoffered = NULL;
    d_parleyrefused = NULL;
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
  int smallxsize = SMALL_PRODUCTION_SHIELD_WIDTH;
  int smallysize = SMALL_PRODUCTION_SHIELD_HEIGHT;
  int mediumxsize = MEDIUM_PRODUCTION_SHIELD_WIDTH;
  int mediumysize = MEDIUM_PRODUCTION_SHIELD_HEIGHT;
  std::vector<PixMask*> prodshield;
  prodshield = disassemble_row
    (File::getVariousFile("prodshieldset.png"), PRODUCTION_SHIELD_TYPES,
     broken);
  if (broken)
    return false;
  for (unsigned int i = 0; i < PRODUCTION_SHIELD_TYPES; i++)
    {
      if (prodshield[i]->get_width() != smallxsize)
	PixMask::scale(prodshield[i], smallxsize, smallysize);
      d_prodshield[0][i] = prodshield[i];
    }
  prodshield.clear();
  prodshield = disassemble_row 
    (File::getVariousFile("prodshieldset-medium.png"), 
     PRODUCTION_SHIELD_TYPES, broken);
  if (broken)
    return false;
  for (unsigned int i = 0; i < PRODUCTION_SHIELD_TYPES; i++)
    {
      if (prodshield[i]->get_width() != mediumxsize)
	PixMask::scale(prodshield[i], mediumxsize, mediumysize);
      d_prodshield[1][i] = prodshield[i];
    }
  return true;
}

bool ImageCache::loadMoveBonusImages()
{
  bool broken = false;
  //load the movement bonus icons
  int xsize = MOVE_BONUS_WIDTH;
  int ysize = MOVE_BONUS_HEIGHT;
  std::vector<PixMask*> movebonus;
  movebonus = disassemble_row(File::getVariousFile("movebonus.png"),
                              MOVE_BONUS_TYPES, broken);
  if (broken)
    return false;
  for (unsigned int i = 0; i < MOVE_BONUS_TYPES; i++)
    {
      if (movebonus[i]->get_width() != xsize)
	PixMask::scale(movebonus[i], xsize, ysize);
      d_movebonus[i] = movebonus[i];
    }
  return true;
}

bool ImageCache::loadNewLevelImages()
{
  bool broken = false;
  std::vector<PixMask*> half;
  half = disassemble_row
    (ScenarioMedia::getDefaultHeroNewLevelMaleImageFilename(), 2, broken);
  if (broken)
    return false;
  d_newlevel_male = half[0];
  d_newlevelmask_male = half[1];
  half = disassemble_row
    (ScenarioMedia::getDefaultHeroNewLevelFemaleImageFilename(), 2, broken);
  if (broken)
    return false;
  d_newlevel_female = half[0];
  d_newlevelmask_female = half[1];
  return true;
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

bool ImageCache::loadMedalImages(Glib::ustring sm, Glib::ustring lg)
{
  bool broken = false;
  //load the medal icons
  int ts = 40;
  std::vector<PixMask*> medal;
  medal = disassemble_row(sm, MEDAL_TYPES, broken);
  if (broken)
    return false;
  for (unsigned int i = 0; i < MEDAL_TYPES; i++)
    {
      if (medal[i]->get_width() != ts)
        PixMask::scale(medal[i], ts, ts);
      d_medal[0][i] = medal[i];
    }
  medal = disassemble_row(lg, MEDAL_TYPES, broken);
  if (broken)
    return false;
  for (unsigned int i = 0; i < MEDAL_TYPES; i++)
    d_medal[1][i] = medal[i];
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
    d_gamebuttons[0][i] = images[i]; 

  images.clear();
  images = disassemble_row(File::getVariousFile("buttons-medium.png"), 
                           NUM_GAME_BUTTON_IMAGES, broken);
  if (broken)
    return false;
  Gtk::IconSize::lookup(Gtk::IconSize(Gtk::ICON_SIZE_LARGE_TOOLBAR), w, h);
  for (unsigned int i = 0; i < NUM_GAME_BUTTON_IMAGES; i++)
    PixMask::scale(images[i], w, h); 
  for (unsigned int i = 0; i < NUM_GAME_BUTTON_IMAGES; i++)
    d_gamebuttons[1][i] = images[i];

  images.clear();
  images = disassemble_row(File::getVariousFile("buttons-medium.png"), 
                           NUM_GAME_BUTTON_IMAGES, broken);
  Gtk::IconSize::lookup(Gtk::IconSize(Gtk::ICON_SIZE_DIALOG), w, h);
  for (unsigned int i = 0; i < NUM_GAME_BUTTON_IMAGES; i++)
    PixMask::scale(images[i], w, h); 
  for (unsigned int i = 0; i < NUM_GAME_BUTTON_IMAGES; i++)
    d_gamebuttons[2][i] = images[i];
  return true;
}

PixMask* ImageCache::loadMiscImage(Glib::ustring pngfile)
{
  bool broken = false;
  return PixMask::create(File::getVariousFile(pngfile), broken);
}

ImageCache::~ImageCache()
{

  for (unsigned int i = 0; i < DIPLOMACY_TYPES;i++)
    {
      delete d_diplomacy[0][i];
      delete d_diplomacy[1][i];
    }

  for (unsigned int i = 0; i < CURSOR_TYPES;i++)
    delete d_cursor[i];

  for (unsigned int i = 0; i < PRODUCTION_SHIELD_TYPES; i++)
    {
      delete d_prodshield[0][i];
      delete d_prodshield[1][i];
    }

  for (unsigned int i = 0; i < MOVE_BONUS_TYPES; i++)
    delete d_movebonus[i];

  delete d_newlevel_male;
  delete d_newlevelmask_male;
  delete d_newlevel_female;
  delete d_newlevelmask_female;

  for (unsigned int i = 0; i < DEFAULT_TILESTYLE_TYPES; i++)
    delete d_default_tilestyles[i];

  for (unsigned int i = 0; i < MEDAL_TYPES; i++)
    {
      delete d_medal[0][i];
      delete d_medal[1][i];
    }

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
    {
      delete d_gamebuttons[0][i];
      delete d_gamebuttons[1][i];
      delete d_gamebuttons[2][i];
    }

  if (d_nextturn)
    delete d_nextturn;
  if (d_citydefeated)
    delete d_citydefeated;
  if (d_winning)
    delete d_winning;
  if (d_malehero)
    delete d_malehero;
  if (d_femalehero)
    delete d_femalehero;
  if (d_ruinsuccess)
    delete d_ruinsuccess;
  if (d_ruindefeat)
    delete d_ruindefeat;
  if (d_parleyoffered)
    delete d_parleyoffered;
  if (d_parleyrefused)
    delete d_parleyrefused;
  reset();
}

void ImageCache::reset()
{
  selectorcache.reset();
  flagcache.reset();
  armycache.reset();
  circledarmycache.reset();
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

  if (movebonuscache.size() >= MOVE_BONUS_TYPES)
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
		    a->getOwner(), NULL, greyed);
}

PixMask* ImageCache::getArmyPic(guint32 armyset, guint32 army_id, 
                                      const Player* p, const bool *medals,
                                      bool greyed)
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

PixMask* ImageCache::getCircledArmyPic(Army *a, bool greyed, guint32 circle_colour_id, bool show_army)
{
  return getCircledArmyPic(a->getOwner()->getArmyset(), a->getTypeId(), 
		    a->getOwner(), NULL, greyed, circle_colour_id, show_army);
}

PixMask* ImageCache::getCircledArmyPic(guint32 armyset, guint32 army_id, 
                                             const Player* p, 
                                             const bool *medals, bool greyed, 
                                             guint32 circle_colour_id,
                                             bool show_army)
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
  i.circle_colour_id = circle_colour_id;
  i.show_army = show_army;
  PixMask *s = circledarmycache.get(i, added);
  d_cachesize += added;
  if (added)
    checkPictures();
  return s;
}

PixMask* ImageCache::getTilePic(int tile_style_id, int fog_type_id, bool has_bag, bool has_standard, int standard_player_id, int stack_size, int stack_player_id, int army_type_id, bool has_tower, bool has_ship, Maptile::Building building_type, int building_subtype, Vector<int> building_tile, int building_player_id, guint32 tilesize, bool has_grid)
{
  guint32 tileset = GameMap::getInstance()->getTilesetId();
  guint32 cityset = GameMap::getInstance()->getCitysetId();
  guint32 shieldset = GameMap::getInstance()->getShieldsetId();
  return getTilePic(tile_style_id, fog_type_id, has_bag, has_standard, standard_player_id, stack_size, stack_player_id, army_type_id, has_tower, has_ship, building_type, building_subtype, building_tile, building_player_id, tilesize, has_grid, tileset, cityset, shieldset);
}

PixMask* ImageCache::getTilePic(int tile_style_id, int fog_type_id, bool has_bag, bool has_standard, int standard_player_id, int stack_size, int stack_player_id, int army_type_id, bool has_tower, bool has_ship, Maptile::Building building_type, int building_subtype, Vector<int> building_tile, int building_player_id, guint32 tilesize, bool has_grid, guint32 tileset, guint32 cityset, guint32 shieldset)
{
  guint added = 0;
  TilePixMaskCacheItem i;
  i.tile_style_id = tile_style_id;
  i.fog_type_id = fog_type_id;
  i.has_bag = has_bag;
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

PixMask* ImageCache::getShieldPic(guint32 type, Player *p)
{
  guint32 shieldset = GameMap::getInstance()->getShieldsetId();
  return getShieldPic(shieldset, type, p->getId());
}

PixMask* ImageCache::getShieldPic(guint32 shieldset, guint32 type, 
                                        guint32 colour)
{
  guint added = 0;
  ShieldPixMaskCacheItem i;
  i.type = type;
  i.shieldset = shieldset;
  i.colour = colour;
  PixMask *s = shieldcache.get(i, added);
  d_cachesize += added;
  if (added)
    checkPictures();
  return s;
}

PixMask* ImageCache::getProdShieldPic(int size, guint32 type, bool prod)
{
  guint added = 0;
  ProdShieldPixMaskCacheItem i;
  i.type = type;
  i.size = size;
  i.prod = prod;
  PixMask *s = prodshieldcache.get(i, added);
  d_cachesize += added;
  if (added)
    checkPictures();
  return s;
}

PixMask* ImageCache::getMoveBonusPic(guint32 bonus, bool has_ship)
{
  guint added = 0;
  MoveBonusPixMaskCacheItem i;
  if (bonus == Tile::isFlying()) // show fly icon
    i.type = 4; 
  else if (bonus & Tile::FOREST && bonus & Tile::HILLS) // show trees and hills
    i.type = 3;
  else if (bonus & Tile::HILLS) // show foothills
    i.type = 2;
  else if (bonus & Tile::FOREST) // show trees
    i.type = 1;
  else // show blank
    i.type = 0;
  if (has_ship && bonus != Tile::isFlying()) // (what a) show boat
    i.type = 5;
  PixMask *s = movebonuscache.get(i, added);
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

PixMask* ImageCache::getNewLevelPic(const Player* p, guint32 gender)
{
  guint added = 0;
  NewLevelPixMaskCacheItem i;
  i.player_id = p->getId();
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

PixMask* ImageCache::getTartanPic(const Player *p, guint32 width, Shieldset *shieldset)
{
  guint added = 0;
  TartanPixMaskCacheItem i;
  i.player_id = p->getId();
  i.width = width;
  i.shieldset = shieldset->getId();
  PixMask *s = tartancache.get(i, added);
  d_cachesize += added;
  if (added)
    checkPictures();
  return s;
}

PixMask* ImageCache::getEmptyTartanPic(const Player *p, guint32 width, Shieldset *shieldset)
{
  guint added = 0;
  EmptyTartanPixMaskCacheItem i;
  i.player_id = p->getId();
  i.width = width;
  i.shieldset = shieldset->getId();
  PixMask *s = emptytartancache.get(i, added);
  d_cachesize += added;
  if (added)
    checkPictures();
  return s;
}

PixMask* ImageCache::getDiplomacyImage(int type, Player::DiplomaticState state)
{
  return d_diplomacy[type][state];
}

PixMask* ImageCache::getMoveBonusImage(guint32 type)
{
  return d_movebonus[type];
}

PixMask* ImageCache::getDefaultTileStyleImage(guint32 type)
{
  return d_default_tilestyles[type];
}

PixMask* ImageCache::getCursorImage(int type)
{
  return d_cursor[type];
}

PixMask *ImageCache::getProdShieldImage(int size, guint32 type)
{
  return d_prodshield[size][type];
}

PixMask* ImageCache::getMedalImage(bool large, int type)
{
  if (large)
    {
      if (ScenarioMedia::getInstance()->getBigMedalsImageName() != "")
        return ScenarioMedia::getInstance()->getBigMedalImage(type);
      else
        return d_medal[1][type];
    }
  else
    {
      if (ScenarioMedia::getInstance()->getSmallMedalsImageName() != "")
        return ScenarioMedia::getInstance()->getSmallMedalImage(type);
      else
        return d_medal[0][type];
    }
}

PixMask *ImageCache::getNewLevelImage(bool female, bool mask)
{
  if (female && mask)
    {
      if (ScenarioMedia::getInstance()->getHeroNewLevelFemaleMask())
        return ScenarioMedia::getInstance()->getHeroNewLevelFemaleMask();
      else
        return d_newlevelmask_female;
    }
  else if (female && !mask)
    {
      if (ScenarioMedia::getInstance()->getHeroNewLevelFemaleImage())
        return ScenarioMedia::getInstance()->getHeroNewLevelFemaleImage();
      else
        return d_newlevel_female;
    }

  if (!female && mask)
    {
      if (ScenarioMedia::getInstance()->getHeroNewLevelMaleMask())
        return ScenarioMedia::getInstance()->getHeroNewLevelMaleMask();
      else
        return d_newlevelmask_male;
    }
  else if (!female && !mask)
    {
      if (ScenarioMedia::getInstance()->getHeroNewLevelMaleImage())
        return ScenarioMedia::getInstance()->getHeroNewLevelMaleImage();
      else
        return d_newlevel_male;
    }
  return NULL;
}

PixMask* ImageCache::getGameButtonImage(guint32 type, int size)
{
  return d_gamebuttons[size][type];
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

PixMask* ImageCache::applyMask(PixMask* image, PixMask* mask, const Player* p)
{
  return applyMask(image, mask, p->getColor());
}

PixMask* ImageCache::applyMask(PixMask* image, PixMask* mask, Gdk::RGBA colour)
{
  int width = image->get_width();
  int height = image->get_height();
  PixMask* result = PixMask::create(image->get_pixmap(), mask->get_pixmap());
  if (!result)
    return NULL;
  if (mask->get_width() != width || (mask->get_height()) != height)
    {
      std::cerr <<"Warning: mask and original image do not match\n";
      return NULL;
    }
  Glib::RefPtr<Gdk::Pixbuf> maskbuf = mask->to_pixbuf();

  guint8 *data = maskbuf->get_pixels();
  guint8 *copy = (guint8*)  malloc (height * width * 4 * sizeof(guint8));
  memcpy(copy, data, height * width * 4 * sizeof(guint8));
  for (int i = 0; i < width; i++)
    for (int j = 0; j < height; j++)
      {
	const int base = (j * 4) + (i * height * 4);

	if (copy[base+3] != 0)
	  {
	    copy[base+0] = colour.get_red() *copy[base+0];
	    copy[base+1] = colour.get_green() * copy[base+1];
	    copy[base+2] = colour.get_blue() * copy[base+2];
	  }
      }
  Glib::RefPtr<Gdk::Pixbuf> colouredmask = 
    Gdk::Pixbuf::create_from_data(copy, Gdk::COLORSPACE_RGB, true, 8, 
				  width, height, width * 4);
  result->draw_pixbuf(colouredmask, 0, 0, 0, 0, width, height);
  free(copy);

  return result;
}

PixMask* ImageCache::greyOut(PixMask* image)
{
  bool broken = false;
  int width = image->get_width();
  int height = image->get_height();
  PixMask* result = PixMask::create(image->to_pixbuf());
  if (broken)
    return NULL;
  
  guint8 *data = result->to_pixbuf()->get_pixels();
  guint8 *copy = (guint8*)  malloc (height * width * 4 * sizeof(guint8));
  memcpy(copy, data, height * width * 4 * sizeof(guint8));
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
  Glib::RefPtr<Gdk::Pixbuf> greyed_out =  
    Gdk::Pixbuf::create_from_data(copy, Gdk::COLORSPACE_RGB, true, 8, 
				  width, height, width * 4);

  result->draw_pixbuf(greyed_out, 0, 0, 0, 0, width, height);
  free(copy);

  return result;
}
void ImageCache::draw_circle(Cairo::RefPtr<Cairo::Context> cr, double width_percent, int width, int height, Gdk::RGBA colour, bool coloured, bool mask)
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

  if (coloured)
    {
      cr->set_line_width(line_width);
      double red = colour.get_red();
      double green = colour.get_green();
      double blue = colour.get_blue();
      cr->set_source_rgb(red, green, blue);
      cr->arc((double)width/2.0, (double)height/2.0, radius + (line_width / 2.0), 0, 2 *M_PI);
      cr->stroke();
    }
}

PixMask* ImageCache::circled(PixMask* image, Gdk::RGBA colour, bool coloured, double width_percent)
{
  PixMask *copy = image->copy();
  int width = image->get_width();
  int height = image->get_height();
  //here we draw a coloured circle on top of the army's image
  Cairo::RefPtr<Cairo::Context> cr = Cairo::Context::create(copy->get_pixmap());

  draw_circle(cr, width_percent, width, height, colour, coloured, false);

  //here we draw a white circle on a copy of the image's mask.
  Cairo::RefPtr<Cairo::Surface> mask = copy->get_mask();

  cr = Cairo::Context::create(mask);
  draw_circle(cr, width_percent, width, height, Gdk::RGBA("white"), coloured, true);
  PixMask *result = PixMask::create(copy->get_pixmap(), mask);
  //draw the army on top again, to make it look like the circle is behind.
  result->draw_pixbuf(image->to_pixbuf(), 0, 0, 0, 0, width, height);
  delete copy;
  return result;
}

PixMask* ImageCache::getNextTurnPic ()
{
  PixMask *i = ScenarioMedia::getInstance()->getNextTurnImage();
  if (i)
    return i;
  if (!d_nextturn)
    {
      bool broken = false;
      i = PixMask::create(ScenarioMedia::getDefaultNextTurnImageFilename(),
                          broken);
      if (!broken)
        d_nextturn = i;
    }
  return d_nextturn;
}

PixMask* ImageCache::getCityDefeatedPic ()
{
  PixMask *i = ScenarioMedia::getInstance()->getCityDefeatedImage();
  if (i)
    return i;
  if (!d_citydefeated)
    {
      bool broken = false;
      i = PixMask::create(ScenarioMedia::getDefaultCityDefeatedImageFilename(),
                          broken);
      if (!broken)
        d_citydefeated = i;
    }
  return d_citydefeated;
}

PixMask * ImageCache::getWinningPic ()
{
  PixMask *i = ScenarioMedia::getInstance()->getWinningImage();
  if (i)
    return i;
  if (!d_winning)
    {
      bool broken = false;
      i = PixMask::create(ScenarioMedia::getDefaultWinningImageFilename(),
                          broken);
      if (!broken)
        d_winning = i;
    }
  return d_winning;
}

PixMask * ImageCache::getHeroPic (Hero::Gender gender)
{
  switch (gender)
    {
    case Hero::NONE:
    case Hero::MALE:
        {
          PixMask *i = ScenarioMedia::getInstance()->getMaleHeroImage();
          if (i)
            return i;
          if (!d_malehero)
            {
              bool broken = false;
              i = PixMask::create
                (ScenarioMedia::getDefaultMaleHeroImageFilename(), broken);
              if (!broken)
                d_malehero = i;
            }
          return d_malehero;
        }
      break;
    case Hero::FEMALE:
        {
          PixMask *i = ScenarioMedia::getInstance()->getFemaleHeroImage();
          if (i)
            return i;
          if (!d_femalehero)
            {
              bool broken = false;
              i = PixMask::create
                (ScenarioMedia::getDefaultFemaleHeroImageFilename(), broken);
              if (!broken)
                d_femalehero = i;
            }
          return d_femalehero;
        }
      break;
    }
  return NULL;
}

PixMask *ImageCache::getRuinSuccessPic()
{
  PixMask *i = ScenarioMedia::getInstance()->getRuinSuccessImage();
  if (i)
    return i;
  if (!d_ruinsuccess)
    {
      bool broken = false;
      i = PixMask::create(ScenarioMedia::getDefaultRuinSuccessImageFilename(),
                          broken);
      if (!broken)
        d_ruinsuccess = i;
    }
  return d_ruinsuccess;
}

PixMask *ImageCache::getRuinDefeatPic()
{
  PixMask *i = ScenarioMedia::getInstance()->getRuinDefeatImage();
  if (i)
    return i;
  if (!d_ruindefeat)
    {
      bool broken = false;
      i = PixMask::create(ScenarioMedia::getDefaultRuinDefeatImageFilename(),
                          broken);
      if (!broken)
        d_ruindefeat = i;
    }
  return d_ruindefeat;
}

PixMask* ImageCache::getParleyOfferedPic ()
{
  PixMask *i = ScenarioMedia::getInstance()->getParleyOfferedImage();
  if (i)
    return i;
  if (!d_parleyoffered)
    {
      bool broken = false;
      i = PixMask::create(ScenarioMedia::getDefaultParleyOfferedImageFilename(),
                          broken);
      if (!broken)
        d_parleyoffered = i;
    }
  return d_parleyoffered;
}

PixMask* ImageCache::getParleyRefusedPic ()
{
  PixMask *i = ScenarioMedia::getInstance()->getParleyRefusedImage();
  if (i)
    return i;
  if (!d_parleyrefused)
    {
      bool broken = false;
      i = PixMask::create(ScenarioMedia::getDefaultParleyRefusedImageFilename(),
                          broken);
      if (!broken)
        d_parleyrefused = i;
    }
  return d_parleyrefused;
}

PixMask *ImageCache::add_border (PixMask *p, Vector<int> dim, double wid)
{
  Glib::RefPtr<Gdk::Pixbuf> pixbuf
    = Gdk::Pixbuf::create(Gdk::COLORSPACE_RGB, true, 8, dim.x, dim.y);
  pixbuf->fill(0x00000000);
  PixMask *box = PixMask::create(pixbuf);
  int width = p->get_width();
  int height = p->get_height();
  Vector<int> siz = Vector<int>(width, height);
  //here we draw a square border on the outside of the image
  p->blit (box->get_pixmap(), (dim-siz)/2);
  Cairo::RefPtr<Cairo::Context> cr = Cairo::Context::create(box->get_pixmap());

  cr->set_line_width(wid);
  //let's make it a yellow box
  cr->set_source_rgb(1.0, 1.0, 0.0);
  cr->rectangle(0, 0, dim.x, dim.y);
  cr->stroke();
  return box;
}

PixMask *ImageCache::add_border (Glib::ustring file, Vector<int> dim, double w)
{
  bool broken = false;
  PixMask *copy = PixMask::create(file, broken);
  if (broken)
    return NULL;
  PixMask *box = add_border (copy, dim, w);
  delete copy;
  return box;
}

PixMask *SelectorPixMaskCacheItem::generate(SelectorPixMaskCacheItem i)
{
  Tileset *ts = Tilesetlist::getInstance()->get(i.tileset);
  Player *p = Playerlist::getInstance()->getPlayer(i.player_id);
  if (i.type == 0)
    return ImageCache::applyMask(ts->getSelectorImage(i.frame), 
                                 ts->getSelectorMask(i.frame), p);
  else
    return ImageCache::applyMask(ts->getSmallSelectorImage(i.frame), 
                                 ts->getSmallSelectorMask(i.frame), p);
}
    
int SelectorPixMaskCacheItem::comp(const SelectorPixMaskCacheItem item) const
{
  return
    (tileset < item.tileset) ? -1 :
    (tileset> item.tileset)?  1 :
    (type < item.type) ? -1 :
    (type> item.type) ?  1 :
    (frame < item.frame) ? -1 :
    (frame > item.frame) ?  1 :
    (player_id < item.player_id) ? -1 :
    (player_id > item.player_id) ?  1 :
    0;
}
    
bool SelectorPixMaskCacheItem::loadSelectorImages(Glib::ustring filename, guint32 size, std::vector<PixMask* > &images, std::vector<PixMask* > &masks)
{
  bool broken = false;
  int num_frames;
  guint32 width, height;
  get_image_width_and_height(filename, width, height, broken);
  if (broken)
    return false;
  num_frames = width / size;
  images = disassemble_row(filename, num_frames, true, broken);
  if (broken)
    return false;
  for (int i = 0; i < num_frames; i++)
    {
      if (images[i]->get_width() != (int)size)
        PixMask::scale(images[i], size, size);
    }

  masks = disassemble_row(filename, num_frames, false, broken);
  if (broken)
    return false;
  for (int i = 0; i < num_frames; i++)
    {
      if (masks[i]->get_width() != (int)size)
        PixMask::scale(masks[i], size, size);
    }

  return true;
}

PixMask *FlagPixMaskCacheItem::generate(FlagPixMaskCacheItem i)
{
  Tileset *ts = Tilesetlist::getInstance()->get(i.tileset);
  Player *p = Playerlist::getInstance()->getPlayer(i.player_id);
  // size of stack starts at 1, but we need the index, which starts at 0
  return ImageCache::applyMask (ts->getFlagImage(i.size-1), 
                                ts->getFlagMask(i.size-1), p);
}

int FlagPixMaskCacheItem::comp(const FlagPixMaskCacheItem item) const
{
  return
    (tileset < item.tileset) ? -1 :
    (tileset> item.tileset)?  1 :
    (size < item.size) ? -1 :
    (size > item.size) ?  1 :
    (player_id < item.player_id) ? -1 :
    (player_id > item.player_id) ?  1 :
    0;
}

bool FlagPixMaskCacheItem::loadFlagImages(Glib::ustring filename, guint32 size, std::vector<PixMask* > &images, std::vector<PixMask* > &masks)
{
  bool broken = false;
  images = disassemble_row(filename, FLAG_TYPES, true, broken);
  if (broken)
    return false;
  for (unsigned int i = 0; i < FLAG_TYPES; i++)
    {
      if (images[i]->get_width() != (int)size)
        PixMask::scale(images[i], size, size);
    }
  masks = disassemble_row(filename, FLAG_TYPES, false, broken);
  if (broken)
    return false;
  for (unsigned int i = 0; i < FLAG_TYPES; i++)
    {
      if (masks[i]->get_width() !=(int) size)
        PixMask::scale(masks[i], size, size);
    }
  return true;
}
 
PixMask *ArmyPixMaskCacheItem::generate(ArmyPixMaskCacheItem i)
{
  PixMask *s;
  const ArmyProto * basearmy = 
    Armysetlist::getInstance()->getArmy(i.armyset, i.army_id);

  // copy the pixmap including player colors
  Player *p = Playerlist::getInstance()->getPlayer(i.player_id);
  Shield::Colour c = Shield::Colour(i.player_id);
  if (basearmy->getImage(c) == NULL || basearmy->getMask(c) == NULL)
    return NULL;
  PixMask *coloured = ImageCache::applyMask(basearmy->getImage(c), 
                                            basearmy->getMask(c), p);
  if (i.greyed)
    {
      PixMask *greyed_out = ImageCache::greyOut(coloured);
      s = greyed_out;
      delete coloured;
    }
  else
    s = coloured;

  for(int j = 0; j < 3; j++)
    { 
      if (i.medals[j])
        ImageCache::getInstance()->getMedalImage(false, j)->blit(s->get_pixmap());
    }
  return s;
}

int ArmyPixMaskCacheItem::comp(const ArmyPixMaskCacheItem item) const
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
    (greyed < item.greyed) ? -1 :
    (greyed > item.greyed) ?  1 :
    0;
}

PixMask *CircledArmyPixMaskCacheItem::generate(CircledArmyPixMaskCacheItem i)
{
  PixMask *s;
  if (i.show_army)
    {
      Player *p = Playerlist::getInstance()->getPlayer(i.player_id);
      PixMask *pre_circle = 
        ImageCache::getInstance()->getArmyPic(i.armyset, i.army_id, p, 
                                              i.medals, i.greyed);
      s = ImageCache::circled(pre_circle, p->getColor(), 
                              i.circle_colour_id != Shield::NEUTRAL);
    }
  else
    {
      guint32 size = Armysetlist::getInstance()->get(i.armyset)->getTileSize();
      Glib::RefPtr<Gdk::Pixbuf> pixbuf
        = Gdk::Pixbuf::create(Gdk::COLORSPACE_RGB, true, 8, size, size);
      pixbuf->fill(0x00000000);
      PixMask *empty = PixMask::create(pixbuf);
      s = ImageCache::circled
        (empty, Shield::get_default_color_for_no(i.circle_colour_id), 
         i.circle_colour_id != Shield::NEUTRAL);
      delete empty;
    }
  return s;
}

int CircledArmyPixMaskCacheItem::comp(const CircledArmyPixMaskCacheItem item) const
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
    (circle_colour_id < item.circle_colour_id) ?  -1 :
    (circle_colour_id > item.circle_colour_id) ?  1 :
    (show_army < item.show_army) ?  -1 :
    (show_army > item.show_army) ?  1 :
    0;
}

PixMask *TilePixMaskCacheItem::generate(TilePixMaskCacheItem i)
{
  PixMask *s;
  Tileset *t = Tilesetlist::getInstance()->get(i.tileset);
  guint32 uts = t->getUnscaledTileSize();
  if (i.fog_type_id == FogMap::ALL)
    s = t->getFogImage(i.fog_type_id - 1)->copy();
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

      if (i.has_standard)
        {
          player = Playerlist::getInstance()->getPlayer(i.standard_player_id) ;
          ImageCache::getInstance()->getPlantedStandardPic(player)->blit(pixmap);
        }

      if (i.has_bag)
        {
          PixMask *pic = ImageCache::getInstance()->getBagPic();
          Vector<int>bagsize = Vector<int>(pic->get_width(), pic->get_height());
          pic->blit(pixmap, Vector<int>(uts,uts)-bagsize);
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
                ImageCache::getInstance()->getArmyPic(player->getArmyset(), i.army_type_id, player, NULL)->blit(pixmap);
            }
        }
      if (i.has_grid)
        {
          Cairo::RefPtr<Cairo::Context> context = s->get_gc();
          context->set_source_rgba(GRID_BOX_COLOUR.get_red(), GRID_BOX_COLOUR.get_blue(), GRID_BOX_COLOUR.get_green(), GRID_BOX_COLOUR.get_alpha());
          context->move_to(0, 0);
          context->rel_line_to(uts, 0);
          context->rel_line_to(0, uts);
          context->rel_line_to(-uts, 0);
          context->rel_line_to(0, -uts);
          context->set_line_width(1.0);
          context->stroke();
        }

      if (i.fog_type_id)
        t->getFogImage(i.fog_type_id - 1)->blit(pixmap);
    }
  int ts = t->getTileSize();
  if (s->get_width () != ts)
    s->scale (s, ts, ts);
  return s;
}

int TilePixMaskCacheItem::comp(const TilePixMaskCacheItem item) const
{
  return
    (tile_style_id < item.tile_style_id) ? -1 :
    (tile_style_id > item.tile_style_id) ?  1 :
    (fog_type_id < item.fog_type_id) ? -1 :
    (fog_type_id > item.fog_type_id) ?  1 :
    (has_bag < item.has_bag) ? -1 :
    (has_bag > item.has_bag) ?  1 :
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
    0;
}

PixMask *CityPixMaskCacheItem::generate(CityPixMaskCacheItem i)
{
  Cityset *cs = Citysetlist::getInstance()->get(i.cityset);
  Player *p = Playerlist::getInstance()->getPlayer(i.player_id);
  if (i.type == -1)
    return cs->getRazedCityImage(p->getId())->copy();
  else
    return cs->getCityImage(p->getId())->copy();
}

int CityPixMaskCacheItem::comp(const CityPixMaskCacheItem item) const
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

PixMask *TowerPixMaskCacheItem::generate(TowerPixMaskCacheItem i)
{
  Cityset *cs = Citysetlist::getInstance()->get(i.cityset);
  return cs->getTowerImage(i.player_id)->copy();
}

int TowerPixMaskCacheItem::comp(const TowerPixMaskCacheItem item) const
{
  return
    (cityset < item.cityset) ? -1 :
    (cityset > item.cityset)?  1 :
    (player_id < item.player_id) ? -1 :
    (player_id > item.player_id) ?  1 :
    0;
}

PixMask *TemplePixMaskCacheItem::generate(TemplePixMaskCacheItem i)
{
  Cityset *cs = Citysetlist::getInstance()->get(i.cityset);
  return cs->getTempleImage(i.type)->copy();
}

int TemplePixMaskCacheItem::comp(const TemplePixMaskCacheItem item) const
{
  return
    (cityset < item.cityset) ? -1 :
    (cityset > item.cityset)?  1 :
    (type < item.type) ? -1 :
    (type > item.type) ?  1 :
    0;
}

PixMask *RuinPixMaskCacheItem::generate(RuinPixMaskCacheItem i)
{
  Cityset *cs = Citysetlist::getInstance()->get(i.cityset);
  return cs->getRuinImage(i.type)->copy();
}

int RuinPixMaskCacheItem::comp(const RuinPixMaskCacheItem item) const
{
  return
    (cityset < item.cityset) ? -1 :
    (cityset > item.cityset)?  1 :
    (type < item.type) ? -1 :
    (type > item.type) ?  1 :
    0;
}

PixMask *DiplomacyPixMaskCacheItem::generate(DiplomacyPixMaskCacheItem i)
{
  return ImageCache::getInstance()->getDiplomacyImage(i.type, Player::DiplomaticState(i.state - Player::AT_PEACE))->copy();
}

int DiplomacyPixMaskCacheItem::comp(const DiplomacyPixMaskCacheItem item) const
{
  return
    (type < item.type) ? -1 :
    (type > item.type) ?  1 :
    (state < item.state) ? -1 :
    (state > item.state) ? 1 :
    0;
}

PixMask *RoadPixMaskCacheItem::generate(RoadPixMaskCacheItem i)
{
  Tileset *ts = Tilesetlist::getInstance()->get(i.tileset);
  return ts->getRoadImage(i.type)->copy();
}

int RoadPixMaskCacheItem::comp(const RoadPixMaskCacheItem item) const
{
  return
    (tileset < item.tileset) ? -1 :
    (tileset > item.tileset)?  1 :
    (type < item.type) ? -1 :
    (type > item.type) ?  1 :
    0;
}

PixMask *FogPixMaskCacheItem::generate(FogPixMaskCacheItem i)
{
  Tileset *ts = Tilesetlist::getInstance()->get(i.tileset);
  return ts->getFogImage(i.type - 1)->copy();
}

int FogPixMaskCacheItem::comp(const FogPixMaskCacheItem item) const
{
  return
    (tileset < item.tileset) ? -1 :
    (tileset > item.tileset)?  1 :
    (type < item.type) ? -1 :
    (type > item.type) ?  1 :
    0;
}

PixMask *BridgePixMaskCacheItem::generate(BridgePixMaskCacheItem i)
{
  Tileset *ts = Tilesetlist::getInstance()->get(i.tileset);
  return ts->getBridgeImage(i.type)->copy();
}

int BridgePixMaskCacheItem::comp(const BridgePixMaskCacheItem item) const
{
  return
    (tileset < item.tileset) ? -1 :
    (tileset > item.tileset)?  1 :
    (type < item.type) ? -1 :
    (type > item.type) ?  1 :
    0;
}

PixMask *CursorPixMaskCacheItem::generate(CursorPixMaskCacheItem i)
{
  return ImageCache::getInstance()->getCursorImage(i.type)->copy();
}

int CursorPixMaskCacheItem::comp(const CursorPixMaskCacheItem item) const
{
  return
    (type < item.type) ? -1 :
    (type > item.type) ?  1 :
    0;
}

PixMask *ShieldPixMaskCacheItem::generate(ShieldPixMaskCacheItem i)
{
  ShieldStyle *sh = Shieldsetlist::getInstance()->getShield(i.shieldset, 
                                                            i.type, i.colour);
  Gdk::RGBA colour = 
    Shieldsetlist::getInstance()->getColor(i.shieldset, i.colour);
  return ImageCache::applyMask(sh->getImage(), sh->getMask(), colour);
}

int ShieldPixMaskCacheItem::comp(const ShieldPixMaskCacheItem item) const
{
  return
    (shieldset < item.shieldset) ? -1 :
    (shieldset > item.shieldset)?  1 :
    (type < item.type) ? -1 :
    (type > item.type) ?  1 :
    (colour < item.colour) ? -1 :
    (colour > item.colour) ?  1 :
    0;
}

PixMask *ProdShieldPixMaskCacheItem::generate(ProdShieldPixMaskCacheItem i)
{
  switch (i.type)
    {
    case 0: //home city
      if (i.prod) //production
        return ImageCache::getInstance()->getProdShieldImage(i.size, 1)->copy();
      else //no production
        return ImageCache::getInstance()->getProdShieldImage(i.size, 0)->copy();
      break;
    case 1: //away city
      if (i.prod) //production
        return ImageCache::getInstance()->getProdShieldImage(i.size, 3)->copy();
      else //no production
        return ImageCache::getInstance()->getProdShieldImage(i.size, 2)->copy();
      break;
    case 2: //destination city
      if (i.prod) //production
        return ImageCache::getInstance()->getProdShieldImage(i.size, 5)->copy();
      else //no production
        return ImageCache::getInstance()->getProdShieldImage(i.size, 4)->copy();
      break;
    case 3: //source city
      return ImageCache::getInstance()->getProdShieldImage(i.size, 6)->copy();
      break;
    case 4: //invalid
      return ImageCache::getInstance()->getProdShieldImage(i.size, 7)->copy();
      break;
    }
  return NULL;
}

int ProdShieldPixMaskCacheItem::comp(const ProdShieldPixMaskCacheItem item) const
{
  return
    (size < item.size) ? -1 :
    (size > item.size) ?  1 :
    (type < item.type) ? -1 :
    (type > item.type) ?  1 :
    (prod < item.prod) ? -1 :
    (prod > item.prod) ?  1 :
    0;
}

PixMask *MoveBonusPixMaskCacheItem::generate(MoveBonusPixMaskCacheItem i)
{
  return ImageCache::getInstance()->getMoveBonusImage(i.type)->copy();
}

int MoveBonusPixMaskCacheItem::comp(const MoveBonusPixMaskCacheItem item) const
{
  return
    (type < item.type) ? -1 :
    (type > item.type) ?  1 :
    0;
}

PixMask *ShipPixMaskCacheItem::generate(ShipPixMaskCacheItem i)
{
  // copy the pixmap including player colors
  return ImageCache::applyMask
    (Armysetlist::getInstance()->getShipPic(i.armyset), 
     Armysetlist::getInstance()->getShipMask(i.armyset), 
     Playerlist::getInstance()->getPlayer(i.player_id));
}

int ShipPixMaskCacheItem::comp(const ShipPixMaskCacheItem item) const
{
  return
    (player_id < item.player_id) ? -1 :
    (player_id > item.player_id) ?  1 :
    (armyset < item.armyset) ? -1 :
    (armyset > item.armyset) ?  1 :
    0;
}

PixMask *PlantedStandardPixMaskCacheItem::generate(PlantedStandardPixMaskCacheItem i)
{
  // copy the pixmap including player colors
  return ImageCache::applyMask
    (Armysetlist::getInstance()->getStandardPic(i.armyset), 
     Armysetlist::getInstance()->getStandardMask(i.armyset), 
     Playerlist::getInstance()->getPlayer(i.player_id));
}

int PlantedStandardPixMaskCacheItem::comp(const PlantedStandardPixMaskCacheItem item) const
{
  return
    (player_id < item.player_id) ? -1 :
    (player_id > item.player_id) ?  1 :
    (armyset < item.armyset) ? -1 :
    (armyset > item.armyset) ?  1 :
    0;
}

PixMask *PortPixMaskCacheItem::generate(PortPixMaskCacheItem i)
{
  return Citysetlist::getInstance()->get(i.cityset)->getPortImage()->copy();
}

int PortPixMaskCacheItem::comp(const PortPixMaskCacheItem item) const
{
  return
    (cityset < item.cityset) ? -1 :
    (cityset > item.cityset) ?  1 :
    0;
}

PixMask *SignpostPixMaskCacheItem::generate(SignpostPixMaskCacheItem i)
{
  return Citysetlist::getInstance()->get(i.cityset)->getSignpostImage()->copy();
}

int SignpostPixMaskCacheItem::comp(const SignpostPixMaskCacheItem item) const
{
  return
    (cityset < item.cityset) ? -1 :
    (cityset > item.cityset) ?  1 :
    0;
}

PixMask *BagPixMaskCacheItem::generate(BagPixMaskCacheItem i)
{
  return Armysetlist::getInstance()->getBagPic(i.armyset)->copy();
}

int BagPixMaskCacheItem::comp(const BagPixMaskCacheItem item) const
{
  return
    (armyset < item.armyset) ? -1 :
    (armyset > item.armyset) ?  1 :
    0;
}

PixMask *ExplosionPixMaskCacheItem::generate(ExplosionPixMaskCacheItem i)
{
  return Tilesetlist::getInstance()->get(i.tileset)->getExplosionImage()->copy();
}

int ExplosionPixMaskCacheItem::comp(const ExplosionPixMaskCacheItem item) const
{
  return
    (tileset < item.tileset) ? -1 :
    (tileset > item.tileset) ?  1 :
    0;
}

PixMask *NewLevelPixMaskCacheItem::generate(NewLevelPixMaskCacheItem i)
{
  bool female = i.gender == Hero::FEMALE;
  return ImageCache::applyMask
    (ImageCache::getInstance()->getNewLevelImage(female, false), 
     ImageCache::getInstance()->getNewLevelImage(female, true), 
     Playerlist::getInstance()->getPlayer(i.player_id));
}

int NewLevelPixMaskCacheItem::comp(const NewLevelPixMaskCacheItem item) const
{
  return
    (player_id < item.player_id) ? -1 :
    (player_id > item.player_id) ?  1 :
    (gender < item.gender) ? -1 :
    (gender > item.gender) ?  1 :
    0;
}

PixMask *DefaultTileStylePixMaskCacheItem::generate(DefaultTileStylePixMaskCacheItem i)
{
  PixMask *s = ImageCache::getInstance()->getDefaultTileStyleImage(i.tilestyle_type)->copy();
  PixMask::scale(s, i.tilesize, i.tilesize);
  return s;
}

int DefaultTileStylePixMaskCacheItem::comp(const DefaultTileStylePixMaskCacheItem item) const
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

PixMask *TartanPixMaskCacheItem::generate(TartanPixMaskCacheItem i)
{
  //okay, here's where we fashion the new image.
  //we take the leftmost tartan image for this player
  //and then we repeat the center tartan image a bunch of times
  //and then finally we cap it off with the rightmost tartan image
  //the images are all masked in the player's colour.

  Gdk::RGBA colour =
    Shieldsetlist::getInstance()->getColor(i.shieldset, i.player_id);
  PixMask *image = NULL, *mask = NULL;
  Shieldsetlist::getInstance()->getTartan(i.shieldset, i.player_id,
                                          Tartan::LEFT, &image, &mask);
  PixMask *left = ImageCache::applyMask(image, mask, colour);
  image = NULL;
  mask = NULL;
  Shieldsetlist::getInstance()->getTartan(i.shieldset, i.player_id,
                                          Tartan::CENTER, &image, &mask);
  PixMask *center = ImageCache::applyMask(image, mask, colour);
  image = NULL;
  mask = NULL;
  Shieldsetlist::getInstance()->getTartan(i.shieldset, i.player_id,
                                          Tartan::RIGHT, &image, &mask);
  PixMask *right = ImageCache::applyMask(image, mask, colour);
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

int TartanPixMaskCacheItem::comp(const TartanPixMaskCacheItem item) const
{
  return
    (player_id < item.player_id) ? -1 :
    (player_id > item.player_id) ?  1 :
    (width < item.width) ? -1 :
    (width > item.width) ?  1 :
    (shieldset < item.shieldset) ? -1 :
    (shieldset > item.shieldset) ?  1 :
    0;
}

PixMask *EmptyTartanPixMaskCacheItem::generate(EmptyTartanPixMaskCacheItem i)
{
  //okay, here's where we fashion the new image.
  //we take the leftmost tartan image for this player
  //and then we repeat the center tartan image a bunch of times
  //and then finally we cap it off with the rightmost tartan image
  //the images are all masked in the player's colour.

  PixMask *image = NULL, *mask = NULL;
  Shieldsetlist::getInstance()->getTartan(i.shieldset, i.player_id,
                                          Tartan::LEFT, &image, &mask);
  PixMask *left = image->copy();
  image = NULL;
  mask = NULL;
  Shieldsetlist::getInstance()->getTartan(i.shieldset, i.player_id,
                                          Tartan::CENTER, &image, &mask);
  PixMask *center = image->copy();
  image = NULL;
  mask = NULL;
  Shieldsetlist::getInstance()->getTartan(i.shieldset, i.player_id,
                                          Tartan::RIGHT, &image, &mask);
  PixMask *right = image->copy();
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

int EmptyTartanPixMaskCacheItem::comp(const EmptyTartanPixMaskCacheItem item) const
{
  return
    (player_id < item.player_id) ? -1 :
    (player_id > item.player_id) ?  1 :
    (width < item.width) ? -1 :
    (width > item.width) ?  1 :
    (shieldset < item.shieldset) ? -1 :
    (shieldset > item.shieldset) ?  1 :
    0;
}

