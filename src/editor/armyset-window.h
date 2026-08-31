//  Copyright (C) 2007, 2008, 2009, 2010, 2011, 2012, 2014, 2015, 2020, 2021,
//  2026 Ben Asselstine
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
#ifndef ARMYSET_WINDOW_H
#define ARMYSET_WINDOW_H

#include "army-set.h"
#include "tar-file-image.h"
#include "tar-file-masked-image.h"
#include "undo-mgr.h"
#include "armyset-undo.h"
#include "about-dialog.h"
#include "image-editor-dialog.h"
#include "masked-image-editor-dialog.h"
#include "armyset-info-dialog.h"
#include "file-label.h"
#include "armies-preview-dialog.h"
#include "lw-column.h"

class ArmySetMenuButton: public Gtk::MenuButton
{
public:
    ArmySetMenuButton (Gtk::Window &p, Glib::RefPtr<Gio::SimpleActionGroup> &group)
      :m_parent (p), m_actions (group)
      {
      }

    ~ArmySetMenuButton () override = default;

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

        menu->append (_("New"), "lw.armyset.file.new");
        menu->append (_("Open..."), "lw.armyset.file.open");
        menu->append (_("Save"), "lw.armyset.file.save");
        menu->append (_("Save As..."), "lw.armyset.file.save-as");
        menu->append (_("Validate"), "lw.armyset.file.validate");
        menu->append (_("Properties"), "lw.armyset.edit.properties");
        menu->append (_("Preview Armies"), "lw.armyset.view.preview-armies");
        auto help_menu = Gio::Menu::create ();
        help_menu->append (_("Tutorial Video"), "lw.armyset.help.tutorial-video");
        help_menu->append (_("About"), "lw.armyset.help.about");
        menu->append_submenu (_("Help"), help_menu);
        menu->append (_("Keyboard Shortcuts"), "lw.armyset.help.keyboard-shortcuts");
        menu->append (_("Quit"), "lw.armyset.file.quit");

        property_primary () = true;
        set_menu_model (menu);

        setup_actions
          ({"armyset.file.new", "armyset.file.open", "armyset.file.save",
            "armyset.file.save-as", "armyset.file.validate",
            "armyset.file.quit", "armyset.edit.properties",
            "armyset.edit.make-same", "armyset.view.preview-armies",
            "armyset.help.tutorial-video", "armyset.help.about",
            "armyset.help.keyboard-shortcuts",

            // the other actions not in the menu, it's just easier to add these
            // here
            "armyset.add-army", "armyset.remove-army",
            "armyset.army-up", "armyset.army-down",
          });

        set_icon_name ("open-menu-symbolic");

      }

    void setup_action (Glib::RefPtr<Gio::SimpleAction> a)
      {
        m_signal_action_added.emit (a);
      }
};

class ArmyProtoRow: public Glib::Object
{
public:
    ArmyProto *m_army;
    sigc::signal<void()> m_signal_changed;

    sigc::signal<void()> signal_changed ()
      {
        return m_signal_changed;
      }

    static Glib::RefPtr<ArmyProtoRow> create (ArmyProto *a)
      {
        return
          Glib::make_refptr_for_instance<ArmyProtoRow> (new ArmyProtoRow (a));
      }

    void changed ()
      {
        m_signal_changed.emit ();
      }

protected:
    ArmyProtoRow (ArmyProto *a)
      : m_army (a)
      {
      }
};

class ArmySetShortcutsDialog : public Gtk::Window
{
public:
    ArmySetShortcutsDialog ()
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

class ArmySetWindow: public Gtk::ApplicationWindow
{
public:

    ArmySetWindow ()
      : m_actions (Gio::SimpleActionGroup::create ())
      {
        m_armyset = NULL;
        set_size_request (750, 500);
        m_umgr = new UndoMgr (UndoMgr::DELAY, UndoMgr::LIMIT);
        m_umgr->signal_execute ().connect
          (sigc::mem_fun (*this, &ArmySetWindow::execute_action));
        m_umgr->signal_added_undo ().connect
          ([this] ()
           {
             m_armyset_modified = true;
             update_window_title ();
             update_actions ();
           });
        set_resizable (false);
      }

    ~ArmySetWindow ()
      {
        disconnect_signals ();
        disconnect_action_signals ();
        delete m_umgr;
      }

    void setup (Shieldset *shieldset, Armyset *armyset)
      {
        m_shieldset = shieldset;
        m_armyset = NULL;
        if (armyset)
          {
            bool broken = false;
            m_armyset = new Armyset (*armyset);
            m_armyset->instantiateImages (broken);
            m_armyset->setLoadTemporaryFile ();
            m_current_save_filename = armyset->getConfigurationFile (true);
          }
        m_armyset_modified = false;
        m_new_armyset_needs_saving = false;

        m_menu_button = Gtk::make_managed<ArmySetMenuButton>(*this, m_actions);
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
        fill_treeview ();

        sync_make_same ();

        connect_action_signals ();
        connect_signals ();

        setup_quit ();

        update ();
      }

