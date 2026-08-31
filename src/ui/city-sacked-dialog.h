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
#ifndef CITY_SACKED_DIALOG_H
#define CITY_SACKED_DIALOG_H
class CitySackedDialog: public LwDialogBase
{
public:
    static std::string get_resource_name ()
      {
        return "city-sacked.ui";
      }

    CitySackedDialog (BaseObjectType* o,
                        const Glib::RefPtr<Gtk::Builder>& xml)
      : LwDialogBase (o, xml)
      {
        m_continue_button = load <Gtk::Button> ("continue_button");
        m_sacked_army_1_cost_label =
          load <Gtk::Label> ("sacked_army_1_cost_label");
        m_sacked_army_1_image =
          load <Gtk::Image> ("sacked_army_1_image");
        m_sacked_army_2_cost_label =
          load <Gtk::Label> ("sacked_army_2_cost_label");
        m_sacked_army_2_image =
          load <Gtk::Image> ("sacked_army_2_image");
        m_sacked_army_3_cost_label =
          load <Gtk::Label> ("sacked_army_3_cost_label");
        m_sacked_army_3_image =
          load <Gtk::Image> ("sacked_army_3_image");
        m_label = load <Gtk::Label> ("label");
      }

    void setup (City *city, int gold, std::list<guint32> sacked_types)
      {
        set_response (m_continue_button, Gtk::ResponseType::ACCEPT);

        setup_label (city, gold, sacked_types.size ());

        setup_images (city, sacked_types);

        signal_response ().connect
          ([this](Gtk::ResponseType)
           {
             hide ();
           });
      }
private:
    Gtk::Button *m_continue_button;
    Gtk::Label *m_sacked_army_1_cost_label;
    Gtk::Image *m_sacked_army_1_image;
    Gtk::Label *m_sacked_army_2_cost_label;
    Gtk::Image *m_sacked_army_2_image;
    Gtk::Label *m_sacked_army_3_cost_label;
    Gtk::Image *m_sacked_army_3_image;
    Gtk::Label *m_label;

    void setup_label (City *city, int gold, unsigned int num_sacked)
      {
        Glib::ustring s =
          String::ucompose
          (_("The city of %1 is sacked\nfor %2 gold!"),
           city->getName(), gold);

        s += "\n\n";

        s +=
          String::ucompose
          (ngettext ("The ability to produce %1 unit has been lost\nand only 1 unit remains.",
                     "The ability to produce %1 units has been lost\nand only 1 unit remains.",
                     num_sacked), num_sacked);
        m_label->set_text (s);

      }

    void setup_images (City *city, std::list<guint32> sacked_types)
      {
        Player *player = city->getOwner ();
        unsigned int as = player->getArmyset ();

        Glib::RefPtr<Gdk::Pixbuf> empty_pic =
          ImageCache::instance ()->getCircledArmyPic
          (as, 0, player->get_shield (), NULL, false, Shield::NEUTRAL, false,
           Lw::get_dark ())->to_pixbuf ();

        int i = 0;
        Gtk::Label *sack_label = NULL;
        Gtk::Image *sack_image = NULL;
        for (auto it = sacked_types.begin(); it != sacked_types.end(); ++it)
          {
            switch (i)
              {
              case 0:
                sack_label = m_sacked_army_1_cost_label;
                sack_image = m_sacked_army_1_image;
                break;
              case 1:
                sack_label = m_sacked_army_2_cost_label;
                sack_image = m_sacked_army_2_image;
                break;
              case 2:
                sack_label = m_sacked_army_3_cost_label;
                sack_image = m_sacked_army_3_image;
                break;
              }
            sack_image->set
              (ImageCache::instance ()->getCircledArmyPic
               (as, *it, player->get_shield (), NULL, false, Shield::NEUTRAL,
                true, Lw::get_dark ())->to_pixbuf ());
            sack_image->set_pixel_size (LW_BUTTON_SIZE);

            const ArmyProto *a = Armysetlist::instance()->getArmy (as, *it);
            sack_label->set_text
              (String::ucompose (_("%1 gp"),a->getNewProductionCost () / 2));
            i++;
          }

        for (i = sacked_types.size(); i < 3; i++)
          {
            switch (i)
              {
              case 0:
                sack_label = m_sacked_army_1_cost_label;
                sack_image = m_sacked_army_1_image;
                break;
              case 1:
                sack_label = m_sacked_army_2_cost_label;
                sack_image = m_sacked_army_2_image;
                break;
              case 2:
                sack_label = m_sacked_army_3_cost_label;
                sack_image = m_sacked_army_3_image;
                break;
              }
            sack_image->set (empty_pic);
            sack_label->set_text ("");
          }
      }
};
#endif
