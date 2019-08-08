/* Support for the generic parts of COFF, for BFD.
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

/* Most of this hacked by  Steve Chamberlain, sac@cygnus.com.
   Split out of coffcode.h by Ian Taylor, ian@cygnus.com.  */

/* This file contains COFF code that is not dependent on any
   particular COFF target.  There is only one version of this file in
   libbfd.a, so no target specific code may be put in here.  Or, to
   put it another way,

   ********** DO NOT PUT TARGET SPECIFIC CODE IN THIS FILE **********

   If you need to add some target specific behaviour, add a new hook
   function to bfd_coff_backend_data.

   Some of these functions are also called by the ECOFF routines.
   Those functions may not use any COFF specific information, such as
   coff_data (abfd).  */

#include "sysdep.h"
#include "bfd.h"
#include "libbfd.h"
#include "coff/internal.h"
#include "libcoff.h"

/* Take a section header read from a coff file (in HOST byte order),
   and make a BFD "section" out of it.  This is used by ECOFF.  */

static bfd_boolean
make_a_section_from_file (bfd *abfd,
			  struct internal_scnhdr *hdr,
			  unsigned int target_index)
{
  asection *return_section;
  char *name;
  bfd_boolean result = TRUE;
  flagword flags;

  name = NULL;

  /* Handle long section names as in PE.  On reading, we want to
    accept long names if the format permits them at all, regardless
    of the current state of the flag that dictates if we would generate
    them in outputs; this construct checks if that is the case by
    attempting to set the flag, without changing its state; the call
    will fail for formats that do not support long names at all.  */
  if (bfd_coff_set_long_section_names (abfd, bfd_coff_long_section_names (abfd))
      && hdr->s_name[0] == '/')
    {
      char buf[SCNNMLEN];
      long strindex;
      char *p;
      const char *strings;

      /* Flag that this BFD uses long names, even though the format might
	 expect them to be off by default.  This won't directly affect the
	 format of any output BFD created from this one, but the information
	 can be used to decide what to do.  */
      bfd_coff_set_long_section_names (abfd, TRUE);
      memcpy (buf, hdr->s_name + 1, SCNNMLEN - 1);
      buf[SCNNMLEN - 1] = '\0';
      strindex = strtol (buf, &p, 10);
      if (*p == '\0' && strindex >= 0)
	{
	  strings = _bfd_coff_read_string_table (abfd);
	  if (strings == NULL)
	    return FALSE;
	  if ((bfd_size_type)(strindex + 2) >= obj_coff_strings_len (abfd))
	    return FALSE;
	  strings += strindex;
	  name = (char *) bfd_alloc (abfd,
				     (bfd_size_type) strlen (strings) + 1 + 1);
	  if (name == NULL)
	    return FALSE;
	  strcpy (name, strings);
	}
    }

  if (name == NULL)
    {
      /* Assorted wastage to null-terminate the name, thanks AT&T! */
      name = (char *) bfd_alloc (abfd,
				 (bfd_size_type) sizeof (hdr->s_name) + 1 + 1);
      if (name == NULL)
	return FALSE;
      strncpy (name, (char *) &hdr->s_name[0], sizeof (hdr->s_name));
      name[sizeof (hdr->s_name)] = 0;
    }

  return_section = bfd_make_section_anyway (abfd, name);
  if (return_section == NULL)
    return FALSE;

  return_section->vma = hdr->s_vaddr;
  return_section->lma = hdr->s_paddr;
  return_section->size = hdr->s_size;
  return_section->filepos = hdr->s_scnptr;
  return_section->rel_filepos = hdr->s_relptr;
  return_section->reloc_count = hdr->s_nreloc;

  bfd_coff_set_alignment_hook (abfd, return_section, hdr);

  return_section->line_filepos = hdr->s_lnnoptr;

  return_section->lineno_count = hdr->s_nlnno;
  return_section->userdata = NULL;
  return_section->next = NULL;
  return_section->target_index = target_index;

  if (! bfd_coff_styp_to_sec_flags_hook (abfd, hdr, name, return_section,
					 & flags))
    result = FALSE;

  return_section->flags = flags;

  /* At least on i386-coff, the line number count for a shared library
     section must be ignored.  */
  if ((return_section->flags & SEC_COFF_SHARED_LIBRARY) != 0)
    return_section->lineno_count = 0;

  if (hdr->s_nreloc != 0)
    return_section->flags |= SEC_RELOC;
  /* FIXME: should this check 'hdr->s_size > 0'.  */
  if (hdr->s_scnptr != 0)
    return_section->flags |= SEC_HAS_CONTENTS;

  /* Compress/decompress DWARF debug sections with names: .debug_* and
     .zdebug_*, after the section flags is set.  */
  if ((flags & SEC_DEBUGGING)
      && strlen (name) > 7
      && ((name[1] == 'd' && name[6] == '_')
	  || (strlen (name) > 8 && name[1] == 'z' && name[7] == '_')))
    {
      enum { nothing, compress, decompress } action = nothing;
      char *new_name = NULL;

      if (bfd_is_section_compressed (abfd, return_section))
	{
	  /* Compressed section.  Check if we should decompress.  */
	  if ((abfd->flags & BFD_DECOMPRESS))
	    action = decompress;
	}
      else if (!bfd_is_section_compressed (abfd, return_section))
	{
	  /* Normal section.  Check if we should compress.  */
	  if ((abfd->flags & BFD_COMPRESS) && return_section->size != 0)
	    action = compress;
	}

      switch (action)
	{
	case nothing:
	  break;
	case compress:
	  if (!bfd_init_section_compress_status (abfd, return_section))
	    {
	      _bfd_error_handler
		/* xgettext: c-format */
		(_("%pB: unable to initialize compress status for section %s"),
		 abfd, name);
	      return FALSE;
	    }
	  if (return_section->compress_status == COMPRESS_SECTION_DONE)
	    {
	      if (name[1] != 'z')
		{
		  unsigned int len = strlen (name);

		  new_name = bfd_alloc (abfd, len + 2);
		  if (new_name == NULL)
		    return FALSE;
		  new_name[0] = '.';
		  new_name[1] = 'z';
		  memcpy (new_name + 2, name + 1, len);
		}
	    }
	 break;
	case decompress:
	  if (!bfd_init_section_decompress_status (abfd, return_section))
	    {
	      _bfd_error_handler
		/* xgettext: c-format */
		(_("%pB: unable to initialize decompress status for section %s"),
		 abfd, name);
	      return FALSE;
	    }
	  if (name[1] == 'z')
	    {
	      unsigned int len = strlen (name);

	      new_name = bfd_alloc (abfd, len);
	      if (new_name == NULL)
		return FALSE;
	      new_name[0] = '.';
	      memcpy (new_name + 1, name + 2, len - 1);
	    }
	  break;
	}
      if (new_name != NULL)
	bfd_rename_section (abfd, return_section, new_name);
    }

  return result;
}

