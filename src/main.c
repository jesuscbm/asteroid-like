/**
 * @file main.c
 * @brief
 * @author Jesús Blázquez
 */
#include <SDL3/SDL.h>
#include <stdio.h>
#include <time.h>

#include <SDL3/SDL_events.h>
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_keycode.h>
#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_surface.h>
#include <SDL3/SDL_video.h>
#include <SDL3_image/SDL_image.h>  // TODO: Use SDL3_image
#include <SDL3_ttf/SDL_ttf.h>

#define SDL_MAIN_USE_CALLBACKS 1 /* No need for main() */
#include <SDL3/SDL_main.h>

#include "config.h"
#include "game.h"

void update_player_vertices(Game* game);
void drawcircle(SDL_Renderer* renderer, int x0, int y0, int radius);

const int frameDelay = 1000 / FPS;	// ms per frame
Uint64 frame_start = 0;				// ms at start of frame
int frame_time = 0;					// ms elapsed this frame

/**
 * @brief Window for the application
 */
static SDL_Window* window;

/**
 * @brief Renderer for the application
 */
static SDL_Renderer* renderer;

/**
 * @brief Audio stream
 */
static SDL_AudioStream* stream = NULL;

// TODO: New file for font-related stuff?
static struct {
	enum { NOTHING, LEVEL, LOAD_MENU, LOAD_GAME_OVER } state;
	TTF_Font* font;
	SDL_Texture* texture1;
	SDL_Texture* texture2;
	SDL_FRect rect1, rect2;
} text_info = { NOTHING, NULL, NULL };

void showMenu(Game* game);
void showGameOver(Game* game);
void showScoreboard(Game* game);

/**
 * @brief Initializes the application. Called once
 */
SDL_AppResult SDL_AppInit(void** appstate, int argc, char* argv[])
{
	SDL_AudioSpec spec;
	Game* game;
	if (!SDL_SetAppMetadata("Asteroids Clone", "0.1", "org.asteroids")) {
		SDL_Log("Unable to set app metadata: %s\n", SDL_GetError());
		return SDL_APP_FAILURE;
	}
	if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO)) {
		SDL_Log("Unable to initialize SDL: %s\n", SDL_GetError());
		return SDL_APP_FAILURE;
	}
	SDL_CreateWindowAndRenderer("Asteroids Game", WIDTH, HEIGHT, 0, &window, &renderer);
	if (!window) {
		SDL_Log("Unable to create window: %s\n", SDL_GetError());
		return SDL_APP_FAILURE;
	}
	if (!renderer) {
		SDL_Log("Unable to create renderer: %s\n", SDL_GetError());
		return SDL_APP_FAILURE;
	}

	/* TTF */
	if (!TTF_Init()) {
		SDL_Log("Couldn't initialise SDL_ttf: %s\n", SDL_GetError());
		return SDL_APP_FAILURE;
	}

	text_info.font = TTF_OpenFont("./font/AzeretMono.ttf", 50);
	if (!text_info.font) {
		SDL_Log("Couldn't load font: %s\n", SDL_GetError());
		return SDL_APP_FAILURE;
	}

	/* Audio */
	spec.channels = 1;
	spec.format = SDL_AUDIO_F32;
	spec.freq = 8000;
	stream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec, NULL, NULL);
	if (!stream) {
		SDL_Log("Couldn't create audio stream: %s", SDL_GetError());
		return SDL_APP_FAILURE;
	}
	SDL_ResumeAudioStreamDevice(stream);

	if (!game_init(&game)) {
		SDL_Log("Couldn't initialize game");
		return SDL_APP_FAILURE;
	}

	update_player_vertices(game);
	*appstate = game;

	return SDL_APP_CONTINUE;
}

/**
 * @brief Handles events
 */
