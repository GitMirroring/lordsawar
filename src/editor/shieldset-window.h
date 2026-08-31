//  Copyright (C) 2007, 2008, 2009, 2010, 2011, 2012, 2014, 2015, 2017, 2020,
//  2021, 2026 Ben Asselstine
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
#ifndef SHIELDSET_WINDOW_H
#define SHIELDSET_WINDOW_H

#include "shield-set.h"
#include "tar-file-image.h"
#include "undo-mgr.h"
#include "shieldset-undo.h"
#include "about-dialog.h"
#include "shieldset-info-dialog.h"
#include "file-label.h"
#include "tar-file-masked-image.h"
#include "masked-image-editor-dialog.h"
#include "tartan-preview-dialog.h"
#include "lw-column.h"

class ShieldRow: public Glib::Object
{
public:
    Shield *m_shield;
    Glib::ustring m_name;
    sigc::signal<void()> m_signal_changed;

    sigc::signal<void()> signal_changed ()
      {
        return m_signal_changed;
      }

    void changed ()
      {
        m_signal_changed.emit ();
      }

    static Glib::RefPtr<ShieldRow> create (Shield *s)
      {
        return Glib::make_refptr_for_instance<ShieldRow> (new ShieldRow (s));
      }

protected:
    ShieldRow (Shield *s)
      : m_shield (s)
      {
        if (s)
          m_name = Shield::colorToFriendlyName (Shield::Color (s->getOwner ()));
        else
          m_name = "";
      }
};

class ShieldSetMenuButton: public Gtk::MenuButton
{
public:
    ShieldSetMenuButton (Gtk::Window &p, Glib::RefPtr<Gio::SimpleActionGroup> &group)
      :m_parent (p), m_actions (group)
      {
      }

    ~ShieldSetMenuButton () override = default;

    void setup ()
      {
        setup_menu ();
      }

    sigc::signal<void(Glib::RefPtr<Gio::SimpleAction>)> m_signal_action_added;

private:
    Gtk::Window &m_parent;
    Glib::RefPtr<Gio::SimpleActionGroup> m_actions;

    void setup_actions (std::initializer_list<const char*> list)
      {
         for (auto s : list)
           setup_action (m_actions->add_action (s));
      }

    void setup_menu ()
      {
        auto menu = Gio::Menu::create ();

        menu->append (_("New"), "lw.shieldset.file.new");
        menu->append (_("Open..."), "lw.shieldset.file.open");
        menu->append (_("Save"), "lw.shieldset.file.save");
        menu->append (_("Save As..."), "lw.shieldset.file.save-as");
        menu->append (_("Validate"), "lw.shieldset.file.validate");
        menu->append (_("Copy White Images Down"), "lw.shieldset.edit.copy_white_down");
        menu->append (_("Preview Tartan"), "lw.shieldset.view.preview-tartan");
        menu->append (_("Properties"), "lw.shieldset.edit.properties");
        auto help_menu = Gio::Menu::create ();
        help_menu->append (_("Tutorial Video"), "lw.shieldset.help.tutorial-video");
        help_menu->append (_("About"), "lw.shieldset.help.about");
        menu->append_submenu (_("Help"), help_menu);
        menu->append (_("Keyboard Shortcuts"), "lw.shieldset.help.keyboard-shortcuts");
        menu->append (_("Quit"), "lw.shieldset.file.quit");

        property_primary () = true;
        set_menu_model (menu);

        setup_actions
          ({"shieldset.file.new", "shieldset.file.open", "shieldset.file.save",
            "shieldset.file.save-as", "shieldset.file.validate",
            "shieldset.file.quit", "shieldset.edit.copy_white_down",
            "shieldset.view.preview-tartan", "shieldset.edit.properties",
            "shieldset.help.tutorial-video", "shieldset.help.about",
            "shieldset.help.keyboard-shortcuts",

            // the other actions not in the menu, it's just easier to add these
            // here
          });

        set_icon_name ("open-menu-symbolic");

      }

    void setup_action (Glib::RefPtr<Gio::SimpleAction> a)
      {
        m_signal_action_added.emit (a);
      }
};

class ShieldSetShortcutsDialog : public Gtk::Window
{
public:
    ShieldSetShortcutsDialog ()
      {
        set_title (_("Keyboard Shortcuts"));
        set_default_size (500, 400);
        set_modal (true);

        auto* scrolled = Gtk::make_managed<Gtk::ScrolledWindow>();
        scrolled->set_policy (Gtk::PolicyType::NEVER, Gtk::PolicyType::AUTOMATIC);

        auto* flow = Gtk::make_managed<Gtk::FlowBox>();
        flow->set_max_children_per_line (1);
        flow->set_min_children_per_line (1);
        flow->set_selection_mode (Gtk::SelectionMode::NONE);
        flow->set_column_spacing (16);
        flow->set_row_spacing (16);
        flow->set_margin (16);
        flow->set_homogeneous (false);

        flow->append (*create_group
                      (_("Window"),
                       {
                           {_("Close"), "Escape"},
                       }));

        flow->append (*create_group
                      ("Editing",
                       {
                           {_("Undo"), "<Ctrl>Z"},
                           {_("Redo"), "<Shift><Ctrl>Z"},

                       }));
        scrolled->set_child (*flow);
        set_child (*scrolled);

        auto controller = Gtk::EventControllerKey::create ();
        controller->signal_key_pressed ().connect
          ([this] (guint keyval, guint, Gdk::ModifierType)
           {
             if (keyval == GDK_KEY_Escape)
               {
                 close ();
                 return true;
               }
             return false;
           }, false);
        add_controller (controller);
    }

private:
    Gtk::Widget* create_group (const Glib::ustring& title,
                              const std::vector<std::pair<Glib::ustring, Glib::ustring>>& shortcuts)
      {
        auto* frame = Gtk::make_managed<Gtk::Frame>();
        frame->add_css_class ("card");

        auto* box = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::VERTICAL);
        box->set_spacing (8);
        box->set_margin (12);

        auto* title_label = Gtk::make_managed<Gtk::Label>(title);
        title_label->add_css_class ("heading");
        title_label->set_halign (Gtk::Align::START);
        box->append (*title_label);

        box->append (*Gtk::make_managed<Gtk::Separator>());

        for (const auto& [action, accel] : shortcuts)
          {
            auto* row = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::HORIZONTAL);
            row->set_spacing (12);

            auto* action_label = Gtk::make_managed<Gtk::Label>(action);
            action_label->set_halign (Gtk::Align::START);
            action_label->set_hexpand (true);

            auto* accel_label = Gtk::make_managed<Gtk::ShortcutLabel>();
            accel_label->set_accelerator (accel);
            accel_label->set_halign (Gtk::Align::END);

            row->append (*action_label);
            row->append (*accel_label);
            box->append (*row);
          }

        frame->set_child (*box);
        return frame;
      }
};
class ShieldSetWindow: public Gtk::ApplicationWindow
{
public:

    ShieldSetWindow ()
      : m_actions (Gio::SimpleActionGroup::create ())
      {
        m_shieldset = NULL;
        set_size_request (650, 500);
        m_umgr = new UndoMgr (UndoMgr::DELAY, UndoMgr::LIMIT);
        m_umgr->signal_execute ().connect
          (sigc::mem_fun (*this, &ShieldSetWindow::execute_action));
        m_umgr->signal_added_undo ().connect
          ([this] ()
           {
             m_shieldset_modified = true;
             update_window_title ();
             update_actions ();
           });
        set_resizable (false);
      }

    ~ShieldSetWindow ()
      {
        disconnect_signals ();
        disconnect_action_signals ();
        delete m_umgr;
      }

