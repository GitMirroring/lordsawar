//  Copyright (C) 2017, 2020, 2021, 2026 Ben Asselstine
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
#include "lw-dialog-base.h"
#ifndef MEDIA_DIALOG_H
#define MEDIA_DIALOG_H
#include "media-undo-actions.h"
#include "file-label.h"
#include "tar-file-sound.h"
#include "lw-dialog.h"
#include "image-editor-dialog.h"
#include "masked-image-editor-dialog.h"
class MediaDialog: public LwDialogBase
{
public:
    static std::string get_resource_name ()
      {
        return "media.ui";
      }

    MediaDialog (BaseObjectType* o,
                 const Glib::RefPtr<Gtk::Builder>& xml)
      : LwDialogBase (o, xml)
      {
        m_close_button = load <Gtk::Button> ("close_button");
        m_next_turn_button = load <Gtk::Button> ("next_turn_button");
        m_city_defeated_button = load <Gtk::Button> ("city_defeated_button");
        m_winning_button = load <Gtk::Button> ("winning_button");
        m_hero_male_button = load <Gtk::Button> ("hero_male_button");
        m_hero_female_button = load <Gtk::Button> ("hero_female_button");
        m_parley_offered_button = load <Gtk::Button> ("parley_offered_button");
        m_ruin_succeed_button = load <Gtk::Button> ("ruin_succeed_button");
        m_ruin_defeat_button = load <Gtk::Button> ("ruin_defeat_button");
        m_parley_refused_button = load <Gtk::Button> ("parley_refused_button");
        m_hero_newlevel_male_button =
          load <Gtk::Button> ("hero_newlevel_male_button");
        m_hero_newlevel_female_button =
          load <Gtk::Button> ("hero_newlevel_female_button");
        m_commentator_button = load <Gtk::Button> ("commentator_button");
        m_small_medals_button = load <Gtk::Button> ("small_medals_button");
        m_big_medals_button = load <Gtk::Button> ("big_medals_button");
        m_bless_button = load <Gtk::Button> ("bless_button");
        m_hero_button = load <Gtk::Button> ("hero_button");
        m_battle_button = load <Gtk::Button> ("battle_button");
        m_defeat_button = load <Gtk::Button> ("defeat_button");
        m_victory_button = load <Gtk::Button> ("victory_button");
        m_back_button = load <Gtk::Button> ("back_button");
        m_clear_bless_button = load <Gtk::Button> ("clear_bless_button");
        m_clear_hero_button = load <Gtk::Button> ("clear_hero_button");
        m_clear_battle_button = load <Gtk::Button> ("clear_battle_button");
        m_clear_defeat_button = load <Gtk::Button> ("clear_defeat_button");
        m_clear_victory_button = load <Gtk::Button> ("clear_victory_button");
        m_clear_back_button = load <Gtk::Button> ("clear_back_button");
      }

    ~MediaDialog ()
      {
        disconnect_signals ();
        delete m_umgr;
      }

