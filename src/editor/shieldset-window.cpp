//  Copyright (C) 2007-2012, 2014, 2015, 2017 Ben Asselstine
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

#include <sigc++/functors/mem_fun.h>
#include <sigc++/functors/ptr_fun.h>
#include <algorithm>
#include <errno.h>

#include <gtkmm.h>
#include "shieldset-window.h"
#include "builder-cache.h"
#include "shieldset-info-dialog.h"
#include "gui/image-helpers.h"
#include "defs.h"
#include "Configuration.h"
#include "ImageCache.h"
#include "shieldsetlist.h"
#include "Tile.h"
#include "File.h"
#include "shield.h"
#include "ucompose.hpp"
#include "editor-quit-dialog.h"
#include "GameMap.h"
#include "past-chooser.h"

Glib::ustring small_none = N_("no small shield set");
Glib::ustring medium_none = N_("no medium shield set");
Glib::ustring large_none = N_("no large shield set");

Glib::ustring left_none = N_("no left tartan set");
Glib::ustring center_none = N_("no center tartan set");
Glib::ustring right_none = N_("no right tartan set");

#define method(x) sigc::mem_fun(*this, &ShieldSetWindow::x)

ShieldSetWindow::ShieldSetWindow(Glib::ustring load_filename)
{
  needs_saving = false;
  d_shieldset = NULL;
    Glib::RefPtr<Gtk::Builder> xml = 
      BuilderCache::editor_get("shieldset-window.ui");

    xml->get_widget("window", window);
    window->set_icon_from_file(File::getVariousFile("castle_icon.png"));
    window->signal_delete_event().connect (method(on_window_closed));

    xml->get_widget("shield_alignment", shield_alignment);
    xml->get_widget("shields_treeview", shields_treeview);
    // connect callbacks for the menu
    xml->get_widget("new_shieldset_menuitem", new_shieldset_menuitem);
    new_shieldset_menuitem->signal_activate().connect (method(on_new_shieldset_activated));
    xml->get_widget("load_shieldset_menuitem", load_shieldset_menuitem);
    load_shieldset_menuitem->signal_activate().connect (method(on_load_shieldset_activated));
    xml->get_widget("save_as_menuitem", save_as_menuitem);
    save_as_menuitem->signal_activate().connect (method(on_save_as_activated));
    xml->get_widget("save_shieldset_menuitem", save_shieldset_menuitem);
    save_shieldset_menuitem->signal_activate().connect (method(on_save_shieldset_activated));
    xml->get_widget("validate_shieldset_menuitem", validate_shieldset_menuitem);
    validate_shieldset_menuitem->signal_activate().connect (method(on_validate_shieldset_activated));
    xml->get_widget("quit_menuitem", quit_menuitem);
    quit_menuitem->signal_activate().connect (method(on_quit_activated));
    xml->get_widget("edit_shieldset_info_menuitem", edit_shieldset_info_menuitem);
    edit_shieldset_info_menuitem->signal_activate().connect
      (method(on_edit_shieldset_info_activated));
    xml->get_widget("edit_copy_shields_menuitem", edit_copy_shields_menuitem);
    edit_copy_shields_menuitem->signal_activate().connect
      (method(on_edit_copy_shields_activated));
    xml->get_widget ("help_about_menuitem", help_about_menuitem);
    help_about_menuitem->signal_activate().connect (method(on_help_about_activated));
    xml->get_widget ("change_smallpic_button", change_smallpic_button);
    change_smallpic_button->signal_clicked().connect
      (sigc::bind(method(on_shieldpic_changed), ShieldStyle::SMALL));
    xml->get_widget ("change_mediumpic_button", change_mediumpic_button);
    change_mediumpic_button->signal_clicked().connect
      (sigc::bind(method(on_shieldpic_changed), ShieldStyle::MEDIUM));
    xml->get_widget ("change_largepic_button", change_largepic_button);
    change_largepic_button->signal_clicked().connect
      (sigc::bind(method(on_shieldpic_changed), ShieldStyle::LARGE));
    xml->get_widget ("change_left_tartan_button", change_left_tartan_button);
    change_left_tartan_button->signal_clicked().connect
      (method(on_left_tartan_changed));
    xml->get_widget ("change_center_tartan_button", change_center_tartan_button);
    change_center_tartan_button->signal_clicked().connect
      (method(on_center_tartan_changed));
    xml->get_widget ("change_right_tartan_button", change_right_tartan_button);
    change_right_tartan_button->signal_clicked().connect
      (method(on_right_tartan_changed));
    xml->get_widget ("player_colorbutton", player_colorbutton);
    player_colorbutton->signal_color_set().connect(method(on_player_color_changed));

    xml->get_widget ("small_image", small_image);
    xml->get_widget ("medium_image", medium_image);
    xml->get_widget ("large_image", large_image);

    xml->get_widget ("left_tartan_image", left_tartan_image);
    xml->get_widget ("center_tartan_image", center_tartan_image);
    xml->get_widget ("right_tartan_image", right_tartan_image);

    window->signal_delete_event().connect(sigc::hide(method(on_delete_event)));

    shields_list = Gtk::ListStore::create(shields_columns);
    shields_treeview->set_model(shields_list);
    shields_treeview->append_column("", shields_columns.name);
    shields_treeview->set_headers_visible(false);
    shields_treeview->get_selection()->signal_changed().connect (method(on_shield_selected));

    update_shield_panel();

    if (load_filename != "")
      current_save_filename = load_filename;
    update_shieldset_menuitems();

    if (load_filename.empty() == false)
      {
	if (load_shieldset (load_filename))
          {
            update_shieldset_menuitems();
            update_shield_panel();
            update_window_title();
          }
      }
}

