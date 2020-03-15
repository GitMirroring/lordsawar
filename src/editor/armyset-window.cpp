//  Copyright (C) 2007, 2008, 2009, 2010, 2011, 2012, 2014, 2015,
//  2020 Ben Asselstine
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

#include <iostream>
#include <errno.h>
#include <sigc++/functors/mem_fun.h>
#include <sigc++/functors/ptr_fun.h>

#include <gtkmm.h>
#include "armyset-window.h"
#include "builder-cache.h"
#include "armyset-info-dialog.h"
#include "masked-image-editor-dialog.h"

#include "defs.h"
#include "Configuration.h"
#include "ImageCache.h"
#include "armysetlist.h"
#include "Tile.h"
#include "File.h"
#include "shield.h"
#include "shieldsetlist.h"
#include "tile-size-editor-dialog.h"
#include "editor-quit-dialog.h"

#include "ucompose.hpp"

#include "image-editor-dialog.h"
#include "playerlist.h"

#define method(x) sigc::mem_fun(*this, &ArmySetWindow::x)

ArmySetWindow::ArmySetWindow(Glib::ustring load_filename)
{
  needs_saving = false;
  inhibit_needs_saving = false;
  d_armyset = NULL;
  Glib::RefPtr<Gtk::Builder> xml = BuilderCache::editor_get("armyset-window.ui");

  xml->get_widget("window", window);
  window->set_icon_from_file(File::getVariousFile("castle_icon.png"));
  window->signal_delete_event().connect (method(on_window_closed));

  xml->get_widget("white_image", white_image);
  xml->get_widget("make_same_button", make_same_button);
  make_same_button->signal_clicked().connect (method(on_make_same_clicked));
  xml->get_widget("green_image", green_image);
  xml->get_widget("yellow_image", yellow_image);
  xml->get_widget("light_blue_image", light_blue_image);
  xml->get_widget("red_image", red_image);
  xml->get_widget("dark_blue_image", dark_blue_image);
  xml->get_widget("orange_image", orange_image);
  xml->get_widget("black_image", black_image);
  xml->get_widget("neutral_image", neutral_image);
  xml->get_widget("name_entry", name_entry);
  name_entry->signal_changed().connect (method(on_name_changed));
  xml->get_widget("armies_treeview", armies_treeview);
  xml->get_widget("armies_scrolledwindow", armies_scrolledwindow);
  xml->get_widget("description_textview", description_textview);
  description_textview->get_buffer()->signal_changed().connect
    (method(on_description_changed));
  xml->get_widget("white_image_button", white_image_button);
  white_image_button->signal_clicked().connect
    (sigc::bind(method(on_image_changed),
                white_image_button, white_image, Shield::WHITE));
  xml->get_widget("green_image_button", green_image_button);
  green_image_button->signal_clicked().connect
    (sigc::bind(method(on_image_changed), green_image_button, green_image, 
                Shield::GREEN));
  xml->get_widget("yellow_image_button", yellow_image_button);
  yellow_image_button->signal_clicked().connect
    (sigc::bind(method(on_image_changed), yellow_image_button, yellow_image, 
                Shield::YELLOW));
  xml->get_widget("light_blue_image_button", light_blue_image_button);
  light_blue_image_button->signal_clicked().connect
    (sigc::bind(method(on_image_changed), light_blue_image_button, 
                light_blue_image, Shield::LIGHT_BLUE));
  xml->get_widget("red_image_button", red_image_button);
  red_image_button->signal_clicked().connect
    (sigc::bind(method(on_image_changed), red_image_button, red_image, 
                Shield::RED));
  xml->get_widget("dark_blue_image_button", dark_blue_image_button);
  dark_blue_image_button->signal_clicked().connect
    (sigc::bind(method(on_image_changed), dark_blue_image_button, dark_blue_image,
                Shield::DARK_BLUE));
  xml->get_widget("orange_image_button", orange_image_button);
  orange_image_button->signal_clicked().connect
    (sigc::bind(method(on_image_changed), orange_image_button, orange_image, 
                Shield::ORANGE));
  xml->get_widget("black_image_button", black_image_button);
  black_image_button->signal_clicked().connect
    (sigc::bind(method(on_image_changed), black_image_button, black_image, 
                Shield::BLACK));
  xml->get_widget("neutral_image_button", neutral_image_button);
  neutral_image_button->signal_clicked().connect
    (sigc::bind(method(on_image_changed), neutral_image_button, neutral_image, 
                Shield::NEUTRAL));
  xml->get_widget("production_spinbutton", production_spinbutton);
  production_spinbutton->set_range
    (double(MIN_PRODUCTION_TURNS_FOR_ARMY_UNITS), 
     double(MAX_PRODUCTION_TURNS_FOR_ARMY_UNITS));
  production_spinbutton->signal_changed().connect (method(on_production_changed));
  production_spinbutton->signal_insert_text().connect
    (sigc::hide(sigc::hide(method(on_production_text_changed))));
  xml->get_widget("cost_spinbutton", cost_spinbutton);
  cost_spinbutton->set_range(double(MIN_COST_FOR_ARMY_UNITS), 
                             double(MAX_COST_FOR_ARMY_UNITS));
  cost_spinbutton->signal_changed().connect (method(on_cost_changed));
  cost_spinbutton->signal_insert_text().connect
    (sigc::hide(sigc::hide(method(on_cost_text_changed))));
  xml->get_widget("new_cost_spinbutton", new_cost_spinbutton);
  new_cost_spinbutton->set_range(double(MIN_NEW_COST_FOR_ARMY_UNITS), 
                                 double(MAX_NEW_COST_FOR_ARMY_UNITS));
  new_cost_spinbutton->signal_changed().connect (method(on_new_cost_changed));
  new_cost_spinbutton->signal_insert_text().connect
    (sigc::hide(sigc::hide(method(on_new_cost_text_changed))));
  xml->get_widget("upkeep_spinbutton", upkeep_spinbutton);
  upkeep_spinbutton->set_range (double(MIN_UPKEEP_FOR_ARMY_UNITS), 
                                double(MAX_UPKEEP_FOR_ARMY_UNITS));
  upkeep_spinbutton->signal_changed().connect (method(on_upkeep_changed));
  upkeep_spinbutton->signal_insert_text().connect
    (sigc::hide(sigc::hide(method(on_upkeep_text_changed))));
  xml->get_widget("strength_spinbutton", strength_spinbutton);
  strength_spinbutton->set_range (double(MIN_STRENGTH_FOR_ARMY_UNITS), 
                                  double(MAX_STRENGTH_FOR_ARMY_UNITS));
  strength_spinbutton->signal_changed().connect (method(on_strength_changed));
  strength_spinbutton->signal_insert_text().connect
    (sigc::hide(sigc::hide(method(on_strength_text_changed))));
  xml->get_widget("moves_spinbutton", moves_spinbutton);
  moves_spinbutton->set_range(double(MIN_MOVES_FOR_ARMY_UNITS), 
                              double(MAX_MOVES_FOR_ARMY_UNITS));
  moves_spinbutton->signal_changed().connect (method(on_moves_changed));
  moves_spinbutton->signal_insert_text().connect
    (sigc::hide(sigc::hide(method(on_moves_text_changed))));
  xml->get_widget("exp_spinbutton", exp_spinbutton);
  exp_spinbutton->set_range(double(MIN_EXP_FOR_ARMY_UNITS), 
                            double(MAX_EXP_FOR_ARMY_UNITS));
  exp_spinbutton->signal_changed().connect (method(on_exp_changed));
  exp_spinbutton->signal_insert_text().connect
    (sigc::hide(sigc::hide(method(on_exp_text_changed))));
  xml->get_widget("hero_combobox", hero_combobox);
  hero_combobox->signal_changed().connect (method(on_hero_combobox_changed));
  xml->get_widget("awardable_switch", awardable_switch);
  awardable_switch->property_active().signal_changed().connect (method(on_awardable_toggled));
  xml->get_widget("defends_ruins_switch", defends_ruins_switch);
  defends_ruins_switch->property_active().signal_changed().connect (method(on_defends_ruins_toggled));
  xml->get_widget("sight_spinbutton", sight_spinbutton);
  sight_spinbutton->set_range(double(MIN_SIGHT_FOR_ARMY_UNITS), 
                              double(MAX_SIGHT_FOR_ARMY_UNITS));
  sight_spinbutton->signal_changed().connect (method(on_sight_changed));
  sight_spinbutton->signal_insert_text().connect
    (sigc::hide(sigc::hide(method(on_sight_text_changed))));
  xml->get_widget("id_spinbutton", id_spinbutton);
  id_spinbutton->set_range(0, 1000);
  id_spinbutton->signal_changed().connect (method(on_id_changed));
  id_spinbutton->signal_insert_text().connect
    (sigc::hide(sigc::hide(method(on_id_text_changed))));
  xml->get_widget("move_forests_switch", move_forests_switch);
  move_forests_switch->property_active().signal_changed().connect
    (sigc::bind(method(on_movebonus_toggled),
                move_forests_switch, Tile::FOREST));
  xml->get_widget("move_marshes_switch", move_marshes_switch);
  move_marshes_switch->property_active().signal_changed().connect
    (sigc::bind(method(on_movebonus_toggled),
                move_marshes_switch, Tile::SWAMP));
  xml->get_widget("move_hills_switch", move_hills_switch);
  move_hills_switch->property_active().signal_changed().connect
    (sigc::bind(method(on_movebonus_toggled),
                move_hills_switch, Tile::HILLS));
  xml->get_widget("move_mountains_switch", move_mountains_switch);
  move_mountains_switch->property_active().signal_changed().connect
    (sigc::bind(method(on_movebonus_toggled),
                move_mountains_switch, Tile::MOUNTAIN));
  xml->get_widget("can_fly_switch", can_fly_switch);
  can_fly_switch->property_active().signal_changed().connect
    (sigc::bind(method(on_movebonus_toggled),
                can_fly_switch, Tile::GRASS | Tile::WATER | 
                Tile::FOREST | Tile::HILLS | Tile::MOUNTAIN | Tile::SWAMP));
  xml->get_widget("add1strinopen_switch", add1strinopen_switch);
  add1strinopen_switch->property_active().signal_changed().connect
    (sigc::bind(method(on_armybonus_toggled), add1strinopen_switch, 
                Army::ADD1STRINOPEN));
  xml->get_widget("add2strinopen_switch", add2strinopen_switch);
  add2strinopen_switch->property_active().signal_changed().connect
    (sigc::bind(method(on_armybonus_toggled), add2strinopen_switch, 
                Army::ADD2STRINOPEN));
  xml->get_widget("add1strinforest_switch", add1strinforest_switch);
  add1strinforest_switch->property_active().signal_changed().connect
    (sigc::bind(method(on_armybonus_toggled), add1strinforest_switch, 
                Army::ADD1STRINFOREST));
  xml->get_widget("add2strinforest_switch", add2strinforest_switch);
  add2strinforest_switch->property_active().signal_changed().connect
    (sigc::bind(method(on_armybonus_toggled), add2strinforest_switch, 
                Army::ADD2STRINFOREST));
  xml->get_widget("add1strinhills_switch", add1strinhills_switch);
  add1strinhills_switch->property_active().signal_changed().connect
    (sigc::bind(method(on_armybonus_toggled), add1strinhills_switch, 
                Army::ADD1STRINHILLS));
  xml->get_widget("add2strinhills_switch", add2strinhills_switch);
  add2strinhills_switch->property_active().signal_changed().connect
    (sigc::bind(method(on_armybonus_toggled), add2strinhills_switch, 
                Army::ADD2STRINHILLS));
  xml->get_widget("add1strincity_switch", add1strincity_switch);
  add1strincity_switch->property_active().signal_changed().connect
    (sigc::bind(method(on_armybonus_toggled), add1strincity_switch,
                Army::ADD1STRINCITY));
  xml->get_widget("add2strincity_switch", add2strincity_switch);
  add2strincity_switch->property_active().signal_changed().connect
    (sigc::bind(method (on_armybonus_toggled), add2strincity_switch, 
                Army::ADD2STRINCITY));
  xml->get_widget("add1stackinhills_switch", 
                  add1stackinhills_switch);
  add1stackinhills_switch->property_active().signal_changed().connect
    (sigc::bind(method (on_armybonus_toggled), add1stackinhills_switch, 
                Army::ADD1STACKINHILLS));
  xml->get_widget("suballcitybonus_switch", suballcitybonus_switch);
  suballcitybonus_switch->property_active().signal_changed().connect
    (sigc::bind(method (on_armybonus_toggled), suballcitybonus_switch,
                Army::SUBALLCITYBONUS));
  xml->get_widget("sub1enemystack_switch", sub1enemystack_switch);
  sub1enemystack_switch->property_active().signal_changed().connect
    (sigc::bind(method (on_armybonus_toggled),
                sub1enemystack_switch, Army::SUB1ENEMYSTACK));
  xml->get_widget("sub2enemystack_switch", sub2enemystack_switch);
  sub2enemystack_switch->property_active().signal_changed().connect
    (sigc::bind(method(on_armybonus_toggled), sub2enemystack_switch, 
                Army::SUB2ENEMYSTACK));
  xml->get_widget("add1stack_switch", add1stack_switch);
  add1stack_switch->property_active().signal_changed().connect
    (sigc::bind(method(on_armybonus_toggled), add1stack_switch, 
                Army::ADD1STACK));
  xml->get_widget("add2stack_switch", add2stack_switch);
  add2stack_switch->property_active().signal_changed().connect
    (sigc::bind(method (on_armybonus_toggled), add2stack_switch, 
                Army::ADD2STACK));
  xml->get_widget("suballnonherobonus_switch", 
                  suballnonherobonus_switch);
  suballnonherobonus_switch->property_active().signal_changed().connect
    (sigc::bind(method (on_armybonus_toggled), suballnonherobonus_switch,
                Army::SUBALLNONHEROBONUS));
  xml->get_widget("suballherobonus_switch", suballherobonus_switch);
  suballherobonus_switch->property_active().signal_changed().connect
    (sigc::bind(method(on_armybonus_toggled), suballherobonus_switch, 
                Army::SUBALLHEROBONUS));
  xml->get_widget("add_army_button", add_army_button);
  add_army_button->signal_clicked().connect (method(on_add_army_clicked));
  xml->get_widget("remove_army_button", remove_army_button);
  remove_army_button->signal_clicked().connect (method(on_remove_army_clicked));
  xml->get_widget("army_vbox", army_vbox);
  // connect callbacks for the menu
  xml->get_widget("new_armyset_menuitem", new_armyset_menuitem);
  new_armyset_menuitem->signal_activate().connect (method(on_new_armyset_activated));
  xml->get_widget("load_armyset_menuitem", load_armyset_menuitem);
  load_armyset_menuitem->signal_activate().connect (method(on_load_armyset_activated));
  xml->get_widget("save_armyset_menuitem", save_armyset_menuitem);
  save_armyset_menuitem->signal_activate().connect (method(on_save_armyset_activated));
  xml->get_widget("save_as_menuitem", save_as_menuitem);
  save_as_menuitem->signal_activate().connect (method(on_save_as_activated));
  xml->get_widget("validate_armyset_menuitem", validate_armyset_menuitem);
  validate_armyset_menuitem->signal_activate().connect
    (method(on_validate_armyset_activated));
  xml->get_widget("quit_menuitem", quit_menuitem);
  quit_menuitem->signal_activate().connect (method(on_quit_activated));
  xml->get_widget("edit_armyset_info_menuitem", edit_armyset_info_menuitem);
  edit_armyset_info_menuitem->signal_activate().connect
    (method(on_edit_armyset_info_activated));
  xml->get_widget("edit_standard_picture_menuitem", edit_standard_picture_menuitem);
  edit_standard_picture_menuitem->signal_activate().connect
    (method(on_edit_standard_picture_activated));
  xml->get_widget("edit_bag_picture_menuitem", edit_bag_picture_menuitem);
  edit_bag_picture_menuitem->signal_activate().connect
    (method(on_edit_bag_picture_activated));
  xml->get_widget("edit_ship_picture_menuitem", edit_ship_picture_menuitem);
  edit_ship_picture_menuitem->signal_activate().connect
    (method(on_edit_ship_picture_activated));
  xml->get_widget ("help_about_menuitem", help_about_menuitem);
  help_about_menuitem->signal_activate().connect (method(on_help_about_activated));
  xml->get_widget ("tutorial_menuitem", tutorial_menuitem);
  tutorial_menuitem->signal_activate().connect (method(on_tutorial_video_activated));
  xml->get_widget("notebook", notebook);

  window->signal_delete_event().connect (sigc::hide(method(on_delete_event)));

  armies_list = Gtk::ListStore::create(armies_columns);
  armies_treeview->set_model(armies_list);
  armies_treeview->append_column("", armies_columns.name);
  armies_treeview->set_headers_visible(false);
  armies_treeview->get_selection()->signal_changed().connect(method(on_army_selected));
  armies_treeview->set_reorderable(true);

  if (load_filename != "")
    current_save_filename = load_filename;
  update_army_panel();
  update_armyset_buttons();
  update_armyset_menuitems();

  if (load_filename.empty() == false)
    {
      load_armyset (load_filename);
      update_armyset_buttons();
      update_armyset_menuitems();
      update_army_panel();
    }
  inhibit_scrolldown = true;
  //we only want inhibit scrolldown to be false when we click add, and we 
  //want the scrolledwindow to scroll down to the bottom.
  //the problem is that an army gets selected as a result of an add.
  //which is fine, but then there are also other times when an army gets
  //selected.  like the with the mouse.
  inhibit_updates = false;
}

