//  Copyright (C) 2007 Ole Laursen
//  Copyright (C) 2007, 2008, 2009, 2012, 2014, 2015, 2020, 2021 Ben Asselstine
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
#include <list>
#include <iostream>
#include <sigc++/functors/mem_fun.h>
#include <gtkmm.h>

#include "load-scenario-dialog.h"
#include "ucompose.hpp"
#include "defs.h"
#include "File.h"
#include "scenario-list.h"
#include "timed-message-dialog.h"

#define method(x) sigc::mem_fun(*this, &LoadScenarioDialog::x)

LoadScenarioDialog::LoadScenarioDialog(Gtk::Window &parent)
 : LwDialog(parent, "load-scenario-dialog.ui"),
    name_column("", name_renderer)
{
  xml->get_widget("description_textview", description_textview);
  xml->get_widget("load_button", load_button);
  xml->get_widget("num_players_label", num_players_label);
  xml->get_widget("num_cities_label", num_cities_label);
  xml->get_widget("dialog-vbox", dialog_vbox);

  scenarios_list = Gtk::ListStore::create(scenarios_columns);
  xml->get_widget("treeview", scenarios_treeview);
  scenarios_treeview->set_model(scenarios_list);

  xml->get_widget("add_scenario_button", add_scenario_button);
  add_scenario_button->signal_clicked().connect
    (method(on_add_scenario_clicked));
  xml->get_widget("remove_scenario_button", remove_scenario_button);
  remove_scenario_button->signal_clicked().connect
    (method(on_remove_scenario_clicked));

  scenarios_treeview->get_selection()->signal_changed()
    .connect(method(on_selection_changed));
  scenarios_treeview->signal_row_activated().connect(sigc::hide(sigc::hide(method(on_scenario_activated))));
  // add the scenarios
  Gtk::TreeIter i = scenarios_list->append();
  (*i)[scenarios_columns.filename] = "random.map";
  (*i)[scenarios_columns.details] = NULL;

  name_renderer.property_editable() = false;
  name_column.set_cell_data_func(name_renderer, method(cell_data_name));
  scenarios_treeview->append_column(name_column);

  for (auto j : *ScenarioList::getInstance ())
    add_scenario (j, false);

  Gtk::TreeModel::Row row;
  row = scenarios_treeview->get_model()->children()[0];
  if(row)
    scenarios_treeview->get_selection()->select(row);
}

void LoadScenarioDialog::cell_data_name(Gtk::CellRenderer *renderer,
                                        const Gtk::TreeIter& i)
{
  ScenarioDetails *details = (*i)[scenarios_columns.details];

  if (details == NULL)
    dynamic_cast<Gtk::CellRendererText*>(renderer)->property_text () =
      _("Random Scenario");
  else
    {
      if (details->getName () == "")
        dynamic_cast<Gtk::CellRendererText*>(renderer)->property_markup () =
          "<i>" + _("(untitled)") + "</i>";
      else
        dynamic_cast<Gtk::CellRendererText*>(renderer)->property_text() =
          details->getName();
    }
}

void LoadScenarioDialog::run()
{
  static int width = -1;
  static int height = -1;

  if (width != -1 && height != -1)
    dialog->set_default_size(width, height);

  dialog->show();
  int response = dialog->run();
  if (response != Gtk::RESPONSE_ACCEPT)
    selected_filename = "";

  if (selected_filename.empty () == false &&
      selected_filename != "random.map")
    {
      bool valid = false;

      Gtk::TreeIter i = scenarios_treeview->get_selection()->get_selected();
      ScenarioDetails *d = (*i)[scenarios_columns.details];
      if (d->getNumberOfCities () > 1 && d->getNumberOfPlayers() > 1)
        {
          setup_progress_bar ();
          bool broken = false;

          GameScenario::load_tick.connect (method (tick_progress));
          GameScenario::load_finish.connect (method (finish_progress));
          GameScenario *g = new GameScenario (selected_filename, broken);
          if (g)
            {
              if (!broken)
                {
                  std::list<Glib::ustring> errors;
                  std::list<Glib::ustring> warnings;
                  valid = g->validate (errors, warnings);
                }
              delete g;
            }
        }
      if (!valid)
        {
          selected_filename = "";
          TimedMessageDialog di(*dialog, _("The scenario isn't valid."), 0);
          di.run_and_hide();
        }
    }
  dialog->get_size(width, height);
}

Glib::ustring LoadScenarioDialog::get_scenario_filename() 
{
  return selected_filename;
}

void LoadScenarioDialog::add_scenario(ScenarioDetails *d, bool sel)
{
  Gtk::TreeIter i = scenarios_list->append();
  (*i)[scenarios_columns.filename] = d->getFilename ();
  (*i)[scenarios_columns.details] = d;
  if (sel)
    {
      scenarios_treeview->scroll_to_row
        (scenarios_treeview->get_model ()->get_path (i));
      scenarios_treeview->get_selection ()->select (i);
    }

}

