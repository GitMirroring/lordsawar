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
#ifndef LW_DIALOG_BASE_H
#define LW_DIALOG_BASE_H
#include "ucompose.hpp"
class LwDialogBase: public Gtk::Window
{
public:

    LwDialogBase (Gtk::Window::BaseObjectType*&o,
                  const Glib::RefPtr<Gtk::Builder> x)
      :Gtk::Window (o), m_builder (x)
      {
        auto controller = Gtk::EventControllerKey::create ();
        controller->signal_key_pressed().connect
          ([this] (guint keyval, guint, Gdk::ModifierType)
           {
             if (keyval == GDK_KEY_Escape)
               {
                 m_signal_response.emit (Gtk::ResponseType::DELETE_EVENT);
                 return true;
               }
             return false;
           }, false);
        add_controller (controller);

        signal_close_request ().connect
          ([this] ()
           {
             m_signal_response.emit (Gtk::ResponseType::DELETE_EVENT);
             return false;
           }, false);
      }

    ~LwDialogBase ()
      {
        if (m_timeout.connected ())
          m_timeout.disconnect ();
        unparent ();
      }

    void set_timeout (int secs)
      {
        m_timeout =
          Glib::signal_timeout ().connect
          ([this]()
           {
             m_timeout.disconnect ();

             set_title (secs_left (3));
             m_timeout =
               Glib::signal_timeout ().connect
               ([this]()
                {
                  m_timeout.disconnect ();

                  set_title (secs_left (2));

                  m_timeout =
                    Glib::signal_timeout ().connect
                    ([this]()
                     {
                       m_timeout.disconnect ();

                       set_title (secs_left (1));
                       m_timeout =
                         Glib::signal_timeout ().connect
                         ([this]()
                          {
                            m_timeout.disconnect ();

                            m_signal_response.emit (Gtk::ResponseType::DELETE_EVENT);

                            return false;
                          }, 1000);
                       return false;
                     }, 1000);
                  return false;
                }, 1000);
             return false;
           }, secs * 1000);
      }

    void set_response (Gtk::Button *button, Gtk::ResponseType response)
      {
        button->signal_clicked ().connect
          ([this, response] ()
           {
             m_signal_response.emit (response);
           });
      }

    sigc::signal<void(Gtk::ResponseType)> signal_response ()
      {
        return m_signal_response;
      }

    sigc::signal<void()> signal_undo ()
      {
        return m_signal_undo;
      }

    sigc::signal<void()> signal_redo ()
      {
        return m_signal_redo;
      }

    template <typename T>
    T* load (const std::string& name)
      {
        return m_builder->get_widget <T> (name);
      }

    void setup_undo_and_redo ()
      {
        auto actions = Gio::SimpleActionGroup::create();

        actions->add_action
          ("undo",
           ([this] ()
            {
              m_signal_undo.emit ();
            }));

        actions->add_action
          ("redo",
           ([this] ()
            {
              m_signal_redo.emit ();
            }));

        insert_action_group ("win", actions);

        auto shortcuts = Gtk::ShortcutController::create();
        shortcuts->set_propagation_phase(Gtk::PropagationPhase::CAPTURE);

        shortcuts->add_shortcut
          (Gtk::Shortcut::create
           (Gtk::ShortcutTrigger::parse_string ("<Control>z"),
            Gtk::NamedAction::create ("win.undo")));

        shortcuts->add_shortcut
          (Gtk::Shortcut::create
           (Gtk::ShortcutTrigger::parse_string ("<Control>Y"),
            Gtk::NamedAction::create ("win.redo")));

        shortcuts->add_shortcut
          (Gtk::Shortcut::create
           (Gtk::ShortcutTrigger::parse_string ("<Control><Shift>Z"),
            Gtk::NamedAction::create ("win.redo")));

        add_controller (shortcuts);
      }
protected:
    sigc::signal<void(Gtk::ResponseType)> m_signal_response;
    const Glib::RefPtr<Gtk::Builder> m_builder;
    sigc::connection m_timeout;
    sigc::signal<void()> m_signal_undo;
    sigc::signal<void()> m_signal_redo;
private:
    Glib::ustring secs_left (int secs)
      {
        return String::ucompose (ngettext ("%1 second left",
                                           "%1 seconds left",
                                           secs), secs);
      }

};
#endif
