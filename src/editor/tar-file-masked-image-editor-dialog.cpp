//  Copyright (C) 2020, 2021 Ben Asselstine
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
#include "tar-file-masked-image-editor-actions.h"
#include "tarfile.h"
#include "mask-validation-dialog.h"

#define method(x) sigc::mem_fun(*this, &TarFileMaskedImageEditorDialog::x)

const int TarFileMaskedImageEditorDialog::MAX_IMAGES_WIDTH = 1000;

TarFileMaskedImageEditorDialog::TarFileMaskedImageEditorDialog(Gtk::Window &parent, TarFileMaskedImage *mi, double ratio, Glib::ustring empty_str, Shieldset *shieldset)
 : LwEditorDialog(parent, "tar-file-masked-image-editor-dialog.ui")
{
  umgr = new UndoMgr (UndoMgr::DELAY, UndoMgr::LIMIT);
  umgr->execute ().connect (method (executeAction));
  d_mim = new TarFileMaskedImage (*mi);
  d_ratio = ratio;
  d_empty_str = empty_str;
  d_shieldset = shieldset;
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
  xml->get_widget("undo_button", undo_button);
  undo_button->signal_activate ().connect (method (on_undo_activated));
  xml->get_widget("redo_button", redo_button);
  redo_button->signal_activate ().connect (method (on_redo_activated));

  Gtk::Box *box;
  xml->get_widget("shieldset_box", box);
  setup_shield_theme_combobox(box);

  d_target_filename = d_mim->getName ();
  d_orig_target_filename = d_target_filename;
  connect_signals ();
  update ();
}

TarFileMaskedImageEditorDialog::~TarFileMaskedImageEditorDialog()
{
  delete umgr;
  delete d_mim;
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
  if (d_orig_target_filename == d_target_filename)
    d_target_filename = "";
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

  umgr->add (new TarFileMaskedImageEditorAction_Set (d_mim, d_target_filename));
  d_target_filename = selected_filename;
  load_image ();
  update_panel ();
  show_image ();
}

void TarFileMaskedImageEditorDialog::update_panel()
{
  Glib::ustring f = File::get_basename (d_target_filename, true);
  if (f.empty () == false)
    imagebutton->set_label (f);
  else
    {
      if (d_empty_str.empty () == true)
        imagebutton->set_label (_("No image set"));
      else
        imagebutton->set_label (d_empty_str);
      show_image ();
    }
  clear_button->set_visible (d_mim->getImage () != NULL);
  image_neutral->property_visible () = 
    d_mim->getMaskOrientation() == TarFileMaskedImage::HORIZONTAL_MASK;
}

void TarFileMaskedImageEditorDialog::show_image()
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
  image_white->show ();
  if (d_mim->getImage() == NULL)
    return;

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
      PixMask *p;
      switch (d_mim->getMaskOrientation ())
        {
        case TarFileMaskedImage::HORIZONTAL_MASK:
          p = d_mim->applyMask (d_shieldset->getColors (i));
          break;
        case TarFileMaskedImage::VERTICAL_MASK:
          p = d_mim->applyMask (i, d_shieldset->getColors (i));
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
  Gtk::FileChooserDialog *d = image_filechooser(d_mim->getImage () != NULL);
  int response = d->run();
  if (response == Gtk::RESPONSE_ACCEPT && d->get_filename() != "")
    {
      if (ImageFileFilter::getInstance ()->hasInvalidExt (d->get_filename ()))
        ImageFileFilter::getInstance()->showErrorDialog (d);
      else
        {
          if (PixMask::checkFormat (d->get_filename ()))
            {
              /*
               * dog's breakfast here.
               * we want to elide the maskvalidation when we can
               * but we also want to check the dimensions of the incoming
               * image
               * sometimes we have to wait until we have a mask count before
               * we can check the dimensions.
               *
               * we can elide the mask validation when it's a horizontal
               * oriented maskedimage and the width is divisible by height.
               * and also when it's a vertical oriented maskedimage and 
               * the width is fixed by max players, and then the number of
               * rows is divisible by the width divided by 8.
               */
              bool got_masks = false;
              bool bad_dim = false;
              if (d_mim->calculateNumberOfMasks (d->get_filename (), bad_dim) == true)
                got_masks = true;
              if (got_masks == false || bad_dim)
                {
                  if (!bad_dim)
                    {
                      MaskValidationDialog v (*d, d->get_filename (),
                                              d_mim->getMaskOrientation ());
                      int resp = v.run ();
                      if (resp == Gtk::RESPONSE_ACCEPT)
                        {
                          d_mim->setNumMasks (v.get_num_masks ());
                          got_masks = true;
                        }
                    }
                  else
                    got_masks = true; //so we force a checkDimension which fails
                }

              if (got_masks)
                {
                  if (d_mim->checkDimension (d->get_filename ()))
                    {
                      PastChooser::getInstance()->set_dir(d);
                      on_image_chosen (d);
                    }
                  else
                    {
                      TimedMessageDialog
                        td(*d,
                           String::ucompose(_("Bad dimensions in image:\n%1"),
                                            d->get_filename ()), 0);
                      td.run_and_hide ();
                    }
                }
            }
          else
            {
              TimedMessageDialog
                td(*d,
                   String::ucompose(_("Couldn't make sense of the image:\n%1"),
                                    d->get_filename ()), 0);
              td.run_and_hide ();
            }
        }
    }
  else if (response == Gtk::RESPONSE_REJECT)
    {
      d_target_filename = "";
      d_mim->clear ();
      clear_button->activate ();
    }
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
  d_shield_row = default_id;

  box->set_center_widget (*shield_theme_combobox);
}

