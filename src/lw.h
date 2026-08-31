//  Copyright (C) 2026 Ben Asselstine
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

#include <config.h>
#include <iostream>
#include <gtkmm.h>
#include "defs.h"
#include "startup.h"
#include "game-host-server.h"

#ifndef LW_H
#define LW_H

class Lw : public Gtk::Application
{
protected:
    Lw ()
      :Gtk::Application (LW_APP_ID,
                         Gio::Application::Flags::HANDLES_COMMAND_LINE |
                         Gio::Application::Flags::NON_UNIQUE),
      m_command (""),
      m_cacheSize (0),
      m_configuration_file_path (""),
      m_save_path (""),
      m_random_number_seed (0),
      m_port (0),
      m_turn_filename (""),
      m_start_test_scenario (false),
      m_start_net_test_scenario (false),
      m_speedy (false),
      m_own_all_on_round_two (false),
      m_start_stress_test (false),
      m_start_stress_test_type (GameParameters::Player::Type::EASY),
      m_start_headless_server (false),
      m_network_debug (false),
      m_view_stress_test (false),
      m_view_stress_test_fast (false),
      m_start_robots (0),
      m_start_editor (false),
      m_start_game_list_server (false),
      m_start_game_host_server (false),
      m_start_game_list_client (false),
      m_start_upgrade_tool (false),
      m_start_import_tool (false),
      m_start_cityset_editor (false),
      m_start_shieldset_editor (false),
      m_start_armyset_editor (false),
      m_start_tileset_editor (false),
      m_start_game_host_client (false),
      m_save_server_messages (""),
      m_load_filename (""),
      m_shieldset_theme (""),
      m_gls_port (0),
      m_gls_foreground (false),
      m_ghs_port (0),
      m_ghs_foreground (false),
      m_ghs_hostname (""),
      m_glc_profile (NULL),
      m_glc_host (""),
      m_glc_advertise (false),
      m_glc_show_list (false),
      m_glc_reload (false),
      m_glc_terminate (false),
      m_glc_port (0),
      m_glc_remove_all (""),
      m_ghc_profile (NULL),
      m_ghc_host (""),
      m_ghc_file (""),
      m_ghc_unhost (""),
      m_ghc_show_list (false),
      m_ghc_reload (false),
      m_ghc_terminate (false),
      m_ghc_port (false),
      m_upgrade_identify_file (false),
      m_upgrade_rewrite (""),
      m_upgrade_filename (""),
      m_import_armyset_filename (""),
      m_import_filename ("")
  {
  }

public:
    static Lw *app;
    static Glib::RefPtr<Glib::MainLoop> loop;
    static int exit_code;
    static void quit_loop ();
    static bool get_dark ();

    static void do_events ()
      {
         while (g_main_context_iteration(NULL, FALSE));
      }

    static Glib::RefPtr<Lw> create ()
      {
        return Glib::make_refptr_for_instance (new Lw ());
      }