    void setup (Shieldset *shieldset)
      {
        m_shieldset = NULL;
        if (shieldset)
          {
            bool broken = false;
            m_shieldset = new Shieldset (*shieldset);
            m_shieldset->instantiateImages (broken);
            m_shieldset->setLoadTemporaryFile ();
            m_current_save_filename = shieldset->getConfigurationFile (true);
          }
        m_shieldset_modified = false;
        m_new_shieldset_needs_saving = false;

        m_menu_button = Gtk::make_managed<ShieldSetMenuButton>(*this, m_actions);
        m_menu_button->m_signal_action_added.connect
          ([this] (Glib::RefPtr<Gio::SimpleAction> action)
           {
             m_simple_actions[action->get_name ()] = action;
           });

        m_menu_button->setup ();

        auto idx = std::string (LW_APP_ID).rfind ('.');
        insert_action_group (std::string (LW_APP_ID).substr (idx + 1), m_actions);
        setup_accels ();

        setup_header_bar (m_menu_button);

        populate ();

        setup_treeview ();

        connect_action_signals ();
        connect_signals ();

        setup_quit ();

        update ();
      }

    sigc::signal<void(guint32)> signal_shieldset_saved ()
      {
        return m_shieldset_saved;
      }

    sigc::signal<void()> signal_closed ()
      {
        return m_signal_closed;
      }

private:
    Glib::RefPtr<Gio::SimpleActionGroup> m_actions;
    Gtk::HeaderBar *m_header_bar;
    Gtk::Notebook *m_notebook;
    Gtk::Label *m_subtitle;
    ShieldSetMenuButton *m_menu_button;
    std::map<Glib::ustring, Glib::RefPtr<Gio::SimpleAction>> m_simple_actions;
    Gtk::ColumnView *m_treeview;
    Gtk::Button *m_small_shield_button;
    FileLabel *m_small_shield_file_label;
    Gtk::Button *m_medium_shield_button;
    FileLabel *m_medium_shield_file_label;
    Gtk::Button *m_large_shield_button;
    FileLabel *m_large_shield_file_label;
    Gtk::Button *m_left_tartan_button;
    FileLabel *m_left_tartan_file_label;
    Gtk::Button *m_middle_tartan_button;
    FileLabel *m_middle_tartan_file_label;
    Gtk::Button *m_right_tartan_button;
    FileLabel *m_right_tartan_file_label;
    Gtk::SpinButton *m_mask_colors_spinbutton;
    Gtk::ColorButton *m_first_colorbutton;
    Gtk::ColorButton *m_second_colorbutton;
    Gtk::ColorButton *m_third_colorbutton;

    std::list<sigc::connection> m_action_connections;
    std::list<sigc::connection> m_connections;

    Glib::RefPtr<Gtk::SingleSelection> m_selection_model;
    Glib::RefPtr<Gio::ListStore<ShieldRow>> m_store;

    Shieldset *m_shieldset;
    Glib::ustring m_current_save_filename;
    bool m_shieldset_modified;
    bool m_new_shieldset_needs_saving;
    UndoMgr *m_umgr;

    sigc::signal<void(guint32)> m_shieldset_saved;
    sigc::signal<void()> m_signal_closed;

    void setup_header_bar (Gtk::MenuButton *menu_button)
      {
        m_header_bar = Gtk::make_managed<Gtk::HeaderBar>();
        m_header_bar->set_show_title_buttons (true);

        m_notebook = Gtk::make_managed<Gtk::Notebook> ();
        m_notebook->set_show_tabs (false);
        m_notebook->set_show_border (false);

        auto page1 = Gtk::make_managed<Gtk::Box> (Gtk::Orientation::VERTICAL);
        auto title1 = Gtk::make_managed<Gtk::Label>(_("Shield Set Editor"));
        title1->add_css_class ("title");
        page1->append (*title1);
        page1->set_valign (Gtk::Align::CENTER);
        m_notebook->append_page (*page1, "");

        auto page2 = Gtk::make_managed<Gtk::Box> (Gtk::Orientation::VERTICAL);
        auto title2 = Gtk::make_managed<Gtk::Label>(_("Shield Set Editor"));
        title2->add_css_class ("title");
        page2->append (*title2);
        page2->set_valign (Gtk::Align::CENTER);


        m_subtitle = Gtk::make_managed<Gtk::Label>("");
        m_subtitle->add_css_class ("subtitle");
        page2->append (*m_subtitle);

        m_notebook->append_page (*page2, "");
        m_header_bar->set_title_widget (*m_notebook);

        m_header_bar->pack_end (*menu_button);
        set_titlebar (*m_header_bar);
      }

    void setup_treeview ()
      {
        m_store = Gio::ListStore<ShieldRow>::create ();
        m_selection_model = Gtk::SingleSelection::create (m_store);
        m_treeview->set_model (m_selection_model);

        LwColumn::setup_name_column<ShieldRow>
          (m_treeview, true, _("Player"),
           [] (const auto& row)
           {
             return row->m_name;
           });

        fill_treeview ();

        m_selection_model->signal_selection_changed ().connect
          ([this] (const guint &, const guint &)
           {
             update_shieldset_panel ();
           });

      }

    void fill_treeview ()
      {
        m_store->remove_all ();
        if (m_shieldset)
          {
            for (auto s : *m_shieldset)
              m_store->append (ShieldRow::create (s));
          }
                      
        m_selection_model->set_selected (0);
      }

    Gtk::Box *create_attribute_row (Glib::ustring title, Glib::ustring desc,
                                    Glib::ustring css)
      {
        auto box = Gtk::make_managed<Gtk::Box> (Gtk::Orientation::HORIZONTAL);
        box->add_css_class (css);
        auto titlebox = Gtk::make_managed<Gtk::Box> (Gtk::Orientation::VERTICAL);
        auto title_label = Gtk::make_managed<Gtk::Label> (title);
        title_label->add_css_class ("setting_label");
        title_label->set_halign (Gtk::Align::START);
        title_label->set_hexpand (true);
        titlebox->append (*title_label);
        if (desc != "")
          {
            auto desc_label = Gtk::make_managed<Gtk::Label> (desc);
            desc_label->add_css_class ("setting_explain_label");
            desc_label->set_hexpand (true);
            desc_label->set_halign (Gtk::Align::START);
            titlebox->append (*desc_label);
          }
        box->set_spacing (6);
        box->append (*titlebox);
        return box;
      }

    void
    add_connection (sigc::connection c)
      {
        m_connections.push_back (c);
      }

    void
    add_action_connection (sigc::connection c)
      {
        m_action_connections.push_back (c);
      }

    void action_connect (Glib::ustring name, sigc::slot<void()> slot)
      {
        auto simple =
          std::dynamic_pointer_cast<Gio::SimpleAction>(m_simple_actions[name]);
        if (simple)
          add_action_connection
            (simple->signal_activate ().connect (sigc::hide (slot)));
      }

