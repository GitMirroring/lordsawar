//  Copyright (C) 2007 Ole Laursen
//  Copyright (C) 2007, 2008, 2009, 2014, 2020 Ben Asselstine
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

#include "map-info-dialog.h"

#include "ucompose.hpp"
#include "defs.h"
#include "signpost.h"
#include "GameScenario.h"


MapInfoDialog::MapInfoDialog(Gtk::Window &parent, GameScenario *g)
 : LwEditorDialog(parent, "map-info-dialog.ui")
{
    game_scenario = g;
    
    xml->get_widget("name_entry", name_entry);
    name_entry->set_text(game_scenario->getName());
    
    xml->get_widget("description_textview", description_textview);
    description_textview->get_buffer()->set_text(game_scenario->getComment());
    xml->get_widget("copyright_textview", copyright_textview);
    copyright_textview->get_buffer()->set_text(game_scenario->getCopyright());
    xml->get_widget("license_textview", license_textview);
    license_textview->get_buffer()->set_text(game_scenario->getLicense());
    xml->get_widget ("notebook", notebook);
    for (guint32 i = 0; i < notebook->get_children().size(); i++)
      notebook->child_property_tab_expand (*notebook->get_nth_page(i)) = true;
}

int MapInfoDialog::run()
{
    dialog->show_all();
    int response = dialog->run();

    if (response == Gtk::RESPONSE_ACCEPT)	// accepted
    {
        game_scenario->setName(name_entry->get_text());
        game_scenario->setComment(description_textview->get_buffer()->get_text());
        game_scenario->setCopyright(copyright_textview->get_buffer()->get_text());
        game_scenario->setLicense(license_textview->get_buffer()->get_text());
    }
    return response;
}

MapInfoDialog::~MapInfoDialog()
{
  notebook->property_show_tabs () = false;
}
