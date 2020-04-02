//  Copyright (C) 2007 Ole Laursen
//  Copyright (C) 2007-2010, 2014, 2015, 2017, 2020 Ben Asselstine
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

#include "editorbigmap.h"

#include "army.h"
#include "stacklist.h"
#include "stack.h"
#include "stacktile.h"
#include "citylist.h"
#include "city.h"
#include "ruinlist.h"
#include "ruin.h"
#include "signpostlist.h"
#include "signpost.h"
#include "templelist.h"
#include "temple.h"
#include "bridgelist.h"
#include "bridge.h"
#include "portlist.h"
#include "port.h"
#include "roadlist.h"
#include "road.h"
#include "stonelist.h"
#include "stone.h"
#include "playerlist.h"
#include "defs.h"
#include "File.h"
#include "GameMap.h"
#include "Configuration.h"
#include "rewardlist.h"
#include "ImageCache.h"
#include "armysetlist.h"
#include "CreateScenario.h"
#include "Backpack.h"
#include "MapBackpack.h"
#include "backpack-editor-dialog.h"
#include "citysetlist.h"
#include "cityset.h"
#include "tileset.h"
#include "font-size.h"

EditorBigMap::EditorBigMap()
 : BigMap(false)
{
  mouse_pos = Vector<int>(-1, -1);
  prev_mouse_pos = Vector<int>(0, 0);

  moving_objects_from = Vector<int>(-1,-1);
  mouse_state = NONE;
  input_locked = false;
  pointer = POINTER;
  pointer_size = 1;
  pointer_terrain = Tile::GRASS;
  pointer_tile_style_id = -1;
  moving_bag = NULL;
}

void EditorBigMap::set_pointer(Pointer p, int size, Tile::Type t, 
			       int tile_style_id)
{
    bool redraw = false;
    if (pointer != p || pointer_size != size || 
	pointer_tile_style_id != tile_style_id)
      redraw = true;
    pointer = p;
    pointer_terrain = t;
    pointer_size = size;
    pointer_tile_style_id = tile_style_id;

    moving_objects_from = Vector<int>(-1,-1);
    if (redraw)
      draw();
    
}

void EditorBigMap::mouse_button_event(MouseButtonEvent e)
{
  if (input_locked)
    return;

  mouse_pos = e.pos;

  if (e.button == MouseButtonEvent::LEFT_BUTTON
      && e.state == MouseButtonEvent::PRESSED &&
      mouse_state == NONE)
    change_map_under_cursor();
  else if (e.button == MouseButtonEvent::LEFT_BUTTON &&
           e.state == MouseButtonEvent::RELEASED &&
           mouse_state == MOVE_DRAGGING && pointer == MOVE)
    {
      mouse_state = NONE;
      change_map_under_cursor();
    }
  else if (e.button == MouseButtonEvent::RIGHT_BUTTON
           && e.state == MouseButtonEvent::PRESSED)
    bring_up_details();
}

