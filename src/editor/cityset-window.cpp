//  Copyright (C) 2009, 2010, 2011, 2012, 2014, 2015, 2020, 2021,
//  2026 Ben Asselstine
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
//  Foundation, Inc., 31 Milk Street #960789, Boston, MA 02196, USA.

#include <gtkmm.h>

#include "cityset-window.h"
#include "city-set.h"
#include "tar-file-image.h"
#include "undo-mgr.h"
#include "cityset-undo.h"
#include "about-dialog.h"
#include "image-editor-dialog.h"
#include "cityset-info-dialog.h"
#include "file-label.h"
#include "game-map.h"
#include "city-set-list.h"

CitySetWindow::CitySetWindow ()
        : m_actions (Gio::SimpleActionGroup::create ())
{
  m_cityset = NULL;
  set_size_request (600, 450);
  m_umgr = new UndoMgr (UndoMgr::DELAY, UndoMgr::LIMIT);
  m_umgr->signal_execute ().connect
    (sigc::mem_fun (*this, &CitySetWindow::execute_action));
  m_umgr->signal_added_undo ().connect
    ([this] ()
     {
       m_cityset_modified = true;
       update_window_title ();
       update_actions ();
     });
  set_resizable (false);
}

CitySetWindow::~CitySetWindow ()
{
  disconnect_signals ();
  disconnect_action_signals ();
  delete m_umgr;
}

void CitySetWindow::setup (Cityset *cityset)
{
  m_cityset = NULL;
  if (cityset)
    {
      bool broken = false;
      m_cityset = new Cityset (*cityset);
      m_cityset->instantiateImages (broken);
      m_cityset->setLoadTemporaryFile ();
      m_current_save_filename = cityset->getConfigurationFile (true);
    }
  m_cityset_modified = false;
  m_new_cityset_needs_saving = false;

  m_menu_button = Gtk::make_managed<CitySetMenuButton>(*this, m_actions);
  m_menu_button->m_signal_action_added.connect
    ([this] (Glib::RefPtr<Gio::SimpleAction> action)
     {
       m_simple_actions[action->get_name ()] = action;
     });

  m_menu_button->setup ();

  auto idx = std::string (LW_APP_ID).rfind ('.');
  insert_action_group (std::string (LW_APP_ID).substr (idx + 1), m_actions);
  setup_accels ();

  setup_header_bar (m_menu_button);

  populate ();

  connect_action_signals ();
  connect_signals ();

  setup_quit ();

  update ();
}

void CitySetWindow::setup_header_bar (Gtk::MenuButton *menu_button)
{
  m_header_bar = Gtk::make_managed<Gtk::HeaderBar>();
  m_header_bar->set_show_title_buttons (true);

  m_notebook = Gtk::make_managed<Gtk::Notebook> ();
  m_notebook->set_show_tabs (false);
  m_notebook->set_show_border (false);

  auto page1 = Gtk::make_managed<Gtk::Box> (Gtk::Orientation::VERTICAL);
  auto title1 = Gtk::make_managed<Gtk::Label>(_("City Set Editor"));
  title1->add_css_class ("title");
  page1->append (*title1);
  page1->set_valign (Gtk::Align::CENTER);
  m_notebook->append_page (*page1, "");

  auto page2 = Gtk::make_managed<Gtk::Box> (Gtk::Orientation::VERTICAL);
  auto title2 = Gtk::make_managed<Gtk::Label>(_("City Set Editor"));
  title2->add_css_class ("title");
  page2->append (*title2);
  page2->set_valign (Gtk::Align::CENTER);


  m_subtitle = Gtk::make_managed<Gtk::Label>("");
  m_subtitle->add_css_class ("subtitle");
  page2->append (*m_subtitle);

  m_notebook->append_page (*page2, "");
  m_header_bar->set_title_widget (*m_notebook);

  m_header_bar->pack_end (*menu_button);
  set_titlebar (*m_header_bar);
}

Gtk::Box *CitySetWindow::create_attribute_row (Glib::ustring title, Glib::ustring desc,
                                               Glib::ustring css)
{
  auto box = Gtk::make_managed<Gtk::Box> (Gtk::Orientation::HORIZONTAL);
  box->add_css_class (css);
  auto titlebox = Gtk::make_managed<Gtk::Box> (Gtk::Orientation::VERTICAL);
  auto title_label = Gtk::make_managed<Gtk::Label> (title);
  title_label->add_css_class ("setting_label");
  title_label->set_halign (Gtk::Align::START);
  title_label->set_hexpand (true);
  titlebox->append (*title_label);
  if (desc != "")
    {
      auto desc_label = Gtk::make_managed<Gtk::Label> (desc);
      desc_label->add_css_class ("setting_explain_label");
      desc_label->set_hexpand (true);
      desc_label->set_halign (Gtk::Align::START);
      titlebox->append (*desc_label);
    }
  box->set_spacing (6);
  box->append (*titlebox);
  return box;
}

void CitySetWindow::add_connection (sigc::connection c)
{
  m_connections.push_back (c);
}

void CitySetWindow::add_action_connection (sigc::connection c)
{
  m_action_connections.push_back (c);
}

void CitySetWindow::action_connect (Glib::ustring name, sigc::slot<void()> slot)
{
  auto simple =
    std::dynamic_pointer_cast<Gio::SimpleAction>(m_simple_actions[name]);
  if (simple)
    add_action_connection
      (simple->signal_activate ().connect (sigc::hide (slot)));
}

