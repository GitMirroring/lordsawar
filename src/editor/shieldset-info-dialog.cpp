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

#include "shieldset-info-dialog.h"
#include "shieldsetlist.h"

#include "ucompose.hpp"
#include "defs.h"
#include "File.h"
#include "shieldset-info-actions.h"

#define method(x) sigc::mem_fun(*this, &ShieldSetInfoDialog::x)

ShieldSetInfoDialog::ShieldSetInfoDialog(Gtk::Window &parent, Shieldset *s)
 : LwEditorDialog(parent, "shieldset-info-dialog.ui")
{
  umgr = new UndoMgr (UndoMgr::DELAY, UndoMgr::LIMIT);
  umgr->execute ().connect (method (executeAction));
  d_shieldset = s;
  dialog->set_title(_("Shield Set Properties"));

  xml->get_widget("close_button", close_button);
  xml->get_widget("status_label", status_label);
  xml->get_widget("location_label", location_label);
  xml->get_widget("name_entry", name_entry);
  xml->get_widget("small_width_spinbutton", small_width_spinbutton);
  xml->get_widget("small_height_spinbutton", small_height_spinbutton);
  xml->get_widget("medium_width_spinbutton", medium_width_spinbutton);
  xml->get_widget("medium_height_spinbutton", medium_height_spinbutton);
  xml->get_widget("large_width_spinbutton", large_width_spinbutton);
  xml->get_widget("large_height_spinbutton", large_height_spinbutton);
  xml->get_widget("fit_button", fit_button);
  xml->get_widget("undo_button", undo_button);
  undo_button->signal_activate ().connect (method (on_undo_activated));
  xml->get_widget("redo_button", redo_button);
  redo_button->signal_activate ().connect (method (on_redo_activated));
  fit_button->signal_clicked().connect (method(on_fit_pressed));


  name_entry->set_text (d_shieldset->getName ());
  location_label->property_label () =
    d_shieldset->getDirectory ().empty () ? "" :
    d_shieldset->getConfigurationFile (true);

  xml->get_widget("copyright_textview", copyright_textview);
  xml->get_widget("license_textview", license_textview);
  xml->get_widget("description_textview", description_textview);
  xml->get_widget("notebook", notebook);
  d_name = d_shieldset->getName ();
  d_description = d_shieldset->getInfo ();
  d_copyright = d_shieldset->getCopyright ();
  d_license = d_shieldset->getLicense ();
  d_small_width = d_shieldset->getSmallWidth ();
  d_small_height = d_shieldset->getSmallHeight ();
  d_medium_width = d_shieldset->getMediumWidth ();
  d_medium_height = d_shieldset->getMediumHeight ();
  d_large_width = d_shieldset->getLargeWidth ();
  d_large_height = d_shieldset->getLargeHeight ();
  d_orig_name = d_name;
  d_orig_description = d_description;
  d_orig_copyright = d_copyright;
  d_orig_license = d_license;
  d_orig_small_width = d_small_width;
  d_orig_small_height = d_small_height;
  d_orig_medium_width = d_medium_width;
  d_orig_medium_height = d_medium_height;
  d_orig_large_width = d_large_width;
  d_orig_large_height = d_large_height;
  d_changed = false;
  update ();
  update_name ();
}

void ShieldSetInfoDialog::update_name ()
{
  Glib::ustring oldname = d_shieldset->getName ();
  d_shieldset->setName (String::utrim (name_entry->get_text ()));
  close_button->set_sensitive (File::sanify (d_shieldset->getName ()) != "");

  Glib::ustring file =
    Shieldsetlist::getInstance()->lookupConfigurationFileByName(d_shieldset);
  if (file != "" && file != d_shieldset->getConfigurationFile (true))
    status_label->set_text (_("That name is already in use."));
  else
    status_label->set_text ("");
  d_shieldset->setName (oldname);
  d_name = String::utrim (name_entry->get_text ());
}

void ShieldSetInfoDialog::on_name_changed()
{
  umgr->add (new ShieldSetInfoAction_Name
             (d_name, name_entry->get_position ()));
  d_changed = true;
  update_name ();
}

bool ShieldSetInfoDialog::run()
{
  dialog->run();
  dialog->hide ();
  if (d_orig_description == d_description &&
      d_orig_copyright == d_copyright &&
      d_orig_license == d_license &&
      d_orig_name == d_name &&
      d_orig_small_width == d_small_width &&
      d_orig_small_height == d_small_height &&
      d_orig_medium_width == d_medium_width &&
      d_orig_medium_height == d_medium_height &&
      d_orig_large_width == d_large_width &&
      d_orig_large_height == d_large_height)
    return false;
  return d_changed;
}

