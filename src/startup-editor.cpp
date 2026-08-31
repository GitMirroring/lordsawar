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
#include "startup-editor.h"

#include "city-set-list.h"
#include "tile-set-list.h"
#include "army-set-list.h"
#include "shield-set-list.h"
#include "create-scenario-randomize.h"

#include "cityset-window.h"
#include "shieldset-window.h"
#include "armyset-window.h"
#include "tileset-window.h"
#include "scenario-builder-window.h"
#include "editor-splash-window.h"

void StartupEditor::cityset_editor (std::string filename)
{
  Startup::instance ()->initialize ();
  if (filename != "")
    {
      if (!File::exists (filename))
        {
          std::cerr <<
            String::ucompose
            (_("%1: `%2' cannot be opened for reading."),
             Lw::get_prgname (), filename) << std::endl;
          Lw::app->quit ();
          return;
        }
      else if (!File::nameEndsWith (filename, CITYSET_EXT))
        {
          std::cerr <<
            String::ucompose
            (_("%1: `%2' isn't a cityset file."),
             Lw::get_prgname (), filename) << std::endl;
          Lw::app->quit ();
          return;
        }
      else
        {
          Cityset::create
            (filename,
             [this, filename] (Cityset *cityset, bool, bool, Glib::ustring)
             {
               if (cityset)
                 cityset_editor (cityset);
               else
                 {
                   std::cerr <<
                     String::ucompose
                     (_("%1: `%2' couldn't be loaded."),
                      Lw::get_prgname (), filename) << std::endl;
                   Lw::app->quit ();
                 }
             });
        }
    }
  else
    cityset_editor (Citysetlist::instance ()->get ("default"));
}

void StartupEditor::cityset_editor (Cityset *cityset)
{
  auto w = new CitySetWindow ();
  w->setup (cityset);
  Lw::app->add_window (*w);
  w->signal_closed ().connect
    ([this, w] ()
     {
       delete w;
     });
  w->present ();
}

void StartupEditor::shieldset_editor (std::string filename)
{
  Startup::instance ()->initialize ();
  if (filename != "")
    {
      if (!File::exists (filename))
        {
          std::cerr <<
            String::ucompose (_("%1: `%2' cannot be opened for reading."),
                              Lw::get_prgname (), filename) << std::endl;
          Lw::app->quit ();
          return;
        }
      else if (!File::nameEndsWith (filename, SHIELDSET_EXT))
        {
          std::cerr <<
            String::ucompose (_("%1: `%2' isn't a shieldset file."),
                              Lw::get_prgname (), filename) <<
            std::endl;
          Lw::app->quit ();
          return;
        }
      else
        {
          Shieldset::create
            (filename,
             [this, filename] (Shieldset *shieldset, bool, bool, Glib::ustring)
             {
               if (shieldset)
                 shieldset_editor (shieldset);
               else
                 {
                   std::cerr <<
                     String::ucompose (_("%1: `%2' couldn't be loaded."),
                                       Lw::get_prgname (), filename) <<
                     std::endl;
                   Lw::app->quit ();
                 }
             });
        }
    }
  else
    shieldset_editor (Shieldsetlist::instance ()->get ("default"));
}

void StartupEditor::shieldset_editor (Shieldset *shieldset)
{
  auto w = new ShieldSetWindow ();
  w->setup (shieldset);
  Lw::app->add_window (*w);
  w->signal_closed ().connect
    ([this, w] ()
     {
       delete w;
     });
  w->present ();

}

void StartupEditor::armyset_editor (std::string shieldset_theme, std::string filename)
{
  Startup::instance ()->initialize ();
  if (shieldset_theme == "")
    shieldset_theme = "default";
  if (filename != "")
    {
      if (!File::exists (filename))
        {
          std::cerr <<
            String::ucompose (_("%1: `%2' cannot be opened for reading."),
                              Lw::get_prgname (), filename) << std::endl;
          Lw::app->quit ();
          return;
        }
      else if (!File::nameEndsWith (filename, ARMYSET_EXT))
        {
          std::cerr <<
            String::ucompose (_("%1: `%2' isn't an armyset file."),
                              Lw::get_prgname (), filename) << std::endl;
          Lw::app->quit ();
          return;
        }
      else
        {
          Armyset::create
            (filename,
             [this, filename, shieldset_theme] (Armyset *armyset, bool, bool,
                                                Glib::ustring)
             {
               if (armyset)
                 {
                   auto shieldset =
                     load_shieldset_from_option (shieldset_theme);
                   armyset_editor (shieldset, armyset);
                 }
               else
                 {
                   std::cerr <<
                     String::ucompose
                     (_("%1: `%2' couldn't be loaded."),
                      Lw::get_prgname (), filename) << std::endl;
                   Lw::app->quit ();
                 }
             });
        }
    }
  else
    {
      auto shieldset = load_shieldset_from_option (shieldset_theme);
      auto armyset = Armysetlist::instance ()->get ("default");
      armyset_editor (shieldset, armyset);
    }
}

