#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "player.h"

const size_t MAX_HEALTH = 100;
const size_t MAX_NUM_BULL = 50;
const size_t RELOAD_CLICKS = 200;
const double DEATH_TIME = 5.0;
const double BULLET_FIRE_TIME = .1; // seconds between allowable bullet shots

struct player {
  char *name;
  ssize_t health;
  size_t points;
  size_t bullets_shot;
  size_t reload_clicks;
  double death_time;

  char *curr_bul_path;

  double bul_delta_t;

  double speed_multiplier;
  double dmg_multiplier;

  size_t health_displayed;
  size_t points_displayed;
  size_t bullets_shot_displayed;

  bool is_alive;
};

player_t *player_init(char *name, char *curr_bul_path) {
  player_t *player = malloc(sizeof(player_t));
  assert(player);
  player->name = name;

  player->health = MAX_HEALTH;
  player->points = 0;
  player->bullets_shot = 0;
  player->reload_clicks = 0;
  player->death_time = DEATH_TIME;
  player->curr_bul_path = curr_bul_path;

  player->bul_delta_t = 0.0;

  player->speed_multiplier = 1;
  player->dmg_multiplier = 1;

  player->health_displayed = MAX_HEALTH;
  player->points_displayed = 0;
  player->bullets_shot_displayed = 0;

  player->is_alive = true;
  return player;
}

size_t player_get_points(player_t *player) { return player->points; }

size_t player_get_health(player_t *player) { return player->health; }

size_t player_get_reload_clicks(player_t *player) {
  return player->reload_clicks;
}

size_t player_get_displayed_health(player_t *player) {
  return player->health_displayed;
}

size_t player_get_displayed_points(player_t *player) {
  return player->points_displayed;
}

size_t player_get_displayed_bull(player_t *player) {
  return player->bullets_shot_displayed;
}

size_t player_get_bull_shot(player_t *player) { return player->bullets_shot; }

// health_change: positive for health boost (gain), negative for damage (loss)
void player_change_health(player_t *player, ssize_t health_change) {
  player->health += health_change;
  if (player->health <= 0) {
    player->health = 0;
    player_kill(player);
  }
}

// points_change: positive for gain, negative for loss
void player_change_points(player_t *player, ssize_t points_change) {
  player->points += points_change;
}

void player_change_displayed_health(player_t *player, size_t value) {
  player->health_displayed = value;
}

void player_change_displayed_points(player_t *player, size_t value) {
  player->points_displayed = value;
}

void player_change_displayed_bull(player_t *player, size_t value) {
  player->bullets_shot_displayed = value;
}

bool player_ok_to_fire(player_t *player) {
  if (player->reload_clicks >= RELOAD_CLICKS) {
    player->reload_clicks = 0;
    player->bullets_shot = 0;
  }
  return player->bul_delta_t >= BULLET_FIRE_TIME &&
         player->bullets_shot < MAX_NUM_BULL;
}

// bull change: + 1 means one bullet shot, -1 means reloaded
void player_increment_bullets_shot(player_t *player, ssize_t bul_shot) {
  if (bul_shot == -1) {
    player->bullets_shot = 0;
  }
  player->bullets_shot++;
}

// reload change: + 1 means one reload clicked, - RELOAD_CLICKS means reload
// clicks resetted
void player_increment_reload_bullets(player_t *player, ssize_t reload_clicks) {
  if (reload_clicks == -1) {
    player->reload_clicks = RELOAD_CLICKS;
  }
  player->reload_clicks++;
}

char *player_return_health_str(player_t *player) {
  char *health_str = malloc(sizeof(char) * 5);
  sprintf(health_str, "%zu", player->health);
  return health_str;
}

char *player_return_points_str(player_t *player) {
  char *points_str = malloc(sizeof(char) * 5);
  sprintf(points_str, "%zu", player->points);
  return points_str;
}

char *player_return_bull_str(player_t *player) {
  char *bull_str = malloc(sizeof(char) * 5);
  sprintf(bull_str, "%zu", MAX_NUM_BULL - player->bullets_shot);
  return bull_str;
}

double player_get_vel_mult(player_t *player) {
  return player->speed_multiplier;
}

double player_get_dmg_mult(player_t *player) { return player->dmg_multiplier; }

void player_set_vel_mult(player_t *player, double multiplier) {
  player->speed_multiplier = multiplier;
}

void player_set_dmg_mult(player_t *player, double multiplier) {
  player->dmg_multiplier = multiplier;
}

void player_kill(player_t *player) {
  player->death_time = 0;
  player->reload_clicks = 0;
  player->speed_multiplier = 0;
  // maybe subtract player points as penality?
}

bool player_is_alive(player_t *player) {
  if (player->death_time >= DEATH_TIME) {
    if (player->speed_multiplier == 0) {
      player->speed_multiplier = 1;
      player->health = MAX_HEALTH;
    }
    return true;
  }
  return false;
}

void player_change_bul_delta_t(player_t *player, double dt) {
  if (dt == -1) {
    player->bul_delta_t = BULLET_FIRE_TIME;
    return;
  }
  if (dt == 0) {
    player->bul_delta_t = 0;
    return;
  }
  player->bul_delta_t += dt;
}

char *player_get_name(player_t *player) { return player->name; }

void player_set_bullet_path(player_t *player, char *path) {
  player->curr_bul_path = path;
}

char *player_get_bullet_path(player_t *player) { return player->curr_bul_path; }

void player_increment_death_time(player_t *player, double dt) {
  player->death_time += dt;
  if (player_is_alive(player)) {
    player->health = MAX_HEALTH;
  }
}

void player_free(player_t *player) { free(player); }