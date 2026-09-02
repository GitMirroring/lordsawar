//  Copyright (C) 2007 Ole Laursen
//  Copyright (C) 2007, 2008, 2009, 2014, 2020, 2026 Ben Asselstine
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
#ifndef SELECT_ARMY_DIALOG_H
#define SELECT_ARMY_DIALOG_H
#include "input-events.h"
#include "ucompose.hpp"
#include "defs.h"
#include "army.h"
#include "city.h"
#include "image-cache.h"
#include "army-set-list.h"
#include "player-list.h"
#include "shield.h"
#include "army-info-tip.h"
#include "lw-combo.h"
class SelectArmyDialog: public LwDialogBase
{
public:
    static std::string get_resource_name ()
      {
        return "select-army.ui";
      }

    enum Mode
      {
        SELECT_NORMAL_WITH_HERO,
        SELECT_NORMAL,
        SELECT_RUIN_DEFENDER,
        SELECT_REWARDABLE_ARMY
      };

    enum
      {
        NO_ARMY_SELECTED = -1
      };

    SelectArmyDialog (BaseObjectType* o,
                      const Glib::RefPtr<Gtk::Builder>& xml)
      : LwDialogBase (o, xml)
      {
        m_allow_choose_player = false;
        m_select_button = load <Gtk::Button> ("select_button");
        m_clear_button = load <Gtk::Button> ("clear_button");
        m_cancel_button = load <Gtk::Button> ("cancel_button");
        m_shield_combobox = load <Gtk::Box> ("shield_combobox");
        m_name_label = load <Gtk::Label> ("name_label");
        m_strength_label = load <Gtk::Label> ("strength_label");
        m_turns_label = load <Gtk::Label> ("turns_label");
        m_moves_label = load <Gtk::Label> ("moves_label");
        m_sight_label = load <Gtk::Label> ("sight_label");
        m_upkeep_label = load <Gtk::Label> ("upkeep_label");
        m_bonus_label = load <Gtk::Label> ("bonus_label");
        m_move_bonus_label = load <Gtk::Label> ("move_bonus_label");

        m_toggles_table = load <Gtk::FlowBox> ("army_toggles_table");
      }

    ~SelectArmyDialog ()
      {
        if (m_army_info_tip)
          delete m_army_info_tip;
      }

    int get_selected_army ()
      {
        return m_selected_army == NO_ARMY_SELECTED ?
          int (NO_ARMY_SELECTED) : m_armies[m_selected_army]->getId ();
      }

    Shield::Color get_selected_shield ()
      {
        return m_shield;
      }

    void set_allow_choose_player ()
      {
        m_allow_choose_player = true;
      }

    void setup (Shield::Color shield, Mode mode, int pre_selected_type)
      {
        m_shield = shield;
        m_mode = mode;
        m_pre_selected_type = pre_selected_type;
        auto p = Playerlist::instance ()->get (shield);
        m_armyset = p->getArmyset ();
        set_response (m_select_button, Gtk::ResponseType::ACCEPT);
        set_response (m_clear_button, Gtk::ResponseType::REJECT);
        set_response (m_cancel_button, Gtk::ResponseType::CANCEL);

        fill_armies ();

        m_clear_button->set_visible (pre_selected_type > -1);
        set_default_widget (*m_select_button);
        update_buttons ();

        fill_in_info ();

        m_army_info_tip = new ArmyInfoTip ();

        fill_shield_combo ();

        if (m_selected_army == NO_ARMY_SELECTED)
          m_cancel_button->grab_focus ();
        else
          m_select_button->grab_focus ();
      }

private:
    Gtk::Button *m_select_button;
    Gtk::Button *m_clear_button;
    Gtk::Button *m_cancel_button;
    Gtk::Label *m_name_label;
    Gtk::Label *m_strength_label;
    Gtk::Label *m_turns_label;
    Gtk::Label *m_moves_label;
    Gtk::Label *m_sight_label;
    Gtk::Label *m_upkeep_label;
    Gtk::Label *m_bonus_label;
    Gtk::Label *m_move_bonus_label;
    Gtk::FlowBox *m_toggles_table;
    Gtk::Box *m_shield_combobox;
    LwCombo *m_shield_combo;
    Mode m_mode;
    int m_pre_selected_type;

    ArmyInfoTip* m_army_info_tip;
    Shield::Color m_shield;
    guint32 m_armyset;
    int m_selected_army;
    bool m_allow_choose_player;

    std::vector<Gtk::ToggleButton *> m_army_toggles;
    bool m_ignore_toggles;
    std::vector<const ArmyProto*> m_armies;

    void fill_pixbuf (int i)
      {
        auto pix =
          ImageCache::instance ()->getArmyPic
          (m_armyset, m_armies[i]->getId (), m_shield, NULL, false);

        Gtk::Image *image =
          dynamic_cast<Gtk::Image*>(m_army_toggles[i]->get_child ());
        image->set_pixel_size (LW_BUTTON_SIZE);
        image->set (pix->to_pixbuf ());
      }

