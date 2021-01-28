//  Copyright (C) 2009, 2010, 2011, 2012, 2014, 2015, 2020, 2021 Ben Asselstine
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
#include <iomanip>
#include <assert.h>
#include <string.h>
#include <errno.h>

#include <sigc++/functors/mem_fun.h>
#include <sigc++/functors/ptr_fun.h>

#include <gtkmm.h>
#include "cityset-window.h"
#include "builder-cache.h"
#include "cityset-info-dialog.h"

#include "defs.h"
#include "File.h"

#include "ucompose.hpp"

#include "image-editor-dialog.h"
#include "ImageCache.h"
#include "citysetlist.h"
#include "editor-quit-dialog.h"
#include "GameMap.h"
#include "editor-save-changes-dialog.h"
#include "timed-message-dialog.h"
#include "TarFileImage.h"

#define method(x) sigc::mem_fun(*this, &CitySetWindow::x)

CitySetWindow::CitySetWindow(Glib::ustring load_filename)
{
  cityset_modified = load_filename == "";
  new_cityset_needs_saving = load_filename == "";
  umgr = new UndoMgr (UndoMgr::DELAY, UndoMgr::LIMIT);
  umgr->execute ().connect (method (executeAction));
  d_cityset = NULL;
    Glib::RefPtr<Gtk::Builder> xml =
      BuilderCache::editor_get("cityset-window.ui");

    xml->get_widget("window", window);
    window->set_icon_from_file(File::getVariousFile("castle_icon.png"));
    window->signal_delete_event().connect (method(on_window_closed));

    xml->get_widget("cityset_alignment", cityset_alignment);
    xml->get_widget("new_cityset_menuitem", new_cityset_menuitem);
    new_cityset_menuitem->signal_activate().connect (method(on_new_cityset_activated));
    xml->get_widget("load_cityset_menuitem", load_cityset_menuitem);
    load_cityset_menuitem->signal_activate().connect (method(on_load_cityset_activated));
    xml->get_widget("save_cityset_menuitem", save_cityset_menuitem);
    save_cityset_menuitem->signal_activate().connect (method(on_save_cityset_activated));
    xml->get_widget("save_as_menuitem", save_as_menuitem);
    save_as_menuitem->signal_activate().connect (method(on_save_as_activated));
    xml->get_widget("validate_cityset_menuitem", validate_cityset_menuitem);
    validate_cityset_menuitem->signal_activate().connect
      (method(on_validate_cityset_activated));
    xml->get_widget("quit_menuitem", quit_menuitem);
    quit_menuitem->signal_activate().connect (method(on_quit_activated));
    xml->get_widget("edit_cityset_info_menuitem", edit_cityset_info_menuitem);
    edit_cityset_info_menuitem->signal_activate().connect
      (method(on_edit_cityset_info_activated));
    xml->get_widget("edit_undo_menuitem", edit_undo_menuitem);
    edit_undo_menuitem->signal_activate().connect
      (method(on_edit_undo_activated));
    xml->get_widget("edit_redo_menuitem", edit_redo_menuitem);
    edit_redo_menuitem->signal_activate().connect
      (method(on_edit_redo_activated));
    xml->get_widget ("help_about_menuitem", help_about_menuitem);
    help_about_menuitem->signal_activate().connect
       (method(on_help_about_activated));
    xml->get_widget ("tutorial_menuitem", tutorial_menuitem);
    tutorial_menuitem->signal_activate().connect
      (method(on_tutorial_video_activated));
    xml->get_widget("city_tile_width_spinbutton", city_tile_width_spinbutton);
    city_tile_width_spinbutton->set_range (1, 4);
    xml->get_widget("ruin_tile_width_spinbutton", ruin_tile_width_spinbutton);
    ruin_tile_width_spinbutton->set_range (1, 4);
    xml->get_widget("temple_tile_width_spinbutton",
		    temple_tile_width_spinbutton);
    temple_tile_width_spinbutton->set_range (1, 4);

    xml->get_widget("change_citypics_button", change_citypics_button);
    xml->get_widget("change_razedcitypics_button", change_razedcitypics_button);
    xml->get_widget("change_portpic_button", change_portpic_button);
    xml->get_widget("change_signpostpic_button", change_signpostpic_button);
    xml->get_widget("change_ruinpics_button", change_ruinpics_button);
    xml->get_widget("change_templepic_button", change_templepic_button);
    xml->get_widget("change_towerpics_button", change_towerpics_button);
    xml->get_widget ("notebook", notebook);

    if (load_filename != "")
      current_save_filename = load_filename;
    update_cityset_panel();

    if (load_filename.empty() == false)
      {
	load_cityset (load_filename);
        update ();
      }
}

