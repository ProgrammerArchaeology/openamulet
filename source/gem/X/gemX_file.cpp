/* ****************************** -*-c++-*- *******************************/
/* ************************************************************************ 
 *         The Amulet User Interface Development Environment              *
 * ************************************************************************
 * This code was written as part of the Amulet project at                 *
 * Carnegie Mellon University, and has been placed in the public          *
 * domain.  If you are using this code or any part of Amulet,             *
 * please contact amulet@cs.cmu.edu to be put on the mailing list.        *
 * ************************************************************************/

/* This file implements the file manipulation functions in gdefs.h for Unix.
*/

**********NOT IMPLEMENTED YET.HERE IS SOME OF ELLEN'S CODE

#include "sys/types.h"
#include "sys/stat.h"
#include "sys/dir.h"
#include "fcntl.h"
#include "sys/unistd.h"

#include "stdlib.h"
#include "string.h"

#ifndef MAXNAMLEN
#define MAXNAMLEN 255
#endif
#define PATHINCR 1025
#define FILESINCR 32

          typedef struct
{
  char *name;
  bool is_dir;
} file_entry;

static char temp_filename[MAXNAMLEN + 1];
static char *pathname;
static long pathlen = 0;
static file_entry *filenames;
static long numfiles = 0;
static long filecount = 0;

// static char * request_ext;
// static char * request_file;
// static char * request_name;
// static char * request_title;

// enum polarity {Open, Save};
// static enum polarity request_polarity;

static void
files_reinit(void **Aptr, long ptr_size, long *Ainitial_count, long increment)
{
  long new_count = (*Ainitial_count) + increment;

  if (*Ainitial_count == 0) {
    *Aptr = (void *)(new char[increment * ptr_size]);
    if (*Aptr == 0) {
      Am_Error("Open/Save Dialog: cannot allocate memory");
    }
  } else {
    void *new_ptr;
    new_ptr = (void *)(new char[new_count * ptr_size]);
    if (new_ptr == 0) {
      Am_Error("Open/Save Dialog: cannot allocate memory");
    }
    memcpy(new_ptr, *Aptr, new_count * ptr_size);
    delete[](char *)(*Aptr);
    *Aptr = new_ptr;
  }
  *Ainitial_count = new_count;
  return;
}

#ifdef NEED_READDIR
/* code to simulate Berkeley directory interface
   adapted from O'Reilly "Using C on the UNIX System" */

typedef struct
{
  int fd;
} DIR;

static DIR current_dir;

static DIR *
opendir(char *dir)
{
  current_dir.fd = open(dir, 0);

  if (current_dir.fd >= 0) {
    struct stat stbuff;

    if (fstat(current_dir.fd, &stbuff) >= 0) {
      if ((stbuff.st_mode & S_IFDIR) != 0) {
        return &current_dir;
      }
    }
    close(current_dir.fd);
  }
  return 0;
}

void
closedir(DIR *dp)
{
  close(dp->fd);
}

struct direct *
readdir(DIR *dp)
{
  static struct direct dir;

  do {
    if (read(dp->fd, &dir, sizeof(dir)) != sizeof(dir))
      return 0;
  } while (dir.d_ino == 0);

  return (&dir);
}
#endif

static char *
cleanup_dname(char *name)
{
  long i;

  for (i = 0; i <= MAXNAMLEN; i++) {
    if (name[i] == 0)
      return name;
  }
  for (i = 0; i < MAXNAMLEN; i++) {
    temp_filename[i] = name[i];
  }
  temp_filename[MAXNAMLEN] = 0;
  return temp_filename;
}

/*
static bool test_ext(
   char * fname,
   char * ext)
{
   long flen = strlen(fname);
   long elen = strlen(ext);
   long i;
      
   fname += (flen - elen);
   for (i = 0; i < elen; i++) {
      if (ext[i] != fname[i]) return false;
   }
   return true;
}
*/

#define is_dot(s) ((s[0] == '.') && (s[1] == 0))
#define is_dotdot(s) ((s[0] == '.') && (s[1] == '.') && (s[2] == 0))

