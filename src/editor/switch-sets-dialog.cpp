//  Copyright (C) 2009, 2010, 2012, 2014, 2020, 2021 Ben Asselstine
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
#include "switch-sets-editor-actions.h"

#define method(x) sigc::mem_fun(*this, &SwitchSetsDialog::x)

SwitchSetsDialog::SwitchSetsDialog(Gtk::Window &parent)
 :LwEditorDialog(parent, "switch-sets-dialog.ui")
{
  umgr = new UndoMgr (UndoMgr::DELAY, UndoMgr::LIMIT);
  umgr->execute ().connect (method (executeAction));
  xml->get_widget("accept_button", accept_button);
  xml->get_widget("make_same_button", make_same_button);
  xml->get_widget("undo_button", undo_button);
  undo_button->signal_activate ().connect (method (on_undo_activated));
  xml->get_widget("redo_button", redo_button);
  redo_button->signal_activate ().connect (method (on_redo_activated));
    
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
  tilesize_row = default_id;
  xml->get_widget("tile_size_box", box);
  box->pack_start(*tile_size_combobox, Gtk::PACK_SHRINK);

  // make new tile themes combobox
  tile_theme_combobox = manage(new Gtk::ComboBox);
  xml->get_widget("tile_theme_box", box);
  box->pack_start(*tile_theme_combobox, Gtk::PACK_SHRINK);
  tilesets_list = Gtk::ListStore::create(tilesets_columns);
  tile_theme_combobox->set_model(tilesets_list);
  tile_theme_combobox->pack_start (tilesets_columns.name);
  bool empty = false;
  fill_tile_themes (empty);
  set_default_tileset_row ();

  // make new city themes combobox
  city_theme_combobox = manage(new Gtk::ComboBox);
  xml->get_widget("city_theme_box", box);
  box->pack_start(*city_theme_combobox, Gtk::PACK_SHRINK);
  citysets_list = Gtk::ListStore::create(citysets_columns);
  city_theme_combobox->set_model(citysets_list);
  city_theme_combobox->pack_start (citysets_columns.name);
  fill_city_themes (empty);
  set_default_cityset_row ();

  // make new shield themes combobox
  shield_theme_combobox = manage(new Gtk::ComboBox);
  xml->get_widget("shield_theme_box", box);
  box->pack_start(*shield_theme_combobox, Gtk::PACK_SHRINK);
  shieldsets_list = Gtk::ListStore::create(shieldsets_columns);
  shield_theme_combobox->set_model(shieldsets_list);
  shield_theme_combobox->pack_start (shieldsets_columns.name);
  fill_shield_themes (empty);
  set_default_shieldset_row ();

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
      armysets_rows.push_back (-1);
      row++;
      army_theme_comboboxes.push_back (c);
      armysets[c] = Armysetlist::getInstance ()->get (p->getArmyset ());
    }
  fill_army_themes (empty);
  set_default_armyset_rows ();

  tileset_changed = false;
  armyset_changed = false;
  cityset_changed = false;
  shieldset_changed = false;
  connect_signals ();
  update ();
}

SwitchSetsDialog::~SwitchSetsDialog()
{
  delete umgr;
}

void SwitchSetsDialog::set_default_armyset_rows ()
{
  Armysetlist *al = Armysetlist::getInstance();
  std::list<Glib::ustring> army_themes =
    al->getValidNames(get_active_tile_size());

  guint32 row = 0;
  for (auto p : *Playerlist::getInstance ())
    {
      guint32 default_id = 0;
      guint32 counter = 0;
      for (std::list<Glib::ustring>::iterator i = army_themes.begin(),
           end = army_themes.end(); i != end; ++i, ++counter)
        {
          if (*i == al->get(p->getArmyset())->getName())
            default_id = counter;
        }
      armysets_rows[p->getId ()] = default_id;
      row++;
    }
}

guint32 SwitchSetsDialog::get_active_tile_size()
{
  return (guint32) atoi(tile_size_combobox->get_active_text().c_str());
}