void CitySetWindow::connect_action_signals ()
{
  action_connect
    ("cityset.file.new",
     [this] ()
     {
       check_discard
         (_("Save these changes before making a new city set?"),
          [this] (bool discard)
          {
            if (discard)
              {
                m_current_save_filename = "";
                disconnect_signals ();
                if (m_cityset)
                  delete m_cityset;

                guint32 num = 0;
                Glib::ustring name =
                  Citysetlist::instance ()->findFreeName
                  (_("Untitled"), 100, num,
                   Cityset::get_default_tile_size ());

                m_cityset =
                  new Cityset (Citysetlist::getNextAvailableId (1), name);
                m_cityset->setNewTemporaryFile ();
                connect_signals ();

                update_cityset_panel ();
                m_cityset_modified = false;
                m_new_cityset_needs_saving = true;
                update ();
                m_umgr->clear ();
                update_actions ();
              }
          });
     });

  action_connect
    ("cityset.file.open",
     [this] ()
     {
       check_discard
         (_("Save these changes before making a new city set?"),
          [this] (bool discard)
          {
            if (discard)
              {
                LwDialog::open 
                  (*this, _("Choose a city set to open"),
                   FileFilter::CITYSET,
                   [this] (std::string path)
                   {
                     load_cityset
                       (path,
                        [this] (bool loaded)
                        {
                          if (loaded)
                            {
                              m_cityset_modified = false;
                              m_new_cityset_needs_saving = false;
                              update ();
                              //update_window_title ();
                              //update_cityset_panel ();
                            }
                        });
                   });
              }
          });
     });

  action_connect
    ("cityset.file.save",
     [this] ()
     {
       if (m_current_save_filename.empty () == true)
         on_save_as_activated ();
       else
         {
           check_save_valid
             (true,
              [this](bool valid)
              {
                if (valid)
                  {
                    save_current_cityset_file
                      ("",
                       [this] (bool saved)
                       {
                         (void) saved;
                       });
                  }
              });
         }
     });

  action_connect
    ("cityset.file.save-as",
     [this] ()
     {
       on_save_as_activated ();

     });

  action_connect
    ("cityset.file.validate",
     [this] ()
     {
       std::list<Glib::ustring> msgs;
       if (m_cityset == NULL)
         return;
       if (msgs.empty () == true)
         {
           bool valid = String::utrim (m_cityset->getName ()) != "";
           if (!valid)
             {
               Glib::ustring s = _("The name of the city set is invalid.");
               msgs.push_back (s);
             }
         }
       if (m_cityset->getCity ()->getName ().empty () == true)
         msgs.push_back (_("The cities image file is not set."));
       if (m_cityset->getRazedCity ()->getName ().empty () == true)
         msgs.push_back (_("The razed cities image file is not set."));
       if (m_cityset->getPort ()->getName ().empty () == true)
         msgs.push_back (_("The port image file is not set."));
       if (m_cityset->getSignpost ()->getName ().empty () == true)
         msgs.push_back (_("The signpost image file is not set."));
       if (m_cityset->getRuin ()->getName ().empty () == true)
         msgs.push_back (_("The ruins image file is not set."));
       if (m_cityset->getTemple ()->getName ().empty () == true)
         msgs.push_back (_("The temple image file is not set."));
       if (m_cityset->getTower ()->getName( ). empty () == true)
         msgs.push_back (_("The towers image file is not set."));
       if (m_cityset->validateCityTileWidth () == false)
         msgs.push_back (_("The tile span for cities must be over zero."));
       if (m_cityset->validateRuinTileWidth () == false)
         msgs.push_back (_("The tile span for ruins must be over zero."));
       if (m_cityset->validateTempleTileWidth () == false)
         msgs.push_back (_("The tile span for temples must be over zero."));
       if (msgs.empty () == true && is_valid_name () == false)
         msgs.push_back (_("The name of the city set is not unique."));

       Glib::ustring msg = "";
       for (auto m : msgs)
         {
           msg += m + "\n";
           break; // we only show one
         }

       if (msg == "")
         msg = _("The city set is valid.");

       Glib::ustring detail = "";
       if (msgs.size () > 1)
         detail =
           String::ucompose
           (ngettext ("(There is %1 more error not shown)",
                      "(There are %1 more errors not shown)",
                      msgs.size () - 1), msgs.size () - 1);
       auto dialog = LwDialog::alert (msg, detail);
       dialog->choose
         (*this,
          [this, dialog] (auto result)
          {
            dialog->choose_finish (result);
            return;
          });
     });

  action_connect
    ("cityset.file.quit",
     [this] ()
     {
       check_quit
         ([this] (bool quit)
          {
            if (quit)
              {
                if (m_cityset)
                  delete m_cityset;
                hide ();
                m_signal_closed.emit ();
              }
          });
     });

  action_connect
    ("cityset.edit.properties",
     [this] ()
     {
       on_edit_cityset_info_activated ();
     });

  action_connect
    ("cityset.help.tutorial-video",
     [this] ()
     {
       Glib::ustring uri = "https://vimeo.com/406899445";
       auto launcher = Gtk::UriLauncher::create (uri);

       launcher->launch
         (*this,
          [launcher](const Glib::RefPtr<Gio::AsyncResult>& result)
          {
            try
              {
                launcher->launch_finish (result);
              }
            catch (const Glib::Error& ex)
              {
                std::cerr << ex.what () << '\n';
              }
          });
     });


  action_connect
    ("cityset.help.keyboard-shortcuts",
     [this] ()
     {
       auto* dialog = Gtk::make_managed<CitySetShortcutsDialog>();
       dialog->set_transient_for (*this);
       dialog->present ();
     });

  action_connect
    ("cityset.help.about",
     [this] ()
     {
       auto d = Gtk::make_managed<AboutDialog> (*this);
       d->set_program_name ("LordsAWar! City Set Editor");
       bool broken;
       auto logo =
         PixMask::create (File::getVariousFile ("tileset_icon.png"),
                          broken);
       if (logo)
         d->set_logo (logo->to_texture ());
       d->present ();
       d->signal_response ().connect
         ([d] (Gtk::ResponseType)
          {
            delete d;
          });
     });

}

