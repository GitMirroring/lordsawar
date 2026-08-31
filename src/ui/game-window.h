//  Copyright (C) 2007, 2008 Ole Laursen
//  Copyright (C) 2007, 2008, 2009, 2010, 2011, 2012, 2014, 2015, 2016, 2017,
//  2020, 2021, 2026 Ben Asselstine
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

#include <random>
#include <cmath>
#ifndef GAME_WINDOW_H
#define GAME_WINDOW_H
#include "lw.h"
#include "map-widget.h"
#include "turn-indicator.h"
#include "game-scenario.h"
#include "next-turn.h"
#include "game.h"
#include "game-button-box.h"
#include "small-map.h"
#include "input-events.h"
#include "quest-manager.h"
#include "configuration.h"
#include "stack.h"
#include "stack-tile.h"
#include "status-box.h"
#include "snd.h"
#include "ruin-list.h"
#include "temple-list.h"

#include "next-turn-dialog.h"
#include "hero-offer-dialog.h"
#include "commentator-dialog.h"
#include "quest-expired-dialog.h"
#include "hero-brings-allies-dialog.h"
#include "surrender-dialog.h"
#include "surrender-refused-dialog.h"
#include "surrender-accepted-dialog.h"
#include "game-over-dialog.h"
#include "city-dialog.h"
#include "city-razed-dialog.h"
#include "ruin-fight-dialog.h"
#include "sage-dialog.h"
#include "fight-result.h"
#include "ruin-searched-dialog.h"
#include "search-temple-dialog.h"
#include "city-info-tip.h"
#include "map-info-tip.h"
#include "stack-info-tip.h"
#include "treachery-dialog.h"
#include "fight-window.h"
#include "quest-completed-dialog.h"
#include "city-looted-dialog.h"
#include "city-defeated-dialog.h"
#include "city-pillaged-dialog.h"
#include "city-sacked-dialog.h"
#include "city-defeated-dialog.h"
#include "ruin-report-dialog.h"
#include "game-loaded-dialog.h"
#include "player-died-dialog.h"
#include "military-advisor-dialog.h"
#include "use-item-dialog.h"
#include "use-item-on-player-dialog.h"
#include "gold-stolen-dialog.h"
#include "ships-sunk-dialog.h"
#include "use-item-on-city-dialog.h"
#include "worms-killed-dialog.h"
#include "keeper-captured-dialog.h"
#include "bridge-burned-dialog.h"
#include "bags-picked-up-dialog.h"
#include "monster-summoned-dialog.h"
#include "mp-added-to-hero-stack-dialog.h"
#include "city-diseased-dialog.h"
#include "city-defended-dialog.h"
#include "city-persuaded-dialog.h"
#include "stack-teleported-dialog.h"
#include "next-turn-hotseat.h"
#include "fight-order-dialog.h"
#include "army-bonus-dialog.h"
#include "item-bonus-dialog.h"
#include "resign-dialog.h"
#include "resign-completed-dialog.h"
#include "signpost-change-dialog.h"
#include "stack-disband-dialog.h"
#include "destination-dialog.h"
#include "hero-levels-dialog.h"
#include "stack-info-dialog.h"
#include "report-dialog.h"
#include "history-report-dialog.h"
#include "item-report-dialog.h"
#include "diplomacy-report-dialog.h"
#include "quest-report-dialog.h"
#include "quest-assigned-dialog.h"
#include "hero-dialog.h"
#include "diplomacy-dialog.h"
#include "preferences-dialog.h"
#include "triumphs-dialog.h"
#include "about-dialog.h"
#include "scenario-info-dialog.h"
#include "lw-dialog.h"
#include "game-client.h"
#include "game-server.h"
#include "next-turn-networked.h"

using GSO = ::GameScenarioOptions;

class LwMenuButton: public Gtk::MenuButton
{
public:
    LwMenuButton (Gtk::Window &p, Glib::RefPtr<Gio::SimpleActionGroup> &group)
      :m_parent (p), m_actions (group)
      {
      }

    ~LwMenuButton () override = default;

    void set_zoom (double scale)
      {
        int percent = static_cast<int>(scale * 100);
        m_zoom_scale_label->set_text (std::to_string (percent) + "%");
        m_zoom_in_button->set_sensitive (scale < MapWidget::MAX_ZOOM_FACTOR);
        m_zoom_out_button->set_sensitive (scale > MapWidget::MIN_ZOOM_FACTOR);
      }

    void setup ()
      {
        setup_menu ();
      }

    sigc::signal<void(Glib::RefPtr<Gio::SimpleAction>)> m_signal_action_added;

private:
    Gtk::Window &m_parent;
    Glib::RefPtr<Gio::SimpleActionGroup> m_actions;

    Gtk::Label *m_zoom_scale_label;
    Gtk::Button *m_zoom_in_button;
    Gtk::Button *m_zoom_out_button;

    void setup_zoom_menuitem ()
      {
        auto box = Gtk::Box (Gtk::Orientation::HORIZONTAL);
        box.set_margin_top (5);
        box.set_margin_bottom (5);
        box.set_margin_start (12);
        m_zoom_out_button = Gtk::make_managed<Gtk::Button>();
        m_zoom_out_button->set_icon_name ("zoom-out-symbolic");
        m_zoom_out_button->set_action_name ("lw.view.zoom-out");
        m_zoom_out_button->add_css_class ("flat");
        m_zoom_out_button->add_css_class ("small");
        m_zoom_in_button = Gtk::make_managed<Gtk::Button>();
        m_zoom_in_button->set_icon_name ("zoom-in-symbolic");
        m_zoom_in_button->set_action_name ("lw.view.zoom-in");
        m_zoom_in_button->add_css_class ("flat");
        m_zoom_in_button->add_css_class ("small");
        m_zoom_scale_label = Gtk::make_managed<Gtk::Label>("100%");
        m_zoom_scale_label->set_width_chars (5);

        auto click = Gtk::GestureClick::create ();
        click->signal_pressed ().connect
          ([this](int, double, double)
           {
             m_actions->lookup_action ("view.reset-zoom")->activate ();
           });
        m_zoom_scale_label->add_controller (click);

        auto toggle_fullscreen_button = Gtk::make_managed<Gtk::Button>();
        toggle_fullscreen_button->set_icon_name ("view-fullscreen-symbolic");
        toggle_fullscreen_button->set_action_name ("lw.view.toggle-fullscreen");
        toggle_fullscreen_button->add_css_class ("flat");
        toggle_fullscreen_button->add_css_class ("small");

        auto l = Gtk::Label ("Zoom");
        box.append (l);
        box.append (*m_zoom_out_button);
        box.append (*m_zoom_scale_label);
        box.append (*m_zoom_in_button);
        box.append (*toggle_fullscreen_button);
        Gtk::PopoverMenu *m = dynamic_cast <Gtk::PopoverMenu*>(get_popover ());
        m->add_child (box, "zoom-menuitem");
      }

    void setup_actions (std::initializer_list<const char*> list)
      {
         for (auto s : list)
           setup_action (m_actions->add_action (s));
      }

    void setup_menu ()
      {
        auto menu = Gio::Menu::create ();

        auto game_menu = Gio::Menu::create ();
        game_menu->append (_("New Game"), "lw.game.new");
        game_menu->append (_("Open"), "lw.game.open");
        game_menu->append (_("Save"), "lw.game.save");
        game_menu->append (_("Save As"), "lw.game.save-as");
        game_menu->append (_("Show Lobby"), "lw.game.show-lobby");
        game_menu->append (_("Scenario Information"), "lw.game.scenario-info");
        game_menu->append (_("Quit"), "lw.game.quit");

        auto order_menu = Gio::Menu::create ();
        order_menu->append (_("Fight Order"), "lw.order.fight-order");
        order_menu->append (_("Move All"), "lw.order.move-all");
        order_menu->append (_("Disband"), "lw.order.disband");
        order_menu->append (_("Signpost"), "lw.order.signpost");
        order_menu->append (_("Group/Ungroup"), "lw.order.toggle-group");
        order_menu->append (_("Stay Here"), "lw.order.stay-here");
        order_menu->append (_("Next"), "lw.order.next");
        order_menu->append (_("Resign"), "lw.order.resign");

        auto reports_menu = Gio::Menu::create ();
        reports_menu->append (_("Army"), "lw.reports.army");
        reports_menu->append (_("City"), "lw.reports.city");
        reports_menu->append (_("Gold"), "lw.reports.gold");
        reports_menu->append (_("Production"), "lw.reports.production");
        reports_menu->append (_("Winning"), "lw.reports.winning");
        reports_menu->append (_("Diplomacy"), "lw.reports.diplomacy");
        reports_menu->append (_("Quest"), "lw.reports.quest");
        reports_menu->append (_("Items"), "lw.reports.items");

        auto hero_menu = Gio::Menu::create ();
        hero_menu->append (_("Inspect"), "lw.hero.inspect");
        hero_menu->append (_("Plant Flag"), "lw.hero.plant-flag");
        hero_menu->append (_("Levels"), "lw.hero.levels");
        hero_menu->append (_("Search"), "lw.hero.search");
        hero_menu->append (_("Use Item"), "lw.hero.use-item");

        auto view_menu = Gio::Menu::create ();
        view_menu->append (_("Preferences"), "lw.view.preferences");
        view_menu->append (_("Toggle Grid"), "lw.view.toggle-grid");
        view_menu->append (_("Army Bonus"), "lw.view.army-bonus");
        view_menu->append (_("Items"), "lw.view.items");
        view_menu->append (_("Cities"), "lw.view.cities");
        view_menu->append (_("Vectoring"), "lw.view.vectoring");
        view_menu->append (_("Ruins"), "lw.view.ruins");
        view_menu->append (_("Stack"), "lw.view.stack");
        view_menu->append (_("Diplomacy"), "lw.view.diplomacy");

        auto history_menu = Gio::Menu::create ();
        history_menu->append (_("City"), "lw.history.city");
        history_menu->append (_("Ruins"), "lw.history.ruins");
        history_menu->append (_("Events"), "lw.history.events");
        history_menu->append (_("Gold"), "lw.history.gold");
        history_menu->append (_("Winners"), "lw.history.winners");
        history_menu->append (_("Triumphs"), "lw.history.triumphs");

        auto help_menu = Gio::Menu::create ();
        help_menu->append (_("Tutorial Video"), "lw.help.tutorial-video");
        help_menu->append (_("Keyboard Shortcuts"), "lw.help.keyboard-shortcuts");
        help_menu->append (_("Online Help"), "lw.help.online-help");
        help_menu->append (_("About"), "lw.help.about");

        auto turn_menu = Gio::Menu::create ();
        turn_menu->append (_("End Turn"), "lw.turn.end-turn");

        menu->append_submenu (_("Game"), game_menu);
        menu->append_submenu (_("Order"), order_menu);
        menu->append_submenu (_("Reports"), reports_menu);
        menu->append_submenu (_("Hero"), hero_menu);
        menu->append_submenu (_("View"), view_menu);
        auto item = Gio::MenuItem::create ("", "");
        auto id = Glib::Variant<Glib::ustring>::create ("zoom-menuitem");
        item->set_attribute_value ("custom", id);
        menu->append_item (item);
        menu->append_submenu (_("History"), history_menu);
        menu->append_submenu (_("Help"), help_menu);
        menu->append_submenu (_("Turn"), turn_menu);

        property_primary () = true;
        set_menu_model (menu);

        setup_actions
          ({"game.new", "game.open", "game.save", "game.save-as",
            "game.show-lobby", "game.scenario-info", "game.quit",
            "order.fight-order", "order.move-all", "order.disband",
            "order.signpost", "order.toggle-group", "order.stay-here",
            "order.next", "order.resign", "reports.army", "reports.city",
            "reports.gold", "reports.production", "reports.winning",
            "reports.diplomacy", "reports.quest", "reports.items",
            "hero.inspect", "hero.plant-flag", "hero.levels", "hero.search",
            "hero.use-item", "view.preferences", "view.toggle-grid",
            "view.army-bonus", "view.items", "view.cities",
            "view.vectoring", "view.ruins", "view.stack",
            "view.zoom-in", "view.zoom-out", "view.reset-zoom",
            "view.toggle-fullscreen", "view.diplomacy", "history.city",
            "history.ruins", "history.events", "history.gold",
            "history.winners", "history.triumphs", "help.tutorial-video",
            "help.keyboard-shortcuts", "help.online-help", "help.about",
            "turn.end-turn",

            //the other game button actions, it's just easier to add these here
            "order.move", "view.center", "order.deselect", "order.defend"
          });

        set_icon_name ("open-menu-symbolic");
        setup_zoom_menuitem ();
      }