void
ArmySetWindow::update_armyset_menuitems()
{
  if (d_armyset == NULL)
    {
      save_armyset_menuitem->set_sensitive(false);
      save_as_menuitem->set_sensitive(false);
      validate_armyset_menuitem->set_sensitive(false);
      edit_armyset_info_menuitem->set_sensitive(false);
      edit_standard_picture_menuitem->set_sensitive(false);
      edit_ship_picture_menuitem->set_sensitive(false);
      edit_bag_picture_menuitem->set_sensitive(false);
    }
  else
    {
      save_armyset_menuitem->set_sensitive(true);
      save_as_menuitem->set_sensitive(true);
      edit_armyset_info_menuitem->set_sensitive(true);
      edit_standard_picture_menuitem->set_sensitive(true);
      edit_ship_picture_menuitem->set_sensitive(true);
      edit_bag_picture_menuitem->set_sensitive(true);
      validate_armyset_menuitem->set_sensitive(true);
    }
}

void
ArmySetWindow::update_armyset_buttons()
{
  if (!armies_treeview->get_selection()->get_selected())
    remove_army_button->set_sensitive(false);
  else
    remove_army_button->set_sensitive(true);
  if (d_armyset == NULL)
    add_army_button->set_sensitive(false);
  else
    add_army_button->set_sensitive(true);
}

