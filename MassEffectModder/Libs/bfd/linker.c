/* linker.c -- BFD linker routines
   Copyright (C) 1993-2019 Free Software Foundation, Inc.
   Written by Steve Chamberlain and Ian Lance Taylor, Cygnus Support

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
#include "libbfd.h"
#include "bfdlink.h"
#include "genlink.h"

bfd_byte *
bfd_simple_get_relocated_section_contents (bfd *abfd,
                       asection *sec,
                       bfd_byte *outbuf,
                       asymbol **symbol_table ATTRIBUTE_UNUSED)
{
  bfd_byte *contents;

  contents = outbuf;
  if (!bfd_get_full_section_contents (abfd, sec, &contents))
    return NULL;
  return contents;
}

void
_bfd_generic_link_hide_symbol (bfd *output_bfd ATTRIBUTE_UNUSED,
                   struct bfd_link_info *info ATTRIBUTE_UNUSED,
                   struct bfd_link_hash_entry *h ATTRIBUTE_UNUSED)
{
}

bfd_boolean
_bfd_generic_link_check_relocs (bfd *abfd ATTRIBUTE_UNUSED,
                struct bfd_link_info *info ATTRIBUTE_UNUSED)
{
  return TRUE;
}

bfd_boolean
_bfd_generic_link_add_one_symbol (struct bfd_link_info *info ATTRIBUTE_UNUSED,
                  bfd *abfd ATTRIBUTE_UNUSED,
                  const char *name ATTRIBUTE_UNUSED,
                  flagword flags ATTRIBUTE_UNUSED,
                  asection *section ATTRIBUTE_UNUSED,
                  bfd_vma value ATTRIBUTE_UNUSED,
                  const char *string ATTRIBUTE_UNUSED,
                  bfd_boolean copy ATTRIBUTE_UNUSED,
                  bfd_boolean collect ATTRIBUTE_UNUSED,
                  struct bfd_link_hash_entry **hashp ATTRIBUTE_UNUSED)
{
  return FALSE;
}

int
_bfd_nolink_sizeof_headers (bfd *abfd ATTRIBUTE_UNUSED,
			    struct bfd_link_info *info ATTRIBUTE_UNUSED)
{
  return 0;
}

bfd_boolean
_bfd_nolink_bfd_relax_section (bfd *abfd,
			       asection *section ATTRIBUTE_UNUSED,
			       struct bfd_link_info *link_info ATTRIBUTE_UNUSED,
			       bfd_boolean *again ATTRIBUTE_UNUSED)
{
  return _bfd_bool_bfd_false_error (abfd);
}

bfd_byte *
_bfd_nolink_bfd_get_relocated_section_contents
    (bfd *abfd,
     struct bfd_link_info *link_info ATTRIBUTE_UNUSED,
     struct bfd_link_order *link_order ATTRIBUTE_UNUSED,
     bfd_byte *data ATTRIBUTE_UNUSED,
     bfd_boolean relocatable ATTRIBUTE_UNUSED,
     asymbol **symbols ATTRIBUTE_UNUSED)
{
  return (bfd_byte *) _bfd_ptr_bfd_null_error (abfd);
}

bfd_boolean
_bfd_nolink_bfd_lookup_section_flags
    (struct bfd_link_info *info ATTRIBUTE_UNUSED,
     struct flag_info *flaginfo ATTRIBUTE_UNUSED,
     asection *section)
{
  return _bfd_bool_bfd_false_error (section->owner);
}

bfd_boolean
_bfd_nolink_bfd_is_group_section (bfd *abfd,
				  const asection *sec ATTRIBUTE_UNUSED)
{
  return _bfd_bool_bfd_false_error (abfd);
}

bfd_boolean
_bfd_nolink_bfd_discard_group (bfd *abfd, asection *sec ATTRIBUTE_UNUSED)
{
  return _bfd_bool_bfd_false_error (abfd);
}

struct bfd_link_hash_table *
_bfd_nolink_bfd_link_hash_table_create (bfd *abfd)
{
  return (struct bfd_link_hash_table *) _bfd_ptr_bfd_null_error (abfd);
}

void
_bfd_nolink_bfd_link_just_syms (asection *sec ATTRIBUTE_UNUSED,
				struct bfd_link_info *info ATTRIBUTE_UNUSED)
{
}

void
_bfd_nolink_bfd_copy_link_hash_symbol_type
    (bfd *abfd ATTRIBUTE_UNUSED,
     struct bfd_link_hash_entry *from ATTRIBUTE_UNUSED,
     struct bfd_link_hash_entry *to ATTRIBUTE_UNUSED)
{
}

bfd_boolean
_bfd_nolink_bfd_link_split_section (bfd *abfd, asection *sec ATTRIBUTE_UNUSED)
{
  return _bfd_bool_bfd_false_error (abfd);
}

bfd_boolean
_bfd_nolink_section_already_linked (bfd *abfd,
				    asection *sec ATTRIBUTE_UNUSED,
				    struct bfd_link_info *info ATTRIBUTE_UNUSED)
{
  return _bfd_bool_bfd_false_error (abfd);
}

bfd_boolean
_bfd_nolink_bfd_define_common_symbol
    (bfd *abfd,
     struct bfd_link_info *info ATTRIBUTE_UNUSED,
     struct bfd_link_hash_entry *h ATTRIBUTE_UNUSED)
{
  return _bfd_bool_bfd_false_error (abfd);
}

struct bfd_link_hash_entry *
_bfd_nolink_bfd_define_start_stop (struct bfd_link_info *info ATTRIBUTE_UNUSED,
				   const char *name ATTRIBUTE_UNUSED,
				   asection *sec)
{
  return (struct bfd_link_hash_entry *) _bfd_ptr_bfd_null_error (sec->owner);
}
