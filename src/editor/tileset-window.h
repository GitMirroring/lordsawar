//  Copyright (C) 2008, 2009, 2010, 2011, 2012, 2014, 2015, 2017, 2020, 2021,
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
#ifndef TILESET_WINDOW_H
#define TILESET_WINDOW_H

#include "tile-set.h"
#include "tar-file-image.h"
#include "tar-file-masked-image.h"
#include "undo-mgr.h"
#include "tileset-undo.h"
#include "about-dialog.h"
#include "image-editor-dialog.h"
#include "masked-image-editor-dialog.h"
#include "tileset-info-dialog.h"
#include "file-label.h"
#include "tilestyles-dialog.h"
#include "tile-preview-dialog.h"
#include "lw-column.h"

class TileSetMenuButton: public Gtk::MenuButton
{
public:
    TileSetMenuButton (Gtk::Window &p, Glib::RefPtr<Gio::SimpleActionGroup> &group)
      :m_parent (p), m_actions (group)
      {
      }

    ~TileSetMenuButton () override = default;

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

        menu->append (_("New"), "lw.tileset.file.new");
        menu->append (_("Open..."), "lw.tileset.file.open");
        menu->append (_("Save"), "lw.tileset.file.save");
        menu->append (_("Save As..."), "lw.tileset.file.save-as");
        menu->append (_("Validate"), "lw.tileset.file.validate");
        menu->append (_("Properties"), "lw.tileset.edit.properties");
        menu->append (_("Tile Styles..."), "lw.tileset.edit.tilestyles");
        menu->append (_("Preview Tile"), "lw.tileset.view.preview-tile");
        auto help_menu = Gio::Menu::create ();
        help_menu->append (_("Tutorial Video"), "lw.tileset.help.tutorial-video");
        help_menu->append (_("About"), "lw.tileset.help.about");
        menu->append_submenu (_("Help"), help_menu);
        menu->append (_("Keyboard Shortcuts"), "lw.tileset.help.keyboard-shortcuts");
        menu->append (_("Quit"), "lw.tileset.file.quit");

        property_primary () = true;
        set_menu_model (menu);

        setup_actions
          ({"tileset.file.new", "tileset.file.open", "tileset.file.save",
            "tileset.file.save-as", "tileset.file.validate",
            "tileset.file.quit", "tileset.edit.properties",
            "tileset.edit.tilestyles", "tileset.view.preview-tile",
            "tileset.help.tutorial-video", "tileset.help.about",
            "tileset.help.keyboard-shortcuts", 

            // the other actions not in the menu, it's just easier to add these
            // here
            "tileset.add-tile", "tileset.remove-tile",
          });

        set_icon_name ("open-menu-symbolic");

      }

    void setup_action (Glib::RefPtr<Gio::SimpleAction> a)
      {
        m_signal_action_added.emit (a);
      }
};

class TileRow: public Glib::Object
{
public:
    Tile *m_tile;
    sigc::signal<void()> m_signal_changed;

    sigc::signal<void()> signal_changed ()
      {
        return m_signal_changed;
      }

    static Glib::RefPtr<TileRow> create (Tile *t)
      {
        return
          Glib::make_refptr_for_instance<TileRow> (new TileRow (t));
      }

    void changed ()
      {
        m_signal_changed.emit ();
      }

protected:
    TileRow (Tile *t)
      : m_tile (t)
      {
      }
};

class TileSetShortcutsDialog : public Gtk::Window
{
public:
    TileSetShortcutsDialog ()
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

class TileSetWindow: public Gtk::ApplicationWindow
{
public:

    TileSetWindow ()
      : m_actions (Gio::SimpleActionGroup::create ())
      {
        m_tileset = NULL;
        set_size_request (650, 450);
        m_umgr = new UndoMgr (UndoMgr::DELAY, UndoMgr::LIMIT);
        m_umgr->signal_execute ().connect
          (sigc::mem_fun (*this, &TileSetWindow::execute_action));
        m_umgr->signal_added_undo ().connect
          ([this] ()
           {
             m_tileset_modified = true;
             update_window_title ();
             update_actions ();
           });
        set_resizable (false);
      }

    ~TileSetWindow ()
      {
        disconnect_signals ();
        disconnect_action_signals ();
        delete m_umgr;
      }

    void setup (Shieldset *shieldset, Tileset *tileset)
      {
        m_shieldset = shieldset;
        m_tileset = NULL;
        if (tileset)
          {
            bool broken = false;
            m_tileset = new Tileset (*tileset);
            m_tileset->instantiateImages (broken);
            m_tileset->setLoadTemporaryFile ();
            m_current_save_filename = tileset->getConfigurationFile (true);
          }
        m_tileset_modified = false;
        m_new_tileset_needs_saving = false;

        m_menu_button = Gtk::make_managed<TileSetMenuButton>(*this, m_actions);
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

        connect_action_signals ();
        connect_signals ();

        setup_quit ();

        update ();
      }