SDL_AppResult SDL_AppEvent(void* appstate, SDL_Event* event)
{
	Game* game = appstate;
	Player* player = game->player;
	if (event->type == SDL_EVENT_WINDOW_RESIZED) {
		SDL_GetWindowSize(window, &game->width, &game->height);
		SDL_SetRenderViewport(renderer, NULL);
		game_resize(game);
		return SDL_APP_CONTINUE;
	}

	if (event->type == SDL_EVENT_QUIT)
		return SDL_APP_SUCCESS;

	if (event->type == SDL_EVENT_KEY_DOWN) {
		if (game->state == MENU) {
			if (event->key.key == SDLK_RETURN || event->key.key == SDLK_Q)
				return SDL_APP_SUCCESS;
			game->state = PLAY;
			return SDL_APP_CONTINUE;
		}
		if (game->state == GAME_OVER) {
			if (event->key.key == SDLK_RETURN || event->key.key == SDLK_Q)
				return SDL_APP_SUCCESS;
			game->state = PLAY;
			game_reset(game);
			return SDL_APP_CONTINUE;
		}
		if (game->state == PAUSE) {
			if (event->key.key == SDLK_RETURN || event->key.key == SDLK_Q)
				return SDL_APP_SUCCESS;
			game->state = PLAY;
			return SDL_APP_CONTINUE;
		}
		switch (event->key.key) {
		case SDLK_LEFT:
			player->direction_state = COUNTER_CLOCKWISE;
			break;
		case SDLK_RIGHT:
			player->direction_state = CLOCKWISE;
			break;
		case SDLK_UP:
			player->acceleration_state = ACCELERATING;
			break;
		case SDLK_DOWN:
			player->acceleration_state = DECELERATING;
			break;
		case SDLK_SPACE:
			game_shoot(game);
			break;
		case SDLK_P:
			game->state = PAUSE;
			break;
		case SDLK_RETURN:
		case SDLK_Q:
			return SDL_APP_SUCCESS;
			break;
		default:
			break;
		}
	}

	if (event->type == SDL_EVENT_KEY_UP) {
		switch (event->key.key) {
		case SDLK_LEFT:
		case SDLK_RIGHT:
			player->direction_state = STILL;
			break;
		case SDLK_UP:
		case SDLK_DOWN:
			player->acceleration_state = CONSTANT;
			break;
		default:
			break;
		}
	}

	return SDL_APP_CONTINUE;
}

/**
 * Once every frame
 */
SDL_AppResult SDL_AppIterate(void* appstate)
{
	Game* game = appstate;
	Player* player = game->player;

	SDL_SetRenderDrawColorFloat(renderer, BG_COLOR, 1.0f);
	SDL_RenderClear(renderer);

	frame_start = SDL_GetTicks();

	SDL_SetRenderDrawColorFloat(renderer, LINE_COLOR, 1.0f);
	/* Menu */
	if (game->state == MENU) {
		showMenu(game);
		SDL_RenderPresent(renderer);
		return SDL_APP_CONTINUE;
	}

	if (game->state == GAME_OVER) {
		showGameOver(game);
		SDL_RenderPresent(renderer);
		return SDL_APP_CONTINUE;
	}

	/* Pause menu */
	if (game->state == PAUSE) {
		/* Paint everything gray */
		SDL_SetRenderDrawColorFloat(renderer, 0.5f, 0.5f, 0.5f, 1.0f);
		for (int i = BOW; i <= AFT; i++) {
			player->vertices[i].color = (SDL_FColor){ 0.5f, 0.5f, 0.5f, 1.0f };
		}
	}

	if (game->state == PLAY) {
		game_update_frame(game);
		update_player_vertices(game);
	}

	showScoreboard(game);

	/* Draw player */
	if (!SDL_RenderGeometry(renderer, NULL, player->vertices, 4, player->indices, 6)) {
		SDL_Log("Couldn't render geometry: %s", SDL_GetError());
		return SDL_APP_FAILURE;
	}

	/* Draw bullets */
	for (int i = 0; i < game->n_bullets; i++) {
		Bullet* bullet = &game->bullets[i];
		drawcircle(renderer, bullet->x, bullet->y, BULLET_RADIUS);
	}

	/* Draw asteroids */
	for (int i = 0; i < game->n_asteroids; i++) {
		Asteroid* asteroid = &game->asteroids[i];
		drawcircle(renderer, asteroid->x, asteroid->y, asteroid->radius);
	}

	SDL_RenderPresent(renderer);

	/* Frame cap */
	frame_time = SDL_GetTicks() - frame_start;
	if (frame_time < frameDelay) {
		SDL_Delay(frameDelay - frame_time);
	}

	return SDL_APP_CONTINUE;
}

