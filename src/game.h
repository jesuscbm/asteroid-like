#ifndef GAME_H
#define GAME_H

#include <stdbool.h>

#include <SDL3/SDL_render.h>

#include "config.h"

typedef enum { STILL, CLOCKWISE, COUNTER_CLOCKWISE } DirectionState;
typedef enum { CONSTANT, DECELERATING, ACCELERATING } AccelerationState;
typedef enum { MENU, PLAY, PAUSE, GAME_OVER } GameState;
enum { BOW = 0, STARBOARD, PORT, AFT };

typedef struct Ship {
	unsigned int direction;	 // As a number from 0 to 360
	DirectionState direction_state;
	float x, y;
	float velocity;
	AccelerationState acceleration_state;
	SDL_Vertex vertices[4];
	int indices[6];
} Player;

typedef struct Asteroid {
	float x, y;
	float radius;
	float dx, dy;
	float velocity;
} Asteroid;

typedef struct Bullet {
	float x, y;
	float dx, dy;
	float direction;
} Bullet;

typedef struct {
	int width, height;
	Player* player;
	Asteroid* asteroids;
	int n_asteroids;
	Bullet* bullets;
	int n_bullets;
	unsigned int level;
	GameState state;
} Game;

bool game_init(Game** game);

/**
 * Updates the state of the game by one frame.
 *
 * @param game Pointer to the game we want to update
 */
bool game_update_frame(Game* game);

/**
 * Makes the player shoot a bullet.
 */
bool game_shoot(Game* game);

void game_resize(Game* game);

void game_free(Game* game);

void game_reset(Game* game);

/* No we are not doing getters and setters */

#endif	// !GAME_H
