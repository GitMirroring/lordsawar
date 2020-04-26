//  Copyright (C) 2007, 2008, 2009, 2010, 2014, 2015, 2020 Ben Asselstine
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

#pragma once
#ifndef GUI_ARMYSET_WINDOW_H
#define GUI_ARMYSET_WINDOW_H

#include <memory>
#include <vector>
#include <sigc++/signal.h>
#include <sigc++/trackable.h>
#include <gtkmm.h>

#include "armyproto.h"
#include "armyset.h"
#include "shield.h"

//! Armyset Editor.  Edit an Armyset.
class ArmySetWindow: public sigc::trackable
{
 public:
    ArmySetWindow(Glib::ustring load_filename = "");
    ~ArmySetWindow();

    void show() {window->show();}
    void hide() {window->hide();}

    Gtk::Window &get_window() { return *window; }

    sigc::signal<void, guint32> armyset_saved;

 private:
    Gtk::Window* window;
    Glib::ustring current_save_filename;
    Armyset *d_armyset; //current armyset
    ArmyProto *d_army; //current army
    bool needs_saving;
    bool inhibit_needs_saving;
    bool inhibit_updates;
    Gtk::Image *white_image;
    Gtk::Image *green_image;
    Gtk::Image *yellow_image;
    Gtk::Image *light_blue_image;
    Gtk::Image *red_image;
    Gtk::Image *dark_blue_image;
    Gtk::Image *orange_image;
    Gtk::Image *black_image;
    Gtk::Image *neutral_image;
    Gtk::Entry *name_entry;
    Gtk::ScrolledWindow *armies_scrolledwindow;
    Gtk::TreeView *armies_treeview;
    Gtk::TextView *description_textview;
    Gtk::Button *white_image_button;
    Gtk::Button *green_image_button;
    Gtk::Button *yellow_image_button;
    Gtk::Button *light_blue_image_button;
    Gtk::Button *red_image_button;
    Gtk::Button *dark_blue_image_button;
    Gtk::Button *orange_image_button;
    Gtk::Button *black_image_button;
    Gtk::Button *neutral_image_button;
    Gtk::SpinButton *production_spinbutton;
    Gtk::SpinButton *cost_spinbutton;
    Gtk::SpinButton *new_cost_spinbutton;
    Gtk::SpinButton *upkeep_spinbutton;
    Gtk::SpinButton *strength_spinbutton;
    Gtk::SpinButton *moves_spinbutton;
    Gtk::SpinButton *exp_spinbutton;
    Gtk::SpinButton *id_spinbutton;
    Gtk::ComboBox *hero_combobox;
    Gtk::Switch *awardable_switch;
    Gtk::Switch *defends_ruins_switch;
    Gtk::SpinButton *sight_spinbutton;
    Gtk::Switch *move_forests_switch;
    Gtk::Switch *move_marshes_switch;
    Gtk::Switch *move_hills_switch;
    Gtk::Switch *move_mountains_switch;
    Gtk::Switch *can_fly_switch;
    Gtk::Switch *add1strinopen_switch;
    Gtk::Switch *add2strinopen_switch;
    Gtk::Switch *add1strinforest_switch;
    Gtk::Switch *add2strinforest_switch;
    Gtk::Switch *add1strinhills_switch;
    Gtk::Switch *add2strinhills_switch;
    Gtk::Switch *add1strincity_switch;
    Gtk::Switch *add2strincity_switch;
    Gtk::Switch *add1stackinhills_switch;
    Gtk::Switch *suballcitybonus_switch;
    Gtk::Switch *sub1enemystack_switch;
    Gtk::Switch *sub2enemystack_switch;
    Gtk::Switch *add1stack_switch;
    Gtk::Switch *add2stack_switch;
    Gtk::Switch *suballnonherobonus_switch;
    Gtk::Switch *suballherobonus_switch;
    Gtk::Button *add_army_button;
    Gtk::Button *remove_army_button;
    Gtk::Box *army_vbox;
    Gtk::MenuItem *new_armyset_menuitem;
    Gtk::MenuItem *load_armyset_menuitem;
    Gtk::MenuItem *save_armyset_menuitem;
    Gtk::MenuItem *save_as_menuitem;
    Gtk::MenuItem *validate_armyset_menuitem;
    Gtk::MenuItem *edit_armyset_info_menuitem;
    Gtk::MenuItem *edit_ship_picture_menuitem;
    Gtk::MenuItem *edit_standard_picture_menuitem;
    Gtk::MenuItem *edit_bag_picture_menuitem;
    Gtk::MenuItem *edit_selector_menuitem;
    Gtk::MenuItem *quit_menuitem;
    Gtk::MenuItem *help_about_menuitem;
    Gtk::MenuItem *tutorial_menuitem;
    Gtk::Button *make_same_button;
    Gtk::Notebook *notebook;

