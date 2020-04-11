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
#include "citylist.h"
#include "ruinlist.h"
#include "templelist.h"
#include "signpostlist.h"
#include "stonelist.h"
#include "portlist.h"
#include "roadlist.h"
#include "bridgelist.h"
#include "playerlist.h"
#include "stacklist.h"
#include "Itemlist.h"
#include "rewardlist.h"
#include "GameMap.h"

#define method(x) sigc::mem_fun(*this, &MapInfoDialog::x)

MapInfoDialog::MapInfoDialog(Gtk::Window &parent, GameScenario *g)
 : LwEditorDialog(parent, "map-info-dialog.ui")
{
  d_changed = false;
  game_scenario = g;

  xml->get_widget("name_entry", name_entry);
  name_entry->set_text(game_scenario->getName());
  name_entry->signal_changed().connect (method(on_name_changed));
  xml->get_widget("description_textview", description_textview);
  description_textview->get_buffer()->set_text(game_scenario->getComment());
  description_textview->get_buffer()->signal_changed().connect
    (method(on_description_changed));
  xml->get_widget("copyright_textview", copyright_textview);
  copyright_textview->get_buffer()->set_text(game_scenario->getCopyright());
  copyright_textview->get_buffer()->signal_changed().connect
    (method(on_copyright_changed));
  xml->get_widget("license_textview", license_textview);
  license_textview->get_buffer()->set_text(game_scenario->getLicense());
  license_textview->get_buffer()->signal_changed().connect
    (method(on_license_changed));
  xml->get_widget ("notebook", notebook);
  xml->get_widget ("cities_label", cities_label);
  xml->get_widget ("ruins_label", ruins_label);
  xml->get_widget ("temples_label", temples_label);
  xml->get_widget ("signposts_label", signposts_label);
  xml->get_widget ("stones_label", stones_label);
  xml->get_widget ("ports_label", ports_label);
  xml->get_widget ("roads_label", roads_label);
  xml->get_widget ("bridges_label", bridges_label);
  xml->get_widget ("stacks_label", stacks_label);
  xml->get_widget ("armies_label", armies_label);
  xml->get_widget ("keepers_label", keepers_label);
  xml->get_widget ("items_label", items_label);
  xml->get_widget ("rewards_label", rewards_label);
  xml->get_widget ("bags_label", bags_label);
  cities_label->property_label () =
    String::ucompose ("%1", Citylist::getInstance()->size ());
  ruins_label->property_label () =
    String::ucompose ("%1", Ruinlist::getInstance()->size ());
  temples_label->property_label () =
    String::ucompose ("%1", Templelist::getInstance()->size ());
  signposts_label->property_label () =
    String::ucompose ("%1", Signpostlist::getInstance()->size ());
  stones_label->property_label () =
    String::ucompose ("%1", Stonelist::getInstance()->size ());
  ports_label->property_label () =
    String::ucompose ("%1", Portlist::getInstance()->size ());
  roads_label->property_label () =
    String::ucompose ("%1", Roadlist::getInstance()->size ());
  bridges_label->property_label () =
    String::ucompose ("%1", Bridgelist::getInstance()->size ());
  stacks_label->property_label () =
    String::ucompose ("%1", Playerlist::getInstance()->countAllStacks ());
  armies_label->property_label () =
    String::ucompose ("%1", Stacklist::getNoOfArmies ());
  guint32 num_empty = Ruinlist::getInstance()->countEmptyKeepers ();
  if (!num_empty)
    keepers_label->property_label () =
      String::ucompose ("%1", Ruinlist::getInstance()->countKeepers ());
  else
    keepers_label->property_label () =
      String::ucompose (_("%1, %2 empty"),
                        Ruinlist::getInstance()->countKeepers (), num_empty);
  items_label->property_label () =
    String::ucompose ("%1", Itemlist::getInstance ()->size ());
  rewards_label->property_label () =
    String::ucompose ("%1", Rewardlist::getInstance ()->size ());
  bags_label->property_label () =
    String::ucompose ("%1", GameMap::countBags ());
}

bool MapInfoDialog::run()
{
  dialog->show_all();
  dialog->run();
  dialog->hide ();
  return d_changed;
}

void MapInfoDialog::on_name_changed()
{
  d_changed = true;
  game_scenario->setName (name_entry->get_text ());
}

void MapInfoDialog::on_copyright_changed ()
{
  d_changed = true;
  game_scenario->setCopyright(copyright_textview->get_buffer()->get_text());
}

void MapInfoDialog::on_license_changed ()
{
  d_changed = true;
  game_scenario->setLicense(license_textview->get_buffer()->get_text());
}

void MapInfoDialog::on_description_changed ()
{
  d_changed = true;
  game_scenario->setComment(description_textview->get_buffer()->get_text());
}

MapInfoDialog::~MapInfoDialog()
{
  notebook->property_show_tabs () = false;
}

