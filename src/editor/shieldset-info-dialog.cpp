//  Copyright (C) 2007, 2008, 2009, 2010, 2014, 2020 Ben Asselstine
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

#define method(x) sigc::mem_fun(*this, &ShieldSetInfoDialog::x)

ShieldSetInfoDialog::ShieldSetInfoDialog(Gtk::Window &parent, Shieldset *s)
 : LwEditorDialog(parent, "shieldset-info-dialog.ui")
{
  d_shieldset = s;
  dialog->set_title(_("Shieldset Properties"));

  xml->get_widget("close_button", close_button);
  xml->get_widget("status_label", status_label);
  xml->get_widget("location_label", location_label);
  xml->get_widget("name_entry", name_entry);

  name_entry->set_text (d_shieldset->getName ());
  location_label->property_label () = 
    d_shieldset->isTemporaryFile () ? "" : d_shieldset->getConfigurationFile ();

  name_entry->signal_changed().connect (method(on_name_changed));

  xml->get_widget("copyright_textview", copyright_textview);
  copyright_textview->get_buffer()->set_text(d_shieldset->getCopyright());
  copyright_textview->get_buffer()->signal_changed().connect
    (method(on_copyright_changed));
  xml->get_widget("license_textview", license_textview);
  license_textview->get_buffer()->set_text(d_shieldset->getLicense());
  license_textview->get_buffer()->signal_changed().connect
    (method(on_license_changed));
  xml->get_widget("description_textview", description_textview);
  description_textview->get_buffer()->set_text(d_shieldset->getInfo());
  description_textview->get_buffer()->signal_changed().connect
    (method(on_description_changed));
  xml->get_widget("notebook", notebook);
  on_name_changed ();
  d_changed = false;
}

void ShieldSetInfoDialog::on_name_changed()
{
  d_changed = true;
  d_shieldset->setName (name_entry->get_text ());
  close_button->set_sensitive (File::sanify (d_shieldset->getName ()) != "");

  Glib::ustring file =
    Shieldsetlist::getInstance()->lookupConfigurationFileByName(d_shieldset);
  if (file != "" && file != d_shieldset->getConfigurationFile ())
    status_label->set_text (_("That name is already in use."));
  else
    status_label->set_text ("");
}

bool ShieldSetInfoDialog::run()
{
    dialog->show_all();
    dialog->run();
    dialog->hide ();
    return d_changed;
}

void ShieldSetInfoDialog::on_copyright_changed ()
{
  d_changed = true;
  d_shieldset->setCopyright(copyright_textview->get_buffer()->get_text());
}

void ShieldSetInfoDialog::on_license_changed ()
{
  d_changed = true;
  d_shieldset->setLicense(license_textview->get_buffer()->get_text());
}

void ShieldSetInfoDialog::on_description_changed ()
{
  d_changed = true;
  d_shieldset->setInfo(description_textview->get_buffer()->get_text());
}

ShieldSetInfoDialog::~ShieldSetInfoDialog()
{
  notebook->property_show_tabs () = false;
}

