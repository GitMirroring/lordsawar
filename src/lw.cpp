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

#include "lw.h"
#include <gst/gst.h>
#include "profile-list.h"
#include "ucompose.hpp"
#include "rnd.h"
#include "vector.h"
#include "army-set-list.h"
#include "tile-set-list.h"
#include "city-set-list.h"
#include "shield-set-list.h"
#include "image-cache.h"
#include "hero-templates.h"
Lw *Lw::app;
Glib::RefPtr<Glib::MainLoop> Lw::loop;
int Lw::exit_code;
using namespace std;
void Lw::initialize ()
{
  if (m_random_number_seed)
    Rnd::instance ()->set_seed (m_random_number_seed);
  else
    Rnd::instance ()->set_seed (time (NULL));

  Vector<int>::setMaximumWidth (1000);
  gst_init (NULL, NULL);
}

void Lw::on_activate ()
{
  setup_css ();
  initialize ();
  if (get_dark ())
    {
      // this is a workaround
      // gtk is supposed to color this application according to the dark style
      // of the system automatically, but nope we have to set this old way
      // of doing it.
      auto settings = Gtk::Settings::get_default ();
      settings->property_gtk_application_prefer_dark_theme () = true;
    }
  Startup *start = Startup::instance ();
  if (m_start_game_host_client)
    Lw::exit_code =
      start->game_host_client (m_ghc_profile, m_ghc_host, m_ghc_file,
                               m_ghc_unhost, m_ghc_show_list, m_ghc_reload,
                               m_ghc_terminate, m_ghc_port);
  else if (m_start_game_list_client)
    Lw::exit_code =
      start->game_list_client (m_glc_profile, m_glc_host, m_glc_advertise,
                               m_glc_show_list, m_glc_reload, m_glc_terminate,
                               m_glc_port, m_glc_unadvertise, m_glc_remove_all);
  else if (m_start_game_list_server)
    Lw::exit_code =
      start->game_list_server (m_gls_port, m_gls_foreground);
  else if (m_start_game_host_server)
    Lw::exit_code =
      start->game_host_server (m_ghs_hostname, m_ghs_port, m_ghs_foreground,
                               m_ghs_members);
  else if (m_start_upgrade_tool)
    Lw::exit_code =
      start->upgrade_tool (m_upgrade_identify_file, m_upgrade_rewrite,
                           m_upgrade_filename);
  else if (m_start_import_tool)
    Lw::exit_code =
      start->import_tool (m_import_armyset_filename, m_import_filename);
  else if (m_start_test_scenario)
    start->test_scenario ();
  else if (m_start_net_test_scenario)
    start->net_test_scenario ();
  else if (m_start_stress_test)
    start->stress_test (m_view_stress_test, m_start_stress_test_type, m_view_stress_test_fast);
  else if (m_start_headless_server)
    start->server (m_load_filename);
  else if (m_start_editor)
    start->editor (m_load_filename);
  else if (m_start_cityset_editor)
    start->cityset_editor (m_load_filename);
  else if (m_start_shieldset_editor)
    start->shieldset_editor (m_load_filename);
  else if (m_start_armyset_editor)
    start->armyset_editor (m_shieldset_theme, m_load_filename);
  else if (m_start_tileset_editor)
    start->tileset_editor (m_shieldset_theme, m_load_filename);
  else if (File::nameEndsWith (m_load_filename, MAP_EXT))
    start->load_map (m_load_filename);
  else if (File::nameEndsWith (m_load_filename, SAVE_EXT))
    start->load_saved_game (m_load_filename);
  else
    start->splash ();
}