/* Read in a COFF object and make it into a BFD.  This is used by
   ECOFF as well.  */
const bfd_target *
coff_real_object_p (bfd *,
		    unsigned,
		    struct internal_filehdr *,
		    struct internal_aouthdr *);
const bfd_target *
coff_real_object_p (bfd *abfd,
		    unsigned nscns,
		    struct internal_filehdr *internal_f,
		    struct internal_aouthdr *internal_a)
{
  flagword oflags = abfd->flags;
  bfd_vma ostart = bfd_get_start_address (abfd);
  void * tdata;
  void * tdata_save;
  bfd_size_type readsize;	/* Length of file_info.  */
  unsigned int scnhsz;
  char *external_sections;

  if (!(internal_f->f_flags & F_RELFLG))
    abfd->flags |= HAS_RELOC;
  if ((internal_f->f_flags & F_EXEC))
    abfd->flags |= EXEC_P;
  if (!(internal_f->f_flags & F_LNNO))
    abfd->flags |= HAS_LINENO;
  if (!(internal_f->f_flags & F_LSYMS))
    abfd->flags |= HAS_LOCALS;

  /* FIXME: How can we set D_PAGED correctly?  */
  if ((internal_f->f_flags & F_EXEC) != 0)
    abfd->flags |= D_PAGED;

  bfd_get_symcount (abfd) = internal_f->f_nsyms;
  if (internal_f->f_nsyms)
    abfd->flags |= HAS_SYMS;

  if (internal_a != (struct internal_aouthdr *) NULL)
    bfd_get_start_address (abfd) = internal_a->entry;
  else
    bfd_get_start_address (abfd) = 0;

  /* Set up the tdata area.  ECOFF uses its own routine, and overrides
     abfd->flags.  */
  tdata_save = abfd->tdata.any;
  tdata = bfd_coff_mkobject_hook (abfd, (void *) internal_f, (void *) internal_a);
  if (tdata == NULL)
    goto fail2;

  scnhsz = bfd_coff_scnhsz (abfd);
  readsize = (bfd_size_type) nscns * scnhsz;
  external_sections = (char *) bfd_alloc (abfd, readsize);
  if (!external_sections)
    goto fail;

  if (bfd_bread ((void *) external_sections, readsize, abfd) != readsize)
    goto fail;

  /* Set the arch/mach *before* swapping in sections; section header swapping
     may depend on arch/mach info.  */
  if (! bfd_coff_set_arch_mach_hook (abfd, (void *) internal_f))
    goto fail;

  /* Now copy data as required; construct all asections etc.  */
  if (nscns != 0)
    {
      unsigned int i;
      for (i = 0; i < nscns; i++)
	{
	  struct internal_scnhdr tmp;
	  bfd_coff_swap_scnhdr_in (abfd,
				   (void *) (external_sections + i * scnhsz),
				   (void *) & tmp);
	  if (! make_a_section_from_file (abfd, &tmp, i + 1))
	    goto fail;
	}
    }

  return abfd->xvec;

 fail:
  bfd_release (abfd, tdata);
 fail2:
  abfd->tdata.any = tdata_save;
  abfd->flags = oflags;
  bfd_get_start_address (abfd) = ostart;
  return (const bfd_target *) NULL;
}

/* Get the BFD section from a COFF symbol section number.  */

