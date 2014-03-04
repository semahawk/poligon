/*
 *
 * poligon.c
 *
 * Created at:  Mon  3 Mar 19:42:32 2014 19:42:32
 *
 * Author:  Szymon Urbaś <szymon.urbas@aol.com>
 *
 * License:  please visit the LICENSE file for details.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>
#include <errno.h>
#include <math.h>

#include <SDL.h>
#include <sge.h>

#include "poligon.h"

/* d'oh */
const static unsigned WINDOW_HEIGHT = 400;
const static unsigned WINDOW_WIDTH  = 647;

/* static function forwards */
static void *handle_so(struct unit_desc *desc, char *fname);
static double side_len(unsigned sides);
static unsigned circum_rad(unsigned sides);

/* some useful macros */
#define R(c) (((c) & 0xff0000) >> 16)
#define G(c) (((c) & 0x00ff00) >> 8)
#define B(c) (((c) & 0x0000ff))
#define RGB(r,g,b) (((r) << 16) | ((g) << 8) | (b))

#define RAD(deg) ((deg) * M_PI / 180.0)
#define DEG(rad) ((rad) * 180.0 / M_PI)

/* an array of functions to which the control would be given upon seeing one of
 * the listed file extensions */
static struct file_handlers file_handlers[] = {
  { "so", handle_so },
  { NULL, NULL      }
};

int main(int argc, char *argv[])
{
  /* the program's name, d'oh */
  const char *progname = argv[0];
  /* the whole file's name */
  char *fname;
  /* here the file's extension will be stored */
  char fext[FEXT_MAX_SIZE + 1] = { 0 };
  /* the user's unit */
  struct unit unit = { 0 };
  /* the unit's description (it's color, etc) */
  struct unit_desc desc = { 0 };

  /* make sure there is at least one argument supplied */
  if (argc < 2){
    fprintf(stderr, "usage: %s <file>\n", progname);
    return 1;
  }

  fname = argv[1];

  /* fetch the extension, and store it in `fext' */
  {
    int i = strlen(fname) - 1; /* minus one to not start at the '\0' */
    /* search for the first dot, starting from the end of the string */
    int j = 0; /* puts the characters into the `fext' array */

    /* set `i' to the last dot's position */
    for (; fname[i] != '.' && i >= 0; i--)
      ;

    if (fname[i] == '.'){
      /* we found a dot, let's see if it would fit into `fext' */
      /* `i + 1' because `i' points at the dot, not at the very first character
       * after it */
      if (strlen(fname) - (i + 1) > FEXT_MAX_SIZE){
        fprintf(stderr, "%s: file extension too long (max. %u chars)\n", progname, FEXT_MAX_SIZE);
        return 1;
      }

      /* see if there is anything after the dot (like, if it's not something
       * like `filename.') */
      if (fname[i + 1] == '\0'){
        fprintf(stderr, "%s: file extension empty\n", progname);
        return 1;
      }
    } else {
      /* we haven't seen any dots, that's a bummer */
      fprintf(stderr, "%s: file extension not found\n", progname);
      return 1;
    }

    /* here, everything should be all set and ready to roll */
    for (i++ /* skip over the dot */; j < FEXT_MAX_SIZE; j++, i++)
      fext[j] = fname[i];
  }

  /* call the function that will handle the file, based on it's extension */
  {
    struct file_handlers *p = file_handlers;
    int found = 0;

    for (; p->fext != NULL && p->handler != NULL; p++){
      if (!strcmp(p->fext, fext)){
        found = 1;
        break;
      }
    }

    if (found){
      /* we know the extension, and can handle it */
      if (p->handler(&desc, fname) == NULL){
        fprintf(stderr, "%s: %s: %s\n", progname, fname, strerror(errno));
        return 1;
      }
    } else {
      /* oops! */
      fprintf(stderr, "%s: file extension `%s' not supported\n", progname, fext);
      return 1;
    }
  }

  /* TODO: safety checks for the `desc' values */
  unit.desc = desc;

  /* initialize the SDL */
  SDL_Init(SDL_INIT_EVERYTHING);
  /* set the title */
  SDL_WM_SetCaption("Poligon", NULL);
  /* create the window */
  SDL_Surface *screen = SDL_SetVideoMode(WINDOW_WIDTH, WINDOW_HEIGHT, 0, SDL_HWSURFACE | SDL_DOUBLEBUF);

  SDL_Event event;
  int running = 1;

  {
    sge_Randomize();

    Sint16 xs[UNIT_MAX_SIDES];
    Sint16 ys[UNIT_MAX_SIDES];
    int i;
    unsigned cr, ca, xmov, ymov;
    unsigned rot = 0 /* unit.rot */;

    /* generate random coords */
    unit.x = screen->w / 2;
    unit.y = screen->h / 2;

    for (i = 0; i < unit.desc.sides; i++){
      /* calculate the circum radius */
      cr = circum_rad(unit.desc.sides);
      /* calculate the central angle */
      ca = floor(360 / unit.desc.sides);
      /* calculate the alpha angle */
      xmov = cr * sin(RAD(rot));
      ymov = cr * cos(RAD(rot));

      xs[i] = unit.x - xmov;
      ys[i] = unit.y + ymov;

      printf("#%u (%u, %u)\n", i, unit.x - xmov, unit.y + ymov);

      rot += ca;
    }

    sge_AAFilledPolygon(screen, unit.desc.sides, xs, ys, unit.desc.color);
  }

  while (running){
    if (SDL_PollEvent(&event)){
      switch (event.type){
        case SDL_QUIT:
          running = 0;
          break;
      }
    }

    /* update the screen */
    SDL_UpdateRect(screen, 0, 0, 0, 0);
  }

  /* clean up after SDL */
  SDL_Quit();

  return 0;
}

