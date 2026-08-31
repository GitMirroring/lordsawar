//  Copyright (C) 2010, 2014, 2017, 2020, 2021, 2026 Ben Asselstine
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
#ifndef VALIDATION_DIALOG_H
#define VALIDATION_DIALOG_H
class ValidationDialog: public LwDialogBase
{
public:
    static std::string get_resource_name ()
      {
        return "validation.ui";
      }

    ValidationDialog (BaseObjectType* o,
                      const Glib::RefPtr<Gtk::Builder>& xml)
      : LwDialogBase (o, xml)
      {
        m_close_button = load <Gtk::Button> ("close_button");
        m_label = load <Gtk::Label> ("label");
        m_textview = load <Gtk::TextView> ("textview");
        m_scrolled_window = load <Gtk::ScrolledWindow> ("scrolled_window");

        std::stringstream ss;
        ss << std::endl;
        m_newline = ss.str ();
      }

    void setup (std::list<Glib::ustring> errors,
                std::list<Glib::ustring> warnings)
      {
        set_response (m_close_button, Gtk::ResponseType::CLOSE);

        if (errors.size () == 0 && warnings.size () == 0)
          {
            m_label->set_text (_("The scenario is valid."));
            m_scrolled_window->set_visible (false);
          }
        else if (!errors.empty () && warnings.empty ())
          {
            m_label->set_text
              (String::ucompose (ngettext ("There is %1 error",
                                           "There are %1 errors",
                                           errors.size ()),
                                 errors.size ()));

            Glib::ustring s = "";
            for (auto e : errors)
              s += e + m_newline;

            m_textview->get_buffer ()->set_text (s);

          }
        else if (errors.empty () && !warnings.empty ())
          {
            m_label->set_text
              (String::ucompose (ngettext ("There is %1 warning",
                                           "There are %1 warnings",
                                           warnings.size ()),
                                 warnings.size ()));

            Glib::ustring s;
            for (auto w : warnings)
              s += w + m_newline;

            m_textview->get_buffer ()->set_text (s);
          }
        else if (!errors.empty () && !warnings.empty ())
          {
            Glib::ustring s =
              String::ucompose (ngettext ("There is %1 error",
                                          "There are %1 errors",
                                          errors.size ()),
                                errors.size ());
            s +=
              String::ucompose (ngettext (", and %1 warning",
                                          ", and %1 warnings",
                                          warnings.size ()),
                                warnings.size ());
            m_label->set_text (s);

            s = _("Errors:") + m_newline;
            for (auto e : errors)
              s += e + m_newline;
            s += m_newline + m_newline;

            s += _("Warnings:") + m_newline;
            for (auto w : warnings)
              s += w + m_newline;

            m_textview->get_buffer ()->set_text (s);
          }
      }
private:
    Gtk::Button *m_close_button;
    Gtk::Label *m_label;
    Gtk::TextView *m_textview;
    Gtk::ScrolledWindow *m_scrolled_window;
    Glib::ustring m_newline;
};
#endif
