//  Copyright (C) 2007 Ole Laursen
//  Copyright (C) 2007, 2008, 2009, 2014, 2020, 2021 Ben Asselstine
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
#include "map-info-actions.h"

#define method(x) sigc::mem_fun(*this, &MapInfoDialog::x)

MapInfoDialog::MapInfoDialog(Gtk::Window &parent, GameScenario *g)
 : LwEditorDialog(parent, "map-info-dialog.ui")
{
  umgr = new UndoMgr (UndoMgr::DELAY, UndoMgr::LIMIT);
  umgr->execute ().connect (method (executeAction));
  d_changed = false;
  game_scenario = g;

  xml->get_widget("name_entry", name_entry);
  name_entry->set_text(game_scenario->getName());
  umgr->addCursor (name_entry);
  xml->get_widget("description_textview", description_textview);
  umgr->addCursor (description_textview);
  xml->get_widget("copyright_textview", copyright_textview);
  umgr->addCursor (copyright_textview);
  xml->get_widget("license_textview", license_textview);
  umgr->addCursor (license_textview);
  xml->get_widget ("undo_button", undo_button);
  undo_button->signal_activate ().connect (method (on_undo_activated));
  xml->get_widget ("redo_button", redo_button);
  redo_button->signal_activate ().connect (method (on_redo_activated));
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
  d_name = g->getName ();
  d_description = g->getComment ();
  d_copyright = g->getCopyright ();
  d_license = g->getLicense ();
  d_orig_description = d_description;
  d_orig_copyright = d_copyright;
  d_orig_license = d_license;
  d_orig_name = d_name;
  connect_signals ();
  update ();
}

bool MapInfoDialog::run()
{
  dialog->run();
  dialog->hide ();
  if (d_orig_description == d_description &&
      d_orig_copyright == d_copyright &&
      d_orig_license == d_license &&
      d_orig_name == d_name)
    return false;
  return d_changed;
}

void MapInfoDialog::on_name_changed()
{
  umgr->add (new MapInfoAction_Name (d_name, umgr, name_entry));
  d_changed = true;
  d_name = String::utrim (name_entry->get_text ());
}

void MapInfoDialog::on_copyright_changed ()
{
  umgr->add (new MapInfoAction_Copyright (d_copyright, umgr,
                                          copyright_textview));
  d_changed = true;
  d_copyright = copyright_textview->get_buffer()->get_text();
}

void MapInfoDialog::on_license_changed ()
{
  umgr->add (new MapInfoAction_License (d_license, umgr,
                                        license_textview));
  d_changed = true;
  d_license = license_textview->get_buffer()->get_text();
}

void MapInfoDialog::on_description_changed ()
{
  umgr->add (new MapInfoAction_Description (d_description, umgr,
                                            description_textview));
  d_changed = true;
  d_description = description_textview->get_buffer()->get_text();
}

MapInfoDialog::~MapInfoDialog()
{
  delete umgr;
  notebook->property_show_tabs () = false;
}

UndoAction* MapInfoDialog::executeAction (UndoAction *action2)
{
  MapInfoAction *action = dynamic_cast<MapInfoAction*>(action2);
  UndoAction *out = NULL;

    switch (action->getType ())
      {
      case MapInfoAction::DESCRIPTION:
          {
            MapInfoAction_Description *a =
              dynamic_cast<MapInfoAction_Description*>(action);
            out = new MapInfoAction_Description (d_description, umgr,
                                                 description_textview);
            d_description = a->getMessage ();
          } 
        break;
      case MapInfoAction::COPYRIGHT:
          {
            MapInfoAction_Copyright *a =
              dynamic_cast<MapInfoAction_Copyright*>(action);
            out = new MapInfoAction_Copyright (d_copyright, umgr,
                                               copyright_textview);
            d_copyright = a->getMessage ();
          } 
        break;
      case MapInfoAction::LICENSE:
          {
            MapInfoAction_License *a =
              dynamic_cast<MapInfoAction_License*>(action);
            out = new MapInfoAction_License (d_license, umgr,
                                             license_textview);
            d_license = a->getMessage ();
          } 
        break;
      case MapInfoAction::NAME:
          {
            MapInfoAction_Name *a = dynamic_cast<MapInfoAction_Name*>(action);
            out = new MapInfoAction_Name (d_name, umgr, name_entry);
            d_name = a->getName ();
          }
        break;
      }
    return out;
}

void MapInfoDialog::on_undo_activated ()
{
  umgr->undo ();
  if (umgr->undoEmpty ())
    d_changed = false;
  update ();
}

void MapInfoDialog::on_redo_activated ()
{
  d_changed = true;
  umgr->redo ();
  update ();
}

void MapInfoDialog::update ()
{
  disconnect_signals ();
  description_textview->get_buffer()->set_text(d_description);
  copyright_textview->get_buffer()->set_text(d_copyright);
  license_textview->get_buffer()->set_text(d_license);
  if (name_entry->get_text () != d_name)
    name_entry->set_text (d_name);
  umgr->setCursors ();
  connect_signals ();
}

void MapInfoDialog::connect_signals ()
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
}

void MapInfoDialog::disconnect_signals ()
{
  umgr->disconnect_signals ();
  for (auto c : connections)
    c.disconnect ();
  connections.clear ();
}
