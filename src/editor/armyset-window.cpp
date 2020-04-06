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
#include "editor-quit-dialog.h"
#include "editor-save-changes-dialog.h"

#include "ucompose.hpp"

#include "image-editor-dialog.h"
#include "playerlist.h"
#include "GameMap.h"
#include "font-size.h"

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
    (sigc::bind(method(on_image_changed), Shield::WHITE));
  xml->get_widget("green_image_button", green_image_button);
  green_image_button->signal_clicked().connect
    (sigc::bind(method(on_image_changed), Shield::GREEN));
  xml->get_widget("yellow_image_button", yellow_image_button);
  yellow_image_button->signal_clicked().connect
    (sigc::bind(method(on_image_changed), Shield::YELLOW));
  xml->get_widget("light_blue_image_button", light_blue_image_button);
  light_blue_image_button->signal_clicked().connect
    (sigc::bind(method(on_image_changed), Shield::LIGHT_BLUE));
  xml->get_widget("red_image_button", red_image_button);
  red_image_button->signal_clicked().connect
    (sigc::bind(method(on_image_changed), Shield::RED));
  xml->get_widget("dark_blue_image_button", dark_blue_image_button);
  dark_blue_image_button->signal_clicked().connect
    (sigc::bind(method(on_image_changed), Shield::DARK_BLUE));
  xml->get_widget("orange_image_button", orange_image_button);
  orange_image_button->signal_clicked().connect
    (sigc::bind(method(on_image_changed), Shield::ORANGE));
  xml->get_widget("black_image_button", black_image_button);
  black_image_button->signal_clicked().connect
    (sigc::bind(method(on_image_changed), Shield::BLACK));
  xml->get_widget("neutral_image_button", neutral_image_button);
  neutral_image_button->signal_clicked().connect
    (sigc::bind(method(on_image_changed), Shield::NEUTRAL));
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

  armies_list = Gtk::ListStore::create(armies_columns);
  armies_treeview->set_model(armies_list);
  armies_treeview->append_column("", armies_columns.name);
  armies_treeview->set_headers_visible(false);
  armies_treeview->get_selection()->signal_changed().connect(method(on_army_selected));
  armies_treeview->set_reorderable(true);
  armies_list->signal_row_inserted().connect(sigc::hide(sigc::hide(method(on_army_moved))));

  if (load_filename != "")
    current_save_filename = load_filename;
  update_army_panel();
  update_armyset_buttons();

  if (load_filename.empty() == false)
    {
      load_armyset (load_filename);
      update_armyset_buttons();
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

void ArmySetWindow::on_new_armyset_activated()
{
  make_new_armyset ();
}

bool ArmySetWindow::make_new_armyset ()
{
  Glib::ustring msg = _("Save these changes before making a new armyset?");
  if (check_discard (msg) == false)
    return false;
  save_armyset_menuitem->set_sensitive (false);
  current_save_filename = "";
  if (d_armyset)
    delete d_armyset;

  guint32 num = 0;
  Glib::ustring name =
    Armysetlist::getInstance()->findFreeName(_("Untitled"), 100, num,
                                             Armyset::get_default_tile_size ());

  d_armyset = new Armyset (Armysetlist::getNextAvailableId (1), name);
  d_armyset->setNewTemporaryFile ();

  inhibit_updates = true;
  armies_list->clear();
  inhibit_updates = false;


  refresh_armies ();
  update_army_panel ();
  needs_saving = true;
  update_window_title();
  return true;
}

void ArmySetWindow::on_load_armyset_activated()
{
  load_armyset ();
}

bool ArmySetWindow::load_armyset ()
{
  bool ret = false;
  Glib::ustring msg = _("Save these changes before opening a new armyset?");
  if (check_discard (msg) == false)
    return ret;
  Gtk::FileChooserDialog chooser(*window,
				 _("Choose an Armyset to Open"));
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
      bool ok = load_armyset(chooser.get_filename());
      chooser.hide();
      if (ok)
        {
          needs_saving = false;
          update_window_title();
          ret = true;
        }
    }

  refresh_armies ();
  update_army_panel ();
  return ret;
}