    void connect_action_signals ()
      {
        action_connect
          ("shieldset.file.new",
           [this] ()
           {
             check_discard
               (_("Save these changes before making a new shield set?"),
                [this] (bool discard)
                {
                  if (discard)
                    {
                      m_current_save_filename = "";
                      if (m_shieldset)
                        delete m_shieldset;

                      guint32 num = 0;
                      Glib::ustring name =
                        Shieldsetlist::instance ()->findFreeName
                        (_("Untitled"), 100, num);

                      m_shieldset =
                        new Shieldset (Shieldsetlist::getNextAvailableId (1),
                                       name);
                      m_shieldset->setNewTemporaryFile ();

                      m_shieldset->populate_with_defaults ();
                      disconnect_signals ();
                      //populate the list with initial entries.
                      fill_treeview ();
                      connect_signals ();

                      update_shieldset_panel();
                      m_selection_model->set_selected (0);
                      m_shieldset_modified = false;
                      m_new_shieldset_needs_saving = true;
                      m_umgr->clear ();
                      update ();
                    }
                });
           });

        action_connect
          ("shieldset.file.open",
           [this] ()
           {
             check_discard
               (_("Save these changes before making a new shield set?"),
                [this] (bool discard)
                {
                  if (discard)
                    {
                      LwDialog::open 
                        (*this, _("Choose a shield set to open"),
                         FileFilter::SHIELDSET,
                         [this] (std::string path)
                         {
                           load_shieldset
                             (path,
                              [this] (bool loaded)
                              {
                                if (loaded)
                                  {
                                    m_shieldset_modified = false;
                                    m_new_shieldset_needs_saving = false;
                                    update ();
                                  }
                              });
                         });
                    }
                });
           });

        action_connect
          ("shieldset.file.save",
           [this] ()
           {
             if (m_current_save_filename.empty () == true)
               on_save_as_activated ();
             else
               {
                 check_save_valid
                     (true,
                      [this](bool valid)
                      {
                        if (valid)
                          {
                            save_current_shieldset_file
                              ("",
                               [this] (bool saved)
                               {
                                 (void) saved;
                               });
                          }
                      });
               }
           });

        action_connect
          ("shieldset.file.save-as",
           [this] ()
           {
             on_save_as_activated ();
           });

        action_connect
          ("shieldset.file.validate",
           [this] ()
           {
             bool valid;
             std::list<Glib::ustring> msgs;
             if (m_shieldset == NULL)
               return;

             valid = String::utrim (m_shieldset->getName ()) != "";
             if (!valid)
               {
                 Glib::ustring s =
                   _("The name of the shield set is invalid.");
                 msgs.push_back(s);
               }

             for (unsigned int i = Shield::WHITE; i <= Shield::NEUTRAL; i++)
               {
                 auto shield = m_shieldset->lookupShieldByColor (i);
                 if (!shield)
                   continue;

                 auto ss = shield->getFirstShieldstyle (ShieldStyle::SMALL);
                 if (ss &&
                     ss->getMaskedImage ()->getName ().empty () == true)
                   msgs.push_back
                     (String::ucompose
                      (_("%1 is missing a small shield."),
                       Shield::colorToFriendlyName
                       (Shield::Color (i))));

                 ss = shield->getFirstShieldstyle (ShieldStyle::MEDIUM);
                 if (ss &&
                     ss->getMaskedImage ()->getName ().empty () == true)
                   msgs.push_back
                     (String::ucompose
                      (_("%1 is missing a medium shield."),
                       Shield::colorToFriendlyName
                       (Shield::Color (i))));

                 ss = shield->getFirstShieldstyle (ShieldStyle::LARGE);
                 if (ss &&
                     ss->getMaskedImage ()->getName ().empty () == true)
                   msgs.push_back
                     (String::ucompose
                      (_("%1 is missing a large shield."),
                       Shield::colorToFriendlyName
                       (Shield::Color (i))));
               }

             for (unsigned int i = Shield::WHITE; i <= Shield::NEUTRAL; i++)
               {
                 auto shield = m_shieldset->lookupShieldByColor (i);
                 if (!shield)
                   continue;
                 auto mim = shield->getTartanMaskedImage (Tartan::LEFT);
                 if (mim->getName ().empty () == true)
                   {
                     Glib::ustring s =
                       String::ucompose
                       (_("%1 is missing a left tartan."),
                        Shield::colorToFriendlyName (Shield::Color (i)));
                     msgs.push_back (s);
                   }

                 mim = shield->getTartanMaskedImage (Tartan::CENTER);
                 if (mim->getName ().empty () == true)
                   {
                     Glib::ustring s =
                       String::ucompose
                       (_("%1 is missing a middle tartan."),
                        Shield::colorToFriendlyName (Shield::Color (i)));
                     msgs.push_back (s);
                   }

                 mim = shield->getTartanMaskedImage (Tartan::RIGHT);
                 if (mim->getName ().empty () == true)
                   {
                     Glib::ustring s =
                       String::ucompose
                       (_("%1 is missing a right tartan."),
                        Shield::colorToFriendlyName (Shield::Color (i)));
                     msgs.push_back (s);
                   }
               }

             Glib::ustring msg = "";
             for (auto m : msgs)
               {
                 msg += m + "\n";
                 break; // we only show one
               }

             if (msg == "")
               msg = _("The shield set is valid.");

             Glib::ustring detail = "";
             if (msgs.size () > 1)
               detail =
                 String::ucompose
                 (ngettext ("(There is %1 more error not shown)",
                            "(There are %1 more errors not shown)",
                            msgs.size () - 1), msgs.size () - 1);
             auto dialog = LwDialog::alert (msg, detail);
             dialog->choose
               (*this,
                [this, dialog] (auto result)
                {
                  dialog->choose_finish (result);
                  return;
                });
           });

        action_connect
          ("shieldset.file.quit",
           [this] ()
           {
             check_quit
               ([this] (bool quit)
                {
                  if (quit)
                    {
                      if (m_shieldset)
                        delete m_shieldset;
                      hide ();
                      m_signal_closed.emit ();
                    }
                });
             return;
           });

        action_connect
          ("shieldset.edit.copy_white_down",
           [this] ()
           {
             Glib::ustring msg = _("Are you sure?");
             Glib::ustring detail =
               _("This will copy images from the white shield to the other "
                 "shields.");
             auto d = LwDialog::alert_yn (msg, detail);
             d->choose
               (*this,
                [this, d] (auto result)
                {
                  bool change = d->choose_finish (result) == 1;
                  if (change)
                    copy_white_down ();
                });
           });

        action_connect
          ("shieldset.view.preview-tartan",
           [this] ()
           {
             auto d = LwDialog::build<TartanPreviewDialog> (this);
             Shield *shield = get_current_shield ();
             d->setup (m_shieldset, Shield::Color (shield->getOwner ()));
             d->signal_response ().connect
               ([this, d] (Gtk::ResponseType)
                {
                  delete d;
                });
           });

        action_connect
          ("shieldset.edit.properties",
           [this] ()
           {
             on_edit_shieldset_info_activated ();
           });

        action_connect
          ("shieldset.help.tutorial-video",
           [this] ()
           {
             Glib::ustring uri = "https://vimeo.com/406882053";
             auto launcher = Gtk::UriLauncher::create (uri);

             launcher->launch
               (*this,
                [launcher](const Glib::RefPtr<Gio::AsyncResult>& result)
                {
                  try
                    {
                      launcher->launch_finish (result);
                    }
                  catch (const Glib::Error& ex)
                    {
                      std::cerr << ex.what () << '\n';
                    }
                });
           });

        action_connect
          ("shieldset.help.keyboard-shortcuts",
           [this] ()
           {
             auto* dialog = Gtk::make_managed<ShieldSetShortcutsDialog>();
             dialog->set_transient_for (*this);
             dialog->present ();
           });

        action_connect
          ("shieldset.help.about",
           [this] ()
           {
             auto d = Gtk::make_managed<AboutDialog> (*this);
             d->set_program_name ("LordsAWar! Shield Set Editor");
             bool broken;
             auto logo =
               PixMask::create (File::getVariousFile ("tileset_icon.png"),
                                broken);
             if (logo)
               d->set_logo (logo->to_texture ());
             d->present ();
             d->signal_response ().connect
               ([d] (Gtk::ResponseType)
                {
                  delete d;
                });
           });

      }

