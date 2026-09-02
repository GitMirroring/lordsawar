//  Copyright (C) 2007 Ole Laursen
//  Copyright (C) 2007, 2008, 2009, 2010, 2012, 2014, 2015, 2016, 2017, 2020,
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

#include "scenario-builder-window.h"
#include "lw.h"
#include "army-set.h"
#include "city-set.h"
#include "shield-set.h"
#include "tile-set.h"
#include "counter.h"
#include "ai-dummy.h"
#include "editor-load-window.h"
#include "map-info-dialog.h"
#include "editor-map-widget.h"
#include "item-list-dialog.h"
#include "battle-calculator-dialog.h"
#include "new-map-dialog.h"
#include "bridge.h"
#include "map-generator.h"
#include "switch-sets-dialog.h"
#include "armyset-window.h"
#include "cityset-window.h"
#include "shieldset-window.h"
#include "tileset-window.h"
#include "players-dialog.h"
#include "pointer-size-menu-button.h"
#include "shield-menu-button.h"
#include "smallmap-editor-window.h"
#include "backpack-editor-dialog.h"
#include "planted-standard-editor-dialog.h"
#include "select-tilestyle-popover.h"
#include "select-stone-popover.h"
#include "select-road-popover.h"
#include "signpost-editor-dialog.h"
#include "temple-editor-dialog.h"
#include "media-dialog.h"
#include "fight-order-editor-dialog.h"
#include "ruin-editor-dialog.h"
#include "stack-editor-dialog.h"
#include "city-editor-dialog.h"
#include "reward-list-editor-dialog.h"
#include "validation-dialog.h"
#include "randomize-dialog.h"
#include "scenario-list.h"
#include "create-scenario.h"
#include "vectored-unit-list.h"
#include "rnd.h"
#include "small-map.h"
#include "army-prod-base.h"
#include "stack-tile.h"
#include "undo-mgr.h"
#include "create-scenario-randomize.h"
#include "about-dialog.h"

ScenarioBuilderWindow::ScenarioBuilderWindow ()
        : m_actions (Gio::SimpleActionGroup::create ())
{
  set_size_request (1050, -1);
  m_scenario = NULL;
  m_scenario_modified = false;
  m_new_scenario_needs_saving = false;
  m_scenario = NULL;
  m_create_scenario_names = NULL;
  m_pointer = EditorMapWidget::UNKNOWN;

  m_umgr = new UndoMgr (UndoMgr::DELAY, UndoMgr::LIMIT);
  m_umgr->signal_execute ().connect
    (sigc::mem_fun (*this, &ScenarioBuilderWindow::execute_action));
  m_umgr->signal_added_undo ().connect
    ([this] ()
     {
       m_scenario_modified = true;
       update_window_title ();
       update_actions ();
     });
}

ScenarioBuilderWindow::~ScenarioBuilderWindow ()
{
  delete m_scenario;
  delete m_create_scenario_names;
  delete m_smallmap;
  delete m_bigmap;
  m_dark_style_handler.disconnect ();
  disconnect_signals ();
  disconnect_action_signals ();
  disconnect_terrain_button_signals ();
  for (auto a : m_battle_calculator_attackers)
    delete a;
  for (auto a : m_battle_calculator_defenders)
    delete a;
  delete m_umgr;
}

void ScenarioBuilderWindow::setup (std::string filename)
{
  setup ();
  m_load_filename = filename;
  if (filename == "")
    create_blank_scenario ();
  else
    {
      load_scenario
        (filename,
         [this] (bool loaded, Glib::ustring err)
         {
           if (!loaded)
             {
               Glib::ustring msg =
                 _("Couldn't load scenario");
               auto dialog = LwDialog::alert (msg, err);
               dialog->choose
                 (*this,
                  [this, dialog] (auto result)
                  {
                    dialog->choose_finish (result);
                    create_blank_scenario ();
                    return;
                  });
             }
           else
             {
               update ();
             }
         });
    }
}

void ScenarioBuilderWindow::load_scenario (std::string filename,
                                           sigc::slot<void(bool, Glib::ustring)> after)
{
  if (m_scenario)
    delete m_scenario;
  auto w = Gtk::make_managed<EditorLoadWindow> ();
  w->setup ();
  w->present ();
  w->set_modal (true);
  w->set_transient_for (*this);

  m_load_tick = GameScenario::load_tick.connect
    ([this, w] (double fraction)
     {
       w->set_fraction (fraction);
       Lw::do_events ();
     });

  Glib::signal_idle ().connect
    ([this, w, after, filename]()
     {
       bool broken;
       Glib::ustring err;
       m_scenario = new GameScenario (filename, broken, err);
       if (!broken)
         {
           m_shield_menu_button->reload ();
           add_terrain_buttons ();
           m_pointer_button->set_active (true);
           m_smallmap->resize ();
           m_current_save_filename = m_load_filename;
         }
       w->hide ();
       m_load_tick.disconnect ();
       after (broken == false, err);
       return false;
     });
}

void ScenarioBuilderWindow::clear_save_file_of_scenario_specific_data ()
{
  for (auto p : *Playerlist::instance ())
    {
      p->clearActionlist ();
      p->clearHistorylist ();
      p->clearFogMap ();
      p->revive ();
    }
  //group all stacks, because the editor doesn't have a way to represent
  //many stacks on the same tile.
  for (auto p : *Playerlist::instance ())
    for (auto pos : p->getStacklist ()->getPositions ())
      {
        Maptile *mtile = GameMap::instance ()->getTile (pos);
        std::vector<Stack*> stacks = mtile->getStacks ()->getStacks ();
        if (stacks.size () > 1)
          GameMap::getStacks (pos)->group ();
      }
}

void ScenarioBuilderWindow::setup ()
{
  m_menu_button =
    Gtk::make_managed<ScenarioBuilderMenuButton>(*this, m_actions);
  m_menu_button->m_signal_action_added.connect
    ([this] (Glib::RefPtr<Gio::SimpleAction> action)
     {
       m_simple_actions[action->get_name ()] = action;
     });

  m_menu_button->setup ();

  auto idx = std::string (LW_APP_ID).rfind ('.');
  insert_action_group (std::string (LW_APP_ID).substr (idx + 1),
                       m_actions);
  setup_accels ();

  setup_header_bar (m_menu_button);

  populate ();

  setup_smallmap ();
  setup_bigmap ();

  connect_action_signals ();
  connect_signals ();

  setup_dark_mode_change ();
  setup_quit ();

  block_signals ();
  set_pointer_button (EditorMapWidget::POINTER);
  m_pointer_button->set_active (true);

  unblock_signals ();
  update_window_title ();
  update_actions ();
}

void ScenarioBuilderWindow::setup_header_bar (Gtk::MenuButton *menu_button)
{
  m_header_bar = Gtk::make_managed<Gtk::HeaderBar>();
  m_header_bar->set_show_title_buttons (true);

  m_notebook = Gtk::make_managed<Gtk::Notebook> ();
  m_notebook->set_show_tabs (false);
  m_notebook->set_show_border (false);

  auto page1 = Gtk::make_managed<Gtk::Box> (Gtk::Orientation::VERTICAL);
  auto title1 = Gtk::make_managed<Gtk::Label>(_("Scenario Builder"));
  title1->add_css_class ("title");
  page1->append (*title1);
  page1->set_valign (Gtk::Align::CENTER);
  m_notebook->append_page (*page1, "");

  auto page2 = Gtk::make_managed<Gtk::Box> (Gtk::Orientation::VERTICAL);
  auto title2 = Gtk::make_managed<Gtk::Label>(_("Scenario Builder"));
  title2->add_css_class ("title");
  page2->append (*title2);
  page2->set_valign (Gtk::Align::CENTER);

  m_subtitle = Gtk::make_managed<Gtk::Label>("");
  m_subtitle->add_css_class ("subtitle");
  page2->append (*m_subtitle);

  m_notebook->append_page (*page2, "");
  m_header_bar->set_title_widget (*m_notebook);

  m_header_bar->pack_end (*menu_button);

  m_shield_menu_button = Gtk::make_managed<ShieldMenuButton> ();
  m_header_bar->pack_end (*m_shield_menu_button);

  m_pointer_size_menu_button =
    Gtk::make_managed<PointerSizeMenuButton> ();
  m_header_bar->pack_end (*m_pointer_size_menu_button);

  m_pointer_position_label = Gtk::make_managed<Gtk::Label> ();
  m_header_bar->pack_start (*m_pointer_position_label);

  set_titlebar (*m_header_bar);
}

void ScenarioBuilderWindow::draw_pointer_buttons ()
{
  m_pointer_size_menu_button->redraw ();
}

void ScenarioBuilderWindow::draw_buttons ()
{
  if (Lw::get_dark ())
    {
      set_button_image (m_pointer_button,
                        File::getEditorFile ("button_selector"));
      set_button_image (m_move_button,
                        File::getEditorFile ("button_move_dark"));
      set_button_image (m_erase_button,
                        File::getEditorFile ("button_erase_dark"));
    }
  else
    {
      set_button_image (m_pointer_button,
                        File::getEditorFile ("button_selector"));
      set_button_image (m_move_button,
                        File::getEditorFile ("button_move"));
      set_button_image (m_erase_button,
                        File::getEditorFile ("button_erase"));
    }

  set_button_image (m_stack_button,
                    File::getEditorFile ("button_stack"));
  set_button_image (m_city_button,
                    File::getEditorFile ("button_castle"));
  set_button_image (m_ruin_button,
                    File::getEditorFile ("button_ruin"));
  set_button_image (m_temple_button,
                    File::getEditorFile ("button_temple"));
  set_button_image (m_port_button,
                    File::getEditorFile ("button_port"));
  set_button_image (m_sign_button,
                    File::getEditorFile ("button_signpost"));
  set_button_image (m_road_button,
                    File::getEditorFile ("button_road"));
  set_button_image (m_bridge_button,
                    File::getEditorFile ("button_bridge"));
  set_button_image (m_stone_button,
                    File::getEditorFile ("button_stone"));
  set_button_image (m_bag_button,
                    File::getEditorFile ("button_bag"));
  set_button_image (m_standard_button,
                    File::getEditorFile ("button_flag"));
  set_button_image (m_add_battle_button,
                    File::getEditorFile ("button_fight"));
  set_button_image (m_tilestyle_button,
                    File::getEditorFile ("button_tilestyle"));
}

void ScenarioBuilderWindow::add_connection (sigc::connection c)
{
  m_connections.push_back (c);
}

void ScenarioBuilderWindow::add_action_connection (sigc::connection c)
{
  m_action_connections.push_back (c);
}

void ScenarioBuilderWindow::action_connect (Glib::ustring name,
                                            sigc::slot<void()> slot)
{
  auto simple =
    std::dynamic_pointer_cast<Gio::SimpleAction>(m_simple_actions[name]);
  if (simple)
    add_action_connection
      (simple->signal_activate ().connect (sigc::hide (slot)));
}

