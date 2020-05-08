//  Copyright (C) 2009, 2010, 2012, 2014, 2020 Ben Asselstine
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
#include <sigc++/functors/mem_fun.h>
#include <gtkmm.h>

#include "switch-sets-dialog.h"

#include "defs.h"
#include "File.h"
#include "tileset.h"
#include "tilesetlist.h"
#include "armysetlist.h"
#include "citysetlist.h"
#include "shieldsetlist.h"
#include "ucompose.hpp"
#include "GameMap.h"
#include "ruinlist.h"
#include "citylist.h"
#include "armyset.h"
#include "shieldset.h"
#include "cityset.h"
#include "playerlist.h"
#include "player.h"
#include "font-size.h"

#define method(x) sigc::mem_fun(*this, &SwitchSetsDialog::x)

SwitchSetsDialog::SwitchSetsDialog(Gtk::Window &parent)
 :LwEditorDialog(parent, "switch-sets-dialog.ui")
{
    xml->get_widget("accept_button", accept_button);
    xml->get_widget("make_same_button", make_same_button);

    // fill in tile themes combobox

    guint32 counter = 0;
    guint32 default_id = 0;
    Gtk::Box *box;

    //fill in tile sizes combobox
    tile_size_combobox = manage(new Gtk::ComboBoxText);
    std::list<guint32> sizes;
    Tilesetlist::getInstance()->getSizes(sizes);
    Citysetlist::getInstance()->getSizes(sizes);
    Armysetlist::getInstance()->getSizes(sizes);
    for (std::list<guint32>::iterator it = sizes.begin(); it != sizes.end();
	 ++it)
      {
	Glib::ustring s = String::ucompose("%1x%1", *it);
	tile_size_combobox->append(s);
	if ((*it) == GameMap::getInstance()->getTileSize())
	  default_id = counter;
	counter++;
      }
    tile_size_combobox->set_active(default_id);
    xml->get_widget("tile_size_box", box);
    box->pack_start(*tile_size_combobox, Gtk::PACK_SHRINK);

    // make new tile themes combobox
    tile_theme_combobox = manage(new Gtk::ComboBoxText);
    xml->get_widget("tile_theme_box", box);
    box->pack_start(*tile_theme_combobox, Gtk::PACK_SHRINK);

    // make new city themes combobox
    city_theme_combobox = manage(new Gtk::ComboBoxText);
    xml->get_widget("city_theme_box", box);
    box->pack_start(*city_theme_combobox, Gtk::PACK_SHRINK);

    // we can make and fill in the shieldset because it doesn't depend on
    // the tile size
    bool empty = false;
    fill_shield_themes (empty);

    xml->get_widget("shield_theme_box", box);
    box->pack_start(*shield_theme_combobox, Gtk::PACK_SHRINK);

    // make the armyset comboboxes and save them for later
    xml->get_widget("armysets_grid", armysets_grid);
    Gdk::RGBA white = Gdk::RGBA ("white");
    armysets_grid->override_background_color(white);
    int px = FontSize::getInstance ()->get_height () / 2;
    armysets_grid->property_row_spacing () = px;
    armysets_grid->property_column_spacing () = px;
    int row = 0;
    for (auto p : *Playerlist::getInstance ())
      {
        Gtk::Label *l = Gtk::manage (new Gtk::Label (p->getName ()));
        l->property_hexpand () = true;
        l->property_halign () = Gtk::ALIGN_START;
        l->property_margin_left () = px;
        armysets_grid->attach (*l, 0, row);
        Gtk::ComboBoxText *c = manage(new Gtk::ComboBoxText);
        c->property_margin_right () = px;
        armysets_grid->attach (*c, 1, row);
        row++;
        army_theme_comboboxes.push_back (c);
      }

    on_tile_size_changed();

    tileset_changed = false;
    armyset_changed = false;
    cityset_changed = false;
    shieldset_changed = false;
}

guint32 SwitchSetsDialog::get_active_tile_size()
{
  return (guint32) atoi(tile_size_combobox->get_active_text().c_str());
}

void SwitchSetsDialog::fill_shield_themes (bool &empty)
{
  guint32 counter = 0;
  guint32 default_id = 0;
  shield_theme_combobox = manage(new Gtk::ComboBoxText);
  Shieldsetlist *sl = Shieldsetlist::getInstance();
  std::list<Glib::ustring> shield_themes = sl->getValidNames();
  for (std::list<Glib::ustring>::iterator i = shield_themes.begin(),
       end = shield_themes.end(); i != end; ++i)
    {
      if (*i == GameMap::getInstance()->getShieldset()->getName())
        default_id = counter;
      shield_theme_combobox->append(Glib::filename_to_utf8(*i));
      counter++;
    }

  shield_theme_combobox->set_active(default_id);
  empty = shield_theme_combobox->get_model()->children().empty ();
}

void SwitchSetsDialog::fill_tile_themes (bool &empty)
{
  guint32 default_id = 0;
  guint32 counter = 0;
  tile_theme_combobox->remove_all();
  Tilesetlist *tl = Tilesetlist::getInstance();
  std::list<Glib::ustring> tile_themes = tl->getValidNames(get_active_tile_size());
  for (std::list<Glib::ustring>::iterator i = tile_themes.begin(),
       end = tile_themes.end(); i != end; ++i)
    {
      if (*i == GameMap::getInstance()->getTileset()->getName())
        default_id = counter;
      tile_theme_combobox->append(Glib::filename_to_utf8(*i));
      counter++;
    }
  tile_theme_combobox->set_active(default_id);
  empty = tile_theme_combobox->get_model()->children().empty ();
}