void CitySetWindow::disconnect_signals ()
{
  for (auto c : m_connections)
    c.disconnect ();
  m_connections.clear ();
}

void CitySetWindow::disconnect_action_signals ()
{
  for (auto c : m_action_connections)
    c.disconnect ();
  m_action_connections.clear ();
}

void CitySetWindow::load_cityset (Glib::ustring filename, sigc::slot<void(bool)> after)
{
  Glib::ustring old = m_current_save_filename;
  m_current_save_filename = filename;

  Cityset::create
    (filename,
     [this, after, old] (Cityset *cityset, bool broken,
                         bool unsupported_version, Glib::ustring err)
     {
       if (cityset == NULL || unsupported_version || broken)
         {
           Glib::ustring msg = _("The city set could not be loaded.");
           auto dialog = LwDialog::alert (msg, err);
           dialog->choose
             (*this,
              [this, dialog, old, after] (auto result)
              {
                dialog->choose_finish (result);
                m_current_save_filename = old;
                return after (false);
              });
           return;
         }
       disconnect_signals ();
       Cityset *old_cityset = m_cityset;
       m_cityset = NULL;
       update ();
       m_cityset = cityset;
       connect_signals ();
       m_cityset->setLoadTemporaryFile ();

       Glib::signal_idle ().connect
         ([this, old_cityset, after] ()
          {
            bool broke = false;
            m_cityset->instantiateImages (broke);
            if (broke)
              {
                delete m_cityset;
                m_cityset = NULL;
                Glib::ustring msg = _("Couldn't load city set images");
                auto dialog = LwDialog::alert (msg);
                dialog->choose
                  (*this,
                   [this, old_cityset, after, dialog] (auto result)
                   {
                     m_cityset = old_cityset;
                     update ();
                     dialog->choose_finish (result);
                     return after (false);
                   });
              }
            else
              {
                delete old_cityset;
                update ();
                after (true);
              }
            return false;
          });
     });
}

void CitySetWindow::connect_signals ()
{
  add_connection
    (m_change_citypics_button->signal_clicked ().connect
     ([this] ()
      {
        on_change_clicked (_("Select a cities image"),
                           m_cityset->getCity ());
      }));

  add_connection
    (m_change_razedcitypics_button->signal_clicked ().connect
     ([this] ()
      {
        on_change_clicked (_("Select a razed cities image"),
                           m_cityset->getRazedCity ());
      }));

  add_connection
    (m_change_portpic_button->signal_clicked ().connect
     ([this] ()
      {
        on_change_clicked (_("Select a port image"),
                           m_cityset->getPort ());
      }));

  add_connection
    (m_change_signpostpic_button->signal_clicked ().connect
     ([this] ()
      {
        on_change_clicked (_("Select a signpost image"),
                           m_cityset->getSignpost ());
      }));

  add_connection
    (m_change_ruinpics_button->signal_clicked ().connect
     ([this] ()
      {
        on_change_clicked (_("Select a ruins image"),
                           m_cityset->getRuin ());
      }));

  add_connection
    (m_change_templepics_button->signal_clicked ().connect
     ([this] ()
      {
        on_change_clicked (_("Select a temples image"),
                           m_cityset->getTemple ());
      }));

  add_connection
    (m_change_towerpics_button->signal_clicked ().connect
     ([this] ()
      {
        on_change_clicked (_("Select a towers image"),
                           m_cityset->getTower ());
      }));

  add_connection
    (m_city_tile_width_spinbutton->signal_value_changed ().connect
     ([this] ()
      {
        if (m_cityset)
          {
            m_umgr->add (new CitySetUndoAction_CityWidth
                         (m_cityset->getCityTileWidth ()));
            m_cityset->setCityTileWidth
              (m_city_tile_width_spinbutton->get_value ());
            update_window_title ();
            update_actions ();
          }

      }));

  add_connection
    (m_ruin_tile_width_spinbutton->signal_value_changed ().connect
     ([this] ()
      {
        if (m_cityset)
          {
            m_umgr->add (new CitySetUndoAction_RuinWidth
                         (m_cityset->getRuinTileWidth ()));
            m_cityset->setRuinTileWidth
              (m_ruin_tile_width_spinbutton->get_value ());
            update_window_title ();
            update_actions ();
          }
      }));

  add_connection
    (m_temple_tile_width_spinbutton->signal_value_changed ().connect
     ([this] ()
      {
        if (m_cityset)
          {
            m_umgr->add (new CitySetUndoAction_TempleWidth
                         (m_cityset->getTempleTileWidth ()));
            m_cityset->setTempleTileWidth
              (m_temple_tile_width_spinbutton->get_value ());
            update_window_title ();
            update_actions ();
          }
      }));
}

void CitySetWindow::update ()
{
  update_window_title ();
  update_cityset_panel ();
  update_actions ();
}

void CitySetWindow::update_actions ()
{
  bool a = m_cityset != NULL;
  m_simple_actions ["cityset.edit.properties"]->set_enabled (a);
  m_simple_actions ["cityset.file.validate"]->set_enabled (a);
  m_simple_actions ["cityset.file.save-as"]->set_enabled (a);
  m_simple_actions ["cityset.file.save"]->set_enabled
    (m_umgr->undo_empty () == false);
}

