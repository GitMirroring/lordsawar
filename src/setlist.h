// Copyright (C) 2009, 2014, 2015, 2020 Ben Asselstine
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

#pragma once
#ifndef SETLIST_H
#define SETLIST_H
#include <iostream>
#include <assert.h>
#include "File.h"
#include "tarhelper.h"
#include "ucompose.hpp"
#include "defs.h"
#include <map>
#include <list>
#include <sigc++/signal.h>

//! Template for Tilesetlist, Armysetlist, Citysetlist, and Shieldsetlist objects.
template<class T>
class SetList: public std::list<T*>
{
public:
    SetList(Glib::ustring ext) : extension (ext) {}
    ~SetList()
      {
        for (class SetList<T>::iterator it = this->begin(); it != this->end();
             ++it)
          delete (*it);
      };
    static Glib::ustring getConfigurationFilename(Glib::ustring dir, Glib::ustring subdir, Glib::ustring ext) {return File::add_slash_if_necessary(dir) + subdir + "/" + subdir + ext;};
    static std::list<Glib::ustring> scan(Glib::ustring extension, bool system = true)
      {
        if (system == false)
          return File::scanForFiles(File::getSetDir(extension, false), extension);
        else
          {
            std::list<Glib::ustring> retlist = 
              File::scanForFiles(File::getSetDir(extension), extension);
            if (retlist.empty())
              {
                //note to translators: %1 is a file extension, %2 is a directory.
                std::cerr << String::ucompose(_("Couldn't find any *%1 files in `%2'."),extension, File::getSetDir(extension)) << std::endl;
                std::cerr << String::ucompose(_("Please check the path settings in %1"), File::getConfigFile(DEFAULT_CONFIG_FILENAME)) << std::endl;
                exit(-1);
              }
            return retlist;
          }

      }

    bool contains(Glib::ustring name) const
      {
        for (class SetList<T>::const_iterator it = this->begin(); 
             it != this->end(); ++it)
          if ((*it)->getName() == name)
            return true;

        return false;
      }

    static int getNextAvailableId(int after)
      {
        bool unsupported_version = false;
        std::list<guint32> ids;
        std::list<Glib::ustring> sets = SetList::scan(T::file_extension);
        for (std::list<Glib::ustring>::const_iterator i = sets.begin(); 
             i != sets.end(); ++i)
          {
            T *set = T::create(*i, unsupported_version);
            if (set != NULL)
              {
                ids.push_back(set->getId());
                delete set;
              }
          }
        sets = SetList::scan(T::file_extension, false);
        for (std::list<Glib::ustring>::const_iterator i = sets.begin(); 
             i != sets.end(); ++i)
          {
            T *set = T::create(*i, unsupported_version);
            if (set != NULL)
              {
                ids.push_back(set->getId());
                delete set;
              }
          }
        for (guint32 i = after + 1; i < 1000000; i++)
          {
            if (find(ids.begin(), ids.end(), i) == ids.end())
              return i;
          }
        return -1;
      }

    T * get(guint32 id) const
      {
        typename SetIdMap::const_iterator it = d_setids.find(id);
        if (it == d_setids.end())
          return NULL;
        return (*it).second;
      }

    T *get(Glib::ustring bname) const
      { 
        typename SetMap::const_iterator it = d_sets.find(bname);
        if (it == d_sets.end())
          return NULL;
        return (*it).second;
      }

    T *get(Glib::ustring name, guint32 size) const
      { 
        Glib::ustring n = String::ucompose ("%1 %2", name, size);
        typename SetNameMap::const_iterator it = d_namesets.find(n);
        if (it == d_namesets.end())
          return NULL;
        return (*it).second;
      }

    void add(T *set, Glib::ustring file)
      {
        Glib::ustring basename = File::get_basename(file);
        this->push_back(set);
        set->setBaseName(basename);
        d_setdirs[String::ucompose("%1 %2", set->getName(),
                                   set->getTileSize())] = basename;
        d_sets[basename] = set;
        d_setids[set->getId()] = set;
        d_namesets[String::ucompose ("%1 %2", set->getName(),
                                     set->getTileSize ())] = set;
        add_signal.emit(set);
      }

