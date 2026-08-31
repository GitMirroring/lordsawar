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

#include <gtkmm.h>
#include <iostream>
#include "defs.h"
#ifndef LW_DIALOG_H
#define LW_DIALOG_H
#include "file-filter.h"
class LwDialog
{
public:
    template <typename T>
    static T* build (Gtk::Window *parent)
      {
        try
          {
            Glib::RefPtr<Gtk::Builder> builder =
              Gtk::Builder::create_from_resource
              (std::string (RESOURCE) + T::get_resource_name ());
            T *dialog = Gtk::Builder::get_widget_derived<T>(builder, "dialog");
            if (dialog->get_title () == "") //odd workaround here
              dialog->set_title ("");
            dialog->set_modal (true);
            dialog->set_transient_for (*parent);
            dialog->present ();
            return dialog;
          }
        catch (Glib::Error &ex)
          {
            std::cerr << T::get_resource_name () << ": " << ex.what () <<
              std::endl;
          }
        return NULL;
      }

    static Glib::RefPtr<Gtk::AlertDialog> alert (Glib::ustring msg, Glib::ustring detail = "")
      {
        auto dialog = Gtk::AlertDialog::create (msg);
        dialog->set_buttons ({_("Close")});
        if (detail != "")
          dialog->set_detail (detail);
        dialog->set_cancel_button (0);
        dialog->set_default_button (0);
        dialog->set_modal (true);
        return dialog;
      }

    static Glib::RefPtr<Gtk::AlertDialog> alert_yn (Glib::ustring msg, Glib::ustring detail = "")
      {
        auto dialog = Gtk::AlertDialog::create (msg);
        if (detail != "")
          dialog->set_detail (detail);
        dialog->set_buttons ({_("No"), _("Yes")});
        dialog->set_cancel_button (0);
        dialog->set_default_button (1);
        dialog->set_modal (true);
        return dialog;
      }

    static void open (Gtk::Window &parent, Glib::ustring title, FileFilter::Extension ext, sigc::slot<void(std::string)> after)
      {
        auto d = Gtk::FileDialog::create ();
        d->set_title (title);

        std::string folder = "";
        switch (ext)
          {
          case FileFilter::PNG:
            FileFilter (_("PNG Files"), {".png"}).add (d);
            break;

          case FileFilter::IMAGE:
            FileFilter (_("Image Files"), {".png", ".svg"}).add (d);
            break;

          case FileFilter::SOUND:
            FileFilter (_("Audio Files"), {".ogg"}).add (d);
            break;

          case FileFilter::ARMYSET:
            FileFilter (_("Army Set Files"), {ARMYSET_EXT}).add (d);
            folder = File::get_user_armyset_dir ();
            break;

          case FileFilter::CITYSET:
            FileFilter (_("City Set Files"), {CITYSET_EXT}).add (d);
            folder = File::get_user_cityset_dir ();
            break;

          case FileFilter::SHIELDSET:
            FileFilter (_("Shield Set Files"), {SHIELDSET_EXT}).add (d);
            folder = File::get_user_shieldset_dir ();
            break;

          case FileFilter::TILESET:
            FileFilter (_("Tile Set Files"), {TILESET_EXT}).add (d);
            folder = File::get_user_tileset_dir ();
            break;

          case FileFilter::SAVED_GAME:
            FileFilter (_("Saved Game Files"), {SAVE_EXT}).add (d);
            folder = File::getSavePath ();
            break;

          case FileFilter::SCENARIO:
            FileFilter (_("Scenario Files"), {MAP_EXT}).add (d);
            folder = File::get_user_map_dir ();
            break;

          }

        if (folder != "")
          d->set_initial_folder (Gio::File::create_for_path (folder));

        d->open
          (parent,
           [d, after](const Glib::RefPtr<Gio::AsyncResult>& result)
           {
             try
               {
                 auto file = d->open_finish (result);
                 if (file)
                   after (file->get_path ());
               }
             catch (const Gtk::DialogError& e)
               {
                 if (e.code () == Gtk::DialogError::Code::DISMISSED)
                   return;
               }
             catch (const Glib::Error& e)
               {
                 std::cerr << "Error: " << e.what () << '\n';
               }
           });
      }

    static void save (Gtk::Window &parent, Glib::ustring title, Glib::ustring filename, FileFilter::Extension ext, sigc::slot<void(std::string)> after)
      {
        auto d = Gtk::FileDialog::create ();
        d->set_title (title);
        if (filename != "")
          d->set_initial_name (filename + FileFilter::get_extension (ext));

        std::string folder = "";
        switch (ext)
          {
          case FileFilter::PNG:
            FileFilter (_("PNG Files"), {".png"}).add (d);
            break;

          case FileFilter::IMAGE:
            FileFilter (_("Image Files"), {".png", ".svg"}).add (d);
            break;

          case FileFilter::SOUND:
            FileFilter (_("Audio Files"), {".ogg"}).add (d);
            break;

          case FileFilter::ARMYSET:
            FileFilter (_("Army Set Files"), {ARMYSET_EXT}).add (d);
            folder = File::get_user_armyset_dir ();
            break;

          case FileFilter::CITYSET:
            FileFilter (_("City Set Files"), {CITYSET_EXT}).add (d);
            folder = File::get_user_cityset_dir ();
            break;

          case FileFilter::SHIELDSET:
            FileFilter (_("Shield Set Files"), {SHIELDSET_EXT}).add (d);
            folder = File::get_user_shieldset_dir ();
            break;

          case FileFilter::TILESET:
            FileFilter (_("Tile Set Files"), {TILESET_EXT}).add (d);
            folder = File::get_user_tileset_dir ();
            break;

          case FileFilter::SAVED_GAME:
            FileFilter (_("Saved Game Files"), {SAVE_EXT}).add (d);
            folder = File::getSavePath ();
            break;

          case FileFilter::SCENARIO:
            FileFilter (_("Scenario Files"), {MAP_EXT}).add (d);
            folder = File::get_user_map_dir ();
            break;

          }

        if (folder != "")
          d->set_initial_folder (Gio::File::create_for_path (folder));

        d->save
          (parent,
           [d, after](const Glib::RefPtr<Gio::AsyncResult>& result)
           {
             try
               {
                 auto file = d->save_finish (result);
                 if (file)
                   after (file->get_path ());
               }
             catch (const Gtk::DialogError& e)
               {
                 if (e.code () == Gtk::DialogError::Code::DISMISSED)
                   return;
               }
             catch (const Glib::Error& e)
               {
                 std::cerr << "Error: " << e.what () << '\n';
               }
           });
      }
};
#endif