void CitySetWindow::update_cityset_panel ()
{
  bool sensitive;
  disconnect_signals ();
  if (m_cityset)
    {
      sensitive = true;
      Glib::ustring s = "";
      if (m_cityset && m_cityset->getCity ()->getName ().empty () == false)
        s = m_cityset->getCity ()->getName ();
      m_change_citypics_filelabel->set_label (s);

      s = "";
      if (m_cityset->getRazedCity ()->getName ().empty () == false)
        s = m_cityset->getRazedCity ()->getName ();
      m_change_razedcitypics_filelabel->set_label (s);

      s = "";
      if (m_cityset->getPort ()->getName ().empty () == false)
        s = m_cityset->getPort ()->getName ();
      m_change_portpic_filelabel->set_label (s);

      s = "";
      if (m_cityset->getSignpost ()->getName ().empty () == false)
        s = m_cityset->getSignpost ()->getName ();
      m_change_signpostpic_filelabel->set_label (s);

      s = "";
      if (m_cityset->getRuin ()->getName ().empty () == false)
        s = m_cityset->getRuin ()->getName ();
      m_change_ruinpics_filelabel->set_label (s);

      s = "";
      if (m_cityset->getTemple ()->getName ().empty () == false)
        s = m_cityset->getTemple ()->getName ();
      m_change_templepics_filelabel->set_label (s);

      s = "";
      if (m_cityset->getTower ()->getName ().empty () == false)
        s = m_cityset->getTower ()->getName ();
      m_change_towerpics_filelabel->set_label (s);

      m_city_tile_width_spinbutton->set_value
        (m_cityset->getCityTileWidth ());
      m_ruin_tile_width_spinbutton->set_value
        (m_cityset->getRuinTileWidth ());
      m_temple_tile_width_spinbutton->set_value
        (m_cityset->getTempleTileWidth ());
    }
  else
    {
      sensitive = false;
      m_change_citypics_filelabel->set_label ("");
      m_change_razedcitypics_filelabel->set_label ("");
      m_change_portpic_filelabel->set_label ("");
      m_change_signpostpic_filelabel->set_label ("");
      m_change_ruinpics_filelabel->set_label ("");
      m_change_templepics_filelabel->set_label ("");
      m_change_towerpics_filelabel->set_label ("");
      m_city_tile_width_spinbutton->set_value (2);
      m_ruin_tile_width_spinbutton->set_value (1);
      m_temple_tile_width_spinbutton->set_value (1);
    }

  m_change_citypics_button->set_sensitive (sensitive);
  m_change_razedcitypics_button->set_sensitive (sensitive);
  m_change_portpic_button->set_sensitive (sensitive);
  m_change_signpostpic_button->set_sensitive (sensitive);
  m_change_ruinpics_button->set_sensitive (sensitive);
  m_change_templepics_button->set_sensitive (sensitive);
  m_change_towerpics_button->set_sensitive (sensitive);
  m_city_tile_width_spinbutton->set_sensitive (sensitive);
  m_ruin_tile_width_spinbutton->set_sensitive (sensitive);
  m_temple_tile_width_spinbutton->set_sensitive (sensitive);

  connect_signals ();
}

void CitySetWindow::update_window_title ()
{
  Glib::ustring title = "";
  if (m_cityset_modified || m_new_cityset_needs_saving)
    title += "*";
  if (m_cityset)
    {
      m_notebook->set_current_page (1);
      title += m_cityset->getName ();
      m_subtitle->set_text (title);
    }
  else
    {
      m_notebook->set_current_page (0);
      m_subtitle->set_text ("");
    }
}