    Glib::ustring lookupConfigurationFileByName(T *set)
      {
        T* f = get (set->getName (), set->getTileSize ());
        if (!f)
          return "";
        else
          return f->getConfigurationFile (true);
      }

    T* loadSet(Glib::ustring name)
      {
        bool unsupported_version = false;

        T *set = T::create(name, unsupported_version);
        if (!set)
          {
            std::cerr << String::ucompose(_("Error!  `%1' is malformed.  Skipping."), File::get_basename(name, true)) << std::endl;
            return NULL;
          }

        if (d_setdirs.find(set->getName()) != d_setdirs.end())
          {
            Glib::ustring basename = (*d_setdirs.find(set->getName())).second;
            if (basename != "")
              {
                T *s = (*d_sets.find(basename)).second;
                std::cerr << String::ucompose(_("Error!  `%1' shares a duplicate name `%2' with `%3'.  Skipping."), set->getConfigurationFile(), s->getName(), s->getConfigurationFile()) << std::endl;
                delete set;
              }
            return NULL;
          }

        if (d_setids.find(set->getId()) != d_setids.end())
          {
            T *s = (*d_setids.find(set->getId())).second;
            std::cerr << String::ucompose(_("Error!  `%1' shares a duplicate id with `%2'.  Skipping."), set->getConfigurationFile(), s->getConfigurationFile()) << std::endl;
            delete set;
            return NULL;
          }

        return set;
      }

    Glib::ustring findFreeName(Glib::ustring n, guint32 max, guint32 &num, guint32 ts = 0) const
      {
        Glib::ustring new_name;
        for (unsigned int count = 1; count < max; count++)
          {
            new_name = String::ucompose("%1 %2", n, count);
            if (get(new_name, ts) == NULL)
              {
                num = count;
                return new_name;
              }
            else
              new_name = "";
          }
        return "";
      }

    Glib::ustring findFreeBaseName(Glib::ustring basename, guint32 max, guint32 &num) const
      {
        Glib::ustring new_basename;
        for (unsigned int count = 1; count < max; count++)
          {
            new_basename = String::ucompose("%1%2", basename, count);
            if (get(new_basename) == NULL)
              {
                num = count;
                break;
              }
            else
              new_basename = "";
          }
        return new_basename;
      }

    bool addToPersonalCollection(T *set, Glib::ustring &new_basename, guint32 &new_id)
      {
        //do we already have this one?

        if (get(set->getBaseName()) == get(set->getId())
            && get(set->getBaseName()) != NULL)
          {
            set->setDirectory(get(set->getId())->getDirectory());
            return false;
          }

        //if the basename conflicts with any other basename, then change it.
        if (get(set->getBaseName()) != NULL)
          {
            if (new_basename != "" && get(new_basename) == NULL)
              ;
            else
              {
                guint32 num = 0;
                new_basename = findFreeBaseName(set->getBaseName(), 100, num);
                if (new_basename == "")
                  return false;
              }
          }
        else if (new_basename == "")
          new_basename = set->getBaseName();

        //if the id conflicts with any other id, then change it
        if (get(set->getId()) != NULL)
          {
            if (new_id != 0 && get(new_id) == NULL)
              set->setId(new_id);
            else
              {
                new_id = getNextAvailableId(set->getId());
                set->setId(new_id);
              }
          }
        else
          new_id = set->getId();

        //make the directory where the armyset is going to live.
        Glib::ustring file = File::getSetDir(extension, false) + new_basename + extension;

        set->save(file, extension);

        if (new_basename != set->getBaseName())
          set->setBaseName(new_basename);
        set->setDirectory(File::get_dirname(file));
        add (set, file);
        return true;
      }