void ArmySetWindow::on_validate_armyset_activated()
{
  std::list<Glib::ustring> msgs;
  if (d_armyset == NULL)
    return;
  bool valid;
  valid = String::utrim (d_armyset->getName ()) != "";
  if (!valid)
    {
      Glib::ustring s = _("The name of the armyset is invalid.");
      msgs.push_back(s);
    }
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
          msgs.push_back(String::ucompose(_("%1 does not have an image for the %2 player"), (*it)->getName(), Shield::colourToString(c)));
          break;
        }
    }

  valid = d_armyset->validateArmyUnitNames();
  if (!valid)
    msgs.push_back(_("An army unit does not have a name."));

  if (msgs.empty() == true && isValidName () == false)
    msgs.push_back(_("The name of the armyset is not unique."));

  Glib::ustring msg = "";
  for (std::list<Glib::ustring>::iterator it = msgs.begin(); it != msgs.end();
       it++)
    {
      msg += (*it) + "\n";
      break;
    }

  if (msg == "")
    msg = _("The armyset is valid.");

  Gtk::MessageDialog dialog(*window, msg);
  dialog.run();
  dialog.hide();
  return;
}

void ArmySetWindow::on_save_as_activated()
{
  if (check_save_valid (false))
    save_current_armyset_file_as ();
}

void ArmySetWindow::sync_armies ()
{
  //Reorder the armyset according to the treeview
  d_armyset->clear ();
  guint32 id = 0;
  for (Gtk::TreeIter i = armies_list->children ().begin (),
       end = armies_list->children ().end (); i != end; ++i)
    {
      ArmyProto *army = (*i)[armies_columns.army];
      if (army)
        {
          army->setId (id);
          id++;
          d_armyset->push_back (army);
        }
    }
}

bool ArmySetWindow::save_current_armyset_file_as ()
{
  sync_armies ();

  bool ret = false;
  while (1)
    {
      Gtk::FileChooserDialog chooser(*window, _("Choose a Name"),
                                     Gtk::FILE_CHOOSER_ACTION_SAVE);
      Glib::RefPtr<Gtk::FileFilter> lwa_filter = Gtk::FileFilter::create();
      lwa_filter->set_name(_("LordsAWar Armysets (*.lwa)"));
      lwa_filter->add_pattern("*" + ARMYSET_EXT);
      chooser.add_filter(lwa_filter);
      chooser.set_current_folder(File::getSetDir(ARMYSET_EXT, false));

      chooser.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
      chooser.add_button(Gtk::Stock::SAVE, Gtk::RESPONSE_ACCEPT);
      chooser.set_default_response(Gtk::RESPONSE_ACCEPT);
      chooser.set_do_overwrite_confirmation();
      chooser.set_current_name (File::sanify(d_armyset->getName ()) +
                                ARMYSET_EXT);

      chooser.show_all();
      int res = chooser.run();

      if (res == Gtk::RESPONSE_ACCEPT)
        {
          Glib::ustring filename = chooser.get_filename();
          Glib::ustring old_filename = current_save_filename;
          guint32 old_id = d_armyset->getId ();
          d_armyset->setId(Armysetlist::getNextAvailableId(old_id));

          ret = save_current_armyset_file(filename);
          if (ret == false)
            {
              current_save_filename = old_filename;
              d_armyset->setId(old_id);
            }
          else
            {
              save_armyset_menuitem->set_sensitive (true);
              needs_saving = false;
              d_armyset->created (filename);
              Glib::ustring dir =
                File::add_slash_if_necessary (File::get_dirname (filename));
              if (dir == File::getSetDir(ARMYSET_EXT, false) ||
                  dir == File::getSetDir(ARMYSET_EXT, true))
                {
                  //if we saved it to a standard place, update the list
                  Armysetlist::getInstance()->add (Armyset::copy (d_armyset),
                                                   filename);
                  armyset_saved.emit(d_armyset->getId());
                }
              refresh_armies ();
              update_army_panel ();
              update_window_title();
            }
        }
      chooser.hide ();
      if (res == Gtk::RESPONSE_CANCEL)
        break;
      if (ret == true)
        break;
    }
  return ret;
}