    void setup (TarFile *tf)
      {
        m_tarfile = tf;
        set_response (m_close_button, Gtk::ResponseType::ACCEPT);

        m_next_turn_filelabel =
          Gtk::make_managed<FileLabel> (m_next_turn_button);
        m_next_turn_filelabel->set_override ();
        m_next_turn_filelabel->clear ();

        m_city_defeated_filelabel =
          Gtk::make_managed<FileLabel> (m_city_defeated_button);
        m_city_defeated_filelabel->set_override ();
        m_city_defeated_filelabel->clear ();

        m_winning_filelabel = Gtk::make_managed<FileLabel> (m_winning_button);
        m_winning_filelabel->set_override ();
        m_winning_filelabel->clear ();

        m_hero_male_filelabel =
          Gtk::make_managed<FileLabel> (m_hero_male_button);
        m_hero_male_filelabel->set_override ();
        m_hero_male_filelabel->clear ();

        m_hero_female_filelabel =
          Gtk::make_managed<FileLabel> (m_hero_female_button);
        m_hero_female_filelabel->set_override ();
        m_hero_female_filelabel->clear ();

        m_parley_offered_filelabel =
          Gtk::make_managed<FileLabel> (m_parley_offered_button);
        m_parley_offered_filelabel->set_override ();
        m_parley_offered_filelabel->clear ();

        m_ruin_succeed_filelabel =
          Gtk::make_managed<FileLabel> (m_ruin_succeed_button);
        m_ruin_succeed_filelabel->set_override ();
        m_ruin_succeed_filelabel->clear ();

        m_ruin_defeat_filelabel =
          Gtk::make_managed<FileLabel> (m_ruin_defeat_button);
        m_ruin_defeat_filelabel->set_override ();
        m_ruin_defeat_filelabel->clear ();

        m_parley_refused_filelabel =
          Gtk::make_managed<FileLabel> (m_parley_refused_button);
        m_parley_refused_filelabel->set_override ();
        m_parley_refused_filelabel->clear ();

        m_hero_newlevel_male_filelabel =
          Gtk::make_managed<FileLabel> (m_hero_newlevel_male_button);
        m_hero_newlevel_male_filelabel->set_override ();
        m_hero_newlevel_male_filelabel->clear ();

        m_hero_newlevel_female_filelabel =
          Gtk::make_managed<FileLabel> (m_hero_newlevel_female_button);
        m_hero_newlevel_female_filelabel->set_override ();
        m_hero_newlevel_female_filelabel->clear ();

        m_commentator_filelabel =
          Gtk::make_managed<FileLabel> (m_commentator_button);
        m_commentator_filelabel->set_override ();
        m_commentator_filelabel->clear ();

        m_small_medals_filelabel =
          Gtk::make_managed<FileLabel> (m_small_medals_button);
        m_small_medals_filelabel->set_override ();
        m_small_medals_filelabel->clear ();

        m_big_medals_filelabel =
          Gtk::make_managed<FileLabel> (m_big_medals_button);
        m_big_medals_filelabel->set_override ();
        m_big_medals_filelabel->clear ();

        m_bless_filelabel =
          Gtk::make_managed<FileLabel> (m_bless_button, "", FileLabel::SOUND);
        m_bless_filelabel->set_override ();
        m_bless_filelabel->clear ();

        m_hero_filelabel =
          Gtk::make_managed<FileLabel> (m_hero_button, "", FileLabel::SOUND);
        m_hero_filelabel->set_override ();
        m_hero_filelabel->clear ();

        m_battle_filelabel =
          Gtk::make_managed<FileLabel> (m_battle_button, "", FileLabel::SOUND);
        m_battle_filelabel->set_override ();
        m_battle_filelabel->clear ();

        m_defeat_filelabel =
          Gtk::make_managed<FileLabel> (m_defeat_button, "", FileLabel::SOUND);
        m_defeat_filelabel->set_override ();
        m_defeat_filelabel->clear ();

        m_victory_filelabel =
          Gtk::make_managed<FileLabel> (m_victory_button, "", FileLabel::SOUND);
        m_victory_filelabel->set_override ();
        m_victory_filelabel->clear ();

        m_back_filelabel =
          Gtk::make_managed<FileLabel> (m_back_button, "", FileLabel::SOUND);
        m_back_filelabel->set_override ();
        m_back_filelabel->clear ();

        setup_undo ();
        update ();
      }

    bool is_changed ()
      {
        return m_umgr->undo_empty () == false;
      }
private:
    Gtk::Button *m_close_button;

    Gtk::Button *m_next_turn_button;
    FileLabel *m_next_turn_filelabel;

    Gtk::Button *m_city_defeated_button;
    FileLabel *m_city_defeated_filelabel;

    Gtk::Button *m_winning_button;
    FileLabel *m_winning_filelabel;

    Gtk::Button *m_hero_male_button;
    FileLabel *m_hero_male_filelabel;

    Gtk::Button *m_hero_female_button;
    FileLabel *m_hero_female_filelabel;

