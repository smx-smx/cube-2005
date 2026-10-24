// main.cpp: initialisation & main loop

#include "cube.h"

void cleanup(char *msg)         // single program exit point;
{
	stop();
    disconnect(true);
    writecfg();
    cleangl();
    cleansound();
    cleanupserver();
    SDL_ShowCursor(1);
    if(msg)
    {
        #ifdef WIN32
        MessageBox(NULL, msg, "cube fatal error", MB_OK|MB_SYSTEMMODAL);
        #else
        printf(msg);
        #endif
    };
    SDL_Quit();
    exit(1);
};

void quit()                     // normal exit
{
    writeservercfg();
    cleanup(NULL);
};

void fatal(char *s, char *o)    // failure exit
{
    sprintf_sd(msg)("%s%s (%s)\n", s, o, SDL_GetError());
    cleanup(msg);
};

void *alloc(int s)              // for some big chunks... most other allocs use the memory pool
{
    void *b = calloc(1,s);
    if(!b) fatal("out of memory!");
    return b;
};

#ifdef CUBE_NC4
int scr_w = 1280;               // the only mode of LG NetCast TVs
int scr_h = 720;
#else
int scr_w = 640;
int scr_h = 480;
#endif

#if SDL_MAJOR_VERSION >= 2
SDL_Window *screen = NULL;
static bool keyrepeaton = false;
static bool textinput = false;                  // characters come from SDL_TEXTINPUT, not from key codes

// SDL 2 key code -> SDL 1.2 key code, which data/keymap.cfg binds
int sdl1keycode(const SDL_Keysym &keysym)
{
    SDL_Keycode k = keysym.sym;
    if(k < 128) return k;                       // ASCII keys have the same codes
    if(k >= SDLK_F1 && k <= SDLK_F12) return 282 + (k - SDLK_F1);
    if(k >= SDLK_KP_1 && k <= SDLK_KP_9) return 257 + (k - SDLK_KP_1);
    switch(k)
    {
        case SDLK_KP_0: return 256;
        case SDLK_KP_PERIOD: return 266;
        case SDLK_KP_DIVIDE: return 267;
        case SDLK_KP_MULTIPLY: return 268;
        case SDLK_KP_MINUS: return 269;
        case SDLK_KP_PLUS: return 270;
        case SDLK_KP_ENTER: return 271;
        case SDLK_KP_EQUALS: return 272;
        // sdl1keys.h redefines SDLK_UP/DOWN/LEFT to the SDL 1.2 codes
        case SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_UP): return 273;
        case SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_DOWN): return 274;
        case SDLK_RIGHT: return 275;
        case SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_LEFT): return 276;
        case SDLK_INSERT: return 277;
        case SDLK_HOME: return 278;
        case SDLK_END: return 279;
        case SDLK_PAGEUP: return 280;
        case SDLK_PAGEDOWN: return 281;
        case SDLK_F13: return 294;
        case SDLK_F14: return 295;
        case SDLK_F15: return 296;
        case SDLK_NUMLOCKCLEAR: return 300;
        case SDLK_CAPSLOCK: return 301;
        case SDLK_SCROLLLOCK: return 302;
        case SDLK_RSHIFT: return 303;
        case SDLK_LSHIFT: return 304;
        case SDLK_RCTRL: return 305;
        case SDLK_LCTRL: return 306;
        case SDLK_RALT: return 307;
        case SDLK_LALT: return 308;
        case SDLK_RGUI: return 309;
        case SDLK_LGUI: return 310;
        case SDLK_MODE: return 313;
        case SDLK_HELP: return 315;
        case SDLK_PRINTSCREEN: return 316;
        case SDLK_SYSREQ: return 317;
        case SDLK_PAUSE: return 19;
        case SDLK_MENU: return 319;
        case SDLK_POWER: return 320;
        case SDLK_UNDO: return 322;
        case SDLK_AC_BACK: return 27;           // remote controls: Back is Escape
        default: return 0;
    }
}

// the console uses SDL 1.2 unicode values, derive them from the key code
static int keyunicode(const SDL_Keysym &keysym)
{
    SDL_Keycode k = keysym.sym;
    if(k < 32 || k >= 127) return k < 32 ? k : 0;
    if((keysym.mod & KMOD_SHIFT) && k >= 'a' && k <= 'z') return k - 'a' + 'A';
    return k;
}
#endif

void screenshot()
{
    SDL_Surface *image;
    SDL_Surface *temp;
    int idx;
    if(image = SDL_CreateRGBSurface(SDL_SWSURFACE, scr_w, scr_h, 24, 0x0000FF, 0x00FF00, 0xFF0000, 0))
    {
        if(temp  = SDL_CreateRGBSurface(SDL_SWSURFACE, scr_w, scr_h, 24, 0x0000FF, 0x00FF00, 0xFF0000, 0))
        {
            glReadPixels(0, 0, scr_w, scr_h, GL_RGB, GL_UNSIGNED_BYTE, image->pixels);
            for (idx = 0; idx<scr_h; idx++)
            {
                char *dest = (char *)temp->pixels+3*scr_w*idx;
                memcpy(dest, (char *)image->pixels+3*scr_w*(scr_h-1-idx), 3*scr_w);
                endianswap(dest, 3, scr_w);
            };
            sprintf_sd(buf)("screenshots/screenshot_%d.bmp", lastmillis);
            SDL_SaveBMP(temp, path(buf));
            SDL_FreeSurface(temp);
        };
        SDL_FreeSurface(image);
    };
};

