//  Copyright (C) 2008, 2009, 2010, 2012, 2014, 2020, 2021 Ben Asselstine
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

#include "tileset-selector-editor-dialog.h"

#include "ucompose.hpp"
#include "defs.h"
#include "File.h"
#include "shieldsetlist.h"
#include "ImageCache.h"
#include "tileset-window.h"
#include "past-chooser.h"
#include "font-size.h"
#include "image-file-filter.h"
#include "timed-message-dialog.h"
#include "TarFileMaskedImage.h"
#include "tileset-selector-editor-actions.h"

#define method(x) sigc::mem_fun(*this, &TilesetSelectorEditorDialog::x)

TilesetSelectorEditorDialog::TilesetSelectorEditorDialog(Gtk::Window &parent, Tileset *tileset)
 : LwEditorDialog(parent, "tileset-selector-editor-dialog.ui")
{
  umgr = new UndoMgr (UndoMgr::DELAY, UndoMgr::LIMIT);
  umgr->execute ().connect (method (executeAction));
  d_changed = false;
  d_tileset = tileset;
  small_selector = new TarFileMaskedImage (*d_tileset->getSelector(false));
  large_selector = new TarFileMaskedImage (*d_tileset->getSelector(true));

  Gtk::Box *box;
  xml->get_widget("shieldset_box", box);
  setup_shield_theme_combobox(box);
  xml->get_widget("preview_table", preview_table);

  xml->get_widget("large_selector_radiobutton", large_selector_radiobutton);
  xml->get_widget("small_selector_radiobutton", small_selector_radiobutton);
  xml->get_widget("selector_imagebutton", selector_imagebutton);
  xml->get_widget("undo_button", undo_button);
  undo_button->signal_activate ().connect (method (on_undo_activated));
  xml->get_widget("redo_button", redo_button);
  redo_button->signal_activate ().connect (method (on_redo_activated));

  d_large = false;
  connect_signals ();
  update ();
}

void TilesetSelectorEditorDialog::on_button_toggle ()
{
  umgr->add (new TileSetSelectorEditorAction_Size (d_large));
  d_large = large_selector_radiobutton->get_active ();
  update ();
}

bool TilesetSelectorEditorDialog::run()
{
  dialog->show_all();
  dialog->run();
  dialog->hide ();
  return d_changed;
}

void TilesetSelectorEditorDialog::setup_shield_theme_combobox(Gtk::Box *box)
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
  d_shield_row = default_id;

  box->set_center_widget (*shield_theme_combobox);
}

void TilesetSelectorEditorDialog::on_shieldset_changed()
{
  umgr->add (new TileSetSelectorEditorAction_Shield (d_shield_row));
  d_shield_row = shield_theme_combobox->get_active_row_number ();
  update ();
}

