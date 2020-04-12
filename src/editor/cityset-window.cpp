//  Copyright (C) 2009, 2010, 2011, 2012, 2014, 2015, 2020 Ben Asselstine
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

#define method(x) sigc::mem_fun(*this, &CitySetWindow::x)

CitySetWindow::CitySetWindow(Glib::ustring load_filename)
{
  needs_saving = false;
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
    xml->get_widget ("help_about_menuitem", help_about_menuitem);
    help_about_menuitem->signal_activate().connect
       (method(on_help_about_activated));
    xml->get_widget ("tutorial_menuitem", tutorial_menuitem);
    tutorial_menuitem->signal_activate().connect
      (method(on_tutorial_video_activated));
    xml->get_widget("city_tile_width_spinbutton", city_tile_width_spinbutton);
    city_tile_width_spinbutton->set_range (1, 4);
    city_tile_width_spinbutton->signal_changed().connect
      (method(on_city_tile_width_changed));
    city_tile_width_spinbutton->signal_insert_text().connect
      (sigc::hide(sigc::hide(method(on_city_tile_width_text_changed))));
    xml->get_widget("ruin_tile_width_spinbutton", ruin_tile_width_spinbutton);
    ruin_tile_width_spinbutton->set_range (1, 4);
    ruin_tile_width_spinbutton->signal_changed().connect
      (method(on_ruin_tile_width_changed));
    ruin_tile_width_spinbutton->signal_insert_text().connect
      (sigc::hide(sigc::hide(method(on_ruin_tile_width_text_changed))));
    xml->get_widget("temple_tile_width_spinbutton",
		    temple_tile_width_spinbutton);
    temple_tile_width_spinbutton->set_range (1, 4);
    temple_tile_width_spinbutton->signal_changed().connect
      (method(on_temple_tile_width_changed));
    temple_tile_width_spinbutton->signal_insert_text().connect
      (sigc::hide(sigc::hide(method(on_temple_tile_width_text_changed))));

    xml->get_widget("change_citypics_button", change_citypics_button);
    change_citypics_button->signal_clicked().connect
      (method(on_change_citypics_clicked));
    xml->get_widget("change_razedcitypics_button", change_razedcitypics_button);
    change_razedcitypics_button->signal_clicked().connect
      (method(on_change_razedcitypics_clicked));
    xml->get_widget("change_portpic_button", change_portpic_button);
    change_portpic_button->signal_clicked().connect(method(on_change_portpic_clicked));
    xml->get_widget("change_signpostpic_button", change_signpostpic_button);
    change_signpostpic_button->signal_clicked().connect
      (method(on_change_signpostpic_clicked));
    xml->get_widget("change_ruinpics_button", change_ruinpics_button);
    change_ruinpics_button->signal_clicked().connect(method(on_change_ruinpics_clicked));
    xml->get_widget("change_templepic_button", change_templepic_button);
    change_templepic_button->signal_clicked().connect
      (method(on_change_templepic_clicked));
    xml->get_widget("change_towerpics_button", change_towerpics_button);
    change_towerpics_button->signal_clicked().connect
      (method(on_change_towerpics_clicked));
    xml->get_widget ("notebook", notebook);

    if (load_filename != "")
      current_save_filename = load_filename;
    update_cityset_panel();

    if (load_filename.empty() == false)
      {
	load_cityset (load_filename);
	update_cityset_panel();
        update_window_title();
      }
}

