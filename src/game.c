#include "game.h"
#include <stdlib.h>
#include <time.h>

#include <SDL3/SDL_init.h>
#include <SDL3/SDL_log.h>
#include <SDL3/SDL_stdinc.h>

#include "config.h"

#define GRACE_SPACING 5

/**
 * Updates the position of the player in the game.
 *
 * @param game The game to update
 */
void update_player_position(Game *game);

/**
 * Updates the position of the bullets in the game.
 *
 * Bullets HAVE TO GO before asteroids.
 *
 * @param game The game to update
 */
void update_bullets_position(Game *game);
/**
 * Updates the position of the asteroids in the game.
 * 
 * @param game The game to update
 */
void update_asteroids_position(Game *game);
/**
 * Handles player-asteroid, bullet-asteroid, and asteroid-asteroid collisions. In that order
 * 
 * @param game The game to update
 */ 
void handle_collisions(Game *game);

/**
 * Creates the asteroids corresponding to the current level
 * 
 * @param game The game to update
 */
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

	(*game)->width = WIDTH;
	(*game)->height = HEIGHT;
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

	update_player_position(game);
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
				  .x = player->x,
				  .y = player->y };

	return true;
}

void game_resize(Game *game)
{
	// We have to make sure this resize doesn't cause problems
	Player *player = game->player;
	if (player->x >= game->width - SHIP_RADIUS) {
		player->x = game->width - SHIP_RADIUS - GRACE_SPACING;
	}
	if (player->x <= SHIP_RADIUS) {
		player->x = SHIP_RADIUS + GRACE_SPACING;
	}
	if (player->y >= game->height - SHIP_RADIUS) {
		player->y = game->height - SHIP_RADIUS - GRACE_SPACING;
	}
	if (player->y <= SHIP_RADIUS) {
		player->y = SHIP_RADIUS + GRACE_SPACING;
	}
	for (int i = 0; i < game->n_asteroids; i++) {
		Asteroid *asteroid = &game->asteroids[i];
		if (asteroid->x >= game->width - asteroid->radius) {
			asteroid->x = game->width - asteroid->radius - GRACE_SPACING;
		}
		if (asteroid->x <= asteroid->radius) {
			asteroid->x = asteroid->radius + GRACE_SPACING;
		}
		if (asteroid->y >= game->height - asteroid->radius) {
			asteroid->y = game->height - asteroid->radius - GRACE_SPACING;
		}
		if (asteroid->y <= asteroid->radius) {
			asteroid->y = asteroid->radius + GRACE_SPACING;
		}
	}
}

