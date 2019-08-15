// Example program:
// Using SDL2 to create an application window

#ifndef CLEANUP_H
#define CLEANUP_H

#include <utility>
#include <SDL.h>

/*
 * Recurse through the list of arguments to clean up, cleaning up
 * the first one in the list each iteration.
 */
template<typename T, typename... Args>
void cleanup(T* t, Args&& ... args) {
	//Cleanup the first item in the list
	cleanup(t);
	//Recurse to clean up the remaining arguments
	cleanup(std::forward<Args>(args)...);
}
/*
 * These specializations serve to free the passed argument and also provide the
 * base cases for the recursive call above, eg. when args is only a single item
 * one of the specializations below will be called by
 * cleanup(std::forward<Args>(args)...), ending the recursion
 * We also make it safe to pass nullptrs to handle situations where we
 * don't want to bother finding out which values failed to load (and thus are null)
 * but rather just want to clean everything up and let cleanup sort it out
 */
template<>
inline void cleanup<SDL_Window>(SDL_Window* win) {
	if (!win) {
		return;
	}
	SDL_DestroyWindow(win);
}
template<>
inline void cleanup<SDL_Renderer>(SDL_Renderer* ren) {
	if (!ren) {
		return;
	}
	SDL_DestroyRenderer(ren);
}
template<>
inline void cleanup<SDL_Texture>(SDL_Texture* tex) {
	if (!tex) {
		return;
	}
	SDL_DestroyTexture(tex);
}
template<>
inline void cleanup<SDL_Surface>(SDL_Surface* surf) {
	if (!surf) {
		return;
	}
	SDL_FreeSurface(surf);
}

#endif

#include "SDL.h"
#include <stdio.h>
#include <iostream>
#include "SDL_image.h"

//#pragma comment(lib ,"sdl2.lib")
//#pragma comment(lib ,"SDL2main.lib")
#pragma comment(lib ,"SDL2_image.lib")


const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;
const int TILE_SIZE = 40;

void logSDLError(std::ostream& os, const std::string& msg) {
	os << msg << " error: " << SDL_GetError() << std::endl;
}

void logSDLError(const std::string& msg) {
	std::cout << msg << " error: " << SDL_GetError() << std::endl;
}


SDL_Texture* loadTexture(const std::string& file, SDL_Renderer* ren) {
	SDL_Texture* texture = IMG_LoadTexture(ren, file.c_str());
	if (texture == nullptr) {
		logSDLError("loadTexture");
	}
	return texture;
}

void renderTexture(SDL_Texture* tex, SDL_Renderer* ren, int x, int y, int w, int h) {
	SDL_Rect dst;
	dst.x = x;
	dst.y = y;
	dst.w = w;
	dst.h = h;
	SDL_RenderCopy(ren, tex, NULL, &dst);
}
void renderTexture(SDL_Texture* tex, SDL_Renderer* ren, int x, int y) {
	int w, h;
	SDL_QueryTexture(tex, NULL, NULL, &w, &h);
	renderTexture(tex, ren, x, y, w, h);
}

int main(int argc, char* argv[]) {


	if (SDL_Init(SDL_INIT_VIDEO) != 0) {
		logSDLError("SDL_Init");
		return 1;
	}

	SDL_Window* win = SDL_CreateWindow("Hello World!", 100, 100, 640, 480, SDL_WINDOW_SHOWN);
	if (win == nullptr) {
		logSDLError("SDL_CreateWindow");
		SDL_Quit();
		return 1;
	}

	SDL_Renderer* ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	if (ren == nullptr) {
		SDL_DestroyWindow(win);
		logSDLError("SDL_CreateRenderer");
		SDL_Quit();
		return 1;
	}

	SDL_Texture* image = loadTexture("image.png", ren);
	if (image == nullptr) {
		cleanup(image, ren, win);
		IMG_Quit();
		SDL_Quit();
		return 1;
	}

	SDL_Event e;
	bool quit = false;
	int posX = 0;
	int posY = 0;
	while (!quit) {
		while (SDL_PollEvent(&e))
		{
			if (e.type == SDL_QUIT)
			{
				quit = true;
			}
			if (e.type == SDL_KEYDOWN)
			{
				switch (e.key.keysym.sym)
				{
					case SDLK_LEFT:
						posX--;
						break;
					case SDLK_RIGHT:
						posX++;
						break;
					case SDLK_UP:
						posY--;
						break;
					case SDLK_DOWN:
						posY++;
						break;
					default:
						break;
				}
				//quit = true;
			}
			if (e.type == SDL_MOUSEBUTTONDOWN)
			{
				posX = 0;
				posY = 0;
				//quit = true;
			}
		}
		SDL_RenderClear(ren);
		renderTexture(image, ren, posX, posY);
		SDL_RenderPresent(ren);
	}

	cleanup(image, ren, win);
	IMG_Quit();
	SDL_Quit();
	return 0;
}
