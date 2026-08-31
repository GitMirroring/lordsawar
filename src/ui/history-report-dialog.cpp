//  Copyright (C) 2007, 2008, 2009, 2012, 2014, 2015, 2016, 2017,
//  2020 Ben Asselstine
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
//  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 
//  02110-1301, USA.

#include <config.h>

#include <gtkmm.h>
#include <sigc++/functors/mem_fun.h>

#include "history-report-dialog.h"

#include "line-chart.h"
#include "report-dialog.h"
#include "ucompose.hpp"
#include "defs.h"
#include "File.h"
#include "citylist.h"
#include "ruinlist.h"
#include "city.h"
#include "ruin.h"
#include "playerlist.h"
#include "history.h"
#include "network-history.h"
#include "ImageCache.h"
#include "boxcompose.h"
#include "ItemProto.h"
#include "font-size.h"

#define method(x) sigc::mem_fun(*this, &HistoryReportDialog::x)

HistoryReportDialog::HistoryReportDialog(Gtk::Window &parent, Player *p, HistoryReportType type)
 : LwDialog(parent, "history-report-dialog.ui")
{
  d_player = p;
  generatePastCitylists();
  generatePastRuinlists();
  generatePastEventlists();
  xml->get_widget("map_image", map_image);
  historymap = new HistoryMap(Citylist::getInstance(), Ruinlist::getInstance());
  historymap->map_changed.connect (method(on_map_changed));

  xml->get_widget("turn_scale", turn_scale);
  dialog->set_title(_("History"));
  turn_scale->set_range(1, past_citylists.size());
  turn_scale->set_value(past_citylists.size());

  turn_scale->signal_value_changed().connect (method(on_turn_changed));

  xml->get_widget("history_notebook", history_notebook);
  history_notebook->set_current_page(type);
  history_notebook->signal_switch_page().connect
    (sigc::hide(sigc::hide(method(on_switch_page))));

  xml->get_widget("city_label", city_label);
  xml->get_widget("ruin_label", ruin_label);
  xml->get_widget("gold_label", gold_label);
  xml->get_widget("winner_label", winner_label);

  xml->get_widget("events_list_box", events_list_box);

  xml->get_widget("city_alignment", city_alignment);
  xml->get_widget("ruin_alignment", ruin_alignment);
  xml->get_widget("gold_alignment", gold_alignment);
  xml->get_widget("winner_alignment", winner_alignment);

  Gdk::RGBA color;
  for (Playerlist::iterator pit = Playerlist::getInstance()->begin();
       pit != Playerlist::getInstance()->end(); ++pit)
    {
      if (*pit == Playerlist::getInstance()->getNeutral())
	continue;
      color = (*pit)->getColor();
      if (*pit == d_player)
	d_colors.push_front(color);
      else
	d_colors.push_back(color);
    }

  generatePastCityCounts();
  city_chart = new LineChart(past_citycounts, d_colors, 
                             Citylist::getInstance()->size(),
			     _("Cities"), _("Turns"));
  city_chart->set_vexpand (true);
  city_alignment->add(*manage(city_chart));

  generatePastRuinCounts();
  ruin_chart = new LineChart(past_ruincounts, d_colors, 
			     Ruinlist::getInstance()->size(),
			     _("Explored Ruins"), _("Turns"));
  ruin_chart->set_vexpand (true);
  ruin_alignment->add(*manage(ruin_chart));

  generatePastGoldCounts();
  gold_chart = new LineChart(past_goldcounts, d_colors, 0, 
			     _("Gold Pieces"), _("Turns"));
  gold_chart->set_vexpand (true);
  gold_alignment->add(*manage(gold_chart));

  generatePastWinningCounts();
  rank_chart = new LineChart(past_rankcounts, d_colors, 100,
			     _("Score"), _("Turns"));
  rank_chart->set_vexpand (true);
  winner_alignment->add(*manage(rank_chart));

  fill_in_turn_info((guint32)turn_scale->get_value());
}

