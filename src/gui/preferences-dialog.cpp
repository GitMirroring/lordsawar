//  Copyright (C) 2007 Ole Laursen
//  Copyright (C) 2007, 2008, 2009, 2014, 2015, 2017, 2020 Ben Asselstine
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
#include <sigc++/functors/mem_fun.h>

#include "preferences-dialog.h"

#include "defs.h"
#include "Configuration.h"
#include "snd.h"
#include "game.h"
#include "GameScenario.h"
#include "playerlist.h"
#include "AI_Diplomacy.h"
#include "AI_Analysis.h"
#include "ai_fast.h"
#include "ImageCache.h"
#include "game-options-dialog.h"
#include "font-size.h"

#define method(x) sigc::mem_fun(*this, &PreferencesDialog::x)

PreferencesDialog::PreferencesDialog(Gtk::Window &parent, bool readonly)
 : LwDialog(parent, "preferences-dialog.ui")
{
  d_readonly = readonly;
    xml->get_widget("commentator_switch", commentator_switch);
    xml->get_widget("speed_scale", speed_scale);
    xml->get_widget("play_music_switch", play_music_switch);
    xml->get_widget("music_volume_scale", music_volume_scale);
    xml->get_widget("players_vbox", players_vbox);
    xml->get_widget("game_options_button", game_options_button);
    game_options_button->signal_clicked().connect(method(on_game_options_clicked));
    
    ImageCache *gc = ImageCache::getInstance();
    for (unsigned int i = 0; i < MAX_PLAYERS; i++)
      {
	Player *p = Playerlist::getInstance()->getPlayer(i);
	if (p == NULL)
	  continue;
	if (p == Playerlist::getInstance()->getNeutral())
	  continue;
	Gtk::Box *player_hbox = new Gtk::Box(Gtk::ORIENTATION_HORIZONTAL);
	Gtk::Image *image = new Gtk::Image();
	image->property_pixbuf() =
          gc->getShieldPic(2, p, false,
                           FontSize::getInstance ()->get_height ())->to_pixbuf();
	Gtk::ComboBoxText *type = new Gtk::ComboBoxText();
	type->signal_changed().connect (sigc::bind(method(on_type_changed), type));
	Gtk::CheckButton *observe = new Gtk::CheckButton(_("Observe"));
	observe->property_active().signal_changed().connect
	  (sigc::bind(method(on_observe_toggled), observe));

	observe->set_active(p->isObservable());
	type->append(_("Human"));
	type->append(_("Computer"));
        if (readonly)
          type->append(_("Networked"));
	if (p->getType() == Player::HUMAN)
	  {
	    observe->set_sensitive(false);
	    type->set_active(0);
	  }
        else if (p->getType() == Player::NETWORKED)
	  {
	    observe->set_sensitive(false);
	    type->set_active(2);
	  }
	else
	  type->set_active(1);
	if (p->isDead() || (Playerlist::getActiveplayer() == p && 
			    p->getType() != Player::HUMAN))
	  {
	    type->set_sensitive(false);
	    observe->set_sensitive(false);
	  }
	if (readonly)
	  type->set_sensitive(false);
	player_hbox->pack_start(*manage(image), Gtk::PACK_SHRINK, 
				Gtk::PACK_SHRINK);
	Gtk::Label *name = new Gtk::Label(p->getName());
        name->set_alignment (Gtk::ALIGN_START, Gtk::ALIGN_CENTER);
        name->set_padding (12, 0);
	player_hbox->pack_start(*manage(name), Gtk::PACK_EXPAND_PADDING, Gtk::PACK_EXPAND_PADDING);
	player_hbox->pack_start(*manage(type), Gtk::PACK_SHRINK, 10);
	player_hbox->pack_start(*manage(observe), Gtk::PACK_SHRINK, 10);
	player_types[p] = type;
	player_observed[p] = observe;
	player_name[p] = name;
	players_vbox->pack_start(*manage(player_hbox));
      }
    players_vbox->show_all_children();
    commentator_switch->property_active().signal_changed().connect(
	method(on_show_commentator_toggled));
    speed_scale->set_value(Configuration::s_displaySpeedDelay);
    speed_scale->signal_value_changed().connect(method(on_speed_changed));
    play_music_switch->property_active().signal_changed().connect(method(on_play_music_toggled));
    music_volume_scale->signal_value_changed().connect
      (method(on_music_volume_changed));

    commentator_switch->set_active(Configuration::s_displayCommentator);
    play_music_switch->set_active(Configuration::s_musicenable);
    music_volume_scale->set_value(Configuration::s_musicvolume * 100.0 / 128);
    music_volume_scale->set_sensitive(Configuration::s_musicenable);
    
}