    Gtk::Button *m_parley_offered_button;
    FileLabel *m_parley_offered_filelabel;

    Gtk::Button *m_ruin_succeed_button;
    FileLabel *m_ruin_succeed_filelabel;

    Gtk::Button *m_ruin_defeat_button;
    FileLabel *m_ruin_defeat_filelabel;

    Gtk::Button *m_parley_refused_button;
    FileLabel *m_parley_refused_filelabel;

    Gtk::Button *m_hero_newlevel_male_button;
    FileLabel *m_hero_newlevel_male_filelabel;

    Gtk::Button *m_hero_newlevel_female_button;
    FileLabel *m_hero_newlevel_female_filelabel;

    Gtk::Button *m_commentator_button;
    FileLabel *m_commentator_filelabel;

    Gtk::Button *m_small_medals_button;
    FileLabel *m_small_medals_filelabel;

    Gtk::Button *m_big_medals_button;
    FileLabel *m_big_medals_filelabel;

    Gtk::Button *m_bless_button;
    FileLabel *m_bless_filelabel;

    Gtk::Button *m_hero_button;
    FileLabel *m_hero_filelabel;

    Gtk::Button *m_battle_button;
    FileLabel *m_battle_filelabel;

    Gtk::Button *m_defeat_button;
    FileLabel *m_defeat_filelabel;

    Gtk::Button *m_victory_button;
    FileLabel *m_victory_filelabel;

    Gtk::Button *m_back_button;
    FileLabel *m_back_filelabel;

    Gtk::Button *m_clear_bless_button;
    Gtk::Button *m_clear_hero_button;
    Gtk::Button *m_clear_battle_button;
    Gtk::Button *m_clear_defeat_button;
    Gtk::Button *m_clear_victory_button;
    Gtk::Button *m_clear_back_button;

    TarFile *m_tarfile;
    UndoMgr *m_umgr;
    std::list<sigc::connection> m_connections;

    void add_connection (sigc::connection c)
      {
        m_connections.push_back (c);
      }

    void update ()
      {
        disconnect_signals ();

        ScenarioMedia *sm = ScenarioMedia::instance ();
        m_next_turn_filelabel->set_label
          (sm->getNextTurnImage ()->getName ());
        m_city_defeated_filelabel->set_label
          (sm->getCityDefeatedImage ()->getName ());
        m_winning_filelabel->set_label (sm->getWinningImage ()->getName ());
        m_hero_male_filelabel->set_label
          (sm->getHeroOfferedImage (false)->getName ());
        m_hero_female_filelabel->set_label
          (sm->getHeroOfferedImage (true)->getName ());
        m_ruin_succeed_filelabel->set_label
          (sm->getRuinSuccessImage ()->getName ());
        m_ruin_defeat_filelabel->set_label
          (sm->getRuinDefeatImage ()->getName ());
        m_hero_newlevel_male_filelabel->set_label
          (sm->getHeroNewLevelMaskedImage (false)->getName ());
        m_hero_newlevel_female_filelabel->set_label
          (sm->getHeroNewLevelMaskedImage (true)->getName ());
        m_parley_offered_filelabel->set_label
          (sm->getParleyOfferedImage ()->getName ());
        m_parley_refused_filelabel->set_label
          (sm->getParleyRefusedImage ()->getName ());
        m_small_medals_filelabel->set_label
          (sm->getMedalImage (false)->getName ());
        m_big_medals_filelabel->set_label
          (sm->getMedalImage (true)->getName ());
        m_commentator_filelabel->set_label
          (sm->getCommentatorImage ()->getName ());

        m_bless_filelabel->set_label (sm->getBlessSound ()->get_name ());
        m_hero_filelabel->set_label (sm->getHeroSound ()->get_name ());
        m_battle_filelabel->set_label (sm->getBattleSound ()->get_name ());
        m_defeat_filelabel->set_label (sm->getDefeatSound ()->get_name ());
        m_victory_filelabel->set_label (sm->getVictorySound ()->get_name ());
        m_back_filelabel->set_label (sm->getBackSound ()->get_name ());

        update_buttons ();

        connect_signals ();
      }