void SwitchSetsDialog::set_default_shieldset_row ()
{
  shieldset_row = -1;
  int counter = 0;
  for (auto i : shieldsets_list->children ())
    {
      Gtk::TreeModel::Row row = *i;
      if (row[shieldsets_columns.shieldset] == GameMap::getShieldset ())
        {
          shieldset_row = counter;
          break;
        }
      counter++;
    }
}
void SwitchSetsDialog::set_default_tileset_row ()
{
  tileset_row = -1;
  int counter = 0;
  for (auto i : tilesets_list->children ())
    {
      Gtk::TreeModel::Row row = *i;
      if (row[tilesets_columns.tileset] == GameMap::getTileset ())
        {
          tileset_row = counter;
          break;
        }
      counter++;
    }
}
void SwitchSetsDialog::set_default_cityset_row ()
{
  cityset_row = -1;
  int counter = 0;
  for (auto i : citysets_list->children ())
    {
      Gtk::TreeModel::Row row = *i;
      if (row[citysets_columns.cityset] == GameMap::getCityset ())
        {
          cityset_row = counter;
          break;
        }
      counter++;
    }
}

void SwitchSetsDialog::fill_shield_themes (bool &empty)
{
  shieldsets_list->clear ();
  Shieldsetlist *sl = Shieldsetlist::getInstance();
  std::list<Glib::ustring> shield_themes = sl->getValidNames();
  for (std::list<Glib::ustring>::iterator i = shield_themes.begin(),
       end = shield_themes.end(); i != end; ++i)
    {
      Gtk::TreeIter j = shieldsets_list->append();
      (*j)[shieldsets_columns.name] = *i;
      (*j)[shieldsets_columns.shieldset] =
        Shieldsetlist::getInstance ()->get (*i, 0);
    }

  empty = shield_theme_combobox->get_model()->children().empty ();
}

void SwitchSetsDialog::fill_tile_themes (bool &empty)
{
  tilesets_list->clear ();
  Tilesetlist *tl = Tilesetlist::getInstance();
  std::list<Glib::ustring> tile_themes =
    tl->getValidNames(get_active_tile_size ());
  for (std::list<Glib::ustring>::iterator i = tile_themes.begin(),
       end = tile_themes.end(); i != end; ++i)
    {
      Gtk::TreeIter j = tilesets_list->append();
      (*j)[tilesets_columns.name] = *i;
      (*j)[tilesets_columns.tileset] =
        Tilesetlist::getInstance ()->get (*i, get_active_tile_size ());
    }

  empty = tile_theme_combobox->get_model()->children().empty ();
}

void SwitchSetsDialog::fill_city_themes (bool &empty)
{
  citysets_list->clear ();
  Citysetlist *cl = Citysetlist::getInstance();
  std::list<Glib::ustring> city_themes =
    cl->getValidNames(get_active_tile_size ());
  for (std::list<Glib::ustring>::iterator i = city_themes.begin(),
       end = city_themes.end(); i != end; ++i)
    {
      Gtk::TreeIter j = citysets_list->append();
      (*j)[citysets_columns.name] = *i;
      (*j)[citysets_columns.cityset] =
        Citysetlist::getInstance ()->get (*i, get_active_tile_size ());
    }

  empty = city_theme_combobox->get_model()->children().empty ();
}

void SwitchSetsDialog::fill_army_themes (bool &empty)
{
  Armysetlist *al = Armysetlist::getInstance();
  std::list<Glib::ustring> army_themes =
    al->getValidNames(get_active_tile_size());

  guint32 row = 0;
  for (guint32 j = 0; j < Playerlist::getInstance ()->size (); j++)
    {
      Gtk::ComboBoxText *c = army_theme_comboboxes[row];
      c->remove_all();
      guint32 counter = 0;
      for (std::list<Glib::ustring>::iterator i = army_themes.begin(),
           end = army_themes.end(); i != end; ++i, ++counter)
        c->append(Glib::filename_to_utf8(*i));
      if (c->get_model()->children().empty ())
        empty = true;
      row++;
    }
}

void SwitchSetsDialog::on_tile_size_changed()
{
  umgr->add (new SwitchSetsEditorAction_TileSize (tilesize_row,
                                                  tileset_row,
                                                  cityset_row,
                                                  shieldset_row,
                                                  get_current_armyset_rows ()));
  tilesize_row = tile_size_combobox->get_active_row_number ();
  update ();
}

int SwitchSetsDialog::run()
{
  dialog->show_all();
  return dialog->run();
}

