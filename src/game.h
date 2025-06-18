#ifndef GAME_H
#define GAME_H

#include <stdbool.h>

#include <SDL3/SDL_render.h>

#include "config.h"

/**
 * How the direction of the player is changing.
 */
typedef enum { STILL, CLOCKWISE, COUNTER_CLOCKWISE } DirectionState;
/**
 * How the acceleration of the player is changing.
 */
typedef enum { CONSTANT, DECELERATING, ACCELERATING } AccelerationState;

/**
 * States of the game.
 */
typedef enum { MENU, PLAY, PAUSE, GAME_OVER } GameState;

/**
 * Used to distinguish the ship's vertices when rendering it.
 */
enum { BOW = 0, STARBOARD, PORT, AFT };

/**
 * Represents the player.
 */
typedef struct Ship {
	unsigned int direction;				  /**< Direction of the ship as a 0-360 value */
	DirectionState direction_state;		  /**< How the ship is turning */
	float x;							  /**< X position of the ship */
	float y;							  /**< Y position of the ship */
	float velocity;						  /**< Velocity of the ship */
	AccelerationState acceleration_state; /**< How the ship is accelerating */
	SDL_Vertex vertices[4];				  /**< Vertices of the ship, to render it */
	int indices[6];						  /**< Indices of the vertices of the ship to render in */
} Player;

/**
 * Represents an asteroid
 */
typedef struct Asteroid {
	float x;	  /**< X position of the asteroid */
	float y;	  /**< Y position of the asteroid */
	float radius; /**< Radius of the asteroid */
	float dx;	  /**< X velocity of the asteroid */
	float dy;	  /**< Y velocity of the asteroid */
} Asteroid;

/**
 * Represents a bullet
 */
typedef struct Bullet {
	float x;  /**< X position of the bullet */
	float y;  /**< Y position of the bullet */
	float dx; /**< X velocity of the bullet */
	float dy; /**< Y velocity of the bullet */
} Bullet;

/**
 * Stores all the information the game needs to emulate.
 */
typedef struct {
	int width;			 /**< Width of the window */
	int height;			 /**< Height of the window */
	Player* player;		 /**< Player of the game */
	Asteroid* asteroids; /**< List with all the asteroids of the game */
	int n_asteroids;	 /**< Number of asteroids in the game */
	Bullet* bullets;	 /**< List with all the bullets of the game */
	int n_bullets;		 /**< Number of bullets in the game */
	unsigned int level;	 /**< Current level */
	GameState state;	 /**< State of the game (menu, play, pause, game over) */
} Game;

/**
 * Creates a game, initializes its values and stores it in a pointer.
 *
 * @param game Pointer to the game we want to create
 * @return True if the game was created successfully, false otherwise
 */
bool game_init(Game** game);

/**
 * Updates the state of the game by one frame.
 *
 * @param game Pointer to the game we want to update
 * @return True if the game was updated successfully, false otherwise
 */
bool game_update_frame(Game* game);

/**
 * Makes the player shoot a bullet.
 *
 * @param game Pointer to the game we want to update
 * @return True if the bullet was shot successfully, false otherwise
 */
bool game_shoot(Game* game);

/**
 * Resizes the game window and updates position to deal with it.
 *
 * @param game Pointer to the game we want to update
 */
void game_resize(Game* game);

/**
 * Frees the memory used by the game.
 *
 * @param game Pointer to the game we want to free
 */
void game_free(Game* game);

/**
 * Resets the game to its initial state.
 *
 * @param game Pointer to the game we want to reset
 */
void game_reset(Game* game);

/* No we are not doing getters and setters */

#endif	// !GAME_H