void
ShieldSetWindow::update_shieldset_menuitems()
{
  if (d_shieldset == NULL)
    {
      save_shieldset_menuitem->set_sensitive(false);
      save_as_menuitem->set_sensitive(false);
      validate_shieldset_menuitem->set_sensitive(false);
      edit_shieldset_info_menuitem->set_sensitive(false);
    }
  else
    {
      save_shieldset_menuitem->set_sensitive(true);
      save_as_menuitem->set_sensitive(true);
      edit_shieldset_info_menuitem->set_sensitive(true);
      validate_shieldset_menuitem->set_sensitive(true);
    }
}

void
ShieldSetWindow::update_shield_panel()
{
  //if nothing selected in the treeview, then we don't show anything in
  //the shield panel
  if (shields_treeview->get_selection()->get_selected() == 0)
    {
      //clear all values
      shield_alignment->set_sensitive(false);
      change_smallpic_button->set_label(small_none);
      change_mediumpic_button->set_label(medium_none);
      change_largepic_button->set_label(large_none);
      change_left_tartan_button->set_label(left_none);
      change_center_tartan_button->set_label(center_none);
      change_right_tartan_button->set_label(right_none);
      small_image->clear();
      medium_image->clear();
      large_image->clear();
      left_tartan_image->clear();
      center_tartan_image->clear();
      right_tartan_image->clear();
      player_colorbutton->set_rgba(Gdk::RGBA("black"));
      return;
    }
  shield_alignment->set_sensitive(true);
  Gtk::TreeModel::iterator iterrow = 
    shields_treeview->get_selection()->get_selected();
  if (iterrow) 
    fill_shield_info((*iterrow)[shields_columns.shield]);
}

bool ShieldSetWindow::on_delete_event()
{
  hide();
  return true;
}

