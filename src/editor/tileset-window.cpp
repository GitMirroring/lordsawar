//  Copyright (C) 2008-2012, 2014, 2015, 2017, 2020 Ben Asselstine
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

#include <iostream>
#include <errno.h>

#include <sigc++/functors/mem_fun.h>
#include <sigc++/functors/ptr_fun.h>
#include "ucompose.hpp"
#include "builder-cache.h"

#include <gtkmm.h>
#include "tileset-window.h"
#include "tileset-info-dialog.h"
#include "tile-preview-dialog.h"
#include "tileset-selector-editor-dialog.h"
#include "tileset-flag-editor-dialog.h"
#include "tileset-explosion-picture-editor-dialog.h"
#include "image-editor-dialog.h"

#include "defs.h"
#include "Configuration.h"
#include "tilesetlist.h"
#include "Tile.h"
#include "File.h"
#include "overviewmap.h"
#include "ImageCache.h"
#include "tile-size-editor-dialog.h"
#include "editor-quit-dialog.h"
#include "tilestyle-organizer-dialog.h"
#include "tileset-smallmap-building-colors-dialog.h"
#include "GameMap.h"

#define method(x) sigc::mem_fun(*this, &TileSetWindow::x)

TileSetWindow::TileSetWindow(Glib::ustring load_filename)
{
  needs_saving = false;
  inhibit_needs_saving = false;
  d_tileset = NULL;
    Glib::RefPtr<Gtk::Builder> xml = 
      BuilderCache::editor_get("tileset-window.ui");

    xml->get_widget("window", window);
    window->set_icon_from_file(File::getVariousFile("tileset_icon.png"));
    window->signal_delete_event().connect (sigc::hide(method(on_window_closed)));

    xml->get_widget("tiles_treeview", tiles_treeview);
    xml->get_widget("tile_name_entry", tile_name_entry);
    tile_name_entry->signal_changed().connect (method(on_tile_name_changed));

    Gtk::Box *type_combo_container;
    xml->get_widget("type_combo_container", type_combo_container);
    tile_type_combobox = new Gtk::ComboBoxText();
    tile_type_combobox->append(Tile::tileTypeToFriendlyName(Tile::GRASS));
    tile_type_combobox->append(Tile::tileTypeToFriendlyName(Tile::WATER));
    tile_type_combobox->append(Tile::tileTypeToFriendlyName(Tile::FOREST));
    tile_type_combobox->append(Tile::tileTypeToFriendlyName(Tile::HILLS));
    tile_type_combobox->append(Tile::tileTypeToFriendlyName(Tile::MOUNTAIN));
    tile_type_combobox->append(Tile::tileTypeToFriendlyName(Tile::SWAMP));
    type_combo_container->add(*manage(tile_type_combobox));
    type_combo_container->show_all();
    tile_type_combobox->signal_changed().connect (method(on_tile_type_changed));

    Gtk::Box *tilestyle_combo_container;
    xml->get_widget("tilestyle_combo_container", tilestyle_combo_container);
    tilestyle_combobox = new Gtk::ComboBoxText();
    tilestyle_combobox->append(_("Lone"));
    tilestyle_combobox->append(_("Outer Top-Left"));
    tilestyle_combobox->append(_("Outer Top-Center"));
    tilestyle_combobox->append(_("Outer Top-Right"));
    tilestyle_combobox->append(_("Outer Bottom-Left"));
    tilestyle_combobox->append(_("Outer Bottom-Center"));
    tilestyle_combobox->append(_("Outer Bottom-Right"));
    tilestyle_combobox->append(_("Outer Middle-Left"));
    tilestyle_combobox->append(_("Inner Middle-Center"));
    tilestyle_combobox->append(_("Outer Middle-Right"));
    tilestyle_combobox->append(_("Inner Top-Left"));
    tilestyle_combobox->append(_("Inner Top-Right"));
    tilestyle_combobox->append(_("Inner Bottom-Left"));
    tilestyle_combobox->append(_("Inner Bottom-Right"));
    tilestyle_combobox->append(_("Top-Left To Bottom-Right"));
    tilestyle_combobox->append(_("Bottom-Left To Top-Right"));
    tilestyle_combobox->append(_("Other"));
    tilestyle_combobox->append(_("Unknown"));
    tilestyle_combo_container->add(*manage(tilestyle_combobox));
    tilestyle_combo_container->show_all();
    tilestyle_combobox->signal_changed().connect (method(on_tilestyle_changed));

    Gtk::Box *pattern_container;
    xml->get_widget("pattern_container", pattern_container);
    tile_smallmap_pattern_combobox = new Gtk::ComboBoxText();
    tile_smallmap_pattern_combobox->append(_("Solid"));
    tile_smallmap_pattern_combobox->append(_("Stippled"));
    tile_smallmap_pattern_combobox->append(_("Randomized"));
    tile_smallmap_pattern_combobox->append(_("Sunken"));
    tile_smallmap_pattern_combobox->append(_("Tablecloth"));
    tile_smallmap_pattern_combobox->append(_("Diagonal"));
    tile_smallmap_pattern_combobox->append(_("Crosshatched"));
    tile_smallmap_pattern_combobox->append(_("Sunken Striped"));
    tile_smallmap_pattern_combobox->append(_("Sunken Radial"));
    pattern_container->add(*manage(tile_smallmap_pattern_combobox));
    pattern_container->show_all();
    tile_smallmap_pattern_combobox->signal_changed().connect (method(on_tile_pattern_changed));

    xml->get_widget("tile_moves_spinbutton", tile_moves_spinbutton);
    xml->get_widget("tile_smallmap_first_colorbutton", 
		    tile_smallmap_first_colorbutton);
    tile_smallmap_first_colorbutton->signal_color_set().connect (method(on_tile_first_color_changed));
    xml->get_widget("tile_smallmap_second_colorbutton", 
		    tile_smallmap_second_colorbutton);
    tile_smallmap_second_colorbutton->signal_color_set().connect
      (method(on_tile_second_color_changed));
    xml->get_widget("tile_smallmap_third_colorbutton", 
		    tile_smallmap_third_colorbutton);
    tile_smallmap_third_colorbutton->signal_color_set().connect
      (method(on_tile_third_color_changed));
    xml->get_widget("tile_smallmap_image", tile_smallmap_image);

    xml->get_widget("add_tile_button", add_tile_button);
    add_tile_button->signal_clicked().connect (method(on_add_tile_clicked));
    xml->get_widget("remove_tile_button", remove_tile_button);
    remove_tile_button->signal_clicked().connect (method(on_remove_tile_clicked));
    xml->get_widget("tile_vbox", tile_vbox);
    // connect callbacks for the menu
    xml->get_widget("new_tileset_menuitem", new_tileset_menuitem);
    new_tileset_menuitem->signal_activate().connect (method(on_new_tileset_activated));
    xml->get_widget("load_tileset_menuitem", load_tileset_menuitem);
    load_tileset_menuitem->signal_activate().connect (method(on_load_tileset_activated));
    xml->get_widget("save_tileset_menuitem", save_tileset_menuitem);
    save_tileset_menuitem->signal_activate().connect (method(on_save_tileset_activated));
    xml->get_widget("save_as_menuitem", save_as_menuitem);
    save_as_menuitem->signal_activate().connect (method(on_save_as_activated));
    xml->get_widget("validate_tileset_menuitem", validate_tileset_menuitem);
    validate_tileset_menuitem->signal_activate().connect (method(on_validate_tileset_activated));
    xml->get_widget("quit_menuitem", quit_menuitem);
    quit_menuitem->signal_activate().connect (method(on_quit_activated));
    xml->get_widget("edit_tileset_info_menuitem", edit_tileset_info_menuitem);
    edit_tileset_info_menuitem->signal_activate().connect
      (method(on_edit_tileset_info_activated));
    xml->get_widget("army_unit_selector_menuitem", army_unit_selector_menuitem);
    army_unit_selector_menuitem->signal_activate().connect
      (method(on_army_unit_selector_activated));
    xml->get_widget("roads_picture_menuitem", roads_picture_menuitem);
    roads_picture_menuitem->signal_activate().connect
      (method(on_roads_picture_activated));
    xml->get_widget("stones_picture_menuitem", stones_picture_menuitem);
    stones_picture_menuitem->signal_activate().connect
      (method(on_stones_picture_activated));
    xml->get_widget("bridges_picture_menuitem", bridges_picture_menuitem);
    bridges_picture_menuitem->signal_activate().connect
      (method(on_bridges_picture_activated));
    xml->get_widget("fog_picture_menuitem", fog_picture_menuitem);
    fog_picture_menuitem->signal_activate().connect (method(on_fog_picture_activated));
    xml->get_widget("flags_picture_menuitem", flags_picture_menuitem);
    flags_picture_menuitem->signal_activate().connect (method(on_flags_picture_activated));

    xml->get_widget("explosion_picture_menuitem", explosion_picture_menuitem);
    explosion_picture_menuitem->signal_activate().connect
      (method(on_explosion_picture_activated));
    xml->get_widget("preview_tile_menuitem", preview_tile_menuitem);
    preview_tile_menuitem->signal_activate().connect
      (method(on_preview_tile_activated));
    xml->get_widget("organize_tilestyles_menuitem", organize_tilestyles_menuitem);
    organize_tilestyles_menuitem->signal_activate().connect
      (method(on_organize_tilestyles_activated));
    xml->get_widget("smallmap_building_colors_menuitem", 
                    smallmap_building_colors_menuitem);
    smallmap_building_colors_menuitem->signal_activate().connect
      (method(on_smallmap_building_colors_activated));
    xml->get_widget ("help_about_menuitem", help_about_menuitem);
    help_about_menuitem->signal_activate().connect
      (method(on_help_about_activated));
    xml->get_widget ("tutorial_menuitem", tutorial_menuitem);
    tutorial_menuitem->signal_activate().connect
      (method(on_tutorial_video_activated));
    xml->get_widget("tilestyle_image", tilestyle_image);

    window->signal_delete_event().connect (sigc::hide(method(on_delete_event)));

    tiles_list = Gtk::ListStore::create(tiles_columns);
    tiles_treeview->set_model(tiles_list);
    tiles_treeview->append_column("", tiles_columns.name);
    tiles_treeview->set_headers_visible(false);
    tiles_treeview->get_selection()->signal_changed().connect (method(on_tile_selected));

    xml->get_widget("tilestylesets_treeview", tilestylesets_treeview);
    tilestylesets_list = Gtk::ListStore::create(tilestylesets_columns);
    tilestylesets_treeview->set_model(tilestylesets_list);
    tilestylesets_treeview->append_column("", tilestylesets_columns.name);
    tilestylesets_treeview->set_headers_visible(false);
    tilestylesets_treeview->get_selection()->signal_changed().connect
      (method(on_tilestyleset_selected));

    xml->get_widget("tilestyles_treeview", tilestyles_treeview);
    tilestyles_list = Gtk::ListStore::create(tilestyles_columns);
    tilestyles_treeview->set_model(tilestyles_list);
    tilestyles_treeview->append_column("", tilestyles_columns.name);
    tilestyles_treeview->set_headers_visible(false);
    tilestyles_treeview->get_selection()->signal_changed().connect
      (method(on_tilestyle_selected));

    xml->get_widget("add_tilestyleset_button", add_tilestyleset_button);
    add_tilestyleset_button->signal_clicked().connect
      (method(on_add_tilestyleset_clicked));
    xml->get_widget("remove_tilestyleset_button", remove_tilestyleset_button);
    remove_tilestyleset_button->signal_clicked().connect
      (method(on_remove_tilestyleset_clicked));
    xml->get_widget("tilestyleset_alignment", tilestyleset_alignment);
    xml->get_widget("tilestyle_alignment", tilestyle_alignment);
    xml->get_widget("image_button", image_button);
    image_button->signal_clicked().connect (method(on_image_chosen));

    xml->get_widget("tilestyle_standard_image", tilestyle_standard_image);
    xml->get_widget("notebook", notebook);

    if (load_filename != "")
      current_save_filename = load_filename;
    update_tile_panel();
    update_tilestyleset_panel();
    update_tilestyle_panel();
    update_tileset_buttons();
    update_tilestyleset_buttons();

    update_tileset_buttons();
    update_tilestyleset_buttons();
    update_tileset_menuitems();
    update_tile_preview_menuitem();

    if (load_filename.empty() == false)
      load_tileset(load_filename);
    inhibit_updates = false;
}

