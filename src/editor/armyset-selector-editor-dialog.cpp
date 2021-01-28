//  Copyright (C) 2020, 2021 Ben Asselstine
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

#include "armyset-selector-editor-dialog.h"

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
#include "armyset-selector-editor-actions.h"

#define method(x) sigc::mem_fun(*this, &ArmysetSelectorEditorDialog::x)

ArmysetSelectorEditorDialog::ArmysetSelectorEditorDialog(Gtk::Window &parent, Armyset *armyset)
 : LwEditorDialog(parent, "armyset-selector-editor-dialog.ui")
{
  umgr = new UndoMgr (UndoMgr::DELAY, UndoMgr::LIMIT);
  umgr->execute ().connect (method (executeAction));
  d_changed = false;

  Gtk::Box *box;
  xml->get_widget("owner_box", box);
  setup_owner_combobox(box);

  d_armyset = armyset;
  small_selector =
    new TarFileMaskedImage (*d_armyset->getSelector (false, Shield::WHITE));
  large_selector =
    new TarFileMaskedImage (*d_armyset->getSelector (true, Shield::WHITE));

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

void ArmysetSelectorEditorDialog::on_button_toggle ()
{
  umgr->add (new ArmySetSelectorEditorAction_Size (d_large));
  d_large = large_selector_radiobutton->get_active ();
  update ();
}

bool ArmysetSelectorEditorDialog::run()
{
  dialog->show_all();
  dialog->run();
  dialog->hide ();
  return d_changed;
}

void ArmysetSelectorEditorDialog::setup_owner_combobox(Gtk::Box *box)
{
  // fill in owner combobox
  owner_combobox = manage(new Gtk::ComboBoxText);

  for (int i = Shield::WHITE; i < Shield::NEUTRAL; i++)
    {
      Shield::Colour c = Shield::Colour (i);
      owner_combobox->append(Shield::colourToFriendlyName (c));
    }

  owner_combobox->set_active(0);
  d_owner_row = 0;

  box->set_center_widget (*owner_combobox);
}

void ArmysetSelectorEditorDialog::setup_shield_theme_combobox(Gtk::Box *box)
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

void ArmysetSelectorEditorDialog::on_shieldset_changed()
{
  umgr->add (new ArmySetSelectorEditorAction_Shield (d_shield_row));
  d_shield_row = shield_theme_combobox->get_active_row_number ();
  update ();
}

bool ArmysetSelectorEditorDialog::on_image_chosen (Gtk::FileChooserDialog *d)
{
  bool broken = false;
  if (PixMask::checkFormat (d->get_filename ()))
    {
      if (checkDimensions (d->get_filename (),
                           TarFileMaskedImage::VERTICAL_MASK))
        {
          Glib::ustring imgname = get_selector_filename ();
          Glib::ustring newname = "";
          bool success = false;
          if (imgname.empty() == true)
            success =
              d_armyset->addFileInCfgFile(d->get_filename(), newname);
          else
            success =
              d_armyset->replaceFileInCfgFile(imgname, d->get_filename(),
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
                                        d_armyset->getConfigurationFile(),
                                        errmsg), 0);
              td.run_and_hide ();
              broken = true;
            }
        }
      else
        {
          TimedMessageDialog td
            (*d, String::ucompose(_("The dimensions of the image are bad:\n%1"),
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

void ArmysetSelectorEditorDialog::show_preview_selectors()
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

void ArmysetSelectorEditorDialog::clearSelector()
{
  if (heartbeat.connected())
    heartbeat.disconnect();

  for (std::list<Glib::RefPtr<Gdk::Pixbuf> >::iterator lit = selectors.begin();
       lit != selectors.end(); ++lit)
    (*lit).clear();
  selectors.clear();
  preview_table->foreach(sigc::mem_fun(preview_table, &Gtk::Container::remove));
}

bool ArmysetSelectorEditorDialog::loadSelector()
{
  TarFileMaskedImage *p = NULL;
  //which one, go get it according to owner
  Shield::Colour o =
    Shield::Colour (owner_combobox->get_active_row_number ());
  if (large_selector_radiobutton->get_active() == true)
    {
      delete large_selector;
      large_selector =
        new TarFileMaskedImage (*d_armyset->getSelector (true, o));
      p = large_selector;
    }
  else if (small_selector_radiobutton->get_active() == true)
    {
      delete small_selector;
      small_selector =
        new TarFileMaskedImage (*d_armyset->getSelector (false, o));
      p = small_selector;
    }
  if (!p)
    return false;
  if (p->getImage() && p->getImage ()->get_unscaled_height () == 0)
    return false;

  if (p->getNumberOfFrames () == 0)
    return false;

  Glib::ustring n = shield_theme_combobox->get_active_text();
  Shieldset *shieldset = Shieldsetlist::getInstance()->get(n, 0);

  selectors = std::list<Glib::RefPtr<Gdk::Pixbuf> >();

  Shield *selected_shield = NULL;
  for (Shieldset::iterator sit = shieldset->begin();
       sit != shieldset->end(); ++sit)
    if ((*sit)->getOwner () ==
        (guint32) owner_combobox->get_active_row_number ())
      {
        selected_shield = *sit;
        break;
      }
  for (guint32 i = 0; i < p->getNumberOfFrames (); i++)
    {
      guint32 owner = selected_shield->getOwner ();
      if (owner == MAX_PLAYERS) //ignore neutral
        continue;
      PixMask *q = p->applyMask (i, selected_shield->getColor());
      double ratio = EDITOR_DIALOG_TILE_PIC_FONTSIZE_MULTIPLE;
      int font_size = FontSize::getInstance()->get_height ();
      double new_height = font_size * ratio;
      int new_width =
        ImageCache::calculate_width_from_adjusted_height
        (q, new_height);
      PixMask::scale (q, new_width, new_height);
      selectors.push_back (q->to_pixbuf ());

      frame = selectors.begin();
    }

  return true;
}

void ArmysetSelectorEditorDialog::on_heartbeat()
{
  preview_table->foreach(sigc::mem_fun(preview_table, &Gtk::Container::remove));
  preview_table->insert_row (0);
  preview_table->insert_column (0);

  preview_table->attach(*manage(new Gtk::Image(*frame)), 0, 0, 1, 1);
  ++frame;
  if (frame == selectors.end())
    frame = selectors.begin();

  preview_table->show_all();
}

void ArmysetSelectorEditorDialog::update_selector_panel()
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

Gtk::FileChooserDialog* ArmysetSelectorEditorDialog::image_filechooser(bool clear)
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

TarFileMaskedImage * ArmysetSelectorEditorDialog::get_selector ()
{
  Shield::Colour c = get_selected_colour ();
  if (large_selector_radiobutton->get_active() == true)
    return d_armyset->getSelector(true,c);
  else if (small_selector_radiobutton->get_active() == true)
    return d_armyset->getSelector(false, c);
  return NULL;
}

Glib::ustring ArmysetSelectorEditorDialog::get_selector_filename ()
{
  return get_selector ()->getName ();
}

void ArmysetSelectorEditorDialog::set_selector_filename (Glib::ustring f)
{
  Shield::Colour c = get_selected_colour ();
  if (large_selector_radiobutton->get_active () == true)
    {
      if (f.empty () == false)
        {
          d_armyset->getSelector (true, c)->load (d_armyset, f);
          d_armyset->getSelector (true, c)->instantiateImages ();
        }
      delete large_selector;
      large_selector =
        new TarFileMaskedImage (*d_armyset->getSelector (true, c));
    }
  else if (small_selector_radiobutton->get_active() == true)
    {
      if (f.empty () == false)
        {
          d_armyset->getSelector (false, c)->load (d_armyset, f);
          d_armyset->getSelector (false, c)->instantiateImages ();
        }
      delete small_selector;
      small_selector =
        new TarFileMaskedImage (*d_armyset->getSelector (false, c));
    }
  return ;
}

void ArmysetSelectorEditorDialog::on_selector_imagebutton_clicked ()
{
  Glib::ustring f = get_selector_filename ();
  Glib::ustring filename = "";
  Gtk::FileChooserDialog *d = image_filechooser(f != "");
  if (f != "")
    filename = d_armyset->getFileFromConfigurationFile(f);
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
              ArmySetSelectorEditorAction_Set *action =
                new ArmySetSelectorEditorAction_Set (d_armyset,
                                                     get_selected_colour (),
                                                     d_large, archive_member);
              if (on_image_chosen (d) == false) //false means not broken
                umgr->add (action);
              else
                delete action;
            }
        }
    }
  else if (response == Gtk::RESPONSE_REJECT && f != "")
    {
      Glib::ustring archive_member = get_selector_filename ();
      ArmySetSelectorEditorAction_Set *action =
        new ArmySetSelectorEditorAction_Set (d_armyset, get_selected_colour (),
                                             d_large, archive_member);
      if (d_armyset->removeFileInCfgFile(f))
        {
          umgr->add (action);
          d_changed = true;
          d_armyset->uninstantiateSameNamedImages (f);
          update ();
        }
      else
        {
          delete action;
          Glib::ustring errmsg = Glib::strerror(errno);
          TimedMessageDialog
            td(*d, String::ucompose(_("Couldn't remove %1 from:\n%2\n%3"),
                                    f, d_armyset->getConfigurationFile(),
                                    errmsg), 0);
          td.run_and_hide ();
        }
    }
  d->hide();
  delete d;
}

ArmysetSelectorEditorDialog::~ArmysetSelectorEditorDialog()
{
  delete umgr;
  if (small_selector)
    delete small_selector;
  if (large_selector)
    delete large_selector;
}

void ArmysetSelectorEditorDialog::on_owner_changed()
{
  umgr->add (new ArmySetSelectorEditorAction_Owner (d_owner_row));
  d_owner_row = owner_combobox->get_active_row_number ();
  update ();
}

Shield::Colour ArmysetSelectorEditorDialog::get_selected_colour ()
{
  return Shield::Colour (owner_combobox->get_active_row_number ());
}

void ArmysetSelectorEditorDialog::on_undo_activated ()
{
  umgr->undo ();
  update ();
  return;
}

void ArmysetSelectorEditorDialog::on_redo_activated ()
{
  umgr->redo ();
  update ();
}

void ArmysetSelectorEditorDialog::update ()
{
  disconnect_signals ();
  shield_theme_combobox->set_active (d_shield_row);
  owner_combobox->set_active (d_owner_row);
  large_selector_radiobutton->set_active (d_large);
  small_selector_radiobutton->set_active (!d_large);
  show_preview_selectors();
  update_selector_panel();
  connect_signals ();
}

void ArmysetSelectorEditorDialog::connect_signals ()
{
  connections.push_back
    (owner_combobox->signal_changed().connect (method(on_owner_changed)));
  connections.push_back
    (shield_theme_combobox->signal_changed().connect
     (method(on_shieldset_changed)));
  connections.push_back
    (large_selector_radiobutton->signal_toggled().connect
     (method(on_button_toggle)));
  connections.push_back
    (small_selector_radiobutton->signal_toggled().connect
     (method(on_button_toggle)));
  connections.push_back
    (selector_imagebutton->signal_clicked().connect
     (method(on_selector_imagebutton_clicked)));
}

void ArmysetSelectorEditorDialog::disconnect_signals ()
{
  for (auto c : connections)
    c.disconnect ();
  connections.clear ();
}

bool ArmysetSelectorEditorDialog::checkDimensions (Glib::ustring filename, TarFileMaskedImage::MaskOrientation o)
{
  bool success = false;
  bool broken = false;
  PixMask *p = PixMask::create (filename, broken);
  if (broken)
    return success;
  switch (o)
    {
    case TarFileMaskedImage::HORIZONTAL_MASK: //mask is to the side
      success = (p->get_unscaled_width () / 2) == p->get_unscaled_height ();
      break;
    case TarFileMaskedImage::VERTICAL_MASK: //mask is underneath
      success = p->get_unscaled_width () % (p->get_unscaled_height () / 2) == 0;
      break;
    }
  delete p;
  return success;
}

UndoAction *ArmysetSelectorEditorDialog::executeAction (UndoAction *action2)
{
  ArmySetSelectorEditorAction *action =
    dynamic_cast<ArmySetSelectorEditorAction*>(action2);
  UndoAction *out = NULL;

  heartbeat.disconnect();
  switch (action->getType ())
    {
    case ArmySetSelectorEditorAction::SET:
        {
          ArmySetSelectorEditorAction_Set *a =
            dynamic_cast<ArmySetSelectorEditorAction_Set*>(action);
          TarFileMaskedImage *im =
            d_armyset->getSelector (a->getLarge (), a->getOwner ());
          out =
            new ArmySetSelectorEditorAction_Set (d_armyset,
                                                 a->getOwner (),
                                                 a->getLarge (),
                                                 im->getName ());
          if (a->getArchiveMember ().empty ())
            im->clear ();
          else
            {
              Glib::ustring ar = a->getArchiveMember ();
              Glib::ustring file = a->getFileName ();
              bool broken = false;
              Glib::ustring newbasename = "";
              bool present = d_armyset->contains (ar, broken);
              if (present)
                d_armyset->replaceFileInCfgFile (ar, file, newbasename);
              else
                d_armyset->addFileInCfgFile (file, newbasename);

              get_selector ()->load (d_armyset, newbasename);
              get_selector ()->instantiateImages ();
            }
        }
      break;
    case ArmySetSelectorEditorAction::SHIELD:
        {
          ArmySetSelectorEditorAction_Shield *a =
            dynamic_cast<ArmySetSelectorEditorAction_Shield*>(action);
          out = new ArmySetSelectorEditorAction_Shield
            (shield_theme_combobox->get_active_row_number ());
          d_shield_row = a->getShield ();
        }
      break;
    case ArmySetSelectorEditorAction::OWNER:
        {
          ArmySetSelectorEditorAction_Owner *a =
            dynamic_cast<ArmySetSelectorEditorAction_Owner*>(action);
          out = new ArmySetSelectorEditorAction_Owner
            (owner_combobox->get_active_row_number ());
          d_owner_row = a->getOwner ();
        }
      break;
    case ArmySetSelectorEditorAction::SIZE:
        {
          ArmySetSelectorEditorAction_Size *a =
            dynamic_cast<ArmySetSelectorEditorAction_Size*>(action);
          out = new ArmySetSelectorEditorAction_Size (d_large);
          d_large = a->getLarge ();
        }
      break;
    }
  return out;
}
