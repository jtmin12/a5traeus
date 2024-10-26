#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include "asset.h"
#include "asset_cache.h"
#include "collision.h"
#include "forces.h"
#include "player.h"
#include "screen.h"
#include "sdl_wrapper.h"
const uint8_t NUM_SCREENS = 2;
const uint8_t OP_SCREEN_IDX = 0;
const uint8_t GAME_SCREEN_IDX = 1;

const double PLAYER_RADIUS = 15;
const double BULLET_RADIUS = 40;
const double PWRUP_RADIUS = 10;
const double EVENT_RADIUS = 10;
const double BLK_HOLE_RADIUS = 50;
const double TXT_RADIUS = 30;

const vector_t MIN = {0, 0};
const vector_t MAX = {1000, 500};
const uint16_t MAX_POINTS = 500;
const uint8_t POINTS_FOR_ASTER = 10;

const vector_t P1_START_POS = {450, 30};
const vector_t P1_RESET_POS = {450, 45};

const vector_t P2_START_POS = {550, 30};
const vector_t P2_RESET_POS = {550, 45};

const double SHIP_MASS = 100;
const double BULLET_MASS = 100;
const double ASTEROID_MASS = 100;
const double PWRUP_MASS = 10;
const double EVENT_MASS = 10000;

const double PWRUP_SPAWN_TIME = 5.0;   // seconds between pwrup spawn
const double EVENT_SPAWN_TIME = 20.0;  // seconds between pot. events
const double CHANCE_EVENT_SPAWN = 1.0; // % chance that event spawn // old: 0.2
const double ASTEROID_SPAWN_TIME = 2.0;

const int8_t BUL_DMG_TO_PLAYER = -10;
const int8_t ASTER_DMG_TO_PLAYER = -20;

const double DMG_MULT = 2.0;
const double VEL_MULT = 1.2;
const double TIME_DIL_VEL_MULT = 0.5;
const vector_t HEALTH_DELTA = {10, 30};

const double BULLET_SPEED = 1000;
const double PWRUP_SPEED = 20;
const double EVENT_SPEED = 3;
const vector_t ASTER_SPEEDS = {100, 300};

const uint8_t OBSTACLE_HEIGHT = 30;
const vector_t OBS_WIDTHS = {30, 70};
const double METAL_WIDTH = 50;
const double METAL_HEIGHT = 250;
const vector_t METAL1_POS = {250, 250};
const vector_t METAL2_POS = {750, 250};

const uint8_t SHIP_NUM_POINTS = 20;

const rgb_color_t WHITE_COLOR = (rgb_color_t){255, 255, 255};
const rgb_color_t BLACK_COLOR = (rgb_color_t){0.0, 0.0, 0.0};

// constants to create movement
const double MAX_VEL = 20;
const uint8_t STEP = 10;
const double ANG_VEL = M_PI / 30;
const double BLK_HOLE_GRAV = 1e2;
const double BUL_ASTER_GRAV = 1e3;

// for the asteroids
const uint8_t INIT_NUM_ASTEROIDS = 5;
const uint8_t INIT_VEL = 100;

const uint8_t CIRC_NPOINTS = 4;
const uint8_t INIT_CAPACITY_BODY_ASSETS = 2;

const char *BLU_SPACESHIP_PATH = "assets/blue_spaceship.png";
const char *RED_SPACESHIP_PATH = "assets/red_spaceship.png";
const char *BLU_GHOST_SHIPPY_PATH = "assets/blue_spaceship_ghost.png";
const char *RED_GHOST_SHIPPY_PATH = "assets/red_spaceship_ghost.png";
const char *ASTEROID_PATH = "assets/asteroid.png";
const char *BACKGROUND_PATH = "assets/space_background.png";
const char *LOGO_PATH = "assets/logo.png";
const char *STARTBTN_PATH = "assets/start.png";
const char *BLU_DMG_BULLET_PATH = "assets/blu_dmg_bullet.png";
const char *RED_DMG_BULLET_PATH = "assets/red_dmg_bullet.png";
const char *POINTS_PATH = "assets/points.png";
const char *BLU_BULLET_PATH = "assets/blue_bullet.png";
const char *RED_BULLET_PATH = "assets/red_bullet.png";
const char *FONT_PATH = "assets/Cascadia.ttf";

const char *SPEED_PWRUP_PATH = "assets/lightning.png";
const char *HEALTH_PWRUP_PATH = "assets/heart.png";
const char *DAMAGE_PWRUP_PATH = "assets/plasma.png";

const char *BLK_HOLE_PATH = "assets/black_hole.png";
const char *TIME_DIL_PATH = "assets/time_dilation.png";

const char *HORIZ_METAL_PATH = "assets/metal.png";
const char *VERT_METAL_PATH = "assets/metal2.png";

const char *EXPLOSION1_PATH = "assets/explosion1.png";
const char *EXPLOSION2_PATH = "assets/explosion2.png";
const char *EXPLOSION3_PATH = "assets/explosion3.png";
const char *EXPLOSION4_PATH = "assets/explosion4.png";

const char *BG_MUSIC = "assets/audio_WithoutFear.ogg";
const char *SHOOTING_SFX = "assets/audio_shoot.wav";

const char *TY_PATH = "assets/ty.jpg";

const char *PLAYER1_NAME = "Player1";
const char *PLAYER2_NAME = "Player2";
const char *ASTEROID_INFO = "Asteroid";
const char *DEAD_ASTER_INFO = "Dead Asteroid";
const char *BULLET_INFO = "Player Bullet";
const char *PWRUP_INFO = "Pwrup";
const char *EVENT_INFO = "Event";
const char *METAL_INFO = "Metal";

const char *HEALTH_SYSTEM_INFO = "Health";
const char *POINTS_SYSTEM_INFO = "Points";
const char *BULL_SYSTEM_INFO = "Bullets";

typedef struct dead_aster {
  double dt;
  int change_num;
  body_t *body;
  asset_t *asset;
} dead_aster_t;

