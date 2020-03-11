//  Copyright (C) 2020 Ben Asselstine
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
#include <map>
#include <sigc++/functors/mem_fun.h>

#include "defs.h"
#include "quick-help-window.h"

#include "builder-cache.h"

#define method(x) sigc::mem_fun(*this, &QuickHelpWindow::x)
QuickHelpWindow::QuickHelpWindow()
{
  xml = BuilderCache::get("quick-help-window.ui");
  xml->get_widget("window", window);
  window->set_title (_("Quick Help"));
  xml->get_widget("close_button", close_button);
  close_button->signal_clicked().connect (method(on_close_button_clicked));
}
    
void QuickHelpWindow::on_close_button_clicked ()
{
  window->hide ();
}