void CitySetWindow::populate ()
{
  auto box = Gtk::make_managed<Gtk::Box> (Gtk::Orientation::VERTICAL);
  auto scrolled_window = Gtk::make_managed<Gtk::ScrolledWindow> ();
  scrolled_window->set_policy
    (Gtk::PolicyType::NEVER, Gtk::PolicyType::AUTOMATIC);
  box->set_margin (6);

  auto images_label = Gtk::make_managed<Gtk::Label> ("Images");
  images_label->add_css_class ("setting_heading");
  box->append (*images_label);

  auto city_images = create_attribute_row
    (_("Cities"),
     _("What the cities look like (9 images on a row in the file)"),
     "setting_top");
  m_change_citypics_button = Gtk::make_managed<Gtk::Button> ();
  m_change_citypics_filelabel =
    Gtk::make_managed<FileLabel> (m_change_citypics_button);
  city_images->append (*m_change_citypics_button);
  box->append (*city_images);

  auto razed_city_images = create_attribute_row
    (_("Razed Cities"),
     _("What the burned cities look like (8 images)"),
     "setting_middle");
  m_change_razedcitypics_button = Gtk::make_managed<Gtk::Button> ();
  m_change_razedcitypics_filelabel =
    Gtk::make_managed<FileLabel> (m_change_razedcitypics_button);
  razed_city_images->append (*m_change_razedcitypics_button);
  box->append (*razed_city_images);

  auto port_image = create_attribute_row
    (_("Port"),
     _("What a port looks like (1 image)"),
     "setting_middle");
  m_change_portpic_button = Gtk::make_managed<Gtk::Button> ();
  m_change_portpic_filelabel =
    Gtk::make_managed<FileLabel> (m_change_portpic_button);
  port_image->append (*m_change_portpic_button);
  box->append (*port_image);

  auto signpost_image = create_attribute_row
    (_("Signpost"),
     _("What a signpost looks like (1 image)"),
     "setting_middle");
  m_change_signpostpic_button = Gtk::make_managed<Gtk::Button> ();
  m_change_signpostpic_filelabel =
    Gtk::make_managed<FileLabel> (m_change_signpostpic_button);
  signpost_image->append (*m_change_signpostpic_button);
  box->append (*signpost_image);

  auto ruin_images = create_attribute_row
    (_("Ruins"),
     _("What a ruin looks like (3 images)"),
     "setting_middle");
  m_change_ruinpics_button = Gtk::make_managed<Gtk::Button> ();
  m_change_ruinpics_filelabel =
    Gtk::make_managed<FileLabel> (m_change_ruinpics_button);
  ruin_images->append (*m_change_ruinpics_button);
  box->append (*ruin_images);

  auto temple_images = create_attribute_row
    (_("Temples"),
     _("What a temple looks like (2 images)"),
     "setting_middle");
  m_change_templepics_button = Gtk::make_managed<Gtk::Button> ();
  m_change_templepics_filelabel =
    Gtk::make_managed<FileLabel> (m_change_templepics_button);
  temple_images->append (*m_change_templepics_button);
  box->append (*temple_images);

  auto tower_images = create_attribute_row
    (_("Towers"),
     _("What a stack looks like in defense (8 images)"),
     "setting_bottom");
  m_change_towerpics_button = Gtk::make_managed<Gtk::Button> ();
  m_change_towerpics_filelabel =
    Gtk::make_managed<FileLabel> (m_change_towerpics_button);
  tower_images->append (*m_change_towerpics_button);
  box->append (*tower_images);

  auto width_label = Gtk::make_managed<Gtk::Label> ("Tile Span");
  width_label->add_css_class ("setting_heading");
  box->append (*width_label);

  auto city_width =
    create_attribute_row (_("City"),
                          _("How many tiles a city occupies.  e.g. 2x2"),
                          "setting_top");
  m_city_tile_width_spinbutton = Gtk::make_managed<Gtk::SpinButton> ();
  m_city_tile_width_spinbutton->set_orientation
    (Gtk::Orientation::HORIZONTAL);
  m_city_tile_width_spinbutton->set_adjustment
    (Gtk::Adjustment::create (1, 1, 4, 1, 1, 0));
  city_width->append (*m_city_tile_width_spinbutton);
  box->append (*city_width);

  auto ruin_width =
    create_attribute_row (_("Ruin"),
                          _("How many tiles a ruin occupies."),
                          "setting_middle");
  m_ruin_tile_width_spinbutton = Gtk::make_managed<Gtk::SpinButton> ();
  m_ruin_tile_width_spinbutton->set_orientation
    (Gtk::Orientation::HORIZONTAL);
  m_ruin_tile_width_spinbutton->set_adjustment
    (Gtk::Adjustment::create (1, 1, 4, 1, 1, 0));
  ruin_width->append (*m_ruin_tile_width_spinbutton);
  box->append (*ruin_width);

  auto temple_width =
    create_attribute_row (_("Temple"),
                          _("How many tiles a temple occupies."),
                          "setting_bottom");
  m_temple_tile_width_spinbutton = Gtk::make_managed<Gtk::SpinButton> ();
  m_temple_tile_width_spinbutton->set_orientation
    (Gtk::Orientation::HORIZONTAL);
  m_temple_tile_width_spinbutton->set_adjustment
    (Gtk::Adjustment::create (1, 1, 4, 1, 1, 0));
  temple_width->append (*m_temple_tile_width_spinbutton);
  box->append (*temple_width);

  scrolled_window->set_child (*box);
  set_child (*scrolled_window);
}

UndoAction* CitySetWindow::execute_action (UndoAction *a2)
{
  auto action = dynamic_cast<CitySetUndoAction*>(a2);
  UndoAction *out = NULL;

  switch (action->getType ())
    {
    case CitySetUndoAction::CHANGE_PROPERTIES:
        {
          auto a = dynamic_cast<CitySetUndoAction_Properties*>(action);
          out = new CitySetUndoAction_Properties
            (m_cityset->getName (),
             m_cityset->getInfo (),
             m_cityset->getCopyright (),
             m_cityset->getLicense (),
             m_cityset->getTileSize ());
          m_cityset->setName (a->get_name ());
          m_cityset->setInfo (a->get_description ());
          m_cityset->setCopyright (a->get_copyright ());
          m_cityset->setLicense (a->get_license ());
          m_cityset->setTileSize (a->get_tile_size ());
          break;
        }

    case CitySetUndoAction::ADD_IMAGE:
        {
          auto a = dynamic_cast<CitySetUndoAction_AddImage*>(action);
          out = new CitySetUndoAction_AddImage (m_cityset);
          reload_cityset (a);
          break;
        }

    case CitySetUndoAction::CLEAR_IMAGE:
        {
          auto a = dynamic_cast<CitySetUndoAction_ClearImage*>(action);
          out = new CitySetUndoAction_ClearImage (m_cityset);
          reload_cityset (a);
          break;
        }

    case CitySetUndoAction::CITY_TILE_WIDTH:
        {
          auto a = dynamic_cast<CitySetUndoAction_CityWidth*>(action);
          out = new CitySetUndoAction_CityWidth
            (m_cityset->getCityTileWidth ());
          disconnect_signals ();
          m_city_tile_width_spinbutton->set_text
            (String::ucompose ("%1", a->get_city_width ()));
          connect_signals ();
          m_cityset->setCityTileWidth (a->get_city_width ());
          break;
        }

    case CitySetUndoAction::RUIN_TILE_WIDTH:
        {
          auto a = dynamic_cast<CitySetUndoAction_RuinWidth*>(action);
          out = new CitySetUndoAction_RuinWidth
            (m_cityset->getRuinTileWidth ());
          disconnect_signals ();
          m_ruin_tile_width_spinbutton->set_text
            (String::ucompose ("%1", a->get_ruin_width ()));
          connect_signals ();
          m_cityset->setRuinTileWidth (a->get_ruin_width ());
          break;
        }

    case CitySetUndoAction::TEMPLE_TILE_WIDTH:
        {
          auto a = dynamic_cast<CitySetUndoAction_TempleWidth*>(action);
          out = new CitySetUndoAction_TempleWidth
            (m_cityset->getTempleTileWidth ());
          disconnect_signals ();
          m_temple_tile_width_spinbutton->set_text
            (String::ucompose ("%1", a->get_temple_width ()));
          connect_signals ();
          m_cityset->setTempleTileWidth (a->get_temple_width ());
          break;
        }
    }
  return out;
}

