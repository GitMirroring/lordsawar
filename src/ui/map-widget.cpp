//  Copyright (C) 2003 Michael Bartl
//  Copyright (C) 2003, 2004, 2005, 2006, 2007 Ulf Lorenz
//  Copyright (C) 2004, 2005 Bryan Duff
//  Copyright (C) 2004, 2005, 2006 Andrea Paternesi
//  Copyright (C) 2006, 2007, 2008, 2009, 2010, 2011, 2014, 2015, 2016, 2017,
//  2020, 2021, 2026 Ben Asselstine
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

#include "map-widget.h"
#include <assert.h>
#include <random>
#include "game-map.h"
#include "pixmask.h"
#include "player-list.h"
#include "army.h"
#include "item.h"
#include "stack-list.h"
#include "city.h"
#include "ruin.h"
#include "signpost.h"
#include "temple.h"
#include "port.h"
#include "bridge.h"
#include "road.h"
#include "stone.h"
#include "file.h"
#include "stack-tile.h"
#include "fog-map.h"
#include "map-backpack.h"
#include "tile-set.h"
#include "stone.h"
#include "tar-file-image.h"
#include "game-scenario-options.h"
#include "player.h"
#include "fog-map.h"
#include "map-tile.h"
#include "path-calculator.h"
#include "game-scenario-options.h"
#include "path.h"
#include "configuration.h"
#include "tar-file-masked-image.h"
#include "army-set-list.h"
#include "army-set.h"
#include "shield.h"
#include "move-result.h"

using GSO = ::GameScenarioOptions;
bool MapWidget::s_show_hidden_ruins;
using namespace std;
MapWidget::MapWidget ()
{
  set_name ("map-widget");
  set_hexpand (true);
  set_vexpand (true);

  setup_gestures ();

               
  m_selector =
    Glib::signal_timeout ().connect
    ([this] () -> bool
     {
       if (Playerlist::getActiveplayer ()->getActivestack ())
         {
           m_current_large_selector_image++;
           if (m_current_large_selector_image >= m_num_large_selector_images)
             m_current_large_selector_image = 0;
           m_current_small_selector_image++;
           if (m_current_small_selector_image >= m_num_small_selector_images)
             m_current_small_selector_image = 0;
           queue_draw ();
         }

       return true;
     }, TIMER_BIGMAP_SELECTOR);

  m_cursor_changed.connect
    ([this] (ImageCache::CursorType c)
     {
       auto pixbuf =
         ImageCache::instance ()->getCursorPic (c)->to_pixbuf ();
       auto texture = Gdk::Texture::create_for_pixbuf (pixbuf);
       auto hotspot = ImageCache::get_hotspot (c);
       auto cursor = Gdk::Cursor::create (texture, hotspot.x, hotspot.y);

       if (c == ImageCache::CursorType::POINTER)
         release_left_button ();

       m_cursor = c;
       set_cursor (cursor);
     });
  m_cursor = ImageCache::POINTER;
  m_cursor_changed.emit (m_cursor);
}

void MapWidget::setup_gestures ()
{
  auto drag = Gtk::GestureDrag::create ();
  drag->signal_drag_begin ().connect
    ([this] (double start_x, double start_y)
     {
       if (m_cursor != ImageCache::CursorType::CLOSED_HAND)
         return;
       if (m_input_locked)
         return;
       (void) start_x;
       (void) start_y;
       if (m_input_locked)
         return;
       m_drag_start_x = m_offset_x;
       m_drag_start_y = m_offset_y;
     });
  drag->signal_drag_update ().connect
    ([this] (double offset_x_delta, double offset_y_delta)
     {
       if (m_cursor != ImageCache::CursorType::CLOSED_HAND)
         return;
       if (m_input_locked)
         return;
       m_offset_x = m_drag_start_x + offset_x_delta;
       m_offset_y = m_drag_start_y + offset_y_delta;
       clamp_offset ();
       m_view_changed.emit (to_logical_extents ());
       queue_draw ();
     });
  add_controller (drag);

  auto scroll = Gtk::EventControllerScroll::create ();
  scroll->set_flags (Gtk::EventControllerScroll::Flags::VERTICAL);
  scroll->signal_scroll ().connect
    ([this] (double dx, double dy) -> bool
     {
       (void) dx;
       if (m_input_locked)
         return true;
       if (dy > 0)
         m_scale *= 1.1;
       else
         m_scale *= 0.9;
       m_scale = clamp (m_scale, MIN_ZOOM_FACTOR, MAX_ZOOM_FACTOR);
       clamp_offset ();
       m_view_changed.emit (to_logical_extents ());
       queue_draw ();
       return true;
     }, false);
  add_controller (scroll);

  auto zoom = Gtk::GestureZoom::create ();
  zoom->signal_begin ().connect
    ([this] (Gdk::EventSequence* sequence)
     {
       (void) sequence;
       if (m_input_locked)
         return;
       m_zoom_start_scale = m_scale;
       m_zoom_in_progress = true;
     });
  zoom->signal_scale_changed (). connect
    ([this] (double scale_delta)
     {
       on_zoom_scale_changed (scale_delta);
     });
  zoom->signal_end ().connect
    ([this] (Gdk::EventSequence* sequence)
     {
       (void) sequence;
       m_zoom_in_progress = false;
     });
  add_controller (zoom);

  struct MouseMotionEvent empty_event{};
  m_mouse_motion = empty_event;
 
  set_focusable (true);
  auto motion = Gtk::EventControllerMotion::create ();
  motion->signal_motion ().connect
    ([this] (double x, double y)
     {
       struct MouseMotionEvent *ev = &m_mouse_motion;
       ev->pos = Vector<int> (std::floor (x), std::floor (y));
       mouse_motion_event (*ev);
     });
  motion->signal_enter ().connect
    ([this](double, double)
     {
       grab_focus ();
     });
  add_controller (motion);

  auto xscroll = Gtk::EventControllerScroll::create ();
  xscroll->set_flags (Gtk::EventControllerScroll::Flags::BOTH_AXES);
  xscroll->signal_scroll ().connect
    ([this] (double dx, double dy)
     {
       if (m_input_locked)
         return true;
       m_offset_x -= dx;
       m_offset_y -= dy;

       clamp_offset ();
       m_view_changed.emit (to_logical_extents ());
       queue_draw ();

       return true;
     }, false);
  add_controller (xscroll);

  auto key_controller = Gtk::EventControllerKey::create ();
  key_controller->signal_key_pressed ().connect
    ([this] (guint keyval, guint, Gdk::ModifierType) -> bool
     {
       if (m_input_locked)
         return false;
       update_key_state (keyval, true);
       return false;
     }, false);

  key_controller->signal_key_released ().connect
    ([this] (guint keyval, guint, Gdk::ModifierType)
     {
       if (m_input_locked)
         return;
       update_key_state (keyval, false);
       return;
     }, false);
  add_controller (key_controller);

  auto click = Gtk::GestureClick::create ();
  click->set_button (1);
  click->signal_pressed ().connect
    ([this] (int n, double x, double y)
     {
       (void) n;
       struct MouseButtonEvent ev = to_input_event (1, x, y, true);
       struct MouseMotionEvent *evmotion = &m_mouse_motion;
       evmotion->pressed[ev.button] = true;
       mouse_button_event (ev);
     });
  add_controller (click);

  click = Gtk::GestureClick::create ();
  click->set_button (1);
  click->signal_released ().connect
    ([this] (int n, double x, double y)
     {
       (void) n;
       struct MouseButtonEvent ev = to_input_event (1, x, y, false);
       struct MouseMotionEvent *evmotion = &m_mouse_motion;
       evmotion->pressed[ev.button] = false;
       mouse_button_event (ev);
     });
  add_controller (click);

  click = Gtk::GestureClick::create ();
  click->set_button (3);
  click->signal_pressed ().connect
    ([this] (int n, double x, double y)
     {
       (void) n;
       struct MouseButtonEvent ev = to_input_event (3, x, y, true);
       struct MouseMotionEvent *evmotion = &m_mouse_motion;
       evmotion->pressed[ev.button] = true;
       mouse_button_event (ev);
     });
  add_controller (click);

  click = Gtk::GestureClick::create ();
  click->set_button (3);
  click->signal_released ().connect
    ([this] (int n, double x, double y)
     {
       (void) n;
       struct MouseButtonEvent ev = to_input_event (3, x, y, false);
       struct MouseMotionEvent *evmotion = &m_mouse_motion;
       evmotion->pressed[ev.button] = false;
       mouse_button_event (ev);
     });
  add_controller (click);
}

