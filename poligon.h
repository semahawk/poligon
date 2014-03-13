/*
 *
 * poligon.h
 *
 * Created at:  Mon  3 Mar 21:14:25 2014 21:14:25
 *
 * Author:  Szymon Urbaś <szymon.urbas@aol.com>
 *
 * License:  please visit the LICENSE file for details.
 *
 */

#ifndef POLIGON_H
#define POLIGON_H

#include <SDL.h>
#include <sge.h>

/* the maximum size of the file's extension */
#define FEXT_MAX_SIZE (4)
/* maximum health */
#define UNIT_MAX_HP (256)
/* the maximum number of one unit's sides */
#define UNIT_MAX_SIDES (12)
/* the minimum number of one unit's sides */
#define UNIT_MIN_SIDES (3)
/* unit's maximum name length */
#define UNIT_MAX_NAME_LEN (32)
/* the unit's height */
#define UNIT_HEIGHT (96)

/* the 'public' unit's type */
typedef unsigned int unit_t;
/* the 'user\'s functions' types */
typedef struct unit_desc (*initf_t)(void);
typedef void (*fetchf_t)(unit_t);

struct unit_desc {
  char name[UNIT_MAX_NAME_LEN + 1];
  unsigned sides;
  Uint32 color;
};

struct unit {
  unit_t id;
  unsigned x;
  unsigned y;
  unsigned rot; /* rotation (in degrees) */
  struct unit_desc desc; /* unit's description */
  int hp; /* hit points */
  /* user functions, so to speak */
  /* the functions that the user defines in his file, functions that get called by
   * the engine on certain situations, etc. */
  struct {
    initf_t init;
    fetchf_t fetch;
  } fun;
};

struct file_handlers {
  char *fext;
  void *(*handler)(struct unit *unit, struct unit_desc *desc, char *fname);
};

enum turn_direction {
  RIGHT, LEFT
};

/* the poligon.so function prototypes */
extern void turn(unit_t unit, enum turn_direction, unsigned degrees);

#endif /* POLIGON_H */

/*
 * vi: ft=c:ts=2:sw=2:expandtab
 */