void
ArmySetWindow::update_army_panel()
{
  //if nothing selected in the treeview, then we don't show anything in
  //the army panel
  if (armies_treeview->get_selection()->get_selected() == 0)
    {
      //clear all values
      inhibit_needs_saving = true;
      name_entry->set_text("");
      description_textview->get_buffer()->set_text("");
      production_spinbutton->set_value(MIN_PRODUCTION_TURNS_FOR_ARMY_UNITS);
      cost_spinbutton->set_value(MIN_COST_FOR_ARMY_UNITS);
      new_cost_spinbutton->set_value(MIN_NEW_COST_FOR_ARMY_UNITS);
      upkeep_spinbutton->set_value(MIN_UPKEEP_FOR_ARMY_UNITS);
      strength_spinbutton->set_value(MIN_STRENGTH_FOR_ARMY_UNITS);
      moves_spinbutton->set_value(MIN_MOVES_FOR_ARMY_UNITS);
      exp_spinbutton->set_value(0);

      hero_combobox->set_active (0);
      awardable_switch->set_active(false);
      defends_ruins_switch->set_active(false);
      sight_spinbutton->set_value(0);
      id_spinbutton->set_value(0);
      move_forests_switch->set_active(false);
      move_marshes_switch->set_active(false);
      move_hills_switch->set_active(false);
      move_mountains_switch->set_active(false);
      can_fly_switch->set_active(false);
      add1strinopen_switch->set_active(false);
      add2strinopen_switch->set_active(false);
      add1strinforest_switch->set_active(false);
      add1strinhills_switch->set_active(false);
      add1strincity_switch->set_active(false);
      add2strincity_switch->set_active(false);
      add1stackinhills_switch->set_active(false);
      suballcitybonus_switch->set_active(false);
      sub1enemystack_switch->set_active(false);
      add1stack_switch->set_active(false);
      add2stack_switch->set_active(false);
      suballnonherobonus_switch->set_active(false);
      suballherobonus_switch->set_active(false);
      white_image->clear();
      green_image->clear();
      yellow_image->clear();
      light_blue_image->clear();
      red_image->clear();
      dark_blue_image->clear();
      orange_image->clear();
      black_image->clear();
      neutral_image->clear();
      army_vbox->set_sensitive(false);
      inhibit_needs_saving = false;
      return;
    }
  army_vbox->set_sensitive(true);
  Glib::RefPtr<Gtk::TreeSelection> selection = armies_treeview->get_selection();
  Gtk::TreeModel::iterator iterrow = selection->get_selected();

  if (iterrow) 
    {
      // Row selected
      Gtk::TreeModel::Row row = *iterrow;

      ArmyProto *a = row[armies_columns.army];
      inhibit_needs_saving = true;
      fill_army_info(a);
      inhibit_needs_saving = false;
    }
}

bool ArmySetWindow::on_delete_event()
{
  hide();
  return true;
}

void ArmySetWindow::on_new_armyset_activated()
{
  Glib::ustring name = "";
  int id = Armysetlist::getNextAvailableId(0);
  Armyset *armyset = new Armyset(id, name);
  ArmySetInfoDialog d(*window, armyset, 
                      File::getSetDir(Armyset::file_extension, false), "", 
                      false, _("Make a New Armyset"));
  int response = d.run();
  if (response != Gtk::RESPONSE_ACCEPT)
    {
      delete armyset;
      return;
    }
  inhibit_updates = true;
  armies_list->clear();
  inhibit_updates = false;
  if (d_armyset)
    delete d_armyset;
  d_armyset = armyset;
  Glib::ustring dir = File::getSetDir(Armyset::file_extension, false);
  d_armyset->setDirectory(dir);
  current_save_filename = d_armyset->getConfigurationFile();
  //here we put a copy into the armysetlist, and keep d_armyset as our
  //current working armyset.
  Armyset *copy = Armyset::copy (d_armyset);
  Glib::ustring new_basename = copy->getBaseName();
  guint32 new_id = copy->getId();
  if (!Armysetlist::getInstance()->addToPersonalCollection(copy, new_basename, new_id))
    delete copy;

  armies_treeview->set_cursor (Gtk::TreePath ("0"));
  update_army_panel();
  update_armyset_buttons();
  update_armyset_menuitems();

  needs_saving = true;
  update_window_title();
}

void ArmySetWindow::on_load_armyset_activated()
{
  Gtk::FileChooserDialog chooser(*window, 
				 _("Choose an Armyset to Load"));
  Glib::RefPtr<Gtk::FileFilter> lwa_filter = Gtk::FileFilter::create();
  lwa_filter->set_name(_("LordsAWar Armysets (*.lwa)"));
  lwa_filter->add_pattern("*" + ARMYSET_EXT);
  chooser.add_filter(lwa_filter);
  chooser.set_current_folder(File::getSetDir(Armyset::file_extension, false));

  chooser.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
  chooser.add_button(Gtk::Stock::OPEN, Gtk::RESPONSE_ACCEPT);
  chooser.set_default_response(Gtk::RESPONSE_ACCEPT);
      
  chooser.show_all();
  int res = chooser.run();

  if (res == Gtk::RESPONSE_ACCEPT)
    {
      load_armyset(chooser.get_filename());
      chooser.hide();
      needs_saving = false;
      update_window_title();
    }

  update_armyset_buttons();
  update_armyset_menuitems();
}

