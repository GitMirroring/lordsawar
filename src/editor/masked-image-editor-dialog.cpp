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

#include "masked-image-editor-dialog.h"

#include "gui/image-helpers.h"
#include "ucompose.hpp"
#include "defs.h"
#include "File.h"
#include "shieldsetlist.h"
#include "shieldset.h"
#include "ImageCache.h"
#include "past-chooser.h"

#define method(x) sigc::mem_fun(*this, &MaskedImageEditorDialog::x)

const int MaskedImageEditorDialog::MAX_IMAGES_WIDTH = 1000;

MaskedImageEditorDialog::MaskedImageEditorDialog(Gtk::Window &parent, Glib::ustring filename, Shieldset *shieldset)
 : LwEditorDialog(parent, "masked-image-editor-dialog.ui")
{
  d_shieldset = shieldset;
    xml->get_widget("filechooserbutton", filechooserbutton);
    xml->get_widget("image_white", image_white);
    xml->get_widget("image_green", image_green);
    xml->get_widget("image_yellow", image_yellow);
    xml->get_widget("image_light_blue", image_light_blue);
    xml->get_widget("image_red", image_red);
    xml->get_widget("image_dark_blue", image_dark_blue);
    xml->get_widget("image_orange", image_orange);
    xml->get_widget("image_black", image_black);
    xml->get_widget("image_neutral", image_neutral);
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

int MaskedImageEditorDialog::run()
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

void MaskedImageEditorDialog::hide()
{
  dialog->hide();
}
void MaskedImageEditorDialog::on_image_chosen()
{
  Glib::ustring selected_filename = filechooserbutton->get_filename();
  if (selected_filename.empty())
    return;

  target_filename = selected_filename;
  show_image(selected_filename);
}


void MaskedImageEditorDialog::update_panel()
{
  if (target_filename != "")
    filechooserbutton->set_filename (target_filename);
}

void MaskedImageEditorDialog::show_image(Glib::ustring filename)
{
  if (filename == "")
    {
      image_white->clear();
      image_green->clear();
      image_yellow->clear();
      image_light_blue->clear();
      image_red->clear();
      image_dark_blue->clear();
      image_orange->clear();
      image_black->clear();
      image_neutral->clear();
      return;
    }
  bool broken = false;
  std::vector<PixMask*> half = disassemble_row(filename, 2, broken);
  if (broken)
    return;
  Vector<int> dim = Vector<int>(half[0]->get_width(), half[0]->get_height());
  if (dim.x * (MAX_PLAYERS + 1) > MAX_IMAGES_WIDTH)
    {
      dim.x = MAX_IMAGES_WIDTH / (MAX_PLAYERS + 1);
      dim.y = half[0]->get_height() *
        (double)((double)dim.x / (double)half[0]->get_width());
    }
  for (unsigned int i = Shield::WHITE; i <= Shield::NEUTRAL; i++)
    {
      Gtk::Image *image = NULL;
      switch (i)
	{
	case Shield::WHITE: image = image_white; break;
	case Shield::GREEN: image = image_green; break;
	case Shield::YELLOW: image = image_yellow; break;
	case Shield::LIGHT_BLUE: image = image_light_blue; break;
	case Shield::RED: image = image_red; break;
	case Shield::DARK_BLUE: image = image_dark_blue; break;
	case Shield::ORANGE: image = image_orange; break;
	case Shield::BLACK: image = image_black; break;
	case Shield::NEUTRAL: image = image_neutral; break;
	default : break;
	}

      if (d_shieldset == NULL)
        d_shieldset = Shieldsetlist::getInstance()->get(1);
      Gdk::RGBA colour = d_shieldset->getColor(i);
      PixMask *army_image = ImageCache::applyMask(half[0],  half[1], colour);
      PixMask::scale (army_image, dim.x, dim.y);
      image->property_pixbuf() = army_image->to_pixbuf();
      delete army_image;
    }
  delete half[0];
  delete half[1];
}

void MaskedImageEditorDialog::on_add(Gtk::Widget *widget)
{
  if (widget)
    {
      Gtk::Button *button = dynamic_cast<Gtk::Button*>(widget);
      button->signal_clicked().connect (method(on_button_pressed));
    }
}

void MaskedImageEditorDialog::on_button_pressed()
{
  Glib::ustring d = PastChooser::getInstance()->get_dir(filechooserbutton);
  if (d.empty() == false)
    filechooserbutton->set_current_folder(d);
}
