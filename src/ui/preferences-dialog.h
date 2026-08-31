//  Copyright (C) 2007 Ole Laursen
//  Copyright (C) 2007, 2008, 2009, 2014, 2015, 2017, 2020, 2026 Ben Asselstine
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
#ifndef PREFERENCES_DIALOG_H
#define PREFERENCES_DIALOG_H
#include "snd.h"
#include "lw-combo.h"
#include "game-options-dialog.h"
#include "ai-fast.h"
#include "real-player.h"
class PreferencesDialog: public LwDialogBase
{
public:


    static std::string get_resource_name ()
      {
        return "preferences.ui";
      }

    PreferencesDialog (BaseObjectType* o,
                       const Glib::RefPtr<Gtk::Builder>& xml)
      : LwDialogBase (o, xml)
      {
        m_close_button = load <Gtk::Button> ("close_button");
        m_commentator_switch = load <Gtk::Switch> ("commentator_switch");
        m_speed_scale = load <Gtk::Scale> ("speed_scale");
        m_play_music_switch = load <Gtk::Switch> ("play_music_switch");
        m_music_volume_scale = load <Gtk::Scale> ("music_volume_scale");
        m_players_vbox = load <Gtk::Box> ("players_vbox");
        m_game_options_button = load <Gtk::Button> ("game_options_button");
      }

    void setup ()
      {
        set_response (m_close_button, Gtk::ResponseType::ACCEPT);

        m_game_options_button->signal_clicked ().connect
          ([this] ()
           {
             auto d = LwDialog::build<GameOptionsDialog> (this);
             d->setup (true);
             d->signal_response ().connect
               ([this, d] (Gtk::ResponseType)
                {
                  delete d;
                });
           });

        m_commentator_switch->set_active (Configuration::s_displayCommentator);
        m_commentator_switch->property_active ().signal_changed ().connect
          ([this] ()
           {
             Configuration::s_displayCommentator =
               m_commentator_switch->get_active ();
           });
        {
          double max = slowest_display_speed;
          guint32 delay = Configuration::s_displaySpeedDelay;
          if (delay > max)
            delay = max;
          double fraction = (double)delay / max;
          fraction = 1 - fraction;
          m_speed_scale->set_value (fraction * 100.0);
        }
        m_speed_scale->signal_value_changed ().connect
          ([this] ()
           {
             double max = slowest_display_speed;
             double fraction = (100.0 - m_speed_scale->get_value ()) / 100.0;
             Configuration::s_displaySpeedDelay = int (fraction * max);
           });

        m_play_music_switch->set_active (Configuration::s_musicenable);
        m_play_music_switch->property_active ().signal_changed ().connect
          ([this] ()
           {
             bool play_music = m_play_music_switch->get_active ();

             Configuration::s_musicenable = play_music;

             if (play_music)
               Snd::instance ()->enableBackground ();
             else
               {
                 Snd::instance ()->halt ();
                 Snd::instance ()->disableBackground ();
               }
             m_music_volume_scale->set_sensitive (Configuration::s_musicenable);
           });
    
        m_music_volume_scale->set_value
          (Configuration::s_musicvolume * 100.0 / 128);
        m_music_volume_scale->set_sensitive (Configuration::s_musicenable);
        m_music_volume_scale->signal_value_changed ().connect
          ([this] ()
           {
             int volume = int(m_music_volume_scale->get_value () / 100 * 128);

             Configuration::s_musicvolume = volume;
             Snd::instance ()->updateVolume ();
           });

        fill_players ();

        signal_response ().connect
          ([this] (Gtk::ResponseType resp)
           {
             Configuration::saveConfigurationFile();
             if (resp != Gtk::ResponseType::ACCEPT)
               return;
             for (auto pt : m_player_types)
               {
                 Player *p = pt.first;
                 if (p == NULL)
                   continue;
                 if (p == Playerlist::getNeutral ())
                   continue;
                 if (p->getType () == Player::HUMAN) //changing human to:
                   {
                     if (pt.second->get_active_text () == _("Human")) //human, no change
                       ;
                     else //computer, change to easy
                       {
                         AI_Fast *new_player = new AI_Fast (*p, true);
                         Player *old_player = p;
                         Playerlist::instance ()->swap (old_player, new_player);
                         //disconnect and connect game signals
                         m_signal_add_player.emit (new_player);
                         delete old_player;
                       }
                   }
                 else //changing computer to:
                   {
                     if (pt.second->get_active_text() == _("Human")) //human, change it
                       {
                         RealPlayer *new_player = new RealPlayer (*p, true);
                         Player *old_player = p;
                         Playerlist::instance ()->swap (old_player, new_player);
                         //disconnect and connect game signals
                         m_signal_add_player.emit (new_player);
                         delete old_player;
                       }
                   }
               }
           });
      }

