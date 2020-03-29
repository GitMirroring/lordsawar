//  Copyright (C) 2009, 2010, 2011, 2014, 2020 Ben Asselstine
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

#include "tileset-explosion-picture-editor-dialog.h"

#include "ucompose.hpp"
#include "defs.h"
#include "File.h"
#include "ImageCache.h"
#include "tarhelper.h"
#include "tile-preview-scene.h"
#include "tileset-window.h"
#include "past-chooser.h"
#include "font-size.h"
#include "image-file-filter.h"

#define method(x) sigc::mem_fun(*this, &TilesetExplosionPictureEditorDialog::x)

TilesetExplosionPictureEditorDialog::TilesetExplosionPictureEditorDialog(Gtk::Window &parent, Tileset *tileset)
 : LwEditorDialog(parent, "tileset-explosion-picture-editor-dialog.ui")
{
  d_changed = false;
  d_tileset = tileset;

  xml->get_widget("explosion_imagebutton", explosion_imagebutton);
  explosion_imagebutton->signal_clicked().connect
    (method(on_explosion_imagebutton_clicked));

  xml->get_widget("large_explosion_radiobutton", large_explosion_radiobutton);
  large_explosion_radiobutton->signal_toggled().connect (method(on_large_toggled));
  xml->get_widget("small_explosion_radiobutton", small_explosion_radiobutton);
  small_explosion_radiobutton->signal_toggled().connect (method(on_small_toggled));

  xml->get_widget("scene_image", scene_image);

  Glib::ustring imgname = d_tileset->getExplosionFilename();
  if (imgname.empty() == false)
    {
      bool broken = false;
      Glib::ustring f =
        d_tileset->getFileFromConfigurationFile(imgname);
      d_explosion = PixMask::create (f, broken);
    }
  else
    d_explosion = NULL;
  on_large_toggled();
}

bool TilesetExplosionPictureEditorDialog::run()
{
  dialog->show_all();
  dialog->run();
  dialog->hide ();
  return d_changed;
}

bool TilesetExplosionPictureEditorDialog::on_image_chosen(Gtk::FileChooserDialog *d)
{
  bool broken = false;
  d_explosion = PixMask::create (d->get_filename (), broken);
  if (!broken)
    {
      Glib::ustring imgname = d_tileset->getExplosionFilename();
      Glib::ustring newname = "";
      bool success = false;
      if (imgname.empty() == true)
        success =
          d_tileset->addFileInCfgFile(d->get_filename(), newname);
      else
        success =
          d_tileset->replaceFileInCfgFile(imgname, d->get_filename(), newname);
      if (success)
        {
          d_tileset->setExplosionFilename (newname);
          d_tileset->instantiateExplosionImage();
          d_changed = true;
          update_panel ();
        }
      else
        {
          Glib::ustring errmsg = Glib::strerror(errno);
          Gtk::MessageDialog
            td(*d, String::ucompose(_("Couldn't add %1 to :\n%2\n%3"),
                                    d->get_filename (),
                                    d_tileset->getConfigurationFile(),
                                    errmsg));
          td.run();
          td.hide();
          broken = true;
        }
    }
  else
    {
      Gtk::MessageDialog
        td(*d, String::ucompose(_("Couldn't make sense of the image:\n%1"),
                                d->get_filename ()));
      td.run();
      td.hide();
      broken = true;
    }
  return broken;
}

void TilesetExplosionPictureEditorDialog::on_large_toggled()
{
  update_panel();
}

void TilesetExplosionPictureEditorDialog::on_small_toggled()
{
  update_panel();
}

void TilesetExplosionPictureEditorDialog::update_panel()
{
  Glib::ustring imgname = d_tileset->getExplosionFilename();
  if (imgname.empty() == false)
    {
      explosion_imagebutton->set_label (imgname);
      show_explosion_image();
    }
  else
    {
      explosion_imagebutton->set_label (_("no image set"));
      scene_image->clear();
    }
}

void TilesetExplosionPictureEditorDialog::show_explosion_image()
{
  guint32 size = FontSize::getInstance ()->get_height () *
    EDITOR_DIALOG_TILE_PIC_FONTSIZE_MULTIPLE;
  TilePreviewScene *s;
  Glib::ustring scene;
  guint32 idx = d_tileset->getIndex(Tile::GRASS);
  Tile *grass = NULL;
  if (d_tileset->size() > 0)
    grass = (*d_tileset)[idx];
  scene.clear();
  if (large_explosion_radiobutton->get_active() == true)
    {
      scene += "aaaaaa";
      scene += "aaaaaa";
      scene += "aaaaaa";
      scene += "aaaaaa";
      scene += "aaaaaa";
      scene += "aaaaaa";
      s = new TilePreviewScene(grass, NULL, 6, 6, scene, size);
      update_scene(s);
    }
  else if (small_explosion_radiobutton->get_active() == true)
    {
      scene += "aaaaaaa";
      scene += "aaaaaaa";
      scene += "aaaaaaa";
      scene += "aaaaaaa";
      scene += "aaaaaaa";
      scene += "aaaaaaa";
      scene += "aaaaaaa";
      s = new TilePreviewScene(grass, NULL, 7, 7, scene, size);
      update_scene(s);
    }
}