bool ArmySetWindow::save_current_armyset_file (Glib::ustring filename)
{
  current_save_filename = filename;
  if (current_save_filename.empty())
    current_save_filename = d_armyset->getConfigurationFile(true);

  sync_armies ();

  bool ok = d_armyset->save(current_save_filename, Armyset::file_extension);
  if (ok)
    {
      if (Armysetlist::getInstance()->reload(d_armyset->getId()))
        refresh_armies();
      update_army_panel ();
      needs_saving = false;
      update_window_title();
      armyset_saved.emit(d_armyset->getId());
    }
  else
    {
      Glib::ustring errmsg = Glib::strerror(errno);
      Glib::ustring msg = _("Error!  Armyset could not be saved.");
      msg += "\n" + current_save_filename + "\n" + errmsg;
      Gtk::MessageDialog dialog(*window, msg);
      dialog.run();
      dialog.hide();
    }
  return ok;
}

void ArmySetWindow::on_save_armyset_activated()
{
  if (check_save_valid (true))
    save_current_armyset_file ();
}

void ArmySetWindow::on_edit_ship_picture_activated()
{
  Glib::ustring imgname = d_armyset->getShipImageName();
  MaskedImageEditorDialog d(*window, imgname,
                            d_armyset->getShipPic (),
                            d_armyset->getShipMask (),
                            EDITOR_DIALOG_TILE_PIC_FONTSIZE_MULTIPLE);
  d.set_title(_("Select a Ship image"));
  int response = d.run();
  if (response == Gtk::RESPONSE_ACCEPT && d.get_filename() != "")
    {
      Glib::ustring newname = "";
      bool success = false;
      if (imgname.empty () == true)
        success =
          d_armyset->addFileInCfgFile( d.get_filename(), newname);
      else
        success =
          d_armyset->replaceFileInCfgFile(imgname, d.get_filename(), newname);
      if (success)
        {
          d_armyset->setShipImageName(newname);
          d_armyset->instantiateShipImage ();
          needs_saving = true;
          update_window_title();
        }
      else
        show_add_file_error(d_armyset, *d.get_dialog(), d.get_filename ());
    }
  else if (response == Gtk::RESPONSE_REJECT)
    {
      if (d_armyset->removeFileInCfgFile(imgname))
        {
          needs_saving = true;
          update_window_title();
          d_armyset->uninstantiateSameNamedImages (imgname);
        }
      else
        show_remove_file_error(d_armyset, *d.get_dialog(), imgname);
    }
}

void ArmySetWindow::on_edit_standard_picture_activated()
{
  Glib::ustring imgname = d_armyset->getStandardImageName();
  MaskedImageEditorDialog d(*window, imgname,
                            d_armyset->getStandardPic (),
                            d_armyset->getStandardMask (),
                            EDITOR_DIALOG_TILE_PIC_FONTSIZE_MULTIPLE);
  d.set_title(_("Select a Hero Flag image"));
  int response = d.run();
  if (response == Gtk::RESPONSE_ACCEPT && d.get_filename() != "")
    {
      Glib::ustring newname = "";
      bool success = false;
      if (imgname.empty () == true)
        success = d_armyset->addFileInCfgFile(d.get_filename(), newname);
      else
        success =
          d_armyset->replaceFileInCfgFile(imgname, d.get_filename(), newname);
      if (success)
        {
          d_armyset->setStandardImageName(newname);
          d_armyset->instantiateStandardImage ();
          needs_saving = true;
          update_window_title();
        }
      else
        show_add_file_error(d_armyset, *d.get_dialog(), d.get_filename ());
    }
  else if (response == Gtk::RESPONSE_REJECT)
    {
      if (d_armyset->removeFileInCfgFile(imgname))
        {
          needs_saving = true;
          update_window_title();
          d_armyset->uninstantiateSameNamedImages (imgname);
        }
      else
        show_remove_file_error(d_armyset, *d.get_dialog(), imgname);
    }
}