void CitySetWindow::reload_cityset (CitySetUndoAction_Save *action)
{
  Glib::ustring olddir = m_cityset->getDirectory ();
  Glib::ustring oldname =
    File::get_basename (m_cityset->getConfigurationFile (true));
  Glib::ustring oldext = m_cityset->getExtension ();

  m_cityset->clean_tmp_dir ();
  delete m_cityset;
  m_cityset = new Cityset (*(action->get_cityset ()));
  m_cityset->setLoadTemporaryFile ();

  m_cityset->setDirectory (olddir);
  m_cityset->setBaseName (oldname);
  m_cityset->setExtension (oldext);
  update ();
}

void CitySetWindow::check_discard (Glib::ustring msg, sigc::slot<void(bool)> after)
{
  if (m_cityset_modified || m_new_cityset_needs_saving)
    {
      auto d = Gtk::AlertDialog::create (_("Save changes?"));
      d->set_detail (msg);
      d->set_buttons ({_("Save"), _("Discard"), _("Cancel")});
      d->set_cancel_button (2);
      d->set_default_button (0);
      d->set_modal (true);
      d->choose
        (*this,
         [this, d, after] (Glib::RefPtr<Gio::AsyncResult>& result)
         {
           int resp = d->choose_finish (result);
           switch (resp)
             {
             case 0: // save
               check_save_valid
                 (true,
                  [this, after] (bool valid)
                  {
                    if (!valid)
                      return after (false);

                    if (m_cityset->getDirectory ().empty () == false)
                      {
                        save_current_cityset_file_as
                          ([this, after] (bool saved)
                           {
                             after (saved);
                           });
                      }
                    else
                      {
                        save_current_cityset_file
                          ("",
                           [this, after] (bool saved)
                           {
                             after (saved);
                           });
                      }
                  });
               break;

             case 1: // discard
               after (true);
               break;

             default: // cancel
               after (false);
               break;
             }
         });
    }
  else
    after (true);
  return;
}

void CitySetWindow::check_name_valid (bool existing, sigc::slot<void(bool)> after)
{
  Glib::ustring name = m_cityset->getName ();
  Glib::ustring newname = "";
  if (existing)
    {
      Cityset *oldcityset =
        Citysetlist::instance ()->get (m_cityset->getId());
      if (oldcityset && oldcityset->getName () != name)
        newname = oldcityset->getName ();
    }
  guint32 num = 0;
  Glib::ustring n = String::utrim (String::strip_trailing_numbers (name));
  if (n == "")
    n = _("Untitled");
  if (newname.empty () == true)
    newname =
      Citysetlist::instance ()->findFreeName (n, 100, num,
                                              m_cityset->getTileSize ());
  if (name == "")
    {
      if (newname.empty() == true)
        {
          Glib::ustring msg =
            _("The city set has an invalid name.\n"
              "Change it and save again.");
          auto d = LwDialog::alert (msg);
          d->choose
            (*this,
             [this, d, after] (auto result)
             {
               d->choose_finish (result);
               on_edit_cityset_info_activated ();
               after (false);
             });
          return;
        }
      else
        {
          Glib::ustring msg =
            String::ucompose (_("The city set has an invalid name.\n"
                                "Change it to '%1'?"), newname);
          auto d = LwDialog::alert_yn (msg);
          d->choose
            (*this,
             [this, d, existing, newname, after] (auto result)
             {
               bool change = d->choose_finish (result) == 1;
               if (change)
                 {
                   m_cityset->setName (newname);
                   check_name_valid2 (existing, newname, after);
                 }
               else
                 after (change);
             });
        }
    }
  else
    check_name_valid2 (existing, newname, after);

  return;
}

void CitySetWindow::check_name_valid2 (bool existing, Glib::ustring newname,
                                       sigc::slot<void(bool)> after)
{
  //okay the question is whether or not the name is already used.
  bool same_name = false;
  Glib::ustring file =
    Citysetlist::instance ()->lookupConfigurationFileByName (m_cityset);
  if (file == "")
    return after (true);

  Glib::ustring cfgfile = m_cityset->getConfigurationFile (true);

  if (existing) // this means we're doing File->Save
    {
      if (file == cfgfile)
        return after (true);
      same_name = true;
    }
  else // this means we're doing File->Save As
    same_name = true;

  if (same_name)
    {
      if (newname.empty () == true)
        {
          Glib::ustring msg =
            _("The city set has the same name as another one.\n"
              "Change it and save again.");
          auto d = LwDialog::alert (msg);
          d->choose
            (*this,
             [this, d, after] (auto result)
             {
               d->choose_finish (result);
               on_edit_cityset_info_activated ();
               after (false);
             });
        }
      else
        {
          Glib::ustring msg =
            String::ucompose (_("The city set has the same name as "
                                "another one.\nChange it to '%1' "
                                "instead?"), newname);
          auto d = LwDialog::alert_yn (msg);
          d->choose
            (*this,
             [this, d, newname, after] (auto result)
             {
               bool change = d->choose_finish (result) == 1;
               if (change)
                 m_cityset->setName (newname);
               after (change);
             });
        }
    }
}

