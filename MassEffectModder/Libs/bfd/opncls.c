/* opncls.c -- open and close a BFD.
   Copyright (C) 1990-2019 Free Software Foundation, Inc.

   Written by Cygnus Support.

   This file is part of BFD, the Binary File Descriptor library.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street - Fifth Floor, Boston,
   MA 02110-1301, USA.  */

#include "sysdep.h"
#include "bfd.h"
#include "objalloc.h"
#include "libbfd.h"
#include "libiberty.h"
#include "elf-bfd.h"

#ifndef S_IXUSR
#define S_IXUSR 0100	/* Execute by owner.  */
#endif
#ifndef S_IXGRP
#define S_IXGRP 0010	/* Execute by group.  */
#endif
#ifndef S_IXOTH
#define S_IXOTH 0001	/* Execute by others.  */
#endif

/* Counters used to initialize the bfd identifier.  */

static unsigned int bfd_id_counter = 0;
static unsigned int bfd_reserved_id_counter = 0;

/*
CODE_FRAGMENT
.{* Set to N to open the next N BFDs using an alternate id space.  *}
.extern unsigned int bfd_use_reserved_id;
*/
unsigned int bfd_use_reserved_id = 0;

/* fdopen is a loser -- we should use stdio exclusively.  Unfortunately
   if we do that we can't use fcntl.  */

/* Return a new BFD.  All BFD's are allocated through this routine.  */

bfd *
_bfd_new_bfd (void)
{
  bfd *nbfd;

  nbfd = (bfd *) bfd_zmalloc (sizeof (bfd));
  if (nbfd == NULL)
    return NULL;

  if (bfd_use_reserved_id)
    {
      nbfd->id = --bfd_reserved_id_counter;
      --bfd_use_reserved_id;
    }
  else
    nbfd->id = bfd_id_counter++;

  nbfd->memory = objalloc_create ();
  if (nbfd->memory == NULL)
    {
      bfd_set_error (bfd_error_no_memory);
      free (nbfd);
      return NULL;
    }

  nbfd->arch_info = &bfd_default_arch_struct;

  if (!bfd_hash_table_init_n (& nbfd->section_htab, bfd_section_hash_newfunc,
                  sizeof (struct section_hash_entry), 13))
    {
      free (nbfd);
      return NULL;
    }

  return nbfd;
}

/* Delete a BFD.  */

static void
_bfd_delete_bfd (bfd *abfd)
{
  if (abfd->memory)
    {
      bfd_hash_table_free (&abfd->section_htab);
      objalloc_free ((struct objalloc *) abfd->memory);
    }

  if (abfd->filename)
    free ((char *) abfd->filename);
  free (abfd->arelt_data);
  free (abfd);
}

/* Free objalloc memory.  */

bfd_boolean
_bfd_free_cached_info (bfd *abfd)
{
  if (abfd->memory)
    {
      bfd_hash_table_free (&abfd->section_htab);
      objalloc_free ((struct objalloc *) abfd->memory);

      abfd->sections = NULL;
      abfd->section_last = NULL;
      abfd->outsymbols = NULL;
      abfd->tdata.any = NULL;
      abfd->usrdata = NULL;
      abfd->memory = NULL;
    }

  return TRUE;
}

/*
SECTION
	Opening and closing BFDs

SUBSECTION
	Functions for opening and closing
*/

/*
FUNCTION
	bfd_fopen

SYNOPSIS
	bfd *bfd_fopen (const char *filename, const char *target,
			const char *mode, int fd);

DESCRIPTION
	Open the file @var{filename} with the target @var{target}.
	Return a pointer to the created BFD.  If @var{fd} is not -1,
	then <<fdopen>> is used to open the file; otherwise, <<fopen>>
	is used.  @var{mode} is passed directly to <<fopen>> or
	<<fdopen>>.

	Calls <<bfd_find_target>>, so @var{target} is interpreted as by
	that function.

	The new BFD is marked as cacheable iff @var{fd} is -1.

	If <<NULL>> is returned then an error has occured.   Possible errors
	are <<bfd_error_no_memory>>, <<bfd_error_invalid_target>> or
	<<system_call>> error.

	On error, @var{fd} is always closed.

	A copy of the @var{filename} argument is stored in the newly created
	BFD.  It can be accessed via the bfd_get_filename() macro.
*/