void StartupEditor::armyset_editor (Shieldset *shieldset, Armyset *armyset)
{
  auto w = new ArmySetWindow ();
  w->setup (shieldset, armyset);
  Lw::app->add_window (*w);
  w->signal_closed ().connect
    ([this, w] ()
     {
       delete w;
     });
  w->present ();
}

void StartupEditor::tileset_editor (std::string shieldset_theme, std::string filename)
{
  Startup::instance ()->initialize ();
  if (shieldset_theme == "")
    shieldset_theme = "default";
  if (filename != "")
    {
      if (!File::exists (filename))
        {
          std::cerr <<
            String::ucompose (_("%1: `%2' cannot be opened for reading."),
                              Lw::get_prgname (), filename) << std::endl;
          Lw::app->quit ();
          return;
        }
      else if (!File::nameEndsWith (filename, TILESET_EXT))
        {
          std::cerr <<
            String::ucompose (_("%1: `%2' isn't a tileset file."),
                              Lw::get_prgname (), filename) << std::endl;
          Lw::app->quit ();
          return;
        }
      else
        {
          Tileset::create
            (filename,
             [this, filename, shieldset_theme] (Tileset *tileset, bool, bool,
                                                Glib::ustring)
             {
               if (tileset)
                 {
                   auto shieldset =
                     load_shieldset_from_option (shieldset_theme);
                   tileset_editor (shieldset, tileset);
                 }
               else
                 {
                   std::cerr <<
                     String::ucompose (_("%1: `%2' couldn't be loaded."),
                                       Lw::get_prgname (), filename) << std::endl;
                   Lw::app->quit ();
                 }
             });
        }
    }
  else
    {
      auto shieldset =
        load_shieldset_from_option (shieldset_theme);
      auto tileset = Tilesetlist::instance ()->get ("default");
      tileset_editor (shieldset, tileset);
    }
}

void StartupEditor::tileset_editor (Shieldset *shieldset, Tileset *tileset)
{
  auto w = new TileSetWindow ();

  w->setup (shieldset, tileset);
  Lw::app->add_window (*w);
  w->signal_closed ().connect
    ([this, w] ()
     {
       delete w;
     });
  w->present ();
}

void StartupEditor::editor (std::string filename)
{
  Startup::instance ()->initialize ();
  if (filename != "")
    {
      if (!File::exists (filename))
        {
          std::cerr <<
            String::ucompose (_("%1: `%2' cannot be opened for reading."),
                              Lw::get_prgname (), filename) << std::endl;
          Lw::app->quit ();
          return;
        }
      else if (!File::nameEndsWith (filename, MAP_EXT) &&
               !File::nameEndsWith (filename, SAVE_EXT))
        {
          std::cerr <<
            String::ucompose
            (_("%1: `%2' isn't a file that the editor can open."),
             Lw::get_prgname (), filename) << std::endl;
          Lw::app->quit ();
          return;
        }
    }

  auto w = Gtk::make_managed<EditorSplashWindow> ();
  w->setup ();
  Lw::app->add_window (*w);
  w->present ();

  Glib::signal_idle ().connect
    ([this, w, filename]()
     {
       w->set_fraction (0.25);
       bool broke1;
       auto armyset = Armysetlist::instance ()->get ("default");
       armyset->instantiateImages (broke1);
       Glib::signal_idle ().connect
         ([this, w, filename]()
          {
            w->set_fraction (0.50);
            bool broke2;
            auto cityset = Citysetlist::instance ()->get ("default");
            cityset->instantiateImages (broke2);
            Glib::signal_idle ().connect
              ([this, w, filename]()
               {
                 w->set_fraction (0.75);
                 bool broke3;
                 auto shieldset = Shieldsetlist::instance ()->get ("default");
                 shieldset->instantiateImages (broke3);
                 Glib::signal_idle ().connect
                   ([this, w, filename]()
                    {
                      bool broke4;
                      w->set_fraction (0.90);
                      auto tileset = Tilesetlist::instance ()->get ("default");
                      tileset->instantiateImages (broke4);
                      Glib::signal_idle ().connect
                        ([this, w, filename]()
                         {
                           w->hide ();
                           Lw::app->remove_window (*w);
                           auto s = Gtk::make_managed<ScenarioBuilderWindow> ();
                           s->setup (filename);
                           Lw::app->add_window (*s);
                           s->present ();
                           return false;
                         });
                      return false;
                    });
                 return false;
               });
            return false;
          });
       return false;
     });
}

Shieldset *StartupEditor::load_shieldset_from_option (std::string shieldset_theme)
{
  auto shieldset =
    Shieldsetlist::instance ()->get (shieldset_theme);
  if (!shieldset)
    {
      std::cerr <<
        String::ucompose (_("%1: unknown shieldset theme '%2'."),
                          Lw::get_prgname (), shieldset_theme) << std::endl;
      Lw::app->quit ();
      exit (0);
      return NULL;
    }
  return shieldset;
}