void game_reset(Game *game)
{
	game->n_asteroids = 0;
	game->n_bullets = 0;
	game->level = 0;
	game->player->x = (float)game->width / 2;
	game->player->y = (float)game->height / 2;
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

void update_player_position(Game *game)
{
	Player *player = game->player;
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
	if (player->x + x_change >= 0 && player->x + x_change <= game->width) {
		player->x += x_change;
	}
	if (player->y + y_change >= 0 && player->y + y_change <= game->height) {
		player->y += y_change;
	}
}

void update_bullets_position(Game *game)
{
	for (int i = 0; i < game->n_bullets; i++) {
		Bullet *bullet = &game->bullets[i];
		bullet->x += bullet->dx;
		bullet->y += bullet->dy;
		if (bullet->x >= game->width || bullet->x <= 0 || bullet->y >= game->height
			|| bullet->y <= 0) {
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
		if (asteroid->x >= game->width - asteroid->radius) {
			asteroid->dx = -asteroid->dx;
			asteroid->x = game->width - asteroid->radius - GRACE_SPACING;
		}
		if (asteroid->x <= asteroid->radius) {
			asteroid->dx = -asteroid->dx;
			asteroid->x = asteroid->radius + GRACE_SPACING;
		}
		if (asteroid->y >= game->height - asteroid->radius) {
			asteroid->dy = -asteroid->dy;
			asteroid->y = game->height - asteroid->radius - GRACE_SPACING;
		}
		if (asteroid->y <= asteroid->radius) {
			asteroid->dy = -asteroid->dy;
			asteroid->y = asteroid->radius + GRACE_SPACING;
		}
	}
}

void handle_collisions(Game *game)
{
	// Asteroid-Player and Bullet-Asteroid collisions
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
				game->bullets[j] = game->bullets[--game->n_bullets];
				j--;

				if (asteroid->radius < ASTEROID_SPLIT_THRESHOLD) {
					game->asteroids[i--] = game->asteroids[--game->n_asteroids];
					continue;
				}
				float vx = asteroid->dx;
				float vy = asteroid->dy;
				float module = SDL_sqrtf(vx * vx + vy * vy);
				float nx = vx / module;
				float ny = vy / module;

#define sqrt2 1.41421356237f

				game->asteroids[game->n_asteroids++] = game->asteroids[i + 1];
				game->asteroids[i + 1]
				  = (Asteroid){ .radius = asteroid->radius / sqrt2,
								.x = asteroid->x + ny * asteroid->radius / sqrt2,
								.y = asteroid->y - nx * asteroid->radius / sqrt2,
								.dx = asteroid->dx + ny,
								.dy = asteroid->dy - nx };
				game->asteroids[i] = (Asteroid){ .radius = asteroid->radius / sqrt2,
												 .x = asteroid->x - ny * asteroid->radius,
												 .y = asteroid->y + nx * asteroid->radius,
												 .dx = asteroid->dx - ny,
												 .dy = asteroid->dy + nx };

				i++;
			}
		}
	}

	// Asteroid-Asteroid collisions
	for (int i = 0; i < game->n_asteroids; i++) {
		Asteroid *a1 = &game->asteroids[i];
		for (int j = i + 1; j < game->n_asteroids; j++) {
			Asteroid *a2 = &game->asteroids[j];

			float dx = a2->x - a1->x;  // (dx, dy) is the collision vector
			float dy = a2->y - a1->y;
			float dist_sq = dx * dx + dy * dy;
			float radius_sum = a1->radius + a2->radius;

			if (dist_sq < radius_sum * radius_sum) {
				// collision <=> module of difference less than sum of radii
				float dist = SDL_sqrtf(dist_sq);
				if (dist == 0.0f)
					continue;  // avoid division by zero

				// Normalize it
				float nx = dx / dist;
				float ny = dy / dist;

				// Push them apart in the opposite direction that they are colliding in
				float overlap = 0.6f * (radius_sum - dist + 1.0f);
				a1->x -= nx * overlap;
				a1->y -= ny * overlap;
				a2->x += nx * overlap;
				a2->y += ny * overlap;

				// (dvx, dvy) is the relative velocity
				float dvx = a2->dx - a1->dx;
				float dvy = a2->dy - a1->dy;

				// Impact speed is the projection of the relative velocity on the collision vectors'
				// direction
				float impact_speed = dvx * nx + dvy * ny;
				if (impact_speed > 0)
					continue;

				// Now that we have the speed impulse (impulse = speed because they are perfectly
				// elastic), we apply it to the asteroids

				// Ponderate the impulse by mass
				float ponderation
				  = a1->radius * a1->radius / (a1->radius * a1->radius + a2->radius * a2->radius);
				a1->dx += nx * 2 * impact_speed * (1.0f - ponderation);
				a1->dy += ny * 2 * impact_speed * (1.0f - ponderation);
				a2->dx -= nx * 2 * impact_speed * ponderation;
				a2->dy -= ny * 2 * impact_speed * ponderation;
			}
		}
	}
}

void create_asteroids(Game *game)
{
	int n_asteroids = (game->level + MIN_ASTEROIDS >= MAX_ASTEROIDS)
						? MAX_ASTEROIDS
						: (game->level + MIN_ASTEROIDS);
	for (int i = 0; i < n_asteroids; i++) {
		Asteroid *asteroid = &game->asteroids[game->n_asteroids++];
		asteroid->radius
		  = rand() % (ASTEROID_RADIUS_MAX - ASTEROID_RADIUS_MIN + 1) + ASTEROID_RADIUS_MIN;
		enum { TOP, RIGHT, BOTTOM, LEFT };
		int side = rand() % 4;
		switch (side) {
		case TOP:
			asteroid->x = rand() % (int)(game->width) + asteroid->radius;
			asteroid->y = asteroid->radius + GRACE_SPACING;
			break;
		case RIGHT:
			asteroid->x = game->width - asteroid->radius - GRACE_SPACING;
			asteroid->y = rand() % (int)(game->height - 2 * asteroid->radius) + asteroid->radius;
			break;
		case BOTTOM:
			asteroid->x = rand() % (int)(game->width - 2 * asteroid->radius) + asteroid->radius;
			asteroid->y = game->height - asteroid->radius - GRACE_SPACING;
			break;
		case LEFT:
			asteroid->x = asteroid->radius + GRACE_SPACING;
			asteroid->y = rand() % (int)(game->height - 2 * asteroid->radius) + asteroid->radius;
			break;
		}
		asteroid->dx = rand() % (ASTEROID_SPEED_MAX - ASTEROID_SPEED_MIN + 1) + ASTEROID_SPEED_MIN;
		asteroid->dy = rand() % (ASTEROID_SPEED_MAX - ASTEROID_SPEED_MIN + 1) + ASTEROID_SPEED_MIN;
	}
}