    sigc::signal<void(guint32)> signal_armyset_saved ()
      {
        return m_armyset_saved;
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
    ArmySetMenuButton *m_menu_button;
    Gtk::StackSwitcher *m_switcher;
    std::map<Glib::ustring, Glib::RefPtr<Gio::SimpleAction>> m_simple_actions;
    Gtk::Button *m_ship_button;
    FileLabel *m_ship_filelabel;
    Gtk::Button *m_hero_flag_button;
    FileLabel *m_hero_flag_filelabel;
    Gtk::Button *m_bag_button;
    FileLabel *m_bag_filelabel;
    Gtk::Button *m_white_small_select_button;
    FileLabel *m_white_small_select_filelabel;
    Gtk::Button *m_white_large_select_button;
    FileLabel *m_white_large_select_filelabel;
    Gtk::Button *m_green_small_select_button;
    FileLabel *m_green_small_select_filelabel;
    Gtk::Button *m_green_large_select_button;
    FileLabel *m_green_large_select_filelabel;
    Gtk::Button *m_yellow_small_select_button;
    FileLabel *m_yellow_small_select_filelabel;
    Gtk::Button *m_yellow_large_select_button;
    FileLabel *m_yellow_large_select_filelabel;
    Gtk::Button *m_dark_blue_small_select_button;
    FileLabel *m_dark_blue_small_select_filelabel;
    Gtk::Button *m_dark_blue_large_select_button;
    FileLabel *m_dark_blue_large_select_filelabel;
    Gtk::Button *m_orange_small_select_button;
    FileLabel *m_orange_small_select_filelabel;
    Gtk::Button *m_orange_large_select_button;
    FileLabel *m_orange_large_select_filelabel;
    Gtk::Button *m_light_blue_small_select_button;
    FileLabel *m_light_blue_small_select_filelabel;
    Gtk::Button *m_light_blue_large_select_button;
    FileLabel *m_light_blue_large_select_filelabel;
    Gtk::Button *m_red_small_select_button;
    FileLabel *m_red_small_select_filelabel;
    Gtk::Button *m_red_large_select_button;
    FileLabel *m_red_large_select_filelabel;
    Gtk::Button *m_black_small_select_button;
    FileLabel *m_black_small_select_filelabel;
    Gtk::Button *m_black_large_select_button;
    FileLabel *m_black_large_select_filelabel;
    Gtk::Entry *m_name_entry;
    Gtk::Button *m_white_image_button;
    FileLabel *m_white_image_filelabel;
    Gtk::Switch *m_make_same_switch;
    Gtk::Button *m_green_image_button;
    FileLabel *m_green_image_filelabel;
    Gtk::Button *m_yellow_image_button;
    FileLabel *m_yellow_image_filelabel;
    Gtk::Button *m_light_blue_image_button;
    FileLabel *m_light_blue_image_filelabel;
    Gtk::Button *m_orange_image_button;
    FileLabel *m_orange_image_filelabel;
    Gtk::Button *m_dark_blue_image_button;
    FileLabel *m_dark_blue_image_filelabel;
    Gtk::Button *m_red_image_button;
    FileLabel *m_red_image_filelabel;
    Gtk::Button *m_black_image_button;
    FileLabel *m_black_image_filelabel;
    Gtk::Button *m_neutral_image_button;
    FileLabel *m_neutral_image_filelabel;
    Gtk::SpinButton *m_production_spinbutton;
    Gtk::SpinButton *m_cost_spinbutton;
    Gtk::SpinButton *m_new_cost_spinbutton;
    Gtk::SpinButton *m_upkeep_spinbutton;
    Gtk::SpinButton *m_strength_spinbutton;
    Gtk::SpinButton *m_moves_spinbutton;
    Gtk::SpinButton *m_exp_spinbutton;
    Gtk::SpinButton *m_id_spinbutton;
    LwCombo *m_hero_combobox;
    Gtk::Switch *m_awardable_switch;
    Gtk::Switch *m_defends_ruins_switch;
    Gtk::SpinButton *m_sight_spinbutton;
    Gtk::Switch *m_move_forests_switch;
    Gtk::Switch *m_move_marshes_switch;
    Gtk::Switch *m_move_hills_switch;
    Gtk::Switch *m_move_mountains_switch;
    Gtk::Switch *m_can_fly_switch;
    Gtk::Switch *m_add1strinopen_switch;
    Gtk::Switch *m_add2strinopen_switch;
    Gtk::Switch *m_add1strinforest_switch;
    Gtk::Switch *m_add2strinforest_switch;
    Gtk::Switch *m_add1strinhills_switch;
    Gtk::Switch *m_add2strinhills_switch;
    Gtk::Switch *m_add1strincity_switch;
    Gtk::Switch *m_add2strincity_switch;
    Gtk::Switch *m_add1stackinhills_switch;
    Gtk::Switch *m_suballcitybonus_switch;
    Gtk::Switch *m_sub1enemystack_switch;
    Gtk::Switch *m_sub2enemystack_switch;
    Gtk::Switch *m_add1stack_switch;
    Gtk::Switch *m_add2stack_switch;
    Gtk::Switch *m_suballnonherobonus_switch;
    Gtk::Switch *m_suballherobonus_switch;
    Gtk::Switch *m_confer_move_bonus_switch;
    Gtk::ScrolledWindow *m_scrolled_window;
    Gtk::ScrolledWindow *m_columnview_scrolled_window;

    Gtk::ColumnView *m_treeview;
    Glib::RefPtr<Gtk::SingleSelection> m_selection_model;
    Glib::RefPtr<Gio::ListStore<ArmyProtoRow>> m_store;

    std::list<sigc::connection> m_action_connections;
    std::list<sigc::connection> m_connections;

    Shieldset *m_shieldset;
    Armyset *m_armyset;
    Glib::ustring m_current_save_filename;
    bool m_armyset_modified;
    bool m_new_armyset_needs_saving;
    UndoMgr *m_umgr;

    bool m_make_same;

    sigc::signal<void(guint32)> m_armyset_saved;
    sigc::signal<void()> m_signal_closed;

    void setup_header_bar (Gtk::MenuButton *menu_button)
      {
        m_header_bar = Gtk::make_managed<Gtk::HeaderBar>();
        m_header_bar->set_show_title_buttons (true);

        m_notebook = Gtk::make_managed<Gtk::Notebook> ();
        m_notebook->set_show_tabs (false);
        m_notebook->set_show_border (false);

        auto page1 = Gtk::make_managed<Gtk::Box> (Gtk::Orientation::VERTICAL);
        auto title1 = Gtk::make_managed<Gtk::Label>(_("Army Set Editor"));
        title1->add_css_class ("title");
        page1->append (*title1);
        page1->set_valign (Gtk::Align::CENTER);
        m_notebook->append_page (*page1, "");

        auto page2 = Gtk::make_managed<Gtk::Box> (Gtk::Orientation::VERTICAL);
        auto title2 = Gtk::make_managed<Gtk::Label>(_("Army Set Editor"));
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
          ("armyset.file.new",
           [this] ()
           {
             check_discard
               (_("Save these changes before making a new army set?"),
                [this] (bool discard)
                {
                  if (discard)
                    {
                      m_current_save_filename = "";
                      disconnect_signals ();
                      if (m_armyset)
                        delete m_armyset;

                      guint32 num = 0;
                      Glib::ustring name =
                        Armysetlist::instance ()->findFreeName
                        (_("Untitled"), 100, num,
                         Armyset::get_default_tile_size ());

                      m_armyset =
                        new Armyset (Armysetlist::getNextAvailableId (1), name);
                      m_armyset->setNewTemporaryFile ();
                      connect_signals ();

                      m_armyset_modified = false;
                      m_new_armyset_needs_saving = true;
                      m_umgr->clear ();
                      fill_treeview ();
                      update ();
                      update_actions ();
                    }
                });
           });

        action_connect
          ("armyset.file.open",
           [this] ()
           {
             check_discard
               (_("Save these changes before making a new army set?"),
                [this] (bool discard)
                {
                  if (discard)
                    {
                      LwDialog::open
                        (*this, _("Choose a army set to open"),
                         FileFilter::ARMYSET,
                         [this] (std::string path)
                         {
                           load_armyset
                             (path,
                              [this] (bool loaded)
                              {
                                if (loaded)
                                  {
                                    m_armyset_modified = false;
                                    m_new_armyset_needs_saving = false;
                                    update ();
                                  }
                              });
                         });
                    }
                });
           });

        action_connect
          ("armyset.file.save",
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
                            save_current_armyset_file
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
          ("armyset.file.save-as",
           [this] ()
           {
             on_save_as_activated ();

           });

        action_connect
          ("armyset.file.validate",
           [this] ()
           {
             std::list<Glib::ustring> msgs;
             if (m_armyset == NULL)
               return;
             if (msgs.empty () == true)
               {
                 bool valid = String::utrim (m_armyset->getName ()) != "";
                 if (!valid)
                   {
                     Glib::ustring s = _("The name of the army set is invalid.");
                     msgs.push_back (s);
                   }
               }

             if (m_armyset->size () == 0)
               msgs.push_back
                 (_("There must be at least one army unit in the army set."));

             if (!m_armyset->validateHero ())
               msgs.push_back
                 (_("There must be at least one hero in the army set."));

             if (!m_armyset->validatePurchasables ())
               msgs.push_back
                 (_("There must be at least one army unit with a production cost of more than zero."));

             if (!m_armyset->validateRuinDefenders ())
               msgs.push_back
                 (_("There must be at least one army unit than can defend a ruin."));

             if (!m_armyset->validateAwardables ())
               msgs.push_back
                 (_("There must be at least one army unit than can be awarded to a hero."));

             if (!m_armyset->validateShip ())
               msgs.push_back (_("The ship image must be set."));

             if (!m_armyset->validateStandard ())
               msgs.push_back (_("The hero's standard (the flag) image must be set."));

             if (!m_armyset->validateBag ())
               msgs.push_back (_("The picture for the bag of items must be set."));

             for (auto army : *m_armyset)
               {
                 Shield::Color c;
                 if (!m_armyset->validateArmyUnitImage (army, c))
                   {
                     msgs.push_back
                       (String::ucompose
                        (_("%1 does not have an image for the %2 player"),
                         army->getName (), Shield::colorToString (c)));
                     break;
                   }
               }

             if (!m_armyset->validateArmyUnitNames ())
               msgs.push_back (_("An army unit does not have a name."));

             if (is_valid_name () == false)
               msgs.push_back (_("The name of the army set is not unique."));

             Glib::ustring msg = "";
             for (auto m : msgs)
               {
                 msg += m + "\n";
                 break; // we only show one
               }

             if (msg == "")
               msg = _("The army set is valid.");

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
          ("armyset.edit.make-same",
           [this] ()
           {
             auto a = get_selected_army ();
             if (!a)
               return;
             TarFileMaskedImage *wmim = a->getMaskedImage (Shield::WHITE);
             if (wmim->getName ().empty () == true)
               return;
             for (guint32 i = Shield::WHITE + 1; i <= Shield::NEUTRAL; i++)
               {
                 auto mim = a->getMaskedImage (Shield::Color (i));
                 if (wmim->getName () != mim->getName () &&
                     mim->getName ().empty () == false)
                   {
                     Glib::ustring err;
                     m_armyset->removeFileInCfgFile (mim->getName (), err);
                   }
                 mim->load (m_armyset, wmim->getName ());
                 mim->instantiateImages ();
               }
             update ();
           });

        action_connect
          ("armyset.view.preview-armies",
           [this] ()
           {
             auto d = LwDialog::build<ArmiesPreviewDialog> (this);
             d->setup (m_shieldset, m_armyset);
             d->signal_response ().connect
               ([d] (Gtk::ResponseType)
                {
                  delete d;
                });
           });

        action_connect
          ("armyset.file.quit",
           [this] ()
           {
             check_quit
               ([this] (bool quit)
                {
                  if (quit)
                    {
                      if (m_armyset)
                        delete m_armyset;
                      hide ();
                      m_signal_closed.emit ();
                    }
                });
           });

        action_connect
          ("armyset.edit.properties",
           [this] ()
           {
             on_edit_armyset_info_activated ();
           });

        action_connect
          ("armyset.add-army",
           [this] ()
           {
             m_umgr->add
               (new ArmySetUndoAction_AddArmy (m_armyset));

             ArmyProto *a = new ArmyProto ();
             a->setName (_("Untitled"));
             if (m_armyset->empty () == true)
               a->setId (0);
             else
               a->setId (m_armyset->getMaxId () + 1);
             m_armyset->push_back (a);
             m_armyset_modified = true;

             m_store->append (ArmyProtoRow::create (a));

             guint n = m_selection_model->get_n_items ();
             if (n > 0)
               m_selection_model->set_selected (n - 1);

             scroll_army_to_top ();
             scroll_treeview_to_bottom ();

             update ();
           });

        action_connect
          ("armyset.remove-army",
           [this] ()
           {
             auto selected = m_selection_model->get_selected_item ();
             if (selected)
               {
                 m_umgr->add
                   (new ArmySetUndoAction_RemoveArmy (m_armyset));
                 auto army = get_selected_army ();
                 m_armyset->remove (army);

                 m_store->remove (m_selection_model->get_selected ());
                 m_armyset_modified = true;
                 scroll_army_to_top ();
                 update ();
               }
           });

        action_connect
          ("armyset.army-up",
           [this] ()
           {
             int i = m_selection_model->get_selected ();
             if (i <= 0)
               return;

             auto item = m_store->get_item (i);
             if (!item)
               return;

             m_umgr->add
               (new ArmySetUndoAction_Reorder (m_armyset));

             m_store->remove (i);
             m_store->insert (i - 1, item);

             reselection (item);

             auto index = m_selection_model->get_selected ();
             if (index < m_store->get_n_items ())
               m_treeview->scroll_to (index);
             update_actions ();
           });

        action_connect
          ("armyset.army-down",
           [this] ()
           {
             int i = m_selection_model->get_selected ();
             if (i < 0 || i + 1 >= (int)m_store->get_n_items ())
               return;

             auto item = m_store->get_item (i);
             if (!item)
               return;

             m_umgr->add
               (new ArmySetUndoAction_Reorder (m_armyset));

             m_store->remove (i);
             m_store->insert (i + 1, item);

             reselection (item);

             //fixme, it doesn't scroll correctly here, the one we moved is
             //off screen down one record.
             //we try to fix it with +1 so we can at least see it,
             //but this causes a problem when we move a record to the bottom.
             //i think most ppl will sort upwards and not downwards to the very
             //bottom, so maybe people won't interact with this bug.
             auto index = m_selection_model->get_selected ();
             if (index + 1 < m_store->get_n_items ())
               m_treeview->scroll_to (index + 1);
             update_actions ();
           });

        action_connect
          ("armyset.help.tutorial-video",
           [this] ()
           {
             Glib::ustring uri = "https://vimeo.com/407659798";
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
          ("armyset.help.keyboard-shortcuts",
           [this] ()
           {
             auto* dialog = Gtk::make_managed<ArmySetShortcutsDialog>();
             dialog->set_transient_for (*this);
             dialog->present ();
           });

        action_connect
          ("armyset.help.about",
           [this] ()
           {
             auto d = Gtk::make_managed<AboutDialog> (*this);
             d->set_program_name ("LordsAWar! Army Set Editor");
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

    void block_signals ()
      {
        for (auto c : m_connections)
          c.block ();
      }

    void unblock_signals ()
      {
        for (auto c : m_connections)
          c.unblock ();
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

    void load_armyset (Glib::ustring filename, sigc::slot<void(bool)> after)
      {
        Glib::ustring old = m_current_save_filename;
        m_current_save_filename = filename;

        Armyset::create
          (filename,
           [this, after, old] (Armyset *armyset, bool broken,
                               bool unsupported_version, Glib::ustring err)
           {
             if (armyset == NULL || unsupported_version || broken)
               {
                 Glib::ustring msg = _("The army set could not be loaded.");
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
             Armyset *old_armyset = m_armyset;
             m_armyset = NULL;
             update ();
             update_actions ();
             m_armyset = armyset;
             fill_treeview ();
             sync_make_same ();
             update ();
             connect_signals ();
             m_armyset->setLoadTemporaryFile ();

             Glib::signal_idle ().connect
               ([this, old_armyset, after] ()
                {
                  bool broke = false;
                  m_armyset->instantiateImages (broke);
                  if (broke)
                    {
                      delete m_armyset;
                      m_armyset = NULL;
                      Glib::ustring msg = _("Couldn't load army set images");
                      auto dialog = LwDialog::alert (msg);
                      dialog->choose
                        (*this,
                         [this, old_armyset, after, dialog] (auto result)
                         {
                           dialog->choose_finish (result);
                           m_armyset = old_armyset;
                           update ();
                           return after (false);
                         });
                    }
                  else
                    {
                      delete old_armyset;
                      update ();
                      after (true);
                    }
                  return false;
                });
           });
      }

    ArmyProto *get_selected_army ()
      {
        auto item = m_selection_model->get_selected_item ();
        if (!item)
          return NULL;
        auto row = std::dynamic_pointer_cast<ArmyProtoRow>(item);
        return row->m_army;
      }

    guint32 get_selected_index ()
      {
        return m_selection_model->get_selected ();
      }

    void set_move_bonus (Gtk::Switch *sw, ArmyProto *a, guint32 val)
      {
        guint32 bonus = a->getMoveBonus ();
        if (sw->get_active () == true)
          bonus |= val;
        else
          {
            if (bonus & val)
              bonus ^= val;
          }
        a->setMoveBonus (bonus);
      }

    void set_army_bonus (Gtk::Switch *sw, ArmyProto *a, guint32 val)
      {
        guint32 bonus = a->getArmyBonus ();
        if (sw->get_active () == true)
          bonus |= val;
        else
          {
            if (bonus & val)
              bonus ^= val;
          }
        a->setArmyBonus (bonus);
      }

    void connect_signals ()
      {
        add_connection
          (m_name_entry->signal_changed ().connect
           ([this] ()
            {
              auto a = get_selected_army ();
              m_umgr->add
                (new ArmySetUndoAction_Name (get_selected_index (),
                                             a->getName (), m_umgr,
                                             m_name_entry));
              a->setName (m_name_entry->get_text ());

              auto item = m_selection_model->get_selected_item ();
              if (item)
                {
                  auto row = std::dynamic_pointer_cast<ArmyProtoRow>(item);
                  row->changed ();
                }
            }));

        add_connection
          (m_white_image_button->signal_clicked ().connect
           ([this] ()
            {
              auto a = get_selected_army ();
              on_army_change_clicked
                (String::ucompose (_("Select a white image for %1"),
                                   a->getName ()),
                 a->getMaskedImage (Shield::WHITE),
                 Shield::WHITE);
            }));

        add_connection
          (m_make_same_switch->property_active ().signal_changed ().connect
           ([this] ()
            {
             m_umgr->add
               (new ArmySetUndoAction_MakeSame (m_armyset, m_make_same));
              bool active = m_make_same_switch->get_active ();
              if (active)
                m_simple_actions["armyset.edit.make-same"]->activate ();
              m_make_same = active;
              update ();
            }));

        add_connection
          (m_green_image_button->signal_clicked ().connect
           ([this] ()
            {
              auto a = get_selected_army ();
              on_army_change_clicked
                (String::ucompose (_("Select a green image for %1"),
                                   a->getName ()),
                 a->getMaskedImage (Shield::GREEN),
                 Shield::GREEN);
            }));

        add_connection
          (m_yellow_image_button->signal_clicked ().connect
           ([this] ()
            {
              auto a = get_selected_army ();
              on_army_change_clicked
                (String::ucompose (_("Select a yellow image for %1"),
                                   a->getName ()),
                 a->getMaskedImage (Shield::YELLOW),
                 Shield::YELLOW);
            }));

        add_connection
          (m_light_blue_image_button->signal_clicked ().connect
           ([this] ()
            {
              auto a = get_selected_army ();
              on_army_change_clicked
                (String::ucompose (_("Select a light blue image for %1"),
                                   a->getName ()),
                 a->getMaskedImage (Shield::LIGHT_BLUE),
                 Shield::LIGHT_BLUE);
            }));

        add_connection
          (m_red_image_button->signal_clicked ().connect
           ([this] ()
            {
              auto a = get_selected_army ();
              on_army_change_clicked
                (String::ucompose (_("Select a red image for %1"),
                                   a->getName ()),
                 a->getMaskedImage (Shield::RED),
                 Shield::RED);
            }));

        add_connection
          (m_dark_blue_image_button->signal_clicked ().connect
           ([this] ()
            {
              auto a = get_selected_army ();
              on_army_change_clicked
                (String::ucompose (_("Select a dark blue image for %1"),
                                   a->getName ()),
                 a->getMaskedImage (Shield::DARK_BLUE),
                 Shield::DARK_BLUE);
            }));

        add_connection
          (m_orange_image_button->signal_clicked ().connect
           ([this] ()
            {
              auto a = get_selected_army ();
              on_army_change_clicked
                (String::ucompose (_("Select a orange image for %1"),
                                   a->getName ()),
                 a->getMaskedImage (Shield::ORANGE),
                 Shield::ORANGE);
            }));

        add_connection
          (m_black_image_button->signal_clicked ().connect
           ([this] ()
            {
              auto a = get_selected_army ();
              on_army_change_clicked
                (String::ucompose (_("Select a black image for %1"),
                                   a->getName ()),
                 a->getMaskedImage (Shield::BLACK),
                 Shield::BLACK);
            }));

        add_connection
          (m_neutral_image_button->signal_clicked ().connect
           ([this] ()
            {
              auto a = get_selected_army ();
              on_army_change_clicked
                (String::ucompose (_("Select a neutral image for %1"),
                                   a->getName ()),
                 a->getMaskedImage (Shield::NEUTRAL),
                 Shield::NEUTRAL);
            }));

        add_connection
          (m_production_spinbutton->signal_value_changed ().connect
           ([this] ()
            {
              auto a = get_selected_army ();
              m_umgr->add
                (new ArmySetUndoAction_Turns (get_selected_index (),
                                              a->getProduction ()));
              a->setProduction (int(m_production_spinbutton->get_value ()));
            }));

        add_connection
          (m_cost_spinbutton->signal_value_changed ().connect
           ([this] ()
            {
              auto a = get_selected_army ();
              m_umgr->add
                (new ArmySetUndoAction_Cost (get_selected_index (),
                                             a->getProductionCost ()));
              a->setProductionCost (int(m_cost_spinbutton->get_value ()));
            }));

        add_connection
          (m_new_cost_spinbutton->signal_value_changed ().connect
           ([this] ()
            {
              auto a = get_selected_army ();
              m_umgr->add
                (new ArmySetUndoAction_NewCost (get_selected_index (),
                                                a->getNewProductionCost ()));
              a->setNewProductionCost
                (int(m_new_cost_spinbutton->get_value ()));
            }));

        add_connection
          (m_upkeep_spinbutton->signal_value_changed ().connect
           ([this] ()
            {
              auto a = get_selected_army ();
              m_umgr->add
                (new ArmySetUndoAction_Upkeep (get_selected_index (),
                                               a->getUpkeep ()));
              a->setUpkeep (int(m_upkeep_spinbutton->get_value ()));
            }));

        add_connection
          (m_strength_spinbutton->signal_value_changed ().connect
           ([this] ()
            {
              auto a = get_selected_army ();
              m_umgr->add
                (new ArmySetUndoAction_Stat (get_selected_index (),
                                             ArmyBase::STRENGTH,
                                             a->getStrength ()));
              a->setStrength (int(m_strength_spinbutton->get_value ()));
            }));

        add_connection
          (m_moves_spinbutton->signal_value_changed ().connect
           ([this] ()
            {
              auto a = get_selected_army ();
              m_umgr->add
                (new ArmySetUndoAction_Stat (get_selected_index (),
                                             ArmyBase::MOVES,
                                             a->getMaxMoves ()));
              a->setMaxMoves (int(m_moves_spinbutton->get_value()));
            }));

        add_connection
          (m_exp_spinbutton->signal_value_changed ().connect
           ([this] ()
            {
              auto a = get_selected_army ();
              m_umgr->add
                (new ArmySetUndoAction_Exp (get_selected_index (),
                                            a->getXpReward ()));
              a->setXpReward (int(m_exp_spinbutton->get_value ()));
            }));

        add_connection
          (m_hero_combobox->signal_changed ().connect
           ([this] ()
            {
              auto a = get_selected_army ();
              m_umgr->add
                (new ArmySetUndoAction_Hero (get_selected_index (),
                                             a->getGender ()));
              a->setGender
                (Hero::Gender(m_hero_combobox->get_active_row_number ()));
            }));

        add_connection
          (m_awardable_switch->property_active ().signal_changed ().connect
           ([this] ()
            {
              auto a = get_selected_army ();
              m_umgr->add
                (new ArmySetUndoAction_RuinAward (get_selected_index (),
                                                  a->getAwardable ()));
              a->setAwardable (m_awardable_switch->get_active ());
            }));

        add_connection
          (m_defends_ruins_switch->property_active ().signal_changed ().connect
           ([this] ()
            {
              auto a = get_selected_army ();
              m_umgr->add
                (new ArmySetUndoAction_DefendsRuins (get_selected_index (),
                                                     a->getDefendsRuins ()));
              a->setDefendsRuins (m_defends_ruins_switch->get_active ());
            }));

        add_connection
          (m_sight_spinbutton->signal_value_changed ().connect
           ([this] ()
            {
              auto a = get_selected_army ();
              m_umgr->add
                (new ArmySetUndoAction_Stat (get_selected_index (),
                                             ArmyBase::SIGHT,
                                             a->getSight ()));
              a->setSight (int(m_sight_spinbutton->get_value ()));
            }));

        add_connection
          (m_id_spinbutton->signal_value_changed ().connect
           ([this] ()
            {
              auto a = get_selected_army ();
              m_umgr->add
                (new ArmySetUndoAction_Id (get_selected_index (), a->getId ()));
              a->setId (int(m_id_spinbutton->get_value ()));
            }));

        add_connection
          (m_move_forests_switch->property_active ().signal_changed ().connect
           ([this] ()
            {
              auto a = get_selected_army ();
              m_umgr->add
                (new ArmySetUndoAction_FasterInForest (get_selected_index (),
                                                       a->getMoveBonus ()));
              set_move_bonus (m_move_forests_switch, a, Tile::FOREST);
            }));

        add_connection
          (m_move_marshes_switch->property_active ().signal_changed ().connect
           ([this] ()
            {
              auto a = get_selected_army ();
              m_umgr->add
                (new ArmySetUndoAction_FasterInMarsh (get_selected_index (),
                                                      a->getMoveBonus ()));
              set_move_bonus (m_move_marshes_switch, a, Tile::SWAMP);
            }));

        add_connection
          (m_move_hills_switch->property_active ().signal_changed ().connect
           ([this] ()
            {
              auto a = get_selected_army ();
              m_umgr->add
                (new ArmySetUndoAction_FasterInHills (get_selected_index (),
                                                      a->getMoveBonus ()));
              set_move_bonus (m_move_hills_switch, a, Tile::HILLS);
            }));

        add_connection
          (m_move_mountains_switch->property_active ().signal_changed ().connect
           ([this] ()
            {
              auto a = get_selected_army ();
              m_umgr->add
                (new ArmySetUndoAction_FasterInMountains (get_selected_index (),
                                                          a->getMoveBonus ()));
              set_move_bonus (m_move_mountains_switch, a, Tile::MOUNTAIN);
            }));

        add_connection
          (m_can_fly_switch->property_active ().signal_changed ().connect
           ([this] ()
            {
              auto a = get_selected_army ();
              m_umgr->add
                (new ArmySetUndoAction_Fly (get_selected_index (),
                                            a->getMoveBonus ()));

              block_signals ();
              bool active = m_can_fly_switch->get_active ();
              m_move_mountains_switch->set_active (active);
              m_move_hills_switch->set_active (active);
              m_move_marshes_switch->set_active (active);
              m_move_forests_switch->set_active (active);
              unblock_signals ();

              set_move_bonus (m_can_fly_switch, a,
                              Tile::GRASS | Tile::WATER |
                              Tile::FOREST | Tile::HILLS |
                              Tile::MOUNTAIN | Tile::SWAMP);
            }));

        add_connection
          (m_add1strinopen_switch->property_active ().signal_changed ().connect
           ([this] ()
            {
              auto a = get_selected_army ();
              ArmyBase::Bonus val = ArmyBase::ADD1STRINOPEN;
              m_umgr->add
                (new ArmySetUndoAction_Bonus (get_selected_index (), val,
                  (a->getArmyBonus () & val) != 0));
              set_army_bonus (m_add1strinopen_switch, a, val);
            }));

        add_connection
          (m_add2strinopen_switch->property_active ().signal_changed ().connect
           ([this] ()
            {
              auto a = get_selected_army ();
              ArmyBase::Bonus val = ArmyBase::ADD2STRINOPEN;
              m_umgr->add
                (new ArmySetUndoAction_Bonus (get_selected_index (), val,
                  (a->getArmyBonus () & val) != 0));
              set_army_bonus (m_add2strinopen_switch, a, val);
            }));

        add_connection
          (m_add1strinforest_switch->property_active ().signal_changed ().connect
           ([this] ()
            {
              auto a = get_selected_army ();
              ArmyBase::Bonus val = ArmyBase::ADD1STRINFOREST;
              m_umgr->add
                (new ArmySetUndoAction_Bonus (get_selected_index (), val,
                  (a->getArmyBonus () & val) != 0));
              set_army_bonus (m_add1strinforest_switch, a, val);
            }));

        add_connection
          (m_add2strinforest_switch->property_active ().signal_changed ().connect
           ([this] ()
            {
              auto a = get_selected_army ();
              ArmyBase::Bonus val = ArmyBase::ADD2STRINFOREST;
              m_umgr->add
                (new ArmySetUndoAction_Bonus (get_selected_index (), val,
                  (a->getArmyBonus () & val) != 0));
              set_army_bonus (m_add2strinforest_switch, a, val);
            }));

        add_connection
          (m_add1strinhills_switch->property_active ().signal_changed ().connect
           ([this] ()
            {
              auto a = get_selected_army ();
              ArmyBase::Bonus val = ArmyBase::ADD1STRINHILLS;
              m_umgr->add
                (new ArmySetUndoAction_Bonus (get_selected_index (), val,
                  (a->getArmyBonus () & val) != 0));
              set_army_bonus (m_add1strinhills_switch, a, val);
            }));

        add_connection
          (m_add2strinhills_switch->property_active ().signal_changed ().connect
           ([this] ()
            {
              auto a = get_selected_army ();
              ArmyBase::Bonus val = ArmyBase::ADD2STRINHILLS;
              m_umgr->add
                (new ArmySetUndoAction_Bonus (get_selected_index (), val,
                  (a->getArmyBonus () & val) != 0));
              set_army_bonus (m_add2strinhills_switch, a, val);
            }));

        add_connection
          (m_add1strincity_switch->property_active ().signal_changed ().connect
           ([this] ()
            {
              auto a = get_selected_army ();
              ArmyBase::Bonus val = ArmyBase::ADD1STRINCITY;
              m_umgr->add
                (new ArmySetUndoAction_Bonus (get_selected_index (), val,
                  (a->getArmyBonus () & val) != 0));
              set_army_bonus (m_add1strincity_switch, a, val);
            }));

        add_connection
          (m_add2strincity_switch->property_active ().signal_changed ().connect
           ([this] ()
            {
              auto a = get_selected_army ();
              ArmyBase::Bonus val = ArmyBase::ADD2STRINCITY;
              m_umgr->add
                (new ArmySetUndoAction_Bonus (get_selected_index (), val,
                  (a->getArmyBonus () & val) != 0));
              set_army_bonus (m_add2strincity_switch, a, val);
            }));

        add_connection
          (m_add1stackinhills_switch->property_active ().signal_changed ().connect
           ([this] ()
            {
              auto a = get_selected_army ();
              ArmyBase::Bonus val = ArmyBase::ADD1STACKINHILLS;
              m_umgr->add
                (new ArmySetUndoAction_Bonus (get_selected_index (), val,
                  (a->getArmyBonus () & val) != 0));
              set_army_bonus (m_add1stackinhills_switch, a, val);
            }));

        add_connection
          (m_suballcitybonus_switch->property_active ().signal_changed ().connect
           ([this] ()
            {
              auto a = get_selected_army ();
              ArmyBase::Bonus val = ArmyBase::SUBALLCITYBONUS;
              m_umgr->add
                (new ArmySetUndoAction_Bonus (get_selected_index (), val,
                  (a->getArmyBonus () & val) != 0));
              set_army_bonus (m_suballcitybonus_switch, a, val);
            }));

        add_connection
          (m_sub1enemystack_switch->property_active ().signal_changed ().connect
           ([this] ()
            {
              auto a = get_selected_army ();
              ArmyBase::Bonus val = ArmyBase::SUB1ENEMYSTACK;
              m_umgr->add
                (new ArmySetUndoAction_Bonus (get_selected_index (), val,
                  (a->getArmyBonus () & val) != 0));
              set_army_bonus (m_sub1enemystack_switch, a, val);
            }));

        add_connection
          (m_sub2enemystack_switch->property_active ().signal_changed ().connect
           ([this] ()
            {
              auto a = get_selected_army ();
              ArmyBase::Bonus val = ArmyBase::SUB2ENEMYSTACK;
              m_umgr->add
                (new ArmySetUndoAction_Bonus (get_selected_index (), val,
                  (a->getArmyBonus () & val) != 0));
              set_army_bonus (m_sub2enemystack_switch, a, val);
            }));

        add_connection
          (m_add1stack_switch->property_active ().signal_changed ().connect
           ([this] ()
            {
              auto a = get_selected_army ();
              ArmyBase::Bonus val = ArmyBase::ADD1STACK;
              m_umgr->add
                (new ArmySetUndoAction_Bonus (get_selected_index (), val,
                  (a->getArmyBonus () & val) != 0));
              set_army_bonus (m_add1stack_switch, a, val);
            }));

        add_connection
          (m_add2stack_switch->property_active ().signal_changed ().connect
           ([this] ()
            {
              auto a = get_selected_army ();
              ArmyBase::Bonus val = ArmyBase::ADD2STACK;
              m_umgr->add
                (new ArmySetUndoAction_Bonus (get_selected_index (), val,
                  (a->getArmyBonus () & val) != 0));
              set_army_bonus (m_add2stack_switch, a, val);
            }));

        add_connection
          (m_suballnonherobonus_switch->property_active ().signal_changed ().connect
           ([this] ()
            {
              auto a = get_selected_army ();
              ArmyBase::Bonus val = ArmyBase::SUBALLNONHEROBONUS;
              m_umgr->add
                (new ArmySetUndoAction_Bonus (get_selected_index (), val,
                  (a->getArmyBonus () & val) != 0));
              set_army_bonus (m_suballnonherobonus_switch, a, val);
            }));

        add_connection
          (m_suballherobonus_switch->property_active ().signal_changed ().connect
           ([this] ()
            {
              auto a = get_selected_army ();
              ArmyBase::Bonus val = ArmyBase::SUBALLHEROBONUS;
              m_umgr->add
                (new ArmySetUndoAction_Bonus (get_selected_index (), val,
                  (a->getArmyBonus () & val) != 0));
              set_army_bonus (m_suballherobonus_switch, a, val);
            }));

        add_connection
          (m_confer_move_bonus_switch->property_active ().signal_changed ()
           .connect
           ([this] ()
           {
              auto a = get_selected_army ();
              ArmyBase::Bonus val = ArmyBase::CONFER_MOVE_BONUS;
              m_umgr->add
                (new ArmySetUndoAction_Bonus (get_selected_index (), val,
                  (a->getArmyBonus () & val) != 0));
              set_army_bonus (m_confer_move_bonus_switch, a, val);
           }));

        add_connection
          (m_white_small_select_button->signal_clicked ().connect
           ([this] ()
            {
              on_change_clicked (_("Select a small white selector image"),
                                 m_armyset->getSelector (false, Shield::WHITE),
                                 Shield::WHITE);
            }));

        add_connection
          (m_white_large_select_button->signal_clicked ().connect
           ([this] ()
            {
              on_change_clicked (_("Select a large white selector image"),
                                 m_armyset->getSelector (true, Shield::WHITE),
                                 Shield::WHITE);
            }));

        add_connection
          (m_green_small_select_button->signal_clicked ().connect
           ([this] ()
            {
              on_change_clicked (_("Select a small green selector image"),
                                 m_armyset->getSelector (false, Shield::GREEN),
                                 Shield::GREEN);
            }));

        add_connection
          (m_green_large_select_button->signal_clicked ().connect
           ([this] ()
            {
              on_change_clicked (_("Select a large green selector image"),
                                 m_armyset->getSelector (true, Shield::GREEN),
                                 Shield::GREEN);
            }));

        add_connection
          (m_yellow_small_select_button->signal_clicked ().connect
           ([this] ()
            {
              on_change_clicked (_("Select a small yellow selector image"),
                                 m_armyset->getSelector (false,
                                                         Shield::YELLOW),
                                 Shield::YELLOW);
            }));

        add_connection
          (m_yellow_large_select_button->signal_clicked ().connect
           ([this] ()
            {
              on_change_clicked (_("Select a large yellow selector image"),
                                 m_armyset->getSelector (true, Shield::YELLOW),
                                 Shield::YELLOW);
            }));

        add_connection
          (m_dark_blue_small_select_button->signal_clicked ().connect
           ([this] ()
            {
              on_change_clicked (_("Select a small dark blue selector image"),
                                 m_armyset->getSelector (false,
                                                         Shield::DARK_BLUE),
                                 Shield::DARK_BLUE);
            }));

        add_connection
          (m_dark_blue_large_select_button->signal_clicked ().connect
           ([this] ()
            {
              on_change_clicked (_("Select a large dark blue selector image"),
                                 m_armyset->getSelector (true,
                                                         Shield::DARK_BLUE),
                                 Shield::DARK_BLUE);
            }));

        add_connection
          (m_orange_small_select_button->signal_clicked ().connect
           ([this] ()
            {
              on_change_clicked (_("Select a small orange selector image"),
                                 m_armyset->getSelector (false,
                                                         Shield::ORANGE),
                                 Shield::ORANGE);
            }));

        add_connection
          (m_orange_large_select_button->signal_clicked ().connect
           ([this] ()
            {
              on_change_clicked (_("Select a large orange selector image"),
                                 m_armyset->getSelector (true, Shield::ORANGE),
                                 Shield::ORANGE);
            }));

        add_connection
          (m_light_blue_small_select_button->signal_clicked ().connect
           ([this] ()
            {
              on_change_clicked (_("Select a large light blue selector image"),
                                 m_armyset->getSelector (false,
                                                         Shield::LIGHT_BLUE),
                                 Shield::LIGHT_BLUE);
            }));

        add_connection
          (m_light_blue_large_select_button->signal_clicked ().connect
           ([this] ()
            {
              on_change_clicked (_("Select a large light blue selector image"),
                                 m_armyset->getSelector (true,
                                                         Shield::LIGHT_BLUE),
                                 Shield::LIGHT_BLUE);
            }));

        add_connection
          (m_red_small_select_button->signal_clicked ().connect
           ([this] ()
            {
              on_change_clicked (_("Select a small red selector image"),
                                 m_armyset->getSelector (false, Shield::RED),
                                 Shield::RED);
            }));

        add_connection
          (m_red_large_select_button->signal_clicked ().connect
           ([this] ()
            {
              on_change_clicked (_("Select a large red selector image"),
                                 m_armyset->getSelector (true, Shield::RED),
                                 Shield::RED);
            }));

        add_connection
          (m_black_small_select_button->signal_clicked ().connect
           ([this] ()
            {
              on_change_clicked (_("Select a small black selector image"),
                                 m_armyset->getSelector (false, Shield::BLACK),
                                 Shield::BLACK);
            }));

        add_connection
          (m_black_large_select_button->signal_clicked ().connect
           ([this] ()
            {
              on_change_clicked (_("Select a large black selector image"),
                                 m_armyset->getSelector (true, Shield::BLACK),
                                 Shield::BLACK);
            }));

        add_connection
          (m_ship_button->signal_clicked ().connect
           ([this] ()
            {
              on_change_clicked (_("Select a ship image"),
                                 m_armyset->getShip (), -1);
            }));

        add_connection
          (m_hero_flag_button->signal_clicked ().connect
           ([this] ()
            {
              on_change_clicked (_("Select a hero flag image"),
                                 m_armyset->getStandard (), -1);
            }));

        add_connection
          (m_bag_button->signal_clicked ().connect
           ([this] ()
            {
              on_change_clicked (_("Select a bag image"),
                                 m_armyset->getBag ());
            }));
      }

    void update ()
      {
        update_window_title ();
        update_armyset_panel ();
        update_actions ();
      }

    bool is_bottom_row_selected ()
      {
        const guint n_items = m_store->get_n_items ();
        if (n_items == 0)
          return false;

        const guint pos = m_selection_model->get_selected ();

        if (!m_selection_model->is_selected (pos))
          return false;

        return pos == (n_items - 1);
      }

    bool is_top_row_selected ()
      {
        const guint n_items = m_store->get_n_items ();
        if (n_items == 0)
          return false;

        const guint pos = m_selection_model->get_selected ();

        if (!m_selection_model->is_selected (pos))
          return false;

        return pos == 0;
      }

    void update_actions ()
      {
        bool a = m_armyset != NULL;
        m_simple_actions ["armyset.edit.properties"]->set_enabled (a);

        bool b =
          m_switcher->get_stack ()->get_visible_child_name () == "armies";
        m_simple_actions ["armyset.edit.make-same"]->set_enabled
          (a && b && m_armyset->size () > 0);

        m_simple_actions ["armyset.view.preview-armies"]->set_enabled
          (a && m_armyset->size () > 0);

        m_simple_actions ["armyset.file.validate"]->set_enabled (a);
        m_simple_actions ["armyset.file.save-as"]->set_enabled (a);
        m_simple_actions ["armyset.file.save"]->set_enabled
          (m_umgr->undo_empty () == false);

        m_simple_actions["armyset.add-army"]->set_enabled (a);

        m_simple_actions["armyset.remove-army"]->set_enabled
          (a && m_store->get_n_items () > 0);

        m_simple_actions["armyset.army-up"]->set_enabled
          (a && is_top_row_selected () == false);

        m_simple_actions["armyset.army-down"]->set_enabled
          (a && is_bottom_row_selected () == false);
      }

    void update_armyset_panel ()
      {
        bool sensitive;
        disconnect_signals ();
        if (m_armyset)
          {
            sensitive = true;
            Glib::ustring s = "";
            if (m_armyset->getShip ()->getName ().empty () == false)
              s = m_armyset->getShip ()->getName ();
            m_ship_filelabel->set_label (s);

            s = "";
            if (m_armyset->getStandard ()->getName ().empty () == false)
              s = m_armyset->getStandard ()->getName ();
            m_hero_flag_filelabel->set_label (s);

            s = "";
            if (m_armyset->getBag ()->getName ().empty () == false)
              s = m_armyset->getBag ()->getName ();
            m_bag_filelabel->set_label (s);

            s = "";
              {
                auto im = m_armyset->getSelector (false, Shield::WHITE);
                if (im->getName ().empty () == false)
                  s = im->getName ();
              }
            m_white_small_select_filelabel->set_label (s);

            s = "";
              {
                auto im = m_armyset->getSelector (true, Shield::WHITE);
                if (im->getName ().empty () == false)
                  s = im->getName ();
              }
            m_white_large_select_filelabel->set_label (s);

            s = "";
              {
                auto im = m_armyset->getSelector (false, Shield::GREEN);
                if (im->getName ().empty () == false)
                  s = im->getName ();
              }
            m_green_small_select_filelabel->set_label (s);

            s = "";
              {
                auto im = m_armyset->getSelector (true, Shield::GREEN);
                if (im->getName ().empty () == false)
                  s = im->getName ();
              }
            m_green_large_select_filelabel->set_label (s);

            s = "";
              {
                auto im = m_armyset->getSelector (false, Shield::YELLOW);
                if (im->getName ().empty () == false)
                  s = im->getName ();
              }
            m_yellow_small_select_filelabel->set_label (s);

            s = "";
              {
                auto im = m_armyset->getSelector (true, Shield::YELLOW);
                if (im->getName ().empty () == false)
                  s = im->getName ();
              }
            m_yellow_large_select_filelabel->set_label (s);

            s = "";
              {
                auto im = m_armyset->getSelector (false, Shield::DARK_BLUE);
                if (im->getName ().empty () == false)
                  s = im->getName ();
              }
            m_dark_blue_small_select_filelabel->set_label (s);

            s = "";
              {
                auto im = m_armyset->getSelector (true, Shield::DARK_BLUE);
                if (im->getName ().empty () == false)
                  s = im->getName ();
              }
            m_dark_blue_large_select_filelabel->set_label (s);

            s = "";
              {
                auto im = m_armyset->getSelector (false, Shield::ORANGE);
                if (im->getName ().empty () == false)
                  s = im->getName ();
              }
            m_orange_small_select_filelabel->set_label (s);

            s = "";
              {
                auto im = m_armyset->getSelector (true, Shield::ORANGE);
                if (im->getName ().empty () == false)
                  s = im->getName ();
              }
            m_orange_large_select_filelabel->set_label (s);

            s = "";
              {
                auto im = m_armyset->getSelector (false, Shield::LIGHT_BLUE);
                if (im->getName ().empty () == false)
                  s = im->getName ();
              }
            m_light_blue_small_select_filelabel->set_label (s);

            s = "";
              {
                auto im = m_armyset->getSelector (true, Shield::LIGHT_BLUE);
                if (im->getName ().empty () == false)
                  s = im->getName ();
              }
            m_light_blue_large_select_filelabel->set_label (s);

            s = "";
              {
                auto im = m_armyset->getSelector (false, Shield::RED);
                if (im->getName ().empty () == false)
                  s = im->getName ();
              }
            m_red_small_select_filelabel->set_label (s);

            s = "";
              {
                auto im = m_armyset->getSelector (true, Shield::RED);
                if (im->getName ().empty () == false)
                  s = im->getName ();
              }
            m_red_large_select_filelabel->set_label (s);

            s = "";
              {
                auto im = m_armyset->getSelector (false, Shield::BLACK);
                if (im->getName ().empty () == false)
                  s = im->getName ();
              }
            m_black_small_select_filelabel->set_label (s);

            s = "";
              {
                auto im = m_armyset->getSelector (true, Shield::BLACK);
                if (im->getName ().empty () == false)
                  s = im->getName ();
              }
            m_black_large_select_filelabel->set_label (s);

            auto a = get_selected_army ();
            guint32 mbonus = 0;
            guint32 abonus = 0;
            if (a)
              {
                mbonus = a->getMoveBonus ();
                abonus = a->getArmyBonus ();
              }

            if (a)
              m_name_entry->set_text (a->getName ());

            s = "";
            if (a)
              {
                auto im = a->getMaskedImage (Shield::WHITE);
                if (im->getName ().empty () == false)
                  s = im->getName ();
              }
            m_white_image_filelabel->set_label (s);
 
            m_make_same_switch->set_active (m_make_same);

            s = "";
            if (a)
              {
                auto im = a->getMaskedImage (Shield::GREEN);
                if (im->getName ().empty () == false)
                  s = im->getName ();
              }
            m_green_image_filelabel->set_label (s);

            s = "";
            if (a)
              {
                auto im = a->getMaskedImage (Shield::YELLOW);
                if (im->getName ().empty () == false)
                  s = im->getName ();
              }
            m_yellow_image_filelabel->set_label (s);

            s = "";
            if (a)
              {
                auto im = a->getMaskedImage (Shield::LIGHT_BLUE);
                if (im->getName ().empty () == false)
                  s = im->getName ();
              }
            m_light_blue_image_filelabel->set_label (s);

            s = "";
            if (a)
              {
                auto im = a->getMaskedImage (Shield::RED);
                if (im->getName ().empty () == false)
                  s = im->getName ();
              }
            m_red_image_filelabel->set_label (s);

            s = "";
            if (a)
              {
                auto im = a->getMaskedImage (Shield::DARK_BLUE);
                if (im->getName ().empty () == false)
                  s = im->getName ();
              }
            m_dark_blue_image_filelabel->set_label (s);

            s = "";
            if (a)
              {
                auto im = a->getMaskedImage (Shield::ORANGE);
                if (im->getName ().empty () == false)
                  s = im->getName ();
              }
            m_orange_image_filelabel->set_label (s);

            s = "";
            if (a)
              {
                auto im = a->getMaskedImage (Shield::BLACK);
                if (im->getName ().empty () == false)
                  s = im->getName ();
              }
            m_black_image_filelabel->set_label (s);

            s = "";
            if (a)
              {
                auto im = a->getMaskedImage (Shield::NEUTRAL);
                if (im->getName ().empty () == false)
                  s = im->getName ();
              }
            m_neutral_image_filelabel->set_label (s);

            if (a)
              {
                m_production_spinbutton->set_value (a->getProduction ());
                m_cost_spinbutton->set_value (a->getProductionCost ());
                m_new_cost_spinbutton->set_value (a->getNewProductionCost ());
                m_upkeep_spinbutton->set_value (a->getUpkeep ());
                m_strength_spinbutton->set_value (a->getStrength ());
                m_moves_spinbutton->set_value (a->getMaxMoves ());
                m_exp_spinbutton->set_value (a->getXpReward ());
                m_id_spinbutton->set_value (a->getId ());
                m_hero_combobox->set_active (a->getGender ());
                m_awardable_switch->set_active (a->getAwardable ());
                m_defends_ruins_switch->set_active (a->getDefendsRuins ());
                m_sight_spinbutton->set_value (a->getSight ());
                m_move_forests_switch->set_active
                  ((mbonus & Tile::FOREST) == Tile::FOREST);
                m_move_marshes_switch->set_active
                  ((mbonus & Tile::SWAMP) == Tile::SWAMP);
                m_move_hills_switch->set_active
                  ((mbonus & Tile::HILLS) == Tile::HILLS);
                m_move_mountains_switch->set_active
                  ((mbonus & Tile::MOUNTAIN) == Tile::MOUNTAIN);
                m_can_fly_switch->set_active
                  (mbonus == (Tile::GRASS | Tile::WATER |
                              Tile::FOREST | Tile::HILLS |
                              Tile::MOUNTAIN | Tile::SWAMP));
                m_add1strinopen_switch->set_active
                  ((abonus & Army::ADD1STRINOPEN) == Army::ADD1STRINOPEN);
                m_add2strinopen_switch->set_active
                  ((abonus & Army::ADD2STRINOPEN) == Army::ADD2STRINOPEN);
                m_add1strinforest_switch->set_active
                  ((abonus & Army::ADD1STRINFOREST) == Army::ADD1STRINFOREST);
                m_add2strinforest_switch->set_active
                  ((abonus & Army::ADD2STRINFOREST) == Army::ADD2STRINFOREST);
                m_add1strinhills_switch->set_active
                  ((abonus & Army::ADD1STRINHILLS) == Army::ADD1STRINHILLS);
                m_add2strinhills_switch->set_active
                  ((abonus & Army::ADD2STRINHILLS) == Army::ADD2STRINHILLS);
                m_add1strincity_switch->set_active
                  ((abonus & Army::ADD1STRINCITY) == Army::ADD1STRINCITY);
                m_add2strincity_switch->set_active
                  ((abonus & Army::ADD2STRINCITY) == Army::ADD2STRINCITY);
                m_add1stackinhills_switch->set_active
                  ((abonus & Army::ADD1STACKINHILLS) == Army::ADD1STACKINHILLS);
                m_suballcitybonus_switch->set_active
                  ((abonus & Army::SUBALLCITYBONUS) == Army::SUBALLCITYBONUS);
                m_sub1enemystack_switch->set_active
                  ((abonus & Army::SUB1ENEMYSTACK) == Army::SUB1ENEMYSTACK);
                m_sub2enemystack_switch->set_active
                  ((abonus & Army::SUB2ENEMYSTACK) == Army::SUB2ENEMYSTACK);
                m_add1stack_switch->set_active
                  ((abonus & Army::ADD1STACK) == Army::ADD1STACK);
                m_add2stack_switch->set_active
                  ((abonus & Army::ADD2STACK) == Army::ADD2STACK);
                m_suballnonherobonus_switch->set_active
                  ((abonus & Army::SUBALLNONHEROBONUS) == Army::SUBALLNONHEROBONUS);
                m_suballherobonus_switch->set_active
                  ((abonus & Army::SUBALLHEROBONUS) == Army::SUBALLHEROBONUS);
                m_confer_move_bonus_switch->set_active
                  ((abonus & Army::CONFER_MOVE_BONUS) == Army::CONFER_MOVE_BONUS);
              }
            else
              clear_army_panel ();
          }
        else
          {
            sensitive = false;
            m_ship_filelabel->set_label ("");
            m_hero_flag_filelabel->set_label ("");
            m_bag_filelabel->set_label ("");
            m_white_small_select_filelabel->set_label ("");
            m_white_large_select_filelabel->set_label ("");
            m_green_small_select_filelabel->set_label ("");
            m_green_large_select_filelabel->set_label ("");
            m_yellow_small_select_filelabel->set_label ("");
            m_yellow_large_select_filelabel->set_label ("");
            m_dark_blue_small_select_filelabel->set_label ("");
            m_dark_blue_large_select_filelabel->set_label ("");
            m_orange_small_select_filelabel->set_label ("");
            m_orange_large_select_filelabel->set_label ("");
            m_light_blue_small_select_filelabel->set_label ("");
            m_light_blue_large_select_filelabel->set_label ("");
            m_red_small_select_filelabel->set_label ("");
            m_red_large_select_filelabel->set_label ("");
            m_black_small_select_filelabel->set_label ("");
            m_black_large_select_filelabel->set_label ("");
            clear_army_panel ();
          }

        m_ship_button->set_sensitive (sensitive);
        m_hero_flag_button->set_sensitive (sensitive);
        m_bag_button->set_sensitive (sensitive);
        m_white_small_select_button->set_sensitive (sensitive);
        m_white_large_select_button->set_sensitive (sensitive);
        m_green_small_select_button->set_sensitive (sensitive);
        m_green_large_select_button->set_sensitive (sensitive);
        m_yellow_small_select_button->set_sensitive (sensitive);
        m_yellow_large_select_button->set_sensitive (sensitive);
        m_dark_blue_small_select_button->set_sensitive (sensitive);
        m_dark_blue_large_select_button->set_sensitive (sensitive);
        m_orange_small_select_button->set_sensitive (sensitive);
        m_orange_large_select_button->set_sensitive (sensitive);
        m_light_blue_small_select_button->set_sensitive (sensitive);
        m_light_blue_large_select_button->set_sensitive (sensitive);
        m_red_small_select_button->set_sensitive (sensitive);
        m_red_large_select_button->set_sensitive (sensitive);
        m_black_small_select_button->set_sensitive (sensitive);
        m_black_large_select_button->set_sensitive (sensitive);
        m_name_entry->set_sensitive (sensitive);
        m_white_image_button->set_sensitive (sensitive);
        bool white_is_filled = false;
          {
            auto a = get_selected_army ();
            if (a)
              white_is_filled =
                a->getMaskedImage (Shield::WHITE)->getName ().empty () == false;
          }
        m_make_same_switch->set_sensitive (sensitive && white_is_filled);
        bool make_same = m_make_same_switch->get_active ();
        m_green_image_button->set_sensitive (sensitive && !make_same);
        m_yellow_image_button->set_sensitive (sensitive && !make_same);
        m_light_blue_image_button->set_sensitive (sensitive && !make_same);
        m_red_image_button->set_sensitive (sensitive && !make_same);
        m_dark_blue_image_button->set_sensitive (sensitive && !make_same);
        m_orange_image_button->set_sensitive (sensitive && !make_same);
        m_black_image_button->set_sensitive (sensitive && !make_same);
        m_neutral_image_button->set_sensitive (sensitive && !make_same);
        m_production_spinbutton->set_sensitive (sensitive);
        m_cost_spinbutton->set_sensitive (sensitive);
        m_new_cost_spinbutton->set_sensitive (sensitive);
        m_upkeep_spinbutton->set_sensitive (sensitive);
        m_strength_spinbutton->set_sensitive (sensitive);
        m_moves_spinbutton->set_sensitive (sensitive);
        m_exp_spinbutton->set_sensitive (sensitive);
        m_id_spinbutton->set_sensitive (sensitive);
        m_hero_combobox->set_sensitive (sensitive);
        m_awardable_switch->set_sensitive (sensitive);
        m_defends_ruins_switch->set_sensitive (sensitive);
        m_sight_spinbutton->set_sensitive (sensitive);
        m_move_forests_switch->set_sensitive (sensitive);
        m_move_marshes_switch->set_sensitive (sensitive);
        m_move_hills_switch->set_sensitive (sensitive);
        m_move_mountains_switch->set_sensitive (sensitive);
        m_can_fly_switch->set_sensitive (sensitive);
        m_add1strinopen_switch->set_sensitive (sensitive);
        m_add2strinopen_switch->set_sensitive (sensitive);
        m_add1strinforest_switch->set_sensitive (sensitive);
        m_add2strinforest_switch->set_sensitive (sensitive);
        m_add1strinhills_switch->set_sensitive (sensitive);
        m_add2strinhills_switch->set_sensitive (sensitive);
        m_add1strincity_switch->set_sensitive (sensitive);
        m_add2strincity_switch->set_sensitive (sensitive);
        m_add1stackinhills_switch->set_sensitive (sensitive);
        m_suballcitybonus_switch->set_sensitive (sensitive);
        m_sub1enemystack_switch->set_sensitive (sensitive);
        m_sub2enemystack_switch->set_sensitive (sensitive);
        m_add1stack_switch->set_sensitive (sensitive);
        m_add2stack_switch->set_sensitive (sensitive);
        m_suballnonherobonus_switch->set_sensitive (sensitive);
        m_suballherobonus_switch->set_sensitive (sensitive);
        m_confer_move_bonus_switch->set_sensitive (sensitive);
        m_switcher->set_sensitive (sensitive);
        m_treeview->set_sensitive (sensitive);

        connect_signals ();
      }

    void update_window_title ()
      {
        Glib::ustring title = "";
        if (m_armyset_modified || m_new_armyset_needs_saving)
          title += "*";
        if (m_armyset)
          {
            m_notebook->set_current_page (1);
            title += m_armyset->getName ();
            m_subtitle->set_text (title);
          }
        else
          {
            m_notebook->set_current_page (0);
            m_subtitle->set_text ("");
          }
      }

    Gtk::ScrolledWindow* populate_misc_images ()
      {
        auto scrolled_window = Gtk::make_managed<Gtk::ScrolledWindow> ();
        scrolled_window->set_policy
          (Gtk::PolicyType::NEVER, Gtk::PolicyType::AUTOMATIC);
        auto box = Gtk::make_managed<Gtk::Box> (Gtk::Orientation::VERTICAL);
        box->set_margin (6);

        auto ship_image = create_attribute_row
          (_("Ship"),
           _("What the stack looks like when it's in a boat"),
           "setting_top");
        m_ship_button = Gtk::make_managed<Gtk::Button> ();
        m_ship_filelabel = Gtk::make_managed<FileLabel> (m_ship_button);
        ship_image->append (*m_ship_button);
        box->append (*ship_image);

        auto hero_flag_image = create_attribute_row
          (_("Hero Flag"),
           _("What the hero's standard looks like when it's planted"),
           "setting_middle");
        m_hero_flag_button = Gtk::make_managed<Gtk::Button> ();
        m_hero_flag_filelabel =
          Gtk::make_managed<FileLabel> (m_hero_flag_button);
        hero_flag_image->append (*m_hero_flag_button);
        box->append (*hero_flag_image);

        auto bag_image = create_attribute_row
          (_("Bag of items"),
           _("What a bag of items looks like on the ground"),
           "setting_bottom");
        m_bag_button = Gtk::make_managed<Gtk::Button> ();
        m_bag_filelabel = Gtk::make_managed<FileLabel> (m_bag_button);
        bag_image->append (*m_bag_button);
        box->append (*bag_image);

        scrolled_window->set_child (*box);
        return scrolled_window;
      }

    Gtk::ScrolledWindow* populate_selector_images ()
      {
        auto scrolled_window = Gtk::make_managed<Gtk::ScrolledWindow> ();
        scrolled_window->set_policy
          (Gtk::PolicyType::NEVER, Gtk::PolicyType::AUTOMATIC);
        auto box = Gtk::make_managed<Gtk::Box> (Gtk::Orientation::VERTICAL);
        box->set_margin (6);

        auto subheading_label =
          Gtk::make_managed<Gtk::Label> ("These override the tileset selector");
        subheading_label->add_css_class ("setting_explain_label");
        box->append (*subheading_label);

        auto white_small_select_image = create_attribute_row
          (_("White Small Selector"),
           _("White active stacks of one unit have this animation"),
           "setting_top");
        m_white_small_select_button = Gtk::make_managed<Gtk::Button> ();
        m_white_small_select_filelabel =
          Gtk::make_managed<FileLabel> (m_white_small_select_button);
        white_small_select_image->append (*m_white_small_select_button);
        box->append (*white_small_select_image);

        auto white_large_select_image = create_attribute_row
          (_("White Large Selector"),
           _("White active stacks of two or more units have this animation"),
           "setting_middle");
        m_white_large_select_button = Gtk::make_managed<Gtk::Button> ();
        m_white_large_select_filelabel =
          Gtk::make_managed<FileLabel> (m_white_large_select_button);
        white_large_select_image->append (*m_white_large_select_button);
        box->append (*white_large_select_image);

        auto green_small_select_image = create_attribute_row
          (_("Green Small Selector"),
           _("Green active stacks of one unit have this animation"),
           "setting_middle");
        m_green_small_select_button = Gtk::make_managed<Gtk::Button> ();
        m_green_small_select_filelabel =
          Gtk::make_managed<FileLabel> (m_green_small_select_button);
        green_small_select_image->append (*m_green_small_select_button);
        box->append (*green_small_select_image);

        auto green_large_select_image = create_attribute_row
          (_("Green Large Selector"),
           _("Green active stacks of two or more units have this animation"),
           "setting_middle");
        m_green_large_select_button = Gtk::make_managed<Gtk::Button> ();
        m_green_large_select_filelabel =
          Gtk::make_managed<FileLabel> (m_green_large_select_button);
        green_large_select_image->append (*m_green_large_select_button);
        box->append (*green_large_select_image);

        auto yellow_small_select_image = create_attribute_row
          (_("Yellow Small Selector"),
           _("Yellow active stacks of one unit have this animation"),
           "setting_middle");
        m_yellow_small_select_button = Gtk::make_managed<Gtk::Button> ();
        m_yellow_small_select_filelabel =
          Gtk::make_managed<FileLabel> (m_yellow_small_select_button);
        yellow_small_select_image->append (*m_yellow_small_select_button);
        box->append (*yellow_small_select_image);

        auto yellow_large_select_image = create_attribute_row
          (_("Yellow Large Selector"),
           _("Yellow active stacks of two or more units have this animation"),
           "setting_middle");
        m_yellow_large_select_button = Gtk::make_managed<Gtk::Button> ();
        m_yellow_large_select_filelabel =
          Gtk::make_managed<FileLabel> (m_yellow_large_select_button);
        yellow_large_select_image->append (*m_yellow_large_select_button);
        box->append (*yellow_large_select_image);

        auto dark_blue_small_select_image = create_attribute_row
          (_("Dark Blue Small Selector"),
           _("Dark Blue active stacks of one unit have this animation"),
           "setting_middle");
        m_dark_blue_small_select_button = Gtk::make_managed<Gtk::Button> ();
        m_dark_blue_small_select_filelabel =
          Gtk::make_managed<FileLabel> (m_dark_blue_small_select_button);
        dark_blue_small_select_image->append (*m_dark_blue_small_select_button);
        box->append (*dark_blue_small_select_image);

        auto dark_blue_large_select_image = create_attribute_row
          (_("Dark Blue Large Selector"),
           _("Dark Blue active stacks of two or more units have this animation"),
           "setting_middle");
        m_dark_blue_large_select_button = Gtk::make_managed<Gtk::Button> ();
        m_dark_blue_large_select_filelabel =
          Gtk::make_managed<FileLabel> (m_dark_blue_large_select_button);
        dark_blue_large_select_image->append (*m_dark_blue_large_select_button);
        box->append (*dark_blue_large_select_image);

        auto orange_small_select_image = create_attribute_row
          (_("Orange Small Selector"),
           _("Orange active stacks of one unit have this animation"),
           "setting_middle");
        m_orange_small_select_button = Gtk::make_managed<Gtk::Button> ();
        m_orange_small_select_filelabel =
          Gtk::make_managed<FileLabel> (m_orange_small_select_button);
        orange_small_select_image->append (*m_orange_small_select_button);
        box->append (*orange_small_select_image);

        auto orange_large_select_image = create_attribute_row
          (_("Orange Large Selector"),
           _("Orange active stacks of two or more units have this animation"),
           "setting_middle");
        m_orange_large_select_button = Gtk::make_managed<Gtk::Button> ();
        m_orange_large_select_filelabel =
          Gtk::make_managed<FileLabel> (m_orange_large_select_button);
        orange_large_select_image->append (*m_orange_large_select_button);
        box->append (*orange_large_select_image);

        auto light_blue_small_select_image = create_attribute_row
          (_("Light Blue Small Selector"),
           _("Light Blue active stacks of one unit have this animation"),
           "setting_middle");
        m_light_blue_small_select_button = Gtk::make_managed<Gtk::Button> ();
        m_light_blue_small_select_filelabel =
          Gtk::make_managed<FileLabel> (m_light_blue_small_select_button);
        light_blue_small_select_image->append
          (*m_light_blue_small_select_button);
        box->append (*light_blue_small_select_image);

        auto light_blue_large_select_image = create_attribute_row
          (_("Light Blue Large Selector"),
           _("Light Blue active stacks of two or more units have this "
             "animation"),
           "setting_middle");
        m_light_blue_large_select_button = Gtk::make_managed<Gtk::Button> ();
        m_light_blue_large_select_filelabel =
          Gtk::make_managed<FileLabel> (m_light_blue_large_select_button);
        light_blue_large_select_image->append
          (*m_light_blue_large_select_button);
        box->append (*light_blue_large_select_image);

        auto red_small_select_image = create_attribute_row
          (_("Red Small Selector"),
           _("Red active stacks of one unit have this animation"),
           "setting_middle");
        m_red_small_select_button = Gtk::make_managed<Gtk::Button> ();
        m_red_small_select_filelabel =
          Gtk::make_managed<FileLabel> (m_red_small_select_button);
        red_small_select_image->append (*m_red_small_select_button);
        box->append (*red_small_select_image);

        auto red_large_select_image = create_attribute_row
          (_("Red Large Selector"),
           _("Red active stacks of two or more units have this animation"),
           "setting_middle");
        m_red_large_select_button = Gtk::make_managed<Gtk::Button> ();
        m_red_large_select_filelabel =
          Gtk::make_managed<FileLabel> (m_red_large_select_button);
        red_large_select_image->append (*m_red_large_select_button);
        box->append (*red_large_select_image);

        auto black_small_select_image = create_attribute_row
          (_("Black Small Selector"),
           _("Black active stacks of one unit have this animation"),
           "setting_middle");
        m_black_small_select_button = Gtk::make_managed<Gtk::Button> ();
        m_black_small_select_filelabel =
          Gtk::make_managed<FileLabel> (m_black_small_select_button);
        black_small_select_image->append (*m_black_small_select_button);
        box->append (*black_small_select_image);

        auto black_large_select_image = create_attribute_row
          (_("Black Large Selector"),
           _("Black active stacks of two or more units have this animation"),
           "setting_middle");
        m_black_large_select_button = Gtk::make_managed<Gtk::Button> ();
        m_black_large_select_filelabel =
          Gtk::make_managed<FileLabel> (m_black_large_select_button);
        black_large_select_image->append (*m_black_large_select_button);
        box->append (*black_large_select_image);

        scrolled_window->set_child (*box);
        return scrolled_window;
      }

    Gtk::ScrolledWindow* populate_army_panel ()
      {
        m_scrolled_window = Gtk::make_managed<Gtk::ScrolledWindow> ();
        auto box = Gtk::make_managed<Gtk::Box> (Gtk::Orientation::VERTICAL);
        box->set_margin (6);
        m_scrolled_window->set_child (*box);

        auto name = create_attribute_row
          (_("Name"),
           _("The name of the army unit"),
           "setting_top");
        m_name_entry = Gtk::make_managed<Gtk::Entry> ();
        m_name_entry->set_placeholder_text (_("Name..."));
        name->append (*m_name_entry);
        box->append (*name);

        auto id = create_attribute_row
          (_("Identifier"),
           _("The unique identifier for this army unit"),
           "setting_bottom");
        m_id_spinbutton = Gtk::make_managed<Gtk::SpinButton> ();
        m_id_spinbutton->set_adjustment
          (Gtk::Adjustment::create (0, 0, 1000, 1, 10, 0));
        id->append (*m_id_spinbutton);
        box->append (*id);

        auto white_image = create_attribute_row
          (_("White Image"),
           _("What the army unit looks like as the white player"),
           "setting_top");
        m_white_image_button = Gtk::make_managed<Gtk::Button> ();
        m_white_image_filelabel =
          Gtk::make_managed<FileLabel> (m_white_image_button);
        white_image->append (*m_white_image_button);
        box->append (*white_image);

        auto make_same = create_attribute_row
          (_("Make Same As White"),
           _("Makes the rest of the images the same as the white one"),
           "setting_middle");

        m_make_same_switch = Gtk::make_managed<Gtk::Switch> ();
        m_make_same_switch->set_valign (Gtk::Align::CENTER);
        make_same->append (*m_make_same_switch);
        box->append (*make_same);

        auto green_image = create_attribute_row
          (_("Green Image"),
           _("What the army unit looks like as the green player"),
           "setting_middle");
        m_green_image_button = Gtk::make_managed<Gtk::Button> ();
        m_green_image_filelabel =
          Gtk::make_managed<FileLabel> (m_green_image_button);
        green_image->append (*m_green_image_button);
        box->append (*green_image);

        auto yellow_image = create_attribute_row
          (_("Yellow"),
           _("What the army unit looks like as the yellow player"),
           "setting_middle");
        m_yellow_image_button = Gtk::make_managed<Gtk::Button> ();
        m_yellow_image_filelabel =
          Gtk::make_managed<FileLabel> (m_yellow_image_button);
        yellow_image->append (*m_yellow_image_button);
        box->append (*yellow_image);

        auto dark_blue_image = create_attribute_row
          (_("Dark Blue Image"),
           _("What the army unit looks like as the dark blue player"),
           "setting_middle");
        m_dark_blue_image_button = Gtk::make_managed<Gtk::Button> ();
        m_dark_blue_image_filelabel =
          Gtk::make_managed<FileLabel> (m_dark_blue_image_button);
        dark_blue_image->append (*m_dark_blue_image_button);
        box->append (*dark_blue_image);

        auto orange_image = create_attribute_row
          (_("Orange Image"),
           _("What the army unit looks like as the orange player"),
           "setting_middle");
        m_orange_image_button = Gtk::make_managed<Gtk::Button> ();
        m_orange_image_filelabel =
          Gtk::make_managed<FileLabel> (m_orange_image_button);
        orange_image->append (*m_orange_image_button);
        box->append (*orange_image);

        auto light_blue_image = create_attribute_row
          (_("Light Blue Image"),
           _("What the army unit looks like as the light blue player"),
           "setting_middle");
        m_light_blue_image_button = Gtk::make_managed<Gtk::Button> ();
        m_light_blue_image_filelabel =
          Gtk::make_managed<FileLabel> (m_light_blue_image_button);
        light_blue_image->append (*m_light_blue_image_button);
        box->append (*light_blue_image);

        auto red_image = create_attribute_row
          (_("Red Image"),
           _("What the army unit looks like as the red player"),
           "setting_middle");
        m_red_image_button = Gtk::make_managed<Gtk::Button> ();
        m_red_image_filelabel =
          Gtk::make_managed<FileLabel> (m_red_image_button);
        red_image->append (*m_red_image_button);
        box->append (*red_image);

        auto black_image = create_attribute_row
          (_("Black Image"),
           _("What the army unit looks like as the black player"),
           "setting_middle");
        m_black_image_button = Gtk::make_managed<Gtk::Button> ();
        m_black_image_filelabel =
          Gtk::make_managed<FileLabel> (m_black_image_button);
        black_image->append (*m_black_image_button);
        box->append (*black_image);

        auto neutral_image = create_attribute_row
          (_("Neutral Image"),
           _("What the army unit looks like as the neutral player"),
           "setting_bottom");
        m_neutral_image_button = Gtk::make_managed<Gtk::Button> ();
        m_neutral_image_filelabel =
          Gtk::make_managed<FileLabel> (m_neutral_image_button);
        neutral_image->append (*m_neutral_image_button);
        box->append (*neutral_image);

        auto production = create_attribute_row
          (_("Turns"),
           _("How long it takes to produce a new instance this unit"),
           "setting_top");
        m_production_spinbutton = Gtk::make_managed<Gtk::SpinButton> ();

        int min = ArmyProto::min_production_turns;
        int max = ArmyProto::max_production_turns;
        m_production_spinbutton->set_adjustment
          (Gtk::Adjustment::create (min, min, max, 1, 10, 0));
        production->append (*m_production_spinbutton);
        box->append (*production);

        auto cost = create_attribute_row
          (_("Cost"),
           _("How many gold pieces to produce one of these army units"),
           "setting_middle");
        m_cost_spinbutton = Gtk::make_managed<Gtk::SpinButton> ();
        min = ArmyProto::min_cost;
        max = ArmyProto::max_cost;
        m_cost_spinbutton->set_adjustment
          (Gtk::Adjustment::create (min, min, max, 1, 10, 0));
        cost->append (*m_cost_spinbutton);
        box->append (*cost);

        auto new_cost = create_attribute_row
          (_("New Cost"),
           _("How many gp to add this army unit to a city's suite of four"),
           "setting_middle");
        m_new_cost_spinbutton = Gtk::make_managed<Gtk::SpinButton> ();
        min = ArmyProto::min_new_cost;
        max = ArmyProto::max_new_cost;
        m_new_cost_spinbutton->set_adjustment
          (Gtk::Adjustment::create (min, min, max, 1, 10, 0));
        new_cost->append (*m_new_cost_spinbutton);
        box->append (*new_cost);

        auto upkeep = create_attribute_row
          (_("Upkeep"),
           _("How many gold pieces it costs to keep this army unit per turn"),
           "setting_middle");
        m_upkeep_spinbutton = Gtk::make_managed<Gtk::SpinButton> ();
        min = ArmyProto::min_upkeep;
        max = ArmyProto::max_upkeep;
        m_upkeep_spinbutton->set_adjustment
          (Gtk::Adjustment::create (min, min, max, 1, 10, 0));
        upkeep->append (*m_upkeep_spinbutton);
        box->append (*upkeep);

        auto strength = create_attribute_row
          (_("Strength"),
           _("How strong the army unit is"),
           "setting_middle");
        min = ArmyProto::min_strength;
        max = ArmyProto::max_strength;
        m_strength_spinbutton = Gtk::make_managed<Gtk::SpinButton> ();
        m_strength_spinbutton->set_adjustment
          (Gtk::Adjustment::create (min, min, max, 1, 10, 0));
        strength->append (*m_strength_spinbutton);
        box->append (*strength);

        auto moves = create_attribute_row
          (_("Max Moves"),
           _("The maximum number of movement points for the army unit"),
           "setting_middle");
        m_moves_spinbutton = Gtk::make_managed<Gtk::SpinButton> ();
        min = ArmyProto::min_moves;
        max = ArmyProto::max_moves;
        m_moves_spinbutton->set_adjustment
          (Gtk::Adjustment::create (min, min, max, 1, 10, 0));
        moves->append (*m_moves_spinbutton);
        box->append (*moves);

        auto exp = create_attribute_row
          (_("Exp Points"),
           _("How many experience points are gained by killing this army unit"),
           "setting_middle");
        m_exp_spinbutton = Gtk::make_managed<Gtk::SpinButton> ();
        min = ArmyProto::min_exp;
        max = ArmyProto::max_exp;
        m_exp_spinbutton->set_adjustment
          (Gtk::Adjustment::create (min, min, max, 1, 10, 0));
        exp->append (*m_exp_spinbutton);
        box->append (*exp);

        auto hero = create_attribute_row
          (_("Hero"),
           _("Whether or not this army unit a hero, and if it is what gender"),
           "setting_middle");
        m_hero_combobox = Gtk::make_managed<LwCombo> ();
        m_hero_combobox->append (_("Not A Hero"));
        m_hero_combobox->append (_("Male Hero"));
        m_hero_combobox->append (_("Female Hero"));
        m_hero_combobox->set_hexpand (false);
        hero->append (*m_hero_combobox);
        box->append (*hero);

        auto awardable = create_attribute_row
          (_("Ruin Award"),
           _("This army unit can be awarded to a hero at a ruin"),
           "setting_middle");
        m_awardable_switch = Gtk::make_managed<Gtk::Switch> ();
        m_awardable_switch->set_valign (Gtk::Align::CENTER);
        awardable->append (*m_awardable_switch);
        box->append (*awardable);

        auto defends_ruins = create_attribute_row
          (_("Defends Ruins"),
           _("This army unit can fight in a ruin against a hero"),
           "setting_middle");
        m_defends_ruins_switch = Gtk::make_managed<Gtk::Switch> ();
        m_defends_ruins_switch->set_valign (Gtk::Align::CENTER);
        defends_ruins->append (*m_defends_ruins_switch);
        box->append (*defends_ruins);

        auto sight = create_attribute_row
          (_("Sight"),
           _("How many tiles this army unit clears on a fogged map (radius)"),
           "setting_bottom");
        m_sight_spinbutton = Gtk::make_managed<Gtk::SpinButton> ();
        min = ArmyProto::min_sight;
        max = ArmyProto::max_sight;
        m_sight_spinbutton->set_adjustment
          (Gtk::Adjustment::create (min, min, max, 1, 10, 0));
        sight->append (*m_sight_spinbutton);
        box->append (*sight);

        auto move_forests = create_attribute_row
          (_("Travels Faster In Forests"),
           _(""),
           "setting_top");
        m_move_forests_switch = Gtk::make_managed<Gtk::Switch> ();
        move_forests->append (*m_move_forests_switch);
        box->append (*move_forests);

        auto move_marshes = create_attribute_row
          (_("Travels Faster In Marshland"),
           _(""),
           "setting_middle");
        m_move_marshes_switch = Gtk::make_managed<Gtk::Switch> ();
        move_marshes->append (*m_move_marshes_switch);
        box->append (*move_marshes);

        auto move_hills = create_attribute_row
          (_("Travels Faster In Hills"),
           _(""),
           "setting_middle");
        m_move_hills_switch = Gtk::make_managed<Gtk::Switch> ();
        move_hills->append (*m_move_hills_switch);
        box->append (*move_hills);

        auto move_mountains = create_attribute_row
          (_("Travels Faster In Mountains"),
           _(""),
           "setting_middle");
        m_move_mountains_switch = Gtk::make_managed<Gtk::Switch> ();
        move_mountains->append (*m_move_mountains_switch);
        box->append (*move_mountains);

        auto can_fly = create_attribute_row
          (_("Can Fly"),
           _(""),
           "setting_middle");
        m_can_fly_switch = Gtk::make_managed<Gtk::Switch> ();
        can_fly->append (*m_can_fly_switch);
        box->append (*can_fly);

        auto confer_move_bonus = create_attribute_row
          (_("Confers Movement Bonus To Stack "),
           _(""),
           "setting_bottom");
        m_confer_move_bonus_switch = Gtk::make_managed<Gtk::Switch> ();
        confer_move_bonus->append (*m_confer_move_bonus_switch);
        box->append (*confer_move_bonus);

        auto add1strinopen = create_attribute_row
          (_("+1 Strength In Open"),
           _(""),
           "setting_top");
        m_add1strinopen_switch = Gtk::make_managed<Gtk::Switch> ();
        add1strinopen->append (*m_add1strinopen_switch);
        box->append (*add1strinopen);

        auto add2strinopen = create_attribute_row
          (_("+2 Strength In Open"),
           _(""),
           "setting_middle");
        m_add2strinopen_switch = Gtk::make_managed<Gtk::Switch> ();
        add2strinopen->append (*m_add2strinopen_switch);
        box->append (*add2strinopen);

        auto add1strinforest = create_attribute_row
          (_("+1 Strength In Forest"),
           _(""),
           "setting_middle");
        m_add1strinforest_switch = Gtk::make_managed<Gtk::Switch> ();
        add1strinforest->append (*m_add1strinforest_switch);
        box->append (*add1strinforest);

        auto add2strinforest = create_attribute_row
          (_("+2 Strength In Forest"),
           _(""),
           "setting_middle");
        m_add2strinforest_switch = Gtk::make_managed<Gtk::Switch> ();
        add2strinforest->append (*m_add2strinforest_switch);
        box->append (*add2strinforest);

        auto add1strinhills = create_attribute_row
          (_("+1 Strength In Hills"),
           _(""),
           "setting_middle");
        m_add1strinhills_switch = Gtk::make_managed<Gtk::Switch> ();
        add1strinhills->append (*m_add1strinhills_switch);
        box->append (*add1strinhills);

        auto add2strinhills = create_attribute_row
          (_("+2 Strength In Hills"),
           _(""),
           "setting_middle");
        m_add2strinhills_switch = Gtk::make_managed<Gtk::Switch> ();
        add2strinhills->append (*m_add2strinhills_switch);
        box->append (*add2strinhills);

        auto add1strincity = create_attribute_row
          (_("+1 Strength In City"),
           _(""),
           "setting_middle");
        m_add1strincity_switch = Gtk::make_managed<Gtk::Switch> ();
        add1strincity->append (*m_add1strincity_switch);
        box->append (*add1strincity);

        auto add2strincity = create_attribute_row
          (_("+2 Strength In City"),
           _(""),
           "setting_middle");
        m_add2strincity_switch = Gtk::make_managed<Gtk::Switch> ();
        add2strincity->append (*m_add2strincity_switch);
        box->append (*add2strincity);

        auto add1stackinhills = create_attribute_row
          (_("+1 Strength To Stack In Hills"),
           _(""),
           "setting_middle");
        m_add1stackinhills_switch = Gtk::make_managed<Gtk::Switch> ();
        add1stackinhills->append (*m_add1stackinhills_switch);
        box->append (*add1stackinhills);

        auto suballcitybonus = create_attribute_row
          (_("Subtract All City Bonuses"),
           _(""),
           "setting_middle");
        m_suballcitybonus_switch = Gtk::make_managed<Gtk::Switch> ();
        suballcitybonus->append (*m_suballcitybonus_switch);
        box->append (*suballcitybonus);

        auto sub1enemystack = create_attribute_row
          (_("-1 Strength To Enemy Stack"),
           _(""),
           "setting_middle");
        m_sub1enemystack_switch = Gtk::make_managed<Gtk::Switch> ();
        sub1enemystack->append (*m_sub1enemystack_switch);
        box->append (*sub1enemystack);

        auto sub2enemystack = create_attribute_row
          (_("-2 Strength To Enemy Stack"),
           _(""),
           "setting_middle");
        m_sub2enemystack_switch = Gtk::make_managed<Gtk::Switch> ();
        sub2enemystack->append (*m_sub2enemystack_switch);
        box->append (*sub2enemystack);

        auto add1stack = create_attribute_row
          (_("+1 Strength to Stack"),
           _(""),
           "setting_middle");
        m_add1stack_switch = Gtk::make_managed<Gtk::Switch> ();
        add1stack->append (*m_add1stack_switch);
        box->append (*add1stack);

        auto add2stack = create_attribute_row
          (_("+2 Strength to Stack"),
           _(""),
           "setting_middle");
        m_add2stack_switch = Gtk::make_managed<Gtk::Switch> ();
        add2stack->append (*m_add2stack_switch);
        box->append (*add2stack);

        auto suballnonherobonus = create_attribute_row
          (_("Subtract All Non-Hero Bonuses"),
           _(""),
           "setting_middle");
        m_suballnonherobonus_switch = Gtk::make_managed<Gtk::Switch> ();
        suballnonherobonus->append (*m_suballnonherobonus_switch);
        box->append (*suballnonherobonus);

        auto suballherobonus = create_attribute_row
          (_("Subtract All Hero Bonuses"),
           _(""),
           "setting_bottom");
        m_suballherobonus_switch = Gtk::make_managed<Gtk::Switch> ();
        suballherobonus->append (*m_suballherobonus_switch);
        box->append (*suballherobonus);

        return m_scrolled_window;
      }

    void reselection (const Glib::RefPtr<ArmyProtoRow>& target)
      {
        guint n = m_store->get_n_items ();

        for (guint i = 0; i < n; i++)
          {
            if (m_store->get_item (i) == target)
              {
                m_selection_model->select_item (i, true);
                return;
              }
          }
      }

    Gtk::Box* populate_side_pane ()
      {
        auto pane = Gtk::make_managed<Gtk::Box> (Gtk::Orientation::VERTICAL);
        pane->set_margin_bottom (6);
        pane->set_margin_start (3);
        pane->set_spacing (6);
        m_columnview_scrolled_window = Gtk::make_managed<Gtk::ScrolledWindow> ();
        m_columnview_scrolled_window->set_vexpand (true);
        m_columnview_scrolled_window->set_policy
          (Gtk::PolicyType::NEVER, Gtk::PolicyType::AUTOMATIC);
        m_treeview = Gtk::make_managed<Gtk::ColumnView> ();
        m_columnview_scrolled_window->set_child (*m_treeview);
        pane->append (*m_columnview_scrolled_window);

        auto button_box =
          Gtk::make_managed<Gtk::Box> (Gtk::Orientation::HORIZONTAL);
        button_box->set_halign (Gtk::Align::CENTER);

        auto add_button = Gtk::make_managed<Gtk::Button> ();
        add_button->set_action_name ("lw.armyset.add-army");
        add_button->set_icon_name ("list-add-symbolic");
        button_box->append (*add_button);

        auto remove_button = Gtk::make_managed<Gtk::Button> ();
        remove_button->set_action_name ("lw.armyset.remove-army");
        remove_button->set_icon_name ("list-remove-symbolic");
        button_box->append (*remove_button);

        auto up_button = Gtk::make_managed<Gtk::Button> ();
        up_button->set_action_name ("lw.armyset.army-up");
        up_button->set_icon_name ("go-up-symbolic");
        button_box->append (*up_button);

        auto down_button = Gtk::make_managed<Gtk::Button> ();
        down_button->set_action_name ("lw.armyset.army-down");
        down_button->set_icon_name ("go-down-symbolic");
        button_box->append (*down_button);

        button_box->set_spacing (6);

        pane->append (*button_box);
        return pane;
      }

    Gtk::Box* populate_armies ()
      {
        auto panel = populate_army_panel ();

        auto hbox = Gtk::make_managed<Gtk::Box> (Gtk::Orientation::HORIZONTAL);
        auto pane = populate_side_pane ();
        hbox->append (*pane);
        hbox->set_vexpand (true);
        hbox->append (*panel);

        auto box = Gtk::make_managed<Gtk::Box> (Gtk::Orientation::VERTICAL);
        box->append (*hbox);

        return box;
      }

    void populate ()
      {
        m_switcher = Gtk::make_managed<Gtk::StackSwitcher> ();
        m_switcher->set_margin_top (6);

        auto stack = Gtk::make_managed<Gtk::Stack> ();
        stack->set_vexpand (true);
        stack->set_hexpand (true);

        auto armies = populate_armies ();
        stack->add (*armies, "armies", _("Armies"));

        auto selectors = populate_selector_images ();
        stack->add (*selectors, "selectors", _("Selectors"));

        auto misc_images = populate_misc_images ();
        stack->add (*misc_images, "misc. images", _("Misc. Images"));

        stack->property_visible_child_name ().signal_changed ().connect
          ([this] ()
           {
             update_actions ();
           });
        m_switcher->set_stack (*stack);

        auto root = Gtk::make_managed<Gtk::Box> (Gtk::Orientation::VERTICAL);

        auto hbox = Gtk::make_managed<Gtk::Box> ();
        hbox->set_halign (Gtk::Align::CENTER);
        hbox->append (*m_switcher);
        root->append (*hbox);

        root->append (*stack);
        set_child (*root);
      }

    UndoAction* execute_action (UndoAction *a2)
      {
        auto action = dynamic_cast<ArmySetUndoAction*>(a2);
        UndoAction *out = NULL;

        switch (action->get_type ())
          {
          case ArmySetUndoAction::CHANGE_PROPERTIES:
              {
                auto a = dynamic_cast<ArmySetUndoAction_Properties*>(action);
                out = new ArmySetUndoAction_Properties
                  (m_armyset->getName (),
                   m_armyset->getInfo (),
                   m_armyset->getCopyright (),
                   m_armyset->getLicense (),
                   m_armyset->getTileSize ());
                m_armyset->setName (a->get_name ());
                m_armyset->setInfo (a->get_description ());
                m_armyset->setCopyright (a->get_copyright ());
                m_armyset->setLicense (a->get_license ());
                m_armyset->setTileSize (a->get_tile_size ());
                break;
              }

          case ArmySetUndoAction::ADD_IMAGE:
              {
                auto a = dynamic_cast<ArmySetUndoAction_AddImage*>(action);
                out = new ArmySetUndoAction_AddImage (m_armyset);
                reload_armyset (a);
                break;
              }

          case ArmySetUndoAction::CLEAR_IMAGE:
              {
                auto a = dynamic_cast<ArmySetUndoAction_ClearImage*>(action);
                out = new ArmySetUndoAction_ClearImage (m_armyset);
                reload_armyset (a);
                break;
              }

          case ArmySetUndoAction::NAME:
              {
                auto a = dynamic_cast<ArmySetUndoAction_Name*>(action);
                out = new ArmySetUndoAction_Name
                  (a->get_index (), get_army_by_index (a)->getName (), m_umgr,
                   m_name_entry);
                get_army_by_index (a)->setName (a->get_name ());

                auto item = m_store->get_item (a->get_index ());
                if (item)
                  {
                    auto row = std::dynamic_pointer_cast<ArmyProtoRow>(item);
                    row->changed ();
                  }
                break;
              }

          case ArmySetUndoAction::MAKE_SAME:
              {
                auto a = dynamic_cast<ArmySetUndoAction_MakeSame*>(action);
                out = new ArmySetUndoAction_MakeSame (m_armyset, m_make_same);
                reload_armyset (a);
                m_make_same = a->get_make_same ();
                break;
              }

          case ArmySetUndoAction::REORDER:
              {
                auto a = dynamic_cast<ArmySetUndoAction_Reorder*>(action);
                out = new ArmySetUndoAction_Reorder (m_armyset);
                reload_armyset (a);
                break;
              }

          case ArmySetUndoAction::ADD_ARMY:
              {
                auto a = dynamic_cast<ArmySetUndoAction_AddArmy*>(action);
                out = new ArmySetUndoAction_AddArmy (m_armyset);
                reload_armyset (a);
                break;
              }

          case ArmySetUndoAction::REMOVE_ARMY:
              {
                auto a = dynamic_cast<ArmySetUndoAction_RemoveArmy*>(action);
                out = new ArmySetUndoAction_RemoveArmy (m_armyset);
                reload_armyset (a);
                break;
              }

          case ArmySetUndoAction::BONUS:
              {
                auto a = dynamic_cast<ArmySetUndoAction_Bonus*>(action);
                out = new ArmySetUndoAction_Bonus
                  (a->get_index (), a->get_bonus_type (),
                   get_army_by_index (a)->getArmyBonus () & a->get_bonus_type ());
                get_army_by_index (a)->setArmyBonus (a->get_bonus_type (),
                                                  a->get_flag ());
                break;
              }

          case ArmySetUndoAction::TURNS:
              {
                auto a = dynamic_cast<ArmySetUndoAction_Turns*>(action);
                out = new ArmySetUndoAction_Turns
                  (a->get_index (), get_army_by_index (a)->getProduction ());
                get_army_by_index (a)->setProduction (a->get_turns ());
                break;
              }

          case ArmySetUndoAction::COST:
              {
                auto a = dynamic_cast<ArmySetUndoAction_Cost*>(action);
                out = new ArmySetUndoAction_Cost
                  (a->get_index (), get_army_by_index (a)->getProductionCost ());
                get_army_by_index (a)->setProductionCost (a->get_cost ());
                break;
              }

          case ArmySetUndoAction::UPKEEP:
              {
                auto a = dynamic_cast<ArmySetUndoAction_Upkeep*>(action);
                out = new ArmySetUndoAction_Upkeep
                  (a->get_index (), get_army_by_index (a)->getUpkeep ());
                get_army_by_index (a)->setUpkeep (a->get_upkeep ());
                break;
              }

          case ArmySetUndoAction::NEW_COST:
              {
                auto a = dynamic_cast<ArmySetUndoAction_NewCost*>(action);
                out = new ArmySetUndoAction_NewCost
                  (a->get_index (),
                   get_army_by_index (a)->getNewProductionCost ());
                get_army_by_index (a)->setNewProductionCost (a->get_new_cost ());
                break;
              }

          case ArmySetUndoAction::STAT:
              {
                auto a = dynamic_cast<ArmySetUndoAction_Stat*>(action);
                guint32 val = 0;
                switch (a->get_stat_type ())
                  {
                  case ArmyBase::STRENGTH:
                    val = get_army_by_index (a)->getStrength ();
                    break;

                  case ArmyBase::MOVES:
                    val = get_army_by_index (a)->getMaxMoves ();
                    break;

                  case ArmyBase::SIGHT:
                    val = get_army_by_index (a)->getSight ();
                    break;

                  case ArmyBase::HP:
                  case ArmyBase::SHIP:
                  case ArmyBase::MOVE_BONUS:
                  case ArmyBase::ARMY_BONUS:
                  case ArmyBase::MOVES_MULTIPLIER:
                    break;
                  }

                out = new ArmySetUndoAction_Stat
                  (a->get_index (), a->get_stat_type (), val);
                switch (a->get_stat_type ())
                  {
                  case ArmyBase::STRENGTH:
                    get_army_by_index (a)->setStrength (a->get_value ());
                    break;

                  case ArmyBase::MOVES:
                    get_army_by_index (a)->setMaxMoves (a->get_value ());
                    break;

                  case ArmyBase::SIGHT:
                    get_army_by_index (a)->setSight (a->get_value ());
                    break;

                  case ArmyBase::HP:
                  case ArmyBase::SHIP:
                  case ArmyBase::MOVE_BONUS:
                  case ArmyBase::ARMY_BONUS:
                  case ArmyBase::MOVES_MULTIPLIER:
                    break;
                  }
                break;
              }

          case ArmySetUndoAction::ID:
              {
                ArmySetUndoAction_Id*a =
                  dynamic_cast<ArmySetUndoAction_Id*>(action);
                out = new ArmySetUndoAction_Id
                  (a->get_index (), get_army_by_index (a)->getId ());
                get_army_by_index (a)->setId (a->get_id ());
                break;
              }

          case ArmySetUndoAction::RUIN_AWARD:
              {
                auto a = dynamic_cast<ArmySetUndoAction_RuinAward*>(action);
                out = new ArmySetUndoAction_RuinAward
                  (a->get_index (), get_army_by_index (a)->getAwardable ());
                get_army_by_index (a)->setAwardable (a->get_award ());
                break;
              }

          case ArmySetUndoAction::DEFENDS_RUIN:
              {
                auto a =
                  dynamic_cast<ArmySetUndoAction_DefendsRuins*>(action);
                out = new ArmySetUndoAction_DefendsRuins
                  (a->get_index (), get_army_by_index (a)->getDefendsRuins ());
                get_army_by_index (a)->setDefendsRuins (a->get_defend ());
                break;
              }

          case ArmySetUndoAction::HERO:
              {
                auto a = dynamic_cast<ArmySetUndoAction_Hero*>(action);
                out = new ArmySetUndoAction_Hero
                  (a->get_index (), get_army_by_index (a)->getGender ());
                get_army_by_index (a)->setGender (a->get_hero ());
                break;
              }

          case ArmySetUndoAction::FASTER_IN_FORESTS:
              {
                auto a =
                  dynamic_cast<ArmySetUndoAction_FasterInForest*>(action);
                out = new ArmySetUndoAction_FasterInForest
                  (a->get_index (), get_army_by_index (a)->getMoveBonus ());
                get_army_by_index (a)->setMoveBonus (a->get_bonus ());
                break;
              }

          case ArmySetUndoAction::FASTER_IN_MARSHES:
              {
                auto a =
                  dynamic_cast<ArmySetUndoAction_FasterInMarsh*>(action);
                out = new ArmySetUndoAction_FasterInMarsh
                  (a->get_index (), get_army_by_index (a)->getMoveBonus ());
                get_army_by_index (a)->setMoveBonus (a->get_bonus ());
                break;
              }

          case ArmySetUndoAction::FASTER_IN_HILLS:
              {
                auto a =
                  dynamic_cast<ArmySetUndoAction_FasterInHills*>(action);
                out = new ArmySetUndoAction_FasterInHills
                  (a->get_index (), get_army_by_index (a)->getMoveBonus ());
                get_army_by_index (a)->setMoveBonus (a->get_bonus ());
                break;
              }

          case ArmySetUndoAction::FASTER_IN_MOUNTAINS:
              {
                auto a =
                  dynamic_cast<ArmySetUndoAction_FasterInMountains*>(action);
                out = new ArmySetUndoAction_FasterInMountains
                  (a->get_index (), get_army_by_index (a)->getMoveBonus ());
                get_army_by_index (a)->setMoveBonus (a->get_bonus ());
                break;
              }

          case ArmySetUndoAction::FLY:
              {
                auto a = dynamic_cast<ArmySetUndoAction_Fly*>(action);
                out = new ArmySetUndoAction_Fly
                  (a->get_index (), get_army_by_index (a)->getMoveBonus ());
                get_army_by_index (a)->setMoveBonus (a->get_bonus ());
                break;
              }

          case ArmySetUndoAction::SELECTOR:
              {
                auto a = dynamic_cast<ArmySetUndoAction_Selector*>(action);
                out = new ArmySetUndoAction_Selector (m_armyset);
                reload_armyset (a);
                break;
              }

          case ArmySetUndoAction::EXP:
              {
                auto a = dynamic_cast<ArmySetUndoAction_Exp*>(action);
                out = new ArmySetUndoAction_Exp
                  (a->get_index (), get_army_by_index (a)->getXpReward ());
                get_army_by_index (a)->setXpReward (a->get_exp ());
                break;
              }
          }
        return out;
      }

    void reload_armyset (ArmySetUndoAction_Save *action)
      {
        Glib::ustring olddir = m_armyset->getDirectory ();
        Glib::ustring oldname =
          File::get_basename (m_armyset->getConfigurationFile (true));
        Glib::ustring oldext = m_armyset->getExtension ();

        m_armyset->clean_tmp_dir ();
        delete m_armyset;
        m_armyset = new Armyset (*(action->get_armyset ()));
        m_armyset->setLoadTemporaryFile ();

        m_armyset->setDirectory (olddir);
        m_armyset->setBaseName (oldname);
        m_armyset->setExtension (oldext);
        fill_treeview ();
        sync_make_same ();
        update ();
      }

    void check_discard (Glib::ustring msg, sigc::slot<void(bool)> after)
      {
        if (m_armyset_modified || m_new_armyset_needs_saving)
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
                   case 0: //save
                     check_save_valid
                       (true,
                        [this, after] (bool valid)
                        {
                          if (!valid)
                            return after (false);

                          if (m_armyset->getDirectory ().empty () == false)
                            {
                              save_current_armyset_file_as
                                ([this, after] (bool saved)
                                 {
                                   after (saved);
                                 });
                            }
                          else
                            {
                              save_current_armyset_file
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

                   default: //cancel
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
        Glib::ustring name = m_armyset->getName ();
        Glib::ustring newname = "";
        if (existing)
          {
            Armyset *oldarmyset =
              Armysetlist::instance ()->get (m_armyset->getId ());
            if (oldarmyset && oldarmyset->getName () != name)
              newname = oldarmyset->getName ();
          }
        guint32 num = 0;
        Glib::ustring n = String::utrim (String::strip_trailing_numbers (name));
        if (n == "")
          n = _("Untitled");
        if (newname.empty () == true)
          newname =
            Armysetlist::instance ()->findFreeName (n, 100, num,
                                                    m_armyset->getTileSize ());
        if (name == "")
          {
            if (newname.empty () == true)
              {
                Glib::ustring msg =
                  _("The army set has an invalid name.\n"
                    "Change it and save again.");
                auto d = LwDialog::alert (msg);
                d->choose
                  (*this,
                   [this, d, after] (auto result)
                   {
                     d->choose_finish (result);
                     on_edit_armyset_info_activated ();
                     after (false);
                   });
                return;
              }
            else
              {
                Glib::ustring msg =
                  String::ucompose (_("The army set has an invalid name.\n"
                                      "Change it to '%1'?"), newname);
                auto d = LwDialog::alert_yn (msg);
                d->choose
                  (*this,
                   [this, d, existing, newname, after] (auto result)
                   {
                     bool change = d->choose_finish (result) == 1;
                     if (change)
                       {
                         m_armyset->setName (newname);
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
          Armysetlist::instance ()->lookupConfigurationFileByName (m_armyset);
        if (file == "")
          return after (true);

        Glib::ustring cfgfile = m_armyset->getConfigurationFile (true);

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
                  _("The army set has the same name as another one.\n"
                    "Change it and save again.");
                auto d = LwDialog::alert (msg);
                d->choose
                  (*this,
                   [this, d, after] (auto result)
                   {
                     d->choose_finish (result);
                     on_edit_armyset_info_activated ();
                     after (false);
                   });
              }
            else
              {
                Glib::ustring msg =
                  String::ucompose (_("The army set has the same name as "
                                      "another one.\nChange it to '%1' "
                                      "instead?"), newname);
                auto d = LwDialog::alert_yn (msg);
                d->choose
                  (*this,
                   [this, d, newname, after] (auto result)
                   {
                     bool change = d->choose_finish (result) == 1;
                     if (change)
                       m_armyset->setName (newname);
                     after (change);
                   });
              }
          }
      }

    bool is_valid_name ()
      {
        Glib::ustring file =
          Armysetlist::instance ()->lookupConfigurationFileByName (m_armyset);
        if (file == "")
          return true;
        if (file == m_armyset->getConfigurationFile (true))
          return true;
        return false;
      }

    void check_save_valid (bool existing, sigc::slot<void(bool)> after)
      {
        check_name_valid
          (existing,
           [this, existing, after] (bool valid)
           {
             if (!valid)
               return after (false);

             if (m_armyset->validate () == false)
               {
                 if (existing &&
                     Playerlist::instance ()->hasArmyset (m_armyset->getId ()))
                   {
                     Glib::ustring errmsg =
                       _("The army set is invalid, and is also one of the current working ones.");
                     Glib::ustring msg = _("The army set could not be saved.");
                     Glib::ustring detail = m_current_save_filename + "\n" + errmsg;
                     auto d = LwDialog::alert (msg, detail);
                     d->choose
                       (*this,
                        [this, d, after] (auto result)
                        {
                          d->choose_finish (result);
                          after (false);
                        });

                     return;
                   }
                 else
                   {
                     Glib::ustring msg =
                       _("The army set is invalid.  Do you want to proceed?");
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
               after (true);
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
              if (m_umgr->undo_empty () && !m_new_armyset_needs_saving)
                m_armyset_modified = false;
              update ();
            }));

        actions->add_action
          ("redo",
           ([this] ()
            {
              m_armyset_modified = true;
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

    void on_edit_armyset_info_activated ()
      {
        auto d = LwDialog::build<ArmySetInfoDialog> (this);
        d->setup (m_armyset);
        d->signal_response ().connect
          ([this, d] (Gtk::ResponseType resp)
           {
             if (d->is_changed () &&
                 resp == Gtk::ResponseType::ACCEPT)
               {
                 ArmySetUndoAction_Properties *action =
                   new ArmySetUndoAction_Properties
                   (m_armyset->getName (),
                    m_armyset->getInfo (),
                    m_armyset->getCopyright (),
                    m_armyset->getLicense (),
                    m_armyset->getTileSize ());
                 m_umgr->add (action);
                 m_armyset->setName (d->get_name ());
                 m_armyset->setInfo (d->get_description ());
                 m_armyset->setCopyright (d->get_copyright ());
                 m_armyset->setLicense (d->get_license ());
                 m_armyset->setTileSize (d->get_tile_size ());
                 m_armyset_modified = true;
                 update ();
               }
             delete d;
           });
      }

    void on_save_as_activated ()
      {
        check_save_valid
          (false,
           [this] (bool valid)
           {
             if (valid)
               {
                 save_current_armyset_file_as
                   ([this] (bool saved)
                    {
                      (void) saved;
                    });
               }
           });
      }

    void save_current_armyset_file_as (sigc::slot<void(bool)> after)
      {
        LwDialog::save
          (*this, _("Choose a Name"),
           File::sanify (m_armyset->getName ()), FileFilter::ARMYSET,
           [this, after] (std::string path)
           {
             Glib::ustring old_filename = m_current_save_filename;
             guint32 old_id = m_armyset->getId ();
             m_armyset->setId (Armysetlist::getNextAvailableId (old_id));

             save_current_armyset_file
               (path,
                [this, after, path, old_filename, old_id] (bool saved)
                {
                  if (saved == false)
                    {
                      m_current_save_filename = old_filename;
                      m_armyset->setId (old_id);
                    }
                  else
                    {
                      m_armyset_modified = false;
                      m_new_armyset_needs_saving = false;
                      m_armyset->created (path);
                      Glib::ustring dir =
                        File::add_slash_if_necessary (File::get_dirname (path));
                      if (dir == File::get_armyset_dir () ||
                          dir == File::get_user_armyset_dir ())
                        {
                          //if we saved it to a standard place, update the list
                          Armysetlist::instance ()->add
                            (Armyset::copy (m_armyset), path);
                          m_armyset_saved.emit (m_armyset->getId ());
                        }
                      update_armyset_panel ();
                      update_window_title ();
                    }
                  after (saved);
                });
           });
      }

    void save_current_armyset_file (Glib::ustring filename, sigc::slot<void(bool)> after)
      {
        m_current_save_filename = filename;
        if (m_current_save_filename.empty ())
          m_current_save_filename = m_armyset->getConfigurationFile (true);

        bool ok =
          m_armyset->save (m_current_save_filename, Armyset::file_extension);
        if (ok)
          {
            if (Armysetlist::instance ()->reload (m_armyset->getId ()))
              update_armyset_panel ();
            m_new_armyset_needs_saving = false;
            m_armyset_modified = false;
            update_window_title ();
            m_armyset_saved.emit (m_armyset->getId ());
            after (true);
          }
        else
          {
            Glib::ustring errmsg = Glib::strerror (errno);
            Glib::ustring msg = _("The army set could not be saved.");

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

    void check_quit (sigc::slot<void(bool)> after)
      {
        if (m_armyset_modified || m_new_armyset_needs_saving)
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

                          if (m_armyset->getDirectory ().empty () == false)
                            {
                              save_current_armyset_file_as
                                ([this, after] (bool saved)
                                 {
                                   after (saved);
                                 });
                            }
                          else
                            {
                              save_current_armyset_file
                                ("",
                                 [this, after] (bool saved)
                                 {
                                   after (saved);
                                 });
                            }
                        });
                     break;

                   case 1: // close and exit
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

    void change_image (Glib::ustring msg, TarFileImage *im,
                       sigc::slot<void(bool, bool, Glib::ustring)> after)
      {
        Glib::ustring imgname = im->getName ();

        auto d = LwDialog::build<ImageEditorDialog> (this);
        d->setup (im);
        d->set_title (msg);
        d->signal_response ().connect
          ([this, d, im, after] (Gtk::ResponseType resp)
           {
             bool cleared = false;
             Glib::ustring newfile = "";
             if (resp == Gtk::ResponseType::ACCEPT &&
                 d->is_changed () && d->get_filename () != "")
               {
                 auto action = new ArmySetUndoAction_AddImage (m_armyset);
                 Glib::ustring newname = "";
                 Glib::ustring err = "";
                 bool success = d->install_file (m_armyset, im,
                                                 d->get_filename (), err);
                 if (success)
                   {
                     m_umgr->add (action);
                     newfile = im->getName ();
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
                 auto action = new ArmySetUndoAction_ClearImage (m_armyset);
                 auto name = im->getName ();
                 if (d->uninstall_file (m_armyset, im, err))
                   {
                     m_umgr->add (action);
                     update ();
                     cleared = true;
                     newfile = "";
                     delete d;
                     m_armyset->uninstantiateSameNamedImages (name);
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

    void on_change_clicked (Glib::ustring msg, TarFileImage *im)
      {
        change_image
          (msg, im,
           [this, im](bool err, bool cleared, Glib::ustring f)
           {
             if (err)
               return;
             if (cleared)
               im->clear ();
             else
               {
                 if (f != "")
                   {
                     im->load (m_armyset, f);
                     im->instantiateImages ();
                   }
               }
             update_armyset_panel ();
           });
      }

    void change_image (Glib::ustring msg, TarFileMaskedImage *im, int lone,
                       sigc::slot<void(bool, bool, Glib::ustring)> after)
      {
        Glib::ustring imgname = im->getName ();

        auto d = LwDialog::build<MaskedImageEditorDialog> (this);
        if (lone != -1)
          d->set_color (m_shieldset, Shield::Color (lone));
        d->set_shieldset (m_shieldset);
        d->setup (im);
        d->set_title (msg);
        d->signal_response ().connect
          ([this, d, im, after] (Gtk::ResponseType resp)
           {
             bool cleared = false;
             Glib::ustring newfile = "";
             if (resp == Gtk::ResponseType::ACCEPT &&
                 d->is_changed () && d->get_filename () != "")
               {
                 auto action = new ArmySetUndoAction_AddImage (m_armyset);
                 Glib::ustring newname = "";
                 Glib::ustring err = "";
                 bool success = d->install_file (m_armyset, im,
                                                 d->get_filename (), err);
                 if (success)
                   {
                     m_umgr->add (action);
                     newfile = im->getName ();
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
                 auto action = new ArmySetUndoAction_ClearImage (m_armyset);
                 auto name = im->getName ();
                 if (d->uninstall_file (m_armyset, im, err))
                   {
                     m_umgr->add (action);
                     update ();
                     cleared = true;
                     newfile = "";
                     delete d;
                     m_armyset->uninstantiateSameNamedImages (name);
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

    void on_change_clicked (Glib::ustring msg, TarFileMaskedImage *im, int lone)
      {
        change_image
          (msg, im, lone,
           [this, im](bool err, bool cleared, Glib::ustring f)
           {
             if (err)
               return;
             if (cleared)
               im->clear ();
             else
               {
                 if (f != "")
                   {
                     im->load (m_armyset, f);
                     im->instantiateImages ();
                   }
               }
             update_armyset_panel ();
           });
      }

    void on_army_change_clicked (Glib::ustring msg, TarFileMaskedImage *im, int lone)
      {
        Glib::ustring oldname = im->getName ();
        change_image
          (msg, im, lone,
           [this, im, lone, oldname](bool err, bool cleared, Glib::ustring f)
           {
             if (err)
               return;
             if (cleared)
               {
                 im->clear ();
                 sync_make_same ();
               }
             else
               {
                 if (f != "")
                   {
                     im->load (m_armyset, f);
                     im->instantiateImages ();
                     if (lone == Shield::WHITE && m_make_same)
                       m_simple_actions["armyset.edit.make-same"]->activate ();
                   }
               }

             // blast other same named ones away
             // unless we changed white while make same is checked
             if (lone == Shield::WHITE && m_make_same && !cleared)
               ;
             else if (oldname.empty () == false)
               {
                 auto a = get_selected_army ();
                 for (guint32 i = Shield::WHITE; i <= Shield::NEUTRAL; i++)
                   {
                     if (lone >= 0 && i == (guint32)lone)
                       continue;
                     auto c = Shield::Color (i);
                     if (oldname == a->getMaskedImage (c)->getName ())
                       a->getMaskedImage (c)->clear ();
                   }
               }

             sync_make_same ();
             update_armyset_panel ();
           });
      }

    void setup_treeview ()
      {
        m_treeview->set_vexpand ();
        m_store = Gio::ListStore<ArmyProtoRow>::create ();
        m_selection_model = Gtk::SingleSelection::create (m_store);
        m_treeview->set_model (m_selection_model);

        LwColumn::setup_name_column<ArmyProtoRow>
        (m_treeview, true, _("Armies"),
           [] (const auto& row)
           {
             return row->m_army->getName ();
           });

        m_selection_model->signal_selection_changed ().connect
          ([this] (const guint &, const guint &)
           {
             sync_make_same ();
             scroll_army_to_top ();
             update_armyset_panel ();
             update_actions ();
           });

      }

    void fill_treeview ()
      {
        m_store->remove_all ();
        if (m_armyset)
          {
            for (auto a : *m_armyset)
              m_store->append (ArmyProtoRow::create (a));
          }

        m_selection_model->set_selected (0);
      }

    ArmyProto* get_army_by_index (ArmySetUndoAction_ArmyIndex *i)
      {
        auto item = m_store->get_item (i->get_index ());
        if (!item)
          return NULL;
        auto row = std::dynamic_pointer_cast<ArmyProtoRow>(item);
        return row->m_army;
      }

    void scroll_army_to_top ()
      {
        m_scrolled_window->get_vadjustment ()->set_value (0.0);
      }

    void scroll_treeview_to_bottom ()
      {
        guint n = m_store->get_n_items ();
        Glib::signal_idle ().connect_once
          ([this, n] ()
           {
             m_treeview->scroll_to (n - 1);
           });
      }

    void setup_quit ()
      {
        auto controller = Gtk::EventControllerKey::create ();
        controller->signal_key_pressed ().connect
          ([this] (guint keyval, guint, Gdk::ModifierType)
           {
             if (keyval == GDK_KEY_Escape)
               {
                 m_simple_actions["armyset.file.quit"]->activate ();
                 return true;
               }
             return false;
           }, false);
        add_controller (controller);

        signal_close_request ().connect
          ([this] () -> bool
           {
             m_simple_actions["armyset.file.quit"]->activate ();
             return true;
           }, false);
      }
                  
    void clear_army_panel ()
      {
        m_name_entry->set_text ("");
        m_white_image_filelabel->set_label ("");
        m_make_same = false;
        m_make_same_switch->set_active (false);
        m_green_image_filelabel->set_label ("");
        m_yellow_image_filelabel->set_label ("");
        m_light_blue_image_filelabel->set_label ("");
        m_red_image_filelabel->set_label ("");
        m_dark_blue_image_filelabel->set_label ("");
        m_orange_image_filelabel->set_label ("");
        m_black_image_filelabel->set_label ("");
        m_production_spinbutton->set_value
          (m_production_spinbutton->get_adjustment ()->get_lower ());
        m_cost_spinbutton->set_value
          (m_cost_spinbutton->get_adjustment ()->get_lower ());
        m_new_cost_spinbutton->set_value
          (m_new_cost_spinbutton->get_adjustment ()->get_lower ());
        m_upkeep_spinbutton->set_value
          (m_upkeep_spinbutton->get_adjustment ()->get_lower ());
        m_strength_spinbutton->set_value
          (m_strength_spinbutton->get_adjustment ()->get_lower ());
        m_moves_spinbutton->set_value
          (m_moves_spinbutton->get_adjustment ()->get_lower ());
        m_exp_spinbutton->set_value
          (m_exp_spinbutton->get_adjustment ()->get_lower ());
        m_id_spinbutton->set_value
          (m_id_spinbutton->get_adjustment ()->get_lower ());
        m_hero_combobox->set_active (0);
        m_awardable_switch->set_active (false);
        m_defends_ruins_switch->set_active (false);
        m_sight_spinbutton->set_value
          (m_sight_spinbutton->get_adjustment ()->get_lower ());
        m_move_forests_switch->set_active (false);
        m_move_marshes_switch->set_active (false);
        m_move_hills_switch->set_active (false);
        m_move_mountains_switch->set_active (false);
        m_can_fly_switch->set_active (false);
        m_add1strinopen_switch->set_active (false);
        m_add2strinopen_switch->set_active (false);
        m_add1strinforest_switch->set_active (false);
        m_add2strinforest_switch->set_active (false);
        m_add1strinhills_switch->set_active (false);
        m_add2strinhills_switch->set_active (false);
        m_add1strincity_switch->set_active (false);
        m_add2strincity_switch->set_active (false);
        m_add1stackinhills_switch->set_active (false);
        m_suballcitybonus_switch->set_active (false);
        m_sub1enemystack_switch->set_active (false);
        m_sub2enemystack_switch->set_active (false);
        m_add1stack_switch->set_active (false);
        m_add2stack_switch->set_active (false);
        m_suballnonherobonus_switch->set_active (false);
        m_suballherobonus_switch->set_active (false);
        m_confer_move_bonus_switch->set_active (false);
      }

    bool calculate_make_same ()
      {
        auto a = get_selected_army ();
        if (a)
          {
            if (a->getMaskedImage (Shield::WHITE)->getName ().empty ())
                return false;
            for (guint32 i = Shield::WHITE + 1; i <= Shield::NEUTRAL; i++)
              {
                if (a->getMaskedImage (Shield::WHITE)->getName () !=
                    a->getMaskedImage (Shield::Color (i))->getName ())
                  return false;
              }
            return true;
          }
        else
          return false;
      }

    void sync_make_same ()
      {
        m_make_same = calculate_make_same ();
      }
};
#endif
