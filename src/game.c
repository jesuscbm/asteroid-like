#include "game.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <SDL3/SDL_init.h>
#include <SDL3/SDL_log.h>
#include <SDL3/SDL_stdinc.h>

#include "config.h"

void update_player_position(Player *player);

/**
 * Bullets HAVE TO GO before asteroids.
 */
void update_bullets_position(Game *game);
void update_asteroids_position(Game *game);
void handle_collisions(Game *game);

void create_asteroids(Game *game);

bool game_init(Game **game)
{
	Player *player;

	if (!game) {
		return false;
	}

	srand(time(NULL));

	/* Initialize the game */
	(*game) = malloc(sizeof(Game));
	if (!(*game)) {
		SDL_Log("Couldn't allocate memory: %s", SDL_GetError());
		return false;
	}

	(*game)->state = MENU;
	(*game)->player = NULL;
	(*game)->asteroids = malloc(MAX_ASTEROIDS * sizeof(Asteroid));
	if (!(*game)->asteroids) {
		SDL_Log("Couldn't allocate memory: %s", SDL_GetError());
		return false;
	}
	(*game)->bullets = malloc(MAX_BULLETS * sizeof(Bullet));
	if (!(*game)->bullets) {
		SDL_Log("Couldn't allocate memory: %s", SDL_GetError());
		return false;
	}
	(*game)->n_asteroids = 0;
	(*game)->n_bullets = 0;
	(*game)->level = 0;

	/* Setup for the player/ship */
	player = malloc(sizeof(Player));
	if (!player) {
		SDL_Log("Couldn't allocate memory: %s", SDL_GetError());
		return false;
	}

	player->x = (float)WIDTH / 2;
	player->y = (float)HEIGHT / 2;
	player->direction = 0;
	player->direction_state = STILL;
	player->velocity = 0;
	player->acceleration_state = CONSTANT;

	memcpy(player->indices, (int[]){ BOW, STARBOARD, AFT, BOW, PORT, AFT },
		   sizeof(player->indices));
	for (int i = BOW; i <= AFT; i++) {
		player->vertices[i].color = (SDL_FColor){ .r = 1.0f, .g = 1.0f, .b = 1.0f, .a = 1.0f };
	}
	(*game)->player = player;

	return true;
}

bool game_update_frame(Game *game)
{
	Player *player = game->player;

	if (!game || !player) {
		return false;
	}

	if (game->n_asteroids == 0) {
		game->level++;
		create_asteroids(game);
	}

	update_player_position(player);
	update_bullets_position(game);
	update_asteroids_position(game);
	handle_collisions(game);

	return true;
}

bool game_shoot(Game *game)
{
	Player *player = game->player;

	if (game->n_bullets >= MAX_BULLETS || game->bullets == NULL) {
		return false;
	}

	game->bullets[game->n_bullets++]
	  = (Bullet){ .dx = SDL_sin(player->direction * SDL_PI_D / 180.0) * BULLET_VELOCITY,
				  .dy = -SDL_cos(player->direction * SDL_PI_D / 180.0) * BULLET_VELOCITY,
				  .direction = player->direction,
				  .x = player->x,
				  .y = player->y };

	return true;
}

void game_reset(Game *game)
{
	game->n_asteroids = 0;
	game->n_bullets = 0;
	game->level = 0;
	game->player->x = (float)WIDTH / 2;
	game->player->y = (float)HEIGHT / 2;
	game->player->direction = 0;
	game->player->direction_state = STILL;
	game->player->velocity = 0;
	game->player->acceleration_state = CONSTANT;
}

void game_free(Game *game)
{
	if (!game)
		return;
	if (game->asteroids)
		free(game->asteroids);
	if (game->bullets)
		free(game->bullets);
	if (game->player)
		free(game->player);
	free(game);
}

void update_player_position(Player *player)
{
	if (player->direction_state == CLOCKWISE) {
		player->direction = (player->direction + ROTATION_SPEED) % 360;
	} else if (player->direction_state == COUNTER_CLOCKWISE) {
		player->direction = (player->direction + 360 - ROTATION_SPEED) % 360;
	}
	if (player->acceleration_state == ACCELERATING && player->velocity < MAX_SPEED) {
		player->velocity += SPEED_ACCEL;
		if (player->velocity > MAX_SPEED) {
			player->velocity = MAX_SPEED;
		}
	} else if (player->acceleration_state == DECELERATING && player->velocity > MIN_SPEED) {
		player->velocity -= SPEED_ACCEL;
		/* Due to floating point, when MIN_SPEED is 0, we can get a bit of backwards movement */
		if (player->velocity < MIN_SPEED) {
			player->velocity = MIN_SPEED;
		}
	}
	float x_change = SDL_sin(player->direction * SDL_PI_D / 180.0) * player->velocity;
	float y_change = -SDL_cos(player->direction * SDL_PI_D / 180.0) * player->velocity;
	if (player->x + x_change >= 0 && player->x + x_change <= WIDTH) {
		player->x += x_change;
	}
	if (player->y + y_change >= 0 && player->y + y_change <= HEIGHT) {
		player->y += y_change;
	}
}