asection *
coff_section_from_bfd_index (bfd *abfd, int section_index)
{
  struct bfd_section *answer = abfd->sections;

  if (section_index == N_ABS)
    return bfd_abs_section_ptr;
  if (section_index == N_UNDEF)
    return bfd_und_section_ptr;
  if (section_index == N_DEBUG)
    return bfd_abs_section_ptr;

  while (answer)
    {
      if (answer->target_index == section_index)
	return answer;
      answer = answer->next;
    }

  /* We should not reach this point, but the SCO 3.2v4 /lib/libc_s.a
     has a bad symbol table in biglitpow.o.  */
  return bfd_und_section_ptr;
}

/* Get the upper bound of a COFF symbol table.  */

long
coff_get_symtab_upper_bound (bfd *abfd)
{
  if (!bfd_coff_slurp_symbol_table (abfd))
    return -1;

  return (bfd_get_symcount (abfd) + 1) * (sizeof (coff_symbol_type *));
}

/* Canonicalize a COFF symbol table.  */

long
coff_canonicalize_symtab (bfd *abfd, asymbol **alocation)
{
  unsigned int counter;
  coff_symbol_type *symbase;
  coff_symbol_type **location = (coff_symbol_type **) alocation;

  if (!bfd_coff_slurp_symbol_table (abfd))
    return -1;

  symbase = obj_symbols (abfd);
  counter = bfd_get_symcount (abfd);
  while (counter-- > 0)
    *location++ = symbase++;

  *location = NULL;

  return bfd_get_symcount (abfd);
}

/* Get the name of a symbol.  The caller must pass in a buffer of size
   >= SYMNMLEN + 1.  */

const char *
_bfd_coff_internal_syment_name (bfd *abfd ATTRIBUTE_UNUSED,
                const struct internal_syment *sym ATTRIBUTE_UNUSED,
                char *buf ATTRIBUTE_UNUSED)
{
  return NULL;
}

alent *
coff_get_lineno (bfd *ignore_abfd ATTRIBUTE_UNUSED, asymbol *symbol ATTRIBUTE_UNUSED)
{
  return NULL;
}

/* This function transforms the offsets into the symbol table into
   pointers to syments.  */

static void
coff_pointerize_aux (bfd *abfd,
             combined_entry_type *table_base,
             combined_entry_type *symbol,
             unsigned int indaux,
             combined_entry_type *auxent,
             combined_entry_type *table_end)
{
  unsigned int type = symbol->u.syment.n_type;
  unsigned int n_sclass = symbol->u.syment.n_sclass;

  BFD_ASSERT (symbol->is_sym);
  if (coff_backend_info (abfd)->_bfd_coff_pointerize_aux_hook)
    {
      if ((*coff_backend_info (abfd)->_bfd_coff_pointerize_aux_hook)
      (abfd, table_base, symbol, indaux, auxent))
    return;
    }

  /* Don't bother if this is a file or a section.  */
  if (n_sclass == C_STAT && type == T_NULL)
    return;
  if (n_sclass == C_FILE)
    return;

  BFD_ASSERT (! auxent->is_sym);
  /* Otherwise patch up.  */
#define N_TMASK coff_data  (abfd)->local_n_tmask
#define N_BTSHFT coff_data (abfd)->local_n_btshft

  if ((ISFCN (type) || ISTAG (n_sclass) || n_sclass == C_BLOCK
       || n_sclass == C_FCN)
      && auxent->u.auxent.x_sym.x_fcnary.x_fcn.x_endndx.l > 0
      && auxent->u.auxent.x_sym.x_fcnary.x_fcn.x_endndx.l
      < (long) obj_raw_syment_count (abfd)
      && table_base + auxent->u.auxent.x_sym.x_fcnary.x_fcn.x_endndx.l
      < table_end)
    {
      auxent->u.auxent.x_sym.x_fcnary.x_fcn.x_endndx.p =
    table_base + auxent->u.auxent.x_sym.x_fcnary.x_fcn.x_endndx.l;
      auxent->fix_end = 1;
    }

  /* A negative tagndx is meaningless, but the SCO 3.2v4 cc can
     generate one, so we must be careful to ignore it.  */
  if ((unsigned long) auxent->u.auxent.x_sym.x_tagndx.l
      < obj_raw_syment_count (abfd)
      && table_base + auxent->u.auxent.x_sym.x_tagndx.l < table_end)
    {
      auxent->u.auxent.x_sym.x_tagndx.p =
    table_base + auxent->u.auxent.x_sym.x_tagndx.l;
      auxent->fix_tag = 1;
    }
}

/* Allocate space for the ".debug" section, and read it.
   We did not read the debug section until now, because
   we didn't want to go to the trouble until someone needed it.  */

static char *
build_debug_section (bfd *abfd, asection ** sect_return)
{
  char *debug_section;
  file_ptr position;
  bfd_size_type sec_size;

  asection *sect = bfd_get_section_by_name (abfd, ".debug");

  if (!sect)
    {
      bfd_set_error (bfd_error_no_debug_section);
      return NULL;
    }

  sec_size = sect->size;
  debug_section = (char *) bfd_alloc (abfd, sec_size);
  if (debug_section == NULL)
    return NULL;

  /* Seek to the beginning of the `.debug' section and read it.
     Save the current position first; it is needed by our caller.
     Then read debug section and reset the file pointer.  */

  position = bfd_tell (abfd);
  if (bfd_seek (abfd, sect->filepos, SEEK_SET) != 0
      || bfd_bread (debug_section, sec_size, abfd) != sec_size
      || bfd_seek (abfd, position, SEEK_SET) != 0)
    return NULL;

  * sect_return = sect;
  return debug_section;
}

