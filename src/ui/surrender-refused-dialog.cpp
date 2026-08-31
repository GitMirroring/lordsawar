//  Copyright (C) 2008, 2009, 2014, 2020 Ben Asselstine
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

#include "surrender-refused-dialog.h"

#include "defs.h"
#include "File.h"
#include "ImageCache.h"
#include "font-size.h"

SurrenderRefusedDialog::SurrenderRefusedDialog(Gtk::Window &parent)
 : LwDialog(parent, "surrender-refused-dialog.ui")
{
  Gtk::Label *label;
  xml->get_widget("label", label);
  xml->get_widget("image", image);

  label->set_text(_("Off with their heads!  I want it ALL!"));
  image->property_pixbuf() =
    ImageCache::getInstance()->getDialogPic
    (ImageCache::DIALOG_PARLEY_REFUSED,
     FontSize::getInstance ()->get_height ())->to_pixbuf();
}
