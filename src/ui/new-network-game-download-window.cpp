//  Copyright (C) 2008, 2014, 2015 Ben Asselstine
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
#include "new-network-game-download-window.h"
#include "defs.h"
#include "builder-cache.h"

NewNetworkGameDownloadWindow::NewNetworkGameDownloadWindow(Glib::ustring title,
                                                           Gtk::Window *w)
{
  Glib::RefPtr<Gtk::Builder> xml = 
    BuilderCache::get("new-network-game-download-window.ui");

  xml->get_widget("window", window);
  xml->get_widget("progress_treeview", progress_treeview);
  if (w)
    window->set_transient_for(*w);
  if (title.empty () == false)
    window->set_title (title);
}

int NewNetworkGameDownloadWindow::run()
{
  window->set_modal ();
  window->show_all();
  progress_treeview->property_headers_visible () = false;
  progress_liststore = Gtk::ListStore::create(progress_columns);
  progress_treeview->set_model (progress_liststore);
  row = *(progress_liststore->append());
  pbar = Gtk::manage (new Gtk::CellRendererProgress());
  pbar->property_text () = window->get_title ();
  int cols_count = progress_treeview->append_column ("progress", *pbar);

  auto pColumn = progress_treeview->get_column(cols_count -1);
  if (pColumn)
    pColumn->add_attribute(pbar->property_value (), progress_columns.perc);
  return 0;
}

void NewNetworkGameDownloadWindow::hide()
{
  window->hide();
}

void NewNetworkGameDownloadWindow::pulse (int amt, int total)
{
  double percent = (double)amt / (double)total * 100.0;
  int perc = int(percent);
  if (perc >= 100)
    perc = 99;
  row[progress_columns.perc] = perc;

  if (row[progress_columns.perc] >= 100)
    row[progress_columns.perc] = 0;
  progress_treeview->queue_draw ();
  while (g_main_context_iteration(NULL, FALSE)); //doEvents
}
