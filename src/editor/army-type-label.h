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

#pragma once

#include <gtkmm.h>
#ifndef ARMY_TYPE_LABEL_H
#define ARMY_TYPE_LABEL_H
#include <string>
#include "select-army-dialog.h"
#include "lw-dialog.h"

/**
 * in the editors we often have buttons that choose army types.
 *
 * we use regular buttons so they can inserted in cambalache.
 * this armytypelabel class implements what's inside those buttons.
 *
 */
class ArmyTypeLabel : public Gtk::Box
{
public:

    ArmyTypeLabel (Gtk::Window *parent, Gtk::Button *button, Shield::Color shield,
                   SelectArmyDialog::Mode mode)
      : Gtk::Box (Gtk::Orientation::HORIZONTAL, 6)
      {
        m_parent = parent;
        m_button = button;
        m_shield = shield;
        auto p = Playerlist::instance ()->get (shield);
        m_armyset = p->getArmyset ();
        m_mode = mode;

        m_label = Gtk::make_managed<Gtk::Label> ();
        m_label->set_max_width_chars (12);
        m_label->set_ellipsize (Pango::EllipsizeMode::END);
        set_margin_start (6);
        set_margin_end (6);

        m_icon = Gtk::make_managed<Gtk::Image> ();
        m_icon->add_css_class ("army-type-icon");
        set_army_type (-1);

        append (*m_icon);
        append (*m_label);

        m_label->set_halign (Gtk::Align::START);
        m_label->set_valign (Gtk::Align::CENTER);
        
        button->set_child (*this);

        button->signal_clicked ().connect
          ([this] ()
           {
             auto d = LwDialog::build<SelectArmyDialog> (m_parent);
             d->setup (m_shield, m_mode, m_selected_army_type_id);
             d->signal_response ().connect
               ([this, d] (Gtk::ResponseType resp)
                {
                  switch (resp)
                    {
                    case Gtk::ResponseType::ACCEPT: //select
                      m_signal_army_selected.emit (d->get_selected_army ());
                      set_army_type (d->get_selected_army ());
                      break;

                    case Gtk::ResponseType::REJECT: //clear
                      m_signal_army_selected.emit (-1);
                      clear ();
                      break;

                    default:
                      break;
                    }
                  d->hide ();
                  delete d;
                });
           });
      }

    void set_army_type (int id)
      {
        m_selected_army_type_id = id;
        update_contents ();
      }

    void clear ()
      {
        set_army_type (-1);
      }

    sigc::signal<void(int)> signal_army_selected ()
      {
        return m_signal_army_selected;
      }
private:
    Gtk::Label *m_label;
    Gtk::Image *m_icon;

    Gtk::Window *m_parent;
    Gtk::Button *m_button;
    Shield::Color m_shield;
    guint32 m_armyset;
    SelectArmyDialog::Mode m_mode;
    int m_selected_army_type_id;

    sigc::signal<void(int)> m_signal_army_selected;
        
    void update_contents ()
      {
        m_label->set_css_classes ({});
        if (m_selected_army_type_id > -1)
          {
            auto pix =
              ImageCache::instance ()->getArmyPic
              (m_armyset, (guint32) m_selected_army_type_id, m_shield, NULL,
               false, false);
            if (pix)
              m_icon->set (pix->to_texture ());
            m_label->add_css_class ("army-type-label");
            guint32 id = (guint32) m_selected_army_type_id;
            auto a = Armysetlist::instance ()->getArmy (m_armyset, id);
            if (a)
              {
                auto text = a->getName ();
                m_label->set_text (text);
                if ((int)text.length () > m_label->get_max_width_chars ())
                  m_button->set_tooltip_text (text);
              }
            m_icon->set_visible (true);

          }
        else
          {
            m_icon->clear ();
            m_label->add_css_class ("army-type-label-unset");
            m_label->set_text (_("No army set"));
            m_icon->set_visible (false);
          }
      }
};
#endif
