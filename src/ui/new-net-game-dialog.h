#include <gtkmm.h>
#include "lw-dialog-base.h"
#ifndef NEW_NET_GAME_DIALOG_H
#define NEW_NET_GAME_DIALOG_H
#include "join-game-dialog.h"
#include "profile-list.h"
#include "profile.h"
#include "lw-dialog.h"
#include "profile-manager-dialog.h"

class NewNetGameDialog: public LwDialogBase
{
public:
    static std::string get_resource_name ()
      {
        return "new-net-game.ui";
      }

    NewNetGameDialog (BaseObjectType* o,
                      const Glib::RefPtr<Gtk::Builder>& xml)
      : LwDialogBase (o, xml)
      {
        m_cancel_button = load <Gtk::Button> ("cancel_button");
        m_host_button = load <Gtk::Button> ("host_button");
        m_join_button = load <Gtk::Button> ("join_button");
        m_listbox = load <Gtk::ListBox> ("profiles_listbox");
        m_profile_manager_button = load <Gtk::Button> ("profile_manager_button");
      }

    void setup ()
      {
        set_response (m_cancel_button, Gtk::ResponseType::CANCEL);
        set_response (m_host_button, Gtk::ResponseType::ACCEPT);
        set_response (m_join_button, Gtk::ResponseType::REJECT);

        fill_profiles ();
        update_buttons ();

        m_profile_manager_button->signal_clicked ().connect
          ([this] ()
           {
              auto d = LwDialog::build<ProfileManagerDialog> (this);
              d->setup ();
              d->signal_response ().connect
                ([this, d] (Gtk::ResponseType)
                 {
                   Profilelist::instance ()->save ();
                   fill_profiles ();
                   update_buttons ();
                   delete d;
                 });
           });

        m_conn = m_listbox->signal_row_selected ().connect
          ([this] (Gtk::ListBoxRow*)
           {
             update_buttons ();
           });

        signal_response ().connect
          ([this](Gtk::ResponseType resp)
           {
             m_conn.disconnect ();
             hide ();

             auto it = Profilelist::instance ()->begin ();
             std::advance (it, get_selected_index ());
             auto profile = *it;

             switch (resp)
               {
               case Gtk::ResponseType::ACCEPT: //host a game
                 m_start_server.emit (profile);
                 break;

               case Gtk::ResponseType::REJECT: //join a game
                   {
                     auto d = LwDialog::build<JoinGameDialog> (this);
                     d->setup (profile);
                     d->signal_game_selected ().connect
                       ([this, profile] (Glib::ustring host, guint32 port)
                        {
                          m_start_client.emit (host, port, profile);
                        });
                   }
                 break;

               default:
                 break;
               }
           });
      }

    sigc::signal<void(Glib::ustring,unsigned short, Profile*)> signal_start_client ()
      {
        return m_start_client;
      }

    sigc::signal<void(Profile*)> signal_start_server ()
      {
        return m_start_server;
      }

private:
    Gtk::ListBox *m_listbox;
    Gtk::Button *m_cancel_button;
    Gtk::Button *m_host_button;
    Gtk::Button *m_join_button;
    Gtk::Button *m_profile_manager_button;
    int m_default_index = -1;
    sigc::connection m_conn;

    sigc::signal<void(Glib::ustring,unsigned short, Profile*)> m_start_client;
    sigc::signal<void(Profile*)> m_start_server;

    void fill_profiles ()
      {
        m_default_index = -1;
        int i = 0;
        for (auto p : *Profilelist::instance ())
          {
            if (p == Profilelist::instance ()->getDefaultProfile ())
              m_default_index = i;
            i++;
          }
        refresh_list ();
      }

    void refresh_list ()
      {
        while (auto* child = m_listbox->get_first_child ())
          m_listbox->remove (*child);

        int i = 0;
        for (auto p : *Profilelist::instance ())
          {
            auto* row_box =
              Gtk::make_managed<Gtk::Box>(Gtk::Orientation::HORIZONTAL, 8);

            auto* name_label = Gtk::make_managed<Gtk::Label>(p->getNickname ());

            name_label->set_hexpand (true);
            name_label->set_halign (Gtk::Align::START);

            row_box->append (*name_label);

            if (i == m_default_index)
              {
                auto* default_label =
                  Gtk::make_managed<Gtk::Label>("(" + _("Default") + ")");

                default_label->add_css_class ("dim-label");
                row_box->append (*default_label);
              }

            auto* row = Gtk::make_managed<Gtk::ListBoxRow> ();
            row->set_child (*row_box);

            m_listbox->append (*row);
            i++;
          }

        if (m_default_index >= 0)
          {
            auto* row = m_listbox->get_row_at_index (m_default_index);
            if (row)
              m_listbox->select_row (*row);
          }
      }

    int get_selected_index ()
      {
        auto* row = m_listbox->get_selected_row ();

        if (!row)
          return -1;

        return row->get_index ();
      }

    void update_buttons ()
      {
        bool active = get_selected_index () != -1;
        m_join_button->set_sensitive (active);
        m_host_button->set_sensitive (active);
      }
};
#endif
