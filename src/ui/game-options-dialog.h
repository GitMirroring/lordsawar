//  Copyright (C) 2007, 2008, 2009, 2014, 2017, 2026 Ben Asselstine
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
#include "lw-combo.h"
#ifndef GAME_OPTIONS_DIALOG_H
#define GAME_OPTIONS_DIALOG_H
class GameOptionsDialog: public LwDialogBase
{
public:
    static std::string get_resource_name ()
      {
        return "game-options.ui";
      }

    sigc::signal<void()> difficulty_option_changed;

    GameOptionsDialog (BaseObjectType* o,
                      const Glib::RefPtr<Gtk::Builder>& xml)
      : LwDialogBase (o, xml)
      {
        m_close_button = xml->get_widget<Gtk::Button> ("close_button");
        m_view_enemies_switch =
          xml->get_widget<Gtk::Switch> ("view_enemies_switch");
        m_view_production_switch =
          xml->get_widget<Gtk::Switch> ("view_production_switch");
        auto c = xml->get_widget<Gtk::ComboBox> ("quests_combobox");
        m_quests_combobox = LwCombo::replace (c);
        m_hidden_map_switch =
          xml->get_widget<Gtk::Switch> ("hidden_map_switch");
        c = xml->get_widget<Gtk::ComboBox> ("neutral_combobox");
        m_neutral_cities_combobox = LwCombo::replace (c);
        c = xml->get_widget<Gtk::ComboBox> ("vectoring_combobox");
        m_vectoring_combobox = LwCombo::replace (c);
        c = xml->get_widget<Gtk::ComboBox> ("build_production_combobox");
        m_build_production_combobox = LwCombo::replace (c);
        c = xml->get_widget<Gtk::ComboBox> ("sack_combobox");
        m_sack_combobox = LwCombo::replace (c);
        c = xml->get_widget<Gtk::ComboBox> ("razing_combobox");
        m_razing_cities_combobox = LwCombo::replace (c);
        m_diplomacy_switch = xml->get_widget<Gtk::Switch> ("diplomacy_switch");
        m_military_advisor_switch =
          xml->get_widget<Gtk::Switch> ("military_advisor_switch");
        c = xml->get_widget<Gtk::ComboBox> ("quick_start_combobox");
        m_quick_start_combobox = LwCombo::replace (c);
        m_cusp_of_war_switch =
          xml->get_widget<Gtk::Switch> ("cusp_of_war_switch");
        m_intense_combat_switch =
          xml->get_widget<Gtk::Switch> ("intense_combat_switch");
        m_random_turns_switch =
          xml->get_widget<Gtk::Switch> ("random_turns_switch");
        m_cities_can_produce_allies_switch =
          xml->get_widget<Gtk::Switch> ("cities_can_produce_allies_switch");
      }