void Lw::parse_parameter (int &i, int argc, char **argv)
{
  Glib::ustring parameter(argv[i-1]); 
  if (parameter == "--cache-size")
    {
      i++;
      if (i - 1 >= argc)
        {
          cerr << _("missing argument for --cache-size") << endl;
          exit (1);
        }
      char* error = 0;
      long size = strtol (argv[i-1], &error, 10);
      if (error && (*error != '\0'))
        {
          cerr << _("non-numerical value for cache size") << endl;
          exit (1);
        }
      m_cacheSize = size;
    }
  else if (parameter == "--config-file")
    {
      i++;
      if (i - 1 >= argc)
        {
          cerr << _("missing argument for --config-file") << endl;
          exit (1);
        }
      m_configuration_file_path = argv[i-1];
    }
  else if (parameter == "--seed")
    {
      i++;
      char* error = 0;
      if (i - 1 >= argc)
        {
          cerr << _("missing argument for --seed") << endl;
          exit (1);
        }
      long seed = strtol (argv[i-1], &error, 10);
      if (error && (*error != '\0'))
        {
          cerr << _("non-numerical value for --seed") << endl;
          exit (1);
        }
      m_random_number_seed = seed;
    }
  else if (parameter == "--save-path")
    {
      i++;
      if (i - 1 >= argc)
        {
          std::cerr << _("missing argument for --save-path") << std::endl;
          exit (1);
        }
      m_save_path = argv[i-1];
    }
  else if (parameter == "--port")
    {
      i++;
      if (i - 1 >= argc)
        {
          cerr << _("missing argument for --port") << endl;
          exit (1);
        }
      char* error = 0;
      long port = strtol (argv[i-1], &error, 10);
      if (error && (*error != '\0'))
        {
          cerr << _("non-numerical value for --port") << endl;
          exit (1);
        }
      if (port > 65535 || port < 1000)
        {
          cerr << _("invalid value for --port") << endl;
          exit (1);
        }
      m_port = port;
    }
  else if (parameter == "--turn")
    {
      i++;
      if (i - 1 >= argc)
        {
          cerr << _("missing argument for --turn") << endl;
          exit (1);
        }
      m_turn_filename = argv[i-1];
    }
  else if (parameter == "--test")
    m_start_test_scenario = true;
  else if (parameter == "--net-test")
    m_start_net_test_scenario = true;
  else if (parameter == "--speedy")
    m_speedy = true;
  else if (parameter == "--own-all-on-round-two")
    m_own_all_on_round_two = true;
  else if (parameter == "--stress-test")
    m_start_stress_test = true;
  else if (parameter == "--stress-test=hard")
    {
      m_start_stress_test = true;
      m_start_stress_test_type = GameParameters::Player::Type::HARD;
    }
  else if (parameter == "--stress-test=easy")
    {
      m_start_stress_test = true;
      m_start_stress_test_type = GameParameters::Player::Type::EASY;
    }
  else if (parameter == "--view-stress-test")
    m_view_stress_test = true;
  else if (parameter == "--view-stress-test=fast")
    {
      m_view_stress_test = true;
      m_view_stress_test_fast = true;
    }
  else if (parameter == "--view-stress-test=slow")
    {
      m_view_stress_test = true;
      m_view_stress_test_fast = false;
    }
  else if (parameter == "--robots")
    m_start_robots = -1;
  else if (parameter == "--host")
    m_start_headless_server = true;
  else if (parameter == "--network-debug")
    m_network_debug = true;
  else if (parameter == "--editor")
    m_start_editor = true;
  else if (parameter == "--save-server-messages")
    {
      i++;
      if (i - 1 >= argc)
        {
          cerr <<
            _("missing argument for --save-server-messages") << endl;
          exit (1);
        }
      m_save_server_messages = argv[i-1];
    }
  else if (parameter == "--help" || parameter == "-?")
    {
      cout <<
        String::ucompose
        (_("Usage: %1 [OPTION]... [FILE]\n"
           "  or:  %1 COMMAND [OPTION]...\n"
           "\n"
           "Options:\n"
           "      --config-file FILE     Use FILE instead of %2\n"
           "      --cache-size SIZE      Set the maximum image cache size to SIZE bytes\n"
           "      --host                 Start a server\n"
           "      --port NUMBER          Start the server on port NUMBER\n"
           "      --editor               Start the scenario builder\n"
           "      --version              Display version information and exit\n"
           "  -?, --help                 Display this help and exit\n"
           "\n"
           "Developer Options:\n"
           "      --test                 Start with a test-scenario\n"
           "      --seed NUMBER          Seed the random number generator with NUMBER\n"
           "      --stress-test          Non-interactive stress test\n"
           "      --robots               Non-interactive network stress test\n"
           "\n"
           "FILE can be a saved game file (.sav), or a map file (.map).\n"
           "Saved games are stored in: %3\n"
           "\n"
           "Commands:\n"
           "  game-host-server           Run a game host server\n"
           "  game-list-server           Run a game list server\n"
           "  ghsctl                     Control a running game host server\n"
           "  glsctl                     Control a running game list server\n"
           "  upgrade-file               Migrate files to this version\n"
           "  import                     Import a Warlords II scenario\n"
           "\n"
           "Report bugs to %4.\n"),
        Lw::get_prgname (),
        "~/.config/" PACKAGE "/" DEFAULT_CONFIG_FILENAME,
        "~/.local/share/" PACKAGE,
        PACKAGE_BUGREPORT);
      exit (0);
    }
  else if (parameter == "--version")
    {
      cout << PACKAGE_STRING << endl << endl;
      cout <<
        _("License GPLv3+: GNU GPL version 3 or later <https://gnu.org/licenses/gpl.html>.\n" \
          "This is free software: you are free to change and redistribute it.\n" \
          "There is NO WARRANTY, to the extent permitted by law.\n");
        exit (0);
    }
  else
    {
      if (parameter.size () >= 2 && parameter.substr (0, 2) == "--")
        {
          std::cerr <<
            String::ucompose (_("%2: Unrecognized option `%1'.\n"
                                "Try `%2 --help' for more information."),
                              parameter, Lw::get_prgname ()) << std::endl;
          exit (EXIT_FAILURE);
        }
      else if (!File::exists (parameter))
        {
          std::cerr <<
            String::ucompose (_("%2: Couldn't open `%1' for reading."),
                              parameter, Lw::get_prgname ()) << std::endl;
          exit (EXIT_FAILURE);
        }
      m_load_filename = parameter;
    }
  validate_option_combinations ();
}
    
