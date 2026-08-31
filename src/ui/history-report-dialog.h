//  Copyright (C) 2007, 2008, 2009, 2012, 2014, 2015, 2016, 2017, 2020,
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
#include "lw-dialog-base.h"
#ifndef HISTORY_REPORT_DIALOG_H
#define HISTORY_REPORT_DIALOG_H
#include "line-chart.h"

#include "history-map.h"
#include "box-compose.h"
#include "network-history.h"
#include "report-dialog.h"

class HistoryReportRow: public Glib::Object
{
public:
    Gtk::Box *m_box;

    static Glib::RefPtr<HistoryReportRow> create (Gtk::Box *box)
      {
        return
          Glib::make_refptr_for_instance<HistoryReportRow>
          (new HistoryReportRow (box));
      }

protected:
    HistoryReportRow (Gtk::Box *box)
      : m_box (box)
      {
      }
};

class HistoryReportDialog: public LwDialogBase
{
public:
    static std::string get_resource_name ()
      {
        return "history-report.ui";
      }

    enum ReportType
      {
        CITY,
        RUIN,
        EVENTS,
        GOLD,
        WINNING
      };

    ~HistoryReportDialog ()
      {
        for (auto it = m_past_eventlists.begin ();
             it != m_past_eventlists.end (); ++it)
          {
            std::list<NetworkHistory*> hist = (*it);
            for (auto hit = hist.begin (); hit != hist.end (); ++hit)
              delete (*hit);
          }
        m_switch_page.disconnect ();
        m_historymap_changed.disconnect ();
        delete m_historymap;
      }

    HistoryReportDialog (BaseObjectType* o,
                         const Glib::RefPtr<Gtk::Builder>& xml)
      : LwDialogBase (o, xml)
      {
        m_close_button = load <Gtk::Button> ("close_button");
        m_history_notebook = load <Gtk::Notebook> ("history_notebook");
        m_turn_scale = load <Gtk::Scale> ("turn_scale");
        m_city_label = load<Gtk::Label> ("city_label");
        m_ruin_label = load<Gtk::Label> ("ruin_label");
        m_gold_label = load<Gtk::Label> ("gold_label");
        m_winner_label = load<Gtk::Label> ("winner_label");
        m_city_alignment = load<Gtk::Box> ("city_alignment");
        m_ruin_alignment = load<Gtk::Box> ("ruin_alignment");
        m_gold_alignment = load<Gtk::Box> ("gold_alignment");
        m_winner_alignment = load<Gtk::Box> ("winner_alignment");
        m_map_drawing_area = load<Gtk::DrawingArea> ("map_drawing_area");
        m_treeview = load<Gtk::ColumnView> ("treeview");
      }

