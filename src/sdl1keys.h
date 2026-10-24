// sdl1keys.h: SDL 1.2 key codes on SDL 2
//
// data/keymap.cfg binds actions to SDL 1.2 key codes, so SDL 2 key events are translated
// back to them (sdl1keycode() in main.cpp) and the SDLK_ names used by the engine get the
// SDL 1.2 values.

#ifndef SDL1KEYS_H
#define SDL1KEYS_H

int sdl1keycode(const SDL_Keysym &keysym);

#undef SDLK_BACKSPACE
#undef SDLK_TAB
#undef SDLK_RETURN
#undef SDLK_ESCAPE
#undef SDLK_UP
#undef SDLK_DOWN
#undef SDLK_LEFT
#define SDLK_BACKSPACE  8
#define SDLK_TAB        9
#define SDLK_RETURN     13
#define SDLK_ESCAPE     27
#define SDLK_UP         273
#define SDLK_DOWN       274
#define SDLK_LEFT       276

#endif
