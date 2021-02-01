//  Copyright (C) 2007 Ole Laursen
//  Copyright (C) 2007, 2008, 2009, 2012, 2014, 2015, 2020, 2021 Ben Asselstine
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
#include "city-editor-actions.h"
#include "stacktile.h"

#define method(x) sigc::mem_fun(*this, &CityEditorDialog::x)

CityEditorDialog::CityEditorDialog(Gtk::Window &parent, City *cit, CreateScenarioRandomize *randomizer)
 : LwEditorDialog(parent, "city-editor-dialog.ui"),
    strength_column(_("Strength"), strength_renderer),
    moves_column(_("Max Moves"), moves_renderer),
    duration_column(_("Turns"), duration_renderer),
    upkeep_column(_("Upkeep"), upkeep_renderer)
{
  city = cit;
  umgr = new UndoMgr (UndoMgr::DELAY, UndoMgr::LIMIT);
  umgr->execute ().connect (method (executeAction));
  d_randomizer = randomizer;
  d_changed = false;

  xml->get_widget("notebook", notebook);
  xml->get_widget("capital_switch", capital_switch);

  xml->get_widget("name_entry", name_entry);

  xml->get_widget("undo_button", undo_button);
  undo_button->signal_activate ().connect (method (on_undo_activated));
  xml->get_widget("redo_button", redo_button);
  redo_button->signal_activate ().connect (method (on_redo_activated));
  xml->get_widget("income_spinbutton", income_spinbutton);
  xml->get_widget("burned_switch", burned_switch);
  xml->get_widget("build_production_switch", build_production_switch);

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
  Gtk::Alignment *alignment;
  xml->get_widget("player_alignment", alignment);
  alignment->add(*player_combobox);

  xml->get_widget("description_textview", description_textview);

  // setup the army list
  army_list = Gtk::ListStore::create(army_columns);

  xml->get_widget("army_treeview", army_treeview);
  army_treeview->set_model(army_list);

  army_treeview->append_column("", army_columns.image);
  strength_renderer.property_editable() = true;
  strength_column.set_cell_data_func
    (strength_renderer, method (cell_data_strength));
  army_treeview->append_column(strength_column);

  moves_renderer.property_editable() = true;
  moves_column.set_cell_data_func (moves_renderer, method (cell_data_moves));
  army_treeview->append_column(moves_column);

  upkeep_renderer.property_editable() = true;
  upkeep_column.set_cell_data_func (upkeep_renderer, method (cell_data_upkeep));
  army_treeview->append_column(upkeep_column);

  duration_renderer.property_editable() = true;
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

  fill_armies ();

  Player *player = get_selected_player ();
  bool neutral = player == Playerlist::getInstance ()->getNeutral ();
  if (city->isBurnt () && neutral)
    burned_switch->set_active (false);
  burned_switch->set_sensitive (!neutral);

  connect_signals ();
  update ();
}

CityEditorDialog::~CityEditorDialog ()
{
  clear_armies ();
  delete umgr;
  notebook->property_show_tabs () = false;
}

void CityEditorDialog::change_city_ownership(Player *player)
{
  // set allegiance
  //look for stacks in the city, and set them to this player
  for (auto s : Stacklist::getDefendersInCity (city))
    {
      s->getOwner()->getStacklist()->on_stack_died (s);
      Stacklist::changeOwnership(s, player);
    }
  city->setOwner(player);
  GameMap::getInstance ()->clearStackPositions();
  GameMap::getInstance ()->updateStackPositions();

  return;
}

bool CityEditorDialog::run()
{
  dialog->show_all();
  dialog->run();
  return d_changed;
}

void CityEditorDialog::on_add_clicked()
{
  CityEditorAction_Add *action = new CityEditorAction_Add (city);
  SelectArmyDialog d(*dialog, SelectArmyDialog::SELECT_NORMAL,
                     city->getOwner(), -1);
  d.run();

  const ArmyProto *army = d.get_selected_army();
  if (army)
    {
      umgr->add (action);
      d_changed = true;
      add_army(new ArmyProdBase(*army));
    }
  else
    delete action;
  update_armies ();
}

void CityEditorDialog::on_remove_clicked()
{
  CityEditorAction_Remove *action = new CityEditorAction_Remove (city);
  Gtk::TreeIter i = army_treeview->get_selection()->get_selected();
  if (i)
    {
      umgr->add (action);
      d_changed = true;
      const ArmyProdBase *a = (*i)[army_columns.army];
      delete a;
      (*i)[army_columns.army] = NULL;
      army_list->erase(i);
    }
  else
    delete action;

  update_armies ();
  set_button_sensitivity();
}