void ArmySetWindow::on_validate_armyset_activated()
{
  std::list<Glib::ustring> msgs;
  if (d_armyset == NULL)
    return;
  bool valid;
  valid = d_armyset->size() > 0;
  if (!valid)
    msgs.push_back(_("There must be at least one army unit in the armyset."));
  valid = d_armyset->validateHero();
  if (!valid)
    msgs.push_back(_("There must be at least one hero in the armyset."));
  valid = d_armyset->validatePurchasables();
  if (!valid)
    msgs.push_back(_("There must be at least one army unit with a production cost of more than zero."));
  valid = d_armyset->validateRuinDefenders();
  if (!valid)
    msgs.push_back(_("There must be at least one army unit than can defend a ruin."));
  valid = d_armyset->validateAwardables();
  if (!valid)
    msgs.push_back(_("There must be at least one army unit than can be awarded to a hero."));
  valid = d_armyset->validateShip();
  if (!valid)
    msgs.push_back(_("The ship image must be set."));
  valid = d_armyset->validateStandard();
  if (!valid)
    msgs.push_back(_("The hero's standard (the flag) image must be set."));
  valid = d_armyset->validateBag();
  if (!valid)
    msgs.push_back(_("The picture for the bag of items must be set."));

  for (Armyset::iterator it = d_armyset->begin(); it != d_armyset->end(); it++)
    {
      Shield::Colour c;
      valid = d_armyset->validateArmyUnitImage(*it, c);
      if (!valid)
	{
	  msgs.push_back(String::ucompose(_("%1 does not have an image set for the %2 player"), (*it)->getName(), Shield::colourToString(c)));
	  break;
	}
    }

  valid = d_armyset->validateArmyUnitNames();
  if (!valid)
    msgs.push_back(_("An army unit does not have a name."));

  Glib::ustring msg = "";
  for (std::list<Glib::ustring>::iterator it = msgs.begin(); it != msgs.end();
       it++)
    msg += (*it) + "\n";

  if (msg == "")
    msg = _("The armyset is valid.");
      
  Gtk::MessageDialog dialog(*window, msg);
  dialog.run();
  dialog.hide();
  return;
}

void ArmySetWindow::on_save_as_activated()
{
  guint32 suggested_tile_size = d_armyset->calculate_preferred_tile_size();
  if (suggested_tile_size != d_armyset->getTileSize())
    {
      TileSizeEditorDialog d(*window, d_armyset->getTileSize(), suggested_tile_size);
      int response = d.run();
      if (response == Gtk::RESPONSE_ACCEPT)
        d_armyset->setTileSize(d.get_selected_tilesize());
    }
  //Reorder the armyset according to the treeview
  d_armyset->clear();
  for (Gtk::TreeIter i = armies_list->children().begin(),
       end = armies_list->children().end(); i != end; ++i) 
    d_armyset->push_back((*i)[armies_columns.army]);

  Armyset *copy = Armyset::copy (d_armyset);
  copy->setId(Armysetlist::getNextAvailableId(d_armyset->getId()));
  ArmySetInfoDialog d(*window, copy, File::getSetDir(Armyset::file_extension, false), "", false,
                        _("Save a Copy of a Armyset"));
  int response = d.run();
  if (response == Gtk::RESPONSE_ACCEPT)
    {
      Glib::ustring new_basename = copy->getBaseName();
      guint32 new_id = copy->getId();
      copy->setDirectory(File::getSetDir(Armyset::file_extension, false));
      guint32 oldid = d_armyset->getId();
      Glib::ustring oldname = d_armyset->getName();
      Glib::ustring oldbasename = d_armyset->getBaseName();
      Glib::ustring olddir = d_armyset->getDirectory();

      Glib::ustring tmpdir = File::get_tmp_file();
      File::erase(tmpdir);
      tmpdir += Armyset::file_extension;
      File::create_dir(tmpdir);
      d_armyset->setName(copy->getName());
      File::copy(d_armyset->getConfigurationFile(),
                 File::getTempFile (tmpdir, copy->getBaseName() + Armyset::file_extension));
      d_armyset->setBaseName(copy->getBaseName());
      d_armyset->setDirectory(tmpdir);
      d_armyset->setId(copy->getId());

      current_save_filename = copy->getConfigurationFile();
      bool ok = Armysetlist::getInstance()->addToPersonalCollection(d_armyset, new_basename, new_id);
      File::erase(File::getTempFile (tmpdir, copy->getBaseName() + Armyset::file_extension));
      File::erase_dir(tmpdir);
      if (ok)
        {
          save_armyset_menuitem->set_sensitive(true);
          d_armyset = copy;
          //our tiles in the treeview are now out of date.
          refresh_armies(); //refresh them.
          needs_saving = false;
          update_window_title();
          armyset_saved.emit(d_armyset->getId());
        }
      else
        {
          d_armyset->setName(oldname);
          d_armyset->setBaseName(oldbasename);
          d_armyset->setId(oldid);
          d_armyset->setDirectory(olddir);
          Glib::ustring errmsg = Glib::strerror(errno);
          Glib::ustring msg;
          msg = _("Error!  Armyset could not be saved.");
          msg += "\n" + current_save_filename + "\n" + errmsg;
          Gtk::MessageDialog dialog(*window, msg);
          dialog.run();
          dialog.hide();
          delete copy;
        }
    }
  else
    delete copy;
}

bool ArmySetWindow::save_current_armyset()
{
  if (Playerlist::getInstance()->hasArmyset(d_armyset->getId()) == true &&
      d_armyset->validate() == false)
    {
      Glib::ustring errmsg = _("Armyset is invalid, and is also the current working armyset.");
      Glib::ustring msg;
      msg = _("Error!  Armyset could not be saved.");
      msg += "\n" + current_save_filename + "\n" +
        errmsg;
      Gtk::MessageDialog dialog(*window, msg);
      dialog.run();
      dialog.hide();
      return false;
    }
  if (current_save_filename.empty())
    current_save_filename = d_armyset->getConfigurationFile();
  
  guint32 suggested_tile_size = d_armyset->calculate_preferred_tile_size();
  if (suggested_tile_size != d_armyset->getTileSize())
    {
      TileSizeEditorDialog d(*window, d_armyset->getTileSize(), suggested_tile_size);
      int response = d.run();
      if (response == Gtk::RESPONSE_ACCEPT)
        d_armyset->setTileSize(d.get_selected_tilesize());
    }
  //Reorder the armyset according to the treeview
  d_armyset->clear();
  for (Gtk::TreeIter i = armies_list->children().begin(),
       end = armies_list->children().end(); i != end; ++i) 
    d_armyset->push_back((*i)[armies_columns.army]);

  bool success = d_armyset->save(current_save_filename, Armyset::file_extension);
  if (success)
    {
      if (Armysetlist::getInstance()->reload(d_armyset->getId()))
        refresh_armies();
      needs_saving = false;
      update_window_title();
      armyset_saved.emit(d_armyset->getId());
    }
  else
    {
      Glib::ustring errmsg = Glib::strerror(errno);
      Glib::ustring msg;
      msg = _("Error!  Armyset could not be saved.");
      msg += "\n" + current_save_filename + "\n" +
        errmsg;
      Gtk::MessageDialog dialog(*window, msg);
      dialog.run();
      dialog.hide();
    }
  return success;
}

void ArmySetWindow::on_save_armyset_activated()
{
  save_current_armyset();
}

