//  Copyright (C) 2007, 2008, 2009, 2014, 2026 Ben Asselstine
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
//  Foundation, Inc., 31 Milk Street #960789, Boston, MA 02196, USA.

#include <gtkmm.h>
#ifndef ITEM_BONUS_DIALOG_H
#define ITEM_BONUS_DIALOG_H
#include "lw-dialog-base.h"
#include "army-set-list.h"
#include "player.h"
#include "game-map.h"
#include "tile-set.h"
#include "image-cache.h"
#include "player-list.h"
#include "item-list.h"
#include "item.h"
#include "lw-column.h"

class ItemBonusRow: public Glib::Object
{
public:
    ItemProto *m_item;

    static Glib::RefPtr<ItemBonusRow> create (ItemProto *i)
      {
        return
          Glib::make_refptr_for_instance<ItemBonusRow>
          (new ItemBonusRow (i));
      }

protected:
    ItemBonusRow (ItemProto *i)
      :m_item (i)
      {
      }
};

class ItemBonusDialog: public LwDialogBase
{
public:
    static std::string get_resource_name ()
      {
        return "item-bonus.ui";
      }

    ItemBonusDialog (BaseObjectType* o,
                     const Glib::RefPtr<Gtk::Builder>& xml)
      : LwDialogBase (o, xml)
      {
        m_close_button = load <Gtk::Button> ("close_button");
        m_treeview = load <Gtk::ColumnView> ("treeview");
      }

    void setup ()
      {
        set_response (m_close_button, Gtk::ResponseType::CLOSE);

        m_store = Gio::ListStore<ItemBonusRow>::create ();
        m_selection_model = Gtk::NoSelection::create (m_store);
        m_treeview->set_model (m_selection_model);

        setup_name_column ();
        setup_bonus_column ();

        fill_items ();

        signal_response ().connect
          ([this](Gtk::ResponseType)
           {
             hide ();
           });
      }

private:
    Gtk::Button *m_close_button = NULL;
    Gtk::ColumnView *m_treeview = NULL;
    Glib::RefPtr<Gtk::NoSelection> m_selection_model;
    Glib::RefPtr<Gio::ListStore<ItemBonusRow>> m_store;

    void fill_items ()
      {
        m_store->remove_all ();
        for (auto i : *Itemlist::instance ())
          m_store->append (ItemBonusRow::create (i.second));
      }

    void setup_name_column ()
      {
        LwColumn::setup_text_column<ItemBonusRow>
          (m_treeview, "name_label",  true, Gtk::Justification::LEFT,
           _("Name"),
           [] (const auto& row)
           {
             return row->m_item->getName ();
           });
      }

    void setup_bonus_column ()
      {
        LwColumn::setup_text_column<ItemBonusRow>
          (m_treeview, "bonus_label",  true, Gtk::Justification::LEFT,
           _("Bonus"),
           [] (const auto& row)
           {
             return row->m_item->getBonusDescription ();
           });
      }
};
#endif