void
CitySetWindow::update_cityset_panel()
{
  cityset_alignment->set_sensitive(d_cityset != NULL);
  Glib::ustring no_image = _("No image set");
  Glib::ustring s;
  if (d_cityset && d_cityset->getCitiesFilename().empty() == false)
    s = d_cityset->getCitiesFilename();
  else
    s = no_image;
  change_citypics_button->set_label(s);
  if (d_cityset && d_cityset->getRazedCitiesFilename().empty() == false)
    s = d_cityset->getRazedCitiesFilename();
  else
    s = no_image;
  change_razedcitypics_button->set_label(s);
  if (d_cityset && d_cityset->getPortFilename().empty() == false)
    s = d_cityset->getPortFilename();
  else
    s = no_image;
  change_portpic_button->set_label(s);
  if (d_cityset && d_cityset->getSignpostFilename().empty() == false)
    s = d_cityset->getSignpostFilename();
  else
    s = no_image;
  change_signpostpic_button->set_label(s);
  if (d_cityset && d_cityset->getRuinsFilename().empty() == false)
    s = d_cityset->getRuinsFilename();
  else
    s = no_image;
  change_ruinpics_button->set_label(s);
  if (d_cityset && d_cityset->getTemplesFilename().empty() == false)
    s = d_cityset->getTemplesFilename();
  else
    s = no_image;
  change_templepic_button->set_label(s);
  if (d_cityset && d_cityset->getTowersFilename().empty() == false)
    s = d_cityset->getTowersFilename();
  else
    s = no_image;
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
  save_cityset_menuitem->set_sensitive (false);
  current_save_filename = "";
  if (d_cityset)
    delete d_cityset;

  guint32 num = 0;
  Glib::ustring name =
    Citysetlist::getInstance()->findFreeName(_("Untitled"), 100, num,
                                             Cityset::get_default_tile_size ());

  d_cityset = new Cityset (Citysetlist::getNextAvailableId (1), name);
  d_cityset->setNewTemporaryFile ();

  update_cityset_panel();
  needs_saving = true;
  update_window_title();
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
  if (d_cityset->validateCitiesFilename() == false)
    msgs.push_back(_("The cities picture is not set."));
  if (d_cityset->validateRazedCitiesFilename() == false)
    msgs.push_back(_("The razed cities picture is not set."));
  if (d_cityset->validatePortFilename() == false)
    msgs.push_back(_("The port picture is not set."));
  if (d_cityset->validateSignpostFilename() == false)
    msgs.push_back(_("The signpost picture is not set."));
  if (d_cityset->validateRuinsFilename() == false)
    msgs.push_back(_("The ruins picture is not set."));
  if (d_cityset->validateTemplesFilename() == false)
    msgs.push_back(_("The temple picture is not set."));
  if (d_cityset->validateTowersFilename() == false)
    msgs.push_back(_("The towers picture is not set."));
  if (d_cityset->validateCityTileWidth() == false)
    msgs.push_back(_("The tile width for temples must be over zero."));
  if (d_cityset->validateRuinTileWidth() == false)
    msgs.push_back(_("The tile width for ruins must be over zero."));
  if (d_cityset->validateTempleTileWidth() == false)
    msgs.push_back(_("The tile width for temples must be over zero."));
  if (msgs.empty() == true && isValidName () == false)
    msgs.push_back(_("The name of the City Set is not unique."));

  Glib::ustring msg = "";
  for (std::list<Glib::ustring>::iterator it = msgs.begin(); it != msgs.end();
       it++)
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
      lwc_filter->set_name(_("LordsAWar City Sets (*.lwc)"));
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
              save_cityset_menuitem->set_sensitive (true);
              needs_saving = false;
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
      needs_saving = false;
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
  if (check_save_valid (true))
    save_current_cityset_file();
}

void CitySetWindow::on_edit_cityset_info_activated()
{
  CitySetInfoDialog d(*window, d_cityset);
  bool changed = d.run();
  if (changed)
    {
      needs_saving = true;
      update_window_title();
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
  lwc_filter->set_name(_("LordsAWar City Sets (*.lwc)"));
  lwc_filter->add_pattern("*" + CITYSET_EXT);
  chooser.add_filter(lwc_filter);
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
          needs_saving = false;
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
  if (cityset == NULL)
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
  if (d_cityset)
    delete d_cityset;
  d_cityset = cityset;
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
  save_cityset_menuitem->set_sensitive (true);
  update_window_title();
  return true;
}

bool CitySetWindow::quit()
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
  d_cityset->setCityTileWidth(city_tile_width_spinbutton->get_value());
  needs_saving = true;
  update_window_title();
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
  d_cityset->setRuinTileWidth(ruin_tile_width_spinbutton->get_value());
  needs_saving = true;
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
  d_cityset->setTempleTileWidth(temple_tile_width_spinbutton->get_value());
  needs_saving = true;
  update_window_title();
}

void CitySetWindow::on_change_citypics_clicked()
{
  bool cleared = false;
  std::vector<PixMask *> frames;
  for (guint32 i = 0; i < MAX_PLAYERS + 1; i++)
    if (d_cityset->getCityImage (i))
      frames.push_back (d_cityset->getCityImage (i));
  Glib::ustring imgname = d_cityset->getCitiesFilename();
  Glib::ustring f =
    change_image(_("Select a Cities image"), imgname, MAX_PLAYERS + 1,
                 frames, cleared, d_cityset->getCityTileWidth ());
  if (cleared)
    d_cityset->uninstantiateSameNamedImages (imgname);
  else
    {
      if (f != "")
        {
          d_cityset->setCitiesFilename (f);
          d_cityset->instantiateCityImages();
        }
    }
  update_cityset_panel();
}

void CitySetWindow::on_change_razedcitypics_clicked()
{
  bool cleared = false;
  std::vector<PixMask *> frames;
  for (guint32 i = 0; i < MAX_PLAYERS; i++)
    if (d_cityset->getRazedCityImage (i))
      frames.push_back (d_cityset->getRazedCityImage (i));
  Glib::ustring imgname = d_cityset->getRazedCitiesFilename();
  Glib::ustring f = 
    change_image(_("Select a Razed Cities image"), imgname, MAX_PLAYERS,
                 frames, cleared, d_cityset->getCityTileWidth ());
  if (cleared)
    d_cityset->uninstantiateSameNamedImages (imgname);
  else
    {
      if (f != "")
        {
          d_cityset->setRazedCitiesFilename (f);
          d_cityset->instantiateRazedCityImages();
        }
    }
  update_cityset_panel();
}

