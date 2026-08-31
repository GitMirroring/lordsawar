//  Copyright (C) 2021 Ben Asselstine
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
#ifndef HERO_STRATEGY_DIALOG_H
#define HERO_STRATEGY_DIALOG_H

#include <gtkmm.h>
#include "lw-editor-dialog.h"
#include "undo-mgr.h"
#include "hero-strategy.h"

//! Scenario editor.  Edits hero strategies.
class HeroStrategyDialog: public LwEditorDialog
{
 public:
    HeroStrategyDialog(Gtk::Window &parent, HeroStrategy *s);
    ~HeroStrategyDialog();

    bool run();

    HeroStrategy *get_strategy() {return d_strategy;}
    
 private:
    bool d_changed;
    UndoMgr *umgr;
    HeroStrategy *d_strategy;
    HeroStrategy_None *d_none_strategy;
    HeroStrategy_Random *d_random_strategy;
    HeroStrategy_SimpleQuester *d_simple_quester_strategy;

    Gtk::ComboBoxText *strategy_type_combobox;
    Gtk::Notebook *notebook;
    Gtk::SpinButton *turns_spinbutton;
    Gtk::SpinButton *num_helpers_spinbutton;
    Gtk::ComboBoxText *fallback_combobox;
    Gtk::Button *undo_button;
    Gtk::Button *redo_button;

    void on_type_changed();
    void on_turns_text_changed();
    void on_num_helpers_text_changed();
    void on_fallback_type_changed();

    void setup_strategies (HeroStrategy *r);
    bool switch_strategy_type (HeroStrategy::Type t);

    void fill_in_strategy_info();
    void on_undo_activated ();
    void on_redo_activated ();
    void connect_signals ();
    void disconnect_signals ();
    void update ();
    std::list<sigc::connection> connections;
    UndoAction* executeAction (UndoAction *action);
};

#endif