void CityEditorDialog::clear_armies ()
{
  for (Gtk::TreeIter i = army_list->children().begin(),
       end = army_list->children().end(); i != end; ++i)
    {
      const ArmyProdBase *a = (*i)[army_columns.army];
      delete a;
    }
  army_list->clear ();
}

void CityEditorDialog::on_randomize_armies_clicked()
{
  umgr->add (new CityEditorAction_Randomize (city));
  d_changed = true;
  const ArmyProdBase *army;
  clear_armies ();
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
  umgr->add (new CityEditorAction_RandomizeName (city));
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
  umgr->add (new CityEditorAction_RandomizeIncome (city));
  int gold = d_randomizer->getRandomCityIncome(capital_switch->get_active());
  income_spinbutton->set_value(gold);
}

void CityEditorDialog::add_army(const ArmyProdBase *a)
{
  Player *player = get_selected_player();
  ImageCache *gc = ImageCache::getInstance();
  Gtk::TreeIter i = army_list->append();
  const ArmyProdBase *aa = new ArmyProdBase (*a);
  (*i)[army_columns.army] = aa;
  guint32 fs = FontSize::getInstance ()->get_height ();
  (*i)[army_columns.image] = gc->getArmyPic(player->getArmyset(),
					    aa->getTypeId(), player,
					    NULL, false, fs)->to_pixbuf();
  (*i)[army_columns.strength] = aa->getStrength();
  (*i)[army_columns.moves] = aa->getMaxMoves();
  (*i)[army_columns.upkeep] = a->getUpkeep();
  (*i)[army_columns.duration] = aa->getProduction();
  (*i)[army_columns.name] = aa->getName();
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
  const ArmyProdBase *army =
    (*army_list->get_iter(Gtk::TreePath(path)))[army_columns.army];
  guint32 old_str = army->getStrength ();
  int i = atoi (path.c_str ());
  umgr->add (new CityEditorAction_Strength (i, old_str));
          
  ArmyProdBase *newarmy = new ArmyProdBase (*army);
  newarmy->setStrength (str);
  replaceProdBase (i, newarmy);

  d_changed = true;
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
  const ArmyProdBase *army =
    (*army_list->get_iter(Gtk::TreePath(path)))[army_columns.army];
  guint32 old_moves = army->getMaxMoves ();
  int i = atoi (path.c_str ());
  umgr->add (new CityEditorAction_Moves (i, old_moves));
  ArmyProdBase *newarmy = new ArmyProdBase (*army);
  newarmy->setMaxMoves (moves);
  replaceProdBase (i, newarmy);
  d_changed = true;
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
  const ArmyProdBase *army =
    (*army_list->get_iter(Gtk::TreePath(path)))[army_columns.army];
  guint32 old_turns = army->getProduction ();
  int i = atoi (path.c_str ());
  umgr->add (new CityEditorAction_Turns (i, old_turns));
  ArmyProdBase *newarmy = new ArmyProdBase (*army);
  newarmy->setProduction (turns);
  replaceProdBase (i, newarmy);
  d_changed = true;
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
  const ArmyProdBase *army =
    (*army_list->get_iter(Gtk::TreePath(path)))[army_columns.army];
  guint32 old_upkeep = army->getUpkeep ();
  int i = atoi (path.c_str ());
  umgr->add (new CityEditorAction_Upkeep (i, old_upkeep));
  ArmyProdBase *newarmy = new ArmyProdBase (*army);
  newarmy->setUpkeep (upkeep);
  replaceProdBase (i, newarmy);
  d_changed = true;
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
  CityEditorAction_Owner *action = new CityEditorAction_Owner (city);
  Player *player = get_selected_player();
  if (player != city->getOwner ())
    {
      umgr->add (action);
      change_city_ownership (player);
      ImageCache *gc = ImageCache::getInstance();
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
      disconnect_signals ();
      if (capital_switch->get_active())
        {
          city->setCapital (false);
          capital_switch->set_active(false);
        }
      bool neutral = player == Playerlist::getInstance ()->getNeutral ();
      if (city->isBurnt () && neutral)
        {
          city->setBurnt (false);
          burned_switch->set_active (false);
        }
      connect_signals ();
      burned_switch->set_sensitive (!neutral);
      d_changed = true;
      update_buttons();
    }
  else
    delete action;
}