void ArmySetWindow::on_edit_ship_picture_activated()
{
  Glib::ustring filename = "";
  if (d_armyset->getShipImageName() != "")
    filename = d_armyset->getFileFromConfigurationFile(d_armyset->getShipImageName() +".png");
  MaskedImageEditorDialog d(*window, filename);
  d.set_title(_("Select a Ship image"));
  int response = d.run();
  if (response == Gtk::RESPONSE_ACCEPT)
    {
      if (d.get_selected_filename() != filename)
        {
          File::erase(filename);
          Glib::ustring file = File::get_basename(d.get_selected_filename());
          if (d_armyset->replaceFileInConfigurationFile(d_armyset->getShipImageName()+".png", d.get_selected_filename()))
            {
              d_armyset->setShipImageName(file);
              needs_saving = true;
              update_window_title();
            }
          else
            show_add_file_error(d_armyset, *d.get_dialog(), file);
        }
    }
}

void ArmySetWindow::on_edit_standard_picture_activated()
{
  Glib::ustring filename = "";
  if (d_armyset->getStandardImageName() != "")
    filename = d_armyset->getFileFromConfigurationFile(d_armyset->getStandardImageName() +".png");
  MaskedImageEditorDialog d(*window, filename);
  d.set_title(_("Select a Hero Flag image"));
  int response = d.run();
  if (response == Gtk::RESPONSE_ACCEPT)
    {
      if (d.get_selected_filename() != filename)
        {
          File::erase(filename);
          Glib::ustring file = File::get_basename(d.get_selected_filename());
          if (d_armyset->replaceFileInConfigurationFile(d_armyset->getStandardImageName()+".png", d.get_selected_filename()))
            {
              d_armyset->setStandardImageName(file);
              needs_saving = true;
              update_window_title();
            }
          else
            show_add_file_error(d_armyset, *d.get_dialog(), file);
        }
    }
}

void ArmySetWindow::on_edit_bag_picture_activated()
{
  Glib::ustring filename = "";
  if (d_armyset->getBagImageName().empty() == false)
    filename = d_armyset->getFileFromConfigurationFile(d_armyset->getBagImageName() +".png");
  ImageEditorDialog d(*window, filename, 1);
  d.set_title(_("Select a Bag image"));
  int response = d.run();
  if (response == Gtk::RESPONSE_ACCEPT)
    {
      if (d.get_selected_filename() != filename)
        {
          File::erase(filename);
          Glib::ustring file = File::get_basename(d.get_selected_filename());
          if (d_armyset->replaceFileInConfigurationFile(d_armyset->getBagImageName()+".png", d.get_selected_filename()))
            {
              d_armyset->setBagImageName(file);
              needs_saving = true;
              update_window_title();
            }
          else
            show_add_file_error(d_armyset, *d.get_dialog(), file);
        }
    }
}

void ArmySetWindow::on_edit_armyset_info_activated()
{
  ArmySetInfoDialog d(*window, d_armyset, 
                      File::get_dirname(current_save_filename), 
                      File::get_basename(current_save_filename), true, 
                      _("Edit Armyset Information"));
  int response = d.run();
  if (response == Gtk::RESPONSE_ACCEPT)
    {
      needs_saving = true;
      update_window_title();
    }
}

void ArmySetWindow::on_help_about_activated()
{
  Gtk::AboutDialog* dialog;

  Glib::RefPtr<Gtk::Builder> xml
    = Gtk::Builder::create_from_file(File::getGladeFile("about-dialog.ui"));

  xml->get_widget("dialog", dialog);
  dialog->set_transient_for(*window);
  dialog->set_icon_from_file(File::getVariousFile("castle_icon.png"));

  dialog->set_version(PACKAGE_VERSION);
  dialog->set_logo(ImageCache::loadMiscImage("castle_icon.png")->to_pixbuf());
  dialog->show_all();
  dialog->run();
  delete dialog;

  return;
}

void ArmySetWindow::addArmyType(guint32 army_type)
{
  ArmyProto *a;
  //go get army_type in d_armyset
  a = d_armyset->lookupArmyByType(army_type);
  Gtk::TreeIter i = armies_list->append();
  (*i)[armies_columns.name] = a->getName();
  (*i)[armies_columns.army] = a;
}

void ArmySetWindow::on_army_selected()
{
  if (inhibit_updates)
    return;
  update_armyset_buttons();
  update_army_panel();
  armies_treeview->queue_draw();
  if (inhibit_scrolldown == false)
    {
      while (g_main_context_iteration(NULL, FALSE)); //doEvents
      armies_scrolledwindow->get_vadjustment()->set_value
        (armies_scrolledwindow->get_vadjustment()->get_upper());
    }
  inhibit_scrolldown = true;
}

void ArmySetWindow::fill_army_image(Gtk::Button *button, Gtk::Image *image, Shield::Colour c, ArmyProto *army)
{
  if (army->getImageName(c) != "")
    {
      bool broken = false;
    
      Glib::ustring filename = "";
      filename = d_armyset->getFileFromConfigurationFile(army->getImageName(c) +".png");
      if (filename != "")
        {
          Gdk::RGBA colour = Shieldsetlist::getInstance()->getColor(1, c);
          army->instantiateImages(d_armyset->getTileSize(), c, filename, 
                                  broken);
          PixMask *army_image = ImageCache::applyMask(army->getImage(c), 
                                                      army->getMask(c), colour);
          image->property_pixbuf() = army_image->to_pixbuf();
          delete army_image;
          File::erase(filename);

          button->set_label(army->getImageName(c) + ".png");
        }
    }
  else
    {
      button->set_label(_("no image set"));
      image->clear();
    }
}

void ArmySetWindow::fill_army_info(ArmyProto *army)
{
  fill_army_image(white_image_button, white_image, Shield::WHITE, army);
  fill_army_image(green_image_button, green_image, Shield::GREEN, army);
  fill_army_image(yellow_image_button, yellow_image, Shield::YELLOW, army);
  fill_army_image(light_blue_image_button, light_blue_image, 
                  Shield::LIGHT_BLUE, army);
  fill_army_image(red_image_button, red_image, Shield::RED, army);
  fill_army_image(dark_blue_image_button, dark_blue_image, Shield::DARK_BLUE, 
                  army);
  fill_army_image(orange_image_button, orange_image, Shield::ORANGE, army);
  fill_army_image(black_image_button, black_image, Shield::BLACK, army);
  fill_army_image(neutral_image_button, neutral_image, Shield::NEUTRAL, army);
  name_entry->set_text(army->getName());
  name_entry->set_max_length (MAX_LENGTH_FOR_ARMY_NAME);
  description_textview->get_buffer()->set_text(army->getDescription());
  double turns = army->getProduction();
  production_spinbutton->set_value(turns);
  cost_spinbutton->set_value(army->getProductionCost());
  new_cost_spinbutton->set_value(army->getNewProductionCost());
  upkeep_spinbutton->set_value(army->getUpkeep());
  strength_spinbutton->set_value(army->getStrength());
  moves_spinbutton->set_value(army->getMaxMoves());
  exp_spinbutton->set_value(int(army->getXpReward()));
  hero_combobox->set_active(int(army->getGender()));
  awardable_switch->set_active(army->getAwardable());
  defends_ruins_switch->set_active(army->getDefendsRuins());
  sight_spinbutton->set_value(army->getSight());
  id_spinbutton->set_value(army->getId());

  guint32 bonus = army->getMoveBonus();
  can_fly_switch->set_active (bonus == 
				   (Tile::GRASS | Tile::WATER | 
				    Tile::FOREST | Tile::HILLS | 
				    Tile::MOUNTAIN | Tile::SWAMP));
  if (can_fly_switch->get_active() == false)
    {
      move_forests_switch->set_active
	((bonus & Tile::FOREST) == Tile::FOREST);
      move_marshes_switch->set_active
	((bonus & Tile::SWAMP) == Tile::SWAMP);
      move_hills_switch->set_active
	((bonus & Tile::HILLS) == Tile::HILLS);
      move_mountains_switch->set_active
	((bonus & Tile::MOUNTAIN) == Tile::MOUNTAIN);
    }
  else
    {
      move_forests_switch->set_active(false);
      move_marshes_switch->set_active(false);
      move_hills_switch->set_active(false);
      move_mountains_switch->set_active(false);
    }
  bonus = army->getArmyBonus();
  add1strinopen_switch->set_active
    ((bonus & Army::ADD1STRINOPEN) == Army::ADD1STRINOPEN);
  add2strinopen_switch->set_active
    ((bonus & Army::ADD2STRINOPEN) == Army::ADD2STRINOPEN);
  add1strinforest_switch->set_active
    ((bonus & Army::ADD1STRINFOREST) == Army::ADD1STRINFOREST);
  add1strinhills_switch->set_active
    ((bonus & Army::ADD1STRINHILLS) == Army::ADD1STRINHILLS);
  add1strincity_switch->set_active
    ((bonus & Army::ADD1STRINCITY) == Army::ADD1STRINCITY);
  add2strincity_switch->set_active
    ((bonus & Army::ADD2STRINCITY) == Army::ADD2STRINCITY);
  add1stackinhills_switch->set_active
    ((bonus & Army::ADD1STACKINHILLS) == Army::ADD1STACKINHILLS);
  suballcitybonus_switch->set_active
    ((bonus & Army::SUBALLCITYBONUS) == Army::SUBALLCITYBONUS);
  sub1enemystack_switch->set_active
    ((bonus & Army::SUB1ENEMYSTACK) == Army::SUB1ENEMYSTACK);
  add1stack_switch->set_active
    ((bonus & Army::ADD1STACK) == Army::ADD1STACK);
  add2stack_switch->set_active
    ((bonus & Army::ADD2STACK) == Army::ADD2STACK);
  suballnonherobonus_switch->set_active
    ((bonus & Army::SUBALLNONHEROBONUS) == Army::SUBALLNONHEROBONUS);
  suballherobonus_switch->set_active
    ((bonus & Army::SUBALLHEROBONUS) == Army::SUBALLHEROBONUS);
}

