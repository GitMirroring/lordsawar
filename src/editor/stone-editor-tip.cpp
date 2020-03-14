//  Copyright (C) 2017 Ben Asselstine
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

#include <gtkmm.h>

#include "stone-editor-tip.h"
#include "builder-cache.h"

#include "ucompose.hpp"
#include "vector.h"
#include "defs.h"
#include "File.h"
#include "road.h"
#include "stone.h"
#include "GameMap.h"
#include "PixMask.h"
#include "ImageCache.h"
#include "tileset.h"

StoneEditorTip::StoneEditorTip(Gtk::Widget *target, MapTipPosition mpos, Stone *s, Road *r)
{
  road = r;
  stone = s;
  if (r)
  types = Stone::getSuitableTypes(Road::Type(r->getType()));
  else
  types = Stone::getTypes();
  Glib::RefPtr<Gtk::Builder> xml = 
    BuilderCache::editor_get("stone-editor-tip.ui");

  xml->get_widget("window", window);
  Gtk::Widget *w = target->get_ancestor (GTK_TYPE_WINDOW);
  if (w)
    window->set_transient_for (*dynamic_cast<Gtk::Window*>(w));
  else
    {
      w = target->get_ancestor (GTK_TYPE_DIALOG);
      if (w)
        window->set_transient_for (*dynamic_cast<Gtk::Dialog*>(w));
    }
  xml->get_widget("button_box", button_box);

  fill_stone_buttons();
  for (unsigned int i = 0; i < types.size(); i++)
    if (types[i] == s->getType())
      buttons[i]->set_active(true);
  connect_signals();

  // move into correct position
  window->get_child()->show_all();
  Vector<int> p(0, 0);
  target->get_window()->get_origin(p.x, p.y);
  if (target->get_has_window() == false)
    {
      Gtk::Allocation a = target->get_allocation();
      p.x += a.get_x();
      p.y += a.get_y();
    }
  Vector<int> size(0, 0);
  window->get_size(size.x, size.y);
  switch (mpos.justification)
    {
    case MapTipPosition::LEFT:
      window->set_gravity(Gdk::GRAVITY_NORTH_WEST);
      break;
    case MapTipPosition::RIGHT:
      window->set_gravity(Gdk::GRAVITY_NORTH_EAST);
      p.x -= size.x;
      break;
    case MapTipPosition::TOP:
      window->set_gravity(Gdk::GRAVITY_NORTH_WEST);
      break;
    case MapTipPosition::BOTTOM:
      window->set_gravity(Gdk::GRAVITY_SOUTH_WEST);
      p.y -= size.y;
      break;
    }

  p += mpos.pos;

  window->move(p.x, p.y);
  window->show();
}

PixMask *StoneEditorTip::get_grass_image()
{
  Tileset *ts = GameMap::getTileset();
  int idx = ts->getIndex(Tile::GRASS);
  if (idx == -1)
    return NULL;
  TileStyle *style = ts->getRandomTileStyle(idx, TileStyle::LONE);
  if (!style)
    return NULL;
  return style->getImage()->copy();
}

void
StoneEditorTip::fill_pixbuf (int i)
{
  Tileset *ts = GameMap::getTileset();
  int siz = GameMap::getTileset()->getTileSize();
  Glib::RefPtr<Gdk::Pixbuf> pixbuf
    = Gdk::Pixbuf::create(Gdk::COLORSPACE_RGB, true, 8, siz, siz);
  pixbuf->fill(0x00000000);
  PixMask *p = PixMask::create (pixbuf);

  //first of all, go get a grass tile
  PixMask *grass = get_grass_image();
  if (grass)
    {
      grass->blit(p->get_pixmap(), Vector<int>(0, 0));
      delete grass;
    }

  //go get the road if we're doing that
  if (road)
    {
      PixMask *r = ts->getRoadImage(road->getType());
      if (r)
        r->blit (p->get_pixmap(), Vector<int>(0, 0));
    }

  //finally, do the stone
  PixMask *s= ts->getStoneImage(types[i]);
  if (s)
    s->blit (p->get_pixmap(), Vector<int>(0, 0));

  buttons[i]->add(*manage(new Gtk::Image(p->to_pixbuf())));
  button_box->pack_start(*buttons[i], Gtk::PACK_SHRINK);
  //Gtk::Image *image = dynamic_cast<Gtk::Image*>(buttons[i]->get_child());
  //image->property_pixbuf() = p->to_pixbuf();
  delete p;
}

void StoneEditorTip::fill_stone_buttons()
{
  for (unsigned int i = 0; i < types.size(); i++)
    {
      buttons[i] = manage(new Gtk::RadioButton);
      buttons[i]->set_group(group);
      buttons[i]->property_active() = false;
      buttons[i]->property_draw_indicator() = false;
      fill_pixbuf (i);
      buttons[i]->show_all();
    }
}

void StoneEditorTip::connect_signals()
{
  for (unsigned int i = 0; i < types.size(); i++)
    buttons[i]->signal_toggled().connect(sigc::bind(sigc::mem_fun(this, &StoneEditorTip::on_stone_selected), i));
}

void StoneEditorTip::on_stone_selected(int i)
{
  if (buttons[i]->get_active() == true)
    stone_picked.emit(stone->getPos(), types[i]);
}
