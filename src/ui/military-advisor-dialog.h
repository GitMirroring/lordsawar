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
#ifndef MILITARY_ADVISOR_DIALOG_H
#define MILITARY_ADVISOR_DIALOG_H
class MilitaryAdvisorDialog: public LwDialogBase
{
public:
    static std::string get_resource_name ()
      {
        return "military-advisor.ui";
      }

    MilitaryAdvisorDialog (BaseObjectType* o,
                           const Glib::RefPtr<Gtk::Builder>& xml)
      : LwDialogBase (o, xml)
      {
        m_continue_button = load <Gtk::Button> ("continue_button");
        m_label = load <Gtk::Label> ("label");
      }

    void setup (float chances)
      {
        set_response (m_continue_button, Gtk::ResponseType::ACCEPT);
        set_title (_("Advisor!"));

        Glib::ustring s;

        int num = Rnd::rand() % 5;
        if (num == 0)
          s = _("My Good Lord!");
        else if (num == 1)
          s = _("Great and Worthy Lord!");
        else if (num == 2)
          s = _("O Champion of Justice!");
        else if (num == 3)
          s = _("O Mighty Leader!");
        else if (num == 4)
          s = _("O Great Warlord!");
        else
          s = _("O Great Warlord!");
        s += "\n";

        num = Rnd::rand() % 7;
        if (chances >= 90.0)
          {
            if (num == 0)
              s += _("This battle will surely be as simple as butchering sleeping cattle!");
            else if (num == 1)
              s += _("A battle here would be as simple as butchering sleeping cattle!");
            else if (num == 2)
              s += _("I believe this battle will surely be as simple as butchering sleeping cattle!");
            else if (num == 3)
              s += _("This battle would be as simple as butchering sleeping cattle!");
            else if (num == 4)
              s += _("A battle here would be as simple as butchering sleeping cattle!");
            else if (num == 5)
              s += _("I believe this battle will be as simple as butchering sleeping cattle!");
            else if (num == 6)
              s += _("This battle shall be as simple as butchering sleeping cattle!");
          }
        else if (chances >= 80.0)
          {
            if (num == 0)
              s += _("This battle will surely be an easy victory!  We cannot lose!");
            else if (num == 1)
              s += _("A battle here would be an easy victory!  We cannot lose!");
            else if (num == 2)
              s += _("I believe this battle will surely be an easy victory!  We cannot lose!");
            else if (num == 3)
              s += _("This battle would be an easy victory!  We cannot lose!");
            else if (num == 4)
              s += _("A battle here would be an easy victory!  We cannot lose!");
            else if (num == 5)
              s += _("I believe this battle will be an easy victory!  We cannot lose!");
            else if (num == 6)
              s += _("This battle shall be an easy victory!  We cannot lose!");
          }
        else if (chances >= 70.0)
          {
            if (num == 0)
              s += _("This battle will surely be a comfortable victory!");
            else if (num == 1)
              s += _("A battle here would be a comfortable victory!");
            else if (num == 2)
              s += _("I believe this battle will surely be a comfortable victory!");
            else if (num == 3)
              s += _("This battle would be a comfortable victory!");
            else if (num == 4)
              s += _("A battle here would be a comfortable victory!");
            else if (num == 5)
              s += _("I believe this battle will be a comfortable victory!");
            else if (num == 6)
              s += _("This battle shall be a comfortable victory!");
          }
        else if (chances >= 60.0)
          {
            if (num == 0)
              s += _("This battle will surely be a hard fought victory! But we shall win!");
            else if (num == 1)
              s += _("A battle here would be a hard fought victory! But we shall win!");
            else if (num == 2)
              s += _("I believe this battle will surely be a hard fought victory! But we shall win!");
            else if (num == 3)
              s += _("This battle would be a hard fought victory! But we shall win!");
            else if (num == 4)
              s += _("A battle here would be a hard fought victory! But we shall win!");
            else if (num == 5)
              s += _("I believe this battle will be a hard fought victory! But we shall win!");
            else if (num == 6)
              s += _("This battle shall be a hard fought victory! But we shall win!");
          }
        else if (chances >= 50.0)
          {
            if (num == 0)
              s += _("This battle will surely be very evenly matched!");
            else if (num == 1)
              s += _("A battle here would be very evenly matched!");
            else if (num == 2)
              s += _("I believe this battle will surely be very evenly matched!");
            else if (num == 3)
              s += _("This battle would be very evenly matched!");
            else if (num == 4)
              s += _("A battle here would be very evenly matched!");
            else if (num == 5)
              s += _("I believe this battle will be very evenly matched!");
            else if (num == 6)
              s += _("This battle shall be very evenly matched!");
          }
        else if (chances >= 40.0)
          {
            if (num == 0)
              s += _("This battle will surely be difficult but not impossible to win!");
            else if (num == 1)
              s += _("A battle here would be difficult but not impossible to win!");
            else if (num == 2)
              s += _("I believe this battle will surely be difficult but not impossible to win!");
            else if (num == 3)
              s += _("This battle would be difficult but not impossible to win!");
            else if (num == 4)
              s += _("A battle here would be difficult but not impossible to win!");
            else if (num == 5)
              s += _("I believe this battle will be difficult but not impossible to win!");
            else if (num == 6)
              s += _("This battle shall be difficult but not impossible to win!");
          }
        else if (chances >= 30.0)
          {
            if (num == 0)
              s += _("This battle will surely be a brave choice! I leave it to thee!");
            else if (num == 1)
              s += _("A battle here would be a brave choice! I leave it to thee!");
            else if (num == 2)
              s += _("I believe this battle will surely be a brave choice! I leave it to thee!");
            else if (num == 3)
              s += _("This battle would be a brave choice! I leave it to thee!");
            else if (num == 4)
              s += _("A battle here would be a brave choice! I leave it to thee!");
            else if (num == 5)
              s += _("I believe this battle will be a brave choice! I leave it to thee!");
            else if (num == 6)
              s += _("This battle shall be a brave choice! I leave it to thee!");
          }
        else if (chances >= 20.0)
          {
            if (num == 0)
              s += _("This battle will surely be a foolish decision!");
            else if (num == 1)
              s += _("A battle here would be a foolish decision!");
            else if (num == 2)
              s += _("I believe this battle will surely be a foolish decision!");
            else if (num == 3)
              s += _("This battle would be a foolish decision!");
            else if (num == 4)
              s += _("A battle here would be a foolish decision!");
            else if (num == 5)
              s += _("I believe this battle will be a foolish decision!");
            else if (num == 6)
              s += _("This battle shall be a foolish decision!");
          }
        else if (chances >= 10.0)
          {
            if (num == 0)
              s += _("This battle will surely be sheerest folly!  Thou shouldst not attack!");
            else if (num == 1)
              s += _("A battle here would be sheerest folly!  Thou shouldst not attack!");
            else if (num == 2)
              s += _("I believe this battle will surely be sheerest folly!  Thou shouldst not attack!");
            else if (num == 3)
              s += _("This battle would be sheerest folly!  Thou shouldst not attack!");
            else if (num == 4)
              s += _("A battle here would be sheerest folly!  Thou shouldst not attack!");
            else if (num == 5)
              s += _("I believe this battle will be sheerest folly!  Thou shouldst not attack!");
            else if (num == 6)
              s += _("This battle shall be sheerest folly!  Thou shouldst not attack!");
          }
        else
          {
            if (num == 0)
              s += _("This battle will surely be complete and utter suicide!");
            else if (num == 1)
              s += _("A battle here would be complete and utter suicide!");
            else if (num == 2)
              s += _("I believe this battle will surely be complete and utter suicide!");
            else if (num == 3)
              s += _("This battle would be complete and utter suicide!");
            else if (num == 4)
              s += _("A battle here would be complete and utter suicide!");
            else if (num == 5)
              s += _("I believe this battle will be complete and utter suicide!");
            else if (num == 6)
              s += _("This battle shall be complete and utter suicide!");
          }
        m_label->set_wrap (true);
        m_label->set_max_width_chars (40);
        m_label->set_text (s);

        signal_response ().connect
          ([this](Gtk::ResponseType)
           {
             hide ();
           });
      }
private:
    Gtk::Button *m_continue_button;
    Gtk::Label *m_label;
};
#endif