void Lw::validate_option_combinations ()
{
  // --view-stress-test needs --stress-test
  if (m_view_stress_test && !m_start_stress_test)
    {
      std::cerr <<
        String::ucompose (_("%1: --view-stress-test requires --stress-test."),
                          Lw::get_prgname ()) << std::endl;
      exit (EXIT_FAILURE);
    }

  // --port needs --host
  if (m_port && !m_start_headless_server)
    {
      std::cerr <<
        String::ucompose (_("%1: --port requires --host."),
                          Lw::get_prgname ()) << std::endl;
      exit (EXIT_FAILURE);
    }

  // --stress-test can't be used with --test
  if (m_start_stress_test && m_start_test_scenario)
    {
      std::cerr <<
        String::ucompose (_("%1: --stress-test and --test cannot be used together."),
                          Lw::get_prgname ()) << std::endl;
      exit (EXIT_FAILURE);
    }

  // --stress-test can't be used with --editor
  if (m_start_stress_test && m_start_editor)
    {
      std::cerr <<
        String::ucompose (_("%1: --stress-test and --editor cannot be used together."),
                          Lw::get_prgname ()) << std::endl;
      exit (EXIT_FAILURE);
    }

  // --stress-test can't be used with --robots
  if (m_start_stress_test && m_start_robots)
    {
      std::cerr <<
        String::ucompose (_("%1: --stress-test and --robots cannot be used together."),
                          Lw::get_prgname ()) << std::endl;
      exit (EXIT_FAILURE);
    }

  // --test can't be used with --editor
  if (m_start_test_scenario && m_start_editor)
    {
      std::cerr <<
        String::ucompose (_("%1: --test and --editor cannot be used together."),
                          Lw::get_prgname ()) << std::endl;
      exit (EXIT_FAILURE);
    }

  // --test can't be used with --robots
  if (m_start_test_scenario && m_start_robots)
    {
      std::cerr <<
        String::ucompose (_("%1: --test and --robots cannot be used together."),
                          Lw::get_prgname ()) << std::endl;
      exit (EXIT_FAILURE);
    }
}

