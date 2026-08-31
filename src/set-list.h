//  Copyright (C) 2009, 2014, 2015, 2020, 2021, 2026 Ben Asselstine
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

#pragma once
#ifndef SETLIST_H
#define SETLIST_H
#include <iostream>
#include <assert.h>
#include "file.h"
#include "tar-helper.h"
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

    SetList (std::string ext)
      : extension (ext)
      {
      }

    ~SetList ()
      {
        for (class SetList<T>::iterator it = this->begin (); it != this->end ();
             ++it)
          delete (*it);
      }

    static std::string getConfigurationFilename (std::string dir,
                                                 std::string subdir,
                                                 std::string ext)
      {
        return File::add_slash_if_necessary (dir) + subdir + "/" + subdir + ext;
      }

    static std::list<std::string> scan (std::string ex, bool system = true)
      {
        if (system == false)
          return File::scanForFiles (File::getSetDir (ex, false), ex);
        else
          {
            std::list<std::string> retlist =
              File::scanForFiles (File::getSetDir (ex), ex);
            if (retlist.empty ())
              {
                //note to translators: %1 is a file extension, %2 is a directory.
                std::cerr <<
                  String::ucompose (_("Couldn't find any *%1 files in `%2'."),
                                    ex, File::getSetDir (ex)) << std::endl;
                std::cerr <<
                  String::ucompose (_("Please check the path settings in %1"),
                                    File::getConfigFile (DEFAULT_CONFIG_FILENAME))
                  << std::endl;
                exit (1);
              }
            return retlist;
          }
      }

    bool contains (std::string name) const
      {
        for (class SetList<T>::const_iterator it = this->begin (); 
             it != this->end (); ++it)
          if ((*it)->getName () == name)
            return true;

        return false;
      }

    static int getNextAvailableId (int after)
      {
        std::list<guint32> ids;
        std::list<std::string> sets = SetList::scan (T::file_extension);
        for (std::list<std::string>::const_iterator i = sets.begin (); 
             i != sets.end (); ++i)
          {
            sigc::slot<void(T*, bool, bool, Glib::ustring)> finish_slot
              ([&ids](T* set, bool broken, bool unsupported, Glib::ustring)
               {
                 if (set && !broken && !unsupported)
                   {
                     ids.push_back (set->getId ());
                     delete set;
                   }
               });
            T::create (*i, finish_slot);
          }
        sets = SetList::scan (T::file_extension, false);
        for (std::list<std::string>::const_iterator i = sets.begin (); 
             i != sets.end (); ++i)
          {
            sigc::slot<void(T*, bool, bool, Glib::ustring)> finish_slot
              ([&ids](T* set, bool broken, bool unsupported, Glib::ustring)
               {
                 if (set && !broken && !unsupported)
                   {
                     ids.push_back (set->getId ());
                     delete set;
                   }
               });
            T::create (*i, finish_slot);
          }
        for (guint32 i = after + 1; i < 1000000; i++)
          {
            if (find (ids.begin (), ids.end (), i) == ids.end ())
              return i;
          }
        return -1;
      }

    T * get (guint32 id) const
      {
        typename SetIdMap::const_iterator it = d_setids.find (id);
        if (it == d_setids.end ())
          return NULL;
        return (*it).second;
      }

    T *get (std::string bname) const
      {
        typename SetMap::const_iterator it = d_sets.find (bname);
        if (it == d_sets.end ())
          return NULL;
        return (*it).second;
      }

    T *get (std::string name, guint32 size) const
      {
        std::string n = String::ucompose ("%1 %2", name, size);
        typename SetNameMap::const_iterator it = d_namesets.find (n);
        if (it == d_namesets.end ())
          return NULL;
        return (*it).second;
      }

    void add (T *set, std::string file)
      {
        std::string basename = File::get_basename (file);
        this->push_back (set);
        set->setBaseName (basename);
        Glib::ustring n = 
          String::ucompose ("%1 %2", set->getName (), set->getTileSize ());
        d_setdirs[n] = basename;
        d_sets[basename] = set;
        d_setids[set->getId ()] = set;
        d_namesets[n] = set;
        add_signal.emit (set);
      }

    std::string lookupConfigurationFileByName (T *set)
      {
        T* f = get (set->getName (), set->getTileSize ());
        if (!f)
          return "";
        else
          return f->getConfigurationFile (true);
      }

    void loadSet (std::string name, sigc::slot<void(T*, bool, bool, Glib::ustring)> &finish)
      {
        sigc::slot<void(T*, bool, bool, Glib::ustring)> finish_slot
          ([this, name, finish](T* set, bool broken, bool unsupported, Glib::ustring err)
           {
             if (!set)
               {
                 finish (set, broken, unsupported, err);
                 return;
               }

             if (d_setdirs.find (set->getName ()) != d_setdirs.end ())
               {
                 std::string basename =
                   (*d_setdirs.find (set->getName ())).second;
                 if (basename != "")
                   {
                     T *s = (*d_sets.find (basename)).second;
                     Glib::ustring err2 =
                       String::ucompose (_("City Set file `%1' shares a duplicate name `%2' with `%3'.  Skipping."),
                                         set->getConfigurationFile (),
                                         s->getName (),
                                         s->getConfigurationFile ());
                     delete set;
                     finish (NULL, broken, unsupported, err2);
                   }
                 return;
               }

             if (d_setids.find (set->getId ()) != d_setids.end ())
               {
                 T *s = (*d_setids.find (set->getId ())).second;
                 Glib::ustring err2 =
                   String::ucompose (_("City Set file `%1' shares a duplicate id with `%2'.  Skipping."),
                                     set->getConfigurationFile (),
                                     s->getConfigurationFile ());
                 delete set;
                 finish (NULL, broken, unsupported, err2);
                 return;
               }
             finish (set, broken, unsupported, err);
           });
        T::create (name, finish_slot);
      }

    std::string findFreeName (std::string n, guint32 max, guint32 &num, guint32 ts = 0) const
      {
        std::string new_name;
        for (unsigned int count = 1; count < max; count++)
          {
            new_name = String::ucompose ("%1 %2", n, count);
            if (get (new_name, ts) == NULL)
              {
                num = count;
                return new_name;
              }
            else
              new_name = "";
          }
        return "";
      }

    std::string findFreeBaseName (std::string basename, guint32 max, guint32 &num) const
      {
        std::string new_basename;
        for (unsigned int count = 1; count < max; count++)
          {
            new_basename = String::ucompose ("%1%2", basename, count);
            if (get (new_basename) == NULL)
              {
                num = count;
                break;
              }
            else
              new_basename = "";
          }
        return new_basename;
      }

    bool addToPersonalCollection (T *set, std::string &new_basename, guint32 &new_id)
      {
        if (get (set->getBaseName ()) == get (set->getId ())
            && get (set->getBaseName ()) != NULL)
          {
            set->setDirectory (get (set->getId ())->getDirectory ());
            return false;
          }

        //if the basename conflicts with any other basename, then change it.
        if (get (set->getBaseName ()) != NULL)
          {
            if (new_basename != "" && get (new_basename) == NULL)
              ;
            else
              {
                guint32 num = 0;
                new_basename = findFreeBaseName (set->getBaseName (), 100, num);
                if (new_basename == "")
                  return false;
              }
          }
        else if (new_basename == "")
          new_basename = set->getBaseName ();

        //if the id conflicts with any other id, then change it
        if (get (set->getId ()) != NULL)
          {
            if (new_id != 0 && get (new_id) == NULL)
              set->setId (new_id);
            else
              {
                new_id = getNextAvailableId (set->getId ());
                set->setId (new_id);
              }
          }
        else
          new_id = set->getId ();

        //make the directory where the armyset is going to live.
        std::string file =
          File::getSetDir (extension, false) + new_basename + extension;

        set->save (file, extension);

        if (new_basename != set->getBaseName ())
          set->setBaseName (new_basename);
        set->setDirectory (File::get_dirname (file));
        add (set, file);
        return true;
      }

    void import_file (Tar_Helper *t, std::string f, bool &broken)
      {
        std::string filename = t->getFile (f, broken);
        if (broken)
          return;
        sigc::slot<void(T*, bool, bool, Glib::ustring)> finish_slot
          ([this, f](T* set, bool broke, bool unsupported, Glib::ustring)
           {
             if (set && !broke && !unsupported)
               {
                 set->setBaseName (File::get_basename (f));

                 std::string basename = "";
                 guint32 id = 0;
                 if (addToPersonalCollection (set, basename, id) == false)
                   {
                     id = set->getId ();
                     delete set;
                     signal_imported ().emit (id);
                   }
               }
           });
        T::create (filename, finish_slot);

        return;
      }

    int getSetId (std::string bname) const
      {
        T *s = get (bname);
        if (s == NULL)
          return -1;
        return s->getId ();
      }

    std::string getSetDir (std::string bname, guint32 tilesize = 0) const
      {
        typename SetDirMap::const_iterator it = 
          d_setdirs.find (String::ucompose ("%1 %2", bname, tilesize));
        if (it == d_setdirs.end ())
          return "";

        return get ((*it).second)->getBaseName ();
      }

    void getSizes (std::list<guint32> &sizes) const
      {
        for (class SetList<T>::const_iterator it = this->begin (); 
             it != this->end (); ++it)
          sizes.push_back ((*it)->getTileSize ());
        sizes.sort ();
        sizes.unique ();
      }

    std::list<guint32> getValidIds (guint32 tilesize) const
      {
        std::vector<T*> objects;
        for (class SetList<T>::const_iterator it = this->begin (); 
             it != this->end (); ++it)
          if ((*it)->getTileSize () == tilesize &&
              (*it)->validate () == true)
            objects.push_back (*it);

        std::sort
          (objects.begin(), objects.end (),
           [](const T* a, const T* b)
           {
             return (a->getName ().casefold () < b->getName ().casefold ());
           });

        std::list<guint32> ids;
        for (auto o : objects)
          ids.push_back (o->getId ());

        return ids;
      }

    bool reload (guint32 id)
      {
        T *set = get (id);
        if (!set)
          return false;
        set->reload ();
        remove_mapping_setid_with_id (id);
        remove_mapping_set_with_id (id);
        remove_mapping_nameset_with_id (id);
        d_setids[set->getId ()] = set;
        d_sets[set->getBaseName ()] = set;
        d_namesets[String::ucompose ("%1 %2", set->getName (),
                                     set->getTileSize ())] = set;
        reload_signal.emit (set);
        return true;
      }

    void loadSets (std::list<std::string> sets)
      {
        for (std::list<std::string>::const_iterator i = sets.begin (); 
             i != sets.end (); ++i)
          {
            sigc::slot<void(T*, bool, bool, Glib::ustring)> finish_slot
              ([this, i](T* set, bool broken, bool unsupported, Glib::ustring)
               {
                 if (!broken && !unsupported)
                   add (set, *i);
               });

            loadSet (*i, finish_slot);
          }
      }

    void replace (T* set)
      {
        T* orig = get (set->getId ());
        if (orig)
          std::replace (this->begin (), this->end (), orig, set);
      }

    sigc::signal<void(T*)> signal_add ()
      {
        return add_signal;
      }

    sigc::signal<void(T*)> signal_reload ()
      {
        return reload_signal;
      }

    sigc::signal<void(guint32)> signal_imported ()
      {
        return import_signal;
      }

    typedef std::map<guint32, T*> SetIdMap;
    typedef std::map<std::string, T*> SetMap;
    typedef std::map<std::string, T*> SetNameMap;
    typedef std::map<std::string, std::string> SetDirMap;
private: 
    std::string extension;
    SetMap d_sets;
    SetIdMap d_setids;
    SetDirMap d_setdirs;
    SetNameMap d_namesets;
    sigc::signal<void(T*)> add_signal;
    sigc::signal<void(T*)> reload_signal;
    sigc::signal<void(guint32)> import_signal;

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