bool CitySetWindow::is_valid_name ()
{
  Glib::ustring file =
    Citysetlist::instance ()->lookupConfigurationFileByName (m_cityset);
  if (file == "")
    return true;
  if (file == m_cityset->getConfigurationFile (true))
    return true;
  return false;
}

void CitySetWindow::check_save_valid (bool existing, sigc::slot<void(bool)> after)
{
  check_name_valid
    (existing,
     [this, existing, after] (bool valid)
     {
       if (!valid)
         return after (false);

       if (m_cityset->validate () == false)
         {
           if (existing &&
               GameMap::instance ()->getCitysetId () == m_cityset->getId ())
             {
               Glib::ustring errmsg =
                 _("The city set is invalid, and is also the current "
                   "working one.");
               Glib::ustring msg = _("The city set could not be saved.");
               Glib::ustring detail = m_current_save_filename + "\n" + errmsg;
               auto d = LwDialog::alert (msg, detail);
               d->choose
                 (*this,
                  [this, d, after] (auto result)
                  {
                    d->choose_finish (result);
                    after (false);
                  });

               return;
             }
           else
             {
               Glib::ustring msg =
                 _("The city set is invalid.  Do you want to proceed?");
               auto d = LwDialog::alert_yn (msg);
               d->choose
                 (*this,
                  [after, d] (auto result)
                  {
                    bool ret = d->choose_finish (result) == 1;
                    after (ret);
                  });
             }
         }
       else
         after (true);
     });

  return;
}

void CitySetWindow::setup_accels ()
{
  auto actions = Gio::SimpleActionGroup::create();

  actions->add_action
    ("undo",
     ([this] ()
      {
        m_umgr->undo ();
        if (m_umgr->undo_empty () && !m_new_cityset_needs_saving)
          m_cityset_modified = false;
        update ();
      }));

  actions->add_action
    ("redo",
     ([this] ()
      {
        m_cityset_modified = true;
        m_umgr->redo ();
        update ();
      }));

  insert_action_group ("win", actions);

  auto shortcuts = Gtk::ShortcutController::create();
  shortcuts->set_propagation_phase(Gtk::PropagationPhase::CAPTURE);

  shortcuts->add_shortcut
    (Gtk::Shortcut::create
     (Gtk::ShortcutTrigger::parse_string ("<Control>z"),
      Gtk::NamedAction::create ("win.undo")));

  shortcuts->add_shortcut
    (Gtk::Shortcut::create
     (Gtk::ShortcutTrigger::parse_string ("<Control>Y"),
      Gtk::NamedAction::create ("win.redo")));

  shortcuts->add_shortcut
    (Gtk::Shortcut::create
     (Gtk::ShortcutTrigger::parse_string ("<Control><Shift>Z"),
      Gtk::NamedAction::create ("win.redo")));

  add_controller (shortcuts);
}

void CitySetWindow::on_change_clicked (Glib::ustring msg, TarFileImage *im)
{
  change_image
    (msg, im,
     [this, im](bool err, bool cleared, Glib::ustring f)
     {
       if (err)
         return;
       if (cleared)
         {
           im->clear ();
         }
       else
         {
           if (f != "")
             {
               im->load (m_cityset, f);
               im->instantiateImages ();
             }
         }
       update_cityset_panel ();
     });
}

void CitySetWindow::change_image (Glib::ustring msg, TarFileImage *im,
                                  sigc::slot<void(bool, bool, Glib::ustring)> after)
{
  Glib::ustring imgname = im->getName ();

  auto d = LwDialog::build<ImageEditorDialog> (this);
  d->setup (im);
  d->set_title (msg);
  d->signal_response ().connect
    ([this, d, im, after] (Gtk::ResponseType resp)
     {
       bool cleared = false;
       Glib::ustring newfile = "";
       if (resp == Gtk::ResponseType::ACCEPT &&
           d->is_changed () && d->get_filename () != "")
         {
           auto action = new CitySetUndoAction_AddImage (m_cityset);
           Glib::ustring newname = "";
           Glib::ustring err = "";
           bool success = d->install_file (m_cityset, im,
                                           d->get_filename (), err);
           if (success)
             {
               m_umgr->add (action);
               newfile = im->getName ();
               update ();
               delete d;
               return after (false, cleared, newfile);
             }
           else
             {
               delete action;
               Glib::ustring emsg = _("Couldn't add file!");
               auto dialog = LwDialog::alert (emsg, err);
               dialog->choose
                 (*d,
                  [this, dialog, cleared, newfile, after, d] (auto result)
                  {
                    dialog->choose_finish (result);
                    delete d;
                    return after (true, cleared, newfile);
                  });
               return;
             }
         }
       else if (resp == Gtk::ResponseType::REJECT)
         {
           Glib::ustring err = "";
           auto action = new CitySetUndoAction_ClearImage (m_cityset);
           auto name = im->getName ();
           if (d->uninstall_file (m_cityset, im, err))
             {
               m_umgr->add  (action);
               update ();
               cleared = true;
               newfile = "";
               delete d;
               m_cityset->uninstantiateSameNamedImages (name);
               return after (false, cleared, newfile);
             }
           else
             {
               delete action;
               newfile = d->get_filename ();
               Glib::ustring emsg = _("Couldn't remove file!");
               auto dialog = LwDialog::alert (emsg, err);
               dialog->choose
                 (*d,
                  [this, dialog, cleared, newfile, after, d] (auto result)
                  {
                    dialog->choose_finish (result);
                    delete d;
                    return after (true, cleared, newfile);
                  });
               return;
             }
         }
       else
         {
           delete d;
           return after (true, false, "");
         }
     });

}