    void disconnect_signals ()
      {
        for (auto c : m_connections)
          c.disconnect ();
        m_connections.clear ();
      }

    void disconnect_action_signals ()
      {
        for (auto c : m_action_connections)
          c.disconnect ();
        m_action_connections.clear ();
      }

    void load_shieldset (Glib::ustring filename, sigc::slot<void(bool)> after)
      {
        Glib::ustring old = m_current_save_filename;
        m_current_save_filename = filename;

        Shieldset::create
          (filename,
           [this, after, old] (Shieldset *shieldset, bool broken,
                               bool unsupported_version, Glib::ustring err)
           {
             if (shieldset == NULL || unsupported_version || broken)
               {
                 Glib::ustring msg = _("The shield set could not be loaded.");
                 auto dialog = LwDialog::alert (msg, err);
                 dialog->choose
                   (*this,
                    [this, dialog, old, after] (auto result)
                    {
                      dialog->choose_finish (result);
                      m_current_save_filename = old;
                      return after (false);
                    });
                 return;
               }
             disconnect_signals ();
             Shieldset *old_shieldset = m_shieldset;
             m_shieldset = NULL;
             update ();
             m_shieldset = shieldset;
             fill_treeview ();
             connect_signals ();
             m_umgr->clear ();
             m_shieldset->setLoadTemporaryFile ();

             Glib::signal_idle ().connect
               ([this, old_shieldset, after] ()
                {
                  bool broke = false;
                  m_shieldset->instantiateImages (broke);
                  if (broke)
                    {
                      delete m_shieldset;
                      m_shieldset = NULL;
                      Glib::ustring msg = _("Couldn't load shield set images");
                      auto dialog = LwDialog::alert (msg);
                      dialog->choose
                        (*this,
                         [this, old_shieldset, after, dialog] (auto result)
                         {
                           m_shieldset = old_shieldset;
                           update ();
                           dialog->choose_finish (result);
                           return after (false);
                         });
                    }
                  else
                    {
                      delete old_shieldset;
                      update ();
                      after (true);
                    }
                  return false;
                });
           });
      }

    std::vector<Gdk::RGBA> get_current_colors ()
      {
        std::vector<Gdk::RGBA> list;
        if (m_mask_colors_spinbutton->get_value () >= 1)
          list.push_back (m_first_colorbutton->get_rgba ());
        if (m_mask_colors_spinbutton->get_value () >= 2)
          list.push_back (m_second_colorbutton->get_rgba ());
        if (m_mask_colors_spinbutton->get_value () >= 3)
          list.push_back (m_third_colorbutton->get_rgba ());
        return list;
      }

    void change_pic (Shield::Color color, TarFileMaskedImage *mim,
                     Glib::ustring msg,
                     sigc::slot<void(bool,bool,Glib::ustring)> after)
      {
        Shield *s = get_current_shield ();

        Glib::ustring imgname = mim->getName ();

        auto d = LwDialog::build<MaskedImageEditorDialog> (this);
        d->set_color (m_shieldset, color);
        d->setup (mim);
        d->set_title (msg);
        d->signal_response ().connect
          ([this, s, d, mim, after] (Gtk::ResponseType resp)
           {
             bool cleared = false;
             Glib::ustring newfile = "";
             if (resp == Gtk::ResponseType::ACCEPT &&
                 d->is_changed () && d->get_filename () != "")
               {
                 auto action = new ShieldSetUndoAction_AddImage (m_shieldset);
                 Glib::ustring newname = "";
                 Glib::ustring err = "";
                 bool success = d->install_file (m_shieldset, mim,
                                                 d->get_filename (), err);
                 if (success)
                   {
                     m_umgr->add (action);
                     newfile = mim->getName ();
                     m_shieldset_modified = true;
                     update ();
                     delete d;
                     return after (false, cleared, newfile);
                   }
                 else
                   {
                     delete action;
                     Glib::ustring emsg = _("Couldn't add file!");
                     auto dialog = LwDialog::alert (emsg, err);
                     dialog->choose
                       (*d,
                        [this, dialog, cleared, newfile, after, d] (auto result)
                        {
                          dialog->choose_finish (result);
                          delete d;
                          return after (true, cleared, newfile);
                        });
                     return;
                   }
               }
             else if (resp == Gtk::ResponseType::REJECT)
               {
                 Glib::ustring err = "";
                 auto action = new ShieldSetUndoAction_ClearImage (m_shieldset);
                 Glib::ustring name = mim->getName ();
                 if (d->uninstall_file (m_shieldset, mim, err))
                   {
                     m_umgr->add (action);

                     m_shieldset->uninstantiateSameNamedImages (name);
                     update ();
                     cleared = true;
                     newfile = "";
                     delete d;
                     return after (false, cleared, newfile);
                   }
                 else
                   {
                     delete action;
                     newfile = d->get_filename ();
                     Glib::ustring emsg = _("Couldn't remove file!");
                     auto dialog = LwDialog::alert (emsg, err);
                     dialog->choose
                       (*d,
                        [this, dialog, cleared, newfile, after, d] (auto result)
                        {
                          dialog->choose_finish (result);
                          delete d;
                          return after (true, cleared, newfile);
                        });
                     return;
                   }
               }
             else
               {
                 delete d;
                 return after (true, false, "");
               }
           });
      }

    void connect_signals ()
      {
        auto s = get_current_shield ();
        add_connection
          (m_small_shield_button->signal_clicked ().connect
           ([this, s] ()
            {
              auto ss = s->getFirstShieldstyle (ShieldStyle::SMALL);
              change_pic
                (Shield::Color (s->getOwner ()), ss->getMaskedImage (),
                 _("Select a small shield image"),
                 [this] (bool, bool, Glib::ustring)
                 {
                 });
            }));

        add_connection
          (m_medium_shield_button->signal_clicked ().connect
           ([this, s] ()
            {
              auto ss = s->getFirstShieldstyle (ShieldStyle::MEDIUM);
              change_pic
                (Shield::Color (s->getOwner ()), ss->getMaskedImage (),
                 _("Select a medium shield image"),
                 [this] (bool, bool, Glib::ustring)
                 {
                 });
            }));

        add_connection
          (m_large_shield_button->signal_clicked ().connect
           ([this, s] ()
            {
              auto ss = s->getFirstShieldstyle (ShieldStyle::LARGE);
              change_pic
                (Shield::Color (s->getOwner ()), ss->getMaskedImage (),
                 _("Select a large shield image"),
                 [this] (bool, bool, Glib::ustring)
                 {
                 });
            }));

        add_connection
          (m_left_tartan_button->signal_clicked ().connect
           ([this, s] ()
            {
              auto t = s->getTartanMaskedImage (Tartan::LEFT);
              change_pic
                (Shield::Color (s->getOwner ()), t,
                 _("Select a left tartan image"),
                 [this] (bool, bool, Glib::ustring)
                 {
                 });
            }));

        add_connection
          (m_middle_tartan_button->signal_clicked ().connect
           ([this, s] ()
            {
              auto t = s->getTartanMaskedImage (Tartan::CENTER);
              change_pic
                (Shield::Color (s->getOwner ()), t,
                 _("Select a middle tartan image"),
                 [this] (bool, bool, Glib::ustring)
                 {
                 });
            }));

        add_connection
          (m_right_tartan_button->signal_clicked ().connect
           ([this, s] ()
            {
              auto t = s->getTartanMaskedImage (Tartan::RIGHT);
              change_pic
                (Shield::Color (s->getOwner ()), t,
                 _("Select a right tartan image"),
                 [this] (bool, bool, Glib::ustring)
                 {
                 });
            }));

        add_connection
          (m_mask_colors_spinbutton->signal_value_changed ().connect
           ([this, s] ()
            {
              int i = m_selection_model->get_selected ();
              ShieldSetUndoAction_Colors *action =
                new ShieldSetUndoAction_Colors (i, s->getColors ());
              m_umgr->add (action);
              s->setColors (get_current_colors ());
              update ();
            }));

        add_connection
          (m_first_colorbutton->property_rgba ().signal_changed ().connect
           ([this, s] ()
            {
              int i = m_selection_model->get_selected ();
              ShieldSetUndoAction_Colors *action =
                new ShieldSetUndoAction_Colors (i, s->getColors ());
              m_umgr->add (action);
              s->setColors (get_current_colors ());
              update ();
            }));

        add_connection
          (m_second_colorbutton->property_rgba ().signal_changed ().connect
           ([this, s] ()
            {
              int i = m_selection_model->get_selected ();
              ShieldSetUndoAction_Colors *action =
                new ShieldSetUndoAction_Colors (i, s->getColors ());
              m_umgr->add (action);
              s->setColors (get_current_colors ());
              update ();
            }));

        add_connection
          (m_third_colorbutton->property_rgba ().signal_changed ().connect
           ([this, s] ()
            {
              int i = m_selection_model->get_selected ();
              ShieldSetUndoAction_Colors *action =
                new ShieldSetUndoAction_Colors (i, s->getColors ());
              m_umgr->add (action);
              s->setColors (get_current_colors ());
              update ();
            }));
      }

