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
#include "lw-dialog-base.h"
#ifndef PLAYER_DIED_DIALOG_H
#define PLAYER_DIED_DIALOG_H
class PlayerDiedDialog: public LwDialogBase
{
public:
    static std::string get_resource_name ()
      {
        return "player-died.ui";
      }

    PlayerDiedDialog (BaseObjectType* o,
                      const Glib::RefPtr<Gtk::Builder>& xml)
      : LwDialogBase (o, xml)
      {
        m_continue_button = load <Gtk::Button> ("continue_button");
        m_label = load <Gtk::Label> ("label");
      }

    void setup (Player *player)
      {
        Glib::ustring s
          = String::ucompose (_("The rule of %1 has permanently ended!"),
                              player->getName ());
        if (Playerlist::instance ()->countHumanPlayersAlive () == 0 &&
            player->isHuman ())
          {
            s += "\n";
            s += _("No further human resistance is possible but the battle will continue!");
            s += "\n";
            s += _("Press `CTRL-P' to stop the war and visit the sites of thy old battles.");
          }

        m_label->set_wrap (true);
        m_label->set_max_width_chars (40);
        m_label->set_text (s);

        set_response (m_continue_button, Gtk::ResponseType::ACCEPT);

        signal_response ().connect
          ([this](Gtk::ResponseType)
           {
             hide ();
           });
      }
private:
    Gtk::Button *m_continue_button;
    Gtk::Label *m_label;
};
#endif
