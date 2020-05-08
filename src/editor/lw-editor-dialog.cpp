//  Copyright (C) 2014, 2015 Ben Asselstine
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

#include "File.h"
#include "lw-editor-dialog.h"
#include "builder-cache.h"

LwEditorDialog::LwEditorDialog(Gtk::Window &parent, Glib::ustring file)
 : xml (BuilderCache::editor_get(file))
{
  xml->get_widget("dialog", dialog);
  dialog->set_transient_for(parent);
  dialog->property_gravity() = Gdk::GRAVITY_STATIC;
}

int LwEditorDialog::run_and_hide()
{
  dialog->show_all();
  int response = dialog->run();
  dialog->hide();
  return response;
}
