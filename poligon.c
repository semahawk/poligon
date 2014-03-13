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

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>
#include <errno.h>
#include <math.h>
#include <limits.h>

#include <SDL.h>
#include <sge.h>

#include "poligon.h"

/* d'oh */
const static unsigned WINDOW_HEIGHT = 400;
const static unsigned WINDOW_WIDTH  = 647;

/* the binary's name */
static char *progname;

/* static function forwards */
static void *nmalloc(size_t size);

static void *handle_so(struct unit *unit, struct unit_desc *desc, char *fname);

static unsigned side_len(unsigned sides);
static unsigned circum_rad(unsigned sides);
static unit_t calcid(struct unit *unit);

static void draw_unit(SDL_Surface *screen, struct unit *unit);
static void push_unit(struct unit *unit);
static struct unit *get_unit(unit_t unit_id);
static void free_unit_list(void);

/* some useful macros */
#define R(c) (((c) & 0xff0000) >> 16)
#define G(c) (((c) & 0x00ff00) >> 8)
#define B(c) (((c) & 0x0000ff))
#define RGB(r,g,b) (((r) << 16) | ((g) << 8) | (b))

#define RAD(deg) ((deg) * M_PI / 180.0)
#define DEG(rad) ((rad) * 180.0 / M_PI)

/* a singly linked list of the units */
struct unit_list {
  struct unit *unit;
  struct unit_list *next;
} *unit_list = NULL;

/* an array of functions to which the control would be given upon seeing one of
 * the listed file extensions */
static struct file_handlers file_handlers[] = {
  { "so", handle_so },
  { NULL, NULL      }
};

/* a little wrapper around malloc */
static void *nmalloc(size_t size)
{
  /* {{{ */
  void *p = malloc(size);

  if (p == NULL){
    fprintf(stderr, "%s: malloc: failed to allocate %lu bytes\n", progname, size);
    exit(1);
  }

  return p;
  /* }}} */
}

int main(int argc, char *argv[])
{
  /* the program's name, d'oh */
  progname = argv[0];
  /* the whole file's name */
  char *fname;
  /* here the file's extension will be stored */
  char fext[FEXT_MAX_SIZE + 1] = { 0 };
  /* the unit's description (it's color, etc) */
  struct unit_desc desc = {{ 0 }};
  /* the user's unit */
  struct unit *unit = nmalloc(sizeof(struct unit));

  /* make sure there is at least one argument supplied */
  if (argc < 2){
    fprintf(stderr, "usage: %s <file>\n", progname);
    return 1;
  }

  fname = argv[1];

  /* fetch the extension, and store it in `fext' */
  {
    /* {{{ */
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
    /* }}} */
  }

  /* call the function that will handle the file, based on it's extension */
  {
    /* {{{ */
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
      if (p->handler(unit, &desc, fname) == NULL){
        return 1;
      }
    } else {
      /* oops! */
      fprintf(stderr, "%s: file extension `%s' not supported\n", progname, fext);
      return 1;
    }
    /* }}} */
  }

  /* TODO: safety checks for the `desc' values */
  unit->desc = desc;
  /* set the defaults */
  unit->hp = UNIT_MAX_HP / 2;
  /* give the unit an id */
  unit->id = calcid(unit);
  /* push the unit onto the `unit_list' */
  push_unit(unit);
  /* call the `fetch' function to give the user his unit's id */
  unit->fun.fetch(unit->id);

  /* initialize the SDL */
  SDL_Init(SDL_INIT_EVERYTHING);
  /* set the title */
  SDL_WM_SetCaption("Poligon", NULL);
  /* create the window */
  SDL_Surface *screen = SDL_SetVideoMode(WINDOW_WIDTH, WINDOW_HEIGHT, 0, SDL_HWSURFACE | SDL_DOUBLEBUF);

  SDL_Event event;
  int running = 1;

  /* put the unit in the middle of the screen */
  unit->x = screen->w / 2;
  unit->y = screen->h / 2;

  while (running){
    /* fill the background */
    SDL_FillRect(screen, NULL, 0x111111);

    if (SDL_PollEvent(&event)){
      switch (event.type){
        case SDL_QUIT:
          running = 0;
          break;
      }
    }

    draw_unit(screen, unit);
    unit->hp += sge_Random(-3, 3);
    /*unit->rot--;*/

    /* update the screen */
    SDL_UpdateRect(screen, 0, 0, 0, 0);
  }

  /* clean up after SDL */
  SDL_Quit();
  /* clean up after ourselves */
  free_unit_list();

  return 0;
}

/*
 * Calculate a (more or less) unique id for the given <unit>.
 * The result depends on the <unit>s name, it's color and it's number of sides.
 * The calculations were.. blindwritten. There is no logic behind them; I just
 * wrote it like this, and it seems to work :)
 */
static unit_t calcid(struct unit *unit)
{
  char *p = unit->desc.name;
  unit_t ret = 64;

  for (; *p != '\0'; p++){
    ret += *p * R(unit->desc.color) * G(unit->desc.color) & B(unit->desc.color);
    ret <<= unit->desc.sides % UNIT_MAX_SIDES;
    ret += *p ^ B(unit->desc.color) + G(unit->desc.color);
    ret *= *p * unit->desc.sides + R(unit->desc.color) * G(unit->desc.color);
  }

  /* TODO: make the modulo more flexible (to depend on size of `unit_t') */
  return ret % UINT_MAX;
}

/*
 * 'Pushes' a given <unit> onto the linked list `units'.
 */
