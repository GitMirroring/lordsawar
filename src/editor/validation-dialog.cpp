//  Copyright (C) 2010, 2014 Ben Asselstine
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
#include <ostream>
#include <iostream>
#include <sstream>

#include <sigc++/functors/mem_fun.h>

#include "validation-dialog.h"

#include "ucompose.hpp"
#include "File.h"
#include "defs.h"


ValidationDialog::ValidationDialog(Gtk::Window &parent, std::list<Glib::ustring> errors, std::list<Glib::ustring> warnings)
 : LwEditorDialog(parent, "validation-dialog.ui")
{
  std::stringstream ss;
  ss << std::endl;
  Glib::ustring newline = ss.str();
  xml->get_widget("label", label);
  xml->get_widget("textview", textview);
  if (errors.size() == 0 && warnings.size() == 0)
    label->set_text(_("No errors"));
  else if (errors.size() && warnings.size() == 0)
    {
      label->set_text
        (String::ucompose(ngettext("There is %1 error", 
                                   "There are %1 errors", errors.size()),
                          errors.size()));
      Glib::ustring s;
      for (auto e : errors)
        s += e + newline;
      textview->get_buffer()->set_text(s);

    }
  else if (errors.size() == 0 && warnings.size())
    {
      label->set_text
      (String::ucompose(ngettext("There is %1 warning",
                                 "There are %1 warnings", warnings.size()),
                        warnings.size()));
      Glib::ustring s;
      for (auto w : warnings)
        s += w + newline;
    }
  else if (errors.size() && warnings.size())
    {
      Glib::ustring s =
        String::ucompose(ngettext("There is %1 error",
                                  "There are %1 errors", errors.size()),
                         errors.size());
      s +=
        String::ucompose(ngettext(", and %1 warning",
                                  ", and %1 warnings", warnings.size()),
                         warnings.size());
      label->set_text(s);
      s = _("Errors:") + newline;
      for (auto e : errors)
        s += e + newline;
      s+= newline + newline;
      s = _("Warnings:") + newline;
      for (auto w : warnings)
        s += w + newline;
      textview->get_buffer()->set_text(s);
    }
}