void Lw::process_game_host_server_options (int argc, char **argv)
{
  for (int i = 2; i <= argc; i++)
    {
      Glib::ustring parameter(argv[i-1]); 
      if (parameter == "--port" || parameter == "-p")
        {
          i++;
          //convert the next argument
          char* error = 0;
          long userport = strtol (argv[i-1], &error, 10);
          if (error && (*error != '\0'))
            {
              std::cerr << _("non-numerical value for --port") << std::endl;
              exit (1);
            }
          if (userport > 65535 || userport < 1000)
            {
              std::cerr << _("invalid value for --port") << std::endl;
              exit (1);
            }
          m_ghs_port = userport;
        }
      else if (parameter == "--host" || parameter == "-h")
        {
          m_ghs_hostname = parameter;
        }
      else if (parameter == "--foreground" || parameter == "-f")
        {
          m_ghs_foreground = true;
        }
      else if (parameter == "--members" || parameter == "-m")
        {
          m_ghs_members = GamehostServer::load_members_from_file (parameter);
        }
      else if (parameter == "--help" || parameter == "-?")
        {
          cout <<
            String::ucompose
            (_("Usage: %1 [OPTION]...\n"
               "\n"
               "Run a server that can start up games.\n"
               "\n"
               "Options:\n"
               "  -f, --foreground           Do not detach from the controlling terminal\n"
               "  -h, --host <hostname>      Advertise the HOSTNAME as this to game clients\n"
               "  -p, --port <number>        Start the server on the given port\n"
               "  -m, --members <file>       Allow the profile ids in FILE to host games\n"
               "  -?, --help                 Display this help and exit\n"
               "\n"
               "Report bugs to %2.\n"),
             Lw::get_prgname (),
             PACKAGE_BUGREPORT);
          exit (0);
        }
    }
}

void Lw::process_game_list_server_options (int argc, char **argv)
{
  for (int i = 2; i <= argc; i++)
    {
      Glib::ustring parameter(argv[i-1]); 
      if (parameter == "--port" || parameter == "-p")
        {
          i++;
          //convert the next argument
          char* error = 0;
          long userport = strtol (argv[i-1], &error, 10);
          if (error && (*error != '\0'))
            {
              std::cerr << _("non-numerical value for --port") << std::endl;
              exit (1);
            }
          if (userport > 65535 || userport < 1000)
            {
              std::cerr << _("invalid value for --port") << std::endl;
              exit (1);
            }
          m_gls_port = userport;
        }
      else if (parameter == "--foreground" || parameter == "-f")
        {
          m_gls_foreground = true;
        }
      else if (parameter == "--help" || parameter == "-?")
        {
          cout <<
            String::ucompose
            (_("Usage: %1 [OPTION]...\n"
               "Run a server that advertises games that others can connect to.\n"
               "\n"
               "Options:\n"
               "  -f, --foreground           Do not detach from the controlling terminal\n"
               "  -p, --port <number>        Start the server on the given port\n"
               "  -?, --help                 Display this help and exit\n"
               "\n"
               "Report bugs to %2.\n"),
             Lw::get_prgname (),
             PACKAGE_BUGREPORT);
          exit (0);
        }
    }
}

