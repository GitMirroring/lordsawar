//  Copyright (C) 2007, 2008, 2009, 2010, 2014, 2020, 2021 Ben Asselstine
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

#include "tileset-info-dialog.h"
#include "tilesetlist.h"

#include "ucompose.hpp"
#include "defs.h"
#include "File.h"
#include "tileset-info-actions.h"

#define method(x) sigc::mem_fun(*this, &TileSetInfoDialog::x)

TileSetInfoDialog::TileSetInfoDialog(Gtk::Window &parent, Tileset *s)
 : LwEditorDialog(parent, "tileset-info-dialog.ui")
{
  umgr = new UndoMgr (UndoMgr::DELAY, UndoMgr::LIMIT);
  umgr->execute ().connect (method (executeAction));
  d_tileset = s;
  dialog->set_title(_("Tile Set Properties"));

  xml->get_widget("close_button", close_button);
  xml->get_widget("status_label", status_label);
  xml->get_widget("location_label", location_label);
  xml->get_widget("name_entry", name_entry);
  xml->get_widget("size_spinbutton", size_spinbutton);
  xml->get_widget("fit_button", fit_button);
  xml->get_widget("undo_button", undo_button);
  undo_button->signal_activate ().connect (method (on_undo_activated));
  xml->get_widget("redo_button", redo_button);
  redo_button->signal_activate ().connect (method (on_redo_activated));

  name_entry->set_text (d_tileset->getName ());
  location_label->property_label () =
    d_tileset->getDirectory ().empty () ? "" :
    d_tileset->getConfigurationFile (true);

  xml->get_widget("copyright_textview", copyright_textview);
  xml->get_widget("license_textview", license_textview);
  xml->get_widget("description_textview", description_textview);
  xml->get_widget("notebook", notebook);
  d_name = d_tileset->getName ();
  d_description = d_tileset->getInfo ();
  d_copyright = d_tileset->getCopyright ();
  d_license = d_tileset->getLicense ();
  d_tilesize = d_tileset->getTileSize ();
  d_orig_name = d_name;
  d_orig_description = d_description;
  d_orig_copyright = d_copyright;
  d_orig_license = d_license;
  d_orig_tilesize = d_tilesize;
  d_changed = false;
  connect_signals ();
  update ();
  update_name ();
}

void TileSetInfoDialog::update_name ()
{
  Glib::ustring oldname = d_tileset->getName ();
  guint32 oldsize = d_tileset->getTileSize ();
  d_tileset->setName (String::utrim (name_entry->get_text ()));
  d_tileset->setTileSize (d_tilesize);
  close_button->set_sensitive (File::sanify (d_tileset->getName ()) != "");

  Glib::ustring file =
    Tilesetlist::getInstance()->lookupConfigurationFileByName(d_tileset);
  if (file != "" && file != d_tileset->getConfigurationFile (true))
    status_label->set_text (_("That name is already in use."));
  else
    status_label->set_text ("");
  d_tileset->setName (oldname);
  d_tileset->setTileSize (oldsize);
  d_name = String::utrim (name_entry->get_text ());
}

void TileSetInfoDialog::on_name_changed()
{
  umgr->add (new TileSetInfoAction_Name (d_name, name_entry->get_position ()));
  d_changed = true;
  update_name ();
}

bool TileSetInfoDialog::run()
{
  dialog->run();
  dialog->hide ();
  if (d_orig_description == d_description &&
      d_orig_copyright == d_copyright &&
      d_orig_license == d_license &&
      d_orig_name == d_name &&
      d_orig_tilesize == d_tilesize)
    return false;
  return d_changed;
}

void TileSetInfoDialog::on_copyright_changed ()
{
  umgr->add (new TileSetInfoAction_Copyright (d_copyright));
  d_changed = true;
  d_copyright = copyright_textview->get_buffer()->get_text();
}

void TileSetInfoDialog::on_license_changed ()
{
  umgr->add (new TileSetInfoAction_License (d_license));
  d_changed = true;
  d_license = license_textview->get_buffer()->get_text();
}

void TileSetInfoDialog::on_description_changed ()
{
  umgr->add (new TileSetInfoAction_Description (d_description));
  d_changed = true;
  d_description = description_textview->get_buffer()->get_text();
}

