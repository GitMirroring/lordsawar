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
#include <gtkmm.h>
#ifndef ABOUT_DIALOG_H
#define ABOUT_DIALOG_H
class AboutDialog: public Gtk::AboutDialog
{
public:

    AboutDialog (Gtk::Window &parent)
      {
        set_transient_for (parent);
        set_modal (true);
        set_program_name ("LordsAWar!");
        set_version (VERSION);
        set_copyright (_("Copyright © 2006-2026 Ben Asselstine, and others"));
        set_comments (_("Thank you for playing!"));
        set_license_type (Gtk::License::GPL_3_0);
        std::vector<Glib::ustring> authors =
          {
            "Ben Asselstine",
            "Ole Laursen",
            "Michael Bartl",
            "Ulf Lorenz",
            "Andrea Paternesi",
            "Josef Spillner",
            "Vibhu Rishi",
            "John Farrell",
            "Bryan Duff",
            "David Sterba",
            "Daniel Nilsson",
            "Marek Publicewicz",
            "Jimmy Chin",
            "Chris Slater",
            "Mark L. Amidon",
            "Thomas Plonka",
            "David Barnsdale",
            "James Andrews",
            "Rene Saucedo",
            "Tiziano Ottaviani",
            "Regis Leroy",
            "Filip Kroczak",
            "Daniel Rigos",
            "Jonathan Blois",
            "Michael Scherer"
          };
        set_authors (authors);

        set_website_label (_("Visit the LordsAWar! Project"));
        set_website  ("https://savannah.nongnu.org/p/lordsawar");
        std::vector<Glib::ustring> documenter =
          {
            "Ben Asselstine"
          };
        set_documenters (documenter);

        std::vector<Glib::ustring> artists =
          {
            "Leon Harmon",
            "Sune Theodorsen",
            "Mark Jones",
            "J. W. Bjerk",
            "David Baumgart",
            "Max Von Juntz (Music)"
          };
        set_artists (artists);

        Glib::ustring translators =
          "Joe Hansen\n"
          "Àngel Mompó\n"
          "Martin Thoma\n"
          "Balázs Úr\n"
          "Erwin Poeze\n"
          "Michal Lisowski\n"
          "Timirtdinov Leonid";
        set_translator_credits (translators);
        set_logo_icon_name ("lordsawar");

        signal_close_request ().connect
          ([this] ()
           {
             m_signal_response.emit (Gtk::ResponseType::DELETE_EVENT);
             return false;
           }, false);
      }

    sigc::signal<void(Gtk::ResponseType)> signal_response ()
      {
        return m_signal_response;
      }

private:
    sigc::signal<void(Gtk::ResponseType)> m_signal_response;
};
#endif