    class ArmiesColumns: public Gtk::TreeModelColumnRecord {
    public:
	ArmiesColumns()
        { add(name); add(army);}
	
	Gtk::TreeModelColumn<Glib::ustring> name;
	Gtk::TreeModelColumn<ArmyProto *> army;
    };
    const ArmiesColumns armies_columns;
    Glib::RefPtr<Gtk::ListStore> armies_list;
    bool inhibit_scrolldown;

    void addArmyType(guint32 army_type);
    void update_army_panel();
    void update_armyset_buttons();

    void on_new_armyset_activated();
    void on_load_armyset_activated();
    void on_save_armyset_activated();
    void on_save_as_activated();
    void on_validate_armyset_activated();
    void on_quit_activated();
    bool on_window_closed(GdkEventAny*);
    bool quit();
    void on_edit_armyset_info_activated();
    void on_edit_standard_picture_activated();
    void on_edit_bag_picture_activated();
    void on_edit_ship_picture_activated();
    void on_edit_selector_picture_activated();
    void on_help_about_activated();
    void on_tutorial_video_activated();
    void on_army_selected();
    void fill_army_image(Gtk::Button *button, Gtk::Image *image, Shield::Colour c, ArmyProto *army);
    void fill_army_info(ArmyProto *army);

    //callbacks
    void on_name_changed();
    void on_description_changed();
    void on_image_changed(Shield::Colour c);
    void on_production_changed();
    void on_production_text_changed();
    void on_cost_changed();
    void on_cost_text_changed();
    void on_new_cost_changed();
    void on_new_cost_text_changed();
    void on_upkeep_changed();
    void on_upkeep_text_changed();
    void on_strength_changed();
    void on_strength_text_changed();
    void on_moves_changed();
    void on_moves_text_changed();
    void on_exp_changed();
    void on_exp_text_changed();
    void on_sight_changed();
    void on_sight_text_changed();
    void on_id_changed();
    void on_id_text_changed();
    void on_hero_combobox_changed();
    void on_awardable_toggled();
    void on_defends_ruins_toggled();
    void on_movebonus_toggled(Gtk::Switch *button, guint32 val);
    void on_armybonus_toggled(Gtk::Switch *button, guint32 val);
    void on_add_army_clicked();
    void on_remove_army_clicked();

    void on_white_all_checked();

    bool load_armyset(Glib::ustring filename);
    void update_window_title();
    void on_make_same_clicked();

    void show_add_file_error(Armyset *a, Gtk::Window &d, Glib::ustring file);
    void show_remove_file_error(Armyset *a, Gtk::Window &d, Glib::ustring file);
    void refresh_armies();

    bool make_new_armyset ();
    bool load_armyset ();
    bool save_current_armyset_file (Glib::ustring filename = "");
    bool save_current_armyset_file_as ();

    bool check_discard (Glib::ustring msg);
    bool check_save_valid (bool existing);
    bool check_name_valid (bool existing);
    bool isValidName ();
    Gtk::Button *lookup_button_by_colour (Shield::Colour s);
    void fill_army_images (ArmyProto *army);
    void sync_armies ();
    void instantiateOthers (ArmyProto *a, Shield::Colour c, Glib::ustring f);
    void on_army_moved ();
};

#endif
