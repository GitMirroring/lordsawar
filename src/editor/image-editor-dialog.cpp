//  Copyright (C) 2009, 2010, 2011, 2014, 2015 Ben Asselstine
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

#include "image-editor-dialog.h"
#include "gui/image-helpers.h"
#include "ucompose.hpp"
#include "defs.h"
#include "File.h"
#include "shieldsetlist.h"
#include "past-chooser.h"

#define method(x) sigc::mem_fun(*this, &ImageEditorDialog::x)

ImageEditorDialog::ImageEditorDialog(Gtk::Window &parent, Glib::ustring filename, int no_frames)
 : LwEditorDialog(parent, "image-editor-dialog.ui")
{
  num_frames = no_frames;

  xml->get_widget("filechooserbutton", filechooserbutton);
  xml->get_widget("image", image);
  show_image(filename);
  target_filename = filename;
  update_panel();
  filechooserbutton->signal_file_set().connect (method(on_image_chosen));
  filechooserbutton->signal_set_focus_child().connect (method(on_add));
  Glib::RefPtr<Gtk::FileFilter> png_filter = Gtk::FileFilter::create();
  png_filter->set_name(_("PNG files (*.png)"));
  png_filter->add_pattern("*.png");
  filechooserbutton->add_filter(png_filter);
}

int ImageEditorDialog::run()
{
    filechooserbutton->set_title(dialog->get_title());
    dialog->show_all();
    int response = dialog->run();
    if (response != Gtk::RESPONSE_ACCEPT)
      target_filename = "";
    else
      PastChooser::getInstance()->set_dir(filechooserbutton);

    return response;
}

void ImageEditorDialog::hide ()
{
  dialog->hide();
}

void ImageEditorDialog::on_image_chosen()
{
  Glib::ustring selected_filename = filechooserbutton->get_filename();
  if (selected_filename.empty())
    return;

  target_filename = selected_filename;
  show_image(selected_filename);
}


void ImageEditorDialog::update_panel()
{
  if (target_filename != "")
    filechooserbutton->set_filename (target_filename);
}

void ImageEditorDialog::show_image(Glib::ustring filename)
{
  if (filename == "")
    {
      image->clear();
      return;
    }

  if (heartbeat.connected())
    heartbeat.disconnect();

  bool broken = false;
  frames = disassemble_row(filename, num_frames, broken);
  if (!broken)
    {
      active_frame = 0;
      image->clear();
      if (num_frames > 0)
        heartbeat = Glib::signal_timeout().connect
          (sigc::bind_return (method (on_heartbeat), true), 500);
      else
        on_heartbeat();
    }
}

void ImageEditorDialog::on_heartbeat()
{
  image->property_pixbuf() = frames[active_frame]->to_pixbuf();
  active_frame++;
  if (active_frame >= num_frames)
    active_frame = 0;
}

void ImageEditorDialog::on_add(Gtk::Widget *widget)
{
  if (widget)
    {
      Gtk::Button *button = dynamic_cast<Gtk::Button*>(widget);
      button->signal_clicked().connect (method(on_button_pressed));
    }
}

void ImageEditorDialog::on_button_pressed()
{
  Glib::ustring d = PastChooser::getInstance()->get_dir(filechooserbutton);
  if (d.empty() == false)
    filechooserbutton->set_current_folder(d);
}
