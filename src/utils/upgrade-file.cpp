// Copyright (C) 2011, 2014, 2021 Ben Asselstine
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
//  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 
//  02110-1301, USA.

#include <config.h>

#include <iostream>
#include "xmlhelper.h"
#include "tarhelper.h"
#include "File.h"
#include "profilelist.h"
#include "recently-played-game-list.h"
#include "gamelist.h"
#include "ucompose.hpp"
#include "file-compat.h"
#include "armyset.h"
#include "shieldset.h"
#include "tileset.h"

int max_vector_width;

void usage (char *progname)
{
  std::cout << File::get_basename(progname, true) << " [OPTION]... FILE" << std::endl << std::endl;
  std::cout << "LordsAWar! File Upgrading Tool " << _("version") << 
    " " << VERSION << std::endl << std::endl;
  std::cout << _("Options:") << std::endl << std::endl; 
  std::cout << "  -?, --help                 " << _("Display this help and exit") <<std::endl;
  std::cout << "  -i, --identify             " << _("Show the file type instead of upgrading") << std::endl;
  //std::cout << "  -r, --rewrite VERSION      " << _("Just change the version instead of upgrading") << std::endl;
  std::cout << std::endl;
  std::cout << _("Report bugs to") << " <" << PACKAGE_BUGREPORT ">." << std::endl;
  exit(0);
}

void kickoff (Glib::ustring filename, Glib::ustring rewrite, bool identify_file)
{
  int err = EXIT_SUCCESS;
  bool same_version = false;
  Armyset::support_backward_compatibility();
  Shieldset::support_backward_compatibility();
  Tileset::support_backward_compatibility();
  Profilelist::support_backward_compatibility();
  RecentlyPlayedGameList::support_backward_compatibility();
  Gamelist::support_backward_compatibility();
  FileCompat::support_backward_compatibility_for_common_files();
  if (identify_file == false && rewrite == "")
    {
      Glib::ustring ext = File::get_extension(filename);
      Glib::ustring tmpfile = File::get_tmp_file(ext);
      File::copy(filename, tmpfile);
      bool upgraded = FileCompat::getInstance()->upgrade(tmpfile, 
                                                         same_version);

      if (same_version)
        {
          std::cout << String::ucompose(_("%1 is already the latest version."), 
                                        filename) << std::endl;

          bool is_tar_file = false;
          FileCompat::Type type = 
            FileCompat::getInstance()->getTypeByFileInspection(tmpfile, 
                                                               is_tar_file);
          if (type == FileCompat::GAMESCENARIO && is_tar_file)
            {
              bool armyset = false, tileset = false, cityset = false, 
                   shieldset = false;
              std::cout << _("Trying to upgrade the other files inside the tar file...") << std::endl;
              upgraded = FileCompat::getInstance()->upgradeGameScenario
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
                    File::copy(tmpfile, filename);
                  if (!armyset && !tileset && !cityset && !shieldset)
                    std::cout << _("None of the other files needed to be upgraded.") << std::endl;
                }
            }
          File::erase(tmpfile);
        }
      else if (!upgraded && !same_version)
        {
          std::cout << String::ucompose(_("Error: %1 could not be upgraded."), 
                                        filename) << std::endl;
          File::erase(tmpfile);
        }
      else
        {
          File::copy(tmpfile, filename);
          File::erase(tmpfile);
        }
      if (!upgraded)
        err = EXIT_FAILURE;
    }
  else if (identify_file && rewrite == "")
    {
      Glib::ustring tag, version;
      FileCompat::Type type = FileCompat::getInstance()->getType(filename);
      FileCompat::getInstance()->get_tag_and_version_from_file(filename, type, tag, version);
      std::cout << String::ucompose("%1 (%2 %3)", 
                                    FileCompat::typeToString(type), 
                                    tag, version) << std::endl;
    }
  else if (identify_file == false && rewrite != "")
    {
      FileCompat *fc = FileCompat::getInstance();
      Glib::ustring tag, version;
      FileCompat::Type type = fc->getType(filename);
      if (fc->get_tag_and_version_from_file(filename, type, tag, version))
        {
          if (!fc->rewrite_with_updated_version(filename, type, tag, rewrite))
            err = EXIT_FAILURE;
        }
      else
        err = EXIT_FAILURE;
    }
  else if (identify_file && rewrite != "")
    {
      std::cout << _("Error: The --identify and --rewrite options cannot be used at the same time.") << std::endl;
      err = EXIT_FAILURE;
    }
  exit (err);
}

int main(int argc, char* argv[])
{
  Glib::ustring filename = "";
  Glib::ustring rewrite;
  bool identify_file = false;
  Vector<int>::setMaximumWidth(1000);

  if (argc > 1)
    {
      for (int i = 2; i <= argc; i++)
	{
          Glib::ustring parameter(argv[i-1]); 
	  if (parameter == "--identify" || parameter == "-i")
            {
              identify_file = true;
            }
          else if (parameter == "--rewrite" || parameter == "-r")
            {
              rewrite = parameter;
            }
	  else if (parameter == "--help" || parameter == "-?")
            usage (argv[0]);
	  else
	    filename = parameter;
	}
    }

  if (filename == "")
    usage (argv[0]);

  auto app = Gtk::Application::create ("org.nongnu.lordsawar.upgrade-file");

  app->signal_startup ().connect (sigc::bind (sigc::ptr_fun (&kickoff), filename, rewrite, identify_file));
  app->run();
  app->quit ();
}