void CityEditorDialog::update_buttons ()
{
  Player *player = get_selected_player();
  capital_switch->set_sensitive
    (player != Playerlist::getInstance()->getNeutral());
  bool burned = burned_switch->get_active ();
  burned_switch->set_sensitive
    (player != Playerlist::getInstance()->getNeutral());
  add_button->set_sensitive (!burned);
  remove_button->set_sensitive (!burned);
  randomize_armies_button->set_sensitive (!burned);
}

void CityEditorDialog::on_burned_changed ()
{
  umgr->add (new CityEditorAction_Razed (city));
  d_changed = true;
  city->setBurnt (burned_switch->get_active ());
  if (city->isBurnt ())
    {
      guint32 c = 0;
      for (; c < city->getMaxNoOfProductionBases(); ++c)
        city->removeProductionBase(c);
      clear_armies ();
    }
  update_buttons ();
}

void CityEditorDialog::on_capital_changed ()
{
  umgr->add (new CityEditorAction_Capital (city));
  d_changed = true;
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
  umgr->add (new CityEditorAction_Name (city, name_entry->get_position ()));
  d_changed = true;
  city->setName (String::utrim (name_entry->get_text ()));
}

void CityEditorDialog::on_income_text_changed ()
{
  umgr->add (new CityEditorAction_Income (city));
  income_spinbutton->set_value(atoi(income_spinbutton->get_text().c_str()));
  city->setGold(income_spinbutton->get_value_as_int());
  d_changed = true;
}

void CityEditorDialog::on_build_production_changed ()
{
  umgr->add (new CityEditorAction_NewProd (city));
  d_changed = true;
  city->setBuildProduction(build_production_switch->get_active ());
}

void CityEditorDialog::on_description_changed ()
{
  umgr->add (new CityEditorAction_Description (city));
  d_changed = true;
  Glib::ustring desc =
    String::utrim (description_textview->get_buffer ()->get_text ());
  city->setDescription (desc);
}

void CityEditorDialog::on_undo_activated ()
{
  umgr->undo ();
  update_armies ();
  if (umgr->undoEmpty ())
    d_changed = false;
  update ();
  return;
}

void CityEditorDialog::on_redo_activated ()
{
  umgr->redo ();
  update_armies ();
  d_changed = true;
  update ();
}

void CityEditorDialog::update ()
{
  disconnect_signals ();
  capital_switch->set_active(city->isCapital());
  name_entry->set_text(city->getName());
  income_spinbutton->set_value(city->getGold());
  burned_switch->set_active(city->isBurnt());
  build_production_switch->set_active(city->getBuildProduction());
  description_textview->get_buffer()->set_text(city->getDescription());

  int player_row = 0;
  int c = 0;
  for (Playerlist::iterator i = Playerlist::getInstance()->begin(),
       end = Playerlist::getInstance()->end(); i != end; ++i, ++c)
    {
      if (*i== city->getOwner())
        player_row = c;
    }
  player_combobox->set_active(player_row);

  Player *player = get_selected_player ();
  ImageCache *gc = ImageCache::getInstance();
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
  update_buttons();
  connect_signals ();
}

void CityEditorDialog::connect_signals ()
{
  connections.push_back
    (name_entry->signal_changed ().connect (method (on_name_changed)));
  connections.push_back
    (capital_switch->property_active ().signal_changed ().connect
     (method (on_capital_changed)));
  connections.push_back
    (income_spinbutton->signal_insert_text().connect
     (sigc::hide(sigc::hide(method(on_income_text_changed)))));
  connections.push_back
    (burned_switch->property_active ().signal_changed ().connect
     (method (on_burned_changed)));
  connections.push_back
    (build_production_switch->property_active ().signal_changed ().connect
     (method (on_build_production_changed)));
  connections.push_back
    (description_textview->get_buffer()->signal_changed().connect
     (method(on_description_changed)));
  connections.push_back
    (player_combobox->signal_changed().connect (method (on_player_changed)));
  connections.push_back
    (strength_renderer.signal_edited().connect(method (on_strength_edited)));
  connections.push_back
    (moves_renderer.signal_edited().connect(method (on_moves_edited)));
  connections.push_back
    (upkeep_renderer.signal_edited().connect(method (on_upkeep_edited)));
  connections.push_back
    (duration_renderer.signal_edited().connect(method (on_turns_edited)));
}

void CityEditorDialog::disconnect_signals ()
{
  for (auto c : connections)
    c.disconnect ();
  connections.clear ();
}

