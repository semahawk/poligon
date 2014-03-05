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
/* the unit's height */
#define UNIT_HEIGHT (96)

struct unit_desc {
  unsigned sides;
  Uint32 color;
};

struct unit {
  unsigned x;
  unsigned y;
  unsigned rot; /* rotation (in degrees) */
  struct unit_desc desc; /* unit's description */
  unsigned hp; /* hit points */
};

struct file_handlers {
  char *fext;
  void *(*handler)(struct unit_desc *desc, char *fname);
};

enum turn_direction {
  RIGHT, LEFT
};

/* the poligon.so function prototypes */
void turn(struct unit *unit, enum turn_direction, unsigned degrees);

#endif /* POLIGON_H */

/*
 * vi: ft=c:ts=2:sw=2:expandtab
 */

