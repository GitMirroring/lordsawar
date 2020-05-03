//  Copyright (C) 2009, 2010, 2012, 2014, 2015, 2020 Ben Asselstine
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

#include <gtkmm.h>
#include <sigc++/functors/mem_fun.h>

#include "tileset-flag-editor-dialog.h"

#include "tileset-window.h"
#include "ucompose.hpp"
#include "defs.h"
#include "File.h"
#include "shieldsetlist.h"
#include "ImageCache.h"
#include "past-chooser.h"
#include "font-size.h"
#include "image-file-filter.h"
#include "timed-message-dialog.h"
#include "TarFileMaskedImage.h"

#define method(x) sigc::mem_fun(*this, &TilesetFlagEditorDialog::x)

TilesetFlagEditorDialog::TilesetFlagEditorDialog(Gtk::Window &parent, Tileset *tileset)
 : LwEditorDialog(parent, "tileset-flag-editor-dialog.ui")
{
  d_changed = false;
  d_tileset = tileset;

  Gtk::Box *box;
  xml->get_widget("shieldset_box", box);
  setup_shield_theme_combobox(box);
  xml->get_widget("preview_table", preview_table);

  xml->get_widget("flag_imagebutton", flag_imagebutton);
  flag_imagebutton->signal_clicked().connect
    (method(on_flag_imagebutton_clicked));

  d_flags = new TarFileMaskedImage (*d_tileset->getFlags ());
  update_flag_panel();
}

bool TilesetFlagEditorDialog::run()
{
  dialog->show_all();
  dialog->run();
  dialog->hide ();
  return d_changed;
}

void TilesetFlagEditorDialog::setup_shield_theme_combobox(Gtk::Box *box)
{
  // fill in shield themes combobox
  shield_theme_combobox = manage(new Gtk::ComboBoxText);

  Shieldsetlist *sl = Shieldsetlist::getInstance();
  std::list<Glib::ustring> shield_themes = sl->getValidNames();
  int counter = 0;
  int default_id = 0;
  for (std::list<Glib::ustring>::iterator i = shield_themes.begin(),
       end = shield_themes.end(); i != end; ++i)
    {
      if (*i == _("Default"))
	default_id = counter;
      shield_theme_combobox->append(Glib::filename_to_utf8(*i));
      counter++;
    }

  shield_theme_combobox->set_active(default_id);
  shield_theme_combobox->signal_changed().connect (method(on_shieldset_changed));

  box->set_center_widget (*shield_theme_combobox);
}

void TilesetFlagEditorDialog::on_shieldset_changed()
{
  show_preview_flags();
}

bool TilesetFlagEditorDialog::on_image_chosen(Gtk::FileChooserDialog *d)
{
  bool broken = false;
  if (PixMask::checkFormat (d->get_filename ()))
    {
      Glib::ustring imgname = d_tileset->getFlags()->getName();
      Glib::ustring newname = "";
      bool success = false;
      if (imgname.empty() == true)
        success =
          d_tileset->addFileInCfgFile(d->get_filename(), newname);
      else
        success =
          d_tileset->replaceFileInCfgFile(imgname, d->get_filename(), newname);
      if (success)
        {
          d_tileset->getFlags ()->setName (newname);
          d_tileset->instantiateFlagImages();
          d_changed = true;
          update_flag_panel();
        }
      else
        {
          Glib::ustring errmsg = Glib::strerror(errno);
          TimedMessageDialog
            td(*d, String::ucompose(_("Couldn't add %1 to :\n%2\n%3"),
                                    d->get_filename (),
                                    d_tileset->getConfigurationFile(),
                                    errmsg), 0);
          td.run_and_hide ();
          broken = true;
        }
    }
  else
    {
      TimedMessageDialog
        td(*d, String::ucompose(_("Couldn't make sense of the image:\n%1"),
                                d->get_filename ()), 0);
      td.run_and_hide ();
      broken = true;
    }
  return broken;
}

void TilesetFlagEditorDialog::show_preview_flags()
{
  //load it up and show in the colours of the selected shield theme
  if (heartbeat.connected())
    heartbeat.disconnect();

  clearFlag();
  if (loadFlag() == true)
    {
      on_heartbeat ();
      heartbeat = Glib::signal_timeout().connect
	(sigc::bind_return (method (on_heartbeat), true),
         TIMER_BIGMAP_SELECTOR);
    }
}

void TilesetFlagEditorDialog::clearFlag()
{
  for (std::map< guint32, std::list<Glib::RefPtr<Gdk::Pixbuf> >* >::iterator it =
       flags.begin(); it != flags.end(); it++)
    {
      for (std::list<Glib::RefPtr<Gdk::Pixbuf> >::iterator lit =
           (*it).second->begin(); lit != (*it).second->end(); lit++)
	{
	  (*lit).clear();
	}
      (*it).second->clear();
      delete ((*it).second);
    }
  flags.clear();
  preview_table->foreach(sigc::mem_fun(preview_table, &Gtk::Container::remove));
}