void TilesetExplosionPictureEditorDialog::update_scene(TilePreviewScene *scene)
{
  if (!d_explosion)
    return;
  if (!scene)
    return;

  Glib::RefPtr<Gdk::Pixbuf> scene_pixbuf;
  scene_pixbuf = scene->renderScene ();
  //center the explosion image on the pixbuf
  //but the large explosion is scaled first

  Glib::RefPtr<Gdk::Pixbuf> explosion;
  if (small_explosion_radiobutton->get_active())
    {
      //explosion = d_explosion->to_pixbuf ();
      PixMask *p = d_explosion->copy ();
      double ratio = EDITOR_DIALOG_TILE_PIC_FONTSIZE_MULTIPLE;
      double new_height = FontSize::getInstance()->get_height () * ratio;
      int new_width =
        ImageCache::calculate_width_from_adjusted_height (p, new_height);
      PixMask::scale (p, new_width, new_height);
      explosion = p->to_pixbuf ();
      delete p;
    }
  else if (large_explosion_radiobutton->get_active())
    {
      PixMask *p = d_explosion->copy ();
      double ratio = EDITOR_DIALOG_TILE_PIC_FONTSIZE_MULTIPLE;
      double new_height = FontSize::getInstance()->get_height () * ratio * 2.0;
      int new_width =
        ImageCache::calculate_width_from_adjusted_height (p, new_height);
      PixMask::scale (p, new_width, new_height);
      explosion = p->to_pixbuf ();
      //guint32 ts = d_tileset->getTileSize ();
      //PixMask::scale (p, ts * 2, ts * 2);
      //explosion = p->to_pixbuf ();
      delete p;
    }

  int i = (scene_pixbuf->get_width() - explosion->get_width()) / 2;
  int j = (scene_pixbuf->get_height() - explosion->get_height()) / 2;
  explosion->composite (scene_pixbuf, i, j, explosion->get_width(), explosion->get_height(), i, j, 1, 1, Gdk::INTERP_NEAREST, 255);
  scene_image->property_pixbuf() = scene_pixbuf;
  scene_image->queue_draw();
}

Gtk::FileChooserDialog* TilesetExplosionPictureEditorDialog::image_filechooser(bool clear)
{
  Glib::ustring filename = "";
  Glib::ustring title = _("Choose an explosion image");
  Gtk::FileChooserDialog *d = new Gtk::FileChooserDialog(*dialog, title);
  ImageFileFilter::getInstance ()->add (d);
  d->add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
  d->add_button(Gtk::Stock::OPEN, Gtk::RESPONSE_ACCEPT);
  if (clear)
    d->add_button(Gtk::Stock::CLEAR, Gtk::RESPONSE_REJECT);
  d->set_default_response(Gtk::RESPONSE_ACCEPT);
  d->set_current_folder(PastChooser::getInstance()->get_dir(d));
  return d;
}

void TilesetExplosionPictureEditorDialog::on_explosion_imagebutton_clicked ()
{
  Glib::ustring f = d_tileset->getExplosionFilename ();
  Glib::ustring filename = "";
  Gtk::FileChooserDialog *d = image_filechooser(f != "");
  if (f != "")
    filename = d_tileset->getFileFromConfigurationFile(f);
  int response = d->run();
  if (filename != "")
    File::erase(filename);
  if (response == Gtk::RESPONSE_ACCEPT && d->get_filename() != "")
    {
      if (ImageFileFilter::getInstance ()->hasInvalidExt (d->get_filename ()))
        ImageFileFilter::getInstance()->showErrorDialog (d);
      else
        {
          if (d->get_filename() != filename)
            {
              PastChooser::getInstance()->set_dir(d);
              on_image_chosen (d);
            }
        }
    }
  else if (response == Gtk::RESPONSE_REJECT && f != "")
    {
      if (d_tileset->removeFileInCfgFile(f))
        {
          d_changed = true;
          d_tileset->uninstantiateSameNamedImages (f);
          if (d_explosion)
            {
              delete d_explosion;
              d_explosion = NULL;
            }
          update_panel ();
        }
      else
        {
          Glib::ustring errmsg = Glib::strerror(errno);
          Gtk::MessageDialog
            td(*d, String::ucompose(_("Couldn't remove %1 from:\n%2\n%3"),
                                    f, d_tileset->getConfigurationFile(),
                                    errmsg));
          td.run();
          td.hide();
        }
    }
  d->hide();
  delete d;
}

TilesetExplosionPictureEditorDialog::~TilesetExplosionPictureEditorDialog ()
{
  if (d_explosion)
    delete d_explosion;
}