void ArmySetWindow::on_name_changed()
{
  Glib::RefPtr<Gtk::TreeSelection> selection = armies_treeview->get_selection();
  Gtk::TreeModel::iterator iterrow = selection->get_selected();

  if (iterrow) 
    {
      Gtk::TreeModel::Row row = *iterrow;
      ArmyProto *a = row[armies_columns.army];
      a->setName(name_entry->get_text());
      row[armies_columns.name] = name_entry->get_text();
      if (inhibit_needs_saving == false)
        {
          needs_saving = true;
          update_window_title();
        }
    }
}

void ArmySetWindow::on_description_changed()
{
  Glib::RefPtr<Gtk::TreeSelection> selection = armies_treeview->get_selection();
  Gtk::TreeModel::iterator iterrow = selection->get_selected();

  if (iterrow) 
    {
      Gtk::TreeModel::Row row = *iterrow;
      ArmyProto *a = row[armies_columns.army];
      a->setDescription(description_textview->get_buffer()->get_text());
      if (inhibit_needs_saving == false)
        {
          needs_saving = true;
          update_window_title();
        }
    }
}

void ArmySetWindow::on_image_changed(Gtk::Button *button, Gtk::Image *image, Shield::Colour c)
{
  Glib::RefPtr<Gtk::TreeSelection> selection = armies_treeview->get_selection();
  Gtk::TreeModel::iterator iterrow = selection->get_selected();

  if (iterrow) 
    {
      Gtk::TreeModel::Row row = *iterrow;
      ArmyProto *a = row[armies_columns.army];
      Glib::ustring filename = "";
      if (a->getImageName(c) != "")
        filename = d_armyset->getFileFromConfigurationFile(a->getImageName(c) + ".png");
      MaskedImageEditorDialog d(*window, filename);
      d.set_title(String::ucompose(_("Select a %1 Army image"), 
                                   Shield::colourToFriendlyName(c)));
      int response = d.run();
      if (response == Gtk::RESPONSE_ACCEPT)
        {
          if (d.get_selected_filename() != "" && d.get_selected_filename() != filename)
            {
              File::erase(filename);
              bool broken = false;
              Glib::ustring file =File::get_basename(d.get_selected_filename());
              if (d_armyset->replaceFileInConfigurationFile(a->getImageName(c)+".png", d.get_selected_filename()))
                {
                  a->setImageName(c, file);
                  Gdk::RGBA colour = Shieldsetlist::getInstance()->getColor(1, c);
                  a->instantiateImages(d_armyset->getTileSize(), c, d.get_selected_filename(), broken);

                  PixMask *army_image = ImageCache::applyMask(a->getImage(c), 
                                                              a->getMask(c), 
                                                              colour);
                  image->property_pixbuf() = army_image->to_pixbuf();
                  delete army_image;

                  if (inhibit_needs_saving == false)
                    {
                      needs_saving = true;
                      update_window_title();
                    }
                }
              else
                show_add_file_error(d_armyset, *d.get_dialog(), file);
            }
        }
      if (a->getImageName(c) == "")
        button->set_label(_("no image set"));
      else
        button->set_label(a->getImageName(c) + ".png");
    }
}

void ArmySetWindow::on_production_text_changed()
{
  production_spinbutton->set_value(atoi(production_spinbutton->get_text().c_str()));
  on_production_changed();
}

void ArmySetWindow::on_production_changed()
{
  Glib::RefPtr<Gtk::TreeSelection> selection = armies_treeview->get_selection();
  Gtk::TreeModel::iterator iterrow = selection->get_selected();

  if (iterrow) 
    {
      Gtk::TreeModel::Row row = *iterrow;
      ArmyProto *a = row[armies_columns.army];
      if (production_spinbutton->get_value() < 
	  MIN_PRODUCTION_TURNS_FOR_ARMY_UNITS)
	production_spinbutton->set_value(MIN_PRODUCTION_TURNS_FOR_ARMY_UNITS);
      else if (production_spinbutton->get_value() > 
	       MAX_PRODUCTION_TURNS_FOR_ARMY_UNITS)
	production_spinbutton->set_value(MAX_PRODUCTION_TURNS_FOR_ARMY_UNITS);
      else
	a->setProduction(int(production_spinbutton->get_value()));
      if (inhibit_needs_saving == false)
        {
          needs_saving = true;
          update_window_title();
        }
    }
}

void ArmySetWindow::on_cost_text_changed()
{
  cost_spinbutton->set_value(atoi(cost_spinbutton->get_text().c_str()));
  on_cost_changed();
}

void ArmySetWindow::on_cost_changed()
{
  Glib::RefPtr<Gtk::TreeSelection> selection = armies_treeview->get_selection();
  Gtk::TreeModel::iterator iterrow = selection->get_selected();

  if (iterrow) 
    {
      Gtk::TreeModel::Row row = *iterrow;
      ArmyProto *a = row[armies_columns.army];
      if (cost_spinbutton->get_value() < MIN_COST_FOR_ARMY_UNITS)
	cost_spinbutton->set_value(MIN_COST_FOR_ARMY_UNITS);
      else if (strength_spinbutton->get_value() > MAX_COST_FOR_ARMY_UNITS)
	cost_spinbutton->set_value(MAX_COST_FOR_ARMY_UNITS);
      else
	a->setProductionCost(int(cost_spinbutton->get_value()));
      if (inhibit_needs_saving == false)
        {
          needs_saving = true;
          update_window_title();
        }
    }
}

void ArmySetWindow::on_new_cost_text_changed()
{
  new_cost_spinbutton->set_value(atoi(new_cost_spinbutton->get_text().c_str()));
  on_new_cost_changed();
}

void ArmySetWindow::on_new_cost_changed()
{
  Glib::RefPtr<Gtk::TreeSelection> selection = armies_treeview->get_selection();
  Gtk::TreeModel::iterator iterrow = selection->get_selected();

  if (iterrow) 
    {
      Gtk::TreeModel::Row row = *iterrow;
      ArmyProto *a = row[armies_columns.army];
      a->setNewProductionCost(int(new_cost_spinbutton->get_value()));
      if (inhibit_needs_saving == false)
        {
          needs_saving = true;
          update_window_title();
        }
    }
}

