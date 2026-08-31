//  Copyright (C) 2007, 2008, Ole Laursen
//  Copyright (C) 2007, 2008, 2009, 2010, 2012, 2014, 2015, 2020,
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
#ifndef FIGHT_WINDOW_H
#define FIGHT_WINDOW_H
#include <cairomm/context.h>
#include "snd.h"
#include "tile-set-list.h"
#include "army.h"
#include "game-map.h"
#include "fight.h"
class FightWindow : public Gtk::Window
{
public:
    static const int max_cols = 8;
    static const int water_height = LW_BUTTON_SIZE / 10;
    inline static bool s_quick = false;
    inline static int s_quick_count = 0;


    struct ArmyItem
    {
        Army *army;
        int hp;
        Gtk::DrawingArea *water_image;
        Gtk::DrawingArea *image;
        bool exploding;
    };

    struct FighterBox
      {
        Gtk::Box *box;
        Gtk::DrawingArea *army;
        Gtk::DrawingArea *water;
      };

    FightWindow (Gtk::Window &parent)
      {
        set_transient_for (parent);
        set_decorated (false);
        set_resizable (false);
        set_modal (true);
        set_can_focus (false);
        add_css_class ("rounded");

        guint32 height = LW_BUTTON_SIZE * 4.25;
        guint32 width = height * (16.0 / 9.0);
        set_size_request (width, height);

        m_quick_count = s_quick_count;
        auto key_controller = Gtk::EventControllerKey::create ();
        key_controller->signal_key_pressed ().connect
          ([this] (guint keyval, guint, Gdk::ModifierType) -> bool
           {
             if (keyval == GDK_KEY_exclam)
               s_quick = !s_quick;
             else
               {
                 m_quick = true;
                 m_quick_count++;
               }
             return false;
           }, false);
        add_controller (key_controller);

        m_vbox = new Gtk::Box (Gtk::Orientation::VERTICAL);
        m_vbox->set_spacing (18);
        m_vbox->set_margin (12);
        set_child (*m_vbox);

        // row 1
        m_defenders_hbox = new Gtk::Box (Gtk::Orientation::HORIZONTAL);
        m_vbox->append (*m_defenders_hbox);

        m_defender_shield_picture = new Gtk::Picture ();
        m_defenders_hbox->append (*m_defender_shield_picture);

        m_defender_close_vbox = new Gtk::Box (Gtk::Orientation::VERTICAL);
        m_defender_close_vbox->set_spacing (12);
        m_defenders_hbox->append (*m_defender_close_vbox);

        // row 2
        m_separator_label = new Gtk::Label ("");
        m_vbox->append (*m_separator_label);

        // row 3
        m_attackers_hbox = new Gtk::Box (Gtk::Orientation::HORIZONTAL);
        m_vbox->append (*m_attackers_hbox);

        m_attacker_shield_picture = new Gtk::Picture ();
        m_attackers_hbox->append (*m_attacker_shield_picture);

        m_attacker_close_vbox = new Gtk::Box (Gtk::Orientation::VERTICAL);
        m_attacker_close_vbox->set_spacing (12);
        m_attackers_hbox->append (*m_attacker_close_vbox);

        // row 4
        m_decision_label = new Gtk::Label ("");
        m_vbox->append (*m_decision_label);

        Snd::instance ()->disableBackground ();
        Snd::instance ()->play ("battle", -1, true);
      }

    guint32 delay ()
      {
        if (m_quick_count > 1)
          {
            int ms = Configuration::s_displayFightRoundDelayFast;
            for (int i = 0; i < m_quick_count - 1; i++)
              ms /= 2;
            return ms;
          }
        if (s_quick)
          return Configuration::s_displayFightRoundDelayFast;
        if (m_quick)
          return Configuration::s_displayFightRoundDelayFast;
        else
          return Configuration::s_displayFightRoundDelaySlow;
      }

