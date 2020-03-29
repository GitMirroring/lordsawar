//  Copyright (C) 2008, 2009, 2010, 2014, 2015, 2020 Ben Asselstine
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

#include "tile-preview-dialog.h"

#include "gui/input-helpers.h"
#include "ucompose.hpp"
#include "defs.h"
#include "File.h"
#include "tilestyle.h"
#include "font-size.h"

#define method(x) sigc::mem_fun(*this, &TilePreviewDialog::x)

TilePreviewDialog::TilePreviewDialog(Gtk::Window &parent, Tile *tile, Tile *sec)
 : LwEditorDialog(parent, "tile-preview-dialog.ui")
{
    xml->get_widget("next_button", next_button);
    next_button->signal_clicked().connect (method(on_next_clicked));
    xml->get_widget("previous_button", previous_button);
    previous_button->signal_clicked().connect (method(on_previous_clicked));
    xml->get_widget("refresh_button", refresh_button);
    refresh_button->signal_clicked().connect (method(on_refresh_clicked));
    xml->get_widget("preview_image", preview_image);
    xml->get_widget("selected_tilestyle_label", selected_tilestyle_label);
    xml->get_widget("eventbox", eventbox);
    eventbox->add_events(Gdk::BUTTON_PRESS_MASK |
                         Gdk::BUTTON_RELEASE_MASK | Gdk::POINTER_MOTION_MASK);
    eventbox->signal_button_press_event().connect (method(on_mouse_button_event));
    eventbox->signal_button_release_event().connect (method(on_mouse_button_event));
    eventbox->signal_motion_notify_event().connect (method(on_mouse_motion_event));

    d_tileSize = FontSize::getInstance ()->get_height () *
      EDITOR_DIALOG_TILE_PIC_FONTSIZE_MULTIPLE;

    Glib::ustring scene;
    TilePreviewScene *s;

    scenes.clear();

    switch (tile->getType())
      {
      case Tile::GRASS:
	scene.clear();
	scene += "aaaaa";
	scene += "aaaaa";
	scene += "aaaaa";
	scene += "aaaaa";
	scene += "aaaaa";
	s = new TilePreviewScene(tile, sec, 5, 5, scene, d_tileSize);
        add_scene(s);
	break;
      case Tile::WATER:
	scene.clear();
	scene += "bcd";
	scene += "hij";
	scene += "efg";
	s = new TilePreviewScene(tile, sec, 3, 3, scene, d_tileSize);
        add_scene(s);
	scene.clear();
	scene += "iiiii";
	scene += "iiiii";
	scene += "iiiii";
	scene += "iiiii";
	scene += "iiiii";
	s = new TilePreviewScene(tile, sec, 5, 5, scene, d_tileSize);
        add_scene(s);
	scene.clear();
	scene += "iiii";
	scene += "ikli";
	scene += "ijhi";
	scene += "imni";
	scene += "iiii";
	s = new TilePreviewScene(tile, sec, 5, 4, scene, d_tileSize);
        add_scene(s);
	scene.clear();
	scene += "ahiii";
	scene += "cplkf";
	scene += "ijhja";
	scene += "ijeoc";
	scene += "ijahi";
	s = new TilePreviewScene(tile, sec, 5, 5, scene, d_tileSize);
	break;
      case Tile::FOREST:
	scene.clear();
	scene += "bcd";
	scene += "hij";
	scene += "efg";
	s = new TilePreviewScene(tile, sec, 3, 3, scene, d_tileSize);
        add_scene(s);
	scene.clear();
	scene += "iiiii";
	scene += "iiiii";
	scene += "iiiii";
	scene += "iiiii";
	scene += "iiiii";
	s = new TilePreviewScene(tile, sec, 5, 5, scene, d_tileSize);
        add_scene(s);
	scene.clear();
	scene += "ahiii";
	scene += "cplkf";
	scene += "ijhja";
	scene += "ijeoc";
	scene += "ijahi";
	s = new TilePreviewScene(tile, sec, 5, 5, scene, d_tileSize);
        add_scene(s);
	break;
      case Tile::HILLS:
	scene.clear();
	scene += "bcd";
	scene += "hij";
	scene += "efg";
	s = new TilePreviewScene(tile, sec, 3, 3, scene, d_tileSize);
        add_scene(s);
	scene.clear();
	scene += "iiiii";
	scene += "iiiii";
	scene += "iiiii";
	scene += "iiiii";
	scene += "iiiii";
	s = new TilePreviewScene(tile, sec, 5, 5, scene, d_tileSize);
        add_scene(s);
	scene.clear();
	scene += "ahiii";
	scene += "cplkf";
	scene += "ijhja";
	scene += "ijeoc";
	scene += "ijahi";
	s = new TilePreviewScene(tile, sec, 5, 5, scene, d_tileSize);
        add_scene(s);
	break;
      case Tile::MOUNTAIN:
	scene.clear();
	scene += "bcd";
	scene += "hij";
	scene += "efg";
	s = new TilePreviewScene(tile, sec, 3, 3, scene, d_tileSize);
        add_scene(s);
	scene.clear();
	scene += "iiiii";
	scene += "iiiii";
	scene += "iiiii";
	scene += "iiiii";
	scene += "iiiii";
	s = new TilePreviewScene(tile, sec, 5, 5, scene, d_tileSize);
        add_scene(s);
	scene.clear();
	scene += "III";
	scene += "IaI";
	scene += "III";
	s = new TilePreviewScene(tile, sec, 3, 3, scene, d_tileSize);
        add_scene(s);
	scene.clear();
	scene += "ahiii";
	scene += "cplkf";
	scene += "ijhja";
	scene += "ijeoc";
	scene += "ijahi";
	s = new TilePreviewScene(tile, sec, 5, 5, scene, d_tileSize);
        add_scene(s);
	break;
      case Tile::SWAMP:
	scene = "aaaaa";
	scene += "aaaaa";
	scene += "aaaaa";
	scene += "aaaaa";
	scene += "aaaaa";
	s = new TilePreviewScene(tile, sec, 5, 5, scene, d_tileSize);
        add_scene(s);
	scene.clear();
	scene += "ahiii";
	scene += "cplkf";
	scene += "ijhja";
	scene += "ijeoc";
	scene += "ijahi";
	s = new TilePreviewScene(tile, sec, 5, 5, scene, d_tileSize);
        add_scene(s);
	scene.clear();
	scene = "iiiii";
	scene += "iiiii";
	scene += "iiiii";
	scene += "iiiii";
	scene += "iiiii";
	s = new TilePreviewScene(tile, sec, 5, 5, scene, d_tileSize);
        add_scene(s);
	break;
      }
    d_tile = tile;
    selected_tilestyle_label->set_text("");
    current_scene = scenes.begin();
    if (*current_scene)
      update_scene(*current_scene);

    update_buttons();
}