void EditorBigMap::mouse_motion_event(MouseMotionEvent e)
{
    if (input_locked)
	return;

    bool redraw = false;

    mouse_pos = e.pos;
    Vector<int> new_tile = mouse_pos_to_tile(mouse_pos);
    if (new_tile != mouse_pos_to_tile(prev_mouse_pos))
    {
	mouse_on_tile.emit(new_tile);
	redraw = true;
    }

    // draw with left mouse button
    if (e.pressed[MouseMotionEvent::LEFT_BUTTON] &&
        pointer != MOVE)
      change_map_under_cursor();
    
    // drag with right mouse button
    if (e.pressed[MouseMotionEvent::RIGHT_BUTTON]
	&& (mouse_state == NONE || mouse_state == DRAGGING))
    {
	Vector<int> delta = -(mouse_pos - prev_mouse_pos);

	// ignore very small drags to ensure that a shaking mouse does not
	// prevent the user from making right clicks
	if (mouse_state == NONE && length(delta) <= 2)
	    return;
	
	// FIXME: show a drag cursor
	
	int ts = GameMap::getInstance()->getTileSize();
	Vector<int> screen_dim(image.get_width(), image.get_height());
	view_pos = clip(Vector<int>(0, 0),
			view_pos + delta,
			GameMap::get_dim() * ts - screen_dim);

	// calculate new view position in tiles, rounding up
	Vector<int> new_view = (view_pos + Vector<int>(ts - 1, ts - 1)) / ts;

	bool redraw_buffer = false;
	
	if (new_view != view.pos)
	{
	    view.x = new_view.x;
	    view.y = new_view.y;
	    view_changed.emit(view);
	    redraw_buffer = true;
	}

	draw(redraw_buffer);
	redraw = false;
	mouse_state = DRAGGING;
    }
    else if (e.pressed[MouseMotionEvent::LEFT_BUTTON] &&
             (mouse_state == NONE || mouse_state == MOVE_DRAGGING) &&
             pointer == MOVE)
      mouse_state = MOVE_DRAGGING;

    if (redraw && pointer != POINTER)
	draw();
    
    prev_mouse_pos = mouse_pos;
}

void EditorBigMap::mouse_leave_event()
{
    mouse_pos.x = mouse_pos.y = -10000;
    mouse_on_tile.emit(Vector<int>(-100, -100));
    draw();
}

std::vector<Vector<int> > EditorBigMap::get_screen_tiles()
{
    // find out which tiles are within bounds
    std::vector<Vector<int> > tiles;

    for (int y = buffer_view.y; y < buffer_view.y + buffer_view.h; y++)
      for (int x = buffer_view.x; x < buffer_view.x + buffer_view.w; x++)
	{
	    Vector<int> tile(x, y);
	    if (tile.x >= 0 && tile.x < GameMap::getWidth() &&
		tile.y >= 0 && tile.y < GameMap::getHeight())
		tiles.push_back(tile);
	}
    
    return tiles;
}

std::vector<Vector<int> > EditorBigMap::get_cursor_tiles()
{
    // find out which cursor tiles are within bounds
    std::vector<Vector<int> > tiles;

    Vector<int> current_tile = mouse_pos_to_tile(mouse_pos);
	
    for (int y = 0; y < pointer_size; ++y)
	for (int x = 0; x < pointer_size; ++x)
	{
	    int offset = - (pointer_size - 1) / 2;
	    Vector<int> tile(x + offset, y + offset);
	    tile += current_tile;

	    if (tile.x >= 0 && tile.x < GameMap::getWidth() &&
		tile.y >= 0 && tile.y < GameMap::getHeight())
		tiles.push_back(tile);
	}
    
    return tiles;
}

Rectangle EditorBigMap::get_cursor_rectangle()
{
    // find out which cursor tiles are within bounds
    std::vector<Vector<int> > tiles;

    Vector<int> current_tile = mouse_pos_to_tile(mouse_pos);
    int offset = (pointer_size - 1) / 2;
    Vector<int> tile = current_tile - Vector<int>(offset, offset);

    return Rectangle (tile.x, tile.y, pointer_size, pointer_size);
}

int EditorBigMap::tile_to_bridge_type(Vector<int> t)
{
    // examine neighbour tiles to discover whether there's a road on them
    bool u = Roadlist::getInstance()->getObjectAt(t + Vector<int>(0, -1));
    bool b = Roadlist::getInstance()->getObjectAt(t + Vector<int>(0, 1));
    bool l = Roadlist::getInstance()->getObjectAt(t + Vector<int>(-1, 0));
    bool r = Roadlist::getInstance()->getObjectAt(t + Vector<int>(1, 0));

    // then translate this to the type
    int type = 0;
    if (!u && !b && !l && !r)
	type = 0;
    else if (u && b && l && r)
	type = 0;
    else if (!u && b && l && r)
	type = 0;
    else if (u && !b && l && r)
	type = 0;
    else if (u && b && !l && r)
	type = 1;
    else if (u && b && l && !r)
	type = 1;
    else if (u && b && !l && !r)
	type = 1;
    else if (!u && !b && l && r)
	type = 0;
    else if (u && !b && l && !r)
	type = 0;
    else if (u && !b && !l && r)
	type = 2;
    else if (!u && b && l && !r)
	type = 0;
    else if (!u && b && !l && r)
	type = 2;
    else if (u && !b && !l && !r)
	type = 3;
    else if (!u && b && !l && !r)
	type = 1;
    else if (!u && !b && l && !r)
	type = 0;
    else if (!u && !b && !l && r)
	type = 2;
    return type;
}

