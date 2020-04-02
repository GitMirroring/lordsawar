//  Copyright (C) 2017, 2020 Ben Asselstine
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
#include <sigc++/functors/mem_fun.h>
#include <assert.h>

#include "stone-editor-dialog.h"

#include "stone.h"
#include "tileset.h"
#include "tilestyle.h"
#include "ImageCache.h"
#include "PixMask.h"
#include "GameMap.h"

#include "ucompose.hpp"
#include "defs.h"
#include "font-size.h"

#define EDITOR_STONE_TILE_RATIO 4.3

#define method(x) sigc::mem_fun(*this, &StoneEditorDialog::x)

StoneEditorDialog::StoneEditorDialog(Gtk::Window &parent, Stone *stone, Road *r)
 : LwEditorDialog(parent, "stone-editor-dialog.ui")
{
  d_changed = false;
  xml->get_widget("grid", grid);

  d_stone = stone;
  d_road = r;
  if (d_road)
    types = Stone::getSuitableTypes (Road::Type(d_road->getType()));
  else
    types = Stone::getTypes ();

  // fill in types
  const int no_columns = 5;
  for (unsigned int i = 0; i < types.size(); ++i)
    {
      Gtk::ToggleButton *toggle = manage(new Gtk::ToggleButton);
      Gtk::Image *image = new Gtk::Image();
      toggle->add(*manage(image));
      type_toggles.push_back(toggle);
      fill_pixbuf (i);

      int x = i % no_columns;
      int y = i / no_columns;
      grid->attach(*toggle, x, y, 1 , 1);
      toggle->show_all();

      if (types[i] == stone->getType())
        toggle->set_active (true);
      toggle->signal_toggled().connect(sigc::bind(method(on_type_toggled), toggle));
    }

  ignore_toggles = false;
  //type_toggles[0]->set_active(true);
}

PixMask *StoneEditorDialog::get_grass_image()
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
StoneEditorDialog::fill_pixbuf (int i)
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
  if (d_road)
    {
      PixMask *r = ts->getRoadImage(d_road->getType());
      if (r)
        r->blit (p->get_pixmap(), Vector<int>(0, 0));
    }

  //finally, do the stone
  PixMask *stone = ts->getStoneImage(types[i]);
  if (stone)
    stone->blit (p->get_pixmap(), Vector<int>(0, 0));

  {
    double ratio = EDITOR_STONE_TILE_RATIO;
    double new_height = FontSize::getInstance()->get_height () * ratio;
    int new_width =
      ImageCache::calculate_width_from_adjusted_height (p, new_height);
    PixMask::scale (p, new_width, new_height);
  }
  Gtk::Image *image = dynamic_cast<Gtk::Image*>(type_toggles[i]->get_child());
  image->property_pixbuf() = p->to_pixbuf();
}

StoneEditorDialog::~StoneEditorDialog()
{
}

bool StoneEditorDialog::run()
{
  Gtk::Image *image = dynamic_cast<Gtk::Image*>(type_toggles[0]->get_child());
  dialog->set_default_size (image->get_pixbuf ()->get_width () * 5, 
                            image->get_pixbuf ()->get_height () * 7);
  dialog->show();
  int response = dialog->run();
  if (response == Gtk::RESPONSE_REJECT)
    {
      d_changed = true;
      GameMap::getInstance ()->removeStone (d_stone->getPos ());
    }
  else if (response == Gtk::RESPONSE_ACCEPT)
    d_stone->setType (get_selected_type ());
  return d_changed;
}

void StoneEditorDialog::on_type_toggled(Gtk::ToggleButton *toggle)
{
  d_changed = true;
  selected_type = lookup_slot (toggle);
  /*
  int i = lookup_slot (toggle);
  if (toggle->get_active () == false)
    {
      if (i != -1)
        fill_pixbuf (i);
    }

    if (ignore_toggles)
	return;
    
    ignore_toggles = true;
    selected_type = i;
    for (unsigned int j = 0; j < type_toggles.size(); ++j)
      type_toggles[j]->set_active(toggle == type_toggles[j]);
    ignore_toggles = false;
    fill_pixbuf (selected_type);
    */
}

int StoneEditorDialog::lookup_slot (Gtk::ToggleButton *toggle)
{
  int slot = -1;
  for (unsigned int i = 0; i < type_toggles.size(); ++i)
    {
      if (toggle == type_toggles[i])
        slot = i;
    }
  return slot;
}
    
int StoneEditorDialog::get_selected_type () const
{
  return types[selected_type];
}