MapWidget::~MapWidget ()
{
  m_selector.disconnect ();
  if (m_path_calculator)
    delete m_path_calculator;
}

void MapWidget::size_allocate_vfunc (int width, int height, int baseline)
{
  Gtk::Widget::size_allocate_vfunc (width, height, baseline);
  static int first = 1;
  if (first)
    m_size_allocated.emit (width, height);
  first = 0;
}

void MapWidget::center_on_smallmap_pos (int x, int y)
{
  int view_width = get_width ();
  int view_height = get_height ();
  double scaled_tile = GameMap::instance ()->getTileSize () * m_scale;

  m_offset_x = (view_width / 2.0) - (x * scaled_tile) - (scaled_tile / 2.0);
  m_offset_y = (view_height / 2.0) - (y * scaled_tile) - (scaled_tile / 2.0);

  clamp_offset ();
  m_view_changed.emit (to_logical_extents ());
  queue_draw ();
}

LwRectangle
MapWidget::to_logical_extents ()
{
  int width = get_width ();
  int height = get_height ();
  double scaled_tile = GameMap::instance ()->getTileSize () * m_scale;
  int start_col = static_cast<int> (-m_offset_x / scaled_tile);
  int start_row = static_cast<int> (-m_offset_y / scaled_tile);
  //int end_col = start_col + (width / scaled_tile) + 2;
  //int end_row = start_row + (height / scaled_tile) + 2;
  return LwRectangle (start_col, start_row,
                      (width / scaled_tile) + 2,
                      (height / scaled_tile) + 2);
}

Vector<int>
MapWidget::to_logical_coords (double x, double y)
{
  double scaled_tile = GameMap::instance ()->getTileSize () * m_scale;

  double nx = (-m_offset_x + x)/ scaled_tile;
  double ny = (-m_offset_y + y)/ scaled_tile;
  return Vector<int> (std::floor (nx), std::floor (ny));
}

void MapWidget::clamp_offset ()
{
  double scaled_tile = GameMap::instance ()->getTileSize () * m_scale;

  double min_offset_x = get_width () - (GameMap::getWidth () * scaled_tile);
  double min_offset_y = get_height () - (GameMap::getHeight () * scaled_tile);

  //welcome to the big suckola, you are here
    {
  
      /*
       * the problem is that centering on a tile doesn't make it get to the edges.
       * here if we're close the edge we just clamp to it.
       */
      if (m_offset_x > -scaled_tile)
        m_offset_x = 0;
      if (m_offset_y > -scaled_tile)
        m_offset_y = 0;
      if (m_offset_x < min_offset_x + scaled_tile)
        m_offset_x = min_offset_x;
      if (m_offset_y < min_offset_y + scaled_tile)
        m_offset_y = min_offset_y;
    }

  m_offset_x = clamp (m_offset_x, min_offset_x, 0.0);
  m_offset_y = clamp (m_offset_y, min_offset_y, 0.0);
}

void MapWidget::get_tile_rect (Vector<int> pos,
                               double& x, double& y, double& w, double& h)
{
  double scaled_tile = GameMap::instance ()->getTileSize () * m_scale;

  x = m_offset_x + (pos.x * scaled_tile);
  y = m_offset_y + (pos.y * scaled_tile);
  w = scaled_tile;
  h = scaled_tile;
}

void MapWidget::update_selector_limits (Stack *s)
{
  Player *p = s->getOwner ();
  Tileset *t = GameMap::getTileset ();

  m_num_large_selector_images = t->getSelector (true)->getNumberOfFrames ();
  m_num_small_selector_images = t->getSelector (false)->getNumberOfFrames ();

  guint32 as = p->getArmyset ();
  Armyset *a = Armysetlist::instance ()->get (as);
  Shield::Color c = Shield::Color (p->getId ());
  if (a->getSelector (true, c)->getNumberOfFrames ())
    m_num_large_selector_images =
      a->getSelector (true, c)->getNumberOfFrames ();
  if (a->getSelector (false, c)->getNumberOfFrames ())
    m_num_small_selector_images =
      a->getSelector (false, c)->getNumberOfFrames ();
}

bool MapWidget::is_tile_visible (Vector<int> pos)
{
  int col = pos.x;
  int row = pos.y;
  double scaled_tile = GameMap::instance ()->getTileSize () * m_scale;

  double tile_left = m_offset_x + (col * scaled_tile);
  double tile_right = tile_left + scaled_tile;
  double tile_top = m_offset_y + (row * scaled_tile);
  double tile_bottom = tile_top + scaled_tile;

  int widget_width = get_width ();
  int widget_height = get_height ();

  if (tile_right <= 0 || tile_left >= widget_width ||
      tile_bottom <= 0 || tile_top >= widget_height)
    return false;

  return true;
}

void MapWidget::draw_stack (Stack *s, const Glib::RefPtr<Gtk::Snapshot> &snapshot)
{
  Vector<int> pos = s->getPos ();
  //this routine is for drawing the active stack.
  //for all other stacks see ImageCache::draw_tile_pic
  Player *player = s->getOwner ();
//  int tilesize = GameMap::instance ()->getTileSize ();

  // check if the object lies in the viewed part of the map
  // otherwise we shouldn't draw it
  if (is_tile_visible (s->getPos ()) && !s->getDeleting ())
    {
      if (s->empty ())
	{
	  std::cerr << "WARNING: empty stack found" << std::endl;
	  return;
	}

      // draw stack

      bool show_army = true;
      //we don't show the army or the flag if we're in fortified tent.
      if (s->hasShip ())
	{
          PixMask *ship = ImageCache::instance ()->getShipPic (player->getArmyset (), player->get_shield ())->copy ();
          MapWidget::blit_pixbuf (snapshot, pos, ship->to_pixbuf ());
          delete ship;
	}
      else
	{
	  if (s->getFortified () == true)
	    {
	      //We don't show the active stack here.
	      if (player->getStacklist ()->getActivestack () != s &&
		  player == Playerlist::getActiveplayer ())
		show_army = false;
	      Maptile *tile = GameMap::instance ()->getTile (s->getPos ());
	      if (tile->getBuilding () != Maptile::CITY &&
		  tile->getBuilding () != Maptile::RUIN &&
		  tile->getBuilding () != Maptile::TEMPLE)
                {
                  PixMask *tower =
                    ImageCache::instance ()->getTowerPic (player)->copy ();
                  MapWidget::blit_pixbuf (snapshot, pos, tower->to_pixbuf ());
                  delete tower;
                }
	      else
		show_army = true;
	    }

	  if (show_army == true)
	    {
	      Army *a = *s->begin ();
	      PixMask *armypic =
                ImageCache::instance ()->getArmyPic (a)->copy ();
              MapWidget::blit_pixbuf (snapshot, pos, armypic->to_pixbuf ());
              delete armypic;
	    }
	}

      if (show_army)
        {
          //does our position have us on that stacktile?
          /*
           * sometimes the stack tile isn't updated right away.
           */
          StackTile *st = GameMap::getStacks (s->getPos ());
          guint32 siz;
          if (!st->contains (s->getId ()))
            siz = st->countNumberOfArmies (player) + s->size ();
          else
            siz = st->countNumberOfArmies (player);
          if (siz > MAX_STACK_SIZE)
            siz = MAX_STACK_SIZE;
          if (siz > 0)
            {
              auto shield = player->get_shield ();
              PixMask *flag =
                ImageCache::instance ()->getFlagPic (siz, shield)->copy ();
              MapWidget::blit_pixbuf (snapshot, pos, flag->to_pixbuf ());
              delete flag;
            }
        }
    }
}