void
TileSetWindow::update_tileset_menuitems()
{
  if (d_tileset == NULL)
    {
      save_tileset_menuitem->set_sensitive(false);
      save_as_menuitem->set_sensitive(false);
      army_unit_selector_menuitem->set_sensitive(false);
      edit_tileset_info_menuitem->set_sensitive(false);
      explosion_picture_menuitem->set_sensitive(false);
      roads_picture_menuitem->set_sensitive(false);
      bridges_picture_menuitem->set_sensitive(false);
      fog_picture_menuitem->set_sensitive(false);
      flags_picture_menuitem->set_sensitive(false);
      organize_tilestyles_menuitem->set_sensitive(false);
      smallmap_building_colors_menuitem->set_sensitive(false);
    }
  else
    {
      save_tileset_menuitem->set_sensitive(true);
      save_as_menuitem->set_sensitive(true);
      army_unit_selector_menuitem->set_sensitive(true);
      edit_tileset_info_menuitem->set_sensitive(true);
      explosion_picture_menuitem->set_sensitive(true);
      roads_picture_menuitem->set_sensitive(true);
      bridges_picture_menuitem->set_sensitive(true);
      fog_picture_menuitem->set_sensitive(true);
      flags_picture_menuitem->set_sensitive(true);
      organize_tilestyles_menuitem->set_sensitive(true);
      smallmap_building_colors_menuitem->set_sensitive(true);
    }
}

void
TileSetWindow::update_tileset_buttons()
{
  if (!tiles_treeview->get_selection()->get_selected())
    remove_tile_button->set_sensitive(false);
  else
    remove_tile_button->set_sensitive(true);
  if (d_tileset == NULL)
    add_tile_button->set_sensitive(false);
  else
    add_tile_button->set_sensitive(true);
}

