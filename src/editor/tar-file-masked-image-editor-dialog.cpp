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
#include <sigc++/functors/mem_fun.h>

#include "tar-file-masked-image-editor-dialog.h"

#include "gui/image-helpers.h"
#include "ucompose.hpp"
#include "defs.h"
#include "File.h"
#include "shieldsetlist.h"
#include "shieldset.h"
#include "ImageCache.h"
#include "past-chooser.h"
#include "font-size.h"
#include "image-file-filter.h"
#include "timed-message-dialog.h"
#include "TarFileMaskedImage.h"

#define method(x) sigc::mem_fun(*this, &TarFileMaskedImageEditorDialog::x)

const int TarFileMaskedImageEditorDialog::MAX_IMAGES_WIDTH = 1000;

TarFileMaskedImageEditorDialog::TarFileMaskedImageEditorDialog(Gtk::Window &parent, TarFileMaskedImage *mi, double ratio, Shieldset *shieldset)
 : LwEditorDialog(parent, "tar-file-masked-image-editor-dialog.ui")
{
  d_mim = mi;
  d_shieldset = shieldset;
  d_ratio = ratio;
  xml->get_widget("imagebutton", imagebutton);
  imagebutton->signal_clicked().connect (method(on_imagebutton_clicked));
  xml->get_widget("image_white", image_white);
  xml->get_widget("image_green", image_green);
  xml->get_widget("image_yellow", image_yellow);
  xml->get_widget("image_light_blue", image_light_blue);
  xml->get_widget("image_red", image_red);
  xml->get_widget("image_dark_blue", image_dark_blue);
  xml->get_widget("image_orange", image_orange);
  xml->get_widget("image_black", image_black);
  xml->get_widget("image_neutral", image_neutral);
  xml->get_widget("clear_button", clear_button);

  Gtk::Box *box;
  xml->get_widget("shieldset_box", box);
  setup_shield_theme_combobox(box);

  d_target_filename = "";
  update_panel();
  image_neutral->property_visible () = 
    d_mim->getMaskOrientation() == TarFileMaskedImage::HORIZONTAL_MASK;
}

TarFileMaskedImageEditorDialog::~TarFileMaskedImageEditorDialog()
{
}

bool TarFileMaskedImageEditorDialog::load_image ()
{
  bool broken = d_mim->loadFromFile (d_target_filename);

  if (!broken)
    d_mim->instantiateImages ();

  return broken;
}

int TarFileMaskedImageEditorDialog::run()
{
  show_image();
  shield_theme_combobox->show_all ();
  int response = dialog->run();
  if (response != Gtk::RESPONSE_ACCEPT)
    d_target_filename = "";

  return response;
}

void TarFileMaskedImageEditorDialog::hide()
{
  dialog->hide();
}

void TarFileMaskedImageEditorDialog::on_image_chosen(Gtk::FileChooserDialog *d)
{
  Glib::ustring selected_filename = d->get_filename();
  if (selected_filename.empty())
    return;

  d_target_filename = selected_filename;
  load_image ();
  update_panel ();
  show_image ();
}

void TarFileMaskedImageEditorDialog::update_panel()
{
  Glib::ustring f = d_target_filename.empty () ?
    d_mim->getName () :File::get_basename (d_target_filename, true);
  if (f.empty () == false)
    imagebutton->set_label (f);
  else
    {
      imagebutton->set_label (_("No image set"));
      show_image ();
    }
  clear_button->set_visible (d_mim->getImage () == NULL);
}

void TarFileMaskedImageEditorDialog::show_image()
{
  if (d_mim->getImage() == NULL)
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

  Vector<int> dim = d_mim->getImageDimensions ();
  if (dim.x * MAX_PLAYERS  > MAX_IMAGES_WIDTH)
    {
      dim.x = MAX_IMAGES_WIDTH / MAX_PLAYERS;
      dim.y = d_mim->getImage ()->get_height() *
        (double)((double)dim.x / (double)d_mim->getImage ()->get_width());
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
	case Shield::NEUTRAL: 
          image = image_neutral;
          if (d_mim->getMaskOrientation() == TarFileMaskedImage::VERTICAL_MASK)
            continue;
          break;
	default : break;
	}

      if (d_shieldset == NULL)
        {
          Glib::ustring n = shield_theme_combobox->get_active_text();
          d_shieldset = Shieldsetlist::getInstance()->get(n, 0);
        }
      Gdk::RGBA colour = d_shieldset->getColor(i);
      PixMask *p;
      switch (d_mim->getMaskOrientation ())
        {
          case TarFileMaskedImage::HORIZONTAL_MASK:
            p = d_mim->applyMask (colour);
            break;
          case TarFileMaskedImage::VERTICAL_MASK:
            p = d_mim->applyMask (i, colour);
            break;
        }
      PixMask::scale (p, dim.x, dim.y); //idk if we need this
      if (d_ratio > 0)
        {
          int font_size = FontSize::getInstance ()->get_height ();
          double new_height = font_size * d_ratio;
          int new_width =
            ImageCache::calculate_width_from_adjusted_height (p, new_height);
          PixMask::scale (p, new_width, new_height);
        }
      image->property_pixbuf() = p->to_pixbuf();
      image->property_visible () = true;
      delete p;
    }
}

Gtk::FileChooserDialog* TarFileMaskedImageEditorDialog::image_filechooser(bool clear)
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

void TarFileMaskedImageEditorDialog::on_imagebutton_clicked ()
{
  Gtk::FileChooserDialog *d = image_filechooser(d_mim->getImage () == NULL);
  int response = d->run();
  if (response == Gtk::RESPONSE_ACCEPT && d->get_filename() != "")
    {
      if (ImageFileFilter::getInstance ()->hasInvalidExt (d->get_filename ()))
        {
          ImageFileFilter::getInstance()->showErrorDialog (d);
          d_target_filename = "";
        }
      else
        {
          bool broken = PixMask::checkFormat (d->get_filename ());
          if (broken)
            {
              TimedMessageDialog
                td(*d,
                   String::ucompose(_("Couldn't make sense of the image:\n%1"),
                                    d->get_filename ()), 0);
              td.run_and_hide ();
              d_target_filename = "";
            }
          else
            {
              PastChooser::getInstance()->set_dir(d);
              on_image_chosen (d);
            }
        }
    }
  else if (response == Gtk::RESPONSE_REJECT && d_mim->getImage () == NULL)
    clear_button->activate ();
  d->hide();
  delete d;
}

void TarFileMaskedImageEditorDialog::setup_shield_theme_combobox(Gtk::Box *box)
{
  // fill in shield themes combobox
  shield_theme_combobox = manage(new Gtk::ComboBoxText);

  Shieldsetlist *sl = Shieldsetlist::getInstance();
  std::list<Glib::ustring> shield_themes = sl->getValidNames();
  int counter = 0;
  int default_id = 0;
  for (std::list<Glib::ustring>::iterator i = shield_themes.begin(),
       end = shield_themes.end(); i != end; ++i)
    {
      if (*i == _("Default"))
	default_id = counter;
      shield_theme_combobox->append(Glib::filename_to_utf8(*i));
      counter++;
    }

  shield_theme_combobox->set_active(default_id);
  shield_theme_combobox->signal_changed().connect (method(on_shieldset_changed));

  box->set_center_widget (*shield_theme_combobox);
}

void TarFileMaskedImageEditorDialog::on_shieldset_changed()
{
  show_image();
}

