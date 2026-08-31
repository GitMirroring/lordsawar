//  Copyright (C) 2007 Ole Laursen
//  Copyright (C) 2007, 2008, 2009, 2011, 2012, 2014, 2015, 2017, 2020,
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
#ifndef CITY_DIALOG_H
#define CITY_DIALOG_H
#include "vector-map.h"
#include "image-cache.h"
#include "city.h"
#include "army-prod-base.h"
#include "player-list.h"
#include "city-list.h"
#include "player.h"
#include "image-helpers.h"
#include "input-events.h"
#include "city-rename-dialog.h"
#include "city-raze-dialog.h"
#include "buy-production-dialog.h"
#include "destination-dialog.h"
#include "city-razed-dialog.h"
#include "lw-dialog.h"

class CityDialog: public LwDialogBase
{
public:
    static std::string get_resource_name ()
      {
        return "city.ui";
      }

    CityDialog (BaseObjectType* o,
                const Glib::RefPtr<Gtk::Builder>& xml)
      : LwDialogBase (o, xml)
      {
        m_close_button = load <Gtk::Button> ("close_button");
        m_map_drawing_area =
          load <Gtk::DrawingArea> ("map_drawing_area");
        m_buy_button = load <Gtk::Button> ("buy_button");
        m_stop_button = load <Gtk::Button> ("stop_button");
        m_rename_button = load <Gtk::Button> ("rename_button");
        m_destination_button =
          load <Gtk::Button> ("destination_button");
        m_raze_button = load <Gtk::Button> ("raze_button");
        m_turns_left_label = load <Gtk::Label> ("turns_left_label");
        m_current_image = load <Gtk::Image> ("current_image");
        m_current_label = load <Gtk::Label> ("current_label");
        m_capital_city_label = load <Gtk::Label> ("capital_city_label");
        m_defense_label = load <Gtk::Label> ("defense_label");
        m_income_label = load <Gtk::Label> ("income_label");
        m_description_label = load <Gtk::Label> ("description_label");
        m_unit_label = load <Gtk::Label> ("unit_label");
        m_time_label = load <Gtk::Label> ("time_label");
        m_moves_label = load <Gtk::Label> ("moves_label");
        m_strength_label = load <Gtk::Label> ("strength_label");
        m_cost_label = load <Gtk::Label> ("cost_label");
        m_bonus_label = load <Gtk::Label> ("bonus_label");
        m_rebellious_label = load <Gtk::Label> ("rebellious_label");
        m_time_title_label = load <Gtk::Label> ("time_title_label");
        m_moves_title_label = load <Gtk::Label> ("moves_title_label");
        m_strength_title_label =
          load <Gtk::Label> ("strength_title_label");
        m_cost_title_label = load <Gtk::Label> ("cost_title_label");
        m_production_toggles_hbox = load <Gtk::Box> ("production_toggles_hbox");
      }

    ~CityDialog ()
      {
        delete m_prodmap;
        if (m_army_info_tip != NULL)
          delete m_army_info_tip;
        m_dark_style_handler.disconnect ();
      }

    void setup_dark_mode_change ()
      {
        m_dark = Lw::get_dark ();
        auto iface = Gio::Settings::create ("org.gnome.desktop.interface",
                                            "/org/gnome/desktop/interface/");
        if (iface)
          {
            m_dark_style_handler =
              iface->signal_changed ().connect
              ([this, iface] (const Glib::ustring &key)
               {
                 (void)key;
                 auto scheme = iface->get_string ("color-scheme");
                 if (scheme == "prefer-dark")
                   m_dark = true;
                 else
                   m_dark = false;
                 fill_in_production_toggles ();
               });
          }
      }