static void push_unit(struct unit *unit)
{
  struct unit_list *l = malloc(sizeof(struct unit_list));

  if (!l){
    perror("malloc");
    exit(1);
  }

  l->unit = unit;
  l->next = unit_list;
  unit_list = l;
}

/*
 * Frees the whole `units' linked list.
 *   NOT including the units; they don't get freed.
 */
static void free_unit_list(void)
{
  struct unit_list *curr = unit_list,
                   *next = NULL;

  for (; curr != NULL; curr = next){
    next = curr->next;
    free(curr);
  }
}

/*
 * Return a `struct unit' associated with the given <id>.
 */
static struct unit *get_unit(unit_t unit_id)
{
  struct unit_list *p = unit_list;

  for (; p != NULL; p = p->next){
    if (p->unit->id == unit_id){
      return p->unit;
    }
  }

  return NULL;
}

/*
 * Draws the given <unit> on the <screen>.
 */
static void draw_unit(SDL_Surface *screen, struct unit *unit)
{
  Sint16 xs[UNIT_MAX_SIDES];
  Sint16 ys[UNIT_MAX_SIDES];
  int i;
  unsigned cr, ca, xmov, ymov;
  unsigned rot = unit->rot % 360;

  for (i = 0; i < unit->desc.sides; i++){
    /* calculate the circum radius */
    cr = circum_rad(unit->desc.sides);
    /* calculate the central angle */
    ca = floor(360 / unit->desc.sides);
    /* calculate the alpha angle */
    xmov = cr * sin(RAD(rot));
    ymov = cr * cos(RAD(rot));

    xs[i] = unit->x - xmov;
    ys[i] = unit->y + ymov;

    rot += ca;
  }

  /* draw the unit */
  sge_AAFilledPolygon(screen, unit->desc.sides, xs, ys, unit->desc.color);

  unsigned loss = UNIT_HEIGHT - (UNIT_HEIGHT * unit->hp / UNIT_MAX_HP);

  /* draw the health bar's background */
  /* top left corner */
  xs[0] = unit->x - (UNIT_HEIGHT / 2) - 2;
  ys[0] = unit->y + (UNIT_HEIGHT / 2) + 14;
  /* top right corner */
  xs[1] = unit->x + (UNIT_HEIGHT / 2) + 2;
  ys[1] = unit->y + (UNIT_HEIGHT / 2) + 14;
  /* bottom right corner */
  xs[2] = unit->x + (UNIT_HEIGHT / 2) + 2;
  ys[2] = unit->y + (UNIT_HEIGHT / 2) + 24;
  /* bottom left corner */
  xs[3] = unit->x - (UNIT_HEIGHT / 2) - 2;
  ys[3] = unit->y + (UNIT_HEIGHT / 2) + 24;
  /* draw it */
  sge_AAFilledPolygon(screen, 4, xs, ys, 0x222222);

  /* draw the health bar */
  if (unit->hp > 0){
    /* just to make sure */
    if (unit->hp >= UNIT_MAX_HP)
      loss = 0;

    /* top left corner */
    xs[0] = unit->x - (UNIT_HEIGHT / 2);
    ys[0] = unit->y + (UNIT_HEIGHT / 2) + 16;
    /* top right corner */
    xs[1] = unit->x + (UNIT_HEIGHT / 2) - loss;
    ys[1] = unit->y + (UNIT_HEIGHT / 2) + 16;
    /* bottom right corner */
    xs[2] = unit->x + (UNIT_HEIGHT / 2) - loss;
    ys[2] = unit->y + (UNIT_HEIGHT / 2) + 22;
    /* bottom left corner */
    xs[3] = unit->x - (UNIT_HEIGHT / 2);
    ys[3] = unit->y + (UNIT_HEIGHT / 2) + 22;
    /* draw it */
    sge_AAFilledPolygon(screen, 4, xs, ys, 0xde3333);
  }
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
    case 8:
      return floor(a * 1.4619);
    case 9:
      return floor(a * 1.4619);
    case 10:
      return floor((a * (1 + sqrt(5))) / 2);
    case 11:
      return floor(a * 1.77473);
    case 12:
      return floor(((a * sqrt(2)) * (sqrt(3) + 1)) / 2);
    default:
      return 0;
  }
}

/*
 * Given the number of sides, this function calculates length of one side.
 */
static unsigned side_len(unsigned sides)
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

static void *handle_so(struct unit *unit, struct unit_desc *desc, char *fname)
{
  void *lib, *p;

  assert(unit);

  if ((lib = dlopen(fname, RTLD_LAZY)) == NULL){
    fprintf(stderr, "%s: %s: %s\n", progname, fname, strerror(errno));
    return NULL;
  }

  dlerror(); /* clear any existing error */

  {
    initf_t init;

    if ((p = dlsym(lib, "init")) != NULL){
      struct unit_desc ret;
      init = (initf_t)p;
      ret = init();
      unit->fun.init = init;
      /* save the description */
      *desc = ret;
    } else {
      fprintf(stderr, "%s: %s: the `init' function was not found", progname, fname);
      return NULL;
    }
  }

  {
    fetchf_t fetch;

    if ((p = dlsym(lib, "fetch")) != NULL){
      fetch = (fetchf_t)p;
      unit->fun.fetch = fetch;
    }
  }

  /* TODO: fetch the functions */

  return lib;
}

void turn(unit_t unit, enum turn_direction dir, unsigned degrees)
{
  struct unit *u = get_unit(unit);

  assert(u);

  if (dir == LEFT)
    u->rot -= degrees;
  else
    u->rot += degrees;
}

/*
 * vi: ft=c:ts=2:sw=2:expandtab
 */