void MapWidget::draw_active_tile (const Glib::RefPtr<Gtk::Snapshot> &snapshot)
{
  ImageCache *gc = ImageCache::instance ();
  int ts = GameMap::instance ()->getTileSize ();
  Cairo::RefPtr<Cairo::ImageSurface> buffer =
    Cairo::ImageSurface::create (Cairo::Surface::Format::ARGB32, ts, ts);

  Stack* stack = Playerlist::getActiveplayer ()->getActivestack ();
  // Draw Path
  if (stack && stack->getPath ()->size () && 
      stack->getOwner ()->getType () == Player::HUMAN)
    {
      // draw all waypoints
      guint32 pathcount = 0;
      bool canMoveThere = true;
      Path::iterator end = stack->getPath ()->end ();
      //if we're dragging, we don't draw the last waypoint circle
      if (stack->getPath ()->size () > 0 && 
          (m_mouse_state == DRAGGING_STACK ||
           m_mouse_state == DRAGGING_ENDPOINT))
        --end;
      for (auto it = stack->getPath ()->begin (); it != end; ++it)
        {
          canMoveThere =
            (pathcount < stack->getPath ()->getMovesExhaustedAtPoint ());
          PixMask *waypoint;
          if (canMoveThere)
            waypoint = gc->getWaypointImage (0)->copy ();
          else
            waypoint = gc->getWaypointImage (1)->copy ();
          MapWidget::blit_pixbuf (snapshot, *it, waypoint->to_pixbuf ());
          delete waypoint;
          pathcount++;
        }

      if (m_mouse_state == DRAGGING_STACK ||
          m_mouse_state == DRAGGING_ENDPOINT ||
          m_cursor == ImageCache::GOTO_ARROW)
        {
          auto it = stack->getPath ()->end ();
          --it;
          //this is where the ghosted army unit picture goes.
          PixMask *armypic = gc->getArmyPic (*stack->begin (), true)->copy ();
          MapWidget::blit_pixbuf (snapshot, *it, armypic->to_pixbuf ());
          delete armypic;
        }
    }

  if (stack)
    {
      Player *viewer = Playerlist::getViewingplayer ();
      // draw the selection
      Vector<int> p = stack->getPos ();
      if (is_tile_visible (p) &&
          Playerlist::getViewingplayer ()->getFogMap ()->isFogged (p) == false)
        {
          draw_stack (stack, snapshot);

          PixMask *tmp = NULL;
          if (stack->size () > 1)
            tmp = gc->getSelectorPic (0, m_current_large_selector_image,
                                      stack->getOwner ())->copy ();
          else
            tmp = gc->getSelectorPic (1, m_current_small_selector_image,
                                      stack->getOwner ())->copy ();
          MapWidget::blit_pixbuf (snapshot, p, tmp->to_pixbuf ());
          delete tmp;
          //now re-fog it up because we just drew over the fog.
          if (viewer->getFogMap ()->isFogged (stack->getPos ()))
            {
              int fog_type_id = 
                viewer->getFogMap ()->getShadeTile (stack->getPos ());
              PixMask *fog = gc->getFogPic (fog_type_id)->copy ();
              MapWidget::blit_pixbuf (snapshot, p, fog->to_pixbuf ());
              delete fog;
            }
        }

      if (m_current_tile != stack->getPos () &&
          Playerlist::getActiveplayer ()->getType () == Player::HUMAN)
        {
          //this is where the ghosted army unit picture goes.
          PixMask *armypic = gc->getArmyPic (*stack->begin (), true)->copy ();
          MapWidget::blit_pixbuf (snapshot, m_current_tile,
                                  armypic->to_pixbuf ());
          delete armypic;
          static Vector<int> prev_current_tile;
          if (m_current_tile != prev_current_tile)
            {
              PathCalculator pc (stack);
              guint32 moves, turns, left;
              pc.calculate (m_current_tile, moves, turns, left);
              if (turns >= 1 && left == stack->getMaxMoves ())
                turns--;
              if (turns > 0)
                m_path_turns.emit (m_current_tile, turns +1);
              else
                m_path_turns.emit (Vector<int> (-1, -1), 0);
              prev_current_tile = m_current_tile;
            }
        }

      double x, y, w, h;
      get_tile_rect (stack->getPos (), x, y, w, h);
      graphene_rect_t dest_rect;
      graphene_rect_init (&dest_rect, x, y, w, h);
      Glib::RefPtr<Gdk::Pixbuf> pixbuf =
        Gdk::Pixbuf::create (buffer, 0, 0, ts, ts);
      auto texture = Gdk::Texture::create_for_pixbuf (pixbuf);
      snapshot->append_texture (texture, &dest_rect);
    }
  return;
}

void
MapWidget::blit_pixbuf (const Glib::RefPtr<Gtk::Snapshot> &snapshot,
                        Vector<int> pos, Glib::RefPtr<Gdk::Pixbuf> pixbuf)
{
  double scaled_tile = GameMap::instance ()->getTileSize () * m_scale;
  int col = pos.x;
  int row = pos.y;
  double x = m_offset_x + col * scaled_tile;
  double y = m_offset_y + row * scaled_tile;

  x = std::floor (x * 256.0) / 256.0;
  y = std::floor (y * 256.0) / 256.0;

  graphene_rect_t dest_rect;
  graphene_rect_init (&dest_rect, x, y,
                      ceil (scaled_tile + 0.5),
                      ceil (scaled_tile + 0.5));

  auto texture = Gdk::Texture::create_for_pixbuf (pixbuf);

  snapshot->append_texture (texture, &dest_rect);
}

void MapWidget::snapshot_vfunc (const Glib::RefPtr<Gtk::Snapshot>& snapshot)
{
  int width = get_width ();
  int height = get_height ();

  Gdk::RGBA bg_color;
  bg_color.set_rgba (0.05, 0.05, 0.1, 1.0);
  graphene_rect_t bounds;
  graphene_rect_init (&bounds, 0, 0, width, height);
  snapshot->append_color (bg_color, &bounds);

  graphene_rect_t clip_bounds;
  graphene_rect_init (&clip_bounds, 0, 0, width, height);
  snapshot->push_clip (&clip_bounds);

  double scaled_tile = GameMap::instance ()->getTileSize () * m_scale;
  int start_col = static_cast<int> (-m_offset_x / scaled_tile);
  int start_row = static_cast<int> (-m_offset_y / scaled_tile);
  int end_col = start_col + (width / scaled_tile) + 2;
  int end_row = start_row + (height / scaled_tile) + 2;

  start_col = std::max (0, start_col);
  start_row = std::max (0, start_row);
  end_col = std::min (GameMap::getWidth (), end_col);
  end_row = std::min (GameMap::getHeight (), end_row);

  for (int row = start_row; row < end_row; ++row)
    for (int col = start_col; col < end_col; ++col)
      blit_pixbuf
        (snapshot, Vector<int>(col, row),
         lookup_tile_graphics (Vector<int> (col, row))->to_pixbuf ());

  if (Playerlist::getActiveplayer()->getActivestack () && !m_blank_screen)
    draw_active_tile (snapshot);

  if (m_fighting.getPos () != Vector<int> (-1, -1) && !m_blank_screen)
    draw_explosion (snapshot);
  snapshot->pop ();
}

void MapWidget::draw_explosion (const Glib::RefPtr<Gtk::Snapshot>& snapshot)
{
  auto expl = ImageCache::instance ()->getExplosionPic ();

  double scaled_tile = GameMap::instance ()->getTileSize () * m_scale;
  double x = m_offset_x + m_fighting.getPos ().x * scaled_tile;
  double y = m_offset_y + m_fighting.getPos ().y * scaled_tile;

  x = std::floor (x * 256.0) / 256.0;
  y = std::floor (y * 256.0) / 256.0;

  graphene_rect_t dest_rect;
  graphene_rect_init (&dest_rect, x, y,
                      ceil ((scaled_tile * m_fighting.getSize ()) + 0.5),
                      ceil ((scaled_tile * m_fighting.getSize ()) + 0.5));

  auto texture = Gdk::Texture::create_for_pixbuf (expl->to_pixbuf ());

  snapshot->append_texture (texture, &dest_rect);
}