    void error_couldnt_remove_file (Glib::ustring err)
      {
        Glib::ustring emsg = _("Couldn't remove file!");
        auto dialog = LwDialog::alert (emsg, err);
        dialog->choose
          (*this,
           [this, dialog] (auto result)
           {
             dialog->choose_finish (result);
             return;
           });
      }

    void connect_signals ()
      {
        add_connection
          (m_clear_bless_button->signal_clicked ().connect
           ([this] ()
            {
              clear_sound (ScenarioMedia::instance ()->getBlessSound (),
                           m_bless_filelabel);
            }));

        add_connection
          (m_clear_hero_button->signal_clicked ().connect
           ([this] ()
            {
              clear_sound (ScenarioMedia::instance ()->getHeroSound (),
                           m_hero_filelabel);
            }));

        add_connection
          (m_clear_battle_button->signal_clicked ().connect
           ([this] ()
            {
              clear_sound (ScenarioMedia::instance ()->getBattleSound (),
                           m_battle_filelabel);
            }));

        add_connection
          (m_clear_defeat_button->signal_clicked ().connect
           ([this] ()
            {
              clear_sound (ScenarioMedia::instance ()->getDefeatSound (),
                           m_defeat_filelabel);
            }));

        add_connection
          (m_clear_victory_button->signal_clicked ().connect
           ([this] ()
            {
              clear_sound (ScenarioMedia::instance ()->getVictorySound (),
                           m_victory_filelabel);
            }));

        add_connection
          (m_clear_back_button->signal_clicked ().connect
           ([this] ()
            {
              clear_sound (ScenarioMedia::instance ()->getBackSound (),
                           m_back_filelabel);
            }));

        add_connection
          (m_next_turn_button->signal_clicked ().connect
           ([this] ()
            {
              change_image (ScenarioMedia::instance ()->getNextTurnImage (),
                            ImageCache::instance ()->getNextTurnImage (),
                            _("Select Next Turn Image"));
            }));

        add_connection
          (m_city_defeated_button->signal_clicked ().connect
           ([this] ()
            {
              change_image (ScenarioMedia::instance ()->getCityDefeatedImage (),
                            ImageCache::instance ()->getCityDefeatedImage (),
                            _("Select City Conquered Image"));
            }));

        add_connection
          (m_winning_button->signal_clicked ().connect
           ([this] ()
            {
              change_image (ScenarioMedia::instance ()->getWinningImage (),
                            ImageCache::instance ()->getWinningImage (),
                            _("Select Won Game Image"));
            }));

        add_connection
          (m_hero_male_button->signal_clicked ().connect
           ([this] ()
            {
              change_image
                (ScenarioMedia::instance ()->getHeroOfferedImage (false),
                 ImageCache::instance ()->getHeroOfferedImage (Hero::MALE),
                 _("Select New Male Hero Image"));
            }));

        add_connection
          (m_hero_female_button->signal_clicked ().connect
           ([this] ()
            {
              change_image
                (ScenarioMedia::instance ()->getHeroOfferedImage (true),
                 ImageCache::instance ()->getHeroOfferedImage (Hero::FEMALE),
                 _("Select New Female Hero Image"));
            }));

        add_connection
          (m_parley_offered_button->signal_clicked ().connect
           ([this] ()
            {
              change_image
                (ScenarioMedia::instance ()->getParleyOfferedImage (),
                 ImageCache::instance ()->getParleyOfferedImage (),
                 _("Select Enemies Offer Parley Image"));
            }));

        add_connection
          (m_ruin_succeed_button->signal_clicked ().connect
           ([this] ()
            {
              change_image (ScenarioMedia::instance ()->getRuinSuccessImage (),
                            ImageCache::instance ()->getRuinSuccessImage (),
                            _("Select Hero Searches Ruin Image"));
            }));

        add_connection
          (m_ruin_defeat_button->signal_clicked ().connect
           ([this] ()
            {
              change_image (ScenarioMedia::instance ()->getRuinDefeatImage (),
                            ImageCache::instance ()->getRuinDefeatImage (),
                            _("Select Hero Dies At Ruin Image"));
            }));

        add_connection
          (m_parley_refused_button->signal_clicked ().connect
           ([this] ()
            {
              change_image (ScenarioMedia::instance ()->getParleyRefusedImage (),
                            ImageCache::instance ()->getParleyRefusedImage (),
                            _("Select Enemy Parley Refused Image"));
            }));

        add_connection
          (m_hero_newlevel_male_button->signal_clicked ().connect
           ([this] ()
            {
              change_image
                (ScenarioMedia::instance ()->getHeroNewLevelMaskedImage (false),
                 ImageCache::instance ()->getHeroNewLevelMaskedImage (false),
                 _("Select Male Hero New Level Image"));
            }));

        add_connection
          (m_hero_newlevel_female_button->signal_clicked ().connect
           ([this] ()
            {
              change_image
                (ScenarioMedia::instance ()->getHeroNewLevelMaskedImage (true),
                 ImageCache::instance ()->getHeroNewLevelMaskedImage (true),
                 _("Select Female Hero New Level Image"));
            }));

        add_connection
          (m_commentator_button->signal_clicked ().connect
           ([this] ()
            {
              change_image (ScenarioMedia::instance ()->getCommentatorImage (),
                            ImageCache::instance ()->getCommentatorImage (),
                            _("Select Commentator Image"));
            }));

        add_connection
          (m_small_medals_button->signal_clicked ().connect
           ([this] ()
            {
              change_image (ScenarioMedia::instance ()->getMedalImage (false),
                            ImageCache::instance ()->getMedalImage (false),
                            _("Select Small Medals Image"));
            }));

        add_connection
          (m_big_medals_button->signal_clicked ().connect
           ([this] ()
            {
              change_image (ScenarioMedia::instance ()->getMedalImage (true),
                            ImageCache::instance ()->getMedalImage (true),
                            _("Select Big Medals Image"));
            }));

        add_connection
          (m_bless_button->signal_clicked ().connect
           ([this] ()
            {
              ScenarioMedia *sm = ScenarioMedia::instance ();
              on_sound_button_activated
                (sm->getBlessSound (),
                 ScenarioMedia::getDefaultBlessSoundFilename ());
            }));

        add_connection
          (m_hero_button->signal_clicked ().connect
           ([this] ()
            {
              ScenarioMedia *sm = ScenarioMedia::instance ();
              on_sound_button_activated
                (sm->getHeroSound (),
                 ScenarioMedia::getDefaultHeroSoundFilename ());
            }));

        add_connection
          (m_battle_button->signal_clicked ().connect
           ([this] ()
            {
              ScenarioMedia *sm = ScenarioMedia::instance ();
              on_sound_button_activated
                (sm->getBattleSound (),
                 ScenarioMedia::getDefaultBattleSoundFilename ());
            }));

        add_connection
          (m_defeat_button->signal_clicked ().connect
           ([this] ()
            {
              ScenarioMedia *sm = ScenarioMedia::instance ();
              on_sound_button_activated
                (sm->getDefeatSound (),
                 ScenarioMedia::getDefaultDefeatSoundFilename ());
            }));

        add_connection
          (m_victory_button->signal_clicked ().connect
           ([this] ()
            {
              ScenarioMedia *sm = ScenarioMedia::instance ();
              on_sound_button_activated
                (sm->getVictorySound (),
                 ScenarioMedia::getDefaultVictorySoundFilename ());
            }));

        add_connection
          (m_back_button->signal_clicked ().connect
           ([this] ()
            {
              ScenarioMedia *sm = ScenarioMedia::instance ();
              on_sound_button_activated
                (sm->getBackSound (),
                 ScenarioMedia::getDefaultBackSoundFilename ());
            }));
      }