void CitySetWindow::on_edit_cityset_info_activated ()
{
  auto d = LwDialog::build<CitySetInfoDialog> (this);
  d->setup (m_cityset);
  d->signal_response ().connect
    ([this, d] (Gtk::ResponseType resp)
     {
       if (d->is_changed () &&
           resp == Gtk::ResponseType::ACCEPT)
         {
           CitySetUndoAction_Properties *action =
             new CitySetUndoAction_Properties
             (m_cityset->getName (),
              m_cityset->getInfo (),
              m_cityset->getCopyright (),
              m_cityset->getLicense (),
              m_cityset->getTileSize ());
           m_umgr->add (action);
           m_cityset->setName (d->get_name ());
           m_cityset->setInfo (d->get_description ());
           m_cityset->setCopyright (d->get_copyright ());
           m_cityset->setLicense (d->get_license ());
           m_cityset->setTileSize (d->get_tile_size ());
           update ();
         }
       delete d;
     });
}

void CitySetWindow::on_save_as_activated()
{
  check_save_valid
    (false,
     [this] (bool valid)
     {
       if (valid)
         {
           save_current_cityset_file_as
             ([this] (bool saved)
              {
                (void) saved;
              });
         }
     });

}

void CitySetWindow::save_current_cityset_file_as (sigc::slot<void(bool)> after)
{
  LwDialog::save
    (*this, _("Choose a Name"), 
     File::sanify (m_cityset->getName ()), FileFilter::CITYSET,
     [this, after] (std::string path)
     {
       Glib::ustring old_filename = m_current_save_filename;
       guint32 old_id = m_cityset->getId ();
       m_cityset->setId(Citysetlist::getNextAvailableId (old_id));

       save_current_cityset_file
         (path,
          [this, after, path, old_filename, old_id] (bool saved)
          {
            if (saved == false)
              {
                m_current_save_filename = old_filename;
                m_cityset->setId (old_id);
              }
            else
              {
                m_cityset_modified = false;
                m_new_cityset_needs_saving = false;
                m_cityset->created (path);
                Glib::ustring dir =
                  File::add_slash_if_necessary (File::get_dirname (path));
                if (dir == File::get_cityset_dir () ||
                    dir == File::get_user_cityset_dir ())
                  {
                    //if we saved it to a standard place, update the list
                    Citysetlist::instance ()->add
                      (Cityset::copy (m_cityset), path);
                    m_cityset_saved.emit (m_cityset->getId ());
                  }
                update_cityset_panel ();
                update_window_title( );
              }
            after (saved);
          });
     });
}

void CitySetWindow::save_current_cityset_file (Glib::ustring filename, sigc::slot<void(bool)> after)
{
  m_current_save_filename = filename;
  if (m_current_save_filename.empty ())
    m_current_save_filename = m_cityset->getConfigurationFile (true);

  bool ok =
    m_cityset->save (m_current_save_filename, Cityset::file_extension);
  if (ok)
    {
      if (Citysetlist::instance ()->reload (m_cityset->getId ()))
        update_cityset_panel ();
      m_new_cityset_needs_saving = false;
      m_cityset_modified = false;
      update_window_title ();
      m_cityset_saved.emit (m_cityset->getId ());
      after (true);
    }
  else
    {
      Glib::ustring errmsg = Glib::strerror (errno);
      Glib::ustring msg = _("The city set could not be saved.");

      auto dialog = LwDialog::alert (msg, errmsg);
      dialog->choose
        (*this,
         [dialog, after] (auto result)
         {
           dialog->choose_finish (result);
           after (false);
           return;
         });
    }
  return;
}

void CitySetWindow::check_quit (sigc::slot<void(bool)> after)
{
  if (m_cityset_modified || m_new_cityset_needs_saving)
    {
      auto d = Gtk::AlertDialog::create (_("Save changes?"));
      d->set_detail (_("Save changes before closing?"));
      d->set_buttons
        ({_("Save"), _("Close without Saving"), _("Cancel")});
      d->set_cancel_button (2);
      d->set_default_button (0);
      d->set_modal (true);
      d->choose
        (*this,
         [this, d, after] (Glib::RefPtr<Gio::AsyncResult>& result)
         {
           int resp = d->choose_finish (result);
           switch (resp)
             {
             case 0: // save and exit
               check_save_valid
                 (true,
                  [this, after] (bool valid)
                  {
                    if (!valid)
                      return after (false);

                    if (m_cityset->getDirectory ().empty () == false)
                      {
                        save_current_cityset_file_as
                          ([this, after] (bool saved)
                           {
                             after (saved);
                           });
                      }
                    else
                      {
                        save_current_cityset_file
                          ("",
                           [this, after] (bool saved)
                           {
                             after (saved);
                           });
                      }
                  });
               break;

             case 1: // close and exit
               after (true);
               break;

             case 2: // cancel, don't exit
               after (false);
               break;

             default:
               after (false);
               break;

             }
         });
    }
  else
    after (true);
  return;
}

void CitySetWindow::setup_quit ()
{
  auto controller = Gtk::EventControllerKey::create ();
  controller->signal_key_pressed().connect
    ([this] (guint keyval, guint, Gdk::ModifierType)
     {
       if (keyval == GDK_KEY_Escape)
         {
           m_simple_actions["cityset.file.quit"]->activate ();
           return true;
         }
       return false;
     }, false);
  add_controller (controller);

  signal_close_request ().connect
    ([this] () -> bool
     {
       m_simple_actions["cityset.file.quit"]->activate ();
       return true;
     }, false);
}