PixMask * MapWidget::lookup_tile_graphics (Vector<int> tile)
{
  guint32 tilesize = GameMap::instance ()->getTileSize ();
  Player *viewing = Playerlist::getViewingplayer ();
  ImageCache *gc = ImageCache::instance ();
  int tile_style_id = GameMap::instance ()->getTile (tile)->getTileStyleId ();
  int fog_type_id = 0;
  if (Playerlist::getViewingplayer ()->getType () != Player::HUMAN &&
      GSO::s_hidden_map == true)
    fog_type_id = FogMap::ALL;
  else
    fog_type_id = viewing->getFogMap ()->getShadeTile (tile);

  bool has_bag = false;
  guint32 bag_player_id = 0;
  bool has_standard = false;
  guint32 player_standard_id = 0;
  int stack_size = -1;
  int stack_player_id = -1;
  int army_type_id = -1;
  bool has_ship = false;
  bool has_tower = false;
  auto building_type = GameMap::instance ()->getTile (tile)->getBuilding ();
  Vector<int> building_tile = Vector<int> (-1,-1);
  int building_subtype = -1;
  int building_player_id = -1;
  int stone_type = -1;

  if (fog_type_id == FogMap::ALL)
    {
      //short circuit.  the tile is completely fogged.
      return gc->getTilePic (tile_style_id, fog_type_id, has_bag, bag_player_id,
                             has_standard, player_standard_id, stack_size,
                             stack_player_id, army_type_id, has_tower, has_ship,
                             building_type, building_subtype, building_tile,
                             building_player_id, tilesize, m_grid_toggled,
                             stone_type);
    }
  MapBackpack *backpack = GameMap::instance ()->getTile (tile)->getBackpack ();
  if (backpack && backpack->empty () == false)
    {
      bool standard_planted = false;
      Item *flag = backpack->getFirstPlantedItem ();
      if (flag)
	standard_planted = true;

      if (standard_planted && flag)
	{
	  has_standard = true;
	  player_standard_id = flag->getPlantableOriginalOwner ()->getId ();
          has_bag = backpack->size () > 1;
	}
      else
        {
          has_bag = true;
          bag_player_id = backpack->getOwnerId ();
        }
    }

  Stack *stack = GameMap::getStrongestStack (tile);
  if (stack)
    {
      if (viewing->getFogMap ()->isCompletelyObscuredFogTile (tile) == false)
	{
	  //selected stack gets drawn in gamebigmap
	  if (Playerlist::getActiveplayer ()->getActivestack () != stack)
	    {
	      stack_player_id = stack->getOwner ()->getId ();
	      Maptile *m = GameMap::instance ()->getTile (tile);
	      if (stack->getFortified () == true &&
		  m->getBuilding () != Maptile::CITY &&
		  m->getBuilding () != Maptile::RUIN &&
		  m->getBuilding () != Maptile::TEMPLE)
		has_tower = true;
	      else if (stack->hasShip () == true)
                {
                  has_ship = true;
                  stack_size =
                    GameMap::getStacks (stack->getPos ())->countNumberOfArmies
                    (Playerlist::instance ()->get (stack_player_id));
                  if (stack_size > 0 && (guint)stack_size > MAX_STACK_SIZE)
                    stack_size = stack->size ();
                  //here we show the number of armies on the tile.
                  //instead of the number of armies in the stack.
                  //so that stacks appear whole before we click on them.
                  //and that a stack of 1 can't hide a stack of 7.
                }
	      else
		{
		  army_type_id = (*stack->begin ())->getTypeId ();
                  stack_size = GameMap::getStacks (stack->getPos ())->countNumberOfArmies
                    (Playerlist::instance ()->get (stack_player_id));
                  if (stack_size > 0 && (guint)stack_size > MAX_STACK_SIZE)
                    stack_size = stack->size ();
		}
	    }
	}
    }

  if (building_type != Maptile::NONE)
    {
      switch (building_type)
	{
	case Maptile::CITY:
	    {
	      City *city = GameMap::getCity (tile);
	      building_player_id = city->getOwner ()->getId ();
	      building_tile = tile - city->getPos ();
	      if (city->isBurnt ())
		building_subtype = -1;
	      else
		building_subtype = 0;
	    }
	  break;
	case Maptile::RUIN:
	    {
	      Ruin *ruin = GameMap::getRuin (tile);
	      if (ruin->isHidden () == true && ruin->getOwner () == viewing)
		{
		  building_tile = tile - ruin->getPos ();
		  building_subtype = ruin->getType ();
		}
	      else if (ruin->isHidden () == false)
		{
		  building_tile = tile - ruin->getPos ();
		  building_subtype = ruin->getType ();
		}
              else if (s_show_hidden_ruins)
                {
		  building_tile = tile - ruin->getPos ();
		  building_subtype = ruin->getType ();
                }
	      else
		building_type = Maptile::NONE;
	    }
	  break;
	case Maptile::TEMPLE:
	    {
	      Temple *temple = GameMap::getTemple (tile);
	      building_tile = tile - temple->getPos ();
	      building_subtype = temple->getType ();
	    }
	  break;
	case Maptile::SIGNPOST:
	    {
	      Signpost *signpost = GameMap::getSignpost (tile);
	      building_tile = tile - signpost->getPos ();
	    }
	  break;
	case Maptile::ROAD:
	    {
	      Road *road = GameMap::getRoad (tile);
	      building_tile = tile - road->getPos ();
	      building_subtype = road->getType ();
              Stone *stone = GameMap::getStone (tile);
              if (stone)
                stone_type = stone->getType ();
	    }
	  break;
	case Maptile::STONE:
	    {
              Stone *stone = GameMap::getStone (tile);
	      building_tile = tile - stone->getPos ();
	      building_subtype = stone->getType ();
              stone_type = stone->getType ();
	    }
	  break;
	case Maptile::PORT:
	    {
	      Port *port = GameMap::getPort (tile);
	      building_tile = tile - port->getPos ();
	    }
	  break;
	case Maptile::BRIDGE:
	    {
	      Bridge *bridge = GameMap::getBridge (tile);
	      building_tile = tile - bridge->getPos ();
	      building_subtype = bridge->getType ();
	    }
	  break;
	case Maptile::NONE: default:
	  break;
	}
    }
  if (GameMap::getTileset ()->getStone ()->getName ().empty () == true)
    stone_type = -1;
  return
    gc->getTilePic (tile_style_id, fog_type_id, has_bag, bag_player_id,
                    has_standard, player_standard_id, stack_size,
                    stack_player_id, army_type_id, has_tower, has_ship,
                    building_type, building_subtype, building_tile,
                    building_player_id, tilesize, m_grid_toggled, stone_type);
}

void MapWidget::determine_mouse_cursor (Stack *stack, Vector<int> tile)
{
  if (tile == Vector<int>(-1, -1)) //we haven't cursored onto the map yet
      return;
  Player *active = Playerlist::getActiveplayer ();
  Player *viewing = Playerlist::getViewingplayer ();
  if (viewing->getFogMap ()->isCompletelyObscuredFogTile (tile))
    {
      m_cursor = ImageCache::HAND;
    }
  else if (m_mouse_state == DRAGGING_MAP)
    m_cursor = ImageCache::CLOSED_HAND;
  else if (stack && 
	   (m_mouse_state == DRAGGING_STACK ||
            m_mouse_state == DRAGGING_ENDPOINT))
    {
      m_cursor = ImageCache::GOTO_ARROW;
    }
  else if (stack)
    {
      m_cursor = ImageCache::FEET;
      if (stack->getPos () == tile)
	m_cursor = ImageCache::TARGET;
      else
	{
	  City *c = GameMap::getCity (tile);
	  if (c)
	    {
	      if (c->getOwner () == active)
		m_cursor = ImageCache::FEET;
	      else
		{
		  int delta = abs (tile.x - stack->getPos ().x);
		  if (delta <= 1)
		    delta = abs (tile.y - stack->getPos ().y);
		  if (delta <= 1)
		    {
		      if (is_shift_key_down ())
			m_cursor = ImageCache::QUESTION;
		      else
			{
			  Player *me = stack->getOwner ();
			  Player *them = c->getOwner ();
			  bool friendly =
                            me->getDiplomaticState (them) == Player::AT_PEACE;
			  if (friendly)
			    m_cursor = ImageCache::HEART;
			  else
			    m_cursor = ImageCache::SWORD;
			}
		    }
		  else if (c->isBurnt () == false)
		    {
		      //can i see other ppl's cities?
		      if (GSO::s_see_opponents_production == true)
			m_cursor = ImageCache::ROOK;
		      else
			m_cursor = ImageCache::HAND;
		    }
		}
	    }
	  else
	    {
	      Maptile *t = GameMap::instance ()->getTile (tile);
	      Stack *st = GameMap::getStack (tile);
	      if (st && st->getOwner () != active)
		{
		  int delta = abs (stack->getPos ().x - st->getPos ().x);
		  if (delta <= 1)
		    delta = abs (stack->getPos ().y - st->getPos ().y);
		  if (delta <= 1)
		    {
		      if (is_shift_key_down ())
			m_cursor = ImageCache::QUESTION;
		      else
			{
			  Player *me = stack->getOwner ();
			  Player *them = st->getOwner ();
			  bool friendly =
                            me->getDiplomaticState (them) == Player::AT_PEACE;
			  if (friendly)
			    m_cursor = ImageCache::HEART;
			  else
			    m_cursor = ImageCache::SWORD;
			}
		    }
		  else
		    m_cursor = ImageCache::HAND;
		}
	      else
		{
		  //Path path;
		  //why is this slower than without a stack selected?
		  //because we need to see if we can get there eventually!

		  //int moves = path.calculate (stack, tile);
		  if (m_path_calculator == NULL)
		    m_path_calculator = new PathCalculator (stack);
		  if (m_path_calculator->isReachable (tile) == false)
		  //if (moves == 0)
		    m_cursor = ImageCache::HAND;
		  else
		    {
		      if (t->getType () == Tile::WATER &&
			  GameMap::getBridge (tile) == NULL)
			{
			  if (stack->isFlying () == true)
			    m_cursor = ImageCache::FEET;
			  else
			    m_cursor = ImageCache::SHIP;
			}
		      else
			m_cursor = ImageCache::FEET;
		    }
		}
	    }
	  if (m_cursor == ImageCache::FEET && is_control_key_down ())
	    m_cursor = ImageCache::GOTO_ARROW;
	}
    }
  else
    {
      m_cursor = ImageCache::HAND;
      Stack *st;

      st = GameMap::getStack (tile);
      if (st)
	{
	  if (st->getOwner () == active)
	    m_cursor = ImageCache::TARGET;
	  else
	    m_cursor = ImageCache::HAND;
	}
      else
	{
	  Maptile *t = GameMap::instance ()->getTile (tile);
	  if (t->getBuilding () == Maptile::CITY)
	    {
	      City *c = GameMap::getCity (tile);
	      if (c->isBurnt () == true)
		m_cursor = ImageCache::HAND;
	      else if (c->getOwner () == active)
		m_cursor = ImageCache::ROOK;
	      else if (GSO::s_see_opponents_production == true)
		m_cursor = ImageCache::ROOK;
	    }
	  else if (t->getBuilding () == Maptile::RUIN)
	    {
	      Ruin *ruin = GameMap::getRuin (tile);
	      if (ruin->isHidden () == true && ruin->getOwner () == active)
		m_cursor = ImageCache::RUIN;
	      else if (ruin->isHidden () == false)
		m_cursor = ImageCache::RUIN;
	    }
	  else if (t->getBuilding () == Maptile::TEMPLE)
	    m_cursor = ImageCache::RUIN;
	}
    }
  m_cursor_changed.emit (m_cursor);
}
  