/* Return a pointer to a malloc'd copy of 'name'.  'name' may not be
   \0-terminated, but will not exceed 'maxlen' characters.  The copy *will*
   be \0-terminated.  */

static char *
copy_name (bfd *abfd, char *name, size_t maxlen)
{
  size_t len;
  char *newname;

  for (len = 0; len < maxlen; ++len)
    if (name[len] == '\0')
      break;

  if ((newname = (char *) bfd_alloc (abfd, (bfd_size_type) len + 1)) == NULL)
    return NULL;

  strncpy (newname, name, len);
  newname[len] = '\0';
  return newname;
}

/* Read in the external symbols.  */

bfd_boolean
_bfd_coff_get_external_symbols (bfd *abfd)
{
  bfd_size_type symesz;
  bfd_size_type size;
  void * syms;

  if (obj_coff_external_syms (abfd) != NULL)
    return TRUE;

  symesz = bfd_coff_symesz (abfd);

  size = obj_raw_syment_count (abfd) * symesz;
  if (size == 0)
    return TRUE;
  /* Check for integer overflow and for unreasonable symbol counts.  */
  if (size < obj_raw_syment_count (abfd)
      || (bfd_get_file_size (abfd) > 0
	  && size > bfd_get_file_size (abfd)))

    {
      _bfd_error_handler (_("%pB: corrupt symbol count: %#" PRIx64 ""),
			  abfd, (uint64_t) obj_raw_syment_count (abfd));
      return FALSE;
    }

  syms = bfd_malloc (size);
  if (syms == NULL)
    {
      /* PR 21013: Provide an error message when the alloc fails.  */
      _bfd_error_handler (_("%pB: not enough memory to allocate space "
			    "for %#" PRIx64 " symbols of size %#" PRIx64),
			  abfd, (uint64_t) obj_raw_syment_count (abfd),
			  (uint64_t) symesz);
      return FALSE;
    }

  if (bfd_seek (abfd, obj_sym_filepos (abfd), SEEK_SET) != 0
      || bfd_bread (syms, size, abfd) != size)
    {
      if (syms != NULL)
	free (syms);
      return FALSE;
    }

  obj_coff_external_syms (abfd) = syms;
  return TRUE;
}

/* Read in the external strings.  The strings are not loaded until
   they are needed.  This is because we have no simple way of
   detecting a missing string table in an archive.  If the strings
   are loaded then the STRINGS and STRINGS_LEN fields in the
   coff_tdata structure will be set.  */

const char *
_bfd_coff_read_string_table (bfd *abfd)
{
  char extstrsize[STRING_SIZE_SIZE];
  bfd_size_type strsize;
  char *strings;
  file_ptr pos;

  if (obj_coff_strings (abfd) != NULL)
    return obj_coff_strings (abfd);

  if (obj_sym_filepos (abfd) == 0)
    {
      bfd_set_error (bfd_error_no_symbols);
      return NULL;
    }

  pos = obj_sym_filepos (abfd);
  pos += obj_raw_syment_count (abfd) * bfd_coff_symesz (abfd);
  if (bfd_seek (abfd, pos, SEEK_SET) != 0)
    return NULL;

  if (bfd_bread (extstrsize, (bfd_size_type) sizeof extstrsize, abfd)
      != sizeof extstrsize)
    {
      if (bfd_get_error () != bfd_error_file_truncated)
    return NULL;

      /* There is no string table.  */
      strsize = STRING_SIZE_SIZE;
    }
  else
    {
#if STRING_SIZE_SIZE == 4
      strsize = H_GET_32 (abfd, extstrsize);
#else
 #error Change H_GET_32
#endif
    }

  if (strsize < STRING_SIZE_SIZE || strsize > bfd_get_file_size (abfd))
    {
      _bfd_error_handler
    /* xgettext: c-format */
    (_("%pB: bad string table size %" PRIu64), abfd, (uint64_t) strsize);
      bfd_set_error (bfd_error_bad_value);
      return NULL;
    }

  strings = (char *) bfd_malloc (strsize + 1);
  if (strings == NULL)
    return NULL;

  /* PR 17521 file: 079-54929-0.004.
     A corrupt file could contain an index that points into the first
     STRING_SIZE_SIZE bytes of the string table, so make sure that
     they are zero.  */
  memset (strings, 0, STRING_SIZE_SIZE);

  if (bfd_bread (strings + STRING_SIZE_SIZE, strsize - STRING_SIZE_SIZE, abfd)
      != strsize - STRING_SIZE_SIZE)
    {
      free (strings);
      return NULL;
    }

  obj_coff_strings (abfd) = strings;
  obj_coff_strings_len (abfd) = strsize;
  /* Terminate the string table, just in case.  */
  strings[strsize] = 0;
  return strings;
}

/* Free up the external symbols and strings read from a COFF file.  */