void ArmySetWindow::on_edit_bag_picture_activated()
{
  Glib::ustring imgname = d_armyset->getBagImageName();
  std::vector<PixMask *> frames;
  if (imgname.empty () == false)
    frames.push_back (d_armyset->getBagPic ());
  ImageEditorDialog d(*window, imgname, 1, frames,
                      EDITOR_DIALOG_TILE_PIC_FONTSIZE_MULTIPLE * 0.5);
  d.set_title(_("Select a Bag image"));
  int response = d.run();
  if (response == Gtk::RESPONSE_ACCEPT && d.get_filename() != "")
    {
      Glib::ustring newname = "";
      bool success = false;
      if (imgname.empty () == true)
        success = d_armyset->addFileInCfgFile(d.get_filename(), newname);
      else
        success =
          d_armyset->replaceFileInCfgFile(imgname, d.get_filename(), newname);
      if (success)
        {
          d_armyset->setBagImageName(newname);
          d_armyset->instantiateBagImage ();
          needs_saving = true;
          update_window_title();
        }
      else
        show_add_file_error(d_armyset, *d.get_dialog(), d.get_filename ());
    }
  else if (response == Gtk::RESPONSE_REJECT)
    {
      if (d_armyset->removeFileInCfgFile(imgname))
        {
          needs_saving = true;
          update_window_title();
          d_armyset->uninstantiateSameNamedImages (imgname);
        }
      else
        show_remove_file_error(d_armyset, *d.get_dialog(), imgname);
    }
}

void ArmySetWindow::on_edit_armyset_info_activated()
{
  ArmySetInfoDialog d(*window, d_armyset);
  bool changed = d.run();
  if (changed)
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
  Glib::ustring imgname = army->getImageName(c);
  if (imgname.empty () == false)
    {
      Gdk::RGBA colour = Shieldsetlist::getInstance()->getColor(1, c);
      PixMask *p =
        ImageCache::applyMask(army->getImage(c), army->getMask(c), colour);
      double ratio = EDITOR_DIALOG_TILE_PIC_FONTSIZE_MULTIPLE;
      int font_size = FontSize::getInstance()->get_height ();
      double new_height = font_size * ratio;
      int new_width =
        ImageCache::calculate_width_from_adjusted_height (p, new_height);
      PixMask::scale (p, new_width, new_height);
      image->property_pixbuf() = p->to_pixbuf();
      delete p;
      button->set_label(imgname);
    }
  else
    {
      button->set_label(_("no image set"));
      image->clear();
    }
}

void ArmySetWindow::fill_army_images (ArmyProto *army)
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
}

