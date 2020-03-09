//  Copyright (C) 2011, 2014, 2015, 2020 Ben Asselstine
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
#ifndef STATUS_BOX_H
#define STATUS_BOX_H

#include <memory>
#include <sigc++/trackable.h>
#include <gtkmm.h>
#include <glibmm.h>
#include "Configuration.h"
#include "sidebar-stats.h"
#include "stack-tile-box.h"
#include "tartan-progress-bar.h"

class StackTileBox;
class StackTile;
// shows stats/progress/stack listing
class StatusBox: public Gtk::Box
{
 public:
     //! Constructor for building this object with gtk::builder
    StatusBox(BaseObjectType* base, const Glib::RefPtr<Gtk::Builder> &xml);

    //!Destructor.
    ~StatusBox();

    static StatusBox * create();

    void show_stats();
    void enforce_height();
    void show_progress();
    void show_stack(StackTile *s);
    void on_stack_info_changed(Stack *s);
    Stack * get_currently_selected_stack() const {return stack_tile_box->get_currently_selected_stack();};
    void clear_selected_stack() {stack_tile_box->clear_selected_stack();};
    void setHeightFudgeFactor(guint32 n) {d_height_fudge_factor = n;};

    void set_progress_label(Glib::ustring s);
    void pulse();
    void reset_progress();
    void update_sidebar_stats(SidebarStats s);
    void toggle_group_ungroup();
    //! Signals
    sigc::signal<void, Stack*> stack_composition_modified;
    sigc::signal<void, bool> stack_tile_group_toggle;
 protected:

 private:
    StackTileBox *stack_tile_box;
    guint32 d_height_fudge_factor;
    Gtk::Notebook *notebook;
    Gtk::Image *cities_stats_image;
    Gtk::Label *cities_stats_label;
    Gtk::Image *gold_stats_image;
    Gtk::Label *gold_stats_label;
    Gtk::Image *income_stats_image;
    Gtk::Label *income_stats_label;
    Gtk::Image *upkeep_stats_image;
    Gtk::Label *upkeep_stats_label;
    Gtk::Box *stack_info_container;
    Gtk::Box *stack_tile_box_container;
    Gtk::Box *stats_box;
    Gtk::Box *tartan_box;
    TartanProgressBar *turn_progressbar;
    Gtk::Label *progress_status_label;
    Glib::TimeVal last_pulsed;

    void drop_connections();
    void pad_image(Gtk::Image *image);
};

#endif // STATUS_BOX