dead_aster_t *dead_aster_init(double dt, int change_num, body_t *body,
                              asset_t *asset) {
  dead_aster_t *dead_aster = malloc(sizeof(dead_aster_t));
  assert(dead_aster);
  dead_aster->dt = dt;
  dead_aster->change_num = change_num;
  dead_aster->body = body;
  dead_aster->asset = asset;
  return dead_aster;
}

void dead_aster_free(dead_aster_t *dead_aster) {
  body_remove(dead_aster->body);
  free(dead_aster);
}

void dead_aster_change(dead_aster_t *dead_aster, list_t *assets) {
  body_t *body = dead_aster->body;

  size_t asset_idx;
  for (size_t j = 0; j < list_size(assets); j++) {
    if (list_get(assets, j) == dead_aster->asset) {
      asset_idx = j;
      break;
    }
  }

  if (dead_aster->change_num == 1 && dead_aster->dt >= 0.25 &&
      dead_aster->dt < 0.5) {
    asset_t *asset = asset_make_image_with_body(EXPLOSION2_PATH, body);
    list_set(assets, asset, asset_idx);
    dead_aster->asset = asset;
    dead_aster->change_num = 2;
  } else if (dead_aster->change_num == 2 && dead_aster->dt >= 0.5 &&
             dead_aster->dt < 0.75) {
    asset_t *asset = asset_make_image_with_body(EXPLOSION3_PATH, body);
    list_set(assets, asset, asset_idx);
    dead_aster->asset = asset;
    dead_aster->change_num = 3;
  } else if (dead_aster->change_num == 3 && dead_aster->dt >= 0.75 &&
             dead_aster->dt < 1) {
    asset_t *asset = asset_make_image_with_body(EXPLOSION4_PATH, body);
    list_set(assets, asset, asset_idx);
    dead_aster->asset = asset;
    dead_aster->change_num = 4;
  }
}

struct state {
  list_t *screens;
  size_t screen_idx;
  uint16_t points;

  body_t *shippy1;
  body_t *shippy2;
  list_t *dead_asters;

  asset_t *shoot_sfx;

  double pwrup_delta_t;    // time since last powerup
  double event_delta_t;    // time since last event
  double asteroid_delta_t; // time since last asteroid
};

void next_screen(state_t *state) {
  state->screen_idx += 1;
  state->screen_idx = state->screen_idx % list_size(state->screens);
}

double rand_double(double low, double high) {
  return (high - low) * rand() / RAND_MAX + low;
}

void rand_boundary_loc(vector_t *center, double *dir_angle) {
  double rand = ceil(rand_double(0, 4));

  double random_x = rand_double(MIN.x, MAX.x);
  double random_y = rand_double(MIN.y, MAX.y);

  if (rand == 1.0) { // spawn at y = MIN.y -> angle in [0, pi]
    *center = (vector_t){random_x, MIN.y};
    *dir_angle = rand_double(0, M_PI);
  } else if (rand == 2.0) { // spawn at x = MAX.x -> angle in [pi/2, 3pi/2]
    *center = (vector_t){MAX.x, random_y};
    *dir_angle = rand_double(M_PI / 2, 3 * M_PI / 2);
  } else if (rand == 3.0) { // spawn at y = MAX.y -> angle in [pi, 2pi]
    *center = (vector_t){random_x, MAX.y};
    *dir_angle = rand_double(M_PI, 2 * M_PI);
  } else { // spawn at x = MIN.x -> angle in [-pi/2, pi/2]
    *center = (vector_t){MIN.x, random_y};
    *dir_angle = rand_double(-M_PI / 2, M_PI / 2);
  }
}

body_t *make_obstacle(size_t w, size_t h, vector_t center, double mass,
                      void *info) {
  list_t *c = list_init(4, free);
  vector_t *v1 = malloc(sizeof(vector_t));
  assert(v1);
  v1->x = 0;
  v1->y = 0;
  list_add(c, v1);

  vector_t *v2 = malloc(sizeof(vector_t));
  assert(v2);
  v2->x = w;
  v2->y = 0;
  list_add(c, v2);

  vector_t *v3 = malloc(sizeof(vector_t));
  assert(v3);
  v3->x = w;
  v3->y = h;
  list_add(c, v3);

  vector_t *v4 = malloc(sizeof(vector_t));
  assert(v4);
  v4->x = 0;
  v4->y = h;
  list_add(c, v4);

  body_t *obstacle = body_init_with_info(c, mass, BLACK_COLOR, info, NULL, 0);
  body_set_centroid(obstacle, center);
  return obstacle;
}

body_t *make_spaceship(vector_t center, player_t *info) {
  center.y += PLAYER_RADIUS;
  list_t *c = list_init(SHIP_NUM_POINTS, free);
  for (size_t i = 0; i < SHIP_NUM_POINTS; i++) {
    double angle = 2 * M_PI * i / SHIP_NUM_POINTS;
    vector_t *v = malloc(sizeof(vector_t));
    assert(v);
    v->x = center.x + PLAYER_RADIUS * cos(angle);
    v->y = center.y + PLAYER_RADIUS * sin(angle);
    list_add(c, v);
  }
  body_t *shippy = body_init_with_info(c, SHIP_MASS, BLACK_COLOR, info,
                                       (free_func_t)player_free, 0);
  return shippy;
}

body_t *make_asteroid() {
  vector_t pos;
  double dir;
  rand_boundary_loc(&pos, &dir);

  double w = rand_double(OBS_WIDTHS.x, OBS_WIDTHS.y);
  body_t *asteroid =
      make_obstacle(w, OBSTACLE_HEIGHT, pos, ASTEROID_MASS, ASTEROID_INFO);

  double aster_speed = rand_double(ASTER_SPEEDS.x, ASTER_SPEEDS.y);
  vector_t vel = create_vector(aster_speed, dir);
  body_set_velocity(asteroid, vel);

  double rotation_speed = rand_double(0, M_PI / 2);
  body_set_rotation_speed(asteroid, rotation_speed);

  return asteroid;
}