void Lw::process_game_list_client_options (int argc, char **argv)
{
  for (int i = 2; i <= argc; i++)
    {
      Glib::ustring parameter(argv[i-1]); 
      if (parameter == "--port" || parameter == "-p")
        {
          i++;
          //convert the next argument
          char* error = 0;
          long userport = strtol (argv[i-1], &error, 10);
          if (error && (*error != '\0'))
            {
              std::cerr << _("non-numerical value for --port") << std::endl;
              exit (1);
            }
          if (userport > 65535 || userport < 1000)
            {
              std::cerr << _("invalid value for --port") << std::endl;
              exit (1);
            }
          m_glc_port = userport;
        }
      else if (parameter == "--profile" || parameter == "-P")
        {
          m_glc_profile = Profilelist::instance ()->findProfileById (parameter);
          if (!m_glc_profile)
            {
              std::cerr << _("invalid profile id") << std::endl;
              exit (1);
            }
        }
      else if (parameter == "--unadvertise" || parameter == "-u")
        {
          m_glc_unadvertise.push_back (parameter);
        }
      else if (parameter == "--advertise" || parameter == "-a")
        {
          m_glc_advertise = true;
        }
      else if (parameter == "--list" || parameter == "-l")
        {
          m_glc_show_list = true;
        }
      else if (parameter == "--reload" || parameter == "-R")
        {
          m_glc_reload = true;
        }
      else if (parameter == "--remove-all" || parameter == "-r")
        {
          m_glc_remove_all = argv[i-1];
        }
      else if (parameter == "--terminate" || parameter == "-t")
        {
          m_glc_terminate = true;
        }
      else if (parameter == "--help" || parameter == "-?")
        {
          cout <<
            String::ucompose
            (_("Usage: %1 [OPTION]... [HOST]\n"
               "\n"
               "Send commands to a game list server.\n"
               "\n"
               "Options:\n"
               "  -P, --profile <id>         Use the identity specified by ID\n"
               "  -p, --port <number>        Connect to the server on the given port\n"
               "  -u, --unadvertise <id>     Remove a game specified by scenario ID\n"
               "  -a, --advertise            Add a game to the list\n"
               "  -l, --list                 See a list of games\n"
               "  -R, --reload               Reload the game list from disk\n"
               "  -r, --remove-all <id>      Remove all games started by the given profile id\n"
               "  -t, --terminate            Stop the server\n"
               "  -?, --help                 Display this help and exit\n"
               "\n"
               "Specifying a profile id of -1 to the --remove-all option will remove all games\n"
               "from the game list.\n"
               "\n"
               "If HOST is not specified it defaults to 127.0.0.1.\n"
               "\n"
               "Report bugs to %2.\n"),
            Lw::get_prgname (),
            PACKAGE_BUGREPORT);
          exit (0);
        }
      else
        m_glc_host = parameter;
    }
}

void Lw::process_game_host_client_options (int argc, char **argv)
{
  for (int i = 2; i <= argc; i++)
    {
      Glib::ustring parameter(argv[i-1]); 
      if (parameter == "--port" || parameter == "-p")
        {
          i++;
          //convert the next argument
          char* error = 0;
          long userport = strtol (argv[i-1], &error, 10);
          if (error && (*error != '\0'))
            {
              std::cerr << _("non-numerical value for --port") << std::endl;
              exit (1);
            }
          if (userport > 65535 || userport < 1000)
            {
              std::cerr << _("invalid value for --port") <<std::endl;
              exit (1);
            }
          m_ghc_port = userport;
        }
      else if (parameter == "--profile" || parameter == "-P")
        {
          m_ghc_profile = Profilelist::instance ()->findProfileById (parameter);
          if (!m_ghc_profile)
            {
              std::cerr << _("invalid profile id") << std::endl;
              exit (1);
            }
        }
      else if (parameter == "--list" || parameter == "-l")
        {
          m_ghc_show_list = true;
        }
      else if (parameter == "--reload" || parameter == "-R")
        {
          m_ghc_reload = true;
        }
      else if (parameter == "--host" || parameter == "-h")
        {
          m_ghc_file = parameter;
        }
      else if (parameter == "--unhost" || parameter == "-u")
        {
          m_ghc_unhost = parameter;
        }
      else if (parameter == "--terminate" || parameter == "-t")
        {
          m_ghc_terminate = true;
        }
      else if (parameter == "--help" || parameter == "-?")
        {
          cout <<
            String::ucompose
            (_("Usage: %1 [OPTION]... [HOST]\n"
               "\n"
               "Send commands to a game host server.\n"
               "\n"
               "Options:\n"
               "  -P, --profile <id>         Use this identity, specified by profile id\n"
               "  -p, --port <number>        Connect to the server on the given port\n"
               "  -l, --list                 See a list of hosted games\n"
               "  -R, --reload               Reload the game list from disk\n"
               "  -u, --unhost <id>          Stop hosting a game (specified by scenario id)\n"
               "  -h, --host <file>          Host a new game\n"
               "  -t, --terminate            Stop the server\n"
               "  -?, --help                 Display this help and exit\n"
               "\n"
               "If HOST is not specified it defaults to 127.0.0.1.\n"
               "\n"
               "Report bugs to %2.\n"),
             Lw::get_prgname (),
             PACKAGE_BUGREPORT);
          exit (0);
        }
      else
        m_ghc_host = parameter;
    }
}