    void update ()
      {
        update_window_title ();
        update_shieldset_panel ();
        update_actions ();
      }

    void update_actions ()
      {
        bool a = m_shieldset != NULL;
        m_simple_actions ["shieldset.edit.copy_white_down"]->set_enabled (a);
        m_simple_actions ["shieldset.edit.properties"]->set_enabled (a);
        m_simple_actions ["shieldset.file.validate"]->set_enabled (a);
        m_simple_actions ["shieldset.file.save-as"]->set_enabled (a);
        m_simple_actions ["shieldset.file.save"]->set_enabled
          (m_umgr->undo_empty () == false);

        Shield *s = get_current_shield ();
        a = false;
        if (s)
          a =
            s->getTartanMaskedImage (Tartan::LEFT)->getName () != "" &&
            s->getTartanMaskedImage (Tartan::CENTER)->getName () != "" &&
            s->getTartanMaskedImage (Tartan::RIGHT)->getName () != "";
        m_simple_actions["shieldset.view.preview-tartan"]->set_enabled (a);
      }

    Shield * get_current_shield ()
      {
        auto item = m_selection_model->get_selected_item ();
        if (item)
          {
            auto row = std::dynamic_pointer_cast<ShieldRow>(item);
            return row->m_shield;
          }
        return NULL;
      }

    void update_shieldset_panel ()
      {
        bool sensitive;
        bool second_colorbutton_sensitive;
        bool third_colorbutton_sensitive;
        disconnect_signals ();
        if (m_shieldset)
          {
            sensitive = true;
            Shield *shield = get_current_shield ();
            Gdk::RGBA black = Gdk::RGBA ("black");
            m_first_colorbutton->set_rgba (shield->getColors ()[0]);
            if (shield->getColors ().size () >= 2)
              m_second_colorbutton->set_rgba (shield->getColors ()[1]);
            else
              m_second_colorbutton->set_rgba (black);
            if (shield->getColors ().size () >= 3)
              m_third_colorbutton->set_rgba (shield->getColors ()[2]);
            else
              m_third_colorbutton->set_rgba (black);
            int num = shield->getColors ().size ();
            m_mask_colors_spinbutton->set_value (num);
            second_colorbutton_sensitive = num >= 2;
            third_colorbutton_sensitive = num >= 3;

            Glib::ustring s = "";
            auto ss = shield->getFirstShieldstyle (ShieldStyle::SMALL);
            if (ss && ss->getMaskedImage ()->getName ().empty () == false)
              s = ss->getMaskedImage ()->getName ();
            m_small_shield_file_label->set_label (s);

            s = "";
            ss = shield->getFirstShieldstyle (ShieldStyle::MEDIUM);
            if (ss && ss->getMaskedImage ()->getName ().empty () == false)
              s = ss->getMaskedImage ()->getName ();
            m_medium_shield_file_label->set_label (s);

            s = "";
            ss = shield->getFirstShieldstyle (ShieldStyle::LARGE);
            if (ss && ss->getMaskedImage ()->getName ().empty () == false)
              s = ss->getMaskedImage ()->getName ();
            m_large_shield_file_label->set_label (s);

            s = "";
            if (shield->getTartanMaskedImage
                (Tartan::LEFT)->getName ().empty () == false)
              s = shield->getTartanMaskedImage (Tartan::LEFT)->getName ();
            m_left_tartan_file_label->set_label (s);

            s = "";
            if (shield->getTartanMaskedImage
                (Tartan::CENTER)->getName ().empty() == false)
              s = shield->getTartanMaskedImage (Tartan::CENTER)->getName ();
            m_middle_tartan_file_label->set_label (s);

            s = "";
            if (shield->getTartanMaskedImage
                (Tartan::RIGHT)->getName ().empty () == false)
              s = shield->getTartanMaskedImage (Tartan::RIGHT)->getName ();
            m_right_tartan_file_label->set_label (s);
          }
        else
          {
            sensitive = false;
            second_colorbutton_sensitive = false;
            third_colorbutton_sensitive = false;
            m_small_shield_file_label->set_label ("");
            m_medium_shield_file_label->set_label ("");
            m_large_shield_file_label->set_label ("");
            m_left_tartan_file_label->set_label ("");
            m_middle_tartan_file_label->set_label ("");
            m_right_tartan_file_label->set_label ("");
            m_mask_colors_spinbutton->set_value (1);
            m_first_colorbutton->set_rgba (Gdk::RGBA ("black"));
            m_second_colorbutton->set_rgba (Gdk::RGBA ("black"));
            m_third_colorbutton->set_rgba (Gdk::RGBA ("black"));
          }

        m_treeview->set_sensitive (sensitive);
        m_small_shield_button->set_sensitive (sensitive);
        m_medium_shield_button->set_sensitive (sensitive);
        m_large_shield_button->set_sensitive (sensitive);
        m_left_tartan_button->set_sensitive (sensitive);
        m_middle_tartan_button->set_sensitive (sensitive);
        m_right_tartan_button->set_sensitive (sensitive);
        m_mask_colors_spinbutton->set_sensitive (sensitive);
        m_first_colorbutton->set_sensitive (sensitive);

        m_second_colorbutton->set_sensitive (second_colorbutton_sensitive);
        m_third_colorbutton->set_sensitive (third_colorbutton_sensitive);

        connect_signals ();
      }

    void update_window_title ()
      {
        Glib::ustring title = "";
        if (m_shieldset_modified || m_new_shieldset_needs_saving)
          title += "*";
        if (m_shieldset)
          {
            m_notebook->set_current_page (1);
            title += m_shieldset->getName ();
            m_subtitle->set_text (title);
          }
        else
          {
            m_notebook->set_current_page (0);
            m_subtitle->set_text ("");
          }
      }