void TarFileMaskedImageEditorDialog::on_shieldset_changed()
{
  umgr->add (new TarFileMaskedImageEditorAction_Shield (d_shield_row));
  show_image();
  d_shield_row = shield_theme_combobox->get_active_row_number ();
}

void TarFileMaskedImageEditorDialog::on_undo_activated ()
{
  umgr->undo ();
  update ();
  return;
}

void TarFileMaskedImageEditorDialog::on_redo_activated ()
{
  umgr->redo ();
  update ();
}

void TarFileMaskedImageEditorDialog::update ()
{
  disconnect_signals ();
  show_image ();
  update_panel ();
  shield_theme_combobox->set_active (d_shield_row);
  connect_signals ();
}

void TarFileMaskedImageEditorDialog::connect_signals ()
{
  connections.push_back
    (shield_theme_combobox->signal_changed().connect
     (method(on_shieldset_changed)));
}

void TarFileMaskedImageEditorDialog::disconnect_signals ()
{
  for (auto c : connections)
    c.disconnect ();
  connections.clear ();
}

UndoAction *TarFileMaskedImageEditorDialog::executeAction (UndoAction *action2)
{
  TarFileMaskedImageEditorAction *action =
    dynamic_cast<TarFileMaskedImageEditorAction*>(action2);
  UndoAction *out = NULL;

    switch (action->getType ())
      {
      case TarFileMaskedImageEditorAction::SET:
          {
            TarFileMaskedImageEditorAction_Set *a =
              dynamic_cast<TarFileMaskedImageEditorAction_Set*>(action);
            out = new TarFileMaskedImageEditorAction_Set (d_mim,
                                                          d_target_filename);
            a->getTarFileMaskedImage ()->copyFrames (d_mim);
            d_target_filename = a->getFileName ();
          } 
        break;
      case TarFileMaskedImageEditorAction::SHIELD:
          {
            TarFileMaskedImageEditorAction_Shield *a =
              dynamic_cast<TarFileMaskedImageEditorAction_Shield*>(action);
            out = new TarFileMaskedImageEditorAction_Shield
              (shield_theme_combobox->get_active_row_number ());
            d_shield_row = a->getShield ();
          }
        break;
      }
    return out;
}

bool TarFileMaskedImageEditorDialog::installFile (TarFile *t, TarFileMaskedImage *im, Glib::ustring filename)
{
  Glib::ustring newname;
  bool success = false;
  if (d_orig_target_filename.empty () == true)
    success = t->addFileInCfgFile (filename, newname);
  else
    success =
      t->replaceFileInCfgFile (d_orig_target_filename, filename, newname);
  im->setName(newname);
  im->load (t, newname);
  im->instantiateImages();
  return success;
}

bool TarFileMaskedImageEditorDialog::uninstallFile (TarFile *t, TarFileMaskedImage *im)
{
  im->clear ();
  return t->removeFileInCfgFile(d_orig_target_filename);
}