void MapWidget::update_key_state (guint keyval, bool pressed)
{
  bool caught = false;
  switch (keyval)
    {
    case GDK_KEY_Shift_L:
      m_left_shift_down = pressed;
      set_shift_key_down (pressed);
      caught = true;
      break;
    case GDK_KEY_Shift_R:
      m_right_shift_down = pressed;
      set_shift_key_down (pressed);
      caught = true;
      break;
    case GDK_KEY_Control_L:
      m_left_control_down = pressed;
      set_control_key_down (pressed);
      caught = true;
      break;
    case GDK_KEY_Control_R:
      m_right_control_down = pressed;
      set_control_key_down (pressed);
      caught = true;
      break;
    }
  if (caught)
    determine_mouse_cursor
      (Playerlist::getActiveplayer ()->getActivestack (), m_current_tile);
}

void MapWidget::on_zoom_scale_reset ()
{
  if (!m_zoom_in_progress)
    return;

  double new_scale = 1.0;

  int width = get_width ();
  int height = get_height ();
  double center_x = width / 2.0;
  double center_y = height / 2.0;

  double map_center_x = (center_x - m_offset_x) / m_scale;
  double map_center_y = (center_y - m_offset_y) / m_scale;

  m_scale = new_scale;

  m_signal_zoom_changed.emit (m_scale);
  m_offset_x = center_x - map_center_x * m_scale;
  m_offset_y = center_y - map_center_y * m_scale;

  clamp_offset ();
  m_view_changed.emit (to_logical_extents ());
  queue_draw ();
}

void MapWidget::on_zoom_scale_changed (double scale_delta)
{
  if (!m_zoom_in_progress)
    return;

  double new_scale = m_zoom_start_scale * scale_delta;
  new_scale = clamp (new_scale, MIN_ZOOM_FACTOR, MAX_ZOOM_FACTOR);

  int width = get_width ();
  int height = get_height ();
  double center_x = width / 2.0;
  double center_y = height / 2.0;

  double map_center_x = (center_x - m_offset_x) / m_scale;
  double map_center_y = (center_y - m_offset_y) / m_scale;

  m_scale = new_scale;

  m_signal_zoom_changed.emit (m_scale);
  m_offset_x = center_x - map_center_x * m_scale;
  m_offset_y = center_y - map_center_y * m_scale;

  clamp_offset ();
  m_view_changed.emit (to_logical_extents ());
  queue_draw ();
}

void MapWidget::release_left_button ()
{
  //we have a problem with left clicking on something that brings up a dialog
  //and bc the mouse button event is on the map, we never see the button
  //as the new dialog has focus.
  //we use this method to release the button here so that at least things get
  //back to normal, rather than waiting for a button release that never
  //happens.

  m_mouse_motion.pressed[MouseMotionEvent::LEFT_BUTTON] = false;
  m_current_tile = Vector<int>(-1, -1);
  m_mouse_state = NONE;
  // later in mouse_motion_event we check if we're dragging a map with a -1,-1
  // starting point, and throw out the event as we just came back from a dialog
  // and haven't updated our cursor yet.
}

