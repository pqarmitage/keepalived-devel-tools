/* Copyright (C) 1991-2016 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#define _GNU_SOURCE
#include <unistd.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <paths.h>
#include <sys/param.h>
#include <sys/stat.h>

#ifndef PATH_MAX
# ifdef MAXPATHLEN
#  define PATH_MAX MAXPATHLEN
# else
#  define PATH_MAX 1024
# endif
#endif

#include <stdio.h>

char *CS_PATH;

int
find_path (const char *file, char **resolved_name)
{
  size_t filename_len;
  *resolved_name = NULL;

  /* We check the simple case first. */
  if (*file == '\0')
      return ENOENT;

  filename_len = strlen(file);
  if (filename_len >= PATH_MAX)
    return ENAMETOOLONG;

  /* Don't search when it contains a slash.  */
  if (strchr (file, '/') != NULL)
    {
      *resolved_name = malloc(filename_len+1);
      strcpy(*resolved_name, file);

      return 0;
    }

  const char *path = getenv ("PATH");
  int cs_path_len;
  if (!path) {
    if (!CS_PATH) {
      CS_PATH = malloc(cs_path_len = confstr(_CS_PATH, NULL, 0));
      confstr(_CS_PATH, CS_PATH, cs_path_len);
    }
    path = CS_PATH;
  }

  /* Although GLIBC does not enforce NAME_MAX, we set it as the maximum
     size to avoid unbounded stack allocation.  Same applies for
     PATH_MAX.  */
  size_t file_len = strnlen (file, NAME_MAX + 1);
  size_t path_len = strnlen (path, PATH_MAX - 1) + 1;

  if (file_len > NAME_MAX)
      return ENAMETOOLONG;

  const char *subp;
  bool got_eacces = false;
  char buffer[path_len + file_len + 1];
  const char *p;
  for (p = path; ; p = subp)
    {
      subp = strchrnul (p, ':');

      /* PATH is larger than PATH_MAX and thus potentially larger than
	 the stack allocation.  */
      if (subp - p >= path_len)
	{
          /* If there is only one path, bail out.  */
	  if (*subp == '\0')
	    break;
	  /* Otherwise skip to next one.  */
	  continue;
	}

      /* Use the current path entry, plus a '/' if nonempty, plus the file to
         execute.  */
      char *pend = mempcpy (buffer, p, subp - p);
      *pend = '/';
      memcpy (pend + (p < subp), file, file_len + 1);

      struct stat buf;
      int ret;
      ret = stat (buffer, &buf);

      if (ret) {
	if (!S_ISREG(buf.st_mode))
	  ret = EACCES;
	else if (!(buf.st_mode & (S_IXUSR | S_IXGRP | S_IXOTH))) {
	  /* TODO - this should be a proper check using uid/gid */
	  ret = EACCES;
	}
      }

      if (!ret) {
	/* Success */
	*resolved_name = malloc(strlen(buffer)+1);
	strcpy(*resolved_name, buffer);
	return 0;
      }
	
      switch (ret)
	{
	  case ENOEXEC:
	  case EACCES:
	  /* Record that we got a 'Permission denied' error.  If we end
	     up finding no executable we can use, we want to diagnose
	     that we did find one but were denied access.  */
	    got_eacces = true;
	  case ENOENT:
	  case ESTALE:
	  case ENOTDIR:
	  /* Those errors indicate the file is missing or not executable
	     by us, in which case we want to just try the next path
	     directory.  */
	  case ENODEV:
	  case ETIMEDOUT:
	  /* Some strange filesystems like AFS return even
	     stranger error numbers.  They cannot reasonably mean
	     anything else so ignore those, too.  */
	    break;

          default:
	  /* Some other error means we found an executable file, but
	     something went wrong executing it; return the error to our
	     caller.  */
	    return -1;
	}

      if (*subp++ == '\0')
	break;
    }

  /* We tried every element and none of them worked.  */
  if (got_eacces)
    /* At least one failure was due to permissions, so report that
       error.  */
    return EACCES;

  return ENOENT;
}

#include <stdio.h>

int main(int argc, char **argv, char **envp)
{
	char *resolved_path;
	int ret;

	printf("NAME_MAX = %d, PATH_MAX = %d\n", NAME_MAX, PATH_MAX);

	if ((ret = find_path(argv[1], &resolved_path))) {
		printf("find_path returned %d\n", ret);
		exit(1);
	}

	printf("Resolved path is %s\n", resolved_path);
}
