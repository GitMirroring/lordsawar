//  Copyright (C) 2007 Ole Laursen
//  Copyright (C) 2007-2012, 2014-2016, 2020 Ben Asselstine
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

#include <gtkmm.h>

#include "splash-window.h"
#include "driver.h"

#include "game-preferences-dialog.h"
#include "load-scenario-dialog.h"
#include "builder-cache.h"
#include "Configuration.h"
#include "defs.h"
#include "snd.h"
#include "File.h"
#include "GameScenario.h"
#include "network-game-selector-dialog.h"
#include "main-preferences-dialog.h"
#include "timed-message-dialog.h"
#include "new-random-map-dialog.h"
#include "new-network-game-dialog.h"
#include "profile.h"

#define method(x) sigc::mem_fun(*this, &SplashWindow::x)

SplashWindow::SplashWindow()
  : network_game_nickname ("")
{

  Glib::RefPtr<Gtk::Builder> xml = BuilderCache::get("splash-window.ui");

  xml->get_widget("window", window);
  window->set_icon_from_file(File::getVariousFile("castle_icon.png"));
  window->signal_delete_event().connect (sigc::hide(method(on_delete_event)));

  xml->get_widget("load_game_button", load_game_button);
  load_game_button->signal_clicked().connect (method(on_load_game_clicked));
  xml->get_widget("load_scenario_button", load_scenario_button);
  load_scenario_button->signal_clicked().connect (method(on_load_scenario_clicked));
  xml->get_widget("quit_button", quit_button);
  quit_button->signal_clicked().connect (method(on_quit_clicked));
  xml->get_widget("new_network_game_button", new_network_game_button);
  new_network_game_button->signal_clicked().connect
    (method(on_new_network_game_clicked));
  xml->get_widget("preferences_button", preferences_button);
  preferences_button->signal_clicked().connect (method(on_preferences_clicked));
  xml->get_widget("editor_button", editor_button);
  editor_button->signal_clicked().connect (method(on_editor_clicked));
  Snd::getInstance()->play("intro");

  xml->get_widget("box", main_box);
  xml->get_widget("eventbox", eventbox);
  eventbox->signal_draw().connect (method(on_draw));
  xml->get_widget("button_box", button_box);
  if (Configuration::s_autosave_policy == 1)
    {
      Glib::ustring filename = File::getSaveFile("autosave" + SAVE_EXT);
      FILE *fileptr = fopen (filename.c_str(), "r");
      if (fileptr)
        {
          bool broken = false;
          fclose (fileptr);
          GameScenario::PlayMode mode = 
            GameScenario::loadPlayMode(File::getSaveFile("autosave" + SAVE_EXT),
                                       broken);
          if (mode == GameScenario::HOTSEAT && broken == false)
            {
              crash_button = Gtk::manage(new Gtk::Button());
              crash_button->set_label(_("Rescue Crashed Game"));
              button_box->pack_start(*crash_button, true, true, 0);
              crash_button->signal_clicked().connect(method(on_rescue_crashed_game_clicked));
              button_box->reorder_child(*crash_button, 0);
            }
          else if (broken == true)
            File::erase (File::getSaveFile("autosave" + SAVE_EXT));
        }
    }

  //set the window size.
  bool broken = false;
  bg = PixMask::create (File::getVariousFile("splash_screen.png"), broken);
  if (broken == false)
    main_box->set_size_request (bg->get_width(), bg->get_height());
}

bool SplashWindow::on_draw(const ::Cairo::RefPtr< ::Cairo::Context >& cr)
{
  cr->set_source(bg->get_pixmap(), 0, 0);
  cr->rectangle(0, 0, bg->get_width(), bg->get_height());
  cr->fill();

  Gtk::Allocation box = main_box->get_allocation();
  cr->rectangle(box.get_x(), box.get_y(), box.get_width(), box.get_height());
  gtk_widget_draw (GTK_WIDGET(main_box->gobj()), cr->cobj());
  return true;
}

SplashWindow::~SplashWindow()
{
  Snd::deleteInstance();
  delete bg;
  delete window;
}

bool SplashWindow::on_delete_event()
{
  quit_requested.emit();
  return true;
}

void SplashWindow::on_quit_clicked()
{
  quit_requested.emit();
}

