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
#ifndef RUIN_FIGHT_FINISHED_DIALOG_H
#define RUIN_FIGHT_FINISHED_DIALOG_H
class RuinFightFinishedDialog: public LwDialogBase
{
public:
    static std::string get_resource_name ()
      {
        return "ruinfight-finished.ui";
      }

    RuinFightFinishedDialog (BaseObjectType* o,
                             const Glib::RefPtr<Gtk::Builder>& xml)
      : LwDialogBase (o, xml)
      {
        m_continue_button = load <Gtk::Button> ("continue_button");
        m_label = load <Gtk::Label> ("label");
        m_picture = load <Gtk::Picture> ("picture");
      }

    void setup (FightResult::Outcome res)
      {
        set_response (m_continue_button, Gtk::ResponseType::CLOSE);

        Glib::ustring s = "";

        if (res == FightResult::ATTACKER_WON)
          s = _("...and is victorious!");
        else
          s = _("...and is slain by it!");

        m_label->set_text (s);

        int i = 0;
          
        if (res == FightResult::ATTACKER_WON)
          i = ImageCache::DIALOG_RUIN_SUCCESS;
        else
          i = ImageCache::DIALOG_RUIN_DEFEAT;
        
        auto im = ImageCache::instance ()->getDialogPic (i);
        m_picture->set_paintable (im->to_texture ());

        signal_response ().connect
          ([this](Gtk::ResponseType)
           {
             hide ();
           });
      }
private:
    Gtk::Button *m_continue_button;
    Gtk::Label *m_label;
    Gtk::Picture *m_picture;
};
#endif