/** Make a circle-shaped body object.
 *
 * @param center a vector representing the center of the body.
 * @param radius the radius of the circle
 * @param mass the mass of the body
 * @param color the color of the circle
 * @param dir_angle the direction angle of movement
 * @return pointer to the circle-shaped body
 */
body_t *make_body(vector_t center, double radius, double mass, double dir_angle,
                  rgb_color_t color, void *info) {
  list_t *c = list_init(CIRC_NPOINTS, free);
  for (size_t i = 0; i < CIRC_NPOINTS; i++) {
    double angle = 2 * M_PI * i / CIRC_NPOINTS;
    vector_t *v = malloc(sizeof(vector_t));
    assert(v);
    *v = (vector_t){center.x + radius * cos(angle),
                    center.y + radius * sin(angle)};
    list_add(c, v);
  }
  return body_init_with_info(c, mass, color, info, NULL, dir_angle);
}

body_t *make_bullet(vector_t center, double angle, char *info) {
  body_t *bullet =
      make_body(center, BULLET_RADIUS, BULLET_MASS, angle, BLACK_COLOR, info);

  vector_t bull_vel = create_vector(BULLET_SPEED, M_PI / 2 - angle);
  body_set_velocity(bullet, bull_vel);
  return bullet;
}

body_t *make_powerup() {
  vector_t center;
  double angle;
  rand_boundary_loc(&center, &angle);

  char *info;
  double pwrup_choice = ceil(rand_double(0, 3));
  if (pwrup_choice == 1.0) {
    info = SPEED_PWRUP_PATH;
  } else if (pwrup_choice == 2.0) {
    info = HEALTH_PWRUP_PATH;
  } else {
    info = DAMAGE_PWRUP_PATH;
  }

  body_t *pwrup =
      make_body(center, PWRUP_RADIUS, PWRUP_MASS, 0, BLACK_COLOR, info);

  vector_t pwrup_vel = create_vector(PWRUP_SPEED, angle);
  body_set_velocity(pwrup, pwrup_vel);
  return pwrup;
}

body_t *make_random_physics_event() {
  char *info;
  double event_choice = rand_double(0, 1);
  double radius;
  if (event_choice <= 0.5) {
    info = BLK_HOLE_PATH;
    radius = BLK_HOLE_RADIUS;
  } else {
    info = TIME_DIL_PATH;
    radius = EVENT_RADIUS;
  }

  vector_t center;
  double angle;
  rand_boundary_loc(&center, &angle);

  body_t *event = make_body(center, radius, EVENT_MASS, 0, BLACK_COLOR, info);

  vector_t event_vel = create_vector(EVENT_SPEED, angle);
  body_set_velocity(event, event_vel);
  return event;
}

void user_wrap_edges(body_t *player) {
  vector_t centroid = body_get_centroid(player);
  if (centroid.y + PLAYER_RADIUS >= MAX.y) {
    centroid.y = MAX.y - PLAYER_RADIUS;
    body_set_centroid(player, centroid);
  }
  if (centroid.y - PLAYER_RADIUS <= MIN.y) {
    centroid.y = MIN.y + PLAYER_RADIUS;
    body_set_centroid(player, centroid);
  }
  if (centroid.x + PLAYER_RADIUS >= MAX.x) {
    centroid.x = MAX.x - PLAYER_RADIUS;
    body_set_centroid(player, centroid);
  }
  if (centroid.x - PLAYER_RADIUS <= MIN.x) {
    centroid.x = MIN.x + PLAYER_RADIUS;
    body_set_centroid(player, centroid);
  }
}

void delete_if_on_edge(body_t *body) {
  vector_t centroid = body_get_centroid(body);
  polygon_t *poly = body_get_polygon(body);
  list_t *vectors = polygon_get_points(poly);
  vector_t *vertex_zero = list_get(vectors, 0);
  double radius = vec_get_length(vec_subtract(*vertex_zero, centroid));

  if (centroid.y - radius >= MAX.y || centroid.y + radius <= MIN.y ||
      centroid.x - radius >= MAX.x || centroid.x + radius <= MIN.x) {
    body_remove(body);
  }
}

void wrap_edges(state_t *state, body_t *body) {
  if (body == state->shippy1) {
    user_wrap_edges(body);
    return;
  }
  if (body == state->shippy2) {
    user_wrap_edges(body);
    return;
  }

  void *info = body_get_info(body);

  if (info == ASTEROID_INFO) {
    vector_t centroid = body_get_centroid(body);
    if (centroid.x > MAX.x) {
      body_set_centroid(body, (vector_t){MIN.x, centroid.y});
    } else if (centroid.x < MIN.x) {
      body_set_centroid(body, (vector_t){MAX.x, centroid.y});
    }
    return;
  }

  delete_if_on_edge(body);
}

void player_bullet_collision_handler(body_t *body1, body_t *body2,
                                     vector_t axis, void *aux,
                                     double force_const) {
  // decrease player health
  player_t *player = body_get_info(body1);
  player_change_health(player, force_const);
  // remove bullet
  body_remove(body2);
}

void create_player_bullet_collision(scene_t *scene, body_t *player,
                                    body_t *bullet) {
  create_collision(scene, player, bullet, player_bullet_collision_handler, NULL,
                   BUL_DMG_TO_PLAYER);
}

void bullet_asteroid_collision_handler(body_t *body1, body_t *body2,
                                       vector_t axis, void *aux,
                                       double force_const) {
  body_remove(body1);
  body_remove(body2);
  state_t *state = aux;
  char *info = body_get_info(body1);
  if (info == RED_BULLET_PATH || info == RED_DMG_BULLET_PATH) {
    player_t *player1 = body_get_info(state->shippy1);
    player_change_points(player1, force_const);
  } else if (info == BLU_BULLET_PATH || info == BLU_DMG_BULLET_PATH) {
    player_t *player2 = body_get_info(state->shippy2);
    player_change_points(player2, force_const);
  }
}