void ShieldSetInfoDialog::on_copyright_changed ()
{
  umgr->add (new ShieldSetInfoAction_Copyright (d_copyright));
  d_changed = true;
  d_copyright = copyright_textview->get_buffer()->get_text();
}

void ShieldSetInfoDialog::on_license_changed ()
{
  umgr->add (new ShieldSetInfoAction_License (d_license));
  d_changed = true;
  d_license = license_textview->get_buffer()->get_text();
}

void ShieldSetInfoDialog::on_description_changed ()
{
  umgr->add (new ShieldSetInfoAction_Description (d_description));
  d_changed = true;
  d_description = description_textview->get_buffer()->get_text();
}

ShieldSetInfoDialog::~ShieldSetInfoDialog()
{
  delete umgr;
  notebook->property_show_tabs () = false;
}

void ShieldSetInfoDialog::on_small_width_changed ()
{
  umgr->add (new ShieldSetInfoAction_SmallWidth (d_small_width));
  d_changed = true;
  d_small_width = small_width_spinbutton->get_value ();
}

void ShieldSetInfoDialog::on_small_height_changed ()
{
  umgr->add (new ShieldSetInfoAction_SmallHeight (d_small_height));
  d_changed = true;
  d_small_height = small_height_spinbutton->get_value ();
}

void ShieldSetInfoDialog::on_medium_width_changed ()
{
  umgr->add (new ShieldSetInfoAction_MediumWidth (d_medium_width));
  d_changed = true;
  d_medium_width = medium_width_spinbutton->get_value ();
}

void ShieldSetInfoDialog::on_medium_height_changed ()
{
  umgr->add (new ShieldSetInfoAction_MediumHeight (d_medium_height));
  d_changed = true;
  d_medium_height = medium_height_spinbutton->get_value ();
}

void ShieldSetInfoDialog::on_large_width_changed ()
{
  umgr->add (new ShieldSetInfoAction_LargeWidth (d_large_width));
  d_changed = true;
  d_large_width = large_width_spinbutton->get_value ();
}

void ShieldSetInfoDialog::on_large_height_changed ()
{
  umgr->add (new ShieldSetInfoAction_LargeHeight (d_large_height));
  d_changed = true;
  d_large_height = large_height_spinbutton->get_value ();
}

void ShieldSetInfoDialog::on_fit_pressed ()
{
  umgr->add (new ShieldSetInfoAction_Fit (d_small_width, d_small_height, d_medium_width, d_medium_height, d_large_width, d_large_height));
  d_changed = true;
  d_shieldset->setHeightsAndWidthsFromImages();
  small_width_spinbutton->set_value ((double)d_shieldset->getSmallWidth());
  small_height_spinbutton->set_value ((double)d_shieldset->getSmallHeight());
  medium_width_spinbutton->set_value ((double)d_shieldset->getMediumWidth());
  medium_height_spinbutton->set_value ((double)d_shieldset->getMediumHeight());
  large_width_spinbutton->set_value ((double)d_shieldset->getLargeWidth());
  large_height_spinbutton->set_value ((double)d_shieldset->getLargeHeight());
}