void ArmySetWindow::fill_army_info(ArmyProto *army)
{
  fill_army_images (army);
  name_entry->set_text(army->getName());
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

void ArmySetWindow::instantiateOthers (ArmyProto *a, Shield::Colour c,
                                       Glib::ustring filename)
{
  if (a->getImageName (c).empty () == true)
    return;
  //what a hassle.  an army can reuse the same file many times
  for (unsigned int i = Shield::WHITE; i <= Shield::NEUTRAL; i++)
    {
      Shield::Colour col = Shield::Colour(i);
      if (col == c)
        continue;
      if (a->getImageName (c) == a->getImageName (col))
        {
          a->setImageName(col, filename);
          a->instantiateImage (d_armyset->getConfigurationFile (),
                               d_armyset->getTileSize (), col);
        }
    }
}

void ArmySetWindow::on_image_changed(Shield::Colour c)
{
  Glib::RefPtr<Gtk::TreeSelection> selection = armies_treeview->get_selection();
  Gtk::TreeModel::iterator iterrow = selection->get_selected();

  if (iterrow)
    {
      Gtk::TreeModel::Row row = *iterrow;
      ArmyProto *a = row[armies_columns.army];
      Glib::ustring imgname = a->getImageName(c);
      MaskedImageEditorDialog d(*window, imgname,
                                a->getImage (c), a->getMask (c),
                                EDITOR_DIALOG_TILE_PIC_FONTSIZE_MULTIPLE);
      d.set_title(String::ucompose(_("Select a %1 Army image"),
                                   Shield::colourToFriendlyName(c)));
      int response = d.run();
      if (response == Gtk::RESPONSE_ACCEPT && d.get_filename() != "")
        {
          Glib::ustring newname = "";
          bool success = false;
          if (imgname.empty () == true)
            success =
              d_armyset->addFileInCfgFile(d.get_filename(), newname);
          else
            success =
              d_armyset->replaceFileInCfgFile(imgname, d.get_filename(),
                                              newname);
          if (success)
            {
              instantiateOthers (a, c, newname);
              a->setImageName(c, newname);
              a->instantiateImage (d_armyset->getConfigurationFile (),
                                   d_armyset->getTileSize (), c);
              fill_army_images (a);

              if (inhibit_needs_saving == false)
                {
                  needs_saving = true;
                  update_window_title();
                }
            }
          else
            show_add_file_error(d_armyset, *d.get_dialog(), d.get_filename ());
        }
      else if (response == Gtk::RESPONSE_REJECT)
        {
          if (d_armyset->removeFileInCfgFile(imgname))
            {
              needs_saving = true;
              update_window_title();
              d_armyset->uninstantiateSameNamedImages (imgname);
              fill_army_images (a);
            }
        }
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
  d_armyset->setLoadTemporaryFile ();

  bool broken = false;
  d_armyset->instantiateImages(false, broken);
  if (broken)
    {
      delete d_armyset;
      d_armyset = NULL;
      Gtk::MessageDialog td(*window, _("Couldn't load armyset images."));
      td.run();
      td.hide();
      return false;
    }
  for (Armyset::iterator i = d_armyset->begin(); i != d_armyset->end(); ++i)
    addArmyType((*i)->getId());
  if (d_armyset->empty() == false)
    {
      Gtk::TreeModel::Row row;
      row = armies_treeview->get_model()->children()[0];
      if(row)
	armies_treeview->get_selection()->select(row);
    }
  save_armyset_menuitem->set_sensitive (true);
  needs_saving = false;
  update_window_title();
  inhibit_scrolldown=false;
  return true;
}

bool ArmySetWindow::quit()
{
  if (needs_saving)
    {
      EditorQuitDialog d (*window);
      int response = d.run_and_hide();

      if (response == Gtk::RESPONSE_CANCEL) // we don't want to new
        return false;

      else if (response == Gtk::RESPONSE_ACCEPT) // save it
        {
          bool saved = false;
          bool existing = d_armyset->getDirectory().empty () == false;
          if (existing)
            {
              if (check_save_valid (true))
                {
                  if (save_current_armyset_file ())
                    saved = true;
                }
              else
                return false;
            }
          else
            {
              if (check_save_valid (false))
                saved = save_current_armyset_file_as ();
              else
                return false;
            }
          if (!saved)
            return false;
        }
    }
  window->hide ();
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
  title += d_armyset->getName();
  title += " - ";
  title += _("Armyset Editor");
  window->set_title(title);
}

Gtk::Button *ArmySetWindow::lookup_button_by_colour (Shield::Colour s)
{
  switch (s)
    {
    case Shield::WHITE: return white_image_button;
    case Shield::GREEN: return green_image_button;
    case Shield::YELLOW: return yellow_image_button;
    case Shield::DARK_BLUE: return dark_blue_image_button;
    case Shield::ORANGE: return orange_image_button;
    case Shield::LIGHT_BLUE: return light_blue_image_button;
    case Shield::RED: return red_image_button;
    case Shield::BLACK: return black_image_button;
    case Shield::NEUTRAL: return neutral_image_button;
    }
  return NULL;
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
  Glib::ustring in = a->getImageName(Shield::Colour(0));
  Glib::ustring white_filename = d_armyset->getFileFromConfigurationFile(in);
  if (white_filename.empty () == true)
    return;


  //we have to save the white army image elsewhere
  //because it gets blown away in its temporary directory otherwise.
  Glib::ustring tmp_dir = File::get_tmp_file();
  File::create_dir (tmp_dir);
  Glib::ustring wbname = File::get_basename (white_filename, true);
  Glib::ustring wdestfile = String::ucompose ("%1/%2", tmp_dir, wbname);

  File::copy (white_filename, wdestfile);
  white_filename = wdestfile;

  //get the image for white and then transfer it to the rest.
  green_image_button->set_label(white_image_button->get_label());
  yellow_image_button->set_label(white_image_button->get_label());
  light_blue_image_button->set_label(white_image_button->get_label());
  red_image_button->set_label(white_image_button->get_label());
  dark_blue_image_button->set_label(white_image_button->get_label());
  orange_image_button->set_label(white_image_button->get_label());
  black_image_button->set_label(white_image_button->get_label());
  neutral_image_button->set_label(white_image_button->get_label());

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
      Glib::ustring imgname = a->getImageName(s);
      Glib::ustring newname = "";

      bool success = false;
      if (imgname.empty () == true)
        success =
          d_armyset->addFileInCfgFile(white_filename, newname);
      else
        success =
          d_armyset->replaceFileInCfgFile(imgname, white_filename, newname);
      if (success)
        {
          a->setImageName(s, newname);
          bool broken = false;
          Gdk::RGBA colour = Shieldsetlist::getInstance()->getColor(1, s);
          broken =
            a->instantiateImage (d_armyset->getConfigurationFile (),
                                 d_armyset->getTileSize (), s);
          if (!broken)
            {
              PixMask *p =
                ImageCache::applyMask(a->getImage(s), a->getMask(s), colour);
              double ratio = EDITOR_DIALOG_TILE_PIC_FONTSIZE_MULTIPLE;
              int font_size = FontSize::getInstance()->get_height ();
              double new_height = font_size * ratio;
              int new_width =
                ImageCache::calculate_width_from_adjusted_height (p, new_height);
              PixMask::scale (p, new_width, new_height);
              images[i]->property_pixbuf() = p->to_pixbuf();
              delete p;
              Gtk::Button *button = lookup_button_by_colour (s);
              if (button)
                button->set_label(a->getImageName (s));
            }
        }
      else
        {
          show_add_file_error(d_armyset, *window, white_filename);
          break;
        }
    }
  File::erase (white_filename);
  File::erase_dir (tmp_dir);
}

void ArmySetWindow::show_add_file_error(Armyset *a, Gtk::Window &d, Glib::ustring file)
{
  Glib::ustring errmsg = Glib::strerror(errno);
  Glib::ustring m = String::ucompose(_("Couldn't add %1 to:\n%2\n%3"), file,
                                     a->getConfigurationFile(), errmsg);
  Gtk::MessageDialog td(d, m);
  td.run();
  td.hide();
}

void ArmySetWindow::show_remove_file_error(Armyset *a, Gtk::Window &d, Glib::ustring file)
{
  Glib::ustring errmsg = Glib::strerror(errno);
  Glib::ustring m =
    String::ucompose(_("Couldn't remove %1 from:\n%2\n%3"), file,
                     a->getConfigurationFile(), errmsg);
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

bool ArmySetWindow::check_discard (Glib::ustring msg)
{
  if (needs_saving)
    {
      EditorSaveChangesDialog d (*window, msg);
      int response = d.run_and_hide();

      if (response == Gtk::RESPONSE_CANCEL) // we don't want to new
        return false;

      else if (response == Gtk::RESPONSE_ACCEPT) // save it
        {
          if (check_save_valid (true))
            {
              bool saved = false;
              if (d_armyset->getDirectory ().empty () == false)
                  saved = save_current_armyset_file_as ();
              else
                {
                  if (save_current_armyset_file ())
                    saved = true;
                }
              if (!saved)
                return false;
            }
          else
            return false;
        }
    }
  return true;
}

bool ArmySetWindow::check_save_valid (bool existing)
{
  if (check_name_valid (existing) == false)
    return false;

  if (d_armyset->validate () == false)
    {
      //welp, we don't do it this way with armysets.
      //we will have to check each of the players to see if they use this.
      if (existing &&
          Playerlist::getInstance()->hasArmyset(d_armyset->getId()))
        {
          Glib::ustring errmsg =
            _("Armyset is invalid, and is also one of the current working armysets.");
          Glib::ustring msg = _("Error!  Armyset could not be saved.");
          msg += "\n" + current_save_filename + "\n" + errmsg;
          Gtk::MessageDialog dialog(*window, msg);
          dialog.run();
          dialog.hide();
          return false;
        }
      else
        {
          Gtk::MessageDialog
            dialog(*window,
                   _("The armyset is invalid.  Do you want to proceed?"));
          dialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
          int response = dialog.run();
          dialog.hide();
          if (response == Gtk::RESPONSE_CANCEL)
            return false;
        }
    }
  return true;
}

bool ArmySetWindow::check_name_valid (bool existing)
{
  Glib::ustring name = d_armyset->getName ();
  Glib::ustring newname = "";
  if (existing)
    {
      Armyset *oldarmyset =
        Armysetlist::getInstance ()->get(d_armyset->getId());
      if (oldarmyset && oldarmyset->getName () != name)
          newname = oldarmyset->getName ();
    }

  guint32 num = 0;
  Glib::ustring n = String::utrim (String::strip_trailing_numbers (name));
  if (n == "")
    n = _("Untitled");
  if (newname.empty () == true)
    newname =
      Armysetlist::getInstance()->findFreeName(n, 100, num,
                                               d_armyset->getTileSize ());
  if (name == "")
    {
      if (newname.empty() == true)
        {
          Glib::ustring msg =
            _("The armyset has an invalid name.\nChange it and save again.");
          Gtk::MessageDialog d(*window, msg);
          d.run();
          d.hide();
          on_edit_armyset_info_activated ();
          return false;
        }
      else
        {
          Glib::ustring msg =
            String::ucompose (_("The armyset has an invalid name.\nChange it to '%1'?"), newname);
          Gtk::MessageDialog d(*window, msg);
          d.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
          int response = d.run();
          d.hide();
          if (response == Gtk::RESPONSE_CANCEL)
            return false;
          d_armyset->setName (newname);
        }
    }

  //okay the question is whether or not the name is already used.
  bool same_name = false;
  Glib::ustring file =
    Armysetlist::getInstance()->lookupConfigurationFileByName(d_armyset);
  if (file == "")
    return true;

  Glib::ustring cfgfile = d_armyset->getConfigurationFile(true);

  if (existing) // this means we're doing File->Save
    {
      if (file == cfgfile)
        return true;
      same_name = true;
    }
  else // this means we're doing File->Save As
    same_name = true;

  if (same_name)
    {
      if (newname.empty() == true)
        {
          Glib::ustring msg =
            _("The armyset has the same name as another armyset.\nChange it and save again.");
          Gtk::MessageDialog d(*window, msg);
          d.run();
          d.hide();
          on_edit_armyset_info_activated ();
          return false;
        }
      else
        {
          Glib::ustring msg =
            String::ucompose (_("The armyset has the same name as another armyset.\nChange it to '%1' instead?."), newname);

          Gtk::MessageDialog d(*window, msg);
          d.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
          int response = d.run();
          d.hide();
          if (response == Gtk::RESPONSE_CANCEL)
            return false;
          d_armyset->setName (newname);
        }
    }

  return true;
}

bool ArmySetWindow::isValidName ()
{
  Glib::ustring file =
    Armysetlist::getInstance()->lookupConfigurationFileByName(d_armyset);
  if (file == "")
    return true;
  if (file == d_armyset->getConfigurationFile (true))
    return true;
  return false;
}

void ArmySetWindow::on_army_moved ()
{
  needs_saving = true;
  update_window_title ();
}

/*
 some test cases
  1. create a new armyset from scratch, save invalid set, close, load it
  2. create a new armyset from scratch, save valid set, then switch sets
  3. save a copy of the default armyset, and switch sets
  4. modify the working armyset so we can see it change in scenario builder
  5. modify the working armyset so that it's invalid, try to save
  6. try adding an image file that isn't a .png
  7. try adding an image file that says it's a .png but is actually random data
  8. modify an existing armyset by replacing the scouts image with another
  9. modify an existing armyset by removing an army, save it, close, load
 10. modify an existing armyset by adding an army, save it, close, load
 11. try saving a new armyset that has a same name
 12. try saving an existing armyset that has a same name
 13. validate an armyset without: a neutral image for pikemen
 14. validate an armyset without: a bag image
 15. validate an armyset without: a hero's flag image
 16. try saving an armyset that has an empty name
 17. validate an armyset with a same name
 18. validate a armyset with an empty name
 19. make a new invalid armyset and quit save it
 20. load a writable armyset, modify and quit save it
 21. load a writable armyset, make it invalid, and then quit save it
 22. try saving an armyset we don't have permission to save
 23. try quit-saving an armyset we don't have permission to save
 24. create a new armyset, add a bag image, save invalid set, close, load it
 25. load a writable armyset, remove bag image, quit-save it.  load it
 26. create a new armyset, add hero flag, save invalid set, close, load it
 27. load a writable armyset, remove hero flag, quit-save it.  load it
 28. create a new armyset, add ship picture, save invalid set, close, load it
 29. load a writable armyset, remove ship picture, quit-save it.  load it
*/
