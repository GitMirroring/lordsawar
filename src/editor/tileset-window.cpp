//  Copyright (C) 2008-2012, 2014, 2015, 2017, 2020, 2021 Ben Asselstine
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
#include "image-file-filter.h"

#include "defs.h"
#include "Configuration.h"
#include "tilesetlist.h"
#include "Tile.h"
#include "File.h"
#include "overviewmap.h"
#include "ImageCache.h"
#include "editor-quit-dialog.h"
#include "editor-save-changes-dialog.h"
#include "tilestyle-organizer-dialog.h"
#include "tileset-smallmap-building-colors-dialog.h"
#include "tileset-move-bonus-image-dialog.h"
#include "GameMap.h"
#include "font-size.h"
#include "past-chooser.h"
#include "timed-message-dialog.h"
#include "TarFileMaskedImage.h"
#include "TarFileImage.h"

#define method(x) sigc::mem_fun(*this, &TileSetWindow::x)

TileSetWindow::TileSetWindow(Glib::ustring load_filename)
{
  tileset_modified = load_filename == "";
  new_tileset_needs_saving = load_filename == "";
  umgr = new UndoMgr (UndoMgr::DELAY, UndoMgr::LIMIT);
  umgr->execute ().connect (method (executeAction));
  d_tileset = NULL;
    Glib::RefPtr<Gtk::Builder> xml =
      BuilderCache::editor_get("tileset-window.ui");

    xml->get_widget("window", window);
    window->set_icon_from_file(File::getVariousFile("tileset_icon.png"));
    window->signal_delete_event().connect (sigc::hide(method(on_window_closed)));

    xml->get_widget("tiles_treeview", tiles_treeview);
    xml->get_widget("tile_name_entry", tile_name_entry);

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

    xml->get_widget("tile_moves_spinbutton", tile_moves_spinbutton);
    tile_moves_spinbutton->set_range(double(MIN_MOVES_FOR_TILES),
                                     double(MAX_MOVES_FOR_TILES));
    xml->get_widget("tile_smallmap_first_colorbutton",
		    tile_smallmap_first_colorbutton);
    xml->get_widget("tile_smallmap_second_colorbutton",
		    tile_smallmap_second_colorbutton);
    xml->get_widget("tile_smallmap_third_colorbutton",
		    tile_smallmap_third_colorbutton);
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
    xml->get_widget("edit_undo_menuitem", edit_undo_menuitem);
    edit_undo_menuitem->signal_activate().connect
      (method(on_edit_undo_activated));
    xml->get_widget("edit_redo_menuitem", edit_redo_menuitem);
    edit_redo_menuitem->signal_activate().connect
      (method(on_edit_redo_activated));
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
    xml->get_widget("move_bonus_images_menuitem",
                    move_bonus_images_menuitem);
    move_bonus_images_menuitem->signal_activate().connect
      (method(on_move_bonus_images_activated));
    xml->get_widget ("help_about_menuitem", help_about_menuitem);
    help_about_menuitem->signal_activate().connect
      (method(on_help_about_activated));
    xml->get_widget ("tutorial_menuitem", tutorial_menuitem);
    tutorial_menuitem->signal_activate().connect
      (method(on_tutorial_video_activated));
    xml->get_widget("tilestyle_image", tilestyle_image);

    tiles_list = Gtk::ListStore::create(tiles_columns);
    tiles_treeview->set_model(tiles_list);
    tiles_treeview->append_column("", tiles_columns.name);
    tiles_treeview->set_headers_visible(false);

    xml->get_widget("tilestylesets_treeview", tilestylesets_treeview);
    tilestylesets_list = Gtk::ListStore::create(tilestylesets_columns);
    tilestylesets_treeview->set_model(tilestylesets_list);
    tilestylesets_treeview->append_column("", tilestylesets_columns.name);
    tilestylesets_treeview->set_headers_visible(false);


    xml->get_widget("tilestyles_treeview", tilestyles_treeview);
    tilestyles_list = Gtk::ListStore::create(tilestyles_columns);
    tilestyles_treeview->set_model(tilestyles_list);
    tilestyles_treeview->append_column("", tilestyles_columns.name);
    tilestyles_treeview->set_headers_visible(false);

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

    connect_signals ();
    if (load_filename != "")
      current_save_filename = load_filename;

    if (load_filename.empty () == false)
      load_tileset (load_filename);
      
    update ();
}

void
TileSetWindow::update_tileset_buttons()
{
  if (!tiles_treeview->get_selection ()->get_selected ())
    remove_tile_button->set_sensitive (false);
  else
    remove_tile_button->set_sensitive (true);
  if (d_tileset == NULL)
    add_tile_button->set_sensitive (false);
  else
    add_tile_button->set_sensitive (true);
}

void
TileSetWindow::update_tilestyleset_buttons ()
{
  if (!tilestylesets_treeview->get_selection ()->get_selected ())
    remove_tilestyleset_button->set_sensitive (false);
  else
    remove_tilestyleset_button->set_sensitive (true);
  if (d_tileset == NULL)
    add_tilestyleset_button->set_sensitive (false);
  else
    add_tilestyleset_button->set_sensitive (true);
}

void
TileSetWindow::update_tilestyle_panel()
{
  if (tilestyles_treeview->get_selection ()->get_selected () == 0)
    {
      //clear all values
      tilestyle_alignment->set_sensitive (false);
      tilestyle_combobox->set_active (0);
      tilestyle_image->clear ();
      tilestyle_standard_image->clear ();
      tilestyle_image->show_all ();
      return;
    }
  tilestyle_alignment->set_sensitive (true);
  TileStyle *t = get_selected_tilestyle ();
  if (t)
    {
      int idx = t->getType ();
      tilestyle_combobox->set_active (idx);
      PixMask *p = t->getImage()->copy ();
      double ratio = EDITOR_DIALOG_TILE_PIC_FONTSIZE_MULTIPLE;
      int font_size = FontSize::getInstance ()->get_height ();
      double new_height = font_size * ratio;
      int new_width =
        ImageCache::calculate_width_from_adjusted_height (p, new_height);
      PixMask::scale (p, new_width, new_height);
      tilestyle_image->clear ();
      tilestyle_image->property_pixbuf () = p->to_pixbuf ();
      tilestyle_image->show_all ();
      delete p;

      p = ImageCache::getInstance ()->getDefaultTileStylePic
        (idx, d_tileset->getTileSize ())->copy ();
      ratio = EDITOR_DIALOG_TILE_PIC_FONTSIZE_MULTIPLE;
      font_size = FontSize::getInstance ()->get_height ();
      new_height = font_size * ratio;
      new_width =
        ImageCache::calculate_width_from_adjusted_height (p, new_height);
      PixMask::scale (p, new_width, new_height);
      tilestyle_standard_image->property_pixbuf () = p->to_pixbuf ();
      delete p;
    }
}

void TileSetWindow::fill_tilestyleset_info (TileStyleSet *t)
{
  if (!t || t->getName() == "")
    {
      tilestyles_list->clear();
      image_button->set_label(_("No image set"));
      update_tilestyle_panel();
      return;
    }

  if (t->getName().empty() == false)
    image_button->set_label(t->getName());
  else
    image_button->set_label(_("No image set"));
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
      tile_smallmap_image->clear();
      tile_vbox->set_sensitive(false);
      tile_type_combobox->set_active(0);
      tile_moves_spinbutton->set_value(MIN_MOVES_FOR_TILES);
      tile_name_entry->set_text("");
      tile_smallmap_pattern_combobox->set_active(0);
      tile_smallmap_first_colorbutton->set_rgba(black);
      tile_smallmap_second_colorbutton->set_rgba(black);
      tile_smallmap_third_colorbutton->set_rgba(black);
      tilestylesets_list->clear();
      return;
    }

  tile_vbox->set_sensitive(true);
  fill_tile_info(t);
  fill_tilestylesets();
}

void TileSetWindow::on_new_tileset_activated()
{
  make_new_tileset ();
}

