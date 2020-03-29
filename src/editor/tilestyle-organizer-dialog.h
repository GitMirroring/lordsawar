//  Copyright (C) 2010, 2014, 2015 Ben Asselstine
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
#ifndef TILESTYLE_ORGANIZER_DIALOG_H
#define TILESTYLE_ORGANIZER_DIALOG_H

#include <gtkmm.h>
#include <sigc++/signal.h>
#include "lw-editor-dialog.h"

class Tile;
class TileStyle;

class TileStyleOrganizerDialog: public LwEditorDialog
{
 public:
    TileStyleOrganizerDialog(Gtk::Window &parent, Tile *tile);
    ~TileStyleOrganizerDialog() {}

    sigc::signal<void, guint32> tilestyle_selected;

 protected:

  class CategoriesColumns : public Gtk::TreeModel::ColumnRecord
  {
    public:
    CategoriesColumns() 
      {
        add(type);
        add(name);
        add(image);
      }

    Gtk::TreeModelColumn<guint32> type;
    Gtk::TreeModelColumn<Glib::ustring> name;
    Gtk::TreeModelColumn<Glib::RefPtr<Gdk::Pixbuf> > image;
  };

  CategoriesColumns categories_columns;
  Glib::RefPtr<Gtk::ListStore> categories_list;

  class TileStyleColumns : public Gtk::TreeModel::ColumnRecord
  {
    public:
    TileStyleColumns() 
      {
        add(style);
        add(name);
        add(image);
      }

    Gtk::TreeModelColumn<TileStyle*> style;
    Gtk::TreeModelColumn<Glib::ustring> name;
    Gtk::TreeModelColumn<Glib::RefPtr<Gdk::Pixbuf> > image;
  };

  TileStyleColumns tilestyle_columns;
  Glib::RefPtr<Gtk::ListStore> category_list;
  Glib::RefPtr<Gtk::ListStore> unsorted_list;
 private:
    Tile *d_tile;
    Gtk::IconView *categories_iconview;
    Gtk::IconView *category_iconview;
    Gtk::IconView *unsorted_iconview;
    Gtk::Label *category_label;
    Gtk::Label *unsorted_label;

    void add_category(guint32 type);
    void fill_in_categories();
    void fill_category(guint32 type);
    void empty_category();
    void on_category_selected();
    void add_tilestyle(Glib::RefPtr<Gtk::ListStore> list, TileStyle *tilestyle);
    void on_category_drag_data_get(const Glib::RefPtr<Gdk::DragContext> &drag_context,
                                   Gtk::SelectionData &data);
    void on_unsorted_drag_data_get(const Glib::RefPtr<Gdk::DragContext> &drag_context,
                                   Gtk::SelectionData &data);
    std::list<TileStyle*> get_selected_unsorted_tilestyles();
    std::list<TileStyle*> get_selected_category_tilestyles();
    int get_selected_category();

    void on_categories_drop_drag_data_received(const Glib::RefPtr<Gdk::DragContext> &context, int a, int b, const Gtk::SelectionData& selection_data, guint c, guint time);
    void on_category_drop_drag_data_received(const Glib::RefPtr<Gdk::DragContext> &context, int a, int b, const Gtk::SelectionData& selection_data, guint c, guint time);
    void on_unsorted_drop_drag_data_received(const Glib::RefPtr<Gdk::DragContext> &context, int a, int b, const Gtk::SelectionData& selection_data, guint c, guint time);
    void on_category_tilestyle_activated(const Gtk::TreeModel::Path &path);
    void on_unsorted_tilestyle_activated(const Gtk::TreeModel::Path &path);

    void on_drag_begin(Gtk::IconView *i);
    std::list<TileStyle*> selected_category_tilestyles;
    void on_selection_made(Gtk::IconView *iconview);
    Glib::TimeVal time_of_last_selection;
    std::vector<Gtk::TreeModel::Path> last_multiple_selection;
    bool inhibit_select;
    sigc::connection selection_timeout_handler;
    bool expire_selection();
};

#endif