void SwitchSetsDialog::fill_city_themes (bool &empty)
{
  guint32 counter = 0;
  guint32 default_id = 0;
  city_theme_combobox->remove_all();
  Citysetlist *cl = Citysetlist::getInstance();
  std::list<Glib::ustring> city_themes = cl->getValidNames(get_active_tile_size());
  Cityset *active = GameMap::getCityset();
  for (std::list<Glib::ustring>::iterator i = city_themes.begin(),
       end = city_themes.end(); i != end; ++i)
    {
      if (*i == active->getName())
        default_id = counter;
      //only append it if the tile widths are identical.
      //Cityset *cityset =
      //cl->getCityset(cl->getCitysetDir(*i, get_active_tile_size()));
      //if (active->tileWidthsEqual(cityset) == true)
      city_theme_combobox->append(Glib::filename_to_utf8(*i));
      counter++;
    }

  city_theme_combobox->set_active(default_id);
  empty = city_theme_combobox->get_model()->children().empty ();
}

void SwitchSetsDialog::fill_army_themes (bool &empty)
{
  Armysetlist *al = Armysetlist::getInstance();
  std::list<Glib::ustring> army_themes =
    al->getValidNames(get_active_tile_size());

  guint32 row = 0;
  for (auto p : *Playerlist::getInstance ())
    {
      Gtk::ComboBoxText *c = army_theme_comboboxes[row];
      c->remove_all();
      guint32 default_id = 0;
      guint32 counter = 0;
      for (std::list<Glib::ustring>::iterator i = army_themes.begin(),
           end = army_themes.end(); i != end; ++i, ++counter)
        {
          c->append(Glib::filename_to_utf8(*i));
          if (*i == al->get(p->getArmyset())->getName())
            default_id = counter;
        }
      c->set_active(default_id);
      if (c->get_model()->children().empty ())
        empty = true;
      row++;
    }
}

void SwitchSetsDialog::on_tile_size_changed()
{
  bool empty = false;
  disconnect_signals ();

  fill_tile_themes (empty);
  if (empty)
    accept_button->set_sensitive(false);

  fill_army_themes (empty);
  if (empty)
    accept_button->set_sensitive(false);

  fill_city_themes (empty);
  if (empty)
    accept_button->set_sensitive(false);

  connect_signals ();
}

int SwitchSetsDialog::run()
{
  dialog->show_all();
  return dialog->run();
}

void SwitchSetsDialog::on_armyset_changed (Gtk::ComboBoxText *c, Player *p)
{
  Glib::ustring subdir =
    Armysetlist::getInstance()->getSetDir
    (Glib::filename_from_utf8(c->get_active_text()),
     get_active_tile_size());
  Armyset *armyset = Armysetlist::getInstance()->get(subdir);
  if (p->getArmyset() != armyset->getId())
    {
      armyset_changed = true;
      GameMap::getInstance()->switchArmysets(p, armyset);
    }
}

void SwitchSetsDialog::on_shieldset_changed ()
{
  Glib::ustring subdir = Shieldsetlist::getInstance()->getSetDir
    (Glib::filename_from_utf8(shield_theme_combobox->get_active_text()));
  selected_shieldset = Shieldsetlist::getInstance()->get(subdir);
  if (selected_shieldset->getBaseName() != GameMap::getShieldset()->getBaseName())
    {
      shieldset_changed = true;
      GameMap::getInstance()->switchShieldset(selected_shieldset);
    }
}

void SwitchSetsDialog::on_cityset_changed ()
{
  Glib::ustring subdir = Citysetlist::getInstance()->getSetDir
    (Glib::filename_from_utf8(city_theme_combobox->get_active_text()),
     get_active_tile_size());
  selected_cityset = Citysetlist::getInstance()->get(subdir);

  if (selected_cityset->getBaseName() != GameMap::getCityset()->getBaseName())
    {
      cityset_changed = true;
      GameMap::getInstance()->switchCityset(selected_cityset);
    }
}

void SwitchSetsDialog::on_tileset_changed ()
{
  Glib::ustring subdir;
  subdir = Tilesetlist::getInstance()->getSetDir
    (Glib::filename_from_utf8(tile_theme_combobox->get_active_text()),
     get_active_tile_size());
  selected_tileset = Tilesetlist::getInstance()->get(subdir);
  if (selected_tileset->getBaseName() != GameMap::getTileset()->getBaseName())
    {
      tileset_changed = true;
      GameMap::getInstance()->switchTileset(selected_tileset);
    }
}

void SwitchSetsDialog::on_make_same_activated ()
{
  armyset_changed = true;
  guint32 id = Playerlist::getInstance ()->getNeutral ()->getArmyset ();
  for (auto p : *Playerlist::getInstance ())
    p->setArmyset (id);
  on_tile_size_changed ();
}

void SwitchSetsDialog::disconnect_signals ()
{
  for (auto c : connections)
    c.disconnect ();
}

void SwitchSetsDialog::connect_signals ()
{
  connections.push_back (make_same_button->signal_clicked ().connect
                         (method (on_make_same_activated)));
  connections.push_back (shield_theme_combobox->signal_changed ().connect
                         (method (on_shieldset_changed)));
  connections.push_back (tile_size_combobox->signal_changed().connect
                         (sigc::mem_fun(*this, &SwitchSetsDialog::on_tile_size_changed)));
  connections.push_back (tile_theme_combobox->signal_changed().connect
                         (method (on_tileset_changed)));
  connections.push_back (city_theme_combobox->signal_changed().connect
                         (method (on_cityset_changed)));

  Playerlist::iterator i = Playerlist::getInstance ()->begin ();
  for (auto c : army_theme_comboboxes)
    {
      connections.push_back (c->signal_changed().connect
                             (sigc::bind (method (on_armyset_changed),
                                          c, *i)));
      ++i;
    }
}