UndoAction* ShieldSetInfoDialog::executeAction (UndoAction *action2)
{
  ShieldSetInfoAction *action = dynamic_cast<ShieldSetInfoAction*>(action2);
  UndoAction *out = NULL;

    switch (action->getType ())
      {
      case ShieldSetInfoAction::DESCRIPTION:
          {
            ShieldSetInfoAction_Description *a =
              dynamic_cast<ShieldSetInfoAction_Description*>(action);
            out = new ShieldSetInfoAction_Description (d_description);
            d_description = a->getMessage ();
          } 
        break;
      case ShieldSetInfoAction::COPYRIGHT:
          {
            ShieldSetInfoAction_Copyright *a =
              dynamic_cast<ShieldSetInfoAction_Copyright*>(action);
            out = new ShieldSetInfoAction_Copyright (d_copyright);
            d_copyright = a->getMessage ();
          } 
        break;
      case ShieldSetInfoAction::LICENSE:
          {
            ShieldSetInfoAction_License *a =
              dynamic_cast<ShieldSetInfoAction_License*>(action);
            out = new ShieldSetInfoAction_License (d_license);
            d_license = a->getMessage ();
          } 
        break;
      case ShieldSetInfoAction::NAME:
          {
            ShieldSetInfoAction_Name *a = dynamic_cast<ShieldSetInfoAction_Name*>(action);
            out = new ShieldSetInfoAction_Name
              (d_name, name_entry->get_position ());
            d_name = a->getName ();
            disconnect_signals ();
            name_entry->set_text (d_name);
            name_entry->set_position (a->getCursorPosition ());
            connect_signals ();
          }
        break;
      case ShieldSetInfoAction::SMALL_WIDTH:
          {
            ShieldSetInfoAction_SmallWidth *a =
              dynamic_cast<ShieldSetInfoAction_SmallWidth*>(action);
            out = new ShieldSetInfoAction_SmallWidth (d_small_width);
            d_small_width = a->getSize ();
          } 
        break;
      case ShieldSetInfoAction::SMALL_HEIGHT:
          {
            ShieldSetInfoAction_SmallHeight *a =
              dynamic_cast<ShieldSetInfoAction_SmallHeight*>(action);
            out = new ShieldSetInfoAction_SmallHeight (d_small_height);
            d_small_height = a->getSize ();
          } 
        break;
      case ShieldSetInfoAction::MEDIUM_WIDTH:
          {
            ShieldSetInfoAction_MediumWidth *a =
              dynamic_cast<ShieldSetInfoAction_MediumWidth*>(action);
            out = new ShieldSetInfoAction_MediumWidth (d_medium_width);
            d_medium_width = a->getSize ();
          } 
        break;
      case ShieldSetInfoAction::MEDIUM_HEIGHT:
          {
            ShieldSetInfoAction_MediumHeight *a =
              dynamic_cast<ShieldSetInfoAction_MediumHeight*>(action);
            out = new ShieldSetInfoAction_MediumHeight (d_medium_height);
            d_medium_height = a->getSize ();
          } 
        break;
      case ShieldSetInfoAction::LARGE_WIDTH:
          {
            ShieldSetInfoAction_LargeWidth *a =
              dynamic_cast<ShieldSetInfoAction_LargeWidth*>(action);
            out = new ShieldSetInfoAction_LargeWidth (d_large_width);
            d_large_width = a->getSize ();
          } 
        break;
      case ShieldSetInfoAction::LARGE_HEIGHT:
          {
            ShieldSetInfoAction_LargeHeight *a =
              dynamic_cast<ShieldSetInfoAction_LargeHeight*>(action);
            out = new ShieldSetInfoAction_LargeHeight (d_large_height);
            d_large_height = a->getSize ();
          } 
        break;
      case ShieldSetInfoAction::FIT:
          {
            ShieldSetInfoAction_Fit *a =
              dynamic_cast<ShieldSetInfoAction_Fit*>(action);
            out = new ShieldSetInfoAction_Fit (d_small_width, d_small_height, d_medium_width, d_medium_height, d_large_width, d_large_height);
            d_small_width = a->getSmallWidth ();
            d_small_height = a->getSmallHeight ();
            d_medium_width = a->getMediumWidth ();
            d_medium_height = a->getMediumHeight ();
            d_large_width = a->getLargeWidth ();
            d_large_height = a->getLargeHeight ();
          } 
        break;
      }
    return out;
}

void ShieldSetInfoDialog::on_undo_activated ()
{
  umgr->undo ();
  if (umgr->undoEmpty ())
    d_changed = false;
  update ();
}

void ShieldSetInfoDialog::on_redo_activated ()
{
  d_changed = true;
  umgr->redo ();
  update ();
}

void ShieldSetInfoDialog::update ()
{
  disconnect_signals ();
  description_textview->get_buffer()->set_text(d_description);
  copyright_textview->get_buffer()->set_text(d_copyright);
  license_textview->get_buffer()->set_text(d_license);
  if (name_entry->get_text () != d_name)
    name_entry->set_text (d_name);
  small_width_spinbutton->set_value (d_small_width);
  small_height_spinbutton->set_value (d_small_height);
  medium_width_spinbutton->set_value (d_medium_width);
  medium_height_spinbutton->set_value (d_medium_height);
  large_width_spinbutton->set_value (d_large_width);
  large_height_spinbutton->set_value (d_large_height);
  connect_signals ();
}

void ShieldSetInfoDialog::connect_signals ()
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
    (fit_button->signal_clicked().connect (method(on_fit_pressed)));
  connections.push_back
    (small_width_spinbutton->signal_changed().connect
     (method(on_small_width_changed)));
  connections.push_back
    (small_height_spinbutton->signal_changed().connect
     (method(on_small_height_changed)));
  connections.push_back
    (medium_width_spinbutton->signal_changed().connect
     (method(on_medium_width_changed)));
  connections.push_back
    (medium_height_spinbutton->signal_changed().connect
     (method(on_medium_height_changed)));
  connections.push_back
    (large_width_spinbutton->signal_changed().connect
     (method(on_large_width_changed)));
  connections.push_back
    (large_height_spinbutton->signal_changed().connect
     (method(on_large_height_changed)));
}

void ShieldSetInfoDialog::disconnect_signals ()
{
  for (auto c : connections)
    c.disconnect ();
  connections.clear ();
}
