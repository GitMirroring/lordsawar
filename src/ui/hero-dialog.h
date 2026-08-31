//  Copyright (C) 2007 Ole Laursen
//  Copyright (C) 2007, 2008, 2009, 2010, 2012, 2014, 2015, 2017,
//  2026 Ben Asselstine
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
#ifndef HERO_DIALOG_H
#define HERO_DIALOG_H
#include "heroes-map.h"
#include "image-helpers.h"
#include "lw-column.h"
class HeroItemRow: public Glib::Object
{
public:
    Item *m_item;
    Glib::ustring m_name;
    Glib::ustring m_status;

    static Glib::RefPtr<HeroItemRow> create (Item *item, Glib::ustring name, Glib::ustring status)
      {
        return
          Glib::make_refptr_for_instance<HeroItemRow> (new HeroItemRow (item, name, status));
      }

protected:
    HeroItemRow (Item *item, Glib::ustring name, Glib::ustring status)
      : m_item (item), m_name (name), m_status (status)
      {
      }
};

class HeroEventRow: public Glib::Object
{
public:
    Glib::ustring m_desc;

    static Glib::RefPtr<HeroEventRow> create (Glib::ustring d)
      {
        return
          Glib::make_refptr_for_instance<HeroEventRow> (new HeroEventRow (d));
      }

protected:
    HeroEventRow (Glib::ustring d)
      : m_desc (d)
      {
      }
};

class HeroDialog: public LwDialogBase
{
public:
    static std::string get_resource_name ()
      {
        return "hero.ui";
      }

    HeroDialog (BaseObjectType* o,
                const Glib::RefPtr<Gtk::Builder>& xml)
      : LwDialogBase (o, xml)
      {
        m_close_button = load <Gtk::Button> ("close_button");
        m_map_drawing_area = load <Gtk::DrawingArea> ("map_drawing_area");
        m_item_treeview = load <Gtk::ColumnView> ("item_treeview");
        m_event_treeview = load <Gtk::ColumnView> ("event_treeview");
        m_drop_button = load <Gtk::Button> ("drop_button");
        m_pick_up_button = load <Gtk::Button> ("pick_up_button");
        m_battle_label = load <Gtk::Label> ("battle_label");
        m_strength_label = load <Gtk::Label> ("strength_label");
        m_command_label = load <Gtk::Label> ("command_label");
        m_moves_label = load <Gtk::Label> ("moves_label");
        m_level_label = load <Gtk::Label> ("level_label");
        m_upkeep_label = load <Gtk::Label> ("upkeep_label");
        m_experience_label = load <Gtk::Label> ("experience_label");
        m_next_button = load <Gtk::Button> ("next_button");
        m_prev_button = load <Gtk::Button> ("prev_button");
      }