    void setup (City *c, bool razing_possible, bool see_opponents_production)
      {
        setup_dark_mode_change ();
        m_city = c;
        m_razing_possible = razing_possible;
        m_see_opponents_production = see_opponents_production;

        set_response (m_close_button, Gtk::ResponseType::CLOSE);
        set_default_widget (*m_close_button);

        m_prodmap = new VectorMap (m_city,
                                   VectorMap::SHOW_ORIGIN_CITY_VECTORING,
                                   m_see_opponents_production);
        m_prodmap->map_changed.connect
          ([this] (Cairo::RefPtr<Cairo::Surface> map)
           {
             cairo_surface_to_drawing_area (map, m_map_drawing_area);
           });
        m_prodmap->resize ();

        auto click = Gtk::GestureClick::create ();
        click->set_button (1);
        click->signal_pressed ().connect
          ([this] (int n, double x, double y)
           {
             struct MouseButtonEvent ev = to_input_event (n, x, y, true);
             m_prodmap->mouse_button_event (ev);
             m_city = m_prodmap->getCity ();
             fill_in_city_info ();
             fill_in_production_toggles ();
             fill_in_production_info ();
           });

        click->signal_released ().connect
          ([this] (int n, double x, double y)
           {
             struct MouseButtonEvent ev = to_input_event (n, x, y, false);
             m_prodmap->mouse_button_event (ev);
             m_city = m_prodmap->getCity ();
             fill_in_city_info ();
             fill_in_production_toggles ();
             fill_in_production_info ();
           });
        m_map_drawing_area->add_controller (click);

        auto motion = Gtk::EventControllerMotion::create ();
        motion->signal_motion ().connect
          ([this](double x, double y)
           {
             auto cr = m_prodmap->get_cursor (x, y);
             static ImageCache::CursorType prev_cursor = ImageCache::SHIP;
             if (cr != prev_cursor)
               {
                 auto hotspot = ImageCache::get_hotspot (cr);
                 auto im = ImageCache::instance ()->getCursorPic (cr);
                 auto cursor = Gdk::Cursor::create (im->to_texture (),
                                                    hotspot.x, hotspot.y);
                 m_map_drawing_area->set_cursor (cursor);
               }
             prev_cursor = cr;
           });
        m_map_drawing_area->add_controller (motion);

        m_stop_button->signal_clicked ().connect //stop button
          ([this] ()
           {
             m_city->setVectoring (Vector<int> (-1,-1));
             m_city->getOwner ()->cityChangeProduction (m_city, -1);
             m_stop_button->set_sensitive (false);
             m_ignore_toggles = true;
             for (unsigned int i = 0; i < m_production_toggles.size (); ++i)
               m_production_toggles[i]->set_active (false);
             m_ignore_toggles = false;
             fill_in_production_info ();
             m_prodmap->draw ();
           });

        m_rename_button->signal_clicked ().connect
          ([this] ()
           {
             auto d = LwDialog::build <CityRenameDialog> (this);
             d->setup (m_city);
             d->signal_response ().connect
               ([this, d] (Gtk::ResponseType resp)
                {
                  switch (resp)
                    {
                    case Gtk::ResponseType::ACCEPT:
                      fill_in_city_info ();
                      break;

                    default:
                      break;
                    }
                  delete d;
                });
           });

        m_buy_button->signal_clicked ().connect
          ([this] ()
           {
             auto d = LwDialog::build <BuyProductionDialog> (this);
             d->setup (m_city);
             d->signal_army_purchased ().connect
               ([this] ()
                {
                  fill_in_production_toggles ();
                  fill_in_production_info ();
                });
             d->signal_response ().connect
               ([d] (Gtk::ResponseType)
                {
                  delete d;
                });
           });

        m_raze_button->signal_clicked ().connect
          ([this] ()
           {
             auto d = LwDialog::build <CityRazeDialog> (this);
             d->setup
               (m_city, false,
                [this, d] (bool raze)
                {
                  if (raze)
                    {
                      Playerlist::getActiveplayer ()->cityRaze (m_city);
                      auto dd = LwDialog::build <CityRazedDialog> (this);
                      dd->setup (m_city);
                      dd->signal_response ().connect
                        ([d, dd, raze] (Gtk::ResponseType)
                         {
                           d->signal_razed ().emit (raze);
                           delete dd;
                           delete d;
                         });
                    }
                  else
                    d->signal_razed ().emit (raze);
                });

             d->signal_razed ().connect
               ([this] (bool raze)
                {
                  if (raze)
                    signal_response ().emit (Gtk::ResponseType::ACCEPT);
                });
           });

        m_destination_button->signal_clicked ().connect
          ([this] ()
           {
             auto d = LwDialog::build <DestinationDialog> (this);
             d->setup (m_city, &m_see_all);
             d->signal_response ().connect
               ([d] (Gtk::ResponseType)
                {
                  delete d;
                });
           });

        m_current_image->set_pixel_size (LW_BUTTON_SIZE);

        for (unsigned int i = 1; i <= m_city->getMaxNoOfProductionBases (); ++i)
          {
            Gtk::ToggleButton *toggle = Gtk::make_managed<Gtk::ToggleButton> ();
            m_production_toggles_hbox->append (*toggle);
            m_production_toggles.push_back (toggle);
            toggle->signal_toggled ().connect
              ([this, toggle] ()
               {
                 //we click on an enemy's production, nothing happens
                 if (m_city->getOwner () != Playerlist::getActiveplayer ())
                   {
                     toggle->set_active (false);
                     return;
                   }

                 //we click on an empty proudction, nothing happens
                 int slot = -1;
                 for (unsigned int j = 0; j < m_production_toggles.size (); ++j)
                   {
                     if (toggle == m_production_toggles[j])
                       slot = j;
                   }
                 if (m_city->getProductionBase (slot) == NULL)
                   return;

                 if (m_ignore_toggles)
                   return;

                 slot = -1;
                 m_ignore_toggles = true;
                 for (unsigned int j = 0; j < m_production_toggles.size (); ++j)
                   {
                     if (toggle == m_production_toggles[j])
                       slot = j;

                     m_production_toggles[j]->set_active
                       (toggle == m_production_toggles[j]);
                   }
                 m_ignore_toggles = false;

                 bool is_empty = m_city->getArmytype (slot) == -1;

                 if (is_empty)
                   m_city->getOwner ()->cityChangeProduction (m_city, -1);
                 else
                   m_city->getOwner ()->cityChangeProduction (m_city, slot);

                 m_stop_button->set_sensitive (!is_empty);

                 for (unsigned int j = 0; j < m_production_toggles.size (); ++j)
                   update_toggle_picture (j);
                 fill_in_production_info ();
               });

            click = Gtk::GestureClick::create ();
            click->set_button (3);
            click->signal_pressed ().connect
              ([this, i, toggle] (int, double x, double y)
               {
                 auto army = m_city->getProductionBase (i - 1);
                 if (army)
                   m_army_info_tip->show
                     (toggle, x, y,
                      [this, army] ()
                      {
                        m_army_info_tip->set (army, m_city);
                      });
               });

            click->signal_released ().connect
              ([this, i] (int, double, double)
               {
                 auto army = m_city->getProductionBase (i - 1);
                 if (army)
                   m_army_info_tip->popdown ();
               });
            toggle->add_controller (click);

          }

        m_army_info_tip = new ArmyInfoTip ();

        fill_in_city_info ();
        fill_in_production_toggles ();
        m_prodmap->draw ();

        m_close_button->grab_focus ();

        signal_response ().connect
          ([this](Gtk::ResponseType)
           {
             hide ();
           });
      }
private:
    Gtk::Button *m_close_button = NULL;
    Gtk::DrawingArea *m_map_drawing_area = NULL;
    Gtk::Button *m_buy_button = NULL;
    Gtk::Button *m_stop_button = NULL;
    Gtk::Button *m_rename_button = NULL;
    Gtk::Button *m_destination_button = NULL;
    Gtk::Button *m_raze_button = NULL;
    Gtk::Label *m_turns_left_label = NULL;
    Gtk::Image *m_current_image = NULL;
    Gtk::Label *m_current_label = NULL;
    Gtk::Label *m_capital_city_label = NULL;
    Gtk::Label *m_defense_label = NULL;
    Gtk::Label *m_income_label = NULL;
    Gtk::Label *m_description_label = NULL;
    Gtk::Label *m_unit_label = NULL;
    Gtk::Label *m_time_label = NULL;
    Gtk::Label *m_moves_label = NULL;
    Gtk::Label *m_strength_label = NULL;
    Gtk::Label *m_cost_label = NULL;
    Gtk::Label *m_bonus_label = NULL;
    Gtk::Label *m_rebellious_label = NULL;
    Gtk::Label *m_time_title_label = NULL;
    Gtk::Label *m_moves_title_label = NULL;
    Gtk::Label *m_strength_title_label = NULL;
    Gtk::Label *m_cost_title_label = NULL;
    Gtk::Box *m_production_toggles_hbox = NULL;
    std::vector<Gtk::ToggleButton *> m_production_toggles = {};
    VectorMap* m_prodmap = NULL;
    ArmyInfoTip* m_army_info_tip = NULL;
    bool m_ignore_toggles = NULL;

