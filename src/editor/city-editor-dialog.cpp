//  Copyright (C) 2007 Ole Laursen
//  Copyright (C) 2007, 2008, 2009, 2012, 2014, 2015, 2020 Ben Asselstine
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
#include <gtkmm.h>

#include "city-editor-dialog.h"

#include "ucompose.hpp"
#include "defs.h"
#include "city.h"
#include "armyprodbase.h"
#include "army.h"
#include "armyproto.h"
#include "playerlist.h"
#include "stacklist.h"
#include "citylist.h"
#include "CreateScenarioRandomize.h"
#include "ImageCache.h"
#include "GameMap.h"
#include "font-size.h"
#include "select-army-dialog.h"

#define method(x) sigc::mem_fun(*this, &CityEditorDialog::x)

CityEditorDialog::CityEditorDialog(Gtk::Window &parent, City *cit, CreateScenarioRandomize *randomizer)
 : LwEditorDialog(parent, "city-editor-dialog.ui"),
    strength_column(_("Strength"), strength_renderer),
    moves_column(_("Max Moves"), moves_renderer),
    duration_column(_("Turns"), duration_renderer),
    upkeep_column(_("Upkeep"), upkeep_renderer)
{
  city = cit;
  d_randomizer = randomizer;

  xml->get_widget("capital_switch", capital_switch);
  capital_switch->set_active(city->isCapital());
  capital_switch->property_active ().signal_changed ().connect (method (on_capital_changed));

  xml->get_widget("name_entry", name_entry);
  name_entry->set_text(city->getName());
  name_entry->signal_changed ().connect (method (on_name_changed));

  xml->get_widget("income_spinbutton", income_spinbutton);
  income_spinbutton->set_value(city->getGold());
  income_spinbutton->signal_changed().connect (method (on_income_changed));

  xml->get_widget("burned_switch", burned_switch);
  burned_switch->set_active(city->isBurnt());
  burned_switch->property_active ().signal_changed ().connect (method (on_burned_changed));

  xml->get_widget("build_production_switch", build_production_switch);
  build_production_switch->set_active(city->getBuildProduction());
  build_production_switch->property_active ().signal_changed ().connect (method (on_build_production_changed));

  // setup the player combo
  player_combobox = manage(new Gtk::ComboBoxText);

  int c = 0, player_no = 0;
  for (Playerlist::iterator i = Playerlist::getInstance()->begin(),
       end = Playerlist::getInstance()->end(); i != end; ++i, ++c)
    {
      Player *player = *i;
      player_combobox->append(player->getName());
      if (player == city->getOwner())
        player_no = c;
    }

  player_combobox->set_active(player_no);
  player_combobox->signal_changed().connect (method (on_player_changed));
  Gtk::Alignment *alignment;
  xml->get_widget("player_alignment", alignment);
  alignment->add(*player_combobox);


  // setup the army list
  army_list = Gtk::ListStore::create(army_columns);

  xml->get_widget("army_treeview", army_treeview);
  army_treeview->set_model(army_list);

  army_treeview->append_column("", army_columns.image);
  strength_renderer.property_editable() = true;
  strength_renderer.signal_edited().connect(method (on_strength_edited));
  strength_column.set_cell_data_func
    (strength_renderer, method (cell_data_strength));
  army_treeview->append_column(strength_column);

  moves_renderer.property_editable() = true;
  moves_renderer.signal_edited().connect(method (on_moves_edited));
  moves_column.set_cell_data_func (moves_renderer, method (cell_data_moves));
  army_treeview->append_column(moves_column);

  upkeep_renderer.property_editable() = true;
  upkeep_renderer.signal_edited().connect(method (on_upkeep_edited));
  upkeep_column.set_cell_data_func (upkeep_renderer, method (cell_data_upkeep));
  army_treeview->append_column(upkeep_column);

  duration_renderer.property_editable() = true;
  duration_renderer.signal_edited().connect(method (on_turns_edited));
  duration_column.set_cell_data_func
    (duration_renderer, method (CityEditorDialog::cell_data_turns));

  army_treeview->append_column(_("Name"), army_columns.name);

  xml->get_widget("add_button", add_button);
  xml->get_widget("remove_button", remove_button);
  xml->get_widget("randomize_armies_button", randomize_armies_button);
  xml->get_widget("randomize_name_button", randomize_name_button);
  xml->get_widget("randomize_income_button", randomize_income_button);

  add_button->signal_clicked().connect (method (on_add_clicked));
  remove_button->signal_clicked().connect (method (on_remove_clicked));
  randomize_armies_button->signal_clicked().connect
    (method (on_randomize_armies_clicked));
  randomize_name_button->signal_clicked().connect
    (method (on_randomize_name_clicked));
  randomize_income_button->signal_clicked().connect
    (method (on_randomize_income_clicked));

  army_treeview->get_selection()->signal_changed().connect
    (method (on_selection_changed));

  for (unsigned int i = 0; i < city->getMaxNoOfProductionBases(); i++)
    {
      const ArmyProdBase* a = city->getProductionBase(i);
      if (a)
        add_army(a);
    }

  Player *player = get_selected_player ();
  bool neutral = player == Playerlist::getInstance ()->getNeutral ();
  if (city->isBurnt () && neutral)
    burned_switch->set_active (false);
  burned_switch->set_sensitive (!neutral);

  update_buttons();
}

