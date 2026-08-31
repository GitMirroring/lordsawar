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
#ifndef FILE_LABEL_H
#define FILE_LABEL_H
#include <string>

/**
 * in the editors we often have filechooserbuttons.
 *
 * we use regular buttons so they can inserted in cambalache.
 * this filelabel class implements what's inside those buttons.
 *
 * we have a filetype, either image, sound or other and this
 * changes what icon is beside the filename.
 *
 * when there is no filename it shows a "no image set", or
 * "no sound set", or "no file set" message.
 *
 * it does this unless the override is set in which case it says
 * "override default" regardless of file type.
 */
class FileLabel : public Gtk::Box
{
public:
    enum FileType
      {
        IMAGE,
        SOUND,
        OTHER
      };

    FileLabel (Gtk::Button *button, const std::string& text = "", FileType type = FileType::IMAGE)
      : Gtk::Box (Gtk::Orientation::HORIZONTAL, 6)
      {
        m_type = type;
        m_override = false;
        m_button = button;
        m_label = Gtk::make_managed<Gtk::Label> ();
        m_label->set_max_width_chars (20);
        m_label->set_ellipsize (Pango::EllipsizeMode::END);
        m_icon = Gtk::make_managed<Gtk::Image> ();
        set_margin_start (6);
        set_margin_end (6);

        set_icon (type);
        set_label (text);

        append (*m_icon);
        append (*m_label);

        m_label->set_halign (Gtk::Align::START);
        m_label->set_valign (Gtk::Align::CENTER);
        
        button->set_child (*this);
      }

    void set_label (Glib::ustring text)
      {
        if (text == "")
          clear ();
        else
          {
            m_label->set_css_classes ({});
            m_label->add_css_class ("file-label");
            m_label->set_text (text);
            if ((int)text.length () > m_label->get_max_width_chars ())
              m_button->set_tooltip_text (text);
          }
      }

    void set_override ()
      {
        m_override = true;
      }

    void clear ()
      {
        m_label->set_css_classes ({});

        if (m_override)
          {
            m_label->add_css_class ("file-label-override");
            m_label->set_text (_("override default"));
          }
        else
          {
            m_label->add_css_class ("file-label-unset");
            switch (m_type)
              {
              case FileType::IMAGE:
                m_label->set_text (_("No image set"));
                break;

              case FileType::SOUND:
                m_label->set_text (_("No sound set"));
                break;

              case FileType::OTHER:
                m_label->set_text (_("No file set"));
                break;
              }
          }
      }

    void set_icon (FileType type)
      {
        m_type = type;
        switch (type)
          {
          case FileType::IMAGE:
            m_icon->set_from_icon_name ("image-x-generic-symbolic");
            break;

          case FileType::SOUND:
            m_icon->set_from_icon_name ("audio-x-generic-symbolic");
            break;

          case FileType::OTHER:
            m_icon->set_from_icon_name ("text-x-generic-symbolic");
            break;
          }

        m_icon->set_pixel_size (13);
      }

private:
    Gtk::Image *m_icon;
    Gtk::Label *m_label;
    FileType m_type;
    bool m_override;
    Gtk::Button *m_button;
};
#endif
