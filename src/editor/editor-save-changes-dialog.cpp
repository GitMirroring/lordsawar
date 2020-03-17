//  Copyright (C) 2020 Ben Asselstine
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

#include <sigc++/functors/mem_fun.h>

#include "editor-save-changes-dialog.h"

#include "ucompose.hpp"
#include "File.h"
#include "defs.h"


EditorSaveChangesDialog::EditorSaveChangesDialog(Gtk::Window &parent, Glib::ustring text)
 : LwEditorDialog(parent, "editor-save-changes-dialog.ui")
{
  dialog->set_title (_("Save Changes?"));
  xml->get_widget ("label", label);
  label->property_label() = text;
}
