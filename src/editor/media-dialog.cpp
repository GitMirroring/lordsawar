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
#include "masked-image-editor-dialog.h"
#include "File.h"
#include "defs.h"
#include "tarfile.h"
#include "shieldsetlist.h"
#include "shieldset.h"
#include "playerlist.h"

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
    button->set_label (name + ".png");
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
  fill_image_button (d_next_turn_button, sm->getNextTurnImageName());
  fill_image_button (d_city_defeated_button, sm->getCityDefeatedImageName());
  fill_image_button (d_winning_button, sm->getWinningImageName());
  fill_image_button (d_hero_male_button, sm->getMaleHeroImageName());
  fill_image_button (d_hero_female_button, sm->getFemaleHeroImageName());
  fill_image_button (d_ruin_success_button, sm->getRuinSuccessImageName());
  fill_image_button (d_ruin_defeat_button, sm->getRuinDefeatImageName());
  fill_image_button (d_hero_newlevel_male_button,
                     sm->getHeroNewLevelMaleImageName());
  fill_image_button (d_hero_newlevel_female_button,
                     sm->getHeroNewLevelFemaleImageName());
  fill_image_button (d_parley_offered_button, sm->getParleyOfferedImageName());
  fill_image_button (d_parley_refused_button, sm->getParleyRefusedImageName());
  fill_image_button (d_small_medals_button, sm->getSmallMedalsImageName());
  fill_image_button (d_big_medals_button, sm->getBigMedalsImageName());

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

void MediaDialog::on_image_button_activated(sigc::slot<Glib::ustring> getName, sigc::slot<Glib::ustring> getDefaultFilename, sigc::slot<void,Glib::ustring> setName, int num_frames)
{
  TarFile *t = d_tarfile;
  Glib::ustring oldfile = getName() == "" ?
    getDefaultFilename() : t->getFileFromConfigurationFile(getName());

  ImageEditorDialog d (*dialog, oldfile, num_frames);
  int response = d.run();
  if (response == Gtk::RESPONSE_ACCEPT)
    {
      Glib::ustring newfile = d.get_selected_filename();
      d.hide();
      if (File::exists (newfile) == false)
        return;
      Glib::ustring bname = File::get_basename(newfile, true);
      if (getName() == "")
        t->addFileInConfigurationFile(newfile);
      else
        t->replaceFileInConfigurationFile(bname, newfile);
      setName(File::get_basename (bname, false));
      if (newfile == getDefaultFilename())
        {
          t->removeFileInConfigurationFile(bname);
          setName("");
        }
      d_needs_saving = true;
      fill_in_buttons();
    }
}

void MediaDialog::on_masked_image_button_activated(sigc::slot<Glib::ustring> getName, sigc::slot<Glib::ustring> getDefaultFilename, sigc::slot<void,Glib::ustring> setName, Shieldset *ss)
{
  TarFile *t = d_tarfile;
  Glib::ustring oldfile = getName() == "" ?
    getDefaultFilename() : t->getFileFromConfigurationFile(getName());

  MaskedImageEditorDialog d (*dialog, oldfile, ss);
  int response = d.run();
  if (response == Gtk::RESPONSE_ACCEPT)
    {
      Glib::ustring newfile = d.get_selected_filename();
      d.hide();
      if (File::exists (newfile) == false)
        return;
      Glib::ustring bname = File::get_basename(newfile, true);
      if (getName() == "")
        t->addFileInConfigurationFile(newfile);
      else
        t->replaceFileInConfigurationFile(bname, newfile);
      setName(File::get_basename (bname, false));
      if (newfile == getDefaultFilename())
        {
          t->removeFileInConfigurationFile(bname);
          setName("");
        }
      d_needs_saving = true;
      fill_in_buttons();
    }
}