    void setup (Hero *hero)
      {
        set_response (m_close_button, Gtk::ResponseType::ACCEPT);

        m_heroes = Playerlist::getActiveplayer ()->getHeroes ();
        m_hero = m_heroes.begin ();
        if (hero)
          set_hero (hero);

        m_heroes_map = new HeroesMap (m_heroes);
        m_heroes_map->map_changed.connect
          ([this] (Cairo::RefPtr<Cairo::Surface> map)
           {
             cairo_surface_to_drawing_area (map, m_map_drawing_area);
           });

        m_heroes_map->resize ();
        m_heroes_map->draw ();

        auto click = Gtk::GestureClick::create ();
        click->set_button (1);
        click->signal_pressed ().connect
          ([this] (int n, double x, double y)
           {
             struct MouseButtonEvent ev = to_input_event (n, x, y, true);
             m_heroes_map->mouse_button_event (ev);
             if (m_heroes_map->get_hero ())
               {
                 set_hero (m_heroes_map->get_hero ());
                 m_heroes_map->draw ();
                 fill_hero (*m_hero);
               }
           });
        m_map_drawing_area->add_controller (click);

        auto motion = Gtk::EventControllerMotion::create ();
        motion->signal_motion ().connect
          ([this](double x, double y)
           {
             auto c = m_heroes_map->get_cursor (x, y);
             static ImageCache::CursorType prev_cursor = ImageCache::SHIP;
             if (c != prev_cursor)
               {
                 auto hotspot = ImageCache::get_hotspot (c);
                 auto im = ImageCache::instance ()->getCursorPic (c);
                 auto cursor = Gdk::Cursor::create (im->to_texture (),
                                                    hotspot.x, hotspot.y);
                 m_map_drawing_area->set_cursor (cursor);
               }
             prev_cursor = c;
           });
        m_map_drawing_area->add_controller (motion);

        m_next_button->set_sensitive (m_heroes.size () > 1);
        m_prev_button->set_sensitive (m_heroes.size () > 1);

        m_item_store = Gio::ListStore<HeroItemRow>::create ();
        m_item_selection_model = Gtk::SingleSelection::create (m_item_store);
        m_item_treeview->set_model (m_item_selection_model);

        setup_item_name_column ();
        setup_item_status_column ();

         m_item_selection_model->signal_selection_changed ().connect
           ([this] (const guint &, const guint &)
            {
              update_buttons ();
            });

        m_event_store = Gio::ListStore<HeroEventRow>::create ();
        m_event_selection_model = Gtk::NoSelection::create (m_event_store);
        m_event_treeview->set_model (m_event_selection_model);

        setup_event_column ();

        m_next_button->signal_clicked ().connect
          ([this] ()
           {
             m_hero++;
             if (m_hero == m_heroes.end ())
               m_hero = m_heroes.begin ();
             m_heroes_map->set_hero (*m_hero);
             m_heroes_map->draw ();
             fill_hero (*m_hero);
           });

        m_prev_button->signal_clicked ().connect
          ([this] ()
           {
             if (m_hero == m_heroes.begin ())
               m_hero = std::prev (m_heroes.end ());
             else
               --m_hero;
             m_heroes_map->set_hero (*m_hero);
             m_heroes_map->draw ();
             fill_hero (*m_hero);
           });

        m_drop_button->signal_clicked ().connect
          ([this] ()
           {
             auto item = m_item_selection_model->get_selected_item ();
             if (item)
               {
                 auto row = std::dynamic_pointer_cast<HeroItemRow> (item);
                 Hero *h = *m_hero;
                 Item *i = row->m_item;
                 Vector<int> pos =
                   h->getOwner ()->getStacklist ()->getPosition (h->getId ());
                 bool splash = false;
                 h->getOwner ()->heroDropItem (h, i, pos, splash);
                 if (splash == false)
                   row->m_status = on_the_ground ();
                 else
                   {
                     m_item_store->remove
                       (m_item_selection_model->get_selected ());
                   }
                 fill_item_treeview (*m_hero);
                 fill_info_labels (*m_hero);
                 update_buttons ();
               }
           });

        m_pick_up_button->signal_clicked ().connect
          ([this] ()
           {
             auto item = m_item_selection_model->get_selected_item ();
             if (item)
               {
                 auto row = std::dynamic_pointer_cast<HeroItemRow> (item);
                 Hero *h = *m_hero;
                 Item *i = row->m_item;
                 if (i->getPlanted () == true)
                   i->setPlanted (false);
                 Vector<int> pos =
                   h->getOwner ()->getStacklist ()->getPosition (h->getId ());
                 h->getOwner ()->heroPickupItem (h, i, pos);
                 row->m_status = in_backpack ();
                 fill_item_treeview (*m_hero);
                 fill_info_labels (*m_hero);
                 update_buttons ();
               }
           });

        fill_hero (*m_hero);
      }

    Glib::ustring in_backpack ()
      {
        return _("In backpack");
      }