static void
appendpath(char *path, char *file)
{
  long plen = strlen(path);
  long flen = strlen(file);

  if (flen == 0) {
    return;
  } else if (is_dot(file)) {
    /* current directory */
    return;
  } else if (is_dotdot(file)) {
    /* parent directory */
    long i;
    for (i = plen - 1; i > 0; --i) {
      if (path[i] == '/') {
        path[i] = 0;
        return;
      }
    }
    if (path[0] != '/')
      Am_Error("Open/Save Dialog: absolute pathname does not start with '/'");
    path[1] = 0;
    return;
  } else {
    /* plain name */
    if (plen != 1) {
      path[plen] = '/';
      path[plen + 1] = 0;
    }
    strcat(path, file);
    return;
  }
}

static void
add_filename(char *fname, bool isdir)
{
  int len = strlen(fname) + 1;
  if (filecount >= numfiles) {
    files_reinit((void **)&filenames, sizeof(file_entry), &numfiles, FILESINCR);
  }
  filenames[filecount].name = new char[len];
  strcpy(filenames[filecount].name, fname);
  filenames[filecount].is_dir = isdir;
  filecount++;
  return;
}

static bool
is_dir(char *path, char *file)
{
  struct stat stbuff;
  long old_len = strlen(path);

  appendpath(path, file);
  if (stat(path, &stbuff) >= 0) {
    if ((stbuff.st_mode & S_IFDIR) != 0) {
      path[old_len] = 0;
      return true;
    }
  }
  path[old_len] = 0;
  return false;
}

static int
compare(const void *a, const void *b)
{
  char *a1 = ((file_entry *)a)->name;
  char *b1 = ((file_entry *)b)->name;

  return strcmp(a1, b1);
}

static void
am_scandir(char *path //,  char * ext
           )
{
  DIR *dp = opendir(path);
  struct direct *entry;

  if (dp == 0) {
    Am_ERROR("Cannot open directory \"" << path << "\"");
  }

  filecount = 0;
  entry = readdir(dp);

  while (entry != 0) {
    char *fname = cleanup_dname(entry->d_name);

    //      if (test_ext(fname,ext)) {
    if (true) {
      if (is_dir(path, fname)) {
        add_filename(fname, true);
      } else {
        add_filename(fname, false);
      }
    } else if (is_dot(entry->d_name)) {
      /* ignore */
    } else if (is_dotdot(entry->d_name)) {
      add_filename(fname, true);
    } else {
      if (is_dir(path, fname)) {
        add_filename(fname, true);
      }
    }
    entry = readdir(dp);
  }
  closedir(dp);
  qsort((void *)filenames, filecount, sizeof(file_entry), compare);
}

/*
static long findfile(char * file)
{
   long i;

   for (i = 0; i < filecount; i++) {
     const char * fname = (const char *) (filenames[i].name);

     if (strcmp(file,fname) == 0) return i;
   }
   return 0;
}
*/

static void
file_open(
    //   char * file,
    char *path //,   char * title,
    //   char * filter_name,
    //   char * filter_ext
    )
{
  if (path == 0)
    path = ".";

  long len = strlen(path);
  long alloc_len = len + MAXNAMLEN + 1;

  //   request_file = file;
  //   request_title = title;
  //   request_name = filter_name;
  //   request_ext = filter_ext;

  //   if (len >= 1024)
  //     Am_Error("Open/Save Dialog: pathname too long");

  if (pathlen < alloc_len) {
    if (alloc_len < PATHINCR)
      alloc_len = PATHINCR;
    files_reinit((void **)&pathname, sizeof(char), &pathlen, alloc_len);
  }

  if (path[0] == '/') {
    // absolute pathname
    strcpy(pathname, path);
  } else {
    long i, j, plen;

    if (getcwd(pathname, (size_t)pathlen) == 0)
      Am_Error("Open/Save Dialog: unable to get current directory");

    plen = strlen(pathname);
    alloc_len = plen + len + MAXNAMLEN + 1;
    if (pathlen < alloc_len) {
      if (alloc_len < PATHINCR)
        alloc_len = PATHINCR;
      files_reinit((void **)&pathname, sizeof(char), &pathlen, alloc_len);
    }

    temp_filename[0] = 0;
    j = 0;
    for (i = 0; i < len; i++) {
      if (path[i] == '/') {
        temp_filename[j] = 0;
        appendpath(pathname, temp_filename);
        j = 0;
      } else {
        temp_filename[j] = path[i];
        j++;
      }
    }
    temp_filename[j] = 0;
    appendpath(pathname, temp_filename);
  }
  //   am_scandir(pathname,filter_ext);
  am_scandir(pathname);
}

static void
end_dialog()
{
  int i;
  for (i = 0; i < filecount; i++) {
    delete[] filenames[i].name;
  }
}
