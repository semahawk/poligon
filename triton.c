/*
 *
 * triton.c
 *
 * Created at:  Mon  3 Mar 22:02:05 2014 22:02:05
 *
 * Author:  Szymon Urbaś <szymon.urbas@aol.com>
 *
 * License:  please visit the LICENSE file for details.
 *
 */

#include "poligon.h"

struct ctl init(void)
{
  struct ctl ret;

  ret.sides = 3;
  ret.colour = 0xf27a02;

  return ret;
}

/*
 * vi: ft=c:ts=2:sw=2:expandtab
 */

