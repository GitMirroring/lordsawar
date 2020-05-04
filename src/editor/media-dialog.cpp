//  Copyright (C) 2017, 2020 Ben Asselstine
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

#include "media-dialog.h"

#include "ucompose.hpp"
#include "ScenarioMedia.h"
#include "image-editor-dialog.h"
#include "tar-file-masked-image-editor-dialog.h"
#include "File.h"
#include "defs.h"
#include "tarfile.h"
#include "shieldsetlist.h"
#include "shieldset.h"
#include "playerlist.h"
#include "past-chooser.h"
#include "ImageCache.h"
#include "timed-message-dialog.h"
#include "TarFileMaskedImage.h"
#include "TarFileImage.h"

#define method(x) sigc::mem_fun(*this, &MediaDialog::x)

MediaDialog::MediaDialog(Gtk::Window &parent, TarFile *tarfile)
 : LwEditorDialog(parent, "media-dialog.ui")
{
  d_tarfile = tarfile;
  d_needs_saving = false;
  xml->get_widget("next_turn_button", d_next_turn_button);
  d_next_turn_button->signal_clicked().connect
    (method(on_next_turn_button_activated));
  xml->get_widget("city_defeated_button", d_city_defeated_button);
  d_city_defeated_button->signal_clicked().connect
    (method(on_city_defeated_button_activated));
  xml->get_widget("winning_button", d_winning_button);
  d_winning_button->signal_clicked().connect
    (method(on_winning_button_activated));
  xml->get_widget("hero_male_button", d_hero_male_button);
  d_hero_male_button->signal_clicked().connect
    (method(on_hero_male_button_activated));
  xml->get_widget("hero_female_button", d_hero_female_button);
  d_hero_female_button->signal_clicked().connect
    (method(on_hero_female_button_activated));
  xml->get_widget("ruin_success_button", d_ruin_success_button);
  d_ruin_success_button->signal_clicked().connect
    (method(on_ruin_success_button_activated));
  xml->get_widget("ruin_defeat_button", d_ruin_defeat_button);
  d_ruin_defeat_button->signal_clicked().connect
    (method(on_ruin_defeat_button_activated));
  xml->get_widget("hero_newlevel_male_button", d_hero_newlevel_male_button);
  d_hero_newlevel_male_button->signal_clicked().connect
    (method(on_hero_newlevel_male_button_activated));
  xml->get_widget("hero_newlevel_female_button", d_hero_newlevel_female_button);
  d_hero_newlevel_female_button->signal_clicked().connect
    (method(on_hero_newlevel_female_button_activated));
  xml->get_widget("parley_offered_button", d_parley_offered_button);
  d_parley_offered_button->signal_clicked().connect
    (method(on_parley_offered_button_activated));
  xml->get_widget("parley_refused_button", d_parley_refused_button);
  d_parley_refused_button->signal_clicked().connect
    (method(on_parley_refused_button_activated));
  xml->get_widget("small_medals_button", d_small_medals_button);
  d_small_medals_button->signal_clicked().connect
    (method(on_small_medals_button_activated));
  xml->get_widget("big_medals_button", d_big_medals_button);
  d_big_medals_button->signal_clicked().connect
    (method(on_big_medals_button_activated));
  xml->get_widget("commentator_button", d_commentator_button);
  d_commentator_button->signal_clicked().connect
    (method(on_commentator_button_activated));
  xml->get_widget("bless_button", d_bless_button);
  d_bless_button->signal_clicked().connect(method(on_bless_button_activated));
  xml->get_widget("hero_button", d_hero_button);
  d_hero_button->signal_clicked().connect(method(on_hero_button_activated));
  xml->get_widget("battle_button", d_battle_button);
  d_battle_button->signal_clicked().connect(method(on_battle_button_activated));
  xml->get_widget("defeat_button", d_defeat_button);
  d_defeat_button->signal_clicked().connect(method(on_defeat_button_activated));
  xml->get_widget("victory_button", d_victory_button);
  d_victory_button->signal_clicked().connect
    (method(on_victory_button_activated));
  xml->get_widget("back_button", d_back_button);
  d_back_button->signal_clicked().connect(method(on_back_button_activated));
  xml->get_widget ("notebook", notebook);

  fill_in_buttons();
}

