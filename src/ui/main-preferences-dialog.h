//  Copyright (C) 2008, 2009, 2014, 2017, 2020 Ben Asselstine

#ifndef MAIN_PREFERENCES_DIALOG_H
#define MAIN_PREFERENCES_DIALOG_H
#include "lw-dialog-base.h"
#include "configuration.h"
#include "snd.h"

class MainPreferencesDialog: public LwDialogBase
{
public:
    static std::string get_resource_name ()
      {
        return "main-prefs.ui";
      }

    MainPreferencesDialog (BaseObjectType* o,
                           const Glib::RefPtr<Gtk::Builder>& xml)
      : LwDialogBase (o, xml)
      {
        m_close_button = load <Gtk::Button> ("close_button");
        m_commentator_switch = load <Gtk::Switch> ("commentator_switch");
        m_play_music_switch = load <Gtk::Switch> ("music_switch");
        m_music_volume_scale = load <Gtk::Scale> ("volume_scale");
      }

    void setup ()
      {
        set_response (m_close_button, Gtk::ResponseType::CLOSE);
        m_commentator_switch->property_active ().signal_changed ().connect
          ([this] ()
           {
             Configuration::s_displayCommentator =
               m_commentator_switch->get_active ();
           });

        m_play_music_switch->property_active ().signal_changed ().connect
          ([this] ()
           {
             Configuration::s_musicenable = m_play_music_switch->get_active ();

             if (m_play_music_switch->get_active ())
               Snd::instance ()->play ("intro", -1, false);
             else
               Snd::instance ()->halt (false);
             m_music_volume_scale->set_sensitive (Configuration::s_musicenable);
           });

        m_music_volume_scale->signal_value_changed ().connect
          ([this] ()
           {
             int volume = int (m_music_volume_scale->get_value () / 100 * 128);

             Configuration::s_musicvolume = volume;
             Snd::instance ()->updateVolume ();
           });

        signal_response ().connect
          ([this] (Gtk::ResponseType)
           {
             hide ();
           });
      }

private:
    Gtk::Button *m_close_button;
    Gtk::Switch *m_commentator_switch;
    Gtk::Switch *m_play_music_switch;
    Gtk::Scale *m_music_volume_scale;
};
#endif