void
TileSetWindow::update_tilestyleset_buttons()
{
  if (!tilestylesets_treeview->get_selection()->get_selected())
    remove_tilestyleset_button->set_sensitive(false);
  else
    remove_tilestyleset_button->set_sensitive(true);
  if (d_tileset == NULL)
    add_tilestyleset_button->set_sensitive(false);
  else
    add_tilestyleset_button->set_sensitive(true);
}

void
TileSetWindow::update_tilestyle_panel()
{
  if (tilestyles_treeview->get_selection()->get_selected() == 0)
    {
      //clear all values
      tilestyle_alignment->set_sensitive(false);
      tilestyle_combobox->set_active(0);
      tilestyle_image->clear();
      tilestyle_standard_image->clear();
      tilestyle_image->show_all();
	  
      return;
    }
  tilestyle_alignment->set_sensitive(true);
  TileStyle *t = get_selected_tilestyle ();
  if (t)
    {
      int idx = t->getType();
      tilestyle_combobox->set_active(idx);
      Glib::RefPtr<Gdk::Pixbuf> pixbuf= t->getImage()->to_pixbuf();
      tilestyle_image->clear();
      tilestyle_image->property_pixbuf() = pixbuf;
      tilestyle_image->show_all();
      tilestyle_standard_image->property_pixbuf() = 
        ImageCache::getInstance()->getDefaultTileStylePic(idx, d_tileset->getTileSize())->to_pixbuf();
    }
}

void TileSetWindow::fill_tilestyleset_info(TileStyleSet *t)
{
  if (!t || t->getName() == "")
    {
      tilestyles_list->clear();
      image_button->set_label(_("no image set"));
      update_tilestyle_panel();
      return;
    }

  Glib::ustring n = d_tileset->getFileFromConfigurationFile(t->getName() + ".png");
  bool broken = false;
  TileStyleSet *set = get_selected_tilestyleset ();
  set->instantiateImages(d_tileset->getTileSize(), n, broken);
  if (broken)
    {
      tilestyles_list->clear();
      image_button->set_label(_("no image set"));
      update_tilestyle_panel();
      return;
    }

  File::erase(n);
  if (t->getName().empty() == false)
    image_button->set_label(File::get_basename(t->getName() + ".png", true));
  else
    image_button->set_label(_("no image set"));
  //add the tilestyles to the tilestyles_treeview
  tilestyles_list->clear();
  for (unsigned int i = 0; i < t->size(); i++)
    {
      Gtk::TreeIter l = tilestyles_list->append();
      (*l)[tilestyles_columns.name] = 
        "0x" + TileStyle::idToString((*t)[i]->getId());
      (*l)[tilestyles_columns.tilestyle] = (*t)[i];
    }
  tilestyles_treeview->set_cursor (Gtk::TreePath ("0"));
}

void
TileSetWindow::update_tilestyleset_panel()
{
  tilestyleset_alignment->set_sensitive(true);
  TileStyleSet *t = get_selected_tilestyleset ();
  if (t)
    fill_tilestyleset_info(t);
  else
    {
      fill_tilestyleset_info(NULL);
      tilestyleset_alignment->set_sensitive(false);
    }
}

void
TileSetWindow::update_tile_panel()
{
  Gdk::RGBA black("black");
  //if nothing selected in the treeview, then we don't show anything in
  //the tile panel
  Tile *t = get_selected_tile ();
      
  if (t == NULL)
    {
      //clear all values
      inhibit_needs_saving = true;
      tile_smallmap_image->clear();
      tile_vbox->set_sensitive(false);
      tile_type_combobox->set_active(0);
      tile_moves_spinbutton->set_value(0);
      tile_name_entry->set_text("");
      tile_smallmap_pattern_combobox->set_active(0);
      tile_smallmap_first_colorbutton->set_rgba(black);
      tile_smallmap_second_colorbutton->set_rgba(black);
      tile_smallmap_third_colorbutton->set_rgba(black);
      tilestylesets_list->clear();
      inhibit_needs_saving = false;
      return;
    }

  tile_vbox->set_sensitive(true);
  fill_tile_info(t);
  fill_tilestylesets();
}

bool TileSetWindow::on_delete_event()
{
  hide();
  return true;
}

void TileSetWindow::on_new_tileset_activated()
{
  Glib::ustring name = "";
  int id = Tilesetlist::getNextAvailableId(0);
  Tileset *tileset = new Tileset(id, name);
  TileSetInfoDialog d(*window, tileset, 
                      File::getSetDir(Tileset::file_extension, false), "", 
                      false, _("Make a New Tileset"));
  int response = d.run();
  if (response != Gtk::RESPONSE_ACCEPT)
    {
      delete tileset;
      return;
    }
  inhibit_updates = true;
  tiles_list->clear();
  tilestyles_list->clear();
  tilestylesets_list->clear();
  inhibit_updates = false;
  if (d_tileset)
    delete d_tileset;
  d_tileset = tileset;
  //add in some default tiles now.
  d_tileset->populateWithDefaultTiles();
  for (Tileset::iterator i = d_tileset->begin(); i != d_tileset->end(); i++)
    {
      Gtk::TreeIter j = tiles_list->append();
      (*j)[tiles_columns.name] = Tile::tileTypeToFriendlyName((*i)->getType());
      (*j)[tiles_columns.tile] = (*i);
    }
  Glib::ustring dir = File::getSetDir(Tileset::file_extension, false);
  d_tileset->setDirectory(dir);
  current_save_filename = d_tileset->getConfigurationFile();
  //here we put a copy into the tilesetlist, and keep d_tileset as our
  //current working tileset.
  Tileset *copy = Tileset::copy (d_tileset);
  Glib::ustring new_basename = copy->getBaseName();
  guint32 new_id = copy->getId();
  if (!Tilesetlist::getInstance()->addToPersonalCollection(copy, new_basename, new_id))
    delete copy;

  //copy images??
  tiles_treeview->set_cursor (Gtk::TreePath ("0"));
  update_tile_panel();
  update_tileset_buttons();
  update_tilestyleset_buttons();
  update_tileset_menuitems();
  update_tile_preview_menuitem();

  needs_saving = true;
  update_window_title();

}

void TileSetWindow::on_load_tileset_activated()
{
  Gtk::FileChooserDialog chooser(*window, 
				 _("Choose a Tileset to Load"));
  Glib::RefPtr<Gtk::FileFilter> lwt_filter = Gtk::FileFilter::create();
  lwt_filter->set_name(_("LordsAWar Tilesets (*.lwt)"));
  lwt_filter->add_pattern("*" + TILESET_EXT);
  chooser.add_filter(lwt_filter);
  chooser.set_current_folder(File::getSetDir(Tileset::file_extension, false));

  chooser.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
  chooser.add_button(Gtk::Stock::OPEN, Gtk::RESPONSE_ACCEPT);
  chooser.set_default_response(Gtk::RESPONSE_ACCEPT);

  chooser.show_all();
  int res = chooser.run();

  if (res == Gtk::RESPONSE_ACCEPT)
    {
      load_tileset(chooser.get_filename());
      chooser.hide();
      needs_saving = false;
      update_window_title();
    }
  update_tile_panel();
}