void MapWidget::mouse_button_event (MouseButtonEvent e)
{
  if (m_input_locked)
    return;

  // coming back from dialogs our tile is -1, -1
  if (m_current_tile == Vector<int>(-1, -1))
    return;

  Player *active = Playerlist::getActiveplayer ();
  Player *viewing = Playerlist::getViewingplayer ();
  Vector<int> tile = mouse_pos_to_tile (e.pos);
  m_current_tile = tile;

  if (e.button == MouseButtonEvent::LEFT_BUTTON &&
      e.state == MouseButtonEvent::PRESSED)
    {
      m_city_unqueried.emit ();
      m_ruin_unqueried.emit ();
      m_temple_unqueried.emit ();
      m_signpost_unqueried.emit ();
      m_stack_unqueried.emit ();
      bool double_clicked = false;
      if (m_last_clicked.to_unix () == 0.0)
	m_last_clicked.create_now_local ();
      else
	{
          auto clicked_now = Glib::DateTime::create_now_local ();
	  double click_delta =
            clicked_now.to_unix () - m_last_clicked.to_unix ();
	  if (click_delta <= Configuration::s_double_click_threshold / 1000.0)
	    double_clicked = true;
	  m_last_clicked = clicked_now;
	}
      if (viewing->getFogMap ()->isCompletelyObscuredFogTile (tile) == true)
	return;

      Stack* stack = Playerlist::getActiveplayer ()->getActivestack ();

      if (m_cursor == ImageCache::HAND)
        {
          m_cursor = ImageCache::CLOSED_HAND;
          m_cursor_changed.emit (m_cursor);
        }
      if (m_cursor == ImageCache::CLOSED_HAND)
        return;

      if (stack)
	{
	  bool path_already_set = stack->getPath ()->size () > 0;
	  // ask for military advice
	  if (m_cursor == ImageCache::QUESTION)
	    {
	      set_shift_key_down (false);
              m_cursor_changed.emit (ImageCache::POINTER);
	      Playerlist::getActiveplayer ()->stackFightAdvise
		(stack, tile, GSO::s_intense_combat); 
	      return;
	    }
	  else if (m_cursor == ImageCache::RUIN)
	    {
              // we're holding shift down here
	      if (Ruin *r = GameMap::getRuin (tile))
		{
		  if ((r->isHidden () == true && 
		       r->getOwner () == viewing) ||
		      r->isHidden () == false)
		    {
		      set_shift_key_down (false);
                      m_cursor_changed.emit (ImageCache::POINTER);
		      m_ruin_visited.emit (r);
		    }
		}
	      else if (Temple *t = GameMap::getTemple (tile))
		{
                  m_cursor_changed.emit (ImageCache::POINTER);
		  m_temple_visited.emit (t);
		  set_shift_key_down (false);
		}
	      return;
	    }
	  else if (m_cursor == ImageCache::ROOK)
	    {
	      City* c = GameMap::getCity (tile);
	      if (c != NULL)
		{
		  if (!c->isBurnt ())
		    {
		      set_control_key_down (false);
		      if (GSO::s_see_opponents_production == true)
			{
                          m_cursor_changed.emit (ImageCache::POINTER);
			  m_city_visited.emit (c);
			  set_shift_key_down (false);
			  return;
			}
		      else
			{
			  if (c->getOwner () == Playerlist::getActiveplayer ())
			    {
                              m_cursor_changed.emit (ImageCache::POINTER);
			      m_city_visited.emit (c);
			      set_shift_key_down (false);
			      return;
			    }
			}
		    }
		}
	    }
	  else if (m_cursor == ImageCache::GOTO_ARROW)
	    {
	      //set in a course, mr crusher.
	      stack->getPath ()->calculate (stack, tile);
	      m_path_set.emit ();
	      queue_draw ();
	      return;
	    }
	  Vector<int> p;
	  p.x = tile.x; p.y = tile.y;

	  // clicked on the already active stack
	  if (stack->getPos () == tile)
	    {
	      if (double_clicked == true && is_control_key_down () == false)
		{
		  StackTile *stile = GameMap::getStacks (stack->getPos ());
		  std::vector<Stack *> stks= stile->getFriendlyStacks (active);
		  if (stks.size () == 1)
		    stile->ungroup (active);
		  else
		    {
		      stile->group (active);
		      active->stackSelect (GameMap::getStack (tile));
		      m_stack_selected.emit (GameMap::getStack (tile));
		    }
		  if (m_path_calculator)
		    delete m_path_calculator;
                  stack = active->getActivestack ();
		  m_path_calculator = new PathCalculator (stack);
		  queue_draw ();
		  m_stack_grouped_or_ungrouped.emit (stack);
		  return;
		}
              else if (double_clicked == true && is_control_key_down () == true)
                {
                  if (active->setPathOfStackToPreviousDestination (stack))
                    {
                      LwRectangle old_view = to_logical_extents ();
                      active->stackMove
                        (stack,
                         [this, active, tile, old_view] (MoveResult *res)
                         {
                           delete res;
                           if (!active->getActivestack ())
                             {
                               unselect_active_stack ();
                               determine_mouse_cursor (NULL, tile);
                             }
                           set_view (old_view);
                           m_view_changed.emit (to_logical_extents ());
                         });
                    }
                  return;
                }
	      else
		{
		  // clear the path
		  stack->getPath ()->clear ();
		  m_path_set.emit ();
		  queue_draw ();
		  return;
		}
	    }

	  //clicked on an enemy city that is too far away
	  City *c = GameMap::getCity (tile);
	  if (c && c->isBurnt () == false)
	    {
	      //restrict going into enemy cities unless they're only
	      //one square away
	      if (c->getOwner () != Playerlist::getActiveplayer ())
		{
		  int delta = abs (tile.x - stack->getPos ().x);
		  if (delta <= 1)
		    delta = abs (tile.y - stack->getPos ().y);
		  if (delta > 1)
		    return;
		}
	    }

	  int dist = stack->getPath ()->calculate (stack, p);
	  if (dist == -2)
	    std::cerr << "error calculating path!" << std::endl;

	  Vector<int> dest = Vector<int> (-1,-1);
	  if (!stack->getPath ()->empty ())
	    dest = stack->getLastPointInPath ();

	  if (dest.x == tile.x && dest.y == tile.y)
	    {
	      Playerlist::getActiveplayer ()->stackMove
                (stack,
                 [this, path_already_set] (MoveResult *r)
                 {
                   if (r->getFightOutcome () == FightResult::ATTACKER_WON)
                     m_cursor_changed.emit (ImageCache::POINTER);
                   if (!Playerlist::getActiveplayer ()->getActivestack ())
                     {
                       unselect_active_stack ();
                       return;
                     }
                   else
                     {
                       //grab our stack again because maybe we joined another stack
                       Stack *s =
                         Playerlist::getActiveplayer ()->getActivestack ();

                       //deslect when:
                       //1. we've moved our stack too far and we've gone as far
                       //   as we can on our path.
                       //2. we've set in a second path and we've gone as far as
                       //   we can on our path.
                       //3. we're next to an enemy city, out of moves, and we 
                       //   try to attack the city.
                       //note that special care is taken to not deselect when
                       //we've proceeded along our path and ran out of nodes
                       //to follow, but we still have moves to make.
                       bool deselect = false;
                       Path *path = s->getPath ();
                       if (path_already_set)
                         {
                           if (!path->empty () && 
                               s->enoughMoves () == false)
                             deselect = true;
                           else if (!path->empty () &&
                                    path->getMovesExhaustedAtPoint () == 0)
                             deselect = true;
                         }
                       else
                         {
                           if (path->empty () == false && 
                               path->getMovesExhaustedAtPoint () == 0)
                             deselect = true;
                           if ((m_cursor == ImageCache::SWORD ||
                                m_cursor == ImageCache::HEART) &&
                               s->canMove () == false)
                             deselect = true;
                           if ((m_cursor == ImageCache::FEET ||
                                m_cursor == ImageCache::SHIP) &&
                               s->canMove () == false)
                             deselect = true;
                         }

                       if (deselect)
                         {
                           Player *player = Playerlist::getActiveplayer ();
                           player->stackDeselect ();
                           unselect_active_stack ();
                         }
                     }
	  
                   m_path_set.emit ();
                   queue_draw ();
                   if (r)
                     delete r;
                 });

	    }

	  queue_draw ();
	}
      // Stack hasn't been active yet
      else
	{
	  stack = GameMap::getStack (tile);
	  if (stack && stack->isFriend (Playerlist::getActiveplayer ()) && 
	      m_cursor == ImageCache::TARGET)
	    {
	      Playerlist::getActiveplayer ()->stackSelect (stack);
	      select_active_stack ();
	    }
	  else
	    {
	      City* c = GameMap::getCity (tile);
	      if (c != NULL && m_cursor == ImageCache::ROOK)
		{
		  if (!c->isBurnt ())
		    {
		      set_control_key_down (false);
		      if (GSO::s_see_opponents_production == true)
			{
                          m_cursor_changed.emit (ImageCache::POINTER);
			  m_city_visited.emit (c);
			  set_shift_key_down (false);
			}
		      else
			{
			  if (c->getOwner () == Playerlist::getActiveplayer ())
			    {
                              m_cursor_changed.emit (ImageCache::POINTER);
			      m_city_visited.emit (c);
			      set_shift_key_down (false);
			    }
			}
		    }
		}
	      else if (Ruin *r = GameMap::getRuin (tile))
		{
		  if ((r->isHidden () == true && r->getOwner () == viewing) ||
		      r->isHidden () == false)
                    {
                      m_cursor_changed.emit (ImageCache::POINTER);
                      m_ruin_visited (r);
                    }
		}
	      else if (Temple *t = GameMap::getTemple (tile))
		{
                  m_cursor_changed.emit (ImageCache::POINTER);
		  m_temple_visited.emit (t);
		}
	    }
	}
    }
  else if (e.button == MouseButtonEvent::LEFT_BUTTON
      && e.state == MouseButtonEvent::RELEASED)
    {
      if (m_cursor == ImageCache::CLOSED_HAND)
        {
          m_cursor = ImageCache::HAND;
	  m_cursor_changed.emit (m_cursor);
        }
      if (m_mouse_state == DRAGGING_ENDPOINT)
	{
	  m_mouse_state = NONE;
	  m_cursor = ImageCache::FEET;
	  m_cursor_changed.emit (m_cursor);
	  m_path_set.emit ();
	}
      else if (m_mouse_state == DRAGGING_STACK)
	{
	  Stack* stack = Playerlist::getActiveplayer ()->getActivestack ();
	  //march a dragged stack!
	  m_mouse_state = NONE;
	  m_cursor = ImageCache::FEET;
	  m_cursor_changed.emit (m_cursor);
	  //watch out here.  
	  //we recurse for least amount of code and most programmer confusion.
	  e.state = MouseButtonEvent::PRESSED;
	  //go get the final spot in the path
	  if (stack->getPath ()->empty () == false)
	    {
	      //check if we dropped on the same tile that the stack lives on.
	      if (mouse_pos_to_tile (e.pos) != stack->getPos ())
		{
		  int ts = GameMap::instance ()->getTileSize ();
		  e.pos = tile_to_buffer_pos (stack->getLastPointInPath ());
		  e.pos.x -= ts/2;
		  e.pos.y -= ts/2;
		  mouse_button_event (e);
		}
	    }
	}
      else
	m_mouse_state = NONE;
    }

  // right mousebutton to get information about things on the map and to
  // unselect the active stack
  else if (e.button == MouseButtonEvent::RIGHT_BUTTON)
    {
      if (e.state == MouseButtonEvent::PRESSED)
	{
	  if (viewing->getFogMap ()->isCompletelyObscuredFogTile (tile) == true)
	    return;
          Stack *selected_stack = Playerlist::getActiveplayer ()->getActivestack ();
          if (selected_stack != NULL && tile == selected_stack->getPos ())
            {
              popup_stack_actions_menu (selected_stack, e.pos);
            }
          else if (City* c = GameMap::getCity (tile))
	    {
	      m_city_queried.emit (e.pos, c);
	      m_mouse_state = SHOWING_CITY;
	    }
	  else if (Ruin* r = GameMap::getRuin (tile))
	    {
	      if ((r->isHidden () == true && 
		   r->getOwner () == Playerlist::getViewingplayer ()) ||
		  r->isHidden () == false)
		{
		  m_ruin_queried.emit (r, e.pos);
		  m_mouse_state = SHOWING_RUIN;
		}
	    }
	  else if (Signpost* s = GameMap::getSignpost (tile))
	    {
	      m_signpost_queried.emit (s, e.pos);
	      m_mouse_state = SHOWING_SIGNPOST;
	    }
	  else if (Temple* t = GameMap::getTemple (tile))
	    {
              m_temple_queried.emit (t, e.pos);
	      m_mouse_state = SHOWING_TEMPLE;
	    }
	  else if (Stack *st = GameMap::getStack (tile))
	    {
	      if (GSO::s_see_opponents_stacks == true)
		{
		  m_stack_queried.emit (st, e.pos);
		  m_mouse_state = SHOWING_STACK;
		}
              else if (GSO::s_see_opponents_stacks == false &&
                       st->getStrongestHero ())
		{
		  m_stack_queried.emit (st, e.pos);
		  m_mouse_state = SHOWING_STACK;
		}
	      else if (st->getOwner () == Playerlist::getActiveplayer () && 
		       GSO::s_see_opponents_stacks == false)
		{
		  m_stack_queried.emit (st, e.pos);
		  m_mouse_state = SHOWING_STACK;
		}
	    }
	}
      else // button released
	{
	  switch (m_mouse_state)
	    {

	    case SHOWING_CITY:
	      m_city_unqueried.emit ();
	      break;

	    case SHOWING_RUIN:
	      m_ruin_unqueried.emit ();
	      break;

	    case SHOWING_TEMPLE:
	      m_temple_unqueried.emit ();
	      break;

	    case SHOWING_SIGNPOST:
	      m_signpost_unqueried.emit ();
	      break;

	    case SHOWING_STACK:
	      m_stack_unqueried.emit ();
	      break;

	    case DRAGGING_ENDPOINT:
	    case DRAGGING_STACK:
	    case DRAGGING_MAP:
	    case NONE:
	      Stack* stack = Playerlist::getActiveplayer ()->getActivestack ();
	      if (stack)
		{
		  Playerlist::getActiveplayer ()->stackDeselect ();
		  unselect_active_stack ();
                  m_path_turns.emit (Vector<int> (-1,-1), 0);
		  m_mouse_state = NONE;
		  determine_mouse_cursor (NULL, m_current_tile);
		}
	      break;
	    }

	  // in any case reset mouse state
	  m_mouse_state = NONE;
	}
    }
}

