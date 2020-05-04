//  Copyright (C) 2017, 2020 Ben Asselstine
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

#pragma once
#ifndef MEDIA_DIALOG_H
#define MEDIA_DIALOG_H

#include <vector>
#include <gtkmm.h>
#include "lw-editor-dialog.h"
#include "PixMask.h"

class TarFile;
class Shieldset;
class TarFileMaskedImage;
class TarFileImage;

class MediaDialog: public LwEditorDialog
{
public:
    MediaDialog(Gtk::Window &parent, TarFile *tarfile);
    ~MediaDialog();

    int run();
    void hide() {dialog->hide();}

    bool get_needs_saving() const {return d_needs_saving;}

private:
    //data
    bool d_needs_saving;
    TarFile *d_tarfile;
    Gtk::Button *d_next_turn_button;
    Gtk::Button *d_city_defeated_button;
    Gtk::Button *d_winning_button;
    Gtk::Button *d_hero_male_button;
    Gtk::Button *d_hero_female_button;
    Gtk::Button *d_ruin_success_button;
    Gtk::Button *d_ruin_defeat_button;
    Gtk::Button *d_hero_newlevel_male_button;
    Gtk::Button *d_hero_newlevel_female_button;
    Gtk::Button *d_parley_offered_button;
    Gtk::Button *d_parley_refused_button;
    Gtk::Button *d_small_medals_button;
    Gtk::Button *d_big_medals_button;
    Gtk::Button *d_commentator_button;
    Gtk::Button *d_bless_button;
    Gtk::Button *d_hero_button;
    Gtk::Button *d_battle_button;
    Gtk::Button *d_defeat_button;
    Gtk::Button *d_victory_button;
    Gtk::Button *d_back_button;
    Gtk::Notebook *notebook;

    //callbacks
    void on_next_turn_button_activated();
    void on_city_defeated_button_activated();
    void on_winning_button_activated();
    void on_hero_male_button_activated();
    void on_hero_female_button_activated();
    void on_ruin_success_button_activated();
    void on_ruin_defeat_button_activated();
    void on_hero_newlevel_male_button_activated();
    void on_hero_newlevel_female_button_activated();
    void on_parley_offered_button_activated();
    void on_parley_refused_button_activated();
    void on_small_medals_button_activated();
    void on_big_medals_button_activated();
    void on_commentator_button_activated();
    void on_bless_button_activated();
    void on_hero_button_activated();
    void on_battle_button_activated();
    void on_defeat_button_activated();
    void on_victory_button_activated();
    void on_back_button_activated();

    //helpers
    void fill_in_buttons();
    void fill_image_button(Gtk::Button *button, Glib::ustring name);
    void fill_sound_button(Gtk::Button *button, Glib::ustring name);
    void on_image_button_activated(TarFileImage *omim, TarFileImage *mim);
    void on_masked_image_button_activated(sigc::slot<Glib::ustring> getName, TarFileMaskedImage *mim, sigc::slot<void,Glib::ustring> setName, Shieldset *ss);
    void on_sound_button_activated(sigc::slot<Glib::ustring> getName, sigc::slot<Glib::ustring> getDefaultFilename, sigc::slot<void, Glib::ustring> setName);
};

#endif