void MediaDialog::on_sound_button_activated(sigc::slot<Glib::ustring> getName, sigc::slot<Glib::ustring> getDefaultFilename, sigc::slot<void, Glib::ustring> setName)
{
  TarFile *t = d_tarfile;
  Gtk::FileChooserDialog chooser(*dialog, _("Choose Sound File"));
  Glib::RefPtr<Gtk::FileFilter> map_filter = Gtk::FileFilter::create();
  map_filter->set_name(_("Sound Files (*.ogg)"));
  map_filter->add_pattern("*.ogg");
  chooser.add_filter(map_filter);

  Glib::ustring oldfile = getDefaultFilename();
  if (getName() != "")
    oldfile = t->getFileFromConfigurationFile(getName());
  chooser.set_current_folder(File::get_dirname (oldfile));
  chooser.set_filename(oldfile);

  chooser.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
  chooser.add_button(Gtk::Stock::OPEN, Gtk::RESPONSE_ACCEPT);
  chooser.set_default_response(Gtk::RESPONSE_ACCEPT);

  chooser.show_all();
  int res = chooser.run();

  if (res == Gtk::RESPONSE_ACCEPT)
    {
      Glib::ustring newfile = chooser.get_filename();
      chooser.hide();
      if (File::exists (newfile) == false)
        return;
      Glib::ustring bname = File::get_basename(newfile, true);
      if (getName() == "")
        t->addFileInConfigurationFile(newfile);
      else
        t->replaceFileInConfigurationFile(bname, newfile);
      setName(File::get_basename (bname, false));
      if (newfile == getDefaultFilename())
        {
          t->removeFileInConfigurationFile(bname);
          setName("");
        }
      fill_in_buttons();
      d_needs_saving = true;
    }
}

void MediaDialog::on_next_turn_button_activated()
{
  ScenarioMedia *sm = ScenarioMedia::getInstance();
  on_image_button_activated
    (sigc::mem_fun (sm, &ScenarioMedia::getNextTurnImageName),
     sigc::ptr_fun (&ScenarioMedia::getDefaultNextTurnImageFilename),
     sigc::mem_fun (sm, &ScenarioMedia::setNextTurnImageName), 1);
}

void MediaDialog::on_city_defeated_button_activated()
{
  ScenarioMedia *sm = ScenarioMedia::getInstance();
  on_image_button_activated
    (sigc::mem_fun (sm, &ScenarioMedia::getCityDefeatedImageName),
     sigc::ptr_fun (&ScenarioMedia::getDefaultCityDefeatedImageFilename),
     sigc::mem_fun (sm, &ScenarioMedia::setCityDefeatedImageName), 1);
}

void MediaDialog::on_winning_button_activated()
{
  ScenarioMedia *sm = ScenarioMedia::getInstance();
  on_image_button_activated
    (sigc::mem_fun (sm, &ScenarioMedia::getWinningImageName),
     sigc::ptr_fun (&ScenarioMedia::getDefaultWinningImageFilename),
     sigc::mem_fun (sm, &ScenarioMedia::setWinningImageName), 1);
}

void MediaDialog::on_hero_male_button_activated()
{
  ScenarioMedia *sm = ScenarioMedia::getInstance();
  on_image_button_activated
    (sigc::mem_fun (sm, &ScenarioMedia::getMaleHeroImageName),
     sigc::ptr_fun (&ScenarioMedia::getDefaultMaleHeroImageFilename),
     sigc::mem_fun (sm, &ScenarioMedia::setMaleHeroImageName), 1);
}

void MediaDialog::on_hero_female_button_activated()
{
  ScenarioMedia *sm = ScenarioMedia::getInstance();
  on_image_button_activated
    (sigc::mem_fun (sm, &ScenarioMedia::getFemaleHeroImageName),
     sigc::ptr_fun (&ScenarioMedia::getDefaultFemaleHeroImageFilename),
     sigc::mem_fun (sm, &ScenarioMedia::setFemaleHeroImageName), 1);
}

