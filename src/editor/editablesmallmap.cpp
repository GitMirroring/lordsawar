// Copyright (C) 2010, 2014, 2017, 2020 Ben Asselstine
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

#include <config.h>

#include <assert.h>

#include "vector.h"
#include "ImageCache.h"
#include "playerlist.h"
#include "tilesetlist.h"
#include "GameMap.h"
#include "citysetlist.h"
#include "cityset.h"
#include "RoadPathCalculator.h"
#include "path.h"
#include "font-size.h"

#include "editablesmallmap.h"

EditableSmallMap::EditableSmallMap()
{
  road_start = Vector<int>(-1,-1);
  road_finish = Vector<int>(-1,-1);
}

void EditableSmallMap::after_draw()
{
  OverviewMap::after_draw();
  draw_cities(false);
  if (road_start != Vector<int>(-1,-1))
      draw_target_box(road_start, ROAD_PLANNER_TARGET_BOX_COLOUR);
  if (road_finish != Vector<int>(-1,-1))
      draw_target_box(road_finish, ROAD_PLANNER_TARGET_BOX_COLOUR);
  map_changed.emit(surface, Gdk::Rectangle(0, 0, get_width(), get_height()));
}

Rectangle EditableSmallMap::get_cursor_rectangle(Vector<int> current_tile)
{
    int offset = (pointer_size - 1) / 2;
    Vector<int> tile = current_tile - Vector<int>(offset, offset);

    return Rectangle (tile.x, tile.y, pointer_size, pointer_size);
}

void EditableSmallMap::change_map(Vector<int> tile)
{
  bool redraw = true;
  switch (pointer)
    {
    case POINTER:
      //fixme: say what's on a given tile, and what kind of tile this is.
      //then don't redraw
      redraw = false;
      break;
    case ERASE: 
        {
          int erase_size = 3;
          int offset = (erase_size - 1) / 2;
          Vector<int> box = tile - Vector<int>(offset, offset);
          Rectangle r(box.x, box.y, erase_size, erase_size);
          bool erased = GameMap::getInstance()->eraseTiles(r);
          if (erased)
            map_edited.emit();
        }
      break;
    case TERRAIN:
        {
          Maptile *maptile = GameMap::getInstance()->getTile(tile);
          // don't change terrain to water if there is a building underneath
          if (maptile->getBuilding() != Maptile::NONE && 
              pointer_terrain == Tile::WATER)
            break;
          // don't change the terrain to anything else than grass if there is
          // a city
          if (maptile->getBuilding() == Maptile::CITY && 
              pointer_terrain != Tile::GRASS)
            break;

          Rectangle tiles = GameMap::getInstance()->putTerrain
            (get_cursor_rectangle(tile), pointer_terrain, -1, true);
          redraw_tiles(tiles);
          map_edited.emit();
        }
      break;
    case CITY:
      GameMap::getInstance()->putNewCity(tile);
      map_edited.emit();
      break;
    case RUIN:
      GameMap::getInstance()->putNewRuin(tile);
      map_edited.emit();
      break;
    case TEMPLE: 
      GameMap::getInstance()->putNewTemple(tile);
      map_edited.emit();
      break;
    case PICK_NEW_ROAD_START: 
      if (GameMap::getInstance()->getTile(tile)->getType() != Tile::WATER)
        {
          road_start = tile;
          road_start_placed.emit(tile);
          check_road();
        }
      break;
    case PICK_NEW_ROAD_FINISH: 
      if (GameMap::getInstance()->getTile(tile)->getType() != Tile::WATER)
        {
          road_finish = tile;
          road_finish_placed.emit(tile);
          check_road();
        }
      break;
    }
  if (redraw)
    draw();
  return;
}

void EditableSmallMap::mouse_button_event(MouseButtonEvent e)
{

  if (e.button == MouseButtonEvent::LEFT_BUTTON
      && e.state == MouseButtonEvent::PRESSED)
    change_map(mapFromScreen(e.pos));
}

void EditableSmallMap::mouse_motion_event(MouseMotionEvent e)
{
  if (e.pressed[MouseMotionEvent::LEFT_BUTTON])
    change_map(mapFromScreen(e.pos));
}

    
void EditableSmallMap::set_pointer(Pointer p, int size, Tile::Type t)
{
  bool redraw = false;
  if (pointer != p || pointer_size != size)
    redraw = true;
  pointer = p;
  pointer_terrain = t;
  pointer_size = size;

  if (redraw)
    draw();
}