void TileSetWindow::on_save_as_activated()
{
  guint32 suggested_tile_size = d_tileset->calculate_preferred_tile_size();
  if (suggested_tile_size != d_tileset->getTileSize())
    {
      TileSizeEditorDialog d(*window, d_tileset->getTileSize(), suggested_tile_size);
      int response = d.run();
      if (response == Gtk::RESPONSE_ACCEPT)
        d_tileset->setTileSize(d.get_selected_tilesize());
    }
  //Reorder the tileset according to the treeview
  d_tileset->clear();
  for (Gtk::TreeIter i = tiles_list->children().begin(),
       end = tiles_list->children().end(); i != end; ++i) 
    d_tileset->push_back((*i)[tiles_columns.tile]);

  Tileset *copy = Tileset::copy (d_tileset);
  copy->setId(Tilesetlist::getNextAvailableId(d_tileset->getId()));
  TileSetInfoDialog d(*window, copy, 
                      File::getSetDir(Tileset::file_extension, false), "", 
                      false, _("Save a Copy of a Tileset"));
  int response = d.run();
  if (response == Gtk::RESPONSE_ACCEPT)
    {
      Glib::ustring new_basename = copy->getBaseName();
      guint32 new_id = copy->getId();
      copy->setDirectory(File::getSetDir(Tileset::file_extension, false));
      guint32 oldid = d_tileset->getId();
      Glib::ustring oldname = d_tileset->getName();
      Glib::ustring oldbasename = d_tileset->getBaseName();
      Glib::ustring olddir = d_tileset->getDirectory();

      Glib::ustring tmpdir = File::get_tmp_file();
      File::erase(tmpdir);
      tmpdir += Tileset::file_extension;
      File::create_dir(tmpdir);
      d_tileset->setName(copy->getName());
      File::copy(d_tileset->getConfigurationFile(), 
                 File::getTempFile(tmpdir, copy->getBaseName() + Tileset::file_extension));
      d_tileset->setBaseName(copy->getBaseName());
      d_tileset->setDirectory(tmpdir);
      d_tileset->setId(copy->getId());
          
      current_save_filename = copy->getConfigurationFile();
      bool ok = Tilesetlist::getInstance()->addToPersonalCollection(d_tileset, new_basename, new_id);
      File::erase(File::getTempFile (tmpdir, copy->getBaseName() + Tileset::file_extension));
      File::erase_dir(tmpdir);
      if (ok)
        {
          save_tileset_menuitem->set_sensitive(true);
          d_tileset = copy;
          //our tiles in the treeview are now out of date.
          refresh_tiles(); //refresh them.
          needs_saving = false;
          update_window_title();
          tileset_saved.emit(d_tileset->getId());
        }
      else
        {
          d_tileset->setName(oldname);
          d_tileset->setBaseName(oldbasename);
          d_tileset->setId(oldid);
          d_tileset->setDirectory(olddir);
          Glib::ustring errmsg = Glib::strerror(errno);
          Glib::ustring msg;
          msg = _("Error!  Tileset could not be saved.");
          msg += "\n" + current_save_filename + "\n" + errmsg;
          Gtk::MessageDialog dialog(*window, msg);
          dialog.run();
          dialog.hide();
          delete copy;
        }
    }
  else
    delete copy;
}

bool TileSetWindow::save_current_tileset()
{
  if (GameMap::getInstance()->getTilesetId() == d_tileset->getId() &&
      d_tileset->validate() == false)
    {
      Glib::ustring errmsg = _("Tileset is invalid, and is also the current working tileset.");
      Glib::ustring msg;
      msg = _("Error!  Tileset could not be saved.");
      msg += "\n" + current_save_filename + "\n" +
        errmsg;
      Gtk::MessageDialog dialog(*window, msg);
      dialog.run();
      dialog.hide();
      return false;
    }
  if (current_save_filename.empty())
    current_save_filename = d_tileset->getConfigurationFile();
  
  guint32 suggested_tile_size = d_tileset->calculate_preferred_tile_size();
  if (suggested_tile_size != d_tileset->getTileSize())
    {
      TileSizeEditorDialog d(*window, d_tileset->getTileSize(), suggested_tile_size);
      int response = d.run();
      if (response == Gtk::RESPONSE_ACCEPT)
        d_tileset->setTileSize(d.get_selected_tilesize());
    }
  //Reorder the tileset according to the treeview
  d_tileset->clear();
  for (Gtk::TreeIter i = tiles_list->children().begin(),
       end = tiles_list->children().end(); i != end; ++i) 
    d_tileset->push_back((*i)[tiles_columns.tile]);

  bool success = d_tileset->save(current_save_filename, Tileset::file_extension);
  if (success)
    {
      if (Tilesetlist::getInstance()->reload(d_tileset->getId()))
        refresh_tiles(); //refresh them.
      needs_saving = false;
      update_window_title();
      tileset_saved.emit(d_tileset->getId());
    }
  else
    {
      Glib::ustring errmsg = Glib::strerror(errno);
      Glib::ustring msg;
      msg = _("Error!  Tileset could not be saved.");
      msg += "\n" + current_save_filename + "\n" + errmsg;
      Gtk::MessageDialog dialog(*window, msg);
      dialog.run();
      dialog.hide();
    }
  return success;
}

void TileSetWindow::on_save_tileset_activated()
{
  save_current_tileset();
}

bool TileSetWindow::quit()
{
  if (needs_saving)
    {
      EditorQuitDialog d(*window);
      int response = d.run_and_hide();
      
      if (response == Gtk::RESPONSE_CANCEL) //we don't want to quit
	return false;

      else if (response == Gtk::RESPONSE_ACCEPT) // save and quit
        {
          if (save_current_tileset() == false)
            return false;
        }
      //else if (Response == Gtk::CLOSE) // don't save just quit
      window->hide();
    }
  else
    window->hide();
  if (d_tileset)
    delete d_tileset;
  return true;
}

bool TileSetWindow::on_window_closed()
{
  return !quit();
}

void TileSetWindow::on_quit_activated()
{
  quit();
}

void TileSetWindow::on_edit_tileset_info_activated()
{
  TileSetInfoDialog d(*window, d_tileset, 
                      File::get_dirname(current_save_filename), 
                      File::get_basename(current_save_filename), true);
  int response = d.run();
  if (response == Gtk::RESPONSE_ACCEPT)
    {
      needs_saving = true;
      update_window_title();
    }
}

void TileSetWindow::on_help_about_activated()
{
  Gtk::AboutDialog* dialog;

  Glib::RefPtr<Gtk::Builder> xml
    = Gtk::Builder::create_from_file(File::getGladeFile("about-dialog.ui"));

  xml->get_widget("dialog", dialog);
  dialog->set_transient_for(*window);
  dialog->set_icon_from_file(File::getVariousFile("tileset_icon.png"));

  dialog->set_version(PACKAGE_VERSION);
  dialog->set_logo(ImageCache::loadMiscImage("tileset_icon.png")->to_pixbuf());
  dialog->show_all();
  dialog->run();
  dialog->hide();
  delete dialog;

  return;
}

void TileSetWindow::update_tile_preview_menuitem()
{
  if (get_selected_tile())
    preview_tile_menuitem->set_sensitive(true);
  else
    preview_tile_menuitem->set_sensitive(false);
}