    void setup (Fight *fight)
      {
        m_fight = fight;
        m_on_city = fight->get_fighting_in_city ();
        std::vector<Army *> defenders;

        Fight::orderArmies (fight->getAttackers (), m_attackers);
        Fight::orderArmies (fight->getDefenders (), defenders);
  
        std::map<guint32, guint32> initial_hps = fight->getInitialHPs ();
  
        for (auto i = m_attackers.begin (); i != m_attackers.end (); ++i)
          add_army (*i, initial_hps[(*i)->getId ()], m_attacker_close_vbox);

        // zip it forward, so defenders start on their own row
        m_current_no = ((m_current_no + max_cols - 1) / max_cols) * max_cols;

        for (auto i = defenders.begin (); i != defenders.end (); ++i)
          add_army (*i, initial_hps[(*i)->getId ()], m_defender_close_vbox);
                                                     
        Player *p = defenders.front ()->getOwner ();
        auto im = ImageCache::instance ()->getShieldPic (2, p, false);
        m_defender_shield_picture->set_paintable (im->to_texture ());

        p = m_attackers.front ()->getOwner ();
        im = ImageCache::instance ()->getShieldPic (2, p, false);
        m_attacker_shield_picture->set_paintable (im->to_texture ());

        m_actions = fight->getCourseOfEvents ();
      }

    ~FightWindow ()
      {
        Snd::instance ()->halt (true);
        Snd::instance ()->enableBackground ();

        for (auto b : m_fighter_boxes)
          {
            delete b.box;
            delete b.army;
            delete b.water;
          }

        for (auto b : m_hboxes)
          delete b;

        delete m_vbox;
        delete m_defenders_hbox;
        delete m_attackers_hbox;
        delete m_separator_label;
        delete m_decision_label;
        delete m_defender_shield_picture;
        delete m_attacker_shield_picture;
        delete m_defender_close_vbox;
        delete m_attacker_close_vbox;
      }

    static void speedy ()
      {
        s_quick = true;
        s_quick_count = 4;
      }

    sigc::signal<void()> signal_battle_finished ()
      {
        return m_battle_finished;
      }

    void do_battle ()
      {
        set_visible (true);
        m_current_action = m_actions.begin ();
        
        // the fight keeps going because do_round calls itself on a timer
        Glib::signal_timeout ().connect_once
          ([this] ()
           {
             do_round ();
           }, delay ());
      }

private:
    sigc::signal<void()> m_battle_finished;
    Gtk::Box *m_vbox;
    Gtk::Box *m_defenders_hbox;
    Gtk::Box *m_attackers_hbox;
    Gtk::Label *m_separator_label;
    Gtk::Label *m_decision_label;
    Gtk::Picture *m_defender_shield_picture;
    Gtk::Picture *m_attacker_shield_picture;
    Gtk::Box *m_defender_close_vbox;
    Gtk::Box *m_attacker_close_vbox;
    std::vector<struct FighterBox> m_fighter_boxes;
    std::vector<Gtk::Box*> m_hboxes;
    int m_current_no = 0;
    std::vector<struct ArmyItem> m_army_items;
    Fight *m_fight;
    std::list<FightItem> m_actions;
    std::list<FightItem>::iterator m_current_action;
    std::vector<Army *> m_attackers;
    bool m_quick = false;
    std::function<void(std::list<Hero*>)> m_next_hero;
    std::function<void(std::list<Army*>)> m_next_army;
    int m_quick_count = 0;
    bool m_on_city;