void PreferencesDialog::on_type_changed(Gtk::ComboBoxText *combo)
{
  for (PlayerTypeMap::iterator it = player_types.begin();
       it != player_types.end(); ++it)
    {
      if (combo == (*it).second)
	{
	  /**
	   * if we're turning this player into a human,
	   * then we desensitize the associated switch
	   * otherwise, we presume that we want to observe it
	   */
	  if (combo->get_active_text() == _("Human"))
	    {
	      player_observed[(*it).first]->set_active(true);
	      player_observed[(*it).first]->set_sensitive(false);
	    }
	  else
	    {
	      player_observed[(*it).first]->set_sensitive(true);
	      player_observed[(*it).first]->set_active(true);
	    }
	}
    }
}

void PreferencesDialog::on_observe_toggled(Gtk::CheckButton *button)
{
  for (PlayerObserveMap::iterator it = player_observed.begin();
       it != player_observed.end(); ++it)
    {
      if (button == (*it).second)
	(*it).first->setObservable(button->get_active());
    }
}

void PreferencesDialog::run(Game *game)
{
    static int width = -1;
    static int height = -1;

    if (width != -1 && height != -1)
	dialog->set_default_size(width, height);
    
    dialog->show();
    int max = 0;
    for (auto p :player_name)
      {
        int w = p.second->get_width();
        if (w > max)
          max = w;
      }
    for (auto p :player_name)
      {
        p.second->property_width_request() = max;
        p.second->property_halign() = Gtk::ALIGN_START;
      }
    dialog->run();
    
    dialog->get_size(width, height);
    Configuration::saveConfigurationFile();
    if (game->getScenario()->getPlayMode() == GameScenario::HOTSEAT)
      {
        for (PlayerTypeMap::iterator j = player_types.begin();
             j != player_types.end(); ++j)
          {
            Player *p = (*j).first;
            if (p == NULL)
              continue;
            if (p == Playerlist::getInstance()->getNeutral())
              continue;
            if (p->getType() == Player::HUMAN) //changing human to:
              {
                if ((*j).second->get_active_text() == _("Human")) //human, no change
                  ;
                else //computer, change to easy
                  {
                    AI_Fast *new_player = new AI_Fast(*p, true);
                    Player *old_player = p;
                    Playerlist::getInstance()->swap(old_player, new_player);
                    //disconnect and connect game signals
                    game->addPlayer(new_player);
                    delete old_player;
                  }
              }
            else //changing computer to:
              {
                if ((*j).second->get_active_text() == _("Human")) //human, change it
                  {
                    RealPlayer *new_player = new RealPlayer(*p, true);
                    Player *old_player = p;
                    Playerlist::getInstance()->swap(old_player, new_player);
                    //disconnect and connect game signals
                    game->addPlayer(new_player);
                    delete old_player;
                  }
              }
          }
      }
    dialog->hide();
}

void PreferencesDialog::on_show_commentator_toggled()
{
    Configuration::s_displayCommentator = commentator_switch->get_active();
}

void PreferencesDialog::on_play_music_toggled()
{
    bool play_music = play_music_switch->get_active();

    Configuration::s_musicenable = play_music;

    if (play_music)
    {
        Snd::getInstance()->enableBackground();
    }
    else
    {
        Snd::getInstance()->halt();
        Snd::getInstance()->disableBackground();
    }
    music_volume_scale->set_sensitive(Configuration::s_musicenable);
}

void PreferencesDialog::on_speed_changed()
{
  Configuration::s_displaySpeedDelay = int(speed_scale->get_value());
}

void PreferencesDialog::on_music_volume_changed()
{
    int volume = int(music_volume_scale->get_value() / 100 * 128);
    
    Configuration::s_musicvolume = volume;
    Snd::getInstance()->updateVolume();
}
    
void PreferencesDialog::on_game_options_clicked()
{
  GameOptionsDialog gd(*dialog, true);
  gd.run();
}