void SwitchSetsDialog::on_armyset_changed (Gtk::ComboBoxText *c, Player *p)
{
  SwitchSetsEditorAction_Armyset *action =
    new SwitchSetsEditorAction_Armyset (get_current_armyset_rows ());
  Glib::ustring subdir =
    Armysetlist::getInstance()->getSetDir
    (Glib::filename_from_utf8(c->get_active_text()),
     get_active_tile_size());
  Armyset *armyset = Armysetlist::getInstance()->get(subdir);
  if (p->getArmyset() != armyset->getId())
    {
      armysets_rows[p->getId ()] = c->get_active_row_number ();
      umgr->add (action);
      armyset_changed = true;
      GameMap::getInstance()->switchArmysets(p, armyset);
    }
  else
    delete action;
}

void SwitchSetsDialog::on_shieldset_changed ()
{
  SwitchSetsEditorAction_Shieldset *action =
    new SwitchSetsEditorAction_Shieldset (shieldset_row);
  shieldset_row = shield_theme_combobox->get_active_row_number ();
  auto it = shield_theme_combobox->get_active (); 
  Gtk::TreeModel::Row row = *it;
  Shieldset *shieldset = row[shieldsets_columns.shieldset];
  if (shieldset->getBaseName() != GameMap::getShieldset()->getBaseName())
    {
      umgr->add (action);
      shieldset_changed = true;
      GameMap::getInstance()->switchShieldset (shieldset);
    }
  else
    delete action;
}

void SwitchSetsDialog::on_cityset_changed ()
{
  SwitchSetsEditorAction_Cityset *action =
    new SwitchSetsEditorAction_Cityset (cityset_row);
  cityset_row = city_theme_combobox->get_active_row_number ();
  auto it = city_theme_combobox->get_active (); 
  Gtk::TreeModel::Row row = *it;
  Cityset *cityset = row[citysets_columns.cityset];
  if (cityset->getBaseName() != GameMap::getCityset()->getBaseName())
    {
      umgr->add (action);
      cityset_changed = true;
      GameMap::getInstance()->switchCityset (cityset);
    }
  else
    delete action;
}

void SwitchSetsDialog::on_tileset_changed ()
{
  SwitchSetsEditorAction_Tileset *action =
    new SwitchSetsEditorAction_Tileset (tileset_row);
  tileset_row = tile_theme_combobox->get_active_row_number ();
  auto it = tile_theme_combobox->get_active (); 
  Gtk::TreeModel::Row row = *it;
  Tileset *tileset = row[tilesets_columns.tileset];
  if (tileset->getBaseName() != GameMap::getTileset()->getBaseName())
    {
      umgr->add (action);
      tileset_changed = true;
      GameMap::getInstance()->switchTileset (tileset);
    }
  else
    delete action;
}

void SwitchSetsDialog::on_make_same_activated ()
{
  SwitchSetsEditorAction_MakeSame *action =
    new SwitchSetsEditorAction_MakeSame (get_current_armyset_rows ());
  bool modified = false;
  guint32 id = Playerlist::getInstance ()->getNeutral ()->getArmyset ();
  Armyset *armyset = Armysetlist::getInstance ()->get (id);
  for (auto p : *Playerlist::getInstance ())
    {
      if (p == Playerlist::getInstance ()->getNeutral ())
        continue;
      if (p->getArmyset () != armyset->getId ())
        {
          armysets_rows[p->getId ()] =
            armysets_rows[Playerlist::getInstance ()->getNeutral ()->getId ()];
          modified = true;
          GameMap::getInstance()->switchArmysets(p, armyset);
        }
    }
  if (modified)
    {
      umgr->add (action);
      armyset_changed = true;
    }
  else
    delete action;
  update ();
}

void SwitchSetsDialog::disconnect_signals ()
{
  umgr->disconnect_signals ();
  for (auto c : connections)
    c.disconnect ();
  connections.clear ();
}

