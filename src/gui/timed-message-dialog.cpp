//  Copyright (C) 2008, 2014, 2015 Ben Asselstine
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
//  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 
//  02110-1301, USA.

#include <config.h>

#include <gtkmm.h>

#include "timed-message-dialog.h"

#include "ucompose.hpp"
#include "defs.h"
#include "timing.h"

#define method(x) sigc::mem_fun(*this, &TimedMessageDialog::x)

TimedMessageDialog::TimedMessageDialog(Gtk::Window &parent, Glib::ustring message, int timeout, int grace)
{
  d_timeout = timeout;
  d_timer_count = 0;
  d_grace = grace;

  window = new Gtk::MessageDialog(message);

  window->set_message(message);
  window->signal_response().connect (method(on_response));
  window->set_transient_for(parent);
  window->set_modal ();
  window->property_window_position () = Gtk::WIN_POS_CENTER_ON_PARENT;
}

void TimedMessageDialog::add_cancel_button ()
{
  window->add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
}

void TimedMessageDialog::on_response(int response)
{
  if (response == Gtk::RESPONSE_OK)
    response = Gtk::RESPONSE_ACCEPT;
  d_response = response;
  window->hide();
  main_loop->quit();
}

void TimedMessageDialog::run_and_hide()
{
  if (d_timeout > 0)
    Timing::instance().register_timer (method(tick), 1000);

  window->show_all();
  main_loop = Glib::MainLoop::create();
  main_loop->run();
  window->hide();
}

bool TimedMessageDialog::tick()
{
  d_timer_count++;
  if (d_grace)
    {
      if (d_timer_count >= d_grace)
	{
	  d_grace = 0;
	  d_timer_count = 0;
	  return Timing::CONTINUE;
	}
    }
  else
    {
      int secs = d_timeout - d_timer_count;
      Glib::ustring s =
        String::ucompose(ngettext("This message will disappear in %1 second.",
                                  "This message will disappear in %1 seconds.",
                                  secs), secs);
      window->set_secondary_text(s);
    }

  if (d_timer_count <= d_timeout)
    return Timing::CONTINUE;

  window->hide();
  main_loop->quit();

  return Timing::STOP;
}

void TimedMessageDialog::set_title(Glib::ustring title)
{
  window->set_title(title);
}

void TimedMessageDialog::set_image(Glib::RefPtr<Gdk::Pixbuf> picture)
{
  Gtk::Image *image = new Gtk::Image(picture);
  window->property_image() = image;
}