void create_bullet_asteroid_collision(state_t *state, scene_t *scene,
                                      body_t *body1, body_t *body2) {
  create_collision(scene, body1, body2, bullet_asteroid_collision_handler,
                   state, POINTS_FOR_ASTER);
}

void ship_aster_collision_handler(body_t *body1, body_t *body2, vector_t axis,
                                  void *aux, double force_const) {
  body_remove(body2);
  player_t *player = body_get_info(body1);
  player_change_health(player, force_const);
}

void create_ship_aster_collision(scene_t *scene, body_t *shippy,
                                 body_t *asteroid) {
  create_collision(scene, shippy, asteroid, ship_aster_collision_handler, NULL,
                   ASTER_DMG_TO_PLAYER);
}

void on_key(char key, key_event_type_t type, double held_time, state_t *state) {
  screen_t *game_screen = list_get(state->screens, GAME_SCREEN_IDX);
  scene_t *game_screen_scene = screen_get_scene(game_screen);
  list_t *game_screen_assets = screen_get_body_assets(game_screen);

  body_t *shippy1 = scene_get_body(game_screen_scene, 0);
  body_t *shippy2 = scene_get_body(game_screen_scene, 1);

  vector_t curr_vel_1 = body_get_velocity(shippy1);
  double curr_rot_1 = body_get_direction_angle(shippy1);

  vector_t curr_vel_2 = body_get_velocity(shippy2);
  double curr_rot_2 = body_get_direction_angle(shippy2);

  player_t *player1 = body_get_info(state->shippy1);
  player_t *player2 = body_get_info(state->shippy2);
  double vel_mult_1 = player_get_vel_mult(player1);
  double vel_mult_2 = player_get_vel_mult(player2);
  double dmg_mult_1 = player_get_dmg_mult(player1);
  double dmg_mult_2 = player_get_dmg_mult(player2);

  if (type == KEY_PRESSED && type != KEY_RELEASED) {
    switch (key) {
    case 'a':
      curr_rot_1 -= ANG_VEL;
      break;
    case 'd':
      curr_rot_1 += ANG_VEL;
      break;
    case 'w':
      if (vec_get_length(curr_vel_1) < MAX_VEL) {
        curr_vel_1.x = STEP * sin(curr_rot_1) * vel_mult_1;
        curr_vel_1.y = STEP * cos(curr_rot_1) * vel_mult_1;
      }
      break;
    case 's':
      if (vec_get_length(curr_vel_1) < MAX_VEL) {
        curr_vel_1.x = -STEP * sin(curr_rot_1) * vel_mult_1;
        curr_vel_1.y = -STEP * cos(curr_rot_1) * vel_mult_1;
      }
      break;
    case 'v': {
      if (!player_ok_to_fire(player1)) {
        player_increment_reload_bullets(player1, 1);
        player_set_bullet_path(player1, RED_BULLET_PATH);
        player_set_dmg_mult(player1, 1.0);
        break;
      }
      char *p1_bul_path = player_get_bullet_path(player1);
      vector_t shippy1_center = body_get_centroid(shippy1);
      body_t *user1_bullet =
          make_bullet(shippy1_center, curr_rot_1, p1_bul_path);

      scene_add_body(game_screen_scene, user1_bullet);

      asset_t *bullet1_asset =
          asset_make_image_with_body(p1_bul_path, user1_bullet);

      list_add(game_screen_assets, bullet1_asset);
      player_change_bul_delta_t(player1, 0);
      player_increment_bullets_shot(player1, 1);

      asset_play_sfx(state->shoot_sfx, 80);

      for (size_t i = 0; i < scene_bodies(game_screen_scene); i++) {
        body_t *obstacle = scene_get_body(game_screen_scene, i);
        void *info = body_get_info(obstacle);
        // add bullet magnetism here...
        if (info == ASTEROID_INFO) {
          create_bullet_asteroid_collision(state, game_screen_scene,
                                           user1_bullet, obstacle);
          create_newtonian_gravity(game_screen_scene, BUL_ASTER_GRAV,
                                   user1_bullet, obstacle);
        } else if (info == player2) {
          create_player_bullet_collision(game_screen_scene, obstacle,
                                         user1_bullet);
        } else if (info == METAL_INFO) {
          create_one_sided_destructive_collision(game_screen_scene, obstacle,
                                                 user1_bullet);
        }
      }
      break;
    }
    case LEFT_ARROW: // cc for player2
      curr_rot_2 -= ANG_VEL;
      break;
    case RIGHT_ARROW: // c for player 2
      curr_rot_2 += ANG_VEL;
      break;
    case UP_ARROW: // forward for p2
      if (vec_get_length(curr_vel_2) < MAX_VEL) {
        curr_vel_2.x = STEP * sin(curr_rot_2) * vel_mult_2;
        curr_vel_2.y = STEP * cos(curr_rot_2) * vel_mult_2;
      }
      break;
    case DOWN_ARROW: // backward for p2
      if (vec_get_length(curr_vel_2) < MAX_VEL) {
        curr_vel_2.x = -STEP * sin(curr_rot_2) * vel_mult_2;
        curr_vel_2.y = -STEP * cos(curr_rot_2) * vel_mult_2;
      }
      break;
    case 'm': {
      if (!player_ok_to_fire(player2)) {
        player_increment_reload_bullets(player2, 1);
        player_set_bullet_path(player2, BLU_BULLET_PATH);
        player_set_dmg_mult(player2, 1.0);
        break;
      }
      char *p2_bul_path = player_get_bullet_path(player2);
      vector_t shippy2_center = body_get_centroid(shippy2);
      body_t *user2_bullet =
          make_bullet(shippy2_center, curr_rot_2, p2_bul_path);

      scene_add_body(game_screen_scene, user2_bullet);

      asset_t *bullet2_asset =
          asset_make_image_with_body(p2_bul_path, user2_bullet);

      list_add(game_screen_assets, bullet2_asset);
      player_change_bul_delta_t(player2, 0);
      player_increment_bullets_shot(player2, 1);

      asset_play_sfx(state->shoot_sfx, 80);

      for (size_t i = 0; i < scene_bodies(game_screen_scene); i++) {
        body_t *obstacle = scene_get_body(game_screen_scene, i);
        void *info = body_get_info(obstacle);
        // add bullet magnetism here...
        if (info == ASTEROID_INFO) {
          create_bullet_asteroid_collision(state, game_screen_scene,
                                           user2_bullet, obstacle);
          create_newtonian_gravity(game_screen_scene, BUL_ASTER_GRAV,
                                   user2_bullet, obstacle);
        }
        if (info == player1) {
          create_player_bullet_collision(game_screen_scene, obstacle,
                                         user2_bullet);
        }
        if (info == METAL_INFO) {
          create_one_sided_destructive_collision(game_screen_scene, obstacle,
                                                 user2_bullet);
        }
      }
      break;
    }
    }

    body_set_rotation(shippy1, curr_rot_1);
    vector_t new_centroid1 = vec_add(body_get_centroid(shippy1), curr_vel_1);
    body_set_centroid(shippy1, new_centroid1);

    body_set_rotation(shippy2, curr_rot_2);
    vector_t new_centroid2 = vec_add(body_get_centroid(shippy2), curr_vel_2);
    body_set_centroid(shippy2, new_centroid2);
  }
}