void TileSetWindow::on_tile_selected()
{
  if (inhibit_updates)
    return;
  update_tile_panel();
  update_tilestyleset_panel();
  update_tilestyle_panel();
  update_tileset_buttons();
  update_tilestyleset_buttons();
  update_tile_preview_menuitem();
}

void TileSetWindow::on_tilestyleset_selected()
{
  update_tilestyleset_panel();
  update_tilestyle_panel();
  update_tilestyleset_buttons();
}

void TileSetWindow::on_tilestyle_selected()
{
  update_tilestyle_panel();
}

void TileSetWindow::fill_tilestylesets()
{
  Tile *t = get_selected_tile();
  if (!t)
    return;
  tilestylesets_list->clear();
  for (std::list<TileStyleSet*>::iterator it = t->begin(); it != t->end(); it++)
    {
      Gtk::TreeIter l = tilestylesets_list->append();
      (*l)[tilestylesets_columns.name] = (*it)->getName();
      (*l)[tilestylesets_columns.tilestyleset] = *it;
    }
  tilestylesets_treeview->set_cursor (Gtk::TreePath ("0"));
}

void TileSetWindow::fill_tile_info(Tile *tile)
{
  inhibit_needs_saving = true;
  tile_name_entry->set_text(tile->getName());
  tile_type_combobox->set_active(tile->getTypeIndex());
  tile_moves_spinbutton->set_value(tile->getMoves());
  tile_smallmap_pattern_combobox->set_active(tile->getSmallTile()->getPattern());
  tile_smallmap_first_colorbutton->set_sensitive(true);
  fill_tilestylesets();
  fill_colours(tile);
  fill_tile_smallmap(tile);
  inhibit_needs_saving = false;
}

void TileSetWindow::on_add_tile_clicked()
{
  //add a new empty tile to the tileset
  Tile *t = new Tile();
  //add it to the treeview
  Gtk::TreeIter i = tiles_list->append();
  t->setName(_("Untitled"));
  (*i)[tiles_columns.name] = t->getName();
  (*i)[tiles_columns.tile] = t;
  d_tileset->push_back(t);
  tiles_treeview->set_cursor (Gtk::TreePath (String::ucompose("%1", d_tileset->size() - 1)));
  update_tile_preview_menuitem();
  needs_saving = true;
  update_window_title();
}

void TileSetWindow::on_remove_tile_clicked()
{
  //erase the selected row from the treeview
  //remove the tile from the tileset
  Glib::RefPtr<Gtk::TreeSelection> selection = tiles_treeview->get_selection();
  Gtk::TreeModel::iterator iterrow = selection->get_selected();

  if (iterrow) 
    {
      Gtk::TreeModel::Row row = *iterrow;
      Tile *a = row[tiles_columns.tile];
      tiles_list->erase(iterrow);

      for (std::vector<Tile*>::iterator it = d_tileset->begin(); it != d_tileset->end(); it++)
	{
	  if (*it == a)
	    {
	      d_tileset->erase(it);
	      break;
	    }
	}
  
      needs_saving = true;
      update_window_title();
    }
  update_tile_preview_menuitem();
}

void TileSetWindow::on_tile_first_color_changed()
{
  Glib::RefPtr<Gtk::TreeSelection> selection = tiles_treeview->get_selection();
  Gtk::TreeModel::iterator iterrow = selection->get_selected();

  if (iterrow) 
    {
      Gtk::TreeModel::Row row = *iterrow;
      Tile *t = row[tiles_columns.tile];
      t->getSmallTile()->setColor(tile_smallmap_first_colorbutton->get_rgba());
      fill_tile_smallmap(t);
      if (inhibit_needs_saving == false)
        {
          needs_saving = true;
          update_window_title();
        }
    }
}

void TileSetWindow::on_tile_second_color_changed()
{
  Glib::RefPtr<Gtk::TreeSelection> selection = tiles_treeview->get_selection();
  Gtk::TreeModel::iterator iterrow = selection->get_selected();

  if (iterrow) 
    {
      Gtk::TreeModel::Row row = *iterrow;
      Tile *t = row[tiles_columns.tile];
      t->getSmallTile()->setSecondColor(tile_smallmap_second_colorbutton->get_rgba());
      fill_tile_smallmap(t);
      if (inhibit_needs_saving == false)
        {
          needs_saving = true;
          update_window_title();
        }
    }
}

void TileSetWindow::on_tile_third_color_changed()
{
  Glib::RefPtr<Gtk::TreeSelection> selection = tiles_treeview->get_selection();
  Gtk::TreeModel::iterator iterrow = selection->get_selected();

  if (iterrow) 
    {
      Gtk::TreeModel::Row row = *iterrow;
      Tile *t = row[tiles_columns.tile];
      t->getSmallTile()->setThirdColor(tile_smallmap_third_colorbutton->get_rgba());
      fill_tile_smallmap(t);
      if (inhibit_needs_saving == false)
        {
          needs_saving = true;
          update_window_title();
        }
    }
}

void TileSetWindow::fill_colours(Tile *tile)
{
  Gdk::RGBA c;
  switch (tile->getSmallTile()->getPattern())
    {
    case SmallTile::SOLID:
      tile_smallmap_second_colorbutton->set_sensitive(false);
      tile_smallmap_second_colorbutton->set_rgba(Gdk::RGBA("black"));
      tile_smallmap_third_colorbutton->set_sensitive(false);
      tile_smallmap_third_colorbutton->set_rgba(Gdk::RGBA("black"));
      tile_smallmap_first_colorbutton->set_rgba(tile->getSmallTile()->getColor());
      break;
    case SmallTile::STIPPLED: case SmallTile::SUNKEN:
      tile_smallmap_second_colorbutton->set_sensitive(true);
      tile_smallmap_third_colorbutton->set_sensitive(false);
      tile_smallmap_third_colorbutton->set_rgba(Gdk::RGBA("black"));
      tile_smallmap_first_colorbutton->set_rgba(tile->getSmallTile()->getColor());
      tile_smallmap_second_colorbutton->set_rgba(tile->getSmallTile()->getSecondColor());
      break;
    case SmallTile::RANDOMIZED: case SmallTile::TABLECLOTH: case SmallTile::DIAGONAL: case SmallTile::CROSSHATCH: case SmallTile::SUNKEN_STRIPED: case SmallTile::SUNKEN_RADIAL:
      tile_smallmap_second_colorbutton->set_sensitive(true);
      tile_smallmap_third_colorbutton->set_sensitive(true);
      tile_smallmap_first_colorbutton->set_rgba(tile->getSmallTile()->getColor());
      tile_smallmap_second_colorbutton->set_rgba(tile->getSmallTile()->getSecondColor());
      tile_smallmap_third_colorbutton->set_rgba(tile->getSmallTile()->getThirdColor());
      break;
    }
}

void TileSetWindow::on_tile_pattern_changed()
{
  Glib::RefPtr<Gtk::TreeSelection> selection = tiles_treeview->get_selection();
  Gtk::TreeModel::iterator iterrow = selection->get_selected();

  if (iterrow) 
    {
      Gtk::TreeModel::Row row = *iterrow;
      Tile *t = row[tiles_columns.tile];
      int idx = tile_smallmap_pattern_combobox->get_active_row_number();
      SmallTile::Pattern pattern = SmallTile::Pattern(idx);
      t->getSmallTile()->setPattern(pattern);
      fill_colours(t);
      fill_tile_smallmap(t);
      if (inhibit_needs_saving == false)
        {
          needs_saving = true;
          update_window_title();
        }
    }
}

