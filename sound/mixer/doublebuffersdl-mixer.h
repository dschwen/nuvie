/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL: https://scummvm.svn.sourceforge.net/svnroot/scummvm/scummvm/trunk/backends/mixer/doublebuffersdl/doublebuffersdl-mixer.h $
 * $Id: doublebuffersdl-mixer.h 54584 2010-11-29 18:48:43Z lordhoto $
 *
 */

#ifndef BACKENDS_MIXER_DOUBLEBUFFERSDL_H
#define BACKENDS_MIXER_DOUBLEBUFFERSDL_H

#include "SDL.h"
#include "nuvieDefs.h"
#include "sdl-mixer.h"

/**
 * SDL mixer manager with double buffering support.
 */
class DoubleBufferSDLMixerManager : public SdlMixerManager {
public:
	DoubleBufferSDLMixerManager();
	virtual ~DoubleBufferSDLMixerManager();

protected:
	SDL_mutex *_soundMutex;
	SDL_cond *_soundCond;
	SDL_Thread *_soundThread;
	bool _soundThreadIsRunning;
	bool _soundThreadShouldQuit;

	uint8 _activeSoundBuf;
	uint32 _soundBufSize;
	uint8 *_soundBuffers[2];

	/**
	 * Handles and swap the sound buffers
	 */
	void mixerProducerThread();

	/**
	 * Finish the mixer manager
	 */
	void deinitThreadedMixer();

	/**
	 * Callback entry point for the sound thread
	 */
	static int SDLCALL mixerProducerThreadEntry(void *arg);

	virtual void startAudio();
	virtual void callbackHandler(uint8 *samples, int len);
};

#endif