void SplashWindow::on_rescue_crashed_game_clicked()
{
  delete crash_button;
  Glib::ustring filename = File::getSaveFile("autosave" + SAVE_EXT);
  load_requested.emit(filename);
}

void SplashWindow::on_load_game_clicked()
{
  Gtk::FileChooserDialog chooser(*window, _("Choose Game to Load"));
  Glib::RefPtr<Gtk::FileFilter> sav_filter = Gtk::FileFilter::create();
  sav_filter->set_name(_("LordsAWar Saved Games (*.sav)"));
  sav_filter->add_pattern("*" + SAVE_EXT);
  chooser.add_filter(sav_filter);
  chooser.set_current_folder(Configuration::s_savePath);

  chooser.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
  chooser.add_button(Gtk::Stock::OPEN, Gtk::RESPONSE_ACCEPT);
  chooser.set_default_response(Gtk::RESPONSE_ACCEPT);

  chooser.show_all();
  int res = chooser.run();

  if (res == Gtk::RESPONSE_ACCEPT)
    {
      Glib::ustring filename = chooser.get_filename();
      chooser.hide();	
      load_requested.emit(filename);
    }
}

void SplashWindow::on_new_network_game_clicked()
{
  NewNetworkGameDialog nngd(*window, false);
  if (nngd.run())
    {
      nngd.hide();
      network_game_nickname = nngd.getProfile()->getNickname();
      if (nngd.isClient() == true)
        {
          NetworkGameSelectorDialog ngsd(*window, nngd.getProfile());
          ngsd.game_selected.connect(sigc::bind(method(on_network_game_selected), nngd.getProfile()));
          ngsd.run();
        }
      else
        {
          //okay, we're a server.
          LoadScenarioDialog d(*window);
          d.run();
          Glib::ustring filename = d.get_scenario_filename();
          if (filename.empty())
            return;
          d.hide();
          if (filename == "random.map")
            {
              NewRandomMapDialog nrmd(*window);
              int res = nrmd.run();
              if (res == Gtk::RESPONSE_ACCEPT)
                filename = nrmd.getRandomMapFilename();
              else
                return;
            }

          GamePreferencesDialog gpd(*window, filename, GameScenario::NETWORKED);

          gpd.set_title(_("New Networked Game"));
          gpd.game_started.connect(sigc::bind(method(on_network_game_created), nngd.getProfile(), nngd.isAdvertised(), nngd.isRemotelyHosted()));
          gpd.run(network_game_nickname);
          gpd.hide();
          return;
        }
    }
  else
    nngd.hide();
}

void SplashWindow::on_load_scenario_clicked()
{
  LoadScenarioDialog d(*window);
  d.run();

  Glib::ustring filename = d.get_scenario_filename();
  if (!filename.empty())
    {
      d.hide();
      if (filename == "random.map")
	{
	  NewRandomMapDialog nrmd(*window);
	  int res = nrmd.run();
	  if (res == Gtk::RESPONSE_ACCEPT)
            filename = nrmd.getRandomMapFilename();
	  else
	    return;
	}
      GamePreferencesDialog gp(*window, filename, GameScenario::HOTSEAT);
      gp.game_started.connect(sigc::bind(sigc::mem_fun (this, &SplashWindow::on_game_started), &gp));

      gp.run();
    } 
  //load_requested.emit(filename);
}

void SplashWindow::on_editor_clicked()
{
  editor_requested.emit ("");
}

void SplashWindow::on_network_game_selected(Glib::ustring ip, unsigned short port, Profile *profile)
{
  new_remote_network_game_requested.emit(ip, port, profile);
}

void SplashWindow::on_game_started(GameParameters g, GamePreferencesDialog *gpd)
{
  new_game_requested.emit(g, gpd);
}

void SplashWindow::on_network_game_created(GameParameters g, Profile *profile,
                                           bool advertised, 
                                           bool remotely_hosted)
{
  new_hosted_network_game_requested.emit(g, LORDSAWAR_PORT, profile,
                                         advertised, remotely_hosted);
}

void SplashWindow::on_preferences_clicked()
{
  MainPreferencesDialog d(*window);
  d.run();
  d.hide();
}
    
void SplashWindow::open_new_game_dialog()
{
  load_scenario_button->clicked();
}
