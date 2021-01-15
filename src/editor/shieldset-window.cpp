//  Copyright (C) 2007, 2008, 2009, 2010, 2011, 2012, 2014, 2015, 2017,
//  2020, 2021 Ben Asselstine
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
#include "editor-save-changes-dialog.h"
#include "GameMap.h"
#include "past-chooser.h"
#include "font-size.h"
#include "image-file-filter.h"
#include "timed-message-dialog.h"
#include "TarFileMaskedImage.h"

Glib::ustring no_shield_msg = N_("No image set");

Glib::ustring no_tartan_msg = N_("No image set");

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
    xml->get_widget("edit_undo_menuitem", edit_undo_menuitem);
    edit_undo_menuitem->signal_activate().connect
      (method(on_edit_undo_activated));
    xml->get_widget("edit_redo_menuitem", edit_redo_menuitem);
    edit_redo_menuitem->signal_activate().connect
      (method(on_edit_redo_activated));
    xml->get_widget("edit_shieldset_info_menuitem", edit_shieldset_info_menuitem);
    edit_shieldset_info_menuitem->signal_activate().connect
      (method(on_edit_shieldset_info_activated));
    xml->get_widget("edit_copy_shields_menuitem", edit_copy_shields_menuitem);
    edit_copy_shields_menuitem->signal_activate().connect
      (method(on_edit_copy_shields_activated));
    xml->get_widget ("help_about_menuitem", help_about_menuitem);
    help_about_menuitem->signal_activate().connect
      (method(on_help_about_activated));
    xml->get_widget ("tutorial_menuitem", tutorial_menuitem);
    tutorial_menuitem->signal_activate().connect
      (method(on_tutorial_video_activated));
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
      (sigc::bind(method(on_tartanpic_changed), Tartan::LEFT));
    xml->get_widget ("change_center_tartan_button", change_center_tartan_button);
    change_center_tartan_button->signal_clicked().connect
      (sigc::bind(method(on_tartanpic_changed), Tartan::CENTER));
    xml->get_widget ("change_right_tartan_button", change_right_tartan_button);
    change_right_tartan_button->signal_clicked().connect
      (sigc::bind(method(on_tartanpic_changed), Tartan::RIGHT));
    xml->get_widget ("player_colorbutton", player_colorbutton);
    player_colorbutton->signal_color_set().connect(method(on_player_color_changed));

    xml->get_widget ("small_image", small_image);
    xml->get_widget ("medium_image", medium_image);
    xml->get_widget ("large_image", large_image);

    xml->get_widget ("left_tartan_image", left_tartan_image);
    xml->get_widget ("center_tartan_image", center_tartan_image);
    xml->get_widget ("right_tartan_image", right_tartan_image);

    shields_list = Gtk::ListStore::create(shields_columns);
    shields_treeview->set_model(shields_list);
    shields_treeview->append_column("", shields_columns.name);
    shields_treeview->set_headers_visible(false);
    connect_shield_treeview();

    update_shield_panel();

    if (load_filename != "")
      current_save_filename = load_filename;

    if (load_filename.empty() == false)
      {
	if (load_shieldset (load_filename))
          update ();
      }
}

void ShieldSetWindow::clearUndoAndRedo ()
{
  for (auto a : redos)
    delete a;
  redos.clear ();
  for (auto a : undos)
    delete a;
  undos.clear ();
}