    void do_round ()
      {
        // first we clear out any explosions
        for (auto &i : m_army_items)
          {
            if (!i.exploding)
              continue;

            i.image->set_draw_func
              (
               [](const Cairo::RefPtr<Cairo::Context>& cr, int, int)
               {
                 cr->set_operator (Cairo::Context::Operator::CLEAR);
                 cr->paint();
                 cr->set_operator(Cairo::Context::Operator::OVER);
               });
            i.image->queue_draw ();
                  
            i.water_image->set_draw_func
              ([](const Cairo::RefPtr<Cairo::Context>& cr, int, int)
               {
                 cr->set_operator (Cairo::Context::Operator::CLEAR);
                 cr->paint ();
               });
            i.water_image->queue_draw ();

            i.exploding = false;

        
            Glib::signal_timeout ().connect_once
              ([this] ()
               {
                 do_round ();
               }, delay ());
            return;
          }

        FightItem &f = *m_current_action;

        // now we find the right army decrement the hitpoints and and then
        // maybe explode it

        for (auto &i : m_army_items)
          if (i.army->getId () == f.id)
            {
              i.hp -= f.damage;
              if (i.hp < 0)
                i.hp = 0;
              double fraction = double(i.hp) / i.army->getStat (Army::HP);
              if (fraction == 0.0)
                {
                  i.image->set_draw_func
                    ([i] (const Cairo::RefPtr<Cairo::Context> &cr, int, int)
                     {
                       auto img =
                         ImageCache::instance ()->getDialogArmyPic (i.army);
                       Gdk::Cairo::set_source_pixbuf
                         (cr, img->to_pixbuf (), 0, 0);
                       cr->paint ();

                       auto expl =
                         ImageCache::instance ()->getExplosionPic ()->copy ();
                       Gdk::Cairo::set_source_pixbuf
                         (cr, expl->to_pixbuf (), 0, 0);
                       cr->paint ();
                       delete expl;
                     });
                  i.image->queue_draw ();
                  i.exploding = true;
                }

              break;
            }

        m_current_action++;

        if (m_current_action == m_actions.end ())
          {
            finalize_battle ();
            return;
          }
        else
          {
            Glib::signal_timeout ().connect_once
              ([this] ()
               {
                 do_round ();
               }, delay ());
          }
        return;
      }

    void finalize_battle ()
      {
        Glib::signal_timeout ().connect_once
          ([this] ()
           {
             m_decision_label->set_text (get_decision_text ());

             Glib::signal_timeout ().connect_once
               ([this] ()
                {
                  hide ();
                  heroes_level_up
                    (m_fight->get_fight_result ().get_advancing_heroes ());
                  return;
                }, delay () * 3);
           }, delay ());
      }

    void add_army (Army *army, int initial_hp, Gtk::Box *vbox)
      {
        struct FighterBox box;

        box.box = new Gtk::Box (Gtk::Orientation::VERTICAL);

        box.army = new Gtk::DrawingArea ();
        box.army->set_size_request (LW_BUTTON_SIZE, LW_BUTTON_SIZE);

        box.water = new Gtk::DrawingArea ();
        box.water->set_size_request (LW_BUTTON_SIZE, water_height);

        box.box->append (*box.army);
        box.box->append (*box.water);

        box.army->set_draw_func
          ([army] (const Cairo::RefPtr<Cairo::Context> &cr, int, int)
           {
             auto pixbuf =
               ImageCache::instance ()->getDialogArmyPic (army)->to_pixbuf ();
             Gdk::Cairo::set_source_pixbuf (cr, pixbuf, 0, 0);
             cr->paint ();
           });

        box.water->set_draw_func
          ([army](const Cairo::RefPtr<Cairo::Context>& cr, int, int)
           {
             SmallTile *water_tile =
               Tilesetlist::instance ()->getSmallTile
               (GameMap::getTileset ()->getBaseName (), Tile::WATER);
             if (army->getStat (Army::SHIP, false) && water_tile)
               {
                 cr->set_source_rgb (0, 0, 1.0);
                 cr->paint ();
               }
           });

        m_fighter_boxes.push_back (box);

        // then add it to the right hbox
        int current_row = (m_current_no / max_cols);

        if (current_row >= int(m_hboxes.size ()))
          {
            // add an hbox if we need one
            Gtk::Box *hbox = new Gtk::Box (Gtk::Orientation::HORIZONTAL);
            hbox->set_spacing (6);
            hbox->set_halign (Gtk::Align::CENTER);
            hbox->set_hexpand (true);
            m_hboxes.push_back (hbox);
            vbox->append (*hbox);
          }
        m_current_no++;
        m_hboxes[current_row]->append (*box.box);

        box.army->queue_draw ();
        box.water->queue_draw ();

        // finally add an entry for book-keeping as we go
        ArmyItem item;
        item.army = army;
        item.hp = initial_hp;
        item.water_image = box.water;
        item.image = box.army;
        item.exploding = false;
        m_army_items.push_back (item);
      }

