//  Copyright (C) 2009, 2010, 2011, 2014, 2015, 2020 Ben Asselstine
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
#include "font-size.h"
#include "ImageCache.h"
#include "image-file-filter.h"

#define method(x) sigc::mem_fun(*this, &ImageEditorDialog::x)

ImageEditorDialog::ImageEditorDialog(Gtk::Window &parent, Glib::ustring bname, int no_frames, std::vector<PixMask *> f, double ratio)
 : LwEditorDialog(parent, "image-editor-dialog.ui")
{
  d_ratio = ratio;
  d_active_frame = 0;
  d_num_frames = no_frames;
  d_target_filename = "";

  xml->get_widget("imagebutton", imagebutton);
  imagebutton->signal_clicked().connect (method(on_imagebutton_clicked));

  xml->get_widget("image", image);
  xml->get_widget("clear_button", clear_button);
  update_imagebutton_label (bname);

  if (f.empty () == false)
    clear_button->set_visible (true);

  for (guint32 i = 0; i < f.size (); i++)
    {
      if (d_ratio > 0)
        {
          int font_size = FontSize::getInstance ()->get_height ();
          double new_height = font_size * d_ratio;
          int new_width =
            ImageCache::calculate_width_from_adjusted_height (f[i],
                                                              new_height);
          PixMask *ff = f[i]->copy ();
          PixMask::scale (ff, new_width, new_height);
          frames.push_back(ff);
        }
      else
        frames.push_back(f[i]->copy ());
    }
}

ImageEditorDialog::~ImageEditorDialog()
{
  for (auto f : frames)
    delete f;
}

void ImageEditorDialog::update_imagebutton_label (Glib::ustring filename)
{
  Glib::ustring f = File::get_basename (filename, true);
  if (f.empty () == false)
    imagebutton->set_label (f);
  else
    imagebutton->set_label (_("no image set"));
}

bool ImageEditorDialog::load_frames (Glib::ustring filename)
{
  bool broken = false;
  for (auto f : frames)
    delete f;
  frames = disassemble_row(filename, d_num_frames, broken);
  if (!broken)
    {
      clear_button->set_visible (true);
      if (d_ratio > 0)
        {
          for (int i = 0; i < d_num_frames; i++)
            {
              int font_size = FontSize::getInstance ()->get_height ();
              double new_height = font_size * d_ratio;
              int new_width =
                ImageCache::calculate_width_from_adjusted_height
                (frames[i], new_height);
              PixMask::scale (frames[i], new_width, new_height);
            }
        }
    }
  return broken;
}

int ImageEditorDialog::run()
{
  show_image();
  int response = dialog->run();
  if (response != Gtk::RESPONSE_ACCEPT)
    d_target_filename = "";

  return response;
}

void ImageEditorDialog::hide ()
{
  dialog->hide();
}

void ImageEditorDialog::on_image_chosen(Gtk::FileChooserDialog *d)
{
  Glib::ustring filename = d->get_filename();
  if (filename.empty())
    return;

  d_target_filename = filename;

  update_imagebutton_label (d_target_filename);
  load_frames (d_target_filename);

  show_image ();
}

void ImageEditorDialog::show_image()
{
  if (heartbeat.connected())
    heartbeat.disconnect();

  image->clear();
  if (frames.empty () == false)
    {
      on_heartbeat (false);
      heartbeat = Glib::signal_timeout().connect
        (sigc::bind_return (sigc::bind (method (on_heartbeat), true),
                            true), 500);
    }
}

void ImageEditorDialog::on_heartbeat(bool incr)
{
  image->property_pixbuf() = frames[d_active_frame]->to_pixbuf();
  if (incr)
    {
      d_active_frame++;
      if (d_active_frame >= d_num_frames)
        d_active_frame = 0;
    }
}

Gtk::FileChooserDialog* ImageEditorDialog::image_filechooser(bool clear)
{
  Gtk::FileChooserDialog *d =
    new Gtk::FileChooserDialog(*dialog, dialog->get_title ());
  ImageFileFilter::getInstance ()->add (d);
  d->add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
  d->add_button(Gtk::Stock::OPEN, Gtk::RESPONSE_ACCEPT);
  if (clear)
    d->add_button(Gtk::Stock::CLEAR, Gtk::RESPONSE_REJECT);
  d->set_default_response(Gtk::RESPONSE_ACCEPT);
  d->set_current_folder(PastChooser::getInstance()->get_dir(d));
  return d;
}

void ImageEditorDialog::on_imagebutton_clicked ()
{
  Gtk::FileChooserDialog *d =
    image_filechooser (frames.empty () == false);
  int response = d->run();
  if (response == Gtk::RESPONSE_ACCEPT && d->get_filename() != "")
    {
      if (ImageFileFilter::getInstance()->hasInvalidExt (d->get_filename ()))
        ImageFileFilter::getInstance ()->showErrorDialog (d);
      else
        {
          bool broken = false;
          PixMask *p = PixMask::create (d->get_filename (), broken);
          if (p)
            delete p;
          if (broken)
            {
              Gtk::MessageDialog
                td(*d,
                   String::ucompose(_("Couldn't make sense of the image:\n%1"),
                                    d->get_filename ()));
              td.run();
              td.hide();
            }
          else
            {
              PastChooser::getInstance()->set_dir(d);
              on_image_chosen (d);
            }
        }
    }
  else if (response == Gtk::RESPONSE_REJECT)
    {
      clear_button->activate ();
    }
  d->hide();
  delete d;
}