    void populate ()
      {
        m_treeview = Gtk::make_managed<Gtk::ColumnView> ();
        m_small_shield_button = Gtk::make_managed<Gtk::Button> ();
        m_small_shield_file_label =
          Gtk::make_managed<FileLabel> (m_small_shield_button);
        m_medium_shield_button = Gtk::make_managed<Gtk::Button> ();
        m_medium_shield_file_label =
          Gtk::make_managed<FileLabel> (m_medium_shield_button);
        m_large_shield_button = Gtk::make_managed<Gtk::Button> ();
        m_large_shield_file_label =
          Gtk::make_managed<FileLabel> (m_large_shield_button);
        m_left_tartan_button = Gtk::make_managed<Gtk::Button> ();
        m_left_tartan_file_label =
          Gtk::make_managed<FileLabel> (m_left_tartan_button);
        m_middle_tartan_button = Gtk::make_managed<Gtk::Button> ();
        m_middle_tartan_file_label =
          Gtk::make_managed<FileLabel> (m_middle_tartan_button);
        m_right_tartan_button = Gtk::make_managed<Gtk::Button> ();
        m_right_tartan_file_label =
          Gtk::make_managed<FileLabel> (m_right_tartan_button);
        m_mask_colors_spinbutton = Gtk::make_managed<Gtk::SpinButton> ();
        m_mask_colors_spinbutton->set_adjustment
          (Gtk::Adjustment::create (1, 1, 3, 1, 1, 0));
        m_first_colorbutton = Gtk::make_managed<Gtk::ColorButton> ();
        m_second_colorbutton = Gtk::make_managed<Gtk::ColorButton> ();
        m_third_colorbutton = Gtk::make_managed<Gtk::ColorButton> ();

        auto box = Gtk::make_managed<Gtk::Box> ();
        box->set_margin (6);
        box->set_spacing (6);
        box->set_orientation (Gtk::Orientation::HORIZONTAL);
        box->set_vexpand (true);
        box->set_hexpand (true);

        box->append (*m_treeview);

        auto scrolled_window = Gtk::make_managed<Gtk::ScrolledWindow> ();
        scrolled_window->set_policy
          (Gtk::PolicyType::NEVER, Gtk::PolicyType::AUTOMATIC);
        scrolled_window->set_vexpand (true);
        scrolled_window->set_hexpand (true);
        box->append (*scrolled_window);

        auto vbox = Gtk::make_managed<Gtk::Box> ();
        vbox->set_orientation (Gtk::Orientation::VERTICAL);
        vbox->set_vexpand (true);

        scrolled_window->set_child (*vbox);

        auto images_label = Gtk::make_managed<Gtk::Label> ("Images");
        images_label->add_css_class ("setting_heading");
        vbox->append (*images_label);

        auto small_shield =
          create_attribute_row (_("Small Shield"),
                                _("The smallest shield appears on mini maps"),
                                "setting_top");
        small_shield->append (*m_small_shield_button);
        vbox->append (*small_shield);

        auto medium_shield =
          create_attribute_row (_("Medium Shield"),
                                _("The medium shield appears in the turn indicator"),
                                "setting_middle");
        medium_shield->append (*m_medium_shield_button);
        vbox->append (*medium_shield);

        auto large_shield =
          create_attribute_row (_("Large Shield"),
                                _("The large shield appears in dialogs"),
                                "setting_middle");
        large_shield->append (*m_large_shield_button);
        vbox->append (*large_shield);

        auto left_tartan =
          create_attribute_row (_("Left Tartan"),
                                _("The leftmost component of a progress bar"),
                                "setting_middle");
        left_tartan->append (*m_left_tartan_button);
        vbox->append (*left_tartan);

        auto middle_tartan =
          create_attribute_row (_("Middle Tartan"),
                                _("The repeating middle component of a progress bar"),
                                "setting_middle");
        middle_tartan->append (*m_middle_tartan_button);
        vbox->append (*middle_tartan);

        auto right_tartan =
          create_attribute_row (_("Right Tartan"),
                                _("The rightmost component of a progress bar"),
                                "setting_bottom");
        right_tartan->append (*m_right_tartan_button);
        vbox->append (*right_tartan);

        auto colors_label = Gtk::make_managed<Gtk::Label> ("Colors");
        colors_label->add_css_class ("setting_heading");
        vbox->append (*colors_label);

        auto mask_colors =
          create_attribute_row (_("Mask Colors"),
                                _("How many masked colors these images have"),
                                "setting_top");
        mask_colors->append (*m_mask_colors_spinbutton);
        vbox->append (*mask_colors);

        auto first_color =
          create_attribute_row (_("First Color"),
                                _("The color the first mask should be colored in as"),
                                "setting_middle");
        first_color->append (*m_first_colorbutton);
        vbox->append (*first_color);

        auto second_color =
          create_attribute_row (_("Second Color"),
                                _("The color the second mask should be colored in as"),
                                "setting_middle");
        second_color->append (*m_second_colorbutton);
        vbox->append (*second_color);

        auto third_color =
          create_attribute_row (_("Third Color"),
                                _("The color the third mask should be colored in as"),
                                "setting_bottom");
        third_color->append (*m_third_colorbutton);
        vbox->append (*third_color);

        set_child (*box);
      }

    Shield* get_shield_by_index (ShieldSetUndoAction_ShieldIndex *a)
      {
        auto row = m_store->get_item(a->get_index ());
        return row->m_shield;
      }

    UndoAction* execute_action (UndoAction *a2)
      {
        ShieldSetUndoAction *action = dynamic_cast<ShieldSetUndoAction*>(a2);
        UndoAction *out = NULL;

        switch (action->get_type ())
          {
          case ShieldSetUndoAction::CHANGE_COLORS:
              {
                ShieldSetUndoAction_Colors *a =
                  dynamic_cast<ShieldSetUndoAction_Colors*>(action);
                out = new ShieldSetUndoAction_Colors
                  (a->get_index (), get_shield_by_index (a)->getColors ());
                get_shield_by_index (a)->setColors (a->get_colors ());
                break;
              }

          case ShieldSetUndoAction::CHANGE_PROPERTIES:
              {
                ShieldSetUndoAction_Properties *a =
                  dynamic_cast<ShieldSetUndoAction_Properties*>(action);
                out = new ShieldSetUndoAction_Properties
                  (m_shieldset->getName (),
                   m_shieldset->getInfo (),
                   m_shieldset->getCopyright (),
                   m_shieldset->getLicense ());
                m_shieldset->setName (a->get_name ());
                m_shieldset->setInfo (a->get_description ());
                m_shieldset->setCopyright (a->get_copyright ());
                m_shieldset->setLicense (a->get_license ());
                break;
              }

          case ShieldSetUndoAction::COPY_WHITE_DOWN:
              {
                ShieldSetUndoAction_WhiteDown *a =
                  dynamic_cast<ShieldSetUndoAction_WhiteDown*>(action);
                out = new ShieldSetUndoAction_WhiteDown (m_shieldset);
                doReloadShieldset (a);
                break;
              }

          case ShieldSetUndoAction::ADD_IMAGE:
              {
                ShieldSetUndoAction_AddImage *a =
                  dynamic_cast<ShieldSetUndoAction_AddImage*>(action);
                out = new ShieldSetUndoAction_AddImage (m_shieldset);
                doReloadShieldset (a);
                break;
              }

          case ShieldSetUndoAction::CLEAR_IMAGE:
              {
                ShieldSetUndoAction_ClearImage *a =
                  dynamic_cast<ShieldSetUndoAction_ClearImage*>(action);
                out = new ShieldSetUndoAction_ClearImage (m_shieldset);
                doReloadShieldset (a);
                break;
              }
          }
        return out;
      }

    void reload_shieldset (ShieldSetUndoAction_Save *action)
      {
        Glib::ustring olddir = m_shieldset->getDirectory ();
        Glib::ustring oldname =
          File::get_basename (m_shieldset->getConfigurationFile (true));
        Glib::ustring oldext = m_shieldset->getExtension ();

        m_shieldset->clean_tmp_dir ();
        delete m_shieldset;
        m_shieldset = new Shieldset (*(action->get_shieldset ()));
        m_shieldset->setLoadTemporaryFile ();

        m_shieldset->setDirectory (olddir);
        m_shieldset->setBaseName (oldname);
        m_shieldset->setExtension (oldext);
        update ();
      }