    void setup_action (Glib::RefPtr<Gio::SimpleAction> a)
      {
        m_signal_action_added.emit (a);
      }
};

class ShortcutsDialog : public Gtk::Window
{
public:
    ShortcutsDialog ()
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
                      (_("Game"),
                       {
                           {_("Load a game"), "<Alt>N"},
                           {_("Save game"), "<Ctrl>S"},
                           {_("Show lobby"), "<Ctrl>L"},
                           {_("Quit"), "<Ctrl>Q"}
                       }));

        flow->append (*create_group
                      ("Orders",
                       {
                           {_("View or change the fight order"), "I"},
                           {_("Move all pre-planned movements"), "M"},
                           {_("Disband a stack"), "Q"},
                           {_("Change a signpost"), "X"},
                           {_("Group/ungroup"), "space"},
                           {_("Stay here"), "Escape"},
                           {_("Select next stack"), "Return"},
                           {_("Resign the game"), "R"}

                       }));
        flow->append (*create_group
                      (_("Reports"),
                       {
                           {_("Army Report"), "A"},
                           {_("City Report"), "K"},
                           {_("Gold Report"), "G"},
                           {_("Production Report"), "N"},
                           {_("Winning Report"), "W"},
                           {_("Diplomacy Report"), "D"},
                           {_("Quest Report"), "equal"}
                       }));
        flow->append (*create_group
                      (_("Hero"),
                       {
                           {_("Inspect"), "comma"},
                           {_("Plant standard"), "F"},
                           {_("View hero levels"), "U"},
                           {_("Search here"), "Z"}
                       }));
        flow->append (*create_group
                      (_("View"),
                       {
                           {_("Fullscreen"), "F11"},
                           {_("Preferences"), "<Ctrl>P"},
                           {_("Toggle Grid"), "<Shift>G"},
                           {_("View Army Bonuses"), "O"},
                           {_("View Items"), "T"},
                           {_("Build"), "B"},
                           {_("Cities"), "C"},
                           {_("Production"), "P"},
                           {_("Vectoring"), "V"},
                           {_("Ruins"), "period"},
                           {_("View Stack"), "S"}
                       }));
        flow->append (*create_group
                      (_("History"),
                       {
                           {_("See a graph of cities over time"), "H"},
                           {_("View event logs"), "H"},
                           {_("See a graph of gold over time"), "J"},
                           {_("See a graph of who's been winning"), "Y"},
                           {_("View past triumphs"), "L"},
                       }));
        flow->append (*create_group
                      (_("Turn"),
                       {
                           {_("End your turn"), "<Alt>E"},
                       }));
        scrolled->set_child (*flow);
        set_child (*scrolled);
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

class QuitConfirmationDialog : public Gtk::MessageDialog
{
public:
    using ConfirmationHandler =
      std::function<void(bool should_quit)>;

    QuitConfirmationDialog (Gtk::Window& parent, ConfirmationHandler handler)
      : Gtk::MessageDialog (parent, _("Quit Game"), false,
                            Gtk::MessageType::WARNING, Gtk::ButtonsType::NONE),
      m_handler (std::move (handler))
  {
    set_secondary_text (_("Are you sure you want to quit?"));

    add_button (_("Cancel"), Gtk::ResponseType::CANCEL);
    auto quit_button = add_button (_("Quit"), Gtk::ResponseType::CLOSE);
    quit_button->add_css_class ("destructive-action");

    signal_response ().connect
      (sigc::mem_fun (*this, &QuitConfirmationDialog::on_response));
  }

protected:
    void on_response (int response)
      {
        bool should_quit = false;

        switch (response)
          {
          case Gtk::ResponseType::CLOSE:
            should_quit = true;
            break;

          case Gtk::ResponseType::CANCEL:
          default:
            should_quit = false;
            break;
          }

        if (m_handler)
          m_handler (should_quit);

        hide ();
      }

private:
    ConfirmationHandler m_handler;
};

class GameWindow : public Gtk::ApplicationWindow
{
public:

    inline static int s_displayBattleExplosionDelay = TIMER_BIGMAP_EXPLOSION_DELAY;
    inline static int s_displayAbbreviatedBattleExplosionDelay = TIMER_BIGMAP_ABBREVIATED_FIGHT_DELAY;

    static Gtk::HeaderBar * create_header_bar (TurnIndicator *turn_indicator, Gtk::MenuButton *menu_button)
      {
        auto header_bar = Gtk::make_managed<Gtk::HeaderBar>();
        header_bar->set_show_title_buttons (true);

        auto title = Gtk::make_managed<Gtk::Label>("Lw");
        header_bar->set_title_widget (*title);

        auto button_box = Gtk::make_managed<Gtk::Box>();
        header_bar->pack_start (*button_box);

        header_bar->pack_end (*menu_button);
        header_bar->pack_end (*turn_indicator);
        return header_bar;
      }

    void new_game (GameScenario *game_scenario, NextTurn *next_turn)
      {
        setup_game_object (game_scenario, next_turn);
        setup_signals ();
        m_game->startGame ();
      }

    void load_game (GameScenario *game_scenario, NextTurn *next_turn)
      {
        setup_game_object (game_scenario, next_turn);
        setup_signals ();
        m_game->loadGame ();
      }

    void new_network_game (GameScenario *game_scenario, NextTurnNetworked *next_turn)
      {
        setup_game_object (game_scenario, next_turn);
        setup_signals ();

        m_game->redraw ();
        m_game->startGame ();

        auto active = Playerlist::getActiveplayer ();

        if (active->getType () == Player::HUMAN)
          Playerlist::instance ()->setViewingplayer (active);

        if (GameServer::instance ()->isRunning () == false)
          if (active->getType () != Player::NETWORKED)
            next_turn->start_player (active);
      }

    GameWindow ()
      {
        signal_close_request ().connect (sigc::mem_fun (*this, &GameWindow::on_close_request), false);

        m_actions = Gio::SimpleActionGroup::create ();
        set_default_size (1200, 800);

        m_main_box =
          Gtk::make_managed<Gtk::Box>(Gtk::Orientation::HORIZONTAL);
        m_main_box->set_spacing (2);
        m_main_box->set_margin (2);

        m_left_box = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::VERTICAL);
        m_left_box->set_hexpand (true);


        m_main_box->append (*m_left_box);
        auto right_box = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::VERTICAL);
        m_menu_button = Gtk::make_managed<LwMenuButton>(*this, m_actions);
        m_menu_button->m_signal_action_added.connect
          (sigc::mem_fun (*this, &GameWindow::on_game_action_added));
        m_menu_button->setup ();

        m_hidden_menu_button = Gtk::make_managed<LwMenuButton>(*this, m_actions);
        m_hidden_menu_button->m_signal_action_added.connect
          (sigc::mem_fun (*this, &GameWindow::on_game_action_added));
        m_hidden_menu_button->setup ();

        auto idx = std::string (LW_APP_ID).rfind ('.');
        insert_action_group (std::string (LW_APP_ID).substr (idx + 1), m_actions);

        setup_smallmap ();
        //bigmap and status box are setup later

        right_box->append (*m_smallmap_drawing_area);
        m_game_button_box = Gtk::make_managed<GameButtonBox> ();
        right_box->append (*m_game_button_box);
        m_main_box->append (*right_box);
        setup_header_bar ();
      }

    bool on_close_request ()
      {
        if (confirm_dialog)
          {
            confirm_dialog->present ();
            return true;
          }

        confirm_dialog =
          new QuitConfirmationDialog
          (*this,
           [this](bool should_quit)
           {
             handle_confirmation_result (should_quit);
           });

        confirm_dialog->signal_hide ().connect
          ([this]()
           {
             if (confirm_dialog)
               {
                 delete confirm_dialog;
                 confirm_dialog = nullptr;
               }
           });

        confirm_dialog->show ();
        return true;
      }

    ~GameWindow ()
      {
        delete m_game;
      }

    sigc::signal<void ()> signal_game_ended ()
      {
        return m_game_ended;
      }

    sigc::signal<void ()> signal_game_ended_start_new ()
      {
        return m_game_ended_start_new;
      }

    sigc::signal<void ()> signal_show_lobby ()
      {
        return m_show_lobby;
      }
