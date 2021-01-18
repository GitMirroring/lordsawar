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

#include <sigc++/functors/mem_fun.h>

#include "tileset-move-bonus-image-dialog.h"

#include "ucompose.hpp"
#include "File.h"
#include "defs.h"
#include "tileset.h"
#include "ImageCache.h"
#include "PixMask.h"
#include "image-editor-dialog.h"
#include "timed-message-dialog.h"
#include "font-size.h"
#include "TarFileImage.h"

#define method(x) sigc::mem_fun(*this, &TilesetMoveBonusImageDialog::x)

TilesetMoveBonusImageDialog::TilesetMoveBonusImageDialog(Gtk::Window &parent, Tileset *tileset)
 : LwEditorDialog(parent, "tileset-move-bonus-image-dialog.ui")
{
  d_tileset = tileset;

  xml->get_widget("all_imagechooser_button", all_imagechooser_button);
  all_imagechooser_button->signal_clicked ().connect
    (sigc::bind (method (on_image_button_clicked),
                 d_tileset->getAllMoveBonus ()));
  xml->get_widget("water_imagechooser_button", water_imagechooser_button);
  water_imagechooser_button->signal_clicked ().connect
    (sigc::bind (method (on_image_button_clicked),
                 d_tileset->getWaterMoveBonus ()));
  xml->get_widget("forest_imagechooser_button", forest_imagechooser_button);
  forest_imagechooser_button->signal_clicked ().connect
    (sigc::bind (method (on_image_button_clicked),
                 d_tileset->getForestMoveBonus ()));
  xml->get_widget("hills_imagechooser_button", hills_imagechooser_button);
  hills_imagechooser_button->signal_clicked ().connect
    (sigc::bind (method (on_image_button_clicked),
                 d_tileset->getHillsMoveBonus ()));
  xml->get_widget("mountains_imagechooser_button",
                  mountains_imagechooser_button);
  mountains_imagechooser_button->signal_clicked ().connect
    (sigc::bind (method (on_image_button_clicked),
                 d_tileset->getMountainsMoveBonus ()));
  xml->get_widget("swamp_imagechooser_button", swamp_imagechooser_button);
  swamp_imagechooser_button->signal_clicked ().connect
    (sigc::bind (method (on_image_button_clicked),
                 d_tileset->getSwampMoveBonus ()));

  xml->get_widget("notebook", notebook);

  xml->get_widget("all_image", all_image);
  xml->get_widget("water_image", water_image);
  xml->get_widget("forest_image", forest_image);
  xml->get_widget("hills_image", hills_image);
  xml->get_widget("mountains_image", mountains_image);
  xml->get_widget("swamp_image", swamp_image);
  xml->get_widget("forest_hills_image", forest_hills_image);
  xml->get_widget("forest_mountains_image", forest_mountains_image);
  xml->get_widget("forest_swamp_image", forest_swamp_image);
  xml->get_widget("hills_mountains_image", hills_mountains_image);
  xml->get_widget("hills_swamp_image", hills_swamp_image);
  xml->get_widget("mountains_swamp_image", mountains_swamp_image);
  xml->get_widget("forest_hills_mountains_image", forest_hills_mountains_image);
  xml->get_widget("forest_hills_swamp_image", forest_hills_swamp_image);
  xml->get_widget("forest_mountains_swamp_image", forest_mountains_swamp_image);
  xml->get_widget("hills_mountains_swamp_image", hills_mountains_swamp_image);
  xml->get_widget("forest_hills_mountains_swamp_image", 
                  forest_hills_mountains_swamp_image);
  update_button_names ();
  update_preview ();
}

TilesetMoveBonusImageDialog::~TilesetMoveBonusImageDialog ()
{
  notebook->property_show_tabs () = false;
}

