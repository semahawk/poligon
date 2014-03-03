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

#ifndef MAIN_H
#define MAIN_H

/* the maximum size of the file's extension */
#define FEXT_MAX_SIZE (4)

struct ctl {
  unsigned sides;
  unsigned colour;
};

struct file_handlers {
  char *fext;
  void *(*handler)(struct ctl *ctl, char *fname);
};

/* file handling function prototypes */
void *handle_so(struct ctl *ctl, char *fname);

#endif /* MAIN_H */

/*
 * vi: ft=c:ts=2:sw=2:expandtab
 */