UndoAction *CityEditorDialog::executeAction (UndoAction *action2)
{
  CityEditorAction *action =
    dynamic_cast<CityEditorAction*>(action2);
  UndoAction *out = NULL;

  switch (action->getType ())
    {
    case CityEditorAction::OWNER:
        {
          CityEditorAction_Owner *a =
            dynamic_cast<CityEditorAction_Owner*>(action);
          out = new CityEditorAction_Owner (city);
          change_city_ownership (a->getCity ()->getOwner ());
          city->setBurnt (a->getCity ()->isBurnt ());
          city->setCapital (a->getCity ()->isCapital ());
          city->setCapitalOwner (a->getCity ()->getCapitalOwner ());
        }
      break;
    case CityEditorAction::CAPITAL:
        {
          CityEditorAction_Capital *a =
            dynamic_cast<CityEditorAction_Capital*>(action);
          out = new CityEditorAction_Capital (city);
          city->setCapital (a->getCity()->isCapital ());
          city->setCapitalOwner (a->getCity()->getCapitalOwner ());
        }
      break;
    case CityEditorAction::RAZED:
        {
          CityEditorAction_Razed *a =
            dynamic_cast<CityEditorAction_Razed*>(action);
          out = new CityEditorAction_Razed (city);
          city->setBurnt (a->getCity ()->isBurnt ());
          replaceProdBases (a);
          fill_armies ();
        }
      break;
    case CityEditorAction::NAME:
        {
          CityEditorAction_Name *a =
            dynamic_cast<CityEditorAction_Name*>(action);
          out = new CityEditorAction_Name (city,
                                           name_entry->get_position ());
          city->setName (a->getCity ()->getName ());
        }
      break;
    case CityEditorAction::INCOME:
        {
          CityEditorAction_Income *a =
            dynamic_cast<CityEditorAction_Income*>(action);
          out = new CityEditorAction_Income (city);
          city->setGold (a->getCity ()->getGold ());
        }
      break;
    case CityEditorAction::NEWPROD:
        {
          CityEditorAction_NewProd *a =
            dynamic_cast<CityEditorAction_NewProd*>(action);
          out = new CityEditorAction_NewProd (city);
          city->setBuildProduction (a->getCity ()->getBuildProduction ());
        }
      break;
    case CityEditorAction::RANDOMIZE_NAME:
        {
          CityEditorAction_RandomizeName *a =
            dynamic_cast<CityEditorAction_RandomizeName *>(action);
          out = new CityEditorAction_RandomizeName (city);
          city->setName (a->getCity ()->getName ());
        }
      break;
    case CityEditorAction::RANDOMIZE_INCOME:
        {
          CityEditorAction_RandomizeIncome *a =
            dynamic_cast<CityEditorAction_RandomizeIncome*>(action);
          out = new CityEditorAction_RandomizeIncome (city);
          city->setGold (a->getCity ()->getGold ());
        }
      break;
    case CityEditorAction::ADD:
        {
          CityEditorAction_Add *a =
            dynamic_cast<CityEditorAction_Add*>(action);
          out = new CityEditorAction_Add (city);
          replaceProdBases (a);
          fill_armies ();
        }
      break;
    case CityEditorAction::REMOVE:
        {
          CityEditorAction_Remove *a =
            dynamic_cast<CityEditorAction_Remove*>(action);
          out = new CityEditorAction_Remove (city);
          replaceProdBases (a);
          fill_armies ();
        }
      break;
    case CityEditorAction::RANDOMIZE:
        {
          CityEditorAction_Randomize *a =
            dynamic_cast<CityEditorAction_Randomize*>(action);
          out = new CityEditorAction_Randomize (city);
          replaceProdBases (a);
          fill_armies ();
        }
      break;
    case CityEditorAction::DESCRIPTION:
        {
          CityEditorAction_Description *a =
            dynamic_cast<CityEditorAction_Description*>(action);
          out = new CityEditorAction_Description (city);
          city->setDescription (a->getCity ()->getDescription ());
        }
      break;
    case CityEditorAction::STRENGTH:
        {
          CityEditorAction_Strength *a =
            dynamic_cast<CityEditorAction_Strength*>(action);
          const ArmyProdBase *army = getProdBaseByIndex (a);
          out = new CityEditorAction_Strength (a->getIndex (),
                                               army->getStrength ());
          ArmyProdBase *newarmy = new ArmyProdBase (*army);
          newarmy->setStrength (a->getStrength ());
          replaceProdBase (a->getIndex (), newarmy);

          auto path = Gtk::TreePath (String::ucompose ("%1", a->getIndex ()));
          auto iterrow = army_treeview->get_model ()->get_iter (path);
          Gtk::TreeModel::Row row = *iterrow;
          row[army_columns.strength] = newarmy->getStrength ();
        }
      break;
    case CityEditorAction::TURNS:
        {
          CityEditorAction_Turns *a =
            dynamic_cast<CityEditorAction_Turns*>(action);
          const ArmyProdBase *army = getProdBaseByIndex (a);
          out = new CityEditorAction_Turns (a->getIndex (),
                                            army->getProduction ());
          ArmyProdBase *newarmy = new ArmyProdBase (*army);
          newarmy->setProduction (a->getTurns ());
          replaceProdBase (a->getIndex (), newarmy);
          auto path = Gtk::TreePath (String::ucompose ("%1", a->getIndex ()));
          auto iterrow = army_treeview->get_model ()->get_iter (path);
          Gtk::TreeModel::Row row = *iterrow;
          row[army_columns.duration] = newarmy->getProduction ();
        }
      break;
    case CityEditorAction::MOVES:
        {
          CityEditorAction_Moves *a =
            dynamic_cast<CityEditorAction_Moves*>(action);
          const ArmyProdBase *army = getProdBaseByIndex (a);
          out = new CityEditorAction_Moves (a->getIndex (),
                                            army->getMaxMoves ());
          ArmyProdBase *newarmy = new ArmyProdBase (*army);
          newarmy->setMaxMoves (a->getMoves ());
          replaceProdBase (a->getIndex (), newarmy);
          auto path = Gtk::TreePath (String::ucompose ("%1", a->getIndex ()));
          auto iterrow = army_treeview->get_model ()->get_iter (path);
          Gtk::TreeModel::Row row = *iterrow;
          row[army_columns.moves] = newarmy->getMaxMoves ();
        }
      break;
    case CityEditorAction::UPKEEP:
        {
          CityEditorAction_Upkeep *a =
            dynamic_cast<CityEditorAction_Upkeep*>(action);
          const ArmyProdBase *army = getProdBaseByIndex (a);
          out = new CityEditorAction_Upkeep (a->getIndex (),
                                             army->getUpkeep ());
          ArmyProdBase *newarmy = new ArmyProdBase (*army);
          newarmy->setUpkeep (a->getUpkeep ());
          replaceProdBase (a->getIndex (), newarmy);
          auto path = Gtk::TreePath (String::ucompose ("%1", a->getIndex ()));
          auto iterrow = army_treeview->get_model ()->get_iter (path);
          Gtk::TreeModel::Row row = *iterrow;
          row[army_columns.upkeep] = newarmy->getUpkeep ();
        }
      break;
    }
  return out;
}