void TileSetWindow::on_tile_type_changed()
{
  int idx = tile_type_combobox->get_active_row_number();
  Glib::RefPtr<Gtk::TreeSelection> selection = tiles_treeview->get_selection();
  Gtk::TreeModel::iterator iterrow = selection->get_selected();

  if (iterrow) 
    {
      Gtk::TreeModel::Row row = *iterrow;
      Tile *t = row[tiles_columns.tile];
      t->setTypeByIndex(idx);
      if (inhibit_needs_saving == false)
        {
          needs_saving = true;
          update_window_title();
        }
    }
}

void TileSetWindow::on_tile_name_changed()
{
  Glib::RefPtr<Gtk::TreeSelection> selection = tiles_treeview->get_selection();
  Gtk::TreeModel::iterator iterrow = selection->get_selected();

  if (iterrow) 
    {
      Gtk::TreeModel::Row row = *iterrow;
      row[tiles_columns.name] = tile_name_entry->get_text();
      Tile *t = row[tiles_columns.tile];
      t->setName(tile_name_entry->get_text());

      if (inhibit_needs_saving == false)
        {
          needs_saving = true;
          update_window_title();
        }
    }
}

void TileSetWindow::fill_tile_smallmap(Tile *tile)
{
  SmallTile *s = tile->getSmallTile();
  Cairo::RefPtr<Cairo::Surface> tile_smallmap_surface= Cairo::ImageSurface::create (Cairo::FORMAT_ARGB32, 32, 32);
  Cairo::RefPtr<Cairo::Context> tile_smallmap_surface_gc;
  tile_smallmap_surface_gc = Cairo::Context::create(tile_smallmap_surface);
  if (s->getPattern() == SmallTile::SUNKEN_RADIAL)
    OverviewMap::draw_radial_gradient(tile_smallmap_surface, s->getColor(), 
                                      s->getSecondColor(), 32, 32);
  for (int i = 0; i < 32; i++)
    {
      for (int j = 0; j < 32; j++)
	{
	  bool shadowed = false;
	  if (i == 32 - 1)
	    shadowed = true;
	  else if (j == 32 - 1)
	    shadowed = true;
	  OverviewMap::draw_terrain_tile (tile_smallmap_surface_gc,
					  tile->getSmallTile()->getPattern(),
					  tile->getSmallTile()->getColor(),
					  tile->getSmallTile()->getSecondColor(),
					  tile->getSmallTile()->getThirdColor(),
					  i, j, shadowed);
	}
    }

  Glib::RefPtr<Gdk::Pixbuf> pixbuf = 
    Gdk::Pixbuf::create(tile_smallmap_surface, 0, 0, 32, 32);
                                                        
  tile_smallmap_image->property_pixbuf() = pixbuf;
  tile_smallmap_image->queue_draw();

}

void TileSetWindow::choose_and_add_or_replace_tilestyleset(Glib::ustring replace_filename)
{
  Gtk::FileChooserDialog chooser(*window, _("Choose an Image"),
				 Gtk::FILE_CHOOSER_ACTION_OPEN);
  Glib::RefPtr<Gtk::FileFilter> png_filter = Gtk::FileFilter::create();
  png_filter->set_name(_("PNG files (*.png)"));
  png_filter->add_pattern("*.png");
  chooser.add_filter(png_filter);
  chooser.set_current_folder(Glib::get_home_dir());

  chooser.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
  chooser.add_button(Gtk::Stock::OPEN, Gtk::RESPONSE_ACCEPT);
  chooser.set_default_response(Gtk::RESPONSE_ACCEPT);

  chooser.show_all();
  int res = chooser.run();

  if (res != Gtk::RESPONSE_ACCEPT)
    return;
  

  Glib::ustring selected_filename = chooser.get_filename();
  if (selected_filename.empty())
    return;

  if (TileStyleSet::validate_image(selected_filename) == false)
    return;

  if (replace_filename.empty() == false)
    on_remove_tilestyleset_clicked();
  
  Tile *tile = get_selected_tile();
  bool success = d_tileset->addTileStyleSet(tile, selected_filename);
  if (!success)
    return;

  if (replace_filename.empty() == true)
    success = d_tileset->replaceFileInConfigurationFile(replace_filename, 
                                                        selected_filename);
  else
    success = d_tileset->addFileInConfigurationFile(selected_filename);

  if (!success)
    {
      show_add_file_error(d_tileset, chooser, selected_filename);
      return;
    }

  //now make a new one
  TileStyleSet *set = tile->back();
  Gtk::TreeIter i = tilestylesets_list->append();
  (*i)[tilestylesets_columns.name] = set->getName();
  (*i)[tilestylesets_columns.tilestyleset] = set;

  tilestylesets_treeview->set_cursor (Gtk::TreePath (String::ucompose("%1", tile->size() - 1)));

  needs_saving = true;
  update_window_title();
}

void TileSetWindow::on_add_tilestyleset_clicked()
{
  choose_and_add_or_replace_tilestyleset("");
  return;
}

TileStyle * TileSetWindow::get_selected_tilestyle ()
{
  Glib::RefPtr<Gtk::TreeSelection> selection = tilestyles_treeview->get_selection();
  Gtk::TreeModel::iterator iterrow = selection->get_selected();

  if (iterrow) 
    {
      Gtk::TreeModel::Row row = *iterrow;
      return row[tilestyles_columns.tilestyle];
    }
  return NULL;
}

TileStyleSet * TileSetWindow::get_selected_tilestyleset ()
{
  Glib::RefPtr<Gtk::TreeSelection> selection = tilestylesets_treeview->get_selection();
  Gtk::TreeModel::iterator iterrow = selection->get_selected();

  if (iterrow) 
    {
      Gtk::TreeModel::Row row = *iterrow;
      return row[tilestylesets_columns.tilestyleset];
    }
  return NULL;
}

Tile * TileSetWindow::get_selected_tile ()
{
  Glib::RefPtr<Gtk::TreeSelection> selection = tiles_treeview->get_selection();
  Gtk::TreeModel::iterator iterrow = selection->get_selected();

  if (iterrow) 
    {
      Gtk::TreeModel::Row row = *iterrow;
      return row[tiles_columns.tile];
    }
  return NULL;
}

void TileSetWindow::on_remove_tilestyleset_clicked()
{
  //erase the selected row from the treeview
  //remove the tile from the tileset
  Glib::RefPtr<Gtk::TreeSelection> selection = tilestylesets_treeview->get_selection();
  Gtk::TreeModel::iterator iterrow = selection->get_selected();

  if (iterrow) 
    {
      Tile *t = get_selected_tile();
      Gtk::TreeModel::Row row = *iterrow;
      TileStyleSet *s = row[tilestylesets_columns.tilestyleset];
      tilestylesets_list->erase(iterrow);

      for (std::list<TileStyleSet*>::iterator it = t->begin(); it != t->end(); it++)
	{
	  if (*it == s)
	    {
	      t->erase(it);
	      break;
	    }
	}
      needs_saving = true;
      fill_tilestyleset_info(NULL);
      update_window_title();
    }
}