    sigc::signal<void(guint32)> signal_tileset_saved ()
      {
        return m_tileset_saved;
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
    TileSetMenuButton *m_menu_button;
    Gtk::StackSwitcher *m_switcher;
    std::map<Glib::ustring, Glib::RefPtr<Gio::SimpleAction>> m_simple_actions;
    Gtk::ScrolledWindow *m_scrolled_window;
    Gtk::ScrolledWindow *m_columnview_scrolled_window;
    Gtk::Entry *m_name_entry;
    Gtk::ColorButton *m_road_colorbutton;
    Gtk::ColorButton *m_ruin_colorbutton;
    Gtk::ColorButton *m_temple_colorbutton;
    Gtk::Button *m_small_select_button;
    FileLabel *m_small_select_filelabel;
    Gtk::Button *m_large_select_button;
    FileLabel *m_large_select_filelabel;
    Gtk::Button *m_explosion_button;
    FileLabel *m_explosion_filelabel;
    Gtk::Button *m_roads_button;
    FileLabel *m_roads_filelabel;
    Gtk::Button *m_stones_button;
    FileLabel *m_stones_filelabel;
    Gtk::Button *m_bridges_button;
    FileLabel *m_bridges_filelabel;
    Gtk::Button *m_fog_button;
    FileLabel *m_fog_filelabel;
    Gtk::Button *m_white_flags_button;
    FileLabel *m_white_flags_filelabel;
    Gtk::Button *m_green_flags_button;
    FileLabel *m_green_flags_filelabel;
    Gtk::Button *m_yellow_flags_button;
    FileLabel *m_yellow_flags_filelabel;
    Gtk::Button *m_dark_blue_flags_button;
    FileLabel *m_dark_blue_flags_filelabel;
    Gtk::Button *m_orange_flags_button;
    FileLabel *m_orange_flags_filelabel;
    Gtk::Button *m_light_blue_flags_button;
    FileLabel *m_light_blue_flags_filelabel;
    Gtk::Button *m_red_flags_button;
    FileLabel *m_red_flags_filelabel;
    Gtk::Button *m_black_flags_button;
    FileLabel *m_black_flags_filelabel;
    Gtk::Button *m_neutral_flags_button;
    FileLabel *m_neutral_flags_filelabel;
    Gtk::Button *m_move_bonus_forest_button;
    FileLabel *m_move_bonus_forest_filelabel;
    Gtk::Button *m_move_bonus_hills_button;
    FileLabel *m_move_bonus_hills_filelabel;
    Gtk::Button *m_move_bonus_water_button;
    FileLabel *m_move_bonus_water_filelabel;
    Gtk::Button *m_move_bonus_swamp_button;
    FileLabel *m_move_bonus_swamp_filelabel;
    Gtk::Button *m_move_bonus_mountains_button;
    FileLabel *m_move_bonus_mountains_filelabel;
    Gtk::Button *m_move_bonus_fly_button;
    FileLabel *m_move_bonus_fly_filelabel;
    LwCombo *m_type_combobox;
    LwCombo *m_pattern_combobox;
    Gtk::SpinButton *m_moves_spinbutton;
    Gtk::ColorButton *m_first_colorbutton;
    Gtk::ColorButton *m_second_colorbutton;
    Gtk::ColorButton *m_third_colorbutton;
    Gtk::Button *m_tilestyles_button;

    Gtk::ColumnView *m_treeview;
    Glib::RefPtr<Gtk::SingleSelection> m_selection_model;
    Glib::RefPtr<Gio::ListStore<TileRow>> m_store;

    std::list<sigc::connection> m_action_connections;
    std::list<sigc::connection> m_connections;

    Shieldset *m_shieldset;
    Tileset *m_tileset;
    Glib::ustring m_current_save_filename;
    bool m_tileset_modified;
    bool m_new_tileset_needs_saving;
    UndoMgr *m_umgr;

    sigc::signal<void(guint32)> m_tileset_saved;
    sigc::signal<void()> m_signal_closed;

    void setup_header_bar (Gtk::MenuButton *menu_button)
      {
        m_header_bar = Gtk::make_managed<Gtk::HeaderBar>();
        m_header_bar->set_show_title_buttons (true);

        m_notebook = Gtk::make_managed<Gtk::Notebook> ();
        m_notebook->set_show_tabs (false);
        m_notebook->set_show_border (false);

        auto page1 = Gtk::make_managed<Gtk::Box> (Gtk::Orientation::VERTICAL);
        auto title1 = Gtk::make_managed<Gtk::Label>(_("Tile Set Editor"));
        title1->add_css_class ("title");
        page1->append (*title1);
        page1->set_valign (Gtk::Align::CENTER);
        m_notebook->append_page (*page1, "");

        auto page2 = Gtk::make_managed<Gtk::Box> (Gtk::Orientation::VERTICAL);
        auto title2 = Gtk::make_managed<Gtk::Label>(_("Tile Set Editor"));
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
          ("tileset.file.new",
           [this] ()
           {
             check_discard
               (_("Save these changes before making a new tile set?"),
                [this] (bool discard)
                {
                  if (discard)
                    {
                      m_current_save_filename = "";
                      disconnect_signals ();
                      if (m_tileset)
                        delete m_tileset;

                      guint32 num = 0;
                      Glib::ustring name =
                        Tilesetlist::instance ()->findFreeName
                        (_("Untitled"), 100, num,
                         Tileset::get_default_tile_size ());

                      m_tileset =
                        new Tileset (Tilesetlist::getNextAvailableId (1), name);
                      m_tileset->setNewTemporaryFile ();
                      connect_signals ();

                      m_tileset_modified = false;
                      m_new_tileset_needs_saving = true;
                      m_umgr->clear ();
                      fill_treeview ();
                      update ();
                      update_actions ();
                    }
                });
           });

        action_connect
          ("tileset.file.open",
           [this] ()
           {
             check_discard
               (_("Save these changes before making a new tile Set?"),
                [this] (bool discard)
                {
                  if (discard)
                    {
                      LwDialog::open
                        (*this, _("Choose a tile set to open"),
                         FileFilter::TILESET,
                         [this] (std::string path)
                         {
                           load_tileset
                             (path,
                              [this] (bool loaded)
                              {
                                if (loaded)
                                  {
                                    m_tileset_modified = false;
                                    m_new_tileset_needs_saving = false;
                                    update ();
                                  }
                              });
                         });
                    }
                });
           });

        action_connect
          ("tileset.file.save",
           [this] ()
           {
             if (m_current_save_filename.empty () == true)
               on_save_as_activated ();
             else
               {
                 check_save_valid
                     (true,
                      [this] (bool valid)
                      {
                        if (valid)
                          {
                            save_current_tileset_file
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
          ("tileset.file.save-as",
           [this] ()
           {
             on_save_as_activated ();

           });

        action_connect
          ("tileset.file.validate",
           [this] ()
           {
             std::list<Glib::ustring> msgs;
             if (m_tileset == NULL)
               return;
             if (String::utrim (m_tileset->getName ()) == "")
               msgs.push_back (_("The name of the tile set is invalid."));
             if (m_tileset->empty () == true)
               msgs.push_back
                 (_("There must be at least one tile in the tile set."));
             if (m_tileset->getIndex (Tile::GRASS) == -1)
               msgs.push_back
                 (_("There must be a grass tile in the tile set."));
             if (m_tileset->getIndex (Tile::WATER) == -1)
               msgs.push_back
                 (_("There must be a water tile in the tile set."));
             if (m_tileset->getIndex(Tile::FOREST) == -1)
               msgs.push_back
                 (_("There must be a forest tile in the tile set."));
             if (m_tileset->getIndex(Tile::HILLS) == -1)
               msgs.push_back
                 (_("There must be a hills tile in the tile set."));
             if (m_tileset->getIndex(Tile::MOUNTAIN) == -1)
               msgs.push_back
                 (_("There must be a mountain tile in the tile set."));
             if (m_tileset->getIndex(Tile::SWAMP) == -1)
               msgs.push_back
                 (_("There must be a swamp tile in the tile set."));
             for (auto it = m_tileset->begin (); it != m_tileset->end (); ++it)
               {
                 if ((*it)->empty ())
                   msgs.push_back
                     (String::ucompose
                      (_("There must be at least one tilestyleset in the %1 "
                         "tile."),
                       (*it)->getName ()));
                 for (auto j = (*it)->begin (); j != (*it)->end (); ++j)
                   {
                     if ((*j)->validate () == false)
                       msgs.push_back
                         (String::ucompose
                          (_("The image %1 file of the %2 tile does not have a "
                             "width as a multiple of its height."),
                           (*j)->getName (),(*it)->getName ()));
                   }

                 //fill up the tile style types so we can validate them.
                 std::list<TileStyle::Type> types;
                 for (auto i = (*it)->begin (); i != (*it)->end (); ++i)
                   (*i)->getUniqueTileStyleTypes (types);

                 switch ((*it)->getType ())
                   {
                   case Tile::GRASS:
                     if ((*it)->validateGrass(types) == false)
                       msgs.push_back
                         (String::ucompose
                          (_("The %1 tile does not have enough of the right "
                             "kind of tile styles."),
                           (*it)->getName ()));
                     break;

                   case Tile::FOREST: case Tile::WATER: case Tile::HILLS:
                   case Tile::SWAMP: case Tile::MOUNTAIN:
                     if ((*it)->validateFeature (types) == false)
                       {
                         if ((*it)->validateGrass (types) == false)
                           msgs.push_back
                             (String::ucompose
                              (_("The %1 tile does not have enough of the "
                                 "right kind of tile styles."),
                               (*it)->getName ()));
                       }
                     break;
                   }
               }
             if (m_tileset->countTilesWithPattern
                 (SmallTile::SUNKEN_RADIAL) > 1)
               msgs.push_back
                 (_("Only one tile can have a sunken radial pattern."));

             if (m_tileset->getSelector (true)->getName ().empty () == true)
               msgs.push_back (_("A large selector image is required."));
             if (m_tileset->getSelector (false)->getName ().empty () == true)
               msgs.push_back (_("A small selector image is required."));
             if (m_tileset->getExplosion ()->getName ().empty () == true)
               msgs.push_back (_("An explosion image is required."));
             if (m_tileset->getRoad ()->getName ().empty () == true)
               msgs.push_back (_("A roads image is required."));
             if (m_tileset->getStone ()->getName().empty () == true)
               msgs.push_back (_("A standing stones image is required."));
             if (m_tileset->getBridge ()->getName ().empty () == true)
               msgs.push_back (_("A bridges image is required."));
             if (m_tileset->getFog ()->getName ().empty () == true)
               msgs.push_back (_("A set of fog images are required."));
             Shield::Color c = Shield::WHITE;
             if (m_tileset->getFlags (c)->getName().empty () == true)
               msgs.push_back (_("A set of white flag images are required."));
             c = Shield::GREEN;
             if (m_tileset->getFlags (c)->getName().empty () == true)
               msgs.push_back (_("A set of green flag images are required."));
             c = Shield::YELLOW;
             if (m_tileset->getFlags (c)->getName().empty () == true)
               msgs.push_back (_("A set of yellow flag images are required."));
             c = Shield::DARK_BLUE;
             if (m_tileset->getFlags (c)->getName().empty () == true)
               msgs.push_back (_("A set of dark blue flag images are required."));
             c = Shield::ORANGE;
             if (m_tileset->getFlags (c)->getName().empty () == true)
               msgs.push_back (_("A set of orange flag images are required."));
             c = Shield::LIGHT_BLUE;
             if (m_tileset->getFlags (c)->getName().empty () == true)
               msgs.push_back (_("A set of light blue flag images are required."));
             c = Shield::RED;
             if (m_tileset->getFlags (c)->getName().empty () == true)
               msgs.push_back (_("A set of red flag images are required."));
             c = Shield::BLACK;
             if (m_tileset->getFlags (c)->getName().empty () == true)
               msgs.push_back (_("A set of black flag images are required."));
             c = Shield::NEUTRAL;
             if (m_tileset->getFlags (c)->getName().empty () == true)
               msgs.push_back (_("A set of neutral flag images are required."));
             if (m_tileset->getAllMoveBonus ()->getName ().empty () == true)
               msgs.push_back (_("A flight movement bonus image is required."));
             if (m_tileset->getWaterMoveBonus ()->getName ().empty () == true)
               msgs.push_back (_("A water movement bonus image is required."));
             if (m_tileset->getForestMoveBonus ()->getName ().empty () == true)
               msgs.push_back (_("A forest movement bonus image is required."));
             if (m_tileset->getHillsMoveBonus ()->getName ().empty () == true)
               msgs.push_back (_("A hills movement bonus image is required."));
             if (m_tileset->getMountainsMoveBonus ()->getName ().empty () ==
                 true)
               msgs.push_back
                 (_("A mountains movement bonus image is required."));
             if (m_tileset->getSwampMoveBonus ()->getName ().empty () == true)
               msgs.push_back (_("A swamp movement bonus image is required."));

             if (is_valid_name () == false)
               msgs.push_back(_("The name of the tile set is not unique."));

             Glib::ustring msg = "";
             for (auto it = msgs.begin (); it != msgs.end (); ++it)
               {
                 msg += (*it) + "\n";
                 break;
               }

             if (msg == "")
               msg = _("The tile set is valid.");

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

             return;
           });

        action_connect
          ("tileset.view.preview-tile",
           [this] ()
           {
             Tile *tile = get_selected_tile ();
             //determine transition tile type
             Tile *sec = NULL;
             int idx = -1;
             if (tile->getType () == Tile::MOUNTAIN)
               idx = m_tileset->getIndex (Tile::HILLS);
             else
               idx = m_tileset->getIndex (Tile::GRASS);
             if (idx > -1)
               sec = (*m_tileset)[idx];
             auto d = LwDialog::build<TilePreviewDialog> (this);
             d->setup (tile, sec, m_tileset->getTileSize ());
             d->signal_response ().connect
               ([d] (Gtk::ResponseType)
                {
                  delete d;
                });
           });

        action_connect
          ("tileset.file.quit",
           [this] ()
           {
             check_quit
               ([this] (bool quit)
                {
                  if (quit)
                    {
                      if (m_tileset)
                        delete m_tileset;
                      hide ();
                      m_signal_closed.emit ();
                    }
                });
           });

        action_connect
          ("tileset.edit.properties",
           [this] ()
           {
             on_edit_tileset_info_activated ();
           });

        action_connect
          ("tileset.add-tile",
           [this] ()
           {
             m_umgr->add
               (new TileSetUndoAction_AddTile (m_tileset));

             Tile *t = new Tile ();
             t->setName (_("Untitled"));
             m_store->append (TileRow::create (t));
             m_tileset->push_back (t);
             m_selection_model->set_selected (m_store->get_n_items () - 1);
             scroll_treeview_to_bottom ();
             update ();
           });

        action_connect
          ("tileset.remove-tile",
           [this] ()
           {
             auto selected = m_selection_model->get_selected_item ();
             if (selected)
               {
                 m_umgr->add
                   (new TileSetUndoAction_RemoveTile (m_tileset));
                 auto tile = get_selected_tile ();

                 for (auto it = m_tileset->begin (); it != m_tileset->end ();
                      ++it)
                   {
                     if (*it == tile)
                       {
                         m_tileset->erase (it);
                         break;
                       }
                   }
                 delete tile;

                 m_store->remove (m_selection_model->get_selected ());
                 scroll_tile_to_top ();
                 update ();
               }
           });

        action_connect
          ("tileset.help.tutorial-video",
           [this] ()
           {
             Glib::ustring uri = "https://vimeo.com/407659798";
             auto launcher = Gtk::UriLauncher::create (uri);

             launcher->launch
               (*this,
                [launcher] (const Glib::RefPtr<Gio::AsyncResult>& result)
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
          ("tileset.help.keyboard-shortcuts",
           [this] ()
           {
             auto* dialog = Gtk::make_managed<TileSetShortcutsDialog>();
             dialog->set_transient_for (*this);
             dialog->present ();
           });

        action_connect
          ("tileset.help.about",
           [this] ()
           {
             auto d = Gtk::make_managed<AboutDialog> (*this);
             d->set_program_name ("LordsAWar! Tile Set Editor");
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

        action_connect
          ("tileset.edit.tilestyles",
           [this] ()
           {
             auto action = new TileSetUndoAction_TileStyles (m_tileset);
             auto d = LwDialog::build<TileStylesDialog> (this);
             d->setup (m_tileset, get_selected_tile ());
             d->signal_response ().connect
               ([this, d, action] (Gtk::ResponseType)
                {
                  bool changed = d->is_changed ();
                  if (changed)
                    m_umgr->add (action);
                  else
                    delete action;
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

    void load_tileset (Glib::ustring filename, sigc::slot<void(bool)> after)
      {
        Glib::ustring old = m_current_save_filename;
        m_current_save_filename = filename;

        Tileset::create
          (filename,
           [this, after, old] (Tileset *tileset, bool broken,
                               bool unsupported_version, Glib::ustring err)
           {
             if (tileset == NULL || unsupported_version || broken)
               {
                 Glib::ustring msg = _("The tile set could not be loaded.");
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
             Tileset *old_tileset = m_tileset;
             m_tileset = NULL;
             update ();
             update_actions ();
             m_tileset = tileset;
             fill_treeview ();
             connect_signals ();
             m_tileset->setLoadTemporaryFile ();

             Glib::signal_idle ().connect
               ([this, old_tileset, after] ()
                {
                  bool broke = false;
                  m_tileset->instantiateImages (broke);
                  if (broke)
                    {
                      delete m_tileset;
                      m_tileset = NULL;
                      Glib::ustring msg = _("Couldn't load tile set images");
                      auto dialog = LwDialog::alert (msg);
                      dialog->choose
                        (*this,
                         [this, old_tileset, after, dialog] (auto result)
                         {
                           dialog->choose_finish (result);
                           m_tileset = old_tileset;
                           update ();
                           return after (false);
                         });
                    }
                  else
                    {
                      delete old_tileset;
                      update ();
                      after (true);
                    }
                  return false;
                });
           });
      }

    Tile *get_selected_tile ()
      {
        auto item = m_selection_model->get_selected_item ();
        if (!item)
          return NULL;
        auto row = std::dynamic_pointer_cast<TileRow>(item);
        return row->m_tile;
      }

    guint32 get_selected_index ()
      {
        return m_selection_model->get_selected ();
      }

    void connect_signals ()
      {
        add_connection
          (m_name_entry->signal_changed ().connect
           ([this] ()
            {
              auto t = get_selected_tile ();
              m_umgr->add
                (new TileSetUndoAction_Name (get_selected_index (),
                                             t->getName (), m_umgr,
                                             m_name_entry));
              t->setName (m_name_entry->get_text ());

              auto item = m_selection_model->get_selected_item ();
              if (item)
                {
                  auto row = std::dynamic_pointer_cast<TileRow>(item);
                  row->changed ();
                }
            }));

        add_connection
          (m_type_combobox->signal_changed ().connect
           ([this] ()
            {
              auto t = get_selected_tile ();

              m_umgr->add
                (new TileSetUndoAction_Type (get_selected_index (), t));
              t->setTypeByIndex (m_type_combobox->get_active_row_number ());
              set_default_values_by_type ();
              update ();
            }));

        add_connection
          (m_pattern_combobox->signal_changed ().connect
           ([this] ()
            {
              auto t = get_selected_tile ();
              m_umgr->add
                (new TileSetUndoAction_Pattern
                 (get_selected_index (), t->getSmallTile ()->getPattern ()));
              t->getSmallTile ()->setPattern
                (SmallTile::Pattern (m_pattern_combobox->get_active_row_number ()));
              update_tileset_panel ();
            }));

        add_connection
          (m_moves_spinbutton->signal_value_changed ().connect
           ([this] ()
            {
              auto t = get_selected_tile ();
              m_umgr->add
                (new TileSetUndoAction_Moves (get_selected_index (),
                                              t->getMoves ()));
              t->setMoves (m_moves_spinbutton->get_value ());
            }));

        add_connection
          (m_first_colorbutton->property_rgba ().signal_changed ().connect
           ([this] ()
            {
              auto t = get_selected_tile ();
              m_umgr->add
                (new TileSetUndoAction_Color (get_selected_index (), 0,
                                              t->getSmallTile ()->getColor ()));
              t->getSmallTile ()->setColor (m_first_colorbutton->get_rgba ());
            }));

        add_connection
          (m_second_colorbutton->property_rgba ().signal_changed ().connect
           ([this] ()
            {
              auto t = get_selected_tile ();
              m_umgr->add
                (new TileSetUndoAction_Color
                 (get_selected_index (), 1,
                  t->getSmallTile ()->getSecondColor ()));
              t->getSmallTile ()->setSecondColor
                (m_second_colorbutton->get_rgba ());
            }));

        add_connection
          (m_third_colorbutton->property_rgba ().signal_changed ().connect
           ([this] ()
            {
              auto t = get_selected_tile ();
              m_umgr->add
                (new TileSetUndoAction_Color
                 (get_selected_index (), 2,
                  t->getSmallTile ()->getThirdColor ()));
              t->getSmallTile ()->setThirdColor
                (m_second_colorbutton->get_rgba ());
            }));

        add_connection
          (m_small_select_button->signal_clicked ().connect
           ([this] ()
            {
              on_change_clicked (_("Select a small selector image"),
                                 m_tileset->getSelector (false), -1);
            }));

        add_connection
          (m_large_select_button->signal_clicked ().connect
           ([this] ()
            {
              on_change_clicked (_("Select a large selector image"),
                                 m_tileset->getSelector (true), -1);
            }));

        add_connection
          (m_explosion_button->signal_clicked ().connect
           ([this] ()
            {
              on_change_clicked (_("Select an explosion image"),
                                 m_tileset->getExplosion ());
            }));

        add_connection
          (m_roads_button->signal_clicked ().connect
           ([this] ()
            {
              on_change_clicked (_("Select a roads image"),
                                 m_tileset->getRoad ());
            }));

        add_connection
          (m_stones_button->signal_clicked ().connect
           ([this] ()
            {
              on_change_clicked (_("Select a stones image"),
                                 m_tileset->getStone ());
            }));

        add_connection
          (m_bridges_button->signal_clicked ().connect
           ([this] ()
            {
              on_change_clicked (_("Select a bridges image"),
                                 m_tileset->getBridge ());
            }));

        add_connection
          (m_fog_button->signal_clicked ().connect
           ([this] ()
            {
              on_change_clicked (_("Select a fog image"),
                                 m_tileset->getFog ());
            }));

        add_connection
          (m_white_flags_button->signal_clicked ().connect
           ([this] ()
            {
              on_flag_change_clicked (_("Select a white flags image"),
                                      m_tileset->getFlags (Shield::WHITE),
                                      Shield::WHITE);
            }));

        add_connection
          (m_green_flags_button->signal_clicked ().connect
           ([this] ()
            {
              on_flag_change_clicked (_("Select a green flags image"),
                                      m_tileset->getFlags (Shield::GREEN),
                                      Shield::GREEN);
            }));

        add_connection
          (m_yellow_flags_button->signal_clicked ().connect
           ([this] ()
            {
              on_flag_change_clicked (_("Select a yellow flags image"),
                                      m_tileset->getFlags (Shield::YELLOW),
                                      Shield::YELLOW);
            }));

        add_connection
          (m_dark_blue_flags_button->signal_clicked ().connect
           ([this] ()
            {
              on_flag_change_clicked (_("Select a dark blue flags image"),
                                      m_tileset->getFlags (Shield::DARK_BLUE),
                                      Shield::DARK_BLUE);
            }));

        add_connection
          (m_orange_flags_button->signal_clicked ().connect
           ([this] ()
            {
              on_flag_change_clicked (_("Select an orange flags image"),
                                      m_tileset->getFlags (Shield::ORANGE),
                                      Shield::ORANGE);
            }));

        add_connection
          (m_light_blue_flags_button->signal_clicked ().connect
           ([this] ()
            {
              on_flag_change_clicked (_("Select a light blue flags image"),
                                      m_tileset->getFlags (Shield::LIGHT_BLUE),
                                      Shield::LIGHT_BLUE);
            }));

        add_connection
          (m_red_flags_button->signal_clicked ().connect
           ([this] ()
            {
              on_flag_change_clicked (_("Select a red flags image"),
                                      m_tileset->getFlags (Shield::RED),
                                      Shield::RED);
            }));

        add_connection
          (m_black_flags_button->signal_clicked ().connect
           ([this] ()
            {
              on_flag_change_clicked (_("Select a black flags image"),
                                      m_tileset->getFlags (Shield::BLACK),
                                      Shield::BLACK);
            }));

        add_connection
          (m_neutral_flags_button->signal_clicked ().connect
           ([this] ()
            {
              on_flag_change_clicked (_("Select a neutral flags image"),
                                      m_tileset->getFlags (Shield::NEUTRAL),
                                      Shield::NEUTRAL);
            }));

        add_connection
          (m_move_bonus_forest_button->signal_clicked ().connect
           ([this] ()
            {
              on_change_clicked (_("Select a forest move bonus image"),
                                 m_tileset->getForestMoveBonus ());
            }));

        add_connection
          (m_move_bonus_hills_button->signal_clicked ().connect
           ([this] ()
            {
              on_change_clicked (_("Select a hills move bonus image"),
                                 m_tileset->getHillsMoveBonus ());
            }));

        add_connection
          (m_move_bonus_water_button->signal_clicked ().connect
           ([this] ()
            {
              on_change_clicked (_("Select a water move bonus image"),
                                 m_tileset->getWaterMoveBonus ());
            }));

        add_connection
          (m_move_bonus_swamp_button->signal_clicked ().connect
           ([this] ()
            {
              on_change_clicked (_("Select a swamp move bonus image"),
                                 m_tileset->getSwampMoveBonus ());
            }));

        add_connection
          (m_move_bonus_mountains_button->signal_clicked ().connect
           ([this] ()
            {
              on_change_clicked (_("Select a mountains move bonus image"),
                                 m_tileset->getMountainsMoveBonus ());
            }));

        add_connection
          (m_move_bonus_fly_button->signal_clicked ().connect
           ([this] ()
            {
              on_change_clicked (_("Select a flight move bonus image"),
                                 m_tileset->getAllMoveBonus ());
            }));

        add_connection
          (m_road_colorbutton->property_rgba ().signal_changed ().connect
           ([this] ()
            {
              m_umgr->add (new TileSetUndoAction_BuildingColors (m_tileset));
              m_tileset->setRoadColor (m_road_colorbutton->get_rgba ());
            }));

        add_connection
          (m_ruin_colorbutton->property_rgba ().signal_changed ().connect
           ([this] ()
            {
              m_umgr->add (new TileSetUndoAction_BuildingColors (m_tileset));
              m_tileset->setRuinColor (m_ruin_colorbutton->get_rgba ());
            }));

        add_connection
          (m_temple_colorbutton->property_rgba ().signal_changed ().connect
           ([this] ()
            {
              m_umgr->add (new TileSetUndoAction_BuildingColors (m_tileset));
              m_tileset->setTempleColor (m_temple_colorbutton->get_rgba ());
            }));
      }

    void update ()
      {
        update_window_title ();
        update_tileset_panel ();
        update_actions ();
      }

    void update_actions ()
      {

        bool a = m_tileset != NULL;
        m_simple_actions ["tileset.edit.properties"]->set_enabled (a);

        bool b =
          m_switcher->get_stack ()->get_visible_child_name () == "tiles";
        m_simple_actions ["tileset.edit.tilestyles"]->set_enabled
          (a && b && m_tileset->size () > 0);

        m_simple_actions ["tileset.view.preview-tile"]->set_enabled
          (a && b && m_tileset->size () > 0);

        m_simple_actions ["tileset.file.validate"]->set_enabled (a);
        m_simple_actions ["tileset.file.save-as"]->set_enabled (a);
        m_simple_actions ["tileset.file.save"]->set_enabled
          (m_umgr->undo_empty () == false);

        m_simple_actions["tileset.add-tile"]->set_enabled (a);

        m_simple_actions["tileset.remove-tile"]->set_enabled
          (a && m_store->get_n_items () > 0);

      }

    void update_tileset_panel ()
      {
        bool sensitive;
        disconnect_signals ();
        if (m_tileset)
          {
            sensitive = true;

            m_road_colorbutton->set_rgba (m_tileset->getRoadColor ());
            m_ruin_colorbutton->set_rgba (m_tileset->getRuinColor ());
            m_temple_colorbutton->set_rgba (m_tileset->getTempleColor ());

            Glib::ustring s = "";
            if (m_tileset->getSelector (false)->getName ().empty () == false)
              s = m_tileset->getSelector (false)->getName ();
            m_small_select_filelabel->set_label (s);

            s = "";
            if (m_tileset->getSelector (true)->getName ().empty () == false)
              s = m_tileset->getSelector (true)->getName ();
            m_large_select_filelabel->set_label (s);

            s = "";
            if (m_tileset->getExplosion ()->getName ().empty () == false)
              s = m_tileset->getExplosion ()->getName ();
            m_explosion_filelabel->set_label (s);

            s = "";
            if (m_tileset->getRoad ()->getName ().empty () == false)
              s = m_tileset->getRoad ()->getName ();
            m_roads_filelabel->set_label (s);

            s = "";
            if (m_tileset->getStone ()->getName ().empty () == false)
              s = m_tileset->getStone ()->getName ();
            m_stones_filelabel->set_label (s);

            s = "";
            if (m_tileset->getBridge ()->getName ().empty () == false)
              s = m_tileset->getBridge ()->getName ();
            m_bridges_filelabel->set_label (s);

            s = "";
            if (m_tileset->getFog ()->getName ().empty () == false)
              s = m_tileset->getFog ()->getName ();
            m_fog_filelabel->set_label (s);

            s = "";
            Shield::Color c = Shield::WHITE;
            if (m_tileset->getFlags (c)->getName ().empty () == false)
              s = m_tileset->getFlags (c)->getName ();
            m_white_flags_filelabel->set_label (s);

            s = "";
            c = Shield::GREEN;
            if (m_tileset->getFlags (c)->getName ().empty () == false)
              s = m_tileset->getFlags (c)->getName ();
            m_green_flags_filelabel->set_label (s);

            s = "";
            c = Shield::YELLOW;
            if (m_tileset->getFlags (c)->getName ().empty () == false)
              s = m_tileset->getFlags (c)->getName ();
            m_yellow_flags_filelabel->set_label (s);

            s = "";
            c = Shield::DARK_BLUE;
            if (m_tileset->getFlags (c)->getName ().empty () == false)
              s = m_tileset->getFlags (c)->getName ();
            m_dark_blue_flags_filelabel->set_label (s);

            s = "";
            c = Shield::ORANGE;
            if (m_tileset->getFlags (c)->getName ().empty () == false)
              s = m_tileset->getFlags (c)->getName ();
            m_orange_flags_filelabel->set_label (s);

            s = "";
            c = Shield::LIGHT_BLUE;
            if (m_tileset->getFlags (c)->getName ().empty () == false)
              s = m_tileset->getFlags (c)->getName ();
            m_light_blue_flags_filelabel->set_label (s);

            s = "";
            c = Shield::RED;
            if (m_tileset->getFlags (c)->getName ().empty () == false)
              s = m_tileset->getFlags (c)->getName ();
            m_red_flags_filelabel->set_label (s);

            s = "";
            c = Shield::BLACK;
            if (m_tileset->getFlags (c)->getName ().empty () == false)
              s = m_tileset->getFlags (c)->getName ();
            m_black_flags_filelabel->set_label (s);

            s = "";
            c = Shield::NEUTRAL;
            if (m_tileset->getFlags (c)->getName ().empty () == false)
              s = m_tileset->getFlags (c)->getName ();
            m_neutral_flags_filelabel->set_label (s);

            s = "";
            if (m_tileset->getForestMoveBonus ()->getName ().empty () == false)
              s = m_tileset->getForestMoveBonus ()->getName ();
            m_move_bonus_forest_filelabel->set_label (s);

            s = "";
            if (m_tileset->getHillsMoveBonus ()->getName ().empty () == false)
              s = m_tileset->getHillsMoveBonus ()->getName ();
            m_move_bonus_hills_filelabel->set_label (s);

            s = "";
            if (m_tileset->getWaterMoveBonus ()->getName ().empty () == false)
              s = m_tileset->getWaterMoveBonus ()->getName ();
            m_move_bonus_water_filelabel->set_label (s);

            s = "";
            if (m_tileset->getSwampMoveBonus ()->getName ().empty () == false)
              s = m_tileset->getSwampMoveBonus ()->getName ();
            m_move_bonus_swamp_filelabel->set_label (s);

            s = "";
            if (m_tileset->getMountainsMoveBonus ()->getName ().empty () ==
                false)
              s = m_tileset->getMountainsMoveBonus ()->getName ();
            m_move_bonus_mountains_filelabel->set_label (s);

            s = "";
            if (m_tileset->getAllMoveBonus ()->getName ().empty () == false)
              s = m_tileset->getAllMoveBonus ()->getName ();
            m_move_bonus_fly_filelabel->set_label (s);

            auto t = get_selected_tile ();

            if (t)
              {
                m_name_entry->set_text (t->getName ());
                m_type_combobox->set_active
                  (Tile::getTypeIndexForType (t->getType ()));
                m_pattern_combobox->set_active
                  (t->getSmallTile ()->getPattern ());
                m_moves_spinbutton->set_value (t->getMoves ());
                update_colorbuttons ();
              }
            else
              clear_tile_panel ();
          }
        else
          {
            sensitive = false;
            m_road_colorbutton->set_rgba (Gdk::RGBA ("black"));
            m_ruin_colorbutton->set_rgba (Gdk::RGBA ("black"));
            m_temple_colorbutton->set_rgba (Gdk::RGBA ("black"));
            m_small_select_filelabel->set_label ("");
            m_large_select_filelabel->set_label ("");
            m_explosion_filelabel->set_label ("");
            m_roads_filelabel->set_label ("");
            m_stones_filelabel->set_label ("");
            m_bridges_filelabel->set_label ("");
            m_fog_filelabel->set_label ("");
            m_white_flags_filelabel->set_label ("");
            m_green_flags_filelabel->set_label ("");
            m_dark_blue_flags_filelabel->set_label ("");
            m_orange_flags_filelabel->set_label ("");
            m_light_blue_flags_filelabel->set_label ("");
            m_red_flags_filelabel->set_label ("");
            m_black_flags_filelabel->set_label ("");
            m_neutral_flags_filelabel->set_label ("");
            m_move_bonus_forest_filelabel->set_label ("");
            m_move_bonus_hills_filelabel->set_label ("");
            m_move_bonus_water_filelabel->set_label ("");
            m_move_bonus_swamp_filelabel->set_label ("");
            m_move_bonus_mountains_filelabel->set_label ("");
            m_move_bonus_fly_filelabel->set_label ("");
            clear_tile_panel ();
          }

        auto t = get_selected_tile ();
        bool panel_sensitive = t != NULL;
        m_name_entry->set_sensitive (panel_sensitive);
        m_type_combobox->set_sensitive (panel_sensitive);
        m_pattern_combobox->set_sensitive (panel_sensitive);
        m_moves_spinbutton->set_sensitive (panel_sensitive);
        m_first_colorbutton->set_sensitive (panel_sensitive);
        if (!panel_sensitive)
          m_second_colorbutton->set_sensitive (panel_sensitive);
        else
          m_second_colorbutton->set_sensitive (is_second_color_sensitive ());

        if (!panel_sensitive)
          m_third_colorbutton->set_sensitive (panel_sensitive);
        else
          m_third_colorbutton->set_sensitive (is_third_color_sensitive ());

        m_road_colorbutton->set_sensitive (sensitive);
        m_ruin_colorbutton->set_sensitive (sensitive);
        m_temple_colorbutton->set_sensitive (sensitive);
        m_small_select_button->set_sensitive (sensitive);
        m_small_select_filelabel->set_sensitive (sensitive);
        m_large_select_button->set_sensitive (sensitive);
        m_large_select_filelabel->set_sensitive (sensitive);
        m_explosion_button->set_sensitive (sensitive);
        m_explosion_filelabel->set_sensitive (sensitive);
        m_roads_button->set_sensitive (sensitive);
        m_roads_filelabel->set_sensitive (sensitive);
        m_stones_button->set_sensitive (sensitive);
        m_stones_filelabel->set_sensitive (sensitive);
        m_bridges_button->set_sensitive (sensitive);
        m_bridges_filelabel->set_sensitive (sensitive);
        m_fog_button->set_sensitive (sensitive);
        m_fog_filelabel->set_sensitive (sensitive);
        m_white_flags_button->set_sensitive (sensitive);
        m_green_flags_button->set_sensitive (sensitive);
        m_yellow_flags_button->set_sensitive (sensitive);
        m_light_blue_flags_button->set_sensitive (sensitive);
        m_red_flags_button->set_sensitive (sensitive);
        m_dark_blue_flags_button->set_sensitive (sensitive);
        m_orange_flags_button->set_sensitive (sensitive);
        m_black_flags_button->set_sensitive (sensitive);
        m_neutral_flags_button->set_sensitive (sensitive);
        m_move_bonus_forest_button->set_sensitive (sensitive);
        m_move_bonus_hills_button->set_sensitive (sensitive);
        m_move_bonus_water_button->set_sensitive (sensitive);
        m_move_bonus_swamp_button->set_sensitive (sensitive);
        m_move_bonus_mountains_button->set_sensitive (sensitive);
        m_move_bonus_fly_button->set_sensitive (sensitive);
        m_move_bonus_fly_filelabel->set_sensitive (sensitive);

        connect_signals ();
      }

    void update_window_title ()
      {
        Glib::ustring title = "";
        if (m_tileset_modified || m_new_tileset_needs_saving)
          title += "*";
        if (m_tileset)
          {
            m_notebook->set_current_page (1);
            title += m_tileset->getName ();
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

        auto small_select_image = create_attribute_row
          (_("Small Selector"),
           _("Active stacks with one unit have this animation"),
           "setting_top");
        m_small_select_button = Gtk::make_managed<Gtk::Button> ();
        m_small_select_filelabel =
          Gtk::make_managed<FileLabel> (m_small_select_button);
        small_select_image->append (*m_small_select_button);
        box->append (*small_select_image);

        auto large_select_image = create_attribute_row
          (_("Large Selector"),
           _("Active stacks of more than one unit have this animation"),
           "setting_bottom");
        m_large_select_button = Gtk::make_managed<Gtk::Button> ();
        m_large_select_filelabel =
          Gtk::make_managed<FileLabel> (m_large_select_button);
        large_select_image->append (*m_large_select_button);
        box->append (*large_select_image);

        auto explosion_image = create_attribute_row
          (_("Explosion"),
           _("Fighting in the field and in cities displays this graphic"),
           "setting_top");
        m_explosion_button = Gtk::make_managed<Gtk::Button> ();
        m_explosion_filelabel =
          Gtk::make_managed<FileLabel> (m_explosion_button);
        explosion_image->append (*m_explosion_button);
        box->append (*explosion_image);

        auto roads_image = create_attribute_row
          (_("Roads"),
           _("What the road tiles look like (1x15 tiles)"),
           "setting_middle");
        m_roads_button = Gtk::make_managed<Gtk::Button> ();
        m_roads_filelabel = Gtk::make_managed<FileLabel> (m_roads_button);
        roads_image->append (*m_roads_button);
        box->append (*roads_image);

        auto stones_image = create_attribute_row
          (_("Standing Stones"),
           _("What the standing stone tiles look like (1x89 tiles)"),
           "setting_middle");
        m_stones_button = Gtk::make_managed<Gtk::Button> ();
        m_stones_filelabel = Gtk::make_managed<FileLabel> (m_stones_button);
        stones_image->append (*m_stones_button);
        box->append (*stones_image);

        auto bridges_image = create_attribute_row
          (_("Bridges"),
           _("What the bridges look like (1x4 tiles)"),
           "setting_middle");
        m_bridges_button = Gtk::make_managed<Gtk::Button> ();
        m_bridges_filelabel = Gtk::make_managed<FileLabel> (m_bridges_button);
        bridges_image->append (*m_bridges_button);
        box->append (*bridges_image);

        auto fog_image = create_attribute_row
          (_("Fog"),
           _("What the fog overlay looks like (1x15 tiles)"),
           "setting_bottom");
        m_fog_button = Gtk::make_managed<Gtk::Button> ();
        m_fog_filelabel = Gtk::make_managed<FileLabel> (m_fog_button);
        fog_image->append (*m_fog_button);
        box->append (*fog_image);

        auto white_flags_image = create_attribute_row
          (_("White Flags"),
           _("What the white player flags that denote stack size look like (1x8 tiles)"),
           "setting_top");
        m_white_flags_button = Gtk::make_managed<Gtk::Button> ();
        m_white_flags_filelabel =
          Gtk::make_managed<FileLabel> (m_white_flags_button);
        white_flags_image->append (*m_white_flags_button);
        box->append (*white_flags_image);

        auto green_flags_image = create_attribute_row
          (_("Green Flags"),
           "",
           "setting_middle");
        m_green_flags_button = Gtk::make_managed<Gtk::Button> ();
        m_green_flags_filelabel =
          Gtk::make_managed<FileLabel> (m_green_flags_button);
        green_flags_image->append (*m_green_flags_button);
        box->append (*green_flags_image);

        auto yellow_flags_image = create_attribute_row
          (_("Yellow Flags"),
           "",
           "setting_middle");
        m_yellow_flags_button = Gtk::make_managed<Gtk::Button> ();
        m_yellow_flags_filelabel =
          Gtk::make_managed<FileLabel> (m_yellow_flags_button);
        yellow_flags_image->append (*m_yellow_flags_button);
        box->append (*yellow_flags_image);

        auto dark_blue_flags_image = create_attribute_row
          (_("Dark Blue Flags"),
           "",
           "setting_middle");
        m_dark_blue_flags_button = Gtk::make_managed<Gtk::Button> ();
        m_dark_blue_flags_filelabel =
          Gtk::make_managed<FileLabel> (m_dark_blue_flags_button);
        dark_blue_flags_image->append (*m_dark_blue_flags_button);
        box->append (*dark_blue_flags_image);

        auto orange_flags_image = create_attribute_row
          (_("Orange Flags"),
           "",
           "setting_middle");
        m_orange_flags_button = Gtk::make_managed<Gtk::Button> ();
        m_orange_flags_filelabel =
          Gtk::make_managed<FileLabel> (m_orange_flags_button);
        orange_flags_image->append (*m_orange_flags_button);
        box->append (*orange_flags_image);

        auto light_blue_flags_image = create_attribute_row
          (_("Light Blue Flags"),
           "",
           "setting_middle");
        m_light_blue_flags_button = Gtk::make_managed<Gtk::Button> ();
        m_light_blue_flags_filelabel =
          Gtk::make_managed<FileLabel> (m_light_blue_flags_button);
        light_blue_flags_image->append (*m_light_blue_flags_button);
        box->append (*light_blue_flags_image);

        auto red_flags_image = create_attribute_row
          (_("Red Flags"),
           "",
           "setting_middle");
        m_red_flags_button = Gtk::make_managed<Gtk::Button> ();
        m_red_flags_filelabel =
          Gtk::make_managed<FileLabel> (m_red_flags_button);
        red_flags_image->append (*m_red_flags_button);
        box->append (*red_flags_image);

        auto black_flags_image = create_attribute_row
          (_("Black Flags"),
           "",
           "setting_middle");
        m_black_flags_button = Gtk::make_managed<Gtk::Button> ();
        m_black_flags_filelabel =
          Gtk::make_managed<FileLabel> (m_black_flags_button);
        black_flags_image->append (*m_black_flags_button);
        box->append (*black_flags_image);

        auto neutral_flags_image = create_attribute_row
          (_("Neutral Flags"),
           "",
           "setting_bottom");
        m_neutral_flags_button = Gtk::make_managed<Gtk::Button> ();
        m_neutral_flags_filelabel =
          Gtk::make_managed<FileLabel> (m_neutral_flags_button);
        neutral_flags_image->append (*m_neutral_flags_button);
        box->append (*neutral_flags_image);

        auto move_bonus_forest_image = create_attribute_row
          (_("Move Bonus Forest"),
           _("Denotes that a stack or unit moves well in forest"),
           "setting_top");
        m_move_bonus_forest_button = Gtk::make_managed<Gtk::Button> ();
        m_move_bonus_forest_filelabel =
          Gtk::make_managed<FileLabel> (m_move_bonus_forest_button);
        move_bonus_forest_image->append (*m_move_bonus_forest_button);
        box->append (*move_bonus_forest_image);

        auto move_bonus_hills_image = create_attribute_row
          (_("Move Bonus Hills"),
           _("Denotes that a stack or unit moves well in hills"),
           "setting_middle");
        m_move_bonus_hills_button = Gtk::make_managed<Gtk::Button> ();
        m_move_bonus_hills_filelabel =
          Gtk::make_managed<FileLabel> (m_move_bonus_hills_button);
        move_bonus_hills_image->append (*m_move_bonus_hills_button);
        box->append (*move_bonus_hills_image);

        auto move_bonus_mountains_image = create_attribute_row
          (_("Move Bonus Mountains"),
           _("Denotes that a stack or unit moves well in mountains"),
           "setting_middle");
        m_move_bonus_mountains_button = Gtk::make_managed<Gtk::Button> ();
        m_move_bonus_mountains_filelabel =
          Gtk::make_managed<FileLabel> (m_move_bonus_mountains_button);
        move_bonus_mountains_image->append (*m_move_bonus_mountains_button);
        box->append (*move_bonus_mountains_image);

        auto move_bonus_water_image = create_attribute_row
          (_("Move Bonus Water"),
           _("Denotes that a stack is in the water"),
           "setting_middle");
        m_move_bonus_water_button = Gtk::make_managed<Gtk::Button> ();
        m_move_bonus_water_filelabel =
          Gtk::make_managed<FileLabel> (m_move_bonus_water_button);
        move_bonus_water_image->append (*m_move_bonus_water_button);
        box->append (*move_bonus_water_image);

        auto move_bonus_swamp_image = create_attribute_row
          (_("Move Bonus Swamp"),
           _("Denotes that a stack is in swamp"),
           "setting_middle");
        m_move_bonus_swamp_button = Gtk::make_managed<Gtk::Button> ();
        m_move_bonus_swamp_filelabel =
          Gtk::make_managed<FileLabel> (m_move_bonus_swamp_button);
        move_bonus_swamp_image->append (*m_move_bonus_swamp_button);
        box->append (*move_bonus_swamp_image);

        auto move_bonus_fly_image = create_attribute_row
          (_("Move Bonus Flight"),
           _("Denotes that a stack or unit moves well on all tiles"),
           "setting_bottom");
        m_move_bonus_fly_button = Gtk::make_managed<Gtk::Button> ();
        m_move_bonus_fly_filelabel =
          Gtk::make_managed<FileLabel> (m_move_bonus_fly_button);
        move_bonus_fly_image->append (*m_move_bonus_fly_button);
        box->append (*move_bonus_fly_image);

        scrolled_window->set_child (*box);
        return scrolled_window;
      }

    Gtk::Box * populate_mini_map_colors ()
      {
        auto box = Gtk::make_managed<Gtk::Box> (Gtk::Orientation::VERTICAL);
        box->set_margin (6);

        auto road_color = create_attribute_row
          (_("Road Color"),
           _("What color the road lines are drawn in on the mini map"),
           "setting_top");
        m_road_colorbutton = Gtk::make_managed<Gtk::ColorButton> ();
        road_color->append (*m_road_colorbutton);
        box->append (*road_color);

        auto ruin_color = create_attribute_row
          (_("Ruin Color"),
           _("What color the ruin dots are drawn in on the mini map"),
           "setting_middle");
        m_ruin_colorbutton = Gtk::make_managed<Gtk::ColorButton> ();
        ruin_color->append (*m_ruin_colorbutton);
        box->append (*ruin_color);

        auto temple_color = create_attribute_row
          (_("Temple Color"),
           _("What color the temple dots are drawn in on the mini map"),
           "setting_bottom");
        m_temple_colorbutton = Gtk::make_managed<Gtk::ColorButton> ();
        temple_color->append (*m_temple_colorbutton);
        box->append (*temple_color);

        return box;
      }

    Gtk::ScrolledWindow* populate_tile_panel ()
      {
        m_scrolled_window = Gtk::make_managed<Gtk::ScrolledWindow> ();
        auto box = Gtk::make_managed<Gtk::Box> (Gtk::Orientation::VERTICAL);
        box->set_margin (6);
        m_scrolled_window->set_child (*box);

        auto name = create_attribute_row
          (_("Name"),
           _("The name of the tile (not displayed in the game)"),
           "setting_top");
        m_name_entry = Gtk::make_managed<Gtk::Entry> ();
        m_name_entry->set_placeholder_text (_("Name..."));
        name->append (*m_name_entry);
        box->append (*name);

        auto type = create_attribute_row
          (_("Type"),
           _("What kind of terrain this tile acts as"),
           "setting_middle");
        m_type_combobox = Gtk::make_managed<LwCombo> ();
        m_type_combobox->append (Tile::tileTypeToFriendlyName (Tile::GRASS));
        m_type_combobox->append (Tile::tileTypeToFriendlyName (Tile::WATER));
        m_type_combobox->append (Tile::tileTypeToFriendlyName (Tile::FOREST));
        m_type_combobox->append (Tile::tileTypeToFriendlyName (Tile::HILLS));
        m_type_combobox->append (Tile::tileTypeToFriendlyName (Tile::MOUNTAIN));
        m_type_combobox->append (Tile::tileTypeToFriendlyName (Tile::SWAMP));
        m_type_combobox->set_hexpand (false);
        type->append (*m_type_combobox);
        box->append (*type);

        auto pattern = create_attribute_row
          (_("Pattern"),
           _("The pixel pattern this tile has on the mini map"),
           "setting_middle");
        m_pattern_combobox = Gtk::make_managed<LwCombo> ();
        m_pattern_combobox->append (_("Solid"));
        m_pattern_combobox->append (_("Stippled"));
        m_pattern_combobox->append (_("Randomized"));
        m_pattern_combobox->append (_("Sunken"));
        m_pattern_combobox->append (_("Tablecloth"));
        m_pattern_combobox->append (_("Diagonal"));
        m_pattern_combobox->append (_("Crosshatched"));
        m_pattern_combobox->append (_("Sunken Striped"));
        m_pattern_combobox->append (_("Sunken Radial"));
        m_pattern_combobox->set_hexpand (false);
        pattern->append (*m_pattern_combobox);
        box->append (*pattern);

        auto moves = create_attribute_row
          (_("Moves"),
           _("The movement points required to cross this tile"),
           "setting_bottom");
        m_moves_spinbutton = Gtk::make_managed<Gtk::SpinButton> ();
        guint32 min = Tile::min_moves;
        guint32 max = Tile::max_moves;
        m_moves_spinbutton->set_adjustment
          (Gtk::Adjustment::create (min, min, max, 1, 10, 0));
        moves->append (*m_moves_spinbutton);
        box->append (*moves);

        auto first_color = create_attribute_row
          (_("First Color"),
           _("The primary color for drawing this tile on the mini map"),
           "setting_top");
        m_first_colorbutton = Gtk::make_managed<Gtk::ColorButton> ();
        first_color->append (*m_first_colorbutton);
        box->append (*first_color);

        auto second_color = create_attribute_row
          (_("Second Color"),
           _("The secondary color for drawing this tile on the mini map"),
           "setting_middle");
        m_second_colorbutton = Gtk::make_managed<Gtk::ColorButton> ();
        second_color->append (*m_second_colorbutton);
        box->append (*second_color);

        auto third_color = create_attribute_row
          (_("Third Color"),
           _("The tertiary color for drawing this tile on the mini map"),
           "setting_bottom");
        m_third_colorbutton = Gtk::make_managed<Gtk::ColorButton> ();
        third_color->append (*m_third_colorbutton);
        box->append (*third_color);

        auto tilestyles = create_attribute_row
          (_("Tile Styles"),
           _("The different images this tile appears as on the big map"),
           "setting_lone");
        m_tilestyles_button = Gtk::make_managed<Gtk::Button> (_("Edit..."));
        m_tilestyles_button->set_action_name ("lw.tileset.edit.tilestyles");
        tilestyles->append (*m_tilestyles_button);
        box->append (*tilestyles);

        return m_scrolled_window;
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
        add_button->set_action_name ("lw.tileset.add-tile");
        add_button->set_icon_name ("list-add-symbolic");
        button_box->append (*add_button);

        auto remove_button = Gtk::make_managed<Gtk::Button> ();
        remove_button->set_action_name ("lw.tileset.remove-tile");
        remove_button->set_icon_name ("list-remove-symbolic");
        button_box->append (*remove_button);

        button_box->set_spacing (6);

        pane->append (*button_box);
        return pane;
      }

    Gtk::Box* populate_tiles ()
      {
        auto panel = populate_tile_panel ();

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

        auto tiles = populate_tiles ();
        stack->add (*tiles, "tiles", _("Tiles"));

        auto misc_images = populate_misc_images ();
        stack->add (*misc_images, "misc. images", _("Misc. Images"));

        auto mini_map_colors = populate_mini_map_colors ();
        stack->add (*mini_map_colors, "mini map colors", _("Mini Map Colors"));

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
        auto action = dynamic_cast<TileSetUndoAction*>(a2);
        UndoAction *out = NULL;

        switch (action->get_type ())
          {
          case TileSetUndoAction::CHANGE_PROPERTIES:
              {
                auto a = dynamic_cast<TileSetUndoAction_Properties*>(action);
                out = new TileSetUndoAction_Properties
                  (m_tileset->getName (),
                   m_tileset->getInfo (),
                   m_tileset->getCopyright (),
                   m_tileset->getLicense (),
                   m_tileset->getTileSize ());
                m_tileset->setName (a->get_name ());
                m_tileset->setInfo (a->get_description ());
                m_tileset->setCopyright (a->get_copyright ());
                m_tileset->setLicense (a->get_license ());
                m_tileset->setTileSize (a->get_tile_size ());
              }
            break;

          case TileSetUndoAction::NAME:
              {
                auto a = dynamic_cast<TileSetUndoAction_Name*>(action);
                out = new TileSetUndoAction_Name
                  (a->get_index (), get_tile_by_index (a)->getName (), m_umgr,
                   m_name_entry);
                get_tile_by_index (a)->setName (a->get_name ());
                auto item = m_store->get_item (a->get_index ());
                if (item)
                  {
                    auto row = std::dynamic_pointer_cast<TileRow>(item);
                    row->changed ();
                  }
              }
            break;

          case TileSetUndoAction::TYPE:
              {
                auto a = dynamic_cast<TileSetUndoAction_Type*>(action);
                out = new TileSetUndoAction_Type
                  (a->get_index (), get_tile_by_index (a));
                auto t = get_tile_by_index (a);
                t->setType (a->get_tile ()->getType ());
                t->getSmallTile ()->setPattern
                  (a->get_tile ()->getSmallTile ()->getPattern ());
                t->setMoves (a->get_tile ()->getMoves ());
                t->getSmallTile ()->setColor
                  (a->get_tile ()->getSmallTile ()->getColor ());
                t->getSmallTile ()->setSecondColor
                  (a->get_tile ()->getSmallTile ()->getSecondColor ());
                t->getSmallTile ()->setThirdColor
                  (a->get_tile ()->getSmallTile ()->getThirdColor ());
              }
            break;

          case TileSetUndoAction::PATTERN:
              {
                auto a = dynamic_cast<TileSetUndoAction_Pattern*>(action);
                out = new TileSetUndoAction_Pattern
                  (a->get_index (), 
                   get_tile_by_index (a)->getSmallTile()->getPattern ());
                get_tile_by_index (a)->getSmallTile ()->setPattern
                  (a->get_pattern ());
              }
            break;

          case TileSetUndoAction::MOVES:
              {
                auto a = dynamic_cast<TileSetUndoAction_Moves*>(action);
                out = new TileSetUndoAction_Moves
                  (a->get_index (), 
                   get_tile_by_index (a)->getMoves ());
                get_tile_by_index (a)->setMoves (a->get_moves ());
              }
            break;

          case TileSetUndoAction::COLOR:
              {
                auto a = dynamic_cast<TileSetUndoAction_Color*>(action);

                Gdk::RGBA c;
                switch (a->get_color_number ())
                  {
                  case 0:
                    c = get_tile_by_index (a)->getSmallTile ()->getColor ();
                    break;

                  case 1:
                    c = get_tile_by_index (a)->getSmallTile ()->getSecondColor ();
                    break;

                  case 2:
                    c = get_tile_by_index (a)->getSmallTile ()->getThirdColor ();
                    break;
                  }
                out = new TileSetUndoAction_Color
                  (a->get_index (), a->get_color_number (), c);
                c = a->get_color ();
                switch (a->get_color_number ())
                  {
                  case 0:
                    get_tile_by_index (a)->getSmallTile ()->setColor (c);
                    break;

                  case 1:
                    get_tile_by_index (a)->getSmallTile ()->setSecondColor (c);
                    break;

                  case 2:
                    get_tile_by_index (a)->getSmallTile ()->setThirdColor (c);
                    break;
                  }
              }
            break;

          case TileSetUndoAction::ADD_IMAGE:
              {
                auto a = dynamic_cast<TileSetUndoAction_AddImage*>(action);
                out = new TileSetUndoAction_AddImage (m_tileset);
                reload_tileset (a);
              }
            break;

          case TileSetUndoAction::CLEAR_IMAGE:
              {
                auto a = dynamic_cast<TileSetUndoAction_ClearImage*>(action);
                out = new TileSetUndoAction_ClearImage (m_tileset);
                reload_tileset (a);
              }
            break;

          case TileSetUndoAction::ADD_TILE:
              {
                auto a = dynamic_cast<TileSetUndoAction_AddTile*>(action);
                out = new TileSetUndoAction_AddTile (m_tileset);
                reload_tileset (a);
              }
            break;

          case TileSetUndoAction::REMOVE_TILE:
              {
                auto a = dynamic_cast<TileSetUndoAction_RemoveTile*>(action);
                out = new TileSetUndoAction_RemoveTile (m_tileset);
                reload_tileset (a);
              }
            break;

          case TileSetUndoAction::TILESTYLES:
              {
                auto a = dynamic_cast<TileSetUndoAction_TileStyles *>(action);
                out = new TileSetUndoAction_TileStyles (m_tileset);
                reload_tileset (a);
              }
            break;

          case TileSetUndoAction::BUILDING_COLORS:
              {
                auto a =
                  dynamic_cast<TileSetUndoAction_BuildingColors *>(action);
                out = new TileSetUndoAction_BuildingColors (m_tileset);
                reload_tileset (a);
              }
            break;

          }
        return out;
      }

    void reload_tileset (TileSetUndoAction_Save *action)
      {
        Glib::ustring olddir = m_tileset->getDirectory ();
        Glib::ustring oldname =
          File::get_basename (m_tileset->getConfigurationFile (true));
        Glib::ustring oldext = m_tileset->getExtension ();

        m_tileset->clean_tmp_dir ();
        delete m_tileset;
        m_tileset = new Tileset (*(action->get_tileset ()));
        m_tileset->setLoadTemporaryFile ();

        m_tileset->setDirectory (olddir);
        m_tileset->setBaseName (oldname);
        m_tileset->setExtension (oldext);
        fill_treeview ();
        update ();
      }

    void check_discard (Glib::ustring msg, sigc::slot<void(bool)> after)
      {
        if (m_tileset_modified || m_new_tileset_needs_saving)
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

                          if (m_tileset->getDirectory ().empty () == false)
                            {
                              save_current_tileset_file_as
                                ([this, after] (bool saved)
                                 {
                                   after (saved);
                                 });
                            }
                          else
                            {
                              save_current_tileset_file
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
        Glib::ustring name = m_tileset->getName ();
        Glib::ustring newname = "";
        if (existing)
          {
            Tileset *oldtileset =
              Tilesetlist::instance ()->get (m_tileset->getId ());
            if (oldtileset && oldtileset->getName () != name)
              newname = oldtileset->getName ();
          }
        guint32 num = 0;
        Glib::ustring n = String::utrim (String::strip_trailing_numbers (name));
        if (n == "")
          n = _("Untitled");
        if (newname.empty () == true)
          newname =
            Tilesetlist::instance ()->findFreeName (n, 100, num,
                                                    m_tileset->getTileSize ());
        if (name == "")
          {
            if (newname.empty () == true)
              {
                Glib::ustring msg =
                  _("The tile set has an invalid name.\n"
                    "Change it and save again.");
                auto d = LwDialog::alert (msg);
                d->choose
                  (*this,
                   [this, d, after] (auto result)
                   {
                     d->choose_finish (result);
                     on_edit_tileset_info_activated ();
                     after (false);
                   });
                return;
              }
            else
              {
                Glib::ustring msg =
                  String::ucompose (_("The tile set has an invalid name.\n"
                                      "Change it to '%1'?"), newname);
                auto d = LwDialog::alert_yn (msg);
                d->choose
                  (*this,
                   [this, d, existing, newname, after] (auto result)
                   {
                     bool change = d->choose_finish (result) == 1;
                     if (change)
                       {
                         m_tileset->setName (newname);
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
          Tilesetlist::instance ()->lookupConfigurationFileByName (m_tileset);
        if (file == "")
          return after (true);

        Glib::ustring cfgfile = m_tileset->getConfigurationFile (true);

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
                  _("The tile set has the same name as another one.\n"
                    "Change it and save again.");
                auto d = LwDialog::alert (msg);
                d->choose
                  (*this,
                   [this, d, after] (auto result)
                   {
                     d->choose_finish (result);
                     on_edit_tileset_info_activated ();
                     after (false);
                   });
              }
            else
              {
                Glib::ustring msg =
                  String::ucompose (_("The tile set has the same name as "
                                      "another one.\nChange it to '%1' "
                                      "instead?"), newname);
                auto d = LwDialog::alert_yn (msg);
                d->choose
                  (*this,
                   [this, d, newname, after] (auto result)
                   {
                     bool change = d->choose_finish (result) == 1;
                     if (change)
                       m_tileset->setName (newname);
                     after (change);
                   });
              }
          }
      }

    bool is_valid_name ()
      {
        Glib::ustring file =
          Tilesetlist::instance ()->lookupConfigurationFileByName (m_tileset);
        if (file == "")
          return true;
        if (file == m_tileset->getConfigurationFile (true))
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

             if (m_tileset->validate () == false)
               {
                 if (existing &&
                     GameMap::instance ()->getTilesetId () == m_tileset->getId ())
                   {
                     Glib::ustring errmsg =
                       _("The tile set is invalid, and is also the current "
                         "working one.");
                     Glib::ustring msg = _("The tile set could not be saved.");
                     Glib::ustring detail =
                       m_current_save_filename + "\n" + errmsg;
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
                       _("The tile set is invalid.  Do you want to proceed?");
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
              if (m_umgr->undo_empty () && !m_new_tileset_needs_saving)
                m_tileset_modified = false;
              update ();
            }));

        actions->add_action
          ("redo",
           ([this] ()
            {
              m_tileset_modified = true;
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

    void on_edit_tileset_info_activated ()
      {
        auto d = LwDialog::build<TileSetInfoDialog> (this);
        d->setup (m_tileset);
        d->signal_response ().connect
          ([this, d] (Gtk::ResponseType resp)
           {
             if (d->is_changed () &&
                 resp == Gtk::ResponseType::ACCEPT)
               {
                 TileSetUndoAction_Properties *action =
                   new TileSetUndoAction_Properties
                   (m_tileset->getName (),
                    m_tileset->getInfo (),
                    m_tileset->getCopyright (),
                    m_tileset->getLicense (),
                    m_tileset->getTileSize ());
                 m_umgr->add (action);
                 m_tileset->setName (d->get_name ());
                 m_tileset->setInfo (d->get_description ());
                 m_tileset->setCopyright (d->get_copyright ());
                 m_tileset->setLicense (d->get_license ());
                 m_tileset->setTileSize (d->get_tile_size ());
                 m_tileset_modified = true;
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
                 save_current_tileset_file_as
                   ([this] (bool saved)
                    {
                      (void) saved;
                    });
               }
           });
      }

    void save_current_tileset_file_as (sigc::slot<void(bool)> after)
      {
        LwDialog::save
          (*this, _("Choose a Name"),
           File::sanify (m_tileset->getName ()), FileFilter::TILESET,
           [this, after] (std::string path)
           {
             Glib::ustring old_filename = m_current_save_filename;
             guint32 old_id = m_tileset->getId ();
             m_tileset->setId (Tilesetlist::getNextAvailableId (old_id));

             save_current_tileset_file
               (path,
                [this, after, path, old_filename, old_id] (bool saved)
                {
                  if (saved == false)
                    {
                      m_current_save_filename = old_filename;
                      m_tileset->setId (old_id);
                    }
                  else
                    {
                      m_tileset_modified = false;
                      m_new_tileset_needs_saving = false;
                      m_tileset->created (path);
                      Glib::ustring dir =
                        File::add_slash_if_necessary (File::get_dirname (path));
                      if (dir == File::get_tileset_dir () ||
                          dir == File::get_user_tileset_dir ())
                        {
                          //if we saved it to a standard place, update the list
                          Tilesetlist::instance ()->add
                            (Tileset::copy (m_tileset), path);
                          m_tileset_saved.emit (m_tileset->getId ());
                        }
                      update_tileset_panel ();
                      update_window_title ();
                    }
                  after (saved);
                });
           });
      }

    void save_current_tileset_file (Glib::ustring filename, sigc::slot<void(bool)> after)
      {
        m_current_save_filename = filename;
        if (m_current_save_filename.empty ())
          m_current_save_filename = m_tileset->getConfigurationFile (true);

        bool ok =
          m_tileset->save (m_current_save_filename, Tileset::file_extension);
        if (ok)
          {
            if (Tilesetlist::instance ()->reload (m_tileset->getId ()))
              update_tileset_panel ();
            m_new_tileset_needs_saving = false;
            m_tileset_modified = false;
            update_window_title ();
            m_tileset_saved.emit (m_tileset->getId ());
            after (true);
          }
        else
          {
            Glib::ustring errmsg = Glib::strerror (errno);
            Glib::ustring msg = _("The tile set could not be saved.");

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
        if (m_tileset_modified || m_new_tileset_needs_saving)
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

                          if (m_tileset->getDirectory ().empty () == false)
                            {
                              save_current_tileset_file_as
                                ([this, after] (bool saved)
                                 {
                                   after (saved);
                                 });
                            }
                          else
                            {
                              save_current_tileset_file
                                ("",
                                 [this, after] (bool saved)
                                 {
                                   after (saved);
                                 });
                            }
                        });
                     break;

                   case 1: //close, exit
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

    void setup_treeview ()
      {
        m_treeview->set_vexpand ();
        m_store = Gio::ListStore<TileRow>::create ();
        m_selection_model = Gtk::SingleSelection::create (m_store);
        m_treeview->set_model (m_selection_model);

        LwColumn::setup_name_column<TileRow>
          (m_treeview, true, _("Tiles"),
           [] (const auto& row)
           {
             return row->m_tile->getName ();
           });

        m_selection_model->signal_selection_changed ().connect
          ([this] (const guint &, const guint &)
           {
             scroll_tile_to_top ();
             update_tileset_panel ();
             update_actions ();
           });

      }

    void fill_treeview ()
      {
        m_store->remove_all ();
        if (m_tileset)
          {
            for (auto t : *m_tileset)
              m_store->append (TileRow::create (t));
          }

        m_selection_model->set_selected (0);
      }

    Tile* get_tile_by_index (TileSetUndoAction_TileIndex *i)
      {
        auto item = m_store->get_item (i->get_index ());
        if (!item)
          return NULL;
        auto row = std::dynamic_pointer_cast<TileRow>(item);
        return row->m_tile;
      }

    void scroll_tile_to_top ()
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
                 m_simple_actions["tileset.file.quit"]->activate ();
                 return true;
               }
             return false;
           }, false);
        add_controller (controller);

        signal_close_request ().connect
          ([this] () -> bool
           {
             m_simple_actions["tileset.file.quit"]->activate ();
             return true;
           }, false);
      }
                  
    void clear_tile_panel ()
      {
        m_name_entry->set_text ("");
        m_type_combobox->set_active (0);
        m_pattern_combobox->set_active (0);
        m_first_colorbutton->set_rgba (Gdk::RGBA ("black"));
        m_second_colorbutton->set_rgba (Gdk::RGBA ("black"));
        m_third_colorbutton->set_rgba (Gdk::RGBA ("black"));
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
                 auto action = new TileSetUndoAction_AddImage (m_tileset);
                 Glib::ustring newname = "";
                 Glib::ustring err = "";
                 bool success = d->install_file (m_tileset, im,
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
                 auto action = new TileSetUndoAction_ClearImage (m_tileset);
                 auto name = im->getName ();
                 if (d->uninstall_file (m_tileset, im, err))
                   {
                     m_umgr->add (action);
                     update ();
                     cleared = true;
                     newfile = "";
                     delete d;
                     m_tileset->uninstantiateSameNamedImages (name);
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
           [this, im] (bool err, bool cleared, Glib::ustring f)
           {
             if (err)
               return;
             if (cleared)
               im->clear ();
             else
               {
                 if (f != "")
                   {
                     im->load (m_tileset, f);
                     im->instantiateImages ();
                   }
               }
             update_tileset_panel ();
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
                 auto action = new TileSetUndoAction_AddImage (m_tileset);
                 Glib::ustring newname = "";
                 Glib::ustring err = "";
                 bool success = d->install_file (m_tileset, im,
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
                 auto action = new TileSetUndoAction_ClearImage (m_tileset);
                 auto name = im->getName ();
                 if (d->uninstall_file (m_tileset, im, err))
                   {
                     m_umgr->add (action);
                     update ();
                     cleared = true;
                     newfile = "";
                     delete d;
                     m_tileset->uninstantiateSameNamedImages (name);
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
           [this, im ](bool err, bool cleared, Glib::ustring f)
           {
             if (err)
               return;
             if (cleared)
               im->clear ();
             else
               {
                 if (f != "")
                   {
                     im->load (m_tileset, f);
                     im->instantiateImages ();
                   }
               }
             update_tileset_panel ();
           });
      }

    void on_flag_change_clicked (Glib::ustring msg, TarFileMaskedImage *im, int lone)
      {
        Glib::ustring oldname = im->getName ();
        change_image
          (msg, im, lone,
           [this, im, lone, oldname] (bool err, bool cleared, Glib::ustring f)
           {
             if (err)
               return;
             if (cleared)
               im->clear ();
             else
               {
                 if (f != "")
                   {
                     im->load (m_tileset, f);
                     im->instantiateImages ();
                   }
               }

             if (oldname.empty () == false)
               {
                 auto t = m_tileset;
                 for (guint32 i = Shield::WHITE; i <= Shield::NEUTRAL; i++)
                   {
                     if (lone >= 0 && i == (guint32)lone)
                       continue;
                     auto c = Shield::Color (i);
                     if (oldname == t->getFlags (c)->getName ())
                       t->getFlags (c)->clear ();
                   }
               }
             update_tileset_panel ();
           });
      }


    bool is_second_color_sensitive ()
      {
        auto tile = get_selected_tile ();
        if (!tile)
          return false;
        auto t = tile->getSmallTile ();
        switch (t->getPattern ())
          {
          case SmallTile::SOLID:
            return false;

          case SmallTile::STIPPLED: case SmallTile::SUNKEN:
            return true;

          case SmallTile::RANDOMIZED: case SmallTile::TABLECLOTH:
          case SmallTile::DIAGONAL: case SmallTile::CROSSHATCH:
          case SmallTile::SUNKEN_STRIPED: case SmallTile::SUNKEN_RADIAL:
            return true;
          }
        return false;
      }

    bool is_third_color_sensitive ()
      {
        auto tile = get_selected_tile ();
        if (!tile)
          return false;
        auto t = tile->getSmallTile ();
        switch (t->getPattern ())
          {
          case SmallTile::SOLID:
            return false;

          case SmallTile::STIPPLED: case SmallTile::SUNKEN:
            return false;

          case SmallTile::RANDOMIZED: case SmallTile::TABLECLOTH:
          case SmallTile::DIAGONAL: case SmallTile::CROSSHATCH:
          case SmallTile::SUNKEN_STRIPED: case SmallTile::SUNKEN_RADIAL:
            return true;
          }
        return false;
      }

    void update_colorbuttons ()
      {
        auto t = get_selected_tile ()->getSmallTile ();
        switch (t->getPattern ())
          {
          case SmallTile::SOLID:
            m_first_colorbutton->set_rgba (t->getColor ());
            m_second_colorbutton->set_rgba (Gdk::RGBA ("black"));
            m_third_colorbutton->set_rgba (Gdk::RGBA("black"));
            break;

          case SmallTile::STIPPLED: case SmallTile::SUNKEN:
            m_first_colorbutton->set_rgba (t->getColor ());
            m_second_colorbutton->set_rgba (t->getSecondColor ());
            m_third_colorbutton->set_rgba (Gdk::RGBA ("black"));
            break;

          case SmallTile::RANDOMIZED: case SmallTile::TABLECLOTH:
          case SmallTile::DIAGONAL: case SmallTile::CROSSHATCH:
          case SmallTile::SUNKEN_STRIPED: case SmallTile::SUNKEN_RADIAL:
            m_first_colorbutton->set_rgba (t->getColor ());
            m_second_colorbutton->set_rgba (t->getSecondColor ());
            m_third_colorbutton->set_rgba (t->getThirdColor ());
            break;
          }
      }

    void set_default_values_by_type ()
      {
        disconnect_signals ();
        auto t = get_selected_tile ();
        switch (t->getType ())
          {
          case Tile::GRASS:
            t->getSmallTile ()->setPattern (SmallTile::SOLID);
            t->setMoves (2);
            t->getSmallTile ()->setColor (Gdk::RGBA ("#50AC1C"));
            t->getSmallTile ()->setSecondColor (Gdk::RGBA ("black"));
            t->getSmallTile ()->setThirdColor (Gdk::RGBA ("black"));
            break;

          case Tile::WATER:
            t->getSmallTile ()->setPattern (SmallTile::SUNKEN_RADIAL);
            t->setMoves (2);
            t->getSmallTile ()->setColor (Gdk::RGBA ("#63C8FC"));
            t->getSmallTile ()->setSecondColor (Gdk::RGBA ("#0068DF"));
            t->getSmallTile ()->setThirdColor (Gdk::RGBA ("#295BE8"));
            break;

          case Tile::FOREST:
            t->getSmallTile ()->setPattern (SmallTile::STIPPLED);
            t->setMoves (3);
            t->getSmallTile ()->setColor (Gdk::RGBA ("#008C00"));
            t->getSmallTile ()->setSecondColor (Gdk::RGBA ("#005800"));
            t->getSmallTile ()->setThirdColor (Gdk::RGBA ("black"));
            break;

          case Tile::HILLS:
            t->getSmallTile ()->setPattern (SmallTile::SOLID);
            t->setMoves (4);
            t->getSmallTile ()->setColor (Gdk::RGBA ("#008C00"));
            t->getSmallTile ()->setSecondColor (Gdk::RGBA ("black"));
            t->getSmallTile ()->setThirdColor (Gdk::RGBA ("black"));
            break;

          case Tile::MOUNTAIN:
            t->getSmallTile ()->setPattern (SmallTile::RANDOMIZED);
            t->setMoves (6);
            t->getSmallTile ()->setColor (Gdk::RGBA ("#909090"));
            t->getSmallTile ()->setSecondColor (Gdk::RGBA ("#505050"));
            t->getSmallTile ()->setThirdColor (Gdk::RGBA ("#707070"));
            break;

          case Tile::SWAMP:
            t->getSmallTile ()->setPattern (SmallTile::TABLECLOTH);
            m_pattern_combobox->set_active (4);
            m_moves_spinbutton->set_value (8);
            t->getSmallTile ()->setColor (Gdk::RGBA ("#005CD0"));
            t->getSmallTile ()->setSecondColor (Gdk::RGBA ("#2CB8FC"));
            t->getSmallTile ()->setThirdColor (Gdk::RGBA ("#50AC1C"));
            break;
         
          }
        connect_signals ();
      }
};
#endif

