/*
Sound.h - sound system class declaration
Provides function for playing sound in Unlimited racer game
Uses FMOD API 
by Pavel Celba
13. 12. 2005
*/

#pragma once

#include "stdafx.h"
#include "..\Globals\Globals.h"
#include "..\Globals\ErrorHandler.h"
#include "..\Globals\Configuration.h"
#include "..\Globals\TimeCounter.h"



// Sound system class
class CSoundSystem
{
public:
	// constructor
	CSoundSystem();

	// Inits sound system
	HRESULT Init();

	// Sound system update - must be called once per frame
	HRESULT Update();

	// MUSIC
	
	// Sets music list
	HRESULT SetMusicList(std::vector<CString> & _MusicList);

	// Starts/stop playing music
	HRESULT PlayMusic(BOOL Music);

	// Pauses music
	HRESULT PauseMusic(BOOL Pause);

	// Sets music volume
	// between 0.0 and 1.0
	void SetMusicVolume(float Volume);

	// Gets music volume
	inline float GetMusicVolume()
	{
		return MusicVolume;
	}

	// Is music playing
	inline BOOL IsMusicPlaying()
	{
		return PlayingMusic;
	}

	// Get title
	inline CString GetTitle() const
	{
		return Title;
	}

	// Get artist
	inline CString GetArtist() const
	{
		return Artist;
	}

	// Is called, when new song starts to play
	// Ptr are data sent to callback function - mainly for being able to callback from class instance
	inline void SetNewSongCallback( void (* _SongCallback)(void *) , void * Ptr)
	{
		SongCallback = _SongCallback;
		CallbackPtr = Ptr;
	}

	// Is called N seconds after new song starts to play
	// Ptr are data sent to callback function - mainly for being able to callback from class instance
	// PRECONDITION: Seconds >= 0
	inline void SetNewSongSecondsCallback( void (* _SongCallback) (void *), void * Ptr, UINT _Seconds)
	{
		SongSecondsCallback = _SongCallback;
		CallbackSecondsPtr = Ptr;
		Seconds = _Seconds * 1000;
	}

private:
	// FMOD sound system
	FMOD::System * System;

	// MUSIC
	// Playing music
	BOOL PlayingMusic;
	BOOL PlayingPaused;
	
	// Music
	FMOD::Sound * Music;

	// Music channel
	FMOD::Channel * MusicChannel;

	// Music list - contains all songs to play
	std::vector<CString> MusicList;

	// Music volume
	float MusicVolume;

	// Track artist
	CString Artist;

	// Track title
	CString Title;

	// Callbacks
	void (* SongCallback ) (void *);
	void * CallbackPtr;

	void (* SongSecondsCallback) (void *);
	void * CallbackSecondsPtr;
	// Milisecounds after song started to play 
	UINT Seconds;

	// Time counter
	CTimeCounter TimeCounter;

	// Updates music
	HRESULT UpdateMusic();

	// Returns artist and title (if found)
	HRESULT GetSoundInfo(FMOD::Sound * Sound, CString & Title, CString & Artist);
};