    void setup (ReportType report_type)
      {
        set_response (m_close_button, Gtk::ResponseType::ACCEPT);

        m_historymap = new HistoryMap (Citylist::instance (),
                                       Ruinlist::instance ());
        m_historymap_changed = m_historymap->map_changed.connect
          ([this] (Cairo::RefPtr<Cairo::Surface> map)
           {
             cairo_surface_to_drawing_area (map, m_map_drawing_area);
           });

        m_historymap->resize ();
        m_historymap->draw ();

        auto motion = Gtk::EventControllerMotion::create ();
        motion->signal_motion ().connect
          ([this](double x, double y)
           {
             auto c = m_historymap->get_cursor (x, y);
             static ImageCache::CursorType prev_cursor = ImageCache::SHIP;
             if (c != prev_cursor)
               {
                 auto hotspot = ImageCache::get_hotspot (c);
                 auto im = ImageCache::instance ()->getCursorPic (c);
                 auto cursor = Gdk::Cursor::create (im->to_texture (),
                                                    hotspot.x, hotspot.y);
                 m_map_drawing_area->set_cursor (cursor);
               }
             prev_cursor = c;
           });
        m_map_drawing_area->add_controller (motion);

        for (auto p : *Playerlist::instance ())
          {
            if (p == Playerlist::getNeutral ())
              continue;
            Gdk::RGBA color =
              GameMap::getShieldset ()->getColor (p->get_shield ());
            if (p == Playerlist::getActiveplayer ())
              m_colors.push_front (color);
            else
              m_colors.push_back (color);
          }

        generate_past_city_lists ();
        fill_city_tab ();
        generate_past_ruin_lists ();
        fill_ruin_tab ();
        fill_events_tab ();
        fill_gold_tab ();
        fill_winning_tab ();

        m_turn_scale->set_range (1, m_past_citylists.size () + 1);
        m_turn_scale->signal_value_changed ().connect
          (sigc::mem_fun (*this, &HistoryReportDialog::on_turn_changed));
        m_turn_scale->set_value (m_past_citylists.size () + 1);
        on_turn_changed ();

        m_switch_page = m_history_notebook->signal_switch_page ().connect
          ([this] (Gtk::Widget *, guint number)
           {
             switch (number)
               {
               case CITY:
                 set_title (_("City Report"));
                 break;

               case RUIN:
                 set_title (_("Ruin Report"));
                 break;

               case EVENTS:
                 set_title (_("Events Report"));
                 break;

               case GOLD:
                 set_title (_("Gold Report"));
                 break;

               case WINNING:
                 set_title (_("Winning Report"));
               }
           });
        //this is how we ensure the changed signal is fired
        m_history_notebook->set_current_page (!report_type);
        m_history_notebook->set_current_page (report_type);
      }

private:
    Gtk::Button *m_close_button;
    Gtk::Notebook *m_history_notebook;
    Gtk::Scale *m_turn_scale;
    Gtk::Label *m_city_label;
    Gtk::Label *m_ruin_label;
    Gtk::Label *m_gold_label;
    Gtk::Label *m_winner_label;
    Gtk::Box *m_city_alignment;
    Gtk::Box *m_ruin_alignment;
    Gtk::Box *m_gold_alignment;
    Gtk::Box *m_winner_alignment;
    Gtk::DrawingArea *m_map_drawing_area;
    Gtk::ColumnView *m_treeview;

    HistoryMap* m_historymap;

    LineChart *m_city_chart;
    LineChart *m_ruin_chart;
    LineChart *m_gold_chart;
    LineChart *m_rank_chart;

    std::vector<LocationList<City*>*> m_past_citylists;
    std::vector<std::list<NetworkHistory *>> m_past_eventlists;
    std::list<std::list<guint32>> m_past_citycounts;
    std::vector<LocationList<Ruin*>*> m_past_ruinlists;
    std::list<std::list<guint32>> m_past_ruincounts;
    std::list<std::list<guint32>> m_past_goldcounts;
    std::list<std::list<guint32>> m_past_rankcounts;
    std::list<Gdk::RGBA> m_colors;

    sigc::connection m_switch_page;
    sigc::connection m_historymap_changed;

    Glib::RefPtr<Gtk::SingleSelection> m_selection_model;
    Glib::RefPtr<Gio::ListStore<HistoryReportRow>> m_store;

    void fill_city_tab ()
      {
        generate_past_city_counts ();
        m_city_chart =
          Gtk::make_managed<LineChart> (m_past_citycounts, m_colors, 
                                        Citylist::instance ()->size (),
                                        _("Cities"), _("Turns"));
        m_city_chart->set_vexpand (true);
        m_city_alignment->append (*m_city_chart);
      }

    void fill_ruin_tab ()
      {
        generate_past_ruin_counts ();
        m_ruin_chart =
          Gtk::make_managed<LineChart> (m_past_ruincounts, m_colors, 
                                        Ruinlist::instance ()->size (),
                                        _("Explored Ruins"), _("Turns"));
        m_ruin_chart->set_vexpand (true);
        m_ruin_alignment->append (*m_ruin_chart);
      }

    void fill_events_tab ()
      {
        generate_past_event_lists ();
        m_store = Gio::ListStore<HistoryReportRow>::create ();
        m_selection_model = Gtk::SingleSelection::create (m_store);
        m_treeview->set_model (m_selection_model);
        setup_column ();
        fill_events (1);
      }

    void fill_events (guint32 turn)
      {
        m_store->remove_all ();
        if (turn < m_past_eventlists.size ())
          for (auto h : m_past_eventlists[turn])
            add_history_event (h);
      }