COMMAND(screenshot, ARG_NONE);
COMMAND(quit, ARG_NONE);

void keyrepeat(bool on)
{
    #if SDL_MAJOR_VERSION >= 2
    keyrepeaton = on;                           // SDL 2 always repeats, repeats are dropped when off
    #else
    SDL_EnableKeyRepeat(on ? SDL_DEFAULT_REPEAT_DELAY : 0,
                             SDL_DEFAULT_REPEAT_INTERVAL);
    #endif
};

void textinputmode(bool on)
{
    #if SDL_MAJOR_VERSION >= 2
    if(!textinput) return;
    if(on) SDL_StartTextInput(); else SDL_StopTextInput();
    SDL_FlushEvent(SDL_TEXTINPUT);              // drop the text of the key that opened the console
    #endif
};

VARF(gamespeed, 10, 100, 1000, if(multiplayer()) gamespeed = 100);
VARP(minmillis, 0, 5, 1000);

int islittleendian = 1;
int framesinmap = 0;

int main(int argc, char **argv)
{    
    bool dedicated = false;
    #if SDL_MAJOR_VERSION >= 2
    int fs = SDL_WINDOW_FULLSCREEN, par = 0, uprate = 0, maxcl = 4;
    #else
    int fs = SDL_FULLSCREEN, par = 0, uprate = 0, maxcl = 4;
    #endif
    char *sdesc = "", *ip = "", *master = NULL, *passwd = "";
    islittleendian = *((char *)&islittleendian);
    #ifdef CUBE_NC4
    setvbuf(stdout, NULL, _IONBF, 0);           // cube.sh logs to a file, don't lose the end of it
    #endif

    #define log(s) conoutf("init: %s", s)
    log("sdl");
    
    for(int i = 1; i<argc; i++)
    {
        char *a = &argv[i][2];
        if(argv[i][0]=='-') switch(argv[i][1])
        {
            case 'd': dedicated = true; break;
            case 't': fs     = 0; break;
            case 'w': scr_w  = atoi(a); break;
            case 'h': scr_h  = atoi(a); break;
            case 'u': uprate = atoi(a); break;
            case 'n': sdesc  = a; break;
            case 'i': ip     = a; break;
            case 'm': master = a; break;
            case 'p': passwd = a; break;
            case 'c': maxcl  = atoi(a); break;
            default:  conoutf("unknown commandline option");
        }
        else conoutf("unknown commandline argument");
    };
    
    #if defined(_DEBUG) && SDL_MAJOR_VERSION < 2
    par = SDL_INIT_NOPARACHUTE;
    fs = 0;
    #endif

    #if SDL_MAJOR_VERSION >= 2
    SDL_SetHint(SDL_HINT_NO_SIGNAL_HANDLERS, "1");  // Ctrl-C quits, also the dedicated server
    #endif
    if(SDL_Init(SDL_INIT_TIMER|SDL_INIT_VIDEO|par)<0) fatal("Unable to initialize SDL");

    log("net");
    if(enet_initialize()<0) fatal("Unable to initialise network module");

    initclient();
    initserver(dedicated, uprate, sdesc, ip, master, passwd, maxcl);  // never returns if dedicated
      
    log("world");
    empty_world(7, true);

    log("video: sdl");
    if(SDL_InitSubSystem(SDL_INIT_VIDEO)<0) fatal("Unable to initialize SDL Video");

    log("video: mode");
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    #if SDL_MAJOR_VERSION >= 2
    #ifdef CUBE_GLES
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
    #endif
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
    screen = SDL_CreateWindow("cube engine", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, scr_w, scr_h,
                              SDL_WINDOW_OPENGL|fs);
    if(!screen || !SDL_GL_CreateContext(screen)) fatal("Unable to create OpenGL screen");
    SDL_GetWindowSize(screen, &scr_w, &scr_h);
    // keyboard layouts need SDL_TEXTINPUT; the nc4 driver (TV remote) only sends key events
    textinput = strcmp(SDL_GetCurrentVideoDriver(), "nc4")!=0;
    SDL_StopTextInput();

    log("video: misc");
    if(SDL_SetRelativeMouseMode(SDL_TRUE)<0) conoutf("no relative mouse mode: %s", (size_t)SDL_GetError());
    #else
    if(SDL_SetVideoMode(scr_w, scr_h, 0, SDL_OPENGL|fs)==NULL) fatal("Unable to create OpenGL screen");

    log("video: misc");
    SDL_WM_SetCaption("cube engine", NULL);
    SDL_WM_GrabInput(SDL_GRAB_ON);
    #endif
    keyrepeat(false);
    SDL_ShowCursor(0);

    log("gl");
    gl_init(scr_w, scr_h);

    log("basetex");
    int xs, ys;
    if(!installtex(2,  path(newstring("data/newchars.png")), xs, ys) ||
       !installtex(3,  path(newstring("data/martin/base.png")), xs, ys) ||
       !installtex(6,  path(newstring("data/martin/ball1.png")), xs, ys) ||
       !installtex(7,  path(newstring("data/martin/smoke.png")), xs, ys) ||
       !installtex(8,  path(newstring("data/martin/ball2.png")), xs, ys) ||
       !installtex(9,  path(newstring("data/martin/ball3.png")), xs, ys) ||
       !installtex(4,  path(newstring("data/explosion.jpg")), xs, ys) ||
       !installtex(5,  path(newstring("data/items.png")), xs, ys) ||
       !installtex(1,  path(newstring("data/crosshair.png")), xs, ys)) fatal("could not find core textures (hint: run cube from the parent of the bin directory)");

    log("sound");
    initsound();

    log("cfg");
    newmenu("frags\tpj\tping\tteam\tname");
    newmenu("ping\tplr\tserver");
    exec("data/keymap.cfg");
    exec("data/menus.cfg");
    exec("data/prefabs.cfg");
    exec("data/sounds.cfg");
    exec("servers.cfg");
    if(!execfile("config.cfg")) execfile("data/defaults.cfg");
    exec("autoexec.cfg");

    log("localconnect");
    localconnect();
    changemap("metl3");		// if this map is changed, also change depthcorrect()
    
    log("mainloop");
    int ignore = 5;
    for(;;)
    {
        int millis = SDL_GetTicks()*gamespeed/100;
        if(millis-lastmillis>200) lastmillis = millis-200;
        else if(millis-lastmillis<1) lastmillis = millis-1;
        if(millis-lastmillis<minmillis) SDL_Delay(minmillis-(millis-lastmillis));
        cleardlights();
        updateworld(millis);
        if(!demoplayback) serverslice((int)time(NULL), 0);
        static float fps = 30.0f;
        fps = (1000.0f/curtime+fps*50)/51;
        computeraytable(player1->o.x, player1->o.y);
        readdepth(scr_w, scr_h);
        #if SDL_MAJOR_VERSION >= 2
        SDL_GL_SwapWindow(screen);
        #else
        SDL_GL_SwapBuffers();
        #endif
        extern void updatevol(); updatevol();
        if(framesinmap++<5)	// cheap hack to get rid of initial sparklies, even when triple buffering etc.
        {
			player1->yaw += 5;
			gl_drawframe(scr_w, scr_h, fps);
			player1->yaw -= 5;
        };
        gl_drawframe(scr_w, scr_h, fps);
        SDL_Event event;
        int lasttype = 0, lastbut = 0;
        while(SDL_PollEvent(&event))
        {
            switch(event.type)
            {
                case SDL_QUIT:
                    quit();
                    break;

                case SDL_KEYDOWN: 
                case SDL_KEYUP: 
                    #if SDL_MAJOR_VERSION >= 2
                    if(event.key.repeat && !keyrepeaton) break;
                {
                    int cooked = keyunicode(event.key.keysym);
                    if(textinput && cooked>=32) cooked = 0;     // arrives as SDL_TEXTINPUT
                    keypress(sdl1keycode(event.key.keysym), event.key.state==SDL_PRESSED, cooked);
                    break;
                }

                case SDL_TEXTINPUT:
                    // the console font is ASCII only
                    for(const char *c = event.text.text; *c; c++) if(*c>=32 && *c<127) keypress(0, true, *c);
                    #else
                    keypress(event.key.keysym.sym, event.key.state==SDL_PRESSED, event.key.keysym.unicode);
                    #endif
                    break;

                case SDL_MOUSEMOTION:
                    if(ignore) { ignore--; break; };
                    mousemove(event.motion.xrel, event.motion.yrel);
                    break;

                case SDL_MOUSEBUTTONDOWN:
                case SDL_MOUSEBUTTONUP:
                    if(lasttype==event.type && lastbut==event.button.button) break; // why?? get event twice without it
                    #if SDL_MAJOR_VERSION >= 2
                    // SDL 1.2 numbers: 4/5 are the wheel, X1/X2 come after them
                    keypress(-(event.button.button>=SDL_BUTTON_X1 ? event.button.button+2 : event.button.button),
                             event.button.state!=0, 0);
                    #else
                    keypress(-event.button.button, event.button.state!=0, 0);
                    #endif
                    lasttype = event.type;
                    lastbut = event.button.button;
                    break;

                #if SDL_MAJOR_VERSION >= 2
                case SDL_MOUSEWHEEL:
                {
                    // SDL 1.2 reported the wheel as buttons 4 (up) and 5 (down), pressed and released
                    int y = event.wheel.direction==SDL_MOUSEWHEEL_FLIPPED ? -event.wheel.y : event.wheel.y;
                    if(!y) break;
                    int code = y>0 ? -4 : -5;
                    keypress(code, true, 0);
                    keypress(code, false, 0);
                    break;
                }
                #endif
            };
        };
    };
    quit();
    return 1;
};