void TilesetMoveBonusImageDialog::update_button_names ()
{
  Glib::ustring no_img_set = _("No image set");

  Glib::ustring image = d_tileset->getAllMoveBonus()->getName ();
  if (image.empty () == false)
    all_imagechooser_button->set_label (image);
  else
    all_imagechooser_button->set_label (no_img_set);

  image = d_tileset->getWaterMoveBonus()->getName ();
  if (image.empty () == false)
    water_imagechooser_button->set_label (image);
  else
    water_imagechooser_button->set_label (no_img_set);

  image = d_tileset->getForestMoveBonus()->getName ();
  if (image.empty () == false)
    forest_imagechooser_button->set_label (image);
  else
    forest_imagechooser_button->set_label (no_img_set);

  image = d_tileset->getHillsMoveBonus()->getName ();
  if (image.empty () == false)
    hills_imagechooser_button->set_label (image);
  else
    hills_imagechooser_button->set_label (no_img_set);

  image = d_tileset->getMountainsMoveBonus()->getName ();
  if (image.empty () == false)
    mountains_imagechooser_button->set_label (image);
  else
    mountains_imagechooser_button->set_label (no_img_set);

  image = d_tileset->getSwampMoveBonus()->getName ();
  if (image.empty () == false)
    swamp_imagechooser_button->set_label (image);
  else
    swamp_imagechooser_button->set_label (no_img_set);
}

void TilesetMoveBonusImageDialog::update_preview ()
{
  PixMask *p;
  guint32 f = FontSize::getInstance ()->get_height ();
  double r = DIALOG_MOVE_BONUS_PIC_FONTSIZE_MULTIPLE  * 2;

  guint32 bonus =
    Tile::WATER | Tile::FOREST | Tile::HILLS | Tile::MOUNTAIN | Tile::SWAMP;
  p = MoveBonusPixMaskCacheItem::getMoveBonusPic(d_tileset, bonus, f, r);
  all_image->property_pixbuf () = p->to_pixbuf ();
  delete p;

  bonus = Tile::WATER;
  p = MoveBonusPixMaskCacheItem::getMoveBonusPic(d_tileset, bonus, f, r);
  water_image->property_pixbuf () = p->to_pixbuf ();
  delete p;

  bonus = Tile::FOREST;
  p = MoveBonusPixMaskCacheItem::getMoveBonusPic(d_tileset, bonus, f, r);
  forest_image->property_pixbuf () = p->to_pixbuf ();
  delete p;

  bonus = Tile::HILLS;
  p = MoveBonusPixMaskCacheItem::getMoveBonusPic(d_tileset, bonus, f, r);
  hills_image->property_pixbuf () = p->to_pixbuf ();
  delete p;

  bonus = Tile::MOUNTAIN;
  p = MoveBonusPixMaskCacheItem::getMoveBonusPic(d_tileset, bonus, f, r);
  mountains_image->property_pixbuf () = p->to_pixbuf ();
  delete p;

  bonus = Tile::SWAMP;
  p = MoveBonusPixMaskCacheItem::getMoveBonusPic(d_tileset, bonus, f, r);
  swamp_image->property_pixbuf () = p->to_pixbuf ();
  delete p;

  bonus = Tile::FOREST | Tile::HILLS;
  p = MoveBonusPixMaskCacheItem::getMoveBonusPic(d_tileset, bonus, f, r);
  forest_hills_image->property_pixbuf () = p->to_pixbuf ();
  delete p;

  bonus = Tile::FOREST | Tile::MOUNTAIN;
  p = MoveBonusPixMaskCacheItem::getMoveBonusPic(d_tileset, bonus, f, r);
  forest_mountains_image->property_pixbuf () = p->to_pixbuf ();
  delete p;

  bonus = Tile::FOREST | Tile::SWAMP;
  p = MoveBonusPixMaskCacheItem::getMoveBonusPic(d_tileset, bonus, f, r);
  forest_swamp_image->property_pixbuf () = p->to_pixbuf ();
  delete p;

  bonus = Tile::HILLS | Tile::MOUNTAIN;
  p = MoveBonusPixMaskCacheItem::getMoveBonusPic(d_tileset, bonus, f, r);
  hills_mountains_image->property_pixbuf () = p->to_pixbuf ();
  delete p;

  bonus = Tile::HILLS | Tile::SWAMP;
  p = MoveBonusPixMaskCacheItem::getMoveBonusPic(d_tileset, bonus, f, r);
  hills_swamp_image->property_pixbuf () = p->to_pixbuf ();
  delete p;

  bonus = Tile::MOUNTAIN | Tile::SWAMP;
  p = MoveBonusPixMaskCacheItem::getMoveBonusPic(d_tileset, bonus, f, r);
  mountains_swamp_image->property_pixbuf () = p->to_pixbuf ();
  delete p;

  bonus = Tile::FOREST | Tile::HILLS | Tile::MOUNTAIN;
  p = MoveBonusPixMaskCacheItem::getMoveBonusPic(d_tileset, bonus, f, r);
  forest_hills_mountains_image->property_pixbuf () = p->to_pixbuf ();
  delete p;

  bonus = Tile::FOREST | Tile::HILLS | Tile::SWAMP;
  p = MoveBonusPixMaskCacheItem::getMoveBonusPic(d_tileset, bonus, f, r);
  forest_hills_swamp_image->property_pixbuf () = p->to_pixbuf ();
  delete p;

  bonus = Tile::FOREST | Tile::MOUNTAIN | Tile::SWAMP;
  p = MoveBonusPixMaskCacheItem::getMoveBonusPic(d_tileset, bonus, f, r);
  forest_mountains_swamp_image->property_pixbuf () = p->to_pixbuf ();
  delete p;

  bonus = Tile::HILLS | Tile::MOUNTAIN | Tile::SWAMP;
  p = MoveBonusPixMaskCacheItem::getMoveBonusPic(d_tileset, bonus, f, r);
  hills_mountains_swamp_image->property_pixbuf () = p->to_pixbuf ();
  delete p;

  bonus = Tile::FOREST | Tile::HILLS | Tile::MOUNTAIN | Tile::SWAMP;
  p = MoveBonusPixMaskCacheItem::getMoveBonusPic(d_tileset, bonus, f, r);
  forest_hills_mountains_swamp_image->property_pixbuf () = p->to_pixbuf ();
  delete p;
}