    void fill_in_gold_turn_info (guint32 turn)
      {
        //update the gold chart
        //on turn # you had # gold pieces
        guint32 count = 1;
        auto goldlist = *m_past_goldcounts.begin ();
        for (auto it = goldlist.begin (); it != goldlist.end (); ++it, count++)
          {
            if (count == turn)
              {
                count = *it;
                break;
              }
          }
        Glib::ustring s;
        turn == m_past_citylists.size () ?
          s = String::ucompose (ngettext ("On turn %1 you have %2 gold piece!",
                                          "On turn %1 you have %2 gold pieces!",
                                          count), turn + 1, count) :
          s = String::ucompose (ngettext ("On turn %1 you had %2 gold piece!",
                                          "On turn %1 you had %2 gold pieces!",
                                          count), turn + 1, count);
        m_gold_label->set_text (s);

      }

    void fill_in_city_turn_info (guint32 turn)
      {
        //update the city chart
        guint32 count = 0;
        auto citylist = *m_past_citycounts.begin ();
        for (auto it = citylist.begin (); it != citylist.end (); ++it, count++)
          {
            if (count == turn)
              {
                count = *it;
                break;
              }
          }
        Glib::ustring s;
        turn == m_past_citylists.size () ?
          s = String::ucompose (ngettext ("On turn %1 you have %2 city!",
                                          "On turn %1 you have %2 cities!",
                                          count), turn + 1, count) :
          s = String::ucompose (ngettext ("On turn %1 you had %2 city!",
                                          "On turn %1 you had %2 cities!",
                                          count), turn + 1, count);
        m_city_label->set_text (s);
      }

    void fill_in_ruin_turn_info (guint32 turn)
      {
        //update the ruin chart
        guint32 count = 0;
        auto ruinlist = *m_past_ruincounts.begin ();
        for (auto it = ruinlist.begin (); it != ruinlist.end (); ++it, count++)
          {
            if (count == turn) 
              {
                count = *it;
                break;
              }
          }
        Glib::ustring s;
        turn == m_past_ruinlists.size () ?
          s = String::ucompose (ngettext ("By turn %1 you explored %2 ruin!",
                                        "By turn %1 you explored %2 ruins!",
                                        count), turn + 1, count) :
          s = String::ucompose (ngettext ("By turn %1 you explored %2 ruin!",
                                        "By turn %1 you explored %2 ruins!",
                                        count), turn + 1, count);
        m_ruin_label->set_text (s);
      }

    void fill_in_rank_turn_info (guint32 turn)
      {
        //on turn # you were coming #
        std::list<guint32> scores;
        for (auto s : m_past_rankcounts)
          {
            guint32 count = 1;
            for (auto it = s.begin (); it != s.end (); ++it, count++)
              {
                if (count == turn)
                  {
                    count = *it;
                    scores.push_back (*it);
                    break;
                  }
              }
          }
        Glib::ustring s;
        turn == m_past_citylists.size () ?
          s =
          String::ucompose (_("On turn %1 you are coming %2!"), turn + 1,
                            ReportDialog::calculate_rank (scores,
                                                          *scores.begin ())) :
          s =
          String::ucompose (_("On turn %1 you were coming %2!"), turn + 1,
                            ReportDialog::calculate_rank (scores,
                                                          *scores.begin ()));
        m_winner_label->set_text (s);
      }

    void fill_in_turn_info (guint32 turn)
      {
        fill_events (turn);
        fill_in_gold_turn_info (turn);
        fill_in_city_turn_info (turn);
        fill_in_ruin_turn_info (turn);
        fill_in_rank_turn_info (turn);
      }

