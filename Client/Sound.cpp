/*
Sound.cpp - sound system implementation file
Implements sound system for Unlimited racer using FMOD
by Pavel Celba
created: 13. 12. 2005
*/

#include "Sound.h"

using namespace std;

// constructor
CSoundSystem::CSoundSystem()
{
	System = 0;
	Music = 0;
	MusicChannel = 0;
	PlayingMusic = FALSE;
	PlayingPaused = FALSE;
	MusicVolume = 1.0f;
	SongCallback = 0;
	SongSecondsCallback = 0;
	Seconds = 0;
}

HRESULT CSoundSystem::Init()
{
	FMOD_RESULT hr;
	
	// Create FMOD Sound system
	hr = FMOD::System_Create(&System);
	if (hr != FMOD_OK)
		ERRORMSG(-1, "CSoundSystem::Init()", FMOD_ErrorString(hr) );

	// Initialize FMOD
	hr = System->init(100, FMOD_INIT_NORMAL, 0);
	if (hr != FMOD_OK)
		ERRORMSG(-1, "CSoundSystem::Init()", FMOD_ErrorString(hr) );

	// Set our metrics into FMOD
	hr = System->set3DSettings(1.0f, ONE_METER, 1.0f);
	if (hr != FMOD_OK)
		ERRORMSG(-1, "CSoundSystem::Init()", FMOD_ErrorString(hr) );

	// seed random number generator
	srand( (unsigned) time(NULL) );

	// Not playing music
	PlayingMusic = FALSE;
	PlayingPaused = FALSE;

	return 0;
}

// Sound system update - must be called once per frame
HRESULT CSoundSystem::Update()
{
	HRESULT Result;
	FMOD_RESULT hr;

	Result = UpdateMusic();
	if (FAILED(Result) ) 
		ERRORMSG(Result, "CSoundSystem::Update()", "Error while updating music play.");
	
	hr = System->update();
	if (hr != FMOD_OK)
		ERRORMSG(-1, "CSoundSystem::Update()", FMOD_ErrorString(hr) );

	return 0;
}

// Sets music list
HRESULT CSoundSystem::SetMusicList(vector<CString> & _MusicList)
{
	MusicList = _MusicList;
	return 0;
}

// Starts/stop playing music
HRESULT CSoundSystem::PlayMusic(BOOL PlayMusic)
{
	FMOD_RESULT hr;
	BOOL MusicLoaded = FALSE;
	

	// RandomNumber
	UINT RandomNum;

	if (PlayMusic)
	{
		// Music will not start playing when music volume is zero
		if (MusicVolume == 0.0) return 0;

		// Music is playing and is paused
		if (PlayingPaused) return 0;

		while (!MusicLoaded && MusicList.size() > 0)
		{
			// get random number
			RandomNum = (UINT) rand() % MusicList.size();
			
			// Release before loading new music
			if (Music)
			{
				Music->release(); Music = 0;
			}
			
			// create sound stream
			hr = System->createStream(Configuration.DataPath + Configuration.MusicPath + MusicList[RandomNum], FMOD_DEFAULT, 0, &Music);

			// get artist and title name
			GetSoundInfo(Music, Title, Artist);

			// Let delete filename from music list, if file or filename is somehow bad
			if (hr != FMOD_OK)
			{
				if (hr == FMOD_ERR_FILE_BAD || hr == FMOD_ERR_FILE_COULDNOTSEEK || hr == FMOD_ERR_FILE_EOF || hr == FMOD_ERR_FILE_NOTFOUND)
				{
					MusicList.erase(MusicList.begin() + RandomNum);
				}
				else
					ERRORMSG(-1, "CSoundSystem::PlayMusic()", FMOD_ErrorString(hr) );
			}
			else 
			{
				// Play music
				hr = System->playSound(FMOD_CHANNEL_FREE, Music, 0, &MusicChannel);
				if (hr != FMOD_OK)
					ERRORMSG(-1, "CSoundSystem::PlayMusic()", FMOD_ErrorString(hr) );

				// set music volume
				MusicChannel->setVolume(MusicVolume);

				// invoke callback if exists
				if (SongCallback)
					SongCallback(CallbackPtr);

				// Start counting for N seconds callback
				if (SongSecondsCallback)
					TimeCounter.Start();

				MusicLoaded = TRUE;
				PlayingMusic = TRUE;
			}
		}
	}
	else
	{
		PlayingMusic = FALSE;
		if (MusicChannel)
		{
			hr = MusicChannel->stop();
			if  (hr != FMOD_OK && hr != FMOD_ERR_INVALID_HANDLE)
				ERRORMSG(-1, "CSoundSystem::PlayMusic()", FMOD_ErrorString(hr) );
			MusicChannel = 0;
		}
	}

	return 0;
}

