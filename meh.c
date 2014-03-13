/*
 *
 * meh.c
 *
 * Created at:  Mon  3 Mar 22:02:05 2014 22:02:05
 *
 * Author:  Szymon Urbaś <szymon.urbas@aol.com>
 *
 * License:  please visit the LICENSE file for details.
 *
 */

#include <stdlib.h>
#include <time.h>

#include "poligon.h"

/* make meself global (but not so much) */
static unit_t me;

struct unit_desc init(void)
{
  struct unit_desc desc;

  srand(time(NULL));

  strncpy(desc.name, "Meh", UNIT_MAX_NAME_LEN);
  desc.sides = (rand() % (UNIT_MAX_SIDES - UNIT_MIN_SIDES)) + UNIT_MIN_SIDES + 1;
  desc.color = 0x336699;

  return desc;
}

void fetch(unit_t unit)
{
  me = unit;

  turn(me, LEFT, 60);
}

/*
 * vi: ft=c:ts=2:sw=2:expandtab
 */

