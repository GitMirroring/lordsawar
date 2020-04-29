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

#define method(x) sigc::mem_fun(*this, &TilesetMoveBonusImageDialog::x)

TilesetMoveBonusImageDialog::TilesetMoveBonusImageDialog(Gtk::Window &parent, Tileset *tileset)
 : LwEditorDialog(parent, "tileset-move-bonus-image-dialog.ui")
{
  d_tileset = tileset;

  xml->get_widget("all_imagechooser_button", all_imagechooser_button);
  all_imagechooser_button->signal_clicked ().connect (method (on_all_clicked));
  xml->get_widget("water_imagechooser_button", water_imagechooser_button);
  water_imagechooser_button->signal_clicked ().connect
    (method (on_water_clicked));
  xml->get_widget("forest_imagechooser_button", forest_imagechooser_button);
  forest_imagechooser_button->signal_clicked ().connect
    (method (on_forest_clicked));
  xml->get_widget("hills_imagechooser_button", hills_imagechooser_button);
  hills_imagechooser_button->signal_clicked ().connect
    (method (on_hills_clicked));
  xml->get_widget("mountains_imagechooser_button",
                  mountains_imagechooser_button);
  mountains_imagechooser_button->signal_clicked ().connect
    (method (on_mountains_clicked));
  xml->get_widget("swamp_imagechooser_button", swamp_imagechooser_button);
  swamp_imagechooser_button->signal_clicked ().connect
    (method (on_swamp_clicked));

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

void TilesetMoveBonusImageDialog::on_water_clicked ()
{
  PixMask *i = d_tileset->getWaterMoveBonusImage ();
  on_image_button_activated
    (sigc::mem_fun (d_tileset, &Tileset::getWaterMoveBonusFilename),
     sigc::mem_fun (d_tileset, &Tileset::setWaterMoveBonusFilename), i);
  if (d_tileset->getWaterMoveBonusFilename().empty () == false)
    d_tileset->instantiateWaterMoveBonusImage (d_tileset);
  else
    d_tileset->clearWaterMoveBonusImage();
  update_preview ();
}

void TilesetMoveBonusImageDialog::on_forest_clicked ()
{
  PixMask *i = d_tileset->getForestMoveBonusImage ();
  on_image_button_activated
    (sigc::mem_fun (d_tileset, &Tileset::getForestMoveBonusFilename),
     sigc::mem_fun (d_tileset, &Tileset::setForestMoveBonusFilename), i);
  if (d_tileset->getForestMoveBonusFilename().empty () == false)
    d_tileset->instantiateForestMoveBonusImage (d_tileset);
  else
    d_tileset->clearForestMoveBonusImage();
  update_preview ();
}

void TilesetMoveBonusImageDialog::on_hills_clicked ()
{
  PixMask *i = d_tileset->getHillsMoveBonusImage ();
  on_image_button_activated
    (sigc::mem_fun (d_tileset, &Tileset::getHillsMoveBonusFilename),
     sigc::mem_fun (d_tileset, &Tileset::setHillsMoveBonusFilename), i);
  if (d_tileset->getHillsMoveBonusFilename().empty () == false)
    d_tileset->instantiateHillsMoveBonusImage (d_tileset);
  else
    d_tileset->clearHillsMoveBonusImage();
  update_preview ();
}

void TilesetMoveBonusImageDialog::on_mountains_clicked ()
{
  PixMask *i = d_tileset->getMountainsMoveBonusImage ();
  on_image_button_activated
    (sigc::mem_fun (d_tileset, &Tileset::getMountainsMoveBonusFilename),
     sigc::mem_fun (d_tileset, &Tileset::setMountainsMoveBonusFilename), i);
  if (d_tileset->getMountainsMoveBonusFilename().empty () == false)
    d_tileset->instantiateMountainsMoveBonusImage (d_tileset);
  else
    d_tileset->clearMountainsMoveBonusImage();
  update_preview ();
}

void TilesetMoveBonusImageDialog::on_swamp_clicked ()
{
  PixMask *i = d_tileset->getSwampMoveBonusImage ();
  on_image_button_activated
    (sigc::mem_fun (d_tileset, &Tileset::getSwampMoveBonusFilename),
     sigc::mem_fun (d_tileset, &Tileset::setSwampMoveBonusFilename), i);
  if (d_tileset->getSwampMoveBonusFilename().empty () == false)
    d_tileset->instantiateSwampMoveBonusImage (d_tileset);
  else
    d_tileset->clearSwampMoveBonusImage();
  update_preview ();
}

void TilesetMoveBonusImageDialog::on_all_clicked ()
{
  PixMask *i = d_tileset->getAllMoveBonusImage ();
  on_image_button_activated
    (sigc::mem_fun (d_tileset, &Tileset::getAllMoveBonusFilename),
     sigc::mem_fun (d_tileset, &Tileset::setAllMoveBonusFilename), i);
  if (d_tileset->getAllMoveBonusFilename().empty () == false)
    d_tileset->instantiateAllMoveBonusImage (d_tileset);
  else
    d_tileset->clearAllMoveBonusImage();
  update_preview ();
}

TilesetMoveBonusImageDialog::~TilesetMoveBonusImageDialog ()
{
  notebook->property_show_tabs () = false;
}

void TilesetMoveBonusImageDialog::update_button_names ()
{
  Glib::ustring no_img_set = _("No image set");

  Glib::ustring image = d_tileset->getAllMoveBonusFilename ();
  if (image.empty () == false)
    all_imagechooser_button->set_label (image);
  else
    all_imagechooser_button->set_label (no_img_set);

  image = d_tileset->getWaterMoveBonusFilename ();
  if (image.empty () == false)
    water_imagechooser_button->set_label (image);
  else
    water_imagechooser_button->set_label (no_img_set);

  image = d_tileset->getForestMoveBonusFilename ();
  if (image.empty () == false)
    forest_imagechooser_button->set_label (image);
  else
    forest_imagechooser_button->set_label (no_img_set);

  image = d_tileset->getHillsMoveBonusFilename ();
  if (image.empty () == false)
    hills_imagechooser_button->set_label (image);
  else
    hills_imagechooser_button->set_label (no_img_set);

  image = d_tileset->getMountainsMoveBonusFilename ();
  if (image.empty () == false)
    mountains_imagechooser_button->set_label (image);
  else
    mountains_imagechooser_button->set_label (no_img_set);

  image = d_tileset->getSwampMoveBonusFilename ();
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

void TilesetMoveBonusImageDialog::on_image_button_activated(sigc::slot<Glib::ustring> getName,
                                                            sigc::slot<void,Glib::ustring> setName, PixMask *im)
{
  TarFile *t = d_tileset;
  Glib::ustring imgname = getName ();
  std::vector<PixMask*> frames;
  if (im)
    frames.push_back (im);
  ImageEditorDialog d (*dialog, imgname, 1, frames, 0);
  int response = d.run();

  if (response == Gtk::RESPONSE_ACCEPT)
    {
      if (d.get_filename () != "")
        {
          Glib::ustring newname = "";
          bool success = false;
          if (getName() == "")
            success = t->addFileInCfgFile(d.get_filename (), newname);
          else
            success = t->replaceFileInCfgFile(imgname, d.get_filename (),
                                              newname);
          if (success)
            {
              setName(newname);
              d_changed = true;
              update_button_names ();
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
              setName ("");
              d_changed = true;
              update_button_names ();
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
