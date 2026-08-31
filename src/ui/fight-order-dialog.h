//  Copyright (C) 2007, 2008, 2009, 2011, 2012, 2014, 2020, 2026 Ben Asselstine
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
#include "lw-dialog-base.h"
#ifndef FIGHT_ORDER_DIALOG_H
#define FIGHT_ORDER_DIALOG_H
#include "army-set-list.h"
#include "player.h"
#include "lw-column.h"
class FightOrderRow: public Glib::Object
{
public:
    PixMask *m_image;
    Glib::ustring m_name;
    guint32 m_army_type;

    static Glib::RefPtr<FightOrderRow> create (PixMask *i, Glib::ustring n,
                                               guint32 a)
      {
        return
          Glib::make_refptr_for_instance<FightOrderRow>
          (new FightOrderRow (i, n, a));
      }

protected:
    FightOrderRow (PixMask *image, Glib::ustring name,
                   guint32 army_type)
      : m_image (image), m_name (name), m_army_type (army_type)
      {
      }
};

class FightOrderDialog: public LwDialogBase
{
public:
    static std::string get_resource_name ()
      {
        return "fight-order.ui";
      }

    FightOrderDialog (BaseObjectType* o,
                      const Glib::RefPtr<Gtk::Builder>& xml)
      : LwDialogBase (o, xml)
      {
        m_accept_button = load <Gtk::Button> ("accept_button");
        m_cancel_button = load <Gtk::Button> ("cancel_button");
        m_up_button = load <Gtk::Button> ("up_button");
        m_down_button = load <Gtk::Button> ("down_button");
        m_treeview = load <Gtk::ColumnView> ("treeview");
      }

    void setup (Player *p)
      {
        set_response (m_accept_button, Gtk::ResponseType::ACCEPT);
        set_response (m_cancel_button, Gtk::ResponseType::CANCEL);

        m_store = Gio::ListStore<FightOrderRow>::create ();
        m_selection_model = Gtk::SingleSelection::create (m_store);
        m_treeview->set_model (m_selection_model);

        std::list<guint32> fight_order = p->getFightOrder ();
        for (auto it = fight_order.begin (); it != fight_order.end (); ++it)
          {
            const ArmyProto *a =
              Armysetlist::instance ()->getArmy (p->getArmyset (), *it);
            auto pix =
              ImageCache::instance ()->getCircledArmyPic
              (p->getArmyset (), *it, p->get_shield (), NULL, false,
               p->getId (), true, Lw::get_dark ());
            m_store->append (FightOrderRow::create (pix, a->getName (), *it));
          }
        setup_image_column ();
        setup_name_column ();

        m_up_button->signal_clicked ().connect
          ([this] ()
           {
             int i = m_selection_model->get_selected ();
             if (i <= 0)
               return;

             auto item = m_store->get_item (i);
             if (!item)
               return;

             m_store->remove (i);
             m_store->insert (i - 1, item);

             reselection (item);
               
             auto index = m_selection_model->get_selected ();
             if (index < m_store->get_n_items ())
               m_treeview->scroll_to (index);
           });

        m_down_button->signal_clicked ().connect
          ([this] ()
           {
             int i = m_selection_model->get_selected ();
             if (i < 0 || i + 1 >= (int)m_store->get_n_items ())
               return;

             auto item = m_store->get_item (i);
             if (!item)
               return;

             m_store->remove (i);
             m_store->insert (i + 1, item);

             reselection (item);

             //fixme, it doesn't scroll correctly here, the one we moved is
             //off screen down one record.
             //we try to fix it with +1 so we can at least see it, 
             //but this causes a problem when we move a record to the bottom.
             //i think most ppl will sort upwards and not downwards to the very
             //bottom, so maybe people won't interact with this bug.
             auto index = m_selection_model->get_selected ();
             if (index + 1 < m_store->get_n_items ())
               m_treeview->scroll_to (index + 1);
           });

        signal_response ().connect
          ([this](Gtk::ResponseType resp)
           {
             hide ();
             switch (resp)
               {
               case Gtk::ResponseType::ACCEPT:
                 m_change_fight_order.emit (get_fight_order ());
                 break;
               default:
                 break;
               }
           });
      }

    std::list<guint32> get_fight_order ()
      {
        std::list<guint32> order;
        for (guint i = 0; i < m_store->get_n_items (); ++i)
          {
            auto item = m_store->get_item (i);
            order.push_back (item->m_army_type);
          }
        return order;
      }

    sigc::signal<void(std::list<guint32>)> signal_change_fight_order ()
      {
        return m_change_fight_order;
      }

private:
    Gtk::Button *m_accept_button = NULL;
    Gtk::Button *m_cancel_button = NULL;
    Gtk::Button *m_up_button = NULL;
    Gtk::Button *m_down_button = NULL;
    Gtk::ColumnView *m_treeview = NULL;
    Glib::RefPtr<Gtk::SingleSelection> m_selection_model;
    Glib::RefPtr<Gio::ListStore<FightOrderRow>> m_store;
    sigc::signal<void(std::list<guint32>)> m_change_fight_order;

    void setup_image_column ()
      {
        LwColumn::setup_picture_column<FightOrderRow>
          (m_treeview, LW_BUTTON_SIZE, LW_BUTTON_SIZE, "army_image", "",
           [] (const auto& row)
           {
             return row->m_image->to_texture ();
           });
      }

    void setup_name_column ()
      {
        LwColumn::setup_text_column<FightOrderRow>
          (m_treeview, "name_label", true, Gtk::Justification::LEFT,
           _("Name"),
           [] (const auto& row)
           {
             return row->m_name;
           });
      }

    void reselection(const Glib::RefPtr<FightOrderRow>& target)
      {
        guint n = m_store->get_n_items ();

        for (guint i = 0; i < n; i++)
          {
            if (m_store->get_item (i) == target)
              {
                m_selection_model->select_item (i, true);
                return;
              }
          }
      }
};
#endif