Glib::RefPtr<Gdk::Pixbuf> EditableSmallMap::get_cursor(Vector<int> & hotspot) const
{
  Glib::RefPtr<Gdk::Pixbuf> cursor;
  switch (pointer)
    {
    case POINTER:
      cursor = ImageCache::getInstance()->getCursorPic
        (ImageCache::POINTER,
         FontSize::getInstance ()->get_height ())->to_pixbuf();
      hotspot = Vector<int>(cursor->get_width() / 2, cursor->get_height() / 2);
      break;
    case PICK_NEW_ROAD_START: 
    case PICK_NEW_ROAD_FINISH: 
      cursor = ImageCache::getInstance()->getCursorPic
        (ImageCache::TARGET,
         FontSize::getInstance ()->get_height ())->to_pixbuf();
      hotspot = Vector<int>(cursor->get_width() / 2, cursor->get_height() / 2);
      break;
    case ERASE: 
      cursor = ImageCache::getInstance()->getCursorPic
        (ImageCache::TARGET,
         FontSize::getInstance ()->get_height ())->to_pixbuf();
      hotspot = Vector<int>(cursor->get_width() / 2, cursor->get_height() / 2);
      break;
    case TERRAIN:
        {
          cursor = ImageCache::getInstance()->getCursorPic
            (ImageCache::TARGET,
             FontSize::getInstance ()->get_height ())->to_pixbuf();
          hotspot = 
            Vector<int>(cursor->get_width() / 2, cursor->get_height() / 2);
        }
      break;
    case CITY:
      cursor = ImageCache::getInstance()->getShieldPic(0, Playerlist::getInstance()->getNeutral(), true, 0)->to_pixbuf();
      hotspot = Vector<int>(cursor->get_width() / 2, cursor->get_height() / 2);
      break;
    case RUIN:
        {
          cursor = ImageCache::getInstance()->getCursorPic
            (ImageCache::TARGET,
             FontSize::getInstance ()->get_height ())->to_pixbuf();
          hotspot = 
            Vector<int>(cursor->get_width() / 2, cursor->get_height() / 2);
        }
      break;
    case TEMPLE: 
        {
          cursor = ImageCache::getInstance()->getCursorPic
            (ImageCache::TARGET,
             FontSize::getInstance ()->get_height ())->to_pixbuf();
          hotspot = 
            Vector<int>(cursor->get_width() / 2, cursor->get_height() / 2);
        }
      break;
    }
  return cursor;
}
          
bool EditableSmallMap::check_road()
{
  bool success = true;
  if (road_start == Vector<int>(-1,-1))
    success = false;
  if (road_finish == Vector<int>(-1,-1))
    success = false;
  if (road_finish == road_start)
    success = false;
  if (success == false)
    {
      road_can_be_created.emit(false);
      return false;
    }

  RoadPathCalculator rpc(road_start);
  Path *p = rpc.calculate(road_finish);
  success = false;
  if (p->size() > 0)
    success = p->back() == road_finish;
  delete p;
  road_can_be_created.emit(success);
  return success;
}

void EditableSmallMap::set_road_start(Vector<int> start)
{
  road_start = start;
  road_start_placed.emit(start);
  check_road();

}

void EditableSmallMap::set_road_finish(Vector<int> finish)
{
  road_finish = finish;
  road_finish_placed.emit(finish);
  check_road();
}

bool EditableSmallMap::create_road()
{
  if (check_road() == false)
    return false;
  RoadPathCalculator rpc(road_start);
  Path *p = rpc.calculate(road_finish);
  GameMap *gm = GameMap::getInstance();
  bool success = true;
  for (Path::iterator it = p->begin(); it != p->end(); it++)
    {
      Vector<int> pos = *it;
      //if (gm->getTile(pos)->getType() == Tile::WATER &&
          //gm->getBuilding(pos) != Maptile::BRIDGE)
        //{
          //success = false;
          //break;
        //}
      if (gm->getBuilding(pos) == Maptile::NONE)
        {
          if (GameMap::getInstance()->getBuilding(pos) == Maptile::NONE)
            GameMap::getInstance()->putNewRoad(pos);
        }
      else if (gm->getBuilding(pos) == Maptile::STONE)
        GameMap::getInstance()->putNewRoad(pos);
    }
  Rectangle r = Rectangle(0,0,GameMap::getWidth(), GameMap::getHeight());
  redraw_tiles(r);
  draw();
  map_edited.emit();
  return success;
}
    
void EditableSmallMap::clear_road()
{
  road_start = Vector<int>(-1,-1);
  road_finish = Vector<int>(-1,-1);
  draw();
  check_road();
}