    void add_history_event (NetworkHistory *event)
      {
        Player *p = event->getOwner ();

        History *history = event->getHistory ();

        Gtk::Box *box = NULL;

        Glib::RefPtr<Gdk::Pixbuf> shield =
          ImageCache::instance ()->getShieldPic (1, p, false)->to_pixbuf ();

        switch (history->getType ())
          {
          case History::FOUND_SAGE:
              {
                History_FoundSage *ev =
                  static_cast<History_FoundSage *>(history);
                box = Box::ucompose (_("%1 %2 finds a sage!"), shield,
                                     ev->getHeroName ());
                break;
              }

          case History::HERO_EMERGES:
              {
                History_HeroEmerges *ev =
                  static_cast<History_HeroEmerges *>(history);
                box = Box::ucompose (_("%1 %2 emerges in %3"), shield,
                                     ev->getHeroName (), ev->getCityName ());
                break;
              }

          case History::HERO_QUEST_STARTED:
              {
                History_HeroQuestStarted *ev =
                  static_cast<History_HeroQuestStarted*>(history);
                box = Box::ucompose (_("%1 %2 begins a quest!"), shield,
                                     ev->getHeroName ());
                break;
              }

          case History::HERO_QUEST_COMPLETED:
              {
                History_HeroQuestCompleted *ev
                  = static_cast<History_HeroQuestCompleted *>(history);
                box = Box::ucompose (_("%1 %2 finishes a quest!"), shield,
                                     ev->getHeroName ());
                break;
              }

          case History::HERO_QUEST_EXPIRED:
              {
                History_HeroQuestExpired *ev
                  = static_cast<History_HeroQuestExpired *>(history);
                box = Box::ucompose (_("%1 %2 could not complete a quest!"), shield,
                                     ev->getHeroName ());
                break;
              }

          case History::HERO_KILLED_IN_CITY:
              {
                History_HeroKilledInCity *ev =
                  static_cast<History_HeroKilledInCity *>(history);
                box = Box::ucompose (_("%1 %2 is killed in %3!"), shield,
                                     ev->getHeroName (), ev->getCityName ());
                break;
              }

          case History::HERO_KILLED_IN_BATTLE:
              {
                History_HeroKilledInBattle *ev =
                  static_cast<History_HeroKilledInBattle *>(history);
                box = Box::ucompose (_("%1 %2 is killed in battle!"), shield,
                                     ev->getHeroName ());
                break;
              }

          case History::HERO_KILLED_SEARCHING:
              {
                History_HeroKilledSearching *ev =
                  static_cast<History_HeroKilledSearching *>(history);
                box = Box::ucompose (_("%1 %2 is killed while searching!"), shield,
                                     ev->getHeroName ());
                break;
              }

          case History::HERO_CITY_WON:
              {
                History_HeroCityWon *ev =
                  static_cast<History_HeroCityWon *>(history);
                box = Box::ucompose (_("%1 %2 conquers %3!"), shield,
                                     ev->getHeroName (), ev->getCityName ());
                break;
              }

          case History::PLAYER_VANQUISHED:
              {
                box = Box::ucompose (_("%1 %2 utterly vanquished!"), shield,
                                     p->getName ());
                break;
              }

          case History::DIPLOMATIC_PEACE:
              {
                History_DiplomacyPeace *ev =
                  static_cast<History_DiplomacyPeace*>(history);
                Player *opponent =
                  Playerlist::instance ()->get (ev->getOpponentId ());
                box =
                  Box::ucompose (_("%1 %2 at peace with %3 %4!"), shield,
                                 p->getName (),
                                 ImageCache::instance ()->getShieldPic
                                 (1, opponent, false)->to_pixbuf (),
                                 opponent->getName ());
                break;
              }

          case History::DIPLOMATIC_WAR:
              {
                History_DiplomacyWar *ev =
                  static_cast<History_DiplomacyWar*>(history);
                Player *opponent =
                  Playerlist::instance ()->get (ev->getOpponentId ());
                box =
                  Box::ucompose (_("%1 %2 at war with %3 %4!"), shield,
                                 p->getName (),
                                 ImageCache::instance ()->getShieldPic
                                 (1, opponent, false)->to_pixbuf (),
                                 opponent->getName ());
                break;
              }

          case History::DIPLOMATIC_TREACHERY:
              {
                History_DiplomacyTreachery *ev =
                  static_cast<History_DiplomacyTreachery*>(history);
                Player *opponent =
                  Playerlist::instance ()->get (ev->getOpponentId ());
                box =
                  Box::ucompose (_("%1 Treachery on %2 %3!"), shield,
                                 ImageCache::instance ()->getShieldPic
                                 (1, opponent, false)->to_pixbuf (),
                                 opponent->getName ());
                break;
              }

          case History::HERO_FINDS_ALLIES:
              {
                History_HeroFindsAllies *ev =
                  static_cast<History_HeroFindsAllies*>(history);
                box = Box::ucompose (_("%1 %2 finds allies!"), shield,
                                     ev->getHeroName ());
                break;
              }

          case History::HERO_RUIN_EXPLORED:
              {
                History_HeroRuinExplored *ev =
                  static_cast<History_HeroRuinExplored *>(history);
                box = Box::ucompose (_("%1 %2 explores %3!"), shield,
                                     ev->getHeroName (),
                                     Ruinlist::instance ()->getById
                                     (ev->getRuinId ())->getName ());
                break;
              }

          case History::USE_ITEM:
              {
                History_HeroUseItem *ev =
                  static_cast<History_HeroUseItem*>(history);
                Player *opponent =
                  Playerlist::instance ()->get (ev->getOpponentId ());
                if (ev->getItemBonus () & ItemProto::USABLE)
                  box =
                    Box::ucompose (_("%1 %2 uses the %3 against %4 %5!"),
                                   shield, ev->getHeroName (),
                                   ev->getItemName (),
                                   ImageCache::instance ()->getShieldPic
                                   (1, opponent, false)->to_pixbuf (),
                                   opponent->getName ());
                else
                  box = Box::ucompose (_("%1 %2 uses the %3!"), shield,
                                       ev->getHeroName (), ev->getItemName ());
                break;
              }

          default:
            box = NULL;
            break;
          }

        if (box)
          m_store->append (HistoryReportRow::create (box));
      }