void MediaDialog::fill_image_button(Gtk::Button *button, Glib::ustring name)
{
  if (name == "")
    button->set_label (_("override default"));
  else
    button->set_label (name);
}

void MediaDialog::fill_sound_button(Gtk::Button *button, Glib::ustring name)
{
  if (name == "")
    button->set_label (_("override default"));
  else
    button->set_label (name + ".ogg");
}

void MediaDialog::fill_in_buttons()
{
  ScenarioMedia *sm = ScenarioMedia::getInstance();
  fill_image_button (d_next_turn_button, sm->getNextTurnImage ()->getName());
  fill_image_button (d_city_defeated_button,
                     sm->getCityDefeatedImage ()->getName());
  fill_image_button (d_winning_button, sm->getWinningImage ()->getName());
  fill_image_button (d_hero_male_button,
                     sm->getHeroOfferedImage (false)->getName());
  fill_image_button (d_hero_female_button,
                     sm->getHeroOfferedImage (true)->getName());
  fill_image_button (d_ruin_success_button,
                     sm->getRuinSuccessImage ()->getName());
  fill_image_button (d_ruin_defeat_button, sm->getRuinDefeatImage ()->getName());
  fill_image_button (d_hero_newlevel_male_button,
                     sm->getHeroNewLevelMaskedImage(false)->getName());
  fill_image_button (d_hero_newlevel_female_button,
                     sm->getHeroNewLevelMaskedImage(true)->getName());
  fill_image_button (d_parley_offered_button,
                     sm->getParleyOfferedImage ()->getName());
  fill_image_button (d_parley_refused_button,
                     sm->getParleyRefusedImage ()->getName());
  fill_image_button (d_small_medals_button,
                     sm->getMedalImage (false)->getName());
  fill_image_button (d_big_medals_button, sm->getMedalImage (true)->getName());
  fill_image_button (d_commentator_button,
                     sm->getCommentatorImage ()->getName());

  fill_sound_button (d_bless_button, sm->getBlessSoundName());
  fill_sound_button (d_hero_button, sm->getHeroSoundName());
  fill_sound_button (d_battle_button, sm->getBattleSoundName());
  fill_sound_button (d_defeat_button, sm->getDefeatSoundName());
  fill_sound_button (d_victory_button, sm->getVictorySoundName());
  fill_sound_button (d_back_button, sm->getBackSoundName());
}

int MediaDialog::run()
{
  dialog->show_all();
  int response = dialog->run();
  return response;
}

void MediaDialog::on_image_button_activated(TarFileImage *oim, TarFileImage *im)
{
  TarFile *t = d_tarfile;
  Glib::ustring imgname = im->getName ();
  ImageEditorDialog d (*dialog, oim, 0);
  int response = d.run();

  if (response == Gtk::RESPONSE_ACCEPT)
    {
      if (d.get_filename () != "")
        {
          Glib::ustring newname = "";
          bool success = false;
          if (im->getName() == "")
            success = t->addFileInCfgFile(d.get_filename (), newname);
          else
            success = t->replaceFileInCfgFile(imgname, d.get_filename (),
                                              newname);
          if (success)
            {
              im->load (t, newname);
              im->instantiateImages ();
              d_needs_saving = true;
              fill_in_buttons();
            }
          else
            {
              Glib::ustring errmsg = Glib::strerror(errno);
              TimedMessageDialog
                td(*d.get_dialog (),
                   String::ucompose(_("Couldn't add %1 to :\n%2\n%3"),
                                    d.get_filename (),
                                    t->getConfigurationFile(), errmsg), 0);
              td.run_and_hide ();
            }
        }
    }
  else if (response == Gtk::RESPONSE_REJECT)
    {
      if (imgname.empty () == false)
        {
          if (t->removeFileInCfgFile(imgname))
            {
              im->clear();
              d_needs_saving = true;
              fill_in_buttons();
            }
          else
            {
              Glib::ustring errmsg = Glib::strerror(errno);
              TimedMessageDialog
                td(*d.get_dialog (),
                   String::ucompose(_("Couldn't remove %1 from:\n%2\n%3"),
                                    imgname,
                                    t->getConfigurationFile(),
                                    errmsg), 0);
              td.run_and_hide ();
            }
        }
    }
  d.hide();
}

