//  Copyright (C) 2010, 2014, 2026 Ben Asselstine
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
#ifndef USE_ITEM_DIALOG_H
#define USE_ITEM_DIALOG_H
#include "lw-dialog-base.h"
#include "army-set-list.h"
#include "player.h"
#include "lw-column.h"
class UseItemRow: public Glib::Object
{
public:
    Item *m_item;

    static Glib::RefPtr<UseItemRow> create (Item *i)
      {
        return
          Glib::make_refptr_for_instance<UseItemRow> (new UseItemRow (i));
      }

protected:
    UseItemRow (Item *i)
      : m_item (i)
      {
      }
};

class UseItemDialog: public LwDialogBase
{
public:
    static std::string get_resource_name ()
      {
        return "use-item.ui";
      }

    UseItemDialog (BaseObjectType* o,
                   const Glib::RefPtr<Gtk::Builder>& xml)
      : LwDialogBase (o, xml)
      {
        m_select_button = load <Gtk::Button> ("select_button");
        m_close_button = load <Gtk::Button> ("close_button");
        m_treeview = load <Gtk::ColumnView> ("treeview");
        m_overlay = load <Gtk::Overlay> ("overlay");
        m_revealer = load <Gtk::Revealer> ("revealer");
        m_error_box = load <Gtk::Box> ("error_box");
        m_error_label = load <Gtk::Label> ("error_label");
      }

    void setup (std::list<Item*> items)
      {
        set_response (m_close_button, Gtk::ResponseType::CLOSE);
        set_response (m_select_button, Gtk::ResponseType::ACCEPT);

        m_store = Gio::ListStore<UseItemRow>::create ();
        m_selection_model = Gtk::SingleSelection::create (m_store);
        m_treeview->set_model (m_selection_model);

        for (auto i : items)
          {
            if (i->getNumberOfUsesLeft () > 0)
              m_store->append (UseItemRow::create (i));
          }
        setup_item_name_column ();
        setup_item_description_column ();
        setup_uses_left_column ();

        m_overlay->add_overlay (*m_revealer);
        m_overlay->set_clip_overlay (*m_revealer, false);

        signal_response ().connect
          ([this](Gtk::ResponseType resp)
           {
             if (resp == Gtk::ResponseType::ACCEPT &&
                 !check_item (get_selected_item ()))
               return;
             hide ();
             switch (resp)
               {
               case Gtk::ResponseType::ACCEPT:
                 m_selected_item.emit (get_selected_item ());
                 break;
               default:
                 break;
               }
           });
      }

    void show_error (Glib::ustring message)
      {
        m_error_label->set_text (message);
        m_revealer->set_reveal_child (true);
        Glib::signal_timeout().connect_once
          ([this] ()
           {
             m_revealer->set_reveal_child (false);
           }, 3000);
      }

    bool check_item (Item *item)
      {
        if (item->getBonus () & ItemProto::BURN_BRIDGE)
          {
            auto stack = Playerlist::getActiveplayer ()->getActivestack ();
            auto bridge = GameMap::getBridge (stack->getPos ());
            if (!bridge)
              {
                show_error (_("Must be standing on a bridge!"));
                return false;
              }
          }
        else if (item->getBonus () & ItemProto::SUMMON_MONSTER)
          {
            auto stack = Playerlist::getActiveplayer ()->getActivestack ();
            auto temple = GameMap::getTemple (stack->getPos ());
            if (!temple)
              {
                show_error (_("Must be standing on a temple!"));
                return false;
              }
          }
        else if (item->getBonus () & ItemProto::SINK_SHIPS)
          {
            int num_army_units =
              Playerlist::instance ()->getArmyUnitsInBoats ().size ();
            if (num_army_units == 0)
              {
                show_error (_("There aren't any ships to sink!"));
                return false;
              }
          }
        else if (item->getBonus () & ItemProto::PICK_UP_BAGS)
          {
            if (GameMap::instance ()->countBags () == 0)
              {
                show_error (_("There aren't any bags to pick up!"));
                return false;
              }
          }
        else if (item->getBonus () & ItemProto::BANISH_WORMS)
          {
            guint32 w = item->getArmyTypeToKill ();
            Player *p = Playerlist::getActiveplayer ();
            auto army = Armysetlist::instance ()->getArmy (p->getArmyset (), w);
            if (Playerlist::instance ()->countArmies (w) == 0)
              {
                show_error
                  (String::ucompose
                   (_("There aren't any %1 to banish!"), army->getName ()));
                return false;
              }
          }
        else if (item->getBonus () & ItemProto::CAPTURE_KEEPER)
          {
            auto stack = Playerlist::getActiveplayer ()->getActivestack ();
            auto ruin = GameMap::getRuin (stack->getPos ());
            if (!ruin || ruin->isSearched () == false)
              {
                show_error (_("Must be standing on an unexplored ruin!"));
                return false;
              }
          }
        else if (item->getBonus () & ItemProto::PERSUADE_NEUTRALS)
          {
            auto num_neutral_cities =
              Citylist::instance ()->countCities (Playerlist::getNeutral ());
            if (num_neutral_cities == 0)
              {
                show_error (_("There aren't any neutral cities!"));
                return false;
              }
          }
        else if (item->getBonus () & ItemProto::RAISE_DEFENDERS)
          {
            auto num_cities =
              Citylist::instance ()->countCities
              (Playerlist::getActiveplayer ());
            if (num_cities == 0)
              {
                show_error (_("You don't have any cities!"));
                return false;
              }
          }

        return true;
      }

    sigc::signal<void(Item*)> signal_selected_item ()
      {
        return m_selected_item;
      }
private:
    Gtk::Button *m_close_button = NULL;
    Gtk::Button *m_select_button = NULL;
    Gtk::Overlay *m_overlay = NULL;
    Gtk::Revealer *m_revealer = NULL;
    Gtk::Box *m_error_box = NULL;
    Gtk::Label *m_error_label = NULL;
    Gtk::ColumnView *m_treeview = NULL;
    Glib::RefPtr<Gtk::SingleSelection> m_selection_model;
    Glib::RefPtr<Gio::ListStore<UseItemRow>> m_store;
    sigc::signal<void(Item*)> m_selected_item;

    Item * get_selected_item ()
      {
        auto single_selection =
          std::dynamic_pointer_cast<Gtk::SingleSelection> (m_selection_model);

        auto item = single_selection->get_selected_item ();

        if (item)
          {
            auto row = std::dynamic_pointer_cast<UseItemRow> (item);
            return row->m_item;
          }
        return NULL;
      }

    void setup_item_name_column ()
      {
        LwColumn::setup_text_column<UseItemRow>
          (m_treeview, "name_label", false, Gtk::Justification::LEFT,
           _("Name"),
           [] (const auto& row)
           {
             return row->m_item->getName ();
           });
      }

    void setup_item_description_column ()
      {
        LwColumn::setup_text_column<UseItemRow>
          (m_treeview, "desc_label", true, Gtk::Justification::LEFT,
           _("Description"),
           [] (const auto& row)
           {
             return row->m_item->getBonusDescription ();
           });
      }

    void setup_uses_left_column ()
      {
        LwColumn::setup_text_column<UseItemRow>
          (m_treeview, "uses_left_label", false, Gtk::Justification::CENTER,
           _("Uses Remaining"),
           [] (const auto& row)
           {
             return String::ucompose ("%1",
                                      row->m_item->getNumberOfUsesLeft ());
           });
      }

};
#endif