    void setup_column ()
      {
        auto factory = Gtk::SignalListItemFactory::create ();

        factory->signal_setup ().connect
          ([] (const Glib::RefPtr<Gtk::ListItem>& item)
           {
             auto box = Gtk::make_managed<Gtk::Box> ();
             item->set_child (*box);
             item->set_data ("reportbox", box);
           });

        factory->signal_bind ().connect
          ([](const Glib::RefPtr<Gtk::ListItem>& item)
           {
             auto row =
               std::dynamic_pointer_cast<HistoryReportRow>(item->get_item ());
             auto box = static_cast<Gtk::Box*>(item->get_data ("reportbox"));

             if (row && box)
               box->append (*row->m_box);
           });

        auto column = Gtk::ColumnViewColumn::create ("", factory);
        m_treeview->append_column (column);
      }

    void fill_gold_tab ()
      {
        generate_past_gold_counts ();
        unsigned int max_gold = 0;
        for (auto ll : m_past_goldcounts)
          {
            for (auto l : ll)
              {
                if (l > max_gold)
                  max_gold = l;
              }
          }
        m_gold_chart =
          Gtk::make_managed<LineChart> (m_past_goldcounts, m_colors, max_gold, 
                                        _("Gold Pieces"), _("Turns"));
        m_gold_chart->set_vexpand (true);
        m_gold_alignment->append (*m_gold_chart);
      }

    void fill_winning_tab ()
      {
        generate_past_rank_lists ();
        unsigned int max_score = 0;
        for (auto ll : m_past_rankcounts)
          {
            for (auto l : ll)
              {
                if (l > max_score)
                  max_score = l;
              }
          }
        m_rank_chart =
          Gtk::make_managed<LineChart> (m_past_rankcounts, m_colors, max_score, 
                                        _("Score"), _("Turns"));
        m_rank_chart->set_vexpand (true);
        m_winner_alignment->append (*m_rank_chart);
      }