void MapWidget::unselect_active_stack ()
{
  Playerlist::getActiveplayer ()->stackDeselect ();
  queue_draw ();
  m_stack_selected.emit (0);
  if (m_path_calculator)
    {
      delete m_path_calculator;
      m_path_calculator = NULL;
    }
  determine_mouse_cursor (NULL, m_current_tile);
}

void MapWidget::select_active_stack ()
{
  auto player = Playerlist::getActiveplayer ();
  // we already have an active stack set, now we do what we need to do to 
  // make a selector animate, handle the path, and we fire off an action
  //
  // computer players just have to do the selector animation bit
  //
  // computer players will call stackSelect to get here, humans do not,
  // instead we generate the stackSelect action
  Stack* stack = player->getActivestack ();
  if (!stack)
    return;
  if (player->isHuman ())
    {
      player->stackSelect (stack);
      reset_path_calculator (stack);

      if (stack->getPath ()->checkPath (stack) == false)
        {
          //original path was blocked, so let's find a new way there.
          //this shouldn't happen because nextTurn of stack recalculates.
          //std::cerr << "original path of stack was blocked" << std::endl;
          stack->getPath ()->recalculate (stack);
        }

      m_stack_selected.emit (stack);
    }
  update_selector_limits (stack);
}

void MapWidget::reset_path_calculator (Stack *s)
{
  if (m_path_calculator)
    delete m_path_calculator;
  m_path_calculator = new PathCalculator (s);
}

void MapWidget::set_control_key_down (bool down)
{
  m_left_control_down = down;
  m_right_control_down = down;
  Player *active = Playerlist::getActiveplayer ();
  Player *viewing = Playerlist::getViewingplayer ();
  if (viewing->getFogMap ()->isCompletelyObscuredFogTile (m_current_tile) == true)
    return;
  Stack* active_stack = active->getActivestack ();
  //if the key has been released, just show what we'd normally show.
  if (m_left_control_down == false)
    {
      determine_mouse_cursor (active_stack, m_current_tile);
      return;
    }
  if (!active_stack)
    {
      if (GameMap::instance ()->getTile (m_current_tile)->getBuilding () != 
	  Maptile::CITY)
	return;

      Stack* stack;
      stack = GameMap::getFriendlyStack (m_current_tile);
      if (!stack)
	return;

      if (m_cursor == ImageCache::TARGET)
	{
	  City *city = GameMap::getCity (m_current_tile);
	  if (city->isBurnt () == false)
	    {
	      m_cursor = ImageCache::ROOK;
	      m_cursor_changed.emit (m_cursor);
	    }
	}
      else if (m_cursor == ImageCache::HAND && 
	       GSO::s_see_opponents_production == true)
	{
	  City *city = GameMap::getCity (m_current_tile);
	  if (city->isBurnt () == false)
	    {
	      m_cursor = ImageCache::ROOK;
	      m_cursor_changed.emit (m_cursor);
	    }
	}
    }
  else
    {
      if (m_cursor == ImageCache::FEET)
	{
	  m_cursor = ImageCache::GOTO_ARROW;
	  m_cursor_changed.emit (m_cursor);
	}
    }
}

void MapWidget::set_shift_key_down (bool down)
{
  m_left_shift_down = down;
  m_right_shift_down = down;
  Player *active = Playerlist::getActiveplayer ();
  Player *viewing = Playerlist::getViewingplayer ();
  if (viewing->getFogMap ()->isCompletelyObscuredFogTile (m_current_tile) == true)
      return;

  Stack* active_stack = active->getActivestack ();

  //if the key has been released, just show what we'd normally show.
  if (m_left_shift_down == false)
    {
      determine_mouse_cursor (active_stack, m_current_tile);
      return;
    }

  //otherwise the shift key is down and we need to do some more checking
  Maptile::Building b = GameMap::instance ()->getTile (m_current_tile)->getBuilding ();
  if (b == Maptile::RUIN)
    {
      Ruin *r = GameMap::getRuin (m_current_tile);
      if (r)
	{
	  if ((r->isHidden () == true && 
	       r->getOwner () == Playerlist::getActiveplayer ()) ||
	      r->isHidden () == false)
	    b = Maptile::RUIN;
	  else
	    b = Maptile::NONE;
	}
    }
  else if (b == Maptile::CITY)
    {
      if (m_cursor == ImageCache::TARGET)
	{
	  m_cursor = ImageCache::ROOK;
	  m_cursor_changed.emit (m_cursor);
	}
      else if (m_cursor == ImageCache::FEET)
	{
	  m_cursor = ImageCache::ROOK;
	  m_cursor_changed.emit (m_cursor);
	}
      else if (m_cursor == ImageCache::HAND &&
	       GSO::s_see_opponents_production == true)
	{
	  m_cursor = ImageCache::ROOK;
	  m_cursor_changed.emit (m_cursor);
	}
    }

  if (active_stack)
    {
      if (m_cursor == ImageCache::SHIP || m_cursor == ImageCache::FEET) 
	{
	  if (b == Maptile::RUIN || b == Maptile::TEMPLE)
	    m_cursor = ImageCache::RUIN;
	  else
	    m_cursor = ImageCache::HAND;
	  m_cursor_changed.emit (m_cursor);
	}
    }
  else
    {
      if (m_cursor != ImageCache::RUIN)
	{
	  if (b == Maptile::RUIN || b == Maptile::TEMPLE)
	    {
	      m_cursor = ImageCache::RUIN;
	      m_cursor_changed.emit (m_cursor);
	    }
	}
    }

  if (GSO::s_military_advisor == true)
    {
      if (active_stack && m_cursor == ImageCache::SWORD)
        {
          m_cursor = ImageCache::QUESTION;
          m_cursor_changed.emit (m_cursor);
        }
    }
}

