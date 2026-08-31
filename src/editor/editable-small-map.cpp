//  Copyright (C) 2010, 2014, 2017, 2020, 2021, 2026 Ben Asselstine
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

#include <config.h>

#include <assert.h>

#include "vector.h"
#include "image-cache.h"
#include "player-list.h"
#include "tile-set-list.h"
#include "game-map.h"
#include "city-set-list.h"
#include "city-set.h"
#include "road-path-calculator.h"
#include "path.h"
#include "smallmap-editor-undo-actions.h"

#include "editable-small-map.h"

EditableSmallMap::EditableSmallMap ()
 : m_pointer (NONE), m_pointer_terrain (0), m_pointer_size (5),
    m_road_start (Vector<int>(-1,-1)), m_road_finish (Vector<int>(-1,-1))
{
}

void EditableSmallMap::after_draw ()
{
  OverviewMap::after_draw ();
  draw_cities (false);
  if (m_road_start != Vector<int>(-1,-1))
    draw_target_box (m_road_start, ROAD_PLANNER_TARGET_BOX_COLOR);
  if (m_road_finish != Vector<int>(-1,-1))
    draw_target_box (m_road_finish, ROAD_PLANNER_TARGET_BOX_COLOR);
  m_map_changed.emit (surface,
                      Gdk::Rectangle (0, 0, get_width (), get_height ()));
}

LwRectangle EditableSmallMap::get_cursor_rectangle (Vector<int> current_tile)
{
  int offset = (m_pointer_size - 1) / 2;
  Vector<int> tile = current_tile - Vector<int>(offset, offset);

  return LwRectangle (tile.x, tile.y, m_pointer_size, m_pointer_size);
}

void EditableSmallMap::change_map (Vector<int> tile)
{
  Cityset *cs = GameMap::getCityset ();
  bool redraw = true;
  switch (m_pointer)
    {
    case NONE:
      redraw = false;
      break;

    case ERASE:
        {
          int erase_size = 3;
          int offset = (erase_size - 1) / 2;
          Vector<int> box = tile - Vector<int>(offset, offset);
          LwRectangle r (box.x, box.y, erase_size, erase_size);
          auto action =
            new SmallMapEditorUndoAction_Erase (GameMap::get_boundary ());
          bool erased = GameMap::instance ()->eraseTiles (r);
          if (erased)
            {
              m_undo_actions.push_back (action);
              m_map_edited.emit ();
            }
          else
            delete action;
        }
      break;

    case TERRAIN:
        {
          m_undo_actions.push_back
            (new SmallMapEditorUndoAction_Terrain
             (m_pointer_terrain, get_cursor_rectangle (tile)));

          auto r = get_cursor_rectangle (tile);
          LwRectangle tiles =
            GameMap::instance ()->putTerrain (r, m_pointer_terrain, -1, true);
          redraw_tiles (tiles);
            {
              auto t = GameMap::getTileset ();
              auto type = (*t)[m_pointer_terrain]->getType ();
              if (type == Tile::WATER)
                m_map_water_changed.emit ();
            }
          m_map_edited.emit ();
        }
      break;

    case CITY:
        {
          GameMap *gm = GameMap::instance ();
          bool city_placeable =
            gm->canPutBuilding (Maptile::CITY, cs->getCityTileWidth (), tile);
          if (city_placeable)
            {
              LwRectangle rect = LwRectangle (tile);
              rect.dim =
                Vector<int>(cs->getCityTileWidth (), cs->getCityTileWidth ());
              m_undo_actions.push_back
                (new SmallMapEditorUndoAction_City (rect));
              GameMap::instance ()->putNewCity (tile);
              m_map_edited.emit ();
            }
        }
      break;

    case RUIN:
        {
          GameMap *gm = GameMap::instance  ();
          bool ruin_placeable =
            gm->canPutBuilding (Maptile::RUIN, cs->getRuinTileWidth (), tile);
          if (ruin_placeable)
            {
              LwRectangle rect = LwRectangle (tile);
              rect.dim =
                Vector<int>(cs->getRuinTileWidth (), cs->getRuinTileWidth ());
              m_undo_actions.push_back
                (new SmallMapEditorUndoAction_Ruin (rect));
              GameMap::instance ()->putNewRuin (tile);
              m_map_edited.emit ();
            }
        }
      break;

    case TEMPLE:
        {
          GameMap *gm = GameMap::instance ();
          bool temple_placeable =
            gm->canPutBuilding (Maptile::TEMPLE, cs->getTempleTileWidth (),
                                tile);
          if (temple_placeable)
            {
              LwRectangle rect = LwRectangle (tile);
              rect.dim = Vector<int>(cs->getTempleTileWidth (),
                                     cs->getTempleTileWidth ());
              m_undo_actions.push_back
                (new SmallMapEditorUndoAction_Temple (rect));
              GameMap::instance ()->putNewTemple (tile);
              m_map_edited.emit ();
            }
        }
      break;

    case PICK_NEW_ROAD_START:
      if (GameMap::instance ()->getTile (tile)->getType () != Tile::WATER)
        {
          m_undo_actions.push_back
            (new SmallMapEditorUndoAction_PlaceStart (m_road_start));
          m_road_start = tile;
          m_road_start_placed.emit (tile);
        }
      break;

    case PICK_NEW_ROAD_FINISH:
      if (GameMap::instance ()->getTile (tile)->getType () != Tile::WATER)
        {
          m_undo_actions.push_back
            (new SmallMapEditorUndoAction_PlaceFinish (m_road_finish));
          m_road_finish = tile;
          m_road_start_placed.emit (tile);
        }
      break;
    }

  if (redraw)
    draw ();
  return;
}