    bool doReloadShieldset (ShieldSetUndoAction_Save *action)
      {
        Glib::ustring olddir = m_shieldset->getDirectory ();
        Glib::ustring oldname =
          File::get_basename (m_shieldset->getConfigurationFile (true));
        Glib::ustring oldext = m_shieldset->getExtension ();

        m_shieldset->clean_tmp_dir ();
        delete m_shieldset;
        m_shieldset = new Shieldset (*(action->get_shieldset ()));
        m_shieldset->setLoadTemporaryFile ();

        disconnect_signals ();
        int idx = m_selection_model->get_selected ();
        fill_treeview ();

        if (idx >= 0)
          {
            if (m_shieldset->size () >= (guint32)idx)
              m_selection_model->set_selected (idx);
            else
              m_selection_model->set_selected (0);

          }
        connect_signals ();

        m_shieldset->setDirectory (olddir);
        m_shieldset->setBaseName (oldname);
        m_shieldset->setExtension (oldext);
        update ();
        return false;
      }

    void check_discard (Glib::ustring msg, sigc::slot<void(bool)> after)
      {
        if (m_shieldset_modified || m_new_shieldset_needs_saving)
          {
            auto d = Gtk::AlertDialog::create (_("Save changes?"));
            d->set_detail (msg);
            d->set_buttons ({_("Save"), _("Discard"), _("Cancel")});
            d->set_cancel_button (2);
            d->set_default_button (0);
            d->set_modal (true);
            d->choose
              (*this,
               [this, d, after] (Glib::RefPtr<Gio::AsyncResult>& result)
               {
                 int resp = d->choose_finish (result);
                 switch (resp)
                   {
                   case 0: // save
                     check_save_valid
                       (true,
                        [this, after] (bool valid)
                        {
                          if (!valid)
                            return after (false);

                          if (m_shieldset->getDirectory ().empty () == false)
                            {
                              save_current_shieldset_file_as
                                ([this, after] (bool saved)
                                 {
                                   after (saved);
                                 });
                            }
                          else
                            {
                              save_current_shieldset_file
                                ("",
                                 [this, after] (bool saved)
                                 {
                                   after (saved);
                                 });
                            }
                        });
                     break;

                   case 1: // discard
                     after (true);
                     break;

                   default: // cancel
                     after (false);
                     break;
                   }
               });
          }
        else
          after (true);
        return;
      }

    void check_name_valid (bool existing, sigc::slot<void(bool)> after)
      {
        Glib::ustring name = m_shieldset->getName ();
        Glib::ustring newname = "";
        if (existing)
          {
            Shieldset *oldshieldset =
              Shieldsetlist::instance ()->get (m_shieldset->getId ());
            if (oldshieldset && oldshieldset->getName () != name)
              newname = oldshieldset->getName ();
          }
        guint32 num = 0;
        Glib::ustring n = String::utrim (String::strip_trailing_numbers (name));
        if (n == "")
          n = _("Untitled");
        if (newname.empty () == true)
          newname =
            Shieldsetlist::instance ()->findFreeName
            (n, 100, num, m_shieldset->getTileSize ());
        //pretty weird that we do getTileSize on shieldset when it doesn't have one
        if (name == "")
          {
            if (newname.empty () == true)
              {
                Glib::ustring msg =
                  _("The shield set has an invalid name.\n"
                    "Change it and save again.");
                auto d = LwDialog::alert (msg);
                d->choose
                  (*this,
                   [this, d, after] (auto result)
                   {
                     d->choose_finish (result);
                     on_edit_shieldset_info_activated ();
                     after (false);
                   });
                return;
              }
            else
              {
                Glib::ustring msg =
                  String::ucompose (_("The shield set has an invalid name.\n"
                                      "Change it to '%1'?"), newname);
                auto d = LwDialog::alert_yn (msg);
                d->choose
                  (*this,
                   [this, d, existing, newname, after] (auto result)
                   {
                     bool change = d->choose_finish (result) == 1;
                     if (change)
                       {
                         m_shieldset->setName (newname);
                         check_name_valid2 (existing, newname, after);
                       }
                     else
                       after (change);
                   });
              }
          }
        else
          check_name_valid2 (existing, newname, after);

        return;
      }

    void check_name_valid2 (bool existing, Glib::ustring newname,
                            sigc::slot<void(bool)> after)
      {
        //okay the question is whether or not the name is already used.
        bool same_name = false;
        Glib::ustring file =
          Shieldsetlist::instance ()->lookupConfigurationFileByName (m_shieldset);
        if (file == "")
          return after (true);

        Glib::ustring cfgfile = m_shieldset->getConfigurationFile (true);

        if (existing) // this means we're doing File->Save
          {
            if (file == cfgfile)
              return after (true);
            same_name = true;
          }
        else // this means we're doing File->Save As
          same_name = true;

        if (same_name)
          {
            if (newname.empty () == true)
              {
                Glib::ustring msg =
                  _("The shield set has the same name as another one.\n"
                    "Change it and save again.");
                auto d = LwDialog::alert (msg);
                d->choose
                  (*this,
                   [this, d, after] (auto result)
                   {
                     d->choose_finish (result);
                     on_edit_shieldset_info_activated ();
                     after (false);
                   });
              }
            else
              {
                Glib::ustring msg =
                  String::ucompose
                  (_("The shield set has the same name as another one.\n"
                     "Change it to '%1' instead?"), newname);
                auto d = LwDialog::alert_yn (msg);
                d->choose
                  (*this,
                   [this, d, newname, after] (auto result)
                   {
                     bool change = d->choose_finish (result) == 1;
                     if (change)
                       m_shieldset->setName (newname);
                     after (change);
                   });
              }
          }
      }

    void check_save_valid (bool existing, sigc::slot<void(bool)> after)
      {
        check_name_valid
          (existing,
           [this, existing, after] (bool valid)
           {
             if (!valid)
               return after (false);

             if (m_shieldset->validate () == false)
               {
                 if (existing &&
                     GameMap::instance ()->getShieldsetId () ==
                     m_shieldset->getId ())
                   {
                     Glib::ustring errmsg =
                       _("shield set is invalid, and is also the current working "
                         "one.");
                     Glib::ustring msg = _("shield Set could not be saved.");
                     Glib::ustring detail = m_current_save_filename + "\n" + errmsg;
                     auto d = LwDialog::alert (msg, detail);
                     d->choose
                       (*this,
                        [this, d, after] (auto result)
                        {
                          d->choose_finish (result);
                          after (false);
                        });
                   }
                 else
                   {
                     Glib::ustring msg =
                       _("The shield set is invalid.  Do you want to proceed?");
                     auto d = LwDialog::alert_yn (msg);
                     d->choose
                       (*this,
                        [after, d] (auto result)
                        {
                          bool ret = d->choose_finish (result) == 1;
                          after (ret);
                        });
                   }
               }
             else
               return after (true);
           });
        return;
      }

