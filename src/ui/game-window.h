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
#include "next-turn-networked.h"
#include "game.h"
#include "game-button-box.h"
#include "small-map.h"
#include "input-events.h"
#include "configuration.h"
#include "status-box.h"
#include "snd.h"
#include "city-info-tip.h"
#include "stack-info-tip.h"
#include "map-info-tip.h"


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

    GameWindow ();
    ~GameWindow ();

    static Gtk::HeaderBar * create_header_bar (TurnIndicator *turn_indicator, Gtk::MenuButton *menu_button);
    void new_game (GameScenario *game_scenario, NextTurn *next_turn);
    void load_game (GameScenario *game_scenario, NextTurn *next_turn);
    void new_network_game (GameScenario *game_scenario, NextTurnNetworked *next_turn);


    bool on_close_request ();

    sigc::signal<void ()> signal_game_ended ();
    sigc::signal<void ()> signal_game_ended_start_new ();
    sigc::signal<void ()> signal_show_lobby ();

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

    void on_show_keyboard_shortcuts ();
    void on_game_action_added (Glib::RefPtr<Gio::SimpleAction> action);
    void setup_accels ();
    void handle_confirmation_result (bool should_quit);
    void application_quit ();
    void action_connect (Glib::ustring name, sigc::slot<void()> slot);
    void setup_header_bar ();
    void setup_smallmap ();
    void add_connection (sigc::connection c);
    void setup_game_object (GameScenario *game_scenario, NextTurn *nextTurn);
    void setup_signals ();
    void setup_network_play_signals ();
    void setup_bigmap ();
    void setup_game_signals ();
    void setup_action_signals ();
    void give_some_cheese ();
    void stop_game (Glib::ustring action);

};

#endif