void TileSetWindow::on_tilestyle_changed()
{
  TileStyle *t = get_selected_tilestyle ();
  if (t)
    {
      t->setType(TileStyle::Type(tilestyle_combobox->get_active_row_number()));
      int idx = t->getType();
      tilestyle_standard_image->property_pixbuf() = 
        ImageCache::getInstance()->getDefaultTileStylePic(idx, d_tileset->getTileSize())->to_pixbuf();
    }
}

void TileSetWindow::on_image_chosen()
{
  TileStyleSet *set = get_selected_tilestyleset();

  choose_and_add_or_replace_tilestyleset(set->getName() + ".png");

}

void TileSetWindow::on_organize_tilestyles_activated()
{
  TileStyleOrganizerDialog d(*window, get_selected_tile());
  d.tilestyle_selected.connect (method(on_tilestyle_id_selected));
  d.run_and_hide();
  update_tilestyle_panel();
}

void TileSetWindow::on_smallmap_building_colors_activated()
{
  TilesetSmallmapBuildingColorsDialog d(*window, d_tileset);
  d.run_and_hide();
}
    
void TileSetWindow::on_tilestyle_id_selected(guint32 id)
{
  Tile *t = NULL;
  TileStyleSet *s = NULL;
  TileStyle *style = NULL;
  bool found = d_tileset->getTileStyle(id, &t, &s, &style);
  if (found)
    {
      select_tile(t);
      select_tilestyleset(s);
      select_tilestyle(style);
    }
}

void TileSetWindow::select_tile(Tile *tile)
{
  for (guint32 i = 0; i < tiles_list->children().size(); i++)
    {
      Gtk::TreeIter iter = tiles_list->children()[i];
      Gtk::TreeModel::Row row = *iter;
      Tile *t = row[tiles_columns.tile];
      if (tile == t)
        {
          Glib::RefPtr<Gtk::TreeSelection> s = tiles_treeview->get_selection();
          s->select(row);
          return;
        }
    }
}

void TileSetWindow::select_tilestyleset(TileStyleSet *set)
{
  for (guint32 i = 0; i < tilestylesets_list->children().size(); i++)
    {
      Gtk::TreeIter iter = tilestylesets_list->children()[i];
      Gtk::TreeModel::Row row = *iter;
      TileStyleSet *t = row[tilestylesets_columns.tilestyleset];
      if (set == t)
        {
          Glib::RefPtr<Gtk::TreeSelection> s = 
            tilestylesets_treeview->get_selection();
          s->select(row);
          return;
        }
    }
}

void TileSetWindow::select_tilestyle(TileStyle *style)
{
  for (guint32 i = 0; i < tilestyles_list->children().size(); i++)
    {
      Gtk::TreeIter iter = tilestyles_list->children()[i];
      Gtk::TreeModel::Row row = *iter;
      TileStyle *t = row[tilestyles_columns.tilestyle];
      if (style == t)
        {
          Glib::RefPtr<Gtk::TreeSelection> s = 
            tilestyles_treeview->get_selection();
          s->select(row);
          return;
        }
    }
}

void TileSetWindow::on_preview_tile_activated()
{
  Tile *tile = get_selected_tile();
  if (tile)
    {
      //determine transition tile type
      Tile *sec = NULL;
      int idx = -1;
      if (tile->getType() == Tile::MOUNTAIN)
        idx = d_tileset->getIndex(Tile::HILLS);
      else
        idx = d_tileset->getIndex(Tile::GRASS);
      if (idx > -1)
        sec = (*d_tileset)[idx];
      TilePreviewDialog d(*window, tile, sec, d_tileset->getTileSize());
      d.tilestyle_selected.connect (method(on_tilestyle_id_selected));
      d.run();
    }
}
      
void TileSetWindow::on_roads_picture_activated()
{
  Glib::ustring filename = "";
  if (d_tileset->getRoadsFilename().empty() == false)
    filename = d_tileset->getFileFromConfigurationFile(d_tileset->getRoadsFilename() +".png");
  ImageEditorDialog d(*window, filename, ROAD_TYPES);
  d.set_title(_("Select a roads image"));
  int response = d.run();
  if (filename.empty() == false)
    File::erase(filename);
  if (response == Gtk::RESPONSE_ACCEPT)
    {
      if (d.get_selected_filename() != filename)
        {
          Glib::ustring file = File::get_basename(d.get_selected_filename());
          if (d_tileset->replaceFileInConfigurationFile(d_tileset->getRoadsFilename()+".png", d.get_selected_filename()))
            {
              d_tileset->setRoadsFilename (file);
              needs_saving = true;
              update_window_title();
            }
          else
            show_add_file_error (d_tileset, *d.get_dialog(), file);
        }
    }
}

void TileSetWindow::on_stones_picture_activated()
{
  Glib::ustring filename = "";
  if (d_tileset->getStonesFilename().empty() == false)
    filename = d_tileset->getFileFromConfigurationFile(d_tileset->getStonesFilename() +".png");
  ImageEditorDialog d(*window, filename, STONE_TYPES);
  d.set_title(_("Select a standing stones image"));
  int response = d.run();
  if (filename.empty() == false)
    File::erase(filename);
  if (response == Gtk::RESPONSE_ACCEPT)
    {
      if (d.get_selected_filename() != filename)
        {
          Glib::ustring file = File::get_basename(d.get_selected_filename());
          if (d_tileset->replaceFileInConfigurationFile(d_tileset->getStonesFilename()+".png", d.get_selected_filename()))
            {
              d_tileset->setStonesFilename (file);
              needs_saving = true;
              update_window_title();
            }
          else
            show_add_file_error (d_tileset, *d.get_dialog(), file);
        }
    }
}

void TileSetWindow::on_bridges_picture_activated()
{
  Glib::ustring filename = "";
  if (d_tileset->getBridgesFilename().empty() == false)
    filename = d_tileset->getFileFromConfigurationFile(d_tileset->getBridgesFilename() +".png");
  ImageEditorDialog d(*window, filename, BRIDGE_TYPES);
  d.set_title(_("Select a bridges image"));
  int response = d.run();
  if (filename.empty() == false)
    File::erase(filename);
  if (response == Gtk::RESPONSE_ACCEPT)
    {
      if (d.get_selected_filename() != filename)
        {
          Glib::ustring file = File::get_basename(d.get_selected_filename());
          if (d_tileset->replaceFileInConfigurationFile(d_tileset->getBridgesFilename()+".png", d.get_selected_filename()))
            {
              d_tileset->setBridgesFilename (file);
              needs_saving = true;
              update_window_title();
            }
          else
            show_add_file_error (d_tileset, *d.get_dialog(), file);
        }
    }
}
void TileSetWindow::on_fog_picture_activated()
{
  Glib::ustring filename = "";
  if (d_tileset->getFogFilename().empty() == false)
    filename = d_tileset->getFileFromConfigurationFile(d_tileset->getFogFilename() +".png");
  ImageEditorDialog d(*window, filename, FOG_TYPES);
  d.set_title(_("Select a fog image"));
  int response = d.run();
  if (filename.empty() == false)
    File::erase(filename);
  if (response == Gtk::RESPONSE_ACCEPT)
    {
      if (d.get_selected_filename() != filename)
        {
          Glib::ustring file = File::get_basename(d.get_selected_filename());
          if (d_tileset->replaceFileInConfigurationFile(d_tileset->getFogFilename()+".png", d.get_selected_filename()))
            {
              d_tileset->setFogFilename (file);
              needs_saving = true;
              update_window_title();
            }
          else
            show_add_file_error (d_tileset, *d.get_dialog(), file);
        }
    }
}