void ship_item_collision_handler(body_t *shippy, body_t *item, vector_t axis,
                                 void *aux, double force_const) {
  state_t *state = aux;
  char *info = body_get_info(item);
  player_t *player = body_get_info(shippy);
  char *ship_info = player_get_name(player);
  if (info == HEALTH_PWRUP_PATH) {
    // increase player health somehow
    ssize_t health_amt = rand_double(HEALTH_DELTA.x, HEALTH_DELTA.y);
    player_change_health(player, health_amt);
  } else if (info == SPEED_PWRUP_PATH) {
    // bump up player speed multiplier
    player_set_vel_mult(player, VEL_MULT);
  } else if (info == DAMAGE_PWRUP_PATH) {
    // bump up player dmg multiplier
    // make future bullets use "dmg_bullet.png" instead of "bullet.png"
    // set some type of count on number of bullets to then change back to
    // "bullet.png"
    player_set_dmg_mult(player, DMG_MULT);
    if (ship_info == PLAYER1_NAME) {
      player_set_bullet_path(player, RED_DMG_BULLET_PATH);
    } else {
      player_set_bullet_path(player, BLU_DMG_BULLET_PATH);
    }
    player_change_bul_delta_t(player, 0);
    player_increment_reload_bullets(player, -1);
    player_increment_bullets_shot(player, -1);
  } else if (info == BLK_HOLE_PATH) {
    // kill ship (reset ship or not, idc), trigger the respawn mechanic
    // DONT BODY_REMOVE(BLACK HOLE)!!!
    player_kill(player);
    return;
  } else if (info == TIME_DIL_PATH) {
    // slow down other player
    if (ship_info == PLAYER1_NAME) {
      player_t *player2 = body_get_info(state->shippy2);
      player_set_vel_mult(player2, TIME_DIL_VEL_MULT);
    } else {
      player_t *player1 = body_get_info(state->shippy1);
      player_set_vel_mult(player1, TIME_DIL_VEL_MULT);
    }
  }
  body_remove(item);
}

void create_item_collision(state_t *state, scene_t *scene, body_t *body,
                           body_t *item) {
  player_t *player = body_get_info(body);
  char *player_name = player_get_name(player);

  if (player_name == PLAYER1_NAME || player_name == PLAYER2_NAME) {
    create_collision(scene, body, item, ship_item_collision_handler, state, 0);
    return;
  }
}

/**
 * Adds an item to the scene and assets list provided,
 * the info field determines whether the item added is
 * a powerup or an event.
 *
 * @param state the state that provides info about where item goes
 * @param info either PWRUP_INFO or EVENT_INFO, does nothing for any other
 * info
 */
void add_item(state_t *state, char *info) {
  body_t *body;
  const char *body_path;
  if (info == PWRUP_INFO) {
    body = make_powerup();
    body_path = body_get_info(body);
  } else if (info == EVENT_INFO) {
    body = make_random_physics_event();
    body_path = body_get_info(body);
  } else {
    return;
  }
  screen_t *screen = list_get(state->screens, state->screen_idx);
  scene_t *scene = screen_get_scene(screen);
  list_t *assets = screen_get_body_assets(screen);

  scene_add_body(scene, body);

  asset_t *body_asset = asset_make_image_with_body(body_path, body);
  list_add(assets, body_asset);

  body_t *shippy1 = state->shippy1;
  body_t *shippy2 = state->shippy2;
  create_item_collision(state, scene, shippy1, body);
  create_item_collision(state, scene, shippy2, body);
  if (body_path == BLK_HOLE_PATH) {
    for (size_t i = 0; i < scene_bodies(scene); i++) {
      body_t *scene_body = scene_get_body(scene, i);
      if (body_get_info(scene_body) == ASTEROID_INFO) {
        create_newtonian_gravity(scene, BLK_HOLE_GRAV, body, scene_body);
      }
    }
  }
}

body_t *add_asteroid(state_t *state) {
  screen_t *screen = list_get(state->screens, GAME_SCREEN_IDX);
  scene_t *scene = screen_get_scene(screen);
  list_t *assets = screen_get_body_assets(screen);

  body_t *shippy1 = scene_get_body(scene, 0);
  body_t *shippy2 = scene_get_body(scene, 1);
  body_t *aster = make_asteroid();

  scene_add_body(scene, aster);
  create_ship_aster_collision(scene, shippy1, aster);
  create_ship_aster_collision(scene, shippy2, aster);
  asset_t *asteroid_asset = asset_make_image_with_body(ASTEROID_PATH, aster);
  list_add(assets, asteroid_asset);

  return aster;
}