void ShieldSetWindow::on_new_shieldset_activated()
{
  Glib::ustring name = "";
  int id = Shieldsetlist::getNextAvailableId(0);
  Shieldset *shieldset = new Shieldset(id, name);
  ShieldSetInfoDialog d(*window, shieldset, 
                        File::getSetDir(Shieldset::file_extension, false), "", 
                        false, _("Make a New Shieldset"));
  int response = d.run();
  if (response != Gtk::RESPONSE_ACCEPT)
    {
      delete shieldset;
      return;
    }
  shields_list->clear();
  if (d_shieldset)
    delete d_shieldset;
  d_shieldset = shieldset;
  Glib::ustring dir = File::getSetDir(Shieldset::file_extension, false);
  d_shieldset->setDirectory(dir);
  current_save_filename = d_shieldset->getConfigurationFile();

  //populate the list with initial entries.
  for (unsigned int i = Shield::WHITE; i <= Shield::NEUTRAL; i++)
    {
      if (i != Shield::NEUTRAL)
        addNewShield(Shield::Colour(i), Shield::get_default_color_for_no(i));
      else
        addNewShield(Shield::Colour(i), Shield::get_default_color_for_neutral());
    }
  //here we put a copy into the shieldsetlist, and keep d_shieldset as our
  //current working shieldset.
  Shieldset *copy = Shieldset::copy (d_shieldset);
  Glib::ustring new_basename = copy->getBaseName();
  guint32 new_id = copy->getId();
  if (!Shieldsetlist::getInstance()->addToPersonalCollection(copy, new_basename, new_id))
    delete copy;
  update_shieldset_menuitems();

  update_shield_panel();
  shields_treeview->set_cursor (Gtk::TreePath ("0"));
  needs_saving = true;
  update_window_title();
}

void ShieldSetWindow::on_load_shieldset_activated()
{
  Gtk::FileChooserDialog chooser(*window, 
				 _("Choose a Shieldset to Load"));
  Glib::RefPtr<Gtk::FileFilter> lws_filter = Gtk::FileFilter::create();
  lws_filter->set_name(_("LordsAWar Shieldsets (*.lws)"));
  lws_filter->add_pattern("*" + SHIELDSET_EXT);
  chooser.add_filter(lws_filter);
  chooser.set_current_folder(File::getSetDir(Shieldset::file_extension, false));

  chooser.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
  chooser.add_button(Gtk::Stock::OPEN, Gtk::RESPONSE_ACCEPT);
  chooser.set_default_response(Gtk::RESPONSE_ACCEPT);
      
  chooser.show_all();
  int res = chooser.run();

  if (res == Gtk::RESPONSE_ACCEPT)
    {
      bool ok = load_shieldset(chooser.get_filename());
      chooser.hide();
      if (ok)
        {
          needs_saving = false;
          update_window_title();
        }
    }

  update_shieldset_menuitems();
  update_shield_panel();
}

void ShieldSetWindow::on_validate_shieldset_activated()
{
  std::list<Glib::ustring> msgs;
  if (d_shieldset == NULL)
    return;
  bool valid = d_shieldset->validateNumberOfShields();
  if (!valid)
    msgs.push_back(_("The shieldset must have 9 shields in it."));

  for (unsigned int i = Shield::WHITE; i <= Shield::NEUTRAL; i++)
    {
      valid = d_shieldset->validateShieldImages(Shield::Colour(i));
      if (!valid)
        {
          Glib::ustring s = 
            String::ucompose(_("%1 must have all three images specified."),
                             Shield::colourToString(Shield::Colour(i)));
          msgs.push_back(s);
          break;
        }
    }
  if (msgs.empty() == true &&
      (!d_shieldset->getSmallWidth() || !d_shieldset->getSmallHeight()))
    msgs.push_back(_("The height or width of a small shield image is zero."));
  if (msgs.empty() == true &&
      (!d_shieldset->getMediumWidth() || !d_shieldset->getMediumHeight()))
    msgs.push_back(_("The height or width of a medium shield image is zero."));
  if (msgs.empty() == true &&
      (!d_shieldset->getLargeWidth() || !d_shieldset->getLargeHeight()))
    msgs.push_back(_("The height or width of a large shield image is zero."));

  Glib::ustring msg = "";
  for (std::list<Glib::ustring>::iterator it = msgs.begin(); it != msgs.end();
       it++)
    msg += (*it) + "\n";

  if (msg == "")
    msg = _("The shieldset is valid.");

  Gtk::MessageDialog dialog(*window, msg);
  dialog.run();
  dialog.hide();
  return;
}