void CitySetWindow::connect_signals ()
{
  connections.push_back
    (change_citypics_button->signal_clicked().connect
     (sigc::bind(method(on_change_clicked), _("Select a Cities image"),
                 d_cityset->getCity (),
                 sigc::mem_fun (d_cityset, &Cityset::getCityTileWidth))));
  connections.push_back
    (change_razedcitypics_button->signal_clicked().connect
     (sigc::bind(method(on_change_clicked), _("Select a Razed Cities image"),
                 d_cityset->getRazedCity (),
                 sigc::mem_fun (d_cityset, &Cityset::getCityTileWidth))));
  connections.push_back
    (change_portpic_button->signal_clicked().connect
     (sigc::bind (method(on_change_clicked), _("Select a Port image"),
                  d_cityset->getPort (),
                  method (getDefaultImageTileWidth))));
  connections.push_back
    (change_signpostpic_button->signal_clicked().connect
     (sigc::bind (method(on_change_clicked), _("Select a Signpost image"),
                  d_cityset->getSignpost (),
                  method (getDefaultImageTileWidth))));
  connections.push_back
    (change_ruinpics_button->signal_clicked().connect
     (sigc::bind (method(on_change_clicked), _("Select a Ruin image"),
                  d_cityset->getRuin (),
                  sigc::mem_fun (d_cityset, &Cityset::getRuinTileWidth))));
  connections.push_back
    (change_templepic_button->signal_clicked().connect
     (sigc::bind (method(on_change_clicked), _("Select a Temple image"),
                  d_cityset->getTemple (),
                  sigc::mem_fun (d_cityset, &Cityset::getTempleTileWidth))));
  connections.push_back
    (change_towerpics_button->signal_clicked().connect
     (sigc::bind (method(on_change_clicked), _("Select a Tower image"),
                  d_cityset->getTower (),
                  method (getDefaultImageTileWidth))));
  connections.push_back
    (city_tile_width_spinbutton->signal_insert_text().connect
     (sigc::hide(sigc::hide(method(on_city_tile_width_text_changed)))));
  connections.push_back
    (ruin_tile_width_spinbutton->signal_insert_text().connect
     (sigc::hide(sigc::hide(method(on_ruin_tile_width_text_changed)))));
  connections.push_back
    (temple_tile_width_spinbutton->signal_insert_text().connect
     (sigc::hide(sigc::hide(method(on_temple_tile_width_text_changed)))));
}

void CitySetWindow::disconnect_signals ()
{
  for (auto c : connections)
    c.disconnect ();
  connections.clear ();
}

void
CitySetWindow::update_cityset_panel()
{
  cityset_alignment->set_sensitive(d_cityset != NULL);
  Glib::ustring no_image = _("No image set");
  Glib::ustring s = no_image;
  if (d_cityset && d_cityset->getCity ()->getName ().empty() == false)
    s = d_cityset->getCity ()->getName ();
  change_citypics_button->set_label(s);

  s = no_image;
  if (d_cityset && d_cityset->getRazedCity ()->getName ().empty() == false)
    s = d_cityset->getRazedCity ()->getName ();
  change_razedcitypics_button->set_label(s);

  s = no_image;
  if (d_cityset && d_cityset->getPort()->getName().empty() == false)
    s = d_cityset->getPort()->getName();
  change_portpic_button->set_label(s);

  s = no_image;
  if (d_cityset && d_cityset->getSignpost ()->getName().empty() == false)
    s = d_cityset->getSignpost ()->getName();
  change_signpostpic_button->set_label(s);

  s = no_image;
  if (d_cityset && d_cityset->getRuin()->getName ().empty() == false)
    s = d_cityset->getRuin()->getName ();
  change_ruinpics_button->set_label(s);

  s = no_image;
  if (d_cityset && d_cityset->getTemple()->getName ().empty() == false)
    s = d_cityset->getTemple()->getName ();
  change_templepic_button->set_label(s);

  s = no_image;
  if (d_cityset && d_cityset->getTower()->getName().empty() == false)
    s = d_cityset->getTower()->getName();
  change_towerpics_button->set_label(s);

  if (d_cityset)
    city_tile_width_spinbutton->set_value(d_cityset->getCityTileWidth());
  else
    city_tile_width_spinbutton->set_value(2);

  if (d_cityset)
    ruin_tile_width_spinbutton->set_value(d_cityset->getRuinTileWidth());
  else
    ruin_tile_width_spinbutton->set_value(1);

  if (d_cityset)
    temple_tile_width_spinbutton->set_value(d_cityset->getTempleTileWidth());
  else
    temple_tile_width_spinbutton->set_value(1);
}