    void change_image (TarFileImage *im, TarFileImage *oim, Glib::ustring msg)
      {
        auto action =
          new MediaUndoAction_ImageSet
          (m_tarfile, im->getName (), im);

        on_image_button_activated
          (oim, im, msg,
           [this, action] (bool changed)
           {
             if (changed)
               m_umgr->add (action);
             else
               delete action;
           });
      }


    void change_image (TarFileMaskedImage *im, TarFileMaskedImage *oim,
                       Glib::ustring msg)
      {
        auto action =
          new MediaUndoAction_MaskedImageSet
          (m_tarfile, im->getName (), im);

        on_masked_image_button_activated
          (oim, im, msg,
           [this, action] (bool changed)
           {
             if (changed)
               m_umgr->add (action);
             else
               delete action;
           });
      }

    void disconnect_signals ()
      {
        for (auto c : m_connections)
          c.disconnect ();
        m_connections.clear ();
      }

    UndoAction *execute_action (UndoAction *action2)
      {
        auto action = dynamic_cast<MediaUndoAction*>(action2);
        UndoAction *out = NULL;

        switch (action->get_type ())
          {
          case MediaUndoAction::IMAGE_SET:
              {
                auto a = dynamic_cast<MediaUndoAction_ImageSet*>(action);
                TarFileImage *im = a->get_image ();
                out =
                  new MediaUndoAction_ImageSet (m_tarfile, im->getName (), im);
                if (a->get_archive_member ().empty ())
                  im->clear ();
                else
                  {
                    Glib::ustring ar = a->get_archive_member ();
                    Glib::ustring file = a->get_filename ();
                    bool broken = false;
                    std::string newbasename = "";
                    bool present = m_tarfile->contains (ar, broken);
                    Glib::ustring err;
                    if (present)
                      m_tarfile->replaceFileInCfgFile (ar, file, newbasename,
                                                       err);
                    else
                      m_tarfile->addFileInCfgFile (file, newbasename, err);

                    a->get_image ()->load (m_tarfile, newbasename);
                    a->get_image ()->instantiateImages ();
                  }
              }
            break;

          case MediaUndoAction::MASKED_IMAGE_SET:
              {
                auto a = dynamic_cast<MediaUndoAction_MaskedImageSet*>(action);
                TarFileMaskedImage *im = a->get_image ();
                out =
                  new MediaUndoAction_MaskedImageSet (m_tarfile,
                                                      im->getName (),im);
                if (a->get_archive_member ().empty ())
                  im->clear ();
                else
                  {
                    Glib::ustring ar = a->get_archive_member ();
                    Glib::ustring file = a->get_filename ();
                    bool broken = false;
                    std::string newbasename = "";
                    bool present = m_tarfile->contains (ar, broken);
                    Glib::ustring err;
                    if (present)
                      m_tarfile->replaceFileInCfgFile (ar, file, newbasename,
                                                       err);
                    else
                      m_tarfile->addFileInCfgFile (file, newbasename, err);

                    a->get_image ()->load (m_tarfile, newbasename);
                    a->get_image ()->instantiateImages ();
                  }
              }
            break;

          case MediaUndoAction::SOUND_SET:
              {
                auto a = dynamic_cast<MediaUndoAction_SoundSet*>(action);
                TarFileSound *s = a->get_sound ();
                out =
                  new MediaUndoAction_SoundSet (m_tarfile, s->get_name (), s);
                Glib::ustring ar = a->get_archive_member ();
                if (ar.empty ())
                  s->clear ();
                else
                  {
                    Glib::ustring file = a->get_filename ();
                    bool broken = false;
                    std::string newbasename = "";
                    bool present = m_tarfile->contains (ar, broken);
                    Glib::ustring err;
                    if (present)
                      m_tarfile->replaceFileInCfgFile (ar, file, newbasename,
                                                       err);
                    else
                      m_tarfile->addFileInCfgFile (file, newbasename, err);

                    a->get_sound ()->load (m_tarfile, newbasename);
                  }
              }
            break;
          }
        return out;
      }

