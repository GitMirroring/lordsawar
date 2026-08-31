//  Copyright (C) 2008, 2009, 2014 Ben Asselstine
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
#ifndef DIPLOMACY_DIALOG_H
#define DIPLOMACY_DIALOG_H

#include <gtkmm.h>

#include "player.h"
#include "lw-dialog.h"

//
//
class DiplomacyDialog: public LwDialog
{
 public:
    DiplomacyDialog(Gtk::Window &parent, Player *player);
    ~DiplomacyDialog() {};

 private:
    void on_report_clicked();
    void on_proposal_toggled (Gtk::ToggleButton *toggle, Player *p, Player::DiplomaticProposal proposal);

    Player *d_player;

    Gtk::Grid *d_proposals_table;
    Gtk::Grid *d_offers_table;
    Gtk::Label *d_player_label;
    Gtk::Image *d_player_shield_image;
    Gtk::Button *d_report_button;
};

#endif