    void generate_past_event_lists ()
      {
        bool last_turn = false;
        std::list<NetworkHistory*> *elist = new std::list<NetworkHistory*>();

        //keep a set of pointers to remember how far we are into each player's history
        std::list<History*> *hist[MAX_PLAYERS];
        for (auto p : *Playerlist::instance ())
          {
            if (p == Playerlist::getNeutral ())
              continue;
            hist[p->getId ()] = p->getHistorylist ();
          }

        std::list<History*>::iterator hit[MAX_PLAYERS];
        for (auto p : *Playerlist::instance ())
          {
            if (p == Playerlist::getNeutral ())
              continue;
            hit[p->getId ()] = hist[p->getId ()]->begin ();
          }

        unsigned int count = 0;
        while (1)
          {
            //now we see what cities we took this turn
            for (auto p : *Playerlist::instance ())
              {
                if (p == Playerlist::getNeutral ())
                  continue;
                //dump everything up to the next turn
                guint32 id = p->getId ();
                if (hit[id] == hist[id]->end ())
                  continue;
                for (; hit[id] != hist[id]->end (); ++hit[id])
                  {
                    if ((*(hit[id]))->getType () == History::START_TURN)
                      {
                        hit[id]++;
                        break;
                      }
                    switch ((*(hit[id]))->getType ())
                      {
                      case History::FOUND_SAGE:
                      case History::HERO_EMERGES:
                      case History::HERO_QUEST_STARTED:
                      case History::HERO_QUEST_COMPLETED:
                      case History::HERO_QUEST_EXPIRED:
                      case History::HERO_KILLED_IN_CITY:
                      case History::HERO_KILLED_IN_BATTLE:
                      case History::HERO_KILLED_SEARCHING:
                      case History::HERO_CITY_WON:
                      case History::HERO_FINDS_ALLIES:
                      case History::PLAYER_VANQUISHED:
                      case History::DIPLOMATIC_TREACHERY:
                      case History::DIPLOMATIC_WAR:
                      case History::DIPLOMATIC_PEACE:
                      case History::HERO_RUIN_EXPLORED:
                      case History::USE_ITEM:
                        elist->push_back
                          (new NetworkHistory (*(hit[id]), p->getId ()));
                        break;
                      case History::START_TURN:
                      case History::GOLD_TOTAL:
                      case History::CITY_WON:
                      case History::CITY_RAZED:
                      case History::SCORE:
                      case History::HERO_REWARD_RUIN:
                      case History::END_TURN:
                        break;
                      }
                  }
                if (hit[id] == hist[id]->end ())
                  {
                    count++;
                    if (count == Playerlist::instance ()->size () - 2)
                      last_turn = true;
                  }
              }
            //and add it to the list
            m_past_eventlists.push_back (*elist);
            std::list<NetworkHistory*> *new_elist =
              new std::list<NetworkHistory*>();
            elist = new_elist;
            if (last_turn == true)
              break;
          }
      }

    void generate_past_city_lists ()
      {
        bool last_turn = false;

        //keep a set of pointers to remember how far we are into each player's history
        std::list<History*> *hist[MAX_PLAYERS];
        for (auto p : *Playerlist::instance ())
          {
            if (p == Playerlist::getNeutral ())
              continue;
            hist[p->getId ()] = p->getHistorylist ();
          }
        std::list<History*>::iterator hit[MAX_PLAYERS];
        for (auto p : *Playerlist::instance ())
          {
            if (p == Playerlist::getNeutral ())
              continue;
            hit[p->getId ()] = hist[p->getId ()]->begin ();
          }

        //start off with an initial city list where all cities are neutral owned
        LocationList<City*> *clist = new LocationList<City*>();
        for (auto c : *Citylist::instance ())
          clist->push_back (new City (*c, true));
        for (auto c : *clist)
          {
            c->setOwner (Playerlist::getNeutral ());
            //is the city burned to begin with?
            bool no_city_history = true;
            guint32 age;
            for (auto p : *Playerlist::instance ())
              if (p->conqueredCity (c, age) == true)
                no_city_history = false;
            if (c->isBurnt () == true && no_city_history)
              c->setBurnt (true);
            else
              c->setBurnt (false);
          }

        unsigned int count = 0;
        while (1)
          {
            //now we see what cities we took this turn
            for (auto p : *Playerlist::instance ())
              {
                if (p == Playerlist::getNeutral ())
                  continue;
                //dump everything up to the next turn
                guint32 id = p->getId ();
                if (hit[id] == hist[id]->end ())
                  continue;
                for (; hit[id] != hist[id]->end (); hit[id]++)
                  {
                    if ((*hit[id])->getType () == History::START_TURN)
                      {
                        hit[id]++;
                        break;
                      }
                    else if ((*hit[id])->getType () == History::CITY_WON)
                      {
                        guint32 city_id;
                        city_id =
                          dynamic_cast<History_CityWon*>(*hit[id])->getCityId ();
                        //find city with this city id in clist
                        for (auto c : *clist)
                          if (c->getId () == city_id)
                            {
                              c->setOwner (p);
                              break;
                            }
                      }
                    else if ((*hit[id])->getType () == History::CITY_RAZED)
                      {
                        guint32 city_id;
                        city_id =
                          dynamic_cast<History_CityRazed*>(*hit[id])->getCityId ();
                        //find city with this city id in clist
                        for (auto c : *clist)
                          if (c->getId () == city_id)
                            {
                              //change the owner to neutral
                              c->setOwner (Playerlist::getNeutral ());
                              c->setBurnt (true);
                              break;
                            }
                      }
                  }
                if (hit[id] == hist[id]->end ())
                  {
                    count++;
                    if (count == Playerlist::instance ()->size () - 2)
                      last_turn = true;
                  }
              }
            //and add it to the list
            m_past_citylists.push_back (clist);
            LocationList<City*> *new_clist = new LocationList<City*>();
            for (auto c : *clist)
              new_clist->push_back (new City (*c));
            clist = new_clist;
            if (last_turn == true)
              break;

          }
        m_past_citylists.erase (--m_past_citylists.end ());
      }