    sigc::signal<void (Player *)> signal_add_player ()
      {
        return m_signal_add_player;
      }
private:
    Gtk::Button *m_close_button;
    Gtk::Switch *m_commentator_switch;
    Gtk::Scale *m_speed_scale;
    Gtk::Switch *m_play_music_switch;
    Gtk::Scale *m_music_volume_scale;
    Gtk::Box *m_players_vbox;
    Gtk::Button *m_game_options_button;

    typedef std::map<Player*, LwCombo*> PlayerTypeMap;
    PlayerTypeMap m_player_types;

    typedef std::map<Player*, Gtk::CheckButton*> PlayerObserveMap;
    PlayerObserveMap m_player_observed;

    typedef std::map<Player*, Gtk::Label*> PlayerNameMap;
    PlayerNameMap m_player_name;

    const double slowest_display_speed = 2000;

    sigc::signal<void (Player *)> m_signal_add_player;

    void fill_player (Player *p)
      {
        auto hbox =
          Gtk::make_managed<Gtk::Box> (Gtk::Orientation::HORIZONTAL);
        hbox->set_spacing (16);
        auto im = Gtk::make_managed<Gtk::Image>();
        im->set_pixel_size (LW_BUTTON_SIZE);
        auto pixbuf =
          ImageCache::instance ()->getShieldPic (2, p, false)->to_pixbuf ();
        im->set (pixbuf);
        auto type = Gtk::make_managed<LwCombo>();
        type->append (_("Human"));
        type->append (_("Computer"));
        type->append (_("Networked"));

        auto observe = Gtk::make_managed<Gtk::CheckButton> (_("Observe"));
        observe->property_active ().signal_changed ().connect
          ([this, button = observe] ()
           {
             for (auto po : m_player_observed)
               if (button == po.second)
                 po.first->setObservable (button->get_active ());
           });

        if (p->getType () == Player::HUMAN)
          {
            observe->set_sensitive (false);
            type->set_active (0);
          }
        else if (p->getType () == Player::NETWORKED)
          {
            observe->set_sensitive (false);
            type->set_active (2);
          }
        else
          type->set_active (1);

        if (p->isDead () || (Playerlist::getActiveplayer () == p && 
                             p->getType () != Player::HUMAN))
          {
            type->set_sensitive (false);
            observe->set_sensitive (false);
          }

        type->signal_changed ().connect
          ([this, combo = type] ()
           {
             for (auto pt : m_player_types)
               {
                 if (combo == pt.second)
                   {
                     /**
                      * if we're turning this player into a human,
                      * then we desensitize the associated switch
                      * otherwise, we presume that we want to observe it
                      */
                     if (combo->get_active_text () == _("Human"))
                       {
                         m_player_observed[pt.first]->set_active (true);
                         m_player_observed[pt.first]->set_sensitive (false);
                       }
                     else
                       {
                         m_player_observed[pt.first]->set_sensitive (true);
                         m_player_observed[pt.first]->set_active (true);
                       }
                   }
               }
           });

        observe->set_active (p->isObservable ());
        if (p->getType () == Player::HUMAN)
          {
            observe->set_sensitive (false);
            type->set_active (0);
          }
        else if (p->getType () == Player::NETWORKED)
          {
            observe->set_sensitive (false);
            type->set_active (2);
          }
        else
          type->set_active (1);

        if (p->isDead () || (Playerlist::getActiveplayer () == p && 
                             p->getType () != Player::HUMAN))
          {
            type->set_sensitive (false);
            observe->set_sensitive (false);
          }
        hbox->append (*im);
        auto name = Gtk::make_managed<Gtk::Label> (p->getName ());
        name->set_halign (Gtk::Align::START);
        name->set_valign (Gtk::Align::CENTER);
        hbox->append (*name);
        hbox->append (*type);
        hbox->append (*observe);

        m_player_types[p] = type;
        m_player_observed[p] = observe;
        m_player_name[p] = name;
        m_players_vbox->append (*hbox);
      }

    void fill_players ()
      {
        for (auto p : *Playerlist::instance ())
          {
            if (p == Playerlist::getNeutral ())
              continue;
            fill_player (p);
          }

        int max = 0;
        for (auto p : m_player_name)
          {
            int w = p.second->get_width ();
            if (w > max)
              max = w;
          }
        for (auto p : m_player_name)
          {
            p.second->set_size_request (max, -1);
            p.second->set_hexpand (true);
            p.second->set_halign (Gtk::Align::START);
          }

        max = 0;
        for (auto p : m_player_types)
          {
            int w = p.second->get_width ();
            if (w > max)
              max = w;
          }
        for (auto p : m_player_types)
          {
            p.second->set_size_request (max, -1);
            p.second->set_halign (Gtk::Align::END);
          }
      }
};
#endif
