//  Copyright (C) 2008, 2014, 2021 Ben Asselstine
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
#ifndef NEW_NETWORK_GAME_DOWNLOAD_WINDOW_H
#define NEW_NETWORK_GAME_DOWNLOAD_WINDOW_H

#include "game-parameters.h"
#include <gtkmm.h>

class NewNetworkGameDownloadWindow : public sigc::trackable
{
  public:
    NewNetworkGameDownloadWindow(Glib::ustring title = "",
                                 Gtk::Window *parent = NULL);
    ~NewNetworkGameDownloadWindow() {delete window;}

    int run();
    void hide();
    void pulse (int amt, int total);
    void setFileSize (goffset s) {file_size = s;}

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
    Gtk::CellRendererProgress *pbar;
    goffset file_size;
};

#endif