void Lw::process_upgrade_tool_options (int argc, char **argv)
{
  for (int i = 2; i <= argc; i++)
    {
      Glib::ustring parameter(argv[i-1]); 
      if (parameter == "--identify" || parameter == "-i")
        {
          m_upgrade_identify_file = true;
        }
      else if (parameter == "--rewrite" || parameter == "-r")
        {
          m_upgrade_rewrite = parameter;
        }
      else if (parameter == "--help" || parameter == "-?")
        {
          cout <<
            String::ucompose
            (_("Usage: %1 [OPTION]... FILE\n"
               "\n"
               "Convert old versions of lordsawar files to the latest version.\n"
               "\n"
               "Options:\n"
               "  -i, --identify             Display file type and exit instead of upgrading\n"
               "  -?, --help                 Display this help and exit\n"
               "\n"
               "The following kinds of files are supported:\n"
               "Configuration, armyset, cityset, tileset, shieldset, saved game files, \n"
               "map files, item list, game list, profile list, and recently played game list.\n"
               "\n"
               "Report bugs to %2.\n"),
             Lw::get_prgname (),
             PACKAGE_BUGREPORT);
          exit (0);
        }
      else
        m_upgrade_filename = parameter;

    }
  if (m_upgrade_filename == "")
    {
      std::cerr <<
        String::ucompose (_("%1: Too few arguments.\n"
                            "Try `%1 --help' for more information."),
                          Lw::get_prgname ()) << std::endl;
      exit (EXIT_FAILURE);
    }
}

void Lw::process_import_tool_options (int argc, char **argv)
{
  for (int i = 2; i <= argc; i++)
    {
      Glib::ustring parameter(argv[i-1]); 
      if (parameter == "--help" || parameter == "-?")
        {
          cout <<
            String::ucompose
            (_("Usage: %1 [OPTION]... FILE\n"
               "\n"
               "Convert scenarios from Warlords II into lordsawar scenarios.\n"
               "\n"
               "Options:\n"
               "  -a, --army-file FILE       Use this WL2 army file\n"
               "  -?, --help                 Display this help and exit\n"
               "\n"
               "Report bugs to %2.\n"),
             Lw::get_prgname (),
             PACKAGE_BUGREPORT);
          exit (0);
        }
      else if (parameter == "--army-file" || parameter == "-a")
        {
          i++;
          m_import_armyset_filename = argv[i-1];
          if (!File::exists (m_import_armyset_filename))
            {
              std::cerr <<
                String::ucompose (_("%2: Couldn't open `%1' for reading."),
                                  m_import_armyset_filename,
                                  Lw::get_prgname ()) << std::endl;
              exit (EXIT_FAILURE);
            }
        }
      else
        m_import_filename = parameter;
    }
  if (m_import_filename == "")
    {
      std::cerr <<
        String::ucompose (_("%1: Too few arguments.\n"
                            "Try `%1 --help' for more information."),
                          Lw::get_prgname ()) << std::endl;
      exit (EXIT_FAILURE);
    }
}

void Lw::cleanup ()
{
  Armysetlist::deleteInstance ();
  Tilesetlist::deleteInstance ();
  Citysetlist::deleteInstance ();
  Shieldsetlist::deleteInstance ();
  ImageCache::deleteInstance ();
  HeroTemplates::deleteInstance ();
}

void Lw::quit_loop ()
{
  Lw::loop->quit ();
  Lw::app->release ();
}

bool Lw::get_dark ()
{
  auto settings = Gio::Settings::create ("org.gnome.desktop.interface", "/org/gnome/desktop/interface/");
  auto scheme = settings->get_string ("color-scheme");
  return scheme == "prefer-dark";
}