void MediaDialog::on_masked_image_button_activated(sigc::slot<Glib::ustring> getName, TarFileMaskedImage *mim, sigc::slot<void,Glib::ustring> setName, Shieldset *ss)
{
  TarFile *t = d_tarfile;
  Glib::ustring imgname = getName ();

  TarFileMaskedImageEditorDialog d (*dialog, mim, 0, ss);
  int response = d.run();

  if (response == Gtk::RESPONSE_ACCEPT && d.get_filename () != "")
    {
      Glib::ustring newname = "";
      bool success = false;
      if (getName() == "")
        success = t->addFileInCfgFile(d.get_filename (), newname);
      else
        success = t->replaceFileInCfgFile(imgname, d.get_filename (), newname);
      if (success)
        {
          setName(newname);
          d_needs_saving = true;
          fill_in_buttons();
        }
      else
        {
          Glib::ustring errmsg = Glib::strerror(errno);
          TimedMessageDialog
            td(*d.get_dialog (),
               String::ucompose(_("Couldn't add %1 to :\n%2\n%3"),
                                d.get_filename (),
                                t->getConfigurationFile(),
                                errmsg), 0);
          td.run_and_hide ();
        }
    }
  else if (response == Gtk::RESPONSE_REJECT)
    {
      if (imgname.empty () == false)
        {
          if (t->removeFileInCfgFile(imgname))
            {
              setName ("");
              d_needs_saving = true;
              fill_in_buttons();
            }
          else
            {
              Glib::ustring errmsg = Glib::strerror(errno);
              TimedMessageDialog
                td(*d.get_dialog (),
                   String::ucompose(_("Couldn't remove %1 from:\n%2\n%3"),
                                    imgname, t->getConfigurationFile(),
                                    errmsg), 0);
              td.run_and_hide ();
            }
        }
    }
  d.hide();
}

void MediaDialog::on_sound_button_activated(sigc::slot<Glib::ustring> getName, sigc::slot<Glib::ustring> getDefaultFilename, sigc::slot<void, Glib::ustring> setName)
{
  TarFile *t = d_tarfile;
  Glib::ustring sndname = getName ();
  Glib::ustring oldfile = getDefaultFilename ();
  if (sndname.empty () == false)
    oldfile = t->getFileFromConfigurationFile(sndname + ".ogg");

  Gtk::FileChooserDialog d(*dialog, _("Choose Sound File"));
  Glib::RefPtr<Gtk::FileFilter> ogg_filter = Gtk::FileFilter::create();
  ogg_filter->set_name(_("Sound Files (*.ogg)"));
  ogg_filter->add_pattern("*.ogg");
  d.add_filter(ogg_filter);

  d.set_current_folder(PastChooser::getInstance()->get_dir(&d));

  if (sndname.empty () == false)
    d.add_button(Gtk::Stock::CLEAR, Gtk::RESPONSE_REJECT);
  d.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
  d.add_button(Gtk::Stock::OPEN, Gtk::RESPONSE_ACCEPT);
  d.set_default_response(Gtk::RESPONSE_ACCEPT);

  d.show_all();
  int res = d.run();
  if (sndname.empty () == false)
    File::erase (oldfile);

  if (res == Gtk::RESPONSE_ACCEPT)
    {
      if (File::nameEndsWith(d.get_filename (), ".ogg") != true)
        {
          TimedMessageDialog td(d,
                                _("Only OGG files can be used for sound."), 0);
          td.run_and_hide();
        }

      if (d.get_filename () != getDefaultFilename ())
        {
          Glib::ustring newname = "";
          bool success = false;
          if (getName() == "")
            success = t->addFileInCfgFile(d.get_filename (), newname);
          else
            success = t->replaceFileInCfgFile(sndname, d.get_filename (),
                                              newname);
          if (success)
            {
              setName(newname);
              d_needs_saving = true;
              fill_in_buttons();
            }
          else
            {
              Glib::ustring errmsg = Glib::strerror(errno);
              TimedMessageDialog
                td(d,
                   String::ucompose(_("Couldn't add %1 to :\n%2\n%3"),
                                    d.get_filename (),
                                    t->getConfigurationFile(), errmsg), 0);
              td.run_and_hide ();
            }
        }
    }
  else if (res == Gtk::RESPONSE_REJECT)
    {
      if (sndname.empty () == false)
        {
          if (t->removeFileInCfgFile(sndname + ".ogg"))
            {
              setName ("");
              d_needs_saving = true;
              fill_in_buttons();
            }
          else
            {
              Glib::ustring errmsg = Glib::strerror(errno);
              TimedMessageDialog
                td(d,
                   String::ucompose(_("Couldn't remove %1 from:\n%2\n%3"),
                                    sndname + ".ogg",
                                    t->getConfigurationFile(),
                                    errmsg), 0);
              td.run_and_hide ();
            }
        }
    }
  d.hide ();
}