void SwitchSetsDialog::connect_signals ()
{
  umgr->connect_signals ();
  connections.push_back (make_same_button->signal_clicked ().connect
                         (method (on_make_same_activated)));
  connections.push_back (shield_theme_combobox->signal_changed ().connect
                         (method (on_shieldset_changed)));
  connections.push_back
    (tile_size_combobox->signal_changed().connect
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

void SwitchSetsDialog::on_undo_activated ()
{
  umgr->undo ();
  if (umgr->undoEmpty ())
    {
      armyset_changed = false;
      tileset_changed = false;
      cityset_changed = false;
      shieldset_changed = false;
    }
  update ();
}

void SwitchSetsDialog::on_redo_activated ()
{
  umgr->redo ();

  if (umgr->getUndoName () == "ArmySet")
    armyset_changed = true;
  else if (umgr->getUndoName () == "ShieldSet")
    shieldset_changed = true;
  else if (umgr->getUndoName () == "CitySet")
    cityset_changed = true;
  else if (umgr->getUndoName () == "TileSet")
    tileset_changed = true;
  update ();
}

void SwitchSetsDialog::update ()
{
  disconnect_signals ();
  bool empty = false;
  fill_tile_themes (empty);
  if (empty)
    accept_button->set_sensitive (false);
  if (tileset_row > -1)
    tile_theme_combobox->set_active (tileset_row);

  empty = false;
  fill_army_themes (empty);
  if (empty)
    accept_button->set_sensitive (false);
  int i = 0;
  for (auto a : armysets_rows)
    {
      if (a > -1)
        {
          Gtk::ComboBoxText *c = army_theme_comboboxes[i];
          c->set_active (a);
        }
      i++;
    }

  empty = false;
  fill_city_themes (empty);
  if (empty)
    accept_button->set_sensitive (false);
  if (cityset_row > -1)
    city_theme_combobox->set_active (cityset_row);

  empty = false;
  fill_shield_themes (empty);
  if (empty)
    accept_button->set_sensitive (false);
  if (shieldset_row > -1)
    shield_theme_combobox->set_active (shieldset_row);

  tile_size_combobox->set_active (tilesize_row);
  connect_signals ();
}

std::map<guint32, int> SwitchSetsDialog::get_current_armyset_rows ()
{
  std::map<guint32, int> rows;
  int c = 0;
  for (auto p : *Playerlist::getInstance ())
    {
      rows[p->getId ()] = armysets_rows[c];
      c++;
    }
  return rows;
}

UndoAction *SwitchSetsDialog::executeAction (UndoAction *action2)
{
  SwitchSetsEditorAction *action = dynamic_cast<SwitchSetsEditorAction*>(action2);
  UndoAction *out = NULL;

  switch (action->getType ())
    {
      case SwitchSetsEditorAction::ARMYSET:
          {
            SwitchSetsEditorAction_Armyset *a =
              dynamic_cast<SwitchSetsEditorAction_Armyset*>(action);
            out = new SwitchSetsEditorAction_Armyset
              (get_current_armyset_rows ());

            for (auto s : a->getArmysetRows ())
              {
                guint32 player_id = s.first;
                Player *p = Playerlist::getInstance ()->getPlayer (player_id);
                if (p)
                  {
                    int armyset_row = s.second;
                    armysets_rows[player_id] = armyset_row;
                    if (armyset_row > -1)
                      {
                        Gtk::ComboBoxText *c = army_theme_comboboxes[player_id];
                        Glib::ustring subdir =
                          Armysetlist::getInstance()->getSetDir
                          (Glib::filename_from_utf8(c->get_active_text()),
                           get_active_tile_size());
                        Armyset *armyset = Armysetlist::getInstance()->get(subdir);
                        if (p->getArmyset() != armyset->getId())
                          GameMap::getInstance()->switchArmysets(p, armyset);
                      }
                  }
              }
          }
        break;
      case SwitchSetsEditorAction::MAKE_SAME:
          {
            SwitchSetsEditorAction_MakeSame *a =
              dynamic_cast<SwitchSetsEditorAction_MakeSame*>(action);
            out = new SwitchSetsEditorAction_MakeSame
              (get_current_armyset_rows ());

            for (auto s : a->getArmysetRows ())
              {
                guint32 player_id = s.first;
                Player *p = Playerlist::getInstance ()->getPlayer (player_id);
                if (p)
                  {
                    int armyset_row = s.second;
                    armysets_rows[player_id] = armyset_row;
                    if (armyset_row > -1)
                      {
                        Gtk::ComboBoxText *c = army_theme_comboboxes[player_id];
                        Glib::ustring subdir =
                          Armysetlist::getInstance()->getSetDir
                          (Glib::filename_from_utf8(c->get_active_text()),
                           get_active_tile_size());
                        Armyset *armyset = Armysetlist::getInstance()->get(subdir);
                        if (p->getArmyset() != armyset->getId())
                          GameMap::getInstance()->switchArmysets(p, armyset);
                      }
                  }
              }
          }
        break;
      case SwitchSetsEditorAction::TILESET:
          {
            SwitchSetsEditorAction_Tileset *a =
              dynamic_cast<SwitchSetsEditorAction_Tileset*>(action);
            out = new SwitchSetsEditorAction_Tileset (tileset_row);
            tileset_row = a->getRow ();
            auto it =
              tile_theme_combobox->get_model ()->children()[tileset_row];
            Gtk::TreeModel::Row row = *it;
            Tileset *ts = row[tilesets_columns.tileset];
            GameMap::getInstance()->switchTileset (ts);
          }
        break;
      case SwitchSetsEditorAction::SHIELDSET:
          {
            SwitchSetsEditorAction_Shieldset *a =
              dynamic_cast<SwitchSetsEditorAction_Shieldset*>(action);
            out = new SwitchSetsEditorAction_Shieldset (shieldset_row);
            shieldset_row = a->getRow ();
            auto it =
              shield_theme_combobox->get_model ()->children()[tileset_row];
            Gtk::TreeModel::Row row = *it;
            Shieldset *ss = row[shieldsets_columns.shieldset];
            GameMap::getInstance()->switchShieldset (ss);
          }
        break;
      case SwitchSetsEditorAction::CITYSET:
          {
            SwitchSetsEditorAction_Cityset *a =
              dynamic_cast<SwitchSetsEditorAction_Cityset*>(action);
            out = new SwitchSetsEditorAction_Cityset (cityset_row);
            cityset_row = a->getRow ();
            auto it =
              city_theme_combobox->get_model ()->children()[cityset_row];
            Gtk::TreeModel::Row row = *it;
            Cityset *cs = row[citysets_columns.cityset];
            GameMap::getInstance()->switchCityset (cs);
          }
        break;
      case SwitchSetsEditorAction::TILE_SIZE:
          {
            SwitchSetsEditorAction_TileSize *a =
              dynamic_cast<SwitchSetsEditorAction_TileSize*>(action);
            out = new SwitchSetsEditorAction_TileSize
              (tilesize_row, tileset_row, cityset_row, shieldset_row,
               get_current_armyset_rows ());
            tilesize_row = a->getRow ();

            tileset_row = a->getTilesetRow ();
            if (tileset_row > -1)
              {
                auto it =
                  tile_theme_combobox->get_model ()->children()[tileset_row];
                Gtk::TreeModel::Row row = *it;
                Tileset *ts = row[tilesets_columns.tileset];
                if (ts != GameMap::getTileset ())
                  GameMap::getInstance()->switchTileset (ts);
              }
            cityset_row = a->getCitysetRow ();
            if (cityset_row > -1)
              {
                auto it =
                  city_theme_combobox->get_model ()->children()[cityset_row];
                Gtk::TreeModel::Row row = *it;
                Cityset *cs = row[citysets_columns.cityset];
                if (cs != GameMap::getCityset ())
                  GameMap::getInstance()->switchCityset (cs);
              }
            shieldset_row = a->getShieldsetRow ();
            if (shieldset_row > -1)
              {
                auto it =
                  shield_theme_combobox->get_model()->children()[shieldset_row];
                Gtk::TreeModel::Row row = *it;
                Shieldset *ss = row[shieldsets_columns.shieldset];
                if (ss != GameMap::getShieldset ())
                  GameMap::getInstance()->switchShieldset (ss);
              }

            for (auto s : a->getArmysetRows ())
              {
                guint32 player_id = s.first;
                Player *p = Playerlist::getInstance ()->getPlayer (player_id);
                if (p)
                  {
                    int armyset_row = s.second;
                    armysets_rows[player_id] = armyset_row;
                    if (armyset_row > -1)
                      {
                        Gtk::ComboBoxText *c = army_theme_comboboxes[player_id];
                        Glib::ustring subdir =
                          Armysetlist::getInstance()->getSetDir
                          (Glib::filename_from_utf8(c->get_active_text()),
                           get_active_tile_size());
                        Armyset *armyset = Armysetlist::getInstance()->get(subdir);
                        if (p->getArmyset() != armyset->getId())
                          GameMap::getInstance()->switchArmysets(p, armyset);
                      }
                  }
              }
          }
        break;
    }
  return out;
}
