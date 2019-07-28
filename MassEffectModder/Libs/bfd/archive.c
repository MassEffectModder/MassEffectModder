/* BFD back-end for archive files (libraries).
   Copyright (C) 1990-2019 Free Software Foundation, Inc.
   Written by Cygnus Support.  Mostly Gumby Henkel-Wallace's fault.

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
   Foundation, Inc., 51 Franklin Street - Fifth Floor, Boston, MA 02110-1301, USA.  */

/*
@setfilename archive-info
SECTION
	Archives

DESCRIPTION
	An archive (or library) is just another BFD.  It has a symbol
	table, although there's not much a user program will do with it.

	The big difference between an archive BFD and an ordinary BFD
	is that the archive doesn't have sections.  Instead it has a
	chain of BFDs that are considered its contents.  These BFDs can
	be manipulated like any other.  The BFDs contained in an
	archive opened for reading will all be opened for reading.  You
	may put either input or output BFDs into an archive opened for
	output; they will be handled correctly when the archive is closed.

	Use <<bfd_openr_next_archived_file>> to step through
	the contents of an archive opened for input.  You don't
	have to read the entire archive if you don't want
	to!  Read it until you find what you want.

	A BFD returned by <<bfd_openr_next_archived_file>> can be
	closed manually with <<bfd_close>>.  If you do not close it,
	then a second iteration through the members of an archive may
	return the same BFD.  If you close the archive BFD, then all
	the member BFDs will automatically be closed as well.

	Archive contents of output BFDs are chained through the
	<<archive_next>> pointer in a BFD.  The first one is findable
	through the <<archive_head>> slot of the archive.  Set it with
	<<bfd_set_archive_head>> (q.v.).  A given BFD may be in only
	one open output archive at a time.

	As expected, the BFD archive code is more general than the
	archive code of any given environment.  BFD archives may
	contain files of different formats (e.g., a.out and coff) and
	even different architectures.  You may even place archives
	recursively into archives!

	This can cause unexpected confusion, since some archive
	formats are more expressive than others.  For instance, Intel
	COFF archives can preserve long filenames; SunOS a.out archives
	cannot.  If you move a file from the first to the second
	format and back again, the filename may be truncated.
	Likewise, different a.out environments have different
	conventions as to how they truncate filenames, whether they
	preserve directory names in filenames, etc.  When
	interoperating with native tools, be sure your files are
	homogeneous.

	Beware: most of these formats do not react well to the
	presence of spaces in filenames.  We do the best we can, but
	can't always handle this case due to restrictions in the format of
	archives.  Many Unix utilities are braindead in regards to
	spaces and such in filenames anyway, so this shouldn't be much
	of a restriction.

	Archives are supported in BFD in <<archive.c>>.

SUBSECTION
	Archive functions
*/

/* Assumes:
   o - all archive elements start on an even boundary, newline padded;
   o - all arch headers are char *;
   o - all arch headers are the same size (across architectures).
*/

/* Some formats provide a way to cram a long filename into the short
   (16 chars) space provided by a BSD archive.  The trick is: make a
   special "file" in the front of the archive, sort of like the SYMDEF
   entry.  If the filename is too long to fit, put it in the extended
   name table, and use its index as the filename.  To prevent
   confusion prepend the index with a space.  This means you can't
   have filenames that start with a space, but then again, many Unix
   utilities can't handle that anyway.

   This scheme unfortunately requires that you stand on your head in
   order to write an archive since you need to put a magic file at the
   front, and need to touch every entry to do so.  C'est la vie.

   We support two variants of this idea:
   The SVR4 format (extended name table is named "//"),
   and an extended pseudo-BSD variant (extended name table is named
   "ARFILENAMES/").  The origin of the latter format is uncertain.

   BSD 4.4 uses a third scheme:  It writes a long filename
   directly after the header.  This allows 'ar q' to work.
*/

/* Summary of archive member names:

 Symbol table (must be first):
 "__.SYMDEF       " - Symbol table, Berkeley style, produced by ranlib.
 "/               " - Symbol table, system 5 style.

 Long name table (must be before regular file members):
 "//              " - Long name table, System 5 R4 style.
 "ARFILENAMES/    " - Long name table, non-standard extended BSD (not BSD 4.4).

 Regular file members with short names:
 "filename.o/     " - Regular file, System 5 style (embedded spaces ok).
 "filename.o      " - Regular file, Berkeley style (no embedded spaces).

 Regular files with long names (or embedded spaces, for BSD variants):
 "/18             " - SVR4 style, name at offset 18 in name table.
 "#1/23           " - Long name (or embedded spaces) 23 characters long,
		      BSD 4.4 style, full name follows header.
 " 18             " - Long name 18 characters long, extended pseudo-BSD.
 */

#include "sysdep.h"
#include "bfd.h"
#include "libbfd.h"

int
bfd_generic_stat_arch_elt (bfd *abfd ATTRIBUTE_UNUSED, struct stat *buf ATTRIBUTE_UNUSED)
{
  return 0;
}

bfd_boolean
_bfd_archive_close_and_cleanup (bfd *abfd ATTRIBUTE_UNUSED)
{
  return TRUE;
}

bfd *
_bfd_noarchive_get_elt_at_index (bfd *abfd,
				 symindex sym_index ATTRIBUTE_UNUSED)
{
  return (bfd *) _bfd_ptr_bfd_null_error (abfd);
}

bfd *
_bfd_noarchive_openr_next_archived_file (bfd *archive,
					 bfd *last_file ATTRIBUTE_UNUSED)
{
  return (bfd *) _bfd_ptr_bfd_null_error (archive);
}

bfd_boolean
_bfd_noarchive_construct_extended_name_table (bfd *abfd ATTRIBUTE_UNUSED,
					      char **tabloc ATTRIBUTE_UNUSED,
					      bfd_size_type *len ATTRIBUTE_UNUSED,
					      const char **name ATTRIBUTE_UNUSED)
{
  return TRUE;
}

bfd_boolean
_bfd_noarchive_write_ar_hdr (bfd *archive, bfd *abfd ATTRIBUTE_UNUSED)
{
  return _bfd_bool_bfd_false_error (archive);
}

void
_bfd_noarchive_truncate_arname (bfd *abfd ATTRIBUTE_UNUSED,
				const char *pathname ATTRIBUTE_UNUSED,
				char *arhdr ATTRIBUTE_UNUSED)
{
}

bfd_boolean
_bfd_noarchive_write_armap
    (bfd *arch ATTRIBUTE_UNUSED,
     unsigned int elength ATTRIBUTE_UNUSED,
     struct orl *map ATTRIBUTE_UNUSED,
     unsigned int orl_count ATTRIBUTE_UNUSED,
     int stridx ATTRIBUTE_UNUSED)
{
  return TRUE;
}
