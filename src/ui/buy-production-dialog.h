//  Copyright (C) 2007 Ole Laursen
//  Copyright (C) 2007, 2008, 2009, 2014, 2017, 2020, 2026 Ben Asselstine
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
#ifndef BUY_PRODUCTION_DIALOG_H
#define BUY_PRODUCTION_DIALOG_H
#include "input-events.h"
#include "ucompose.hpp"
#include "defs.h"
#include "army.h"
#include "city.h"
#include "image-cache.h"
#include "army-set-list.h"
#include "player-list.h"
#include "shield.h"
class BuyProductionDialog: public LwDialogBase
{
public:
    static std::string get_resource_name ()
      {
        return "buy-production.ui";
      }
    
    enum
      {
        NO_ARMY_SELECTED = -1
      };

    BuyProductionDialog (BaseObjectType* o,
                         const Glib::RefPtr<Gtk::Builder>& xml)
      : LwDialogBase (o, xml)
      {
        m_close_button = load <Gtk::Button> ("close_button");
        m_accept_button = load <Gtk::Button> ("accept_button");
        m_unit_label = load <Gtk::Label> ("unit_label");
        m_cost_label = load <Gtk::Label> ("cost_label");
        m_time_label = load <Gtk::Label> ("time_label");
        m_moves_label = load <Gtk::Label> ("moves_label");
        m_strength_label = load <Gtk::Label> ("strength_label");
        m_upkeep_label = load <Gtk::Label> ("upkeep_label");
        m_combat_bonus_label = load <Gtk::Label> ("combat_bonus_label");
        m_move_bonus_label = load <Gtk::Label> ("move_bonus_label");
        m_toggles_table = load <Gtk::FlowBox> ("production_toggles_table");
      }

    ~BuyProductionDialog ()
      {
        if (m_army_info_tip)
          delete m_army_info_tip;
      }

    int get_selected_army ()
      {
        return m_selected_army == NO_ARMY_SELECTED ?
          int (NO_ARMY_SELECTED) : m_purchasables[m_selected_army]->getId ();
      }

    void setup (City *c)
      {
        m_city = c;
        set_response (m_close_button, Gtk::ResponseType::CLOSE);
        set_response (m_accept_button, Gtk::ResponseType::ACCEPT);

        const Armysetlist* al = Armysetlist::instance ();

        Player *p = Playerlist::instance ()->getActiveplayer ();
        // fill in purchasable armies
        Armyset *as = al->get (p->getArmyset ());
        for (Armyset::iterator j = as->begin (); j != as->end (); ++j)
          {
            const ArmyProto *a = al->getArmy (p->getArmyset (), (*j)->getId ());
            if (a->getNewProductionCost () > 0)
              m_purchasables.push_back (a);
          }

        // fill in production options
        for (unsigned int i = 0; i < m_purchasables.size (); ++i)
          {
            Gtk::ToggleButton *toggle = Gtk::make_managed<Gtk::ToggleButton>();
            Gtk::Image *image = Gtk::make_managed<Gtk::Image> ();
            toggle->set_child (*image);
            m_production_toggles.push_back (toggle);
            fill_pixbuf (i);

            m_toggles_table->append (*toggle);
            toggle->show ();

            auto click = Gtk::GestureClick::create ();
            click->set_button (3);
            click->signal_pressed ().connect
              ([this, i, toggle] (int, double x, double y)
               {
                 int slot = lookup_slot (toggle);
                 if (slot >= 0)
                   {
                     const ArmyProto *army = m_purchasables[slot];
                     if (army)
                       m_army_info_tip->show
                         (toggle, x, y,
                          [this, army] ()
                          {
                            m_army_info_tip->set (army);
                          });
                   }
               });

            click->signal_released ().connect
              ([this, i] (int, double, double)
               {
                 m_army_info_tip->popdown ();
               });
            toggle->add_controller (click);

            toggle->signal_toggled ().connect
              ([this, toggle] ()
               {
                 int slot = lookup_slot (toggle);
                 if (toggle->get_active () == false)
                   {
                     if (slot != -1)
                       fill_pixbuf (slot);
                   }

                 if (m_ignore_toggles)
                   return;

                 m_ignore_toggles = true;
                 if (slot == -1)
                   m_selected_army = NO_ARMY_SELECTED;
                 else
                   m_selected_army = slot;
                 for (unsigned int j = 0; j < m_production_toggles.size (); ++j)
                   m_production_toggles[j]->set_active
                     (toggle == m_production_toggles[j]);
                 m_ignore_toggles = false;
                 fill_pixbuf (m_selected_army);
                 fill_in_production_info ();
                 set_accept_button_state ();
               });
          }

        for (unsigned int i = 1; i < m_production_toggles.size (); ++i)
          m_production_toggles[i]->set_group (*m_production_toggles[0]);

        m_ignore_toggles = false;
        //m_production_toggles[0]->set_active (true);
        m_production_toggles[0]->toggled ();

        m_army_info_tip = new ArmyInfoTip ();

        signal_response ().connect
          ([this](Gtk::ResponseType resp)
           {
             switch (resp)
               {
               case Gtk::ResponseType::ACCEPT:
                   {
                     int army = get_selected_army ();
                     int slot = m_city->getFreeSlot ();

                     bool warn = false;
                     if  (slot == -1)
                       {
                         //no free slots available.  change the one we're on.
                         slot = m_city->getActiveProductionSlot ();
                         if (slot == -1) 
                           slot = 0;
                         warn = true;
                       }
                     if (warn)
                       {
                         Glib::ustring s = 
                           String::ucompose
                           (_("Are you sure you want to replace %1 with %2?"),
                            m_city->getActiveProductionBase ()->getName (),
                            m_purchasables[army]->getName ());

                         auto d =
                           Gtk::make_managed<Gtk::MessageDialog>
                           (*this, _("Replace?"), false,
                            Gtk::MessageType::QUESTION,
                            Gtk::ButtonsType::NONE);

                         d->add_button (_("No"),  Gtk::ResponseType::NO);

                         auto yes_button =
                           d->add_button (_("Yes"),Gtk::ResponseType::YES);
                         yes_button->add_css_class("destructive-action");

                         d->set_secondary_text (s);
                         d->signal_response ().connect
                           ([this, slot, army, d] (int response)
                            {
                              switch (response)
                                {
                                case Gtk::ResponseType::YES:
                                  update_city_production (slot, army);
                                  d->hide ();
                                  hide ();
                                  break;

                                default:
                                  break;
                                }
                              d->hide ();
                            });
                         d->show ();
                         return;
                       }
                     else
                       update_city_production (slot, army);
                   }
                 break;

               default:
                 break;
               }
             hide ();
           });
      }