    Glib::ustring on_the_ground ()
      {
        return _("On the ground");
      }
private:
    Gtk::Button *m_close_button;
    Gtk::DrawingArea *m_map_drawing_area;
    Gtk::ColumnView *m_item_treeview;
    Gtk::ColumnView *m_event_treeview;
    Gtk::Button *m_drop_button;
    Gtk::Button *m_pick_up_button;
    Gtk::Label *m_battle_label;
    Gtk::Label *m_strength_label;
    Gtk::Label *m_command_label;
    Gtk::Label *m_moves_label;
    Gtk::Label *m_level_label;
    Gtk::Label *m_upkeep_label;
    Gtk::Label *m_experience_label;
    Gtk::Button *m_next_button;
    Gtk::Button *m_prev_button;
    Glib::RefPtr<Gtk::SingleSelection> m_item_selection_model;
    Glib::RefPtr<Gio::ListStore<HeroItemRow>> m_item_store;
    Glib::RefPtr<Gtk::NoSelection> m_event_selection_model;
    Glib::RefPtr<Gio::ListStore<HeroEventRow>> m_event_store;

    HeroesMap *m_heroes_map = NULL;

    std::list<Hero*> m_heroes;
    std::list<Hero*>::iterator m_hero;

    Glib::ustring history_to_string (History *history)
      {
        Glib::ustring s = "";

        switch (history->getType ())
          {
          case History::FOUND_SAGE:
              {
                auto *ev = static_cast<History_FoundSage *>(history);
                s = String::ucompose (_("%1 finds a sage!"),
                                      ev->getHeroName ());
                break;
              }

          case History::HERO_EMERGES:
              {
                auto *ev = static_cast<History_HeroEmerges *>(history);
                s = String::ucompose (_("%1 emerges in %2!"),
                                      ev->getHeroName (), ev->getCityName ());
                break;
              }

          case History::HERO_QUEST_STARTED:
              {
                auto *ev = static_cast<History_HeroQuestStarted*>(history);
                s = String::ucompose (_("%1 begins a quest!"),
                                      ev->getHeroName ());
                break;
              }

          case History::HERO_QUEST_EXPIRED:
              {
                auto *ev = static_cast<History_HeroQuestExpired *>(history);
                s = String::ucompose (_("%1 failed to complete a quest!"),
                                      ev->getHeroName ());
                break;
              }

          case History::HERO_QUEST_COMPLETED:
              {
                auto *ev = static_cast<History_HeroQuestCompleted *>(history);
                s = String::ucompose (_("%1 finishes a quest!"),
                                      ev->getHeroName ());
                break;
              }

          case History::HERO_KILLED_IN_CITY:
              {
                auto *ev = static_cast<History_HeroKilledInCity *>(history);
                s = String::ucompose (_("%1 is killed in %2!"),
                                      ev->getHeroName (), ev->getCityName ());
                break;
              }

          case History::HERO_KILLED_IN_BATTLE:
              {
                auto *ev = static_cast<History_HeroKilledInBattle *>(history);
                s = String::ucompose (_("%1 is killed in battle!"),
                                      ev->getHeroName ());
                break;
              }

          case History::HERO_KILLED_SEARCHING:
              {
                auto *ev = static_cast<History_HeroKilledSearching *>(history);
                s = String::ucompose (_("%1 is killed while searching!"),
                                      ev->getHeroName ());
                break;
              }

          case History::HERO_CITY_WON:
              {
                auto *ev = static_cast<History_HeroCityWon *>(history);
                s = String::ucompose (_("%1 conquers %2!"), ev->getHeroName (),
                                      ev->getCityName ());
                break;
              }

          case History::HERO_FINDS_ALLIES:
              {
                auto *ev = static_cast<History_HeroFindsAllies*>(history);
                s = String::ucompose (_("%1 finds allies!"),
                                      ev->getHeroName ());
                break;
              }

          default:
            s = _("unknown");
            break;
          }

        return s;
      }

    void setup_event_column ()
      {
        LwColumn::setup_text_column<HeroEventRow>
          (m_event_treeview, "event_desc_label", true, Gtk::Justification::LEFT,
           _("Events"),
           [] (const auto& row)
           {
             return row->m_desc;
           });
      }

    void setup_item_name_column ()
      {
        LwColumn::setup_text_column<HeroItemRow>
          (m_item_treeview, "item_name_label", true, Gtk::Justification::LEFT,
           _("Item"),
           [] (const auto& row)
           {
             return row->m_name;
           });
      }