    void setup_undo ()
      {
        m_umgr = new UndoMgr (UndoMgr::DELAY, UndoMgr::LIMIT);
        m_umgr->signal_execute ().connect
          (sigc::mem_fun (*this, &MediaDialog::execute_action));

        setup_undo_and_redo ();

        signal_undo ().connect
          ([this] ()
           {
             m_umgr->undo ();
             update ();
           });

        signal_redo ().connect
          ([this] ()
           {
             m_umgr->redo ();
             update ();
           });
      }

    void on_image_button_activated (TarFileImage *oim, TarFileImage *im,
                                    Glib::ustring msg,
                                    sigc::slot<void(bool)> after)
      {
        TarFile *t = m_tarfile;
        bool use_default = im->getName () == "";
        auto d = LwDialog::build<ImageEditorDialog> (this);
        d->set_override ();
        d->setup (use_default ? oim : im);
        d->set_title (msg);
        d->signal_response ().connect
          ([this, t, d, im, after] (Gtk::ResponseType resp)
           {
             if (resp == Gtk::ResponseType::ACCEPT &&
                 d->is_changed () && d->get_filename () != "")
               {
                 if (d->get_filename () != "")
                   {
                     Glib::ustring err;
                     bool success = d->install_file (t, im, d->get_filename (),
                                                     err);
                     if (success)
                       {
                         update ();
                         delete d;
                         return after (true);
                       }
                     else
                       {
                         Glib::ustring emsg = _("Couldn't add file!");
                         auto dialog = LwDialog::alert (emsg, err);
                         dialog->choose
                           (*d,
                            [this, dialog, after, d] (auto result)
                            {
                              dialog->choose_finish (result);
                              delete d;
                              return after (false);
                            });
                       }
                   }
               }
             else if (resp == Gtk::ResponseType::REJECT)
               {
                 Glib::ustring imgname = im->getName ();
                 Glib::ustring err;
                 if (d->uninstall_file (t, im, err))
                   {
                     ScenarioMedia::instance ()->uninstantiateSameNamedImages
                       (imgname);
                     update ();
                     delete d;
                     return after (true);
                   }
                 else
                   {
                     Glib::ustring emsg = _("Couldn't remove file!");
                     auto dialog = LwDialog::alert (emsg, err);
                     dialog->choose
                       (*d,
                        [this, dialog, after, d] (auto result)
                        {
                          dialog->choose_finish (result);
                          delete d;
                          return after (true);
                        });
                     delete d;
                     return after (false);
                   }
               }
             else
               {
                 delete d;
                 return after (false);
               }
           });
        return;
      }

