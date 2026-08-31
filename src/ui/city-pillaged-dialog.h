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
#ifndef CITY_PILLAGED_DIALOG_H
#define CITY_PILLAGED_DIALOG_H
class CityPillagedDialog: public LwDialogBase
{
public:
    static std::string get_resource_name ()
      {
        return "city-pillaged.ui";
      }

    CityPillagedDialog (BaseObjectType* o,
                        const Glib::RefPtr<Gtk::Builder>& xml)
      : LwDialogBase (o, xml)
      {
        m_continue_button = load <Gtk::Button> ("continue_button");
        m_pillaged_army_type_cost_label =
          load <Gtk::Label> ("pillaged_army_type_cost_label");
        m_pillaged_army_type_image =
          load <Gtk::Image> ("pillaged_army_type_image");
        m_label = load <Gtk::Label> ("label");
      }

    void setup (City *city, int gold, int pillaged_army_type)
      {
        set_response (m_continue_button, Gtk::ResponseType::ACCEPT);

        setup_label (city, gold, pillaged_army_type);

        setup_image (city, gold, pillaged_army_type);

        signal_response ().connect
          ([this](Gtk::ResponseType)
           {
             hide ();
           });
      }
private:
    Gtk::Button *m_continue_button;
    Gtk::Label *m_pillaged_army_type_cost_label;
    Gtk::Image *m_pillaged_army_type_image;
    Gtk::Label *m_label;

    void setup_label (City *city, int gold, int pillaged_army_type)
      {
        Glib::ustring s =
          String::ucompose
          (_("Your troops have pillaged the city of %1!"),
             city->getName ());

        s += "\n\n";

        unsigned int a = Playerlist::getActiveplayer ()->getArmyset ();
        Glib::ustring name =
          Armysetlist::instance ()->getArmy (a, pillaged_army_type)->getName ();
        s += String::ucompose
          (_("The ability to produce %1 has been lost."), name);

        s += "\n\n";

        s += String::ucompose
          (ngettext ("The loot is worth %1 gold piece.",
                     "The loot is worth %1 gold pieces.",
                     gold), gold);

        m_label->set_text (s);
      }

    void setup_image (City *city, int gold, int pillaged_army_type)
      {
        Player *player = city->getOwner ();
        unsigned int as = player->getArmyset ();

        if (gold == 0)
          {
            m_pillaged_army_type_image->set
              (ImageCache::instance ()->getCircledArmyPic
               (as, 0, player->get_shield (), NULL, false, Shield::NEUTRAL,
                false, Lw::get_dark ())->to_pixbuf ());
            m_pillaged_army_type_cost_label->set_text ("");
          }
        else
          {
            m_pillaged_army_type_image->set
              (ImageCache::instance ()-> getCircledArmyPic
               (as, pillaged_army_type, player->get_shield (), NULL, false,
                Shield::NEUTRAL, true, Lw::get_dark ())->to_pixbuf ());
            m_pillaged_army_type_cost_label->set_text
              (String::ucompose("%1 gp", gold));
          }
        m_pillaged_army_type_image->set_pixel_size (LW_BUTTON_SIZE);
      }
};
#endif