/*
 * Calculates the circum radius for the <sides>ed polygon
 */
static unsigned circum_rad(unsigned sides)
{
  unsigned a = side_len(sides);

  switch (sides){
    case 3:
      return floor(a * sqrt(3) / 3);
    case 4:
      return floor(a * sqrt(2) / 2);
    case 5:
      return floor((2 * a) / sqrt(2 * (5 - sqrt(5))));
    case 6:
      return a;
    case 7:
      return floor(a * 1.15238);
    case 10:
      return floor((a * (1 + sqrt(5))) / 2);
    case 12:
      return floor(((a * sqrt(2)) * (sqrt(3) + 1)) / 2);
    default:
      return 0;
  }
}

/*
 * Given the number of sides, this function calculates length of one side.
 */
static double side_len(unsigned sides)
{
  switch (sides){
    case 3:
      return floor(2 * UNIT_HEIGHT / sqrt(3));
    case 4:
      return UNIT_HEIGHT;
    case 5:
      return floor(UNIT_HEIGHT / 1.53884);
    case 6:
      return floor(UNIT_HEIGHT / sqrt(3));
    case 7:
      return floor(UNIT_HEIGHT / 2.19064);
    case 8:
      return floor(UNIT_HEIGHT / 2.41421);
    case 9:
      return floor(UNIT_HEIGHT / 2.83564);
    case 10:
      return floor(UNIT_HEIGHT / 3.07768);
    case 11:
      return floor(UNIT_HEIGHT / 3.47758);
    case 12:
      return floor(UNIT_HEIGHT / (2 + sqrt(3)));
    default:
      return 0;
  }
}

static void *handle_so(struct unit_desc *desc, char *fname)
{
  void *lib, *p;
  struct unit_desc (*init)(void);

  if ((lib = dlopen(fname, RTLD_LAZY)) == NULL){
    return NULL;
  }

  dlerror(); /* clear any existing error */

  if ((p = dlsym(lib, "init")) != NULL){
    struct unit_desc ret;
    init = (struct unit_desc (*)(void))p;
    ret = init();

    if (p){
      *desc = ret;
    }
  }

  /* TODO: fetch the functions */

  return lib;
}

/*
 * vi: ft=c:ts=2:sw=2:expandtab
 */

