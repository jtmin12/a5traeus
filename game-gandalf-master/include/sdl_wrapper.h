#ifndef __SDL_WRAPPER_H__
#define __SDL_WRAPPER_H__

#include "color.h"
#include "list.h"
#include "polygon.h"
#include "scene.h"
#include "state.h"
#include "vector.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_keyboard.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_ttf.h>
#include <stdbool.h>

// Values passed to a key handler when the given arrow key is pressed
typedef enum {
  LEFT_ARROW = 1,
  UP_ARROW = 2,
  RIGHT_ARROW = 3,
  DOWN_ARROW = 4,
  SPACE_BAR = 5,
} arrow_key_t;

/**
 * The possible types of key events.
 * Enum types in C are much more primitive than in Java; this is equivalent to:
 * typedef unsigned int KeyEventType;
 * #define KEY_PRESSED 0
 * #define KEY_RELEASED 1
 */
typedef enum { KEY_PRESSED, KEY_RELEASED } key_event_type_t;

/**
 * A keypress handler.
 * When a key is pressed or released, the handler is passed its char value.
 * Most keys are passed as their char value, e.g. 'a', '1', or '\r'.
 * Arrow keys have the special values listed above.
 *
 * @param key a character indicating which key was pressed
 * @param type the type of key event (KEY_PRESSED or KEY_RELEASED)
 * @param held_time if a press event, the time the key has been held in seconds
 */
typedef void (*key_handler_t)(char key, key_event_type_t type, double held_time,
                              void *state);

/**
 * A mouse handler.
 * When a mouse button is pressed or released, the handler deals with it.
 *
 * @param x location of the mouse click
 * @param y location of the mouse click
 */
typedef void (*mouse_handler_t)(void *state, double x, double y);

/**
 * Initializes the SDL window and renderer.
 * Must be called once before any of the other SDL functions.
 *
 * @param min the x and y coordinates of the bottom left of the scene
 * @param max the x and y coordinates of the top right of the scene
 */
void sdl_init(vector_t min, vector_t max);

/**
 * Processes all SDL events and returns whether the window has been closed.
 * This function must be called in order to handle inputs.
 *
 * @return true if the window was closed, false otherwise
 */
bool sdl_is_done(void *state);

/**
 * Clears the screen. Should be called before drawing polygons in each frame.
 */
void sdl_clear(void);

/**
 * Draws a polygon from the given list of vertices and a color.
 *
 * @param poly a struct representing the polygon
 * @param color the color used to fill in the polygon
 */
void sdl_draw_polygon(polygon_t *poly, rgb_color_t *color);

/**
 * Displays the rendered frame on the SDL window.
 * Must be called after drawing the polygons in order to show them.
 */
void sdl_show(void);

/**
 * Draws all bodies in a scene.
 * This internally calls sdl_clear(), sdl_draw_polygon(), and sdl_show(),
 * so those functions should not be called directly.
 *
 * @param scene the scene to draw
 * @param aux an additional body to draw (can be NULL if no additional bodies)
 */
void sdl_render_scene(scene_t *scene, void *aux);

/**
 * Registers a function to be called every time a key is pressed.
 * Overwrites any existing handler.
 *
 * Example:
 * ```
 * void on_key(char key, key_event_type_t type, double held_time) {
 *     if (type == KEY_PRESSED) {
 *         switch (key) {
 *             case 'a':
 *                 printf("A pressed\n");
 *                 break;
 *             case UP_ARROW:
 *                 printf("UP pressed\n");
 *                 break;
 *         }
 *     }
 * }
 * int main(void) {
 *     sdl_on_key(on_key);
 *     while (!sdl_is_done());
 * }
 * ```
 *
 * @param handler the function to call with each key press
 */
void sdl_on_key(key_handler_t handler);

/**
 * Registers a function to be called every time a mouse button is pressed.
 * Overwrites any existing handler.
 *
 * @param handler the function to call with each mouse button press
 */
void sdl_on_click(mouse_handler_t handler);

/**
 * Gets the amount of time that has passed since the last time
 * this function was called, in seconds.
 *
 * @return the number of seconds that have elapsed
 */
double time_since_last_tick(void);

/**
 * Returns the SDL_Rect that encompasses the entire body in minimal space
 *
 * @param body body that is to be encompassed by the constructed SDL_Rect
 *
 * @return SDL_Rect that has spans minimally to cover entire body's required
 * space
 */
SDL_Rect sdl_get_bounding_box(body_t *body);

/**
 * Returns the text texture
 *
 * @param TTF_Font the opened TTF_Font to use with msg
 * @param msg the text message to turn into a texture
 * @param color the color to display message in
 *
 * @return texture of the message in font with font color applied
 */
SDL_Texture *sdl_load_text_texture(TTF_Font *font, const char *msg,
                                   rgb_color_t color);

/**
 * Returns the image texture
 *
 * @param IMG_PATH the file path of the image
 *
 * @return texture of the image loaded
 */
SDL_Texture *sdl_load_img_texture(const char *IMG_PATH);

/**
 * Renders the texture in the SDL_Rect box
 *
 * @param texture the image's texture
 * @param SDL_Rect with the location and dimensions for texture
 */
void sdl_render_texture(SDL_Texture *texture, SDL_Rect bounding_box);

/**
 * Renders the texture in the SDL_Rect box but rotated
 *
 * @param texture the image's texture
 * @param SDL_Rect with the location and dimensions for texture
 * @param angle in radians to rotate SDL_Rect about its center clockwise
 */
void sdl_render_rotate_texture(SDL_Texture *texture, SDL_Rect bounding_box,
                               double angle);

/**
 * Renders the texture in the SDL_Rect box but rotated
 *
 * @param texture the image's texture
 * @param SDL_Rect with the location and dimensions for texture
 * @param angle in radians to rotate SDL_Rect about its center clockwise
 */
void sdl_render_rotate_texture(SDL_Texture *texture, SDL_Rect bounding_box,
                               double angle);

#endif // #ifndef __SDL_WRAPPER_H__
