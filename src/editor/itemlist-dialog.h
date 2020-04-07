//  Copyright (C) 2008, 2009, 2011, 2014, 2015, 2020 Ben Asselstine
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
#ifndef GUI_ITEMLIST_DIALOG_H
#define GUI_ITEMLIST_DIALOG_H

#include <gtkmm.h>

#include "Itemlist.h"
#include "lw-editor-dialog.h"

class ArmyProto;
class ArmyChooserButton;

//! Scenario editor.  Edits the global list of Item objects in the scenario.
class ItemlistDialog: public LwEditorDialog
{
 public:
    ItemlistDialog(Gtk::Window &parent);
    ~ItemlistDialog();
    bool item_was_changed () const {return d_changed;}

 private:
    bool d_changed;
    Glib::ustring current_save_filename;
    Itemlist *d_itemlist; //current itemlist
    ItemProto *d_item; //current item
    Gtk::Entry *name_entry;
    Gtk::TreeView *items_treeview;
    Gtk::Button *add_item_button;
    Gtk::Button *remove_item_button;
    Gtk::Box *item_vbox;
    Gtk::Switch *add1str_switch;
    Gtk::Switch *add2str_switch;
    Gtk::Switch *add3str_switch;
    Gtk::Switch *add1stack_switch;
    Gtk::Switch *add2stack_switch;
    Gtk::Switch *add3stack_switch;
    Gtk::Switch *flystack_switch;
    Gtk::Switch *doublemovestack_switch;
    Gtk::Switch *add2goldpercity_switch;
    Gtk::Switch *add3goldpercity_switch;
    Gtk::Switch *add4goldpercity_switch;
    Gtk::Switch *add5goldpercity_switch;
    Gtk::Switch *steals_gold_switch;
    Gtk::Switch *sinks_ships_switch;
    Gtk::Switch *pickup_bags_switch;
    Gtk::Switch *add_mp_switch;
    Gtk::Switch *banish_worms_switch;
    Gtk::Switch *burn_bridge_switch;
    Gtk::Switch *capture_keeper_switch;
    Gtk::Switch *summon_monster_switch;
    Gtk::Switch *disease_city_switch;
    Gtk::Switch *raise_defenders_switch;
    Gtk::Switch *persuade_neutral_city_switch;
    Gtk::Switch *teleport_to_city_switch;
    Gtk::SpinButton *uses_spinbutton;
    ArmyChooserButton *kill_army_type_button;
    Gtk::SpinButton *steal_percent_spinbutton;
    ArmyChooserButton *summon_army_type_button;
    Gtk::ComboBox *building_type_to_summon_on_combobox;
    Gtk::SpinButton *disease_armies_percent_spinbutton;
    Gtk::SpinButton *add_mp_spinbutton;
    ArmyChooserButton *defender_army_type_button;
    Gtk::SpinButton *num_defenders_spinbutton;
    std::vector<sigc::connection> connections;


    class ItemsColumns: public Gtk::TreeModelColumnRecord {
    public:
	ItemsColumns() 
        { add(name); add(item);}
	
	Gtk::TreeModelColumn<Glib::ustring> name;
	Gtk::TreeModelColumn<ItemProto *> item;
    };
    const ItemsColumns items_columns;
    Glib::RefPtr<Gtk::ListStore> items_list;

    bool on_delete_event(GdkEventAny *e);

    void addItemProto(ItemProto *itemproto);
    void update_item_panel();
    void update_itemlist_buttons();

    void fill_item_info(ItemProto *item);

    //callbacks
    void on_name_changed();
    void on_add_item_clicked();
    void on_remove_item_clicked();
    void on_item_selected();


    void on_switch_toggled(Gtk::Switch *sw, 
				ItemProto::Bonus bonus);
    void on_add1str_toggled();
    void on_add2str_toggled();
    void on_add3str_toggled();
    void on_add1stack_toggled();
    void on_add2stack_toggled();
    void on_add3stack_toggled();
    void on_flystack_toggled();
    void on_doublemovestack_toggled();
    void on_add2goldpercity_toggled();
    void on_add3goldpercity_toggled();
    void on_add4goldpercity_toggled();
    void on_add5goldpercity_toggled();
    void on_steals_gold_toggled();
    void on_sinks_ships_toggled();
    void on_banish_worms_toggled();
    void on_burn_bridge_toggled();
    void on_uses_changed();
    void on_kill_army_type_selected (const ArmyProto *a);
    void on_capture_keeper_toggled();
    void on_pickup_bags_toggled();
    void on_add_mp_toggled();
    void on_summon_monster_toggled();
    void on_summon_army_type_selected(const ArmyProto *a);
    void on_building_type_to_summon_on_changed ();
    void update_summon_army_type_name();
    void on_disease_city_toggled();
    void on_steal_percent_changed();
    void on_steal_percent_text_changed();
    void on_disease_armies_percent_changed();
    void on_disease_armies_percent_text_changed();
    void on_add_mp_changed();
    void on_add_mp_text_changed();
    void on_raise_defenders_toggled();
    void on_defender_type_selected(const ArmyProto *a);
    void on_num_defenders_changed();
    void on_num_defenders_text_changed();
    void on_persuade_neutral_city_toggled();
    void on_teleport_to_city_toggled();
    void connect_signals ();
    void disconnect_signals ();
    void load_widgets ();
};

#endif