void EditableSmallMap::mouse_button_event (MouseButtonEvent e)
{
  if (e.state == MouseButtonEvent::RELEASED)
    {
      for (auto undo : m_undo_actions)
        m_undo_map.emit (undo);
      m_undo_actions.clear ();
    }
  if (e.button == MouseButtonEvent::LEFT_BUTTON &&
      e.state == MouseButtonEvent::PRESSED)
    change_map (mapFromScreen (e.pos));
  //else if (e.button == MouseButtonEvent::LEFT_BUTTON &&
           //e.state == MouseButtonEvent::RELEASED &&
           //m_pointer == TERRAIN)
    //m_undo_map.emit (new SmallMapEditorUndoAction_Blank ());
}

void EditableSmallMap::mouse_motion_event (MouseMotionEvent e)
{
  if (e.pressed[MouseMotionEvent::LEFT_BUTTON] && m_pointer == TERRAIN)
    change_map (mapFromScreen (e.pos));
}

void EditableSmallMap::set_pointer (Pointer p, int size, int t)
{
  bool redraw = false;
  if (m_pointer != p || m_pointer_size != size)
    redraw = true;
  m_pointer = p;
  m_pointer_terrain = t;
  m_pointer_size = size;

  if (redraw)
    draw ();
}

PixMask *EditableSmallMap::get_cursor (Vector<int> &sp) const
{
  PixMask *cursor = NULL;
  switch (m_pointer)
    {
    case NONE:
      cursor = ImageCache::instance ()->getCursorPic (ImageCache::POINTER);
      sp = Vector<int>(cursor->get_width () / 2, cursor->get_height () / 2);
      break;

    case PICK_NEW_ROAD_START:
    case PICK_NEW_ROAD_FINISH:
      cursor = ImageCache::instance ()->getCursorPic (ImageCache::TARGET);
      sp = Vector<int>(cursor->get_width () / 2, cursor->get_height () / 2);
      break;

    case ERASE:
      cursor = ImageCache::instance ()->getCursorPic (ImageCache::TARGET);
      sp = Vector<int>(cursor->get_width () / 2, cursor->get_height () / 2);
      break;

    case TERRAIN:
      cursor =
        ImageCache::instance ()->getBoxPic
        (m_pointer_size * pixels_per_tile, Gdk::RGBA ("white"),
         false, false, 1, false);
      sp = Vector<int>(cursor->get_width () / 2, cursor->get_height () / 2);
      break;

    case CITY:
      cursor =
        ImageCache::instance ()->getShieldPic (0, Playerlist::getNeutral (),
                                               true);
      sp = Vector<int>(cursor->get_width () / 2, cursor->get_height () / 2);
      break;

    case RUIN:
      cursor = ImageCache::instance ()->getCursorPic (ImageCache::TARGET);
      sp = Vector<int>(cursor->get_width () / 2, cursor->get_height () / 2);
      break;

    case TEMPLE:
      cursor = ImageCache::instance ()->getCursorPic (ImageCache::TARGET);
      sp = Vector<int>(cursor->get_width () / 2, cursor->get_height () / 2);
      break;
    }
  return cursor;
}

bool EditableSmallMap::check_road ()
{
  bool success = true;
  if (m_road_start == Vector<int>(-1,-1))
    success = false;
  if (m_road_finish == Vector<int>(-1,-1))
    success = false;
  if (m_road_finish == m_road_start)
    success = false;
  if (success == false)
    return false;

  RoadPathCalculator rpc (m_road_start);
  Path *p = rpc.calculate (m_road_finish);
  success = false;
  if (p->size () > 0)
    success = p->back () == m_road_finish;
  delete p;
  return success;
}

bool EditableSmallMap::create_road ()
{
  if (check_road () == false)
    return false;
  RoadPathCalculator rpc (m_road_start);
  Path *p = rpc.calculate (m_road_finish);
  GameMap *gm = GameMap::instance ();
  bool success = true;
  for (auto pos : *p)
    {
      if (gm->getBuilding (pos) == Maptile::NONE)
        {
          if (GameMap::instance ()->getBuilding (pos) == Maptile::NONE)
            GameMap::instance ()->putNewRoad (pos);
        }
      else if (gm->getBuilding (pos) == Maptile::STONE)
        GameMap::instance ()->putNewRoad (pos);
    }
  LwRectangle r =
    LwRectangle (0, 0, GameMap::getWidth (), GameMap::getHeight ());

  redraw_tiles (r);
  draw ();
  m_map_edited.emit ();
  return success;
}

void EditableSmallMap::update ()
{
  LwRectangle r =
    LwRectangle (0, 0, GameMap::getWidth (), GameMap::getHeight ());
  redraw_tiles (r);
  draw ();
  m_map_changed.emit (surface,
                      Gdk::Rectangle (0, 0, get_width (), get_height ()));
  m_map_edited.emit ();
}

void EditableSmallMap::clear_road ()
{
  m_road_start = Vector<int>(-1,-1);
  m_road_finish = Vector<int>(-1,-1);
  draw ();
}

void EditableSmallMap::setRoadFinish (Vector<int> p)
{
  m_road_finish = p;
  draw ();
  check_road ();
}

void EditableSmallMap::setRoadStart (Vector<int> p)
{
  m_road_start = p;
  draw ();
  check_road ();
}
