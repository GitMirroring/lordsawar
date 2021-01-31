//  Copyright (C) 2010, 2012, 2014, 2015, 2020, 2021 Ben Asselstine
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
#ifndef SMALLMAP_EDITOR_DIALOG_H
#define SMALLMAP_EDITOR_DIALOG_H

#include <vector>
#include <gtkmm.h>

#include "vector.h"
#include "editablesmallmap.h"
#include "lw-editor-dialog.h"
#include "undo-mgr.h"
#include "smallmap-editor-actions.h"

class SmallmapEditorDialog: public LwEditorDialog
{
 public:
    SmallmapEditorDialog(Gtk::Window &parent);
    ~SmallmapEditorDialog();

    bool run();
    void hide();
    
 private:
    EditableSmallMap* smallmap;
    UndoMgr *umgr;
    bool d_changed;

    Gtk::Image *smallmap_image;
    Gtk::Box *modes_hbox;
    Gtk::Grid *terrain_type_table;
    Gtk::Box *building_types_hbox;
    Gtk::RadioButton *road_start_radiobutton;
    Gtk::RadioButton *road_finish_radiobutton;
    Gtk::Button *create_road_button;
    Gtk::Button *clear_points_button;
    Gtk::RadioButton *pointer_radiobutton;
    Gtk::EventBox *map_eventbox;
    Gtk::Button *undo_button;
    Gtk::Button *redo_button;
    Vector<int> road_start_point;
    Vector<int> road_finish_point;

    struct PointerItem
    {
	Gtk::RadioButton *button;
	EditableSmallMap::Pointer pointer;
        Glib::ustring image_file;
	int size;
    };

    std::vector<PointerItem> pointer_items;
    
    struct TerrainItem
    {
	Gtk::RadioButton *button;
	Tile::Type terrain;
    };

    std::vector<TerrainItem> terrain_items;

    void on_map_changed(Cairo::RefPtr<Cairo::Surface> map);
    bool on_map_mouse_button_event(GdkEventButton *e);
    bool on_map_mouse_motion_event(GdkEventMotion *e);
    void on_road_start_toggled();
    void on_road_finish_toggled();
    void on_create_road_clicked();
    void on_clear_points_clicked(bool act = true);
    void on_terrain_radiobutton_toggled();
    void on_pointer_radiobutton_toggled();
    bool on_smallmap_exposed();
    void on_road_start_placed(Vector<int> pos);
    void on_road_finish_placed(Vector<int> pos);
    void on_road_can_be_created(bool create_road);
    void on_map_edited();

    void setup_terrain_radiobuttons();
    void setup_pointer_radiobutton(Glib::RefPtr<Gtk::Builder> xml,
	Glib::ustring prefix, Glib::ustring image_file,
	EditableSmallMap::Pointer pointer, int size);
    void setup_pointer_radiobuttons(Glib::RefPtr<Gtk::Builder> xml);

    void update_cursor();
    void update_terrain_buttons();
    void update_road_buttons ();
    Tile::Type get_terrain();

    void on_got_undo (UndoAction *action);
    void on_undo_activated ();
    void on_redo_activated ();
    void update (bool first = false);
    void connect_signals ();
    void disconnect_signals ();
    std::list<sigc::connection> connections;
    UndoAction *executeAction (UndoAction *action);
    void doChangeMap (SmallmapEditorAction_ChangeMap *action);
};

#endif