    City *m_city = NULL;
    bool m_see_opponents_production = false;
    bool m_razing_possible = false;
    bool m_see_all = false;
    bool m_dark = false;
    sigc::connection m_dark_style_handler;

    void fill_in_city_info ()
      {
        set_title (m_city->getName ());

        // fill in status label
        if (m_city->isCapital ())
          m_capital_city_label->set_text
            (String::ucompose (_("Capital city of %1"),
                               m_city->getCapitalOwner ()->getName ()));
        else
          m_capital_city_label->set_text ("");

        m_defense_label->set_text
          (String::ucompose ("%1", m_city->getDefenseLevel ()));
        m_income_label->set_text (String::ucompose ("%1", m_city->getGold ()));
        m_description_label->set_text (m_city->getDescription ());
        switch (GameScenarioOptions::s_build_production_mode)
          {
          case GameParameters::BUILD_PRODUCTION_ALWAYS:
          case GameParameters::BUILD_PRODUCTION_NEVER:
            m_rebellious_label->set_text ("");
            break;
          case GameParameters::BUILD_PRODUCTION_USUALLY:
          case GameParameters::BUILD_PRODUCTION_SELDOM:
            if (m_city->getBuildProduction ())
              m_rebellious_label->set_text (_("The inhabitants are unruly!"));
            else
              m_rebellious_label->set_text ("");
            break;
          }
      }