    void setup_item_status_column ()
      {
        LwColumn::setup_text_column<HeroItemRow>
          (m_item_treeview, "itet_status_label", false, Gtk::Justification::LEFT,
           "",
           [] (const auto& row)
           {
             return row->m_status;
           });
      }

    void fill_info_labels (Hero *h)
      {
        guint32 bonus = 0;
        Backpack *backpack = h->getBackpack ();
        for (auto item : *backpack)
          {
            if (item->getBonus (Item::ADD1STR))
              bonus += 1;
            if (item->getBonus (Item::ADD2STR))
              bonus += 2;
            if (item->getBonus (Item::ADD3STR))
              bonus += 3;
          }
        m_battle_label->set_text (String::ucompose ("%1", bonus));

        bonus = 0;
        for (auto item : *backpack)
          {
            if (item->getBonus (Item::ADD1STACK))
              bonus += 1;
            if (item->getBonus (Item::ADD2STACK))
              bonus += 2;
            if (item->getBonus (Item::ADD3STACK))
              bonus += 3;
          }

        //now add natural command
        bonus += h->calculateNaturalCommand ();

        m_command_label->set_text (String::ucompose ("%1", bonus));
        m_level_label->set_text (String::ucompose ("%1", h->getLevel ()));
        m_experience_label->set_text
          (String::ucompose ("%1", int(h->getXP ())));

        m_strength_label->set_text
          (String::ucompose ("%1", h->getStat (Army::STRENGTH)));

        // note to translators: %1 is remaining moves, %2 is total moves
        m_moves_label->set_text
          (String::ucompose (_("%1/%2"),
                             h->getMoves (), h->getStat (Army::MOVES)));
        m_upkeep_label->set_text (String::ucompose ("%1", h->getUpkeep ()));
      }

    void fill_hero (Hero *h)
      {
        set_title (h->getName ());
        fill_info_labels (h);
        fill_event_treeview (h);
        fill_item_treeview (h);
        update_buttons ();
      }
            
    void set_hero (Hero *hero)
      {
        // zip to the hero passed as a parameter
        m_hero = m_heroes.begin ();
        for (; m_hero != m_heroes.end (); ++m_hero)
          if (*m_hero == hero)
            break;
      }

    void fill_event_treeview (Hero *h)
      {
        std::list<History*> events =
          h->getOwner ()->getHistoryForHeroId (h->getId ());
        m_event_store->remove_all ();
        for (auto ev : events)
          m_event_store->append (HeroEventRow::create (history_to_string (ev)));
      }

    void fill_item_treeview (Hero *h)
      {
        m_item_store->remove_all ();
        Backpack *backpack = h->getBackpack ();
        for (auto item : *backpack)
          add_item (item, true);

        Vector<int> pos =
          h->getOwner ()->getStacklist ()->getPosition (h->getId ());
        MapBackpack *ground = GameMap::instance ()->getTile (pos)->getBackpack ();
        for (auto item : *ground)
          add_item (item, false);
      }

    void add_item (Item *item, bool in_pack)
      {
        std::stringstream ss;
        ss << std::endl;
        Glib::ustring newline = ss.str();

        Glib::ustring name = 
          item->getName () + newline +
          String::indent (item->getBonusDescription (), 2);

        Glib::ustring status = in_backpack ();
        if (!in_pack)
          status = on_the_ground ();

        m_item_store->append (HeroItemRow::create (item, name, status));
      }

              
    void update_buttons ()
      {
        auto item = m_item_selection_model->get_selected_item ();
        if (item)
          {
            auto row = std::dynamic_pointer_cast<HeroItemRow> (item);
            if (row->m_status == on_the_ground ())
              {
                m_pick_up_button->set_sensitive (true);
                m_drop_button->set_sensitive (false);
              }
            else // in the pack
              {
                m_pick_up_button->set_sensitive (false);
                m_drop_button->set_sensitive (true);
              }
          }
        else
          {
            m_pick_up_button->set_sensitive (false);
            m_drop_button->set_sensitive (false);
          }
      }
};
#endif