void TilePreviewDialog::add_scene(TilePreviewScene *s)
{
  s->hovered_tilestyle_id.connect (method(on_tilestyle_id_hovered));
  s->selected_tilestyle_id.connect
    (sigc::mem_fun (tilestyle_selected, &sigc::signal<void, guint32>::emit));
  scenes.push_back(s);
}

void TilePreviewDialog::on_tilestyle_id_hovered(guint32 id)
{
  selected_tilestyle_label->set_text("0x" + TileStyle::idToString(id));
}

void TilePreviewDialog::run()
{
    dialog->show_all();
    dialog->run();
    return;
}

void TilePreviewDialog::on_next_clicked()
{
  if (scenes.end() != current_scene)
    {
      selected_tilestyle_label->set_text("");
      current_scene++;
      TilePreviewScene *scene = *current_scene;
      if (scene)
	{
	  update_scene(scene);
	  update_buttons();
	}
    }
}

void TilePreviewDialog::on_previous_clicked()
{
  if (scenes.begin() != current_scene)
    {
      selected_tilestyle_label->set_text("");
      current_scene--;
      TilePreviewScene *scene = *current_scene;
      if (scene)
	{
	  update_scene(scene);
	  update_buttons();
	}
    }
}

void TilePreviewDialog::on_refresh_clicked()
{
  selected_tilestyle_label->set_text("");
  TilePreviewScene *scene = *current_scene;
  if (scene)
    {
      scene->regenerate();
      update_scene(scene);
    }
}

void TilePreviewDialog::update_scene(TilePreviewScene *scene)
{
  if (!scene)
    return;
  preview_image->property_pixbuf() =
    scene->renderScene();
  guint32 width = scene->getWidth();
  guint32 height = scene->getHeight();
  eventbox->set_size_request(width * d_tileSize, height * d_tileSize);
  preview_image->show_all();
}

void TilePreviewDialog::update_buttons()
{
  std::list<TilePreviewScene*>::iterator it = current_scene;
  next_button->set_sensitive(++it != scenes.end());
  previous_button->set_sensitive(current_scene != scenes.begin());
}

bool TilePreviewDialog::on_mouse_button_event(GdkEventButton *e)
{
  (*current_scene)->mouse_button_event(to_input_event(e));
  return true;
}

bool TilePreviewDialog::on_mouse_motion_event(GdkEventMotion *e)
{
  static guint prev = 0;
  gint delta = e->time - prev;
  if (delta > 40 || delta < 0)
    {
      (*current_scene)->mouse_motion_event(to_input_event(e));
      prev = e->time;
    }
  return true;
}