void ScenarioBuilderWindow::connect_action_signals ()
{
  action_connect
    ("editor.battle-calculator.set-attacking-stack",
     [this] ()
     {
       if (!m_selected_stack_for_battle_calculator)
         return;
       //get rid of the old ones, if any
       for (auto a : m_battle_calculator_attackers)
         delete a;
       m_battle_calculator_attackers.clear ();

       for (auto a : *m_selected_stack_for_battle_calculator)
         {
           if (a->isHero ())
             m_battle_calculator_attackers.push_back
               (new Hero (*dynamic_cast<Hero*>(a)));
           else
             m_battle_calculator_attackers.push_back
               (new Army (*a, a->getOwner()));
         }
       m_selected_stack_for_battle_calculator = NULL;
       m_simple_actions["editor.tools.battle-calculator"]->activate ();
     });

  action_connect
    ("editor.battle-calculator.set-defending-stack",
     [this] ()
     {
       if (!m_selected_stack_for_battle_calculator)
         return;
       //get rid of the old ones, if any
       for (auto a : m_battle_calculator_defenders)
         delete a;
       m_battle_calculator_defenders.clear ();

       for (auto a : *m_selected_stack_for_battle_calculator)
         {
           if (a->isHero ())
             m_battle_calculator_defenders.push_back
               (new Hero (*dynamic_cast<Hero*>(a)));
           else
             m_battle_calculator_defenders.push_back
               (new Army (*a, a->getOwner()));
         }
       m_selected_stack_for_battle_calculator = NULL;
       m_simple_actions["editor.tools.battle-calculator"]->activate ();
     });

  action_connect
    ("editor.battle-calculator.add-defending-stack",
     [this] ()
     {
       if (!m_selected_stack_for_battle_calculator)
         return;
       int span = GameMap::getCityset ()->getCityTileWidth ();
       for (auto a : *m_selected_stack_for_battle_calculator)
         {
           if (m_battle_calculator_defenders.size () >=
               MAX_STACK_SIZE * span * span)
             continue;
           if (a->isHero ())
             m_battle_calculator_defenders.push_back
               (new Hero (*dynamic_cast<Hero*>(a)));
           else
             m_battle_calculator_defenders.push_back
               (new Army (*a, a->getOwner()));
         }
       m_selected_stack_for_battle_calculator = NULL;
       m_simple_actions["editor.tools.battle-calculator"]->activate ();
     });

  action_connect
    ("editor.file.new",
     [this] ()
     {
       check_discard
         (_("Save these changes before making a new scenario?"),
          [this] (bool discard)
          {
            if (discard)
              {
                m_current_save_filename = "";
                m_new_scenario_needs_saving = true;
                create_blank_scenario ();
              }
          });
     });

  action_connect
    ("editor.file.new-random-map",
     [this] ()
     {
       check_discard
         (_("Save these changes before making a new scenario?"),
          [this] (bool discard)
          {
            if (discard)
              {
                m_current_save_filename = "";
                m_new_scenario_needs_saving = true;
                auto d = LwDialog::build<NewMapDialog> (this);
                d->setup ();
                d->signal_response ().connect
                  ([this, d] (Gtk::ResponseType resp)
                   {
                     switch (resp)
                       {
                       case Gtk::ResponseType::ACCEPT:
                           {
                             if (d->get_filled_map ())
                               set_filled_map
                                 (d->get_width (), d->get_height (),
                                  d->get_filled_terrain_tile (),
                                  d->get_armyset_themes (),
                                  d->get_cityset_theme (),
                                  d->get_shieldset_theme (),
                                  d->get_tileset_theme (),
                                  d->get_players ());
                             else
                               set_random_map (d);

                             m_umgr->clear ();
                           }
                         break;

                       default:
                         create_blank_scenario ();
                         break;
                       }
                     delete d;
                   });
              }
          });
     });

  action_connect
    ("editor.file.open",
     [this] ()
     {
       check_discard
         (_("Save these changes before making a new scenario?"),
          [this] (bool discard)
          {
            if (discard)
              {
                LwDialog::open
                  (*this, _("Choose a scenario to open"),
                   FileFilter::SCENARIO,
                   [this] (std::string path)
                   {
                     load_scenario
                       (path,
                        [this] (bool loaded, Glib::ustring err)
                        {
                          if (loaded)
                            {
                              if (m_scenario->getRound () > 0)
                                {
                                  Glib::ustring msg =
                                    _("Reset the scenario?");
                                  auto d = LwDialog::alert_yn (msg);
                                  d->choose
                                    (*this,
                                     [this, d] (auto result)
                                     {
                                       bool reset =
                                         d->choose_finish (result) == 1;
                                       if (reset)
                                         {
                                           convert_sav_to_map ();
                                           m_scenario_modified = true;
                                           m_new_scenario_needs_saving =
                                             false;
                                         }
                                       else
                                         {
                                           clear_save_file_of_scenario_specific_data ();
                                           m_scenario_modified = false;
                                           m_new_scenario_needs_saving =
                                             false;
                                         }
                                       update ();
                                     });
                                }
                            }
                          else
                            {
                              Glib::ustring msg =
                                _("Couldn't load scenario");
                              auto dialog = LwDialog::alert (msg, err);
                              dialog->choose
                                (*this,
                                 [this, dialog] (auto result)
                                 {
                                   dialog->choose_finish (result);
                                   create_blank_scenario ();
                                   return;
                                 });
                            }
                        });
                   });
              }
          });
     });

  action_connect
    ("editor.file.save",
     [this] ()
     {
       if (m_current_save_filename.empty () == true)
         on_save_as_activated ();
       else
         {
           check_save_valid
             ([this](bool valid)
              {
                if (valid)
                  {
                    save_current_scenario_file
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
    ("editor.file.save-as",
     [this] ()
     {
       on_save_as_activated ();
     });

  action_connect
    ("editor.file.validate",
     [this] ()
     {
       std::list<Glib::ustring> errors;
       std::list<Glib::ustring> warnings;
       m_scenario->validate (errors, warnings);
       auto d = LwDialog::build<ValidationDialog> (this);
       d->setup (errors, warnings);
       d->signal_response ().connect
         ([d] (Gtk::ResponseType)
          {
            delete d;
          });
     });

  action_connect
    ("editor.file.quit",
     [this] ()
     {
       check_quit
         ([this] (bool quit)
          {
            if (quit)
              {
                if (m_scenario)
                  delete m_scenario;
                hide ();
              }
          });
       return;
     });

  action_connect
    ("editor.edit.properties",
     [this] ()
     {
       auto d = LwDialog::build<MapInfoDialog> (this);
       d->setup (m_scenario);
       d->signal_response ().connect
         ([this, d] (Gtk::ResponseType resp)
          {
            if (d->is_changed () &&
                resp == Gtk::ResponseType::ACCEPT)
              {
                m_umgr->add
                  (new EditorUndoAction_Properties
                   (m_scenario->getName (),
                    m_scenario->getComment (),
                    m_scenario->getCopyright (),
                    m_scenario->getLicense ()));

                m_scenario->setName (d->get_name ());
                m_scenario->setComment (d->get_description ());
                m_scenario->setCopyright (d->get_copyright ());
                m_scenario->setLicense (d->get_license ());
                m_scenario_modified = true;
                update ();
              }
            delete d;
          });
     });

  action_connect
    ("editor.help.tutorial-video",
     [this] ()
     {
       Glib::ustring uri = "https://vimeo.com/406899445";
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
    ("editor.help.about",
     [this] ()
     {
       auto d = Gtk::make_managed<AboutDialog> (*this);
       d->set_program_name ("LordsAWar! Scenario Builder");
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
    ("editor.help.keyboard-shortcuts",
     [this] ()
     {
       auto* dialog = Gtk::make_managed<EditorShortcutsDialog>();
       dialog->set_transient_for (*this);
       dialog->present ();
     });

  action_connect
    ("editor.edit.undo",
     [this] ()
     {
       m_umgr->undo ();
       if (m_umgr->undo_empty () && !m_new_scenario_needs_saving)
         m_scenario_modified = false;
       update ();
     });

  action_connect
    ("editor.edit.redo",
     [this] ()
     {
       m_scenario_modified = true;
       m_umgr->redo ();
       update ();
     });

  action_connect
    ("editor.edit.mini-map",
     [this] ()
     {

       auto action = new EditorUndoAction_MiniMap (m_scenario);
       auto w = new SmallmapEditorWindow (this);
       w->setup ();
       w->signal_response ().connect
         ([this, w, action] (Gtk::ResponseType resp)
          {
            switch (resp)
              {
              case Gtk::ResponseType::ACCEPT:
                m_smallmap->resize ();
                update ();
                break;

              default:
                break;
              }
            if (w->is_changed ())
              m_umgr->add (action);
            else
              delete action;
            delete w;
          });
     });

  action_connect
    ("editor.edit.scenario-media",
     [this] ()
     {
       auto action = new EditorUndoAction_ScenarioMedia (m_scenario);
       auto d = LwDialog::build<MediaDialog> (this);
       d->setup (m_scenario);
       d->signal_response ().connect
         ([this, d, action] (Gtk::ResponseType resp)
          {
            switch (resp)
              {
              case Gtk::ResponseType::ACCEPT:
                if (d->is_changed ())
                  m_umgr->add (action);
                break;

              default:
                delete action;
                break;
              }
            delete d;
          });
     });

  action_connect
    ("editor.edit.players",
     [this] ()
     {
       auto d = LwDialog::build<PlayersDialog> (this);
       d->setup (m_create_scenario_names, Playerlist::instance (),
                 HeroTemplates::instance ());
       d->signal_response ().connect
         ([this, d] (Gtk::ResponseType resp)
          {
            switch (resp)
              {
              case Gtk::ResponseType::ACCEPT:
                if (d->is_changed ())
                  {
                    m_umgr->add
                      (new EditorUndoAction_Players (m_scenario));
                    auto g = d->get_params ();
                    Playerlist::instance ()->syncPlayers (g);
                    for (guint32 i = Shield::WHITE; i <= Shield::BLACK;
                         i++)
                      {
                        auto p = Playerlist::instance ()->get (i);
                        if (p)
                          p->setGold (d->get_gold (Shield::Color (i)));
                      }
                    HeroTemplates::reset (d->get_hero_templates ()->copy ());
                    m_shield_menu_button->reload ();
                    update ();
                  }
                break;

              default:
                break;
              }
            delete d;
          });
     });

  action_connect
    ("editor.edit.items",
     [this] ()
     {
       auto d = LwDialog::build<ItemListDialog> (this);
       d->setup (Itemlist::instance ());
       d->signal_response ().connect
         ([this, d] (Gtk::ResponseType)
          {
            if (d->is_changed ())
              {
                m_umgr->add
                  (new EditorUndoAction_Items (m_scenario));
                Itemlist::reset (d->get_item_list ()->copy ());
              }
            delete d;
          });
     });

  action_connect
    ("editor.edit.rewards",
     [this] ()
     {
       auto d = LwDialog::build<RewardListEditorDialog> (this);
       d->setup (Rewardlist::instance (), m_scenario->getRound () == 0);
       d->signal_response ().connect
         ([this, d] (Gtk::ResponseType)
          {
            if (d->is_changed ())
              {
                m_umgr->add
                  (new EditorUndoAction_Rewards (m_scenario));
                Rewardlist::reset (d->get_reward_list ()->copy ());
              }
            delete d;
          });
     });

  action_connect
    ("editor.edit.smooth-screen",
     [this] ()
     {
       auto view = m_bigmap->get_view ();
       m_umgr->add (new EditorUndoAction_Smooth (view));

       GameMap::instance ()->applyTileStyles
         (view.x, view.y, view.h, view.w, true);
       m_bigmap->queue_draw ();
       update ();
     });

  action_connect
    ("editor.edit.smooth-map",
     [this] ()
     {
       m_umgr->add
         (new EditorUndoAction_Smooth
          (LwRectangle (0, 0,
                        GameMap::getWidth (), GameMap::getHeight ())));

       GameMap::instance ()->applyTileStyles
         (0, 0, GameMap::getHeight (), GameMap::getWidth (), true);
       m_bigmap->queue_draw ();
       update ();
     });

  action_connect
    ("editor.edit.switch-sets",
     [this] ()
     {
       auto d = LwDialog::build<SwitchSetsDialog> (this);
       d->setup ();
       d->signal_response ().connect
         ([this, d] (Gtk::ResponseType resp)
          {
            bool broken = false;
            std::vector<Armyset*> armysets = d->get_armysets ();
            Cityset *cityset = d->get_cityset ();
            Shieldset *shieldset = d->get_shieldset ();
            Tileset *tileset = d->get_tileset ();

            ImageCache::instance ()->reset ();
            switch (resp)
              {
              case Gtk::ResponseType::ACCEPT:
                if (d->is_changed ())
                  {
                    auto w = Gtk::make_managed<EditorLoadWindow> ();
                    w->setup ();
                    w->present ();
                    w->set_modal (true);
                    w->set_transient_for (*this);

                    m_umgr->add
                      (new EditorUndoAction_SwitchSets (m_scenario));
                    if (d->is_armyset_changed ())
                      {
                        auto curr = GameMap::instance ()->getArmysets ();

                        for (guint32 i = Shield::WHITE;
                             i <= Shield::NEUTRAL; i++)
                          {
                            w->set_fraction (i * 0.7);
                            auto a = armysets[i];
                            if (!a || !curr[i])
                              continue;
                            auto sh = Shield::Color (i);
                            auto p = Playerlist::instance ()->get (sh);
                            if (a->getId () != curr[i]->getId ())
                              GameMap::instance ()->switchArmysets (p, a);
                            if (!a->get_images_instantiated ())
                              a->instantiateImages (broken);
                          }
                      }

                    w->set_fraction (0.75);
                    if (d->is_cityset_changed ())
                      {
                        if (!cityset->get_images_instantiated ())
                          cityset->instantiateImages (broken);
                        GameMap::instance ()->switchCityset (cityset);
                      }

                    w->set_fraction (0.85);
                    if (d->is_shieldset_changed ())
                      {
                        GameMap::instance ()->switchShieldset
                          (shieldset);
                        if (!shieldset->get_images_instantiated ())
                          shieldset->instantiateImages (broken);
                        m_smallmap->resize ();
                        m_shield_menu_button->reload ();
                      }

                    w->set_fraction (0.95);
                    if (d->is_tileset_changed ())
                      {
                        GameMap::instance ()->switchTileset (tileset);
                        if (!tileset->get_images_instantiated ())
                          tileset->instantiateImages (broken);
                        m_smallmap->resize ();
                        add_terrain_buttons ();
                      }

                    w->set_fraction (1.0);

                    update ();
                    delete w;
                  }
                break;

              default:
                break;
              }

            delete d;
          });
     });

  action_connect
    ("editor.edit.army-set",
     [this] ()
     {
       auto w = new ArmySetWindow ();
       auto armyset = Armysetlist::instance ()->get 
         (Playerlist::getActiveplayer ()->getArmyset ());
       auto shieldset = GameMap::getShieldset ();
       w->set_modal (true);
       w->set_transient_for (*this);
       w->setup (shieldset, armyset);

       w->signal_armyset_saved ().connect
         ([this] (guint32 id)
          {
            for (auto as : GameMap::getArmysets ())
              {
                if (!as)
                  continue;
                if (id == as->getId ())
                  {
                    ImageCache::instance ()->reset ();
                    Armysetlist::instance ()->reload (id);
                    auto pl = Playerlist::instance ();
                    for (auto p : pl->getPlayersWithArmyset (id))
                      GameMap::instance ()->switchArmysets
                        (p, Armysetlist::instance ()->get (id));
                    m_shield_menu_button->reload ();
                    update ();
                    break;
                  }
              }
          });

       w->signal_closed ().connect
         ([w] ()
          {
            delete w;
          });
       w->present ();
     });

  action_connect
    ("editor.edit.city-set",
     [this] ()
     {
       auto w = new CitySetWindow ();
       auto cityset = GameMap::getCityset ();
       w->set_modal (true);
       w->set_transient_for (*this);
       w->setup (cityset);

       w->signal_cityset_saved ().connect
         ([this] (guint32 id)
          {
            if (id == GameMap::getCityset ()->getId ())
              {
                ImageCache::instance ()->reset ();
                Citysetlist::instance ()->reload (id);
                GameMap::resetCityset ();
                GameMap::instance ()->reloadCityset ();
                update ();
              }
          });

       w->signal_closed ().connect
         ([w] ()
          {
            delete w;
          });
       w->present ();
     });

  action_connect
    ("editor.edit.shield-set",
     [this] ()
     {
       auto w = new ShieldSetWindow ();
       auto shieldset = GameMap::getShieldset ();
       w->set_modal (true);
       w->set_transient_for (*this);
       w->setup (shieldset);

       w->signal_shieldset_saved ().connect
         ([this] (guint32 id)
          {
            if (id == GameMap::getShieldset ()->getId ())
              {
                ImageCache::instance ()->reset ();
                Shieldsetlist::instance ()->reload (id);
                GameMap::resetShieldset ();
                GameMap::instance ()->reloadShieldset ();
                m_shield_menu_button->reload ();
                m_smallmap->resize ();
                update ();
              }
          });

       w->signal_closed ().connect
         ([w] ()
          {
            delete w;
          });
       w->present ();
     });

  action_connect
    ("editor.edit.tile-set",
     [this] ()
     {
       auto w = new TileSetWindow ();
       auto tileset = GameMap::getTileset ();
       auto shieldset = GameMap::getShieldset ();
       w->set_modal (true);
       w->set_transient_for (*this);
       w->setup (shieldset, tileset);

       w->signal_tileset_saved ().connect
         ([this] (guint32 id)
          {
            if (id == GameMap::getTileset ()->getId ())
              {
                ImageCache::instance ()->reset ();
                GameMap::resetTileset ();
                Tilesetlist::instance ()->reload (id);
                GameMap::instance ()->switchTileset
                  (Tilesetlist::instance ()->get (id));
                add_terrain_buttons ();
                m_smallmap->resize ();
                update ();
              }
          });

       w->signal_closed ().connect
         ([w] ()
          {
            delete w;
          });
       w->present ();
     });

  action_connect
    ("editor.edit.fight-order",
     [this] ()
     {
       auto action = new EditorUndoAction_FightOrder (m_scenario);
       auto d = LwDialog::build<FightOrderEditorDialog> (this);
       d->setup ();
       d->signal_response ().connect
         ([this, d, action] (Gtk::ResponseType resp)
          {
            switch (resp)
              {
              case Gtk::ResponseType::ACCEPT:
                if (d->is_changed ())
                  m_umgr->add (action);
                break;

              default:
                delete action;
                break;
              }
            delete d;
          });
     });

  action_connect
    ("editor.view.toggle-grid",
     [this] ()
     {
       m_bigmap->set_show_grid (!m_bigmap->get_show_grid ());
       m_bigmap->queue_draw ();
     });

  action_connect
    ("editor.tools.battle-calculator",
     [this] ()
     {
       auto d = LwDialog::build<BattleCalculatorDialog> (this);
       d->setup (m_battle_calculator_attackers,
                 m_battle_calculator_defenders);
       d->signal_response ().connect
         ([this, d] (Gtk::ResponseType)
          {
            delete d;
          });
     });

  action_connect
    ("editor.edit.randomize-objects",
     [this] ()
     {
       auto d = LwDialog::build<RandomizeDialog> (this);
       d->setup (m_create_scenario_names);
       d->signal_response ().connect
         ([this, d] (Gtk::ResponseType resp)
          {
            switch (resp)
              {
              case Gtk::ResponseType::ACCEPT:
                  {
                    m_umgr->add
                      (new EditorUndoAction_RandomizeObjects
                       (m_scenario));

                    d->randomize_objects ();
                    update ();
                  }
                break;

              default:
                break;
              }
            delete d;
          });
     });

  action_connect
    ("editor.edit.assign-capitals",
     [this] ()
     {
       bool modified = false;
       auto action = new EditorUndoAction_AssignCapitals (m_scenario);

       bool capitals_set = false;
       for (auto p : *Playerlist::instance ())
         {
           if (p == Playerlist::getNeutral ())
             continue;

           if (Citylist::instance ()->getCapitalCity (p))
             {
               capitals_set = true;
               break;
             }
         }

       bool only_capitals_set = false;
       if (capitals_set)
         {
           only_capitals_set = true;
           for (auto c : *Citylist::instance ())
             {
               if (c->getOwner () == Playerlist::getNeutral ())
                 continue;
               if (c->isCapital () == false && c->isBurnt () == false)
                 {
                   only_capitals_set = false;
                   break;
                 }
             }
         }

       //clear capitals
       if (capitals_set)
         {
           for (auto c : *Citylist::instance ())
             {
               if (c->isCapital ())
                 {
                   modified = true;
                   c->setCapital (false);
                   c->setCapitalOwner (NULL);
                   if (only_capitals_set)
                     change_city_ownership (c, Playerlist::getNeutral ());
                 }
             }
         }

       //for each player except neutral, pick a new capital
       for (auto p : *Playerlist::instance ())
         {
           if (p == Playerlist::getNeutral ())
             continue;
           std::vector<City*> neutral_cities;
           std::vector<City*> player_cities;
           for (auto c : *Citylist::instance ())
             {
               if (c->getOwner () == Playerlist::getNeutral ())
                 neutral_cities.push_back (c);
               if (c->getOwner () == p)
                 player_cities.push_back (c);
             }

           //pick one.

           std::vector<City*> cities = player_cities;
           if (cities.empty ())
             cities = neutral_cities;
           if (cities.empty() == false)
             {
               modified = true;
               City *capital = cities[Rnd::rand () % cities.size ()];
               change_city_ownership (capital, p);
               capital->setCapital (true);
               capital->setCapitalOwner (p);
             }
         }

       if (modified)
         {
           GameMap::instance ()->clearStackPositions ();
           GameMap::instance ()->updateStackPositions ();
           m_umgr->add (action);
         }
       else
         delete action;
     });

  action_connect
    ("editor.map.stack-details",
     [this] ()
     {
       auto pos = m_selected_stack_for_details->getPos ();
       auto d = LwDialog::build<StackEditorDialog> (this);
       d->setup (m_selected_stack_for_details, pos);
       d->signal_response ().connect
         ([this, d, pos] (Gtk::ResponseType resp)
          {
            switch (resp)
              {
              case Gtk::ResponseType::ACCEPT:
                if (d->is_changed ())
                  {
                    m_umgr->add
                      (new EditorUndoAction_EditStack
                       (LwRectangle (pos)));

                    auto s = m_selected_stack_for_details;
                    GameMap::instance ()->removeStack (s);

                    auto ns = d->get_stack ();
                    GameMap::instance ()->putStack (new Stack (*ns),
                                                    true);
                    update ();
                  }
                break;

              default:
                break;
              }
            delete d;
          });
     });

  action_connect
    ("editor.map.city-details",
     [this] ()
     {
       auto pos = m_selected_city_for_details->getPos ();
       auto d = LwDialog::build<CityEditorDialog> (this);
       auto p = m_selected_city_for_details->getOwner ();
       d->setup (m_selected_city_for_details, m_create_scenario_names);
       d->signal_response ().connect
         ([this, d, p, pos] (Gtk::ResponseType resp)
          {
            switch (resp)
              {
              case Gtk::ResponseType::ACCEPT:
                if (d->is_changed ())
                  {
                    m_umgr->add
                      (new EditorUndoAction_EditCity (LwRectangle (pos)));
                    City *city = new City (*d->get_city (), true);
                    Citylist::instance ()->replace
                      (m_selected_city_for_details, city);

                    //is it razed? we lose the production bases.
                    if (city->isBurnt ())
                      {
                        guint32 c = 0;
                        for (; c < city->getMaxNoOfProductionBases ();
                             ++c)
                          city->removeProductionBase (c);
                      }
                    // set allegiance of stacks under the city

                    auto old_owner = p;
                    auto new_owner = city->getOwner ();
                    //hack, here we temporarily set the owner back to the
                    //original so getdefendersincity will be able to 
                    //detect stacks on the city
                    city->setOwner (old_owner);
                    for (auto s : Stacklist::getDefendersInCity (city))
                      {
                        p->getStacklist ()->on_stack_died (s);
                        Stacklist::changeOwnership (s, new_owner);
                      }
                    city->setOwner (new_owner);
                    GameMap::instance ()->clearStackPositions ();
                    GameMap::instance ()->updateStackPositions ();

                    update ();
                  }
                break;

              default:
                break;
              }
            delete d;
          });
     });

  action_connect
    ("editor.map.ruin-details",
     [this] ()
     {
       auto pos = m_selected_ruin_for_details->getPos ();
       auto d = LwDialog::build<RuinEditorDialog> (this);
       d->setup (m_selected_ruin_for_details, m_create_scenario_names);
       d->signal_response ().connect
         ([this, d, pos] (Gtk::ResponseType resp)
          {
            switch (resp)
              {
              case Gtk::ResponseType::ACCEPT:
                if (d->is_changed ())
                  {
                    m_umgr->add
                      (new EditorUndoAction_EditRuin
                       (LwRectangle (pos)));
                    Ruin *ruin = new Ruin (*d->get_ruin (), true);
                    Ruinlist::instance ()->replace
                      (m_selected_ruin_for_details, ruin);
                    GameMap::instance ()->update_ruin_rewards
                      (ruin->getPos (), ruin);
                    update ();
                  }
                break;

              default:
                break;
              }
            delete d;
          });
     });

  action_connect
    ("editor.map.signpost-details",
     [this] ()
     {
       auto pos = m_selected_signpost_for_details->getPos ();
       auto d = LwDialog::build<SignpostEditorDialog> (this);
       d->setup (pos, m_create_scenario_names);
       d->signal_response ().connect
         ([this, d, pos] (Gtk::ResponseType resp)
          {
            switch (resp)
              {
              case Gtk::ResponseType::ACCEPT:
                if (d->is_changed ())
                  {
                    m_umgr->add
                      (new EditorUndoAction_EditSignpost
                       (LwRectangle (pos)));
                    auto n = d->get_signpost ()->getName ();
                    m_selected_signpost_for_details->setName (n);
                    update ();
                  }
                break;

              default:
                break;
              }
            delete d;
          });
     });

  action_connect
    ("editor.map.temple-details",
     [this] ()
     {

       auto temple = m_selected_temple_for_details;
       auto pos = temple->getPos ();
       auto d = LwDialog::build<TempleEditorDialog> (this);
       d->setup (temple, m_create_scenario_names);
       d->signal_response ().connect
         ([this, d, pos, temple] (Gtk::ResponseType resp)
          {
            switch (resp)
              {
              case Gtk::ResponseType::ACCEPT:
                if (d->is_changed ())
                  {
                    m_umgr->add
                      (new EditorUndoAction_EditTemple
                       (LwRectangle (pos)));
                    auto ntemple = d->get_temple ();
                    temple->setType (ntemple->getType ());
                    temple->setName (ntemple->getName ());
                    temple->setDescription (ntemple->getDescription ());

                    update ();
                  }
                break;

              default:
                break;
              }
            delete d;
          });
     });

  action_connect
    ("editor.map.road-details",
     [this] ()
     {
       auto mouse_pos = m_selected_road_mouse_position;
       auto pos = m_selected_road_for_details->getPos ();

       auto w = Gtk::make_managed<SelectRoadPopover> ();
       w->setup (GameMap::getRoad (pos));
       w->signal_road_selected ().connect
         ([this, pos] (Road::Type id)
          {
            m_umgr->add
              (new EditorUndoAction_EditRoad (LwRectangle (pos)));
            m_selected_road_for_details->setType (id);
            update ();
          });

       w->signal_closed ().connect
         ([this, w] ()
          {
            delete w;
          });

       w->set_position (Gtk::PositionType::LEFT);
       w->set_parent (*m_bigmap);

       Gdk::Rectangle rect;
       rect.set_x (mouse_pos.x);
       rect.set_y (mouse_pos.y);
       rect.set_width (1);
       rect.set_height (1);

       w->set_pointing_to (rect);
       w->popup ();
     });

  action_connect
    ("editor.map.stone-details",
     [this] ()
     {
       auto mouse_pos = m_selected_stone_mouse_position;
       auto pos = m_selected_stone_for_details->getPos ();

       auto w = Gtk::make_managed<SelectStonePopover> ();
       w->setup (GameMap::getStone (pos), GameMap::getRoad (pos));
       w->signal_stone_selected ().connect
         ([this, pos] (Stone::Type id)
          {
            m_umgr->add
              (new EditorUndoAction_EditStone (LwRectangle (pos)));
            m_selected_stone_for_details->setType (id);
            update ();
          });

       w->signal_closed ().connect
         ([this, w] ()
          {
            delete w;
          });

       w->set_position (Gtk::PositionType::LEFT);
       w->set_parent (*m_bigmap);

       Gdk::Rectangle rect;
       rect.set_x (mouse_pos.x);
       rect.set_y (mouse_pos.y);
       rect.set_width (1);
       rect.set_height (1);

       w->set_pointing_to (rect);
       w->popup ();
     });

  action_connect
    ("editor.map.bag-details",
     [this] ()
     {
       auto pos = m_selected_bag_for_details->getPos ();
       auto d = LwDialog::build<BackpackEditorDialog> (this);
       d->setup (m_selected_bag_for_details,
                 Playerlist::getActiveplayer ()->get_shield ());
       d->signal_response ().connect
         ([this, d, pos] (Gtk::ResponseType resp)
          {
            switch (resp)
              {
              case Gtk::ResponseType::ACCEPT:
                if (d->is_changed ())
                  {
                    m_umgr->add
                      (new EditorUndoAction_EditBackpack
                       (LwRectangle (pos)));
                    auto mapbackpack = GameMap::getBackpack (pos);
                    mapbackpack->removeAllFromBackpack ();
                    mapbackpack->add (d->get_backpack ());
                    update ();
                  }
                break;

              default:
                break;
              }
            delete d;
          });

     });

  action_connect
    ("editor.map.flag-details",
     [this] ()
     {
       auto pos = m_selected_bag_for_details->getPos ();
       auto flag = m_selected_bag_for_details->getFirstPlantedItem ();
       auto d = LwDialog::build<PlantedStandardEditorDialog> (this);
       d->setup (pos);
       d->signal_response ().connect
         ([this, d, flag, pos] (Gtk::ResponseType resp)
          {
            switch (resp)
              {
              case Gtk::ResponseType::ACCEPT:
                if (d->is_changed ())
                  {
                    m_umgr->add
                      (new EditorUndoAction_EditFlag (LwRectangle (pos)));
                    auto new_flag = new Item (*d->get_item (), true);
                    m_selected_bag_for_details->replace (flag, new_flag);
                    update ();
                  }
                break;

              default:
                break;
              }
            delete d;
          });
     });

  action_connect
    ("editor.map.tilestyle-details",
     [this] ()
     {
       auto pos = m_selected_tilestyle_for_details;
       auto mouse_pos = m_selected_tilestyle_mouse_position;

       auto w = Gtk::make_managed<SelectTileStylePopover> ();
       w->setup (pos);
       w->signal_tilestyle_selected ().connect
         ([this, pos] (guint32 id)
          {
            auto mtile = GameMap::instance ()->getTile (pos);
            auto old_id = mtile->getTileStyleId ();

            m_umgr->add
              (new EditorUndoAction_TileStyle (pos, old_id));
            mtile->setTileStyleId (id);
            update ();
          });

       w->signal_closed ().connect
         ([this, w] ()
          {
            delete w;
          });

       w->set_position (Gtk::PositionType::LEFT);
       w->set_parent (*m_bigmap);

       Gdk::Rectangle rect;
       rect.set_x (mouse_pos.x);
       rect.set_y (mouse_pos.y);
       rect.set_width (1);
       rect.set_height (1);

       w->set_pointing_to (rect);
       w->popup ();
     });
}

void ScenarioBuilderWindow::disconnect_signals ()
{
  for (auto c : m_connections)
    c.disconnect ();
  m_connections.clear ();
}

void ScenarioBuilderWindow::disconnect_action_signals ()
{
  for (auto c : m_action_connections)
    c.disconnect ();
  m_action_connections.clear ();
}

void ScenarioBuilderWindow::block_terrain_button_signals ()
{
  for (auto c : m_terrain_button_connections)
    c.block ();
}

void ScenarioBuilderWindow::unblock_terrain_button_signals ()
{
  for (auto c : m_terrain_button_connections)
    c.unblock ();
}

void ScenarioBuilderWindow::block_signals ()
{
  for (auto c : m_connections)
    c.block ();
}

void ScenarioBuilderWindow::unblock_signals ()
{
  for (auto c : m_connections)
    c.unblock ();
}

void ScenarioBuilderWindow::connect_signals ()
{
  add_connection
    (m_bigmap->signal_view_changed ().connect
     (sigc::mem_fun (*m_smallmap, &SmallMap::set_view)));

  add_connection
    (m_smallmap->signal_view_changed ().connect
     (sigc::mem_fun (*m_bigmap, &EditorMapWidget::set_view)));

  add_connection
    (m_bigmap->signal_size_allocated ().connect
     ([this] (int, int)
      {
        auto tile = m_smallmap->get_view ().pos;
        m_bigmap->center_on_smallmap_pos (tile.x, tile.y);
      }));

  add_connection
    (m_bigmap->signal_pointing_at_new_tile ().connect
     ([this] (Vector<int> tile)
      {
        auto b = GameMap::instance ()->getTile (tile)->getBuilding ();
        Glib::ustring s = "";
        switch (b)
          {
          case Maptile::NONE:
            s = String::ucompose ("(%1, %2)", tile.x, tile.y);
            break;

          case Maptile::CITY:
            s = String::ucompose ("(%1, %2) [%3]", tile.x, tile.y,
                                  _("City"));
            break;

          case Maptile::RUIN:
            s = String::ucompose ("(%1, %2) [%3]", tile.x, tile.y,
                                  _("Ruin"));
            break;

          case Maptile::TEMPLE:
            s = String::ucompose ("(%1, %2) [%3]", tile.x, tile.y,
                                  _("Temple"));
            break;

          case Maptile::SIGNPOST:
            s = String::ucompose ("(%1, %2) [%3]", tile.x, tile.y,
                                  _("Signpost"));
            break;

          case Maptile::ROAD:
            s = String::ucompose ("(%1, %2) [%3]", tile.x, tile.y,
                                  _("Road"));
            break;

          case Maptile::PORT:
            s = String::ucompose ("(%1, %2) [%3]", tile.x, tile.y,
                                  _("Port"));
            break;

          case Maptile::BRIDGE:
            s = String::ucompose ("(%1, %2) [%3]", tile.x, tile.y,
                                  _("Bridge"));
            break;

          case Maptile::STONE:
            s = String::ucompose ("(%1, %2) [%3]", tile.x, tile.y,
                                  _("Stone"));
            break;
          }

        m_pointer_position_label->set_text (s);
        m_bigmap->queue_draw ();
      }));

  add_connection
    (m_bigmap->signal_map_tiles_changed ().connect
     ([this] (LwRectangle r)
      {
        m_smallmap->redraw_tiles (r);
        update_window_title ();
      }));

  add_connection
    (m_bigmap->signal_undo_generated ().connect
     ([this] (EditorUndoAction *action)
      {
        m_umgr->add (action);
      }));

  add_connection
    (m_bigmap->signal_object_select_choice ().connect
     ([this] (EditorMapWidget::map_selection_seq seq,
              Vector<int> mouse_pos)
      {
        select_object (mouse_pos, seq);
      }));

  add_connection
    (m_bigmap->signal_map_water_changed ().connect
     ([this] ()
      {

        //this is so that the radial water can be redrawn again.
        //otherwise the land doesn't get erased on the smallmap

        m_smallmap->resize ();

      }));

  add_connection
    (m_bigmap->signal_bag_selected ().connect
     ([this] (Vector<int> pos)
      {
        m_selected_bag_for_details = GameMap::getBackpack (pos);
        m_simple_actions["editor.map.bag-details"]->activate ();
      }));

  add_connection
    (m_bigmap->signal_flag_selected ().connect
     ([this] (Vector<int> pos)
      {
        m_selected_bag_for_details = GameMap::getBackpack (pos);
        m_simple_actions["editor.map.flag-details"]->activate ();
      }));

  add_connection
    (m_bigmap->signal_tilestyle_tile_selected ().connect
     ([this] (Vector<int> pos, Vector<int> mouse_pos)
      {
        m_selected_tilestyle_mouse_position = mouse_pos;
        m_selected_tilestyle_for_details = pos;
        m_simple_actions["editor.map.tilestyle-details"]->activate ();
      }));

  add_connection
    (m_bigmap->signal_stack_selected_for_battle_calculator ().connect
     ([this] (Stack *s, Vector<int> pos)
      {
        auto menu = Gio::Menu::create ();

        menu->append (_("Set as attacking stack"),
                      "lw.editor.battle-calculator.set-attacking-stack");
        menu->append (_("Set as defending stack"),
                      "lw.editor.battle-calculator.set-defending-stack");
        menu->append (_("Add defending stack"),
                      "lw.editor.battle-calculator.add-defending-stack");

        m_selected_stack_for_battle_calculator = s;
        auto context_menu = Gtk::make_managed<Gtk::PopoverMenu> ();
        context_menu->set_menu_model (menu);
        context_menu->set_parent (*m_bigmap);
        Gdk::Rectangle rect;
        rect.set_x (pos.x);
        rect.set_y (pos.y);
        rect.set_width (1);
        rect.set_height (1);

        context_menu->set_position (Gtk::PositionType::TOP);
        context_menu->set_pointing_to (rect);
        context_menu->popup ();
      }));

  add_connection
    (m_smallmap->signal_map_changed ().connect
     ([this] (Cairo::RefPtr<Cairo::Surface> map, Gdk::Rectangle)
      {
        std::shared_ptr<Cairo::ImageSurface> img_surface =
          std::static_pointer_cast<Cairo::ImageSurface>(map);
        m_smallmap_surface = map;
        m_map_drawing_area->set_size_request
          (img_surface->get_width (), img_surface->get_height ());
        map->flush ();

        m_map_drawing_area->queue_draw ();
      }));

  add_connection
    (m_pointer_size_menu_button->signal_pointer_size_selected ().connect
     ([this] (guint32 size)
      {
        m_bigmap->set_pointer_size (size);
        m_pointer_size = size;
      }));

  add_connection
    (m_shield_menu_button->signal_shield_selected ().connect
     ([this] (Shield::Color shield)
      {
        Playerlist::instance ()->setActiveplayer (shield);
      }));

  add_connection
    (m_pointer_button->signal_clicked ().connect
     ([this] ()
      {
        set_pointer_button (EditorMapWidget::POINTER);
        m_bigmap->set_pointer_size (1);
      }));

  add_connection
    (m_move_button->signal_clicked ().connect
     ([this] ()
      {
        set_pointer_button (EditorMapWidget::MOVE);
        m_bigmap->set_pointer_size (1);
      }));

  add_connection
    (m_erase_button->signal_clicked ().connect
     ([this] ()
      {
        set_pointer_button (EditorMapWidget::ERASE);
      }));

  add_connection
    (m_stack_button->signal_clicked ().connect
     ([this] ()
      {
        set_pointer_button (EditorMapWidget::STACK);
        m_bigmap->set_pointer_size (1);
      }));

  add_connection
    (m_city_button->signal_clicked ().connect
     ([this] ()
      {
        set_pointer_button (EditorMapWidget::CITY);
        m_bigmap->set_pointer_size (1);
      }));

  add_connection
    (m_ruin_button->signal_clicked ().connect
     ([this] ()
      {
        set_pointer_button (EditorMapWidget::RUIN);
        m_bigmap->set_pointer_size (1);
      }));

  add_connection
    (m_temple_button->signal_clicked ().connect
     ([this] ()
      {
        set_pointer_button (EditorMapWidget::TEMPLE);
        m_bigmap->set_pointer_size (1);
      }));

  add_connection
    (m_port_button->signal_clicked ().connect
     ([this] ()
      {
        set_pointer_button (EditorMapWidget::PORT);
        m_bigmap->set_pointer_size (1);
      }));

  add_connection
    (m_sign_button->signal_clicked ().connect
     ([this] ()
      {
        set_pointer_button (EditorMapWidget::SIGNPOST);
        m_bigmap->set_pointer_size (1);
      }));

  add_connection
    (m_road_button->signal_clicked ().connect
     ([this] ()
      {
        set_pointer_button (EditorMapWidget::ROAD);
        m_bigmap->set_pointer_size (1);
      }));

  add_connection
    (m_bridge_button->signal_clicked ().connect
     ([this] ()
      {
        set_pointer_button (EditorMapWidget::BRIDGE);
        m_bigmap->set_pointer_size (1);
      }));

  add_connection
    (m_stone_button->signal_clicked ().connect
     ([this] ()
      {
        set_pointer_button (EditorMapWidget::STONE);
        m_bigmap->set_pointer_size (1);
      }));

  add_connection
    (m_bag_button->signal_clicked ().connect
     ([this] ()
      {
        set_pointer_button (EditorMapWidget::BAG);
        m_bigmap->set_pointer_size (1);
      }));

  add_connection
    (m_standard_button->signal_clicked ().connect
     ([this] ()
      {
        set_pointer_button (EditorMapWidget::FLAG);
        m_bigmap->set_pointer_size (1);
      }));

  add_connection
    (m_add_battle_button->signal_clicked ().connect
     ([this] ()
      {
        set_pointer_button (EditorMapWidget::FIGHT);
        m_bigmap->set_pointer_size (1);
      }));

  add_connection
    (m_tilestyle_button->signal_clicked ().connect
     ([this] ()
      {
        set_pointer_button (EditorMapWidget::TILESTYLE);
        m_bigmap->set_pointer_size (1);
      }));

  add_connection
    (m_bigmap->signal_create_new_stack ().connect
     ([this] (Vector<int> pos)
      {
        auto d = LwDialog::build<StackEditorDialog> (this);
        d->setup (NULL, pos);
        d->signal_response ().connect
          ([this, d, pos] (Gtk::ResponseType resp)
           {
             switch (resp)
               {
               case Gtk::ResponseType::ACCEPT:
                   {
                     m_umgr->add
                       (new EditorUndoAction_Stack (LwRectangle (pos)));
                     auto ns = d->get_stack ();
                     GameMap::instance ()->putStack (new Stack (*ns),
                                                     true);
                     update ();
                     break;
                   }

               default:
                 break;
               }
             delete d;
           });
      }));
}

void ScenarioBuilderWindow::update ()
{
  update_window_title ();
  update_actions ();
  m_smallmap->draw ();
  m_bigmap->queue_draw ();
}

void ScenarioBuilderWindow::update_actions ()
{
}

void ScenarioBuilderWindow::update_window_title ()
{
  Glib::ustring title = "";
  if (m_scenario_modified || m_new_scenario_needs_saving)
    title += "*";
  if (m_scenario)
    {
      m_notebook->set_current_page (1);
      title += m_scenario->getName ();
      m_subtitle->set_text (title);
    }
  else
    {
      m_notebook->set_current_page (0);
      m_subtitle->set_text ("");
    }
}

void ScenarioBuilderWindow::populate ()
{
  auto root = Gtk::make_managed<Gtk::Box> (Gtk::Orientation::HORIZONTAL);
  root->set_spacing (2);

  auto map_box = create_map_box ();
  root->append (*map_box);

  auto right_pane = create_right_pane ();
  root->append (*right_pane);

  set_child (*root);
}

Gtk::ToggleButton* ScenarioBuilderWindow::setup_toggle_button (Glib::ustring tooltip)
{
  auto button = Gtk::make_managed<Gtk::ToggleButton> ();
  button->add_css_class ("editor-toggle-button");
  button->set_valign (Gtk::Align::START);
  button->set_group (*m_pointer_button);
  button->set_tooltip_text (tooltip);
  return button;
}

Gtk::FlowBox *ScenarioBuilderWindow::create_button_box ()
{
  m_button_box = Gtk::make_managed<Gtk::FlowBox> ();
  m_button_box->set_valign (Gtk::Align::START);
  m_button_box->set_orientation (Gtk::Orientation::HORIZONTAL);
  m_button_box->set_selection_mode (Gtk::SelectionMode::NONE);
  m_button_box->set_min_children_per_line (6);
  m_button_box->set_max_children_per_line (6);
  auto box = m_button_box;

  m_pointer_button = Gtk::make_managed<Gtk::ToggleButton> ();
  m_pointer_button->add_css_class ("editor-toggle-button");
  m_pointer_button->set_tooltip_text (_("Select objects"));
  box->append (*m_pointer_button);

  m_move_button = setup_toggle_button (_("Move object"));
  box->append (*m_move_button);

  m_erase_button = setup_toggle_button (_("Remove object"));
  box->append (*m_erase_button);

  m_stack_button = setup_toggle_button (_("Add a stack"));
  box->append (*m_stack_button);

  m_city_button = setup_toggle_button (_("Add a city"));
  box->append (*m_city_button);

  m_ruin_button = setup_toggle_button (_("Add a ruin"));
  box->append (*m_ruin_button);

  m_temple_button = setup_toggle_button (_("Add a temple"));
  box->append (*m_temple_button);

  m_port_button = setup_toggle_button (_("Add a port"));
  box->append (*m_port_button);

  m_sign_button = setup_toggle_button (_("Add a signpost"));
  box->append (*m_sign_button);

  m_road_button = setup_toggle_button (_("Draw a road"));
  box->append (*m_road_button);

  m_bridge_button = setup_toggle_button (_("Draw a bridge"));
  box->append (*m_bridge_button);

  m_stone_button = setup_toggle_button (_("Add a standing stone"));
  box->append (*m_stone_button);

  m_bag_button = setup_toggle_button (_("Add a bag"));
  box->append (*m_bag_button);

  m_standard_button = setup_toggle_button (_("Add a planted standard"));
  box->append (*m_standard_button);

  m_add_battle_button =
    setup_toggle_button (_("Add stack to battle calculator"));
  box->append (*m_add_battle_button);

  m_tilestyle_button =
    setup_toggle_button (_("Modify the tile graphic"));
  box->append (*m_tilestyle_button);

  draw_buttons ();
  return box;
}

Gtk::Box *ScenarioBuilderWindow::create_map_box ()
{
  auto box = Gtk::make_managed<Gtk::Box> (Gtk::Orientation::VERTICAL);
  box->set_hexpand (true);
  box->set_vexpand (true);
  m_bigmap = new EditorMapWidget ();
  m_bigmap->set_hexpand (true);
  m_bigmap->set_vexpand (true);
  box->append (*m_bigmap);
  return box;
}

Gtk::Box *ScenarioBuilderWindow::create_right_pane ()
{
  auto box = Gtk::make_managed<Gtk::Box> (Gtk::Orientation::VERTICAL);
  m_map_drawing_area = Gtk::make_managed<Gtk::DrawingArea> ();
  m_map_drawing_area->set_valign (Gtk::Align::FILL);
  m_map_drawing_area->set_halign (Gtk::Align::CENTER);
  box->append (*m_map_drawing_area);
  m_smallmap = new SmallMap (false);
  auto button_box = create_button_box ();
  box->append (*button_box);
  return box;
}

UndoAction* ScenarioBuilderWindow::execute_action (UndoAction *action2)
{
  EditorUndoAction *action = dynamic_cast<EditorUndoAction*>(action2);
  UndoAction *out = NULL;

  switch (action->get_type ())
    {
    case EditorUndoAction::CHANGE_PROPERTIES:
        {
          auto a = dynamic_cast<EditorUndoAction_Properties*>(action);
          out = new EditorUndoAction_Properties
            (m_scenario->getName (),
             m_scenario->getComment (),
             m_scenario->getCopyright (),
             m_scenario->getLicense ());
          m_scenario->setName (a->get_name ());
          m_scenario->setComment (a->get_description ());
          m_scenario->setCopyright (a->get_copyright ());
          m_scenario->setLicense (a->get_license ());
          break;
        }

    case EditorUndoAction::SCENARIO_MEDIA:
        {
          auto a = dynamic_cast<EditorUndoAction_ScenarioMedia*>(action);
          out = new EditorUndoAction_ScenarioMedia (m_scenario);
          reload_scenario (a);
          break;
        }

    case EditorUndoAction::TERRAIN:
        {
          auto a = dynamic_cast<EditorUndoAction_Terrain*>(action);
          out = new EditorUndoAction_Terrain (a->get_tile_index (),
                                              a->get_area ());
          change_map (a);
          break;
        }

    case EditorUndoAction::ERASE:
        {
          auto a = dynamic_cast<EditorUndoAction_Erase*>(action);
          out = new EditorUndoAction_Erase (a->get_area ());
          change_map (a);
          break;
        }

    case EditorUndoAction::MOVE:
        {
          auto a = dynamic_cast<EditorUndoAction_Move*>(action);

          LwRectangle r1 = a->get_rectangles ().front ();
          LwRectangle r2 = a->get_rectangles ().back ();
          out = new EditorUndoAction_Move (r1, r2);
          change_map (a);
          break;
        }

    case EditorUndoAction::STACK:
        {
          auto a = dynamic_cast<EditorUndoAction_Stack*>(action);
          out = new EditorUndoAction_Stack (a->get_area ());
          change_map (a);
          break;
        }

    case EditorUndoAction::CITY:
        {
          auto a = dynamic_cast<EditorUndoAction_City*>(action);
          out = new EditorUndoAction_City (a->get_area ());
          change_map (a);
          break;
        }

    case EditorUndoAction::RUIN:
        {
          auto a = dynamic_cast<EditorUndoAction_Ruin*>(action);
          out = new EditorUndoAction_Ruin (a->get_area ());
          change_map (a);
          break;
        }

    case EditorUndoAction::TEMPLE:
        {
          auto a = dynamic_cast<EditorUndoAction_Temple*>(action);
          out = new EditorUndoAction_Temple (a->get_area ());
          change_map (a);
          break;
        }

    case EditorUndoAction::STONE:
        {
          auto a = dynamic_cast<EditorUndoAction_Stone*>(action);
          out = new EditorUndoAction_Stone (a->get_area ());
          change_map (a);
          break;
        }

    case EditorUndoAction::SIGNPOST:
        {
          auto a = dynamic_cast<EditorUndoAction_Signpost*>(action);
          out = new EditorUndoAction_Signpost (a->get_area ());
          change_map (a);
          break;
        }

    case EditorUndoAction::PORT:
        {
          auto a = dynamic_cast<EditorUndoAction_Port*>(action);
          out = new EditorUndoAction_Port (a->get_area ());
          change_map (a);
          break;
        }

    case EditorUndoAction::BRIDGE:
        {
          auto a = dynamic_cast<EditorUndoAction_Bridge*>(action);
          out = new EditorUndoAction_Bridge (a->get_area ());
          change_map (a);
          break;
        }

    case EditorUndoAction::ROAD:
        {
          auto a = dynamic_cast<EditorUndoAction_Road*>(action);
          out = new EditorUndoAction_Road (a->get_area ());
          change_map (a);
          break;
        }

    case EditorUndoAction::FLAG:
        {
          auto a = dynamic_cast<EditorUndoAction_Flag*>(action);
          out = new EditorUndoAction_Flag (a->get_area ());
          change_map (a);
          break;
        }

    case EditorUndoAction::EDIT_FLAG:
        {
          auto a = dynamic_cast<EditorUndoAction_EditFlag*>(action);
          out = new EditorUndoAction_EditFlag (a->get_area ());
          change_map (a);
          break;
        }

    case EditorUndoAction::EDIT_STACK:
        {
          auto a = dynamic_cast<EditorUndoAction_EditStack*>(action);
          out = new EditorUndoAction_EditStack (a->get_area ());
          change_map (a);
          break;
        }

    case EditorUndoAction::EDIT_CITY:
        {
          auto a = dynamic_cast<EditorUndoAction_EditCity*>(action);
          out = new EditorUndoAction_EditCity (a->get_area ());
          change_map (a);
          break;
        }

    case EditorUndoAction::EDIT_RUIN:
        {
          auto a = dynamic_cast<EditorUndoAction_EditRuin*>(action);
          out = new EditorUndoAction_EditRuin (a->get_area ());
          change_map (a);

          auto pos = a->get_area ().pos;
          auto ruin = GameMap::getRuin (pos);
          GameMap::instance ()->update_ruin_rewards (pos, ruin);
          break;
        }

    case EditorUndoAction::EDIT_TEMPLE:
        {
          auto a = dynamic_cast<EditorUndoAction_EditTemple*>(action);
          out = new EditorUndoAction_EditTemple (a->get_area ());
          change_map (a);
          break;
        }

    case EditorUndoAction::EDIT_STONE:
        {
          auto a = dynamic_cast<EditorUndoAction_EditStone*>(action);
          out = new EditorUndoAction_EditStone (a->get_area ());
          change_map (a);
          break;
        }

    case EditorUndoAction::EDIT_SIGNPOST:
        {
          auto a = dynamic_cast<EditorUndoAction_EditSignpost*>(action);
          out = new EditorUndoAction_EditSignpost (a->get_area ());
          change_map (a);
          break;
        }

    case EditorUndoAction::EDIT_ROAD:
        {
          auto a = dynamic_cast<EditorUndoAction_EditRoad*>(action);
          out = new EditorUndoAction_EditRoad (a->get_area ());
          change_map (a);
          break;
        }

    case EditorUndoAction::EDIT_BACKPACK:
        {
          auto a = dynamic_cast<EditorUndoAction_EditBackpack*>(action);
          out = new EditorUndoAction_EditBackpack (a->get_area ());
          change_map (a);
          break;
        }

    case EditorUndoAction::MINIMAP:
        {
          auto a = dynamic_cast<EditorUndoAction_MiniMap *>(action);
          out = new EditorUndoAction_MiniMap (m_scenario);
          reload_scenario (a);
          break;
        }

    case EditorUndoAction::PLAYERS:
        {
          auto a = dynamic_cast<EditorUndoAction_Players*>(action);
          out = new EditorUndoAction_Players (m_scenario);
          reload_scenario (a);
          m_shield_menu_button->reload ();
          break;
        }

    case EditorUndoAction::ITEMS:
        {
          auto a = dynamic_cast<EditorUndoAction_Items*>(action);
          out = new EditorUndoAction_Items (m_scenario);
          reload_scenario (a);
          break;
        }

    case EditorUndoAction::REWARDS:
        {
          auto a = dynamic_cast<EditorUndoAction_Rewards*>(action);
          out = new EditorUndoAction_Rewards (m_scenario);
          reload_scenario (a);
          break;
        }

    case EditorUndoAction::SMOOTH:
        {
          auto a = dynamic_cast<EditorUndoAction_Smooth*>(action);
          out = new EditorUndoAction_Smooth (a->get_area ());
          change_map (a);
          break;
        }

    case EditorUndoAction::SWITCH_SETS:
        {
          auto a = dynamic_cast<EditorUndoAction_SwitchSets*>(action);
          out = new EditorUndoAction_SwitchSets (m_scenario);
          reload_scenario (a);
          break;
        }

    case EditorUndoAction::TILESET:
        {
          auto a = dynamic_cast<EditorUndoAction_TileSet*>(action);
          out = new EditorUndoAction_TileSet (GameMap::getTileset ());
          reload_tileset (a);
          break;
        }

    case EditorUndoAction::CITYSET:
        {
          auto a = dynamic_cast<EditorUndoAction_CitySet*>(action);
          out = new EditorUndoAction_CitySet (GameMap::getCityset ());
          reload_cityset (a);
          break;
        }

    case EditorUndoAction::SHIELDSET:
        {
          auto a = dynamic_cast<EditorUndoAction_ShieldSet*>(action);
          out = new EditorUndoAction_ShieldSet (GameMap::getShieldset ());
          reload_shieldset (a);
          break;
        }

    case EditorUndoAction::ARMYSET:
        {
          auto a = dynamic_cast<EditorUndoAction_ArmySet*>(action);
          Player *p = Playerlist::getActiveplayer ();
          Armyset *ar = Armysetlist::instance ()->get (p->getArmyset ());
          out = new EditorUndoAction_ArmySet (ar);
          reload_armyset (a);
          break;
        }

    case EditorUndoAction::FIGHT_ORDER:
        {
          auto a = dynamic_cast<EditorUndoAction_FightOrder*>(action);
          out = new EditorUndoAction_FightOrder (m_scenario);
          reload_scenario (a);
          break;
        }

    case EditorUndoAction::REMOVE_STACKS:
        {
          auto a = dynamic_cast<EditorUndoAction_RemoveStacks*>(action);
          out = new EditorUndoAction_RemoveStacks (m_scenario);
          reload_scenario (a);
          break;
        }

    case EditorUndoAction::RANDOMIZE_OBJECTS:
        {
          auto a = dynamic_cast<EditorUndoAction_RandomizeObjects*>(action);
          out = new EditorUndoAction_RandomizeObjects (m_scenario);
          reload_scenario (a);
          break;
        }

    case EditorUndoAction::ASSIGN_CAPITALS:
        {
          auto a = dynamic_cast<EditorUndoAction_AssignCapitals*>(action);
          out = new EditorUndoAction_AssignCapitals (m_scenario);
          reload_scenario (a);
          break;
        }

    case EditorUndoAction::TILESTYLE:
        {
          auto a = dynamic_cast<EditorUndoAction_TileStyle*>(action);
          auto pos = a->get_pos ();
          auto mtile = GameMap::instance ()->getTile (pos);
          auto old_id = mtile->getTileStyleId ();
          out = new EditorUndoAction_TileStyle (pos, old_id);
          mtile->setTileStyleId (a->get_tilestyle_id ());
          break;
        }
    }
  return out;
}

void ScenarioBuilderWindow::setup_accels ()
{
  auto app = Lw::app;
  auto idx = std::string (LW_APP_ID).rfind ('.');
  std::string prefix = std::string (LW_APP_ID).substr (idx + 1);
  app->set_accel_for_action (prefix + "." + "editor.edit.undo",
                             "<Control>Z");
  app->set_accels_for_action (prefix + "." + "editor.edit.redo",
                              {"<Control>Y", "<Control><Shift>Z"});
  app->set_accels_for_action (prefix + "." + "editor.file.new",
                              {"<Control>N"});
  app->set_accels_for_action (prefix + "." + "editor.file.open",
                              {"<Control>O"});
  app->set_accels_for_action (prefix + "." + "editor.file.save",
                              {"<Control>S"});
  app->set_accels_for_action (prefix + "." + "editor.file.save-as",
                              {"<Control><Shift>S"});
  app->set_accels_for_action (prefix + "." + "editor.file.quit",
                              {"<Control>Q"});
}

void ScenarioBuilderWindow::destroy_accels ()
{
  std::list<std::string> accels =
    {
        {"editor.edit.undo"},
        {"editor.edit.redo"}
    };
  auto idx = std::string (LW_APP_ID).rfind ('.');
  std::string prefix = std::string (LW_APP_ID).substr (idx + 1);
  for (auto accel : accels)
    Lw::app->set_accel_for_action (prefix + "." + accel, {});
}

void ScenarioBuilderWindow::setup_dark_mode_change ()
{
  m_dark = Lw::get_dark ();
  auto iface = Gio::Settings::create ("org.gnome.desktop.interface",
                                      "/org/gnome/desktop/interface/");
  if (iface)
    {
      m_dark_style_handler =
        iface->signal_changed ().connect
        ([this, iface] (const Glib::ustring &key)
         {
           (void)key;
           auto scheme = iface->get_string ("color-scheme");
           if (scheme == "prefer-dark")
             m_dark = true;
           else
             m_dark = false;
           draw_pointer_buttons ();
           draw_buttons ();
         });
    }
}

void ScenarioBuilderWindow::set_button_image (Gtk::Button *button, std::string file)
{
  if (button->get_child ())
    button->unset_child ();
  auto image = Gtk::make_managed<Gtk::Image> ();
  image->set (file);
  image->set_pixel_size (get_editor_button_size ());
  button->set_child (*image);
  button->add_css_class ("editor-control-button");
}

void ScenarioBuilderWindow::add_terrain_buttons ()
{
  disconnect_terrain_button_signals ();
  auto box = m_button_box;

  int i = 0;
  int active = -1;
  for (auto button : m_terrain_buttons)
    {
      if (button->get_active ())
        active = i;
      box->remove (*button);
    }

  m_terrain_buttons.clear ();

  auto tileset = GameMap::getTileset ();
  for (auto it = tileset->rbegin (); it != tileset->rend (); ++it)
    {
      Tile *tile = (*it);
      if (tile->empty ())
        continue;
      auto button = Gtk::make_managed<Gtk::ToggleButton>();
      button->add_css_class ("editor-control-button");
      button->set_group (*m_pointer_button);
      button->set_valign (Gtk::Align::START);
      button->set_tooltip_text
        (String::ucompose (_("Draw %1"), tile->getName ()));
      if (button->get_child ())
        button->unset_child ();
      auto image = Gtk::make_managed<Gtk::Image> ();
      auto pix = tile->front ()->front ()->getImage ();
      image->set_pixel_size (get_editor_button_size ());
      image->set (pix->to_texture ());
      button->set_child (*image);
      m_terrain_buttons.push_back (button);
      box->prepend (*button);
    }
  if (active >= 0)
    m_terrain_buttons[tileset->size () - active - 1]->set_active (true);

  connect_terrain_button_signals ();
}

void ScenarioBuilderWindow::connect_terrain_button_signals ()
{
  int tile_idx = GameMap::getTileset ()->size ();
  for (auto button : m_terrain_buttons)
    {
      tile_idx--;
      add_terrain_button_connection
        (button->signal_clicked ().connect
         ([this, tile_idx] ()
          {
            m_bigmap->set_pointer_terrain (tile_idx);
            set_pointer_button (EditorMapWidget::TERRAIN);
          }));
    }
}

void ScenarioBuilderWindow::add_terrain_button_connection (sigc::connection c)
{
  m_terrain_button_connections.push_back (c);
}

void ScenarioBuilderWindow::disconnect_terrain_button_signals ()
{
  for (auto conn : m_terrain_button_connections)
    conn.disconnect ();
  m_terrain_button_connections.clear ();
}

void ScenarioBuilderWindow::set_pointer_button (enum EditorMapWidget::Pointer pointer)
{
  m_bigmap->set_pointer (pointer);
  if (pointer == m_pointer)
    return;
  block_signals ();
  block_terrain_button_signals ();

  //update the pointer menu button
  switch (pointer)
    {
    case EditorMapWidget::ERASE:
    case EditorMapWidget::TERRAIN:
      m_pointer_size_menu_button->set_sensitive (true);
      m_pointer_size_menu_button->set_active (1);
      break;

    case EditorMapWidget::POINTER:
    case EditorMapWidget::STACK:
    case EditorMapWidget::CITY:
    case EditorMapWidget::RUIN:
    case EditorMapWidget::TEMPLE:
    case EditorMapWidget::SIGNPOST:
    case EditorMapWidget::ROAD:
    case EditorMapWidget::MOVE:
    case EditorMapWidget::PORT:
    case EditorMapWidget::BRIDGE:
    case EditorMapWidget::BAG:
    case EditorMapWidget::FIGHT:
    case EditorMapWidget::STONE:
    case EditorMapWidget::FLAG:
    case EditorMapWidget::TILESTYLE:
      m_pointer_size_menu_button->set_sensitive (false);
      break;

    case EditorMapWidget::UNKNOWN:
      break;
    }

  //update the shield menu button
  switch (pointer)
    {
    case EditorMapWidget::STACK:
    case EditorMapWidget::CITY:
    case EditorMapWidget::FLAG:
      m_shield_menu_button->set_sensitive (true);
      m_shield_menu_button->set_active (Shield::WHITE);
      break;

    case EditorMapWidget::ERASE:
    case EditorMapWidget::TERRAIN:
    case EditorMapWidget::POINTER:
    case EditorMapWidget::RUIN:
    case EditorMapWidget::TEMPLE:
    case EditorMapWidget::SIGNPOST:
    case EditorMapWidget::ROAD:
    case EditorMapWidget::MOVE:
    case EditorMapWidget::PORT:
    case EditorMapWidget::BRIDGE:
    case EditorMapWidget::BAG:
    case EditorMapWidget::FIGHT:
    case EditorMapWidget::STONE:
    case EditorMapWidget::TILESTYLE:
      m_shield_menu_button->set_sensitive (false);
      break;

    case EditorMapWidget::UNKNOWN:
      break;
    }
  unblock_terrain_button_signals ();
  unblock_signals ();
  m_pointer = pointer;
}

void ScenarioBuilderWindow::create_blank_scenario ()
{
  std::vector<Glib::ustring> armysets;
  std::vector<bool> players_active;
  for (guint32 i = 0; i < MAX_PLAYERS + 1; i++)
    {
      armysets.push_back ("default");
      players_active.push_back (true);
    }
  set_filled_map (MAP_SIZE_NORMAL_WIDTH, MAP_SIZE_NORMAL_HEIGHT,
                  Tile::WATER, armysets, "default", "default",
                  "default", players_active);
  m_umgr->clear ();
}

void ScenarioBuilderWindow::set_filled_map (int width, int height,
                                            int fill_style,
                                            std::vector<Glib::ustring> armysets,
                                            std::string cityset,
                                            std::string shieldset,
                                            std::string tileset,
                                            std::vector<bool> players_active)
{
  if (m_create_scenario_names)
    {
      delete m_create_scenario_names;
      m_create_scenario_names = NULL;
    }
  ImageCache::deleteInstance ();

  m_width = width;
  m_height = height;

  if (m_scenario)
    delete m_scenario;

  GameMap::deleteInstance ();
  GameMap::setWidth (width);
  GameMap::setHeight (height);
  GameMap::instance  (tileset, shieldset, cityset);
  Itemlist::createStandardInstance ();

  Glib::ustring scenario_name =
    ScenarioList::instance ()->find_free_name (_("Untitled"));
  m_scenario = new GameScenario (scenario_name, "");
  setup_create_scenario_randomize ();

  // ...however we need to do some of the setup by hand. We need to create a
  // neutral player to give cities a player upon creation...

  for (guint32 i = Shield::WHITE; i <= Shield::BLACK; i++)
    {
      if (players_active[i])
        {
          guint32 armyset_id =
            Armysetlist::instance ()->get (armysets[i])->getId ();
          Glib::ustring name =
            m_create_scenario_names->getPlayerName (Shield::Color (i));
          Player *human = new RealPlayer (name, armyset_id,
                                          Shield::Color (i), width,
                                          height, Player::HUMAN);
          Playerlist::instance ()->add (human);
        }
    }

  Glib::ustring name =
    m_create_scenario_names->getPlayerName (Shield::NEUTRAL);

  guint32 neutral_armyset_id =
    Armysetlist::instance ()->get (armysets[MAX_PLAYERS])->getId ();
  Player* neutral =
    new AI_Dummy (name, neutral_armyset_id, Shield::NEUTRAL, width, height);
  Playerlist::instance ()->add (neutral);
  Playerlist::instance ()->setNeutral (neutral);
  Playerlist::instance ()->nextPlayer ();

  GameMap::instance ()->fill (fill_style);

  GameMap::instance ()->calculateBlockedAvenues ();
  File::erase (get_default_map_filename ());
  Playerlist::instance ()->setActiveplayer (Playerlist::getNeutral ());
  m_scenario->dump (get_default_map_filename (), MAP_EXT);
  m_scenario->created (get_default_map_filename ());

  m_shield_menu_button->reload ();
  add_terrain_buttons ();
  m_smallmap->resize ();
  m_pointer_button->set_active (true);
  update ();
}

std::string ScenarioBuilderWindow::get_default_map_filename ()
{
  return
    File::add_slash_if_necessary (File::getCacheDir ()) +
    "current" + MAP_EXT;
}

void ScenarioBuilderWindow::setup_create_scenario_randomize ()
{
  if (m_create_scenario_names)
    delete m_create_scenario_names;
  m_create_scenario_names = new CreateScenarioRandomize ();
  m_create_scenario_names->signal_collect_city_names ().connect
    ([this] ()
     {
       std::list<Glib::ustring> names;
       for (auto c : *Citylist::instance ())
         names.push_back (c->getName ());
       return names;
     });

}

void ScenarioBuilderWindow::setup_smallmap ()
{
  struct MouseMotionEvent empty_event{};
  m_smallmap_mouse_motion = empty_event;

  auto motion = Gtk::EventControllerMotion::create ();
  motion->signal_motion ().connect
    ([this] (double x, double y)
     {
       struct MouseMotionEvent *ev = &m_smallmap_mouse_motion;
       ev->pos = Vector<int>(std::round (x), std::round (y));
       m_smallmap->mouse_motion_event (*ev);
     });
  m_map_drawing_area->add_controller (motion);

  auto click = Gtk::GestureClick::create ();
  click->signal_pressed ().connect
    ([this] (int n, double x, double y)
     {
       struct MouseButtonEvent ev = to_input_event (n, x, y, true);
       struct MouseMotionEvent *evmotion = &m_smallmap_mouse_motion;
       evmotion->pressed[ev.button] = true;
       m_smallmap->mouse_button_event (ev);
     });

  click->signal_released ().connect
    ([this] (int n, double x, double y)
     {
       struct MouseButtonEvent ev = to_input_event (n, x, y, false);
       struct MouseMotionEvent *evmotion = &m_smallmap_mouse_motion;
       evmotion->pressed[ev.button] = false;
       m_smallmap->mouse_button_event (ev);
     });
  m_map_drawing_area->add_controller (click);

  m_map_drawing_area->set_draw_func
    ([this](const Cairo::RefPtr<Cairo::Context>& cr, int, int)
     {
       if (!m_smallmap_surface)
         return;

       cr->set_source (m_smallmap_surface, 0, 0);
       cr->paint ();
     });

  ImageCache::CursorType c = ImageCache::MAGNIFYING_GLASS;
  auto hotspot = ImageCache::get_hotspot (c);
  auto im = ImageCache::instance ()->getCursorPic (c);
  auto cursor = Gdk::Cursor::create (im->to_texture (),
                                     hotspot.x, hotspot.y);
  m_map_drawing_area->set_cursor (cursor);
}

void ScenarioBuilderWindow::check_discard (Glib::ustring msg,
                                           sigc::slot<void(bool)> after)
{
  if (m_scenario_modified || m_new_scenario_needs_saving)
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
                 ([this, after] (bool valid)
                  {
                    if (!valid)
                      return after (false);

                    if (m_current_save_filename == "")
                      {
                        save_current_scenario_as
                          ([this, after] (bool saved)
                           {
                             after (saved);
                           });
                      }
                    else
                      {
                        save_current_scenario_file
                          ("",
                           [this, after] (bool saved)
                           {
                             after (saved);
                           });
                      }
                  });
               break;

             case 1: //discard
               after (true);
               break;

             default:
               after (false); //cancel
               break;
             }
         });
    }
  else
    after (true);
  return;
}

void ScenarioBuilderWindow::check_save_valid (sigc::slot<void(bool)> after)
{
  std::list<Glib::ustring> errors, warnings;
  if (m_scenario->validate (errors, warnings) == false)
    {
      Glib::ustring msg =
        _("The scenario is invalid.  Do you want to proceed?");
      auto d = LwDialog::alert_yn (msg);
      d->choose
        (*this,
         [after, d] (auto result)
         {
           bool ret = d->choose_finish (result) == 1;
           after (ret);
         });
    }
  else
    return after (true);
  return;
}

void ScenarioBuilderWindow::save_current_scenario_as (sigc::slot<void(bool)> after)
{
  LwDialog::save
    (*this, _("Choose a Name"),
     File::sanify (m_scenario->getName ()), FileFilter::SCENARIO,
     [this, after] (std::string path)
     {
       Glib::ustring old_filename = m_current_save_filename;

       save_current_scenario_file
         (path,
          [this, after, old_filename] (bool saved)
          {
            if (saved == false)
              {
                m_current_save_filename = old_filename;
              }
            else
              {
                m_scenario_modified = false;
                m_new_scenario_needs_saving = false;
                update_window_title ();
                m_scenario->moved (m_current_save_filename);
              }
            after (saved);
          });
     });
}

void ScenarioBuilderWindow::save_current_scenario_file (std::string filename,
                                                        sigc::slot<void(bool)> after)
{
  m_current_save_filename = filename;
  if (m_current_save_filename.empty ())
    m_current_save_filename = m_scenario->getConfigurationFile (true);

  bool ok = m_scenario->saveGame (m_current_save_filename, MAP_EXT);
  if (ok)
    {
      m_new_scenario_needs_saving = false;
      m_scenario_modified = false;
      m_scenario->moved (m_current_save_filename);
      update_window_title ();
      after (true);
      return;
    }
  else
    {
      Glib::ustring errmsg = Glib::strerror (errno);
      Glib::ustring msg = _("The scenario could not be saved.");

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
}

void ScenarioBuilderWindow::check_quit (sigc::slot<void(bool)> after)
{
  if (m_scenario_modified || m_new_scenario_needs_saving ||
      m_umgr->undo_empty () == false)
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
             case 0: //save and exit
               check_save_valid
                 ([this, after] (bool valid)
                  {
                    if (!valid)
                      return after (false);

                    if (m_current_save_filename == "")
                      {
                        save_current_scenario_as
                          ([this, after] (bool saved)
                           {
                             after (saved);
                           });
                      }
                    else
                      {
                        save_current_scenario_file
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

void ScenarioBuilderWindow::setup_quit ()
{
  auto controller = Gtk::EventControllerKey::create ();
  controller->signal_key_pressed().connect
    ([this] (guint keyval, guint, Gdk::ModifierType)
     {
       if (keyval == GDK_KEY_Escape)
         {
           m_simple_actions["editor.file.quit"]->activate ();
           return true;
         }
       return false;
     }, false);
  add_controller (controller);

  signal_close_request ().connect
    ([this] () -> bool
     {
       m_simple_actions["editor.file.quit"]->activate ();
       return true;
     }, false);
}

void ScenarioBuilderWindow::on_save_as_activated ()
{
  check_save_valid
    ( [this] (bool valid)
      {
        if (valid)
          {
            save_current_scenario_as
              ([this] (bool saved)
               {
                 (void) saved;
               });
          }
      });

}

void ScenarioBuilderWindow::setup_bigmap ()
{
  auto motion = Gtk::EventControllerMotion::create ();
  motion->signal_leave ().connect
    ([this] ()
     {
       m_pointer_position_label->set_text ("");
     });
  m_bigmap->add_controller (motion);
}

guint32 ScenarioBuilderWindow::get_editor_button_size ()
{
  return LW_BUTTON_SIZE / 1.6667;
}

void ScenarioBuilderWindow::change_map (EditorUndoAction_ChangeMap *action)
{
  GameMap::instance  ()->updateMaptiles (action->get_map_tiles ());
  m_smallmap->resize ();
  if (action->get_only_maptiles ())
    return;

  GameMap::instance  ()->updateObjects (action->get_objects (),
                                        action->get_rectangles ());
  action->clear_objects ();
}

void ScenarioBuilderWindow::reload_scenario (EditorUndoAction_Save *action)
{
  Glib::ustring olddir = m_scenario->getDirectory ();
  Glib::ustring oldname =
    File::get_basename (m_scenario->getConfigurationFile (true));
  Glib::ustring oldext = m_scenario->getExtension ();
  delete m_scenario;
  Scenario::reset (action->get_scenario ());
  m_scenario = action->get_scenario ()->get_game_scenario ();
  m_scenario->setUnique (true);
  File::copy (action->get_scenario_filename (),
              m_scenario->getConfigurationFile (true));
  m_scenario->setDirectory (olddir);
  m_scenario->setBaseName (oldname);
  m_scenario->setExtension (oldext);
  m_smallmap->resize ();
  add_terrain_buttons ();
  m_shield_menu_button->reload ();
  GameMap::instance ()->applyTileStyles (0, 0, m_height, m_width, false);
}

void ScenarioBuilderWindow::reload_armyset (EditorUndoAction_ArmySet *a)
{
  Armyset *ar = a->get_armyset ();
  File::copy (a->get_armyset_filename (),
              ar->getConfigurationFile (true));
  Armysetlist::instance  ()->replace (ar);
  reload_armyset (ar->getId ());
}

void ScenarioBuilderWindow::reload_armyset (guint32 id)
{
  ImageCache::instance ()->reset();
  //we're doing reload before, because we need the maps to be updated.
  //but then the armyset* gets changed and the switch has no effect.
  Armysetlist::instance ()->reload (id);

  for (auto p : Playerlist::instance ()->getPlayersWithArmyset (id))
    GameMap::instance ()->switchArmysets
      (p, Armysetlist::instance ()->get (id));
  m_bigmap->queue_draw ();
}

void ScenarioBuilderWindow::reload_cityset (EditorUndoAction_CitySet *a)
{
  Cityset *c = a->get_cityset ();
  File::copy (a->get_cityset_filename (),
              c->getConfigurationFile (true));
  Citysetlist::instance ()->replace (c);
  reload_cityset ();
}

void ScenarioBuilderWindow::reload_cityset ()
{
  ImageCache::instance ()->reset ();
  GameMap::resetCityset ();
  GameMap::instance ()->reloadCityset ();
  m_bigmap->queue_draw ();
}

void ScenarioBuilderWindow::reload_tileset (EditorUndoAction_TileSet *a)
{
  Tileset *t = a->get_tileset ();
  File::copy (a->get_tileset_filename (),
              t->getConfigurationFile (true));
  Tilesetlist::instance ()->replace (t);
  reload_cityset (t->getId ());
}

void ScenarioBuilderWindow::reload_cityset (guint32 id)
{
  ImageCache::instance ()->reset ();
  GameMap::resetTileset ();
  Tilesetlist::instance ()->reload (id);
  GameMap::instance ()->switchTileset
    (Tilesetlist::instance ()->get (id));
  m_smallmap->resize ();
  m_bigmap->queue_draw ();
  add_terrain_buttons ();
  m_pointer_button->set_active (true);
}

void ScenarioBuilderWindow::reload_shieldset (EditorUndoAction_ShieldSet *a)
{
  Shieldset *s = a->get_shieldset ();
  File::copy (a->get_shieldset_filename (),
              s->getConfigurationFile (true));
  Shieldsetlist::instance ()->replace (s);
  reload_shieldset ();
}

void ScenarioBuilderWindow::reload_shieldset ()
{
  ImageCache::instance ()->reset ();
  GameMap::resetShieldset ();
  GameMap::instance ()->reloadShieldset ();
  m_smallmap->resize ();
  m_shield_menu_button->reload ();
  m_bigmap->queue_draw ();
}
                                   
void ScenarioBuilderWindow::set_random_map (NewMapDialog *d)
{
  auto w = Gtk::make_managed<EditorLoadWindow> ();
  w->setup ();
  w->present ();
  w->set_modal (true);
  w->set_transient_for (*this);

  auto g = d->get_map ();

  if (m_scenario)
    delete m_scenario;
  GameMap::deleteInstance ();
  GameMap::setWidth (g.map.width);
  GameMap::setHeight (g.map.height);
  GameMap::instance (g.tile_theme, g.shield_theme, g.city_theme);

  w->set_fraction (0.1);
  Lw::do_events ();

  //zip past the player IDs
  if (id_counter)
    delete id_counter;
  id_counter = new ID_Counter ();

  // First we need to create a neutral player to give cities a player upon
  // creation.
  auto armysets = d->get_armyset_themes ();
  guint32 neutral_armyset_id =
    Armysetlist::instance ()->get (armysets[MAX_PLAYERS])->getId ();
  Player* neutral = 
    new AI_Dummy (m_create_scenario_names->getPlayerName
                  (Shield::NEUTRAL), neutral_armyset_id, Shield::NEUTRAL,
                  g.map.width, g.map.height);

  Playerlist::instance ()->add (neutral);
  Playerlist::instance ()->setNeutral (neutral);
  Playerlist::instance ()->nextPlayer ();

  // create a random map
  MapGenerator gen;

  // first, fill the generator with data
  gen.setNoCities (g.map.cities);
  gen.setNoRuins (g.map.ruins);
  gen.setNoTemples (g.map.temples);
  gen.setNoSignposts (g.map.signposts);
  gen.setNoStones (d->get_number_of_standing_stones ());
  gen.setChanceOfStoneOnRoad (d->get_standing_stone_on_road_chance ());

  // if sum > 100 (percent), divide everything by a factor, the numeric
  // error is said to be grass
  int sum =
    g.map.grass + g.map.water + g.map.forest + g.map.swamp +
    g.map.hills + g.map.mountains;

  if (sum > 100)
    {
      double factor = 100 / static_cast<double>(sum);
      g.map.water = static_cast<int>(g.map.water / factor);
      g.map.forest = static_cast<int>(g.map.forest / factor);
      g.map.swamp = static_cast<int>(g.map.swamp / factor);
      g.map.hills = static_cast<int>(g.map.hills / factor);
      g.map.mountains = static_cast<int>(g.map.mountains / factor);
    }

  gen.setPercentages (g.map.water, g.map.forest, g.map.swamp,
                      g.map.hills, g.map.mountains);

  w->set_fraction (0.2);
  Lw::do_events ();

  gen.setCityset (GameMap::getCityset ());
  gen.makeMap (g.map.width, g.map.height, d->get_random_roads ());
  GameMap::deleteInstance ();
  GameMap::setWidth (g.map.width);
  GameMap::setHeight (g.map.height);
  GameMap::instance (g.tile_theme, g.shield_theme, g.city_theme);
  GameMap::instance ()->fill (&gen);

  w->set_fraction (0.3);
  Lw::do_events ();

  Itemlist::createStandardInstance ();
  Glib::ustring scenario_name =
    ScenarioList::instance ()->find_free_name (_("Untitled"));
  m_scenario = new GameScenario (scenario_name, "");
  setup_create_scenario_randomize ();

  Cityset *cs = Citysetlist::instance ()->get (g.city_theme);
  // now fill the lists
  const Maptile::Building* build =
    gen.getBuildings (g.map.width, g.map.height);
  auto cl = Citylist::instance ();
  for (int j = 0; j < g.map.height; j++)
    for (int i = 0; i < g.map.width; i++)
      switch(build[j * g.map.width + i])
        {
        case Maptile::CITY:
          cl->add
            (new City (Vector<int>(i,j), cs->getCityTileWidth ()));
          (*cl->rbegin ())->setOwner (Playerlist::getNeutral ());
          break;

        case Maptile::TEMPLE:
          Templelist::instance ()->add
            (new Temple (Vector<int>(i,j), cs->getTempleTileWidth ()));
          break;

        case Maptile::RUIN:
          Ruinlist::instance ()->add
            (new Ruin(Vector<int>(i,j), cs->getRuinTileWidth ()));
          break;

        case Maptile::SIGNPOST:
          Signpostlist::instance ()->add
            (new Signpost(Vector<int>(i, j)));
          break;

        case Maptile::ROAD:
          Roadlist::instance ()->add (new Road (Vector<int>(i,j)));
          break;

        case Maptile::BRIDGE:
          Bridgelist::instance ()->add (new Bridge (Vector<int>(i, j)));
          break;

        case Maptile::PORT:
          Portlist::instance ()->add (new Port (Vector<int>(i, j)));
          break;

        case Maptile::STONE:
          Stonelist::instance ()->add (new Stone (Vector<int>(i, j)));
          break;

        case Maptile::NONE:
          break;
        }

  w->set_fraction (0.4);
  Lw::do_events ();

  auto active_players = d->get_players ();
  for (guint32 i = Shield::WHITE; i <= Shield::BLACK; i++)
    {
      if (active_players[i])
        {
          guint32 armyset_id =
            Armysetlist::instance ()->get (armysets[i])->getId ();
          Glib::ustring name =
            m_create_scenario_names->getPlayerName (Shield::Color (i));
          Player *human = new RealPlayer (name, armyset_id,
                                          Shield::Color (i),
                                          g.map.width, g.map.height,
                                          Player::HUMAN);
          Playerlist::instance ()->add (human);
        }
    }

    {
      //put neutral on the end
      auto pl = Playerlist::instance ();
      auto first = pl->front ();
      pl->erase (pl->begin ());
      pl->push_back (first);
    }

  w->set_fraction (0.5);
  Lw::do_events ();


  File::erase (get_default_map_filename ());
  Playerlist::instance ()->setActiveplayer (Playerlist::getNeutral ());
  m_scenario->dump (get_default_map_filename (), MAP_EXT);

  w->set_fraction (0.6);
  Lw::do_events ();

  m_scenario->created (get_default_map_filename ());
  if (d->get_random_names ())
    {
      m_simple_actions["editor.randomize.all-cities"]->activate ();
      m_simple_actions["editor.randomize.all-ruins"]->activate ();
      m_simple_actions["editor.randomize.all-temples"]->activate ();
      m_simple_actions["editor.randomize.all-signs"]->activate ();
    }
  if (d->get_random_roads ())
    for (auto pos : gen.getRoadStones ())
      Stonelist::instance ()->add (new Stone (pos));
  CreateScenario::updateRoadsBridgesAndStones ();

  w->set_fraction (0.7);
  Lw::do_events ();

  for (auto p : *Playerlist::instance ())
    {
      auto as = Armysetlist::instance ()->get (p->getArmyset ());
      if (as->get_images_instantiated () == false)
        {
          bool broken = false;
          as->instantiateImages (broken);
        }
    }

  w->set_fraction (0.8);
  Lw::do_events ();

  auto ccs = GameMap::getCityset ();
  if (ccs->get_images_instantiated () == false)
    {
      bool broken = false;
      ccs->instantiateImages (broken);
    }

  auto ts = GameMap::getTileset ();
  if (ts->get_images_instantiated () == false)
    {
      bool broken = false;
      ts->instantiateImages (broken);
    }

  w->set_fraction (0.9);
  Lw::do_events ();

  auto ss = GameMap::getShieldset ();
  if (ss->get_images_instantiated () == false)
    {
      bool broken = false;
      ss->instantiateImages (broken);
    }

  m_shield_menu_button->reload ();
  add_terrain_buttons ();
  m_smallmap->resize ();
  m_pointer_button->set_active (true);
  update ();

  delete w;
}

void ScenarioBuilderWindow::select_object (Vector<int> pos,
                                           std::vector<UniquelyIdentified*> objects)
{
  //edge case here, a planted standard and a bag is awkward bc it just
  //looks like a bag if we don't take special care
  bool bag_and_flag = false;
  if (objects.size() == 1)
    {
      MapBackpack *b = dynamic_cast<MapBackpack*>(objects.front ());
      if (b)
        {
          if (b->getFirstPlantedItem () && b->size () > 1)
            bag_and_flag = true;
        }
    }

  if (objects.size() == 1 && !bag_and_flag)
    {
      popup_dialog_for_object (objects.front (), "", pos);
      return;
    }

  auto menu = Gio::Menu::create ();
  for (auto i = objects.begin (), end = objects.end (); i != end; ++i)
    {
      if (dynamic_cast<Stack *>(*i))
        {
          m_selected_stack_for_details = dynamic_cast<Stack *> (*i);
          menu->append (_("Stack"), "lw.editor.map.stack-details");
        }
      else if (dynamic_cast<City *>(*i))
        {
          m_selected_city_for_details = dynamic_cast<City *> (*i);
          menu->append (_("City"), "lw.editor.map.city-details");
        }
      else if (dynamic_cast<Ruin *>(*i))
        {
          m_selected_ruin_for_details = dynamic_cast<Ruin *> (*i);
          menu->append (_("Ruin"), "lw.editor.map.ruin-details");
        }
      else if (dynamic_cast<Signpost *>(*i))
        {
          m_selected_signpost_for_details = dynamic_cast<Signpost *> (*i);
          menu->append (_("Signpost"), "lw.editor.map.signpost-details");
        }
      else if (dynamic_cast<Temple *>(*i))
        {
          m_selected_temple_for_details = dynamic_cast<Temple *> (*i);
          menu->append (_("Temple"), "lw.editor.map.temple-details");
        }
      else if (dynamic_cast<Road*>(*i))
        {
          m_selected_road_mouse_position = pos;
          m_selected_road_for_details = dynamic_cast<Road *> (*i);
          menu->append (_("Road"), "lw.editor.map.road-details");
        }
      else if (dynamic_cast<Stone*>(*i))
        {
          m_selected_stone_mouse_position = pos;
          m_selected_stone_for_details = dynamic_cast<Stone *> (*i);
          menu->append (_("Standing Stone"),
                        "lw.editor.map.stone-details");
        }
    }

  for (auto i = objects.begin (), end = objects.end (); i != end; ++i)
    {
      Glib::ustring s = "";
      if (dynamic_cast<MapBackpack*>(*i))
        {
          bool add_a_bag = true;
          MapBackpack *b = dynamic_cast<MapBackpack*>(*i);
          if (b->getFirstPlantedItem ())
            {
              m_selected_bag_for_details =
                dynamic_cast<MapBackpack *> (*i);
              menu->append (_("Planted Standard"),
                            "lw.editor.map.flag-details");
              if (b->size () == 1)
                add_a_bag = false;
            }

          if (add_a_bag)
            {
              m_selected_bag_for_details =
                dynamic_cast<MapBackpack *> (*i);
              menu->append (_("Bag"),
                            "lw.editor.map.bag-details");
            }
        }
    }

  auto context_menu = Gtk::make_managed<Gtk::PopoverMenu> ();
  context_menu->set_menu_model (menu);
  context_menu->set_parent (*m_bigmap);
  Gdk::Rectangle rect;
  rect.set_x (pos.x);
  rect.set_y (pos.y);
  rect.set_width (1);
  rect.set_height (1);

  context_menu->set_position (Gtk::PositionType::TOP);
  context_menu->set_pointing_to (rect);
  context_menu->popup ();
}

void ScenarioBuilderWindow::popup_dialog_for_object (UniquelyIdentified *obj,
                                                     Glib::ustring tag,
                                                     Vector<int> mouse_pos)
{
  if (Stack *s = dynamic_cast<Stack *>(obj))
    {
      m_selected_stack_for_details = s;
      m_simple_actions["editor.map.stack-details"]->activate ();
    }
  else if (City *c = dynamic_cast<City *>(obj))
    {
      m_selected_city_for_details = c;
      m_simple_actions["editor.map.city-details"]->activate ();
    }
  else if (Ruin *r = dynamic_cast<Ruin *>(obj))
    {
      m_selected_ruin_for_details = r;
      m_simple_actions["editor.map.ruin-details"]->activate ();
    }
  else if (Signpost *si = dynamic_cast<Signpost *>(obj))
    {
      m_selected_signpost_for_details = si;
      m_simple_actions["editor.map.signpost-details"]->activate ();
    }
  else if (Temple *t = dynamic_cast<Temple *>(obj))
    {
      m_selected_temple_for_details = t;
      m_simple_actions["editor.map.temple-details"]->activate ();
    }
  else if (Road *rd = dynamic_cast<Road*>(obj))
    {
      m_selected_road_for_details = rd;
      m_selected_road_mouse_position = mouse_pos;
      m_simple_actions["editor.map.road-details"]->activate ();
    }
  else if (Stone *st = dynamic_cast<Stone*>(obj))
    {
      m_selected_stone_for_details = st;
      m_selected_stone_mouse_position = mouse_pos;
      m_simple_actions["editor.map.stone-details"]->activate ();
    }
  else if (MapBackpack *b = dynamic_cast<MapBackpack*>(obj))
    {
      if (tag == "")
        tag = b->getFirstPlantedItem () ? "flag" : "bag";

      if (tag == "bag")
        {
          m_selected_bag_for_details = b;
          m_simple_actions["editor.map.bag-details"]->activate ();
        }
      else if (tag == "flag")
        {
          m_selected_bag_for_details = b;
          m_simple_actions["editor.map.flag-details"]->activate ();
        }
    }
}

// convert a saved game file to a scenario file
void ScenarioBuilderWindow::convert_sav_to_map ()
{
  // reset players
  for (auto p : *Playerlist::instance ())
    {
      p->clearActionlist ();
      p->clearHistorylist ();
      p->clearFogMap ();
      p->revive ();
    }

  // remove all stacks
  for (auto p: *Playerlist::instance ())
    p->clearStacklist ();

  // rebuild cities
  for (auto c : *Citylist::instance ())
    {
      if (c->isBurnt ())
        {
          c->setBurnt (false);
          // give it a unit to produce
          auto a =
            Armysetlist::instance ()->lookupWeakestQuickestArmy
            (c->getOwner ()->getArmyset ());
          c->addProductionBase (0, new ArmyProdBase (*a));
        }
    }

  // turn off production
  for (auto c : *Citylist::instance ())
    c->setActiveProductionSlot (-1);

  // turn off vectoring
  for (auto c : *Citylist::instance ())
    c->setVectoring (Vector<int>(-1, -1));

  VectoredUnitlist::deleteInstance ();

  // revert city owners
  for (auto c : *Citylist::instance ())
    {
      if (c->isCapital ())
        c->setOwner (c->getCapitalOwner ());
      else
        c->setOwner (Playerlist::getNeutral ());
    }

  // remove bags and planted standards
  for (auto bag : GameMap::instance ()->getBackpacks ())
    bag->removeAllFromBackpack ();

  // reset rewards
  Rewardlist::instance ()->flClear ();

  // reset the turn indicator
  Playerlist::instance ()->setActiveplayer
    (*Playerlist::instance ()->begin ());

  // reset the ruins
  for (auto r : *Ruinlist::instance ())
    if (r->isSearched ())
      r->setSearched (false);

  // reset the turn counter
  m_scenario->s_round = 0;
}

void ScenarioBuilderWindow::change_city_ownership (City *city, Player *player)
{
  // set allegiance
  city->setOwner (player);
  //look for stacks in the city, and set them to this player
  for (unsigned int x = 0; x < city->getSize (); x++)
    {
      for (unsigned int y = 0; y < city->getSize (); y++)
        {
          Stack *s =
            GameMap::getStack (city->getPos () + Vector<int>(x,y));
          if (s)
            Stacklist::changeOwnership (s, player);
        }
    }
}