bool CitySetWindow::make_new_cityset ()
{
  Glib::ustring msg = _("Save these changes before making a new City Set?");
  if (check_discard (msg) == false)
    return false;
  current_save_filename = "";
  disconnect_signals ();
  if (d_cityset)
    delete d_cityset;

  guint32 num = 0;
  Glib::ustring name =
    Citysetlist::getInstance()->findFreeName(_("Untitled"), 100, num,
                                             Cityset::get_default_tile_size ());

  d_cityset = new Cityset (Citysetlist::getNextAvailableId (1), name);
  d_cityset->setNewTemporaryFile ();
  connect_signals ();

  update_cityset_panel();
  cityset_modified = false;
  new_cityset_needs_saving = true;
  clearUndoAndRedo ();
  update ();
  return true;
}

void CitySetWindow::on_new_cityset_activated()
{
  make_new_cityset ();
}

void CitySetWindow::on_load_cityset_activated()
{
  load_cityset ();
}

void CitySetWindow::on_validate_cityset_activated()
{
  std::list<Glib::ustring> msgs;
  if (d_cityset == NULL)
    return;
  if (msgs.empty () == true)
    {
      bool valid = String::utrim (d_cityset->getName ()) != "";
      if (!valid)
        {
          Glib::ustring s = _("The name of the City Set is invalid.");
          msgs.push_back(s);
        }
    }
  if (d_cityset->getCity ()->getName ().empty () == true)
    msgs.push_back(_("The cities picture is not set."));
  if (d_cityset->getRazedCity ()->getName ().empty () == true)
    msgs.push_back(_("The razed cities picture is not set."));
  if (d_cityset->getPort ()->getName ().empty () == true)
    msgs.push_back(_("The port picture is not set."));
  if (d_cityset->getSignpost ()->getName ().empty () == true)
    msgs.push_back(_("The signpost picture is not set."));
  if (d_cityset->getRuin ()->getName ().empty () == true)
    msgs.push_back(_("The ruins picture is not set."));
  if (d_cityset->getTemple ()->getName ().empty () == true)
    msgs.push_back(_("The temple picture is not set."));
  if (d_cityset->getTower ()->getName(). empty() == true)
    msgs.push_back(_("The towers picture is not set."));
  if (d_cityset->validateCityTileWidth() == false)
    msgs.push_back(_("The tile width for cities must be over zero."));
  if (d_cityset->validateRuinTileWidth() == false)
    msgs.push_back(_("The tile width for ruins must be over zero."));
  if (d_cityset->validateTempleTileWidth() == false)
    msgs.push_back(_("The tile width for temples must be over zero."));
  if (msgs.empty() == true && isValidName () == false)
    msgs.push_back(_("The name of the City Set is not unique."));

  Glib::ustring msg = "";
  for (std::list<Glib::ustring>::iterator it = msgs.begin(); it != msgs.end();
       ++it)
    {
      msg += (*it) + "\n";
      break;
    }

  if (msg == "")
    msg = _("The City Set is valid.");

  TimedMessageDialog dialog(*window, msg, 0);
  dialog.run_and_hide();

  return;
}

void CitySetWindow::on_save_as_activated()
{
  if (check_save_valid (false))
    save_current_cityset_file_as ();
}

