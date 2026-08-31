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
#ifndef GAME_OVER_DIALOG_H
#define GAME_OVER_DIALOG_H
class GameOverDialog: public LwDialogBase
{
public:
    static std::string get_resource_name ()
      {
        return "game-over.ui";
      }

    GameOverDialog (BaseObjectType* o,
                       const Glib::RefPtr<Gtk::Builder>& xml)
      : LwDialogBase (o, xml)
      {
        m_picture = load <Gtk::Picture> ("picture");
        m_label = load <Gtk::Label> ("label");
        m_button = load <Gtk::Button> ("continue_button");
      }

    void setup (Player *winner)
      {
        set_response (m_button, Gtk::ResponseType::CLOSE);

        auto im =
          ImageCache::instance ()->getDialogPic (ImageCache::DIALOG_WINNING);
        m_picture->set_paintable (im->to_texture ());

        Glib::ustring s =
          String::ucompose (_("Congratulations to %1 for conquering the world!"),
                            winner->getName ());

        m_label->set_text (s);

        signal_response ().connect
          ([this](Gtk::ResponseType)
           {
             hide ();
           });
      }
private:
    Gtk::Picture *m_picture;
    Gtk::Label *m_label;
    Gtk::Button *m_button;
};
#endif