    void on_masked_image_button_activated (TarFileMaskedImage *oim,
                                           TarFileMaskedImage *im,
                                           Glib::ustring msg,
                                           sigc::slot<void(bool)> after)
      {
        TarFile *t = m_tarfile;
        bool use_default = im->getName () == "";
        auto d = LwDialog::build<MaskedImageEditorDialog> (this);
        d->set_override ();
        d->set_color (GameMap::getShieldset (),
                      Playerlist::getActiveplayer ()->get_shield ());
        d->setup (use_default ? oim : im);
        d->set_title (msg);
        d->signal_response ().connect
          ([this, t, d, im, after] (Gtk::ResponseType resp)
           {
             if (resp == Gtk::ResponseType::ACCEPT &&
                 d->is_changed () && d->get_filename () != "")
               {
                 if (d->get_filename () != "")
                   {
                     Glib::ustring err;
                     bool success = d->install_file (t, im, d->get_filename (),
                                                     err);
                     if (success)
                       {
                         update ();
                         delete d;
                         return after (true);
                       }
                     else
                       {
                         Glib::ustring emsg = _("Couldn't add file!");
                         auto dialog = LwDialog::alert (emsg, err);
                         dialog->choose
                           (*d,
                            [this, dialog, after, d] (auto result)
                            {
                              dialog->choose_finish (result);
                              delete d;
                              return after (false);
                            });
                       }
                   }
               }
             else if (resp == Gtk::ResponseType::REJECT)
               {
                 Glib::ustring imgname = im->getName ();
                 Glib::ustring err;
                 if (d->uninstall_file (t, im, err))
                   {
                     ScenarioMedia::instance ()->uninstantiateSameNamedImages
                       (imgname);
                     update ();
                     delete d;
                     return after (true);
                   }
                 else
                   {
                     Glib::ustring emsg = _("Couldn't remove file!");
                     auto dialog = LwDialog::alert (emsg, err);
                     dialog->choose
                       (*d,
                        [this, dialog, after, d] (auto result)
                        {
                          dialog->choose_finish (result);
                          delete d;
                          return after (true);
                        });
                     delete d;
                     return after (false);
                   }
               }
             else
               {
                 delete d;
                 return after (false);
               }
           });
        return;
      }