bool TileSetWindow::make_new_tileset ()
{
  Glib::ustring msg = _("Save these changes before making a new Tile Set?");
  if (check_discard (msg) == false)
    return false;
  current_save_filename = "";
  disconnect_signals ();
  tiles_list->clear();
  tilestyles_list->clear();
  tilestylesets_list->clear();
  connect_signals ();
  if (d_tileset)
    delete d_tileset;

  clearUndoAndRedo ();
  guint32 num = 0;
  Glib::ustring name =
    Tilesetlist::getInstance()->findFreeName(_("Untitled"), 100, num,
                                             Tileset::get_default_tile_size ());

  d_tileset = new Tileset (Tilesetlist::getNextAvailableId (1), name);
  d_tileset->setNewTemporaryFile ();
  d_tileset->populateWithDefaultTiles();

  disconnect_signals ();
  for (Tileset::iterator i = d_tileset->begin(); i != d_tileset->end(); ++i)
    {
      Gtk::TreeIter j = tiles_list->append();
      (*j)[tiles_columns.name] = Tile::tileTypeToFriendlyName((*i)->getType());
      (*j)[tiles_columns.tile] = (*i);
    }

  tiles_treeview->set_cursor (Gtk::TreePath ("0"));
  connect_signals ();
  tileset_modified = false;
  new_tileset_needs_saving = true;
  update ();
  return true;
}

void TileSetWindow::on_load_tileset_activated()
{
  load_tileset ();
}

bool TileSetWindow::load_tileset ()
{
  bool ret = false;
  Glib::ustring msg = _("Save these changes before opening a new Tile Set?");
  if (check_discard (msg) == false)
    return ret;
  Gtk::FileChooserDialog chooser(*window,
				 _("Choose a Tile Set to Open"));
  Glib::RefPtr<Gtk::FileFilter> lwt_filter = Gtk::FileFilter::create();
  lwt_filter->set_name(String::ucompose (_("LordsAWar Tile Sets (*%1)"),
                                         TILESET_EXT));
  lwt_filter->add_pattern("*" + TILESET_EXT);
  chooser.add_filter(lwt_filter);
  Glib::RefPtr<Gtk::FileFilter> all_filter = Gtk::FileFilter::create();
  all_filter->set_name(_("All Files"));
  all_filter->add_pattern("*.*");
  chooser.add_filter(all_filter);
  chooser.set_filter (lwt_filter);
  chooser.set_current_folder(File::getSetDir(Tileset::file_extension, false));

  chooser.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
  chooser.add_button(Gtk::Stock::OPEN, Gtk::RESPONSE_ACCEPT);
  chooser.set_default_response(Gtk::RESPONSE_ACCEPT);

  chooser.show_all();
  int res = chooser.run();

  if (res == Gtk::RESPONSE_ACCEPT)
    {
      bool ok = load_tileset (chooser.get_filename ());
      chooser.hide();
      if (ok)
        {
          new_tileset_needs_saving = false;
          tileset_modified = false;
          update_window_title();
          ret = true;
        }
    }

  update ();
  return ret;
}

void TileSetWindow::on_save_as_activated()
{
  if (check_save_valid (false))
    save_current_tileset_file_as ();
}

bool TileSetWindow::save_current_tileset_file_as ()
{
  //Reorder the tileset according to the treeview
  d_tileset->clear();
  for (Gtk::TreeIter i = tiles_list->children().begin(),
       end = tiles_list->children().end(); i != end; ++i)
    d_tileset->push_back((*i)[tiles_columns.tile]);

  bool ret = false;
  while (1)
    {
      Gtk::FileChooserDialog chooser(*window, _("Choose a Name"),
                                     Gtk::FILE_CHOOSER_ACTION_SAVE);
      Glib::RefPtr<Gtk::FileFilter> lwt_filter = Gtk::FileFilter::create();
      lwt_filter->set_name(String::ucompose (_("LordsAWar Tile Sets (*%1)"),
                                             TILESET_EXT));
      lwt_filter->add_pattern("*" + TILESET_EXT);
      chooser.add_filter(lwt_filter);
      chooser.set_current_folder(File::getSetDir(TILESET_EXT, false));

      chooser.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
      chooser.add_button(Gtk::Stock::SAVE, Gtk::RESPONSE_ACCEPT);
      chooser.set_default_response(Gtk::RESPONSE_ACCEPT);
      chooser.set_do_overwrite_confirmation();
      chooser.set_current_name (File::sanify(d_tileset->getName ()) +
                                TILESET_EXT);

      chooser.show_all();
      int res = chooser.run();

      if (res == Gtk::RESPONSE_ACCEPT)
        {
          Glib::ustring filename = chooser.get_filename();
          Glib::ustring old_filename = current_save_filename;
          guint32 old_id = d_tileset->getId ();
          d_tileset->setId(Tilesetlist::getNextAvailableId(old_id));

          ret = save_current_tileset_file(filename);
          if (ret == false)
            {
              current_save_filename = old_filename;
              d_tileset->setId(old_id);
            }
          else
            {
              new_tileset_needs_saving = false;
              tileset_modified = false;
              d_tileset->created (filename);
              Glib::ustring dir =
                File::add_slash_if_necessary (File::get_dirname (filename));
              if (dir == File::getSetDir(TILESET_EXT, false) ||
                  dir == File::getSetDir(TILESET_EXT, true))
                {
                  //if we saved it to a standard place, update the list
                  Tilesetlist::getInstance()->add (Tileset::copy (d_tileset),
                                                   filename);
                  tileset_saved.emit(d_tileset->getId());
                }
              refresh_tiles ();
              update_window_title();
            }
        }
      chooser.hide ();
      if (res == Gtk::RESPONSE_CANCEL)
        break;
      if (ret == true)
        break;
    }
  return ret;
}

bool TileSetWindow::save_current_tileset_file (Glib::ustring filename)
{

  //Reorder the tileset according to the treeview
  d_tileset->clear();
  for (Gtk::TreeIter i = tiles_list->children().begin(),
       end = tiles_list->children().end(); i != end; ++i)
    d_tileset->push_back((*i)[tiles_columns.tile]);

  current_save_filename = filename;
  if (current_save_filename.empty())
    current_save_filename = d_tileset->getConfigurationFile(true);

  bool ok = d_tileset->save(current_save_filename, Tileset::file_extension);
  if (ok)
    {
      if (Tilesetlist::getInstance()->reload(d_tileset->getId()))
        refresh_tiles();
      new_tileset_needs_saving = false;
      tileset_modified = false;
      update_window_title();
      tileset_saved.emit(d_tileset->getId());
    }
  else
    {
      Glib::ustring errmsg = Glib::strerror(errno);
      Glib::ustring msg = _("Error!  Tile Set could not be saved.");
      msg += "\n" + current_save_filename + "\n" + errmsg;
      TimedMessageDialog dialog(*window, msg, 0);
      dialog.run_and_hide ();
    }
  return ok;
}

void TileSetWindow::on_save_tileset_activated()
{
  if (current_save_filename.empty() == true)
    on_save_as_activated ();
  else
    {
      if (check_save_valid (true))
        save_current_tileset_file();
    }
}