    void generate_past_ruin_lists ()
      {
        //we don't do this per player
        //we just count how many ruins are unexplored at every turn.
        //how do we deal with hidden ruins?
        //they should pop up when found.
        bool last_turn = false;

        //keep a set of pointers to remember how far we are into each player's history
        std::list<History*> *hist[MAX_PLAYERS];
        for (auto p : *Playerlist::instance ())
          {
            if (p == Playerlist::getNeutral ())
              continue;
            hist[p->getId ()] = p->getHistorylist ();
          }
        std::list<History*>::iterator hit[MAX_PLAYERS];
        for (auto p : *Playerlist::instance ())
          {
            if (p == Playerlist::getNeutral ())
              continue;
            hit[p->getId ()] = hist[p->getId ()]->begin ();
          }

        //start off with an initial ruin list where all ruins are unexplored and hidden.
        //all hidden ruins haven't been found yet, unless they started off that way.
        LocationList<Ruin*> *rlist = new LocationList<Ruin*>();
        for (auto r : *Ruinlist::instance ())
          rlist->push_back (new Ruin (*r));
        for (auto r : *rlist)
          {
            //is the ruin searched to begin with?
            bool no_ruin_history = true;
            for (auto p : *Playerlist::instance ())
              if (p->searchedRuin (r) == true)
                no_ruin_history = false;
            if (r->isSearched () == true && no_ruin_history)
              r->setSearched (true);
            else
              {
                r->setSearched (false);
                if (r->isHidden ())
                  r->setOwner (NULL);
              }
          }

        unsigned int count = 0;
        while (1)
          {
            //now we see what ruins we took this turn
            for (auto p : *Playerlist::instance ())
              {
                if (p == Playerlist::getNeutral ())
                  continue;
                //dump everything up to the next turn
                guint32 id = p->getId ();
                if (hit[id] == hist[id]->end ())
                  continue;
                for (; hit[id] != hist[id]->end (); ++(hit[id]))
                  {
                    if ((*hit[id])->getType () == History::START_TURN)
                      {
                        ++(hit[id]);
                        break;
                      }
                    //when a ruin becomes visible all of a sudden, we mark it as visible
                    else if ((*hit[id])->getType () == History::HERO_REWARD_RUIN)
                      {
                        guint32 ruin_id;
                        ruin_id = 
                          dynamic_cast<History_HeroRewardRuin*>(*hit[id])->getRuinId ();
                        //find ruin with this ruin id in rlist
                        for (auto r : *rlist)
                          if (r->getId () == ruin_id)
                            {
                              r->setOwner (p);
                              break;
                            }
                      }
                    else if ((*hit[id])->getType () == History::HERO_RUIN_EXPLORED)
                      {
                        guint32 ruin_id;
                        ruin_id = 
                          dynamic_cast<History_HeroRuinExplored*>(*hit[id])->getRuinId ();
                        //find ruin with this ruin id in rlist
                        for (auto r : *rlist)
                          if (r->getId () == ruin_id)
                            {
                              r->setSearched (true);
                              break;
                            }
                      }
                  }
                if (hit[id] == hist[id]->end ())
                  {
                    count++;
                    if (count == Playerlist::instance ()->size () - 2)
                      last_turn = true;
                  }
              }
            //and add it to the list
            m_past_ruinlists.push_back (rlist);
            LocationList<Ruin*> *new_rlist = new LocationList<Ruin*>();
            for (auto r : *rlist)
              new_rlist->push_back (new Ruin (*r));
            rlist = new_rlist;
            if (last_turn == true)
              break;

          }
        m_past_ruinlists.erase (--m_past_ruinlists.end ());
      }

