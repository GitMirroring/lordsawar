// Copyright (C) 2006 Ulf Lorenz
// Copyright (C) 2006 Andrea Paternesi
// Copyright (C) 2007, 2009, 2014, 2015, 2020 Ben Asselstine
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
#ifndef SND_H
#define SND_H

#include <map>
#include <vector>
#include <sigc++/trackable.h>

class XML_Helper;
namespace Gst{
class Message;
}

//! A helper struct to represent a single item in the sound configuration file.
struct MusicItem
{
    // The file where the sound piece can be loaded from
    Glib::ustring file;
    // Can it be played in the background?
    bool background;
    // If loading this file fails, we can define an alias to load instead.
    Glib::ustring alias;
};


//! This class manages sound within the game.
/** Snd class
  * 
  * The purpose of putting the sound code into one class is (besides hiding the
  * internals and supplying a friendly interface) to put all these ugly ifdefs
  * in one place (sound can be disabled for fewer dependencies).
  *
  * @note: As this becomes too complicated, I throw away the caching stuff.
  *
  * Sound and music are treated a bit differently since the mixer calls are others.
  * However, both are referenced by strings (I think in this case, id's aren't
  * very readable). There are basically two types of music. Background music
  * is enabled by enableBackground(). It will always play until disabled again
  * by looking through the databse of available background music pieces. On top
  * of that, it is possible to play other music pieces. In this case, the
  * background music fades out (maybe), the other music fades or pops in
  * and goes away again with the background music taking its place again
  * afterwards.
  */
class Snd : public sigc::trackable
{
    public:
	// Get Methods
        
        //! Returns whether music is enabled
        bool isMusicEnabled();

        //! Returns the music volume in the range 0..128
        int getMusicVolume();

        //! Get the filename for the given piece.
        Glib::ustring getFile(Glib::ustring piece);


	// Methods that operate on class data and modify the class.

        /** Plays a given music piece.
          *
          * The current (background) track will be stopped (faded out) if
          * neccessary and the new piece will be faded in. Each call to
          * play should be accompanied by a call to halt(), otherwise the
          * background music wil not continue.
          * 
          * @param piece        the identifier(name) of the music track to play.
          * @param nloops       the amount of time the piece should be played
          *                     (-1: infinitely often)
          * @param fade         if set to true, fade out a playing music piece 
          * @return false if any error occurred.
          */
        bool play(Glib::ustring piece, int nloops = -1, bool fade = true);
        
        void updateVolume();

        /** Stops the current (event) music. Note that the background music might
          * continue with playing.
          *
          * @param fade         if set to true, fade out a playing piece.
          *
          * @return false on error.
          */
        bool halt(bool fade = true);

        /** Enables background music.
          * 
          * Starts playing background music. Picks a random piece that has
          * the background tag enabled and starts playing it, then picks the
          * next etc.
          */
        void enableBackground();

        /** Stops playing of background music
          * 
          */
        void disableBackground();

        //! Activates the next background piece
        void nextPiece();


	// Static Methods

        //! Singleton getter
        static Snd* getInstance();

        //! Explicitly delete the singleton
        static void deleteInstance();

    private:
        //! Constructor.  Initializes the sound and loads the music data
        Snd();

        //! Destructor.  Deinitializes sound
        ~Snd();

        //! Callback for the music data, see XML_Helper
        bool loadMusic(Glib::ustring tag, XML_Helper* helper);

	// DATA
        
        // music is stored here, access by d_musicMap[name]
        std::map<Glib::ustring, MusicItem*> d_musicMap;
        std::vector<Glib::ustring> d_bgMap;  // shallow copy of background pieces


        // how many more times we have to loop an effect.
        gint32 d_nloops;

        // if initialization failed, set this to true => no music/sound played
        bool d_broken;

        // if set to true, play background music
        bool d_background;

        struct Impl;
        Impl *impl;
        // callbacks

        bool on_bus_message(const Glib::RefPtr<Gst::Message> & msg, guint32 source);
        bool on_effect_fade (double step);

        // static instanton pointer
        static Snd* s_instance;
};

#endif //SND_H