    void heroes_level_up (std::list<Hero*> hero_list)
      {
        m_next_hero = [this](std::list<Hero*> heroes) mutable
          {
            if (heroes.empty ())
              {
                // after we level up heroes we give out three different kinds
                // of medals
                int medal_type = 0;
                army_gets_medal
                  (m_fight->get_fight_result ().get_medalists (medal_type),
                   medal_type);
                return;
              }
            auto d = LwDialog::build<HeroGainsLevelDialog> (this);
            d->setup (heroes.front (), GameScenario::s_hidden_map);
            d->signal_response ().connect
              ([this, heroes, d] (Gtk::ResponseType) mutable
               {
                 Hero *hero = heroes.front ();
                 Army::Stat stat = d->get_selected_stat ();
                 Player *p = hero->getOwner ();
                 p->heroGainsLevel (hero, stat);
                 heroes.pop_front ();
                 m_next_hero (heroes);
                 delete d;
               });
          };

        m_next_hero (hero_list);
      }

    void army_gets_medal (std::list<Army*> army_list, int medal_type)
      {
        m_next_army = [this, medal_type](std::list<Army*> armies) mutable
          {
            if (armies.empty ())
              {
                if (medal_type == 2)
                  m_battle_finished.emit ();
                else
                  {
                    medal_type++;
                    army_gets_medal
                      (m_fight->get_fight_result ().get_medalists (medal_type),
                       medal_type);
                  }
                return;
              }
            auto d = LwDialog::build<ArmyAwardedMedalDialog> (this);
            d->setup (armies.front (), medal_type);
            d->signal_response ().connect
              ([this, d, armies] (Gtk::ResponseType) mutable
               {
                 armies.pop_front ();
                 m_next_army (armies);
                 delete d;
               });
          };

        m_next_army (army_list);
      }

    Glib::ustring get_decision_text ()
      {
        Glib::ustring s = "";
        auto p = Playerlist::getActiveplayer ();
        if (p->isHuman ())
          {
            if (m_fight->get_outcome () == FightResult::ATTACKER_WON)
              {
                Glib::ustring hero =
                  m_fight->getStrongestLivingHeroName (m_attackers);
                if (m_on_city)
                  {
                    if (hero.empty () == true)
                      s = _("Your armies have won the city!");
                    else
                      s = String::ucompose (_("%1 has won the city!"), hero);
                  }
                else
                  {
                    if (hero.empty () == true)
                      s = _("Your armies have won the battle!");
                    else
                      s = String::ucompose (_("%1 has won the battle!"), hero);
                  }
              }
            else
              s = _("You have lost!");
          }
        else
          {
            if (m_fight->get_outcome () == FightResult::ATTACKER_WON)
              {
                Glib::ustring hero =
                  m_fight->getStrongestLivingHeroName (m_attackers);
                if (m_on_city)
                  {
                    if (hero.empty () == true)
                      s = String::ucompose (_("%1 has won the city!"),
                                            p->getName ());
                    else
                      s = String::ucompose (_("%1 has won the city!"), hero);
                  }
                else
                  {
                    if (hero.empty () == true)
                      s = String::ucompose (_("%1 has won the battle!"),
                                            p->getName ());
                    else
                      s = String::ucompose (_("%1 has won the battle!"), hero);
                  }
              }
            else
              s = String::ucompose (_("%1 has lost!"), p->getName ());
          }
        return s;
      }
};
#endif