void CityEditorDialog::change_city_ownership()
{
  // set allegiance
  Player *player = get_selected_player();
  if (player == city->getOwner()) //no change? do nothing.
    return;
  city->setOwner(player);
  //look for stacks in the city, and set them to this player
  for (unsigned int x = 0; x < city->getSize(); x++)
    {
      for (unsigned int y = 0; y < city->getSize(); y++)
	{
	  Stack *s = GameMap::getStack(city->getPos() + Vector<int>(x,y));
	  if (s)
	    Stacklist::changeOwnership(s, player);
	}
    }
}

int CityEditorDialog::run()
{
  dialog->show_all();
  return dialog->run();
}

void CityEditorDialog::on_add_clicked()
{
  SelectArmyDialog d(*dialog, false, city->getOwner());
  d.run();

  const ArmyProto *army = d.get_selected_army();
  if (army)
    add_army(new ArmyProdBase(*army));
  update_armies ();
}

void CityEditorDialog::on_remove_clicked()
{
  Gtk::TreeIter i = army_treeview->get_selection()->get_selected();
  if (i)
    army_list->erase(i);

  update_armies ();
  set_button_sensitivity();
}

void CityEditorDialog::on_randomize_armies_clicked()
{
  const ArmyProdBase *army;
  army_list->clear();
  city->setRandomArmytypes(true, 1);
  for (unsigned int i = 0; i < city->getMaxNoOfProductionBases(); i++)
    {
      army = city->getProductionBase(i);
      if (army)
	add_army(army);
    }
  set_button_sensitivity();
}

void CityEditorDialog::on_randomize_name_clicked()
{
  Glib::ustring existing_name = name_entry->get_text();
  if (existing_name == City::getDefaultName())
    name_entry->set_text(d_randomizer->popRandomCityName());
  else
    {
      name_entry->set_text(d_randomizer->popRandomCityName());
      d_randomizer->pushRandomCityName(existing_name);
    }
}

void CityEditorDialog::on_randomize_income_clicked()
{
  int gold = d_randomizer->getRandomCityIncome(capital_switch->get_active());
  income_spinbutton->set_value(gold);
}

void CityEditorDialog::add_army(const ArmyProdBase *a)
{
  Player *player = get_selected_player();
  ImageCache *gc = ImageCache::getInstance();
  Gtk::TreeIter i = army_list->append();
  (*i)[army_columns.army] = a;
  guint32 fs = FontSize::getInstance ()->get_height ();
  (*i)[army_columns.image] = gc->getArmyPic(player->getArmyset(),
					    a->getTypeId(), player,
					    NULL, false, fs)->to_pixbuf();
  (*i)[army_columns.strength] = a->getStrength();
  (*i)[army_columns.moves] = a->getMaxMoves();
  (*i)[army_columns.upkeep] = a->getUpkeep();
  (*i)[army_columns.duration] = a->getProduction();
  (*i)[army_columns.name] = a->getName();
  army_treeview->get_selection()->select(i);

  set_button_sensitivity();
}

