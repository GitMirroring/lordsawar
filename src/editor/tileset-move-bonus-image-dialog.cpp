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
  umgr = new UndoMgr (UndoMgr::DELAY, UndoMgr::LIMIT);
  umgr->execute ().connect (method (executeAction));
  d_tileset = tileset;

  xml->get_widget("all_imagechooser_button", all_imagechooser_button);
  xml->get_widget("water_imagechooser_button", water_imagechooser_button);
  xml->get_widget("forest_imagechooser_button", forest_imagechooser_button);
  xml->get_widget("hills_imagechooser_button", hills_imagechooser_button);
  xml->get_widget("mountains_imagechooser_button",
                  mountains_imagechooser_button);
  xml->get_widget("swamp_imagechooser_button", swamp_imagechooser_button);

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
  xml->get_widget("undo_button", undo_button);
  undo_button->signal_activate ().connect (method (on_undo_activated));
  xml->get_widget("redo_button", redo_button);
  redo_button->signal_activate ().connect (method (on_redo_activated));

  d_changed = false;
  connect_signals ();
  update ();
}

TilesetMoveBonusImageDialog::~TilesetMoveBonusImageDialog ()
{
  delete umgr;
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

void TilesetMoveBonusImageDialog::on_all_image_button_clicked (TarFileImage *im)
{
  TileSetMoveBonusImageAction_All *action =
    new TileSetMoveBonusImageAction_All (d_tileset, im->getName ());
  if (on_image_button_clicked (im))
    umgr->add (action);
  else
    delete action;
}

void TilesetMoveBonusImageDialog::on_water_image_button_clicked (TarFileImage *im)
{
  TileSetMoveBonusImageAction_Water *action =
    new TileSetMoveBonusImageAction_Water (d_tileset, im->getName ());
  if (on_image_button_clicked (im))
    umgr->add (action);
  else
    delete action;
}

void TilesetMoveBonusImageDialog::on_forest_image_button_clicked (TarFileImage *im)
{
  TileSetMoveBonusImageAction_Forest *action =
    new TileSetMoveBonusImageAction_Forest (d_tileset, im->getName ());
  if (on_image_button_clicked (im))
    umgr->add (action);
  else
    delete action;
}

void TilesetMoveBonusImageDialog::on_hills_image_button_clicked (TarFileImage *im)
{
  TileSetMoveBonusImageAction_Hills *action =
    new TileSetMoveBonusImageAction_Hills (d_tileset, im->getName ());
  if (on_image_button_clicked (im))
    umgr->add (action);
  else
    delete action;
}

void TilesetMoveBonusImageDialog::on_mountains_image_button_clicked (TarFileImage *im)
{
  TileSetMoveBonusImageAction_Mountains *action =
    new TileSetMoveBonusImageAction_Mountains (d_tileset, im->getName ());
  if (on_image_button_clicked (im))
    umgr->add (action);
  else
    delete action;
}

void TilesetMoveBonusImageDialog::on_swamp_image_button_clicked (TarFileImage *im)
{
  TileSetMoveBonusImageAction_Swamp *action =
    new TileSetMoveBonusImageAction_Swamp (d_tileset, im->getName ());
  if (on_image_button_clicked (im))
    umgr->add (action);
  else
    delete action;
}

bool TilesetMoveBonusImageDialog::on_image_button_clicked (TarFileImage *im)
{
  bool ret = false;
  TarFile *t = d_tileset;
  Glib::ustring imgname = im->getName ();
  ImageEditorDialog d (*dialog, im, 0);
  int response = d.run();

  if (response == Gtk::RESPONSE_ACCEPT)
    {
      if (d.get_filename () != "")
        {
          bool success = d.installFile (d_tileset, im, d.get_filename ());
          if (success)
            {
              d_changed = true;
              update ();
              ret = true;
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
          if (d.uninstallFile (d_tileset, im))
            {
              d_changed = true;
              update ();
              ret = true;
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
  return ret;
}

void TilesetMoveBonusImageDialog::on_undo_activated ()
{
  umgr->undo ();
  if (umgr->undoEmpty ())
    d_changed = false;
  update ();
  return;
}

void TilesetMoveBonusImageDialog::on_redo_activated ()
{
  umgr->redo ();
  d_changed = true;
  update ();
}

void TilesetMoveBonusImageDialog::update ()
{
  disconnect_signals ();
  update_button_names ();
  update_preview ();
  connect_signals ();
}

void TilesetMoveBonusImageDialog::connect_signals ()
{
  connections.push_back
    (all_imagechooser_button->signal_clicked ().connect
     (sigc::bind (method (on_all_image_button_clicked),
                  d_tileset->getAllMoveBonus ())));
  connections.push_back
    (water_imagechooser_button->signal_clicked ().connect
     (sigc::bind (method (on_water_image_button_clicked),
                  d_tileset->getWaterMoveBonus ())));
  connections.push_back
    (forest_imagechooser_button->signal_clicked ().connect
     (sigc::bind (method (on_forest_image_button_clicked),
                  d_tileset->getForestMoveBonus ())));
  connections.push_back
    (hills_imagechooser_button->signal_clicked ().connect
     (sigc::bind (method (on_hills_image_button_clicked),
                  d_tileset->getHillsMoveBonus ())));
  connections.push_back
    (mountains_imagechooser_button->signal_clicked ().connect
     (sigc::bind (method (on_mountains_image_button_clicked),
                  d_tileset->getMountainsMoveBonus ())));
  connections.push_back
    (swamp_imagechooser_button->signal_clicked ().connect
     (sigc::bind (method (on_swamp_image_button_clicked),
                  d_tileset->getSwampMoveBonus ())));
}

void TilesetMoveBonusImageDialog::disconnect_signals ()
{
  for (auto c : connections)
    c.disconnect ();
  connections.clear ();
}

UndoAction *TilesetMoveBonusImageDialog::executeAction (UndoAction *action2)
{
  TileSetMoveBonusImageAction *action =
    dynamic_cast<TileSetMoveBonusImageAction*>(action2);
  UndoAction *out = NULL;

  switch (action->getType ())
    {
    case TileSetMoveBonusImageAction::ALL:
        {
          TileSetMoveBonusImageAction_All *a =
            dynamic_cast<TileSetMoveBonusImageAction_All*>(action);
          TarFileImage *im = d_tileset->getAllMoveBonus ();
          out = new TileSetMoveBonusImageAction_All (d_tileset,
                                                     im->getName ());
          doUpdateImage (a, im);
        }
      break;
    case TileSetMoveBonusImageAction::WATER:
        {
          TileSetMoveBonusImageAction_Water *a =
            dynamic_cast<TileSetMoveBonusImageAction_Water*>(action);
          TarFileImage *im = d_tileset->getWaterMoveBonus ();
          out = new TileSetMoveBonusImageAction_Water (d_tileset,
                                                       im->getName ());
          doUpdateImage (a, im);
        }
      break;
    case TileSetMoveBonusImageAction::FOREST:
        {
          TileSetMoveBonusImageAction_Forest *a =
            dynamic_cast<TileSetMoveBonusImageAction_Forest*>(action);
          TarFileImage *im = d_tileset->getForestMoveBonus ();
          out = new TileSetMoveBonusImageAction_Forest (d_tileset,
                                                        im->getName ());
          doUpdateImage (a, im);
        }
      break;
    case TileSetMoveBonusImageAction::HILLS:
        {
          TileSetMoveBonusImageAction_Hills *a =
            dynamic_cast<TileSetMoveBonusImageAction_Hills*>(action);
          TarFileImage *im = d_tileset->getHillsMoveBonus ();
          out = new TileSetMoveBonusImageAction_Hills (d_tileset,
                                                       im->getName ());
          doUpdateImage (a, im);
        }
      break;
    case TileSetMoveBonusImageAction::MOUNTAINS:
        {
          TileSetMoveBonusImageAction_Mountains *a =
            dynamic_cast<TileSetMoveBonusImageAction_Mountains*>(action);
          TarFileImage *im = d_tileset->getMountainsMoveBonus ();
          out =
            new TileSetMoveBonusImageAction_Mountains (d_tileset,
                                                       im->getName ());
          doUpdateImage (a, im);
        }
      break;
    case TileSetMoveBonusImageAction::SWAMP:
        {
          TileSetMoveBonusImageAction_Swamp *a =
            dynamic_cast<TileSetMoveBonusImageAction_Swamp*>(action);
          TarFileImage *im = d_tileset->getSwampMoveBonus ();
          out = new TileSetMoveBonusImageAction_Swamp (d_tileset,
                                                       im->getName ());
          doUpdateImage (a, im);
        }
      break;
    }
  return out;
}

void TilesetMoveBonusImageDialog::doUpdateImage (TileSetMoveBonusImageAction_Set *a, TarFileImage *im)
{
  if (a->getArchiveMember ().empty ())
    im->clear ();
  else
    {
      Glib::ustring ar = a->getArchiveMember ();
      Glib::ustring file = a->getFileName ();
      bool broken = false;
      Glib::ustring newbasename = "";
      bool present = d_tileset->contains (ar, broken);
      if (present)
        d_tileset->replaceFileInCfgFile (ar, file, newbasename);
      else
        d_tileset->addFileInCfgFile (file, newbasename);

      im->load (d_tileset, newbasename);
      im->instantiateImages ();
    }
}