bfd *
bfd_fopen (const char *filename, const char *target, const char *mode, int fd)
{
  bfd *nbfd;
  const bfd_target *target_vec;

  nbfd = _bfd_new_bfd ();
  if (nbfd == NULL)
    {
      if (fd != -1)
	close (fd);
      return NULL;
    }

  target_vec = bfd_find_target (target, nbfd);
  if (target_vec == NULL)
    {
      if (fd != -1)
	close (fd);
      _bfd_delete_bfd (nbfd);
      return NULL;
    }

#ifdef HAVE_FDOPEN
  if (fd != -1)
    nbfd->iostream = fdopen (fd, mode);
  else
#endif
    nbfd->iostream = _bfd_real_fopen (filename, mode);
  if (nbfd->iostream == NULL)
    {
      bfd_set_error (bfd_error_system_call);
      _bfd_delete_bfd (nbfd);
      return NULL;
    }

  /* OK, put everything where it belongs.  */

  /* PR 11983: Do not cache the original filename, but
     rather make a copy - the original might go away.  */
  nbfd->filename = xstrdup (filename);

  /* Figure out whether the user is opening the file for reading,
     writing, or both, by looking at the MODE argument.  */
  if ((mode[0] == 'r' || mode[0] == 'w' || mode[0] == 'a')
      && mode[1] == '+')
    nbfd->direction = both_direction;
  else if (mode[0] == 'r')
    nbfd->direction = read_direction;
  else
    nbfd->direction = write_direction;

  if (! bfd_cache_init (nbfd))
    {
      _bfd_delete_bfd (nbfd);
      return NULL;
    }
  nbfd->opened_once = TRUE;

  /* If we opened the file by name, mark it cacheable; we can close it
     and reopen it later.  However, if a file descriptor was provided,
     then it may have been opened with special flags that make it
     unsafe to close and reopen the file.  */
  if (fd == -1)
    (void) bfd_set_cacheable (nbfd, TRUE);

  return nbfd;
}

/*
FUNCTION
	bfd_openr

SYNOPSIS
	bfd *bfd_openr (const char *filename, const char *target);

DESCRIPTION
	Open the file @var{filename} (using <<fopen>>) with the target
	@var{target}.  Return a pointer to the created BFD.

	Calls <<bfd_find_target>>, so @var{target} is interpreted as by
	that function.

	If <<NULL>> is returned then an error has occured.   Possible errors
	are <<bfd_error_no_memory>>, <<bfd_error_invalid_target>> or
	<<system_call>> error.

	A copy of the @var{filename} argument is stored in the newly created
	BFD.  It can be accessed via the bfd_get_filename() macro.
*/

bfd *
bfd_openr (const char *filename, const char *target)
{
  return bfd_fopen (filename, target, FOPEN_RB, -1);
}

/* Don't try to `optimize' this function:

   o - We lock using stack space so that interrupting the locking
       won't cause a storage leak.
   o - We open the file stream last, since we don't want to have to
       close it if anything goes wrong.  Closing the stream means closing
       the file descriptor too, even though we didn't open it.  */
/*
FUNCTION
	bfd_fdopenr

SYNOPSIS
	bfd *bfd_fdopenr (const char *filename, const char *target, int fd);

DESCRIPTION
	<<bfd_fdopenr>> is to <<bfd_fopenr>> much like <<fdopen>> is to
	<<fopen>>.  It opens a BFD on a file already described by the
	@var{fd} supplied.

	When the file is later <<bfd_close>>d, the file descriptor will
	be closed.  If the caller desires that this file descriptor be
	cached by BFD (opened as needed, closed as needed to free
	descriptors for other opens), with the supplied @var{fd} used as
	an initial file descriptor (but subject to closure at any time),
	call bfd_set_cacheable(bfd, 1) on the returned BFD.  The default
	is to assume no caching; the file descriptor will remain open
	until <<bfd_close>>, and will not be affected by BFD operations
	on other files.

	Possible errors are <<bfd_error_no_memory>>,
	<<bfd_error_invalid_target>> and <<bfd_error_system_call>>.

	On error, @var{fd} is closed.

	A copy of the @var{filename} argument is stored in the newly created
	BFD.  It can be accessed via the bfd_get_filename() macro.
*/

bfd *
bfd_fdopenr (const char *filename, const char *target, int fd)
{
  const char *mode;
#if defined(HAVE_FCNTL) && defined(F_GETFL)
  int fdflags;
#endif

#if ! defined(HAVE_FCNTL) || ! defined(F_GETFL)
  mode = FOPEN_RUB; /* Assume full access.  */
#else
  fdflags = fcntl (fd, F_GETFL, NULL);
  if (fdflags == -1)
    {
      int save = errno;

      close (fd);
      errno = save;
      bfd_set_error (bfd_error_system_call);
      return NULL;
    }

  /* (O_ACCMODE) parens are to avoid Ultrix header file bug.  */
  switch (fdflags & (O_ACCMODE))
    {
    case O_RDONLY: mode = FOPEN_RB; break;
    case O_WRONLY: mode = FOPEN_RUB; break;
    case O_RDWR:   mode = FOPEN_RUB; break;
    default: abort ();
    }
#endif

  return bfd_fopen (filename, target, mode, fd);
}

static inline void
_maybe_make_executable (bfd * abfd ATTRIBUTE_UNUSED)
{
}

