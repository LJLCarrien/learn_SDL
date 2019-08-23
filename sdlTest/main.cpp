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

//#pragma comment(lib ,"SDL2.lib")
//#pragma comment(lib ,"SDL2main.lib")
//#pragma comment(lib ,"SDL2_image.lib")
//#pragma comment(lib ,"SDL2_ttf.lib")


const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;
const int TILE_SIZE = 40;

//Frees media and shuts down SDL
void close();

void logSDLError(std::ostream& os, const std::string& msg) {
	os << msg << "[SDL_GetError] error: " << SDL_GetError() << std::endl;
}

void logSDLError(const std::string& msg) {
	std::cout << msg << "[SDL_GetError] error: " << SDL_GetError() << std::endl;
}

void logIMGError(const std::string& msg) {
	std::cout << msg << "[IMG_GetError] error: " << IMG_GetError() << std::endl;
}

SDL_Texture* lazyFoo_loadTexture(std::string path, SDL_Renderer* ren)
{
	//The final texture
	SDL_Texture* newTexture = NULL;

	//Load image at specified path
	SDL_Surface* loadedSurface = IMG_Load(path.c_str());
	if (loadedSurface == NULL)
	{
		printf("Unable to load image %s! SDL_image Error: %s\n", path.c_str(), IMG_GetError());
	}
	else
	{
		//Create texture from surface pixels
		newTexture = SDL_CreateTextureFromSurface(ren, loadedSurface);
		if (newTexture == NULL)
		{
			printf("Unable to create texture from %s! SDL Error: %s\n", path.c_str(), SDL_GetError());
		}

		//Get rid of old loaded surface
		SDL_FreeSurface(loadedSurface);
	}

	return newTexture;
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

SDL_Window* gWindow = NULL;
SDL_Renderer* gRenderer = NULL;
//The surface contained by the window
SDL_Surface* gScreenSurface = NULL;


//Current displayed texture
SDL_Texture* gTexture = NULL;

SDL_Surface* loadSurface(std::string path)
{
	//The final optimized image
	SDL_Surface* optimizedSurface = NULL;

	//Load image at specified path
	SDL_Surface* loadedSurface = IMG_Load(path.c_str());
	if (loadedSurface == NULL)
	{
		//Unable to load image
		logIMGError("IMG_Load");
	}
	else
	{
		//Convert surface to screen format
		optimizedSurface = SDL_ConvertSurface(loadedSurface, gScreenSurface->format, 0);
		if (optimizedSurface == NULL)
		{
			logSDLError("SDL_ConvertSurface");
			//printf("Unable to optimize image %s! SDL Error: %s\n", path.c_str(), SDL_GetError());
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

	gTexture=lazyFoo_loadTexture("lession9_viewport.png", gRenderer);

	if (gTexture == NULL)
	{
		printf("Failed to load stretching image!\n");
		success = false;
	}
	return success;
}

SDL_Renderer* InitRender(SDL_Window* window) {
	bool success = true;

	SDL_Renderer* render = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	if (render == nullptr) {
		logSDLError("SDL_CreateRenderer");
	}
	return render;

}

bool init()
{
	//Initialization flag
	bool success = true;

	//Initialize SDL
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		logSDLError("SDL_Init");
		return false;
	}
	if (TTF_Init() != 0) {
		logSDLError(std::cout, "TTF_Init");
		return false;
	}
	//Create window
	gWindow = SDL_CreateWindow("SDL Tutorial", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
	if (gWindow == NULL)
	{
		logSDLError("SDL_CreateWindow");
		return false;
	}

	gRenderer = InitRender(gWindow);
	SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);

	//Initialize PNG loading
	int imgFlags = IMG_INIT_PNG;
	if (!(IMG_Init(imgFlags) & imgFlags))
	{
		logIMGError("IMG_Init");
		return false;
	}
	return success;
}

void DrawLession8()
{
	//Clear screen
	SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
	SDL_RenderClear(gRenderer);

	//Render red filled quad
	SDL_Rect fillRect = { SCREEN_WIDTH / 4, SCREEN_HEIGHT / 4, SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 };
	SDL_SetRenderDrawColor(gRenderer, 0xFF, 0x00, 0x00, 0xFF);
	SDL_RenderFillRect(gRenderer, &fillRect);

	//Render green outlined quad
	SDL_Rect outlineRect = { SCREEN_WIDTH / 6, SCREEN_HEIGHT / 6, SCREEN_WIDTH * 2 / 3, SCREEN_HEIGHT * 2 / 3 };
	SDL_SetRenderDrawColor(gRenderer, 0x00, 0xFF, 0x00, 0xFF);
	SDL_RenderDrawRect(gRenderer, &outlineRect);

	//Draw blue horizontal line
	SDL_SetRenderDrawColor(gRenderer, 0x00, 0x00, 0xFF, 0xFF);
	SDL_RenderDrawLine(gRenderer, 0, SCREEN_HEIGHT / 2, SCREEN_WIDTH, SCREEN_HEIGHT / 2);

	//Draw vertical line of yellow dots
	SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0x00, 0xFF);
	for (int i = 0; i < SCREEN_HEIGHT; i += 4)
	{
		SDL_RenderDrawPoint(gRenderer, SCREEN_WIDTH / 2, i);
	}


	//Update screen
	SDL_RenderPresent(gRenderer);
}

void DrawViewPort(SDL_Renderer* render,int x,int y ,int w,int h)
{
	SDL_Rect rect;
	rect.x = x;
	rect.y = y;
	rect.w = w;
	rect.h = h;
	SDL_RenderSetViewport(render, &rect);
}
void DrawLession9()
{
	//Top left corner viewport
	DrawViewPort(gRenderer,0,0, SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2);

	//Render texture to screen
	SDL_RenderCopy(gRenderer, gTexture, NULL, NULL);

	//Top right viewport
	DrawViewPort(gRenderer, SCREEN_WIDTH / 2, 0, SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2);

	//Render texture to screen
	SDL_RenderCopy(gRenderer, gTexture, NULL, NULL);

	//Bottom viewport
	SDL_Rect bottomViewport;
	bottomViewport.x = 0;
	bottomViewport.y = SCREEN_HEIGHT / 2;
	bottomViewport.w = SCREEN_WIDTH;
	bottomViewport.h = SCREEN_HEIGHT / 2;
	DrawViewPort(gRenderer,  0, SCREEN_HEIGHT / 2, SCREEN_WIDTH, SCREEN_HEIGHT / 2);

	//Render texture to screen
	SDL_RenderCopy(gRenderer, gTexture, NULL, NULL);


	//Update screen
	SDL_RenderPresent(gRenderer);
}

int main(int argc, char* argv[]) {

	bool quit = false;

	quit = !init();
	quit = !loadMedia();
	
	SDL_Event e;
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
		//DrawLession8();
		DrawLession9();

	}
	close();
	return 0;
}

void close()
{
	cleanup(gTexture,gWindow,gRenderer);
	gTexture = NULL;
	gWindow = NULL;
	gRenderer = NULL;

	//Quit SDL subsystems
	IMG_Quit();
	SDL_Quit();
}
