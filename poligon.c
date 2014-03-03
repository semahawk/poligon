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

#include <SDL.h>
#include <SDL_gfxPrimitives.h>

#include "poligon.h"

/* d'oh */
const static unsigned WINDOW_HEIGHT = 400;
const static unsigned WINDOW_WIDTH  = 647;

/* some useful macros */
#define R(c) (((c) & 0xff0000) >> 16)
#define G(c) (((c) & 0x00ff00) >> 8)
#define B(c) (((c) & 0x0000ff))

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
  /* the, hm... control unit? */
  struct ctl ctl = { 0 };

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
      if (p->handler(&ctl, fname) == NULL){
        fprintf(stderr, "%s: %s: %s\n", progname, fname, strerror(errno));
        return 1;
      }
    } else {
      /* oops! */
      fprintf(stderr, "%s: file extension `%s' not supported\n", progname, fext);
      return 1;
    }
  }

  /* TODO: safety checks for the `ctl' values */

  /* initialize the SDL */
  SDL_Init(SDL_INIT_VIDEO);
  /* set the title */
  SDL_WM_SetCaption("Sidecross", NULL);
  /* create the window */
  SDL_Surface *screen = SDL_SetVideoMode(WINDOW_WIDTH, WINDOW_HEIGHT, 0, SDL_HWSURFACE | SDL_DOUBLEBUF);

  SDL_Event event;
  int running = 1;

  while (running){
    if (SDL_PollEvent(&event)){
      switch (event.type){
        case SDL_QUIT:
          running = 0;
          break;
      }
    }

    trigonRGBA(screen, 500, 50, 550, 200, 600, 150,
        R(ctl.colour), G(ctl.colour), B(ctl.colour), 255);

    /* update the screen */
    SDL_UpdateRect(screen, 0, 0, 0, 0);
  }

  /* clean up after SDL */
  SDL_Quit();

  return 0;
}

void *handle_so(struct ctl *ctl, char *fname)
{
  void *lib, *p;
  struct ctl (*init)(void);

  if ((lib = dlopen(fname, RTLD_LAZY)) == NULL){
    return NULL;
  }

  dlerror(); /* clear any existing error */

  if ((p = dlsym(lib, "init")) != NULL){
    struct ctl ret;
    init = (struct ctl (*)(void))p;
    ret = init(); /* hooray `p' is reusable */

    if (p){
      *ctl = ret;
    }
  }

  /* TODO: fetch the functions */

  printf("sides: %u\n", ctl->sides);
  printf("colour: 0x%06x\n", ctl->colour);

  return lib;
}

/*
 * vi: ft=c:ts=2:sw=2:expandtab
 */