    void setup (bool readonly)
      {
        d_readonly = readonly;
        set_response (m_close_button, Gtk::ResponseType::CLOSE);

        m_view_enemies_switch->property_active ().signal_changed ().connect
          ([this] ()
           {
             GameScenarioOptions::s_see_opponents_stacks =
               m_view_enemies_switch->get_active ();
             difficulty_option_changed.emit ();
           });

        m_view_production_switch->property_active ().signal_changed ().connect
          ([this] ()
           {
             GameScenarioOptions::s_see_opponents_production =
               m_view_production_switch->get_active ();
             difficulty_option_changed.emit ();
           });

        m_quests_combobox->signal_changed ().connect
          ([this] ()
           {
             GameScenarioOptions::s_play_with_quests =
               GameParameters::QuestPolicy
               (m_quests_combobox->get_active_row_number ());
             difficulty_option_changed.emit ();
           });

        m_hidden_map_switch->property_active ().signal_changed ().connect
          ([this] ()
           {
             GameScenarioOptions::s_hidden_map =
               m_hidden_map_switch->get_active ();
             difficulty_option_changed.emit ();
           });

        m_neutral_cities_combobox->signal_changed ().connect
          ([this] ()
           {
             GameScenarioOptions::s_neutral_cities = GameParameters::NeutralCities 
               (m_neutral_cities_combobox->get_active_row_number ());
             difficulty_option_changed.emit ();
           });

        m_vectoring_combobox->signal_changed ().connect
          ([this] ()
           {
             GameScenarioOptions::s_vectoring_mode = GameParameters::VectoringMode
               (m_vectoring_combobox->get_active_row_number ());
           });

        m_build_production_combobox->signal_changed ().connect
          ([this] ()
           {
             GameScenarioOptions::s_build_production_mode =
               GameParameters::BuildProductionMode
               (m_build_production_combobox->get_active_row_number ());
           });

        m_sack_combobox->signal_changed ().connect
          ([this] ()
           {
             GameScenarioOptions::s_sacking_mode =
               GameParameters::SackingMode
               (m_sack_combobox->get_active_row_number ());
           });

        m_razing_cities_combobox->signal_changed ().connect
          ([this] ()
           {
             GameScenarioOptions::s_razing_cities =
               GameParameters::RazingCities 
               (m_razing_cities_combobox->get_active_row_number ());
             difficulty_option_changed.emit ();
           });

        m_diplomacy_switch->property_active ().signal_changed ().connect
          ([this] ()
           {
             if (m_diplomacy_switch->get_active () == true)
               m_cusp_of_war_switch->set_sensitive (true);
             else
               m_cusp_of_war_switch->set_sensitive (false);
             GameScenarioOptions::s_diplomacy =
               m_diplomacy_switch->get_active ();
             difficulty_option_changed.emit ();
           });

        m_cusp_of_war_switch->property_active ().signal_changed ().connect
          ([this] ()
           {
             GameScenarioOptions::s_cusp_of_war =
               m_cusp_of_war_switch->get_active ();
           });

        m_random_turns_switch->property_active ().signal_changed ().connect
          ([this] ()
           {
             GameScenarioOptions::s_random_turns =
               m_random_turns_switch->get_active ();
           });

        m_quick_start_combobox->signal_changed ().connect
          ([this] ()
           {
             Configuration::s_quick_start = GameParameters::QuickStartPolicy
               (m_quick_start_combobox->get_active_row_number ());
             difficulty_option_changed.emit ();
           });

        m_intense_combat_switch->property_active ().signal_changed ().connect
          ([this] ()
           {
             GameScenarioOptions::s_intense_combat =
               m_intense_combat_switch->get_active ();
           });

        m_military_advisor_switch->property_active ().signal_changed ().connect
          ([this] ()
           {
             GameScenarioOptions::s_military_advisor =
               m_military_advisor_switch->get_active ();
           });

        m_cities_can_produce_allies_switch->property_active ().signal_changed ()
          .connect
          ([this] ()
           {
             GameScenarioOptions::s_cities_can_produce_allies =
               m_cities_can_produce_allies_switch->get_active ();
           });

        signal_response ().connect
          ([this] (Gtk::ResponseType response)
           {
             switch (response)
               {
               case Gtk::ResponseType::CLOSE:
               case Gtk::ResponseType::CANCEL:
               case Gtk::ResponseType::DELETE_EVENT:
                 GameScenarioOptions::s_see_opponents_stacks =
                   m_view_enemies_switch->get_active ();
                 GameScenarioOptions::s_see_opponents_production =
                   m_view_production_switch->get_active ();
                 GameScenarioOptions::s_play_with_quests =
                   GameParameters::QuestPolicy
                   (m_quests_combobox->get_active_row_number ());
                 GameScenarioOptions::s_hidden_map =
                   m_hidden_map_switch->get_active ();
                 GameScenarioOptions::s_neutral_cities =
                   GameParameters::NeutralCities
                   (m_neutral_cities_combobox->get_active_row_number ());
                 GameScenarioOptions::s_vectoring_mode =
                   GameParameters::VectoringMode
                   (m_vectoring_combobox->get_active_row_number ());
                 GameScenarioOptions::s_build_production_mode =
                   GameParameters::BuildProductionMode
                   (m_build_production_combobox->get_active_row_number ());
                 GameScenarioOptions::s_sacking_mode =
                   GameParameters::SackingMode
                   (m_sack_combobox->get_active_row_number ());
                 GameScenarioOptions::s_razing_cities =
                   GameParameters::RazingCities
                   (m_razing_cities_combobox->get_active_row_number ());

                 GameScenarioOptions::s_diplomacy =
                   m_diplomacy_switch->get_active ();
                 GameScenarioOptions::s_random_turns =
                   m_random_turns_switch->get_active ();
                 Configuration::s_quick_start =
                   GameParameters::QuickStartPolicy
                   (m_quick_start_combobox->get_active_row_number ());
                 GameScenarioOptions::s_cusp_of_war =
                   m_cusp_of_war_switch->get_active ();
                 GameScenarioOptions::s_intense_combat =
                   m_intense_combat_switch->get_active ();
                 GameScenarioOptions::s_military_advisor =
                   m_military_advisor_switch->get_active ();
                 GameScenarioOptions::s_cities_can_produce_allies =
                   m_cities_can_produce_allies_switch->get_active ();
                 //save it all to Configuration too
                 Configuration::s_see_opponents_stacks = 
                   GameScenarioOptions::s_see_opponents_stacks;
                 Configuration::s_see_opponents_production = 
                   GameScenarioOptions::s_see_opponents_production;
                 Configuration::s_play_with_quests =
                   GameScenarioOptions::s_play_with_quests;
                 Configuration::s_hidden_map =
                   GameScenarioOptions::s_hidden_map;
                 Configuration::s_neutral_cities =
                   GameScenarioOptions::s_neutral_cities;
                 Configuration::s_vectoring_mode =
                   GameScenarioOptions::s_vectoring_mode;
                 Configuration::s_build_production_mode =
                   GameScenarioOptions::s_build_production_mode;
                 Configuration::s_sacking_mode =
                   GameScenarioOptions::s_sacking_mode;
                 Configuration::s_razing_cities =
                   GameScenarioOptions::s_razing_cities;
                 Configuration::s_diplomacy =
                   GameScenarioOptions::s_diplomacy;
                 Configuration::s_random_turns =
                   GameScenarioOptions::s_random_turns;
                 Configuration::s_cusp_of_war =
                   GameScenarioOptions::s_cusp_of_war;
                 Configuration::s_intense_combat =
                   GameScenarioOptions::s_intense_combat;
                 Configuration::s_military_advisor =
                   GameScenarioOptions::s_military_advisor;
                 Configuration::s_cities_can_produce_allies =
                   GameScenarioOptions::s_cities_can_produce_allies;
                 Configuration::saveConfigurationFile ();
                 break;

               default:
                 break;
               }
             hide ();
           });

        fill_in_options ();
        m_close_button->grab_focus ();
      }
private:
    bool d_readonly;
    Gtk::Button *m_close_button;
    LwCombo *m_quests_combobox;
    Gtk::Switch *m_view_enemies_switch;
    Gtk::Switch *m_view_production_switch;
    Gtk::Switch *m_hidden_map_switch;
    LwCombo *m_neutral_cities_combobox;
    LwCombo *m_razing_cities_combobox;
    Gtk::Switch *m_diplomacy_switch;
    Gtk::Switch *m_intense_combat_switch;
    Gtk::Switch *m_military_advisor_switch;
    Gtk::Switch *m_random_turns_switch;
    LwCombo *m_quick_start_combobox;
    Gtk::Switch *m_cusp_of_war_switch;
    LwCombo *m_vectoring_combobox;
    LwCombo *m_build_production_combobox;
    LwCombo *m_sack_combobox;
    Gtk::Switch *m_cities_can_produce_allies_switch;