void ArmySetWindow::on_upkeep_text_changed()
{
  upkeep_spinbutton->set_value(atoi(upkeep_spinbutton->get_text().c_str()));
  on_upkeep_changed();
}

void ArmySetWindow::on_upkeep_changed()
{
  Glib::RefPtr<Gtk::TreeSelection> selection = armies_treeview->get_selection();
  Gtk::TreeModel::iterator iterrow = selection->get_selected();

  if (iterrow) 
    {
      Gtk::TreeModel::Row row = *iterrow;
      ArmyProto  *a = row[armies_columns.army];
      if (upkeep_spinbutton->get_value() < MIN_UPKEEP_FOR_ARMY_UNITS)
	upkeep_spinbutton->set_value(MIN_UPKEEP_FOR_ARMY_UNITS);
      else if (upkeep_spinbutton->get_value() > MAX_UPKEEP_FOR_ARMY_UNITS)
	upkeep_spinbutton->set_value(MAX_UPKEEP_FOR_ARMY_UNITS);
      else
	a->setUpkeep(int(upkeep_spinbutton->get_value()));
      if (inhibit_needs_saving == false)
        {
          needs_saving = true;
          update_window_title();
        }
    }
}

void ArmySetWindow::on_strength_text_changed()
{
  strength_spinbutton->set_value(atoi(strength_spinbutton->get_text().c_str()));
  on_strength_changed();
}

void ArmySetWindow::on_strength_changed()
{
  Glib::RefPtr<Gtk::TreeSelection> selection = armies_treeview->get_selection();
  Gtk::TreeModel::iterator iterrow = selection->get_selected();

  if (iterrow) 
    {
      Gtk::TreeModel::Row row = *iterrow;
      ArmyProto *a = row[armies_columns.army];
      if (strength_spinbutton->get_value() < MIN_STRENGTH_FOR_ARMY_UNITS)
	strength_spinbutton->set_value(MIN_STRENGTH_FOR_ARMY_UNITS);
      else if (strength_spinbutton->get_value() > MAX_STRENGTH_FOR_ARMY_UNITS)
	strength_spinbutton->set_value(MAX_STRENGTH_FOR_ARMY_UNITS);
      else
	a->setStrength(int(strength_spinbutton->get_value()));
      if (inhibit_needs_saving == false)
        {
          needs_saving = true;
          update_window_title();
        }
    }
}

void ArmySetWindow::on_moves_text_changed()
{
  moves_spinbutton->set_value(atoi(moves_spinbutton->get_text().c_str()));
  on_moves_changed();
}

void ArmySetWindow::on_moves_changed()
{
  Glib::RefPtr<Gtk::TreeSelection> selection = armies_treeview->get_selection();
  Gtk::TreeModel::iterator iterrow = selection->get_selected();

  if (iterrow) 
    {
      Gtk::TreeModel::Row row = *iterrow;
      ArmyProto *a = row[armies_columns.army];
      if (moves_spinbutton->get_value() < MIN_MOVES_FOR_ARMY_UNITS)
	moves_spinbutton->set_value(MIN_MOVES_FOR_ARMY_UNITS);
      else if (moves_spinbutton->get_value() > MAX_MOVES_FOR_ARMY_UNITS)
	moves_spinbutton->set_value(MAX_MOVES_FOR_ARMY_UNITS);
      else
	a->setMaxMoves(int(moves_spinbutton->get_value()));
      if (inhibit_needs_saving == false)
        {
          needs_saving = true;
          update_window_title();
        }
    }
}

void ArmySetWindow::on_exp_text_changed()
{
  exp_spinbutton->set_value(atoi(exp_spinbutton->get_text().c_str()));
  on_exp_changed();
}

void ArmySetWindow::on_exp_changed()
{
  Glib::RefPtr<Gtk::TreeSelection> selection = armies_treeview->get_selection();
  Gtk::TreeModel::iterator iterrow = selection->get_selected();

  if (iterrow) 
    {
      Gtk::TreeModel::Row row = *iterrow;
      ArmyProto *a = row[armies_columns.army];
      a->setXpReward(int(exp_spinbutton->get_value()));
      if (inhibit_needs_saving == false)
        {
          needs_saving = true;
          update_window_title();
        }
    }
}

void ArmySetWindow::on_sight_text_changed()
{
  sight_spinbutton->set_value(atoi(sight_spinbutton->get_text().c_str()));
  on_sight_changed();
}

void ArmySetWindow::on_sight_changed()
{
  Glib::RefPtr<Gtk::TreeSelection> selection = armies_treeview->get_selection();
  Gtk::TreeModel::iterator iterrow = selection->get_selected();

  if (iterrow) 
    {
      Gtk::TreeModel::Row row = *iterrow;
      ArmyProto *a = row[armies_columns.army];
      a->setSight(int(sight_spinbutton->get_value()));
      if (inhibit_needs_saving == false)
        {
          needs_saving = true;
          update_window_title();
        }
    }
}

void ArmySetWindow::on_id_text_changed()
{
  id_spinbutton->set_value(atoi(id_spinbutton->get_text().c_str()));
  on_sight_changed();
}

void ArmySetWindow::on_id_changed()
{
  Glib::RefPtr<Gtk::TreeSelection> selection = armies_treeview->get_selection();
  Gtk::TreeModel::iterator iterrow = selection->get_selected();

  if (iterrow) 
    {
      Gtk::TreeModel::Row row = *iterrow;
      ArmyProto *a = row[armies_columns.army];
      a->setId(int(id_spinbutton->get_value()));
      if (inhibit_needs_saving == false)
        {
          needs_saving = true;
          update_window_title();
        }
    }
}

void ArmySetWindow::on_hero_combobox_changed()
{
  Glib::RefPtr<Gtk::TreeSelection> selection = armies_treeview->get_selection();
  Gtk::TreeModel::iterator iterrow = selection->get_selected();

  if (iterrow) 
    {
      Gtk::TreeModel::Row row = *iterrow;
      ArmyProto *a = row[armies_columns.army];
      a->setGender(Hero::Gender(hero_combobox->get_active_row_number()));
      if (inhibit_needs_saving == false)
        {
          needs_saving = true;
          update_window_title();
        }
    }
}

void ArmySetWindow::on_awardable_toggled()
{
  Glib::RefPtr<Gtk::TreeSelection> selection = armies_treeview->get_selection();
  Gtk::TreeModel::iterator iterrow = selection->get_selected();

  if (iterrow) 
    {
      Gtk::TreeModel::Row row = *iterrow;
      ArmyProto *a = row[armies_columns.army];
      a->setAwardable(awardable_switch->get_active());
      if (inhibit_needs_saving == false)
        {
          needs_saving = true;
          update_window_title();
        }
    }
}

void ArmySetWindow::on_defends_ruins_toggled()
{
  Glib::RefPtr<Gtk::TreeSelection> selection = armies_treeview->get_selection();
  Gtk::TreeModel::iterator iterrow = selection->get_selected();

  if (iterrow) 
    {
      Gtk::TreeModel::Row row = *iterrow;
      ArmyProto *a = row[armies_columns.army];
      a->setDefendsRuins(defends_ruins_switch->get_active());
      if (inhibit_needs_saving == false)
        {
          needs_saving = true;
          update_window_title();
        }
    }
}

void ArmySetWindow::on_movebonus_toggled(Gtk::Switch *button, guint32 val)
{
  Glib::RefPtr<Gtk::TreeSelection> selection = armies_treeview->get_selection();
  Gtk::TreeModel::iterator iterrow = selection->get_selected();

  if (iterrow) 
    {
      Gtk::TreeModel::Row row = *iterrow;
      ArmyProto *a = row[armies_columns.army];
      guint32 bonus = a->getMoveBonus();
      if (button->get_active() == true)
	  bonus |= val;
      else
	{
	  if (bonus & val)
	    bonus ^= val;
	}
      a->setMoveBonus(bonus);
      if (inhibit_needs_saving == false)
        {
          needs_saving = true;
          update_window_title();
        }
    }
}

