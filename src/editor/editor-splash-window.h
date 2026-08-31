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
#ifndef EDITOR_SPLASH_WINDOW_H
#define EDITOR_SPLASH_WINDOW_H

class EditorSplashWindow: public Gtk::ApplicationWindow
{
public:

    EditorSplashWindow ()
      {
        set_decorated (false);
      }

    void setup ()
      {
        populate ();
      }

    void set_fraction (double fraction)
      {
        m_progress->set_fraction (fraction);
      }

private:
    Gtk::ProgressBar *m_progress;

    void populate ()
      {
        auto box = Gtk::make_managed<Gtk::Box> (Gtk::Orientation::VERTICAL);
        box->set_margin (12);
        box->set_spacing (3);
        auto title = Gtk::make_managed<Gtk::Label> ("LordsAWar!");
        title->add_css_class ("editor-splash-title");
        box->append (*title);
        auto subtitle = Gtk::make_managed<Gtk::Label> (_("Scenario Builder"));
        subtitle->add_css_class ("editor-splash-subtitle");
        box->append (*subtitle);
        auto hbox = Gtk::make_managed<Gtk::Box> (Gtk::Orientation::HORIZONTAL);
        m_progress = Gtk::make_managed<Gtk::ProgressBar> ();
        m_progress->set_hexpand (true);
        hbox->append (*m_progress);
        box->append (*hbox);
        set_child (*box);
      }
};
#endif