bool CitySetWindow::save_current_cityset_file_as ()
{
  bool ret = false;
  while (1)
    {
      Gtk::FileChooserDialog chooser(*window, _("Choose a Name"),
                                     Gtk::FILE_CHOOSER_ACTION_SAVE);
      Glib::RefPtr<Gtk::FileFilter> lwc_filter = Gtk::FileFilter::create();
      lwc_filter->set_name(String::ucompose (_("LordsAWar City Sets (*%1)"),
                                             SHIELDSET_EXT));
      lwc_filter->add_pattern("*" + CITYSET_EXT);
      chooser.add_filter(lwc_filter);
      chooser.set_current_folder(File::getSetDir(CITYSET_EXT, false));

      chooser.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
      chooser.add_button(Gtk::Stock::SAVE, Gtk::RESPONSE_ACCEPT);
      chooser.set_default_response(Gtk::RESPONSE_ACCEPT);
      chooser.set_do_overwrite_confirmation();
      chooser.set_current_name (File::sanify(d_cityset->getName ()) +
                                CITYSET_EXT);

      chooser.show_all();
      int res = chooser.run();

      if (res == Gtk::RESPONSE_ACCEPT)
        {
          Glib::ustring filename = chooser.get_filename();
          Glib::ustring old_filename = current_save_filename;
          guint32 old_id = d_cityset->getId ();
          d_cityset->setId(Citysetlist::getNextAvailableId(old_id));

          ret = save_current_cityset_file(filename);
          if (ret == false)
            {
              current_save_filename = old_filename;
              d_cityset->setId(old_id);
            }
          else
            {
              cityset_modified = false;
              new_cityset_needs_saving = false;
              d_cityset->created (filename);
              Glib::ustring dir =
                File::add_slash_if_necessary (File::get_dirname (filename));
              if (dir == File::getSetDir(CITYSET_EXT, false) ||
                  dir == File::getSetDir(CITYSET_EXT, true))
                {
                  //if we saved it to a standard place, update the list
                  Citysetlist::getInstance()->add (Cityset::copy (d_cityset),
                                                   filename);
                  cityset_saved.emit(d_cityset->getId());
                }
              update_cityset_panel();
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

bool CitySetWindow::save_current_cityset_file (Glib::ustring filename)
{

  current_save_filename = filename;
  if (current_save_filename.empty())
    current_save_filename = d_cityset->getConfigurationFile(true);

  bool ok = d_cityset->save(current_save_filename, Cityset::file_extension);
  if (ok)
    {
      if (Citysetlist::getInstance()->reload(d_cityset->getId()))
        update_cityset_panel();
      new_cityset_needs_saving = false;
      cityset_modified = false;
      update_window_title();
      cityset_saved.emit(d_cityset->getId());
    }
  else
    {
      Glib::ustring errmsg = Glib::strerror(errno);
      Glib::ustring msg = _("Error!  City Set could not be saved.");
      msg += "\n" + current_save_filename + "\n" + errmsg;
      TimedMessageDialog dialog(*window, msg, 0);
      dialog.run_and_hide();
    }
  return ok;
}

void CitySetWindow::on_save_cityset_activated()
{
  if (current_save_filename.empty () == true)
    on_save_as_activated ();
  else
    {
      if (check_save_valid (true))
        save_current_cityset_file();
    }
}

void CitySetWindow::on_edit_cityset_info_activated()
{
  CitySetInfoDialog d(*window, d_cityset);
  bool changed = d.run();
  if (changed)
    {
      CitySetEditorAction_Properties *action = 
        new CitySetEditorAction_Properties
        (d_cityset->getName (), 
         d_cityset->getInfo (),
         d_cityset->getCopyright (),
         d_cityset->getLicense (),
         d_cityset->getTileSize ());
      addUndo (action);
      d_cityset->setName (d.getName ());
      d_cityset->setInfo (d.getDescription ());
      d_cityset->setCopyright (d.getCopyright ());
      d_cityset->setLicense (d.getLicense ());
      d_cityset->setTileSize (d.getTileSize ());
      cityset_modified = true;
      update ();
    }
}

void CitySetWindow::on_help_about_activated()
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

bool CitySetWindow::load_cityset ()
{
  bool ret = false;
  Glib::ustring msg = _("Save these changes before opening a new City Set?");
  if (check_discard (msg) == false)
    return ret;
  Gtk::FileChooserDialog chooser(*window,
				 _("Choose a City Set to Open"));
  Glib::RefPtr<Gtk::FileFilter> lwc_filter = Gtk::FileFilter::create();
  lwc_filter->set_name(String::ucompose (_("LordsAWar City Sets (*%1)"),
                                         CITYSET_EXT));
  lwc_filter->add_pattern("*" + CITYSET_EXT);
  chooser.add_filter(lwc_filter);
  Glib::RefPtr<Gtk::FileFilter> all_filter = Gtk::FileFilter::create();
  all_filter->set_name(_("All Files"));
  all_filter->add_pattern("*.*");
  chooser.add_filter(all_filter);
  chooser.set_filter (lwc_filter);
  chooser.set_current_folder(File::getSetDir(Cityset::file_extension, false));

  chooser.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
  chooser.add_button(Gtk::Stock::OPEN, Gtk::RESPONSE_ACCEPT);
  chooser.set_default_response(Gtk::RESPONSE_ACCEPT);

  chooser.show_all();
  int res = chooser.run();

  if (res == Gtk::RESPONSE_ACCEPT)
    {
      bool ok = load_cityset(chooser.get_filename());
      chooser.hide();
      if (ok)
        {
          cityset_modified = false;
          new_cityset_needs_saving = false;
          update_window_title();
          ret = true;
        }
    }

  update_cityset_panel();
  return ret;
}

bool CitySetWindow::load_cityset(Glib::ustring filename)
{
  Glib::ustring old_current_save_filename = current_save_filename;
  current_save_filename = filename;

  bool unsupported_version = false;
  Cityset *cityset = Cityset::create(filename, unsupported_version);
  if (cityset == NULL || unsupported_version)
    {
      Glib::ustring msg;
      if (unsupported_version)
        msg = _("Error!  The version of City Set is unsupported.");
      else
        msg = _("Error!  City Set could not be loaded.");
      TimedMessageDialog dialog(*window, msg, 0);
      current_save_filename = old_current_save_filename;
      dialog.run_and_hide();
      return false;
    }
  disconnect_signals ();
  if (d_cityset)
    delete d_cityset;
  d_cityset = cityset;
  connect_signals ();
  d_cityset->setLoadTemporaryFile ();

  bool broken = false;
  d_cityset->instantiateImages(false, broken);
  if (broken)
    {
      delete d_cityset;
      d_cityset = NULL;
      TimedMessageDialog td(*window, _("Couldn't load City Set images."), 0);
      td.run_and_hide();
      return false;
    }
  update ();
  return true;
}

bool CitySetWindow::quit()
{
  if (cityset_modified || new_cityset_needs_saving)
    {
      EditorQuitDialog d (*window);
      int response = d.run_and_hide();

      if (response == Gtk::RESPONSE_CANCEL) // we don't want to new
        return false;

      else if (response == Gtk::RESPONSE_ACCEPT) // save it
        {
          bool saved = false;
          bool existing = d_cityset->getDirectory().empty () == false;
          if (existing)
            {
              if (check_save_valid (true))
                {
                  if (save_current_cityset_file ())
                    saved = true;
                }
              else
                return false;
            }
          else
            {
              if (check_save_valid (false))
                saved = save_current_cityset_file_as ();
              else
                return false;
            }
          if (!saved)
            return false;
        }
    }
  window->hide ();
  if (d_cityset)
    delete d_cityset;
  return true;
}

bool CitySetWindow::on_window_closed(GdkEventAny*)
{
  return !quit();
}

void CitySetWindow::on_quit_activated()
{
  quit();
}

void CitySetWindow::on_city_tile_width_text_changed()
{
  city_tile_width_spinbutton->set_value(atoi(city_tile_width_spinbutton->get_text().c_str()));
  on_city_tile_width_changed();
}

void CitySetWindow::on_city_tile_width_changed()
{
  if (!d_cityset)
    return;
  CitySetEditorAction_CityWidth *action = 
    new CitySetEditorAction_CityWidth (d_cityset->getCityTileWidth ());
  addUndo (action);
  d_cityset->setCityTileWidth(city_tile_width_spinbutton->get_value());
  cityset_modified = true;
  update ();
}

void CitySetWindow::on_ruin_tile_width_text_changed()
{
  ruin_tile_width_spinbutton->set_value(atoi(ruin_tile_width_spinbutton->get_text().c_str()));
  on_ruin_tile_width_changed();
}

void CitySetWindow::on_ruin_tile_width_changed()
{
  if (!d_cityset)
    return;
  CitySetEditorAction_RuinWidth *action = 
    new CitySetEditorAction_RuinWidth (d_cityset->getRuinTileWidth ());
  addUndo (action);
  d_cityset->setRuinTileWidth(ruin_tile_width_spinbutton->get_value());
  cityset_modified = true;
  update_window_title();
}

void CitySetWindow::on_temple_tile_width_text_changed()
{
  temple_tile_width_spinbutton->set_value(atoi(temple_tile_width_spinbutton->get_text().c_str()));
  on_temple_tile_width_changed();
}

void CitySetWindow::on_temple_tile_width_changed()
{
  if (!d_cityset)
    return;
  CitySetEditorAction_TempleWidth *action = 
    new CitySetEditorAction_TempleWidth (d_cityset->getTempleTileWidth ());
  addUndo (action);
  d_cityset->setTempleTileWidth(temple_tile_width_spinbutton->get_value());
  cityset_modified = true;
  update_window_title();
}

void CitySetWindow::on_change_clicked(Glib::ustring msg, TarFileImage *im, sigc::slot<guint32> getTileWidth)
{
  bool cleared = false;
  Glib::ustring f = change_image(msg, im, cleared, getTileWidth ());
  if (cleared)
    {
      d_cityset->uninstantiateSameNamedImages (im->getName ());
      im->clear ();
    }
  else
    {
      if (f != "")
        {
          im->load (d_cityset, f);
          im->instantiateImages ();
        }
    }
  update_cityset_panel();
}

Glib::ustring CitySetWindow::change_image(Glib::ustring msg, TarFileImage *im,
                                          bool &cleared, guint32 tw)
{
  Glib::ustring newfile = "";
  Glib::ustring imgname = im->getName ();

  ImageEditorDialog d(*window, im,
                      EDITOR_DIALOG_TILE_PIC_FONTSIZE_MULTIPLE * (double)tw);
  d.set_title(msg);
  int response = d.run();
  if (response == Gtk::RESPONSE_ACCEPT && d.get_filename() != "")
    {
      CitySetEditorAction_AddImage *action =
        new CitySetEditorAction_AddImage (d_cityset);
      Glib::ustring newname = "";
      bool success = d.installFile (d_cityset, im, d.get_filename ());
      if (success)
        {
          addUndo (action);
          newfile = im->getName ();
          cityset_modified = true;
          update ();
        }
      else
        {
          delete action;
          show_add_file_error(*d.get_dialog(), d.get_filename ());
        }
    }
  else if (response == Gtk::RESPONSE_REJECT)
    {
      CitySetEditorAction_ClearImage *action =
        new CitySetEditorAction_ClearImage (d_cityset);
      if (d.uninstallFile (d_cityset, im))
        {
          addUndo (action);
          cityset_modified = true;
          update ();
          cleared = true;
          newfile = "";
        }
      else
        {
          delete action;
          show_remove_file_error(*d.get_dialog(), imgname);
        }
    }
  return newfile;
}

void CitySetWindow::update_window_title()
{
  Glib::ustring title = "";
  if (cityset_modified || new_cityset_needs_saving)
    title += "*";
  title += d_cityset->getName();
  title += " - ";
  title += _("City Set Editor");
  window->set_title(title);
}

void CitySetWindow::show_add_file_error(Gtk::Dialog &d, Glib::ustring file)
{
  Glib::ustring errmsg = Glib::strerror(errno);
  Glib::ustring m =
    String::ucompose(_("Couldn't add %1 to:\n%2\n%3"),
                     file, d_cityset->getConfigurationFile(), errmsg);
  TimedMessageDialog td(d, m, 0);
  td.run_and_hide();
}

void CitySetWindow::show_remove_file_error(Gtk::Dialog &d, Glib::ustring file)
{
  Glib::ustring errmsg = Glib::strerror(errno);
  Glib::ustring m =
    String::ucompose(_("Couldn't remove %1 from:\n%2\n%3"),
                     file, d_cityset->getConfigurationFile(), errmsg);
  TimedMessageDialog td(d, m, 0);
  td.run_and_hide();
}

CitySetWindow::~CitySetWindow()
{
  notebook->property_show_tabs () = false;
  delete umgr;
  delete window;
}

void CitySetWindow::on_tutorial_video_activated()
{
  GError *errs = NULL;
  gtk_show_uri(window->get_screen()->gobj(),
               "http://vimeo.com/406899445", 0, &errs);
}

bool CitySetWindow::check_discard (Glib::ustring msg)
{
  if (cityset_modified || new_cityset_needs_saving)
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
              if (d_cityset->getDirectory ().empty () == false)
                  saved = save_current_cityset_file_as ();
              else
                {
                  if (save_current_cityset_file ())
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

bool CitySetWindow::check_save_valid (bool existing)
{
  if (check_name_valid (existing) == false)
    return false;

  if (d_cityset->validate () == false)
    {
      if (existing &&
          GameMap::getInstance()->getCitysetId() == d_cityset->getId())
        {
          Glib::ustring errmsg =
            _("City Set is invalid, and is also the current working City Set.");
          Glib::ustring msg = _("Error!  City Set could not be saved.");
          msg += "\n" + current_save_filename + "\n" + errmsg;
          TimedMessageDialog dialog(*window, msg, 0);
          dialog.run_and_hide();
          return false;
        }
      else
        {
          TimedMessageDialog
            dialog(*window,
                   _("The City Set is invalid.  Do you want to proceed?"), 0);
          dialog.add_cancel_button ();
          dialog.run_and_hide ();
          if (dialog.get_response () == Gtk::RESPONSE_CANCEL)
            return false;
        }
    }
  return true;
}

bool CitySetWindow::check_name_valid (bool existing)
{
  Glib::ustring name = d_cityset->getName ();
  Glib::ustring newname = "";
  if (existing)
    {
      Cityset *oldcityset =
        Citysetlist::getInstance ()->get(d_cityset->getId());
      if (oldcityset && oldcityset->getName () != name)
          newname = oldcityset->getName ();
    }
  guint32 num = 0;
  Glib::ustring n = String::utrim (String::strip_trailing_numbers (name));
  if (n == "")
    n = _("Untitled");
  if (newname.empty () == true)
    newname =
      Citysetlist::getInstance()->findFreeName(n, 100, num,
                                               d_cityset->getTileSize ());
  if (name == "")
    {
      if (newname.empty() == true)
        {
          Glib::ustring msg =
            _("The City Set has an invalid name.\nChange it and save again.");
          TimedMessageDialog d(*window, msg, 0);
          d.run_and_hide();
          on_edit_cityset_info_activated ();
          return false;
        }
      else
        {
          Glib::ustring msg =
            String::ucompose (_("The City Set has an invalid name.\nChange it to '%1'?"), newname);
          TimedMessageDialog d(*window, msg, 0);
          d.add_cancel_button ();
          d.run_and_hide ();
          if (d.get_response () == Gtk::RESPONSE_CANCEL)
            return false;
          d_cityset->setName (newname);
        }
    }

  //okay the question is whether or not the name is already used.
  bool same_name = false;
  Glib::ustring file =
    Citysetlist::getInstance()->lookupConfigurationFileByName(d_cityset);
  if (file == "")
    return true;

  Glib::ustring cfgfile = d_cityset->getConfigurationFile(true);

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
            _("The City Set has the same name as another one.\nChange it and save again.");
          TimedMessageDialog d(*window, msg, 0);
          d.run_and_hide();
          on_edit_cityset_info_activated ();
          return false;
        }
      else
        {
          Glib::ustring msg =
            String::ucompose (_("The City Set has the same name as another one.\nChange it to '%1' instead?."), newname);

          TimedMessageDialog d(*window, msg, 0);
          d.add_cancel_button ();
          d.run_and_hide ();
          if (d.get_response () == Gtk::RESPONSE_CANCEL)
            return false;
          d_cityset->setName (newname);
        }
    }

  return true;
}

bool CitySetWindow::isValidName ()
{
  Glib::ustring file =
    Citysetlist::getInstance()->lookupConfigurationFileByName(d_cityset);
  if (file == "")
    return true;
  if (file == d_cityset->getConfigurationFile (true))
    return true;
  return false;
}

guint32 CitySetWindow::getDefaultImageTileWidth ()
{
  return 1;
}

void CitySetWindow::on_edit_undo_activated ()
{
  umgr->undo ();
  if (umgr->undoEmpty () && !new_cityset_needs_saving)
    cityset_modified = false;
  update ();
}
      
void CitySetWindow::on_edit_redo_activated ()
{
  cityset_modified = true;
  umgr->redo ();
  update ();
}

void CitySetWindow::update_menuitems ()
{
  umgr->updateMenuItems (edit_undo_menuitem, edit_redo_menuitem);
}

UndoAction*
CitySetWindow::executeAction (UndoAction *action2)
{
  CitySetEditorAction *action = dynamic_cast<CitySetEditorAction*>(action2);
  UndoAction *out = NULL;

    switch (action->getType ())
      {
      case CitySetEditorAction::CHANGE_PROPERTIES:
          {
            CitySetEditorAction_Properties *a =
              dynamic_cast<CitySetEditorAction_Properties*>(action);
            out = new CitySetEditorAction_Properties
              (d_cityset->getName (),
               d_cityset->getInfo (),
               d_cityset->getCopyright (),
               d_cityset->getLicense (),
               d_cityset->getTileSize ());
            d_cityset->setName (a->getName ());
            d_cityset->setInfo (a->getDescription ());
            d_cityset->setCopyright (a->getCopyright ());
            d_cityset->setLicense (a->getLicense ());
            d_cityset->setTileSize (a->getTileSize ());
            break;
          }
      case CitySetEditorAction::ADD_IMAGE:
          {
            CitySetEditorAction_AddImage *a =
              dynamic_cast<CitySetEditorAction_AddImage*>(action);
            out = new CitySetEditorAction_AddImage (d_cityset);
            doReloadCityset (a);
            break;
          }
      case CitySetEditorAction::CLEAR_IMAGE:
          {
            CitySetEditorAction_ClearImage *a =
              dynamic_cast<CitySetEditorAction_ClearImage*>(action);
            out = new CitySetEditorAction_ClearImage (d_cityset);
            doReloadCityset (a);
            break;
          }
      case CitySetEditorAction::CITY_TILE_WIDTH:
          {
            CitySetEditorAction_CityWidth *a =
              dynamic_cast<CitySetEditorAction_CityWidth*>(action);
            out = new CitySetEditorAction_CityWidth 
              (d_cityset->getCityTileWidth ());
            disconnect_signals ();
            city_tile_width_spinbutton->set_text
              (String::ucompose ("%1", a->getCityWidth ()));
            connect_signals ();
            d_cityset->setCityTileWidth (a->getCityWidth ());
            break;
          }
      case CitySetEditorAction::RUIN_TILE_WIDTH:
          {
            CitySetEditorAction_RuinWidth *a =
              dynamic_cast<CitySetEditorAction_RuinWidth*>(action);
            out = new CitySetEditorAction_RuinWidth 
              (d_cityset->getRuinTileWidth ());
            disconnect_signals ();
            ruin_tile_width_spinbutton->set_text
              (String::ucompose ("%1", a->getRuinWidth ()));
            connect_signals ();
            d_cityset->setRuinTileWidth (a->getRuinWidth ());
            break;
          }
      case CitySetEditorAction::TEMPLE_TILE_WIDTH:
          {
            CitySetEditorAction_TempleWidth *a =
              dynamic_cast<CitySetEditorAction_TempleWidth*>(action);
            out = new CitySetEditorAction_TempleWidth 
              (d_cityset->getTempleTileWidth ());
            disconnect_signals ();
            temple_tile_width_spinbutton->set_text
              (String::ucompose ("%1", a->getTempleWidth ()));
            connect_signals ();
            d_cityset->setTempleTileWidth (a->getTempleWidth ());
            break;
          }
      }
    return out;
}

void
CitySetWindow::doReloadCityset (CitySetEditorAction_Save *action)
{
  Glib::ustring olddir = d_cityset->getDirectory ();
  Glib::ustring oldname =
    File::get_basename (d_cityset->getConfigurationFile (true));
  Glib::ustring oldext = d_cityset->getExtension ();

  d_cityset->clean_tmp_dir ();
  delete d_cityset;
  d_cityset = new Cityset (*(action->getCityset ()));
  d_cityset->setLoadTemporaryFile ();

  d_cityset->setDirectory (olddir);
  d_cityset->setBaseName (oldname);
  d_cityset->setExtension (oldext);
  update ();
}

void CitySetWindow::update ()
{
  update_window_title ();
  update_cityset_panel ();
  update_menuitems ();
}

void CitySetWindow::clearUndoAndRedo ()
{
  umgr->clear ();
}

void CitySetWindow::addUndo (CitySetEditorAction *a)
{
  umgr->add (a);
}
/*
 some test cases
  1. create a new cityset from scratch, save invalid set, close, load it
  2. create a new cityset from scratch, save valid set, then switch sets
  3. save a copy of the default cityset, and switch sets
  4. modify the working cityset so we can see it change in scenario builder
  5. modify the working cityset so that it's invalid, try to save
  6. try adding an image file that isn't a .png
  7. try adding an image file that says it's a .png but is actually a .jpg
  8. try adding an image file that says it's a .png but is actually random data
  9. try saving a new cityset that has a same name
 10. try saving an existing cityset that has a same name
 11. validate a cityset without: the port picture
 12. validate a cityset without: the towers picture
 13. try saving a new cityset that has an empty name
 14. validate a cityset with a same name
 15. validate a cityset with an empty name
 16. make a new invalid cityset and quit save it
 17. load a writable cityset, modify and quit save it
 18. load a writable cityset, make it invalid, and then quit save it
 19. try saving a cityset we don't have permission to save
 20. try quit-saving a cityset we don't have permission to save
*/