void MediaDialog::on_ruin_success_button_activated()
{
  ScenarioMedia *sm = ScenarioMedia::getInstance();
  on_image_button_activated
    (sigc::mem_fun (sm, &ScenarioMedia::getRuinSuccessImageName),
     sigc::ptr_fun (&ScenarioMedia::getDefaultRuinSuccessImageFilename),
     sigc::mem_fun (sm, &ScenarioMedia::setRuinSuccessImageName), 1);
}

void MediaDialog::on_ruin_defeat_button_activated()
{
  ScenarioMedia *sm = ScenarioMedia::getInstance();
  on_image_button_activated
    (sigc::mem_fun (sm, &ScenarioMedia::getRuinDefeatImageName),
     sigc::ptr_fun (&ScenarioMedia::getDefaultRuinDefeatImageFilename),
     sigc::mem_fun (sm, &ScenarioMedia::setRuinDefeatImageName), 1);
}

void MediaDialog::on_hero_newlevel_male_button_activated()
{
  ScenarioMedia *sm = ScenarioMedia::getInstance();
  on_masked_image_button_activated
    (sigc::mem_fun (sm, &ScenarioMedia::getHeroNewLevelMaleImageName),
     sigc::ptr_fun (&ScenarioMedia::getDefaultHeroNewLevelMaleImageFilename),
     sigc::mem_fun (sm, &ScenarioMedia::setHeroNewLevelMaleImageName),
     Shieldsetlist::getInstance()->get(Playerlist::getActiveplayer()->getId()));
}

void MediaDialog::on_hero_newlevel_female_button_activated()
{
  ScenarioMedia *sm = ScenarioMedia::getInstance();
  on_masked_image_button_activated
    (sigc::mem_fun (sm, &ScenarioMedia::getHeroNewLevelFemaleImageName),
     sigc::ptr_fun (&ScenarioMedia::getDefaultHeroNewLevelFemaleImageFilename),
     sigc::mem_fun (sm, &ScenarioMedia::setHeroNewLevelFemaleImageName),
     Shieldsetlist::getInstance()->get(Playerlist::getActiveplayer()->getId()));
}

void MediaDialog::on_parley_offered_button_activated()
{
  ScenarioMedia *sm = ScenarioMedia::getInstance();
  on_image_button_activated
    (sigc::mem_fun (sm, &ScenarioMedia::getParleyOfferedImageName),
     sigc::ptr_fun (&ScenarioMedia::getDefaultParleyOfferedImageFilename),
     sigc::mem_fun (sm, &ScenarioMedia::setParleyOfferedImageName), 1);
}

void MediaDialog::on_parley_refused_button_activated()
{
  ScenarioMedia *sm = ScenarioMedia::getInstance();
  on_image_button_activated
    (sigc::mem_fun (sm, &ScenarioMedia::getParleyRefusedImageName),
     sigc::ptr_fun (&ScenarioMedia::getDefaultParleyRefusedImageFilename),
     sigc::mem_fun (sm, &ScenarioMedia::setParleyRefusedImageName), 1);
}

void MediaDialog::on_small_medals_button_activated()
{
  ScenarioMedia *sm = ScenarioMedia::getInstance();
  on_image_button_activated
    (sigc::mem_fun (sm, &ScenarioMedia::getSmallMedalsImageName),
     sigc::ptr_fun (&ScenarioMedia::getDefaultSmallMedalsImageFilename),
     sigc::mem_fun (sm, &ScenarioMedia::setSmallMedalsImageName), MEDAL_TYPES);
}

void MediaDialog::on_big_medals_button_activated()
{
  ScenarioMedia *sm = ScenarioMedia::getInstance();
  on_image_button_activated
    (sigc::mem_fun (sm, &ScenarioMedia::getBigMedalsImageName),
     sigc::ptr_fun (&ScenarioMedia::getDefaultBigMedalsImageFilename),
     sigc::mem_fun (sm, &ScenarioMedia::setBigMedalsImageName), MEDAL_TYPES);
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