void CityEditorDialog::update_armies ()
{
  guint32 c = 0;
  for (; c < city->getMaxNoOfProductionBases(); ++c)
    city->removeProductionBase(c);
  c = 0;
  for (Gtk::TreeIter i = army_list->children().begin(),
       end = army_list->children().end(); i != end; ++i, ++c)
    {
      const ArmyProdBase *a = (*i)[army_columns.army];
      ArmyProdBase *army = new ArmyProdBase(*a);
      army->setStrength((*i)[army_columns.strength]);
      army->setProduction((*i)[army_columns.duration]);
      army->setMaxMoves((*i)[army_columns.moves]);
      army->setUpkeep ((*i)[army_columns.upkeep]);
      city->addProductionBase(c, army);
    }
}

void CityEditorDialog::on_selection_changed()
{
  set_button_sensitivity();
}

void CityEditorDialog::set_button_sensitivity()
{
  Gtk::TreeIter i = army_treeview->get_selection()->get_selected();
  unsigned int armies = army_list->children().size();
  add_button->set_sensitive(armies < city->getMaxNoOfProductionBases());
  remove_button->set_sensitive(i);
}

void CityEditorDialog::cell_data_strength(Gtk::CellRenderer *renderer,
				     const Gtk::TreeIter& i)
{
  dynamic_cast<Gtk::CellRendererSpin*>(renderer)->property_adjustment()
    = Gtk::Adjustment::create((*i)[army_columns.strength],
                              MIN_STRENGTH_FOR_ARMY_UNITS,
                              MAX_STRENGTH_FOR_ARMY_UNITS, 1);
  dynamic_cast<Gtk::CellRendererSpin*>(renderer)->property_text() =
    String::ucompose("%1", (*i)[army_columns.strength]);
}

void CityEditorDialog::on_strength_edited(const Glib::ustring &path,
				   const Glib::ustring &new_text)
{
  int str = atoi(new_text.c_str());
  if (str < (int)MIN_STRENGTH_FOR_ARMY_UNITS ||
      str > (int)MAX_STRENGTH_FOR_ARMY_UNITS)
    return;
  (*army_list->get_iter(Gtk::TreePath(path)))[army_columns.strength] = str;
  update_armies ();
}

void CityEditorDialog::cell_data_moves(Gtk::CellRenderer *renderer,
				  const Gtk::TreeIter& i)
{
  dynamic_cast<Gtk::CellRendererSpin*>(renderer)->property_adjustment()
    = Gtk::Adjustment::create((*i)[army_columns.moves],
                              MIN_MOVES_FOR_ARMY_UNITS,
                              MAX_MOVES_FOR_ARMY_UNITS, 1);
  dynamic_cast<Gtk::CellRendererSpin*>(renderer)->property_text() =
    String::ucompose("%1", (*i)[army_columns.moves]);
}

void CityEditorDialog::on_moves_edited(const Glib::ustring &path,
				   const Glib::ustring &new_text)
{
  int moves = atoi(new_text.c_str());
  if (moves < (int)MIN_MOVES_FOR_ARMY_UNITS ||
      moves > (int)MAX_MOVES_FOR_ARMY_UNITS)
    return;
  (*army_list->get_iter(Gtk::TreePath(path)))[army_columns.moves] = moves;
  update_armies ();
}

void CityEditorDialog::cell_data_turns(Gtk::CellRenderer *renderer,
				   const Gtk::TreeIter& i)
{
  dynamic_cast<Gtk::CellRendererSpin*>(renderer)->property_adjustment()
    = Gtk::Adjustment::create((*i)[army_columns.duration],
                              MIN_PRODUCTION_TURNS_FOR_ARMY_UNITS,
                              MAX_PRODUCTION_TURNS_FOR_ARMY_UNITS, 1);
  dynamic_cast<Gtk::CellRendererSpin*>(renderer)->property_text() =
    String::ucompose("%1", (*i)[army_columns.duration]);
}

void CityEditorDialog::on_turns_edited(const Glib::ustring &path,
				   const Glib::ustring &new_text)
{
  int turns = atoi(new_text.c_str());
  if (turns < (int)MIN_PRODUCTION_TURNS_FOR_ARMY_UNITS ||
      turns > (int)MAX_PRODUCTION_TURNS_FOR_ARMY_UNITS)
    return;
  (*army_list->get_iter(Gtk::TreePath(path)))[army_columns.duration] = turns;
  update_armies ();
}

