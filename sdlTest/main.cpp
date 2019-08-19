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
#include "SDL_ttf.h"

//#pragma comment(lib ,"sdl2.lib")
//#pragma comment(lib ,"SDL2main.lib")
#pragma comment(lib ,"SDL2_image.lib")
#pragma comment(lib ,"SDL2_ttf.lib")


const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;
const int TILE_SIZE = 40;

//Frees media and shuts down SDL
void close();

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
void renderTexture(SDL_Texture* tex, SDL_Renderer* ren, SDL_Rect dst, SDL_Rect* clip = nullptr)
{
	SDL_RenderCopy(ren, tex, clip, &dst);
}
void renderTexture(SDL_Texture* tex, SDL_Renderer* ren, int x, int y, SDL_Rect* clip = nullptr)
{
	SDL_Rect dst;
	dst.x = x;
	dst.y = y;
	if (clip != nullptr) {
		dst.w = clip->w;
		dst.h = clip->h;
	}
	else {
		SDL_QueryTexture(tex, NULL, NULL, &dst.w, &dst.h);
	}
	SDL_RenderCopy(ren, tex, clip, &dst);
}
void renderTexture(SDL_Texture* tex, SDL_Renderer* ren, int x, int y, int w, int h) {
	SDL_Rect dst;
	dst.x = x;
	dst.y = y;
	dst.w = w;
	dst.h = h;
	SDL_RenderCopy(ren, tex, NULL, &dst);
}

SDL_Texture* renderText(const std::string& message, const std::string& fontFile,
	SDL_Color color, int fontSize, SDL_Renderer* renderer)
{
	TTF_Font* font = TTF_OpenFont(fontFile.c_str(), fontSize);
	if (font == nullptr) {
		logSDLError(std::cout, "TTF_OpenFont");
		return nullptr;
	}
	SDL_Surface* surf = TTF_RenderText_Blended(font, message.c_str(), color);
	if (surf == nullptr) {
		TTF_CloseFont(font);
		logSDLError(std::cout, "TTF_RenderText");
		return nullptr;
	}
	SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surf);
	if (texture == nullptr) {
		logSDLError(std::cout, "CreateTexture");
	}
	SDL_FreeSurface(surf);
	TTF_CloseFont(font);
	return texture;
}

SDL_Window* win = NULL;

//The surface contained by the window
SDL_Surface* gScreenSurface = NULL;

//Current displayed image
SDL_Surface* gStretchedSurface = NULL;

SDL_Surface* loadSurface(std::string path)
{
	//The final optimized image
	SDL_Surface* optimizedSurface = NULL;

	//Load image at specified path
	SDL_Surface* loadedSurface = SDL_LoadBMP(path.c_str());
	if (loadedSurface == NULL)
	{
		printf("Unable to load image %s! SDL Error: %s\n", path.c_str(), SDL_GetError());
	}
	else
	{
		//Convert surface to screen format
		optimizedSurface = SDL_ConvertSurface(loadedSurface, gScreenSurface->format, 0);
		if (optimizedSurface == NULL)
		{
			printf("Unable to optimize image %s! SDL Error: %s\n", path.c_str(), SDL_GetError());
		}

		//Get rid of old loaded surface
		SDL_FreeSurface(loadedSurface);
	}

	return optimizedSurface;
}

bool loadMedia()
{
	//Loading success flag
	bool success = true;

	//Load stretching surface
	gStretchedSurface = loadSurface("stretch.bmp");
	if (gStretchedSurface == NULL)
	{
		printf("Failed to load stretching image!\n");
		success = false;
	}

	return success;
}

bool InitRender(SDL_Window* window) {
	bool success = true;

	SDL_Renderer* ren = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	if (ren == nullptr) {
		SDL_DestroyWindow(window);
		logSDLError("SDL_CreateRenderer");
		SDL_Quit();
		success = false;
	}
	return success;

}

int main(int argc, char* argv[]) {


	if (SDL_Init(SDL_INIT_VIDEO) != 0) {
		logSDLError("SDL_Init");
		return 1;
	}
	if (TTF_Init() != 0) {
		logSDLError(std::cout, "TTF_Init");
		SDL_Quit();
		return 1;
	}
	win = SDL_CreateWindow("Hello World!", 100, 100, 640, 480, SDL_WINDOW_SHOWN);
	if (win == nullptr) {
		logSDLError("SDL_CreateWindow");
		SDL_Quit();
		return 1;
	}
	else {
		gScreenSurface = SDL_GetWindowSurface(win);
	}

	if (!loadMedia())
	{
		return 1;
	}


	SDL_Event e;
	bool quit = false;
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
				case SDLK_1:
					break;
				case SDLK_ESCAPE:
					quit = true;
					break;
				default:
					break;
				}
			}
			if (e.type == SDL_MOUSEBUTTONDOWN)
			{
				quit = true;
			}
		}


		SDL_Rect stretchRect;
		stretchRect.x = 0;
		stretchRect.y = 0;
		stretchRect.w = SCREEN_WIDTH;
		stretchRect.h = SCREEN_HEIGHT;
		SDL_BlitScaled(gStretchedSurface, NULL, gScreenSurface, &stretchRect);
		SDL_UpdateWindowSurface(win);

	}
	cleanup(win);
	close();
	return 0;
}

void close()
{
	//Free loaded image
	SDL_FreeSurface(gStretchedSurface);
	gStretchedSurface = NULL;

	win = NULL;

	//Quit SDL subsystems
	SDL_Quit();
}
