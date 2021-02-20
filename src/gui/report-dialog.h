//  Copyright (C) 2007, 2008, 2009, 2012, 2014, 2015, 2021 Ben Asselstine
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
#ifndef REPORT_DIALOG_H
#define REPORT_DIALOG_H

#include <gtkmm.h>

#include "LocationList.h"
#include "vectormap.h"
#include "citymap.h"
#include "armymap.h"
#include "player.h"
#include "lw-dialog.h"

class Action;
class BarChart;

//
//
class ReportDialog: public LwDialog
{
 public:
    enum ReportType {ARMY = 0, CITY, GOLD, PRODUCTION, WINNING};

    ReportDialog(Gtk::Window &parent, Player *player, ReportType type);
    ~ReportDialog();

    static Glib::ustring calculateRank(std::list<guint32> scores, guint32 score);
    void run();
    void hide() {return dialog->hide();};
    
 private:
    VectorMap* vectormap;
    ArmyMap* armymap;
    CityMap* citymap;

    Gtk::Image *map_image;
    
    Gtk::Label *army_label;
    Gtk::Label *city_label;
    Gtk::Label *gold_label;
    Gtk::Label *production_label;
    Gtk::Label *winning_label;
    Gtk::Notebook *report_notebook;
    Gtk::Box *army_alignment;
    BarChart *army_chart;
    Gtk::Box *city_alignment;
    BarChart *city_chart;
    Gtk::Box *gold_alignment;
    BarChart *gold_chart;
    Gtk::Box *winning_alignment;
    BarChart *winning_chart;
    sigc::connection switch_conn;

    Player *d_player;
    void on_army_map_changed(Cairo::RefPtr<Cairo::Surface> map);
    void on_city_map_changed(Cairo::RefPtr<Cairo::Surface> map);
    void on_vector_map_changed(Cairo::RefPtr<Cairo::Surface> map);
    void on_switch_page(guint number);

    Gtk::TreeView *armies_treeview;

    class ArmiesColumns: public Gtk::TreeModelColumnRecord {
    public:
	ArmiesColumns() 
        { add(city_id) ;add(image); add(desc);}
	
	Gtk::TreeModelColumn<guint32> city_id;
	Gtk::TreeModelColumn<Glib::RefPtr<Gdk::Pixbuf> > image;
	Gtk::TreeModelColumn<Glib::ustring> desc;
    };
    const ArmiesColumns armies_columns;
    Glib::RefPtr<Gtk::ListStore> armies_list;
    void addProduction(const Action *action);
    void fill_in_info();
    void updateArmyChart();
    void updateCityChart();
    void updateGoldChart();
    void updateWinningChart();
    void on_army_selected();

    static Glib::ustring get_rank_string(int rank);
};

#endif