bool TilesetFlagEditorDialog::loadFlag()
{
  Glib::ustring n = shield_theme_combobox->get_active_text();
  Shieldset *shieldset = Shieldsetlist::getInstance()->get(n, 0);

  for (unsigned int i = 0; i < MAX_PLAYERS; i++)
    {
      std::list<Glib::RefPtr<Gdk::Pixbuf> > *mylist =
        new std::list<Glib::RefPtr<Gdk::Pixbuf> >();
      flags[i] = mylist;
    }

  for (guint32 i = 0; i < d_flags->getNumberOfFrames (); i++)
    {
      for (Shieldset::iterator sit = shieldset->begin();
           sit != shieldset->end(); sit++)
        {
          if ((*sit)->getOwner() == 8) //ignore neutral
            continue;
          PixMask *q = d_flags->applyMask (i, (*sit)->getColor());
          double ratio = EDITOR_DIALOG_TILE_PIC_FONTSIZE_MULTIPLE;
          int font_size = FontSize::getInstance()->get_height ();
          double new_height = font_size * ratio;
          int new_width =
            ImageCache::calculate_width_from_adjusted_height
            (q, new_height);
          PixMask::scale (q, new_width, new_height);
          flags[(*sit)->getOwner()]->push_back (q->to_pixbuf ());
          frame[(*sit)->getOwner()] = flags[(*sit)->getOwner()]->begin();
        }
    }

  return true;
}

void TilesetFlagEditorDialog::update_flag_panel()
{
  Glib::ustring imgname = d_tileset->getFlags()->getName();
  if (imgname.empty() == false)
    {
      flag_imagebutton->set_label (imgname);
      show_preview_flags ();
    }
  else
    {
      flag_imagebutton->set_label (_("No image set"));
      if (heartbeat.connected ())
        heartbeat.disconnect ();
      clearFlag();
    }
}

void TilesetFlagEditorDialog::on_heartbeat()
{
  preview_table->foreach(sigc::mem_fun(preview_table, &Gtk::Container::remove));
  for (int i = 0; i < 4; i++)
    preview_table->insert_row (i);
  for (int i = 0; i < 2; i++)
    preview_table->insert_column (i);

  int x = 0;
  int y = 0;
  int count = 0;
  for (std::map< guint32, std::list<Glib::RefPtr<Gdk::Pixbuf> >* >::iterator it = flags.begin();
       it != flags.end(); it++)
    {
      //make a pixbuf and attach it
      switch (count)
	{
	case 0: x = 0; y = 0; break;
	case 1: x = 0; y = 1; break;
	case 2: x = 0; y = 2; break;
	case 3: x = 0; y = 3; break;
	case 4: x = 1; y = 0; break;
	case 5: x = 1; y = 1; break;
	case 6: x = 1; y = 2; break;
	case 7: x = 1; y = 3; break;
	}
      preview_table->attach(*manage(new Gtk::Image(*frame[count])), y, x, 1,1);

      frame[count]++;
      if (frame[count] == flags[count]->end())
	frame[count] = flags[count]->begin();
      count++;
    }
  preview_table->show_all();

}

Gtk::FileChooserDialog* TilesetFlagEditorDialog::image_filechooser(bool clear)
{
  Glib::ustring filename = "";
  Glib::ustring title = _("Choose a flag image");
  Gtk::FileChooserDialog *d = new Gtk::FileChooserDialog(*dialog, title);
  ImageFileFilter::getInstance ()->add (d);
  d->add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
  d->add_button(Gtk::Stock::OPEN, Gtk::RESPONSE_ACCEPT);
  if (clear)
    d->add_button(Gtk::Stock::CLEAR, Gtk::RESPONSE_REJECT);
  d->set_default_response(Gtk::RESPONSE_ACCEPT);
  d->set_current_folder(PastChooser::getInstance()->get_dir(d));
  return d;
}

void TilesetFlagEditorDialog::on_flag_imagebutton_clicked ()
{
  Glib::ustring f = d_tileset->getFlags()->getName ();
  Glib::ustring filename = "";
  Gtk::FileChooserDialog *d = image_filechooser(f != "");
  if (f != "")
    filename = d_tileset->getFileFromConfigurationFile(f);
  int response = d->run();
  if (filename != "")
    File::erase(filename);
  if (response == Gtk::RESPONSE_ACCEPT && d->get_filename() != "")
    {
      if (ImageFileFilter::getInstance()->hasInvalidExt(d->get_filename ()))
        ImageFileFilter::getInstance()->showErrorDialog (d);
      else
        {
          if (d->get_filename() != filename)
            {
              PastChooser::getInstance()->set_dir(d);
              on_image_chosen (d);
            }
        }
    }
  else if (response == Gtk::RESPONSE_REJECT && f != "")
    {
      if (d_tileset->removeFileInCfgFile(f))
        {
          d_changed = true;
          d_tileset->uninstantiateSameNamedImages (f);
          d_flags->clear ();
          clearFlag ();
          update_flag_panel();
        }
      else
        {
          Glib::ustring errmsg = Glib::strerror(errno);
          TimedMessageDialog
            td(*d, String::ucompose(_("Couldn't remove %1 from:\n%2\n%3"),
                                    f, d_tileset->getConfigurationFile(),
                                    errmsg), 0);
          td.run_and_hide ();
        }
    }
  d->hide();
  delete d;

}

TilesetFlagEditorDialog::~TilesetFlagEditorDialog ()
{
  delete d_flags;
}
