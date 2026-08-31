//  Copyright (C) 2007 Ole Laursen
//  Copyright (C) 2007, 2008, 2009, 2014, 2015, 2020, 2021 Ben Asselstine
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
#ifndef CHOOSE_SCENARIO_DIALOG_H
#define CHOOSE_SCENARIO_DIALOG_H

#include "lw-dialog.h"
#include "lw-dialog-base.h"
#include "scenario-details.h"
#include "scenario-list.h"
#include "new-random-map-dialog.h"
#include "startup.h"
#include "lw-column.h"
class ScenarioRow: public Glib::Object
{
public:
    ScenarioDetails *m_details;

    static Glib::RefPtr<ScenarioRow> create (ScenarioDetails *d)
      {
        return
          Glib::make_refptr_for_instance<ScenarioRow> (new ScenarioRow (d));
      }

protected:
    ScenarioRow (ScenarioDetails *d)
      : m_details (d)
      {
      }
};

class ChooseScenarioDialog: public LwDialogBase
{
public:
    static std::string get_resource_name ()
      {
        return "choose-scenario.ui";
      }

    ChooseScenarioDialog (BaseObjectType* o,
                         const Glib::RefPtr<Gtk::Builder>& xml)
      : LwDialogBase (o, xml)
      {
        m_num_players_label = load <Gtk::Label> ("num_players_label");
        m_num_cities_label = load <Gtk::Label> ("num_cities_label");
        m_desc_textview = load <Gtk::TextView> ("description_textview");
        m_load_button = load <Gtk::Button> ("load_button");
        set_response (m_load_button, Gtk::ResponseType::ACCEPT);

        auto cancel_button = load <Gtk::Button> ("cancel_button");
        set_response (cancel_button, Gtk::ResponseType::CANCEL);

        m_add_button = load <Gtk::Button> ("add_scenario_button");
        m_remove_button = load <Gtk::Button> ("remove_scenario_button");

        m_treeview = load <Gtk::ColumnView> ("treeview");
      }

    ~ChooseScenarioDialog ()
      {
        m_coming_up.disconnect ();
      }

    void setup ()
      {
        m_coming_up = 
          Startup::instance ()->signal_game_window_coming_up ().connect
          ([this] ()
           {
             hide ();
             delete this;
           });

        m_store = Gio::ListStore<ScenarioRow>::create ();
        m_selection_model = Gtk::SingleSelection::create (m_store);
        m_treeview->set_model (m_selection_model);

        for (auto j : *ScenarioList::instance ())
          m_store->append (ScenarioRow::create (j));
        m_store->append (ScenarioRow::create (NULL));
        setup_column ();

        m_selection_model->signal_selection_changed ().connect
          ([this] (const guint &, const guint &)
           {
             fill_details ();
           });

        m_treeview->signal_activate ().connect
          ([this] (guint position)
           {
             (void) position;
             m_load_button->activate ();
           });

        m_add_button->signal_clicked ().connect
          ([this] ()
           {
             auto d = Gtk::FileDialog::create ();
             d->set_title (_("Select a scenario file to add to the library"));

             auto filter = Gtk::FileFilter::create ();
             filter->set_name
               (String::ucompose (_("LordsAWar Scenarios (*%1)"), MAP_EXT));
             filter->add_pattern ("*" + MAP_EXT);
             auto filters = Gio::ListStore<Gtk::FileFilter>::create ();
             filters->append (filter);
             d->set_filters (filters);

             d->open
               (*this,
                [this, d](const Glib::RefPtr<Gio::AsyncResult>& result)
                {
                   try 
                     {
                       auto file = d->open_finish (result);
                       if (file)
                         {
                           std::string f = file->get_path ();
                           Glib::ustring mapname = Glib::path_get_basename (f);
                           File::copy (f, File::getUserMapFile (mapname));
                           auto sl = ScenarioList::instance ();
                           if (sl->add_file (File::getUserMapFile (mapname)))
                             m_store->append
                               (ScenarioRow::create (sl->back ()));
                         }
                     }
                   catch (const Gtk::DialogError& e)
                     {
                       // User pressed Escape or closed dialog
                       if (e.code() == Gtk::DialogError::Code::DISMISSED)
                         {
                           return;
                         }
                     }
                   catch (const Glib::Error& e)
                     {
                       std::cerr << "Error: " << e.what() << '\n';
                     }
                });
           });

        m_remove_button->signal_clicked ().connect
          ([this] ()
           {
             auto details = get_selected_scenario ();
             if (!details)
               return;
             std::string name = details->get_name ();
             auto dialog =
               Gtk::make_managed<Gtk::MessageDialog>
               (*this, String::ucompose (_("Delete %1?\n"
                                           "This action cannot be undone."),
                                         name),
                         false, Gtk::MessageType::QUESTION,
                Gtk::ButtonsType::NONE, true);

             dialog->add_button (_("Cancel"), Gtk::ResponseType::CANCEL);
             auto delete_btn = dialog->add_button (_("Delete"),
                                                   Gtk::ResponseType::ACCEPT);
             delete_btn->add_css_class ("destructive-action");

             dialog->signal_response ().connect
               ([this, dialog, details] (int response)
                {
                  if (response == Gtk::ResponseType::ACCEPT)
                    {
                      guint position = m_selection_model->get_selected ();
                      if (position != GTK_INVALID_LIST_POSITION)
                        m_store->remove (position);
                      ScenarioList::instance ()->remove_file
                        (details->get_filename ());
             
                      ScenarioDetails *deets =
                        get_selected_scenario ();
                      m_remove_button->set_sensitive (deets != NULL);
                      position = m_selection_model->get_selected ();
                      m_load_button->set_sensitive
                        (position != GTK_INVALID_LIST_POSITION);
                    }
                  dialog->hide ();
                });
             dialog->present ();
           });

        signal_response ().connect
          ([this] (Gtk::ResponseType response)
           {
             switch (response)
               {
               case Gtk::ResponseType::ACCEPT:
                 m_selected_scenario.emit (get_selected_scenario ());
                 break;

               default:
                 hide ();
                 break;
               }
           });

        fill_details ();

        m_treeview->grab_focus ();
      }