/**
 * Called once at the end
 */
void SDL_AppQuit(void* appstate, SDL_AppResult result)
{
	Game* game = appstate;
	if (game)
		game_free(game);
	TTF_Quit();
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
}

void update_player_vertices(Game* game)
{
	Player* player = game->player;
	/*
	 * Imagine the shape as an inscribed isosceles triangle in a circle. The direction is the angle
	 * between the vertical and the radius that goes through the acutest corner (aft)
	 */
	int direction = player->direction % 360;
	float dir_rad = direction * SDL_PI_D / 180.0;

	float sin_dir = SDL_sin(dir_rad);
	float cos_dir = SDL_cos(dir_rad);

	// Using the law of the sine and cosine of a+-b, since we know b is a constant, we can
	// precompute it
#define sin_135 0.7071067
#define cos_135 -0.7071067

	player->vertices[AFT].position.x = player->x;
	player->vertices[AFT].position.y = player->y;
	player->vertices[AFT].color = (SDL_FColor){ 255, 255, 255, 255 };

	player->vertices[BOW].position.x = player->x + sin_dir * SHIP_RADIUS;
	player->vertices[BOW].position.y = player->y - cos_dir * SHIP_RADIUS;
	player->vertices[BOW].color = (SDL_FColor){ 255, 255, 255, 255 };

	player->vertices[PORT].position.x
	  = player->x + (sin_dir * cos_135 - cos_dir * sin_135) * SHIP_RADIUS;
	player->vertices[PORT].position.y
	  = player->y - (cos_dir * cos_135 + sin_dir * sin_135) * SHIP_RADIUS;
	player->vertices[PORT].color = (SDL_FColor){ 255, 255, 255, 255 };

	player->vertices[STARBOARD].position.x
	  = player->x + (sin_dir * cos_135 + cos_dir * sin_135) * SHIP_RADIUS;
	player->vertices[STARBOARD].position.y
	  = player->y - (cos_dir * cos_135 - sin_dir * sin_135) * SHIP_RADIUS;
	player->vertices[STARBOARD].color = (SDL_FColor){ 255, 255, 255, 255 };
}

/**
 * Midpoint circle algorithm
 */
void drawcircle(SDL_Renderer* renderer, int x0, int y0, int radius)
{
	int x = radius - 1;
	int y = 0;
	int dx = 1;
	int dy = 1;
	int err = dx - (radius << 1);

	while (x >= y) {
		SDL_RenderPoint(renderer, x0 + x, y0 + y);
		SDL_RenderPoint(renderer, x0 + y, y0 + x);
		SDL_RenderPoint(renderer, x0 - y, y0 + x);
		SDL_RenderPoint(renderer, x0 - x, y0 + y);
		SDL_RenderPoint(renderer, x0 - x, y0 - y);
		SDL_RenderPoint(renderer, x0 - y, y0 - x);
		SDL_RenderPoint(renderer, x0 + y, y0 - x);
		SDL_RenderPoint(renderer, x0 + x, y0 - y);

		if (err <= 0) {
			y++;
			err += dy;
			dy += 2;
		}

		if (err > 0) {
			x--;
			dx += 2;
			err += dx - (radius << 1);
		}
	}
}

/**
 *
 */