void TileSetWindow::on_flags_picture_activated()
{
  TilesetFlagEditorDialog d(*window, d_tileset);
  int response = d.run();
  if (response == Gtk::RESPONSE_ACCEPT)
    {
      needs_saving = true;
      update_window_title();
    }
}

void TileSetWindow::on_army_unit_selector_activated()
{
  TilesetSelectorEditorDialog d(*window, d_tileset);
  int response = d.run();
  if (response == Gtk::RESPONSE_ACCEPT)
    {
      needs_saving = true;
      update_window_title();
    }
}

void TileSetWindow::on_explosion_picture_activated()
{
  TilesetExplosionPictureEditorDialog d(*window, d_tileset);
  int response = d.run();
  if (response == Gtk::RESPONSE_ACCEPT)
    {
      needs_saving = true;
      update_window_title();
    }
}

bool TileSetWindow::load_tileset(Glib::ustring filename)
{
  Glib::ustring old_current_save_filename = current_save_filename;
  current_save_filename = filename;

  bool unsupported_version = false;
  Tileset *tileset = Tileset::create(filename, unsupported_version);
  if (tileset == NULL)
    {
      Glib::ustring msg;
      if (unsupported_version)
        msg = _("Error!  The version of the tileset is unsupported.");
      else
        msg = _("Error!  Tileset could not be loaded.");
      Gtk::MessageDialog dialog(*window, msg);
      current_save_filename = old_current_save_filename;
      dialog.run();
      dialog.hide();
      return false;
    }
  if (d_tileset)
    delete d_tileset;
  d_tileset = tileset;

  bool broken = false;
  d_tileset->instantiateImages(broken);

  tilestyles_list->clear();
  tilestylesets_list->clear();
  tiles_list->clear();
  for (Tileset::iterator i = d_tileset->begin(); i != d_tileset->end(); ++i)
    {
      Gtk::TreeIter l = tiles_list->append();
      (*l)[tiles_columns.name] = (*i)->getName();
      (*l)[tiles_columns.tile] = *i;
    }
  if (d_tileset->size())
    tiles_treeview->set_cursor (Gtk::TreePath ("0"));

  update_tileset_buttons();
  update_tilestyleset_buttons();
  update_tileset_menuitems();
  update_tile_panel();
  update_tilestyleset_panel();
  update_tilestyle_panel();
  update_tile_preview_menuitem();
  update_window_title();
  return true;
}

void TileSetWindow::update_window_title()
{
  Glib::ustring title = "";
  if (needs_saving)
    title += "*";
  title += File::get_basename(current_save_filename, true);
  title += " - ";
  title += _("LordsAWar! Tileset Editor");
  window->set_title(title);
}

void TileSetWindow::on_validate_tileset_activated()
{
  std::list<Glib::ustring> msgs;
  if (d_tileset == NULL)
    return;
  if (d_tileset->empty() == true)
    msgs.push_back(_("There must be at least one tile in the tileset."));

  if (d_tileset->getIndex(Tile::GRASS) == -1)
    msgs.push_back(_("There must be a grass tile in the tileset."));
  if (d_tileset->getIndex(Tile::WATER) == -1)
    msgs.push_back(_("There must be a water tile in the tileset."));
  if (d_tileset->getIndex(Tile::FOREST) == -1)
    msgs.push_back(_("There must be a forest tile in the tileset."));
  if (d_tileset->getIndex(Tile::HILLS) == -1)
    msgs.push_back(_("There must be a hills tile in the tileset."));
  if (d_tileset->getIndex(Tile::MOUNTAIN) == -1)
    msgs.push_back(_("There must be a mountain tile in the tileset."));
  if (d_tileset->getIndex(Tile::SWAMP) == -1)
    msgs.push_back(_("There must be a swamp tile in the tileset."));
  for (Tileset::iterator it = d_tileset->begin(); it != d_tileset->end(); ++it)
    {
      if ((*it)->empty())
        msgs.push_back(String::ucompose(_("There must be at least one tilestyleset in the %1 tile."),(*it)->getName()));
      for (Tile::iterator j = (*it)->begin(); j != (*it)->end(); ++j)
        {
          if ((*j)->validate() == false)
            msgs.push_back(String::ucompose(_("The image %1.png file of the %2 tile does not have a width as a multiple of its height."),(*j)->getName(),(*it)->getName()));
        }

      //fill up the tile style types so we can validate them.
      std::list<TileStyle::Type> types;
      for (Tile::const_iterator i = (*it)->begin(); i != (*it)->end(); ++i)
        (*i)->getUniqueTileStyleTypes(types);

      switch ((*it)->getType())
        {
        case Tile::GRASS:
          if ((*it)->validateGrass(types) == false)
            msgs.push_back(String::ucompose(_("The %1 tile does not have enough of the right kind of tile styles."),(*it)->getName()));
          break;
        case Tile::FOREST: case Tile::WATER: case Tile::HILLS: 
        case Tile::SWAMP: case Tile::MOUNTAIN:
          if ((*it)->validateFeature(types) == false)
            {
              if ((*it)->validateGrass(types) == false)
                msgs.push_back(String::ucompose(_("The %1 tile does not have enough of the right kind of tile styles."),(*it)->getName()));
            }
          break;
        }
    }
  if (d_tileset->countTilesWithPattern(SmallTile::SUNKEN_RADIAL) > 1)
    msgs.push_back(_("Only one tile can have a sunken radial pattern."));

  Glib::ustring msg = "";
  for (std::list<Glib::ustring>::iterator it = msgs.begin(); it != msgs.end();
       it++)
    msg += (*it) + "\n";

  if (msg == "")
    msg = _("The tileset is valid.");
      
  Gtk::MessageDialog dialog(*window, msg);
  dialog.run();
  dialog.hide();

  return;
}

void TileSetWindow::refresh_tiles()
{
  Tileset::iterator j = d_tileset->begin();
  for (Gtk::TreeNodeChildren::iterator i = tiles_list->children().begin();
       i != tiles_list->children().end(); i++, j++)
    (*i)[tiles_columns.tile] = *j;
}

void TileSetWindow::show_add_file_error(Tileset *t, Gtk::Dialog &d, Glib::ustring file)
{
  Glib::ustring m = 
    String::ucompose(_("Couldn't add %1.png to:\n%2"), file, 
                     t->getConfigurationFile());
  Gtk::MessageDialog td(d, m);
  td.run();
  td.hide();
}

TileSetWindow::~TileSetWindow()
{
  notebook->property_show_tabs () = false;
  delete window;
}

void TileSetWindow::on_tutorial_video_activated()
{
  GError *errs = NULL;
  gtk_show_uri(window->get_screen()->gobj(), 
               "http://vimeo.com/96931755", 0, &errs);
  return;
}
