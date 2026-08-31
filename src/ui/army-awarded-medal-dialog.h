//  Copyright (C) 2007 Ole Laursen
//  Copyright (C) 2007, 2008, 2009, 2011, 2014, 2020, 2026 Ben Asselstine
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
#ifndef ARMY_AWARDED_MEDAL_DIALOG_H
#define ARMY_AWARDED_MEDAL_DIALOG_H
#include "player.h"
#include "image-cache.h"
class ArmyAwardedMedalDialog: public LwDialogBase
{
public:
    static std::string get_resource_name ()
      {
        return "army-awarded-medal.ui";
      }

    ArmyAwardedMedalDialog (BaseObjectType* o,
                            const Glib::RefPtr<Gtk::Builder>& xml)
      : LwDialogBase (o, xml)
      {
        m_continue_button = load <Gtk::Button> ("continue_button");
        m_label = load <Gtk::Label> ("label");
        m_picture = load <Gtk::Picture> ("picture");
        m_army_icon = load <Gtk::Image> ("army_icon");
      }

    void setup (Army *army, int medal_type)
      {
        set_response (m_continue_button, Gtk::ResponseType::ACCEPT);

        Glib::ustring s = "";
        switch (medal_type)
          {
          case 0:
            s = String::ucompose
              (_("Your unit of %1 is awarded the avenger's medal of valor!"),
               army->getName ());
            break;

          case 1:
            s = String::ucompose
              (_("Your unit of %1 is awarded the defender's medal of bravery!"),
               army->getName ());
            break;

          case 2:
            s = String::ucompose
              (_("Your unit of %1 is awarded the veteran's medal!"),
               army->getName ());
            break;

          default:
            s = String::ucompose
              (_("Your unit of %1 is awarded a medal!"), army->getName ());
            break;
          }
        m_label->set_text (s);

        auto pi = ImageCache::instance ()->getMedalPic (true, medal_type);
        m_picture->set_pixbuf (pi->to_pixbuf ());

        Player *p = army->getOwner ();
        auto as = p->getArmyset ();
        m_army_icon->set_pixel_size (LW_BUTTON_SIZE);
        m_army_icon->set
          (ImageCache::instance ()->getCircledArmyPic
           (as, army->getTypeId (), p->get_shield (), army->getMedalBonuses (),
            false, Shield::NEUTRAL, true, Lw::get_dark ())->to_pixbuf ());

        signal_response ().connect
          ([this](Gtk::ResponseType)
           {
             hide ();
           });
      }
private:
    Gtk::Button *m_continue_button = NULL;
    Gtk::Label *m_label = NULL;
    Gtk::Picture *m_picture = NULL;
    Gtk::Image *m_army_icon = NULL;
};
#endif