// Pauses music
HRESULT CSoundSystem::PauseMusic(BOOL Pause)
{
	FMOD_RESULT hr;

	if (Pause && PlayingMusic)
	{
		if (MusicChannel)
		{
			hr = MusicChannel->setPaused(true);
			if (hr != FMOD_OK && hr != FMOD_ERR_INVALID_HANDLE)
				ERRORMSG(-1, "CSoundSystem::PauseMusic()", FMOD_ErrorString(hr) );
			PlayingPaused = TRUE;
		}
	}
	else if (PlayingMusic && PlayingPaused)
	{
		if (MusicChannel)
			hr = MusicChannel->setPaused(false);
		if (hr != FMOD_OK && hr != FMOD_ERR_INVALID_HANDLE)
			ERRORMSG(-1, "CSoundSystem::PauseMusic()", FMOD_ErrorString(hr) );

		PlayingPaused = FALSE;
	}

	return 0;
}

// Updates music
HRESULT CSoundSystem::UpdateMusic()
{
	HRESULT Result;
	FMOD_RESULT hr;
	bool IsPlaying = false;
	UINT CurTime;

	if (PlayingMusic && MusicChannel)
	{
		hr = MusicChannel->isPlaying(&IsPlaying);
		if (hr != FMOD_OK && hr != FMOD_ERR_INVALID_HANDLE)
			ERRORMSG(-1, "CSoundSystem::UpdateMusic()", FMOD_ErrorString(hr) );
		
		// Test whether to invoke N seconds callback
		if (IsPlaying && SongSecondsCallback && TimeCounter.IsActive())
		{
			CurTime = TimeCounter.GetRunTime();
			if (CurTime >= Seconds)
			{
				SongSecondsCallback(CallbackSecondsPtr);
				TimeCounter.Pause();
			}
		}

		if (!IsPlaying)
		{
			hr = MusicChannel->stop();
			if (hr != FMOD_OK && hr != FMOD_ERR_INVALID_HANDLE)
				ERRORMSG(-1, "CSoundSystem::UpdateMusic()", FMOD_ErrorString(hr) );

			Result = PlayMusic(TRUE);
			if (FAILED(Result) )
				ERRORMSG(-1, "CSoundSystem::UpdateMusic()", "Error while trying to play another song.");
		}
	}

	return 0;
}

// Returns artist and title (if found)
HRESULT CSoundSystem::GetSoundInfo(FMOD::Sound * Sound, CString & Title, CString & Artist)
{
	FMOD_RESULT hr;
	FMOD_TAG Tag;
	CString TagName;

	// clear in case nothing better is found
	Title = "";
	Artist = "";

	while (true)
	{
		hr = Sound->getTag(0, -1, &Tag);
		if (hr != FMOD_OK) break;

		if (Tag.type == FMOD_TAGTYPE_ID3V1)
		{
			TagName = Tag.name;
			if (TagName.MakeUpper() == "TITLE")
			{
				Title = (char *) Tag.data;
			}
			else if (TagName.MakeUpper() == "ARTIST")
			{
				Artist = (char *) Tag.data;
			}
		}

	}

	return 0;
}

// Sets music volume
// between 0.0 and 1.0
void CSoundSystem::SetMusicVolume(float Volume)
{
	if (Volume < 0.0) Volume = 0.0;
	if (Volume > 1.0) Volume = 1.0;
	
	MusicVolume = Volume;

	// Disable music playing, when volume is zero
	if (MusicVolume == 0.0) 
		PlayMusic(false);
	// If music is not playing, start playing
	//else if (!PlayingMusic)
	//	PlayMusic(true);

	// Change music volume, if song is playing
	if (PlayingMusic && MusicChannel)
		MusicChannel->setVolume(MusicVolume);
}