int CityEditorDialog::getCurIndex ()
{
  int idx = -1;
  Gtk::TreeIter i = army_treeview->get_selection()->get_selected();
  if (i)
    {
      auto path = army_treeview->get_model ()->get_path (i);
      idx = atoi (path.to_string ().c_str ());
    }
  return idx;
}

const ArmyProdBase* CityEditorDialog::getProdBaseByIndex (CityEditorAction_Index *a)
{
  auto path = Gtk::TreePath (String::ucompose ("%1", a->getIndex ()));
  auto iterrow = army_treeview->get_model ()->get_iter (path);
  Gtk::TreeModel::Row row = *iterrow;
  const ArmyProdBase *army = row[army_columns.army];
  return army;
}

void CityEditorDialog::replaceProdBase (int i, ArmyProdBase *a)
{
  auto path = Gtk::TreePath (String::ucompose ("%1", i));
  auto iterrow = army_treeview->get_model ()->get_iter (path);
  Gtk::TreeModel::Row row = *iterrow;
  const ArmyProdBase *army = row[army_columns.army];
  delete army;
  row[army_columns.army] = a;
}

void CityEditorDialog::fill_armies ()
{
  clear_armies ();
  for (unsigned int i = 0; i < city->getMaxNoOfProductionBases(); i++)
    {
      const ArmyProdBase* a = city->getProductionBase(i);
      if (a)
        add_army(a);
    }
}
void CityEditorDialog::replaceProdBases (CityEditorAction_City *a)
{
  guint32 c = 0;
  for (; c < city->getMaxNoOfProductionBases(); ++c)
    city->removeProductionBase(c);
  c = 0;
  for (; c < city->getMaxNoOfProductionBases (); ++c)
    {
      const ArmyProdBase *army = a->getCity ()->getProductionBase (c);
      if (army)
        city->addProductionBase (c, new ArmyProdBase (*army));
    }
}
