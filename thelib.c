/*
 *
 * thelib.c
 *
 * Created at:  Wed  5 Mar 12:28:03 2014 12:28:03
 *
 * Author:  Szymon Urbaś <szymon.urbas@aol.com>
 *
 * License:  please visit the LICENSE file for details.
 *
 */

#include <assert.h>

#include "poligon.h"

void turn(struct unit *unit, enum turn_direction dir, unsigned degrees)
{
  assert(unit);

  if (dir == LEFT)
    unit->rot -= degrees;
  else
    unit->rot += degrees;
}

/*
 * vi: ft=c:ts=2:sw=2:expandtab
 */