bool TileSetWindow::quit()
{
  if (tileset_modified || new_tileset_needs_saving)
    {
      EditorQuitDialog d (*window);
      int response = d.run_and_hide();

      if (response == Gtk::RESPONSE_CANCEL) // we don't want to new
        return false;

      else if (response == Gtk::RESPONSE_ACCEPT) // save it
        {
          bool saved = false;
          bool existing = d_tileset->getDirectory ().empty () == false;
          if (existing)
            {
              if (check_save_valid (true))
                {
                  if (save_current_tileset_file ())
                    saved = true;
                }
              else
                return false;
            }
          else
            {
              if (check_save_valid (false))
                saved = save_current_tileset_file_as ();
              else
                return false;
            }
          if (!saved)
            return false;
        }
    }
  window->hide ();
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
  TileSetInfoDialog d(*window, d_tileset);
  bool changed = d.run();
  if (changed)
    {
      TileSetEditorAction_Properties *action = 
        new TileSetEditorAction_Properties
        (d_tileset->getName (), 
         d_tileset->getInfo (),
         d_tileset->getCopyright (),
         d_tileset->getLicense (),
         d_tileset->getTileSize ());
      addUndo (action);
      d_tileset->setName (d.getName ());
      d_tileset->setInfo (d.getDescription ());
      d_tileset->setCopyright (d.getCopyright ());
      d_tileset->setLicense (d.getLicense ());
      d_tileset->setTileSize (d.getTileSize ());
      dirty ();
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

void TileSetWindow::on_tile_selected ()
{
  update ();
}

void TileSetWindow::on_tilestyleset_selected ()
{
  update ();
}

void TileSetWindow::on_tilestyle_selected ()
{
  disconnect_signals ();
  update_tilestyle_panel ();
  connect_signals ();
}

void TileSetWindow::fill_tilestylesets()
{
  Tile *t = get_selected_tile();
  if (!t)
    return;
  tilestylesets_list->clear();
  for (std::list<TileStyleSet*>::iterator it = t->begin(); it != t->end(); ++it)
    {
      Gtk::TreeIter l = tilestylesets_list->append();
      (*l)[tilestylesets_columns.name] = (*it)->getName();
      (*l)[tilestylesets_columns.tilestyleset] = *it;
    }
  tilestylesets_treeview->set_cursor (Gtk::TreePath ("0"));
}

void TileSetWindow::fill_tile_info(Tile *tile)
{
  tile_name_entry->set_text(tile->getName());
  tile_name_entry->set_position (tile_name_entry->get_text_length ());
  tile_type_combobox->set_active(tile->getTypeIndex());
  tile_moves_spinbutton->set_value(tile->getMoves());
  tile_smallmap_pattern_combobox->set_active(tile->getSmallTile()->getPattern());
  tile_smallmap_first_colorbutton->set_sensitive(true);
  fill_tilestylesets();
  fill_colours(tile);
  fill_tile_smallmap(tile);
}

void TileSetWindow::on_add_tile_clicked()
{
  TileSetEditorAction_AddTile *action =
    new TileSetEditorAction_AddTile (d_tileset);
  addUndo (action);
  //add a new empty tile to the tileset
  Tile *t = new Tile();
  //add it to the treeview
  Gtk::TreeIter i = tiles_list->append();
  t->setName(_("Untitled"));
  (*i)[tiles_columns.name] = t->getName();
  (*i)[tiles_columns.tile] = t;
  d_tileset->push_back(t);
  tiles_treeview->set_cursor (Gtk::TreePath (String::ucompose("%1", d_tileset->size() - 1)));
  update_menuitems ();
  dirty ();
}

bool TileSetWindow::remove_tilestyleset_files (Tile *a)
{
  bool broken = false;
  bool first = true;
  for (auto b : *a)
    {
      Glib::ustring imgname = b->getName ();
      bool ret = d_tileset->removeFileInCfgFile(imgname);
      if (!ret)
        {
          broken = true;
          if (first)
            {
              first = false;
              show_remove_file_error(d_tileset, *window, imgname);
            }
        }
    }
  return broken == false;
}

void TileSetWindow::on_remove_tile_clicked()
{
  //erase the selected row from the treeview
  //remove the tile from the tileset
  Glib::RefPtr<Gtk::TreeSelection> selection = tiles_treeview->get_selection();
  Gtk::TreeModel::iterator iterrow = selection->get_selected();

  if (iterrow)
    {
      TileSetEditorAction_RemoveTile *action =
        new TileSetEditorAction_RemoveTile (d_tileset);
      addUndo (action);
      Gtk::TreeModel::Row row = *iterrow;
      Tile *a = row[tiles_columns.tile];
      if (a)
        remove_tilestyleset_files (a);
      tiles_list->erase(iterrow);

      for (std::vector<Tile*>::iterator it = d_tileset->begin();
           it != d_tileset->end(); ++it)
        {
          if (*it == a)
            {
              d_tileset->erase(it);
              break;
            }
        }

      dirty ();
    }
  update_menuitems ();
}

void TileSetWindow::on_tile_first_color_changed()
{
  Glib::RefPtr<Gtk::TreeSelection> selection = tiles_treeview->get_selection();
  Gtk::TreeModel::iterator iterrow = selection->get_selected();

  if (iterrow)
    {
      Gtk::TreeModel::Row row = *iterrow;
      Tile *t = row[tiles_columns.tile];
      TileSetEditorAction_Colour *action =
        new TileSetEditorAction_Colour (getCurIndex (), 0, 
                                         t->getSmallTile ()->getColor ());
      addUndo (action);
      t->getSmallTile()->setColor(tile_smallmap_first_colorbutton->get_rgba());
      fill_tile_smallmap(t);
      dirty ();
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
      TileSetEditorAction_Colour *action =
        new TileSetEditorAction_Colour (getCurIndex (), 1, 
                                         t->getSmallTile ()->getSecondColor ());
      addUndo (action);
      t->getSmallTile()->setSecondColor(tile_smallmap_second_colorbutton->get_rgba());
      fill_tile_smallmap(t);
      dirty ();
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
      TileSetEditorAction_Colour *action =
        new TileSetEditorAction_Colour (getCurIndex (), 2, 
                                         t->getSmallTile ()->getThirdColor ());
      addUndo (action);
      t->getSmallTile()->setThirdColor(tile_smallmap_third_colorbutton->get_rgba());
      fill_tile_smallmap(t);
      dirty ();
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
      TileSetEditorAction_Pattern *action =
        new TileSetEditorAction_Pattern (getCurIndex (),
                                         t->getSmallTile ()->getPattern ());
      addUndo (action);
      int idx = tile_smallmap_pattern_combobox->get_active_row_number();
      SmallTile::Pattern pattern = SmallTile::Pattern(idx);
      t->getSmallTile()->setPattern(pattern);
      fill_colours(t);
      fill_tile_smallmap(t);
      dirty ();
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
      TileSetEditorAction_Type *action =
        new TileSetEditorAction_Type (getCurIndex (), t->getType ());
      addUndo (action);
      t->setTypeByIndex(idx);
      dirty ();
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
      TileSetEditorAction_Name *action =
        new TileSetEditorAction_Name (getCurIndex (), t->getName (),
                                      tile_name_entry->get_position ());
      addUndo (action);
      t->setName(tile_name_entry->get_text());

      dirty ();
    }
}

void TileSetWindow::fill_tile_smallmap(Tile *tile)
{
  double ratio = EDITOR_DIALOG_TILE_PIC_FONTSIZE_MULTIPLE;
  int font_size = FontSize::getInstance()->get_height ();
  double height = font_size * ratio;
  int width = height;
  SmallTile *s = tile->getSmallTile();
  Cairo::RefPtr<Cairo::Surface> tile_smallmap_surface= Cairo::ImageSurface::create (Cairo::FORMAT_ARGB32, width, height);
  Cairo::RefPtr<Cairo::Context> tile_smallmap_surface_gc;
  tile_smallmap_surface_gc = Cairo::Context::create(tile_smallmap_surface);
  if (s->getPattern() == SmallTile::SUNKEN_RADIAL)
    OverviewMap::draw_radial_gradient(tile_smallmap_surface, s->getColor(),
                                      s->getSecondColor(), width, height);
  for (int i = 0; i < width; i++)
    {
      for (int j = 0; j < height; j++)
	{
	  bool shadowed = false;
	  if (i == width - 1)
	    shadowed = true;
	  else if (j == height - 1)
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
    Gdk::Pixbuf::create(tile_smallmap_surface, 0, 0, width, height);

  tile_smallmap_image->property_pixbuf() = pixbuf;
  tile_smallmap_image->queue_draw();
}

void TileSetWindow::choose_and_add_or_replace_tilestyleset(Glib::ustring replace_filename)
{
  Gtk::FileChooserDialog chooser(*window, _("Choose an Image"),
				 Gtk::FILE_CHOOSER_ACTION_OPEN);
  ImageFileFilter::getInstance()->add (&chooser);
  chooser.set_current_folder(PastChooser::getInstance()->get_dir(&chooser));

  chooser.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
  chooser.add_button(Gtk::Stock::OPEN, Gtk::RESPONSE_ACCEPT);
  if (replace_filename.empty () == false)
    chooser.add_button(Gtk::Stock::CLEAR, Gtk::RESPONSE_REJECT);
  chooser.set_default_response(Gtk::RESPONSE_ACCEPT);

  chooser.show_all();
  int res = chooser.run();

  if (res == Gtk::RESPONSE_CANCEL)
    return;

  if (res == Gtk::RESPONSE_REJECT)
    {
      bool ret = remove_selected_tilestyleset (&chooser);
      if (ret == false)
        return;
    }
  else if (res == Gtk::RESPONSE_ACCEPT)
    {
      Glib::ustring selected_filename = chooser.get_filename();
      if (selected_filename.empty())
        return;
      if (ImageFileFilter::getInstance()->hasInvalidExt (selected_filename))
        {
          ImageFileFilter::getInstance ()->showErrorDialog (&chooser);
          return;
        }

      bool broken = false;
      PixMask *p = PixMask::create (chooser.get_filename (), broken);
      if (p)
        delete p;
      if (broken)
        {
          TimedMessageDialog
            td(chooser,
               String::ucompose(_("Couldn't make sense of the image:\n%1"),
                                chooser.get_filename ()), 0);
          td.run_and_hide();
          return;
        }

      if (TileStyleSet::validate_image(selected_filename) == false)
        {
          TimedMessageDialog
            td(chooser, _("The image width is not a multiple of the height."),
               0);
          td.run_and_hide ();
          return;
        }

      TileSetEditorAction_AddTileStyleSet *action =
        new TileSetEditorAction_AddTileStyleSet (d_tileset);
      if (replace_filename.empty() == false)
        {
          bool ret = remove_selected_tilestyleset (&chooser);
          if (ret == false)
            {
              delete action;
              return;
            }
        }

      Tile *tile = get_selected_tile();
      bool success = d_tileset->addTileStyleSet(tile, selected_filename);
      if (!success)
        {
          delete action;
          return;
        }

      Glib::ustring newname = "";
      success = d_tileset->addFileInCfgFile(selected_filename, newname);

      if (!success)
        {
          delete action;
          show_add_file_error(d_tileset, chooser, selected_filename);
          return;
        }

      addUndo (action);
      PastChooser::getInstance()->set_dir(&chooser);
      //now make a new one
      TileStyleSet *set = tile->back();
      set->setName(newname);
      set->instantiateImages(d_tileset);
      Gtk::TreeIter i = tilestylesets_list->append();
      (*i)[tilestylesets_columns.name] = set->getName();
      (*i)[tilestylesets_columns.tilestyleset] = set;

      tilestylesets_treeview->set_cursor (Gtk::TreePath (String::ucompose("%1", tile->size() - 1)));
    }

  dirty ();
  update ();
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

bool TileSetWindow::remove_selected_tilestyleset (Gtk::Window *d)
{
  bool ret = false;
  //erase the selected row from the treeview
  //remove the tile from the tileset
  Glib::RefPtr<Gtk::TreeSelection> selection =
    tilestylesets_treeview->get_selection();
  Gtk::TreeModel::iterator iterrow = selection->get_selected();

  if (iterrow)
    {
      Gtk::TreeModel::Row row = *iterrow;
      TileStyleSet *s = row[tilestylesets_columns.tilestyleset];

      TileSetEditorAction_RemoveTileStyleSet *action =
        new TileSetEditorAction_RemoveTileStyleSet (d_tileset);
      Glib::ustring imgname = s->getName ();
      ret = d_tileset->removeFileInCfgFile(imgname);
      if (!ret)
        {
          delete action;
          show_remove_file_error(d_tileset, *d, imgname);
          return ret;
        }
      addUndo (action);
      tilestylesets_list->erase(iterrow);

      d_tileset->uninstantiateSameNamedImages (imgname);
      Tile *tile = get_selected_tile ();
      if (tile)
        tile->remove (s);
      ret = true;
      dirty ();
    }
  return ret;
}

void TileSetWindow::on_remove_tilestyleset_clicked()
{
  remove_selected_tilestyleset (window);
  update ();
}

void TileSetWindow::dirty ()
{
  tileset_modified = true;
  update_window_title ();
  update_menuitems ();
}

void TileSetWindow::on_tilestyle_changed()
{
  TileStyle *t = get_selected_tilestyle ();
  if (t)
    {
      TileSetEditorAction_TileStyle *action =
        new TileSetEditorAction_TileStyle (getCurIndex (), t->getId (),
                                           t->getType ());
      addUndo (action);
      dirty ();
      t->setType(TileStyle::Type(tilestyle_combobox->get_active_row_number()));
      int idx = t->getType();
      if (idx >= 0)
        {
          int ts = d_tileset->getTileSize();
          PixMask *p =
            ImageCache::getInstance()->getDefaultTileStylePic(idx, ts)->copy ();
          double ratio = EDITOR_DIALOG_TILE_PIC_FONTSIZE_MULTIPLE;
          int font_size = FontSize::getInstance()->get_height ();
          double new_height = font_size * ratio;
          int new_width =
            ImageCache::calculate_width_from_adjusted_height (p, new_height);
          PixMask::scale (p, new_width, new_height);
          tilestyle_standard_image->property_pixbuf() = p->to_pixbuf ();
          delete p;
        }
    }
}

void TileSetWindow::on_image_chosen()
{
  TileStyleSet *set = get_selected_tilestyleset();
  if (set)
    choose_and_add_or_replace_tilestyleset(set->getName());
}

void TileSetWindow::on_organize_tilestyles_activated()
{ 
  TileSetEditorAction_TileStyles *action =
    new TileSetEditorAction_TileStyles (d_tileset);
  TileStyleOrganizerDialog d(*window, get_selected_tile());
  if (d.run ())
    {
      addUndo (action);
      dirty ();
      update ();
    }
  else
    delete action;
}

void TileSetWindow::on_smallmap_building_colors_activated()
{
  TileSetEditorAction_BuildingColours *action =
    new TileSetEditorAction_BuildingColours (d_tileset);
  TilesetSmallmapBuildingColorsDialog d(*window, d_tileset);
  d.run_and_hide();
  if (d.get_changed ())
    {
      addUndo (action);
      dirty ();
    }
  else
    delete action;
}

void TileSetWindow::on_move_bonus_images_activated()
{
  TileSetEditorAction_MoveBonus *action =
    new TileSetEditorAction_MoveBonus (d_tileset);
  TilesetMoveBonusImageDialog d(*window, d_tileset);
  d.run_and_hide();
  if (d.get_changed ())
    {
      addUndo (action);
      dirty ();
    }
  else
    delete action;
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
      TilePreviewDialog d(*window, tile, sec);
      d.tilestyle_selected.connect (method(on_tilestyle_id_selected));
      d.run();
    }
}

void TileSetWindow::on_roads_picture_activated()
{
  Glib::ustring imgname = d_tileset->getRoad()->getName();
  ImageEditorDialog d(*window, d_tileset->getRoad (),
                      EDITOR_DIALOG_TILE_PIC_FONTSIZE_MULTIPLE);
  d.set_title(_("Select a roads image"));
  int response = d.run();
  if (response == Gtk::RESPONSE_ACCEPT && d.get_filename() != "")
    {
      TileSetEditorAction_Roads *action =
        new TileSetEditorAction_Roads (d_tileset);
      Glib::ustring newname = "";
      bool success = false;
      if (imgname.empty() == true)
        success =
          d_tileset->addFileInCfgFile(d.get_filename(), newname);
      else
        success =
          d_tileset->replaceFileInCfgFile(imgname, d.get_filename(), newname);
      if (success)
        {
          addUndo (action);
          d_tileset->getRoad()->load (d_tileset, newname);
          d_tileset->getRoad()->instantiateImages ();
          dirty ();
        }
      else
        {
          delete action;
          show_add_file_error (d_tileset, *d.get_dialog(), d.get_filename ());
        }
    }
  else if (response == Gtk::RESPONSE_REJECT)
    {
      TileSetEditorAction_Roads *action =
        new TileSetEditorAction_Roads (d_tileset);
      if (d_tileset->removeFileInCfgFile(imgname))
        {
          addUndo (action);
          d_tileset->uninstantiateSameNamedImages (imgname);
          dirty ();
        }
      else
        {
          delete action;
          show_remove_file_error(d_tileset, *d.get_dialog(), imgname);
        }
    }
}

void TileSetWindow::on_stones_picture_activated()
{
  Glib::ustring imgname = d_tileset->getStone()->getName();
  ImageEditorDialog d(*window, d_tileset->getStone (),
                      EDITOR_DIALOG_TILE_PIC_FONTSIZE_MULTIPLE);
  d.set_title(_("Select a standing stones image"));
  int response = d.run();
  if (response == Gtk::RESPONSE_ACCEPT && d.get_filename() != "")
    {
      TileSetEditorAction_Stones *action =
        new TileSetEditorAction_Stones (d_tileset);
      Glib::ustring newname = "";
      bool success = false;
      if (imgname.empty() == true)
        success =
          d_tileset->addFileInCfgFile(d.get_filename(), newname);
      else
        success =
          d_tileset->replaceFileInCfgFile(imgname, d.get_filename(), newname);
      if (success)
        {
          addUndo (action);
          d_tileset->getStone()->load (d_tileset, newname);
          d_tileset->getStone()->instantiateImages ();
          dirty ();
        }
      else
        {
          delete action;
          show_add_file_error (d_tileset, *d.get_dialog(), d.get_filename ());
        }
    }
  else if (response == Gtk::RESPONSE_REJECT)
    {
      TileSetEditorAction_Stones *action =
        new TileSetEditorAction_Stones (d_tileset);
      if (d_tileset->removeFileInCfgFile(imgname))
        {
          addUndo (action);
          d_tileset->uninstantiateSameNamedImages (imgname);
          dirty ();
        }
      else
        {
          delete action;
          show_remove_file_error(d_tileset, *d.get_dialog(), imgname);
        }
    }
}

void TileSetWindow::on_bridges_picture_activated()
{
  Glib::ustring imgname = d_tileset->getBridge()->getName();
  ImageEditorDialog d(*window, d_tileset->getBridge (),
                      EDITOR_DIALOG_TILE_PIC_FONTSIZE_MULTIPLE);
  d.set_title(_("Select a bridges image"));
  int response = d.run();
  if (response == Gtk::RESPONSE_ACCEPT && d.get_filename() != "")
    {
      TileSetEditorAction_Bridges *action =
        new TileSetEditorAction_Bridges (d_tileset);
      Glib::ustring newname = "";
      bool success = false;
      if (imgname.empty () == true)
        success =
          d_tileset->addFileInCfgFile(d.get_filename(), newname);
      else
        success =
          d_tileset->replaceFileInCfgFile(imgname, d.get_filename(), newname);
      if (success)
        {
          addUndo (action);
          d_tileset->getBridge ()->load (d_tileset, newname);
          d_tileset->getBridge ()->instantiateImages ();
          dirty ();
        }
      else
        {
          delete action;
          show_add_file_error (d_tileset, *d.get_dialog(), d.get_filename ());
        }
    }
  else if (response == Gtk::RESPONSE_REJECT)
    {
      TileSetEditorAction_Bridges *action =
        new TileSetEditorAction_Bridges (d_tileset);
      if (d_tileset->removeFileInCfgFile(imgname))
        {
          addUndo (action);
          d_tileset->uninstantiateSameNamedImages (imgname);
          dirty ();
        }
      else
        {
          delete action;
          show_remove_file_error(d_tileset, *d.get_dialog(), imgname);
        }
    }
}

void TileSetWindow::on_fog_picture_activated()
{
  Glib::ustring imgname = d_tileset->getFog()->getName();
  ImageEditorDialog d(*window, d_tileset->getFog (),
                      EDITOR_DIALOG_TILE_PIC_FONTSIZE_MULTIPLE);
  d.set_title(_("Select a fog image"));
  int response = d.run();
  if (response == Gtk::RESPONSE_ACCEPT && d.get_filename() != "")
    {
      TileSetEditorAction_Fog *action =
        new TileSetEditorAction_Fog (d_tileset);
      Glib::ustring newname = "";
      bool success = false;
      if (imgname.empty () == true)
        success = d_tileset->addFileInCfgFile(d.get_filename(), newname);
      else
        success =
          d_tileset->replaceFileInCfgFile(imgname, d.get_filename(), newname);
      if (success)
        {
          addUndo (action);
          d_tileset->getFog ()->load(d_tileset, newname);
          d_tileset->getFog ()->instantiateImages ();
          dirty ();
        }
      else
        {
          delete action;
          show_add_file_error (d_tileset, *d.get_dialog(), d.get_filename ());
        }
    }
  else if (response == Gtk::RESPONSE_REJECT)
    {
      TileSetEditorAction_Fog *action =
        new TileSetEditorAction_Fog (d_tileset);
      if (d_tileset->removeFileInCfgFile(imgname))
        {
          addUndo (action);
          d_tileset->uninstantiateSameNamedImages (imgname);
          dirty ();
        }
      else
        {
          delete action;
          show_remove_file_error(d_tileset, *d.get_dialog(), imgname);
        }
    }
}

void TileSetWindow::on_flags_picture_activated()
{
  TileSetEditorAction_Flag *action = new TileSetEditorAction_Flag (d_tileset);
  TilesetFlagEditorDialog d(*window, d_tileset);
  if (d.run())
    {
      addUndo (action);
      dirty ();
    }
  else
    delete action;
}

void TileSetWindow::on_army_unit_selector_activated()
{
  TileSetEditorAction_Selector *action =
    new TileSetEditorAction_Selector (d_tileset);
  TilesetSelectorEditorDialog d(*window, d_tileset);
  if (d.run ())
    {
      addUndo (action);
      dirty ();
    }
  else
    delete action;
}

void TileSetWindow::on_explosion_picture_activated()
{
  TileSetEditorAction_Explosion *action =
    new TileSetEditorAction_Explosion (d_tileset);
  TilesetExplosionPictureEditorDialog d(*window, d_tileset);
  if (d.run())
    {
      addUndo (action);
      dirty ();
    }
  else
    delete action;
}

bool TileSetWindow::load_tileset (Glib::ustring filename)
{
  Glib::ustring old_current_save_filename = current_save_filename;
  current_save_filename = filename;

  bool unsupported_version = false;
  Tileset *tileset = Tileset::create(filename, unsupported_version);
  if (tileset == NULL)
    {
      Glib::ustring msg;
      if (unsupported_version)
        msg = _("Error!  The version of Tile Set is unsupported.");
      else
        msg = _("Error!  Tile Set could not be loaded.");
      TimedMessageDialog dialog(*window, msg, 0);
      current_save_filename = old_current_save_filename;
      dialog.run_and_hide ();
      return false;
    }
  if (d_tileset)
    delete d_tileset;
  d_tileset = tileset;
  d_tileset->setLoadTemporaryFile ();

  bool broken = false;
  d_tileset->instantiateImages(false, broken);
  if (broken)
    {
      delete d_tileset;
      d_tileset = NULL;
      TimedMessageDialog td(*window, _("Couldn't load Tile Set images."), 0);
      td.run_and_hide ();
      return false;
    }

  disconnect_signals ();
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
  connect_signals ();
  clearUndoAndRedo ();

  return true;
}

void TileSetWindow::update_window_title()
{
  Glib::ustring title = "";
  if (tileset_modified || new_tileset_needs_saving)
    title += "*";
  title += d_tileset->getName();
  title += " - ";
  title += _("Tile Set Editor");
  window->set_title(title);
}

void TileSetWindow::on_validate_tileset_activated()
{
  std::list<Glib::ustring> msgs;
  if (d_tileset == NULL)
    return;
  bool valid = String::utrim (d_tileset->getName ()) != "";
  if (!valid)
    msgs.push_back (_("The name of the Tile Set is invalid."));
  if (d_tileset->empty() == true)
    msgs.push_back(_("There must be at least one tile in the Tile Set."));
  if (d_tileset->getIndex(Tile::GRASS) == -1)
    msgs.push_back(_("There must be a grass tile in the Tile Set."));
  if (d_tileset->getIndex(Tile::WATER) == -1)
    msgs.push_back(_("There must be a water tile in the Tile Set."));
  if (d_tileset->getIndex(Tile::FOREST) == -1)
    msgs.push_back(_("There must be a forest tile in the Tile Set."));
  if (d_tileset->getIndex(Tile::HILLS) == -1)
    msgs.push_back(_("There must be a hills tile in the Tile Set."));
  if (d_tileset->getIndex(Tile::MOUNTAIN) == -1)
    msgs.push_back(_("There must be a mountain tile in the Tile Set."));
  if (d_tileset->getIndex(Tile::SWAMP) == -1)
    msgs.push_back(_("There must be a swamp tile in the Tile Set."));
  for (Tileset::iterator it = d_tileset->begin(); it != d_tileset->end(); ++it)
    {
      if ((*it)->empty())
        msgs.push_back(String::ucompose(_("There must be at least one tilestyleset in the %1 tile."),(*it)->getName()));
      for (Tile::iterator j = (*it)->begin(); j != (*it)->end(); ++j)
        {
          if ((*j)->validate() == false)
            msgs.push_back(String::ucompose(_("The image %1 file of the %2 tile does not have a width as a multiple of its height."),(*j)->getName(),(*it)->getName()));
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

  if (d_tileset->getSelector(true)->getName().empty () == true)
    msgs.push_back(_("A large selector image is required."));
  if (d_tileset->getSelector(false)->getName().empty () == true)
    msgs.push_back(_("A small selector image is required."));
  if (d_tileset->getExplosion()->getName().empty () == true)
    msgs.push_back(_("An explosion image is required."));
  if (d_tileset->getRoad()->getName().empty () == true)
    msgs.push_back(_("A roads image is required."));
  if (d_tileset->getStone()->getName().empty () == true)
    msgs.push_back(_("A standing stones image is required."));
  if (d_tileset->getBridge()->getName ().empty () == true)
    msgs.push_back(_("A bridges image is required."));
  if (d_tileset->getFog()->getName ().empty () == true)
    msgs.push_back(_("A set of fog images are required."));
  if (d_tileset->getFlags()->getName().empty () == true)
    msgs.push_back(_("A set of flag images are required."));
  if (d_tileset->getAllMoveBonus ()->getName ().empty () == true)
    msgs.push_back(_("An all (flight) movement bonus image is required."));
  if (d_tileset->getWaterMoveBonus ()->getName ().empty () == true)
    msgs.push_back(_("A water movement bonus image is required."));
  if (d_tileset->getForestMoveBonus ()->getName ().empty () == true)
    msgs.push_back(_("A forest movement bonus image is required."));
  if (d_tileset->getHillsMoveBonus ()->getName ().empty () == true)
    msgs.push_back(_("A hills movement bonus image is required."));
  if (d_tileset->getMountainsMoveBonus ()->getName ().empty () == true)
    msgs.push_back(_("A mountains movement bonus image is required."));
  if (d_tileset->getSwampMoveBonus ()->getName ().empty () == true)
    msgs.push_back(_("A swamp movement bonus image is required."));

  if (isValidName () == false)
    msgs.push_back(_("The name of the Tile Set is not unique."));

  Glib::ustring msg = "";
  for (std::list<Glib::ustring>::iterator it = msgs.begin(); it != msgs.end();
       ++it)
    {
      msg += (*it) + "\n";
      break;
    }

  if (msg == "")
    msg = _("The Tile Set is valid.");

  TimedMessageDialog dialog(*window, msg, 0);
  dialog.run_and_hide ();

  return;
}

void TileSetWindow::refresh_tiles()
{
  Tileset::iterator j = d_tileset->begin();
  for (Gtk::TreeNodeChildren::iterator i = tiles_list->children().begin();
       i != tiles_list->children().end(); ++i, ++j)
    (*i)[tiles_columns.tile] = *j;
}

void TileSetWindow::show_add_file_error(Tileset *t, Gtk::Dialog &d, Glib::ustring file)
{
  Glib::ustring errmsg = Glib::strerror(errno);
  Glib::ustring m =
    String::ucompose(_("Couldn't add %1 to:\n%2\n%3"), file,
                     t->getConfigurationFile(), errmsg);
  TimedMessageDialog td(d, m, 0);
  td.run_and_hide ();
}

void TileSetWindow::show_remove_file_error(Tileset *t, Gtk::Window &d, Glib::ustring file)
{
  Glib::ustring errmsg = Glib::strerror(errno);
  Glib::ustring m =
    String::ucompose(_("Couldn't remove %1 from:\n%2\n%3"), file,
                     t->getConfigurationFile(), errmsg);
  TimedMessageDialog td(d, m, 0);
  td.run_and_hide ();
}

TileSetWindow::~TileSetWindow()
{
  notebook->property_show_tabs () = false;
  delete umgr;
  delete window;
}

void TileSetWindow::on_tutorial_video_activated()
{
  GError *errs = NULL;
  gtk_show_uri(window->get_screen()->gobj(),
               "http://vimeo.com/407781865", 0, &errs);
  return;
}

bool TileSetWindow::check_discard (Glib::ustring msg)
{
  if (tileset_modified || new_tileset_needs_saving)
    {
      EditorSaveChangesDialog d (*window, msg);
      int response = d.run_and_hide();

      if (response == Gtk::RESPONSE_CANCEL) // we don't want to new
        return false;

      else if (response == Gtk::RESPONSE_ACCEPT) // save it
        {
          if (check_save_valid (true))
            {
              bool saved = false;
              if (d_tileset->getDirectory ().empty () == false)
                  saved = save_current_tileset_file_as ();
              else
                {
                  if (save_current_tileset_file ())
                    saved = true;
                }
              if (!saved)
                return false;
            }
          else
            return false;
        }
    }
  return true;
}

bool TileSetWindow::check_save_valid (bool existing)
{
  if (check_name_valid (existing) == false)
    return false;

  if (d_tileset->validate () == false)
    {
      if (existing &&
          GameMap::getInstance()->getTilesetId() == d_tileset->getId())
        {
          Glib::ustring errmsg =
            _("Tile Set is invalid, and is also the current working Tile Set.");
          Glib::ustring msg = _("Error!  Tile Set could not be saved.");
          msg += "\n" + current_save_filename + "\n" + errmsg;
          TimedMessageDialog dialog(*window, msg, 0);
          dialog.run_and_hide ();
          return false;
        }
      else
        {
          TimedMessageDialog
            dialog(*window,
                   _("The Tile Set is invalid.  Do you want to proceed?"), 0);
          dialog.add_cancel_button ();
          dialog.run_and_hide ();
          if (dialog.get_response () == Gtk::RESPONSE_CANCEL)
            return false;
        }
    }
  return true;
}

bool TileSetWindow::check_name_valid (bool existing)
{
  Glib::ustring name = d_tileset->getName ();
  Glib::ustring newname = "";
  if (existing)
    {
      Tileset *oldtileset =
        Tilesetlist::getInstance ()->get(d_tileset->getId());
      if (oldtileset && oldtileset->getName () != name)
          newname = oldtileset->getName ();
    }
  guint32 num = 0;
  Glib::ustring n = String::utrim (String::strip_trailing_numbers (name));
  if (n == "")
    n = _("Untitled");
  if (newname.empty () == true)
    newname =
      Tilesetlist::getInstance()->findFreeName(n, 100, num,
                                               d_tileset->getTileSize ());
  if (name == "")
    {
      if (newname.empty() == true)
        {
          Glib::ustring msg =
            _("The Tile Set has an invalid name.\nChange it and save again.");
          TimedMessageDialog d(*window, msg, 0);
          d.run_and_hide ();
          on_edit_tileset_info_activated ();
          return false;
        }
      else
        {
          Glib::ustring msg =
            String::ucompose (_("The Tile Set has an invalid name.\nChange it to '%1'?"), newname);
          TimedMessageDialog d(*window, msg, 0);
          d.add_cancel_button ();
          d.run_and_hide ();
          if (d.get_response () == Gtk::RESPONSE_CANCEL)
            return false;
          d_tileset->setName (newname);
        }
    }

  //okay the question is whether or not the name is already used.
  bool same_name = false;
  Glib::ustring file =
    Tilesetlist::getInstance()->lookupConfigurationFileByName(d_tileset);
  if (file == "")
    return true;

  Glib::ustring cfgfile = d_tileset->getConfigurationFile(true);

  if (existing) // this means we're doing File->Save
    {
      if (file == cfgfile)
        return true;
      same_name = true;
    }
  else // this means we're doing File->Save As
    same_name = true;

  if (same_name)
    {
      if (newname.empty() == true)
        {
          Glib::ustring msg =
            _("The Tile Set has the same name as another one.\nChange it and save again.");
          TimedMessageDialog d(*window, msg, 0);
          d.run_and_hide ();
          on_edit_tileset_info_activated ();
          return false;
        }
      else
        {
          Glib::ustring msg =
            String::ucompose (_("The Tile Set has the same name as another one.\nChange it to '%1' instead?."), newname);

          TimedMessageDialog d(*window, msg, 0);
          d.add_cancel_button ();
          d.run_and_hide ();
          if (d.get_response () == Gtk::RESPONSE_CANCEL)
            return false;
          d_tileset->setName (newname);
        }
    }

  return true;
}

bool TileSetWindow::isValidName ()
{
  Glib::ustring file =
    Tilesetlist::getInstance()->lookupConfigurationFileByName(d_tileset);
  if (file == "")
    return true;
  if (file == d_tileset->getConfigurationFile (true))
    return true;
  return false;
}

void TileSetWindow::on_edit_undo_activated ()
{
  umgr->undo ();
  if (umgr->undoEmpty () && !new_tileset_needs_saving)
    tileset_modified = false;
  update ();
}
      
void TileSetWindow::on_edit_redo_activated ()
{
  tileset_modified = true;
  umgr->redo ();
  update ();
}

void TileSetWindow::update_menuitems ()
{
  umgr->updateMenuItems (edit_undo_menuitem, edit_redo_menuitem);

  if (get_selected_tile())
    preview_tile_menuitem->set_sensitive(true);
  else
    preview_tile_menuitem->set_sensitive(false);
}

void TileSetWindow::update ()
{
  disconnect_signals ();
  update_window_title ();
  update_tile_panel ();
  update_tileset_buttons();
  update_menuitems ();
  update_tilestyleset_buttons ();
  update_tilestyleset_panel ();
  update_tilestyle_panel ();
  connect_signals ();
}

Tile* TileSetWindow::getTileByIndex (TileSetEditorAction_TileIndex *i)
{
  Gtk::TreeModel::iterator iterrow = 
    tiles_treeview->get_model ()->get_iter (String::ucompose ("%1", i->getIndex ()));
  Gtk::TreeModel::Row row = *iterrow;
  Tile *t = row[tiles_columns.tile];
  return t;
}

UndoAction*
TileSetWindow::executeAction (UndoAction *action2)
{
  TileSetEditorAction *action = dynamic_cast<TileSetEditorAction*>(action2);
  UndoAction *out = NULL;

    switch (action->getType ())
      {
      case TileSetEditorAction::CHANGE_PROPERTIES:
          {
            TileSetEditorAction_Properties *a =
              dynamic_cast<TileSetEditorAction_Properties*>(action);
            out = new TileSetEditorAction_Properties
              (d_tileset->getName (),
               d_tileset->getInfo (),
               d_tileset->getCopyright (),
               d_tileset->getLicense (),
               d_tileset->getTileSize ());
            d_tileset->setName (a->getName ());
            d_tileset->setInfo (a->getDescription ());
            d_tileset->setCopyright (a->getCopyright ());
            d_tileset->setLicense (a->getLicense ());
            d_tileset->setTileSize (a->getTileSize ());
          }
        break;
      case TileSetEditorAction::NAME:
          {
            TileSetEditorAction_Name *a =
              dynamic_cast<TileSetEditorAction_Name*>(action);
            out = new TileSetEditorAction_Name
              (a->getIndex (), getTileByIndex (a)->getName (),
               tile_name_entry->get_position ());
            getTileByIndex (a)->setName (a->getName ());
            Gtk::TreeModel::iterator iterrow = 
              tiles_treeview->get_model ()->get_iter
              (String::ucompose ("%1", a->getIndex ()));
            if (iterrow)
              {
                Gtk::TreeModel::Row row = *iterrow;
                row[tiles_columns.name] = a->getName ();
              }
            tile_name_entry->set_text (a->getName ());
            tile_name_entry->set_position (a->getCursorPosition ());
          }
        break;
      case TileSetEditorAction::TYPE:
          {
            TileSetEditorAction_Type *a =
              dynamic_cast<TileSetEditorAction_Type*>(action);
            out = new TileSetEditorAction_Type
              (a->getIndex (), getTileByIndex (a)->getType ());
            getTileByIndex (a)->setType (a->getTileType ());
          }
        break;
      case TileSetEditorAction::PATTERN:
          {
            TileSetEditorAction_Pattern *a =
              dynamic_cast<TileSetEditorAction_Pattern*>(action);
            out = new TileSetEditorAction_Pattern
              (a->getIndex (), 
               getTileByIndex (a)->getSmallTile()->getPattern ());
            getTileByIndex (a)->getSmallTile ()->setPattern (a->getPattern ());
          }
        break;
      case TileSetEditorAction::MOVES:
          {
            TileSetEditorAction_Moves *a =
              dynamic_cast<TileSetEditorAction_Moves*>(action);
            out = new TileSetEditorAction_Moves
              (a->getIndex (), 
               getTileByIndex (a)->getMoves ());
            getTileByIndex (a)->setMoves (a->getMoves ());
          }
        break;
      case TileSetEditorAction::COLOUR:
          {
            TileSetEditorAction_Colour *a =
              dynamic_cast<TileSetEditorAction_Colour*>(action);
            
            Gdk::RGBA c;
            switch (a->getColourNumber ())
              {
              case 0:
                c = getTileByIndex (a)->getSmallTile ()->getColor ();
                break;
              case 1:
                c = getTileByIndex (a)->getSmallTile ()->getSecondColor ();
                break;
              case 2:
                c = getTileByIndex (a)->getSmallTile ()->getThirdColor ();
                break;
              }
            out = new TileSetEditorAction_Colour
              (a->getIndex (), a->getColourNumber (), c);
            c = a->getColour ();
            switch (a->getColourNumber ())
              {
              case 0:
                getTileByIndex (a)->getSmallTile ()->setColor (c);
                break;
              case 1:
                getTileByIndex (a)->getSmallTile ()->setSecondColor (c);
                break;
              case 2:
                getTileByIndex (a)->getSmallTile ()->setThirdColor (c);
                break;
              }
          }
        break;
      case TileSetEditorAction::ADD_TILESTYLESET:
          {
            TileSetEditorAction_AddTileStyleSet *a =
              dynamic_cast<TileSetEditorAction_AddTileStyleSet*>(action);
            out = new TileSetEditorAction_AddTileStyleSet (d_tileset);
            doReloadTileset (a);
          }
        break;
      case TileSetEditorAction::REMOVE_TILESTYLESET:
          {
            TileSetEditorAction_RemoveTileStyleSet *a =
              dynamic_cast<TileSetEditorAction_RemoveTileStyleSet*>(action);
            out = new TileSetEditorAction_RemoveTileStyleSet (d_tileset);
            doReloadTileset (a);
          }
        break;
      case TileSetEditorAction::TILESTYLE:
          {
            TileSetEditorAction_TileStyle *a =
              dynamic_cast<TileSetEditorAction_TileStyle*>(action);
            TileStyle *ts = d_tileset->getTileStyle (a->getTileStyleIndex ());
            out = new TileSetEditorAction_TileStyle
              (a->getIndex (),  ts->getId (), ts->getType ());
            ts->setType (a->getTileStyleType ());
          }
        break;
      case TileSetEditorAction::ADD_TILE:
          {
            TileSetEditorAction_AddTile *a =
              dynamic_cast<TileSetEditorAction_AddTile*>(action);
            out = new TileSetEditorAction_AddTile (d_tileset);
            doReloadTileset (a);
          }
        break;
      case TileSetEditorAction::REMOVE_TILE:
          {
            TileSetEditorAction_RemoveTile *a =
              dynamic_cast<TileSetEditorAction_RemoveTile*>(action);
            out = new TileSetEditorAction_RemoveTile (d_tileset);
            doReloadTileset (a);
          }
        break;
      case TileSetEditorAction::SELECTOR:
          {
            TileSetEditorAction_Selector *a =
              dynamic_cast<TileSetEditorAction_Selector *>(action);
            out = new TileSetEditorAction_Selector (d_tileset);
            doReloadTileset (a);
          }
        break;
      case TileSetEditorAction::EXPLOSION:
          {
            TileSetEditorAction_Explosion *a =
              dynamic_cast<TileSetEditorAction_Explosion *>(action);
            out = new TileSetEditorAction_Explosion (d_tileset);
            doReloadTileset (a);
          }
        break;
      case TileSetEditorAction::ROADS:
          {
            TileSetEditorAction_Roads *a =
              dynamic_cast<TileSetEditorAction_Roads *>(action);
            out = new TileSetEditorAction_Roads (d_tileset);
            doReloadTileset (a);
          }
        break;
      case TileSetEditorAction::STONES:
          {
            TileSetEditorAction_Stones *a =
              dynamic_cast<TileSetEditorAction_Stones *>(action);
            out = new TileSetEditorAction_Stones (d_tileset);
            doReloadTileset (a);
          }
        break;
      case TileSetEditorAction::BRIDGES:
          {
            TileSetEditorAction_Bridges *a =
              dynamic_cast<TileSetEditorAction_Bridges *>(action);
            out = new TileSetEditorAction_Bridges (d_tileset);
            doReloadTileset (a);
          }
        break;
      case TileSetEditorAction::FOG:
          {
            TileSetEditorAction_Fog *a =
              dynamic_cast<TileSetEditorAction_Fog *>(action);
            out = new TileSetEditorAction_Fog (d_tileset);
            doReloadTileset (a);
          }
        break;
      case TileSetEditorAction::FLAGS:
          {
            TileSetEditorAction_Flag *a =
              dynamic_cast<TileSetEditorAction_Flag *>(action);
            out = new TileSetEditorAction_Flag (d_tileset);
            doReloadTileset (a);
          }
        break;
      case TileSetEditorAction::TILESTYLES:
          {
            TileSetEditorAction_TileStyles *a =
              dynamic_cast<TileSetEditorAction_TileStyles *>(action);
            out = new TileSetEditorAction_TileStyles (d_tileset);
            doReloadTileset (a);
          }
        break;
      case TileSetEditorAction::BUILDING_COLOURS:
          {
            TileSetEditorAction_BuildingColours *a =
              dynamic_cast<TileSetEditorAction_BuildingColours *>(action);
            out = new TileSetEditorAction_BuildingColours (d_tileset);
            doReloadTileset (a);
          }
        break;
      case TileSetEditorAction::MOVE_BONUS:
          {
            TileSetEditorAction_MoveBonus *a =
              dynamic_cast<TileSetEditorAction_MoveBonus *>(action);
            out = new TileSetEditorAction_MoveBonus (d_tileset);
            doReloadTileset (a);
          }
        break;
      }
    return out;
  return NULL;
}

bool
TileSetWindow::doReloadTileset (TileSetEditorAction_Save *a)
{
  Glib::ustring olddir = d_tileset->getDirectory ();
  Glib::ustring oldname =
    File::get_basename (d_tileset->getConfigurationFile (true));
  Glib::ustring oldext = d_tileset->getExtension ();

  d_tileset->clean_tmp_dir ();
  delete d_tileset;
  d_tileset = new Tileset (*(a->getTileset ()));
  d_tileset->setLoadTemporaryFile ();

  disconnect_signals ();
  tilestyles_list->clear();
  tilestylesets_list->clear();
  int idx = getCurIndex ();
  tiles_list->clear();
  for (Tileset::iterator i = d_tileset->begin(); i != d_tileset->end(); ++i)
    {
      Gtk::TreeIter l = tiles_list->append();
      (*l)[tiles_columns.name] = (*i)->getName();
      (*l)[tiles_columns.tile] = *i;
    }
  if (idx >= 0)
    {
      if (d_tileset->size() >= (guint32)idx)
        tiles_treeview->set_cursor
          (Gtk::TreePath (String::ucompose ("%1", idx)));
      else
        tiles_treeview->set_cursor (Gtk::TreePath ("0"));

      Glib::RefPtr<Gtk::TreeSelection> selection =
        tiles_treeview->get_selection();
      Gtk::TreeModel::iterator i = selection->get_selected ();
      tiles_treeview->scroll_to_row
        (tiles_treeview->get_model ()->get_path (i));
    }
  connect_signals ();

  d_tileset->setDirectory (olddir);
  d_tileset->setBaseName (oldname);
  d_tileset->setExtension (oldext);
  update ();
  return false;
}

void TileSetWindow::on_moves_text_changed()
{
  tile_moves_spinbutton->set_value
    (atoi(tile_moves_spinbutton->get_text().c_str()));
  on_moves_changed();
}

int TileSetWindow::getCurIndex ()
{
  Glib::RefPtr<Gtk::TreeSelection> selection = tiles_treeview->get_selection ();
  Gtk::TreeModel::iterator i = selection->get_selected ();
  if (i)
    return
      atoi (tiles_treeview->get_model ()->get_path (i).to_string ().c_str ());
  else
    return -1;
}

void TileSetWindow::on_moves_changed()
{
  Glib::RefPtr<Gtk::TreeSelection> selection = tiles_treeview->get_selection ();
  Gtk::TreeModel::iterator iterrow = selection->get_selected ();

  if (iterrow)
    {
      Gtk::TreeModel::Row row = *iterrow;
      Tile *t = row[tiles_columns.tile];
      TileSetEditorAction_Moves *action =
        new TileSetEditorAction_Moves (getCurIndex (), t->getMoves ());
      addUndo (action);
      if (tile_moves_spinbutton->get_value () < MIN_MOVES_FOR_TILES)
        tile_moves_spinbutton->set_value (MIN_MOVES_FOR_TILES);
      else if (tile_moves_spinbutton->get_value () > MAX_MOVES_FOR_TILES)
        tile_moves_spinbutton->set_value (MAX_MOVES_FOR_TILES);
      else
        t->setMoves (int (tile_moves_spinbutton->get_value ()));
      dirty ();
    }
}

void TileSetWindow::disconnect_signals ()
{
  for (auto c : connections)
    c.disconnect ();
  connections.clear ();
}

bool TileSetWindow::connect_signals ()
{
  connections.push_back
    (tile_name_entry->signal_changed().connect (method(on_tile_name_changed)));
  connections.push_back
    (tile_name_entry->signal_changed().connect (method(on_tile_name_changed)));
  connections.push_back
    (tile_type_combobox->signal_changed().connect
     (method(on_tile_type_changed)));
  connections.push_back
    (tilestyle_combobox->signal_changed().connect
     (method(on_tilestyle_changed)));
  connections.push_back
    (tile_smallmap_pattern_combobox->signal_changed().connect
     (method(on_tile_pattern_changed)));
  connections.push_back
    (tile_moves_spinbutton->signal_insert_text().connect
     (sigc::hide(sigc::hide(method(on_moves_text_changed)))));
  connections.push_back
    (tile_smallmap_first_colorbutton->signal_color_set().connect
     (method(on_tile_first_color_changed)));
  connections.push_back
    (tile_smallmap_second_colorbutton->signal_color_set().connect
     (method(on_tile_second_color_changed)));
  connections.push_back
    (tile_smallmap_third_colorbutton->signal_color_set().connect
     (method(on_tile_third_color_changed)));
  connections.push_back
    (tilestylesets_treeview->get_selection()->signal_changed().connect
     (method(on_tilestyleset_selected)));
  connections.push_back
    (tilestyles_treeview->get_selection()->signal_changed().connect
     (method(on_tilestyle_selected)));
  connections.push_back
    (tiles_treeview->get_selection()->signal_changed().connect (method(on_tile_selected)));
  return true;
}

void TileSetWindow::clearUndoAndRedo ()
{
  umgr->clear ();
}

void TileSetWindow::addUndo (TileSetEditorAction *a)
{
  umgr->add (a);
}
/*
 some test cases
  1. create a new tileset from scratch, save invalid set, close, load it
  2. create a new tileset from scratch, save valid set, then switch sets
  3. save a copy of the default tileset, and switch sets
  4. modify the working tileset so we can see it change in scenario builder
  5. modify the working tileset so that it's invalid, try to save
  6. try adding an image file that isn't a .png
  7. try adding an image file that doesn't have the width as a multiple of the height
  8. try adding an image file that says it's a .png but is actually random data
  9. try to replace a tilestyleset image with another valid image
 10. try to clear a tilestyleset image by clicking clear in the filechooser
 11. try to remove a tilestyleset image by clicking remove
 12. try saving a new tileset that has a same name
 13. try saving an existing tileset that has a same name
 14. validate a tileset without: the mountains tile
 15. validate a tileset without: flags
 16. try saving a tileset that has an empty name
 17. validate a tileset with a same name
 18. validate a tileset with an empty name
 19. make a new invalid tileset and quit save it
 20. load a writable tileset, modify and quit save it
 21. load a writable tileset, make it invalid, and then quit save it
 22. try saving a tileset we don't have permission to save
 23. try quit-saving a tileset we don't have permission to save
 24. create a new tileset, add a small selector, save invalid set, close, load it
 25. load a writable tileset, remove small selector, quit-save it.  load it
 26. create a new tileset, add flags, save invalid set, close, load it
 27. load a writable tileset, remove flags, quit-save it.  load it
 28. create a new tileset, add explosion, save invalid set, close, load it
 29. load a writable tileset, remove explosion, quit-save it.  load it
 30. create a new tileset, add a roads picture, save invalid set, close, load it
 31. load a writable tileset, remove roads, quit-save it.  load it
 32. create a new tileset, add a bridges picture, save invalid set, close, load it
 33. load a writable tileset, remove bridges, quit-save it.  load it
 34. create a new tileset, add a stones picture, save invalid set, close, load it
 35. load a writable tileset, remove stones, quit-save it.  load it
 36. create a new tileset, add a fog picture, save invalid set, close, load it
 37. load a writable tileset, remove fog, quit-save it.  load it
*/
