//  Copyright (C) 2026 Ben Asselstine
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

#pragma once
#ifndef FILE_FILTER_H
#define FILE_FILTER_H

#include <gtkmm.h>
#include "file.h"

class FileFilter
{
public:

    enum Extension
      {
        IMAGE,
        PNG,
        SOUND,

        ARMYSET,
        CITYSET,
        SHIELDSET,
        TILESET,

        SAVED_GAME,
        SCENARIO
      };

    static Glib::ustring get_extension (Extension ext)
      {
        switch (ext)
          {
          case IMAGE:
          case PNG:
          case SOUND:
            break;

          case ARMYSET:
            return ARMYSET_EXT;

          case CITYSET:
            return CITYSET_EXT;

          case SHIELDSET:
            return SHIELDSET_EXT;

          case TILESET:
            return TILESET_EXT;

          case SAVED_GAME:
            return SAVE_EXT;

          case SCENARIO:
            return MAP_EXT;
          }
        return "";
      }

    bool has_invalid_ext (Glib::ustring filename)
      {
        if (filename == "")
          return true;
        for (auto ext : m_exts)
          if (File::nameEndsWith (filename, ext))
            return false;
        return true;
      }

    void add (std::shared_ptr<Gtk::FileDialog> d)
      {
        auto filters = Gio::ListStore<Gtk::FileFilter>::create ();

        Glib::RefPtr<Gtk::FileFilter> filter = Gtk::FileFilter::create ();
        Glib::ustring s;
        bool first = true;
        for (auto ext : m_exts)
          {
            if (!first)
              s += ", *" + ext;
            else
              s += "*" + ext;
            if (first)
              first = false;
          }
        filter->set_name (m_name + " (" + s + ")");
        for (auto ext : m_exts)
          filter->add_pattern ("*" + ext);
        filters->append (filter);

        Glib::RefPtr<Gtk::FileFilter> all_filter = Gtk::FileFilter::create ();
        all_filter->set_name (_("All Files"));
        all_filter->add_pattern ("*.*");
        filters->append (all_filter);

        d->set_filters (filters);
      }

    FileFilter (Glib::ustring name, std::vector<std::string> exts)
      : m_name (name), m_exts (exts)
      {
      }

    FileFilter (std::vector<std::string> exts)
      : m_name (""), m_exts (exts)
      {
      }

    ~FileFilter ()
      {
      }

    Glib::ustring m_name;
    std::vector<std::string> m_exts;
};

#endif