    void fill_in_info ()
      {
        Glib::ustring s1, s2;
        const ArmyProto *a = army_id_to_army ();
        if (a)
          {
            m_name_label->set_text (a->getName ());
            m_strength_label->set_text
              (String::ucompose ("%1", a->getStrength ()));
            m_turns_label->set_text
              (String::ucompose ("%1", a->getProduction ()));
            m_moves_label->set_text
              (String::ucompose ("%1", a->getMaxMoves ()));
            m_sight_label->set_text (String::ucompose ("%1", a->getSight ()));
            m_upkeep_label->set_text
              (String::ucompose ("%1", a->getUpkeep ()));
            if (a->getArmyBonusDescription () == "")
              m_bonus_label->set_text (_("No Army Bonus"));
            else
              m_bonus_label->set_text (a->getArmyBonusDescription ());
            if (a->getMoveBonusDescription () == "")
              m_move_bonus_label->set_text (_("No Move Bonus"));
            else
              m_move_bonus_label->set_text (a->getMoveBonusDescription ());
          }
        else
          {
            m_name_label->set_text ("");
            m_strength_label->set_text ("");
            m_turns_label->set_text ("");
            m_moves_label->set_text ("");
            m_sight_label->set_text ("");
            m_upkeep_label->set_text ("");
            m_bonus_label->set_text ("");
            m_move_bonus_label->set_text ("");
          }

      }

    void update_buttons ()
      {
        m_select_button->set_sensitive (m_selected_army != NO_ARMY_SELECTED);
      }

    const ArmyProto *army_id_to_army ()
      {
        if (m_selected_army == NO_ARMY_SELECTED)
          return NULL;
        return m_armies[m_selected_army];
      }

    int lookup_slot (Gtk::ToggleButton *toggle)
      {
        int slot = -1;
        for (unsigned int i = 0; i < m_army_toggles.size (); ++i)
          {
            if (toggle == m_army_toggles[i])
              slot = i;
          }
        return slot;
      }

    void fill_shield_combo ()
      {
        m_shield_combo = Gtk::make_managed<LwCombo> ();
        int i = 0;
        int found = -1;
        for (auto p : *Playerlist::instance ())
          {
            if (m_shield == p->get_shield ())
              found = i;
            m_shield_combo->append (p->getName ());
          }
        m_shield_combo->set_visible (m_allow_choose_player);
        m_shield_combobox->append (*m_shield_combo);
        if (found >= 0)
          m_shield_combo->set_active (found);

        m_shield_combo->signal_changed ().connect
          ([this] ()
           {
             auto p = Playerlist::instance ()->get
               (m_shield_combo->get_active_text ());
             m_shield = p->get_shield ();
             m_armyset = p->getArmyset ();

             fill_armies ();
           });
      }

    void fill_armies ()
      {
        m_armies.clear ();
        m_toggles_table->remove_all ();
        m_army_toggles.clear ();

        const Armysetlist* al = Armysetlist::instance ();

        Armyset *as = al->get (m_armyset);
        for (auto j = as->begin (); j != as->end (); ++j)
          {
            const ArmyProto *a = al->getArmy (m_armyset, (*j)->getId ());
            if (m_mode == SELECT_NORMAL_WITH_HERO)
              m_armies.push_back (a);
            else if (m_mode == SELECT_NORMAL && a->isHero () == false)
              m_armies.push_back (a);
            else if (m_mode == SELECT_RUIN_DEFENDER && a->getDefendsRuins ())
              m_armies.push_back (a);
            else if (m_mode == SELECT_REWARDABLE_ARMY && a->getAwardable ())
              m_armies.push_back (a);
          }

        for (unsigned int i = 0; i < m_armies.size (); ++i)
          {
            Gtk::ToggleButton *toggle = Gtk::make_managed<Gtk::ToggleButton>();
            Gtk::Image *image = Gtk::make_managed<Gtk::Image> ();
            toggle->set_child (*image);
            m_army_toggles.push_back (toggle);
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
                     const ArmyProto *army = m_armies[slot];
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

                 if (m_ignore_toggles)
                   return;

                 m_ignore_toggles = true;
                 if (slot == -1)
                   m_selected_army = NO_ARMY_SELECTED;
                 else
                   m_selected_army = slot;
                 for (unsigned int j = 0; j < m_army_toggles.size (); ++j)
                   m_army_toggles[j]->set_active
                     (toggle == m_army_toggles[j]);
                 m_ignore_toggles = false;
                 fill_in_info ();
                 update_buttons ();
               });
          }

        for (unsigned int i = 1; i < m_army_toggles.size (); ++i)
          m_army_toggles[i]->set_group (*m_army_toggles[0]);

        m_ignore_toggles = false;

        if (m_pre_selected_type > -1)
          {
            int i = 0;
            for (auto a : m_armies)
              {
                if (a->getId () == (guint32) m_pre_selected_type)
                  {
                    m_army_toggles[i]->toggled ();
                    m_selected_army = i;
                    break;
                  }
                i++;
              }
          }
        else
          m_selected_army = NO_ARMY_SELECTED;
      }
};
#endif