Vector<int> MapWidget::tile_to_buffer_pos (Vector<int> pos)
{
  double scaled_tile = GameMap::instance ()->getTileSize () * m_scale;

  double x = m_offset_x + (pos.x * scaled_tile) + (scaled_tile / 2.0);
  double y = m_offset_y + (pos.y * scaled_tile) + (scaled_tile / 2.0);
  return Vector<int> ((int)x, (int)y);
}

MapTipPosition MapWidget::map_tip_position (Vector<int> pos)
{
  return map_tip_position (LwRectangle (pos.x, pos.y, 1, 1)); 
}

Vector<int> MapWidget::get_view_pos_from_view ()
{
  double scaled_tile = GameMap::instance ()->getTileSize () * m_scale;

  double x = -m_offset_x / scaled_tile;
  double y = -m_offset_y / scaled_tile;
  return Vector<int> (std::floor (x), std::floor (y));
}

MapTipPosition MapWidget::map_tip_position (LwRectangle pos_area)
{
  LwRectangle area = pos_area;

  int left, right, top, bottom;

  left = area.x;
  right = get_width () - (area.x + area.w);
  top = area.y;
  bottom = get_height () - (area.y + area.h);

  int const MARGIN = LW_BUTTON_SIZE / 2;

  // then set the position
  MapTipPosition m;
  if (right >= left && right >= top && right >= bottom)
    {
      m.pos.x = area.x + area.w + MARGIN;
      m.pos.y = area.y;
      m.justification = MapTipPosition::LEFT;
    }
  else if (left >= top && left >= bottom)
    {
      m.pos.x = area.x - MARGIN;
      m.pos.y = area.y;
      m.justification = MapTipPosition::RIGHT;
    }
  else if (bottom >= top)
    {
      m.pos.x = area.x;
      m.pos.y = area.y + area.h + MARGIN;
      m.justification = MapTipPosition::TOP;
    }
  else
    {
      m.pos.x = area.x;
      m.pos.y = area.y - MARGIN;
      m.justification = MapTipPosition::BOTTOM;
    }

  int const threshold = LW_BUTTON_SIZE * 3;
  if (get_width () > threshold && get_height () > threshold)
    {
      left = threshold;
      right = get_width () - threshold;
      top = threshold;
      bottom = get_height () - threshold;

      if (area.x >= left && area.x <= right &&
          area.y >= top && area.y <= bottom)
        {
          m.pos.x = area.x;
          m.pos.y = area.y - MARGIN;
          m.justification = MapTipPosition::BOTTOM;
        }
    }

  return m;
}

void MapWidget::mouse_motion_event (MouseMotionEvent e)
{
  static Vector<int> last_tile;
  if (m_input_locked)
    return;

  // avoid the bug of dragging the map on a bad cursor when we come back
  // from a dialog
  if (e.pressed[MouseMotionEvent::LEFT_BUTTON] &&
      m_current_tile == Vector<int>(-1, -1))
    return;

  Player *active = Playerlist::getActiveplayer ();
  Player *viewing = Playerlist::getViewingplayer ();
  Stack* stack = active->getActivestack ();
  Vector<int> tile = mouse_pos_to_tile (e.pos);
  m_current_tile = tile;
  if (tile.x < 0)
    tile.x = 0;
  if (tile.y < 0)
    tile.y = 0;
  if (tile.x >= GameMap::getWidth ())
    tile.x = GameMap::getWidth () - 1;
  if (tile.y >= GameMap::getHeight ())
    tile.y = GameMap::getHeight () - 1;

  if (e.pressed[MouseMotionEvent::LEFT_BUTTON] &&
      (m_mouse_state == NONE || m_mouse_state == SHOWING_STACK) && 
      stack && stack->getPos () == tile &&
      viewing->getFogMap ()->isCompletelyObscuredFogTile (tile) == false &&
      (m_cursor != ImageCache::HAND && m_cursor != ImageCache::CLOSED_HAND))
    {
      //initial dragging of stack from it's tile
      m_mouse_state = DRAGGING_STACK;
    }
  else if (e.pressed[MouseMotionEvent::LEFT_BUTTON] && stack &&
	   m_cursor == ImageCache::GOTO_ARROW && m_mouse_state == NONE)
    {
      //initial dragging of endpoint from it's tile
      m_mouse_state = DRAGGING_ENDPOINT;
    }
  else if (e.pressed[MouseMotionEvent::LEFT_BUTTON] &&
           (m_mouse_state == NONE || m_mouse_state == DRAGGING_MAP) &&
           (m_cursor == ImageCache::HAND ||
            m_cursor == ImageCache::CLOSED_HAND))
    {
      Vector<int> delta = -(e.pos - m_prev_mouse_pos);

      // ignore very small drags to ensure that a shaking mouse does not
      // prevent the user from making right clicks
      if (m_mouse_state == NONE && length (delta) <= 2)
	return;

      //dragging code is done in the drag gesture 
      m_mouse_state = DRAGGING_MAP;
    }

  // the following block of code shows the correct mouse cursor
  if (tile == last_tile)
    {
      m_prev_mouse_pos = e.pos;
      return;
    }

  // drag stack with left mouse button
  if (e.pressed[MouseMotionEvent::LEFT_BUTTON] &&
      (m_mouse_state == DRAGGING_STACK ||
       m_mouse_state == DRAGGING_ENDPOINT) && 
      viewing->getFogMap ()->isCompletelyObscuredFogTile (tile) == false)
    {
      //subsequent dragging
      //alright.  calculate the path, and show it but don't move
      //be careful that we don't drop our path on bad objects
      //also, slide the whole view if we drag out of view
      
      LwRectangle view = to_logical_extents ();
      if (is_inside (view, tile) == false)
	{
	  Vector<int> delta (0, 0);
	  if (tile.x >= view.x + view.w)
	    delta.x += 1;
	  if (tile.x < view.x)
	    delta.x -= 1;
	  if (tile.y > view.y + view.h)
	    delta.y += 1;
	  if (tile.y < view.y)
	    delta.y -= 1;
	  LwRectangle new_view = view;
	  new_view.pos += delta;
	  set_view (new_view);
	  m_view_changed.emit (view);
	}
      mouse_state_enum orig_state = m_mouse_state;
      m_mouse_state = NONE;
      determine_mouse_cursor (stack, tile);
      m_mouse_state = orig_state;
      if (m_cursor == ImageCache::FEET ||
	  m_cursor == ImageCache::SHIP || 
	  m_cursor == ImageCache::GOTO_ARROW || 
	  m_cursor == ImageCache::TARGET)
	{
	  guint32 moves = 0, turns = 0, left = 0;
	  Path *new_path =
            m_path_calculator->calculate (tile, moves, turns, left, true);
	  if (new_path->size ())
	    stack->setPath (*new_path);
	  delete new_path;
	  //stack->getPath ()->calculate (stack, tile);
	  m_path_set.emit ();
	  queue_draw ();
	}
    }

  determine_mouse_cursor (stack, tile);
  if (m_left_control_down || m_right_control_down)
    set_control_key_down (true);
  if (m_left_shift_down || m_right_shift_down)
    set_shift_key_down (true);

  m_prev_mouse_pos = e.pos;
  last_tile = tile;
}

void MapWidget::popup_stack_actions_menu (Stack *stack, Vector<int> pos)
{
  auto* root = get_root ();
  auto* window = dynamic_cast<Gtk::Window*>(root);

  auto menu = Gio::Menu::create ();

  menu->append (_("Info..."), "lw.view.stack");
  menu->append (_("Search..."), "lw.hero.search");
  StackTile *st = GameMap::getStacks (stack->getPos ());
  if (st->size () > 1)
    menu->append (_("Group"), "lw.order.toggle-group");
  else
    menu->append (_("Ungroup"), "lw.order.toggle-group");
  menu->append (_("Travel Along Path"), "lw.order.move");
  menu->append (_("Stay Here"), "lw.order.stay-here");
  menu->append (_("Use Item..."), "lw.hero.use-item");
  menu->append (_("Defend"), "lw.order.defend");
  menu->append (_("Disband"), "lw.order.disband");

  auto context_menu = Gtk::make_managed<Gtk::PopoverMenu> ();
  context_menu->set_menu_model (menu);
  context_menu->set_parent (*window);

  Gdk::Rectangle rect;
  rect.set_x (pos.x);
  rect.set_y (pos.y);
  rect.set_width (1);
  rect.set_height (1);

  context_menu->set_position (Gtk::PositionType::TOP);
  context_menu->set_pointing_to (rect);
  context_menu->popup ();
}