void CityEditorDialog::cell_data_upkeep(Gtk::CellRenderer *renderer,
				   const Gtk::TreeIter& i)
{
  dynamic_cast<Gtk::CellRendererSpin*>(renderer)->property_adjustment()
    = Gtk::Adjustment::create((*i)[army_columns.upkeep], 0, 20, 1);
  dynamic_cast<Gtk::CellRendererSpin*>(renderer)->property_text() =
    String::ucompose("%1", (*i)[army_columns.upkeep]);
}

void CityEditorDialog::on_upkeep_edited(const Glib::ustring &path,
				   const Glib::ustring &new_text)
{
  int upkeep = atoi(new_text.c_str());
  if (upkeep < (int) MIN_UPKEEP_FOR_ARMY_UNITS ||
      upkeep > (int) MAX_UPKEEP_FOR_ARMY_UNITS)
    return;
  (*army_list->get_iter(Gtk::TreePath(path)))[army_columns.upkeep] = upkeep;
  update_armies ();
}

Player *CityEditorDialog::get_selected_player()
{
  int c = 0, row = player_combobox->get_active_row_number();
  Player *player = Playerlist::getInstance()->getNeutral();
  for (Playerlist::iterator i = Playerlist::getInstance()->begin(),
       end = Playerlist::getInstance()->end(); i != end; ++i, ++c)
    if (c == row)
      {
	player = *i;
	break;
      }
  return player;
}

void CityEditorDialog::on_player_changed()
{
  ImageCache *gc = ImageCache::getInstance();
  Player *player = get_selected_player();
  guint32 fs = FontSize::getInstance ()->get_height ();
  for (Gtk::TreeIter j = army_list->children().begin(),
       jend = army_list->children().end(); j != jend; ++j)
    {
      const ArmyProdBase *a = (*j)[army_columns.army];
      (*j)[army_columns.image] = gc->getArmyPic(player->getArmyset(),
						a->getTypeId(),
						player, NULL, false,
                                                fs)->to_pixbuf();
    }
  if (capital_switch->get_active())
    capital_switch->set_active(false);

  bool neutral = player == Playerlist::getInstance ()->getNeutral ();
  if (city->isBurnt () && neutral)
    burned_switch->set_active (false);
  burned_switch->set_sensitive (!neutral);
  change_city_ownership ();
  update_buttons();
}

void CityEditorDialog::update_buttons ()
{
  Player *player = get_selected_player();
  capital_switch->set_sensitive
    (player != Playerlist::getInstance()->getNeutral());
  bool burned = burned_switch->get_active ();
  add_button->set_sensitive (!burned);
  remove_button->set_sensitive (!burned);
  randomize_armies_button->set_sensitive (!burned);
}

void CityEditorDialog::on_burned_changed ()
{
  city->setBurnt (burned_switch->get_active ());
  if (city->isBurnt ())
    {
      guint32 c = 0;
      for (; c < city->getMaxNoOfProductionBases(); ++c)
        city->removeProductionBase(c);
      army_list->clear ();
    }
  update_buttons ();
}

void CityEditorDialog::on_capital_changed ()
{
  Player *player = get_selected_player();
  // make sure player doesn't have other capitals
  Citylist* cl = Citylist::getInstance();
  for (Citylist::iterator i = cl->begin(); i != cl->end(); ++i)
    if ((*i)->isCapital() && (*i)->getOwner() == player)
      {
        (*i)->setCapital(false);
        (*i)->setCapitalOwner(NULL);
      }
  if (capital_switch->get_active ())
    {
      city->setCapital(true);
      city->setCapitalOwner(player);
    }
  else
    {
      city->setCapital(false);
      city->setCapitalOwner(NULL);
    }
}

void CityEditorDialog::on_name_changed ()
{
  city->setName (String::utrim (name_entry->get_text ()));
}

void CityEditorDialog::on_income_changed ()
{
  city->setGold(income_spinbutton->get_value_as_int());
}

void CityEditorDialog::on_income_text_changed ()
{
  income_spinbutton->set_value(atoi(income_spinbutton->get_text().c_str()));
  on_income_changed();
}

void CityEditorDialog::on_build_production_changed ()
{
  city->setBuildProduction(build_production_switch->get_active ());
}
