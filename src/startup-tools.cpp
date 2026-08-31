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

#include <gtkmm.h>
#include "lw.h"
#include "game-list.h"
#include "game-list-server.h"
#include "file-compat.h"
#include "game-host-client-tool.h"
#include "game-list-client-tool.h"
#include "import.h"

int StartupTools::game_host_server (std::string hostname, int port,
                                    bool foreground,
                                    std::list<Glib::ustring> members)
{
  Lw::loop = Glib::MainLoop::create ();
  initialize_configuration ();
  Profilelist::support_backward_compatibility ();
  Gamelist::support_backward_compatibility ();
  FileCompat::instance ()->initialize ();

  Glib::ustring program = Glib::find_program_in_path (PACKAGE);
  if (program == "")
    {
      std::cerr <<
        String::ucompose (_("%2: could not find `%1' executable in path."),
                          PACKAGE, Glib::get_prgname ()) << std::endl;
      Lw::app->quit ();
    }

  if (foreground == false)
    {
#ifndef __MINGW32__
      if (daemon (0, 0) == -1)
        std::cerr <<
          String::ucompose (_("%1: Could not detach from controlling terminal."),
                            Lw::get_prgname ()) << std::endl;
#endif
    }

  Lw::loop = Glib::MainLoop::create ();
  GamehostServer *gamehostserver = GamehostServer::instance ();
  if (port == 0)
    port = LORDSAWAR_GAMEHOST_PORT;
  if (hostname == "")
    hostname = Configuration::s_gamehost_server_hostname;
  gamehostserver->setHostname (hostname);
  gamehostserver->setMembers (members);
  gamehostserver->start (port);
  Lw::app->hold ();
  Lw::loop->run ();
  return 0;
}

int StartupTools::game_list_server (int port, bool foreground)
{
  Lw::loop = Glib::MainLoop::create ();
  initialize_configuration ();
  Profilelist::support_backward_compatibility ();
  Gamelist::support_backward_compatibility ();
  FileCompat::instance ()->initialize ();

  if (foreground == false)
    {
#ifndef __MINGW32__
      if (daemon (0, 0) == -1)
        std::cerr <<
          String::ucompose
          (_("%1: Could not detach from controlling terminal."),
           Lw::get_prgname ()) << std::endl;
#endif
    }

  Lw::loop = Glib::MainLoop::create ();

  GamelistServer *gamelistserver = GamelistServer::instance ();
  if (port == 0)
    port = LORDSAWAR_GAMELIST_PORT;

  if (foreground == false)
    {
#ifndef __MINGW32__
      if (daemon (0, 0) == -1)
        std::cerr <<
          String::ucompose
          (_("%1: Could not detach from controlling terminal."),
           Lw::get_prgname ()) << std::endl;
#endif
    }

  gamelistserver->start (port);

  Lw::app->hold ();
  Lw::loop->run ();
  return 0;
}

int StartupTools::game_list_client (Profile *profile, Glib::ustring host,
                                    bool advertise, bool show_list, bool reload,
                                    bool terminate, int port,
                                    std::list<Glib::ustring> unadvertise,
                                    Glib::ustring remove_all)
{
  Lw::loop = Glib::MainLoop::create ();
  initialize_configuration ();

  if (port == 0)
    port = LORDSAWAR_GAMELIST_PORT;

  if (!show_list && unadvertise.empty () == true && !advertise && !reload &&
      remove_all.empty () == true)
    {
      std::cerr <<
        String::ucompose (_("%1: Try `%1 --help' for more information."),
                          Lw::get_prgname ()) << std::endl;
      return 1;
    }
  if (host == "")
    host = "127.0.0.1";
  GlsClientTool tool (host, port, profile, show_list, unadvertise,
                      advertise, reload, remove_all, terminate);
  Lw::app->hold ();
  Lw::loop->run ();
  return 0;
}

int StartupTools::game_host_client (Profile *profile, Glib::ustring host,
                                    Glib::ustring file, Glib::ustring unhost,
                                    bool show_list, bool reload,
                                    bool terminate, int port)
{
  Lw::loop = Glib::MainLoop::create ();
  initialize_configuration ();

  if (port == 0)
    port = LORDSAWAR_GAMEHOST_PORT;

  if (!show_list && !reload && unhost.empty () && file.empty ())
    {
      std::cerr <<
        String::ucompose (_("%1: Try `%1 --help' for more information."),
                          Lw::get_prgname ()) << std::endl;
      return 1;
    }
  if (host == "")
    host = "127.0.0.1";
  GhsClientTool tool (host, port, profile, show_list, reload, unhost, file,
                      terminate);
  Lw::app->hold ();
  Lw::loop->run ();
  return 0;
}