void ShieldSetWindow::on_save_as_activated()
{
  Shieldset *copy = Shieldset::copy (d_shieldset);
  copy->setId(Shieldsetlist::getNextAvailableId(d_shieldset->getId()));
  ShieldSetInfoDialog d(*window, copy, 
                        File::getSetDir(Shieldset::file_extension, false), "", 
                        false, _("Save a Copy of a Shieldset"));
  int response = d.run();
  if (response == Gtk::RESPONSE_ACCEPT)
    {
      Glib::ustring new_basename=copy->getBaseName();
      guint32 new_id = copy->getId();
      copy->setDirectory(File::getSetDir(Shieldset::file_extension, false));
      guint32 oldid = d_shieldset->getId();
      Glib::ustring oldname = d_shieldset->getName();
      Glib::ustring oldbasename = d_shieldset->getBaseName();
      Glib::ustring olddir = d_shieldset->getDirectory();

      Glib::ustring tmpdir = File::get_tmp_file();
      File::erase(tmpdir);
      tmpdir += Shieldset::file_extension;
      File::create_dir(tmpdir);
      d_shieldset->setName(copy->getName());
      File::copy(d_shieldset->getConfigurationFile(), 
                 File::getTempFile (tmpdir, copy->getBaseName() + Shieldset::file_extension));
      d_shieldset->setBaseName(copy->getBaseName());
      d_shieldset->setDirectory(tmpdir);
      d_shieldset->setId(copy->getId());
          
      current_save_filename = copy->getConfigurationFile();
      bool ok = Shieldsetlist::getInstance()->addToPersonalCollection(d_shieldset, new_basename, new_id);
      File::erase(File::getTempFile (tmpdir, copy->getBaseName() + Shieldset::file_extension));
      File::erase_dir(tmpdir);
      if (ok)
        {
          save_shieldset_menuitem->set_sensitive(true);
          d_shieldset = copy;
          //our shields in the treeview are now out of date.
          refresh_shields(); //refresh them.
          needs_saving = false;
          update_window_title();
          shieldset_saved.emit(d_shieldset->getId());
        }
      else
        {
          d_shieldset->setName(oldname);
          d_shieldset->setBaseName(oldbasename);
          d_shieldset->setId(oldid);
          d_shieldset->setDirectory(olddir);
          Glib::ustring errmsg = Glib::strerror(errno);
          Glib::ustring msg;
          msg = _("Error!  Shieldset could not be saved.");
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

bool ShieldSetWindow::save_current_shieldset()
{
  if (GameMap::getInstance()->getShieldsetId() == d_shieldset->getId() &&
      d_shieldset->validate() == false)
    {
      Glib::ustring errmsg = _("Shieldset is invalid, and is also the current working shieldset.");
      Glib::ustring msg;
      msg = _("Error!  Shieldset could not be saved.");
      msg += "\n" + current_save_filename + "\n" +
        errmsg;
      Gtk::MessageDialog dialog(*window, msg);
      dialog.run();
      dialog.hide();
      return false;
    }
  if (current_save_filename.empty())
    current_save_filename = d_shieldset->getConfigurationFile();
  
  bool ok = d_shieldset->save(current_save_filename, Shieldset::file_extension);
  if (ok)
    {
      if (Shieldsetlist::getInstance()->reload(d_shieldset->getId()))
        refresh_shields();
      needs_saving = false;
      update_window_title();
      shieldset_saved.emit(d_shieldset->getId());
    }
  else
    {
      Glib::ustring errmsg = Glib::strerror(errno);
      Glib::ustring msg;
      msg = _("Error!  Shieldset could not be saved.");
      msg += "\n" + current_save_filename + "\n" +
        errmsg;
      Gtk::MessageDialog dialog(*window, msg);
      dialog.run();
      dialog.hide();
    }
  return ok;
}

void ShieldSetWindow::on_save_shieldset_activated()
{
  save_current_shieldset();
}

void ShieldSetWindow::on_edit_shieldset_info_activated()
{
  ShieldSetInfoDialog d(*window, d_shieldset, 
                        File::get_dirname(current_save_filename), 
                        File::get_basename(current_save_filename), true, 
                        _("Edit Shieldset Information"));
  int response = d.run();
  if (response == Gtk::RESPONSE_ACCEPT)
    {
      needs_saving = true;
      update_window_title();
    }
}

void ShieldSetWindow::on_help_about_activated()
{
  Gtk::AboutDialog* dialog;

  Glib::RefPtr<Gtk::Builder> xml
    = Gtk::Builder::create_from_file(File::getGladeFile("about-dialog.ui"));

  xml->get_widget("dialog", dialog);
  dialog->set_transient_for(*window);
  dialog->set_icon_from_file(File::getVariousFile("castle_icon.png"));

  dialog->set_version(PACKAGE_VERSION);
  dialog->set_logo(ImageCache::loadMiscImage("castle_icon.png")->to_pixbuf());
  dialog->show_all();
  dialog->run();
  delete dialog;
  return;
}

void ShieldSetWindow::on_shield_selected()
{
  update_shield_panel();
}

void ShieldSetWindow::show_tartan(Shield *s, Glib::ustring f, Gtk::Image *image)
{
  bool broken = false;
  if (!s)
    {
      image->clear();
      return;
    }
  std::vector<PixMask*> h = disassemble_row
    (d_shieldset->getFileFromConfigurationFile(f + ".png"), 2,
     broken);
  if (!broken)
    {
      PixMask *i = ImageCache::applyMask(h[0], h[1], s->getColor());
      if (i)
        {
          image->property_pixbuf() = i->to_pixbuf();
          delete i;
        }
      else
        image->clear();
      delete h[0];
      delete h[1];
    }
  else
    image->clear();
}

void ShieldSetWindow::show_shield(ShieldStyle *ss, Shield *s, Gtk::Image *image)
{
  bool broken = false;
  if (!ss || !s)
    {
      image->clear();
      return;
    }
  std::vector<PixMask*> h = disassemble_row
    (d_shieldset->getFileFromConfigurationFile(ss->getImageName() + ".png"), 2,
     broken);
  if (!broken)
    {
      PixMask *i = ImageCache::applyMask(h[0], h[1], s->getColor());
      if (i)
        {
          image->property_pixbuf() = i->to_pixbuf();
          delete i;
        }
      else
        image->clear();
      delete h[0]; 
      delete h[1];
    }
  else
    image->clear();
}

void ShieldSetWindow::fill_shield_info(Shield*shield)
{
  if (shield)
    {
      player_colorbutton->set_rgba(shield->getColor());
      Glib::ustring s;
      ShieldStyle* ss = shield->getFirstShieldstyle(ShieldStyle::SMALL);
      if (ss && ss->getImageName().empty() == false)
	s = ss->getImageName() + ".png";
      else
	s = small_none;
      show_shield(ss, shield, small_image);
      change_smallpic_button->set_label(s);

      ss = shield->getFirstShieldstyle(ShieldStyle::MEDIUM);
      if (ss && ss->getImageName().empty() == false)
	s = ss->getImageName() + ".png";
      else
	s = medium_none;
      change_mediumpic_button->set_label(s);
      show_shield(ss, shield, medium_image);

      ss = shield->getFirstShieldstyle(ShieldStyle::LARGE);
      if (ss && ss->getImageName().empty() == false)
	s = ss->getImageName() + ".png";
      else
	s = large_none;
      change_largepic_button->set_label(s);
      show_shield(ss, shield, large_image);

      if (shield->getName(Tartan::LEFT).empty() == false)
        s = shield->getName(Tartan::LEFT) + ".png";
      else
        s = left_none;
      show_tartan (shield, shield->getName(Tartan::LEFT), left_tartan_image);
      change_left_tartan_button->set_label(s);

      if (shield->getName(Tartan::CENTER).empty() == false)
        s = shield->getName(Tartan::CENTER) + ".png";
      else
        s = center_none;
      show_tartan (shield, shield->getName(Tartan::CENTER), center_tartan_image);
      change_center_tartan_button->set_label(s);

      if (shield->getName(Tartan::RIGHT).empty() == false)
        s = shield->getName(Tartan::RIGHT) + ".png";
      else
        s = right_none;
      show_tartan (shield, shield->getName(Tartan::RIGHT), right_tartan_image);
      change_right_tartan_button->set_label(s);
    }
}

bool ShieldSetWindow::load_shieldset(Glib::ustring filename)
{
  Glib::ustring old_current_save_filename = current_save_filename;
  current_save_filename = filename;

  Glib::ustring name = File::get_basename(filename);

  bool unsupported_version = false;
  Shieldset *shieldset = Shieldset::create(filename, unsupported_version);
  if (shieldset == NULL)
    {
      Glib::ustring msg;
      if (unsupported_version)
        msg = _("Error!  The version of the shieldset is not supported.");
      else
        msg = _("Error!  Shieldset could not be loaded.");
      Gtk::MessageDialog dialog(*window, msg);
      current_save_filename = old_current_save_filename;
      dialog.run();
      dialog.hide();
      return false;
    }
  shields_list->clear();
  if (d_shieldset)
    delete d_shieldset;
  d_shieldset = shieldset;

  bool broken = false;
  d_shieldset->instantiateImages(broken);
  for (Shieldset::iterator i = d_shieldset->begin(); i != d_shieldset->end(); 
       ++i)
    loadShield(*i);
  if (d_shieldset->size())
    shields_treeview->set_cursor (Gtk::TreePath ("0"));
  update_shield_panel();
  update_window_title();
  return true;
}

bool ShieldSetWindow::quit()
{
  if (needs_saving == true)
    {
      EditorQuitDialog d(*window);
      int response = d.run_and_hide();
      
      if (response == Gtk::RESPONSE_CANCEL) //we don't want to quit
	return false;

      else if (response == Gtk::RESPONSE_ACCEPT) // save and quit
        {
          if (save_current_shieldset() == false)
            return false;
        }
      window->hide();
    }
  else
    window->hide();
  if (d_shieldset)
    delete d_shieldset;
  return true;
}

bool ShieldSetWindow::on_window_closed(GdkEventAny*)
{
  return !quit();
}

void ShieldSetWindow::on_quit_activated()
{
  quit();
}
    
void ShieldSetWindow::on_shieldpic_changed(ShieldStyle::Type type)
{
  Gtk::TreeModel::iterator iterrow = 
    shields_treeview->get_selection()->get_selected();

  if (iterrow) 
    {
      Gtk::TreeModel::Row row = *iterrow;
      Shield *shield = row[shields_columns.shield];
      Glib::ustring filename = "";
      ShieldStyle *ss = shield->getFirstShieldstyle(type);
      if (ss->getImageName() != "")
	filename = d_shieldset->getFileFromConfigurationFile(ss->getImageName() +".png");
      Gtk::FileChooserDialog *d = shield_filechooser (shield, type);
      int response = d->run();
      if (filename != "")
        File::erase(filename);
      if (response == Gtk::RESPONSE_ACCEPT && d->get_filename() != "")
	{
	  if (d->get_filename() != filename)
            process_shieldstyle(ss, d);
	  update_shield_panel();
	}
      d->hide();
      delete d;
    }
}

void ShieldSetWindow::on_player_color_changed()
{
  Gtk::TreeModel::iterator iterrow = 
    shields_treeview->get_selection()->get_selected();

  if (iterrow) 
    {
      Gtk::TreeModel::Row row = *iterrow;
      Shield *s = row[shields_columns.shield];
      s->setColor(player_colorbutton->get_rgba());
      update_shield_panel();
      needs_saving = true;
      update_window_title();
    }
}

void ShieldSetWindow::addNewShield(Shield::Colour owner, Gdk::RGBA colour)
{
  Glib::ustring name = Shield::colourToFriendlyName(owner);
  Shield *shield = new Shield(owner, colour);
  if (shield)
    {
      shield->push_back(new ShieldStyle(ShieldStyle::SMALL));
      shield->push_back(new ShieldStyle(ShieldStyle::MEDIUM));
      shield->push_back(new ShieldStyle(ShieldStyle::LARGE));
      Gtk::TreeIter i = shields_list->append();
      (*i)[shields_columns.name] = name;
      (*i)[shields_columns.shield] = shield;
      d_shieldset->push_back(shield);
    }
}

void ShieldSetWindow::loadShield(Shield *shield)
{
  Glib::ustring name = 
    Shield::colourToFriendlyName(Shield::Colour(shield->getOwner()));
  Gtk::TreeIter i = shields_list->append();
  (*i)[shields_columns.name] = name;
  (*i)[shields_columns.shield] = shield;
}

void ShieldSetWindow::update_window_title()
{
  Glib::ustring title = "";
  if (needs_saving)
    title += "*";
  title += File::get_basename(current_save_filename, true);
  title += " - ";
  title += _("LordsAWar! Shieldset Editor");
  window->set_title(title);
}
    
void ShieldSetWindow::on_edit_copy_shields_activated()
{
  for (unsigned int j = ShieldStyle::SMALL; j <= ShieldStyle::LARGE; j++)
    d_shieldset->lookupShieldByTypeAndColour(Shield::WHITE, j)->uninstantiateImages();
  for (unsigned int i = Shield::WHITE+1; i <= Shield::NEUTRAL; i++)
    for (unsigned int j = ShieldStyle::SMALL; j <= ShieldStyle::LARGE; j++)
      {
        d_shieldset->lookupShieldByTypeAndColour(j, i)->setImageName(d_shieldset->lookupShieldByTypeAndColour(j, Shield::WHITE)->getImageName());
        d_shieldset->lookupShieldByTypeAndColour(j, i)->uninstantiateImages();
      }
  needs_saving = true;
  bool broken = false;
  d_shieldset->instantiateImages(broken);
  d_shieldset->setHeightsAndWidthsFromImages();
  d_shieldset->instantiateImages(broken);
  update_shield_panel();
  update_window_title();
}

void ShieldSetWindow::refresh_shields()
{
  Glib::RefPtr<Gtk::TreeSelection> selection = shields_treeview->get_selection();
  for (Shieldset::iterator i = d_shieldset->begin(); i != d_shieldset->end();
       i++)
    {
      shields_treeview->set_cursor(Gtk::TreePath 
                                   (String::ucompose("%1", (*i)->getOwner())));
      (*selection->get_selected())[shields_columns.shield] = *i;
    }
  shields_treeview->set_cursor (Gtk::TreePath ("0"));
}

Gtk::FileChooserDialog* ShieldSetWindow::shield_filechooser(Shield *s, ShieldStyle::Type type)
{
  Glib::ustring title = String::ucompose 
    (_("Choose a %1 %2 Shield image"), 
     ShieldStyle::shieldStyleTypeToFriendlyName(type),
     Shield::colourToFriendlyName(Shield::Colour(s->getOwner())));
  Gtk::FileChooserDialog *d = new Gtk::FileChooserDialog(*window, title);
  Glib::RefPtr<Gtk::FileFilter> png_filter = Gtk::FileFilter::create();
  png_filter->set_name(_("PNG files (*.png)"));
  png_filter->add_pattern("*.png");
  d->add_filter(png_filter);
  d->add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
  d->add_button(Gtk::Stock::OPEN, Gtk::RESPONSE_ACCEPT);
  d->set_default_response(Gtk::RESPONSE_ACCEPT);
  d->set_current_folder(PastChooser::getInstance()->get_dir(d));
  return d;
}

void ShieldSetWindow::process_shieldstyle(ShieldStyle *ss, Gtk::FileChooserDialog *d)
{
  PastChooser::getInstance()->set_dir(d);
  Glib::ustring file = File::get_basename(d->get_filename());
  if (d_shieldset->replaceFileInConfigurationFile(ss->getImageName()+".png", 
                                                  d->get_filename()))
    {
      bool broken = false;
      ss->setImageName(file);
      ss->instantiateImages(d->get_filename(), d_shieldset, broken);
      d_shieldset->setHeightsAndWidthsFromImages();
      needs_saving = true;
      update_window_title();
    }
  else
    {
      Gtk::MessageDialog 
        td(*d, String::ucompose(_("Couldn't add %1.png to:\n%2"),
                               file, d_shieldset->getConfigurationFile()));
      td.run();
      td.hide();
    }
}

void
ShieldSetWindow::on_left_tartan_changed()
{
}

void
ShieldSetWindow::on_center_tartan_changed()
{
}

void
ShieldSetWindow::on_right_tartan_changed()
{
}
