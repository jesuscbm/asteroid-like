#ifndef CONFIG_H
#define CONFIG_H

/**
 * @brief Initial WIDTH of the window
 */
#define WIDTH				800
/**
 * @brief Initial HEIGHT of the window
 */
#define HEIGHT				600
/**
 * @brief Background color in "R, G, B" format
 */
#define BG_COLOR			0.0f, 0.0f, 0.0f
/**
 * @brief Line color in "R, G, B" format
 */
#define LINE_COLOR			1.0f, 1.0f, 1.0f
/**
 * Maximum number of frames per second
 */
#define FPS					60

/**
 * @brief Radius of the ship
 */
#define SHIP_RADIUS			30
/**
 * @brief Maximum speed of the ship
 */
#define MAX_SPEED			5
/**
 * @brief Minimum speed of the ship. Negative values mean you can decelerate.
 */
#define MIN_SPEED			0
/**
 * @brief Speed step
 */
#define SPEED_ACCEL			0.1f
/**
 * @brief Rotation step
 */
#define ROTATION_SPEED		6

/**
 * @brief Maximum number of bullets that can be in the screen at any time
 */
#define MAX_BULLETS			30
/**
 * @brief Velocity of the bullet
 */
#define BULLET_VELOCITY		10
/**
 * @brief Radius of the bullet. Only for rendering purposes
 */
#define BULLET_RADIUS		5

/**
 * @brief Asteroids to appear in level 1
 */
#define MIN_ASTEROIDS		1
/**
 * @brief Maximum number of asteroids that can appear in any level
 */
#define MAX_ASTEROIDS		10
/**
 * @brief Minimum radius of an asteroid
 */
#define ASTEROID_RADIUS_MIN 10
/**
 * @brief Asteroids with radies bigger than this will split when shot at
 */
#define ASTEROID_SPLIT_THRESHOLD 25
/**
 * @brief Maximum radius of an asteroid
 */
#define ASTEROID_RADIUS_MAX 30
/**
 * @brief Minimum speed of an asteroid
 */
#define ASTEROID_SPEED_MIN	1
/**
 * @brief Maximum speed of an asteroid
 */
#define ASTEROID_SPEED_MAX	3

#endif	// !CONFIG_H