void TilesetMoveBonusImageDialog::on_image_button_clicked (TarFileImage *im)
{
  TarFile *t = d_tileset;
  Glib::ustring imgname = im->getName ();
  ImageEditorDialog d (*dialog, im, 0);
  int response = d.run();

  if (response == Gtk::RESPONSE_ACCEPT)
    {
      if (d.get_filename () != "")
        {
          Glib::ustring newname = "";
          bool success = false;
          if (im->getName() == "")
            success = t->addFileInCfgFile(d.get_filename (), newname);
          else
            success = t->replaceFileInCfgFile(imgname, d.get_filename (),
                                              newname);
          if (success)
            {
              im->load (d_tileset, newname);
              im->instantiateImages ();
              d_changed = true;
              update_button_names ();
              update_preview ();
            }
          else
            {
              Glib::ustring errmsg = Glib::strerror(errno);
              TimedMessageDialog
                td(*d.get_dialog (),
                   String::ucompose(_("Couldn't add %1 to :\n%2\n%3"),
                                    d.get_filename (),
                                    t->getConfigurationFile(), errmsg), 0);
              td.run_and_hide ();
            }
        }
    }
  else if (response == Gtk::RESPONSE_REJECT)
    {
      if (imgname.empty () == false)
        {
          if (t->removeFileInCfgFile(imgname))
            {
              im->clear();
              d_changed = true;
              update_button_names ();
              update_preview ();
            }
          else
            {
              Glib::ustring errmsg = Glib::strerror(errno);
              TimedMessageDialog
                td(*d.get_dialog (),
                   String::ucompose(_("Couldn't remove %1 from:\n%2\n%3"),
                                    imgname,
                                    t->getConfigurationFile(),
                                    errmsg), 0);
              td.run_and_hide ();
            }
        }
    }
  d.hide();
}
