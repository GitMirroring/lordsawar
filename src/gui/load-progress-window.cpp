//  Copyright (C) 2020 Ben Asselstine
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

#include <sigc++/functors/mem_fun.h>

#include "load-progress-window.h"
#include "builder-cache.h"

LoadProgressWindow::LoadProgressWindow(Gtk::Window *w)
{
  Glib::RefPtr<Gtk::Builder> xml = 
    BuilderCache::get("load-progress-window.ui");

  xml->get_widget("window", window);
  xml->get_widget("progress_treeview", progress_treeview);
  if (w)
    window->set_transient_for(*w);
}

int LoadProgressWindow::run()
{
  window->set_modal ();
  window->show_all();
  progress_treeview->property_headers_visible () = false;
  progress_liststore = Gtk::ListStore::create(progress_columns);
  progress_treeview->set_model (progress_liststore);
  row = *(progress_liststore->append());
  auto cell = Gtk::manage (new Gtk::CellRendererProgress());
  cell->property_text () = "";
  int cols_count = progress_treeview->append_column ("progress", *cell);
  auto pColumn = progress_treeview->get_column(cols_count -1);
  if (pColumn)
    pColumn->add_attribute(cell->property_value (), progress_columns.perc);
  return 0;
}

void LoadProgressWindow::hide()
{
  window->hide();
}

void LoadProgressWindow::tick_progress ()
{
  if (!progress_treeview)
    return;
  if (row[progress_columns.perc] < 98)
    {
      row[progress_columns.perc] = row[progress_columns.perc] + 3;
      while (g_main_context_iteration(NULL, FALSE)); //doEvents
    }
      while (g_main_context_iteration(NULL, FALSE)); //doEvents
}

void LoadProgressWindow::finish_progress ()
{
  if (!progress_treeview)
    return;
  //finish off the progressbar
  while (row[progress_columns.perc] < 100)
    {
      row[progress_columns.perc] = row[progress_columns.perc] + 1;
      while (g_main_context_iteration(NULL, FALSE)); //doEvents
      Glib::usleep (10000);
    }
  row[progress_columns.perc] = 100;
  progress_treeview->queue_draw ();
  Glib::usleep (20000);
  while (g_main_context_iteration(NULL, FALSE)); //doEvents
  hide ();
}
