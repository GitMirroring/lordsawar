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

#include "armyset-info-dialog.h"
#include "armysetlist.h"

#include "ucompose.hpp"
#include "defs.h"
#include "File.h"
#include "armyset-info-actions.h"
#include "TarFileMaskedImage.h"
#include "TarFileImage.h"

#define method(x) sigc::mem_fun(*this, &ArmySetInfoDialog::x)

ArmySetInfoDialog::ArmySetInfoDialog(Gtk::Window &parent, Armyset *armyset)
 : LwEditorDialog(parent, "armyset-info-dialog.ui")
{
  umgr = new UndoMgr (UndoMgr::DELAY, UndoMgr::LIMIT);
  umgr->execute ().connect (method (executeAction));
  d_armyset = armyset;
  dialog->set_title(_("Army Set Properties"));

  xml->get_widget("close_button", close_button);
  xml->get_widget("status_label", status_label);
  xml->get_widget("location_label", location_label);
  location_label->property_label () =
    d_armyset->getDirectory ().empty () ? "" :
    d_armyset->getConfigurationFile (true);

  xml->get_widget("name_entry", name_entry);
  umgr->addCursor (name_entry);

  xml->get_widget("copyright_textview", copyright_textview);
  umgr->addCursor (copyright_textview);
  xml->get_widget("license_textview", license_textview);
  umgr->addCursor (license_textview);
  xml->get_widget("description_textview", description_textview);
  umgr->addCursor (description_textview);
  xml->get_widget("notebook", notebook);
  xml->get_widget("size_spinbutton", size_spinbutton);
  xml->get_widget("fit_button", fit_button);
  xml->get_widget("undo_button", undo_button);
  undo_button->signal_activate ().connect (method (on_undo_activated));
  xml->get_widget("redo_button", redo_button);
  redo_button->signal_activate ().connect (method (on_redo_activated));
  xml->get_widget("armies_label", armies_label);
  armies_label->set_text (String::ucompose ("%1", armyset->size ()));
  xml->get_widget("selectors_label", selectors_label);
  selectors_label->set_text
    (String::ucompose ("%1", armyset->countSelectors ()));
  xml->get_widget("bag_label", bag_label);
  bag_label->set_text (armyset->getBag ()->getName () == "" ?
                       _("Not present") : _("Present"));
  xml->get_widget("ship_label", ship_label);
  ship_label->set_text (armyset->getShip ()->getName () == "" ?
                        _("Not present") : _("Present"));
  xml->get_widget("flag_label", flag_label);
  flag_label->set_text (armyset->getStandard ()->getName () == "" ?
                        _("Not present") : _("Present"));
  xml->get_widget("images_label", images_label);
  images_label->set_text (String::ucompose ("%1", armyset->countImages ()));

  d_name = d_armyset->getName ();
  d_description = d_armyset->getInfo ();
  d_copyright = d_armyset->getCopyright ();
  d_license = d_armyset->getLicense ();
  d_tilesize = d_armyset->getTileSize ();
  d_orig_description = d_description;
  d_orig_copyright = d_copyright;
  d_orig_license = d_license;
  d_orig_tilesize = d_tilesize;
  connect_signals ();
  update ();
  update_name ();
  d_changed = false;
}

void ArmySetInfoDialog::update_name ()
{
  Glib::ustring oldname = d_armyset->getName ();
  guint32 oldsize = d_armyset->getTileSize ();
  d_armyset->setName (String::utrim (name_entry->get_text ()));
  d_armyset->setTileSize (d_tilesize);
  close_button->set_sensitive (File::sanify (d_armyset->getName ()) != "");

  Glib::ustring file =
    Armysetlist::getInstance()->lookupConfigurationFileByName(d_armyset);
  if (file != "" && file != d_armyset->getConfigurationFile (true))
    status_label->set_text (_("That name is already in use."));
  else
    status_label->set_text ("");
  d_armyset->setName (oldname);
  d_armyset->setTileSize (oldsize);
  d_name = String::utrim (name_entry->get_text ());
}

void ArmySetInfoDialog::on_name_changed()
{
  auto a = new ArmySetInfoAction_Name (d_name, umgr, name_entry);
  umgr->add (a);
  d_changed = true;
  update_name ();
}

bool ArmySetInfoDialog::run()
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

