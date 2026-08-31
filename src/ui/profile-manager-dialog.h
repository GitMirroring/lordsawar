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
#include <vector>
#include <string>

#include "lw-dialog-base.h"
#ifndef PROFILE_MANAGER_DIALOG_H
#define PROFILE_MANAGER_DIALOG_H
#include "add-profile-dialog.h"
class ProfileManagerDialog : public LwDialogBase
{
public:
    static std::string get_resource_name ()
      {
        return "profile-manager.ui";
      }

    ProfileManagerDialog (BaseObjectType* o,
                          const Glib::RefPtr<Gtk::Builder>& xml)
      : LwDialogBase (o, xml)
      {
        m_add_button = load <Gtk::Button> ("add_button");
        m_remove_button = load <Gtk::Button> ("remove_button");
        m_set_default_button = load <Gtk::Button> ("set_default_button");
        m_close_button = load <Gtk::Button> ("close_button");
        m_listbox = load <Gtk::ListBox> ("profiles_listbox");
      }

    void setup ()
      {
        set_response (m_close_button, Gtk::ResponseType::CLOSE);
        m_add_button->signal_clicked ().connect
          ([this] ()
           {
              auto d = LwDialog::build<AddProfileDialog> (this);
              d->setup ();
              d->signal_response ().connect
                ([this, d] (Gtk::ResponseType)
                 {
                   refresh_list ();
                   delete d;
                 });
           });

        m_remove_button->signal_clicked ().connect
          ([this] ()
           {
             int index = get_selected_index ();

             if (index < 0)
               return;

             auto &profiles = *Profilelist::instance ();
             auto it = profiles.begin ();
             std::advance (it, index);
             profiles.remove (*it);

             if (profiles.empty ())
               m_default_index = -1;
             else if (m_default_index >= (int)profiles.size ())
               m_default_index = -1;
             else if (index == m_default_index)
               m_default_index = -1;

             refresh_list ();
           });

        m_set_default_button->signal_clicked ().connect
          ([this] ()
           {
             int index = get_selected_index ();

             if (index < 0)
               return;

             m_default_index = index;

             auto &profiles = *Profilelist::instance ();
             auto it = profiles.begin ();
             std::advance (it, index);
             profiles.setDefaultProfile (*it);

             refresh_list ();
           });

        m_listbox->signal_row_selected ().connect
          ([this] (Gtk::ListBoxRow*)
           {
             update_buttons ();
           });

        signal_response ().connect
          ([this] (Gtk::ResponseType)
           {
             hide ();
           });

        fill_profiles ();
      }
private:
    Gtk::Button *m_add_button;
    Gtk::Button *m_remove_button;
    Gtk::Button *m_set_default_button;
    Gtk::Button *m_close_button;
    Gtk::ListBox *m_listbox;
    int m_default_index;

    void fill_profiles ()
      {
        int i = 0;
        for (auto p : *Profilelist::instance ())
          {
            if (p == Profilelist::instance ()->getDefaultProfile ())
              m_default_index = i;
            i++;
          }
        refresh_list ();
      }

    void update_buttons ()
      {
        auto selected = get_selected_index () != -1;
        m_remove_button->set_sensitive (selected);
        auto on_default =
          get_selected_index () == m_default_index &&
          m_default_index != -1;
        m_set_default_button->set_sensitive (selected && !on_default);
      }

    void refresh_list ()
      {
        while (auto* child = m_listbox->get_first_child ())
          m_listbox->remove (*child);

        int i = 0;
        for (auto p : *Profilelist::instance ())
          {
            auto* row_box =
              Gtk::make_managed<Gtk::Box>(Gtk::Orientation::HORIZONTAL, 8);

            auto* name_label = Gtk::make_managed<Gtk::Label>(p->getNickname ());

            name_label->set_hexpand (true);
            name_label->set_halign (Gtk::Align::START);

            row_box->append (*name_label);

            if (i == m_default_index)
              {
                auto* default_label =
                  Gtk::make_managed<Gtk::Label>("(" + _("Default") + ")");

                default_label->add_css_class ("dim-label");
                row_box->append (*default_label);
              }

            auto* row = Gtk::make_managed<Gtk::ListBoxRow> ();
            row->set_child (*row_box);

            m_listbox->append (*row);
            i++;
          }
        update_buttons ();
      }

    int get_selected_index ()
      {
        auto* row = m_listbox->get_selected_row ();

        if (!row)
          return -1;

        return row->get_index ();
      }
};
#endif