void EditorBigMap::change_map_under_cursor()
{
  Player* active = Playerlist::getInstance()->getActiveplayer();
  std::vector<Vector<int> > tiles = get_cursor_tiles();

  if (tiles.size() == 0)
    return;
  Vector<int> tile = tiles.front();
  Rectangle changed_tiles(tile, Vector<int>(-1, -1));
  Maptile* maptile = GameMap::getInstance()->getTile(tile);
  switch (pointer)
    {
    case POINTER:
      bring_up_details();
      break;

    case TERRAIN:

      changed_tiles = GameMap::getInstance()->putTerrain(get_cursor_rectangle(), pointer_terrain, pointer_tile_style_id, true);
      if (pointer_terrain == Tile::WATER)
        map_water_changed.emit();
      break;

    case MOVE:
      if (moving_objects_from == Vector<int>(-1,-1))
        {
          if (GameMap::getInstance()->getBuilding(tile) != Maptile::NONE ||
              GameMap::getStack(tile) != NULL ||
              GameMap::getBackpack(tile)->empty() == false)
            {
              moving_objects_from = tile;
              if (GameMap::getBackpack(tile)->empty() == false)
                moving_bag = GameMap::getBackpack (tile);
            }
        }
      else
        {
          if (mouse_state == MOVE_DRAGGING)
            break;
          Vector<int> from = moving_objects_from;
          //here we go with the move!
          GameMap *gm = GameMap::getInstance();
          if (gm->getStack(from) != NULL)
            {
              Stack *s = gm->getStack(from);
              if (!s)
                s = gm->getStack(from);
              auto enemy_stacks = gm->getEnemyStacks(tile, s->getOwner());
              if (gm->canPutStack(s->size(), s->getOwner(), tile) == true &&
                  enemy_stacks.empty() == true)
                {
                  std::vector<Stack*> friendly_stacks = gm->getFriendlyStacks(tile, s->getOwner());
                  if (friendly_stacks.empty() == true)
                    gm->moveStack(s, tile);
                  else
                    {
                      gm->moveStack(s, tile);
                      gm->groupStacks(tile, s->getOwner());
                      //big hack here.
                      //apparently the stacktile state is all messed up after 
                      //we group a stack.
                      //the signals in the game make the game state work
                      //but we don't to do all that signalling, so we cheat.
                      gm->clearStackPositions();
                      gm->updateStackPositions();
                      //also we need to clear the active stack to have it show.
                      gm->getStack(tile)->getOwner()->setActivestack(0);
                    }
                }
            }
          else if (gm->getBackpack(from)->empty() == false)
            {
              if (gm->canDropBag (tile))
                {
                  if (moving_bag->getPos () != tile)
                    gm->moveBackpack(moving_bag, tile);
                  moving_bag = NULL;
                }
              else
                break;
            }
          else if (gm->getBuilding(from) != Maptile::NONE)
            {
              guint32 s = gm->getBuildingSize(from);
              if (gm->canPutBuilding
                  (gm->getBuilding(from), s, tile, false) == true)
                gm->moveBuilding(from, tile);
              else
                {
                  if (gm->getLocation(from)->contains(tile) ||
                      LocationBox(tile, s).contains(from))
                    gm->moveBuilding(from, tile);
                }
            }
          moving_objects_from = Vector<int>(-1,-1);
        }
      break;
    case ERASE:
      // check if there is a building or a stack there and remove it
      if (GameMap::getInstance()->eraseTile(tile))
        {
          changed_tiles.pos = tile;
          changed_tiles.dim = Vector<int>(1, 1);
        }
      break;

    case STACK:
      if (GameMap::getInstance()->getStack(tile) != NULL)
        {
          map_selection_seq seq;
          Stack *s = GameMap::getStack(tile);
          if (s)
            seq.push_back(s);

          if (!seq.empty())
            objects_selected.emit(seq);
        }
      else if (GameMap::getInstance()->canPutStack(1, active, tile) == true)
        {
          // Create a new dummy stack. As we don't want to have empty
          // stacks hanging around, it's assumed that the default armyset
          // has at least one entry.
          Stack* s = new Stack(active, tile);
          const Armysetlist* al = Armysetlist::getInstance();
          Army* a = new Army(*al->getArmy(active->getArmyset(), 0), active);
          s->add(a);
          GameMap::getInstance()->putStack(s);
          //if we're on a city, change the allegiance of the stack
          //and it's armies to that of the city
          if (GameMap::getInstance()->getBuilding(s->getPos()) == Maptile::CITY)
            {
              City *c = GameMap::getCity(s->getPos());
              if (c->getOwner() != active)
                {
                  GameMap::getStacks(s->getPos())->leaving(s);
                  s = Stacklist::changeOwnership(s, c->getOwner());
                  GameMap::getStacks(s->getPos())->arriving(s);
                }
            }
        }

      break;

    case CITY:
      if (GameMap::getInstance()->getBuilding(tile) == Maptile::CITY)
        {
          map_selection_seq seq;
          City *c = GameMap::getCity(tile);
          if (c)
            seq.push_back(c);

          if (!seq.empty())
            objects_selected.emit(seq);
        }
      else
        GameMap::getInstance()->putNewCity(tile);
      break;

    case RUIN:
      if (GameMap::getInstance()->getBuilding(tile) == Maptile::RUIN)
        {
          map_selection_seq seq;
          Ruin *r = GameMap::getRuin(tile);
          if (r)
            seq.push_back(r);

          if (!seq.empty())
            objects_selected.emit(seq);
        }
      else
        GameMap::getInstance()->putNewRuin(tile);
      break;

    case TEMPLE:
      if (GameMap::getInstance()->getBuilding(tile) == Maptile::TEMPLE)
        {
          map_selection_seq seq;
          Temple *t = GameMap::getTemple(tile);
          if (t)
            seq.push_back(t);

          if (!seq.empty())
            objects_selected.emit(seq);
        }
      else
        GameMap::getInstance()->putNewTemple(tile);
      break;

    case SIGNPOST:
        {
          if (GameMap::getInstance()->getBuilding(tile) == Maptile::SIGNPOST)
            {
              map_selection_seq seq;
              Signpost *s = GameMap::getSignpost(tile);
              if (s)
                seq.push_back(s);

              if (!seq.empty())
                objects_selected.emit(seq);
            }
          else
            {
              bool signpost_placeable = GameMap::getInstance()->canPutBuilding
                (Maptile::SIGNPOST, 1, tile);
              if (!signpost_placeable)
                break;
              Signpost *s = new Signpost(tile);
              GameMap::getInstance()->putSignpost(s);
            }
          break;
        }

    case PORT:
        {
          bool port_placeable = GameMap::getInstance()->canPutBuilding
            (Maptile::PORT, 1, tile);
          if (!port_placeable)
            break;
          Port *p = new Port(tile);
          GameMap::getInstance()->putPort(p);
          break;
        }

    case BRIDGE:
        {
          if (GameMap::getBridge(tile))
            {
              GameMap::getInstance()->removeBridge(tile);
              Bridge *b = new Bridge(tile, tile_to_bridge_type (tile));
              GameMap::getInstance()->putBridge(b);
              break;
            }
          bool bridge_placeable = GameMap::getInstance()->canPutBuilding
            (Maptile::BRIDGE, 1, tile);
          if (!bridge_placeable)
            break;
          Bridge *b = new Bridge(tile, tile_to_bridge_type (tile));
          GameMap::getInstance()->putBridge(b);
          break;
        }

    case ROAD:
        {
          Maptile::Building bldg =
            GameMap::getInstance()->getTile(tile)->getBuilding();
          switch (bldg)
            {
            case Maptile::ROAD:
            case Maptile::STONE:
            case Maptile::NONE:
                {
                  bool had_stone = GameMap::getStone(tile) != NULL;
                  if (GameMap::getRoad(tile) != NULL)
                    GameMap::getInstance()->removeRoad(tile);

                  int type = CreateScenario::calculateRoadType(tile);
                  Road *r = new Road(tile, type);
                  GameMap::getInstance()->putRoad(r);
                  if (had_stone)
                    {
                      Stone *s = new Stone (tile, Road::Type(r->getType()));
                      GameMap::getInstance()->putStone(s);
                    }

                  changed_tiles.pos -= Vector<int>(1, 1);
                  changed_tiles.dim = Vector<int>(3, 3);
                }
              break;
            case Maptile::CITY:
            case Maptile::RUIN:
            case Maptile::TEMPLE:
            case Maptile::SIGNPOST:
            case Maptile::PORT:
            case Maptile::BRIDGE:
              break;
            }
          break;
        }
    case BAG:
      if (maptile->getType() != Tile::WATER)
        bag_selected.emit(tile);
      break;

    case FIGHT:
        {
          Stack *s = GameMap::getStack(tile);
          if (s)
            stack_selected_for_battle_calculator.emit(s);
        }
      break;
    case STONE:
        {
          if (GameMap::getStone(tile) != NULL)
            {
              map_selection_seq seq;
              seq.push_back(GameMap::getStone(tile));
              objects_selected.emit(seq);
            }
          else
            {
              int type = Stone::ROAD_E_AND_W_STONE_N;
              Road *r = GameMap::getRoad(tile);
              if (r)
                type = Stone::getRandomType(Road::Type(r->getType()));
              Stone *s = new Stone(tile, type);
              GameMap::getInstance()->putStone(s);
            }
        }
      break;
    }

  if (changed_tiles.w > 0 && changed_tiles.h > 0)
    map_tiles_changed.emit(changed_tiles);

  draw();
}

