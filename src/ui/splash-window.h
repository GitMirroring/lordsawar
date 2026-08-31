//  Copyright (C) 2007 Ole Laursen
//  Copyright (C) 2007, 2008, 2009, 2010, 2011, 2012, 2014, 2015, 2016, 2020,
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
#ifndef SPLASH_WINDOW_H
#define SPLASH_WINDOW_H
#include "lw-dialog.h"
#include "main-preferences-dialog.h"
#include "new-net-game-dialog.h"
#include "snd.h"
#include "profile.h"
class SplashWindow : public Gtk::ApplicationWindow
{
public:
    SplashWindow ()
      {
        int width, height;
        set_title ("Lw");
        set_resizable (false);
        auto new_game_button = Gtk::make_managed<Gtk::Button> (_("New Game"));
        auto new_net_game_button =
          Gtk::make_managed<Gtk::Button> (_("New Network Game"));
        auto load_game_button = Gtk::make_managed<Gtk::Button> (_("Load Game"));
        auto preferences_button =
          Gtk::make_managed<Gtk::Button> (_("Preferences"));
        auto editor_button = Gtk::make_managed<Gtk::Button> (_("Scenario Builder"));
        auto quit_button = Gtk::make_managed<Gtk::Button> (_("Quit"));
        auto overlay = Gtk::make_managed<Gtk::Overlay>();
        try
          {
            auto texture =
              Gdk::Texture::create_from_filename
              (File::getVariousFile ("splash_screen.png"));
            auto picture = Gtk::make_managed<Gtk::Picture> ();
            picture->set_paintable (texture);

            width = texture->get_width ();
            height = texture->get_height ();

            set_default_size (width, height);

            picture->set_can_shrink (false);
            picture->set_keep_aspect_ratio (true);
            overlay->set_child (*picture);
          }
        catch (const Glib::Error& ex)
          {
            std::cerr << "Could not load image: " << ex.what () << std::endl;
            width = 640;
            height = 480;
            set_default_size (width, height);
          }

        auto box = Gtk::make_managed<Gtk::Box> ();
        box->set_orientation (Gtk::Orientation::VERTICAL);
        box->set_spacing (6);
        box->set_margin_top (height / 2.5);
        box->set_margin_end (10);
        box->set_margin_bottom (10);

        box->append (*new_game_button);
        box->append (*new_net_game_button);
        box->append (*load_game_button);
        box->append (*preferences_button);
        box->append (*editor_button);
        box->append (*quit_button);

        box->set_halign (Gtk::Align::END);
        box->set_valign (Gtk::Align::START);

        overlay->add_overlay (*box);
        set_child (*overlay);

        new_game_button->signal_clicked ().connect
          ([this] ()
           {
             m_signal_setup_hotseat_game.emit (this);
           });

        new_net_game_button->signal_clicked ().connect
          ([this] ()
           {
             auto d = LwDialog::build<NewNetGameDialog>(this);
             d->setup ();
             d->signal_start_client ().connect
               ([this] (Glib::ustring host, unsigned short port,
                        Profile *profile)
                {
                  m_signal_setup_net_client.emit (this, host, port, profile);
                });

             d->signal_start_server ().connect
               ([this] (Profile *profile)
                {
                  m_signal_setup_net_server.emit (this, profile);
                });
             d->signal_response ().connect
               ([d] (Gtk::ResponseType resp)
                {
                  if (resp != Gtk::ResponseType::ACCEPT &&
                      resp != Gtk::ResponseType::REJECT)
                    delete d;
                });
           });

        load_game_button->signal_clicked ().connect
          ([this] ()
           {
             load_game ();
           });

        preferences_button->signal_clicked ().connect
          ([this] ()
           {
             auto d = LwDialog::build<MainPreferencesDialog>(this);
             d->setup ();
             d->signal_response ().connect
               ([d] (Gtk::ResponseType)
                {
                  delete d;
                });
           });

        editor_button->signal_clicked ().connect
          ([this] ()
           {
             m_signal_scenario_builder.emit ();
           });

        quit_button->signal_clicked ().connect
          ([this] ()
           {
             get_application ()->quit ();
           });

        Snd::instance ()->play ("intro");

        auto controller = Gtk::EventControllerKey::create ();
        controller->signal_key_pressed().connect
          ([this] (guint keyval, guint, Gdk::ModifierType)
           {
             if (keyval == GDK_KEY_Escape)
               {
                 hide ();
                 return true;
               }
             return false;
           }, false);
        add_controller (controller);
      }

    ~SplashWindow ()
      {
      }

    sigc::signal<void(Gtk::ApplicationWindow*)> signal_setup_hotseat_game ()
      {
        return m_signal_setup_hotseat_game;
      }

    sigc::signal<void(Gtk::ApplicationWindow *, Glib::ustring,
                      unsigned short port, Profile *)> 
      signal_setup_net_client  ()
        {
          return m_signal_setup_net_client;
        }

    sigc::signal<void(Gtk::ApplicationWindow *, Profile *)>
      signal_setup_net_server ()
        {
          return m_signal_setup_net_server;
        }

    sigc::signal<void(Gtk::ApplicationWindow*, std::string)> signal_load_game ()
      {
        return m_signal_load_game;
      }

    sigc::signal<void()> signal_scenario_builder ()
      {
        return m_signal_scenario_builder;
      }
protected:

    void load_game ()
      {
        auto dialog = Gtk::FileDialog::create ();
        dialog->set_title (_("Load A Game"));

        auto filter = Gtk::FileFilter::create ();
        filter->set_name
          (String::ucompose (_("LordsAWar Saved Games (*%1)"), SAVE_EXT));
        filter->add_pattern ("*" + SAVE_EXT);

        auto filters = Gio::ListStore<Gtk::FileFilter>::create ();
        filters->append (filter);
        dialog->set_filters (filters);

        dialog->open
          (*this,
           [this, dialog](const Glib::RefPtr<Gio::AsyncResult>& result)
           {
             try 
               {
                 auto file = dialog->open_finish (result);
                 if (file)
                   m_signal_load_game.emit (this, file->get_path ());
               }
             catch (const Gtk::DialogError& e)
               {
                 // User pressed Escape or closed dialog
                 if (e.code() == Gtk::DialogError::Code::DISMISSED)
                   {
                     return;
                   }
               }
             catch (const Glib::Error& e)
               {
                 std::cerr << "Error: " << e.what() << '\n';
               }
           });
      }

private:
    sigc::signal<void(Gtk::ApplicationWindow*)> m_signal_setup_hotseat_game;
    sigc::signal<void(Gtk::ApplicationWindow*,std::string)> m_signal_load_game;
    sigc::signal<void()> m_signal_scenario_builder;
    sigc::signal<void(Gtk::ApplicationWindow*, Glib::ustring,
                      unsigned short port, Profile *)>
      m_signal_setup_net_client;
    sigc::signal<void(Gtk::ApplicationWindow*, Profile *)>
      m_signal_setup_net_server;
};
#endif