    void fill_in_options ()
      {
        m_neutral_cities_combobox->set_active
          (GameScenarioOptions::s_neutral_cities);
        m_vectoring_combobox->set_active
          (GameScenarioOptions::s_vectoring_mode);
        m_build_production_combobox->set_active
          (GameScenarioOptions::s_build_production_mode);
        m_sack_combobox->set_active
          (GameScenarioOptions::s_sacking_mode);
        m_razing_cities_combobox->set_active
          (GameScenarioOptions::s_razing_cities);
        m_view_enemies_switch->set_active
          (GameScenarioOptions::s_see_opponents_stacks);
        m_view_production_switch->set_active
          (GameScenarioOptions::s_see_opponents_production);
        m_quests_combobox->set_active
          (int (GameScenarioOptions::s_play_with_quests));
        m_hidden_map_switch->set_active
          (GameScenarioOptions::s_hidden_map);
        m_razing_cities_combobox->set_active
          (int (GameScenarioOptions::s_razing_cities));
        m_diplomacy_switch->set_active
          (GameScenarioOptions::s_diplomacy);
        m_military_advisor_switch->set_active
          (GameScenarioOptions::s_military_advisor);
        m_quick_start_combobox->set_active (Configuration::s_quick_start);
        m_cusp_of_war_switch->set_active (GameScenarioOptions::s_cusp_of_war);
        m_cusp_of_war_switch->set_sensitive (m_diplomacy_switch->get_active ());
        m_intense_combat_switch->set_active
          (GameScenarioOptions::s_intense_combat);
        m_random_turns_switch->set_active (GameScenarioOptions::s_random_turns);
        m_cities_can_produce_allies_switch->set_active
          (GameScenarioOptions::s_cities_can_produce_allies);

        if (d_readonly)
          {
            m_quests_combobox->set_sensitive (false);
            m_view_enemies_switch->set_sensitive (false);
            m_view_production_switch->set_sensitive (false);
            m_hidden_map_switch->set_sensitive (false);
            m_neutral_cities_combobox->set_sensitive (false);
            m_razing_cities_combobox->set_sensitive (false);
            m_diplomacy_switch->set_sensitive (false);
            m_intense_combat_switch->set_sensitive (false);
            m_military_advisor_switch->set_sensitive (false);
            m_random_turns_switch->set_sensitive (false);
            m_quick_start_combobox->set_sensitive (false);
            m_cusp_of_war_switch->set_sensitive (false);
            m_vectoring_combobox->set_sensitive (false);
            m_build_production_combobox->set_sensitive (false);
            m_sack_combobox->set_sensitive (false);
            m_cities_can_produce_allies_switch->set_sensitive (false);
          }
      }
};
#endif