private:
    Gtk::Box *m_main_box = NULL;
    Gtk::Box *m_left_box = NULL;
    MapWidget *m_bigmap = NULL;
    Gtk::DrawingArea *m_smallmap_drawing_area = NULL;
    Cairo::RefPtr<Cairo::Surface> m_smallmap_surface;
    GameButtonBox *m_game_button_box = NULL;
    LwMenuButton *m_menu_button = NULL;
    LwMenuButton *m_hidden_menu_button = NULL;
    StatusBox *m_status_box = NULL;
    Gtk::Revealer* m_headerbar_revealer = NULL;
    TurnIndicator *m_main_turn_indicator;
    TurnIndicator *m_fullscreen_turn_indicator;

    GameScenario *m_game_scenario = NULL;
    NextTurn *m_next_turn = NULL;
    Game *m_game = NULL;

    sigc::connection m_hide_timeout;
    QuitConfirmationDialog* confirm_dialog = NULL;
    struct MouseMotionEvent m_smallmap_mouse_motion = {};
    Glib::ustring m_stop_action = "";
    Player *m_game_winner = NULL;
    sigc::signal<void ()> m_game_ended;
    sigc::signal<void ()> m_game_ended_start_new;
    sigc::signal<void ()> m_show_lobby;
    std::list<sigc::connection> m_connections;
    Glib::RefPtr<Gio::SimpleActionGroup> m_actions;
    std::map<Glib::ustring, Glib::RefPtr<Gio::SimpleAction>> m_simple_actions;
    std::string m_current_save_filename;
    std::string m_current_load_filename;
    CityInfoTip *m_city_info_tip = NULL;
    MapInfoTip *m_map_info_tip = NULL;
    StackInfoTip *m_stack_info_tip = NULL;

    void on_show_keyboard_shortcuts ()
      {
        auto* dialog = Gtk::make_managed<ShortcutsDialog>();
        dialog->set_transient_for (*this);
        dialog->present ();
      }

    void on_game_action_added (Glib::RefPtr<Gio::SimpleAction> action)
      {
        m_simple_actions[action->get_name ()] = action;
      }

    void setup_accels ()
      {
        auto app = get_application ();
        if (!app)
          return;
        std::list<std::pair<std::string, std::string>> accels =
          {
              {"game.open", "<Alt>l"}, {"game.save", "<Alt>s"},
              {"game.show-lobby", "<Ctrl>l"}, {"game.quit", "<Ctrl>q"},
              {"order.fight-order", "i"}, {"order.move-all", "m"},
              {"order.disband", "q"}, {"order.signpost", "x"},
              {"order.toggle-group", "space"}, {"order.stay-here", "Escape"},
              {"order.next", "Return"}, {"order.resign", "r"},
              {"reports.army", "a"}, {"reports.city", "k"},
              {"reports.gold", "g"}, {"reports.production", "n"},
              {"reports.winning", "w"}, {"reports.diplomacy", "d"},
              {"reports.quest", "equal"}, {"hero.inspect", "comma"},
              {"hero.plant-flag", "f"}, {"hero.levels", "u"},
              {"hero.search", "z"}, {"view.toggle-fullscreen", "F11"},
              {"view.preferences", "<Ctrl>p"},
              {"view.toggle-grid", "<Shift>g"}, {"view.army-bonus", "o"},
              {"view.items", "t"}, {"view.cities", "c"},
              {"view.vectoring", "v"}, {"view.ruins", "period"},
              {"view.stack", "s"}, {"history.city", "h"},
              {"history.events", "e"}, {"history.gold", "j"},
              {"history.winners", "y"}, {"history.triumphs", "l"},
              {"help.online-help", "F1"}, {"turn.end-turn", "<Alt>e"}
          };
        auto idx = std::string (LW_APP_ID).rfind ('.');
        std::string prefix = std::string (LW_APP_ID).substr (idx + 1);
         for (auto accel : accels)
           app->set_accel_for_action (prefix + "." + accel.first, accel.second);
      }

    void handle_confirmation_result (bool should_quit)
      {
        if (!should_quit)
          return;

        m_next_turn->stop ();
        application_quit ();
      }

    void application_quit ()
      {
        //maybe we were started on our own mainloop
        if (Lw::app->m_view_stress_test)
          Lw::quit_loop ();
        get_application ()->quit ();
      }

    void action_connect (Glib::ustring name, sigc::slot<void()> slot)
      {
        auto simple =
          std::dynamic_pointer_cast<Gio::SimpleAction>(m_simple_actions[name]);
        if (simple)
          add_connection
            (simple->signal_activate ().connect (sigc::hide (slot)));
      }

    void setup_header_bar ()
      {
        m_main_turn_indicator = Gtk::make_managed<TurnIndicator>();
        auto header_bar = GameWindow::create_header_bar (m_main_turn_indicator,
                                                         m_menu_button);
        set_titlebar (*header_bar);

        m_fullscreen_turn_indicator = Gtk::make_managed<TurnIndicator>();
        auto hidden_header_bar =
          GameWindow::create_header_bar (m_fullscreen_turn_indicator,
                                         m_hidden_menu_button);

        m_headerbar_revealer = Gtk::make_managed<Gtk::Revealer> ();
        m_headerbar_revealer->set_child (*hidden_header_bar);
        m_headerbar_revealer->set_transition_type
          (Gtk::RevealerTransitionType::SLIDE_DOWN);
        m_headerbar_revealer->set_transition_duration (200);
        m_headerbar_revealer->set_reveal_child (false);

        m_headerbar_revealer->set_valign (Gtk::Align::START);
        m_headerbar_revealer->set_vexpand (false);
        m_headerbar_revealer->set_hexpand (true);

        auto overlay = Gtk::make_managed<Gtk::Overlay>();
        overlay->set_child (*m_main_box);
        overlay->add_overlay (*m_headerbar_revealer);

        set_child (*overlay);

        auto motion = Gtk::EventControllerMotion::create ();
        motion->signal_motion ().connect
          ([this,hidden_header_bar](double x, double y)
           {
             (void)x;
             if (!is_fullscreen ())
               {
                 m_headerbar_revealer->set_reveal_child (false);
                 return;
               }

             int header_height = hidden_header_bar->get_allocated_height ();

             if (y <= 5 || y <= header_height)
               {
                 if (m_hide_timeout)
                   m_hide_timeout.disconnect ();
                 m_headerbar_revealer->set_reveal_child (true);
               }
             else
               {
                 if (m_hide_timeout)
                   m_hide_timeout.disconnect ();

                 m_hide_timeout = Glib::signal_timeout ().connect
                   ([this]()
                    {
                      if (is_fullscreen ())
                        m_headerbar_revealer->set_reveal_child (false);
                      return false;
                    }, 800);
               }
           });
        add_controller (motion);
      }

    void setup_smallmap ()
      {
        struct MouseMotionEvent empty_event{};
        m_smallmap_mouse_motion = empty_event;
        m_smallmap_drawing_area = Gtk::make_managed<Gtk::DrawingArea> ();
        m_smallmap_drawing_area->set_valign (Gtk::Align::FILL);
        m_smallmap_drawing_area->set_halign (Gtk::Align::CENTER);

        auto motion = Gtk::EventControllerMotion::create ();
        motion->signal_motion ().connect
          ([this] (double x, double y)
           {
             struct MouseMotionEvent *ev = &m_smallmap_mouse_motion;
             ev->pos = Vector<int>(std::round (x), std::round (y));
             m_game->get_smallmap ().mouse_motion_event (*ev);
           });
        m_smallmap_drawing_area->add_controller (motion);

        auto click = Gtk::GestureClick::create ();
        click->signal_pressed ().connect
          ([this] (int n, double x, double y)
           {
             struct MouseButtonEvent ev = to_input_event (n, x, y, true);
             struct MouseMotionEvent *evmotion = &m_smallmap_mouse_motion;
             evmotion->pressed[ev.button] = true;
             m_game->get_smallmap ().mouse_button_event (ev);
           });

        click->signal_released ().connect
          ([this] (int n, double x, double y)
           {
             struct MouseButtonEvent ev = to_input_event (n, x, y, false);
             struct MouseMotionEvent *evmotion = &m_smallmap_mouse_motion;
             evmotion->pressed[ev.button] = false;
             m_game->get_smallmap ().mouse_button_event (ev);
           });
        m_smallmap_drawing_area->add_controller (click);

        ImageCache::CursorType c = ImageCache::MAGNIFYING_GLASS;
        auto hotspot = ImageCache::get_hotspot (c);
        auto im = ImageCache::instance ()->getCursorPic (c);
        auto cursor = Gdk::Cursor::create (im->to_texture (),
                                           hotspot.x, hotspot.y);
        m_smallmap_drawing_area->set_cursor (cursor);
      }

    void
    add_connection (sigc::connection c)
      {
        m_connections.push_back (c);
      }

    void setup_game_object (GameScenario *game_scenario, NextTurn *nextTurn)
      {
        m_game_scenario = game_scenario;
        m_next_turn = nextTurn;
        Snd::instance ()->halt (true);
        Snd::instance ()->enableBackground ();

        if (m_game)
          delete m_game;
        m_game = new Game (game_scenario, nextTurn);

        setup_accels ();

        setup_bigmap ();
        m_status_box->clear_selected_stack ();
        m_game->get_bigmap ()->reset_zoom ();

        m_smallmap_drawing_area->set_draw_func
          ([this](const Cairo::RefPtr<Cairo::Context>& cr, int, int)
           {
             if (!m_smallmap_surface)
               return;

             cr->set_source (m_smallmap_surface, 0, 0);
             cr->paint();
           });

        m_game->get_smallmap ().signal_map_changed ().connect
          ([this] (Cairo::RefPtr<Cairo::Surface> map, Gdk::Rectangle r)
           {
             (void) r;
             std::shared_ptr<Cairo::ImageSurface> img_surface =
               std::static_pointer_cast<Cairo::ImageSurface>(map);
             m_smallmap_surface = map;
             m_smallmap_drawing_area->set_size_request
               (img_surface->get_width (), img_surface->get_height ());
             map->flush ();

             m_smallmap_drawing_area->queue_draw ();
           });

        m_game->get_smallmap ().signal_view_slid ().connect
          ([this] (LwRectangle r)
           {
             (void) r;
           });

        m_bigmap->signal_view_changed ().connect
          (sigc::mem_fun (m_game->get_smallmap (), &SmallMap::set_view));

        m_game->get_smallmap ().signal_view_changed ().connect
          (sigc::mem_fun (*m_bigmap, &MapWidget::set_view));

        m_game->get_smallmap ().draw ();

        //status_box->setHeightFudgeFactor(turn_label->get_height());
        m_status_box->enforce_height ();
        m_status_box->show_stats ();

        m_status_box->m_stack_composition_modified.connect
          (sigc::mem_fun (*m_game, &Game::recalculate_moves_for_stack));
        m_status_box->m_stack_tile_group_toggle.connect
          ([this] (bool lock)
           {
             //this is a weird one here
             m_game->get_bigmap ()->set_input_locked (lock);
           });

        return;
      }

    void setup_signals ()
      {
        for (auto c : m_connections)
          c.disconnect ();
        setup_action_signals ();
        setup_game_signals ();
        if (m_game_scenario->getPlayMode () == GameScenario::NETWORKED)
          setup_network_play_signals ();
      }

    void setup_network_play_signals ()
      {
        auto next_turn = dynamic_cast<NextTurnNetworked*>(m_next_turn);

        if (GameServer::instance ()->isRunning ()) // server
          {
            auto game_server = GameServer::instance ();

            add_connection
              (game_server->signal_round_begins ().connect
               ([this] ()
                {
                  printf ("got signal round begins\n");
                }));

          }
        else // client
          {
            auto game_client = GameClient::instance ();

            add_connection
              (game_client->signal_hosted_player_sits ().connect
               ([this] (Shield::Color shield)
                {
                  auto p = Playerlist::instance ()->get (shield);
                  m_game->addPlayer (p);
                }));

            add_connection
              (game_client->signal_hosted_player_stands ().connect
               ([this] (Shield::Color shield)
                {
                  auto p = Playerlist::instance ()->get (shield);
                  m_game->addPlayer (p);
                }));

            add_connection
              (game_client->signal_start_player_turn ().connect
               (sigc::mem_fun (*next_turn, &NextTurnNetworked::start_player)));

            add_connection
              (game_client->signal_start_player_turn ().connect
               ([] (Player *p)
                {
                  printf ("got signal start player turn! (%s)\n", p->getName ().c_str ());
                }));

            add_connection
              (game_client->signal_round_ends ().connect
               (sigc::mem_fun (*next_turn, &NextTurnNetworked::finishRound)));

            add_connection
              (game_client->signal_round_ends ().connect
               ([] ()
                {
                  printf ("got signal round ends\n");
                }));

            add_connection
              (GameClient::instance ()->signal_playerlist_reorder_received ().connect
               ([this] ()
                {
                  printf ("playerlist order recieved\n");
                }));
          }
      }

    void setup_bigmap ()
      {
        m_bigmap = m_game->get_bigmap ();
        m_bigmap->set_hexpand (true);
        m_bigmap->set_vexpand (true);
        m_bigmap->signal_zoom_changed ().connect
          (sigc::mem_fun (*m_menu_button, &LwMenuButton::set_zoom));
        m_bigmap->signal_zoom_changed ().connect
          (sigc::mem_fun (*m_hidden_menu_button, &LwMenuButton::set_zoom));
        m_bigmap->signal_size_allocated ().connect
          ([this] (int width, int height)
           {
             (void) width;
             (void) height;
             auto tile = m_game->get_smallmap ().get_view ().pos;
             m_bigmap->center_on_smallmap_pos (tile.x, tile.y);
           });

        m_left_box->append (*m_bigmap);

        m_status_box = StatusBox::create ();
        m_left_box->append (*m_status_box);
        m_status_box->setup ();
      }

    void setup_game_signals ()
      {
        add_connection
          (m_game->signal_city_too_poor_to_produce ().connect
           ([this] (sigc::slot<void()> after)
            {
              auto d = LwDialog::build<ReportDialog>(this);
              d->setup (ReportDialog::PRODUCTION);
              d->signal_response ().connect
                ([d, after] (Gtk::ResponseType)
                 {
                   delete d;
                   after ();
                 });
            }));

        add_connection
          (m_game->signal_commentator_comments ().connect
           ([this] (Glib::ustring comment, sigc::slot<void()> finish)
            {
              auto d = LwDialog::build<CommentatorDialog> (this);
              d->setup (comment);
              d->signal_response ().connect
                ([d, finish] (Gtk::ResponseType)
                 {
                   finish ();
                   delete d;
                 });
            }));

        add_connection
          (m_game->signal_game_stopped ().connect
           ([this] ()
            {
              if (m_stop_action == "quit")
                {
                  if (m_game)
                    {
                      delete m_game;
                      m_game = NULL;
                    }
                  m_game_ended.emit ();
                }
              else if (m_stop_action == "new")
                {
                  if (m_game)
                    {
                      delete m_game;
                      m_game = NULL;
                    }
                  m_game_ended_start_new.emit ();
                }
              else if (m_stop_action == "game-over")
                {
                  if (m_game_winner)
                    {
                      if (m_game_winner->getType () != Player::HUMAN)
                        {
                          if (m_game)
                            {
                              delete m_game;
                              m_game = NULL;
                            }
                          m_game_ended.emit ();
                        }
                      else
                        {
                          //we need to keep the game object around
                          //so that we can give out some cheese
                          //give_some_cheese (m_game_winner);
                        }
                    }
                  else
                    {
                      if (m_game)
                        {
                          delete m_game;
                          m_game = NULL;
                        }
                      m_game_ended.emit ();
                    }
                }
              else if (m_stop_action == "load-game")
                {
                  if (m_game)
                    {
                      delete m_game;
                      m_game = NULL;
                    }
                  bool broken = false;
                  Glib::ustring err;

                  GameScenario* game_scenario =
                    new GameScenario (m_current_load_filename, broken, err);

                  if (broken)
                    {
                      //on_message_requested(_("Corrupted saved game file."));
                      m_game_ended.emit ();
                      return;
                    }

                  if (game_scenario->getPlayMode () == GameScenario::HOTSEAT)
                    load_game (game_scenario, new NextTurnHotseat);
                  else if (game_scenario->getPlayMode () == GameScenario::NETWORKED)
                    {
                      /*
                      NewNetworkGameDialog nngd(*get_window(), true);
                      bool retval = nngd.run();
                      nngd.hide();
                      hide();
                      if (retval)
                        {
                          load_hosted_network_game.emit 
                            (d_load_filename, LORDSAWAR_PORT, nngd.getProfile(), 
                             nngd.isAdvertised(), nngd.isRemotelyHosted());
                        }
                        */
                    }
                }
            }));

        add_connection
          (m_game->signal_round_begins ().connect
           ([this] ()
            {
              int round_number = m_game_scenario->getRound ();
              m_main_turn_indicator->update_round (round_number);
              m_fullscreen_turn_indicator->update_round (round_number);
            }));

        add_connection
          (m_game->signal_turn_begins ().connect
           ([this] ()
            {
              int round_number = m_game_scenario->getRound ();
              m_main_turn_indicator->update_round (round_number);
              m_fullscreen_turn_indicator->update_round (round_number);
            }));

        add_connection
          (m_game->signal_sidebar_stats_changed ().connect
           ([this] (SidebarStats s)
            {
              m_status_box->update_sidebar_stats (s);
            }));

        add_connection
          (m_game->signal_progress_status_changed ().connect
           ([this] (Glib::ustring message)
            {
              m_status_box->set_progress_label (message);
            }));

        add_connection
          (m_game->signal_progress_changed ().connect
           ([this] ()
            {
              if (Playerlist::getActiveplayer () != Playerlist::getNeutral ())
                m_status_box->pulse ();
            }));

        add_connection
          (m_game->signal_stack_info_changed ().connect
           ([this] (Stack *s)
            {
              m_status_box->on_stack_info_changed (s);
            }));

        add_connection
          (m_game->signal_map_tip_changed ().connect
           ([this] (Glib::ustring message, MapTipPosition mpos)
            {
              if (message != "")
                {
                  if (m_map_info_tip)
                    {
                      m_map_info_tip->popdown ();
                      delete m_map_info_tip;
                    }
                  m_map_info_tip = new MapInfoTip ();
                  m_map_info_tip->set_justification (mpos.justification);
                  auto click = Gtk::GestureClick::create ();
                  click->set_button (1);
                  click->signal_pressed ().connect
                    ([this](int, double, double)
                     {
                       m_map_info_tip->popdown ();
                     });
                  m_map_info_tip->add_controller (click);
                  Gdk::Rectangle r ((int)mpos.pos.x, (int)mpos.pos.y, 1, 1);
                  m_map_info_tip->set_parent (*m_bigmap);
                  m_map_info_tip->set_pointing_to (r);
                  m_map_info_tip->set (message);
                  m_map_info_tip->popup ();
                }
              else
                {
                  if (m_map_info_tip)
                    {
                      m_map_info_tip->popdown ();
                      delete m_map_info_tip;
                      m_map_info_tip = NULL;
                    }
                }
            }));

        add_connection
          (m_game->signal_stack_tip_changed ().connect
           ([this] (StackTile *stile, MapTipPosition mpos)
            {
              if (stile)
                {
                  if (m_stack_info_tip)
                    {
                      m_stack_info_tip->popdown ();
                      delete m_stack_info_tip;
                    }
                  m_stack_info_tip = new StackInfoTip ();
                  m_stack_info_tip->set_justification (mpos.justification);
                  auto click = Gtk::GestureClick::create ();
                  click->set_button (1);
                  click->signal_pressed ().connect
                    ([this](int, double, double)
                     {
                       m_stack_info_tip->popdown ();
                     });
                  m_stack_info_tip->add_controller (click);
                  Gdk::Rectangle r ((int)mpos.pos.x, (int)mpos.pos.y, 1, 1);
                  m_stack_info_tip->set_parent (*m_bigmap);
                  m_stack_info_tip->set_pointing_to (r);
                  m_stack_info_tip->set (stile);
                  m_stack_info_tip->popup ();
                }
              else
                {
                  if (m_stack_info_tip)
                    {
                      m_stack_info_tip->popdown ();
                      delete m_stack_info_tip;
                      m_stack_info_tip = NULL;
                    }
                }
            }));

        add_connection
          (m_game->signal_city_tip_changed ().connect
           ([this] (City *city, MapTipPosition mpos)
            {
              if (city)
                {
                  if (m_city_info_tip)
                    {
                      m_city_info_tip->popdown ();
                      delete m_city_info_tip;
                    }
                  m_city_info_tip = new CityInfoTip ();
                  m_city_info_tip->set_justification (mpos.justification);
                  auto click = Gtk::GestureClick::create ();
                  click->set_button (1);
                  click->signal_pressed ().connect
                    ([this](int, double, double)
                     {
                       m_city_info_tip->popdown ();
                     });
                  m_city_info_tip->add_controller (click);
                  Gdk::Rectangle r ((int)mpos.pos.x, (int)mpos.pos.y, 1, 1);
                  m_city_info_tip->set_parent (*m_bigmap);
                  m_city_info_tip->set_pointing_to (r);
                  m_city_info_tip->set (city);
                  m_city_info_tip->popup ();
                }
              else
                {
                  if (m_city_info_tip)
                    {
                      m_city_info_tip->popdown ();
                      delete m_city_info_tip;
                      m_city_info_tip = NULL;
                    }
                }
            }));

        add_connection
          (m_game->signal_ruin_searched ().connect
           ([this] (Ruin *ruin, Stack *stack, Reward *reward)
            {
              auto d = LwDialog::build<RuinSearchedDialog> (this);
              d->setup (ruin, stack, reward);
              d->signal_response ().connect
                ([d] (Gtk::ResponseType)
                 {
                   delete d;
                 });
            }));

        add_connection
          (m_game->signal_sage_visited ().connect
           ([this] (Ruin *ruin, Sage *sage, Stack *stack,
                    sigc::slot<void(Reward*)> finish)
            {
              auto d = LwDialog::build<SageDialog> (this);
              d->setup (sage, static_cast<Hero*>(stack->getFirstHero ()), ruin);
              d->signal_response ().connect
                ([this, finish, sage, d] (Gtk::ResponseType resp)
                 {
                   switch (resp)
                     {
                     case Gtk::ResponseType::ACCEPT:
                       finish (sage->getSelectedReward ());
                       break;

                     default:
                       finish (NULL);
                       break;
                     }
                   delete d;
                 });

              return;
            }));

        add_connection
          (m_game->signal_fight_started ().connect
           ([this] (Fight *fight, sigc::slot<void(Fight *)> after)
            {
              LocationBox box =
                Fight::calculateFightBox (*fight);
              m_game->get_bigmap ()->set_fighting (box);
              m_game->get_bigmap ()->queue_draw ();

              Glib::signal_timeout ().connect_once
                ([this, fight, after, box] ()
                 {
                   auto d = new FightWindow (*this);
                   d->setup (fight);
                   d->signal_battle_finished ().connect
                     ([this, d, fight, after] () mutable
                      {
                        LocationBox nullbox = Vector<int>(-1, -1);
                        m_game->get_bigmap ()->set_fighting (nullbox);
                        m_game->get_bigmap ()->queue_draw ();
                        after (fight);
                        d->hide ();
                        delete d;
                      });
                   d->do_battle ();
                 }, s_displayBattleExplosionDelay);
            }));

        add_connection
          (m_game->signal_abbreviated_fight_started ().connect
           ([this] (Fight *fight, sigc::slot<void(Fight*)> after)
            {
              LocationBox box =
                Fight::calculateFightBox (*fight);
              m_game->get_bigmap ()->set_fighting (box);
              m_game->get_bigmap ()->queue_draw ();
              Glib::signal_timeout ().connect
                ([this, after, fight] () -> bool
                 {
                   LocationBox nullbox = Vector<int>(-1, -1);
                   m_game->get_bigmap ()->set_fighting (nullbox);
                   m_game->get_bigmap ()->queue_draw ();
                   after (fight);
                   return false;
                 }, s_displayAbbreviatedBattleExplosionDelay);
            }));

        add_connection
          (m_game->signal_ruinfight ().connect
           ([this] (Glib::ustring hero_name, Glib::ustring keeper_name, FightResult result, sigc::slot<void()> after)
            {
              if (keeper_name == "")
                {
                  after ();
                  return;
                }
              auto d = LwDialog::build<RuinFightDialog> (this);
              d->setup (hero_name, keeper_name, result);
              d->signal_ruinfight_finished ().connect
                ([after] ()
                 {
                   after ();
                 });
              d->signal_response ().connect
                ([d] (Gtk::ResponseType)
                 {
                   delete d;
                 });
            }));

        add_connection
          (m_game->signal_hero_offers_service ().connect
           ([this] (Player *player, HeroProto *hero, City *city, int gold, sigc::slot<void(bool,Glib::ustring,Hero::Gender)> finish)
            {
              auto d = LwDialog::build<HeroOfferDialog> (this);
              d->setup (player, hero, city, gold);
              d->signal_offer_accepted ().connect
                ([finish] (Glib::ustring name, Hero::Gender gender)
                 {
                   finish (true, name, gender);
                 });
              d->signal_offer_declined ().connect
                ([finish] ()
                 {
                   finish (false, "", Hero::Gender::NONE);
                 });
              d->signal_response ().connect
                ([d] (Gtk::ResponseType)
                 {
                   delete d;
                 });
              return;
            }));

        add_connection
          (m_game->signal_enemy_offers_surrender ().connect
           ([this] (int num_players, sigc::slot<void(bool)> after)
            {
              auto d = LwDialog::build<SurrenderDialog> (this);
              d->setup (num_players);
              d->signal_response ().connect
                ([d, after] (Gtk::ResponseType resp)
                 {
                   after (resp == Gtk::ResponseType::ACCEPT);
                   delete d;
                 });
                return;
            }));

        add_connection
          (m_game->signal_surrender_answered ().connect
           ([this] (bool accepted, sigc::slot<void(bool)> after)
            {
              if (accepted)
                {
                  auto d = LwDialog::build<SurrenderAcceptedDialog> (this);
                  d->setup ();
                  d->signal_response ().connect
                    ([d, after, accepted] (Gtk::ResponseType)
                     {
                       after (accepted);
                       delete d;
                     });
                }
              else
                {
                  auto d = LwDialog::build<SurrenderRefusedDialog> (this);
                  d->setup ();
                  d->signal_response ().connect
                    ([d, after, accepted] (Gtk::ResponseType)
                     {
                       after (accepted);
                       delete d;
                     });
                }
            }));

        add_connection
          (m_game->signal_stack_considers_treachery ().connect
           ([this] (Player *enemy, sigc::slot<void(bool)> finish)
            {
              auto d = LwDialog::build<TreacheryDialog> (this);
              d->setup (enemy);
              d->signal_response ().connect
                ([d, finish] (Gtk::ResponseType resp)
                 {
                   switch (resp)
                     {
                     case Gtk::ResponseType::ACCEPT:
                       finish (true);
                       break;
                     default:
                       finish (false);
                       break;
                     }
                   delete d;
                 });

              return;
            }));

        add_connection
          (m_game->signal_search_temple ().connect
           ([this] (Hero *hero, Temple *temple, int armies_blessed,
                    Game::SearchStackCallback after)
            {
              auto d = LwDialog::build<SearchTempleDialog> (this);
              d->setup (hero, temple, armies_blessed);
              d->signal_temple_searched ().connect
                ([this, d, armies_blessed, hero, temple, after] (bool got_quest)
                 {
                   d->hide ();
                   if (got_quest)
                     {
                       Player *p = Playerlist::getActiveplayer ();
                       Quest *quest = p->heroGetQuest 
                         (hero, temple, 
                          GameScenario::s_razing_cities != GameParameters::NEVER);
                       auto dd = LwDialog::build<QuestAssignedDialog> (this);
                       dd->setup (hero, quest);
                       dd->signal_response ().connect
                         ([armies_blessed, got_quest, dd, after] (Gtk::ResponseType)
                          {
                            after (got_quest, armies_blessed, false);
                            delete dd;
                          });
                     }
                   else
                     after (got_quest, armies_blessed, false);
                 });
              d->signal_response ().connect
                ([d] (Gtk::ResponseType)
                 {
                   delete d;
                 });

              return;
            }));

        add_connection
          (m_game->signal_city_defeated ().connect
           ([this] (City *city, sigc::slot<void(CityDefeatedChoice)> after)
            {
              auto d = LwDialog::build<CityDefeatedDialog> (this);
              d->setup (city);

              d->signal_response ().connect
                ([d, after] (Gtk::ResponseType resp)
                 {
                   switch (resp)
                     {
                     case Gtk::ResponseType::ACCEPT:
                       after (CityDefeatedChoice::CITY_DEFEATED_OCCUPY);
                       break;
                     case Gtk::ResponseType::CLOSE:
                       after (CityDefeatedChoice::CITY_DEFEATED_PILLAGE);
                       break;
                     case Gtk::ResponseType::OK:
                       after (CityDefeatedChoice::CITY_DEFEATED_SACK);
                       break;
                     case Gtk::ResponseType::REJECT:
                       after (CityDefeatedChoice::CITY_DEFEATED_RAZE);
                       break;
                     default:
                       after (CityDefeatedChoice::CITY_DEFEATED_OCCUPY);
                       break;
                     }
                   delete d;
                 });
            }));

        add_connection
          (m_game->signal_open_city_dialog ().connect
           ([this] (City *city, sigc::slot<void()> after)
            {
              auto d = LwDialog::build<CityDialog> (this);
              d->setup (city, GSO::s_razing_cities == GameParameters::ALWAYS,
                        GSO::s_see_opponents_production);
              d->signal_response ().connect
                ([d, this, after] (Gtk::ResponseType)
                 {
                   m_game->redraw ();
                   after ();
                   delete d;
                 });
            }));

        add_connection
          (m_game->signal_city_pillaged ().connect
           ([this] (City *city, int gold, int pillaged_army_type, sigc::slot<void()> after)
            {
              auto d = LwDialog::build<CityPillagedDialog> (this);
              d->setup (city, gold, pillaged_army_type);
              d->signal_response ().connect
                ([d, after] (Gtk::ResponseType)
                 {
                   after ();
                   delete d;
                 });
            }));

        add_connection
          (m_game->signal_city_sacked ().connect
           ([this] (City *city, int gold, std::list<guint32> sacked_army_types, sigc::slot<void()> after)
            {
              auto d = LwDialog::build<CitySackedDialog> (this);
              d->setup (city, gold, sacked_army_types);
              d->signal_response ().connect
                ([d, after] (Gtk::ResponseType)
                 {
                   after ();
                   delete d;
                 });
            }));

        add_connection
          (m_game->signal_city_raze_query ().connect
           ([this] (City *city, sigc::slot<void(bool)> finish)
            {
              auto d = LwDialog::build<CityRazeDialog> (this);
              d->setup (city, true,
                        [] (bool)
                        {
                        });
              d->signal_response ().connect
                ([this, d, finish] (Gtk::ResponseType resp)
                 {
                   bool raze = resp == Gtk::ResponseType::ACCEPT;
                   finish (raze);
                   m_game->redraw ();
                   delete d;
                 });
            }));

        add_connection
          (m_game->signal_city_razed ().connect
           ([this] (City *city, sigc::slot<void()> finish)
            {
              auto d = LwDialog::build<CityRazedDialog> (this);
              d->setup (city);
              d->signal_response ().connect
                ([finish, d] (Gtk::ResponseType)
                 {
                   finish ();
                   delete d;
                 });
            }));

        add_connection
          (m_game->signal_city_visited ().connect
           ([this] (City *city, sigc::slot<void()> after)
            {
              auto d = LwDialog::build<CityDialog> (this);
              d->setup (city, GSO::s_razing_cities == GameParameters::ALWAYS,
                        GSO::s_see_opponents_production);
              d->signal_response ().connect
                ([d ,after] (Gtk::ResponseType)
                 {
                   after ();
                   delete d;
                 });
            }));

        add_connection
          (m_game->signal_ruin_visited ().connect
           ([this] (Ruin *ruin)
            {
              auto d = LwDialog::build<RuinReportDialog> (this);
              d->setup (ruin);
              d->signal_response ().connect
                ([d] (Gtk::ResponseType)
                 {
                   delete d;
                 });
            }));

        add_connection
          (m_game->signal_temple_visited ().connect
           ([this] (Temple *temple)
            {
              auto d = LwDialog::build<RuinReportDialog> (this);
              d->setup (temple);
              d->signal_response ().connect
                ([d] (Gtk::ResponseType)
                 {
                   delete d;
                 });
            }));

        add_connection
          (m_game->signal_next_turn ().connect
           ([this] (Player *player, sigc::slot<void()> finish)
            {
              m_status_box->reset_progress (player);
              m_status_box->on_stack_info_changed (NULL);
              if (player->getType() != Player::HUMAN)
                {
                  finish ();
                  return;
                }
                 
              auto d = LwDialog::build<NextTurnDialog> (this);
              d->setup (player, m_game_scenario->getRound ());
              d->signal_response ().connect
                ([d, finish] (Gtk::ResponseType)
                 {
                   finish ();
                   delete d;
                 });
            }));

        add_connection
          (m_game->signal_hero_brings_allies ().connect
           ([this] (int num_allies, sigc::slot<void()> finish)
            {
              auto d = LwDialog::build<HeroBringsAlliesDialog> (this);
              d->setup (num_allies);
              d->signal_response ().connect
                ([d, finish] (Gtk::ResponseType)
                 {
                   finish ();
                   delete d;
                 });
            }));

        add_connection
          (m_game->signal_game_loaded ().connect
           ([this] (Player *player)
            {
              auto d = LwDialog::build<GameLoadedDialog> (this);
              d->setup (player);
              d->signal_response ().connect
                ([d](Gtk::ResponseType)
                 {
                   delete d;
                 });
            }));

        add_connection
          (m_game->signal_game_over ().connect
           ([this] (Player *player, sigc::slot<void()> after)
            {
              auto d = LwDialog::build<GameOverDialog> (this);
              d->setup (player);
              d->signal_response ().connect
                ([this, d, after, player] (Gtk::ResponseType)
                 {
                   m_game_winner = player;
                   give_some_cheese ();
                   stop_game ("game-over");
                   after ();
                   delete d;
                 });
            }));

        add_connection
          (m_game->signal_player_died ().connect
           ([this] (Player *player, std::shared_ptr<sigc::slot<void()>> after)
            {
              auto d = LwDialog::build<PlayerDiedDialog> (this);
              d->set_timeout (30);
              d->setup (player);
              d->signal_response ().connect
                ([d, after] (Gtk::ResponseType)
                 {
                   (*after) ();
                   delete d;
                 });

            }));

        add_connection
          (m_game->signal_advice_asked ().connect
           ([this] (float chances)
            {
              auto d = LwDialog::build<MilitaryAdvisorDialog> (this);
              d->setup (chances);
              d->signal_response ().connect
                ([d] (Gtk::ResponseType)
                 {
                   delete d;
                 });
            }));

        add_connection
          (m_game->signal_sunk_ships ().connect
           ([this] (Player *player, guint32 num_armies)
            {
              auto d = LwDialog::build<ShipsSunkDialog> (this);
              d->setup (player, num_armies);
              d->signal_response ().connect
                ([d] (Gtk::ResponseType)
                 {
                   delete d;
                 });
            }));

        add_connection
          (m_game->signal_bags_picked_up ().connect
           ([this] (Hero *hero, guint32 num_bags)
            {
              auto d = LwDialog::build<BagsPickedUpDialog> (this);
              d->setup (hero, num_bags);
              d->signal_response ().connect
                ([d] (Gtk::ResponseType)
                 {
                   delete d;
                 });
            }));

        add_connection
          (m_game->signal_mp_added_to_hero_stack ().connect
           ([this] (Hero *hero, guint32 mp)
            {
              auto d = LwDialog::build<MPAddedToHeroStackDialog> (this);
              d->setup (hero, mp);
              d->signal_response ().connect
                ([this, d] (Gtk::ResponseType)
                 {
                   auto s = Playerlist::getActiveplayer ()->getActivestack ();
                   if (s)
                     m_game->recalculate_moves_for_stack (s);
                   delete d;
                 });
            }));

        add_connection
          (m_game->signal_worms_killed ().connect
           ([this] (Hero *hero, Glib::ustring name, guint32 mp)
            {
              auto d = LwDialog::build<WormsKilledDialog> (this);
              d->setup (hero, name, mp);
              d->signal_response ().connect
                ([d] (Gtk::ResponseType)
                 {
                   delete d;
                 });
            }));

        add_connection
          (m_game->signal_bridge_burned ().connect
           ([this] (Hero *hero)
            {
              auto d = LwDialog::build<BridgeBurnedDialog> (this);
              d->setup (hero);
              d->signal_response ().connect
                ([d] (Gtk::ResponseType)
                 {
                   delete d;
                 });
            }));

        add_connection
          (m_game->signal_keeper_captured ().connect
           ([this] (Hero *hero, Ruin *ruin, Glib::ustring keeper_name)
            {
              auto d = LwDialog::build<KeeperCapturedDialog> (this);
              d->setup (hero, ruin, keeper_name);
              d->signal_response ().connect
                ([d] (Gtk::ResponseType)
                 {
                   delete d;
                 });
            }));

        add_connection
          (m_game->signal_monster_summoned ().connect
           ([this] (Hero *hero, Glib::ustring monster_army_type_name)
            {
              auto d = LwDialog::build<MonsterSummonedDialog> (this);
              d->setup (hero, monster_army_type_name);
              d->signal_response ().connect
                ([d] (Gtk::ResponseType)
                 {
                   delete d;
                 });
            }));

        add_connection
          (m_game->signal_stole_gold ().connect
           ([this] (Player *victim, guint32 gold)
            {
              auto d = LwDialog::build<GoldStolenDialog> (this);
              d->setup (victim, gold);
              d->signal_response ().connect
                ([d] (Gtk::ResponseType)
                 {
                   delete d;
                 });
            }));

        add_connection
          (m_game->signal_stack_moves ().connect
           ([this] (Stack *stack, Vector<int> pos)
            {
              Player *active = Playerlist::instance ()->getActiveplayer ();
              if (!active)
                return;
              if (active->getActivestack () != stack)
                return;
              if (GameMap::getEnemyCity (pos))
                return;
              if (GameMap::getEnemyStack (pos))
                return;

              m_game->get_bigmap ()->queue_draw ();
            }));

        add_connection
          (m_game->signal_select_item_victim_player ().connect
           ([this] (Item *item)
            {
              auto d = LwDialog::build<UseItemOnPlayerDialog> (this);
              d->setup ();
              d->signal_player_selected ().connect
                ([this, item] (Player *player)
                 {
                   if (player)
                     m_game->use_item_on_player (item, player);
                 });
              d->signal_response ().connect
                ([d] (Gtk::ResponseType)
                 {
                   delete d;
                 });
              return;
            }));

        add_connection
          (m_game->signal_select_city_to_use_item_on ().connect
           ([this] (Item *item, SelectCityMap::Type type) 
            {
              auto d = LwDialog::build<UseItemOnCityDialog> (this);
              d->setup (type);
              d->signal_city_selected ().connect
                ([this, d, item, type] (City *city)
                 {
                   switch (type)
                     {
                     case SelectCityMap::FRIENDLY_CITY:
                       m_game->use_item_on_friendly_city (item, city);
                       break;
                     case SelectCityMap::ENEMY_CITY:
                       m_game->use_item_on_enemy_city (item, city);
                       break;
                     case SelectCityMap::NEUTRAL_CITY:
                       m_game->use_item_on_neutral_city (item, city);
                       break;
                     case SelectCityMap::ANY_CITY:
                       m_game->use_item_on_any_city (item, city);
                       break;
                     }
                 });
              d->signal_response ().connect
                ([d] (Gtk::ResponseType)
                 {
                   delete d;
                 });

              return;
            }));

        add_connection
          (m_game->signal_city_diseased ().connect
           ([this] (Glib::ustring city_name, guint32 num_armies)
            {
              auto d = LwDialog::build<CityDiseasedDialog> (this);
              d->setup (city_name, num_armies);
              d->signal_response ().connect
                ([d] (Gtk::ResponseType)
                 {
                   delete d;
                 });
            }));

        add_connection
          (m_game->signal_city_defended ().connect
           ([this] (Glib::ustring city_name,
                    Glib::ustring army_type_name, guint32 num_armies)
            {
              auto d = LwDialog::build<CityDefendedDialog> (this);
              d->setup (city_name, army_type_name, num_armies);
              d->signal_response ().connect
                ([d] (Gtk::ResponseType)
                 {
                   delete d;
                 });
            }));

        add_connection
          (m_game->signal_city_persuaded ().connect
           ([this] (Glib::ustring city_name, guint32 num_armies)
            {
              auto d = LwDialog::build<CityPersuadedDialog> (this);
              d->setup (city_name, num_armies);
              d->signal_response ().connect
                ([this, d] (Gtk::ResponseType)
                 {
                   m_game->redraw ();
                   delete d;
                 });
            }));

        add_connection
          (m_game->signal_stack_teleported ().connect
           ([this] (Hero *hero, Glib::ustring city_name)
            {
              auto d = LwDialog::build<StackTeleportedDialog> (this);
              d->setup (hero, city_name);
              d->signal_response ().connect
                ([d] (Gtk::ResponseType)
                 {
                   delete d;
                 });
            }));

        add_connection
          (m_game->signal_remote_next_player_turn ().connect
           ([this] ()
            {
              int round_number = m_game_scenario->getRound ();
              m_main_turn_indicator->update_round (round_number);
              m_fullscreen_turn_indicator->update_round (round_number);
              /*
                 status_box->reset_progress ();
                 status_box->on_stack_info_changed (NULL);
                 Lw::do_events ();

                 update both of our turn indicators
                 show_shield_turn();
                 turn_label->set_markup(String::ucompose("<b>%1 %2</b>", _("Turn"),
                 GameScenarioOptions::s_round));
                 */
            }));

        add_connection
          (m_game->signal_quest_completed ().connect
           ([this] (Quest *quest, sigc::slot<void()> finish)
            {
              if (Playerlist::getActiveplayer ()->getType () != Player::HUMAN)
                return;
              auto d = LwDialog::build<QuestCompletedDialog> (this);
              d->setup (quest);
              d->signal_response ().connect
                ([d, finish, quest] (Gtk::ResponseType)
                 {
                   Reward *reward = quest->getReward ();
                   delete reward;
                   finish ();
                   delete d;
                 });
            }));

        add_connection
          (m_game->signal_quest_expired ().connect
           ([this] (Quest *quest, sigc::slot<void()> finish)
            {
              if (Playerlist::getActiveplayer ()->getType () != Player::HUMAN)
                return;
              auto d = LwDialog::build<QuestExpiredDialog> (this);
              d->setup (quest);
              d->signal_response ().connect
                ([d, finish] (Gtk::ResponseType)
                 {
                   finish ();
                   delete d;
                 });
            }));

        add_connection
          (m_game->signal_received_diplomatic_proposal ().connect
           ([this] (bool active)
            {
              m_game_button_box->update_diplomacy_button (active);
            }));

        add_connection
          (m_game->signal_can_select_next_movable_stack ().connect
           ([this] (bool active)
            {
              m_simple_actions["order.next"]->set_enabled (active);
            }));

        add_connection
          (m_game->signal_can_center_selected_stack ().connect
           ([this] (bool active)
            {
              m_simple_actions["view.center"]->set_enabled (active);
            }));

        add_connection
          (m_game->signal_can_defend_selected_stack ().connect
           ([this] (bool active)
            {
              m_simple_actions["order.defend"]->set_enabled (active);
            }));

        add_connection
          (m_game->signal_can_park_selected_stack ().connect
           ([this] (bool active)
            {
              m_simple_actions["order.stay-here"]->set_enabled (active);
            }));

        add_connection
          (m_game->signal_can_deselect_selected_stack ().connect
           ([this] (bool active)
            {
              m_simple_actions["order.deselect"]->set_enabled (active);
            }));

        add_connection
          (m_game->signal_can_search_selected_stack ().connect
           ([this] (bool active)
            {
              m_simple_actions["hero.search"]->set_enabled (active);
            }));

        add_connection
          (m_game->signal_can_inspect ().connect
           ([this] (bool active)
            {
              m_simple_actions["hero.inspect"]->set_enabled (active);
            }));

        add_connection
          (m_game->signal_can_see_hero_levels ().connect
           ([this] (bool active)
            {
              m_simple_actions["hero.levels"]->set_enabled (active);
            }));

        add_connection
          (m_game->signal_can_use_item ().connect
           ([this] (bool active)
            {
              m_simple_actions["hero.use-item"]->set_enabled (active);
            }));

        add_connection
          (m_game->signal_can_plant_standard_selected_stack ().connect
           ([this] (bool active)
            {
              m_simple_actions["hero.plant-flag"]->set_enabled (active);
            }));

        add_connection
          (m_game->signal_can_move_selected_stack_along_path ().connect
           ([this] (bool active)
            {
              m_simple_actions["order.move"]->set_enabled (active);
            }));

        add_connection
          (m_game->signal_can_group_ungroup_selected_stack ().connect
           ([this] (bool active)
            {
              m_simple_actions["order.toggle-group"]->set_enabled (active);
            }));

        add_connection
          (m_game->signal_can_move_all_stacks ().connect
           ([this] (bool active)
            {
              m_simple_actions["order.move-all"]->set_enabled (active);
            }));

        add_connection
          (m_game->signal_can_disband_stack ().connect
           ([this] (bool active)
            {
              m_simple_actions["order.disband"]->set_enabled (active);
            }));

        add_connection
          (m_game->signal_can_change_signpost ().connect
           ([this] (bool active)
            {
              m_simple_actions["order.signpost"]->set_enabled (active);
            }));

        add_connection
          (m_game->signal_can_see_city_history ().connect
           ([this] (bool active)
            {
              m_simple_actions["history.city"]->set_enabled (active);
            }));

        add_connection
          (m_game->signal_can_see_ruin_history ().connect
           ([this] (bool active)
            {
              m_simple_actions["history.ruins"]->set_enabled (active);
            }));

        add_connection
          (m_game->signal_can_see_event_history ().connect
           ([this] (bool active)
            {
              m_simple_actions["history.events"]->set_enabled (active);
            }));

        add_connection
          (m_game->signal_can_see_gold_history ().connect
           ([this] (bool active)
            {
              m_simple_actions["history.gold"]->set_enabled (active);
            }));

        add_connection
          (m_game->signal_can_see_winning_history ().connect
           ([this] (bool active)
            {
              m_simple_actions["history.winners"]->set_enabled (active);
            }));

        add_connection
          (m_game->signal_can_see_triumph_history ().connect
           ([this] (bool active)
            {
              m_simple_actions["history.triumphs"]->set_enabled (active);
            }));

        add_connection
          (m_game->signal_can_save_game ().connect
           ([this] (bool active)
            {
              m_simple_actions["game.save"]->set_enabled (active);
              m_simple_actions["game.save-as"]->set_enabled (active);
            }));

        add_connection
          (m_game->signal_can_load_game ().connect
           ([this] (bool active)
            {
              m_simple_actions["game.open"]->set_enabled (active);
            }));

        add_connection
          (m_game->signal_can_new_game ().connect
           ([this] (bool active)
            {
              m_simple_actions["game.new"]->set_enabled (active);
            }));

        add_connection
          (m_game->signal_can_change_fight_order ().connect
           ([this] (bool active)
            {
              m_simple_actions["order.fight-order"]->set_enabled (active);
            }));

        add_connection
          (m_game->signal_can_resign ().connect
           ([this] (bool active)
            {
              m_simple_actions["order.resign"]->set_enabled (active);
            }));

        add_connection
          (m_game->signal_can_see_diplomacy_report ().connect
           ([this] (bool active)
            {
              m_simple_actions["reports.diplomacy"]->set_enabled (active);
            }));

        add_connection
          (m_game->signal_can_see_army_report ().connect
           ([this] (bool active)
            {
              m_simple_actions["reports.army"]->set_enabled (active);
            }));

        add_connection
          (m_game->signal_can_see_city_report ().connect
           ([this] (bool active)
            {
              m_simple_actions["reports.city"]->set_enabled (active);
            }));

        add_connection
          (m_game->signal_can_see_gold_report ().connect
           ([this] (bool active)
            {
              m_simple_actions["reports.gold"]->set_enabled (active);
            }));

        add_connection
          (m_game->signal_can_see_production_report ().connect
           ([this] (bool active)
            {
              m_simple_actions["reports.production"]->set_enabled (active);
            }));

        add_connection
          (m_game->signal_can_see_winning_report ().connect
           ([this] (bool active)
            {
              m_simple_actions["reports.winning"]->set_enabled (active);
            }));

        add_connection
          (m_game->signal_can_see_quest_report ().connect
           ([this] (bool active)
            {
              m_simple_actions["reports.quest"]->set_enabled (active);
            }));

        add_connection
          (m_game->signal_can_see_items_report ().connect
           ([this] (bool active)
            {
              m_simple_actions["reports.items"]->set_enabled (active);
            }));

        add_connection
          (m_game->signal_can_see_view_menu_army_bonus ().connect
           ([this] (bool active)
            {
              m_simple_actions["view.army-bonus"]->set_enabled (active);
            }));

        add_connection
          (m_game->signal_can_see_view_menu_items ().connect
           ([this] (bool active)
            {
              m_simple_actions["view.items"]->set_enabled (active);
            }));

        add_connection
          (m_game->signal_can_see_view_menu_cities ().connect
           ([this] (bool active)
            {
              m_simple_actions["view.cities"]->set_enabled (active);
            }));

        add_connection
          (m_game->signal_can_see_view_menu_vectoring ().connect
           ([this] (bool active)
            {
              m_simple_actions["view.vectoring"]->set_enabled (active);
            }));

        add_connection
          (m_game->signal_can_see_view_menu_ruins ().connect
           ([this] (bool active)
            {
              m_simple_actions["view.ruins"]->set_enabled (active);
            }));

        add_connection
          (m_game->signal_can_see_view_menu_stack ().connect
           ([this] (bool active)
            {
              m_simple_actions["view.stack"]->set_enabled (active);
            }));

        add_connection
          (m_game->signal_can_see_view_menu_diplomacy ().connect
           ([this] (bool active)
            {
              m_simple_actions["view.diplomacy"]->set_enabled (active);
            }));

        add_connection
          (m_game->signal_can_launch_tutorial_video ().connect
           ([this] (bool active)
            {
              m_simple_actions["help.tutorial-video"]->set_enabled (active);
            }));

        add_connection
          (m_game->signal_can_launch_online_help ().connect
           ([this] (bool active)
            {
              m_simple_actions["help.online-help"]->set_enabled (active);
            }));

        add_connection
          (m_game->signal_can_see_about_dialog ().connect
           ([this] (bool active)
            {
              m_simple_actions["help.about"]->set_enabled (active);
            }));

        add_connection
          (m_game->signal_can_see_keyboard_shortcuts_dialog ().connect
           ([this] (bool active)
            {
              m_simple_actions["help.keyboard-shortcuts"]->set_enabled (active);
            }));

        add_connection
          (m_game->signal_can_end_turn ().connect
           ([this] (bool active)
            {
              m_simple_actions["turn.end-turn"]->set_enabled (active);
            }));

        add_connection
          (m_game->signal_can_show_lobby ().connect
           ([this] (bool active)
            {
              m_simple_actions["game.show-lobby"]->set_enabled (active);
            }));

        add_connection
          (m_game->signal_looting_city ().connect
           ([this] (int gp_looted, sigc::slot<void()> finish)
            {
              auto d = LwDialog::build<CityLootedDialog> (this);
              d->setup (gp_looted);
              d->signal_response ().connect
                ([d, finish] (Gtk::ResponseType)
                 {
                   finish ();
                   delete d;
                 });
            }));
      }

    void setup_action_signals ()
      {
        // it's our big list of actions
        action_connect
          ("view.zoom-in",
           [this] ()
           {
             m_bigmap->zoom_in ();
           });

        action_connect
          ("view.zoom-out",
           [this] ()
           {
             m_bigmap->zoom_out ();
           });

        action_connect
          ("view.reset-zoom",
           [this] ()
           {
             m_bigmap->reset_zoom ();
             m_menu_button->get_popover ()->popdown ();
             m_hidden_menu_button->get_popover ()->popdown ();
           });

        action_connect
          ("view.toggle-fullscreen",
           [this] ()
           {
             //shut them all down R2
             m_menu_button->get_popover ()->popdown ();
             m_hidden_menu_button->get_popover ()->popdown ();
             if (is_fullscreen ())
               {
                 m_headerbar_revealer->set_reveal_child (false);
                 unfullscreen ();
               }
             else
               fullscreen ();
           });

        action_connect
          ("game.quit",
          [this] ()
           {
             close ();
           });

        action_connect
          ("help.keyboard-shortcuts",
          [this] ()
          {
            on_show_keyboard_shortcuts ();
          });

        action_connect
          ("game.new",
           [this] ()
           {
             auto d =
               new QuitConfirmationDialog
               (*this,
                [this](bool should_quit)
                {
                  if (should_quit)
                    {
                      stop_game ("new");
                    }
                });

             d->signal_hide ().connect
               ([this, d]()
                {
                  delete d;
                });

             d->present ();
           });

        action_connect
          ("game.open",
           [this] ()
           {
             auto dialog = Gtk::FileDialog::create ();
             dialog->set_title (_("Load A Game"));

             auto filter = Gtk::FileFilter::create ();
             filter->set_name
               (String::ucompose (_("LordsAWar Saved Games (*%1)"), SAVE_EXT));
             filter->add_pattern ("*" + SAVE_EXT);
             auto filters = Gio::ListStore<Gtk::FileFilter>::create ();
             filters->append (filter);
             dialog->set_filters (filters);
             dialog->open
               (*this,
                [this, dialog](const Glib::RefPtr<Gio::AsyncResult>& result)
                {
                   try 
                     {
                       auto file = dialog->open_finish (result);
                       if (file)
                         {
                          m_current_save_filename = file->get_path ();
                          if (m_current_save_filename ==
                              File::getSaveFile("autosave" + SAVE_EXT))
                            m_game->inhibitAutosaveRemoval (true);
                          m_current_load_filename = m_current_save_filename;
                          stop_game ("load-game");
                         }
                     }
                   catch (const Gtk::DialogError& e)
                     {
                       // User pressed Escape or closed dialog
                       if (e.code() == Gtk::DialogError::Code::DISMISSED)
                         {
                           return;
                         }
                     }
                   catch (const Glib::Error& e)
                     {
                       std::cerr << "Error: " << e.what() << '\n';
                     }
                });
           });


        action_connect
          ("game.save",
           [this] ()
           {
             if (m_current_save_filename.empty ())
               {
                 auto action =
                   std::dynamic_pointer_cast<Gio::SimpleAction>
                   (m_simple_actions["game.save-as"]);
                 action->activate ();
               }
             else
               {
                 if (m_game)
                   m_game->saveGame (m_current_save_filename);
               }

           });

        action_connect
          ("game.save-as",
           [this] ()
           {
             auto dialog = Gtk::FileDialog::create ();
             dialog->set_title (_("Save A Game"));

             auto filter = Gtk::FileFilter::create ();
             filter->set_name
               (String::ucompose (_("LordsAWar Saved Games (*%1)"), SAVE_EXT));
             filter->add_pattern ("*" + SAVE_EXT);
             auto filters = Gio::ListStore<Gtk::FileFilter>::create ();
             filters->append (filter);
             dialog->set_filters (filters);
             dialog->save
               (*this,
                [this, dialog](const Glib::RefPtr<Gio::AsyncResult>& result)
                {
                   try 
                     {
                       auto file = dialog->save_finish (result);
                       if (file)
                         {
                           m_current_save_filename = file->get_path ();
                           m_game->saveGame (m_current_save_filename);
                         }
                     }
                   catch (const Gtk::DialogError& e)
                     {
                       // User pressed Escape or closed dialog
                       if (e.code() == Gtk::DialogError::Code::DISMISSED)
                         {
                           return;
                         }
                     }
                   catch (const Glib::Error& e)
                     {
                       std::cerr << "Error: " << e.what() << '\n';
                     }
                });
           });

        action_connect
          ("game.show-lobby",
           [this] ()
           {
             m_show_lobby.emit ();
           });

        action_connect
          ("game.scenario-info",
           [this] ()
           {
             auto d = LwDialog::build<ScenarioInfoDialog> (this);
             d->setup (m_game_scenario);
             d->signal_response ().connect
               ([d] (Gtk::ResponseType)
                {
                  delete d;
                });
           });

        action_connect
          ("order.fight-order",
           [this] ()
           {
             auto d = LwDialog::build<FightOrderDialog> (this);
             d->setup (Playerlist::getActiveplayer ());
             d->signal_change_fight_order ().connect
               ([] (std::list<guint32> order)
                {
                  Playerlist::getActiveplayer ()->setFightOrder (order);
                });
             d->signal_response ().connect
               ([d] (Gtk::ResponseType)
                {
                  delete d;
                });
           });

        action_connect
          ("order.move-all",
           [this] ()
           {
             m_game->move_all_stacks ();
           });

        action_connect
          ("order.disband",
           [this] ()
           {
             Player *p = Playerlist::getActiveplayer ();
             Stack *s = p->getActivestack ();
             if (s)
               {
                 auto d = LwDialog::build<StackDisbandDialog> (this);
                 d->setup (s);
                 d->signal_response ().connect
                   ([d, s, p] (Gtk::ResponseType resp)
                    {
                      d->hide ();
                      if (resp == Gtk::ResponseType::ACCEPT)
                        p->stackDisband (s);
                      delete d;
                    });
               }
           });

        action_connect
          ("order.signpost",
           [this] ()
           {
             Player *p = Playerlist::getActiveplayer ();
             Stack *s = p->getActivestack ();
             Signpost *sign = NULL;
             if (s)
               sign = GameMap::getSignpost (s->getPos ());
             if (sign)
               {
                 auto d = LwDialog::build<SignpostChangeDialog> (this);
                 d->setup (sign);
                 d->signal_response ().connect
                   ([d, sign, p] (Gtk::ResponseType resp)
                    {
                      d->hide ();
                      if (resp == Gtk::ResponseType::ACCEPT)
                        p->signpostChange (sign, d->get_message ());
                      delete d;
                    });
               }
           });

        action_connect
          ("order.toggle-group",
           [this] ()
           {
             m_status_box->toggle_group_ungroup ();
           });

        action_connect
          ("order.stay-here",
           [this] ()
           {
             m_game->park_selected_stack ();
           });

        action_connect
          ("order.next",
           [this] ()
           {
             m_game->select_next_movable_stack ();
           });

        action_connect
          ("order.resign",
           [this] ()
           {
             auto d = LwDialog::build<ResignDialog> (this);
             d->setup ();
             d->signal_response ().connect
               ([this, d] (Gtk::ResponseType resp)
                {
                  if (resp == Gtk::ResponseType::ACCEPT)
                    {
                      Playerlist::getActiveplayer ()->resign ();
                      auto dd = LwDialog::build<ResignCompletedDialog> (this);
                      dd->setup ();
                      dd->signal_response ().connect
                        ([dd] (Gtk::ResponseType)
                         {
                           dd->hide ();
                           delete dd;
                         });
                    }
                  d->hide ();
                  delete d;
                });
           });

        action_connect
          ("reports.army",
           [this] ()
           {
             auto d = LwDialog::build<ReportDialog> (this);
             d->setup (ReportDialog::ReportType::ARMY);
             d->signal_response ().connect
               ([d] (Gtk::ResponseType)
                {
                  delete d;
                });
           });

        action_connect
          ("reports.city",
           [this] ()
           {
             auto d = LwDialog::build<ReportDialog> (this);
             d->setup (ReportDialog::ReportType::CITY);
             d->signal_response ().connect
               ([d] (Gtk::ResponseType)
                {
                  delete d;
                });
           });

        action_connect
          ("reports.gold",
           [this] ()
           {
             auto d = LwDialog::build<ReportDialog> (this);
             d->setup (ReportDialog::ReportType::GOLD);
             d->signal_response ().connect
               ([d] (Gtk::ResponseType)
                {
                  delete d;
                });
           });

        action_connect
          ("reports.production",
           [this] ()
           {
             auto d = LwDialog::build<ReportDialog> (this);
             d->setup (ReportDialog::ReportType::PRODUCTION);
             d->signal_response ().connect
               ([d] (Gtk::ResponseType)
                {
                  delete d;
                });
           });

        action_connect
          ("reports.winning",
           [this] ()
           {
             auto d = LwDialog::build<ReportDialog> (this);
             d->setup (ReportDialog::ReportType::WINNING);
             d->signal_response ().connect
               ([d] (Gtk::ResponseType)
                {
                  delete d;
                });
           });

        action_connect
          ("reports.diplomacy",
           [this] ()
           {
             auto d = LwDialog::build<DiplomacyReportDialog> (this);
             d->setup ();
             d->signal_response ().connect
               ([d] (Gtk::ResponseType)
                {
                  delete d;
                });
           });

        action_connect
          ("reports.quest",
           [this] ()
           {
             Player *p = Playerlist::getActiveplayer ();
             std::vector<Quest*> quests =
               QuestsManager::instance ()->getPlayerQuests (p);
             Stack *s = p->getActivestack ();
             Hero *hero = NULL;
             if (s)
               hero = s->getFirstHeroWithAQuest ();
             auto d = LwDialog::build<QuestReportDialog> (this);
             d->setup (quests, hero);
             d->signal_response ().connect
               ([d] (Gtk::ResponseType)
                {
                  delete d;
                });
           });

        action_connect
          ("reports.items",
           [this] ()
           {
             auto d = LwDialog::build<ItemReportDialog> (this);
             std::list<Stack*> stacks =
               Playerlist::getActiveplayer ()->getStacksWithItems ();
             std::list<MapBackpack*> bags =
               GameMap::instance ()->getBackpacks ();
             d->setup (stacks, bags);
             d->signal_response ().connect
               ([d] (Gtk::ResponseType)
                {
                  delete d;
                });
           });

        action_connect
          ("hero.inspect",
           [this] ()
           {
             Hero *hero = NULL;
             if (m_status_box->get_currently_selected_stack () != NULL)
               {
                 auto s = m_status_box->get_currently_selected_stack ();
                 hero = dynamic_cast<Hero*>(s->getFirstHero ());
               }
             auto d = LwDialog::build<HeroDialog> (this);
             d->setup (hero);
             d->signal_response ().connect
               ([d] (Gtk::ResponseType)
                {
                  delete d;
                });
           });

        action_connect
          ("hero.plant-flag",
           [this] ()
           {
             m_game->hero_plant_standard ();
           });

        action_connect
          ("hero.levels",
           [this] ()
           {
             auto d = LwDialog::build<HeroLevelsDialog> (this);
             d->setup ();
             d->signal_response ().connect
               ([d] (Gtk::ResponseType)
                {
                  delete d;
                });
           });

        action_connect
          ("hero.search",
           [this] ()
           {
             m_game->search_selected_stack ();
           });

        action_connect
          ("hero.use-item",
           [this] ()
           {
             auto d = LwDialog::build<UseItemDialog> (this);
             d->setup (Playerlist::getActiveplayer ()->getUsableItems ());
             d->signal_selected_item ().connect
               ([this] (Item *item)
                {
                  m_game->use_item (item);
                });
             d->signal_response ().connect
               ([d] (Gtk::ResponseType)
                {
                  delete d;
                });
           });

        action_connect
          ("view.preferences",
           [this] ()
           {
             auto d = LwDialog::build<PreferencesDialog> (this);
             d->setup ();
             d->signal_add_player ().connect
               ([this] (Player *p)
                {
                  m_game->addPlayer (p);
                });
             d->signal_response ().connect
               ([this, d] (Gtk::ResponseType)
                {
                  delete d;
                });
           });

        action_connect
          ("view.diplomacy",
           [this] ()
           {
             auto d = LwDialog::build<DiplomacyDialog> (this);
             d->setup ();
             d->signal_response ().connect
               ([this, d] (Gtk::ResponseType)
                {
                  m_game_button_box->update_diplomacy_button (false);
                  delete d;
                });
           });


        action_connect
          ("view.toggle-grid",
           [this] ()
           {
             m_bigmap->toggle_grid ();
           });

        action_connect
          ("view.army-bonus",
           [this] ()
           {
             auto d = LwDialog::build<ArmyBonusDialog> (this);
             d->setup ();
             d->signal_response ().connect
               ([d] (Gtk::ResponseType)
                {
                  delete d;
                });
           });

        action_connect
          ("view.items",
           [this] ()
           {
             auto d = LwDialog::build<ItemBonusDialog> (this);
             d->setup ();
             d->signal_response ().connect
               ([d] (Gtk::ResponseType)
                {
                  delete d;
                });
           });

        action_connect
          ("view.cities",
           [this] ()
           {
             City *city;
             if (m_status_box->get_currently_selected_stack ())
               {
                 Vector<int> pos =
                   m_status_box->get_currently_selected_stack ()->getPos ();
                 city =
                   Citylist::instance ()->getNearestVisibleFriendlyCity (pos);
               }
             else
               city = Playerlist::getActiveplayer ()->getFirstCity ();

             if (city)
               {
                 auto d = LwDialog::build<CityDialog> (this);
                 d->setup (city, GSO::s_razing_cities == GameParameters::ALWAYS,
                           GSO::s_see_opponents_production);
                 d->signal_response ().connect
                   ([d, this] (Gtk::ResponseType)
                    {
                      m_game->redraw ();
                      delete d;
                    });
               }
           });

        action_connect
          ("view.vectoring",
           [this] ()
           {
             City *city;
             if (m_status_box->get_currently_selected_stack ())
               {
                 Vector<int> pos =
                   m_status_box->get_currently_selected_stack ()->getPos ();
                 city =
                   Citylist::instance ()->getNearestVisibleFriendlyCity (pos);
               }
             else
               city = Playerlist::getActiveplayer ()->getFirstCity ();
             if (city)
               {
                 bool see_all = false;
                 auto d = LwDialog::build <DestinationDialog> (this);
                 d->setup (city, &see_all);
                 d->signal_response ().connect
                   ([d] (Gtk::ResponseType)
                    {
                      delete d;
                    });
               }
           });

        action_connect
          ("view.ruins",
           [this] ()
           {
             Vector<int> pos = Vector<int>(0, 0);
             NamedLocation *bldg = NULL;
             if (m_status_box->get_currently_selected_stack ())
               {

                 //are we on a ruin or temple?
                 pos = m_status_box->get_currently_selected_stack ()->getPos ();
                 bldg = GameMap::instance ()->get_nearest_ruin_or_temple (pos);
               }
             else
               {
                 //no, well maybe get the closest one from where we started
                 auto c = Playerlist::getActiveplayer ()->getFirstCity ();
                 if (c)
                   {
                     pos = c->getPos ();
                     bldg =
                       GameMap::instance ()->get_nearest_ruin_or_temple (pos);
                   }
               }

             //stil nothing? just grab any old ruin or temple to start at
             if (!bldg)
               {
                 if (Templelist::instance ()->empty () == false)
                   bldg = Templelist::instance ()->front ();
                 else if (Ruinlist::instance ()->empty () == false)
                   {
                     for (auto rr : *Ruinlist::instance ())
                       {
                         if (rr->isHidden () &&
                             rr->getOwner () != Playerlist::getActiveplayer ())
                           {
                             bldg = rr;
                             break;
                           }
                       }
                   }
               }

             if (bldg)
               {
                 auto d = LwDialog::build<RuinReportDialog> (this);
                 d->setup (bldg);
                 d->signal_response ().connect
                   ([d] (Gtk::ResponseType)
                    {
                      delete d;
                    });
               }
           });

        action_connect
          ("view.stack",
           [this] ()
           {
             auto p = Playerlist::getActiveplayer ();
             if (p)
               {
                 auto stack = p->getActivestack ();
                 if (stack)
                   {
                     auto d = LwDialog::build<StackInfoDialog> (this);
                     d->setup (stack->getPos ());
                     d->signal_response ().connect
                       ([d] (Gtk::ResponseType)
                        {
                          delete d;
                        });
                   }
               }
           });

        action_connect
          ("history.city",
           [this] ()
           {
             auto d = LwDialog::build<HistoryReportDialog> (this);
             d->setup (HistoryReportDialog::ReportType::CITY);
             d->signal_response ().connect
               ([d] (Gtk::ResponseType)
                {
                  delete d;
                });
           });

        action_connect
          ("history.ruins",
           [this] ()
           {
             auto d = LwDialog::build<HistoryReportDialog> (this);
             d->setup (HistoryReportDialog::ReportType::RUIN);
             d->signal_response ().connect
               ([d] (Gtk::ResponseType)
                {
                  delete d;
                });
           });

        action_connect
          ("history.events",
           [this] ()
           {
             auto d = LwDialog::build<HistoryReportDialog> (this);
             d->setup (HistoryReportDialog::ReportType::EVENTS);
             d->signal_response ().connect
               ([d] (Gtk::ResponseType)
                {
                  delete d;
                });
           });

        action_connect
          ("history.gold",
           [this] ()
           {
             auto d = LwDialog::build<HistoryReportDialog> (this);
             d->setup (HistoryReportDialog::ReportType::GOLD);
             d->signal_response ().connect
               ([d] (Gtk::ResponseType)
                {
                  delete d;
                });
           });

        action_connect
          ("history.winners",
           [this] ()
           {
             auto d = LwDialog::build<HistoryReportDialog> (this);
             d->setup (HistoryReportDialog::ReportType::WINNING);
             d->signal_response ().connect
               ([d] (Gtk::ResponseType)
                {
                  delete d;
                });
           });

        action_connect
          ("history.triumphs",
           [this] ()
           {
             auto d = LwDialog::build<TriumphsDialog> (this);
             d->setup (Playerlist::getActiveplayer ());
             d->signal_response ().connect
               ([d] (Gtk::ResponseType)
                {
                  delete d;
                });
           });

        action_connect
          ("help.tutorial-video",
           [this] ()
           {
             Glib::ustring uri = "https://vimeo.com/409439854";
             auto launcher = Gtk::UriLauncher::create(uri);

             launcher->launch(
                              *this,
                              [launcher](const Glib::RefPtr<Gio::AsyncResult>& result)
                              {
                                try
                                  {
                                    launcher->launch_finish(result);
                                  }
                                catch (const Glib::Error& ex)
                                  {
                                    std::cerr << ex.what() << '\n';
                                  }
                              });
           });

        action_connect
          ("help.online-help",
           [this] ()
           {
             Glib::ustring uri = 
               "http://www.nongnu.org/lordsawar/manual/" PACKAGE_VERSION
               "/lordsawar.html";
             auto launcher = Gtk::UriLauncher::create(uri);

             launcher->launch(
                              *this,
                              [launcher](const Glib::RefPtr<Gio::AsyncResult>& result)
                              {
                                try
                                  {
                                    launcher->launch_finish(result);
                                  }
                                catch (const Glib::Error& ex)
                                  {
                                    std::cerr << ex.what() << '\n';
                                  }
                              });
           });

        action_connect
          ("help.about",
           [this] ()
           {
             auto d = Gtk::make_managed<AboutDialog> (*this);
             d->present ();
             d->signal_response ().connect
               ([d] (Gtk::ResponseType)
                {
                  delete d;
                });
           });

        action_connect
          ("turn.end-turn",
           [this] ()
           {
             m_game->end_turn ();
           });

        action_connect
          ("order.move",
           [this] ()
           {
             m_game->move_selected_stack_along_path ();
           });

        action_connect
          ("view.center",
           [this] ()
           {
             m_game->center_selected_stack ();
           });

        action_connect
          ("order.deselect",
           [this] ()
           {
             m_game->deselect_selected_stack ();
           });

        action_connect
          ("order.defend",
           [this] ()
           {
             m_game->defend_selected_stack ();
           });
      }

    void give_some_cheese ()
      {
        if (m_game_winner != NULL)
          {
            m_game->endOfGameRoaming (m_game_winner);
            Playerlist::getActiveplayer ()->clearFogMap ();
            m_game->redraw();
            //disable some actions

            m_simple_actions["turn.end-turn"]->set_enabled (false);
            m_simple_actions["game.save-as"]->set_enabled (false);
            m_simple_actions["game.save"]->set_enabled (false);
          }
      }

    void stop_game (Glib::ustring action)
      {
        m_stop_action = action;
        Snd::instance ()->disableBackground ();
        if (m_game)
          {
            m_current_save_filename = "";
            if (action == "game-over" &&
                m_game->getScenario ()->getPlayMode() ==
                GameScenario::NETWORKED)
              give_some_cheese ();
            else
              m_game->stopGame ();
          }
      }

};

#endif