void EditorBigMap::bring_up_details()
{
  Vector<int> tile = mouse_pos_to_tile(mouse_pos);
  map_selection_seq seq;

  if (Stack* s = GameMap::getStack(tile))
    seq.push_back(s);
  if (City* c = GameMap::getCity(tile))
    seq.push_back(c);
  if (Ruin* r = GameMap::getRuin(tile))
    seq.push_back(r);
  if (Signpost* s = GameMap::getSignpost(tile))
    seq.push_back(s);
  if (Temple* t = GameMap::getTemple(tile))
    seq.push_back(t);
  if (Road* rd = GameMap::getRoad(tile))
    seq.push_back(rd);
  MapBackpack *b = GameMap::getInstance()->getTile(tile)->getBackpack();
  if (b->empty() == false)
    seq.push_back(b);
  if (Stone * st = GameMap::getStone(tile))
    seq.push_back(st);

  if (!seq.empty())
    objects_selected.emit(seq);
}

void EditorBigMap::smooth_view()
{
  GameMap::getInstance()->applyTileStyles(view.y, view.x, view.y+view.h, 
					  view.x+view.w, true);
  Roadlist::iterator i = Roadlist::getInstance()->begin();
  for (; i != Roadlist::getInstance()->end(); i++)
    (*i)->setType(CreateScenario::calculateRoadType((*i)->getPos()));
  draw();
}