void showMenu(Game* game)
{
	if (!game)
		return;

	if (text_info.texture1 && text_info.state == LOAD_MENU) {
		SDL_RenderTexture(renderer, text_info.texture1, NULL, &text_info.rect1);
		SDL_RenderTexture(renderer, text_info.texture2, NULL, &text_info.rect2);
		return;
	}

	if (text_info.texture1) {
		SDL_DestroyTexture(text_info.texture1);
		SDL_DestroyTexture(text_info.texture2);
		text_info.texture1 = NULL;
		text_info.texture2 = NULL;
	}

	text_info.state = LOAD_MENU;

	SDL_Surface* text
	  = TTF_RenderText_Solid(text_info.font, "Asteroids", 9, (SDL_Color){ 255, 255, 255, 255 });
	if (!text)
		return;

	text_info.texture1 = SDL_CreateTextureFromSurface(renderer, text);
	text_info.rect1.x = ((float)game->width - text->w) / 2;
	text_info.rect1.y = (float)game->height / 3;
	text_info.rect1.w = text->w;
	text_info.rect1.h = text->h;
	SDL_DestroySurface(text);

	text = TTF_RenderText_Solid(text_info.font, "Click any key to start", 23,
								(SDL_Color){ 255, 255, 255, 255 });
	if (!text)
		return;

	text_info.texture2 = SDL_CreateTextureFromSurface(renderer, text);
	text_info.rect2.x = ((float)game->width - (float)text->w / 2) / 2;
	text_info.rect2.y = (float)game->height / 2;
	text_info.rect2.w = (float)text->w / 2;
	text_info.rect2.h = (float)text->h / 2;
	SDL_DestroySurface(text);

	SDL_RenderTexture(renderer, text_info.texture1, NULL, &text_info.rect1);
	SDL_RenderTexture(renderer, text_info.texture2, NULL, &text_info.rect2);
}

/**
 *
 */
void showGameOver(Game* game)
{
	if (!game)
		return;

	if (text_info.texture1 && text_info.state == LOAD_GAME_OVER) {
		SDL_RenderTexture(renderer, text_info.texture1, NULL, &text_info.rect1);
		SDL_RenderTexture(renderer, text_info.texture2, NULL, &text_info.rect2);
		return;
	}

	if (text_info.texture1) {
		SDL_DestroyTexture(text_info.texture1);
		SDL_DestroyTexture(text_info.texture2);
		text_info.texture1 = NULL;
		text_info.texture2 = NULL;
	}

	text_info.state = LOAD_GAME_OVER;

	SDL_Surface* text
	  = TTF_RenderText_Solid(text_info.font, "Game Over", 9, (SDL_Color){ 255, 255, 255, 255 });
	if (!text)
		return;

	text_info.texture1 = SDL_CreateTextureFromSurface(renderer, text);
	text_info.rect1.x = ((float)game->width - text->w) / 2;
	text_info.rect1.y = (float)game->height / 3;
	text_info.rect1.w = text->w;
	text_info.rect1.h = text->h;
	SDL_DestroySurface(text);

	text = TTF_RenderText_Solid(text_info.font, "Click any key to restart", 25,
								(SDL_Color){ 255, 255, 255, 255 });
	if (!text)
		return;

	text_info.texture2 = SDL_CreateTextureFromSurface(renderer, text);
	text_info.rect2.x = ((float)game->width - (float)text->w / 2) / 2;
	text_info.rect2.y = (float)game->height / 2;
	text_info.rect2.w = text->w >> 1;
	text_info.rect2.h = text->h >> 1;
	SDL_DestroySurface(text);

	SDL_RenderTexture(renderer, text_info.texture1, NULL, &text_info.rect1);
	SDL_RenderTexture(renderer, text_info.texture2, NULL, &text_info.rect2);
}

void showScoreboard(Game* game)
{
	char level[11];
	SDL_Color color;
	
	if (!game)
		return;

	if (text_info.texture1 && text_info.state == LEVEL) {
		SDL_DestroyTexture(text_info.texture1);
		SDL_DestroyTexture(text_info.texture2);
		text_info.texture1 = NULL;
		text_info.texture2 = NULL;
	}

	text_info.state = LEVEL;

	sprintf(level, "Level: %d", game->level);

	color = (game->state == PAUSE) ? (SDL_Color){ 177, 177, 177, 255 } : (SDL_Color){ 255, 255, 255, 255 };
	SDL_Surface* text = TTF_RenderText_Solid(text_info.font, level, strlen(level), color);
											 
	if (!text)
		return;

	text_info.texture1 = SDL_CreateTextureFromSurface(renderer, text);
	text_info.rect1.x = 10;
	text_info.rect1.y = 10;
	text_info.rect1.w = text->w >> 2;
	text_info.rect1.h = text->h >> 2;
	SDL_DestroySurface(text);

	SDL_RenderTexture(renderer, text_info.texture1, NULL, &text_info.rect1);
}
