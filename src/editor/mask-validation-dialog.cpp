//  Copyright (C) 2021 Ben Asselstine
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

#include "mask-validation-dialog.h"
#include "gui/image-helpers.h"
#include "ucompose.hpp"
#include "defs.h"
#include "File.h"
#include "shieldsetlist.h"
#include "past-chooser.h"
#include "font-size.h"
#include "ImageCache.h"
#include "image-file-filter.h"
#include "timed-message-dialog.h"
#include "TarFileMaskedImage.h"
#include "mask-validation-actions.h"

#define method(x) sigc::mem_fun(*this, &MaskValidationDialog::x)

MaskValidationDialog::MaskValidationDialog(Gtk::Window &parent,
                                           Glib::ustring filename,
                                           TarFileMaskedImage::MaskOrientation o)
 : LwEditorDialog(parent, "mask-validation-dialog.ui"), d_filename (filename),
    d_orientation (o)
{
  umgr = new UndoMgr (UndoMgr::DELAY, UndoMgr::LIMIT);
  umgr->execute ().connect (method (executeAction));

  xml->get_widget("image", image);
  xml->get_widget("undo_button", undo_button);
  xml->get_widget("redo_button", redo_button);
  xml->get_widget("masks_spinbutton", masks_spinbutton);

  bool broken = false;
  masked_image = PixMask::create (filename, broken);
  d_num_masks = 1;
  connect_signals ();
  update ();
}

MaskValidationDialog::~MaskValidationDialog()
{
  delete umgr;
  delete masked_image;
}

int MaskValidationDialog::run()
{
  return dialog->run ();
}

void MaskValidationDialog::hide ()
{
  dialog->hide();
}

Glib::RefPtr<Gdk::Pixbuf> MaskValidationDialog::draw_lines_for_vertical ()
{
  //we need to draw horizontal lines
  int width = masked_image->get_unscaled_width ();
  int height = masked_image->get_unscaled_height ();
  Cairo::RefPtr<Cairo::Surface> surface = Cairo::ImageSurface::create (Cairo::FORMAT_ARGB32, width, height);
  masked_image->blit (surface, 0, 0);
  Cairo::RefPtr<Cairo::Context> cr = Cairo::Context::create(surface);
  cr->set_source_rgb (1.0, 0, 0);
  cr->set_line_width (1.0);
  guint32 row_height = height / (d_num_masks + 1);
  for (guint32 i = 0; i < d_num_masks; i++)
    {
      cr->move_to (0, (i + 1) * row_height);
      cr->line_to (width, (i + 1) * row_height);
      cr->stroke ();
    }


  Glib::RefPtr<Gdk::Pixbuf> pixbuf =
    Gdk::Pixbuf::create(surface, 0, 0, width, height);
  return pixbuf;
}

Glib::RefPtr<Gdk::Pixbuf> MaskValidationDialog::draw_lines_for_horizontal ()
{
  //we need to draw vertical lines

  int width = masked_image->get_unscaled_width ();
  int height = masked_image->get_unscaled_height ();
  Cairo::RefPtr<Cairo::Surface> surface = Cairo::ImageSurface::create (Cairo::FORMAT_ARGB32, width, height);
  masked_image->blit (surface, 0, 0);
  Cairo::RefPtr<Cairo::Context> cr = Cairo::Context::create(surface);
  cr->set_source_rgb (1.0, 0, 0);
  cr->set_line_width (1.0);

  guint32 column_width = width / (d_num_masks + 1);
  for (guint32 i = 0; i < d_num_masks; i++)
    {
      cr->move_to ((i + 1) * column_width, 0);
      cr->line_to ((i + 1) * column_width, height);
      cr->stroke ();
    }

  Glib::RefPtr<Gdk::Pixbuf> pixbuf =
    Gdk::Pixbuf::create(surface, 0, 0, width, height);
  return pixbuf;
}

void MaskValidationDialog::show_image()
{
  Glib::RefPtr<Gdk::Pixbuf> pixbuf;
  if (d_orientation == TarFileMaskedImage::HORIZONTAL_MASK)
    pixbuf = draw_lines_for_horizontal ();
  else if (d_orientation == TarFileMaskedImage::VERTICAL_MASK)
    pixbuf = draw_lines_for_vertical ();

  PixMask *p = PixMask::create (pixbuf);

  double ratio = 10;
  int font_size = FontSize::getInstance()->get_height ();
  double new_height = font_size * ratio;
  double new_width =
    ImageCache::calculate_width_from_adjusted_height (p, new_height);
  if (new_width > 900)
    {
      double perc = 900 / new_width;
      new_width *= perc;
      new_height *= perc;
    }
  PixMask::scale (p, new_width, new_height, Gdk::INTERP_TILES);
  image->property_pixbuf () = p->to_pixbuf ();
  delete p;
}

void MaskValidationDialog::on_masks_text_changed()
{
  masks_spinbutton->set_value(atoi(masks_spinbutton->get_text().c_str()));
  on_masks_changed();
}

void MaskValidationDialog::on_masks_changed()
{
  umgr->add (new MaskValidationAction_Masks (d_num_masks));
  d_num_masks = guint32 (masks_spinbutton->get_value ());
  update ();
}

void MaskValidationDialog::on_undo_activated ()
{
  umgr->undo ();
  update ();
  return;
}

void MaskValidationDialog::on_redo_activated ()
{
  umgr->redo ();
  update ();
}

void MaskValidationDialog::update ()
{
  disconnect_signals ();
  masks_spinbutton->set_value(d_num_masks);
  show_image ();
  connect_signals ();
}

void MaskValidationDialog::connect_signals ()
{
  connections.push_back
    (undo_button->signal_activate ().connect (method (on_undo_activated)));
  connections.push_back
    (redo_button->signal_activate ().connect (method (on_redo_activated)));
  connections.push_back
    (masks_spinbutton->signal_insert_text().connect
     (sigc::hide(sigc::hide(method(on_masks_text_changed)))));
}

void MaskValidationDialog::disconnect_signals ()
{
  for (auto c : connections)
    c.disconnect ();
  connections.clear ();
}

UndoAction *MaskValidationDialog::executeAction (UndoAction *action2)
{
  MaskValidationAction *action = dynamic_cast<MaskValidationAction*>(action2);
  UndoAction *out = NULL;

    switch (action->getType ())
      {
      case MaskValidationAction::MASKS:
          {
            MaskValidationAction_Masks *a =
              dynamic_cast<MaskValidationAction_Masks*>(action);
            out = new MaskValidationAction_Masks (d_num_masks);
            d_num_masks = a->getNumMasks ();
          } 
        break;
      }
    return out;
}