/*
Function that returns the minimum value in a double array.
*/
double find_min(double *arr, size_t length) {
  double min = __DBL_MAX__;
  for (size_t i = 0; i < length; i++) {
    if (arr[i] < min) {
      min = arr[i];
    }
  }
  return min;
}

void obstacle_collisions(body_t *player, body_t *metal) {
  vector_t player_pos = body_get_centroid(player);
  vector_t metal_pos = body_get_centroid(metal);

  double top = metal_pos.y + METAL_HEIGHT / 2;
  double bottom = metal_pos.y - METAL_HEIGHT / 2;
  double right = metal_pos.x + METAL_WIDTH / 2;
  double left = metal_pos.x - METAL_WIDTH / 2;

  double dist_to_top = top - player_pos.y;
  double dist_to_bottom = player_pos.y - bottom;
  double dist_to_right = right - player_pos.x;
  double dist_to_left = player_pos.x - left;

  if (player_pos.y - PLAYER_RADIUS <= top &&
      player_pos.y + PLAYER_RADIUS >= bottom &&
      player_pos.x - PLAYER_RADIUS <= right &&
      player_pos.x + PLAYER_RADIUS >= left) {

    double arr[4] = {dist_to_top, dist_to_bottom, dist_to_right, dist_to_left};
    double min = find_min(arr, 4);

    if (min == dist_to_top) {
      player_pos.y = top + PLAYER_RADIUS;
    } else if (min == dist_to_bottom) {
      player_pos.y = bottom - PLAYER_RADIUS;
    } else if (min == dist_to_right) {
      player_pos.x = right + PLAYER_RADIUS;
    } else if (min == dist_to_left) {
      player_pos.x = left - PLAYER_RADIUS;
    }
    body_set_centroid(player, player_pos);
  }
}

void input_img_asset(list_t *assets, char *path, ssize_t x, ssize_t y, size_t w,
                     size_t h) {
  SDL_Rect icon_box = (SDL_Rect){x, y, w, h};
  asset_t *icon = asset_make_image(path, icon_box);
  list_add(assets, icon);
}

asset_t *make_text_asset(char *str, list_t *assets, char *system, size_t x,
                         size_t y, size_t w, size_t h, ssize_t idx) {
  SDL_Rect text_box = (SDL_Rect){x, y, w, h};
  return asset_make_text(FONT_PATH, text_box, system, str, WHITE_COLOR);
}

void input_text_asset(char *str, list_t *assets, char *system, size_t x,
                      size_t y, size_t w, size_t h, ssize_t idx) {
  asset_t *text = make_text_asset(str, assets, system, x, y, w, h, idx);
  list_add_at_index(assets, text, idx);
}

ssize_t find_score_asset_idx(list_t *assets, char *info, size_t new_score) {
  for (size_t i = 0; i < list_size(assets); i++) {
    asset_t *asset = list_get(assets, i);
    char *system = asset_get_system(asset);
    if (!system) {
      continue;
    }
    char *text = asset_get_text(asset);
    if (!text) {
      continue;
    }
    char *ptr;
    size_t old_score = strtol(text, &ptr, 10);
    if (system == info && old_score == new_score) {
      return i;
    }
  }
  return -1;
}

void update_score_display(player_t *player, asset_t *assets) {
  size_t player_health = player_get_health(player);
  size_t player_displayed_health = player_get_displayed_health(player);
  if (player_health != player_displayed_health) {
    size_t idx = find_score_asset_idx(assets, HEALTH_SYSTEM_INFO,
                                      player_displayed_health);
    if (idx == -1) {
      return;
    }
    asset_t *health_asset = list_get(assets, idx);
    SDL_Rect health_bounding_box = asset_get_bounding_box(health_asset);

    char *health_str = player_return_health_str(player);
    asset_t *new_health_asset =
        make_text_asset(health_str, assets, HEALTH_SYSTEM_INFO,
                        health_bounding_box.x, health_bounding_box.y,
                        health_bounding_box.w, health_bounding_box.h, idx);

    list_set(assets, new_health_asset, idx);
    player_change_displayed_health(player, player_health);
  }

  size_t player_points = player_get_points(player);
  size_t player_displayed_points = player_get_displayed_points(player);
  if (player_points != player_displayed_points) {
    size_t idx = find_score_asset_idx(assets, POINTS_SYSTEM_INFO,
                                      player_displayed_points);
    if (idx == -1) {
      return;
    }

    asset_t *points_asset = list_get(assets, idx);
    SDL_Rect points_bounding_box = asset_get_bounding_box(points_asset);

    char *points_str = player_return_points_str(player);
    asset_t *new_points_asset =
        make_text_asset(points_str, assets, POINTS_SYSTEM_INFO,
                        points_bounding_box.x, points_bounding_box.y,
                        points_bounding_box.w, points_bounding_box.h, idx);
    list_set(assets, new_points_asset, idx);

    player_change_displayed_points(player, player_points);
  }

  size_t player_bull = player_get_bull_shot(player);
  size_t player_displayed_bull = player_get_displayed_bull(player);
  if (player_bull != player_displayed_bull) {
    size_t idx =
        find_score_asset_idx(assets, BULL_SYSTEM_INFO, player_displayed_bull);
    if (idx == -1) {
      return;
    }

    asset_t *bull_asset = list_get(assets, idx);
    SDL_Rect bull_bounding_box = asset_get_bounding_box(bull_asset);

    char *bull_str = player_return_bull_str(player);
    asset_t *new_bull_asset = make_text_asset(
        bull_str, assets, BULL_SYSTEM_INFO, bull_bounding_box.x,
        bull_bounding_box.y, bull_bounding_box.w, bull_bounding_box.h, idx);
    list_set(assets, new_bull_asset, idx);

    player_change_displayed_bull(player, player_bull);
  }
}