    void update_city_production (int slot, int army)
      {
        m_city->getOwner ()->cityBuyProduction (m_city, slot, army);
        m_city->getOwner ()->cityChangeProduction (m_city, slot);
        m_army_purchased.emit ();
      }

    sigc::signal<void()> signal_army_purchased ()
      {
        return m_army_purchased;
      }
private:
    Gtk::Button *m_close_button;
    Gtk::Button *m_accept_button;
    Gtk::Label *m_unit_label;
    Gtk::Label *m_cost_label;
    Gtk::Label *m_time_label;
    Gtk::Label *m_moves_label;
    Gtk::Label *m_strength_label;
    Gtk::Label *m_upkeep_label;
    Gtk::Label *m_combat_bonus_label;
    Gtk::Label *m_move_bonus_label;
    Gtk::FlowBox *m_toggles_table;

    ArmyInfoTip* m_army_info_tip;
    City *m_city;
    int m_selected_army;

    std::vector<Gtk::ToggleButton *> m_production_toggles;
    bool m_ignore_toggles;
    std::vector<const ArmyProto*> m_purchasables;

    sigc::signal<void()> m_army_purchased;

    void fill_pixbuf (int i)
      {
        Player *p = Playerlist::instance ()->getActiveplayer ();
        ImageCache *gc = ImageCache::instance ();
        bool greyed_out = false;
        guint32 selected = Shield::NEUTRAL;
        if (m_city->hasProductionBase (m_purchasables[i]) == true)
          greyed_out = true;
        if ((int)m_purchasables[i]->getNewProductionCost () >
            m_city->getOwner ()->getGold ())
          greyed_out = true;
        if (m_production_toggles[i]->get_active ())
          selected = p->getId ();
        Glib::RefPtr<Gdk::Pixbuf> pix =
          gc->getCircledArmyPic (p->getArmyset (), m_purchasables[i]->getId (),
                                 p->get_shield (), NULL, greyed_out, selected,
                                 true, Lw::get_dark ())->to_pixbuf ();
        Gtk::Image *image =
          dynamic_cast<Gtk::Image*>(m_production_toggles[i]->get_child ());
        image->set_pixel_size (LW_BUTTON_SIZE);
        image->set (pix);
      }

    void fill_in_production_info ()
      {
        const ArmyProto *a = army_id_to_army ();
        if (a)
          {
            m_unit_label->set_text (a->getName ());
            m_cost_label->set_text
              (String::ucompose ("%1", a->getNewProductionCost ()));
            m_time_label->set_text
              (String::ucompose ("%1", a->getProduction ()));
            m_moves_label->set_text
              (String::ucompose ("%1", a->getMaxMoves ()));
            m_strength_label->set_text
              (String::ucompose ("%1", a->getStrength ()));
            m_upkeep_label->set_text
              (String::ucompose ("%1", a->getUpkeep ()));
        
            auto move_bonus = a->getMoveBonusDescription (true);
            if (move_bonus == "")
              move_bonus = "-";
            m_move_bonus_label->set_text (move_bonus);

            auto army_bonus = a->getArmyBonusDescription (true);
            if (army_bonus == "")
              army_bonus = "-";
            m_combat_bonus_label->set_text (army_bonus);
          }
      }

    void set_accept_button_state ()
      {
        bool can_buy = true;

        if (m_selected_army == NO_ARMY_SELECTED)
          can_buy = false;
        else
          {
            int gold = m_city->getOwner ()->getGold ();
            const ArmyProto *a = army_id_to_army ();

            if (int (a->getNewProductionCost ()) > gold ||
                m_city->hasProductionBase (m_selected_army))
              can_buy = false;
          }

        m_accept_button->set_sensitive (can_buy);
      }

    const ArmyProto *army_id_to_army ()
      {
        return m_purchasables[m_selected_army];
      }

    int lookup_slot (Gtk::ToggleButton *toggle)
      {
        int slot = -1;
        for (unsigned int i = 0; i < m_production_toggles.size (); ++i)
          {
            if (toggle == m_production_toggles[i])
              slot = i;
          }
        return slot;
      }
};
#endif