bfd_boolean
_bfd_coff_free_symbols (bfd *abfd)
{
  if (! bfd_family_coff (abfd))
    return FALSE;

  if (obj_coff_external_syms (abfd) != NULL
      && ! obj_coff_keep_syms (abfd))
    {
      free (obj_coff_external_syms (abfd));
      obj_coff_external_syms (abfd) = NULL;
    }

  if (obj_coff_strings (abfd) != NULL
      && ! obj_coff_keep_strings (abfd))
    {
      free (obj_coff_strings (abfd));
      obj_coff_strings (abfd) = NULL;
      obj_coff_strings_len (abfd) = 0;
    }

  return TRUE;
}

/* Read a symbol table into freshly bfd_allocated memory, swap it, and
   knit the symbol names into a normalized form.  By normalized here I
   mean that all symbols have an n_offset pointer that points to a null-
   terminated string.  */

combined_entry_type *
coff_get_normalized_symtab (bfd *abfd)
{
  combined_entry_type *internal;
  combined_entry_type *internal_ptr;
  combined_entry_type *symbol_ptr;
  combined_entry_type *internal_end;
  size_t symesz;
  char *raw_src;
  char *raw_end;
  const char *string_table = NULL;
  asection * debug_sec = NULL;
  char *debug_sec_data = NULL;
  bfd_size_type size;

  if (obj_raw_syments (abfd) != NULL)
    return obj_raw_syments (abfd);

  if (! _bfd_coff_get_external_symbols (abfd))
    return NULL;

  size = obj_raw_syment_count (abfd) * sizeof (combined_entry_type);
  /* Check for integer overflow.  */
  if (size < obj_raw_syment_count (abfd))
    return NULL;
  internal = (combined_entry_type *) bfd_zalloc (abfd, size);
  if (internal == NULL && size != 0)
    return NULL;
  internal_end = internal + obj_raw_syment_count (abfd);

  raw_src = (char *) obj_coff_external_syms (abfd);

  /* Mark the end of the symbols.  */
  symesz = bfd_coff_symesz (abfd);
  raw_end = (char *) raw_src + obj_raw_syment_count (abfd) * symesz;

  /* FIXME SOMEDAY.  A string table size of zero is very weird, but
     probably possible.  If one shows up, it will probably kill us.  */

  /* Swap all the raw entries.  */
  for (internal_ptr = internal;
       raw_src < raw_end;
       raw_src += symesz, internal_ptr++)
    {
      unsigned int i;

      bfd_coff_swap_sym_in (abfd, (void *) raw_src,
			    (void *) & internal_ptr->u.syment);
      symbol_ptr = internal_ptr;
      internal_ptr->is_sym = TRUE;

      /* PR 17512: file: 1353-1166-0.004.  */
      if (symbol_ptr->u.syment.n_sclass == C_FILE
	  && symbol_ptr->u.syment.n_numaux > 0
	  && raw_src + symesz + symbol_ptr->u.syment.n_numaux
	  * symesz > raw_end)
	{
	  bfd_release (abfd, internal);
	  return NULL;
	}

      for (i = 0;
	   i < symbol_ptr->u.syment.n_numaux;
	   i++)
	{
	  internal_ptr++;
	  /* PR 17512: Prevent buffer overrun.  */
	  if (internal_ptr >= internal_end)
	    {
	      bfd_release (abfd, internal);
	      return NULL;
	    }

	  raw_src += symesz;
	  bfd_coff_swap_aux_in (abfd, (void *) raw_src,
				symbol_ptr->u.syment.n_type,
				symbol_ptr->u.syment.n_sclass,
				(int) i, symbol_ptr->u.syment.n_numaux,
				&(internal_ptr->u.auxent));

	  internal_ptr->is_sym = FALSE;
	  coff_pointerize_aux (abfd, internal, symbol_ptr, i,
			       internal_ptr, internal_end);
	}
    }

  /* Free the raw symbols, but not the strings (if we have them).  */
  obj_coff_keep_strings (abfd) = TRUE;
  if (! _bfd_coff_free_symbols (abfd))
    return NULL;

  for (internal_ptr = internal; internal_ptr < internal_end;
       internal_ptr++)
    {
      BFD_ASSERT (internal_ptr->is_sym);

      if (internal_ptr->u.syment.n_sclass == C_FILE
	  && internal_ptr->u.syment.n_numaux > 0)
	{
	  combined_entry_type * aux = internal_ptr + 1;

	  /* Make a file symbol point to the name in the auxent, since
	     the text ".file" is redundant.  */
	  BFD_ASSERT (! aux->is_sym);

	  if (aux->u.auxent.x_file.x_n.x_zeroes == 0)
	    {
	      /* The filename is a long one, point into the string table.  */
	      if (string_table == NULL)
		{
		  string_table = _bfd_coff_read_string_table (abfd);
		  if (string_table == NULL)
		    return NULL;
		}

	      if ((bfd_size_type)(aux->u.auxent.x_file.x_n.x_offset)
		  >= obj_coff_strings_len (abfd))
		internal_ptr->u.syment._n._n_n._n_offset = (bfd_hostptr_t) _("<corrupt>");
	      else
		internal_ptr->u.syment._n._n_n._n_offset =
		  (bfd_hostptr_t) (string_table + (aux->u.auxent.x_file.x_n.x_offset));
	    }
	  else
	    {
	      /* Ordinary short filename, put into memory anyway.  The
		 Microsoft PE tools sometimes store a filename in
		 multiple AUX entries.  */
	      if (internal_ptr->u.syment.n_numaux > 1
		  && coff_data (abfd)->pe)
		internal_ptr->u.syment._n._n_n._n_offset =
		  (bfd_hostptr_t)
		  copy_name (abfd,
			     aux->u.auxent.x_file.x_fname,
			     internal_ptr->u.syment.n_numaux * symesz);
	      else
		internal_ptr->u.syment._n._n_n._n_offset =
		  ((bfd_hostptr_t)
		   copy_name (abfd,
			      aux->u.auxent.x_file.x_fname,
			      (size_t) bfd_coff_filnmlen (abfd)));
	    }
	}
      else
	{
	  if (internal_ptr->u.syment._n._n_n._n_zeroes != 0)
	    {
	      /* This is a "short" name.  Make it long.  */
	      size_t i;
	      char *newstring;

	      /* Find the length of this string without walking into memory
		 that isn't ours.  */
	      for (i = 0; i < 8; ++i)
		if (internal_ptr->u.syment._n._n_name[i] == '\0')
		  break;

	      newstring = (char *) bfd_zalloc (abfd, (bfd_size_type) (i + 1));
	      if (newstring == NULL)
		return NULL;
	      strncpy (newstring, internal_ptr->u.syment._n._n_name, i);
	      internal_ptr->u.syment._n._n_n._n_offset = (bfd_hostptr_t) newstring;
	      internal_ptr->u.syment._n._n_n._n_zeroes = 0;
	    }
	  else if (internal_ptr->u.syment._n._n_n._n_offset == 0)
	    internal_ptr->u.syment._n._n_n._n_offset = (bfd_hostptr_t) "";
	  else if (!bfd_coff_symname_in_debug (abfd, &internal_ptr->u.syment))
	    {
	      /* Long name already.  Point symbol at the string in the
		 table.  */
	      if (string_table == NULL)
		{
		  string_table = _bfd_coff_read_string_table (abfd);
		  if (string_table == NULL)
		    return NULL;
		}
	      if (internal_ptr->u.syment._n._n_n._n_offset >= obj_coff_strings_len (abfd)
		  || string_table + internal_ptr->u.syment._n._n_n._n_offset < string_table)
		internal_ptr->u.syment._n._n_n._n_offset = (bfd_hostptr_t) _("<corrupt>");
	      else
		internal_ptr->u.syment._n._n_n._n_offset =
		  ((bfd_hostptr_t)
		   (string_table
		    + internal_ptr->u.syment._n._n_n._n_offset));
	    }
	  else
	    {
	      /* Long name in debug section.  Very similar.  */
	      if (debug_sec_data == NULL)
		debug_sec_data = build_debug_section (abfd, & debug_sec);
	      if (debug_sec_data != NULL)
		{
		  BFD_ASSERT (debug_sec != NULL);
		  /* PR binutils/17512: Catch out of range offsets into the debug data.  */
		  if (internal_ptr->u.syment._n._n_n._n_offset > debug_sec->size
		      || debug_sec_data + internal_ptr->u.syment._n._n_n._n_offset < debug_sec_data)
		    internal_ptr->u.syment._n._n_n._n_offset = (bfd_hostptr_t) _("<corrupt>");
		  else
		    internal_ptr->u.syment._n._n_n._n_offset = (bfd_hostptr_t)
		      (debug_sec_data + internal_ptr->u.syment._n._n_n._n_offset);
		}
	      else
		internal_ptr->u.syment._n._n_n._n_offset = (bfd_hostptr_t) "";
	    }
	}
      internal_ptr += internal_ptr->u.syment.n_numaux;
    }

  obj_raw_syments (abfd) = internal;
  BFD_ASSERT (obj_raw_syment_count (abfd)
	      == (unsigned int) (internal_ptr - internal));

  return internal;
}