void Lw::process_cityset_editor_options (int argc, char **argv)
{
  for (int i = 2; i <= argc; i++)
    {
      Glib::ustring parameter(argv[i-1]); 
      if (parameter == "--help" || parameter == "-?")
        {
          cout <<
            String::ucompose
            (_("Usage: %1 [OPTION]... [FILE]\n"
               "\n"
               "View and modify cityset (%3) files.\n"
               "\n"
               "Options:\n"
               "  -?, --help                 Display this help and exit\n"
               "\n"
               "Report bugs to %2.\n"),
             Lw::get_prgname (),
             PACKAGE_BUGREPORT, CITYSET_EXT);
          exit (0);
        }
      else
        m_load_filename = parameter;
    }
}

void Lw::process_armyset_editor_options (int argc, char **argv)
{
  for (int i = 2; i <= argc; i++)
    {
      Glib::ustring parameter(argv[i-1]); 
      if (parameter == "--help" || parameter == "-?")
        {
          cout <<
            String::ucompose
            (_("Usage: %1 [OPTION]... [FILE]\n"
               "\n"
               "View and modify armyset (%3) files.\n"
               "\n"
               "Options:\n"
               "  -s, --shieldset BNAME      The basename of a shieldset\n"
               "  -?, --help                 Display this help and exit\n"
               "\n"
               "Shieldsets hold the colors to paint the armies in.\n"
               "\n"
               "Report bugs to %2.\n"),
             Lw::get_prgname (),
             PACKAGE_BUGREPORT, ARMYSET_EXT);
          exit (0);
        }
      else if (parameter == "--shieldset" || parameter == "-s")
        {
          i++;
          m_shieldset_theme = argv[i-1];
        }
      else
        m_load_filename = parameter;
    }
}

void Lw::process_tileset_editor_options (int argc, char **argv)
{
  for (int i = 2; i <= argc; i++)
    {
      Glib::ustring parameter(argv[i-1]); 
      if (parameter == "--help" || parameter == "-?")
        {
          cout <<
            String::ucompose
            (_("Usage: %1 [OPTION]... [FILE]\n"
               "\n"
               "View and modify tileset (%3) files.\n"
               "\n"
               "Options:\n"
               "  -s, --shieldset BNAME      The basename of a shieldset\n"
               "  -?, --help                 Display this help and exit\n"
               "\n"
               "Shieldsets hold the colors to paint the selectors and flags in.\n"
               "\n"
               "Report bugs to %2.\n"),
             Lw::get_prgname (),
             PACKAGE_BUGREPORT, TILESET_EXT);
          exit (0);
        }
      else if (parameter == "--shieldset" || parameter == "-s")
        {
          i++;
          m_shieldset_theme = argv[i-1];
        }
      else
        m_load_filename = parameter;
    }
}

void Lw::process_shieldset_editor_options (int argc, char **argv)
{
  for (int i = 2; i <= argc; i++)
    {
      Glib::ustring parameter(argv[i-1]); 
      if (parameter == "--help" || parameter == "-?")
        {
          cout <<
            String::ucompose
            (_("Usage: %1 [OPTION]... [FILE]\n"
               "\n"
               "View and modify shieldset (%3) files.\n"
               "\n"
               "Options:\n"
               "  -?, --help                 Display this help and exit\n"
               "\n"
               "Report bugs to %2.\n"),
             Lw::get_prgname (),
             PACKAGE_BUGREPORT, SHIELDSET_EXT);
          exit (0);
        }
      else
        m_load_filename = parameter;
    }
}

void Lw::process_editor_options (int argc, char **argv)
{
  for (int i = 2; i <= argc; i++)
    {
      Glib::ustring parameter(argv[i-1]); 
      if (parameter == "--help" || parameter == "-?")
        {
          cout <<
            String::ucompose
            (_("Usage: %1 [OPTION]... [FILE]\n"
               "\n"
               "View and modify scenario (%3) or saved-game (%4) files.\n"
               "\n"
               "Options:\n"
               "  -?, --help                 Display this help and exit\n"
               "\n"
               "Report bugs to %2.\n"),
             Lw::get_prgname (),
             PACKAGE_BUGREPORT, MAP_EXT, SAVE_EXT);
          exit (0);
        }
      else
        m_load_filename = parameter;
    }
}
