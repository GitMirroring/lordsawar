//  Copyright (C) 2010, 2011, 2014, 2015 Ben Asselstine
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

#include "editor-splash-window.h"
#include "builder-cache.h"

#include "ucompose.hpp"
#include "File.h"
#include "defs.h"
#include "ImageCache.h"
#include "armysetlist.h"
#include "tilesetlist.h"
#include "citysetlist.h"
#include "shieldsetlist.h"

EditorSplashWindow::EditorSplashWindow()
{
    Glib::RefPtr<Gtk::Builder> xml = 
      BuilderCache::editor_get("editor-splash-window.ui");

    xml->get_widget("window", window);
    xml->get_widget("progressbar", progressbar);
}

int EditorSplashWindow::run()
{
  bool broken = false;
  window->show_all();
  progressbar->property_fraction() = 0.0;
  progressbar->property_text() = _("Loading Army Sets");
  while (g_main_context_iteration(NULL, FALSE));
  ImageCache::getInstance();
  Armysetlist::getInstance()->instantiateImages(broken);
  if (broken)
    return -1;
  progressbar->property_fraction() = 0.25;
  progressbar->property_text() = _("Loading City Sets");
  while (g_main_context_iteration(NULL, FALSE));
  Citysetlist::getInstance()->instantiateImages(broken);
  if (broken)
    return -1;
  progressbar->property_fraction() = 0.50;
  progressbar->property_text() = _("Loading Tile Sets");
  while (g_main_context_iteration(NULL, FALSE));
  Tilesetlist::getInstance()->instantiateImages(broken);
  if (broken)
    return -1;
  progressbar->property_fraction() = 0.75;
  progressbar->property_text() = _("Loading Shield Sets");
  while (g_main_context_iteration(NULL, FALSE));
  Shieldsetlist::getInstance()->instantiateImages(broken);
  if (broken)
    return -1;
  progressbar->property_fraction() = 1.00;
  while (g_main_context_iteration(NULL, FALSE));
  return 0;
}

void EditorSplashWindow::hide()
{
  window->hide();
}