void update_bullets_position(Game *game)
{
	for (int i = 0; i < game->n_bullets; i++) {
		Bullet *bullet = &game->bullets[i];
		bullet->x += bullet->dx;
		bullet->y += bullet->dy;
		if (bullet->x >= WIDTH || bullet->x <= 0 || bullet->y >= HEIGHT || bullet->y <= 0) {
			game->bullets[i] = game->bullets[--game->n_bullets];
			i--;
		}
	}
}

void update_asteroids_position(Game *game)
{
	for (int i = 0; i < game->n_asteroids; i++) {
		Asteroid *asteroid = &game->asteroids[i];
		asteroid->x += asteroid->dx;
		asteroid->y += asteroid->dy;
		if (asteroid->x >= WIDTH - asteroid->radius || asteroid->x <= asteroid->radius)
			asteroid->dx = -asteroid->dx;
		if (asteroid->y >= HEIGHT - asteroid->radius || asteroid->y <= asteroid->radius)
			asteroid->dy = -asteroid->dy;
	}
}

void handle_collisions(Game *game)
{
	/* Colissions. TODO: Consider dividing the list in bins for better performance
	 * Consider quad trees
	 * Damn collisions are hard
	 */
	for (int i = 0; i < game->n_asteroids; i++) {
		Asteroid *asteroid = &game->asteroids[i];
		float dx = game->player->x - asteroid->x;
		float dy = game->player->y - asteroid->y;
		float dist_sq = dx * dx + dy * dy;
		float radius_sum = asteroid->radius + SHIP_RADIUS * 0.80f;
		if (dist_sq <= radius_sum * radius_sum) {
			game->state = GAME_OVER;
			return;
		}
		for (int j = 0; j < game->n_bullets; j++) {
			Bullet *bullet = &game->bullets[j];
			float dx = bullet->x - asteroid->x;
			float dy = bullet->y - asteroid->y;
			float dist_sq = dx * dx + dy * dy;
			float radius_sum = asteroid->radius + BULLET_RADIUS;
			if (dist_sq <= radius_sum * radius_sum) {
				game->asteroids[i] = game->asteroids[--game->n_asteroids];
				game->bullets[j] = game->bullets[--game->n_bullets];
				i--;
				j--;
			}
		}
		for (int j = 0; j < game->n_asteroids; j++) {
			if (i == j) {
				continue;
			}
			Asteroid *other = &game->asteroids[j];
			float dx = other->x - asteroid->x;
			float dy = other->y - asteroid->y;
			float dist_sq = dx * dx + dy * dy;
			float radius_sum = asteroid->radius + other->radius;
			if (dist_sq <= radius_sum * radius_sum) {
				Asteroid *smaller = (other->radius < asteroid->radius) ? other : asteroid;
				smaller->dx = -smaller->dx;
                smaller->dy = -smaller->dy;
				smaller->x += smaller->dx;
				smaller->y += smaller->dy;
			}
		}
	}
}

void create_asteroids(Game *game)
{
	int n_asteroids = (game->level >= MAX_ASTEROIDS) ? MAX_ASTEROIDS : (game->level);
	for (int i = 0; i < n_asteroids; i++) {
		Asteroid *asteroid = &game->asteroids[game->n_asteroids++];
		asteroid->radius
		  = rand() % (ASTEROID_RADIUS_MAX - ASTEROID_RADIUS_MIN + 1) + ASTEROID_RADIUS_MIN;
		enum { TOP, RIGHT, BOTTOM, LEFT };
#define GRACE_SPACING 5
		int side = rand() % 4;
		switch (side) {
		case TOP:
			asteroid->x = rand() % (int)(WIDTH - 2 * asteroid->radius) + asteroid->radius;
			asteroid->y = asteroid->radius + GRACE_SPACING;
			break;
		case RIGHT:
			asteroid->x = WIDTH - asteroid->radius - GRACE_SPACING;
			asteroid->y = rand() % (int)(HEIGHT - 2 * asteroid->radius) + asteroid->radius;
			break;
		case BOTTOM:
			asteroid->x = rand() % (int)(WIDTH - 2 * asteroid->radius) + asteroid->radius;
			asteroid->y = HEIGHT - asteroid->radius - GRACE_SPACING;
			break;
		case LEFT:
			asteroid->x = asteroid->radius + GRACE_SPACING;
			asteroid->y = rand() % (int)(HEIGHT - 2 * asteroid->radius) + asteroid->radius;
			break;
		}
		asteroid->dx = rand() % (ASTEROID_SPEED_MAX - ASTEROID_SPEED_MIN + 1) + ASTEROID_SPEED_MIN;
		asteroid->dy = rand() % (ASTEROID_SPEED_MAX - ASTEROID_SPEED_MIN + 1) + ASTEROID_SPEED_MIN;
	}
}