void LoadScenarioDialog::on_selection_changed()
{
  Gtk::TreeIter i = scenarios_treeview->get_selection()->get_selected();

  if (i)
    {
      Glib::ustring filename = (*i)[scenarios_columns.filename];
      ScenarioDetails *details = (*i)[scenarios_columns.details];
      if (filename == "random.map")
	{
	  load_button->set_sensitive(true);
	  description_textview->get_buffer()->set_text(_("Play a new scenario with a random map.  You get to decide the number of players, and number of cities on the map.  You can also control the amount of the map that is covered in forest, water, swamps and mountains."));
	  num_players_label->set_markup (String::ucompose("<b>--</b>", ""));
	  num_cities_label->set_markup(String::ucompose("<b>--</b>", ""));
	  remove_scenario_button->set_sensitive(false);
	  load_button->set_sensitive(true);
	  selected_filename = filename;
	  return;
	}

      selected_filename = filename;
      remove_scenario_button->set_sensitive(true);
      load_button->set_sensitive(true);
      num_players_label->set_markup 
	("<b>" + String::ucompose("%1", details->getNumberOfPlayers () - 1) + "</b>");
      num_cities_label->set_markup 
	("<b>" + String::ucompose("%1", details->getNumberOfCities ()) + "</b>");
      description_textview->get_buffer()->set_text(details->getDescription ());
    }
  else
    load_button->set_sensitive(false);
}

void LoadScenarioDialog::on_add_scenario_clicked() 
{
  // go get a .map file from somewhere.
  Gtk::FileChooserDialog *load_map_filechooser = new 
    Gtk::FileChooserDialog(_("Select a scenario file to add to the library"), 
			   Gtk::FILE_CHOOSER_ACTION_OPEN);
  load_map_filechooser->add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
  load_map_filechooser->add_button(Gtk::Stock::OPEN, Gtk::RESPONSE_ACCEPT);
  load_map_filechooser->set_default_response(Gtk::RESPONSE_ACCEPT);
  
  Glib::RefPtr<Gtk::FileFilter> map_filter = Gtk::FileFilter::create();
  map_filter->add_pattern("*.map");
  map_filter->set_name(_("LordsAWar Maps (*.map)"));
  load_map_filechooser->set_current_folder(Glib::get_home_dir ());
  load_map_filechooser->add_filter(map_filter);
  int res = load_map_filechooser->run();
  load_map_filechooser->hide();
  if (res == Gtk::RESPONSE_ACCEPT) 
    {
      Glib::ustring filename = load_map_filechooser->get_filename();
      Glib::ustring mapname = Glib::path_get_basename (filename);
      // copy it into our ~/.lordsawar/ dir.
      File::copy (filename, File::getUserMapFile(mapname));
      // add it to the list
      if (ScenarioList::getInstance ()->add_file
          (File::getUserMapFile (mapname)))
        add_scenario (ScenarioList::getInstance()->back (), true);
    }
  delete load_map_filechooser;
}

void LoadScenarioDialog::on_remove_scenario_clicked() 
{
  //remove THIS scenario.
  //only highlight this button when we have something selected.

  //erase the selected row from the treeview
  //remove the scenario from the list of scenarios 
  //delete the file, if we can.
  Gtk::TreeIter i = scenarios_treeview->get_selection()->get_selected();
  if (i)
    {
      Glib::ustring filename = (*i)[scenarios_columns.filename];
      if (filename == "random.map")
        return;
      if (ScenarioList::getInstance ()->remove_file (filename))
        {
          scenarios_list->erase(i);
          description_textview->get_buffer()->set_text("");
          num_players_label->set_text ("");
          num_cities_label->set_text ("");
        }
      else
        {
          Glib::ustring errmsg = Glib::strerror(errno);
          Glib::ustring msg = _("Error!  Scenario could not be removed.");
          msg += "\n" + filename + "\n" + errmsg;
          TimedMessageDialog d(*dialog, msg, 0);
          d.run_and_hide();
        }
    }
  return;
}
  
void LoadScenarioDialog::on_scenario_activated()
{
  load_button->activate();
}

void LoadScenarioDialog::setup_progress_bar ()
{
  progress_treeview = Gtk::manage (new Gtk::TreeView ());
  progress_treeview->property_headers_visible () = false;
  progress_liststore = Gtk::ListStore::create(progress_columns);
  progress_treeview->set_model (progress_liststore);
  progressrow = *(progress_liststore->append());
  auto cell = Gtk::manage (new Gtk::CellRendererProgress());
  cell->property_text () = "";
  int cols_count = progress_treeview->append_column ("progress", *cell);
  auto pColumn = progress_treeview->get_column(cols_count -1);
  if (pColumn)
    pColumn->add_attribute(cell->property_value (), progress_columns.perc);

  dialog_vbox->pack_end (*progress_treeview, true, true);
  dialog_vbox->show_all ();
  while (g_main_context_iteration(NULL, FALSE)); //doEvents
  dialog_vbox->set_sensitive(false);
}

void LoadScenarioDialog::tick_progress ()
{
  if (!progress_treeview)
    return;
  if (progressrow[progress_columns.perc] < 88)
    {
      progressrow[progress_columns.perc] =
        progressrow[progress_columns.perc] + 13;
      while (g_main_context_iteration(NULL, FALSE)); //doEvents
    }
}

void LoadScenarioDialog::finish_progress ()
{
  if (!progress_treeview)
    return;
  //finish off the progressbar
  while (progressrow[progress_columns.perc] < 100)
    {
      progressrow[progress_columns.perc] =
        progressrow[progress_columns.perc] + 1;
      while (g_main_context_iteration(NULL, FALSE)); //doEvents
      Glib::usleep (10000);
    }
  progressrow[progress_columns.perc] = 100;
  progress_treeview->queue_draw ();
  Glib::usleep (20000);
  while (g_main_context_iteration(NULL, FALSE)); //doEvents
  dialog->hide ();
}