HistoryReportDialog::~HistoryReportDialog()
{
  for (std::vector<std::list<NetworkHistory *> >::iterator it = past_eventlists.begin();
       it != past_eventlists.end(); ++it)
    {
      std::list<NetworkHistory*> hist = (*it);
      for (std::list<NetworkHistory*>::iterator hit = hist.begin();
           hit != hist.end(); ++hit)
	delete (*hit);
    }
  history_notebook->property_show_tabs () = false;
  delete historymap;
}

void HistoryReportDialog::generatePastEventlists()
{
  bool last_turn = false;
  std::list<NetworkHistory*> *elist = new std::list<NetworkHistory*>();

  //keep a set of pointers to remember how far we are into each player's history
  std::list<History*> *hist[MAX_PLAYERS];
  for (Playerlist::iterator pit = Playerlist::getInstance()->begin();
       pit != Playerlist::getInstance()->end(); ++pit)
    {
      if (*pit == Playerlist::getInstance()->getNeutral())
	continue;
      hist[(*pit)->getId()] = (*pit)->getHistorylist();
    }
  std::list<History*>::iterator hit[MAX_PLAYERS];
  for (Playerlist::iterator pit = Playerlist::getInstance()->begin();
       pit != Playerlist::getInstance()->end(); ++pit)
    {
      if (*pit == Playerlist::getInstance()->getNeutral())
	continue;
      hit[(*pit)->getId()] = hist[(*pit)->getId()]->begin();
    }

  unsigned int count = 0;
  while (1)
    {
      //now we see what cities we took this turn
      for (Playerlist::iterator pit = Playerlist::getInstance()->begin();
           pit != Playerlist::getInstance()->end(); ++pit)
	{
	  if (*pit == Playerlist::getInstance()->getNeutral())
	    continue;
	  //dump everything up to the next turn
	  guint32 id = (*pit)->getId();
	  if (hit[id] == hist[id]->end())
	    continue;
	  for (; hit[id] != hist[id]->end(); ++hit[id])
	    {
	      if ((*(hit[id]))->getType() == History::START_TURN)
		{
		  hit[id]++;
		  break;
		}
	      switch ((*(hit[id]))->getType())
		{
		case History::FOUND_SAGE: 
		case History::HERO_EMERGES:
		case History::HERO_QUEST_STARTED:
		case History::HERO_QUEST_COMPLETED:
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
		  elist->push_back(new NetworkHistory(*(hit[id]), (*pit)->getId()));
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
	  if (hit[id] == hist[id]->end())
	    {
	      count++;
	      if (count == Playerlist::getInstance()->size() - 2)
		last_turn = true;
	    }
	}
      //and add it to the list
      past_eventlists.push_back(*elist);
      std::list<NetworkHistory*> *new_elist = new std::list<NetworkHistory*>();
      elist = new_elist;
      if (last_turn == true)
	break;

    }
}

void HistoryReportDialog::generatePastCitylists()
{
  bool last_turn = false;

  //keep a set of pointers to remember how far we are into each player's history
  std::list<History*> *hist[MAX_PLAYERS];
  for (Playerlist::iterator pit = Playerlist::getInstance()->begin();
       pit != Playerlist::getInstance()->end(); ++pit)
    {
      if (*pit == Playerlist::getInstance()->getNeutral())
	continue;
      hist[(*pit)->getId()] = (*pit)->getHistorylist();
    }
  std::list<History*>::iterator hit[MAX_PLAYERS];
  for (Playerlist::iterator pit = Playerlist::getInstance()->begin();
       pit != Playerlist::getInstance()->end(); ++pit)
    {
      if (*pit == Playerlist::getInstance()->getNeutral())
	continue;
      hit[(*pit)->getId()] = hist[(*pit)->getId()]->begin();
    }

  //start off with an initial city list where all cities are neutral owned
  LocationList<City*> *clist = new LocationList<City*>();
  Citylist *cl = Citylist::getInstance();
  for (Citylist::iterator it = cl->begin(); it != cl->end(); ++it)
    clist->push_back(new City(**it));
  for (LocationList<City*>::iterator it = clist->begin(); it != clist->end(); ++it)
    {
      (*it)->setOwner(Playerlist::getInstance()->getNeutral());
      //is the city burned to begin with?
      bool no_city_history = true;
      guint32 age;
      for (Playerlist::iterator pit = Playerlist::getInstance()->begin();
           pit != Playerlist::getInstance()->end(); ++pit)
	if ((*pit)->conqueredCity(*it, age) == true)
	  no_city_history = false;
      if ((*it)->isBurnt() == true && no_city_history)
	(*it)->setBurnt(true);
      else
	(*it)->setBurnt(false);
    }

  unsigned int count = 0;
  while (1)
    {
      //now we see what cities we took this turn
      for (Playerlist::iterator pit = Playerlist::getInstance()->begin();
           pit != Playerlist::getInstance()->end(); ++pit)
	{
	  if (*pit == Playerlist::getInstance()->getNeutral())
	    continue;
	  //dump everything up to the next turn
	  guint32 id = (*pit)->getId();
	  if (hit[id] == hist[id]->end())
	    continue;
	  for (; hit[id] != hist[id]->end(); hit[id]++)
	    {
	      if ((*hit[id])->getType() == History::START_TURN)
		{
		  hit[id]++;
		  break;
		}
	      else if ((*hit[id])->getType() == History::CITY_WON)
		{
		  guint32 city_id;
		  city_id = dynamic_cast<History_CityWon*>(*hit[id])->getCityId();
		  //find city with this city id in clist
		  for (LocationList<City*>::iterator cit = clist->begin();
                       cit != clist->end(); ++cit)
		    if ((*cit)->getId() == city_id)
		      {
			(*cit)->setOwner(*pit);
			break;
		      }
		}
	      else if ((*hit[id])->getType() == History::CITY_RAZED)
		{
		  guint32 city_id;
		  city_id = dynamic_cast<History_CityRazed*>(*hit[id])->getCityId();
		  //find city with this city id in clist
		  for (LocationList<City*>::iterator cit = clist->begin();
                       cit != clist->end(); ++cit)
		    if ((*cit)->getId() == city_id)
		      {
			//change the owner to neutral 
			(*cit)->setOwner(Playerlist::getInstance()->getNeutral());
			(*cit)->setBurnt(true);
			break;
		      }
		}
	    }
	  if (hit[id] == hist[id]->end())
	    {
	      count++;
	      if (count == Playerlist::getInstance()->size() - 2)
		last_turn = true;
	    }
	}
      //and add it to the list
      past_citylists.push_back(clist);
      LocationList<City*> *new_clist = new LocationList<City*>();
      for (LocationList<City*>::iterator it = clist->begin(); 
	   it != clist->end(); ++it)
	new_clist->push_back(new City(**it));
      clist = new_clist;
      if (last_turn == true)
	break;

    }
  past_citylists.erase(--past_citylists.end());
}

void HistoryReportDialog::run()
{
  historymap->resize();
  historymap->draw();

  dialog->show_all();
  dialog->run();
}

void HistoryReportDialog::on_map_changed(Cairo::RefPtr<Cairo::Surface> map)
{
  Glib::RefPtr<Gdk::Pixbuf> pixbuf = 
    Gdk::Pixbuf::create(map, 0, 0, 
                        historymap->get_width(), historymap->get_height());
  map_image->property_pixbuf() = pixbuf;
}

void HistoryReportDialog::on_turn_changed()
{
  //tell the historymap to show another set of cities
  guint32 turn = (guint32)turn_scale->get_value();
  if (turn > past_citylists.size() - 1)
    historymap->updateCities(Citylist::getInstance(), Ruinlist::getInstance());
  else
    historymap->updateCities(past_citylists[turn], past_ruinlists[turn]);
  city_chart->set_x_indicator(turn);
  ruin_chart->set_x_indicator(turn);
  gold_chart->set_x_indicator(turn);
  rank_chart->set_x_indicator(turn);
  fill_in_turn_info(turn);
}

void HistoryReportDialog::update_window_title()
{
  switch (history_notebook->get_current_page())
    {
    case CITY:
      dialog->set_title(_("City History"));
      break;
    case RUIN:
      dialog->set_title(_("Ruin History"));
      break;
    case EVENTS: 
      dialog->set_title(_("Event History"));
      break;
    case GOLD: 
      dialog->set_title(_("Gold History"));
      break;
    case WINNING:
      dialog->set_title(_("Winner History"));
      break;
    }
}

void HistoryReportDialog::fill_in_turn_info(guint32 turn)
{
  Glib::ustring s;
  guint32 count;
  update_window_title();

  //update the event list
  //events_list->clear();
  std::vector<Gtk::Widget*> kids = events_list_box->get_children();
  for (unsigned int i = 0; i < kids.size(); i++)
    events_list_box->remove(*kids[i]);

  if (turn < past_eventlists.size())
    {
      std::list<NetworkHistory*> hist = past_eventlists[turn];
      for (std::list<NetworkHistory*>::iterator hit = hist.begin();
           hit != hist.end(); ++hit)
	addHistoryEvent(*hit);
    }

  //update the gold chart
  //on turn # you had # gold pieces
  std::list<guint32> goldlist = *past_goldcounts.begin();
  count=1;
  for (std::list<guint32>::iterator it = goldlist.begin();
       it != goldlist.end(); ++it, count++)
    {
      if (count == turn)
	{
	  count = *it;
	  break;
	}
    }
  turn == past_citylists.size() ?
    s = String::ucompose(ngettext("On turn %1 you have %2 gold piece!",
				  "On turn %1 you have %2 gold pieces!",
				  count), turn, count) :
    s = String::ucompose(ngettext("On turn %1 you had %2 gold piece!",
				  "On turn %1 you had %2 gold pieces!",
				  count), turn, count);
  gold_label->set_text(s);

  //update the city chart
  std::list<guint32> citylist = *past_citycounts.begin();
  count = 0;
  for (std::list<guint32>::iterator it = citylist.begin();
       it != citylist.end(); ++it, count++)
    {
      if (count == turn)
	{
	  count = *it;
	  break;
	}
    }
  turn == past_citylists.size() ?
    s = String::ucompose(ngettext("On turn %1 you have %2 city!",
				  "On turn %1 you have %2 cities!",
				  count), turn, count) :
    s = String::ucompose(ngettext("On turn %1 you had %2 city!",
				  "On turn %1 you had %2 cities!",
				  count), turn, count);
  city_label->set_text(s);

  //update the ruin chart
  std::list<guint32> ruinlist = *past_ruincounts.begin();
  count = 0;
  for (std::list<guint32>::iterator it = ruinlist.begin ();
       it != ruinlist.end(); ++it, count++)
    {
      if (count == turn)
	{
	  count = *it;
	  break;
	}
    }
  turn == past_ruinlists.size() ?
  s = String::ucompose(ngettext("By turn %1 you explored %2 ruin!",
                                "By turn %1 you explored %2 ruins!",
                                count), turn, count) :
  s = String::ucompose(ngettext("By turn %1 you explored %2 ruin!",
                                "By turn %1 you explored %2 ruins!",
                                count), turn, count);
  ruin_label->set_text(s);

  //on turn # you were coming #
  std::list<guint32> scores;
  for (std::list<std::list<guint32> >::iterator rit = past_rankcounts.begin();
       rit != past_rankcounts.end(); ++rit)
    {
      count=1;
      for (std::list<guint32>::iterator it = (*rit).begin();
           it != (*rit).end(); ++it, count++)
	{
	  if (count == turn)
	    {
	      count = *it;
	      scores.push_back(*it);
	      break;
	    }
	}
    }
  turn == past_citylists.size() ?
    s = String::ucompose(_("On turn %1 you are coming %2!"),
			 turn, ReportDialog::calculateRank(scores, *scores.begin())):
    s = String::ucompose(_("On turn %1 you were coming %2!"),
			 turn, ReportDialog::calculateRank(scores, *scores.begin()));
  winner_label->set_text(s);
}

void HistoryReportDialog::on_switch_page()
{
  update_window_title();
}

void HistoryReportDialog::addHistoryEvent(NetworkHistory *event)
{
  ImageCache *gc = ImageCache::getInstance();
  Playerlist *pl = Playerlist::getInstance();
  Player *p = event->getOwner();

  History *history = event->getHistory();

  Gtk::Box *box = NULL;

                              
  Glib::RefPtr<Gdk::Pixbuf> shield =
    gc->getShieldPic(1, p, false,
                     FontSize::getInstance ()->get_height ())->to_pixbuf();
  switch (history->getType())
    {
    case History::FOUND_SAGE: 
	{
	  History_FoundSage *ev = static_cast<History_FoundSage *>(history);
          box = Box::ucompose(_("%1 %2 finds a sage!"), shield,
                              ev->getHeroName());
	  break;
	}
    case History::HERO_EMERGES:
	{
	  History_HeroEmerges *ev = 
            static_cast<History_HeroEmerges *>(history);
          box = Box::ucompose(_("%1 %2 emerges in %3"), shield,
                              ev->getHeroName(), ev->getCityName());
	  break;
	}
    case History::HERO_QUEST_STARTED:
	{
	  History_HeroQuestStarted *ev = 
            static_cast<History_HeroQuestStarted*>(history);
          box = Box::ucompose(_("%1 %2 begins a quest!"), shield,
                              ev->getHeroName());
	  break;
	}
    case History::HERO_QUEST_COMPLETED:
	{
	  History_HeroQuestCompleted *ev
            = static_cast<History_HeroQuestCompleted *>(history);
          box = Box::ucompose(_("%1 %2 finishes a quest!"), shield,
                              ev->getHeroName());
	  break;
	}
    case History::HERO_KILLED_IN_CITY:
	{
	  History_HeroKilledInCity *ev = 
            static_cast<History_HeroKilledInCity *>(history);
          box = Box::ucompose(_("%1 %2 is killed in %3!"), shield,
                              ev->getHeroName(), ev->getCityName());
	  break;
	}
    case History::HERO_KILLED_IN_BATTLE:
	{
	  History_HeroKilledInBattle *ev = 
            static_cast<History_HeroKilledInBattle *>(history);
          box = Box::ucompose(_("%1 %2 is killed in battle!"), shield,
                              ev->getHeroName());
	  break;
	}
    case History::HERO_KILLED_SEARCHING:
	{
	  History_HeroKilledSearching *ev = 
            static_cast<History_HeroKilledSearching *>(history);
          box = Box::ucompose(_("%1 %2 is killed while searching!"), shield,
                              ev->getHeroName());
	  break;
	}
    case History::HERO_CITY_WON:
	{
	  History_HeroCityWon *ev = 
            static_cast<History_HeroCityWon *>(history);
          box = Box::ucompose(_("%1 %2 conquers %3!"), shield,
                              ev->getHeroName(), ev->getCityName());
	  break;
	}
    case History::PLAYER_VANQUISHED:
	{
          box = Box::ucompose(_("%1 %2 utterly vanquished!"), shield,
                              p->getName());
	  break;
	}
    case History::DIPLOMATIC_PEACE:
	{
	  History_DiplomacyPeace *ev = 
            static_cast<History_DiplomacyPeace*>(history);
	  Player *opponent = pl->getPlayer(ev->getOpponentId());
          box =
            Box::ucompose (_("%1 %2 at peace with %3 %4!"), shield,
                           p->getName(), gc->getShieldPic
                           (1, opponent, false,
                            FontSize::getInstance ()->get_height ())->to_pixbuf(),
                           opponent->getName());
	  break;
	}
    case History::DIPLOMATIC_WAR:
	{
	  History_DiplomacyWar *ev = 
            static_cast<History_DiplomacyWar*>(history);
	  Player *opponent = pl->getPlayer(ev->getOpponentId());
          box =
            Box::ucompose (_("%1 %2 at war with %3 %4!"), shield,
                           p->getName(), gc->getShieldPic
                           (1, opponent, false,
                            FontSize::getInstance ()->get_height ())->to_pixbuf(),
                           opponent->getName());
	  break;
	}
    case History::DIPLOMATIC_TREACHERY:
	{
	  History_DiplomacyTreachery *ev = 
            static_cast<History_DiplomacyTreachery*>(history);
	  Player *opponent = pl->getPlayer(ev->getOpponentId());
          box =
            Box::ucompose(_("%1 Treachery on %2 %3!"), shield,
                          gc->getShieldPic
                          (1, opponent, false,
                           FontSize::getInstance ()->get_height ())->to_pixbuf(),
                          opponent->getName());
	  break;
	}
    case History::HERO_FINDS_ALLIES:
	{
	  History_HeroFindsAllies *ev = 
            static_cast<History_HeroFindsAllies*>(history);
          box = Box::ucompose(_("%1 %2 finds allies!"), shield,
                              ev->getHeroName());
	  break;
	}
    case History::HERO_RUIN_EXPLORED:
	{
	  History_HeroRuinExplored *ev = 
            static_cast<History_HeroRuinExplored *>(history);
          Ruinlist *rl = Ruinlist::getInstance();
          box = Box::ucompose(_("%1 %2 explores %3!"), shield,
                              ev->getHeroName(),
                              rl->getById(ev->getRuinId())->getName());
	  break;
	}
    case History::USE_ITEM:
	{
	  History_HeroUseItem *ev = 
            static_cast<History_HeroUseItem*>(history);
	  Player *opponent = pl->getPlayer(ev->getOpponentId());
          if (ev->getItemBonus() & ItemProto::USABLE)
            box =
              Box::ucompose(_("%1 %2 uses the %3 against %4 %5!"), shield,
                            ev->getHeroName(), ev->getItemName(),
                            gc->getShieldPic
                            (1, opponent, false,
                             FontSize::getInstance ()->get_height ())->to_pixbuf(),
                            opponent->getName());
          else
            box = Box::ucompose(_("%1 %2 uses the %3!"), shield,
                                ev->getHeroName(), ev->getItemName());
	  break;
	}
    default:
      box = NULL;
      break;
    }

  if (box)
    {
      events_list_box->pack_start(*manage(box), Gtk::PACK_SHRINK, 0);
      events_list_box->show_all();
    }
}

void HistoryReportDialog::generatePastWinningCounts()
{
  //go through the history list looking for score events, per player
  for (Playerlist::iterator pit = Playerlist::getInstance()->begin();
       pit != Playerlist::getInstance()->end(); ++pit)
    {
      if (*pit == Playerlist::getInstance()->getNeutral())
	continue;
      std::list<History*> *hist = (*pit)->getHistorylist();
      std::list<guint32> line;
      for (std::list<History*>::iterator hit = hist->begin();
           hit != hist->end(); ++hit)
	{
	  if ((*hit)->getType() == History::SCORE)
	    {
	      History_Score *event = static_cast<History_Score*>(*hit);
	      line.push_back (event->getScore());
	    }
	}
      line.push_back ((guint32)(*pit)->getScore());
      if (*pit == d_player)
	past_rankcounts.push_front(line);
      else
	past_rankcounts.push_back(line);
    }
}

void HistoryReportDialog::generatePastCityCounts()
{
  // go through the past city list
  for (Playerlist::iterator pit = Playerlist::getInstance()->begin();
       pit != Playerlist::getInstance()->end(); ++pit)
    {
      if (*pit == Playerlist::getInstance()->getNeutral())
	continue;
      //go through the past city lists, searching for cities owned by this
      //player

      std::list<guint32> line;
      for (unsigned int i = 0; i < past_citylists.size(); i++)
	{
	  guint32 total_cities = 0;
	  for (LocationList<City*>::iterator it = past_citylists[i]->begin();
               it != past_citylists[i]->end(); ++it)
	    {
	      if ((*it)->getOwner() == *pit)
		total_cities++;
	    }
	  line.push_back(total_cities);
	}

      line.push_back(Citylist::getInstance()->countCities(*pit));
      if (*pit == d_player)
	past_citycounts.push_front(line);
      else
	past_citycounts.push_back(line);

    }
}

void HistoryReportDialog::generatePastGoldCounts()
{
  //go through the history list looking for gold events, per player
  for (Playerlist::iterator pit = Playerlist::getInstance()->begin();
       pit != Playerlist::getInstance()->end(); ++pit)
    {
      if (*pit == Playerlist::getInstance()->getNeutral())
	continue;
      std::list<History*> *hist = (*pit)->getHistorylist();
      std::list<guint32> line;
      for (std::list<History*>::iterator hit = hist->begin();
           hit != hist->end(); ++hit)
	{
	  if ((*hit)->getType() == History::GOLD_TOTAL)
	    {
	      History_GoldTotal *event = static_cast<History_GoldTotal*>(*hit);
	      line.push_back (event->getGold());
	    }
	}
      line.push_back ((guint32)(*pit)->getGold());
      if (*pit == d_player)
	past_goldcounts.push_front(line);
      else
	past_goldcounts.push_back(line);
    }
}

void HistoryReportDialog::generatePastRuinCounts()
{
  //how many ruins did the players search at each turn?

  for (Playerlist::iterator pit = Playerlist::getInstance()->begin();
       pit != Playerlist::getInstance()->end(); ++pit)
    {
      if (*pit == Playerlist::getInstance()->getNeutral())
        continue;
      std::list<guint32> line;
      for (unsigned int i = 0; i < past_citylists.size(); i++)
        {
          guint32 total_ruins = 0;
          for (LocationList<Ruin*>::iterator it = past_ruinlists[i]->begin();
               it != past_ruinlists[i]->end(); ++it)
            {
              Ruin *ruin = *it;
              if (ruin->isHidden() == true && ruin->getOwner() != *pit)
                continue;
              if (ruin->isSearched() == true && 
                  (*pit)->searchedRuin(ruin) == true)
                {
                  ruin->setOwner(*pit);
                  total_ruins++;
                }
            }
          line.push_back(total_ruins);
        }
      line.push_back(Ruinlist::getInstance()->countExploredRuins(*pit));
      if (*pit == d_player)
	past_ruincounts.push_front(line);
      else
	past_ruincounts.push_back(line);
    }
}

void HistoryReportDialog::generatePastRuinlists()
{
  //we don't do this per player
  //we just count how many ruins are unexplored at every turn.
  //how do we deal with hidden ruins?
  //they should pop up when found.
  bool last_turn = false;

  //keep a set of pointers to remember how far we are into each player's history
  std::list<History*> *hist[MAX_PLAYERS];
  for (Playerlist::iterator pit = Playerlist::getInstance()->begin();
       pit != Playerlist::getInstance()->end(); ++pit)
    {
      if (*pit == Playerlist::getInstance()->getNeutral())
	continue;
      hist[(*pit)->getId()] = (*pit)->getHistorylist();
    }
  std::list<History*>::iterator hit[MAX_PLAYERS];
  for (Playerlist::iterator pit = Playerlist::getInstance()->begin();
       pit != Playerlist::getInstance()->end(); ++pit)
    {
      if (*pit == Playerlist::getInstance()->getNeutral())
	continue;
      hit[(*pit)->getId()] = hist[(*pit)->getId()]->begin();
    }

  //start off with an initial ruin list where all ruins are unexplored and hidden.
  //all hidden ruins haven't been found yet, unless they started off that way.
  LocationList<Ruin*> *rlist = new LocationList<Ruin*>();
  Ruinlist *rl = Ruinlist::getInstance();
  for (Ruinlist::iterator it = rl->begin(); it != rl->end(); ++it)
    rlist->push_back(new Ruin(**it));
  for (LocationList<Ruin*>::iterator it = rlist->begin(); it != rlist->end(); ++it)
    {
      //is the ruin searched to begin with?
      bool no_ruin_history = true;
      for (Playerlist::iterator pit = Playerlist::getInstance()->begin();
           pit != Playerlist::getInstance()->end(); ++pit)
	if ((*pit)->searchedRuin(*it) == true)
	  no_ruin_history = false;
      if ((*it)->isSearched() == true && no_ruin_history)
        (*it)->setSearched(true);
      else
        {
          (*it)->setSearched(false);
          if ((*it)->isHidden())
            (*it)->setOwner(NULL);
        }
    }

  unsigned int count = 0;
  while (1)
    {
      //now we see what ruins we took this turn
      for (Playerlist::iterator pit = Playerlist::getInstance()->begin();
           pit != Playerlist::getInstance()->end(); ++pit)
	{
	  if (*pit == Playerlist::getInstance()->getNeutral())
	    continue;
	  //dump everything up to the next turn
	  guint32 id = (*pit)->getId();
	  if (hit[id] == hist[id]->end())
	    continue;
	  for (; hit[id] != hist[id]->end(); ++(hit[id]))
	    {
	      if ((*hit[id])->getType() == History::START_TURN)
		{
		  ++(hit[id]);
		  break;
		}
              //when a ruin becomes visible all of a sudden, we mark it as visible
	      else if ((*hit[id])->getType() == History::HERO_REWARD_RUIN)
                {
		  guint32 ruin_id;
		  ruin_id = 
                    dynamic_cast<History_HeroRewardRuin*>(*hit[id])->getRuinId();
		  //find ruin with this ruin id in rlist
		  for (LocationList<Ruin*>::iterator rit = rlist->begin();
                       rit != rlist->end(); ++rit)
		    if ((*rit)->getId() == ruin_id)
		      {
                        (*rit)->setOwner(*pit);
			break;
		      }
                }
	      else if ((*hit[id])->getType() == History::HERO_RUIN_EXPLORED)
		{
		  guint32 ruin_id;
		  ruin_id = 
                    dynamic_cast<History_HeroRuinExplored*>(*hit[id])->getRuinId();
		  //find ruin with this ruin id in rlist
		  for (LocationList<Ruin*>::iterator rit = rlist->begin();
                       rit != rlist->end(); ++rit)
		    if ((*rit)->getId() == ruin_id)
		      {
			(*rit)->setSearched(true);
			break;
		      }
		}
	    }
	  if (hit[id] == hist[id]->end())
	    {
	      count++;
	      if (count == Playerlist::getInstance()->size() - 2)
		last_turn = true;
	    }
	}
      //and add it to the list
      past_ruinlists.push_back(rlist);
      LocationList<Ruin*> *new_rlist = new LocationList<Ruin*>();
      for (LocationList<Ruin*>::iterator it = rlist->begin(); 
	   it != rlist->end(); ++it)
	new_rlist->push_back(new Ruin(**it));
      rlist = new_rlist;
      if (last_turn == true)
	break;

    }
  past_ruinlists.erase(--past_ruinlists.end());
}