void MediaDialog::on_next_turn_button_activated()
{
  ScenarioMedia *sm = ScenarioMedia::getInstance();
  ImageCache *ic = ImageCache::getInstance ();
  on_image_button_activated (ic->getNextTurnImage (),
                             sm->getNextTurnImage ());
}

void MediaDialog::on_city_defeated_button_activated()
{
  ScenarioMedia *sm = ScenarioMedia::getInstance();
  ImageCache *ic = ImageCache::getInstance ();
  on_image_button_activated (ic->getCityDefeatedImage (),
                             sm->getCityDefeatedImage ());
}

void MediaDialog::on_winning_button_activated()
{
  ScenarioMedia *sm = ScenarioMedia::getInstance();
  ImageCache *ic = ImageCache::getInstance ();
  on_image_button_activated (ic->getWinningImage (),
                             sm->getWinningImage ());
}

void MediaDialog::on_hero_male_button_activated()
{
  ScenarioMedia *sm = ScenarioMedia::getInstance();
  ImageCache *ic = ImageCache::getInstance ();
  on_image_button_activated (ic->getHeroOfferedImage (Hero::MALE),
                             sm->getHeroOfferedImage (false));
}

void MediaDialog::on_hero_female_button_activated()
{
  ScenarioMedia *sm = ScenarioMedia::getInstance();
  ImageCache *ic = ImageCache::getInstance ();
  on_image_button_activated (ic->getHeroOfferedImage (Hero::MALE),
                             sm->getHeroOfferedImage (true));
}

void MediaDialog::on_ruin_success_button_activated()
{
  ScenarioMedia *sm = ScenarioMedia::getInstance();
  ImageCache *ic = ImageCache::getInstance ();
  on_image_button_activated (ic->getRuinSuccessImage (),
                             sm->getRuinSuccessImage ());
}

void MediaDialog::on_ruin_defeat_button_activated()
{
  ScenarioMedia *sm = ScenarioMedia::getInstance();
  ImageCache *ic = ImageCache::getInstance ();
  on_image_button_activated (ic->getRuinDefeatImage (),
                             sm->getRuinDefeatImage ());
}

void MediaDialog::on_hero_newlevel_male_button_activated()
{
  ScenarioMedia *sm = ScenarioMedia::getInstance();
  TarFileMaskedImage *omim =
    ImageCache::getInstance ()->getHeroNewLevelMaskedImage(false);
  TarFileMaskedImage *mim = sm->getHeroNewLevelMaskedImage(false);
  on_masked_image_button_activated
    (sigc::mem_fun (mim, &TarFileMaskedImage::getName),
     omim, sigc::mem_fun (mim, &TarFileMaskedImage::setName),
     Shieldsetlist::getInstance()->get(Playerlist::getActiveplayer()->getId()));
  if (mim->getName ().empty () == false)
    {
      mim->load (d_tarfile, mim->getName ());
      mim->instantiateImages ();
    }
  else
    sm->getHeroNewLevelMaskedImage(false)->clear();
}

void MediaDialog::on_hero_newlevel_female_button_activated()
{
  ScenarioMedia *sm = ScenarioMedia::getInstance();
  TarFileMaskedImage *omim =
    ImageCache::getInstance ()->getHeroNewLevelMaskedImage(true);
  TarFileMaskedImage *mim = sm->getHeroNewLevelMaskedImage(true);
  on_masked_image_button_activated
    (sigc::mem_fun (mim, &TarFileMaskedImage::getName),
     omim, sigc::mem_fun (mim, &TarFileMaskedImage::setName),
     Shieldsetlist::getInstance()->get(Playerlist::getActiveplayer()->getId()));
  if (mim->getName().empty () == false)
    {
      mim->load (d_tarfile, mim->getName ());
      mim->instantiateImages ();
    }
  else
    sm->getHeroNewLevelMaskedImage(true)->clear();
}