void CitySetWindow::on_change_portpic_clicked()
{
  bool cleared = false;
  std::vector<PixMask *> frames;
  if (d_cityset->getPortImage ())
    frames.push_back (d_cityset->getPortImage ());
  Glib::ustring imgname = d_cityset->getPortFilename();
  Glib::ustring f = change_image(_("Select a Port image"), imgname, 1, frames,
                                 cleared, 1);
  if (cleared)
    d_cityset->uninstantiateSameNamedImages (imgname);
  else
    {
      if (f != "")
        {
          d_cityset->setPortFilename (f);
          d_cityset->instantiatePortImage();
        }
    }
  update_cityset_panel();
}

void CitySetWindow::on_change_signpostpic_clicked()
{
  bool cleared = false;
  std::vector<PixMask *> frames;
  if (d_cityset->getSignpostImage ())
    frames.push_back (d_cityset->getSignpostImage ());
  Glib::ustring imgname = d_cityset->getSignpostFilename();
  Glib::ustring f =
    change_image(_("Select a Signpost image"), imgname, 1, frames, cleared, 1);
  if (cleared)
    d_cityset->uninstantiateSameNamedImages (imgname);
  else
    {
      if (f != "")
        {
          d_cityset->setSignpostFilename (f);
          d_cityset->instantiateSignpostImage();
        }
    }
  update_cityset_panel();
}

void CitySetWindow::on_change_ruinpics_clicked()
{
  bool cleared = false;
  std::vector<PixMask *> frames;
  for (guint32 i = 0; i < RUIN_TYPES; i++)
    if (d_cityset->getRuinImage (i))
      frames.push_back (d_cityset->getRuinImage (i));
  Glib::ustring imgname = d_cityset->getRuinsFilename();
  Glib::ustring f =
    change_image(_("Select a Ruins image"), imgname, RUIN_TYPES, frames,
                 cleared, d_cityset->getRuinTileWidth ());
  if (cleared)
    d_cityset->uninstantiateSameNamedImages (imgname);
  else
    {
      if (f != "")
        {
          d_cityset->setRuinsFilename (f);
          d_cityset->instantiateRuinImages();
        }
    }
  update_cityset_panel();
}

void CitySetWindow::on_change_templepic_clicked()
{
  bool cleared = false;
  std::vector<PixMask *> frames;
  for (guint32 i = 0; i < TEMPLE_TYPES; i++)
    if (d_cityset->getTempleImage (i))
      frames.push_back (d_cityset->getTempleImage (i));
  Glib::ustring imgname = d_cityset->getTemplesFilename();
  Glib::ustring f =
    change_image(_("Select a Temples image"), imgname, TEMPLE_TYPES, frames,
                 cleared, d_cityset->getTempleTileWidth ());
  if (cleared)
    d_cityset->uninstantiateSameNamedImages (imgname);
  else
    {
      if (f != "")
        {
          d_cityset->setTemplesFilename (f);
          d_cityset->instantiateTempleImages();
        }
    }
  update_cityset_panel();
}

void CitySetWindow::on_change_towerpics_clicked()
{
  bool cleared = false;
  std::vector<PixMask *> frames;
  for (guint32 i = 0; i < MAX_PLAYERS; i++)
    if (d_cityset->getTowerImage (i))
      frames.push_back (d_cityset->getTowerImage (i));
  Glib::ustring imgname = d_cityset->getTowersFilename();
  Glib::ustring f =
    change_image(_("Select a Towers image"), imgname, MAX_PLAYERS, frames,
                 cleared, 1);
  if (cleared)
    d_cityset->uninstantiateSameNamedImages (imgname);
  else
    {
      if (f != "")
        {
          d_cityset->setTowersFilename (f);
          d_cityset->instantiateTowerImages();
        }
    }
  update_cityset_panel();
}

Glib::ustring CitySetWindow::change_image(Glib::ustring msg, Glib::ustring imgname, int num, std::vector<PixMask *> frames, bool &cleared, int tw)
{
  Glib::ustring newfile = "";

  ImageEditorDialog d(*window, imgname, num, frames,
                      EDITOR_DIALOG_TILE_PIC_FONTSIZE_MULTIPLE * (double)tw);
  d.set_title(msg);
  int response = d.run();
  if (response == Gtk::RESPONSE_ACCEPT && d.get_filename() != "")
    {
      Glib::ustring newname = "";
      bool success = false;
      if (imgname.empty () == true)
        success = d_cityset->addFileInCfgFile(d.get_filename(), newname);
      else
        success =
          d_cityset->replaceFileInCfgFile(imgname, d.get_filename(), newname);
      if (success)
        {
          newfile = newname;
          needs_saving = true;
          update_window_title();
        }
      else
        show_add_file_error(*d.get_dialog(), d.get_filename ());
    }
  else if (response == Gtk::RESPONSE_REJECT)
    {
      if (d_cityset->removeFileInCfgFile(imgname))
        {
          needs_saving = true;
          update_window_title();
          cleared = true;
          newfile = "";
        }
      else
        show_remove_file_error(*d.get_dialog(), imgname);
    }
  return newfile;
}

void CitySetWindow::update_window_title()
{
  Glib::ustring title = "";
  if (needs_saving)
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