void ArmySetInfoDialog::on_copyright_changed ()
{
  umgr->add (new ArmySetInfoAction_Copyright (d_copyright, umgr,
                                              copyright_textview));
  d_changed = true;
  d_copyright = copyright_textview->get_buffer()->get_text();
}

void ArmySetInfoDialog::on_license_changed ()
{
  umgr->add (new ArmySetInfoAction_License (d_license, umgr, license_textview));
  d_changed = true;
  d_license = license_textview->get_buffer()->get_text();
}

void ArmySetInfoDialog::on_description_changed ()
{
  umgr->add (new ArmySetInfoAction_Description (d_description, umgr,
                                                description_textview));
  d_changed = true;
  d_description = description_textview->get_buffer()->get_text();
}

ArmySetInfoDialog::~ArmySetInfoDialog()
{
  delete umgr;
  notebook->property_show_tabs () = false;
}

void ArmySetInfoDialog::on_size_changed()
{
  umgr->add (new ArmySetInfoAction_TileSize (d_tilesize));
  d_changed = true;
  d_tilesize = size_spinbutton->get_value ();
  update_name ();
}

void ArmySetInfoDialog::on_fit_pressed()
{
  umgr->add (new ArmySetInfoAction_TileSize (d_tilesize));
  d_changed = true;
  guint32 ts = 0;
  d_armyset->calculate_preferred_tile_size (ts);
  size_spinbutton->set_value (ts);
  update_name ();
}

UndoAction* ArmySetInfoDialog::executeAction (UndoAction *action2)
{
  ArmySetInfoAction *action = dynamic_cast<ArmySetInfoAction*>(action2);
  UndoAction *out = NULL;

    switch (action->getType ())
      {
      case ArmySetInfoAction::DESCRIPTION:
          {
            ArmySetInfoAction_Description *a =
              dynamic_cast<ArmySetInfoAction_Description*>(action);
            out = new ArmySetInfoAction_Description (d_description, umgr,
                                                     description_textview);
            d_description = a->getMessage ();
          } 
        break;
      case ArmySetInfoAction::COPYRIGHT:
          {
            ArmySetInfoAction_Copyright *a =
              dynamic_cast<ArmySetInfoAction_Copyright*>(action);
            out = new ArmySetInfoAction_Copyright (d_copyright, umgr,
                                                   copyright_textview);
            d_copyright = a->getMessage ();
          } 
        break;
      case ArmySetInfoAction::LICENSE:
          {
            ArmySetInfoAction_License *a =
              dynamic_cast<ArmySetInfoAction_License*>(action);
            out = new ArmySetInfoAction_License (d_license, umgr,
                                                 license_textview);
            d_license = a->getMessage ();
          } 
        break;
      case ArmySetInfoAction::NAME:
          {
            ArmySetInfoAction_Name *a =
              dynamic_cast<ArmySetInfoAction_Name*>(action);
            out = new ArmySetInfoAction_Name (d_name, umgr, name_entry);
            d_name = a->getName ();
          }
        break;
      case ArmySetInfoAction::TILE_SIZE:
          {
            ArmySetInfoAction_TileSize *a =
              dynamic_cast<ArmySetInfoAction_TileSize*>(action);
            out = new ArmySetInfoAction_TileSize (d_tilesize);
            d_tilesize = a->getTileSize ();
            update_name ();
          } 
        break;
      }
    return out;
}

void ArmySetInfoDialog::on_undo_activated ()
{
  umgr->undo ();
  if (umgr->undoEmpty ())
    d_changed = false;
  update ();
}

void ArmySetInfoDialog::on_redo_activated ()
{
  d_changed = true;
  umgr->redo ();
  update ();
}

void ArmySetInfoDialog::update ()
{
  disconnect_signals ();
  description_textview->get_buffer()->set_text(d_description);
  copyright_textview->get_buffer()->set_text(d_copyright);
  license_textview->get_buffer()->set_text(d_license);
  name_entry->set_text (d_name);
  size_spinbutton->set_value (d_tilesize);
  umgr->setCursors ();
  connect_signals ();
}

void ArmySetInfoDialog::connect_signals ()
{
  umgr->connect_signals ();
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

void ArmySetInfoDialog::disconnect_signals ()
{
  umgr->disconnect_signals ();
  for (auto c : connections)
    c.disconnect ();
  connections.clear ();
}
