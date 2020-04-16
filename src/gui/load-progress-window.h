//  Copyright (C) 2020 Ben Asselstine
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
#ifndef LOAD_PROGRESS_WINDOW_H
#define LOAD_PROGRESS_WINDOW_H

#include <memory>
#include <sigc++/trackable.h>
#include <gtkmm.h>

class LoadProgressWindow: public sigc::trackable
{
 public:
    LoadProgressWindow(Gtk::Window *parent = NULL);
    ~LoadProgressWindow() {delete window;}

    int run();
    void hide();
    void tick_progress ();
    void finish_progress ();
    
 private:
    Gtk::Window * window;
    Gtk::TreeView *progress_treeview;
    class ProgressModelColumns : public Gtk::TreeModel::ColumnRecord
      {
    public:
        ProgressModelColumns ()
          { add (perc);}
        Gtk::TreeModelColumn<int> perc;
      };
    ProgressModelColumns progress_columns;
    Glib::RefPtr<Gtk::ListStore> progress_liststore;
    Gtk::TreeModel::Row row;
};

#endif
