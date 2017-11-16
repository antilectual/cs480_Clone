#ifndef WINDOW_H
#define WINDOW_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include <string>

#include "imgui_impl_sdl_gl3.h"

using namespace std;

#define FLIPPER_SOUND "sounds/flipper.wav"
#define BUMPER_SOUND "sounds/bumper.wav"
#define BGMUSIC_SOUND "sounds/music.mp3"
#define LAUNCHER_SOUND "sounds/launch.wav"

class Window
{
	public:
		Window();
		~Window();
		bool Initialize(const string &name, int* width, int* height);
		void Swap();

		SDL_Window* getSDL_Window() const;

		// Sound files to be loaded with SDL
		static Mix_Chunk *bumperSound;
		static Mix_Chunk *flipperSound;
		static Mix_Music *bgMusicSound;
		static Mix_Chunk *launcherSound;


	private:
		SDL_Window* gWindow;
		SDL_GLContext gContext;
};

#endif /* WINDOW_H */