    sigc::signal<void(ScenarioDetails*)> signal_scenario_selected ()
      {
        return m_selected_scenario;
      }
private:
    Gtk::ColumnView *m_treeview;
    Glib::RefPtr<Gtk::SingleSelection> m_selection_model;
    Glib::RefPtr<Gio::ListStore<ScenarioRow>> m_store;
    Gtk::Label *m_num_players_label;
    Gtk::Label *m_num_cities_label;
    Gtk::TextView *m_desc_textview;
    Gtk::Button *m_load_button;
    Gtk::Button *m_add_button;
    Gtk::Button *m_remove_button;
    sigc::signal<void(ScenarioDetails*)> m_selected_scenario;
    sigc::connection m_coming_up;

    void setup_column ()
      {
        LwColumn::setup_text_column<ScenarioRow>
          (m_treeview, "name_label", true, Gtk::Justification::LEFT,
           _("Scenarios"),
           [] (const auto& row)
           {
             if (row->m_details)
               return row->m_details->get_name ();
             else
               return _("Random Scenario");
           });
      }

    ScenarioDetails * get_selected_scenario ()
      {
        auto single_selection =
          std::dynamic_pointer_cast<Gtk::SingleSelection> (m_selection_model);

        auto item = single_selection->get_selected_item ();

        if (item)
          {
            auto row = std::dynamic_pointer_cast<ScenarioRow> (item);
            return row->m_details;
          }
        return NULL;
      }

    bool can_remove_file (std::string path)
      {
        auto file = Gio::File::create_for_path (path);

        if (!file)
          return false;
        try
          {
            auto info = file->query_info ("access::can-delete",
                                          Gio::FileQueryInfoFlags::NONE);

            if (info->has_attribute ("access::can-delete"))
              return info->get_attribute_boolean ("access::can-delete");

            return false;
          }
        catch (const Glib::Error& e)
          {
            return false;
          }
      }

    void fill_details ()
      {
        guint position = m_selection_model->get_selected ();
        m_load_button->set_sensitive
          (position != GTK_INVALID_LIST_POSITION);

        ScenarioDetails *deets = get_selected_scenario ();
        if (!deets)
          m_remove_button->set_sensitive (false);
        else
          m_remove_button->set_sensitive
            (can_remove_file (deets->get_filename ()));
        if (deets)
          {
            m_num_players_label->set_text 
              (String::ucompose ("%1", deets->get_number_of_players () - 1));
            m_num_cities_label->set_text 
              (String::ucompose ("%1", deets->get_number_of_cities ()));
            Glib::ustring desc = deets->get_description ();
            m_desc_textview->get_buffer ()->set_text (desc);
          }
        else
          {
            m_num_players_label->set_text ("--");
            m_num_cities_label->set_text ("--");
            m_desc_textview->get_buffer ()->set_text
              (_("Play a new scenario with a random map.  "
                 "You get to decide the number of players, "
                 "and number of cities on the map.  "
                 "You can also control the amount of the map "
                 "that is covered in forest, water, swamps and "
                 "mountains."));
          }
      }
};
#endif