void MediaDialog::on_parley_offered_button_activated()
{
  ScenarioMedia *sm = ScenarioMedia::getInstance();
  ImageCache *ic = ImageCache::getInstance ();
  on_image_button_activated (ic->getParleyOfferedImage (),
                             sm->getParleyOfferedImage ());
}

void MediaDialog::on_parley_refused_button_activated()
{
  ScenarioMedia *sm = ScenarioMedia::getInstance();
  ImageCache *ic = ImageCache::getInstance ();
  on_image_button_activated (ic->getParleyRefusedImage (),
                             sm->getParleyRefusedImage ());
}

void MediaDialog::on_small_medals_button_activated()
{
  ScenarioMedia *sm = ScenarioMedia::getInstance();
  ImageCache *ic = ImageCache::getInstance ();
  on_image_button_activated (ic->getMedalImage (false),
                             sm->getMedalImage (false));
}

void MediaDialog::on_big_medals_button_activated()
{
  ScenarioMedia *sm = ScenarioMedia::getInstance();
  ImageCache *ic = ImageCache::getInstance ();
  on_image_button_activated (ic->getMedalImage (true),
                             sm->getMedalImage (true));
}

void MediaDialog::on_commentator_button_activated()
{
  ScenarioMedia *sm = ScenarioMedia::getInstance();
  ImageCache *ic = ImageCache::getInstance ();
  on_image_button_activated (ic->getCommentatorImage (),
                             sm->getCommentatorImage ());
}

void MediaDialog::on_bless_button_activated()
{
  ScenarioMedia *sm = ScenarioMedia::getInstance();
  on_sound_button_activated
    (sigc::mem_fun(sm, &ScenarioMedia::getBlessSoundName),
     sigc::ptr_fun (&ScenarioMedia::getDefaultBlessSoundFilename),
     sigc::mem_fun (sm, &ScenarioMedia::setBlessSoundName));
}

void MediaDialog::on_hero_button_activated()
{
  ScenarioMedia *sm = ScenarioMedia::getInstance();
  on_sound_button_activated
    (sigc::mem_fun(sm, &ScenarioMedia::getHeroSoundName),
     sigc::ptr_fun (&ScenarioMedia::getDefaultHeroSoundFilename),
     sigc::mem_fun (sm, &ScenarioMedia::setHeroSoundName));
}

void MediaDialog::on_battle_button_activated()
{
  ScenarioMedia *sm = ScenarioMedia::getInstance();
  on_sound_button_activated
    (sigc::mem_fun(sm, &ScenarioMedia::getBattleSoundName),
     sigc::ptr_fun (&ScenarioMedia::getDefaultBattleSoundFilename),
     sigc::mem_fun (sm, &ScenarioMedia::setBattleSoundName));
}

void MediaDialog::on_defeat_button_activated()
{
  ScenarioMedia *sm = ScenarioMedia::getInstance();
  on_sound_button_activated
    (sigc::mem_fun(sm, &ScenarioMedia::getDefeatSoundName),
     sigc::ptr_fun (&ScenarioMedia::getDefaultDefeatSoundFilename),
     sigc::mem_fun (sm, &ScenarioMedia::setDefeatSoundName));
}

void MediaDialog::on_victory_button_activated()
{
  ScenarioMedia *sm = ScenarioMedia::getInstance();
  on_sound_button_activated
    (sigc::mem_fun(sm, &ScenarioMedia::getVictorySoundName),
     sigc::ptr_fun (&ScenarioMedia::getDefaultVictorySoundFilename),
     sigc::mem_fun (sm, &ScenarioMedia::setVictorySoundName));
}

void MediaDialog::on_back_button_activated()
{
  ScenarioMedia *sm = ScenarioMedia::getInstance();
  on_sound_button_activated
    (sigc::mem_fun(sm, &ScenarioMedia::getBackSoundName),
     sigc::ptr_fun (&ScenarioMedia::getDefaultBackSoundFilename),
     sigc::mem_fun (sm, &ScenarioMedia::setBackSoundName));
}

MediaDialog::~MediaDialog()
{
  notebook->property_show_tabs () = false;
}