asymbol *
coff_make_empty_symbol (bfd *abfd)
{
  bfd_size_type amt = sizeof (coff_symbol_type);
  coff_symbol_type *new_symbol = (coff_symbol_type *) bfd_zalloc (abfd, amt);

  if (new_symbol == NULL)
    return NULL;
  new_symbol->symbol.section = 0;
  new_symbol->native = NULL;
  new_symbol->lineno = NULL;
  new_symbol->done_lineno = FALSE;
  new_symbol->symbol.the_bfd = abfd;

  return & new_symbol->symbol;
}

/* Make a debugging symbol.  */

asymbol *
coff_bfd_make_debug_symbol (bfd *abfd ATTRIBUTE_UNUSED,
                void * ptr ATTRIBUTE_UNUSED,
                unsigned long sz ATTRIBUTE_UNUSED)
{
  return NULL;
}

void
coff_get_symbol_info (bfd *abfd, asymbol *symbol, symbol_info *ret)
{
  bfd_symbol_info (symbol, ret);

  if (coffsymbol (symbol)->native != NULL
      && coffsymbol (symbol)->native->fix_value
      && coffsymbol (symbol)->native->is_sym)
    ret->value = coffsymbol (symbol)->native->u.syment.n_value -
      (bfd_hostptr_t) obj_raw_syments (abfd);
}