    void on_sound_button_activated (TarFileSound *s,
                                    Glib::ustring default_name)
      {
        TarFile *t = m_tarfile;
        Glib::ustring sndname = s->get_name ();

        LwDialog::open
          (*this, _("Open sound file"), FileFilter::SOUND,
           [this, t, sndname,
           default_name, s] (std::string path)
           {
             if (File::nameEndsWith (path, ".ogg") == false)
               {
                 Glib::ustring emsg = _("Bad sound file!");
                 auto dialog =
                   LwDialog::alert (emsg,
                                    _("Only .ogg files can be used for sound"));
                 dialog->choose
                   (*this,
                    [this, dialog] (auto result)
                    {
                      dialog->choose_finish (result);
                      return;
                    });
                 return;
               }

            if (path != default_name)
              {
                auto action =
                  new MediaUndoAction_SoundSet (m_tarfile, s->get_name (), s);

                std::string newname = "";
                Glib::ustring err;
                bool success = false;
                if (get_name () == "")
                  success = t->addFileInCfgFile (path, newname, err);
                else
                  success = t->replaceFileInCfgFile (sndname, path, newname,
                                                     err);
                if (success)
                  {
                    m_umgr->add (action);
                    s->set_name (newname);
                    update ();
                  }
                else
                  {
                    delete action;
                    Glib::ustring emsg = _("Couldn't add file!");
                    auto dialog = LwDialog::alert (emsg, err);
                    dialog->choose
                      (*this,
                       [this, dialog] (auto result)
                       {
                         dialog->choose_finish (result);
                         return;
                       });
                  }
              }
           });
      }

    void update_buttons ()
      {
        auto name = ScenarioMedia::instance ()->getBlessSound ()->get_name ();
        m_clear_bless_button->set_sensitive (name != "");

        name = ScenarioMedia::instance ()->getHeroSound ()->get_name ();
        m_clear_hero_button->set_sensitive (name != "");

        name = ScenarioMedia::instance ()->getBattleSound ()->get_name ();
        m_clear_battle_button->set_sensitive (name != "");

        name = ScenarioMedia::instance ()->getDefeatSound ()->get_name ();
        m_clear_defeat_button->set_sensitive (name != "");

        name = ScenarioMedia::instance ()->getVictorySound ()->get_name ();
        m_clear_victory_button->set_sensitive (name != "");

        name = ScenarioMedia::instance ()->getBackSound ()->get_name ();
        m_clear_back_button->set_sensitive (name != "");
      }

    void clear_sound (TarFileSound *sound, FileLabel *filelabel)
      {
        Glib::ustring err;
        auto name = sound->get_name ();
        if (name.empty () == false)
          {
            auto action =
              new MediaUndoAction_SoundSet (m_tarfile, name, sound);
            bool success = m_tarfile->removeFileInCfgFile (name, err);
            if (success)
              {
                m_umgr->add (action);
                sound->clear ();
                filelabel->clear ();
                update ();
              }
            else
              {
                delete action;
                error_couldnt_remove_file (err);
              }
          }
      }
};
#endif