bool TilesetSelectorEditorDialog::on_image_chosen (Gtk::FileChooserDialog *d)
{
  bool broken = false;
  if (PixMask::checkFormat (d->get_filename ()))
    {
      if (d_tileset->getSelector (d_large)->checkDimension (d->get_filename ()))
        {
          Glib::ustring imgname = get_selector_filename ();
          Glib::ustring newname = "";
          bool success = false;
          if (imgname.empty() == true)
            success = d_tileset->addFileInCfgFile(d->get_filename(), newname);
          else
            success = d_tileset->replaceFileInCfgFile (imgname,
                                                       d->get_filename(),
                                                       newname);
          if (success)
            {
              set_selector_filename (newname);
              d_changed = true;
              update ();
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
          Glib::ustring errmsg = Glib::strerror(errno);
          TimedMessageDialog
            td(*d, String::ucompose(_("Bad dimensions in image:\n%1"),
                                    d->get_filename ()), 0);
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

void TilesetSelectorEditorDialog::show_preview_selectors()
{
  //load it up and show in the colours of the selected shield theme
  clearSelector();
  if (loadSelector () == true)
    {
      on_heartbeat ();
      heartbeat = Glib::signal_timeout().connect
        (sigc::bind_return (method (on_heartbeat), true),
         TIMER_BIGMAP_SELECTOR);
    }
}

void TilesetSelectorEditorDialog::clearSelector()
{
  if (heartbeat.connected())
    heartbeat.disconnect();

  for (std::map< guint32, std::list<Glib::RefPtr<Gdk::Pixbuf> >* >::iterator it = selectors.begin();
       it != selectors.end(); ++it)
    {
      for (std::list<Glib::RefPtr<Gdk::Pixbuf> >::iterator lit =
           (*it).second->begin(); lit != (*it).second->end(); ++lit)
	{
	  (*lit).clear();
	}
      (*it).second->clear();
      delete ((*it).second);
    }
  selectors.clear();
  preview_table->foreach(sigc::mem_fun(preview_table, &Gtk::Container::remove));
}

bool TilesetSelectorEditorDialog::loadSelector()
{
  TarFileMaskedImage *p = NULL;
  if (large_selector_radiobutton->get_active() == true)
    p = large_selector;
  else if (small_selector_radiobutton->get_active() == true)
    p = small_selector;
  if (!p)
    return false;
  if (p->getImage () && p->getImage ()->get_unscaled_height () == 0)
    return false;

  Glib::ustring n = shield_theme_combobox->get_active_text();
  Shieldset *shieldset = Shieldsetlist::getInstance()->get(n, 0);

  for (unsigned int i = 0; i < MAX_PLAYERS; i++)
    {
      std::list<Glib::RefPtr<Gdk::Pixbuf> > *mylist =
        new std::list<Glib::RefPtr<Gdk::Pixbuf> >();
      selectors[i] = mylist;
    }

  frame.clear ();
  if (p->getBackingImage () == NULL)
    return false;

  for (guint32 i = 0; i < p->getNumberOfFrames (); i++)
    {
      for (Shieldset::iterator sit = shieldset->begin();
           sit != shieldset->end(); ++sit)
        {
          if ((*sit)->getOwner() == 8) //ignore neutral
            continue;
          PixMask *q = p->applyMask (i, (*sit)->getColor ());
          double ratio = EDITOR_DIALOG_TILE_PIC_FONTSIZE_MULTIPLE;
          int font_size = FontSize::getInstance()->get_height ();
          double new_height = font_size * ratio;
          int new_width =
            ImageCache::calculate_width_from_adjusted_height
            (q, new_height);
          PixMask::scale (q, new_width, new_height);
          selectors[(*sit)->getOwner()]->push_back (q->to_pixbuf ());

          frame[(*sit)->getOwner()] =
            selectors[(*sit)->getOwner()]->begin();
        }
    }

  return true;
}

void TilesetSelectorEditorDialog::on_heartbeat()
{
  preview_table->foreach(sigc::mem_fun(preview_table, &Gtk::Container::remove));
  for (int i = 0; i < 4; i++)
    preview_table->insert_row (i);
  for (int i = 0; i < 2; i++)
    preview_table->insert_column (i);

  int x = 0;
  int y = 0;
  int count = 0;
  for (std::map< guint32, std::list<Glib::RefPtr<Gdk::Pixbuf> >* >::iterator it = selectors.begin();
       it != selectors.end(); ++it)
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
      preview_table->attach(*manage(new Gtk::Image(*frame[count])), y, x, 1, 1);

      ++frame[count];
      if (frame[count] == selectors[count]->end())
	frame[count] = selectors[count]->begin();
      count++;
    }
  preview_table->show_all();
}

void TilesetSelectorEditorDialog::update_selector_panel()
{
  Glib::ustring f = get_selector_filename ();
  if (f.empty () == false)
    selector_imagebutton->set_label (f);
  else
    {
      selector_imagebutton->set_label (_("No image set"));
      clearSelector();
    }
}

Gtk::FileChooserDialog* TilesetSelectorEditorDialog::image_filechooser(bool clear)
{
  Glib::ustring filename = "";
  Glib::ustring title = "";
  if (large_selector_radiobutton->get_active() == true)
    title = _("Choose a large selector image");
  else if (small_selector_radiobutton->get_active() == true)
    title = _("Choose a small selector image");
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

Glib::ustring TilesetSelectorEditorDialog::get_selector_filename ()
{
  if (large_selector_radiobutton->get_active() == true)
    return d_tileset->getSelector(true)->getName ();
  else if (small_selector_radiobutton->get_active() == true)
    return d_tileset->getSelector(false)->getName ();
  return "";
}

void TilesetSelectorEditorDialog::set_selector_filename (Glib::ustring f)
{
  if (large_selector_radiobutton->get_active() == true)
    {
      if (f.empty () == false)
        {
          d_tileset->getSelector (true)->load(d_tileset, f);
          d_tileset->getSelector (true)->instantiateImages();
        }
    }
  else if (small_selector_radiobutton->get_active() == true)
    {
      if (f.empty () == false)
        {
          d_tileset->getSelector (false)->load(d_tileset, f);
          d_tileset->getSelector (false)->instantiateImages();
        }
    }
  return ;
}

void TilesetSelectorEditorDialog::clear_selector_image ()
{
  if (large_selector_radiobutton->get_active() == true)
    {
      large_selector->clear ();
      d_tileset->getSelector(true)->clear();
    }
  else if (small_selector_radiobutton->get_active() == true)
    {
      small_selector->clear ();
      d_tileset->getSelector(false)->clear();
    }
  return;
}

void TilesetSelectorEditorDialog::on_selector_imagebutton_clicked ()
{
  Glib::ustring f = get_selector_filename ();
  Glib::ustring filename = "";
  Gtk::FileChooserDialog *d = image_filechooser(f != "");
  if (f != "")
    filename = d_tileset->getFileFromConfigurationFile(f);
  int response = d->run();
  if (filename != "")
    File::erase(filename);
  if (response == Gtk::RESPONSE_ACCEPT && d->get_filename() != "")
    {
      if (ImageFileFilter::getInstance ()->hasInvalidExt (d->get_filename ()))
        ImageFileFilter::getInstance ()->showErrorDialog (d);
      else
        {
          if (d->get_filename() != filename)
            {
              PastChooser::getInstance()->set_dir(d);
              Glib::ustring archive_member = get_selector_filename ();
              TileSetSelectorEditorAction_Set *action =
                new TileSetSelectorEditorAction_Set (d_tileset,
                                                     d_large, archive_member);
              if (on_image_chosen (d) == false)
                umgr->add (action);
              else
                delete action;
            }
        }
    }
  else if (response == Gtk::RESPONSE_REJECT && f != "")
    {
      Glib::ustring archive_member = get_selector_filename ();
      TileSetSelectorEditorAction_Set *action =
        new TileSetSelectorEditorAction_Set (d_tileset, d_large,
                                             archive_member);
      if (d_tileset->removeFileInCfgFile(f))
        {
          umgr->add (action);
          d_changed = true;
          d_tileset->uninstantiateSameNamedImages (f);
          update ();
        }
      else
        {
          delete action;
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

TilesetSelectorEditorDialog::~TilesetSelectorEditorDialog()
{
  delete umgr;
  if (small_selector)
    delete small_selector;
  if (large_selector)
    delete large_selector;
}

void TilesetSelectorEditorDialog::on_undo_activated ()
{
  umgr->undo ();
  if (umgr->undoEmpty ())
    d_changed = false;
  update ();
  return;
}

void TilesetSelectorEditorDialog::on_redo_activated ()
{
  umgr->redo ();
  d_changed = true;
  update ();
}

void TilesetSelectorEditorDialog::update ()
{
  disconnect_signals ();
  shield_theme_combobox->set_active (d_shield_row);
  if (d_large)
    large_selector_radiobutton->set_active (d_large);
  else
    small_selector_radiobutton->set_active (!d_large);
  show_preview_selectors();
  update_selector_panel();
  connect_signals ();
}

void TilesetSelectorEditorDialog::connect_signals ()
{
  connections.push_back
    (shield_theme_combobox->signal_changed().connect
     (method(on_shieldset_changed)));
  connections.push_back
    (large_selector_radiobutton->signal_toggled().connect
     (method(on_button_toggle)));
  connections.push_back
    (selector_imagebutton->signal_clicked().connect
     (method(on_selector_imagebutton_clicked)));
}

void TilesetSelectorEditorDialog::disconnect_signals ()
{
  for (auto c : connections)
    c.disconnect ();
  connections.clear ();
}

UndoAction *TilesetSelectorEditorDialog::executeAction (UndoAction *action2)
{
  TileSetSelectorEditorAction *action =
    dynamic_cast<TileSetSelectorEditorAction*>(action2);
  UndoAction *out = NULL;

  heartbeat.disconnect();
  switch (action->getType ())
    {
    case TileSetSelectorEditorAction::SET:
        {
          TileSetSelectorEditorAction_Set *a =
            dynamic_cast<TileSetSelectorEditorAction_Set*>(action);
          TarFileMaskedImage *im = d_tileset->getSelector (d_large);
          out = new TileSetSelectorEditorAction_Set (d_tileset, a->getLarge (),
                                                     im->getName ());
          if (a->getArchiveMember ().empty ())
            im->clear ();
          else
            {
              Glib::ustring ar = a->getArchiveMember ();
              Glib::ustring file = a->getFileName ();
              bool broken = false;
              Glib::ustring newbasename = "";
              bool present = d_tileset->contains (ar, broken);
              if (present)
                d_tileset->replaceFileInCfgFile (ar, file, newbasename);
              else
                d_tileset->addFileInCfgFile (file, newbasename);

              d_tileset->getSelector (d_large)->load (d_tileset, newbasename);
              d_tileset->getSelector (d_large)->instantiateImages ();
            }
        }
      break;
    case TileSetSelectorEditorAction::SHIELD:
        {
          TileSetSelectorEditorAction_Shield *a =
            dynamic_cast<TileSetSelectorEditorAction_Shield*>(action);
          out = new TileSetSelectorEditorAction_Shield
            (shield_theme_combobox->get_active_row_number ());
          d_shield_row = a->getShield ();
        }
      break;
    case TileSetSelectorEditorAction::SIZE:
        {
          TileSetSelectorEditorAction_Size *a =
            dynamic_cast<TileSetSelectorEditorAction_Size*>(action);
          out = new TileSetSelectorEditorAction_Size (d_large);
          d_large = a->getLarge ();
        }
      break;
    }
  return out;
}
