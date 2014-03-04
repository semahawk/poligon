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

#include "poligon.h"

struct unit_desc init(void)
{
  struct unit_desc desc;

  desc.sides = 7;
  desc.color = 0x336699;

  return desc;
}

/*
 * vi: ft=c:ts=2:sw=2:expandtab
 */