TileSetInfoDialog::~TileSetInfoDialog()
{
  delete umgr;
  notebook->property_show_tabs () = false;
}

void TileSetInfoDialog::on_size_changed()
{
  umgr->add (new TileSetInfoAction_TileSize (d_tilesize));
  d_changed = true;
  d_tilesize = size_spinbutton->get_value ();
  update_name ();
}

void TileSetInfoDialog::on_fit_pressed()
{
  umgr->add (new TileSetInfoAction_TileSize (d_tilesize));
  d_changed = true;
  guint32 ts = 0;
  d_tileset->calculate_preferred_tile_size (ts);
  size_spinbutton->set_value (ts);
  update_name ();
}

UndoAction* TileSetInfoDialog::executeAction (UndoAction *action2)
{
  TileSetInfoAction *action = dynamic_cast<TileSetInfoAction*>(action2);
  UndoAction *out = NULL;

    switch (action->getType ())
      {
      case TileSetInfoAction::DESCRIPTION:
          {
            TileSetInfoAction_Description *a =
              dynamic_cast<TileSetInfoAction_Description*>(action);
            out = new TileSetInfoAction_Description (d_description);
            d_description = a->getMessage ();
          } 
        break;
      case TileSetInfoAction::COPYRIGHT:
          {
            TileSetInfoAction_Copyright *a =
              dynamic_cast<TileSetInfoAction_Copyright*>(action);
            out = new TileSetInfoAction_Copyright (d_copyright);
            d_copyright = a->getMessage ();
          } 
        break;
      case TileSetInfoAction::LICENSE:
          {
            TileSetInfoAction_License *a =
              dynamic_cast<TileSetInfoAction_License*>(action);
            out = new TileSetInfoAction_License (d_license);
            d_license = a->getMessage ();
          } 
        break;
      case TileSetInfoAction::NAME:
          {
            TileSetInfoAction_Name *a = dynamic_cast<TileSetInfoAction_Name*>(action);
            out = new TileSetInfoAction_Name
              (d_name, name_entry->get_position ());
            d_name = a->getName ();
            disconnect_signals ();
            name_entry->set_text (d_name);
            name_entry->set_position (a->getCursorPosition ());
            connect_signals ();
          }
        break;
      case TileSetInfoAction::TILE_SIZE:
          {
            TileSetInfoAction_TileSize *a =
              dynamic_cast<TileSetInfoAction_TileSize*>(action);
            out = new TileSetInfoAction_TileSize (d_tilesize);
            d_tilesize = a->getTileSize ();
            update_name ();
          } 
        break;
      }
    return out;
}

void TileSetInfoDialog::on_undo_activated ()
{
  umgr->undo ();
  if (umgr->undoEmpty ())
    d_changed = false;
  update ();
}

void TileSetInfoDialog::on_redo_activated ()
{
  d_changed = true;
  umgr->redo ();
  update ();
}

void TileSetInfoDialog::update ()
{
  disconnect_signals ();
  description_textview->get_buffer()->set_text(d_description);
  copyright_textview->get_buffer()->set_text(d_copyright);
  license_textview->get_buffer()->set_text(d_license);
  if (name_entry->get_text () != d_name)
    name_entry->set_text (d_name);
  size_spinbutton->set_value (d_tilesize);
  connect_signals ();
}

void TileSetInfoDialog::connect_signals ()
{
  connections.push_back
    (description_textview->get_buffer()->signal_changed().connect
     (method(on_description_changed)));
  connections.push_back
    (copyright_textview->get_buffer()->signal_changed().connect
     (method(on_copyright_changed)));
  connections.push_back
    (license_textview->get_buffer()->signal_changed().connect
     (method(on_license_changed)));
  connections.push_back
    (name_entry->signal_changed().connect (method(on_name_changed)));
  connections.push_back
    (size_spinbutton->signal_changed().connect (method(on_size_changed)));
  connections.push_back
    (fit_button->signal_clicked().connect (method(on_fit_pressed)));
}

void TileSetInfoDialog::disconnect_signals ()
{
  for (auto c : connections)
    c.disconnect ();
  connections.clear ();
}