bool game_is_over(state_t *state) {
  player_t *player1 = body_get_info(state->shippy1);
  player_t *player2 = body_get_info(state->shippy2);

  size_t p1_points = player_get_points(player1);
  size_t p2_points = player_get_points(player2);

  if (p1_points >= MAX_POINTS || p2_points >= MAX_POINTS) {
    return true;
  }
  return false;
}

state_t *emscripten_init() {
  asset_cache_init();
  sdl_init(MIN, MAX);
  state_t *state = malloc(sizeof(state_t));
  assert(state);

  state->screens = list_init(NUM_SCREENS, (free_func_t)screen_free);

  for (size_t i = 0; i < NUM_SCREENS; i++) {
    list_t *body_assets = list_init(INIT_CAPACITY_BODY_ASSETS, NULL);
    scene_t *scene = scene_init();
    screen_t *screen = screen_init(scene, body_assets);
    list_add(state->screens, screen);
  }

  state->screen_idx = 0;

  srand(time(NULL));

  // op screen's scene & assets
  screen_t *op_screen = list_get(state->screens, OP_SCREEN_IDX);
  list_t *op_screen_assets = screen_get_body_assets(op_screen);

  // opening screen

  // op background
  double background_w = MAX.x;
  double background_h = MAX.y;

  input_img_asset(op_screen_assets, BACKGROUND_PATH, MIN.x, MIN.y, background_w,
                  background_h);

  input_img_asset(op_screen_assets, LOGO_PATH, MAX.x / 6, MAX.y / 5,
                  background_w / 1.5, background_h / 2);

  SDL_Rect startbtn_box =
      (SDL_Rect){(MAX.x - background_w / 7) / 2, 0.6 * MAX.y, background_w / 6,
                 background_h / 4};
  asset_t *button_image = asset_make_image(STARTBTN_PATH, startbtn_box);
  asset_t *start_button = asset_make_button(startbtn_box, button_image, NULL,
                                            (button_handler_t)next_screen);
  list_add(op_screen_assets, start_button);

  // game screen's scene & assets
  screen_t *game_screen = list_get(state->screens, GAME_SCREEN_IDX);
  scene_t *game_scene = screen_get_scene(game_screen);
  list_t *game_assets = screen_get_body_assets(game_screen);

  input_img_asset(game_assets, BACKGROUND_PATH, MIN.x, MIN.y, background_w,
                  background_h);

  // players
  player_t *player1 = player_init(PLAYER1_NAME, RED_BULLET_PATH);
  player_t *player2 = player_init(PLAYER2_NAME, BLU_BULLET_PATH);

  // both ships
  body_t *shippy1 = make_spaceship(VEC_ZERO, player1);
  body_t *shippy2 = make_spaceship(VEC_ZERO, player2);
  state->shippy1 = shippy1;
  state->shippy2 = shippy2;
  body_set_centroid(shippy1, P1_RESET_POS);
  body_set_centroid(shippy2, P2_RESET_POS);

  scene_add_body(game_scene, shippy1);
  scene_add_body(game_scene, shippy2);

  asset_t *ship1_asset =
      asset_make_image_with_body(RED_SPACESHIP_PATH, shippy1);
  asset_t *ship2_asset =
      asset_make_image_with_body(BLU_SPACESHIP_PATH, shippy2);
  list_add(game_assets, ship1_asset);
  list_add(game_assets, ship2_asset);

  // asteroids
  for (size_t r = 0; r < INIT_NUM_ASTEROIDS; r++) {
    add_asteroid(state);
  }

  // add obstacles
  body_t *metal1 = make_obstacle(METAL_WIDTH, METAL_HEIGHT, METAL1_POS,
                                 INFINITY, METAL_INFO);
  body_t *metal2 = make_obstacle(METAL_WIDTH, METAL_HEIGHT, METAL2_POS,
                                 INFINITY, METAL_INFO);
  scene_add_body(game_scene, metal1);
  scene_add_body(game_scene, metal2);
  asset_t *metal1_asset = asset_make_image_with_body(VERT_METAL_PATH, metal1);
  asset_t *metal2_asset = asset_make_image_with_body(VERT_METAL_PATH, metal2);
  list_add(game_assets, metal1_asset);
  list_add(game_assets, metal2_asset);

  // scoring icons
  input_img_asset(game_assets, HEALTH_PWRUP_PATH, MIN.x + 10, MIN.y + 10,
                  PWRUP_RADIUS * 4, PWRUP_RADIUS * 4);
  input_img_asset(game_assets, HEALTH_PWRUP_PATH, MAX.x - 80, MIN.y + 10,
                  PWRUP_RADIUS * 4, PWRUP_RADIUS * 4);

  input_img_asset(game_assets, POINTS_PATH, MAX.x - 80, MIN.y + 50,
                  PWRUP_RADIUS * 4, PWRUP_RADIUS * 4);
  input_img_asset(game_assets, POINTS_PATH, MIN.x + 10, MIN.y + 50,
                  PWRUP_RADIUS * 4, PWRUP_RADIUS * 4);

  input_img_asset(game_assets, POINTS_PATH, MAX.x - 80, MIN.y + 50,
                  PWRUP_RADIUS * 4, PWRUP_RADIUS * 4);

  char *health_str1 = player_return_health_str(player1);

  input_text_asset(health_str1, game_assets, HEALTH_SYSTEM_INFO, MIN.x + 50,
                   MIN.y + 15, TXT_RADIUS * 1.5, TXT_RADIUS,
                   list_size(game_assets) - 1);

  char *health_str2 = player_return_health_str(player2);

  input_text_asset(health_str2, game_assets, HEALTH_SYSTEM_INFO, MAX.x - 50,
                   MIN.y + 15, TXT_RADIUS * 1.5, TXT_RADIUS,
                   list_size(game_assets) - 1);

  char *points_str1 = player_return_points_str(player1);
  input_text_asset(points_str1, game_assets, POINTS_SYSTEM_INFO, MAX.x - 50,
                   MIN.y + 55, TXT_RADIUS, TXT_RADIUS,
                   list_size(game_assets) - 1);

  char *points_str2 = player_return_points_str(player2);
  input_text_asset(points_str2, game_assets, POINTS_SYSTEM_INFO, MIN.x + 50,
                   MIN.y + 55, TXT_RADIUS, TXT_RADIUS,
                   list_size(game_assets) - 1);

  asset_t *shooting_sfx = asset_make_sfx(SHOOTING_SFX);
  list_add(game_assets, shooting_sfx);
  state->shoot_sfx = shooting_sfx;

  asset_t *bg_music = asset_make_music(BG_MUSIC);
  list_add(game_assets, bg_music);

  asset_play_music(bg_music, 80);

  state->dead_asters = list_init(10, NULL);

  sdl_on_key((key_handler_t)on_key);
  return state;
}