int StartupTools::upgrade_file (Glib::ustring filename, Glib::ustring rewrite,
                                bool identify_file)
{
  bool same_version = false;
  int err = EXIT_SUCCESS;
  Armyset::support_backward_compatibility ();
  Shieldset::support_backward_compatibility ();
  Tileset::support_backward_compatibility ();
  Profilelist::support_backward_compatibility ();
  RecentlyPlayedGameList::support_backward_compatibility ();
  Gamelist::support_backward_compatibility ();
  FileCompat::support_backward_compatibility_for_common_files ();
  if (identify_file == false && rewrite == "")
    {
      Glib::ustring ext = File::get_extension (filename);
      Glib::ustring tmpfile = File::get_tmp_file (ext);
      File::copy (filename, tmpfile);
      bool upgraded = FileCompat::instance ()->upgrade (tmpfile, same_version);

      if (same_version)
        {
          std::cout <<
            String::ucompose (_("%1: %2 is already the latest version."),
                              Lw::get_prgname (), filename) << std::endl;

          bool is_tar_file = false;
          FileCompat::Type type =
            FileCompat::instance ()->getTypeByFileInspection (tmpfile,
                                                              is_tar_file);
          if (type == FileCompat::GAMESCENARIO && is_tar_file)
            {
              bool armyset = false, tileset = false, cityset = false,
                   shieldset = false;
              std::cout <<
                String::ucompose
                (_("%1: Trying to upgrade the other files inside the tar file..."),
                 Lw::get_prgname ()) << std::endl;
              upgraded = FileCompat::instance ()->upgradeGameScenario
                (tmpfile, LORDSAWAR_SAVEGAME_VERSION,
                 armyset, tileset, cityset, shieldset);
              if (upgraded)
                {
                  if (armyset)
                    std::cout << _("Armyset has been upgraded.") << std::endl;
                  if (tileset)
                    std::cout << _("Tileset has been upgraded.") << std::endl;
                  if (cityset)
                    std::cout << _("Cityset has been upgraded.") << std::endl;
                  if (shieldset)
                    std::cout << _("Shieldset has been upgraded.") << std::endl;
                  if (armyset || tileset || cityset || shieldset)
                    File::copy (tmpfile, filename);
                  if (!armyset && !tileset && !cityset && !shieldset)
                    std::cout <<
                     String::ucompose
                     (_("%1: None of the other files needed to be upgraded."),
                      Lw::get_prgname ()) << std::endl;
                }
            }
          File::erase (tmpfile);
        }
      else if (!upgraded && !same_version)
        {
          std::cerr <<
            String::ucompose (_("%2: `%1' could not be upgraded."),
                              filename, Lw::get_prgname ()) << std::endl;
          File::erase (tmpfile);
        }
      else
        {
          File::copy (tmpfile, filename);
          File::erase (tmpfile);
        }
      if (!upgraded)
        err = EXIT_FAILURE;
    }
  else if (identify_file && rewrite == "")
    {
      Glib::ustring tag, version;
      FileCompat::Type type = FileCompat::instance ()->getType (filename);
      FileCompat::instance ()->get_tag_and_version_from_file (filename, type,
                                                              tag, version);
      std::cout <<
        String::ucompose ("%1: %2 (%3 %4)", Lw::get_prgname (),
                          FileCompat::typeToString (type), tag,
                          version) << std::endl;
    }
  else if (identify_file == false && rewrite != "")
    {
      FileCompat *fc = FileCompat::instance ();
      Glib::ustring tag, version;
      FileCompat::Type type = fc->getType (filename);
      if (fc->get_tag_and_version_from_file (filename, type, tag, version))
        {
          if (!fc->rewrite_with_updated_version (filename, type, tag, rewrite))
            err = EXIT_FAILURE;
        }
      else
        err = EXIT_FAILURE;
    }
  else if (identify_file && rewrite != "")
    {
      std::cerr <<
        String::ucompose
        (_("%1: The --identify and --rewrite options cannot be used at the same time."),
         Lw::get_prgname ()) << std::endl;
      err = EXIT_FAILURE;
    }
  return err;
}

int StartupTools::upgrade_tool (bool identify_file, std::string rewrite,
                                std::string filename)
{
  int err;
  Lw::loop = Glib::MainLoop::create ();
  Glib::signal_idle ().connect
    ([this, identify_file, rewrite, filename, &err]()
     {
       err = upgrade_file (filename, rewrite, identify_file);
       Lw::quit_loop ();
       return false;
     });
  Lw::app->hold ();
  Lw::loop->run ();
  return err;
}

int StartupTools::import_tool (std::string armyset_file, std::string filename)
{
  int err = 0;
  Lw::loop = Glib::MainLoop::create ();
  Glib::signal_idle ().connect
    ([this, armyset_file, filename, &err]()
     {
       err = import_file (armyset_file, filename);
       Lw::quit_loop ();
       return false;
     });
  Lw::app->hold ();
  Lw::loop->run ();
  return err;
}
