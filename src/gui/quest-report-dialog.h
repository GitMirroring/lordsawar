//  Copyright (C) 2007, 2008, 2009, 2012, 2014 Ben Asselstine
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
#ifndef QUEST_REPORT_DIALOG_H
#define QUEST_REPORT_DIALOG_H

#include <memory>
#include <vector>
#include <sigc++/trackable.h>
#include <gtkmm.h>

#include "questmap.h"
#include "Quest.h"
#include "reward.h"
#include "lw-dialog.h"

// dialog for depicting a quest
class QuestReportDialog: public LwDialog
{
 public:
    QuestReportDialog(Gtk::Window &parent, const std::vector<Quest *>&quests, Hero *preferred_hero);
    ~QuestReportDialog() {delete questmap;};

    void run();
    void hide() {return dialog->hide();};
 private:
    QuestMap* questmap;

    Gtk::Image *map_image;
    Gtk::Label *label;
    Gtk::Label *hero_label;
    
    std::vector<Quest*> quests;
    Quest *quest;

    void on_map_changed(Cairo::RefPtr<Cairo::Surface> map);
    Gtk::TreeView *heroes_treeview;

    class HeroesColumns: public Gtk::TreeModelColumnRecord {
    public:
	HeroesColumns() 
        { add(hero_name); add(quest);}
	
	Gtk::TreeModelColumn<Glib::ustring> hero_name;
	Gtk::TreeModelColumn<Quest*> quest;
    };
    const HeroesColumns heroes_columns;
    Glib::RefPtr<Gtk::ListStore> heroes_list;

    void fill_quest_info(Quest *q);

    void add_questing_hero(Quest *quest, Hero *h);

    void on_hero_changed();
};

#endif