    void fill_in_production_toggles ()
      {
        int production_index = m_city->getActiveProductionSlot ();

        m_ignore_toggles = true;
        for (unsigned int i = 0; i < m_city->getMaxNoOfProductionBases (); i++)
          {
            Gtk::ToggleButton *toggle = m_production_toggles[i];
            update_toggle_picture (i);
            toggle->set_active ((int)i == production_index);
            toggle->show ();
          }
        m_ignore_toggles = false;

        m_stop_button->set_sensitive (production_index != -1);
        fill_in_production_info ();
      }

    void show_stat_titles (bool s)
      {
        m_time_title_label->set_visible (s);
        m_moves_title_label->set_visible (s);
        m_strength_title_label->set_visible (s);
        m_cost_title_label->set_visible (s);
      }

    void fill_in_production_info ()
      {
        Player *player = m_city->getOwner ();
        unsigned int as = player->getArmyset ();
        Glib::RefPtr<Gdk::Pixbuf> pic;
        ImageCache *gc = ImageCache::instance ();
        int slot = m_city->getActiveProductionSlot ();
        Glib::RefPtr<Gdk::Pixbuf> empty_pic =
          ImageCache::instance ()->getEmptyCircledArmyPic
          (Lw::get_dark ())->to_pixbuf ();

        Glib::ustring s1, s2, s3, s5;
        Glib::ustring s4 = _("Current:");

        show_stat_titles (slot != -1);
        if (slot == -1)
          {
            pic = empty_pic;
            m_unit_label->set_text ("");
            m_time_label->set_text ("");
            m_moves_label->set_text ("");
            m_strength_label->set_text ("");
            m_cost_label->set_text ("");
            m_bonus_label->set_text ("");
          }
        else
          {
            const ArmyProdBase * a = m_city->getProductionBase (slot);

            m_unit_label->set_text (a->getName ());
            m_time_label->set_text (String::ucompose ("%1",
                                                      a->getProduction ()));
            m_strength_label->set_text (String::ucompose ("%1",
                                                          a->getStrength ()));
            m_moves_label->set_text (String::ucompose ("%1",
                                                       a->getMaxMoves ()));
            m_cost_label->set_text (String::ucompose ("%1", a->getUpkeep ()));

            if (m_city->getVectoring () != Vector<int>(-1, -1))
              {
                Citylist *cl = Citylist::instance ();
                City *dest =
                  cl->getNearestFriendlyCity (m_city->getVectoring (), 4);
                //note to translators, Standard means like a flag
                m_time_label->set_text
                  (String::ucompose (_("%1t, then to %2"),
                                     m_city->getDuration (),
                                     dest ? dest->getName () : _("Standard")));
              }
            else
              //note to translators, t means turn.
              m_time_label->set_text
                (String::ucompose (_("%1t"), m_city->getDuration ()));
            pic = gc->getCircledArmyPic (as, a->getTypeId (),
                                         player->get_shield (), NULL,
                                         false, Shield::NEUTRAL,
                                         true, Lw::get_dark ())->to_pixbuf ();

            auto bonus = a->getArmyBonusDescription ();
            if (bonus != "" && a->getMoveBonusDescription () != "")
              bonus += "\n" + a->getMoveBonusDescription ();
            else if (bonus == "" && a->getMoveBonusDescription () != "")
              bonus = a->getMoveBonusDescription ();
            m_bonus_label->set_text (bonus);
          }

        m_current_image->set (pic);
        m_turns_left_label->set_text (s3);
        m_current_label->set_text (s4);

        if (m_city->getOwner () != Playerlist::getActiveplayer ())
          {
            m_turns_left_label->set_text ("");
            m_current_label->set_text ("");
            pic->fill (0x00000000);
            m_current_image->set (pic);
            m_buy_button->set_sensitive (false);
            m_raze_button->set_sensitive (false);
            m_rename_button->set_sensitive (false);
            m_destination_button->set_sensitive (false);
            m_stop_button->set_sensitive (false);
            for (unsigned int i = 0; i < m_production_toggles.size (); ++i)
              m_production_toggles[i]->set_active (false);
            m_unit_label->set_text ("");
            m_time_label->set_text ("");
            m_moves_label->set_text ("");
            m_strength_label->set_text ("");
            m_cost_label->set_text ("");
            m_bonus_label->set_text ("");
          }
        else
          {
            m_buy_button->set_sensitive (m_city->getBuildProduction ());
            m_raze_button->set_sensitive (m_razing_possible);
            m_rename_button->set_sensitive (true);
            m_destination_button->set_sensitive (true);
            m_stop_button->set_sensitive (true);
          }
      }

    void update_toggle_picture (int slot)
      {
        Player *player = m_city->getOwner ();
        unsigned int as = player->getArmyset ();
        ImageCache *gc = ImageCache::instance ();
        Gtk::ToggleButton *toggle = m_production_toggles[slot];
        Glib::RefPtr<Gdk::Pixbuf> pic;
        if (m_city->getArmytype (slot) == -1)
          pic =
            gc->getCircledArmyPic (as, 0, player->get_shield (), NULL, false,
                                   Shield::NEUTRAL, false,
                                   Lw::get_dark ())->to_pixbuf ();
        else
          {
            int type = m_city->getArmytype (slot);
            pic =
              gc->getCircledArmyPic (as, type, player->get_shield (), NULL,
                                     false,
                                     slot == m_city->getActiveProductionSlot ()
                                     ?
                                     player->getId (): int(Shield::NEUTRAL),
                                     true, Lw::get_dark ())->to_pixbuf ();
          }
        Gtk::Image *image = Gtk::make_managed<Gtk::Image> ();
        image->set_pixel_size (LW_BUTTON_SIZE);
        image->set (pic);
        if (toggle->get_child ())
          toggle->unset_child ();
        toggle->set_child (*image);
        toggle->show ();
      }
};
#endif
