//  Copyright (C) 2008, 2009, 2014, 2017, 2020 Ben Asselstine
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

#include "main-preferences-dialog.h"

#include "Configuration.h"
#include "snd.h"

#define method(x) sigc::mem_fun(*this, &MainPreferencesDialog::x)

MainPreferencesDialog::MainPreferencesDialog(Gtk::Window &parent)
 : LwDialog(parent, "main-preferences-dialog.ui")
{
  xml->get_widget("commentator_switch", commentator_switch);
  xml->get_widget("play_music_switch", play_music_switch);
  xml->get_widget("music_volume_scale", music_volume_scale);
  commentator_switch->property_active().signal_changed().connect
    (method(on_show_commentator_toggled));
  play_music_switch->property_active().signal_changed().connect (method(on_play_music_toggled));
  music_volume_scale->signal_value_changed().connect
    (method(on_music_volume_changed));

  commentator_switch->set_active(Configuration::s_displayCommentator);
  play_music_switch->set_active(Configuration::s_musicenable);
  music_volume_scale->set_value(Configuration::s_musicvolume * 100.0 / 128);
  music_volume_scale->set_sensitive(Configuration::s_musicenable);
}

void MainPreferencesDialog::run()
{
  dialog->show();
  dialog->run();

  Configuration::saveConfigurationFile();
  dialog->hide();
}

void MainPreferencesDialog::on_play_music_toggled()
{
  Configuration::s_musicenable = play_music_switch->get_active();

  if (play_music_switch->get_active())
    Snd::getInstance()->play("intro", -1, false);
  else
    Snd::getInstance()->halt(false);
  music_volume_scale->set_sensitive(Configuration::s_musicenable);
}

void MainPreferencesDialog::on_music_volume_changed()
{
  int volume = int(music_volume_scale->get_value() / 100 * 128);

  Configuration::s_musicvolume = volume;
  Snd::getInstance()->updateVolume();
}

void MainPreferencesDialog::on_show_commentator_toggled()
{
  Configuration::s_displayCommentator = commentator_switch->get_active();
}