ShieldSetWindow::~ShieldSetWindow()
{
  clearUndoAndRedo ();
  delete window;
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
      change_smallpic_button->set_label(no_shield_msg);
      change_mediumpic_button->set_label(no_shield_msg);
      change_largepic_button->set_label(no_shield_msg);
      change_left_tartan_button->set_label(no_tartan_msg);
      change_center_tartan_button->set_label(no_tartan_msg);
      change_right_tartan_button->set_label(no_tartan_msg);
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

bool ShieldSetWindow::make_new_shieldset ()
{
  Glib::ustring msg = _("Save these changes before making a new Shield Set?");
  if (check_discard (msg) == false)
    return false;
  current_save_filename = "";
  shields_list->clear();
  if (d_shieldset)
    delete d_shieldset;

  guint32 num = 0;
  Glib::ustring name =
    Shieldsetlist::getInstance()->findFreeName(_("Untitled"), 100, num);

  d_shieldset = new Shieldset (Shieldsetlist::getNextAvailableId (1), name);
  d_shieldset->setNewTemporaryFile ();

  //populate the list with initial entries.
  for (unsigned int i = Shield::WHITE; i <= Shield::NEUTRAL; i++)
    {
      Gdk::RGBA colour = Shield::get_default_color_for_no(i);
      if (i == Shield::NEUTRAL)
        colour = Shield::get_default_color_for_neutral();
      Shield *shield = new Shield(Shield::Colour(i), colour);
      if (shield)
        {
          shield->push_back(new ShieldStyle(ShieldStyle::SMALL));
          shield->push_back(new ShieldStyle(ShieldStyle::MEDIUM));
          shield->push_back(new ShieldStyle(ShieldStyle::LARGE));
          add_shield_to_treeview (shield);
          d_shieldset->push_back(shield);
        }
    }

  update_shield_panel();
  shields_treeview->set_cursor (Gtk::TreePath ("0"));
  needs_saving = true;
  clearUndoAndRedo ();
  update ();
  return true;
}

void ShieldSetWindow::on_new_shieldset_activated()
{
  make_new_shieldset ();
}

bool ShieldSetWindow::check_discard (Glib::ustring msg)
{
  if (needs_saving)
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
              if (d_shieldset->getDirectory ().empty () == false)
                  saved = save_current_shieldset_file_as ();
              else
                {
                  if (save_current_shieldset_file ())
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

bool ShieldSetWindow::load_shieldset ()
{
  bool ret = false;
  Glib::ustring msg = _("Save these changes before opening a new Shield Set?");
  if (check_discard (msg) == false)
    return ret;
  Gtk::FileChooserDialog chooser(*window,
				 _("Choose a Shield Set to Open"));
  Glib::RefPtr<Gtk::FileFilter> lws_filter = Gtk::FileFilter::create();
  lws_filter->set_name(_("LordsAWar Shield Sets (*.lws)"));
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
          ret = true;
        }
    }

  update_shield_panel();
  return ret;
}

void ShieldSetWindow::on_load_shieldset_activated()
{
  load_shieldset ();
}

bool ShieldSetWindow::isValidName ()
{
  Glib::ustring file =
    Shieldsetlist::getInstance()->lookupConfigurationFileByName(d_shieldset);
  if (file == "")
    return true;
  if (file == d_shieldset->getConfigurationFile (true))
    return true;
  return false;
}

void ShieldSetWindow::on_validate_shieldset_activated()
{
  std::list<Glib::ustring> msgs;
  if (d_shieldset == NULL)
    return;
  bool valid = d_shieldset->validateNumberOfShields();
  if (!valid)
    msgs.push_back(_("The Shield Set must have 9 shields in it."));

  if (msgs.empty () == true)
    {
      valid = String::utrim (d_shieldset->getName ()) != "";
      if (!valid)
        {
          Glib::ustring s = _("The name of the Shield Set is invalid.");
          msgs.push_back(s);
        }
    }
  if (msgs.empty () == true)
    {
      for (unsigned int i = Shield::WHITE; i <= Shield::NEUTRAL; i++)
        {
          valid = d_shieldset->validateShieldImages(Shield::Colour(i));
          if (!valid)
            {
              Glib::ustring s =
                String::ucompose
                (_("%1 must have all three shield images specified."),
                 Shield::colourToString(Shield::Colour(i)));
              msgs.push_back(s);
              break;
            }
        }
    }
  if (msgs.empty () == true)
    {
      for (unsigned int i = Shield::WHITE; i <= Shield::NEUTRAL; i++)
        {
          valid = d_shieldset->validateTartanImages(Shield::Colour(i));
          if (!valid)
            {
              Glib::ustring s =
                String::ucompose
                (_("%1 must have all three tartan images specified."),
                 Shield::colourToString(Shield::Colour(i)));
              msgs.push_back(s);
              break;
            }
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
  if (msgs.empty() == true && isValidName () == false)
    msgs.push_back(_("The name of the Shield Set is not unique."));

  Glib::ustring msg = "";
  for (std::list<Glib::ustring>::iterator it = msgs.begin(); it != msgs.end();
       ++it)
    msg += (*it) + "\n";

  if (msg == "")
    msg = _("The Shield Set is valid.");

  TimedMessageDialog dialog(*window, msg, 0);
  dialog.run_and_hide ();
  return;
}

bool ShieldSetWindow::check_name_valid (bool existing)
{
  Glib::ustring name = d_shieldset->getName ();
  Glib::ustring newname = "";
  if (existing)
    {
      Shieldset *oldshieldset =
        Shieldsetlist::getInstance ()->get(d_shieldset->getId());
      if (oldshieldset && oldshieldset->getName () != name)
          newname = oldshieldset->getName ();
    }
  guint32 num = 0;
  Glib::ustring n = String::utrim (String::strip_trailing_numbers (name));
  if (n == "")
    n = _("Untitled");
  if (newname.empty () == true)
    newname =
      Shieldsetlist::getInstance()->findFreeName(n, 100, num,
                                                 d_shieldset->getTileSize ());
  if (name == "")
    {
      if (newname.empty() == true)
        {
          Glib::ustring msg =
            _("The Shield Set has an invalid name.\nChange it and save again.");
          TimedMessageDialog d(*window, msg, 0);
          d.run_and_hide();
          on_edit_shieldset_info_activated ();
          return false;
        }
      else
        {
          Glib::ustring msg =
            String::ucompose (_("The Shield Set has an invalid name.\nChange it to '%1'?"), newname);
          TimedMessageDialog d(*window, msg, 0);
          d.add_cancel_button ();
          d.run_and_hide ();
          if (d.get_response () == Gtk::RESPONSE_CANCEL)
            return false;
          d_shieldset->setName (newname);
        }
    }

  //okay the question is whether or not the name is already used.
  bool same_name = false;
  Glib::ustring file =
    Shieldsetlist::getInstance()->lookupConfigurationFileByName(d_shieldset);
  if (file == "")
    return true;

  Glib::ustring cfgfile = d_shieldset->getConfigurationFile(true);

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
            _("The Shield Set has the same name as another one.\nChange it and save again.");
          TimedMessageDialog d(*window, msg, 0);
          d.run_and_hide ();
          on_edit_shieldset_info_activated ();
          return false;
        }
      else
        {
          Glib::ustring msg =
            String::ucompose (_("The Shield Set has the same name as another one.\nChange it to '%1' instead?."), newname);

          TimedMessageDialog d(*window, msg, 0);
          d.add_cancel_button ();
          d.run_and_hide ();
          if (d.get_response () == Gtk::RESPONSE_CANCEL)
            return false;
          d_shieldset->setName (newname);
        }
    }

  return true;
}

bool ShieldSetWindow::check_save_valid (bool existing)
{
  if (check_name_valid (existing) == false)
    return false;

  if (d_shieldset->validate () == false)
    {
      if (existing &&
          GameMap::getInstance()->getShieldsetId() == d_shieldset->getId())
        {
          Glib::ustring errmsg =
            _("Shield Set is invalid, and is also the current working Shield Set.");
          Glib::ustring msg = _("Error!  Shield Set could not be saved.");
          msg += "\n" + current_save_filename + "\n" + errmsg;
          TimedMessageDialog dialog(*window, msg, 0);
          dialog.run_and_hide();
          return false;
        }
      else
        {
          TimedMessageDialog
            dialog(*window,
                   _("The Shield Set is invalid.  Do you want to proceed?"),
                   0);
          dialog.add_cancel_button ();
          dialog.run_and_hide ();
          if (dialog.get_response () == Gtk::RESPONSE_CANCEL)
            return false;
        }
    }
  return true;
}

void ShieldSetWindow::on_save_as_activated()
{
  if (check_save_valid (false))
    save_current_shieldset_file_as ();
}

bool ShieldSetWindow::save_current_shieldset_file_as ()
{
  bool ret = false;
  while (1)
    {
      Gtk::FileChooserDialog chooser(*window, _("Choose a Name"),
                                     Gtk::FILE_CHOOSER_ACTION_SAVE);
      Glib::RefPtr<Gtk::FileFilter> lws_filter = Gtk::FileFilter::create();
      lws_filter->set_name(_("LordsAWar Shield Sets (*.lws)"));
      lws_filter->add_pattern("*" + SHIELDSET_EXT);
      chooser.add_filter(lws_filter);
      chooser.set_current_folder(File::getSetDir(SHIELDSET_EXT, false));

      chooser.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
      chooser.add_button(Gtk::Stock::SAVE, Gtk::RESPONSE_ACCEPT);
      chooser.set_default_response(Gtk::RESPONSE_ACCEPT);
      chooser.set_do_overwrite_confirmation();
      chooser.set_current_name (File::sanify(d_shieldset->getName ()) +
                                SHIELDSET_EXT);

      chooser.show_all();
      int res = chooser.run();

      if (res == Gtk::RESPONSE_ACCEPT)
        {
          Glib::ustring filename = chooser.get_filename();
          Glib::ustring old_filename = current_save_filename;
          guint32 old_id = d_shieldset->getId ();
          d_shieldset->setId(Shieldsetlist::getNextAvailableId(old_id));

          ret = save_current_shieldset_file(filename);
          if (ret == false)
            {
              current_save_filename = old_filename;
              d_shieldset->setId(old_id);
            }
          else
            {
              needs_saving = false;
              d_shieldset->created (filename);
              Glib::ustring dir =
                File::add_slash_if_necessary (File::get_dirname (filename));
              if (dir == File::getSetDir(SHIELDSET_EXT, false) ||
                  dir == File::getSetDir(SHIELDSET_EXT, true))
                {
                  //if we saved it to a standard place, update the list
                  Shieldset *newshieldset = Shieldset::copy (d_shieldset);
                  Shieldsetlist::getInstance()->add (newshieldset, filename);
                  shieldset_saved.emit(d_shieldset->getId());
                }
              refresh_shields();
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

bool ShieldSetWindow::save_current_shieldset_file (Glib::ustring filename)
{
  current_save_filename = filename;
  if (current_save_filename.empty())
    current_save_filename = d_shieldset->getConfigurationFile(true);

  if (!d_shieldset->isSmallHeightAndWidthSet ())
    d_shieldset->setSmallHeightsAndWidthsFromImages();
  if (!d_shieldset->isMediumHeightAndWidthSet ())
    d_shieldset->setMediumHeightsAndWidthsFromImages();
  if (!d_shieldset->isLargeHeightAndWidthSet ())
    d_shieldset->setLargeHeightsAndWidthsFromImages();

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
      msg = _("Error!  Shield Set could not be saved.");
      msg += "\n" + current_save_filename + "\n" +
        errmsg;
      TimedMessageDialog dialog(*window, msg, 0);
      dialog.run_and_hide ();
    }
  return ok;
}

void ShieldSetWindow::on_save_shieldset_activated()
{
  if (current_save_filename.empty () == true)
    on_save_as_activated ();
  else
    {
      if (check_save_valid (true))
        save_current_shieldset_file();
    }
}

void ShieldSetWindow::on_edit_shieldset_info_activated()
{
  ShieldSetInfoDialog d(*window, d_shieldset);
  bool changed = d.run();
  if (changed)
    {
      ShieldSetEditorAction_Properties *action = 
        new ShieldSetEditorAction_Properties
        (d_shieldset->getName (), 
         d_shieldset->getInfo (),
         d_shieldset->getCopyright (),
         d_shieldset->getLicense (),
         d_shieldset->getSmallWidth (), d_shieldset->getSmallHeight (),
         d_shieldset->getMediumWidth (), d_shieldset->getMediumHeight (),
         d_shieldset->getLargeWidth (), d_shieldset->getLargeHeight ());
      undos.push_front (action);

      d_shieldset->setName (d.getName ());
      d_shieldset->setInfo (d.getDescription ());
      d_shieldset->setCopyright (d.getCopyright ());
      d_shieldset->setLicense (d.getLicense ());
      d_shieldset->setSmallWidth (d.getSmallWidth ());
      d_shieldset->setSmallHeight (d.getSmallHeight ());
      d_shieldset->setMediumWidth (d.getMediumWidth ());
      d_shieldset->setMediumHeight (d.getMediumHeight ());
      d_shieldset->setLargeWidth (d.getLargeWidth ());
      d_shieldset->setLargeHeight (d.getLargeHeight ());
      needs_saving = true;
      update ();
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

void ShieldSetWindow::show_tartan (Shield *s, Tartan::Type t, Gtk::Image *image)
{
  if (!s || s->getTartanMaskedImage (t)->getName ().empty ())
    {
      image->clear();
      return;
    }
  PixMask *i = s->getTartanMaskedImage (t)->applyMask (s->getColor ());
  double ratio = DIALOG_TARTAN_PIC_FONTSIZE_MULTIPLE;
  double new_height = FontSize::getInstance()->get_height () * ratio;
  int new_width =
    ImageCache::calculate_width_from_adjusted_height (i, new_height);
  PixMask::scale (i, new_width, new_height);
  if (i)
    {
      image->property_pixbuf() = i->to_pixbuf();
      delete i;
    }
  else
    image->clear();
}

void ShieldSetWindow::show_shield(ShieldStyle *ss, Shield *s, Gtk::Image *image)
{
  if (!ss || !s || ss->getMaskedImage()->getName().empty ())
    {
      image->clear();
      return;
    }
  PixMask *i = ss->getMaskedImage ()->applyMask (s->getColor ());
  double ratio = 1.0;
  switch (ss->getType ())
    {
    case ShieldStyle::SMALL:
      ratio = DIALOG_SMALL_SHIELD_PIC_FONTSIZE_MULTIPLE;
      break;
    case ShieldStyle::MEDIUM:
      ratio = DIALOG_MEDIUM_SHIELD_PIC_FONTSIZE_MULTIPLE;
      break;
    case ShieldStyle::LARGE:
      ratio = DIALOG_LARGE_SHIELD_PIC_FONTSIZE_MULTIPLE;
      break;
    }
  double new_height = FontSize::getInstance()->get_height () * ratio;
  int new_width =
    ImageCache::calculate_width_from_adjusted_height (i, new_height);
  PixMask::scale (i, new_width, new_height);
  if (i)
    {
      image->property_pixbuf() = i->to_pixbuf();
      delete i;
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
      if (ss && ss->getMaskedImage()->getName().empty() == false)
	s = ss->getMaskedImage()->getName();
      else
	s = no_shield_msg;
      show_shield(ss, shield, small_image);
      change_smallpic_button->set_label(s);

      ss = shield->getFirstShieldstyle(ShieldStyle::MEDIUM);
      if (ss && ss->getMaskedImage()->getName().empty() == false)
	s = ss->getMaskedImage()->getName();
      else
	s = no_shield_msg;
      change_mediumpic_button->set_label(s);
      show_shield(ss, shield, medium_image);

      ss = shield->getFirstShieldstyle(ShieldStyle::LARGE);
      if (ss && ss->getMaskedImage()->getName().empty() == false)
	s = ss->getMaskedImage()->getName();
      else
	s = no_shield_msg;
      change_largepic_button->set_label(s);
      show_shield(ss, shield, large_image);

      if (shield->getTartanMaskedImage(Tartan::LEFT)->getName ().empty() == false)
        s = shield->getTartanMaskedImage(Tartan::LEFT)->getName ();
      else
        s = no_tartan_msg;
      show_tartan (shield, Tartan::LEFT, left_tartan_image);
      change_left_tartan_button->set_label(s);

      if (shield->getTartanMaskedImage(Tartan::CENTER)->getName ().empty() == false)
        s = shield->getTartanMaskedImage(Tartan::CENTER)->getName ();
      else
        s = no_tartan_msg;
      show_tartan (shield, Tartan::CENTER, center_tartan_image);
      change_center_tartan_button->set_label(s);

      if (shield->getTartanMaskedImage(Tartan::RIGHT)->getName ().empty() == false)
        s = shield->getTartanMaskedImage(Tartan::RIGHT)->getName ();
      else
        s = no_tartan_msg;
      show_tartan (shield, Tartan::RIGHT, right_tartan_image);
      change_right_tartan_button->set_label(s);
    }
}

bool ShieldSetWindow::load_shieldset(Glib::ustring filename)
{
  Glib::ustring old_current_save_filename = current_save_filename;
  current_save_filename = filename;

  bool unsupported_version = false;
  bool success = replaceCurrentShieldset (filename, unsupported_version);

  if (!success)
    {
      Glib::ustring msg;
      if (unsupported_version)
        msg = _("Error!  The version of Shield Set is not supported.");
      else
        msg = _("Error!  Shield Set could not be loaded.");
      TimedMessageDialog dialog(*window, msg, 0);
      current_save_filename = old_current_save_filename;
      dialog.run_and_hide ();
      return false;
    }

  clearUndoAndRedo ();

  bool broken = false;
  d_shieldset->instantiateImages(false, broken);

  if (broken)
    {
      delete d_shieldset;
      d_shieldset = NULL;
      TimedMessageDialog td(*window, _("Couldn't load Shield Set images."), 0);
      td.run_and_hide();
      return false;
    }
      
  for (Shieldset::iterator i = d_shieldset->begin(); i != d_shieldset->end();
       ++i)
    add_shield_to_treeview (*i);
      
  if (d_shieldset->empty () == false)
    shields_treeview->set_cursor (Gtk::TreePath ("0"));
  update ();
  return true;
}

bool ShieldSetWindow::quit()
{
  if (needs_saving)
    {
      EditorQuitDialog d (*window);
      int response = d.run_and_hide();

      if (response == Gtk::RESPONSE_CANCEL) // we don't want to new
        return false;

      else if (response == Gtk::RESPONSE_ACCEPT) // save it
        {
          bool saved = false;
          bool existing = d_shieldset->getDirectory ().empty () == false;
          if (existing)
            {
              if (check_save_valid (true))
                {
                  if (save_current_shieldset_file ())
                    saved = true;
                }
              else
                return false;
            }
          else
            {
              if (check_save_valid (false))
                saved = save_current_shieldset_file_as ();
              else
                return false;
            }
          if (!saved)
            return false;
        }
    }
  window->hide ();
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
      ShieldStyle *ss = shield->getFirstShieldstyle(type);
      Glib::ustring f = ss->getMaskedImage()->getName ();
      Gtk::FileChooserDialog *d = shield_filechooser (shield, type, f != "");
      int response = d->run();
      if (response == Gtk::RESPONSE_ACCEPT && d->get_filename() != "")
	{
          if (ImageFileFilter::getInstance()->hasInvalidExt(d->get_filename()))
            ImageFileFilter::getInstance()->showErrorDialog (d);
          else
            {
              bool broken = false;
              PixMask *p = PixMask::create (d->get_filename (), broken);
              if (p)
                delete p;
              if (broken)
                {
                  TimedMessageDialog
                    td (*d,
                        String::ucompose
                        (_("Couldn't make sense of the image:\n%1"),
                         d->get_filename ()), 0);
                  td.run_and_hide();
                }
              else
                {
                  PastChooser::getInstance()->set_dir(d);
                  process_shieldstyle(ss, d);
                  d_shieldset->setHeightsAndWidthsFromImages(ss);
                }
              update_shield_panel();
            }
	}
      else if (response == Gtk::RESPONSE_REJECT && f != "")
        {
          ShieldSetEditorAction_ClearImage *action =
            new ShieldSetEditorAction_ClearImage (d_shieldset);
          if (d_shieldset->removeFileInCfgFile(f))
            {
              undos.push_front (action);
              d_shieldset->uninstantiateSameNamedImages
                (ss->getMaskedImage()->getName ());
              d_shieldset->setHeightsAndWidthsFromImages(ss);
              needs_saving = true;
            }
          else
            {
              delete action;
              Glib::ustring errmsg = Glib::strerror(errno);
              TimedMessageDialog
                td(*d, String::ucompose(_("Couldn't remove %1 from:\n%2\n%3"),
                                        f, d_shieldset->getConfigurationFile(),
                                        errmsg), 0);
              td.run_and_hide ();
            }
	  update ();
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
      ShieldSetEditorAction_Color *action = 
        new ShieldSetEditorAction_Color (s->getOwner (), s->getColor ());
      undos.push_front (action);
      s->setColor(player_colorbutton->get_rgba ());
      update_shield_panel();
      update_menuitems ();
      needs_saving = true;
      update_window_title();
    }
}

void ShieldSetWindow::add_shield_to_treeview (Shield *shield)
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
  title += d_shieldset->getName();
  title += " - ";
  title += _("Shield Set Editor");
  window->set_title(title);
}

void ShieldSetWindow::on_edit_copy_shields_activated()
{
  ShieldSetEditorAction_WhiteDown *action =
    new ShieldSetEditorAction_WhiteDown (d_shieldset);
  undos.push_front (action);

  Shield *w = d_shieldset->lookupShieldByColour (Shield::WHITE);
  for (guint32 i = Shield::WHITE + 1; i <= Shield::NEUTRAL; i++)
    {
      Shield *s = d_shieldset->lookupShieldByColour (i);
      for (auto ss : *s)
        {
          TarFileMaskedImage *mim = ss->getMaskedImage ();
          ShieldStyle *wss =
            d_shieldset->lookupShieldByTypeAndColour (ss->getType (),
                                                      Shield::WHITE);
          wss->getMaskedImage ()->copy (d_shieldset, mim);
        }
      for (guint32 k = Tartan::LEFT; k <= Tartan::RIGHT; k++)
        {
          TarFileMaskedImage *mim = s->getTartanMaskedImage (Tartan::Type (k));
          w->getTartanMaskedImage (Tartan::Type (k))->copy (d_shieldset, mim);
        }
    }
  needs_saving = true;
  bool broken = false;
  d_shieldset->instantiateImages (false, broken);
  update ();
}

void ShieldSetWindow::refresh_shields()
{
  Glib::RefPtr<Gtk::TreeSelection> selection = shields_treeview->get_selection();
  for (Shieldset::iterator i = d_shieldset->begin(); i != d_shieldset->end();
       ++i)
    {
      shields_treeview->set_cursor(Gtk::TreePath
                                   (String::ucompose("%1", (*i)->getOwner())));
      (*selection->get_selected())[shields_columns.shield] = *i;
    }
  shields_treeview->set_cursor (Gtk::TreePath ("0"));
}

Gtk::FileChooserDialog* ShieldSetWindow::image_filechooser (Glib::ustring title, bool clear)
{
  Gtk::FileChooserDialog *d = new Gtk::FileChooserDialog(*window, title);
  ImageFileFilter::getInstance ()->add (d);
  d->add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
  d->add_button(Gtk::Stock::OPEN, Gtk::RESPONSE_ACCEPT);
  if (clear)
    d->add_button(Gtk::Stock::CLEAR, Gtk::RESPONSE_REJECT);
  d->set_default_response(Gtk::RESPONSE_ACCEPT);
  d->set_current_folder(PastChooser::getInstance()->get_dir(d));
  return d;
}

Gtk::FileChooserDialog* ShieldSetWindow::tartan_filechooser(Shield *s, Tartan::Type type, bool clear)
{
  /* e.g. choose a white left tartan image */
  Glib::ustring title = String::ucompose
    (_("Choose a %1 %2 Tartan image"),
     Shield::colourToFriendlyName(Shield::Colour(s->getOwner())),
     Tartan::tartanTypeToFriendlyName(type));
  return image_filechooser (title, clear);
}

Gtk::FileChooserDialog* ShieldSetWindow::shield_filechooser(Shield *s, ShieldStyle::Type type, bool clear)
{
  /* e.g. choose a small white shield image */
  Glib::ustring title = String::ucompose
    (_("Choose a %1 %2 Shield image"),
     ShieldStyle::shieldStyleTypeToFriendlyName(type),
     Shield::colourToFriendlyName(Shield::Colour(s->getOwner())));
  return image_filechooser (title, clear);
}

void ShieldSetWindow::process_shieldstyle(ShieldStyle *ss, Gtk::FileChooserDialog *d)
{
  ShieldSetEditorAction_AddImage *action =
    new ShieldSetEditorAction_AddImage (d_shieldset);
  Glib::ustring newname = "";
  bool ret = false;
  if (ss->getMaskedImage()->getName() == "")
    ret = d_shieldset->addFileInCfgFile(d->get_filename (), newname);
  else
    ret = d_shieldset->replaceFileInCfgFile(ss->getMaskedImage()->getName(),
                                            d->get_filename(), newname);
  if (ret == true)
    {
      undos.push_front (action);
      ss->getMaskedImage ()->uninstantiateImages ();
      ss->getMaskedImage ()->load (d_shieldset, newname);
      ss->getMaskedImage ()->instantiateImages ();
      needs_saving = true;
      update ();
    }
  else
    {
      delete action;
      Glib::ustring errmsg = Glib::strerror(errno);
      TimedMessageDialog
        td(*d, String::ucompose(_("Couldn't add %1 to:\n%2\n%3"),
                               d->get_filename (),
                               d_shieldset->getConfigurationFile(), errmsg),
           0);
      td.run_and_hide();
    }
}

void ShieldSetWindow::on_tartanpic_changed (Tartan::Type type)
{
  Gtk::TreeModel::iterator iterrow =
    shields_treeview->get_selection()->get_selected();

  if (iterrow)
    {
      Gtk::TreeModel::Row row = *iterrow;
      Shield *shield = row[shields_columns.shield];

      Glib::ustring f = shield->getTartanMaskedImage(type)->getName ();
      Gtk::FileChooserDialog *d = tartan_filechooser (shield, type, f != "");
      int response = d->run();
      if (response == Gtk::RESPONSE_ACCEPT && d->get_filename() != "")
        {
          if (ImageFileFilter::getInstance()->hasInvalidExt(d->get_filename()))
            ImageFileFilter::getInstance ()->showErrorDialog (d);
          else
            {
              bool broken = false;
              PixMask *p = PixMask::create (d->get_filename (), broken);
              if (p)
                delete p;
              if (broken)
                {
                  TimedMessageDialog
                    td (*d,
                        String::ucompose
                        (_("Couldn't make sense of the image:\n%1"),
                         d->get_filename ()), 0);
                  td.run_and_hide ();
                }
              else
                {
                  PastChooser::getInstance()->set_dir(d);
                  process_tartanpic (type, shield, d);
                }
              update_shield_panel();
            }
        }
      else if (response == Gtk::RESPONSE_REJECT)
        {
          ShieldSetEditorAction_ClearImage *action =
            new ShieldSetEditorAction_ClearImage (d_shieldset);
          Glib::ustring file = shield->getTartanMaskedImage(type)->getName ();
          if (d_shieldset->removeFileInCfgFile(file))
            {
              undos.push_front (action);
              d_shieldset->uninstantiateSameNamedImages
                (shield->getTartanMaskedImage(type)->getName ());

              needs_saving = true;
            }
          else
            {
              delete action;
              Glib::ustring errmsg = Glib::strerror(errno);
              TimedMessageDialog
                td(*d, String::ucompose(_("Couldn't remove %1 from:\n%2\n%3"),
                                        file, d_shieldset->getConfigurationFile(),
                                        errmsg), 0);
              td.run_and_hide ();
            }
          update ();
        }
      d->hide();
      delete d;
    }
}

void ShieldSetWindow::process_tartanpic (Tartan::Type type, Shield *shield, Gtk::FileChooserDialog *d)
{
  ShieldSetEditorAction_AddImage *action =
    new ShieldSetEditorAction_AddImage (d_shieldset);

  Glib::ustring newname = "";
  Glib::ustring f = shield->getTartanMaskedImage(type)->getName ();
  bool ret = false;
  if (f == "")
    ret = d_shieldset->addFileInCfgFile(d->get_filename(), newname);
  else
    ret = d_shieldset->replaceFileInCfgFile(f, d->get_filename(), newname);
  if (ret == true)
    {
      undos.push_front (action);
      TarFileMaskedImage *mim = shield->getTartanMaskedImage (type);
      mim->uninstantiateImages ();
      mim->load (d_shieldset, newname);
      mim->instantiateImages ();
      needs_saving = true;
      update ();
    }
  else
    {
      delete action;
      Glib::ustring errmsg = Glib::strerror(errno);
      TimedMessageDialog
        td(*d, String::ucompose(_("Couldn't add %1 to:\n%2\n%3"),
                               d->get_filename (),
                               d_shieldset->getConfigurationFile(), errmsg),
           0);
      td.run_and_hide ();
    }
}

void ShieldSetWindow::on_tutorial_video_activated()
{
  GError *errs = NULL;
  gtk_show_uri(window->get_screen()->gobj(),
               "http://vimeo.com/406882053", 0, &errs);
  return;
}

void ShieldSetWindow::connect_shield_treeview()
{
  shield_selected_connection =
    shields_treeview->get_selection()->signal_changed().connect (method(on_shield_selected));
}

void ShieldSetWindow::disconnect_shield_treeview()
{
  if (shield_selected_connection.connected ())
    shield_selected_connection.disconnect ();
}

void ShieldSetWindow::on_edit_undo_activated ()
{
  ShieldSetEditorAction *a = undos.front ();
  undos.pop_front ();
  ShieldSetEditorAction *redo = executeAction (a);
  if (redo)
    redos.push_front (redo);
  delete a;
  if (undos.empty ())
    needs_saving = false;
  update ();
}
      
void ShieldSetWindow::on_edit_redo_activated ()
{
  needs_saving = true;
  ShieldSetEditorAction *a = redos.front ();
  redos.pop_front ();
  ShieldSetEditorAction *undo = executeAction (a);
  delete a;
  undos.push_front (undo);
  update ();
}

void ShieldSetWindow::update_menuitems ()
{
  edit_redo_menuitem->set_sensitive (redos.empty () == false);
  edit_undo_menuitem->set_sensitive (undos.empty () == false);
}

void ShieldSetWindow::update ()
{
  update_window_title ();
  update_shield_panel ();
  update_menuitems ();
}

void
ShieldSetWindow::executeColor (ShieldSetEditorAction_Color *action)
{
  Gtk::TreeModel::iterator iterrow =
    shields_treeview->get_selection()->get_selected();
  Shield *active_shield = (*iterrow)[shields_columns.shield];

  if (action->getPlayerId () == active_shield->getOwner ())
    player_colorbutton->set_rgba (action->getColor ());
  Shield *s = d_shieldset->lookupShieldByColour (action->getPlayerId ());
  s->setColor (action->getColor ());
  return;
}

void
ShieldSetWindow::executeProperties (ShieldSetEditorAction_Properties *action)
{
  d_shieldset->setName (action->getName ());
  d_shieldset->setInfo (action->getDescription ());
  d_shieldset->setCopyright (action->getCopyright ());
  d_shieldset->setLicense (action->getLicense ());
  d_shieldset->setSmallWidth (action->getSmallWidth ());
  d_shieldset->setSmallHeight (action->getSmallHeight ());
  d_shieldset->setMediumWidth (action->getMediumWidth ());
  d_shieldset->setMediumHeight (action->getMediumHeight ());
  d_shieldset->setLargeWidth (action->getLargeWidth ());
  d_shieldset->setLargeHeight (action->getLargeHeight ());
  return;
}

bool ShieldSetWindow::doReloadShieldset (ShieldSetEditorAction_Save *action)
{
  Glib::RefPtr<Gtk::TreeSelection> s = shields_treeview->get_selection ();
  std::vector<Gtk::TreeModel::Path> v = s->get_selected_rows ();

  Glib::ustring olddir = d_shieldset->getDirectory ();
  Glib::ustring oldname =
    File::get_basename (d_shieldset->getConfigurationFile (true));
  Glib::ustring oldext = d_shieldset->getExtension ();

  bool unsupported = false;
  replaceCurrentShieldset (action->getShieldsetFilename (), unsupported);

  bool broken = false;
  d_shieldset->instantiateImages(false, broken);

  for (Shieldset::iterator i = d_shieldset->begin(); i != d_shieldset->end();
       ++i)
    add_shield_to_treeview (*i);
      
  shields_treeview->set_cursor (v[0]);
  update ();

  d_shieldset->setDirectory (olddir);
  d_shieldset->setBaseName (oldname);
  d_shieldset->setExtension (oldext);
  return broken;
}

ShieldSetEditorAction*
ShieldSetWindow::executeAction (ShieldSetEditorAction *action)
{
  ShieldSetEditorAction *out = NULL;

    switch (action->getType ())
      {
      case ShieldSetEditorAction::CHANGE_COLOR:
          {
            ShieldSetEditorAction_Color *a =
              dynamic_cast<ShieldSetEditorAction_Color*>(action);
            ShieldSetEditorAction_Color *c =
              new ShieldSetEditorAction_Color (*a);
            c->setColor (player_colorbutton->get_rgba  ());
            out = c;
            executeColor (a);
            break;
          }
      case ShieldSetEditorAction::CHANGE_PROPERTIES:
          {
            ShieldSetEditorAction_Properties *a =
              dynamic_cast<ShieldSetEditorAction_Properties*>(action);
            out = new ShieldSetEditorAction_Properties
              (d_shieldset->getName (),
               d_shieldset->getInfo (),
               d_shieldset->getCopyright (),
               d_shieldset->getLicense (),
               d_shieldset->getSmallWidth (), d_shieldset->getSmallHeight (),
               d_shieldset->getMediumWidth (), d_shieldset->getMediumHeight (),
               d_shieldset->getLargeWidth (), d_shieldset->getLargeHeight ());
            executeProperties (a);
            break;
          }
      case ShieldSetEditorAction::COPY_WHITE_DOWN:
          {
            ShieldSetEditorAction_WhiteDown *a =
              dynamic_cast<ShieldSetEditorAction_WhiteDown*>(action);
            out = new ShieldSetEditorAction_WhiteDown (d_shieldset);
            doReloadShieldset (a);
            break;
          }
      case ShieldSetEditorAction::ADD_IMAGE:
          {
            ShieldSetEditorAction_AddImage *a =
              dynamic_cast<ShieldSetEditorAction_AddImage*>(action);
            out = new ShieldSetEditorAction_AddImage (d_shieldset);
            doReloadShieldset (a);
            break;
          }
      case ShieldSetEditorAction::CLEAR_IMAGE:
          {
            ShieldSetEditorAction_ClearImage *a =
              dynamic_cast<ShieldSetEditorAction_ClearImage*>(action);
            out = new ShieldSetEditorAction_ClearImage (d_shieldset);
            doReloadShieldset (a);
            break;
          }
      }
    return out;
}

bool ShieldSetWindow::replaceCurrentShieldset (Glib::ustring filename, bool &unsupported_version)
{
  Shieldset *shieldset = Shieldset::create(filename, unsupported_version);
  if (unsupported_version || shieldset == NULL)
    return false;
  disconnect_shield_treeview ();
  shields_list->clear();
  connect_shield_treeview ();
  if (d_shieldset)
    delete d_shieldset;
  d_shieldset = shieldset;
  d_shieldset->setLoadTemporaryFile ();
  return true;
}
/*
 some test cases
  1. create a new shieldset from scratch, save invalid set, close, load it
  2. create a new shieldset from scratch, save valid set, then switch sets
  3. save a copy of the default shieldset, and switch sets
  4. modify the working shieldset so we can see it change in scenario builder
  5. modify the working shieldset so that it's invalid, try to save
  6. try adding an image file that isn't a .png
  7. try adding an image file that says it's a .png but is actually a .jpg
  8. try adding an image file that says it's a .png but is actually random data
  9. try saving a new shieldset that has a same name
 10. try modifying an existing shieldset that has a same name
 11. validate a shieldset without: one of the shields
 12. validate a shieldset without: one of the tartans
 13. try saving a new shieldset that has an empty name
 14. validate a shieldset with a same name
 15. validate a shieldset with an empty name
 16. make a new invalid shieldset and quit save it
 17. load a writable shieldset, modify and quit save it
 18. load a writable shieldset, make it invalid, and then quit save it
 19. try saving a shieldset we don't have permission to save
 20. try quit-saving a shieldset we don't have permission to save
 */