void EditorBigMap::display_moving_building(Vector<int> src, Vector<int> dest)
{
  PixMask *pic = NULL;
  double scale = GameMap::getCityset()->get_scale ();
  switch (GameMap::getInstance()->getBuilding(src))
    {
    case Maptile::CITY:
      pic = ImageCache::getInstance()->getCityPic (GameMap::getCity (src));
      break;
    case Maptile::RUIN:
      pic = ImageCache::getInstance()->getRuinPic (GameMap::getRuin (src));
      break;
    case Maptile::TEMPLE:
      pic = ImageCache::getInstance()->getTemplePic (GameMap::getTemple (src));
      break;
    case Maptile::SIGNPOST:
      pic = ImageCache::getInstance()->getSignpostPic ();
      break;
    case Maptile::ROAD:
      pic = ImageCache::getInstance()->getRoadPic (GameMap::getRoad (src));
      break;
    case Maptile::STONE:
      pic = GameMap::getTileset()->getStoneImage (GameMap::getStone (src)->getType());
      scale = GameMap::getTileset()->get_scale ();
      break;
    case Maptile::PORT:
      pic = ImageCache::getInstance()->getPortPic ();
      break;
    case Maptile::BRIDGE:
      pic = ImageCache::getInstance()->getBridgePic (GameMap::getBridge (src));
      break;
    default:
      break;
    }
  if (pic)
    {
      PixMask *p = pic->copy ();
      double new_height = p->get_unscaled_height () * scale;
      int new_width =
        ImageCache::calculate_width_from_adjusted_height (p, new_height);
      PixMask::scale (p, new_width, new_height);
      p->blit(buffer, dest);
      delete p;
    }
  if (GameMap::getInstance()->getBuilding(src) == Maptile::ROAD)
    {
      if (GameMap::getStone(src))
        {
          PixMask *stone = GameMap::getTileset()->getStoneImage (GameMap::getStone (src)->getType());
          if (stone)
            {
              PixMask *p = stone->copy ();
              double new_height = p->get_unscaled_height () * scale;
              int new_width =
                ImageCache::calculate_width_from_adjusted_height
                (p, new_height);
              PixMask::scale (p, new_width, new_height);
              p->blit(buffer, dest);
              delete p;
            }
        }
    }
}