void
coff_print_symbol (bfd *abfd ATTRIBUTE_UNUSED,
           void * filep ATTRIBUTE_UNUSED,
           asymbol *symbol ATTRIBUTE_UNUSED,
           bfd_print_symbol_type how ATTRIBUTE_UNUSED)
{
}

/* Return whether a symbol name implies a local symbol.  In COFF,
   local symbols generally start with ``.L''.  Most targets use this
   function for the is_local_label_name entry point, but some may
   override it.  */

bfd_boolean
_bfd_coff_is_local_label_name (bfd *abfd ATTRIBUTE_UNUSED,
                   const char *name)
{
  return name[0] == '.' && name[1] == 'L';
}

/* Provided a BFD, a section and an offset (in bytes, not octets) into the
   section, calculate and return the name of the source file and the line
   nearest to the wanted location.  */

bfd_boolean
coff_find_nearest_line_with_names (bfd *abfd,
				   asymbol **symbols,
				   asection *section,
				   bfd_vma offset,
				   const char **filename_ptr,
				   const char **functionname_ptr,
				   unsigned int *line_ptr,
				   const struct dwarf_debug_section *debug_sections)
{
  bfd_boolean found;
  unsigned int i;
  unsigned int line_base;
  coff_data_type *cof = coff_data (abfd);
  /* Run through the raw syments if available.  */
  combined_entry_type *p;
  combined_entry_type *pend;
  alent *l;
  struct coff_section_tdata *sec_data;
  bfd_size_type amt;

  /* Before looking through the symbol table, try to use a .stab
     section to find the information.  */
  if (! _bfd_stab_section_find_nearest_line (abfd, symbols, section, offset,
					     &found, filename_ptr,
					     functionname_ptr, line_ptr,
					     &coff_data(abfd)->line_info))
    return FALSE;

  if (found)
    return TRUE;

  /* Also try examining DWARF2 debugging information.  */
  if (_bfd_dwarf2_find_nearest_line (abfd, symbols, NULL, section, offset,
				     filename_ptr, functionname_ptr,
				     line_ptr, NULL, debug_sections, 0,
				     &coff_data(abfd)->dwarf2_find_line_info))
    return TRUE;

  sec_data = coff_section_data (abfd, section);

  /* If the DWARF lookup failed, but there is DWARF information available
     then the problem might be that the file has been rebased.  This tool
     changes the VMAs of all the sections, but it does not update the DWARF
     information.  So try again, using a bias against the address sought.  */
  if (coff_data (abfd)->dwarf2_find_line_info != NULL)
    {
      bfd_signed_vma bias;

      /* Create a cache of the result for the next call.  */
      if (sec_data == NULL && section->owner == abfd)
	{
	  amt = sizeof (struct coff_section_tdata);
	  section->used_by_bfd = bfd_zalloc (abfd, amt);
	  sec_data = (struct coff_section_tdata *) section->used_by_bfd;
	}

      if (sec_data != NULL && sec_data->saved_bias)
	bias = sec_data->saved_bias;
      else
	{
	  bias = _bfd_dwarf2_find_symbol_bias (symbols,
					       & coff_data (abfd)->dwarf2_find_line_info);
	  if (sec_data)
	    {
	      sec_data->saved_bias = TRUE;
	      sec_data->bias = bias;
	    }
	}

      if (bias
	  && _bfd_dwarf2_find_nearest_line (abfd, symbols, NULL, section,
					    offset + bias,
					    filename_ptr, functionname_ptr,
					    line_ptr, NULL, debug_sections, 0,
					    &coff_data(abfd)->dwarf2_find_line_info))
	return TRUE;
    }

  *filename_ptr = 0;
  *functionname_ptr = 0;
  *line_ptr = 0;

  /* Don't try and find line numbers in a non coff file.  */
  if (!bfd_family_coff (abfd))
    return FALSE;

  if (cof == NULL)
    return FALSE;

  /* Find the first C_FILE symbol.  */
  p = cof->raw_syments;
  if (!p)
    return FALSE;

  pend = p + cof->raw_syment_count;
  while (p < pend)
    {
      BFD_ASSERT (p->is_sym);
      if (p->u.syment.n_sclass == C_FILE)
	break;
      p += 1 + p->u.syment.n_numaux;
    }

  if (p < pend)
    {
      bfd_vma sec_vma;
      bfd_vma maxdiff;

      /* Look through the C_FILE symbols to find the best one.  */
      sec_vma = bfd_get_section_vma (abfd, section);
      *filename_ptr = (char *) p->u.syment._n._n_n._n_offset;
      maxdiff = (bfd_vma) 0 - (bfd_vma) 1;
      while (1)
	{
	  bfd_vma file_addr;
	  combined_entry_type *p2;

	  for (p2 = p + 1 + p->u.syment.n_numaux;
	       p2 < pend;
	       p2 += 1 + p2->u.syment.n_numaux)
	    {
	      BFD_ASSERT (p2->is_sym);
	      if (p2->u.syment.n_scnum > 0
		  && (section
		      == coff_section_from_bfd_index (abfd,
						      p2->u.syment.n_scnum)))
		break;
	      if (p2->u.syment.n_sclass == C_FILE)
		{
		  p2 = pend;
		  break;
		}
	    }
	  if (p2 >= pend)
	    break;

	  file_addr = (bfd_vma) p2->u.syment.n_value;
	  /* PR 11512: Include the section address of the function name symbol.  */
	  if (p2->u.syment.n_scnum > 0)
	    file_addr += coff_section_from_bfd_index (abfd,
						      p2->u.syment.n_scnum)->vma;
	  /* We use <= MAXDIFF here so that if we get a zero length
	     file, we actually use the next file entry.  */
	  if (p2 < pend
	      && offset + sec_vma >= file_addr
	      && offset + sec_vma - file_addr <= maxdiff)
	    {
	      *filename_ptr = (char *) p->u.syment._n._n_n._n_offset;
	      maxdiff = offset + sec_vma - p2->u.syment.n_value;
	    }

	  /* Avoid endless loops on erroneous files by ensuring that
	     we always move forward in the file.  */
	  if (p >= cof->raw_syments + p->u.syment.n_value)
	    break;

	  p = cof->raw_syments + p->u.syment.n_value;
	  if (p > pend || p->u.syment.n_sclass != C_FILE)
	    break;
	}
    }

  if (section->lineno_count == 0)
    {
      *functionname_ptr = NULL;
      *line_ptr = 0;
      return TRUE;
    }

  /* Now wander though the raw linenumbers of the section.
     If we have been called on this section before, and the offset
     we want is further down then we can prime the lookup loop.  */
  if (sec_data != NULL
      && sec_data->i > 0
      && offset >= sec_data->offset)
    {
      i = sec_data->i;
      *functionname_ptr = sec_data->function;
      line_base = sec_data->line_base;
    }
  else
    {
      i = 0;
      line_base = 0;
    }

  if (section->lineno != NULL)
    {
      bfd_vma last_value = 0;

      l = &section->lineno[i];

      for (; i < section->lineno_count; i++)
	{
	  if (l->line_number == 0)
	    {
	      /* Get the symbol this line number points at.  */
	      coff_symbol_type *coff = (coff_symbol_type *) (l->u.sym);
	      if (coff->symbol.value > offset)
		break;

	      *functionname_ptr = coff->symbol.name;
	      last_value = coff->symbol.value;
	      if (coff->native)
		{
		  combined_entry_type *s = coff->native;

		  BFD_ASSERT (s->is_sym);
		  s = s + 1 + s->u.syment.n_numaux;

		  /* In XCOFF a debugging symbol can follow the
		     function symbol.  */
		  if (s->u.syment.n_scnum == N_DEBUG)
		    s = s + 1 + s->u.syment.n_numaux;

		  /* S should now point to the .bf of the function.  */
		  if (s->u.syment.n_numaux)
		    {
		      /* The linenumber is stored in the auxent.  */
		      union internal_auxent *a = &((s + 1)->u.auxent);

		      line_base = a->x_sym.x_misc.x_lnsz.x_lnno;
		      *line_ptr = line_base;
		    }
		}
	    }
	  else
	    {
	      if (l->u.offset > offset)
		break;
	      *line_ptr = l->line_number + line_base - 1;
	    }
	  l++;
	}

      /* If we fell off the end of the loop, then assume that this
	 symbol has no line number info.  Otherwise, symbols with no
	 line number info get reported with the line number of the
	 last line of the last symbol which does have line number
	 info.  We use 0x100 as a slop to account for cases where the
	 last line has executable code.  */
      if (i >= section->lineno_count
	  && last_value != 0
	  && offset - last_value > 0x100)
	{
	  *functionname_ptr = NULL;
	  *line_ptr = 0;
	}
    }

  /* Cache the results for the next call.  */
  if (sec_data == NULL && section->owner == abfd)
    {
      amt = sizeof (struct coff_section_tdata);
      section->used_by_bfd = bfd_zalloc (abfd, amt);
      sec_data = (struct coff_section_tdata *) section->used_by_bfd;
    }

  if (sec_data != NULL)
    {
      sec_data->offset = offset;
      sec_data->i = i - 1;
      sec_data->function = *functionname_ptr;
      sec_data->line_base = line_base;
    }

  return TRUE;
}

bfd_boolean
coff_find_nearest_line (bfd *abfd,
            asymbol **symbols,
            asection *section,
            bfd_vma offset,
            const char **filename_ptr,
            const char **functionname_ptr,
            unsigned int *line_ptr,
            unsigned int *discriminator_ptr)
{
  if (discriminator_ptr)
    *discriminator_ptr = 0;
  return coff_find_nearest_line_with_names (abfd, symbols, section, offset,
                        filename_ptr, functionname_ptr,
                        line_ptr, dwarf_debug_sections);
}

bfd_boolean
coff_find_inliner_info (bfd *abfd ATTRIBUTE_UNUSED,
            const char **filename_ptr ATTRIBUTE_UNUSED,
            const char **functionname_ptr ATTRIBUTE_UNUSED,
            unsigned int *line_ptr ATTRIBUTE_UNUSED)
{
  return FALSE;
}