/*
FUNCTION
	bfd_close

SYNOPSIS
	bfd_boolean bfd_close (bfd *abfd);

DESCRIPTION
	Close a BFD. If the BFD was open for writing, then pending
	operations are completed and the file written out and closed.
	If the created file is executable, then <<chmod>> is called
	to mark it as such.

	All memory attached to the BFD is released.

	The file descriptor associated with the BFD is closed (even
	if it was passed in to BFD by <<bfd_fdopenr>>).

RETURNS
	<<TRUE>> is returned if all is ok, otherwise <<FALSE>>.
*/

bfd_boolean
bfd_close (bfd *abfd)
{
  return bfd_close_all_done (abfd);
}

/*
FUNCTION
	bfd_close_all_done

SYNOPSIS
	bfd_boolean bfd_close_all_done (bfd *);

DESCRIPTION
	Close a BFD.  Differs from <<bfd_close>> since it does not
	complete any pending operations.  This routine would be used
	if the application had just used BFD for swapping and didn't
	want to use any of the writing code.

	If the created file is executable, then <<chmod>> is called
	to mark it as such.

	All memory attached to the BFD is released.

RETURNS
	<<TRUE>> is returned if all is ok, otherwise <<FALSE>>.
*/

bfd_boolean
bfd_close_all_done (bfd *abfd)
{
  bfd_boolean ret;

  if (! BFD_SEND (abfd, _close_and_cleanup, (abfd)))
    return FALSE;

  ret = abfd->iovec->bclose (abfd) == 0;

  if (ret)
    _maybe_make_executable (abfd);

  _bfd_delete_bfd (abfd);

  return ret;
}

/*
FUNCTION
	bfd_alloc

SYNOPSIS
	void *bfd_alloc (bfd *abfd, bfd_size_type wanted);

DESCRIPTION
	Allocate a block of @var{wanted} bytes of memory attached to
	<<abfd>> and return a pointer to it.
*/

void *
bfd_alloc (bfd *abfd, bfd_size_type size)
{
  void *ret;
  unsigned long ul_size = (unsigned long) size;

  if (size != ul_size
      /* Note - although objalloc_alloc takes an unsigned long as its
	 argument, internally the size is treated as a signed long.  This can
	 lead to problems where, for example, a request to allocate -1 bytes
	 can result in just 1 byte being allocated, rather than
	 ((unsigned long) -1) bytes.  Also memory checkers will often
	 complain about attempts to allocate a negative amount of memory.
	 So to stop these problems we fail if the size is negative.  */
      || ((signed long) ul_size) < 0)
    {
      bfd_set_error (bfd_error_no_memory);
      return NULL;
    }

  ret = objalloc_alloc ((struct objalloc *) abfd->memory, ul_size);
  if (ret == NULL)
    bfd_set_error (bfd_error_no_memory);
  return ret;
}

/*
INTERNAL_FUNCTION
	bfd_alloc2

SYNOPSIS
	void *bfd_alloc2 (bfd *abfd, bfd_size_type nmemb, bfd_size_type size);

DESCRIPTION
	Allocate a block of @var{nmemb} elements of @var{size} bytes each
	of memory attached to <<abfd>> and return a pointer to it.
*/

void *
bfd_alloc2 (bfd *abfd, bfd_size_type nmemb, bfd_size_type size)
{
  if ((nmemb | size) >= HALF_BFD_SIZE_TYPE
      && size != 0
      && nmemb > ~(bfd_size_type) 0 / size)
    {
      bfd_set_error (bfd_error_no_memory);
      return NULL;
    }

  return bfd_alloc (abfd, size * nmemb);
}

/*
FUNCTION
	bfd_zalloc

SYNOPSIS
	void *bfd_zalloc (bfd *abfd, bfd_size_type wanted);

DESCRIPTION
	Allocate a block of @var{wanted} bytes of zeroed memory
	attached to <<abfd>> and return a pointer to it.
*/

void *
bfd_zalloc (bfd *abfd, bfd_size_type size)
{
  void *res;

  res = bfd_alloc (abfd, size);
  if (res)
    memset (res, 0, (size_t) size);
  return res;
}

/*
INTERNAL_FUNCTION
	bfd_zalloc2

SYNOPSIS
	void *bfd_zalloc2 (bfd *abfd, bfd_size_type nmemb, bfd_size_type size);

DESCRIPTION
	Allocate a block of @var{nmemb} elements of @var{size} bytes each
	of zeroed memory attached to <<abfd>> and return a pointer to it.
*/

void *
bfd_zalloc2 (bfd *abfd, bfd_size_type nmemb, bfd_size_type size)
{
  void *res;

  if ((nmemb | size) >= HALF_BFD_SIZE_TYPE
      && size != 0
      && nmemb > ~(bfd_size_type) 0 / size)
    {
      bfd_set_error (bfd_error_no_memory);
      return NULL;
    }

  size *= nmemb;

  res = bfd_alloc (abfd, size);
  if (res)
    memset (res, 0, (size_t) size);
  return res;
}

/* Free a block allocated for a BFD.
   Note:  Also frees all more recently allocated blocks!  */

void
bfd_release (bfd *abfd, void *block)
{
  objalloc_free_block ((struct objalloc *) abfd->memory, block);
}
