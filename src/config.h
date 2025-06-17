#ifndef CONFIG_H
#define CONFIG_H

#define WIDTH				800
#define HEIGHT				600
#define BG_COLOR			0.0f, 0.0f, 0.0f
#define LINE_COLOR			1.0f, 1.0f, 1.0f
#define FPS					60

#define SHIP_RADIUS			30
#define MAX_SPEED			5
#define MIN_SPEED			0
#define SPEED_ACCEL			0.1f
#define ROTATION_SPEED		6

#define MAX_BULLETS			30
#define BULLET_VELOCITY		10
#define BULLET_RADIUS		5

#define MIN_ASTEROIDS		1
#define MAX_ASTEROIDS		10
#define ASTEROID_RADIUS_MIN 15
#define ASTEROID_SPLIT_THRESHOLD 25
#define ASTEROID_RADIUS_MAX 40
#define ASTEROID_SPEED_MIN	1
#define ASTEROID_SPEED_MAX	3

#endif	// !CONFIG_H
