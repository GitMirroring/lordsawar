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
#ifndef ARMIES_PREVIEW_DIALOG_H
#define ARMIES_PREVIEW_DIALOG_H
#include "image-cache.h"
#include "shield-set.h"
class ArmiesPreviewDialog: public LwDialogBase
{
public:
    static std::string get_resource_name ()
      {
        return "armies-preview.ui";
      }
    
    ArmiesPreviewDialog (BaseObjectType* o,
                         const Glib::RefPtr<Gtk::Builder>& xml)
      : LwDialogBase (o, xml)
      {
        m_close_button = load <Gtk::Button> ("close_button");
        m_next_button = load <Gtk::Button> ("next_button");
        m_previous_button = load <Gtk::Button> ("previous_button");
        m_armies_table = load <Gtk::FlowBox> ("armies_table");
      }

    ~ArmiesPreviewDialog ()
      {
      }

    void setup (Shieldset *shieldset, Armyset *armyset)
      {
        m_shieldset = shieldset;
        m_armyset = armyset;
        m_shield = shieldset->begin ();

        set_response (m_close_button, Gtk::ResponseType::CLOSE);

        m_next_button->set_icon_name ("go-next-symbolic");
        m_next_button->signal_clicked ().connect
          ([this] ()
           {
             m_shield++;
             if (m_shield == m_shieldset->end ())
               m_shield = m_shieldset->begin ();
             fill_armies ();
           });

        m_previous_button->set_icon_name ("go-previous-symbolic");
        m_previous_button->signal_clicked ().connect
          ([this] ()
           {
             if (m_shield == m_shieldset->begin ())
               m_shield = m_shieldset->end ();
             m_shield--;
             fill_armies ();
           });

        fill_armies ();
      }

private:
    Gtk::Button *m_close_button;
    Gtk::Button *m_next_button;
    Gtk::Button *m_previous_button;
    Gtk::FlowBox *m_armies_table;

    Shieldset *m_shieldset;
    Armyset *m_armyset;
    std::list<Shield*>::iterator m_shield;

    void fill_armies ()
      {
        m_armies_table->remove_all ();
        for (auto army : *m_armyset)
          {
            auto shield = *m_shield;
            Shield::Color color = Shield::Color (shield->getOwner ());

            auto im = army->getMaskedImage (color);
            if (im && im->getName ().empty () == false)
              {
                auto pic = im->applyMask (m_shieldset, color);
                auto picture = Gtk::make_managed<Gtk::Picture> ();
                picture->set_paintable (pic->to_texture ());
                picture->set_can_shrink (false);
                picture->set_size_request (LW_BUTTON_SIZE, LW_BUTTON_SIZE);
                delete pic;
                m_armies_table->append (*picture);
              }
          }
      }
};
#endif