    guint32 import(Tar_Helper *t, Glib::ustring f, bool &broken)
      {
        bool unsupported_version;
        Glib::ustring filename = t->getFile(f, broken);
        if (broken)
          return 0;
        T*set = T::create(filename, unsupported_version);
        assert (set != NULL);
        set->setBaseName(File::get_basename(f));

        Glib::ustring basename = "";
        guint32 id = 0;
        if (addToPersonalCollection(set, basename, id) == false)
          {
            id = set->getId();
            delete set;
          }

        return id;
      }

    int getSetId(Glib::ustring bname) const
      {
        T *s = get(bname);
        if (s == NULL)
          return -1;
        return s->getId();
      }

    Glib::ustring getSetDir(Glib::ustring bname, guint32 tilesize = 0) const
      {
        typename SetDirMap::const_iterator it = 
          d_setdirs.find(String::ucompose("%1 %2", bname, tilesize));
        if (it == d_setdirs.end())
          return NULL;

        return get((*it).second)->getBaseName();
      }

    void getSizes(std::list<guint32> &sizes) const
      {
        for (class SetList<T>::const_iterator it = this->begin(); 
             it != this->end(); ++it)
          sizes.push_back((*it)->getUnscaledTileSize());
        sizes.sort ();
        sizes.unique ();
      }

    std::list<Glib::ustring> getValidNames(guint32 tilesize) const
      {
        std::list<Glib::ustring> names;
        for (class SetList<T>::const_iterator it = this->begin(); 
             it != this->end(); ++it)
          if ((*it)->getUnscaledTileSize() == tilesize &&
              (*it)->validate() == true)
            names.push_back((*it)->getName());
        names.sort(case_insensitive);
        return names;
      }

    bool reload(guint32 id) 
      {
        T *set = get(id);
        if (!set)
          return false;
        bool broken = false;
        set->reload(broken);
        if (broken)
          return false;
        remove_mapping_setid_with_id (id);
        remove_mapping_set_with_id (id);
        remove_mapping_nameset_with_id (id);
        d_setids[set->getId()] = set;
        d_sets[set->getBaseName()] = set;
        d_namesets[String::ucompose ("%1 %2", set->getName(),
                                     set->getTileSize ())] = set;
        reload_signal.emit(set);
        return true;
      }

    void loadSets(std::list<Glib::ustring> sets)
      {
        for (std::list<Glib::ustring>::const_iterator i = sets.begin(); 
             i != sets.end(); ++i)
          {
            T *set = loadSet(*i);
            if (!set)
              continue;

            add(set, *i);
          }
      }
    sigc::signal<void, T*> signal_add() {return add_signal;}
    sigc::signal<void, T*> signal_reload() {return reload_signal;}

    typedef std::map<guint32, T*> SetIdMap;
    typedef std::map<Glib::ustring, T*> SetMap;
    typedef std::map<Glib::ustring, T*> SetNameMap;
    typedef std::map<Glib::ustring, Glib::ustring> SetDirMap;
private: 
    Glib::ustring extension;
    SetMap d_sets;
    SetIdMap d_setids;
    SetDirMap d_setdirs;
    SetNameMap d_namesets;
    sigc::signal<void, T*> add_signal;
    sigc::signal<void, T*> reload_signal;

    void remove_mapping_setid_with_id (guint32 id)
      {
        for (typename SetIdMap::iterator i = d_setids.begin ();
             i != d_setids.end (); ++i)
          {
            T *s = (*i).second;
            if (s && s->getId () == id)
              {
                d_setids.erase (i);
                break;
              }
          }
      }

    void remove_mapping_set_with_id (guint32 id)
      {
        for (typename SetMap::iterator i = d_sets.begin ();
             i != d_sets.end (); ++i)
          {
            T *s = (*i).second;
            if (s && s->getId () == id)
              {
                d_sets.erase (i);
                break;
              }
          }
      }

    void remove_mapping_nameset_with_id (guint32 id)
      {
        for (typename SetNameMap::iterator i = d_namesets.begin ();
             i != d_namesets.end (); ++i)
          {
            T *s = (*i).second;
            if (s && s->getId () == id)
              {
                d_namesets.erase (i);
                break;
              }
          }
      }
};

#endif