bool emscripten_main(state_t *state) {
  if (game_is_over(state)) {
    return true;
  }

  double dt = time_since_last_tick();

  screen_t *screen = list_get(state->screens, state->screen_idx);
  scene_t *scene = screen_get_scene(screen);
  list_t *assets = screen_get_body_assets(screen);

  if (state->screen_idx == GAME_SCREEN_IDX) {
    player_t *player1 = body_get_info(state->shippy1);
    player_t *player2 = body_get_info(state->shippy2);
    player_change_bul_delta_t(player1, dt);
    player_change_bul_delta_t(player2, dt);
    state->pwrup_delta_t += dt;
    state->event_delta_t += dt;
    state->asteroid_delta_t += dt;

    if (!player_is_alive(player1)) {
      player_increment_death_time(player1, dt);
      body_set_centroid(state->shippy1, P1_RESET_POS);
      asset_t *new_asset =
          asset_make_image_with_body(RED_GHOST_SHIPPY_PATH, state->shippy1);
      list_set(assets, new_asset, 1);
    } else {
      asset_t *new_asset =
          asset_make_image_with_body(RED_SPACESHIP_PATH, state->shippy1);
      list_set(assets, new_asset, 1);
    }

    if (!player_is_alive(player2)) {
      player_increment_death_time(player2, dt);
      body_set_centroid(state->shippy2, P2_RESET_POS);
      asset_t *new_asset =
          asset_make_image_with_body(BLU_GHOST_SHIPPY_PATH, state->shippy2);
      list_set(assets, new_asset, 2);
    } else {
      asset_t *new_asset =
          asset_make_image_with_body(BLU_SPACESHIP_PATH, state->shippy2);
      list_set(assets, new_asset, 2);
    }

    if (state->pwrup_delta_t >= PWRUP_SPAWN_TIME) {
      state->pwrup_delta_t = 0;
      add_item(state, PWRUP_INFO);
    }

    if (state->asteroid_delta_t >= ASTEROID_SPAWN_TIME) {
      state->asteroid_delta_t = 0;
      body_t *aster1 = add_asteroid(state);
      body_t *aster2 = add_asteroid(state);
      for (size_t i = 0; i < scene_bodies(scene); i++) {
        body_t *maybe_blk_hole = scene_get_body(scene, i);
        if (body_get_info(maybe_blk_hole) == BLK_HOLE_PATH) {
          create_newtonian_gravity(scene, BLK_HOLE_GRAV, aster1,
                                   maybe_blk_hole);
          create_newtonian_gravity(scene, BLK_HOLE_GRAV, aster2,
                                   maybe_blk_hole);
        }
      }
    }

    update_score_display(player1, assets);
    update_score_display(player2, assets);

    if (state->event_delta_t >= EVENT_SPAWN_TIME) {
      state->event_delta_t = 0;
      double rand = rand_double(0, 1);
      if (rand < CHANCE_EVENT_SPAWN) {
        add_item(state, EVENT_INFO);
      }
    }

    for (size_t i = 0; i < scene_bodies(scene); i++) {
      body_t *body = scene_get_body(scene, i);
      wrap_edges(state, body);
      if (body_get_info(body) == METAL_INFO) {
        obstacle_collisions(state->shippy1, body);
        obstacle_collisions(state->shippy2, body);
      }
    }

    for (size_t i = 0; i < list_size(state->dead_asters); i++) {
      dead_aster_t *aster = list_get(state->dead_asters, i);
      aster->dt += dt;
      if (aster->change_num == 4 && aster->dt >= 1) {
        dead_aster_t *dead_aster = list_remove(state->dead_asters, i);

        for (size_t j = 0; j < list_size(assets); j++) {
          if (list_get(assets, j) == dead_aster->asset) {
            list_remove(assets, j);
            break;
          }
        }

        dead_aster_free(dead_aster);
      } else {
        dead_aster_change(aster, assets);
      }
    }
  }

  sdl_clear();
  for (size_t i = 0; i < list_size(assets); i++) {
    asset_t *asset = list_get(assets, i);
    if (asset) {
      asset_render(asset);
    }
  }
  sdl_show();

  list_t *aster_pos = scene_tick(scene, assets, dt);

  for (size_t i = 0; i < list_size(aster_pos); i++) {
    vector_t *pos = list_get(aster_pos, i);
    vector_t new_pos = {pos->x, pos->y};

    body_t *body = make_obstacle((OBS_WIDTHS.x + OBS_WIDTHS.y) / 2,
                                 OBSTACLE_HEIGHT, new_pos, 1, DEAD_ASTER_INFO);
    scene_add_body(scene, body);
    asset_t *asset = asset_make_image_with_body(EXPLOSION1_PATH, body);
    list_add(assets, asset);

    dead_aster_t *dead_aster = dead_aster_init(0, 1, body, asset);

    list_t *dead_asters = state->dead_asters;
    list_add(dead_asters, dead_aster);
  }

  list_free(aster_pos);

  return false;
}

void emscripten_free(state_t *state) {
  list_free(state->screens);
  list_free(state->dead_asters);
  asset_cache_destroy();
  free(state);
}