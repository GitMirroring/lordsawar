//  Copyright (C) 2007 Ole Laursen
//  Copyright (C) 2007, 2008, 2009, 2014, 2020 Ben Asselstine
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
#include <sigc++/functors/mem_fun.h>

#include "signpost-editor-dialog.h"

#include "ucompose.hpp"
#include "defs.h"
#include "CreateScenarioRandomize.h"
#include "signpost.h"
#include "rnd.h"

#define method(x) sigc::mem_fun(*this, &SignpostEditorDialog::x)

SignpostEditorDialog::SignpostEditorDialog(Gtk::Window &parent, Signpost *s, CreateScenarioRandomize *randomizer)
 : LwEditorDialog(parent, "signpost-editor-dialog.ui")
{
    d_randomizer = randomizer;
    signpost = s;
    
    xml->get_widget("sign_textview", sign_textview);
    sign_textview->get_buffer()->signal_changed().connect
      (method(on_sign_changed));
    sign_textview->get_buffer()->set_text(s->getName());
    
    xml->get_widget("randomize_button", randomize_button);
    randomize_button->signal_clicked().connect(method(on_randomize_clicked));
}

void SignpostEditorDialog::on_sign_changed ()
{
  signpost->setName(sign_textview->get_buffer()->get_text());
}

int SignpostEditorDialog::run()
{
  dialog->show_all();
  return dialog->run();
}

void SignpostEditorDialog::on_randomize_clicked()
{
  Glib::ustring existing_name = sign_textview->get_buffer()->get_text();
  bool dynamic = ((Rnd::rand() % d_randomizer->getNumSignposts()) == 0);
  if (existing_name == DEFAULT_SIGNPOST)
    {
      if (dynamic)
	sign_textview->get_buffer()->set_text
	  (d_randomizer->getDynamicSignpost(signpost));
      else
	sign_textview->get_buffer()->set_text
	  (d_randomizer->popRandomSignpost());
    }
  else
    {
      if (dynamic)
	sign_textview->get_buffer()->set_text
	  (d_randomizer->getDynamicSignpost(signpost));
      else
	{
	  sign_textview->get_buffer()->set_text
	    (d_randomizer->popRandomSignpost());
	  d_randomizer->pushRandomSignpost(existing_name);
	}
    }
}