    void setup_accels ()
      {
        auto actions = Gio::SimpleActionGroup::create();

        actions->add_action
          ("undo",
           ([this] ()
            {
              m_umgr->undo ();
              if (m_umgr->undo_empty () && !m_new_shieldset_needs_saving)
                m_shieldset_modified = false;
              update ();
            }));

        actions->add_action
          ("redo",
           ([this] ()
            {
              m_shieldset_modified = true;
              m_umgr->redo ();
              update ();
            }));

        insert_action_group ("win", actions);

        auto shortcuts = Gtk::ShortcutController::create();
        shortcuts->set_propagation_phase(Gtk::PropagationPhase::CAPTURE);

        shortcuts->add_shortcut
          (Gtk::Shortcut::create
           (Gtk::ShortcutTrigger::parse_string ("<Control>z"),
            Gtk::NamedAction::create ("win.undo")));

        shortcuts->add_shortcut
          (Gtk::Shortcut::create
           (Gtk::ShortcutTrigger::parse_string ("<Control>Y"),
            Gtk::NamedAction::create ("win.redo")));

        shortcuts->add_shortcut
          (Gtk::Shortcut::create
           (Gtk::ShortcutTrigger::parse_string ("<Control><Shift>Z"),
            Gtk::NamedAction::create ("win.redo")));

        add_controller (shortcuts);
      }

    void on_edit_shieldset_info_activated ()
      {
        auto d = LwDialog::build<ShieldSetInfoDialog> (this);
        d->setup (m_shieldset);
        d->signal_response ().connect
          ([this, d] (Gtk::ResponseType resp)
           {
             if (d->is_changed () &&
                 resp == Gtk::ResponseType::ACCEPT)
               {
                 ShieldSetUndoAction_Properties *action = 
                   new ShieldSetUndoAction_Properties
                   (m_shieldset->getName (), 
                    m_shieldset->getInfo (),
                    m_shieldset->getCopyright (),
                    m_shieldset->getLicense ());
                 m_umgr->add (action);

                 m_shieldset->setName (d->get_name ());
                 m_shieldset->setInfo (d->get_description ());
                 m_shieldset->setCopyright (d->get_copyright ());
                 m_shieldset->setLicense (d->get_license ());
                 update ();
               }
             delete d;
           });
      }

    void copy_white_down ()
      {
        Glib::ustring err = "";
        auto action = new ShieldSetUndoAction_WhiteDown (m_shieldset);
        m_umgr->add (action);

        auto w = m_shieldset->lookupShieldByColor (Shield::WHITE);
        for (guint32 i = Shield::WHITE + 1; i <= Shield::NEUTRAL; i++)
          {
            Shield *s = m_shieldset->lookupShieldByColor (i);
            for (auto ss : *s)
              {
                TarFileMaskedImage *mim = ss->getMaskedImage ();
                auto wss =
                  m_shieldset->lookupShieldByTypeAndColor (ss->getType (),
                                                           Shield::WHITE);
                wss->getMaskedImage ()->copy (m_shieldset, mim, err);
              }
            for (guint32 k = Tartan::LEFT; k <= Tartan::RIGHT; k++)
              {
                TarFileMaskedImage *mim =
                  s->getTartanMaskedImage (Tartan::Type (k));
                w->getTartanMaskedImage (Tartan::Type (k))->copy (m_shieldset,
                                                                  mim, err);
              }
          }
        bool broken = false;
        m_shieldset->instantiateImages (broken);
        update ();
      }

    void save_current_shieldset_file_as (sigc::slot<void(bool)> after)
      {
        LwDialog::save
          (*this, _("Choose a Name"), 
           File::sanify (m_shieldset->getName ()), FileFilter::SHIELDSET,
           [this, after] (std::string path)
           {
             Glib::ustring old_filename = m_current_save_filename;
             guint32 old_id = m_shieldset->getId ();
             m_shieldset->setId (Shieldsetlist::getNextAvailableId (old_id));

             save_current_shieldset_file
               (path,
                [this, after, path, old_filename, old_id] (bool saved)
                {
                  if (saved == false)
                    {
                      m_current_save_filename = old_filename;
                      m_shieldset->setId (old_id);
                    }
                  else
                    {
                      m_shieldset_modified = false;
                      m_new_shieldset_needs_saving = false;
                      m_shieldset->created (path);
                      Glib::ustring dir =
                        File::add_slash_if_necessary (File::get_dirname (path));
                      if (dir == File::get_shieldset_dir () ||
                          dir == File::get_user_shieldset_dir ())
                        {
                          //if we saved it to a standard place, update the list
                          Shieldsetlist::instance ()->add
                            (Shieldset::copy (m_shieldset), path);
                          m_shieldset_saved.emit (m_shieldset->getId ());
                        }
                      update_shieldset_panel ();
                      update_window_title( );
                    }
                  after (saved);
                });
           });
      }

    void save_current_shieldset_file (Glib::ustring filename, sigc::slot<void(bool)> after)
      {
        m_current_save_filename = filename;
        if (m_current_save_filename.empty ())
          m_current_save_filename = m_shieldset->getConfigurationFile (true);

        bool ok =
          m_shieldset->save (m_current_save_filename, Shieldset::file_extension);
        if (ok)
          {
            if (Shieldsetlist::instance ()->reload (m_shieldset->getId ()))
              update_shieldset_panel ();
            m_new_shieldset_needs_saving = false;
            m_shieldset_modified = false;
            update_window_title ();
            m_shieldset_saved.emit (m_shieldset->getId ());
            after (true);
          }
        else
          {
            Glib::ustring errmsg = Glib::strerror (errno);
            Glib::ustring msg = _("The shield set could not be saved.");

            auto dialog = LwDialog::alert (msg, errmsg);
            dialog->choose
              (*this,
               [dialog, after] (auto result)
               {
                 dialog->choose_finish (result);
                 after (false);
                 return;
               });
          }
        return;
      }

    void on_save_as_activated ()
      {
        check_save_valid
          (false,
           [this] (bool valid)
           {
             if (valid)
               {
                 save_current_shieldset_file_as
                   ([this] (bool saved)
                    {
                      (void) saved;
                    });
               }
           });

      }

    void check_quit (sigc::slot<void(bool)> after)
      {
        if (m_shieldset_modified || m_new_shieldset_needs_saving)
          {
            auto d = Gtk::AlertDialog::create (_("Save changes?"));
            d->set_detail (_("Save changes before closing?"));
            d->set_buttons
              ({_("Save"), _("Close without Saving"), _("Cancel")});
            d->set_cancel_button (2);
            d->set_default_button (0);
            d->set_modal (true);
            d->choose
              (*this,
               [this, d, after] (Glib::RefPtr<Gio::AsyncResult>& result)
               {
                 int resp = d->choose_finish (result);
                 switch (resp)
                   {
                   case 0: // save and exit
                     check_save_valid
                       (true,
                        [this, after] (bool valid)
                        {
                          if (!valid)
                            return after (false);

                          if (m_shieldset->getDirectory ().empty () == false)
                            {
                              save_current_shieldset_file_as
                                ([this, after] (bool saved)
                                 {
                                   after (saved);
                                 });
                            }
                          else
                            {
                              save_current_shieldset_file
                                ("",
                                 [this, after] (bool saved)
                                 {
                                   after (saved);
                                 });
                            }
                        });
                     break;

                   case 1: // close, exit
                     after (true);
                     break;

                   case 2: // cancel, don't exit
                     after (false);
                     break;

                   default:
                     after (false);
                     break;

                   }
               });
          }
        else
          after (true);
        return;
      }
        
    void setup_quit ()
      {
        auto controller = Gtk::EventControllerKey::create ();
        controller->signal_key_pressed().connect
          ([this] (guint keyval, guint, Gdk::ModifierType)
           {
             if (keyval == GDK_KEY_Escape)
               {
                 m_simple_actions["shieldset.file.quit"]->activate ();
                 return true;
               }
             return false;
           }, false);
        add_controller (controller);

        signal_close_request ().connect
          ([this] () -> bool
           {
             m_simple_actions["shieldset.file.quit"]->activate ();
             return true;
           }, false);
      }
};
#endif