void ArmySetWindow::on_armybonus_toggled(Gtk::Switch *button, guint32 val)
{
  Glib::RefPtr<Gtk::TreeSelection> selection = armies_treeview->get_selection();
  Gtk::TreeModel::iterator iterrow = selection->get_selected();

  if (iterrow) 
    {
      Gtk::TreeModel::Row row = *iterrow;
      ArmyProto *a = row[armies_columns.army];
      guint32 bonus = a->getArmyBonus();
      if (button->get_active() == true)
	bonus |= val;
      else
	{
	  if (bonus & val)
	    bonus ^= val;
	}
      a->setArmyBonus(bonus);
      if (inhibit_needs_saving == false)
        {
          needs_saving = true;
          update_window_title();
        }
    }
}

void ArmySetWindow::on_add_army_clicked()
{
  inhibit_scrolldown=false;
  //add a new empty army to the armyset
  ArmyProto *a = new ArmyProto();
  //add it to the treeview
  Gtk::TreeIter i = armies_list->append();
  a->setName(_("Untitled"));
  (*i)[armies_columns.name] = a->getName();
  (*i)[armies_columns.army] = a;
  if (d_armyset->empty() == true)
    a->setId(0);
  else
    a->setId(d_armyset->getMaxId() + 1);
  d_armyset->push_back(a);
  needs_saving = true;
  update_window_title();
  if (d_armyset->empty() == false)
    armies_treeview->get_selection()->select(i);
}

void ArmySetWindow::on_remove_army_clicked()
{
  //erase the selected row from the treeview
  //remove the army from the armyset
  Glib::RefPtr<Gtk::TreeSelection> selection = armies_treeview->get_selection();
  Gtk::TreeModel::iterator iterrow = selection->get_selected();

  if (iterrow) 
    {
      Gtk::TreeModel::Row row = *iterrow;
      ArmyProto *a = row[armies_columns.army];
      armies_list->erase(iterrow);
      d_armyset->remove(a);
      needs_saving = true;
      update_window_title();
    }
}

bool ArmySetWindow::load_armyset(Glib::ustring filename)
{
  inhibit_scrolldown=true;
  Glib::ustring old_current_save_filename = current_save_filename;
  current_save_filename = filename;

  Glib::ustring name = File::get_basename(filename);

  bool unsupported_version;
  Armyset *armyset = Armyset::create(current_save_filename, unsupported_version);
  if (armyset == NULL)
    {
      Glib::ustring msg;
      if (unsupported_version)
        msg = _("Error!  The version of the armyset is unsupported.");
      else
        msg = _("Error!  Armyset could not be loaded.");
      Gtk::MessageDialog dialog(*window, msg);
      current_save_filename = old_current_save_filename;
      dialog.run();
      dialog.hide();
      return false;
    }
  armies_list->clear();
  if (d_armyset)
    delete d_armyset;
  d_armyset = armyset;
  bool broken = false;
  d_armyset->instantiateImages(broken);
  for (Armyset::iterator i = d_armyset->begin(); i != d_armyset->end(); ++i)
    addArmyType((*i)->getId());
  if (d_armyset->empty() == false)
    {
      Gtk::TreeModel::Row row;
      row = armies_treeview->get_model()->children()[0];
      if(row)
	armies_treeview->get_selection()->select(row);
    }
  needs_saving = false;
  update_window_title();
  inhibit_scrolldown=false;
  return true;
}

bool ArmySetWindow::quit()
{
  if (needs_saving == true)
    {
      EditorQuitDialog d(*window);
      int response = d.run_and_hide();
      
      if (response == Gtk::RESPONSE_CANCEL) //we don't want to quit
	return false;

      else if (response == Gtk::RESPONSE_ACCEPT) // save and quit
        {
          if (save_current_armyset() == false)
            return false;
        }
      //else if (Response == Gtk::CLOSE) // don't save just quit
      window->hide();
    }
  else
    window->hide();
  if (d_armyset)
    delete d_armyset;
  return true;
}

bool ArmySetWindow::on_window_closed(GdkEventAny*)
{
  return !quit();
}

void ArmySetWindow::on_quit_activated()
{
  quit();
}

void ArmySetWindow::update_window_title()
{
  Glib::ustring title = "";
  if (needs_saving)
    title += "*";
  title += File::get_basename(current_save_filename, true);
  title += " - ";
  title += _("Armyset Editor");
  window->set_title(title);
}
    
void ArmySetWindow::on_make_same_clicked()
{
  Glib::RefPtr<Gtk::TreeSelection> selection = armies_treeview->get_selection();
  Gtk::TreeModel::iterator iterrow = selection->get_selected();
  Gtk::TreeModel::Row row = *iterrow;
  if (!row)
    return;
  ArmyProto *a = row[armies_columns.army];
  if (!a)
    return;
  if (a->getImageName(Shield::WHITE).empty())
    return;
  //get the image for white and then transfer it to the rest.
  green_image_button->set_label(white_image_button->get_label());
  yellow_image_button->set_label(white_image_button->get_label());
  light_blue_image_button->set_label(white_image_button->get_label());
  red_image_button->set_label(white_image_button->get_label());
  dark_blue_image_button->set_label(white_image_button->get_label());
  orange_image_button->set_label(white_image_button->get_label());
  black_image_button->set_label(white_image_button->get_label());
  neutral_image_button->set_label(white_image_button->get_label());
        
  Glib::ustring white_filename = d_armyset->getFileFromConfigurationFile(a->getImageName(Shield::Colour(0)) + ".png");
  Gtk::Image *images[MAX_PLAYERS+1];
  images[0] = NULL;
  images[1] = green_image;
  images[2] = yellow_image;
  images[3] = light_blue_image;
  images[4] = red_image;
  images[5] = dark_blue_image;
  images[6] = orange_image;
  images[7] = black_image;
  images[8] = neutral_image;
  for (unsigned int i = Shield::GREEN; i <= Shield::NEUTRAL; i++)
    {
      Shield::Colour s = Shield::Colour(i);
      Glib::ustring filename = d_armyset->getFileFromConfigurationFile(a->getImageName(s) + ".png");
      File::erase(filename);
      Glib::ustring file = File::get_basename(white_filename);
      if (d_armyset->replaceFileInConfigurationFile(a->getImageName(s)+".png", white_filename))
        {
          a->setImageName(s, file);
          bool broken = false;
          Gdk::RGBA colour = Shieldsetlist::getInstance()->getColor(1, s);
          a->instantiateImages(d_armyset->getTileSize(), s, white_filename, broken);
          PixMask *army_image = ImageCache::applyMask(a->getImage(s), 
                                                      a->getMask(s), colour);
          images[i]->property_pixbuf() = army_image->to_pixbuf();
          delete army_image;
        }
      else
        {
          show_add_file_error(d_armyset, *window, file);
          break;
        }
    }
}

void ArmySetWindow::show_add_file_error(Armyset *a, Gtk::Window &d, Glib::ustring file)
{
  Glib::ustring m = String::ucompose(_("Couldn't add %1.png to:\n%2"), file, 
                                     a->getConfigurationFile());
  Gtk::MessageDialog td(d, m);
  td.run();
  td.hide();
}

void ArmySetWindow::refresh_armies()
{
  Armyset::iterator j = d_armyset->begin();
  for (Gtk::TreeNodeChildren::iterator i = armies_list->children().begin();
       i != armies_list->children().end(); i++, j++)
    (*i)[armies_columns.army] = *j;
}

ArmySetWindow::~ArmySetWindow()
{
  notebook->property_show_tabs () = false;
  delete window;
}

void ArmySetWindow::on_tutorial_video_activated()
{
  GError *errs = NULL;
  gtk_show_uri(window->get_screen()->gobj(), 
               "http://vimeo.com/97927589", 0, &errs);
  return;
}
