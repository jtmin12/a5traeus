#ifndef __PLAYER_H__
#define __PLAYER_H__

#include <stdbool.h>
#include <stddef.h>

typedef struct player player_t;

/**
 * Returns a pointer to a player_t struct
 *
 * @param name name of player
 * @param bul_path path of bullet image that player is currently shooting
 * @return a pointer to a player_t struct
 */
player_t *player_init(char *name, char *bul_path);

/**
 * Returns the current number of points that a player has
 *
 * @param player to check points on
 * @return number of points currently held by player
 */
size_t player_get_points(player_t *player);

/**
 * Returns the number of health points that a player has
 *
 * @param player to check health of
 * @return number of health points currently held by player
 */
size_t player_get_health(player_t *player);

/**
 * Returns the number of reload clicks that a player has done
 *
 * @param player to check reload clicks
 * @return number of reload clicks
 */
size_t player_get_reload_clicks(player_t *player);

/**
 * Gets the displayed health of the player.
 *
 * @param player The player object.
 * @return The displayed health of the player.
 */
size_t player_get_displayed_health(player_t *player);

/**
 * Gets the displayed points of the player.
 *
 * @param player The player object.
 * @return The displayed points of the player.
 */
size_t player_get_displayed_points(player_t *player);

/**
 * Gets the displayed bullets shot of the player.
 *
 * @param player The player object.
 * @return The displayed number of bullets shot by the player.
 */
size_t player_get_displayed_bull(player_t *player);

/**
 * Returns the current number of bullets shot by a player so far since their gun
 * was last disabled
 *
 * @param player to check number of bullets shot
 * @return number of bullets shot by player
 */
size_t player_get_bull_shot(player_t *player);

/**
 * Changes the amount of health that a player has by the value passed in
 *
 * @param player to change health
 * @param health_change to alter player health by
 */
void player_change_health(player_t *player, ssize_t health_change);

/**
 * Changes the number of points that a player has by the value passed in
 *
 * @param player to change points amount
 * @param points_change to change player's points by
 */
void player_change_points(player_t *player, ssize_t points_change);

/**
 * Changes the displayed health of the player.
 *
 * @param player The player object.
 * @param value The new value for the displayed health.
 */
void player_change_displayed_health(player_t *player, size_t value);

/**
 * Changes the displayed points of the player.
 *
 * @param player The player object.
 * @param value The new value for the displayed points.
 */
void player_change_displayed_points(player_t *player, size_t value);

/**
 * Changes the displayed bullets shot by the player.
 *
 * @param player The player object.
 * @param value The new value for the displayed bullets shot.
 */
void player_change_displayed_bull(player_t *player, size_t value);

/**
 * Returns true if player is allowed to fire, false otherwise
 *
 * @param player to check if allowed to shoot gun
 * @return true if allowed to shoot, false otherwise
 */
bool player_ok_to_fire(player_t *player);

/**
 * Increments player shot bullet count by bul_shot if 1, otherwise it resets the
 * count for -1
 *
 * @param player to increment bullets shot count
 * @param bul_shot number to change player bullets shot number by
 */
void player_increment_bullets_shot(player_t *player, ssize_t bul_shot);

/**
 * Increments player reload clicks if reload_clicks is 1, otherwise it resets
 * reload_clicks to 0
 *
 * @param player to change reload clicks
 * @param reload_clicks number to change reload clicks of player by
 */
void player_increment_reload_bullets(player_t *player, ssize_t reload_clicks);

/**
 * Returns the health amount of a player as a string
 *
 * @param player to get health amount from
 * @return health of player as a null-terminated string
 */
char *player_return_health_str(player_t *player);

/**
 * Returns the points amount of a player as a string
 *
 * @param player to get points amount from
 * @return points of a player as a null-terminated string
 */
char *player_return_points_str(player_t *player);

/**
 * Returns the number of bullets a player has left as a string
 *
 * @param player to get bullets left from
 * @return bullets left to shoot of a player as a null-terminated string
 */
char *player_return_bull_str(player_t *player);

/**
 * Returns the player's velocity multiplier
 *
 * @param player to get velocity multiplier from
 * @return the player's velocity multiplier
 */
double player_get_vel_mult(player_t *player);

/**
 * Returns the player's damage multiplier
 *
 * @param player to get damage multiplier from
 * @return the player's damage multiplier
 */
double player_get_dmg_mult(player_t *player);

/**
 * Sets the players velocity multiplier
 *
 * @param player to set velocity multiplier
 * @param multiplier number to set multiplier to
 */
void player_set_vel_mult(player_t *player, double multiplier);

/**
 * Sets the player's damage multiplier
 *
 * @param player to set damage multiplier
 * @param multiplier number to set multiplier to
 */
void player_set_dmg_mult(player_t *player, double multiplier);

/**
 * Kills player, makes player unable to move forward or backward or shoot until
 * death timer is ended
 *
 * @param player to kill
 */
void player_kill(player_t *player);

/**
 * Checks if the player is alive.
 *
 * @param player The player object.
 * @return True if the player is alive, false otherwise.
 */
bool player_is_alive(player_t *player);

/**
 * Changes the bullet delta time of the player.
 *
 * @param player The player object.
 * @param dt The change in delta time.
 */
void player_change_bul_delta_t(player_t *player, double dt);

/**
 * Gets the name of the player.
 *
 * @param player The player object.
 * @return The name of the player.
 */
char *player_get_name(player_t *player);

/**
 * Sets the bullet path of the player.
 *
 * @param player The player object.
 * @param path The new bullet path.
 */
void player_set_bullet_path(player_t *player, char *path);

/**
 * Gets the bullet path of the player.
 *
 * @param player The player object.
 * @return The bullet path of the player.
 */
char *player_get_bullet_path(player_t *player);

/**
 * Increments the death time of the player.
 * @param player The player object.
 * @param dt The amount to increment the death time by.
 */
void player_increment_death_time(player_t *player, double dt);

/**
 * Frees the player object.
 * @param player The player object to free.
 */
void player_free(player_t *player);

#endif // #ifndef __PLAYER_H__