void EditorBigMap::blit (PixMask *src, Cairo::RefPtr<Cairo::Surface> dest, Vector<int> pos, double scale)
{
  PixMask *p = src->copy ();
  double new_height = p->get_unscaled_height () * scale;
  int new_width =
    ImageCache::calculate_width_from_adjusted_height (p, new_height);
  PixMask::scale (p, new_width, new_height);
  p->blit(dest, pos);
  delete p;
}

void EditorBigMap::after_draw()
{
  guint32 army_set_id = Playerlist::getActiveplayer()->getArmyset();
  Armyset *armyset = Armysetlist::getInstance()->get(army_set_id);

  int tilesize = GameMap::getInstance()->getTileSize();
  std::vector<Vector<int> > tiles;

  if (mouse_pos == Vector<int>(-1,-1))
    return;

  // we need to draw a drawing cursor on the map
  tiles = get_cursor_tiles();
  // draw each tile

  Gdk::RGBA terrain_box_color = Gdk::RGBA();
  terrain_box_color.set_rgba(200.0/255.0, 200.0/255.0, 200.0/255.0);
  Gdk::RGBA erase_box_color = Gdk::RGBA();
  erase_box_color.set_rgba(1.0, 1.0, 1.0);
  Gdk::RGBA move_box_color = Gdk::RGBA();
  move_box_color.set_rgba(50.0/255.0, 200.0/255.0, 50.0/255.0);
  Gdk::RGBA moving_box_color = Gdk::RGBA();
  moving_box_color.set_rgba(250.0/255.0, 250.0/255.0, 0.0/255.0);
  for (std::vector<Vector<int> >::iterator i = tiles.begin(),
       end = tiles.end(); i != end; ++i)
    {
      Vector<int> pos = tile_to_buffer_pos(*i);

      PixMask *pic;


      switch (pointer)
        {
        case POINTER:
          break;

        case TERRAIN:
          buffer_gc->set_source_rgb(terrain_box_color.get_red(),
                                    terrain_box_color.get_green(),
                                    terrain_box_color.get_blue());
          buffer_gc->move_to(pos.x+1, pos.y+1);
          buffer_gc->rel_line_to(tilesize-2, 0);
          buffer_gc->rel_line_to(0, tilesize-2);
          buffer_gc->rel_line_to(-tilesize +2, 0);
          buffer_gc->rel_line_to(0, -tilesize+2);
          buffer_gc->set_line_width(1.0);
          buffer_gc->unset_dash ();
          buffer_gc->stroke();
          break;

        case ERASE:
          buffer_gc->set_source_rgb(erase_box_color.get_red(),
                                    erase_box_color.get_green(),
                                    erase_box_color.get_blue());
          buffer_gc->move_to(pos.x+1, pos.y+1);
          buffer_gc->rel_line_to(tilesize-2, 0);
          buffer_gc->rel_line_to(0, tilesize-2);
          buffer_gc->rel_line_to(-tilesize +2, 0);
          buffer_gc->rel_line_to(0, -tilesize+2);
          buffer_gc->set_line_width(1.0);
            {
              std::vector<double> dashes;
              dashes.push_back (tilesize / 7);
              dashes.push_back (tilesize / 7);
              buffer_gc->set_dash (dashes, 0);
            }
          buffer_gc->stroke();
          break;

        case MOVE:
              
          buffer_gc->unset_dash ();
          if (moving_objects_from != Vector<int>(-1,-1))
            {
              Vector<int> tile = *i;
              buffer_gc->set_source_rgb(moving_box_color.get_red(),
                                        moving_box_color.get_green(),
                                        moving_box_color.get_blue());
              GameMap *gm = GameMap::getInstance();
              Vector<int> from = moving_objects_from;
              if (gm->getStack(from) != NULL)
                {
                  Stack *s = gm->getStack(from);
                  if (!s)
                    s = gm->getStack(from);
                  std::vector<Stack *> enemy_stacks =
                    gm->getEnemyStacks(tile, s->getOwner());
                  if (gm->canPutStack(s->size(), s->getOwner(), tile) == true &&
                      enemy_stacks.empty() == true)
                    {
                      Playerlist *plist = Playerlist::getInstance();
                      pic = ImageCache::getInstance()->getArmyPic
                        (s->getOwner()->getArmyset(), 0,
                         s->getOwner (), NULL, true, 0);
                      blit (pic, buffer, pos, armyset->get_scale ());
                      Player *o = plist->getActiveplayer ();
                      plist->setActiveplayer (s->getOwner());
                      pic = ImageCache::getInstance()->getFlagPic
                        (gm->countArmyUnits(s->getPos()),
                         s->getOwner ());
                      plist->setActiveplayer (o);
                      blit (pic, buffer, pos, gm->getTileset()->get_scale ());
                    }
                }
              else if (gm->getBackpack(from)->empty() == false)
                {
                  pic = ImageCache::getInstance()->getBagPic();
                  blit (pic, buffer, pos, armyset->get_scale ());
                }
              else if (gm->getBuilding(from) != Maptile::NONE)
                {
                  guint32 s = gm->getBuildingSize(from);
                  bool same = false;
                  if (gm->getLocation(from)->contains(tile) ||
                      LocationBox(tile, s).contains(from))
                    same = true;
                  if (gm->canPutBuilding
                      (gm->getBuilding(from), s, tile, false) == true ||
                      same)
                    display_moving_building (from, pos);
                }
            }
          else
            buffer_gc->set_source_rgb(move_box_color.get_red(),
                                      move_box_color.get_green(),
                                      move_box_color.get_blue());
          buffer_gc->move_to(pos.x+1, pos.y+1);
          buffer_gc->rel_line_to(tilesize-2, 0);
          buffer_gc->rel_line_to(0, tilesize-2);
          buffer_gc->rel_line_to(-tilesize +2, 0);
          buffer_gc->rel_line_to(0, -tilesize+2);
          buffer_gc->set_line_width(1.0);
          buffer_gc->stroke();
          break;

        case STACK:
          pic = ImageCache::getInstance()->getArmyPic
            (Playerlist::getInstance()->getActiveplayer()->getArmyset(), 0,
             Playerlist::getInstance()->getActiveplayer(), NULL, true, 0);

          blit (pic, buffer, pos, armyset->get_scale ());
          break;

        case CITY:
          pic = ImageCache::getInstance()->getCityPic(0, Playerlist::getInstance()->getActiveplayer(), GameMap::getInstance()->getCitysetId());
          blit (pic, buffer, pos, GameMap::getCityset()->get_scale ());
          break;

        case RUIN:
          pic = ImageCache::getInstance()->getRuinPic(0, GameMap::getInstance()->getCitysetId());
          blit (pic, buffer, pos, GameMap::getCityset()->get_scale ());
          break;

        case TEMPLE:
          pic = ImageCache::getInstance()->getTemplePic(0, GameMap::getInstance()->getCitysetId());
          blit (pic, buffer, pos, GameMap::getCityset()->get_scale ());
          break;

        case SIGNPOST:
          pic = ImageCache::getInstance()->getSignpostPic();
          blit (pic, buffer, pos, GameMap::getCityset()->get_scale ());
          break;

        case ROAD:
            {
              Road *r = GameMap::getRoad(*i);
              if (r)
                pic = ImageCache::getInstance()->getRoadPic(r->getType());
              else
                pic = ImageCache::getInstance()->getRoadPic(CreateScenario::calculateRoadType(*i));
              blit (pic, buffer, pos, GameMap::getTileset()->get_scale ());
            }
          break;
        case STONE:
            {
              Tileset *t = GameMap::getTileset();
              Stone *s = GameMap::getStone(*i);
              if (s)
                pic = t->getStoneImage(s->getType());
              else
                {
                  Road *r = GameMap::getRoad(*i);
                  if (r)
                    pic = t->getStoneImage(Stone::getRandomType
                                           (Road::Type(r->getType())));
                  else
                    pic = t->getStoneImage
                      (Stone::ROAD_ALL_DIRECTIONS_STONES_NW_NE_SW_SE);
                }
              if (pic)
                blit (pic, buffer, pos, GameMap::getTileset()->get_scale ());
            }
          break;
        case PORT:
          pic = ImageCache::getInstance()->getPortPic();
          blit (pic, buffer, pos, GameMap::getCityset()->get_scale ());
          break;
        case BRIDGE:
          pic = ImageCache::getInstance()->getBridgePic(tile_to_bridge_type(*i));
          blit (pic, buffer, pos, GameMap::getCityset()->get_scale ());
          break;
        case BAG:
            {
              pic = ImageCache::getInstance()->getBagPic();
              //Vector<int> offset = Vector<int>(tilesize,tilesize) - 
                //Vector<int>(pic->get_width(), pic->get_height());
              blit (pic, buffer, pos, armyset->get_scale ());
            }
          break;
        case FIGHT:
            {
              pic =
                ImageCache::getInstance()->getCursorPic
                (ImageCache::SWORD, FontSize::getInstance ()->get_height ());
              PixMask *copy = pic->copy();
              PixMask::scale (copy, tilesize * 0.66, tilesize * 0.66);
              copy->blit_centered(buffer, pos +
                         Vector<int>(tilesize /2, tilesize /2));
              delete copy;
            }
          break;
        }
    }
  return;
}