    void setup_css ()
      {
        auto provider = Gtk::CssProvider::create ();
        provider->load_from_resource (RESOURCE "lw.css");
        Gtk::StyleContext::add_provider_for_display
          (Gdk::Display::get_default (), provider,
           GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
      }

    void initialize ();

    Glib::ustring m_command;
    int m_cacheSize;
    Glib::ustring m_configuration_file_path;
    std::string m_save_path;
    int m_random_number_seed;
    int m_port;
    Glib::ustring m_turn_filename;
    bool m_start_test_scenario;
    bool m_start_net_test_scenario;
    bool m_speedy;
    bool m_own_all_on_round_two;
    bool m_start_stress_test;
    GameParameters::Player::Type m_start_stress_test_type;
    bool m_start_headless_server;
    bool m_network_debug;
    bool m_view_stress_test;
    bool m_view_stress_test_fast;
    int m_start_robots;
    bool m_start_editor;
    bool m_start_game_list_server;
    bool m_start_game_host_server;
    bool m_start_game_list_client;
    bool m_start_upgrade_tool;
    bool m_start_import_tool;
    bool m_start_cityset_editor;
    bool m_start_shieldset_editor;
    bool m_start_armyset_editor;
    bool m_start_tileset_editor;
    bool m_start_game_host_client;
    Glib::ustring m_save_server_messages;
    Glib::ustring m_load_filename;

    // tileset and armyset editor options
    Glib::ustring m_shieldset_theme;

    //game list server options
    int m_gls_port;
    bool m_gls_foreground;

    //game host server options
    int m_ghs_port;
    bool m_ghs_foreground;
    std::string m_ghs_hostname;
    std::list<Glib::ustring> m_ghs_members;

    //game list client options
    Profile *m_glc_profile;
    Glib::ustring m_glc_host;
    bool m_glc_advertise;
    bool m_glc_show_list;
    bool m_glc_reload;
    bool m_glc_terminate;
    int m_glc_port;
    std::list<Glib::ustring> m_glc_unadvertise;
    Glib::ustring m_glc_remove_all;

    //game host client options
    Profile *m_ghc_profile;
    Glib::ustring m_ghc_host;
    Glib::ustring m_ghc_file;
    Glib::ustring m_ghc_unhost;
    bool m_ghc_show_list;
    bool m_ghc_reload;
    bool m_ghc_terminate;
    int m_ghc_port;

    //upgrade tool options
    bool m_upgrade_identify_file;
    std::string m_upgrade_rewrite;
    std::string m_upgrade_filename;

    //import tool options
    std::string m_import_armyset_filename;
    std::string m_import_filename;

    void process_game_list_server_options (int argc, char **argv);
    void process_game_host_server_options (int argc, char **argv);
    void process_game_list_client_options (int argc, char **argv);
    void process_game_host_client_options (int argc, char **argv);
    void process_upgrade_tool_options (int argc, char **argv);
    void process_import_tool_options (int argc, char **argv);
    void process_armyset_editor_options (int argc, char **argv);
    void process_cityset_editor_options (int argc, char **argv);
    void process_shieldset_editor_options (int argc, char **argv);
    void process_tileset_editor_options (int argc, char **argv);
    void process_editor_options (int argc, char **argv);

    int on_command_line (const Glib::RefPtr<Gio::ApplicationCommandLine>& cmd) override
      {
        Lw::app = this;
        int argc;
        char **argv = cmd->get_arguments (argc);

        if (argc >= 2)
          {
            if (strcmp (argv[1], "game-list-server") == 0)
              m_start_game_list_server = true;
            else if (strcmp (argv[1], "game-host-server") == 0)
              m_start_game_host_server = true;
            else if (strcmp (argv[1], "glsctl") == 0)
              m_start_game_list_client = true;
            else if (strcmp (argv[1], "ghsctl") == 0)
              m_start_game_host_client = true;
            else if (strcmp (argv[1], "upgrade-file") == 0)
              m_start_upgrade_tool = true;
            else if (strcmp (argv[1], "import") == 0)
              m_start_import_tool = true;
            else if (strcmp (argv[1], "cityset-editor") == 0)
              m_start_cityset_editor = true;
            else if (strcmp (argv[1], "shieldset-editor") == 0)
              m_start_shieldset_editor = true;
            else if (strcmp (argv[1], "armyset-editor") == 0)
              m_start_armyset_editor = true;
            else if (strcmp (argv[1], "tileset-editor") == 0)
              m_start_tileset_editor = true;
            else if (strcmp (argv[1], "editor") == 0)
              m_start_editor = true;
          }
        if (m_start_game_list_server)
          {
            m_command = argv[1];
            argv++;
            argc--;
            process_game_list_server_options (argc, argv);
          }
        else if (m_start_game_host_server)
          {
            m_command = argv[1];
            argv++;
            argc--;
            process_game_host_server_options (argc, argv);
          }
        else if (m_start_game_list_client)
          {
            m_command = argv[1];
            argv++;
            argc--;
            process_game_list_client_options (argc, argv);
          }
        else if (m_start_game_host_client)
          {
            m_command = argv[1];
            argv++;
            argc--;
            process_game_host_client_options (argc, argv);
          }
        else if (m_start_upgrade_tool)
          {
            m_command = argv[1];
            argv++;
            argc--;
            process_upgrade_tool_options (argc, argv);
          }
        else if (m_start_import_tool)
          {
            m_command = argv[1];
            argv++;
            argc--;
            process_import_tool_options (argc, argv);
          }
        else if (m_start_cityset_editor)
          {
            m_command = argv[1];
            argv++;
            argc--;
            process_cityset_editor_options (argc, argv);
          }
        else if (m_start_shieldset_editor)
          {
            m_command = argv[1];
            argv++;
            argc--;
            process_shieldset_editor_options (argc, argv);
          }
        else if (m_start_armyset_editor)
          {
            m_command = argv[1];
            argv++;
            argc--;
            process_armyset_editor_options (argc, argv);
          }
        else if (m_start_tileset_editor)
          {
            m_command = argv[1];
            argv++;
            argc--;
            process_tileset_editor_options (argc, argv);
          }
        else if (m_start_editor)
          {
            m_command = argv[1];
            argv++;
            argc--;
            process_editor_options (argc, argv);
          }
        else
          {
            for (int i = 2; i <= argc; i++)
              parse_parameter (i, argc, argv);
          }

        activate ();
        return 0;
      }

    void cleanup ();

    static Glib::ustring get_prgname ()
      {
        if (Lw::app->m_command == "")
          return Glib::get_prgname ();
        return String::ucompose ("%1 %2", Glib::get_prgname (),
                                 Lw::app->m_command);
      }
protected:
    void on_activate () override;

private:

    void parse_parameter (int &i, int argc, char **argv);
    void validate_option_combinations ();
};
#endif