    void generate_past_gold_counts ()
      {
        //go through the history list looking for gold events, per player
        for (auto p : *Playerlist::instance ())
          {
            if (p == Playerlist::getNeutral ())
              continue;
            std::list<guint32> line;
            std::list<History*> *hist = p->getHistorylist ();
            for (auto h : *hist)
              {
                if (h->getType () == History::GOLD_TOTAL)
                  {
                    History_GoldTotal *event =
                      static_cast<History_GoldTotal*>(h);
                    line.push_back (event->getGold ());
                  }
              }
            line.push_back ((guint32)(p->getGold ()));
            if (p == Playerlist::getActiveplayer ())
              m_past_goldcounts.push_front (line);
            else
              m_past_goldcounts.push_back (line);
          }
      }

   void generate_past_rank_lists ()
     {
       //go through the history list looking for score events, per player
       for (auto p : *Playerlist::instance ())
         {
           if (p == Playerlist::getNeutral ())
             continue;
           std::list<History*> *hist = p->getHistorylist ();
           std::list<guint32> line;
           for (auto h : *hist)
             {
               if (h->getType () == History::SCORE)
                 {
                   History_Score *event = static_cast<History_Score*>(h);
                   line.push_back (event->getScore ());
                 }
             }
           line.push_back ((guint32)p->getScore ());
           if (p == Playerlist::getActiveplayer ())
             m_past_rankcounts.push_front (line);
           else
             m_past_rankcounts.push_back (line);
         }
     }
        
   void on_turn_changed ()
     {
       //tell the historymap to show another set of cities
       guint32 turn = (guint32)m_turn_scale->get_value () - 1;
       if (turn > m_past_citylists.size () - 1)
         m_historymap->updateCities
           (Citylist::instance (), Ruinlist::instance ());
       else
         m_historymap->updateCities (m_past_citylists[turn],
                                     m_past_ruinlists[turn]);
       m_city_chart->set_x_indicator (turn);
       m_ruin_chart->set_x_indicator (turn);
       m_gold_chart->set_x_indicator (turn);
       m_rank_chart->set_x_indicator (turn);
       fill_in_turn_info (turn);
     }

   void generate_past_city_counts ()
     {
       // go through the past city list
       for (auto p : *Playerlist::instance ())
         {
           if (p == Playerlist::getNeutral ())
             continue;
           //go through the past city lists, searching for cities owned by this
           //player

           std::list<guint32> line;
           for (unsigned int i = 0; i < m_past_citylists.size (); i++)
             {
               guint32 total_cities = 0;
               for (auto c : *m_past_citylists[i])
                 {
                   if (c->getOwner () == p)
                     total_cities++;
                 }
               line.push_back (total_cities);
             }

           line.push_back(Citylist::instance ()->countCities (p));
           if (p == Playerlist::getActiveplayer ())
             m_past_citycounts.push_front (line);
           else
             m_past_citycounts.push_back (line);

         }
     }

   void generate_past_ruin_counts ()
     {
       //how many ruins did the players search at each turn?
       for (auto p : *Playerlist::instance ())
         {
           if (p == Playerlist::getNeutral ())
             continue;
           std::list<guint32> line;
           for (unsigned int i = 0; i < m_past_citylists.size(); i++)
             {
               guint32 total_ruins = 0;
               for (auto r : *m_past_ruinlists[i])
                 {
                   if (r->isHidden () == true && r->getOwner () != p)
                     continue;
                   if (r->isSearched () == true && 
                       p->searchedRuin (r) == true)
                     {
                       r->setOwner (p);
                       total_ruins++;
                     }
                 }
               line.push_back (total_ruins);
             }
           line.push_back (Ruinlist::instance ()->countExploredRuins (p));
           if (p == Playerlist::getActiveplayer ())
             m_past_ruincounts.push_front (line);
           else
             m_past_ruincounts.push_back (line);
         }
     }
};
#endif
