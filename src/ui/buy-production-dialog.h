//  Copyright (C) 2007 Ole Laursen
//  Copyright (C) 2007-2009, 2014, 2017 Ben Asselstine
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
#ifndef BUY_PRODUCTION_DIALOG_H
#define BUY_PRODUCTION_DIALOG_H

#include <gtkmm.h>

#include "army-info-tip.h"
#include "armyproto.h"
#include "lw-dialog.h"

class City;

// dialog for buying a production slot for a city
class BuyProductionDialog: public LwDialog
{
 public:
    BuyProductionDialog(Gtk::Window &parent, City *city);
    ~BuyProductionDialog();

    void run();
    void hide() {dialog->hide();};

    enum { NO_ARMY_SELECTED = -1 };
    int get_selected_army() { return selected_army == NO_ARMY_SELECTED ? int(NO_ARMY_SELECTED) : purchasables[selected_army]->getId(); }
    
 private:
    ArmyInfoTip* army_info_tip;
    Gtk::Label *unit_label;
    Gtk::Label *cost_label;
    Gtk::Label *time_label;
    Gtk::Label *moves_label;
    Gtk::Label *strength_label;
    Gtk::Label *upkeep_label;
    Gtk::Button *buy_button;

    City *city;
    int selected_army;

    std::vector<Gtk::ToggleButton *> production_toggles;
    bool ignore_toggles;
    std::vector<const ArmyProto*> purchasables;

    void on_production_toggled(Gtk::ToggleButton *toggle);
    bool on_production_button_event(GdkEventButton *e, Gtk::ToggleButton *toggle);
    void fill_in_production_info();
    void fill_pixbuf (int i);
    void set_buy_button_state();
    const ArmyProto *army_id_to_army();
    int lookup_slot (Gtk::ToggleButton *toggle);
};

#endif
