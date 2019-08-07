/* ELF executable support for BFD.

   Copyright (C) 1993-2019 Free Software Foundation, Inc.

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


/*
SECTION
	ELF backends

	BFD support for ELF formats is being worked on.
	Currently, the best supported back ends are for sparc and i386
	(running svr4 or Solaris 2).

	Documentation of the internals of the support code still needs
	to be written.  The code is changing quickly enough that we
	haven't bothered yet.  */

/* For sparc64-cross-sparc32.  */
#define _SYSCALL32
#include "sysdep.h"
#include <limits.h>
#include "bfd.h"
#include "bfdlink.h"
#include "libbfd.h"
#define ARCH_SIZE 0
#include "elf-bfd.h"
#include "libiberty.h"
#include "elf-linux-core.h"

#ifdef CORE_HEADER
#include CORE_HEADER
#endif

#include <ctype.h>

static int elf_sort_sections (const void *, const void *);
static bfd_boolean elf_read_notes (bfd *, file_ptr, bfd_size_type,
				   size_t align) ;
static bfd_boolean elf_parse_notes (bfd *abfd, char *buf, size_t size,
				    file_ptr offset, size_t align);

/* Swap version information in and out.  The version information is
   currently size independent.  If that ever changes, this code will
   need to move into elfcode.h.  */

/* Swap in a Verdef structure.  */

void
_bfd_elf_swap_verdef_in (bfd *abfd,
			 const Elf_External_Verdef *src,
			 Elf_Internal_Verdef *dst)
{
  dst->vd_version = H_GET_16 (abfd, src->vd_version);
  dst->vd_flags   = H_GET_16 (abfd, src->vd_flags);
  dst->vd_ndx     = H_GET_16 (abfd, src->vd_ndx);
  dst->vd_cnt     = H_GET_16 (abfd, src->vd_cnt);
  dst->vd_hash    = H_GET_32 (abfd, src->vd_hash);
  dst->vd_aux     = H_GET_32 (abfd, src->vd_aux);
  dst->vd_next    = H_GET_32 (abfd, src->vd_next);
}

/* Swap out a Verdef structure.  */

void
_bfd_elf_swap_verdef_out (bfd *abfd,
			  const Elf_Internal_Verdef *src,
			  Elf_External_Verdef *dst)
{
  H_PUT_16 (abfd, src->vd_version, dst->vd_version);
  H_PUT_16 (abfd, src->vd_flags, dst->vd_flags);
  H_PUT_16 (abfd, src->vd_ndx, dst->vd_ndx);
  H_PUT_16 (abfd, src->vd_cnt, dst->vd_cnt);
  H_PUT_32 (abfd, src->vd_hash, dst->vd_hash);
  H_PUT_32 (abfd, src->vd_aux, dst->vd_aux);
  H_PUT_32 (abfd, src->vd_next, dst->vd_next);
}

/* Swap in a Verdaux structure.  */

void
_bfd_elf_swap_verdaux_in (bfd *abfd,
			  const Elf_External_Verdaux *src,
			  Elf_Internal_Verdaux *dst)
{
  dst->vda_name = H_GET_32 (abfd, src->vda_name);
  dst->vda_next = H_GET_32 (abfd, src->vda_next);
}

/* Swap out a Verdaux structure.  */

void
_bfd_elf_swap_verdaux_out (bfd *abfd,
			   const Elf_Internal_Verdaux *src,
			   Elf_External_Verdaux *dst)
{
  H_PUT_32 (abfd, src->vda_name, dst->vda_name);
  H_PUT_32 (abfd, src->vda_next, dst->vda_next);
}

/* Swap in a Verneed structure.  */

void
_bfd_elf_swap_verneed_in (bfd *abfd,
			  const Elf_External_Verneed *src,
			  Elf_Internal_Verneed *dst)
{
  dst->vn_version = H_GET_16 (abfd, src->vn_version);
  dst->vn_cnt     = H_GET_16 (abfd, src->vn_cnt);
  dst->vn_file    = H_GET_32 (abfd, src->vn_file);
  dst->vn_aux     = H_GET_32 (abfd, src->vn_aux);
  dst->vn_next    = H_GET_32 (abfd, src->vn_next);
}

/* Swap out a Verneed structure.  */

void
_bfd_elf_swap_verneed_out (bfd *abfd,
			   const Elf_Internal_Verneed *src,
			   Elf_External_Verneed *dst)
{
  H_PUT_16 (abfd, src->vn_version, dst->vn_version);
  H_PUT_16 (abfd, src->vn_cnt, dst->vn_cnt);
  H_PUT_32 (abfd, src->vn_file, dst->vn_file);
  H_PUT_32 (abfd, src->vn_aux, dst->vn_aux);
  H_PUT_32 (abfd, src->vn_next, dst->vn_next);
}

/* Swap in a Vernaux structure.  */

void
_bfd_elf_swap_vernaux_in (bfd *abfd,
			  const Elf_External_Vernaux *src,
			  Elf_Internal_Vernaux *dst)
{
  dst->vna_hash  = H_GET_32 (abfd, src->vna_hash);
  dst->vna_flags = H_GET_16 (abfd, src->vna_flags);
  dst->vna_other = H_GET_16 (abfd, src->vna_other);
  dst->vna_name  = H_GET_32 (abfd, src->vna_name);
  dst->vna_next  = H_GET_32 (abfd, src->vna_next);
}

/* Swap out a Vernaux structure.  */

void
_bfd_elf_swap_vernaux_out (bfd *abfd,
			   const Elf_Internal_Vernaux *src,
			   Elf_External_Vernaux *dst)
{
  H_PUT_32 (abfd, src->vna_hash, dst->vna_hash);
  H_PUT_16 (abfd, src->vna_flags, dst->vna_flags);
  H_PUT_16 (abfd, src->vna_other, dst->vna_other);
  H_PUT_32 (abfd, src->vna_name, dst->vna_name);
  H_PUT_32 (abfd, src->vna_next, dst->vna_next);
}

/* Swap in a Versym structure.  */

void
_bfd_elf_swap_versym_in (bfd *abfd,
			 const Elf_External_Versym *src,
			 Elf_Internal_Versym *dst)
{
  dst->vs_vers = H_GET_16 (abfd, src->vs_vers);
}

/* Swap out a Versym structure.  */

void
_bfd_elf_swap_versym_out (bfd *abfd,
			  const Elf_Internal_Versym *src,
			  Elf_External_Versym *dst)
{
  H_PUT_16 (abfd, src->vs_vers, dst->vs_vers);
}

/* Standard ELF hash function.  Do not change this function; you will
   cause invalid hash tables to be generated.  */

unsigned long
bfd_elf_hash (const char *namearg)
{
  const unsigned char *name = (const unsigned char *) namearg;
  unsigned long h = 0;
  unsigned long g;
  int ch;

  while ((ch = *name++) != '\0')
    {
      h = (h << 4) + ch;
      if ((g = (h & 0xf0000000)) != 0)
	{
	  h ^= g >> 24;
	  /* The ELF ABI says `h &= ~g', but this is equivalent in
	     this case and on some machines one insn instead of two.  */
	  h ^= g;
	}
    }
  return h & 0xffffffff;
}

/* DT_GNU_HASH hash function.  Do not change this function; you will
   cause invalid hash tables to be generated.  */

unsigned long
bfd_elf_gnu_hash (const char *namearg)
{
  const unsigned char *name = (const unsigned char *) namearg;
  unsigned long h = 5381;
  unsigned char ch;

  while ((ch = *name++) != '\0')
    h = (h << 5) + h + ch;
  return h & 0xffffffff;
}

/* Create a tdata field OBJECT_SIZE bytes in length, zeroed out and with
   the object_id field of an elf_obj_tdata field set to OBJECT_ID.  */
bfd_boolean
bfd_elf_allocate_object (bfd *abfd,
			 size_t object_size,
			 enum elf_target_id object_id)
{
  BFD_ASSERT (object_size >= sizeof (struct elf_obj_tdata));
  abfd->tdata.any = bfd_zalloc (abfd, object_size);
  if (abfd->tdata.any == NULL)
    return FALSE;

  elf_object_id (abfd) = object_id;
  return TRUE;
}


bfd_boolean
bfd_elf_make_object (bfd *abfd)
{
  const struct elf_backend_data *bed = get_elf_backend_data (abfd);
  return bfd_elf_allocate_object (abfd, sizeof (struct elf_obj_tdata),
				  bed->target_id);
}

bfd_boolean
bfd_elf_mkcorefile (bfd *abfd)
{
  /* I think this can be done just like an object file.  */
  if (!abfd->xvec->_bfd_set_format[(int) bfd_object] (abfd))
    return FALSE;
  elf_tdata (abfd)->core = bfd_zalloc (abfd, sizeof (*elf_tdata (abfd)->core));
  return elf_tdata (abfd)->core != NULL;
}

static char *
bfd_elf_get_str_section (bfd *abfd, unsigned int shindex)
{
  Elf_Internal_Shdr **i_shdrp;
  bfd_byte *shstrtab = NULL;
  file_ptr offset;
  bfd_size_type shstrtabsize;

  i_shdrp = elf_elfsections (abfd);
  if (i_shdrp == 0
      || shindex >= elf_numsections (abfd)
      || i_shdrp[shindex] == 0)
    return NULL;

  shstrtab = i_shdrp[shindex]->contents;
  if (shstrtab == NULL)
    {
      /* No cached one, attempt to read, and cache what we read.  */
      offset = i_shdrp[shindex]->sh_offset;
      shstrtabsize = i_shdrp[shindex]->sh_size;

      /* Allocate and clear an extra byte at the end, to prevent crashes
	 in case the string table is not terminated.  */
      if (shstrtabsize + 1 <= 1
	  || shstrtabsize > bfd_get_file_size (abfd)
	  || bfd_seek (abfd, offset, SEEK_SET) != 0
	  || (shstrtab = (bfd_byte *) bfd_alloc (abfd, shstrtabsize + 1)) == NULL)
	shstrtab = NULL;
      else if (bfd_bread (shstrtab, shstrtabsize, abfd) != shstrtabsize)
	{
	  if (bfd_get_error () != bfd_error_system_call)
	    bfd_set_error (bfd_error_file_truncated);
	  bfd_release (abfd, shstrtab);
	  shstrtab = NULL;
	  /* Once we've failed to read it, make sure we don't keep
	     trying.  Otherwise, we'll keep allocating space for
	     the string table over and over.  */
	  i_shdrp[shindex]->sh_size = 0;
	}
      else
	shstrtab[shstrtabsize] = '\0';
      i_shdrp[shindex]->contents = shstrtab;
    }
  return (char *) shstrtab;
}

char *
bfd_elf_string_from_elf_section (bfd *abfd,
				 unsigned int shindex,
				 unsigned int strindex)
{
  Elf_Internal_Shdr *hdr;

  if (strindex == 0)
    return "";

  if (elf_elfsections (abfd) == NULL || shindex >= elf_numsections (abfd))
    return NULL;

  hdr = elf_elfsections (abfd)[shindex];

  if (hdr->contents == NULL)
    {
      if (hdr->sh_type != SHT_STRTAB && hdr->sh_type < SHT_LOOS)
	{
	  /* PR 17512: file: f057ec89.  */
	  /* xgettext:c-format */
	  _bfd_error_handler (_("%pB: attempt to load strings from"
				" a non-string section (number %d)"),
			      abfd, shindex);
	  return NULL;
	}

      if (bfd_elf_get_str_section (abfd, shindex) == NULL)
	return NULL;
    }

  if (strindex >= hdr->sh_size)
    {
      unsigned int shstrndx = elf_elfheader(abfd)->e_shstrndx;
      _bfd_error_handler
	/* xgettext:c-format */
	(_("%pB: invalid string offset %u >= %" PRIu64 " for section `%s'"),
	 abfd, strindex, (uint64_t) hdr->sh_size,
	 (shindex == shstrndx && strindex == hdr->sh_name
	  ? ".shstrtab"
	  : bfd_elf_string_from_elf_section (abfd, shstrndx, hdr->sh_name)));
      return NULL;
    }

  return ((char *) hdr->contents) + strindex;
}

bfd_boolean
elf_swap_symbol_in (bfd *abfd,
                    const void *psrc,
                    const void *pshn,
                    Elf_Internal_Sym *dst);

/* Read and convert symbols to internal format.
   SYMCOUNT specifies the number of symbols to read, starting from
   symbol SYMOFFSET.  If any of INTSYM_BUF, EXTSYM_BUF or EXTSHNDX_BUF
   are non-NULL, they are used to store the internal symbols, external
   symbols, and symbol section index extensions, respectively.
   Returns a pointer to the internal symbol buffer (malloced if necessary)
   or NULL if there were no symbols or some kind of problem.  */

Elf_Internal_Sym *
bfd_elf_get_elf_syms (bfd *ibfd,
		      Elf_Internal_Shdr *symtab_hdr,
		      size_t symcount,
		      size_t symoffset,
		      Elf_Internal_Sym *intsym_buf,
		      void *extsym_buf,
		      Elf_External_Sym_Shndx *extshndx_buf)
{
  Elf_Internal_Shdr *shndx_hdr;
  void *alloc_ext;
  const bfd_byte *esym;
  Elf_External_Sym_Shndx *alloc_extshndx;
  Elf_External_Sym_Shndx *shndx;
  Elf_Internal_Sym *alloc_intsym;
  Elf_Internal_Sym *isym;
  Elf_Internal_Sym *isymend;
  size_t extsym_size;
  bfd_size_type amt;
  file_ptr pos;

  if (bfd_get_flavour (ibfd) != bfd_target_elf_flavour)
    abort ();

  if (symcount == 0)
    return intsym_buf;

  /* Normal syms might have section extension entries.  */
  shndx_hdr = NULL;
  if (elf_symtab_shndx_list (ibfd) != NULL)
    {
      elf_section_list * entry;
      Elf_Internal_Shdr **sections = elf_elfsections (ibfd);

      /* Find an index section that is linked to this symtab section.  */
      for (entry = elf_symtab_shndx_list (ibfd); entry != NULL; entry = entry->next)
	{
	  /* PR 20063.  */
	  if (entry->hdr.sh_link >= elf_numsections (ibfd))
	    continue;

	  if (sections[entry->hdr.sh_link] == symtab_hdr)
	    {
	      shndx_hdr = & entry->hdr;
	      break;
	    };
	}

      if (shndx_hdr == NULL)
	{
	  if (symtab_hdr == & elf_symtab_hdr (ibfd))
	    /* Not really accurate, but this was how the old code used to work.  */
	    shndx_hdr = & elf_symtab_shndx_list (ibfd)->hdr;
	  /* Otherwise we do nothing.  The assumption is that
	     the index table will not be needed.  */
	}
    }

  /* Read the symbols.  */
  alloc_ext = NULL;
  alloc_extshndx = NULL;
  alloc_intsym = NULL;
  extsym_size = sizeof (Elf64_External_Sym);
  amt = (bfd_size_type) symcount * extsym_size;
  pos = symtab_hdr->sh_offset + symoffset * extsym_size;
  if (extsym_buf == NULL)
    {
      alloc_ext = bfd_malloc2 (symcount, extsym_size);
      extsym_buf = alloc_ext;
    }
  if (extsym_buf == NULL
      || bfd_seek (ibfd, pos, SEEK_SET) != 0
      || bfd_bread (extsym_buf, amt, ibfd) != amt)
    {
      intsym_buf = NULL;
      goto out;
    }

  if (shndx_hdr == NULL || shndx_hdr->sh_size == 0)
    extshndx_buf = NULL;
  else
    {
      amt = (bfd_size_type) symcount * sizeof (Elf_External_Sym_Shndx);
      pos = shndx_hdr->sh_offset + symoffset * sizeof (Elf_External_Sym_Shndx);
      if (extshndx_buf == NULL)
	{
	  alloc_extshndx = (Elf_External_Sym_Shndx *)
	      bfd_malloc2 (symcount, sizeof (Elf_External_Sym_Shndx));
	  extshndx_buf = alloc_extshndx;
	}
      if (extshndx_buf == NULL
	  || bfd_seek (ibfd, pos, SEEK_SET) != 0
	  || bfd_bread (extshndx_buf, amt, ibfd) != amt)
	{
	  intsym_buf = NULL;
	  goto out;
	}
    }

  if (intsym_buf == NULL)
    {
      alloc_intsym = (Elf_Internal_Sym *)
	  bfd_malloc2 (symcount, sizeof (Elf_Internal_Sym));
      intsym_buf = alloc_intsym;
      if (intsym_buf == NULL)
	goto out;
    }

  /* Convert the symbols to internal form.  */
  isymend = intsym_buf + symcount;
  for (esym = (const bfd_byte *) extsym_buf, isym = intsym_buf,
	   shndx = extshndx_buf;
       isym < isymend;
       esym += extsym_size, isym++, shndx = shndx != NULL ? shndx + 1 : NULL)
    if (!elf_swap_symbol_in (ibfd, esym, shndx, isym))
      {
	symoffset += (esym - (bfd_byte *) extsym_buf) / extsym_size;
	/* xgettext:c-format */
	_bfd_error_handler (_("%pB symbol number %lu references"
			      " nonexistent SHT_SYMTAB_SHNDX section"),
			    ibfd, (unsigned long) symoffset);
	if (alloc_intsym != NULL)
	  free (alloc_intsym);
	intsym_buf = NULL;
	goto out;
      }

 out:
  if (alloc_ext != NULL)
    free (alloc_ext);
  if (alloc_extshndx != NULL)
    free (alloc_extshndx);

  return intsym_buf;
}

/* Look up a symbol name.  */
const char *
bfd_elf_sym_name (bfd *abfd,
		  Elf_Internal_Shdr *symtab_hdr,
		  Elf_Internal_Sym *isym,
		  asection *sym_sec)
{
  const char *name;
  unsigned int iname = isym->st_name;
  unsigned int shindex = symtab_hdr->sh_link;

  if (iname == 0 && ELF_ST_TYPE (isym->st_info) == STT_SECTION
      /* Check for a bogus st_shndx to avoid crashing.  */
      && isym->st_shndx < elf_numsections (abfd))
    {
      iname = elf_elfsections (abfd)[isym->st_shndx]->sh_name;
      shindex = elf_elfheader (abfd)->e_shstrndx;
    }

  name = bfd_elf_string_from_elf_section (abfd, shindex, iname);
  if (name == NULL)
    name = "(null)";
  else if (sym_sec && *name == '\0')
    name = bfd_section_name (abfd, sym_sec);

  return name;
}

/* Elf_Internal_Shdr->contents is an array of these for SHT_GROUP
   sections.  The first element is the flags, the rest are section
   pointers.  */

typedef union elf_internal_group {
  Elf_Internal_Shdr *shdr;
  unsigned int flags;
} Elf_Internal_Group;

/* Return the name of the group signature symbol.  Why isn't the
   signature just a string?  */

static const char *
group_signature (bfd *abfd, Elf_Internal_Shdr *ghdr)
{
  Elf_Internal_Shdr *hdr;
  unsigned char esym[sizeof (Elf64_External_Sym)];
  Elf_External_Sym_Shndx eshndx;
  Elf_Internal_Sym isym;

  /* First we need to ensure the symbol table is available.  Make sure
     that it is a symbol table section.  */
  if (ghdr->sh_link >= elf_numsections (abfd))
    return NULL;
  hdr = elf_elfsections (abfd) [ghdr->sh_link];
  if (hdr->sh_type != SHT_SYMTAB
      || ! bfd_section_from_shdr (abfd, ghdr->sh_link))
    return NULL;

  /* Go read the symbol.  */
  hdr = &elf_tdata (abfd)->symtab_hdr;
  if (bfd_elf_get_elf_syms (abfd, hdr, 1, ghdr->sh_info,
			    &isym, esym, &eshndx) == NULL)
    return NULL;

  return bfd_elf_sym_name (abfd, hdr, &isym, NULL);
}

/* Set next_in_group list pointer, and group name for NEWSECT.  */

static bfd_boolean
setup_group (bfd *abfd, Elf_Internal_Shdr *hdr, asection *newsect)
{
  unsigned int num_group = elf_tdata (abfd)->num_group;

  /* If num_group is zero, read in all SHT_GROUP sections.  The count
     is set to -1 if there are no SHT_GROUP sections.  */
  if (num_group == 0)
    {
      unsigned int i, shnum;

      /* First count the number of groups.  If we have a SHT_GROUP
	 section with just a flag word (ie. sh_size is 4), ignore it.  */
      shnum = elf_numsections (abfd);
      num_group = 0;

#define IS_VALID_GROUP_SECTION_HEADER(shdr, minsize)	\
	(   (shdr)->sh_type == SHT_GROUP		\
	 && (shdr)->sh_size >= minsize			\
	 && (shdr)->sh_entsize == GRP_ENTRY_SIZE	\
	 && ((shdr)->sh_size % GRP_ENTRY_SIZE) == 0)

      for (i = 0; i < shnum; i++)
	{
	  Elf_Internal_Shdr *shdr = elf_elfsections (abfd)[i];

	  if (IS_VALID_GROUP_SECTION_HEADER (shdr, 2 * GRP_ENTRY_SIZE))
	    num_group += 1;
	}

      if (num_group == 0)
	{
	  num_group = (unsigned) -1;
	  elf_tdata (abfd)->num_group = num_group;
	  elf_tdata (abfd)->group_sect_ptr = NULL;
	}
      else
	{
	  /* We keep a list of elf section headers for group sections,
	     so we can find them quickly.  */
	  bfd_size_type amt;

	  elf_tdata (abfd)->num_group = num_group;
	  elf_tdata (abfd)->group_sect_ptr = (Elf_Internal_Shdr **)
	      bfd_alloc2 (abfd, num_group, sizeof (Elf_Internal_Shdr *));
	  if (elf_tdata (abfd)->group_sect_ptr == NULL)
	    return FALSE;
	  memset (elf_tdata (abfd)->group_sect_ptr, 0,
		  num_group * sizeof (Elf_Internal_Shdr *));
	  num_group = 0;

	  for (i = 0; i < shnum; i++)
	    {
	      Elf_Internal_Shdr *shdr = elf_elfsections (abfd)[i];

	      if (IS_VALID_GROUP_SECTION_HEADER (shdr, 2 * GRP_ENTRY_SIZE))
		{
		  unsigned char *src;
		  Elf_Internal_Group *dest;

		  /* Make sure the group section has a BFD section
		     attached to it.  */
		  if (!bfd_section_from_shdr (abfd, i))
		    return FALSE;

		  /* Add to list of sections.  */
		  elf_tdata (abfd)->group_sect_ptr[num_group] = shdr;
		  num_group += 1;

		  /* Read the raw contents.  */
		  BFD_ASSERT (sizeof (*dest) >= 4);
		  amt = shdr->sh_size * sizeof (*dest) / 4;
		  shdr->contents = (unsigned char *)
		      bfd_alloc2 (abfd, shdr->sh_size, sizeof (*dest) / 4);
		  /* PR binutils/4110: Handle corrupt group headers.  */
		  if (shdr->contents == NULL)
		    {
		      _bfd_error_handler
			/* xgettext:c-format */
			(_("%pB: corrupt size field in group section"
			   " header: %#" PRIx64),
			 abfd, (uint64_t) shdr->sh_size);
		      bfd_set_error (bfd_error_bad_value);
		      -- num_group;
		      continue;
		    }

		  memset (shdr->contents, 0, amt);

		  if (bfd_seek (abfd, shdr->sh_offset, SEEK_SET) != 0
		      || (bfd_bread (shdr->contents, shdr->sh_size, abfd)
			  != shdr->sh_size))
		    {
		      _bfd_error_handler
			/* xgettext:c-format */
			(_("%pB: invalid size field in group section"
			   " header: %#" PRIx64 ""),
			 abfd, (uint64_t) shdr->sh_size);
		      bfd_set_error (bfd_error_bad_value);
		      -- num_group;
		      /* PR 17510: If the group contents are even
			 partially corrupt, do not allow any of the
			 contents to be used.  */
		      memset (shdr->contents, 0, amt);
		      continue;
		    }

		  /* Translate raw contents, a flag word followed by an
		     array of elf section indices all in target byte order,
		     to the flag word followed by an array of elf section
		     pointers.  */
		  src = shdr->contents + shdr->sh_size;
		  dest = (Elf_Internal_Group *) (shdr->contents + amt);

		  while (1)
		    {
		      unsigned int idx;

		      src -= 4;
		      --dest;
		      idx = H_GET_32 (abfd, src);
		      if (src == shdr->contents)
			{
			  dest->flags = idx;
			  if (shdr->bfd_section != NULL && (idx & GRP_COMDAT))
			    shdr->bfd_section->flags
			      |= SEC_LINK_ONCE | SEC_LINK_DUPLICATES_DISCARD;
			  break;
			}
		      if (idx < shnum)
			{
			  dest->shdr = elf_elfsections (abfd)[idx];
			  /* PR binutils/23199: All sections in a
			     section group should be marked with
			     SHF_GROUP.  But some tools generate
			     broken objects without SHF_GROUP.  Fix
			     them up here.  */
			  dest->shdr->sh_flags |= SHF_GROUP;
			}
		      if (idx >= shnum
			  || dest->shdr->sh_type == SHT_GROUP)
			{
			  _bfd_error_handler
			    (_("%pB: invalid entry in SHT_GROUP section [%u]"),
			       abfd, i);
			  dest->shdr = NULL;
			}
		    }
		}
	    }

	  /* PR 17510: Corrupt binaries might contain invalid groups.  */
	  if (num_group != (unsigned) elf_tdata (abfd)->num_group)
	    {
	      elf_tdata (abfd)->num_group = num_group;

	      /* If all groups are invalid then fail.  */
	      if (num_group == 0)
		{
		  elf_tdata (abfd)->group_sect_ptr = NULL;
		  elf_tdata (abfd)->num_group = num_group = -1;
		  _bfd_error_handler
		    (_("%pB: no valid group sections found"), abfd);
		  bfd_set_error (bfd_error_bad_value);
		}
	    }
	}
    }

  if (num_group != (unsigned) -1)
    {
      unsigned int search_offset = elf_tdata (abfd)->group_search_offset;
      unsigned int j;

      for (j = 0; j < num_group; j++)
	{
	  /* Begin search from previous found group.  */
	  unsigned i = (j + search_offset) % num_group;

	  Elf_Internal_Shdr *shdr = elf_tdata (abfd)->group_sect_ptr[i];
	  Elf_Internal_Group *idx;
	  bfd_size_type n_elt;

	  if (shdr == NULL)
	    continue;

	  idx = (Elf_Internal_Group *) shdr->contents;
	  if (idx == NULL || shdr->sh_size < 4)
	    {
	      /* See PR 21957 for a reproducer.  */
	      /* xgettext:c-format */
	      _bfd_error_handler (_("%pB: group section '%pA' has no contents"),
				  abfd, shdr->bfd_section);
	      elf_tdata (abfd)->group_sect_ptr[i] = NULL;
	      bfd_set_error (bfd_error_bad_value);
	      return FALSE;
	    }
	  n_elt = shdr->sh_size / 4;

	  /* Look through this group's sections to see if current
	     section is a member.  */
	  while (--n_elt != 0)
	    if ((++idx)->shdr == hdr)
	      {
		asection *s = NULL;

		/* We are a member of this group.  Go looking through
		   other members to see if any others are linked via
		   next_in_group.  */
		idx = (Elf_Internal_Group *) shdr->contents;
		n_elt = shdr->sh_size / 4;
		while (--n_elt != 0)
		  if ((++idx)->shdr != NULL
		      && (s = idx->shdr->bfd_section) != NULL
		      && elf_next_in_group (s) != NULL)
		    break;
		if (n_elt != 0)
		  {
		    /* Snarf the group name from other member, and
		       insert current section in circular list.  */
		    elf_group_name (newsect) = elf_group_name (s);
		    elf_next_in_group (newsect) = elf_next_in_group (s);
		    elf_next_in_group (s) = newsect;
		  }
		else
		  {
		    const char *gname;

		    gname = group_signature (abfd, shdr);
		    if (gname == NULL)
		      return FALSE;
		    elf_group_name (newsect) = gname;

		    /* Start a circular list with one element.  */
		    elf_next_in_group (newsect) = newsect;
		  }

		/* If the group section has been created, point to the
		   new member.  */
		if (shdr->bfd_section != NULL)
		  elf_next_in_group (shdr->bfd_section) = newsect;

		elf_tdata (abfd)->group_search_offset = i;
		j = num_group - 1;
		break;
	      }
	}
    }

  if (elf_group_name (newsect) == NULL)
    {
      /* xgettext:c-format */
      _bfd_error_handler (_("%pB: no group info for section '%pA'"),
			  abfd, newsect);
      return FALSE;
    }
  return TRUE;
}

bfd_boolean
_bfd_elf_setup_sections (bfd *abfd)
{
  unsigned int i;
  unsigned int num_group = elf_tdata (abfd)->num_group;
  bfd_boolean result = TRUE;
  asection *s;

  /* Process SHF_LINK_ORDER.  */
  for (s = abfd->sections; s != NULL; s = s->next)
    {
      Elf_Internal_Shdr *this_hdr = &elf_section_data (s)->this_hdr;
      if ((this_hdr->sh_flags & SHF_LINK_ORDER) != 0)
	{
	  unsigned int elfsec = this_hdr->sh_link;
	  /* FIXME: The old Intel compiler and old strip/objcopy may
	     not set the sh_link or sh_info fields.  Hence we could
	     get the situation where elfsec is 0.  */
	  if (elfsec == 0)
	    {
	      const struct elf_backend_data *bed = get_elf_backend_data (abfd);
	      if (bed->link_order_error_handler)
		bed->link_order_error_handler
		  /* xgettext:c-format */
		  (_("%pB: warning: sh_link not set for section `%pA'"),
		   abfd, s);
	    }
	  else
	    {
	      asection *linksec = NULL;

	      if (elfsec < elf_numsections (abfd))
		{
		  this_hdr = elf_elfsections (abfd)[elfsec];
		  linksec = this_hdr->bfd_section;
		}

	      /* PR 1991, 2008:
		 Some strip/objcopy may leave an incorrect value in
		 sh_link.  We don't want to proceed.  */
	      if (linksec == NULL)
		{
		  _bfd_error_handler
		    /* xgettext:c-format */
		    (_("%pB: sh_link [%d] in section `%pA' is incorrect"),
		     s->owner, elfsec, s);
		  result = FALSE;
		}

	      elf_linked_to_section (s) = linksec;
	    }
	}
      else if (this_hdr->sh_type == SHT_GROUP
	       && elf_next_in_group (s) == NULL)
	{
	  _bfd_error_handler
	    /* xgettext:c-format */
	    (_("%pB: SHT_GROUP section [index %d] has no SHF_GROUP sections"),
	     abfd, elf_section_data (s)->this_idx);
	  result = FALSE;
	}
    }

  /* Process section groups.  */
  if (num_group == (unsigned) -1)
    return result;

  for (i = 0; i < num_group; i++)
    {
      Elf_Internal_Shdr *shdr = elf_tdata (abfd)->group_sect_ptr[i];
      Elf_Internal_Group *idx;
      unsigned int n_elt;

      /* PR binutils/18758: Beware of corrupt binaries with invalid group data.  */
      if (shdr == NULL || shdr->bfd_section == NULL || shdr->contents == NULL)
	{
	  _bfd_error_handler
	    /* xgettext:c-format */
	    (_("%pB: section group entry number %u is corrupt"),
	     abfd, i);
	  result = FALSE;
	  continue;
	}

      idx = (Elf_Internal_Group *) shdr->contents;
      n_elt = shdr->sh_size / 4;

      while (--n_elt != 0)
	{
	  ++ idx;

	  if (idx->shdr == NULL)
	    continue;
	  else if (idx->shdr->bfd_section)
	    elf_sec_group (idx->shdr->bfd_section) = shdr->bfd_section;
	  else if (idx->shdr->sh_type != SHT_RELA
		   && idx->shdr->sh_type != SHT_REL)
	    {
	      /* There are some unknown sections in the group.  */
	      _bfd_error_handler
		/* xgettext:c-format */
		(_("%pB: unknown type [%#x] section `%s' in group [%pA]"),
		 abfd,
		 idx->shdr->sh_type,
		 bfd_elf_string_from_elf_section (abfd,
						  (elf_elfheader (abfd)
						   ->e_shstrndx),
						  idx->shdr->sh_name),
		 shdr->bfd_section);
	      result = FALSE;
	    }
	}
    }

  return result;
}

bfd_boolean
bfd_elf_is_group_section (bfd *abfd ATTRIBUTE_UNUSED, const asection *sec)
{
  return elf_next_in_group (sec) != NULL;
}

static char *
convert_debug_to_zdebug (bfd *abfd, const char *name)
{
  unsigned int len = strlen (name);
  char *new_name = bfd_alloc (abfd, len + 2);
  if (new_name == NULL)
    return NULL;
  new_name[0] = '.';
  new_name[1] = 'z';
  memcpy (new_name + 2, name + 1, len);
  return new_name;
}

static char *
convert_zdebug_to_debug (bfd *abfd, const char *name)
{
  unsigned int len = strlen (name);
  char *new_name = bfd_alloc (abfd, len);
  if (new_name == NULL)
    return NULL;
  new_name[0] = '.';
  memcpy (new_name + 1, name + 2, len - 1);
  return new_name;
}

/* Make a BFD section from an ELF section.  We store a pointer to the
   BFD section in the bfd_section field of the header.  */

bfd_boolean
_bfd_elf_make_section_from_shdr (bfd *abfd,
				 Elf_Internal_Shdr *hdr,
				 const char *name,
				 int shindex)
{
  asection *newsect;
  flagword flags;

  if (hdr->bfd_section != NULL)
    return TRUE;

  newsect = bfd_make_section_anyway (abfd, name);
  if (newsect == NULL)
    return FALSE;

  hdr->bfd_section = newsect;
  elf_section_data (newsect)->this_hdr = *hdr;
  elf_section_data (newsect)->this_idx = shindex;

  /* Always use the real type/flags.  */
  elf_section_type (newsect) = hdr->sh_type;
  elf_section_flags (newsect) = hdr->sh_flags;

  newsect->filepos = hdr->sh_offset;

  if (! bfd_set_section_vma (abfd, newsect, hdr->sh_addr)
      || ! bfd_set_section_size (abfd, newsect, hdr->sh_size)
      || ! bfd_set_section_alignment (abfd, newsect,
				      bfd_log2 (hdr->sh_addralign)))
    return FALSE;

  flags = SEC_NO_FLAGS;
  if (hdr->sh_type != SHT_NOBITS)
    flags |= SEC_HAS_CONTENTS;
  if (hdr->sh_type == SHT_GROUP)
    flags |= SEC_GROUP;
  if ((hdr->sh_flags & SHF_ALLOC) != 0)
    {
      flags |= SEC_ALLOC;
      if (hdr->sh_type != SHT_NOBITS)
	flags |= SEC_LOAD;
    }
  if ((hdr->sh_flags & SHF_WRITE) == 0)
    flags |= SEC_READONLY;
  if ((hdr->sh_flags & SHF_EXECINSTR) != 0)
    flags |= SEC_CODE;
  else if ((flags & SEC_LOAD) != 0)
    flags |= SEC_DATA;
  if ((hdr->sh_flags & SHF_MERGE) != 0)
    {
      flags |= SEC_MERGE;
      newsect->entsize = hdr->sh_entsize;
    }
  if ((hdr->sh_flags & SHF_STRINGS) != 0)
    flags |= SEC_STRINGS;
  if (hdr->sh_flags & SHF_GROUP)
    if (!setup_group (abfd, hdr, newsect))
      return FALSE;
  if ((hdr->sh_flags & SHF_TLS) != 0)
    flags |= SEC_THREAD_LOCAL;
  if ((hdr->sh_flags & SHF_EXCLUDE) != 0)
    flags |= SEC_EXCLUDE;

  if ((flags & SEC_ALLOC) == 0)
    {
      /* The debugging sections appear to be recognized only by name,
	 not any sort of flag.  Their SEC_ALLOC bits are cleared.  */
      if (name [0] == '.')
	{
	  const char *p;
	  int n;
	  if (name[1] == 'd')
	    p = ".debug", n = 6;
	  else if (name[1] == 'g' && name[2] == 'n')
	    p = ".gnu.linkonce.wi.", n = 17;
	  else if (name[1] == 'g' && name[2] == 'd')
	    p = ".gdb_index", n = 11; /* yes we really do mean 11.  */
	  else if (name[1] == 'l')
	    p = ".line", n = 5;
	  else if (name[1] == 's')
	    p = ".stab", n = 5;
	  else if (name[1] == 'z')
	    p = ".zdebug", n = 7;
	  else
	    p = NULL, n = 0;
	  if (p != NULL && strncmp (name, p, n) == 0)
	    flags |= SEC_DEBUGGING;
	}
    }

  /* As a GNU extension, if the name begins with .gnu.linkonce, we
     only link a single copy of the section.  This is used to support
     g++.  g++ will emit each template expansion in its own section.
     The symbols will be defined as weak, so that multiple definitions
     are permitted.  The GNU linker extension is to actually discard
     all but one of the sections.  */
  if (CONST_STRNEQ (name, ".gnu.linkonce")
      && elf_next_in_group (newsect) == NULL)
    flags |= SEC_LINK_ONCE | SEC_LINK_DUPLICATES_DISCARD;

  if (! bfd_set_section_flags (abfd, newsect, flags))
    return FALSE;

  /* We do not parse the PT_NOTE segments as we are interested even in the
     separate debug info files which may have the segments offsets corrupted.
     PT_NOTEs from the core files are currently not parsed using BFD.  */
  if (hdr->sh_type == SHT_NOTE)
    {
      bfd_byte *contents;

      if (!bfd_malloc_and_get_section (abfd, newsect, &contents))
	return FALSE;

      elf_parse_notes (abfd, (char *) contents, hdr->sh_size,
		       hdr->sh_offset, hdr->sh_addralign);
      free (contents);
    }

  if ((flags & SEC_ALLOC) != 0)
    {
      Elf_Internal_Phdr *phdr;
      unsigned int i, nload;

      /* Some ELF linkers produce binaries with all the program header
	 p_paddr fields zero.  If we have such a binary with more than
	 one PT_LOAD header, then leave the section lma equal to vma
	 so that we don't create sections with overlapping lma.  */
      phdr = elf_tdata (abfd)->phdr;
      for (nload = 0, i = 0; i < elf_elfheader (abfd)->e_phnum; i++, phdr++)
	if (phdr->p_paddr != 0)
	  break;
	else if (phdr->p_type == PT_LOAD && phdr->p_memsz != 0)
	  ++nload;
      if (i >= elf_elfheader (abfd)->e_phnum && nload > 1)
	return TRUE;

      phdr = elf_tdata (abfd)->phdr;
      for (i = 0; i < elf_elfheader (abfd)->e_phnum; i++, phdr++)
	{
	  if (((phdr->p_type == PT_LOAD
		&& (hdr->sh_flags & SHF_TLS) == 0)
	       || phdr->p_type == PT_TLS)
	      && ELF_SECTION_IN_SEGMENT (hdr, phdr))
	    {
	      if ((flags & SEC_LOAD) == 0)
		newsect->lma = (phdr->p_paddr
				+ hdr->sh_addr - phdr->p_vaddr);
	      else
		/* We used to use the same adjustment for SEC_LOAD
		   sections, but that doesn't work if the segment
		   is packed with code from multiple VMAs.
		   Instead we calculate the section LMA based on
		   the segment LMA.  It is assumed that the
		   segment will contain sections with contiguous
		   LMAs, even if the VMAs are not.  */
		newsect->lma = (phdr->p_paddr
				+ hdr->sh_offset - phdr->p_offset);

	      /* With contiguous segments, we can't tell from file
		 offsets whether a section with zero size should
		 be placed at the end of one segment or the
		 beginning of the next.  Decide based on vaddr.  */
	      if (hdr->sh_addr >= phdr->p_vaddr
		  && (hdr->sh_addr + hdr->sh_size
		      <= phdr->p_vaddr + phdr->p_memsz))
		break;
	    }
	}
    }

  /* Compress/decompress DWARF debug sections with names: .debug_* and
     .zdebug_*, after the section flags is set.  */
  if ((flags & SEC_DEBUGGING)
      && ((name[1] == 'd' && name[6] == '_')
	  || (name[1] == 'z' && name[7] == '_')))
    {
      enum { nothing, compress, decompress } action = nothing;
      int compression_header_size;
      bfd_size_type uncompressed_size;
      unsigned int uncompressed_align_power;
      bfd_boolean compressed
	= bfd_is_section_compressed_with_header (abfd, newsect,
						 &compression_header_size,
						 &uncompressed_size,
						 &uncompressed_align_power);
      if (compressed)
	{
	  /* Compressed section.  Check if we should decompress.  */
	  if ((abfd->flags & BFD_DECOMPRESS))
	    action = decompress;
	}

      /* Compress the uncompressed section or convert from/to .zdebug*
	 section.  Check if we should compress.  */
      if (action == nothing)
	{
	  if (newsect->size != 0
	      && (abfd->flags & BFD_COMPRESS)
	      && compression_header_size >= 0
	      && uncompressed_size > 0
	      && (!compressed
		  || ((compression_header_size > 0)
		      != ((abfd->flags & BFD_COMPRESS_GABI) != 0))))
	    action = compress;
	  else
	    return TRUE;
	}

      if (action == compress)
	{
	  if (!bfd_init_section_compress_status (abfd, newsect))
	    {
	      _bfd_error_handler
		/* xgettext:c-format */
		(_("%pB: unable to initialize compress status for section %s"),
		 abfd, name);
	      return FALSE;
	    }
	}
      else
	{
	  if (!bfd_init_section_decompress_status (abfd, newsect))
	    {
	      _bfd_error_handler
		/* xgettext:c-format */
		(_("%pB: unable to initialize decompress status for section %s"),
		 abfd, name);
	      return FALSE;
	    }
	}

      if (abfd->is_linker_input)
	{
	  if (name[1] == 'z'
	      && (action == decompress
		  || (action == compress
		      && (abfd->flags & BFD_COMPRESS_GABI) != 0)))
	    {
	      /* Convert section name from .zdebug_* to .debug_* so
		 that linker will consider this section as a debug
		 section.  */
	      char *new_name = convert_zdebug_to_debug (abfd, name);
	      if (new_name == NULL)
		return FALSE;
	      bfd_rename_section (abfd, newsect, new_name);
	    }
	}
      else
	/* For objdump, don't rename the section.  For objcopy, delay
	   section rename to elf_fake_sections.  */
	newsect->flags |= SEC_ELF_RENAME;
    }

  return TRUE;
}

const char *const bfd_elf_section_type_names[] =
{
  "SHT_NULL", "SHT_PROGBITS", "SHT_SYMTAB", "SHT_STRTAB",
  "SHT_RELA", "SHT_HASH", "SHT_DYNAMIC", "SHT_NOTE",
  "SHT_NOBITS", "SHT_REL", "SHT_SHLIB", "SHT_DYNSYM",
};

/* ELF relocs are against symbols.  If we are producing relocatable
   output, and the reloc is against an external symbol, and nothing
   has given us any additional addend, the resulting reloc will also
   be against the same symbol.  In such a case, we don't want to
   change anything about the way the reloc is handled, since it will
   all be done at final link time.  Rather than put special case code
   into bfd_perform_relocation, all the reloc types use this howto
   function.  It just short circuits the reloc if producing
   relocatable output against an external symbol.  */

bfd_reloc_status_type
bfd_elf_generic_reloc (bfd *abfd ATTRIBUTE_UNUSED,
		       arelent *reloc_entry,
		       asymbol *symbol,
		       void *data ATTRIBUTE_UNUSED,
		       asection *input_section,
		       bfd *output_bfd,
		       char **error_message ATTRIBUTE_UNUSED)
{
  if (output_bfd != NULL
      && (symbol->flags & BSF_SECTION_SYM) == 0
      && (! reloc_entry->howto->partial_inplace
	  || reloc_entry->addend == 0))
    {
      reloc_entry->address += input_section->output_offset;
      return bfd_reloc_ok;
    }

  return bfd_reloc_continue;
}

/* Copy the program header and other data from one object module to
   another.  */

bfd_boolean
_bfd_elf_copy_private_bfd_data (bfd *ibfd ATTRIBUTE_UNUSED, bfd *obfd ATTRIBUTE_UNUSED)
{
  return TRUE;
}

/* Print out the program headers.  */

bfd_boolean
_bfd_elf_print_private_bfd_data (bfd *abfd ATTRIBUTE_UNUSED, void *farg ATTRIBUTE_UNUSED)
{
  return TRUE;
}

/* Get version string.  */

const char *
_bfd_elf_get_symbol_version_string (bfd *abfd, asymbol *symbol,
				    bfd_boolean *hidden)
{
  const char *version_string = NULL;
  if (elf_dynversym (abfd) != 0
      && (elf_dynverdef (abfd) != 0 || elf_dynverref (abfd) != 0))
    {
      unsigned int vernum = ((elf_symbol_type *) symbol)->version;

      *hidden = (vernum & VERSYM_HIDDEN) != 0;
      vernum &= VERSYM_VERSION;

      if (vernum == 0)
	version_string = "";
      else if (vernum == 1
	       && (vernum > elf_tdata (abfd)->cverdefs
		   || (elf_tdata (abfd)->verdef[0].vd_flags
		       == VER_FLG_BASE)))
	version_string = "Base";
      else if (vernum <= elf_tdata (abfd)->cverdefs)
	version_string =
	  elf_tdata (abfd)->verdef[vernum - 1].vd_nodename;
      else
	{
	  Elf_Internal_Verneed *t;

	  version_string = _("<corrupt>");
	  for (t = elf_tdata (abfd)->verref;
	       t != NULL;
	       t = t->vn_nextref)
	    {
	      Elf_Internal_Vernaux *a;

	      for (a = t->vn_auxptr; a != NULL; a = a->vna_nextptr)
		{
		  if (a->vna_other == vernum)
		    {
		      version_string = a->vna_nodename;
		      break;
		    }
		}
	    }
	}
    }
  return version_string;
}

/* Display ELF-specific fields of a symbol.  */

void
bfd_elf_print_symbol (bfd *abfd ATTRIBUTE_UNUSED,
              void *filep ATTRIBUTE_UNUSED,
              asymbol *symbol ATTRIBUTE_UNUSED,
              bfd_print_symbol_type how ATTRIBUTE_UNUSED)
{
}

/* ELF .o/exec file reading */

/* Create a new bfd section from an ELF section header.  */

bfd_boolean
bfd_section_from_shdr (bfd *abfd, unsigned int shindex)
{
  Elf_Internal_Shdr *hdr;
  Elf_Internal_Ehdr *ehdr;
  const char *name;
  bfd_boolean ret = TRUE;
  static bfd_boolean * sections_being_created = NULL;
  static bfd * sections_being_created_abfd = NULL;
  static unsigned int nesting = 0;

  if (shindex >= elf_numsections (abfd))
    return FALSE;

  if (++ nesting > 3)
    {
      /* PR17512: A corrupt ELF binary might contain a recursive group of
	 sections, with each the string indices pointing to the next in the
	 loop.  Detect this here, by refusing to load a section that we are
	 already in the process of loading.  We only trigger this test if
	 we have nested at least three sections deep as normal ELF binaries
	 can expect to recurse at least once.

	 FIXME: It would be better if this array was attached to the bfd,
	 rather than being held in a static pointer.  */

      if (sections_being_created_abfd != abfd)
	sections_being_created = NULL;
      if (sections_being_created == NULL)
	{
	  /* FIXME: It would be more efficient to attach this array to the bfd somehow.  */
	  sections_being_created = (bfd_boolean *)
	    bfd_zalloc (abfd, elf_numsections (abfd) * sizeof (bfd_boolean));
	  sections_being_created_abfd = abfd;
	}
      if (sections_being_created [shindex])
	{
	  _bfd_error_handler
	    (_("%pB: warning: loop in section dependencies detected"), abfd);
	  return FALSE;
	}
      sections_being_created [shindex] = TRUE;
    }

  hdr = elf_elfsections (abfd)[shindex];
  ehdr = elf_elfheader (abfd);
  name = bfd_elf_string_from_elf_section (abfd, ehdr->e_shstrndx,
					  hdr->sh_name);
  if (name == NULL)
    goto fail;

  switch (hdr->sh_type)
    {
    case SHT_NULL:
      /* Inactive section. Throw it away.  */
      goto success;

    case SHT_PROGBITS:		/* Normal section with contents.  */
    case SHT_NOBITS:		/* .bss section.  */
    case SHT_HASH:		/* .hash section.  */
    case SHT_NOTE:		/* .note section.  */
    case SHT_INIT_ARRAY:	/* .init_array section.  */
    case SHT_FINI_ARRAY:	/* .fini_array section.  */
    case SHT_PREINIT_ARRAY:	/* .preinit_array section.  */
    case SHT_GNU_LIBLIST:	/* .gnu.liblist section.  */
    case SHT_GNU_HASH:		/* .gnu.hash section.  */
      ret = _bfd_elf_make_section_from_shdr (abfd, hdr, name, shindex);
      goto success;

    case SHT_DYNAMIC:	/* Dynamic linking information.  */
      if (! _bfd_elf_make_section_from_shdr (abfd, hdr, name, shindex))
	goto fail;

      if (hdr->sh_link > elf_numsections (abfd))
	{
	  /* PR 10478: Accept Solaris binaries with a sh_link
	     field set to SHN_BEFORE or SHN_AFTER.  */
	  switch (bfd_get_arch (abfd))
	    {
	    case bfd_arch_i386:
	    case bfd_arch_sparc:
	      if (hdr->sh_link == (SHN_LORESERVE & 0xffff) /* SHN_BEFORE */
		  || hdr->sh_link == ((SHN_LORESERVE + 1) & 0xffff) /* SHN_AFTER */)
		break;
	      /* Otherwise fall through.  */
	    default:
	      goto fail;
	    }
	}
      else if (elf_elfsections (abfd)[hdr->sh_link] == NULL)
	goto fail;
      else if (elf_elfsections (abfd)[hdr->sh_link]->sh_type != SHT_STRTAB)
	{
	  Elf_Internal_Shdr *dynsymhdr;

	  /* The shared libraries distributed with hpux11 have a bogus
	     sh_link field for the ".dynamic" section.  Find the
	     string table for the ".dynsym" section instead.  */
	  if (elf_dynsymtab (abfd) != 0)
	    {
	      dynsymhdr = elf_elfsections (abfd)[elf_dynsymtab (abfd)];
	      hdr->sh_link = dynsymhdr->sh_link;
	    }
	  else
	    {
	      unsigned int i, num_sec;

	      num_sec = elf_numsections (abfd);
	      for (i = 1; i < num_sec; i++)
		{
		  dynsymhdr = elf_elfsections (abfd)[i];
		  if (dynsymhdr->sh_type == SHT_DYNSYM)
		    {
		      hdr->sh_link = dynsymhdr->sh_link;
		      break;
		    }
		}
	    }
	}
      goto success;

    case SHT_SYMTAB:		/* A symbol table.  */
      if (elf_onesymtab (abfd) == shindex)
	goto success;

      if (hdr->sh_entsize != sizeof (Elf64_External_Sym))
    goto fail;

      if (hdr->sh_info * hdr->sh_entsize > hdr->sh_size)
	{
	  if (hdr->sh_size != 0)
	    goto fail;
	  /* Some assemblers erroneously set sh_info to one with a
	     zero sh_size.  ld sees this as a global symbol count
	     of (unsigned) -1.  Fix it here.  */
	  hdr->sh_info = 0;
	  goto success;
	}

      /* PR 18854: A binary might contain more than one symbol table.
	 Unusual, but possible.  Warn, but continue.  */
      if (elf_onesymtab (abfd) != 0)
	{
	  _bfd_error_handler
	    /* xgettext:c-format */
	    (_("%pB: warning: multiple symbol tables detected"
	       " - ignoring the table in section %u"),
	     abfd, shindex);
	  goto success;
	}
      elf_onesymtab (abfd) = shindex;
      elf_symtab_hdr (abfd) = *hdr;
      elf_elfsections (abfd)[shindex] = hdr = & elf_symtab_hdr (abfd);
      abfd->flags |= HAS_SYMS;

      /* Sometimes a shared object will map in the symbol table.  If
	 SHF_ALLOC is set, and this is a shared object, then we also
	 treat this section as a BFD section.  We can not base the
	 decision purely on SHF_ALLOC, because that flag is sometimes
	 set in a relocatable object file, which would confuse the
	 linker.  */
      if ((hdr->sh_flags & SHF_ALLOC) != 0
	  && (abfd->flags & DYNAMIC) != 0
	  && ! _bfd_elf_make_section_from_shdr (abfd, hdr, name,
						shindex))
	goto fail;

      /* Go looking for SHT_SYMTAB_SHNDX too, since if there is one we
	 can't read symbols without that section loaded as well.  It
	 is most likely specified by the next section header.  */
      {
	elf_section_list * entry;
	unsigned int i, num_sec;

	for (entry = elf_symtab_shndx_list (abfd); entry != NULL; entry = entry->next)
	  if (entry->hdr.sh_link == shindex)
	    goto success;

	num_sec = elf_numsections (abfd);
	for (i = shindex + 1; i < num_sec; i++)
	  {
	    Elf_Internal_Shdr *hdr2 = elf_elfsections (abfd)[i];

	    if (hdr2->sh_type == SHT_SYMTAB_SHNDX
		&& hdr2->sh_link == shindex)
	      break;
	  }

	if (i == num_sec)
	  for (i = 1; i < shindex; i++)
	    {
	      Elf_Internal_Shdr *hdr2 = elf_elfsections (abfd)[i];

	      if (hdr2->sh_type == SHT_SYMTAB_SHNDX
		  && hdr2->sh_link == shindex)
		break;
	    }

	if (i != shindex)
	  ret = bfd_section_from_shdr (abfd, i);
	/* else FIXME: we have failed to find the symbol table - should we issue an error ? */
	goto success;
      }

    case SHT_DYNSYM:		/* A dynamic symbol table.  */
      if (elf_dynsymtab (abfd) == shindex)
	goto success;

      if (hdr->sh_entsize != sizeof (Elf64_External_Sym))
    goto fail;

      if (hdr->sh_info * hdr->sh_entsize > hdr->sh_size)
	{
	  if (hdr->sh_size != 0)
	    goto fail;

	  /* Some linkers erroneously set sh_info to one with a
	     zero sh_size.  ld sees this as a global symbol count
	     of (unsigned) -1.  Fix it here.  */
	  hdr->sh_info = 0;
	  goto success;
	}

      /* PR 18854: A binary might contain more than one dynamic symbol table.
	 Unusual, but possible.  Warn, but continue.  */
      if (elf_dynsymtab (abfd) != 0)
	{
	  _bfd_error_handler
	    /* xgettext:c-format */
	    (_("%pB: warning: multiple dynamic symbol tables detected"
	       " - ignoring the table in section %u"),
	     abfd, shindex);
	  goto success;
	}
      elf_dynsymtab (abfd) = shindex;
      elf_tdata (abfd)->dynsymtab_hdr = *hdr;
      elf_elfsections (abfd)[shindex] = hdr = &elf_tdata (abfd)->dynsymtab_hdr;
      abfd->flags |= HAS_SYMS;

      /* Besides being a symbol table, we also treat this as a regular
	 section, so that objcopy can handle it.  */
      ret = _bfd_elf_make_section_from_shdr (abfd, hdr, name, shindex);
      goto success;

    case SHT_SYMTAB_SHNDX:	/* Symbol section indices when >64k sections.  */
      {
	elf_section_list * entry;

	for (entry = elf_symtab_shndx_list (abfd); entry != NULL; entry = entry->next)
	  if (entry->ndx == shindex)
	    goto success;

	entry = bfd_alloc (abfd, sizeof * entry);
	if (entry == NULL)
	  goto fail;
	entry->ndx = shindex;
	entry->hdr = * hdr;
	entry->next = elf_symtab_shndx_list (abfd);
	elf_symtab_shndx_list (abfd) = entry;
	elf_elfsections (abfd)[shindex] = & entry->hdr;
	goto success;
      }

    case SHT_STRTAB:		/* A string table.  */
      if (hdr->bfd_section != NULL)
	goto success;

      if (ehdr->e_shstrndx == shindex)
	{
	  elf_tdata (abfd)->shstrtab_hdr = *hdr;
	  elf_elfsections (abfd)[shindex] = &elf_tdata (abfd)->shstrtab_hdr;
	  goto success;
	}

      if (elf_elfsections (abfd)[elf_onesymtab (abfd)]->sh_link == shindex)
	{
	symtab_strtab:
	  elf_tdata (abfd)->strtab_hdr = *hdr;
	  elf_elfsections (abfd)[shindex] = &elf_tdata (abfd)->strtab_hdr;
	  goto success;
	}

      if (elf_elfsections (abfd)[elf_dynsymtab (abfd)]->sh_link == shindex)
	{
	dynsymtab_strtab:
	  elf_tdata (abfd)->dynstrtab_hdr = *hdr;
	  hdr = &elf_tdata (abfd)->dynstrtab_hdr;
	  elf_elfsections (abfd)[shindex] = hdr;
	  /* We also treat this as a regular section, so that objcopy
	     can handle it.  */
	  ret = _bfd_elf_make_section_from_shdr (abfd, hdr, name,
						 shindex);
	  goto success;
	}

      /* If the string table isn't one of the above, then treat it as a
	 regular section.  We need to scan all the headers to be sure,
	 just in case this strtab section appeared before the above.  */
      if (elf_onesymtab (abfd) == 0 || elf_dynsymtab (abfd) == 0)
	{
	  unsigned int i, num_sec;

	  num_sec = elf_numsections (abfd);
	  for (i = 1; i < num_sec; i++)
	    {
	      Elf_Internal_Shdr *hdr2 = elf_elfsections (abfd)[i];
	      if (hdr2->sh_link == shindex)
		{
		  /* Prevent endless recursion on broken objects.  */
		  if (i == shindex)
		    goto fail;
		  if (! bfd_section_from_shdr (abfd, i))
		    goto fail;
		  if (elf_onesymtab (abfd) == i)
		    goto symtab_strtab;
		  if (elf_dynsymtab (abfd) == i)
		    goto dynsymtab_strtab;
		}
	    }
	}
      ret = _bfd_elf_make_section_from_shdr (abfd, hdr, name, shindex);
      goto success;

    case SHT_REL:
    case SHT_RELA:
      /* *These* do a lot of work -- but build no sections!  */
      {
	asection *target_sect;
	Elf_Internal_Shdr *hdr2, **p_hdr;
	unsigned int num_sec = elf_numsections (abfd);
	struct bfd_elf_section_data *esdt;

	/* Check for a bogus link to avoid crashing.  */
	if (hdr->sh_link >= num_sec)
	  {
	    _bfd_error_handler
	      /* xgettext:c-format */
	      (_("%pB: invalid link %u for reloc section %s (index %u)"),
	       abfd, hdr->sh_link, name, shindex);
	    ret = _bfd_elf_make_section_from_shdr (abfd, hdr, name,
						   shindex);
	    goto success;
	  }

	/* For some incomprehensible reason Oracle distributes
	   libraries for Solaris in which some of the objects have
	   bogus sh_link fields.  It would be nice if we could just
	   reject them, but, unfortunately, some people need to use
	   them.  We scan through the section headers; if we find only
	   one suitable symbol table, we clobber the sh_link to point
	   to it.  I hope this doesn't break anything.

	   Don't do it on executable nor shared library.  */
	if ((abfd->flags & (DYNAMIC | EXEC_P)) == 0
	    && elf_elfsections (abfd)[hdr->sh_link]->sh_type != SHT_SYMTAB
	    && elf_elfsections (abfd)[hdr->sh_link]->sh_type != SHT_DYNSYM)
	  {
	    unsigned int scan;
	    int found;

	    found = 0;
	    for (scan = 1; scan < num_sec; scan++)
	      {
		if (elf_elfsections (abfd)[scan]->sh_type == SHT_SYMTAB
		    || elf_elfsections (abfd)[scan]->sh_type == SHT_DYNSYM)
		  {
		    if (found != 0)
		      {
			found = 0;
			break;
		      }
		    found = scan;
		  }
	      }
	    if (found != 0)
	      hdr->sh_link = found;
	  }

	/* Get the symbol table.  */
	if ((elf_elfsections (abfd)[hdr->sh_link]->sh_type == SHT_SYMTAB
	     || elf_elfsections (abfd)[hdr->sh_link]->sh_type == SHT_DYNSYM)
	    && ! bfd_section_from_shdr (abfd, hdr->sh_link))
	  goto fail;

	/* If this is an alloc section in an executable or shared
	   library, or the reloc section does not use the main symbol
	   table we don't treat it as a reloc section.  BFD can't
	   adequately represent such a section, so at least for now,
	   we don't try.  We just present it as a normal section.  We
	   also can't use it as a reloc section if it points to the
	   null section, an invalid section, another reloc section, or
	   its sh_link points to the null section.  */
	if (((abfd->flags & (DYNAMIC | EXEC_P)) != 0
	     && (hdr->sh_flags & SHF_ALLOC) != 0)
	    || hdr->sh_link == SHN_UNDEF
	    || hdr->sh_link != elf_onesymtab (abfd)
	    || hdr->sh_info == SHN_UNDEF
	    || hdr->sh_info >= num_sec
	    || elf_elfsections (abfd)[hdr->sh_info]->sh_type == SHT_REL
	    || elf_elfsections (abfd)[hdr->sh_info]->sh_type == SHT_RELA)
	  {
	    ret = _bfd_elf_make_section_from_shdr (abfd, hdr, name,
						   shindex);
	    goto success;
	  }

	if (! bfd_section_from_shdr (abfd, hdr->sh_info))
	  goto fail;

	target_sect = bfd_section_from_elf_index (abfd, hdr->sh_info);
	if (target_sect == NULL)
	  goto fail;

	esdt = elf_section_data (target_sect);
	if (hdr->sh_type == SHT_RELA)
	  p_hdr = &esdt->rela.hdr;
	else
	  p_hdr = &esdt->rel.hdr;

	/* PR 17512: file: 0b4f81b7.  */
	if (*p_hdr != NULL)
	  goto fail;
	hdr2 = (Elf_Internal_Shdr *) bfd_alloc (abfd, sizeof (*hdr2));
	if (hdr2 == NULL)
	  goto fail;
	*hdr2 = *hdr;
	*p_hdr = hdr2;
	elf_elfsections (abfd)[shindex] = hdr2;
/*    target_sect->reloc_count += (NUM_SHDR_ENTRIES (hdr)
                     * bed->s->int_rels_per_ext_rel);*/
	target_sect->flags |= SEC_RELOC;
	target_sect->relocation = NULL;
	target_sect->rel_filepos = hdr->sh_offset;
	/* In the section to which the relocations apply, mark whether
	   its relocations are of the REL or RELA variety.  */
	if (hdr->sh_size != 0)
	  {
	    if (hdr->sh_type == SHT_RELA)
	      target_sect->use_rela_p = 1;
	  }
	abfd->flags |= HAS_RELOC;
	goto success;
      }

    case SHT_GNU_verdef:
      elf_dynverdef (abfd) = shindex;
      elf_tdata (abfd)->dynverdef_hdr = *hdr;
      ret = _bfd_elf_make_section_from_shdr (abfd, hdr, name, shindex);
      goto success;

    case SHT_GNU_versym:
      if (hdr->sh_entsize != sizeof (Elf_External_Versym))
	goto fail;

      elf_dynversym (abfd) = shindex;
      elf_tdata (abfd)->dynversym_hdr = *hdr;
      ret = _bfd_elf_make_section_from_shdr (abfd, hdr, name, shindex);
      goto success;

    case SHT_GNU_verneed:
      elf_dynverref (abfd) = shindex;
      elf_tdata (abfd)->dynverref_hdr = *hdr;
      ret = _bfd_elf_make_section_from_shdr (abfd, hdr, name, shindex);
      goto success;

    case SHT_SHLIB:
      goto success;

    case SHT_GROUP:
      if (! IS_VALID_GROUP_SECTION_HEADER (hdr, GRP_ENTRY_SIZE))
	goto fail;

      if (!_bfd_elf_make_section_from_shdr (abfd, hdr, name, shindex))
	goto fail;

      goto success;

    default:
      /* Possibly an attributes section.  */

      if (hdr->sh_type >= SHT_LOUSER && hdr->sh_type <= SHT_HIUSER)
	{
	  if ((hdr->sh_flags & SHF_ALLOC) != 0)
	    /* FIXME: How to properly handle allocated section reserved
	       for applications?  */
	    _bfd_error_handler
	      /* xgettext:c-format */
	      (_("%pB: unknown type [%#x] section `%s'"),
	       abfd, hdr->sh_type, name);
	  else
	    {
	      /* Allow sections reserved for applications.  */
	      ret = _bfd_elf_make_section_from_shdr (abfd, hdr, name,
						     shindex);
	      goto success;
	    }
	}
      else if (hdr->sh_type >= SHT_LOPROC
	       && hdr->sh_type <= SHT_HIPROC)
	/* FIXME: We should handle this section.  */
	_bfd_error_handler
	  /* xgettext:c-format */
	  (_("%pB: unknown type [%#x] section `%s'"),
	   abfd, hdr->sh_type, name);
      else if (hdr->sh_type >= SHT_LOOS && hdr->sh_type <= SHT_HIOS)
	{
	  /* Unrecognised OS-specific sections.  */
	  if ((hdr->sh_flags & SHF_OS_NONCONFORMING) != 0)
	    /* SHF_OS_NONCONFORMING indicates that special knowledge is
	       required to correctly process the section and the file should
	       be rejected with an error message.  */
	    _bfd_error_handler
	      /* xgettext:c-format */
	      (_("%pB: unknown type [%#x] section `%s'"),
	       abfd, hdr->sh_type, name);
	  else
	    {
	      /* Otherwise it should be processed.  */
	      ret = _bfd_elf_make_section_from_shdr (abfd, hdr, name, shindex);
	      goto success;
	    }
	}
      else
	/* FIXME: We should handle this section.  */
	_bfd_error_handler
	  /* xgettext:c-format */
	  (_("%pB: unknown type [%#x] section `%s'"),
	   abfd, hdr->sh_type, name);

      goto fail;
    }

 fail:
  ret = FALSE;
 success:
  if (sections_being_created && sections_being_created_abfd == abfd)
    sections_being_created [shindex] = FALSE;
  if (-- nesting == 0)
    {
      sections_being_created = NULL;
      sections_being_created_abfd = abfd;
    }
  return ret;
}

/* Return the local symbol specified by ABFD, R_SYMNDX.  */

Elf_Internal_Sym *
bfd_sym_from_r_symndx (struct sym_cache *cache,
		       bfd *abfd,
		       unsigned long r_symndx)
{
  unsigned int ent = r_symndx % LOCAL_SYM_CACHE_SIZE;

  if (cache->abfd != abfd || cache->indx[ent] != r_symndx)
    {
      Elf_Internal_Shdr *symtab_hdr;
      unsigned char esym[sizeof (Elf64_External_Sym)];
      Elf_External_Sym_Shndx eshndx;

      symtab_hdr = &elf_tdata (abfd)->symtab_hdr;
      if (bfd_elf_get_elf_syms (abfd, symtab_hdr, 1, r_symndx,
				&cache->sym[ent], esym, &eshndx) == NULL)
	return NULL;

      if (cache->abfd != abfd)
	{
	  memset (cache->indx, -1, sizeof (cache->indx));
	  cache->abfd = abfd;
	}
      cache->indx[ent] = r_symndx;
    }

  return &cache->sym[ent];
}

/* Given an ELF section number, retrieve the corresponding BFD
   section.  */

asection *
bfd_section_from_elf_index (bfd *abfd, unsigned int sec_index)
{
  if (sec_index >= elf_numsections (abfd))
    return NULL;
  return elf_elfsections (abfd)[sec_index]->bfd_section;
}

static const struct bfd_elf_special_section special_sections_b[] =
{
  { STRING_COMMA_LEN (".bss"), -2, SHT_NOBITS,   SHF_ALLOC + SHF_WRITE },
  { NULL,		    0,	0, 0,		 0 }
};

static const struct bfd_elf_special_section special_sections_c[] =
{
  { STRING_COMMA_LEN (".comment"), 0, SHT_PROGBITS, 0 },
  { NULL,			0, 0, 0,	    0 }
};

static const struct bfd_elf_special_section special_sections_d[] =
{
  { STRING_COMMA_LEN (".data"),		-2, SHT_PROGBITS, SHF_ALLOC + SHF_WRITE },
  { STRING_COMMA_LEN (".data1"),	 0, SHT_PROGBITS, SHF_ALLOC + SHF_WRITE },
  /* There are more DWARF sections than these, but they needn't be added here
     unless you have to cope with broken compilers that don't emit section
     attributes or you want to help the user writing assembler.  */
  { STRING_COMMA_LEN (".debug"),	 0, SHT_PROGBITS, 0 },
  { STRING_COMMA_LEN (".debug_line"),	 0, SHT_PROGBITS, 0 },
  { STRING_COMMA_LEN (".debug_info"),	 0, SHT_PROGBITS, 0 },
  { STRING_COMMA_LEN (".debug_abbrev"),	 0, SHT_PROGBITS, 0 },
  { STRING_COMMA_LEN (".debug_aranges"), 0, SHT_PROGBITS, 0 },
  { STRING_COMMA_LEN (".dynamic"),	 0, SHT_DYNAMIC,  SHF_ALLOC },
  { STRING_COMMA_LEN (".dynstr"),	 0, SHT_STRTAB,	  SHF_ALLOC },
  { STRING_COMMA_LEN (".dynsym"),	 0, SHT_DYNSYM,	  SHF_ALLOC },
  { NULL,		       0,	 0, 0,		  0 }
};

static const struct bfd_elf_special_section special_sections_f[] =
{
  { STRING_COMMA_LEN (".fini"),	       0, SHT_PROGBITS,	  SHF_ALLOC + SHF_EXECINSTR },
  { STRING_COMMA_LEN (".fini_array"), -2, SHT_FINI_ARRAY, SHF_ALLOC + SHF_WRITE },
  { NULL,			   0 , 0, 0,		  0 }
};

static const struct bfd_elf_special_section special_sections_g[] =
{
  { STRING_COMMA_LEN (".gnu.linkonce.b"), -2, SHT_NOBITS,      SHF_ALLOC + SHF_WRITE },
  { STRING_COMMA_LEN (".gnu.lto_"),	  -1, SHT_PROGBITS,    SHF_EXCLUDE },
  { STRING_COMMA_LEN (".got"),		   0, SHT_PROGBITS,    SHF_ALLOC + SHF_WRITE },
  { STRING_COMMA_LEN (".gnu.version"),	   0, SHT_GNU_versym,  0 },
  { STRING_COMMA_LEN (".gnu.version_d"),   0, SHT_GNU_verdef,  0 },
  { STRING_COMMA_LEN (".gnu.version_r"),   0, SHT_GNU_verneed, 0 },
  { STRING_COMMA_LEN (".gnu.liblist"),	   0, SHT_GNU_LIBLIST, SHF_ALLOC },
  { STRING_COMMA_LEN (".gnu.conflict"),	   0, SHT_RELA,	       SHF_ALLOC },
  { STRING_COMMA_LEN (".gnu.hash"),	   0, SHT_GNU_HASH,    SHF_ALLOC },
  { NULL,			 0,	   0, 0,	       0 }
};

static const struct bfd_elf_special_section special_sections_h[] =
{
  { STRING_COMMA_LEN (".hash"), 0, SHT_HASH,	 SHF_ALLOC },
  { NULL,		     0, 0, 0,		 0 }
};

static const struct bfd_elf_special_section special_sections_i[] =
{
  { STRING_COMMA_LEN (".init"),	       0, SHT_PROGBITS,	  SHF_ALLOC + SHF_EXECINSTR },
  { STRING_COMMA_LEN (".init_array"), -2, SHT_INIT_ARRAY, SHF_ALLOC + SHF_WRITE },
  { STRING_COMMA_LEN (".interp"),      0, SHT_PROGBITS,	  0 },
  { NULL,		       0,      0, 0,		  0 }
};

static const struct bfd_elf_special_section special_sections_l[] =
{
  { STRING_COMMA_LEN (".line"), 0, SHT_PROGBITS, 0 },
  { NULL,		     0, 0, 0,		 0 }
};

static const struct bfd_elf_special_section special_sections_n[] =
{
  { STRING_COMMA_LEN (".note.GNU-stack"), 0, SHT_PROGBITS, 0 },
  { STRING_COMMA_LEN (".note"),		 -1, SHT_NOTE,	   0 },
  { NULL,		     0,		  0, 0,		   0 }
};

static const struct bfd_elf_special_section special_sections_p[] =
{
  { STRING_COMMA_LEN (".preinit_array"), -2, SHT_PREINIT_ARRAY, SHF_ALLOC + SHF_WRITE },
  { STRING_COMMA_LEN (".plt"),		  0, SHT_PROGBITS,	SHF_ALLOC + SHF_EXECINSTR },
  { NULL,		    0,		  0, 0,			0 }
};

static const struct bfd_elf_special_section special_sections_r[] =
{
  { STRING_COMMA_LEN (".rodata"), -2, SHT_PROGBITS, SHF_ALLOC },
  { STRING_COMMA_LEN (".rodata1"), 0, SHT_PROGBITS, SHF_ALLOC },
  { STRING_COMMA_LEN (".rela"),	  -1, SHT_RELA,	    0 },
  { STRING_COMMA_LEN (".rel"),	  -1, SHT_REL,	    0 },
  { NULL,		    0,	   0, 0,	    0 }
};

static const struct bfd_elf_special_section special_sections_s[] =
{
  { STRING_COMMA_LEN (".shstrtab"), 0, SHT_STRTAB, 0 },
  { STRING_COMMA_LEN (".strtab"),   0, SHT_STRTAB, 0 },
  { STRING_COMMA_LEN (".symtab"),   0, SHT_SYMTAB, 0 },
  /* See struct bfd_elf_special_section declaration for the semantics of
     this special case where .prefix_length != strlen (.prefix).  */
  { ".stabstr",			5,  3, SHT_STRTAB, 0 },
  { NULL,			0,  0, 0,	   0 }
};

static const struct bfd_elf_special_section special_sections_t[] =
{
  { STRING_COMMA_LEN (".text"),	 -2, SHT_PROGBITS, SHF_ALLOC + SHF_EXECINSTR },
  { STRING_COMMA_LEN (".tbss"),	 -2, SHT_NOBITS,   SHF_ALLOC + SHF_WRITE + SHF_TLS },
  { STRING_COMMA_LEN (".tdata"), -2, SHT_PROGBITS, SHF_ALLOC + SHF_WRITE + SHF_TLS },
  { NULL,		      0,  0, 0,		   0 }
};

static const struct bfd_elf_special_section special_sections_z[] =
{
  { STRING_COMMA_LEN (".zdebug_line"),	  0, SHT_PROGBITS, 0 },
  { STRING_COMMA_LEN (".zdebug_info"),	  0, SHT_PROGBITS, 0 },
  { STRING_COMMA_LEN (".zdebug_abbrev"),  0, SHT_PROGBITS, 0 },
  { STRING_COMMA_LEN (".zdebug_aranges"), 0, SHT_PROGBITS, 0 },
  { NULL,		      0,  0, 0,		   0 }
};

static const struct bfd_elf_special_section * const special_sections[] =
{
  special_sections_b,		/* 'b' */
  special_sections_c,		/* 'c' */
  special_sections_d,		/* 'd' */
  NULL,				/* 'e' */
  special_sections_f,		/* 'f' */
  special_sections_g,		/* 'g' */
  special_sections_h,		/* 'h' */
  special_sections_i,		/* 'i' */
  NULL,				/* 'j' */
  NULL,				/* 'k' */
  special_sections_l,		/* 'l' */
  NULL,				/* 'm' */
  special_sections_n,		/* 'n' */
  NULL,				/* 'o' */
  special_sections_p,		/* 'p' */
  NULL,				/* 'q' */
  special_sections_r,		/* 'r' */
  special_sections_s,		/* 's' */
  special_sections_t,		/* 't' */
  NULL,				/* 'u' */
  NULL,				/* 'v' */
  NULL,				/* 'w' */
  NULL,				/* 'x' */
  NULL,				/* 'y' */
  special_sections_z		/* 'z' */
};

const struct bfd_elf_special_section *
_bfd_elf_get_special_section (const char *name,
			      const struct bfd_elf_special_section *spec,
			      unsigned int rela)
{
  int i;
  int len;

  len = strlen (name);

  for (i = 0; spec[i].prefix != NULL; i++)
    {
      int suffix_len;
      int prefix_len = spec[i].prefix_length;

      if (len < prefix_len)
	continue;
      if (memcmp (name, spec[i].prefix, prefix_len) != 0)
	continue;

      suffix_len = spec[i].suffix_length;
      if (suffix_len <= 0)
	{
	  if (name[prefix_len] != 0)
	    {
	      if (suffix_len == 0)
		continue;
	      if (name[prefix_len] != '.'
		  && (suffix_len == -2
		      || (rela && spec[i].type == SHT_REL)))
		continue;
	    }
	}
      else
	{
	  if (len < prefix_len + suffix_len)
	    continue;
	  if (memcmp (name + len - suffix_len,
		      spec[i].prefix + prefix_len,
		      suffix_len) != 0)
	    continue;
	}
      return &spec[i];
    }

  return NULL;
}

const struct bfd_elf_special_section *
_bfd_elf_get_sec_type_attr (bfd *abfd, asection *sec)
{
  int i;
  const struct bfd_elf_special_section *spec;
  const struct elf_backend_data *bed;

  /* See if this is one of the special sections.  */
  if (sec->name == NULL)
    return NULL;

  bed = get_elf_backend_data (abfd);
  spec = bed->special_sections;
  if (spec)
    {
      spec = _bfd_elf_get_special_section (sec->name,
					   bed->special_sections,
					   sec->use_rela_p);
      if (spec != NULL)
	return spec;
    }

  if (sec->name[0] != '.')
    return NULL;

  i = sec->name[1] - 'b';
  if (i < 0 || i > 'z' - 'b')
    return NULL;

  spec = special_sections[i];

  if (spec == NULL)
    return NULL;

  return _bfd_elf_get_special_section (sec->name, spec, sec->use_rela_p);
}

bfd_boolean
_bfd_elf_new_section_hook (bfd *abfd, asection *sec)
{
  struct bfd_elf_section_data *sdata;

  sdata = (struct bfd_elf_section_data *) sec->used_by_bfd;
  if (sdata == NULL)
    {
      sdata = (struct bfd_elf_section_data *) bfd_zalloc (abfd,
							  sizeof (*sdata));
      if (sdata == NULL)
	return FALSE;
      sec->used_by_bfd = sdata;
    }

  /* Indicate whether or not this section should use RELA relocations.  */
  //bed = get_elf_backend_data (abfd);
  //sec->use_rela_p = bed->default_use_rela_p;

  return _bfd_generic_new_section_hook (abfd, sec);
}

/* Create a new bfd section from an ELF program header.

   Since program segments have no names, we generate a synthetic name
   of the form segment<NUM>, where NUM is generally the index in the
   program header table.  For segments that are split (see below) we
   generate the names segment<NUM>a and segment<NUM>b.

   Note that some program segments may have a file size that is different than
   (less than) the memory size.  All this means is that at execution the
   system must allocate the amount of memory specified by the memory size,
   but only initialize it with the first "file size" bytes read from the
   file.  This would occur for example, with program segments consisting
   of combined data+bss.

   To handle the above situation, this routine generates TWO bfd sections
   for the single program segment.  The first has the length specified by
   the file size of the segment, and the second has the length specified
   by the difference between the two sizes.  In effect, the segment is split
   into its initialized and uninitialized parts.

 */

bfd_boolean
_bfd_elf_make_section_from_phdr (bfd *abfd,
				 Elf_Internal_Phdr *hdr,
				 int hdr_index,
				 const char *type_name)
{
  asection *newsect;
  char *name;
  char namebuf[64];
  size_t len;
  int split;

  split = ((hdr->p_memsz > 0)
	    && (hdr->p_filesz > 0)
	    && (hdr->p_memsz > hdr->p_filesz));

  if (hdr->p_filesz > 0)
    {
      sprintf (namebuf, "%s%d%s", type_name, hdr_index, split ? "a" : "");
      len = strlen (namebuf) + 1;
      name = (char *) bfd_alloc (abfd, len);
      if (!name)
	return FALSE;
      memcpy (name, namebuf, len);
      newsect = bfd_make_section (abfd, name);
      if (newsect == NULL)
	return FALSE;
      newsect->vma = hdr->p_vaddr;
      newsect->lma = hdr->p_paddr;
      newsect->size = hdr->p_filesz;
      newsect->filepos = hdr->p_offset;
      newsect->flags |= SEC_HAS_CONTENTS;
      newsect->alignment_power = bfd_log2 (hdr->p_align);
      if (hdr->p_type == PT_LOAD)
	{
	  newsect->flags |= SEC_ALLOC;
	  newsect->flags |= SEC_LOAD;
	  if (hdr->p_flags & PF_X)
	    {
	      /* FIXME: all we known is that it has execute PERMISSION,
		 may be data.  */
	      newsect->flags |= SEC_CODE;
	    }
	}
      if (!(hdr->p_flags & PF_W))
	{
	  newsect->flags |= SEC_READONLY;
	}
    }

  if (hdr->p_memsz > hdr->p_filesz)
    {
      bfd_vma align;

      sprintf (namebuf, "%s%d%s", type_name, hdr_index, split ? "b" : "");
      len = strlen (namebuf) + 1;
      name = (char *) bfd_alloc (abfd, len);
      if (!name)
	return FALSE;
      memcpy (name, namebuf, len);
      newsect = bfd_make_section (abfd, name);
      if (newsect == NULL)
	return FALSE;
      newsect->vma = hdr->p_vaddr + hdr->p_filesz;
      newsect->lma = hdr->p_paddr + hdr->p_filesz;
      newsect->size = hdr->p_memsz - hdr->p_filesz;
      newsect->filepos = hdr->p_offset + hdr->p_filesz;
      align = newsect->vma & -newsect->vma;
      if (align == 0 || align > hdr->p_align)
	align = hdr->p_align;
      newsect->alignment_power = bfd_log2 (align);
      if (hdr->p_type == PT_LOAD)
	{
	  /* Hack for gdb.  Segments that have not been modified do
	     not have their contents written to a core file, on the
	     assumption that a debugger can find the contents in the
	     executable.  We flag this case by setting the fake
	     section size to zero.  Note that "real" bss sections will
	     always have their contents dumped to the core file.  */
	  if (bfd_get_format (abfd) == bfd_core)
	    newsect->size = 0;
	  newsect->flags |= SEC_ALLOC;
	  if (hdr->p_flags & PF_X)
	    newsect->flags |= SEC_CODE;
	}
      if (!(hdr->p_flags & PF_W))
	newsect->flags |= SEC_READONLY;
    }

  return TRUE;
}

bfd_boolean
bfd_section_from_phdr (bfd *abfd, Elf_Internal_Phdr *hdr, int hdr_index)
{
  const struct elf_backend_data *bed;

  switch (hdr->p_type)
    {
    case PT_NULL:
      return _bfd_elf_make_section_from_phdr (abfd, hdr, hdr_index, "null");

    case PT_LOAD:
      return _bfd_elf_make_section_from_phdr (abfd, hdr, hdr_index, "load");

    case PT_DYNAMIC:
      return _bfd_elf_make_section_from_phdr (abfd, hdr, hdr_index, "dynamic");

    case PT_INTERP:
      return _bfd_elf_make_section_from_phdr (abfd, hdr, hdr_index, "interp");

    case PT_NOTE:
      if (! _bfd_elf_make_section_from_phdr (abfd, hdr, hdr_index, "note"))
	return FALSE;
      if (! elf_read_notes (abfd, hdr->p_offset, hdr->p_filesz,
			    hdr->p_align))
	return FALSE;
      return TRUE;

    case PT_SHLIB:
      return _bfd_elf_make_section_from_phdr (abfd, hdr, hdr_index, "shlib");

    case PT_PHDR:
      return _bfd_elf_make_section_from_phdr (abfd, hdr, hdr_index, "phdr");

    case PT_GNU_EH_FRAME:
      return _bfd_elf_make_section_from_phdr (abfd, hdr, hdr_index,
					      "eh_frame_hdr");

    case PT_GNU_STACK:
      return _bfd_elf_make_section_from_phdr (abfd, hdr, hdr_index, "stack");

    case PT_GNU_RELRO:
      return _bfd_elf_make_section_from_phdr (abfd, hdr, hdr_index, "relro");

    default:
      /* Check for any processor-specific program segment types.  */
      bed = get_elf_backend_data (abfd);
      return bed->elf_backend_section_from_phdr (abfd, hdr, hdr_index, "proc");
    }
}

/* Return the REL_HDR for SEC, assuming there is only a single one, either
   REL or RELA.  */

Elf_Internal_Shdr *
_bfd_elf_single_rel_hdr (asection *sec)
{
  if (elf_section_data (sec)->rel.hdr)
    {
      BFD_ASSERT (elf_section_data (sec)->rela.hdr == NULL);
      return elf_section_data (sec)->rel.hdr;
    }
  else
    return elf_section_data (sec)->rela.hdr;
}

static bfd_boolean
_bfd_elf_set_reloc_sh_name (bfd *abfd,
			    Elf_Internal_Shdr *rel_hdr,
			    const char *sec_name,
			    bfd_boolean use_rela_p)
{
  char *name = (char *) bfd_alloc (abfd,
				   sizeof ".rela" + strlen (sec_name));
  if (name == NULL)
    return FALSE;

  sprintf (name, "%s%s", use_rela_p ? ".rela" : ".rel", sec_name);
  rel_hdr->sh_name =
    (unsigned int) _bfd_elf_strtab_add (elf_shstrtab (abfd), name,
					FALSE);
  if (rel_hdr->sh_name == (unsigned int) -1)
    return FALSE;

  return TRUE;
}

/* Return the default section type based on the passed in section flags.  */

int
bfd_elf_get_default_section_type (flagword flags)
{
  if ((flags & (SEC_ALLOC | SEC_IS_COMMON)) != 0
      && (flags & (SEC_LOAD | SEC_HAS_CONTENTS)) == 0)
    return SHT_NOBITS;
  return SHT_PROGBITS;
}

struct fake_section_arg
{
  struct bfd_link_info *link_info;
  bfd_boolean failed;
};

/* Fill in the contents of a SHT_GROUP section.  Called from
   _bfd_elf_compute_section_file_positions for gas, objcopy, and
   when ELF targets use the generic linker, ld.  Called for ld -r
   from bfd_elf_final_link.  */

void
bfd_elf_set_group_contents (bfd *abfd, asection *sec, void *failedptrarg)
{
  bfd_boolean *failedptr = (bfd_boolean *) failedptrarg;
  asection *elt, *first;
  unsigned char *loc;
  bfd_boolean gas;

  /* Ignore linker created group section.  See elfNN_ia64_object_p in
     elfxx-ia64.c.  */
  if (((sec->flags & (SEC_GROUP | SEC_LINKER_CREATED)) != SEC_GROUP)
      || *failedptr)
    return;

  if (elf_section_data (sec)->this_hdr.sh_info == 0)
    {
      unsigned long symindx = 0;

      /* elf_group_id will have been set up by objcopy and the
	 generic linker.  */
      if (elf_group_id (sec) != NULL)
	symindx = elf_group_id (sec)->udata.i;

      if (symindx == 0)
	{
	  /* If called from the assembler, swap_out_syms will have set up
	     elf_section_syms.  */
	  BFD_ASSERT (elf_section_syms (abfd) != NULL);
	  symindx = elf_section_syms (abfd)[sec->index]->udata.i;
	}
      elf_section_data (sec)->this_hdr.sh_info = symindx;
    }
  else if (elf_section_data (sec)->this_hdr.sh_info == (unsigned int) -2)
    {
      /* The ELF backend linker sets sh_info to -2 when the group
	 signature symbol is global, and thus the index can't be
	 set until all local symbols are output.  */
      asection *igroup;
      struct bfd_elf_section_data *sec_data;
      unsigned long symndx;
      unsigned long extsymoff;
      struct elf_link_hash_entry *h;

      /* The point of this little dance to the first SHF_GROUP section
	 then back to the SHT_GROUP section is that this gets us to
	 the SHT_GROUP in the input object.  */
      igroup = elf_sec_group (elf_next_in_group (sec));
      sec_data = elf_section_data (igroup);
      symndx = sec_data->this_hdr.sh_info;
      extsymoff = 0;
      if (!elf_bad_symtab (igroup->owner))
	{
	  Elf_Internal_Shdr *symtab_hdr;

	  symtab_hdr = &elf_tdata (igroup->owner)->symtab_hdr;
	  extsymoff = symtab_hdr->sh_info;
	}
      h = elf_sym_hashes (igroup->owner)[symndx - extsymoff];
      while (h->root.type == bfd_link_hash_indirect
	     || h->root.type == bfd_link_hash_warning)
	h = (struct elf_link_hash_entry *) h->root.u.i.link;

      elf_section_data (sec)->this_hdr.sh_info = h->indx;
    }

  /* The contents won't be allocated for "ld -r" or objcopy.  */
  gas = TRUE;
  if (sec->contents == NULL)
    {
      gas = FALSE;
      sec->contents = (unsigned char *) bfd_alloc (abfd, sec->size);

      /* Arrange for the section to be written out.  */
      elf_section_data (sec)->this_hdr.contents = sec->contents;
      if (sec->contents == NULL)
	{
	  *failedptr = TRUE;
	  return;
	}
    }

  loc = sec->contents + sec->size;

  /* Get the pointer to the first section in the group that gas
     squirreled away here.  objcopy arranges for this to be set to the
     start of the input section group.  */
  first = elt = elf_next_in_group (sec);

  /* First element is a flag word.  Rest of section is elf section
     indices for all the sections of the group.  Write them backwards
     just to keep the group in the same order as given in .section
     directives, not that it matters.  */
  while (elt != NULL)
    {
      asection *s;

      s = elt;
      if (!gas)
	s = s->output_section;
      if (s != NULL
	  && !bfd_is_abs_section (s))
	{
	  struct bfd_elf_section_data *elf_sec = elf_section_data (s);
	  struct bfd_elf_section_data *input_elf_sec = elf_section_data (elt);

	  if (elf_sec->rel.hdr != NULL
	      && (gas
		  || (input_elf_sec->rel.hdr != NULL
		      && input_elf_sec->rel.hdr->sh_flags & SHF_GROUP) != 0))
	    {
	      elf_sec->rel.hdr->sh_flags |= SHF_GROUP;
	      loc -= 4;
	      H_PUT_32 (abfd, elf_sec->rel.idx, loc);
	    }
	  if (elf_sec->rela.hdr != NULL
	      && (gas
		  || (input_elf_sec->rela.hdr != NULL
		      && input_elf_sec->rela.hdr->sh_flags & SHF_GROUP) != 0))
	    {
	      elf_sec->rela.hdr->sh_flags |= SHF_GROUP;
	      loc -= 4;
	      H_PUT_32 (abfd, elf_sec->rela.idx, loc);
	    }
	  loc -= 4;
	  H_PUT_32 (abfd, elf_sec->this_idx, loc);
	}
      elt = elf_next_in_group (elt);
      if (elt == first)
	break;
    }

  loc -= 4;
  BFD_ASSERT (loc == sec->contents);

  H_PUT_32 (abfd, sec->flags & SEC_LINK_ONCE ? GRP_COMDAT : 0, loc);
}

/* Given NAME, the name of a relocation section stripped of its
   .rel/.rela prefix, return the section in ABFD to which the
   relocations apply.  */

asection *
_bfd_elf_plt_get_reloc_section (bfd *abfd, const char *name)
{
  /* If a target needs .got.plt section, relocations in rela.plt/rel.plt
     section likely apply to .got.plt or .got section.  */
  if (get_elf_backend_data (abfd)->want_got_plt
      && strcmp (name, ".plt") == 0)
    {
      asection *sec;

      name = ".got.plt";
      sec = bfd_get_section_by_name (abfd, name);
      if (sec != NULL)
	return sec;
      name = ".got";
    }

  return bfd_get_section_by_name (abfd, name);
}

/* Filter global symbols of ABFD to include in the import library.  All
   SYMCOUNT symbols of ABFD can be examined from their pointers in
   SYMS.  Pointers of symbols to keep should be stored contiguously at
   the beginning of that array.

   Returns the number of symbols to keep.  */

unsigned int
_bfd_elf_filter_global_symbols (bfd *abfd ATTRIBUTE_UNUSED, struct bfd_link_info *info ATTRIBUTE_UNUSED,
                asymbol **syms ATTRIBUTE_UNUSED, long symcount ATTRIBUTE_UNUSED)
{
  return 0;
}

/* Align to the maximum file alignment that could be required for any
   ELF data structure.  */

static inline file_ptr
align_file_position (file_ptr off, int align)
{
  return (off + align - 1) & ~(align - 1);
}

/* Assign a file position to a section, optionally aligning to the
   required section alignment.  */

file_ptr
_bfd_elf_assign_file_position_for_section (Elf_Internal_Shdr *i_shdrp,
					   file_ptr offset,
					   bfd_boolean align)
{
  if (align && i_shdrp->sh_addralign > 1)
    offset = BFD_ALIGN (offset, i_shdrp->sh_addralign);
  i_shdrp->sh_offset = offset;
  if (i_shdrp->bfd_section != NULL)
    i_shdrp->bfd_section->filepos = offset;
  if (i_shdrp->sh_type != SHT_NOBITS)
    offset += i_shdrp->sh_size;
  return offset;
}

/* Compute the file positions we are going to put the sections at, and
   otherwise prepare to begin writing out the ELF file.  If LINK_INFO
   is not NULL, this is being called by the ELF backend linker.  */

bfd_boolean
_bfd_elf_compute_section_file_positions (bfd *abfd ATTRIBUTE_UNUSED,
                     struct bfd_link_info *link_info ATTRIBUTE_UNUSED)
{
  return TRUE;
}

/* Make an initial estimate of the size of the program header.  If we
   get the number wrong here, we'll redo section placement.  */

static bfd_size_type
get_program_header_size (bfd *abfd, struct bfd_link_info *info)
{
  size_t segs;
  asection *s;
  const struct elf_backend_data *bed;

  /* Assume we will need exactly two PT_LOAD segments: one for text
     and one for data.  */
  segs = 2;

  s = bfd_get_section_by_name (abfd, ".interp");
  if (s != NULL && (s->flags & SEC_LOAD) != 0 && s->size != 0)
    {
      /* If we have a loadable interpreter section, we need a
	 PT_INTERP segment.  In this case, assume we also need a
	 PT_PHDR segment, although that may not be true for all
	 targets.  */
      segs += 2;
    }

  if (bfd_get_section_by_name (abfd, ".dynamic") != NULL)
    {
      /* We need a PT_DYNAMIC segment.  */
      ++segs;
    }

  if (info != NULL && info->relro)
    {
      /* We need a PT_GNU_RELRO segment.  */
      ++segs;
    }

  if (elf_eh_frame_hdr (abfd))
    {
      /* We need a PT_GNU_EH_FRAME segment.  */
      ++segs;
    }

  if (elf_stack_flags (abfd))
    {
      /* We need a PT_GNU_STACK segment.  */
      ++segs;
    }

  s = bfd_get_section_by_name (abfd,
			       NOTE_GNU_PROPERTY_SECTION_NAME);
  if (s != NULL && s->size != 0)
    {
      /* We need a PT_GNU_PROPERTY segment.  */
      ++segs;
    }

  for (s = abfd->sections; s != NULL; s = s->next)
    {
      if ((s->flags & SEC_LOAD) != 0
	  && elf_section_type (s) == SHT_NOTE)
	{
	  unsigned int alignment_power;
	  /* We need a PT_NOTE segment.  */
	  ++segs;
	  /* Try to create just one PT_NOTE segment for all adjacent
	     loadable SHT_NOTE sections.  gABI requires that within a
	     PT_NOTE segment (and also inside of each SHT_NOTE section)
	     each note should have the same alignment.  So we check
	     whether the sections are correctly aligned.  */
	  alignment_power = s->alignment_power;
	  while (s->next != NULL
		 && s->next->alignment_power == alignment_power
		 && (s->next->flags & SEC_LOAD) != 0
		 && elf_section_type (s->next) == SHT_NOTE)
	    s = s->next;
	}
    }

  for (s = abfd->sections; s != NULL; s = s->next)
    {
      if (s->flags & SEC_THREAD_LOCAL)
	{
	  /* We need a PT_TLS segment.  */
	  ++segs;
	  break;
	}
    }

  bed = get_elf_backend_data (abfd);

 if ((abfd->flags & D_PAGED) != 0)
   {
     /* Add a PT_GNU_MBIND segment for each mbind section.  */
     unsigned int page_align_power = bfd_log2 (bed->commonpagesize);
     for (s = abfd->sections; s != NULL; s = s->next)
       if (elf_section_flags (s) & SHF_GNU_MBIND)
	 {
	   if (elf_section_data (s)->this_hdr.sh_info
	       > PT_GNU_MBIND_NUM)
	     {
	       _bfd_error_handler
		 /* xgettext:c-format */
		 (_("%pB: GNU_MBIN section `%pA' has invalid sh_info field: %d"),
		     abfd, s, elf_section_data (s)->this_hdr.sh_info);
	       continue;
	     }
	   /* Align mbind section to page size.  */
	   if (s->alignment_power < page_align_power)
	     s->alignment_power = page_align_power;
	   segs ++;
	 }
   }

 /* Let the backend count up any program headers it might need.  */
 if (bed->elf_backend_additional_program_headers)
    {
      int a;

      a = (*bed->elf_backend_additional_program_headers) (abfd, info);
      if (a == -1)
	abort ();
      segs += a;
    }

  return segs * bed->s->sizeof_phdr;
}

/* Find the segment that contains the output_section of section.  */

Elf_Internal_Phdr *
_bfd_elf_find_segment_containing_section (bfd * abfd, asection * section)
{
  struct elf_segment_map *m;
  Elf_Internal_Phdr *p;

  for (m = elf_seg_map (abfd), p = elf_tdata (abfd)->phdr;
       m != NULL;
       m = m->next, p++)
    {
      int i;

      for (i = m->count - 1; i >= 0; i--)
	if (m->sections[i] == section)
	  return p;
    }

  return NULL;
}

/* Create a mapping from a set of sections to a program segment.  */

static struct elf_segment_map *
make_mapping (bfd *abfd,
	      asection **sections,
	      unsigned int from,
	      unsigned int to,
	      bfd_boolean phdr)
{
  struct elf_segment_map *m;
  unsigned int i;
  asection **hdrpp;
  bfd_size_type amt;

  amt = sizeof (struct elf_segment_map) - sizeof (asection *);
  amt += (to - from) * sizeof (asection *);
  m = (struct elf_segment_map *) bfd_zalloc (abfd, amt);
  if (m == NULL)
    return NULL;
  m->next = NULL;
  m->p_type = PT_LOAD;
  for (i = from, hdrpp = sections + from; i < to; i++, hdrpp++)
    m->sections[i - from] = *hdrpp;
  m->count = to - from;

  if (from == 0 && phdr)
    {
      /* Include the headers in the first PT_LOAD segment.  */
      m->includes_filehdr = 1;
      m->includes_phdrs = 1;
    }

  return m;
}

/* Create the PT_DYNAMIC segment, which includes DYNSEC.  Returns NULL
   on failure.  */

struct elf_segment_map *
_bfd_elf_make_dynamic_segment (bfd *abfd, asection *dynsec)
{
  struct elf_segment_map *m;

  m = (struct elf_segment_map *) bfd_zalloc (abfd,
					     sizeof (struct elf_segment_map));
  if (m == NULL)
    return NULL;
  m->next = NULL;
  m->p_type = PT_DYNAMIC;
  m->count = 1;
  m->sections[0] = dynsec;

  return m;
}

/* Possibly add or remove segments from the segment map.  */

static bfd_boolean
elf_modify_segment_map (bfd *abfd,
			struct bfd_link_info *info,
			bfd_boolean remove_empty_load)
{
  struct elf_segment_map **m;
  const struct elf_backend_data *bed;

  /* The placement algorithm assumes that non allocated sections are
     not in PT_LOAD segments.  We ensure this here by removing such
     sections from the segment map.  We also remove excluded
     sections.  Finally, any PT_LOAD segment without sections is
     removed.  */
  m = &elf_seg_map (abfd);
  while (*m)
    {
      unsigned int i, new_count;

      for (new_count = 0, i = 0; i < (*m)->count; i++)
	{
	  if (((*m)->sections[i]->flags & SEC_EXCLUDE) == 0
	      && (((*m)->sections[i]->flags & SEC_ALLOC) != 0
		  || (*m)->p_type != PT_LOAD))
	    {
	      (*m)->sections[new_count] = (*m)->sections[i];
	      new_count++;
	    }
	}
      (*m)->count = new_count;

      if (remove_empty_load
	  && (*m)->p_type == PT_LOAD
	  && (*m)->count == 0
	  && !(*m)->includes_phdrs)
	*m = (*m)->next;
      else
	m = &(*m)->next;
    }

  bed = get_elf_backend_data (abfd);
  if (bed->elf_backend_modify_segment_map != NULL)
    {
      if (!(*bed->elf_backend_modify_segment_map) (abfd, info))
	return FALSE;
    }

  return TRUE;
}

#define IS_TBSS(s) \
  ((s->flags & (SEC_THREAD_LOCAL | SEC_LOAD)) == SEC_THREAD_LOCAL)

/* Set up a mapping from BFD sections to program segments.  */

bfd_boolean
_bfd_elf_map_sections_to_segments (bfd *abfd, struct bfd_link_info *info)
{
  unsigned int count;
  struct elf_segment_map *m;
  asection **sections = NULL;
  const struct elf_backend_data *bed = get_elf_backend_data (abfd);
  bfd_boolean no_user_phdrs;

  no_user_phdrs = elf_seg_map (abfd) == NULL;

  if (info != NULL)
    info->user_phdrs = !no_user_phdrs;

  if (no_user_phdrs && bfd_count_sections (abfd) != 0)
    {
      asection *s;
      unsigned int i;
      struct elf_segment_map *mfirst;
      struct elf_segment_map **pm;
      asection *last_hdr;
      bfd_vma last_size;
      unsigned int hdr_index;
      bfd_vma maxpagesize;
      asection **hdrpp;
      bfd_boolean phdr_in_segment;
      bfd_boolean writable;
      bfd_boolean executable;
      int tls_count = 0;
      asection *first_tls = NULL;
      asection *first_mbind = NULL;
      asection *dynsec, *eh_frame_hdr;
      bfd_size_type amt;
      bfd_vma addr_mask, wrap_to = 0;
      bfd_size_type phdr_size;

      /* Select the allocated sections, and sort them.  */

      sections = (asection **) bfd_malloc2 (bfd_count_sections (abfd),
					    sizeof (asection *));
      if (sections == NULL)
	goto error_return;

      /* Calculate top address, avoiding undefined behaviour of shift
	 left operator when shift count is equal to size of type
	 being shifted.  */
      addr_mask = ((bfd_vma) 1 << (bfd_arch_bits_per_address (abfd) - 1)) - 1;
      addr_mask = (addr_mask << 1) + 1;

      i = 0;
      for (s = abfd->sections; s != NULL; s = s->next)
	{
	  if ((s->flags & SEC_ALLOC) != 0)
	    {
	      sections[i] = s;
	      ++i;
	      /* A wrapping section potentially clashes with header.  */
	      if (((s->lma + s->size) & addr_mask) < (s->lma & addr_mask))
		wrap_to = (s->lma + s->size) & addr_mask;
	    }
	}
      BFD_ASSERT (i <= bfd_count_sections (abfd));
      count = i;

      qsort (sections, (size_t) count, sizeof (asection *), elf_sort_sections);

      phdr_size = elf_program_header_size (abfd);
      if (phdr_size == (bfd_size_type) -1)
	phdr_size = get_program_header_size (abfd, info);
      phdr_size += bed->s->sizeof_ehdr;
      maxpagesize = bed->maxpagesize;
      if (maxpagesize == 0)
	maxpagesize = 1;
      phdr_in_segment = info != NULL && info->load_phdrs;
      if (count != 0
	  && (((sections[0]->lma & addr_mask) & (maxpagesize - 1))
	      >= (phdr_size & (maxpagesize - 1))))
	/* For compatibility with old scripts that may not be using
	   SIZEOF_HEADERS, add headers when it looks like space has
	   been left for them.  */
	phdr_in_segment = TRUE;

      /* Build the mapping.  */
      mfirst = NULL;
      pm = &mfirst;

      /* If we have a .interp section, then create a PT_PHDR segment for
	 the program headers and a PT_INTERP segment for the .interp
	 section.  */
      s = bfd_get_section_by_name (abfd, ".interp");
      if (s != NULL && (s->flags & SEC_LOAD) != 0 && s->size != 0)
	{
	  amt = sizeof (struct elf_segment_map);
	  m = (struct elf_segment_map *) bfd_zalloc (abfd, amt);
	  if (m == NULL)
	    goto error_return;
	  m->next = NULL;
	  m->p_type = PT_PHDR;
	  m->p_flags = PF_R;
	  m->p_flags_valid = 1;
	  m->includes_phdrs = 1;
	  phdr_in_segment = TRUE;
	  *pm = m;
	  pm = &m->next;

	  amt = sizeof (struct elf_segment_map);
	  m = (struct elf_segment_map *) bfd_zalloc (abfd, amt);
	  if (m == NULL)
	    goto error_return;
	  m->next = NULL;
	  m->p_type = PT_INTERP;
	  m->count = 1;
	  m->sections[0] = s;

	  *pm = m;
	  pm = &m->next;
	}

      /* Look through the sections.  We put sections in the same program
	 segment when the start of the second section can be placed within
	 a few bytes of the end of the first section.  */
      last_hdr = NULL;
      last_size = 0;
      hdr_index = 0;
      writable = FALSE;
      executable = FALSE;
      dynsec = bfd_get_section_by_name (abfd, ".dynamic");
      if (dynsec != NULL
	  && (dynsec->flags & SEC_LOAD) == 0)
	dynsec = NULL;

      if ((abfd->flags & D_PAGED) == 0)
	phdr_in_segment = FALSE;

      /* Deal with -Ttext or something similar such that the first section
	 is not adjacent to the program headers.  This is an
	 approximation, since at this point we don't know exactly how many
	 program headers we will need.  */
      if (phdr_in_segment && count > 0)
	{
	  bfd_vma phdr_lma;
	  bfd_boolean separate_phdr = FALSE;

	  phdr_lma = (sections[0]->lma - phdr_size) & addr_mask & -maxpagesize;
	  if (info != NULL
	      && info->separate_code
	      && (sections[0]->flags & SEC_CODE) != 0)
	    {
	      /* If data sections should be separate from code and
		 thus not executable, and the first section is
		 executable then put the file and program headers in
		 their own PT_LOAD.  */
	      separate_phdr = TRUE;
	      if ((((phdr_lma + phdr_size - 1) & addr_mask & -maxpagesize)
		   == (sections[0]->lma & addr_mask & -maxpagesize)))
		{
		  /* The file and program headers are currently on the
		     same page as the first section.  Put them on the
		     previous page if we can.  */
		  if (phdr_lma >= maxpagesize)
		    phdr_lma -= maxpagesize;
		  else
		    separate_phdr = FALSE;
		}
	    }
	  if ((sections[0]->lma & addr_mask) < phdr_lma
	      || (sections[0]->lma & addr_mask) < phdr_size)
	    /* If file and program headers would be placed at the end
	       of memory then it's probably better to omit them.  */
	    phdr_in_segment = FALSE;
	  else if (phdr_lma < wrap_to)
	    /* If a section wraps around to where we'll be placing
	       file and program headers, then the headers will be
	       overwritten.  */
	    phdr_in_segment = FALSE;
	  else if (separate_phdr)
	    {
	      m = make_mapping (abfd, sections, 0, 0, phdr_in_segment);
	      if (m == NULL)
		goto error_return;
	      m->p_paddr = phdr_lma;
	      m->p_vaddr_offset
		= (sections[0]->vma - phdr_size) & addr_mask & -maxpagesize;
	      m->p_paddr_valid = 1;
	      *pm = m;
	      pm = &m->next;
	      phdr_in_segment = FALSE;
	    }
	}

      for (i = 0, hdrpp = sections; i < count; i++, hdrpp++)
	{
	  asection *hdr;
	  bfd_boolean new_segment;

	  hdr = *hdrpp;

	  /* See if this section and the last one will fit in the same
	     segment.  */

	  if (last_hdr == NULL)
	    {
	      /* If we don't have a segment yet, then we don't need a new
		 one (we build the last one after this loop).  */
	      new_segment = FALSE;
	    }
	  else if (last_hdr->lma - last_hdr->vma != hdr->lma - hdr->vma)
	    {
	      /* If this section has a different relation between the
		 virtual address and the load address, then we need a new
		 segment.  */
	      new_segment = TRUE;
	    }
	  else if (hdr->lma < last_hdr->lma + last_size
		   || last_hdr->lma + last_size < last_hdr->lma)
	    {
	      /* If this section has a load address that makes it overlap
		 the previous section, then we need a new segment.  */
	      new_segment = TRUE;
	    }
	  else if ((abfd->flags & D_PAGED) != 0
		   && (((last_hdr->lma + last_size - 1) & -maxpagesize)
		       == (hdr->lma & -maxpagesize)))
	    {
	      /* If we are demand paged then we can't map two disk
		 pages onto the same memory page.  */
	      new_segment = FALSE;
	    }
	  /* In the next test we have to be careful when last_hdr->lma is close
	     to the end of the address space.  If the aligned address wraps
	     around to the start of the address space, then there are no more
	     pages left in memory and it is OK to assume that the current
	     section can be included in the current segment.  */
	  else if ((BFD_ALIGN (last_hdr->lma + last_size, maxpagesize)
		    + maxpagesize > last_hdr->lma)
		   && (BFD_ALIGN (last_hdr->lma + last_size, maxpagesize)
		       + maxpagesize <= hdr->lma))
	    {
	      /* If putting this section in this segment would force us to
		 skip a page in the segment, then we need a new segment.  */
	      new_segment = TRUE;
	    }
	  else if ((last_hdr->flags & (SEC_LOAD | SEC_THREAD_LOCAL)) == 0
		   && (hdr->flags & (SEC_LOAD | SEC_THREAD_LOCAL)) != 0)
	    {
	      /* We don't want to put a loaded section after a
		 nonloaded (ie. bss style) section in the same segment
		 as that will force the non-loaded section to be loaded.
		 Consider .tbss sections as loaded for this purpose.  */
	      new_segment = TRUE;
	    }
	  else if ((abfd->flags & D_PAGED) == 0)
	    {
	      /* If the file is not demand paged, which means that we
		 don't require the sections to be correctly aligned in the
		 file, then there is no other reason for a new segment.  */
	      new_segment = FALSE;
	    }
	  else if (info != NULL
		   && info->separate_code
		   && executable != ((hdr->flags & SEC_CODE) != 0))
	    {
	      new_segment = TRUE;
	    }
	  else if (! writable
		   && (hdr->flags & SEC_READONLY) == 0)
	    {
	      /* We don't want to put a writable section in a read only
		 segment.  */
	      new_segment = TRUE;
	    }
	  else
	    {
	      /* Otherwise, we can use the same segment.  */
	      new_segment = FALSE;
	    }

	  /* Allow interested parties a chance to override our decision.  */
	  if (last_hdr != NULL
	      && info != NULL
	      && info->callbacks->override_segment_assignment != NULL)
	    new_segment
	      = info->callbacks->override_segment_assignment (info, abfd, hdr,
							      last_hdr,
							      new_segment);

	  if (! new_segment)
	    {
	      if ((hdr->flags & SEC_READONLY) == 0)
		writable = TRUE;
	      if ((hdr->flags & SEC_CODE) != 0)
		executable = TRUE;
	      last_hdr = hdr;
	      /* .tbss sections effectively have zero size.  */
	      last_size = !IS_TBSS (hdr) ? hdr->size : 0;
	      continue;
	    }

	  /* We need a new program segment.  We must create a new program
	     header holding all the sections from hdr_index until hdr.  */

	  m = make_mapping (abfd, sections, hdr_index, i, phdr_in_segment);
	  if (m == NULL)
	    goto error_return;

	  *pm = m;
	  pm = &m->next;

	  if ((hdr->flags & SEC_READONLY) == 0)
	    writable = TRUE;
	  else
	    writable = FALSE;

	  if ((hdr->flags & SEC_CODE) == 0)
	    executable = FALSE;
	  else
	    executable = TRUE;

	  last_hdr = hdr;
	  /* .tbss sections effectively have zero size.  */
	  last_size = !IS_TBSS (hdr) ? hdr->size : 0;
	  hdr_index = i;
	  phdr_in_segment = FALSE;
	}

      /* Create a final PT_LOAD program segment, but not if it's just
	 for .tbss.  */
      if (last_hdr != NULL
	  && (i - hdr_index != 1
	      || !IS_TBSS (last_hdr)))
	{
	  m = make_mapping (abfd, sections, hdr_index, i, phdr_in_segment);
	  if (m == NULL)
	    goto error_return;

	  *pm = m;
	  pm = &m->next;
	}

      /* If there is a .dynamic section, throw in a PT_DYNAMIC segment.  */
      if (dynsec != NULL)
	{
	  m = _bfd_elf_make_dynamic_segment (abfd, dynsec);
	  if (m == NULL)
	    goto error_return;
	  *pm = m;
	  pm = &m->next;
	}

      /* For each batch of consecutive loadable SHT_NOTE  sections,
	 add a PT_NOTE segment.  We don't use bfd_get_section_by_name,
	 because if we link together nonloadable .note sections and
	 loadable .note sections, we will generate two .note sections
	 in the output file.  */
      for (s = abfd->sections; s != NULL; s = s->next)
	{
	  if ((s->flags & SEC_LOAD) != 0
	      && elf_section_type (s) == SHT_NOTE)
	    {
	      asection *s2;
	      unsigned int alignment_power = s->alignment_power;

	      count = 1;
	      for (s2 = s; s2->next != NULL; s2 = s2->next)
		{
		  if (s2->next->alignment_power == alignment_power
		      && (s2->next->flags & SEC_LOAD) != 0
		      && elf_section_type (s2->next) == SHT_NOTE
		      && align_power (s2->lma + s2->size,
				      alignment_power)
		      == s2->next->lma)
		    count++;
		  else
		    break;
		}
	      amt = sizeof (struct elf_segment_map) - sizeof (asection *);
	      amt += count * sizeof (asection *);
	      m = (struct elf_segment_map *) bfd_zalloc (abfd, amt);
	      if (m == NULL)
		goto error_return;
	      m->next = NULL;
	      m->p_type = PT_NOTE;
	      m->count = count;
	      while (count > 1)
		{
		  m->sections[m->count - count--] = s;
		  BFD_ASSERT ((s->flags & SEC_THREAD_LOCAL) == 0);
		  s = s->next;
		}
	      m->sections[m->count - 1] = s;
	      BFD_ASSERT ((s->flags & SEC_THREAD_LOCAL) == 0);
	      *pm = m;
	      pm = &m->next;
	    }
	  if (s->flags & SEC_THREAD_LOCAL)
	    {
	      if (! tls_count)
		first_tls = s;
	      tls_count++;
	    }
	  if (first_mbind == NULL
	      && (elf_section_flags (s) & SHF_GNU_MBIND) != 0)
	    first_mbind = s;
	}

      /* If there are any SHF_TLS output sections, add PT_TLS segment.  */
      if (tls_count > 0)
	{
	  amt = sizeof (struct elf_segment_map) - sizeof (asection *);
	  amt += tls_count * sizeof (asection *);
	  m = (struct elf_segment_map *) bfd_zalloc (abfd, amt);
	  if (m == NULL)
	    goto error_return;
	  m->next = NULL;
	  m->p_type = PT_TLS;
	  m->count = tls_count;
	  /* Mandated PF_R.  */
	  m->p_flags = PF_R;
	  m->p_flags_valid = 1;
	  s = first_tls;
	  for (i = 0; i < (unsigned int) tls_count; ++i)
	    {
	      if ((s->flags & SEC_THREAD_LOCAL) == 0)
		{
		  _bfd_error_handler
		    (_("%pB: TLS sections are not adjacent:"), abfd);
		  s = first_tls;
		  i = 0;
		  while (i < (unsigned int) tls_count)
		    {
		      if ((s->flags & SEC_THREAD_LOCAL) != 0)
			{
			  _bfd_error_handler (_("	    TLS: %pA"), s);
			  i++;
			}
		      else
			_bfd_error_handler (_("	non-TLS: %pA"), s);
		      s = s->next;
		    }
		  bfd_set_error (bfd_error_bad_value);
		  goto error_return;
		}
	      m->sections[i] = s;
	      s = s->next;
	    }

	  *pm = m;
	  pm = &m->next;
	}

      if (first_mbind && (abfd->flags & D_PAGED) != 0)
	for (s = first_mbind; s != NULL; s = s->next)
	  if ((elf_section_flags (s) & SHF_GNU_MBIND) != 0
	      && (elf_section_data (s)->this_hdr.sh_info
		  <= PT_GNU_MBIND_NUM))
	    {
	      /* Mandated PF_R.  */
	      unsigned long p_flags = PF_R;
	      if ((s->flags & SEC_READONLY) == 0)
		p_flags |= PF_W;
	      if ((s->flags & SEC_CODE) != 0)
		p_flags |= PF_X;

	      amt = sizeof (struct elf_segment_map) + sizeof (asection *);
	      m = bfd_zalloc (abfd, amt);
	      if (m == NULL)
		goto error_return;
	      m->next = NULL;
	      m->p_type = (PT_GNU_MBIND_LO
			   + elf_section_data (s)->this_hdr.sh_info);
	      m->count = 1;
	      m->p_flags_valid = 1;
	      m->sections[0] = s;
	      m->p_flags = p_flags;

	      *pm = m;
	      pm = &m->next;
	    }

      s = bfd_get_section_by_name (abfd,
				   NOTE_GNU_PROPERTY_SECTION_NAME);
      if (s != NULL && s->size != 0)
	{
	  amt = sizeof (struct elf_segment_map) + sizeof (asection *);
	  m = bfd_zalloc (abfd, amt);
	  if (m == NULL)
	    goto error_return;
	  m->next = NULL;
	  m->p_type = PT_GNU_PROPERTY;
	  m->count = 1;
	  m->p_flags_valid = 1;
	  m->sections[0] = s;
	  m->p_flags = PF_R;
	  *pm = m;
	  pm = &m->next;
	}

      /* If there is a .eh_frame_hdr section, throw in a PT_GNU_EH_FRAME
	 segment.  */
      eh_frame_hdr = elf_eh_frame_hdr (abfd);
      if (eh_frame_hdr != NULL
	  && (eh_frame_hdr->output_section->flags & SEC_LOAD) != 0)
	{
	  amt = sizeof (struct elf_segment_map);
	  m = (struct elf_segment_map *) bfd_zalloc (abfd, amt);
	  if (m == NULL)
	    goto error_return;
	  m->next = NULL;
	  m->p_type = PT_GNU_EH_FRAME;
	  m->count = 1;
	  m->sections[0] = eh_frame_hdr->output_section;

	  *pm = m;
	  pm = &m->next;
	}

      if (elf_stack_flags (abfd))
	{
	  amt = sizeof (struct elf_segment_map);
	  m = (struct elf_segment_map *) bfd_zalloc (abfd, amt);
	  if (m == NULL)
	    goto error_return;
	  m->next = NULL;
	  m->p_type = PT_GNU_STACK;
	  m->p_flags = elf_stack_flags (abfd);
	  m->p_align = bed->stack_align;
	  m->p_flags_valid = 1;
	  m->p_align_valid = m->p_align != 0;
	  if (info->stacksize > 0)
	    {
	      m->p_size = info->stacksize;
	      m->p_size_valid = 1;
	    }

	  *pm = m;
	  pm = &m->next;
	}

      if (info != NULL && info->relro)
	{
	  for (m = mfirst; m != NULL; m = m->next)
	    {
	      if (m->p_type == PT_LOAD
		  && m->count != 0
		  && m->sections[0]->vma >= info->relro_start
		  && m->sections[0]->vma < info->relro_end)
		{
		  i = m->count;
		  while (--i != (unsigned) -1)
		    if ((m->sections[i]->flags & (SEC_LOAD | SEC_HAS_CONTENTS))
			== (SEC_LOAD | SEC_HAS_CONTENTS))
		      break;

		  if (i != (unsigned) -1)
		    break;
		}
	    }

	  /* Make a PT_GNU_RELRO segment only when it isn't empty.  */
	  if (m != NULL)
	    {
	      amt = sizeof (struct elf_segment_map);
	      m = (struct elf_segment_map *) bfd_zalloc (abfd, amt);
	      if (m == NULL)
		goto error_return;
	      m->next = NULL;
	      m->p_type = PT_GNU_RELRO;
	      *pm = m;
	      pm = &m->next;
	    }
	}

      free (sections);
      elf_seg_map (abfd) = mfirst;
    }

  if (!elf_modify_segment_map (abfd, info, no_user_phdrs))
    return FALSE;

  for (count = 0, m = elf_seg_map (abfd); m != NULL; m = m->next)
    ++count;
  elf_program_header_size (abfd) = count * bed->s->sizeof_phdr;

  return TRUE;

 error_return:
  if (sections != NULL)
    free (sections);
  return FALSE;
}

/* Sort sections by address.  */

static int
elf_sort_sections (const void *arg1, const void *arg2)
{
  const asection *sec1 = *(const asection **) arg1;
  const asection *sec2 = *(const asection **) arg2;
  bfd_size_type size1, size2;

  /* Sort by LMA first, since this is the address used to
     place the section into a segment.  */
  if (sec1->lma < sec2->lma)
    return -1;
  else if (sec1->lma > sec2->lma)
    return 1;

  /* Then sort by VMA.  Normally the LMA and the VMA will be
     the same, and this will do nothing.  */
  if (sec1->vma < sec2->vma)
    return -1;
  else if (sec1->vma > sec2->vma)
    return 1;

  /* Put !SEC_LOAD sections after SEC_LOAD ones.  */

#define TOEND(x) (((x)->flags & (SEC_LOAD | SEC_THREAD_LOCAL)) == 0)

  if (TOEND (sec1))
    {
      if (TOEND (sec2))
	{
	  /* If the indices are the same, do not return 0
	     here, but continue to try the next comparison.  */
	  if (sec1->target_index - sec2->target_index != 0)
	    return sec1->target_index - sec2->target_index;
	}
      else
	return 1;
    }
  else if (TOEND (sec2))
    return -1;

#undef TOEND

  /* Sort by size, to put zero sized sections
     before others at the same address.  */

  size1 = (sec1->flags & SEC_LOAD) ? sec1->size : 0;
  size2 = (sec2->flags & SEC_LOAD) ? sec2->size : 0;

  if (size1 < size2)
    return -1;
  if (size1 > size2)
    return 1;

  return sec1->target_index - sec2->target_index;
}

/* Assign file positions for all the reloc sections which are not part
   of the loadable file image, and the file position of section headers.  */

static bfd_boolean
_bfd_elf_assign_file_positions_for_non_load (bfd *abfd)
{
  file_ptr off;
  Elf_Internal_Shdr **shdrpp, **end_shdrpp;
  Elf_Internal_Shdr *shdrp;
  Elf_Internal_Ehdr *i_ehdrp;
  const struct elf_backend_data *bed;

  off = elf_next_file_pos (abfd);

  shdrpp = elf_elfsections (abfd);
  end_shdrpp = shdrpp + elf_numsections (abfd);
  for (shdrpp++; shdrpp < end_shdrpp; shdrpp++)
    {
      shdrp = *shdrpp;
      if (shdrp->sh_offset == -1)
	{
	  asection *sec = shdrp->bfd_section;
	  bfd_boolean is_rel = (shdrp->sh_type == SHT_REL
				|| shdrp->sh_type == SHT_RELA);
	  if (is_rel
	      || (sec != NULL && (sec->flags & SEC_ELF_COMPRESS)))
	    {
	      if (!is_rel)
		{
		  const char *name = sec->name;
		  struct bfd_elf_section_data *d;

		  /* Compress DWARF debug sections.  */
		  if (!bfd_compress_section (abfd, sec,
					     shdrp->contents))
		    return FALSE;

		  if (sec->compress_status == COMPRESS_SECTION_DONE
		      && (abfd->flags & BFD_COMPRESS_GABI) == 0)
		    {
		      /* If section is compressed with zlib-gnu, convert
			 section name from .debug_* to .zdebug_*.  */
		      char *new_name
			= convert_debug_to_zdebug (abfd, name);
		      if (new_name == NULL)
			return FALSE;
		      name = new_name;
		    }
		  /* Add section name to section name section.  */
		  if (shdrp->sh_name != (unsigned int) -1)
		    abort ();
		  shdrp->sh_name
		    = (unsigned int) _bfd_elf_strtab_add (elf_shstrtab (abfd),
							  name, FALSE);
		  d = elf_section_data (sec);

		  /* Add reloc section name to section name section.  */
		  if (d->rel.hdr
		      && !_bfd_elf_set_reloc_sh_name (abfd,
						      d->rel.hdr,
						      name, FALSE))
		    return FALSE;
		  if (d->rela.hdr
		      && !_bfd_elf_set_reloc_sh_name (abfd,
						      d->rela.hdr,
						      name, TRUE))
		    return FALSE;

		  /* Update section size and contents.  */
		  shdrp->sh_size = sec->size;
		  shdrp->contents = sec->contents;
		  shdrp->bfd_section->contents = NULL;
		}
	      off = _bfd_elf_assign_file_position_for_section (shdrp,
							       off,
							       TRUE);
	    }
	}
    }

  /* Place section name section after DWARF debug sections have been
     compressed.  */
  _bfd_elf_strtab_finalize (elf_shstrtab (abfd));
  shdrp = &elf_tdata (abfd)->shstrtab_hdr;
  shdrp->sh_size = _bfd_elf_strtab_size (elf_shstrtab (abfd));
  off = _bfd_elf_assign_file_position_for_section (shdrp, off, TRUE);

  /* Place the section headers.  */
  i_ehdrp = elf_elfheader (abfd);
  bed = get_elf_backend_data (abfd);
  off = align_file_position (off, 1 << bed->s->log_file_align);
  i_ehdrp->e_shoff = off;
  off += i_ehdrp->e_shnum * i_ehdrp->e_shentsize;
  elf_next_file_pos (abfd) = off;

  return TRUE;
}

bfd_boolean
_bfd_elf_write_object_contents (bfd *abfd)
{
  const struct elf_backend_data *bed = get_elf_backend_data (abfd);
  Elf_Internal_Shdr **i_shdrp;
  bfd_boolean failed;
  unsigned int count, num_sec;
  struct elf_obj_tdata *t;

  if (! abfd->output_has_begun
      && ! _bfd_elf_compute_section_file_positions (abfd, NULL))
    return FALSE;
  /* Do not rewrite ELF data when the BFD has been opened for update.
     abfd->output_has_begun was set to TRUE on opening, so creation of new
     sections, and modification of existing section sizes was restricted.
     This means the ELF header, program headers and section headers can't have
     changed.
     If the contents of any sections has been modified, then those changes have
     already been written to the BFD.  */
  else if (abfd->direction == both_direction)
    {
      BFD_ASSERT (abfd->output_has_begun);
      return TRUE;
    }

  i_shdrp = elf_elfsections (abfd);

  failed = FALSE;
  bfd_map_over_sections (abfd, bed->s->write_relocs, &failed);
  if (failed)
    return FALSE;

  if (!_bfd_elf_assign_file_positions_for_non_load (abfd))
    return FALSE;

  /* After writing the headers, we need to write the sections too...  */
  num_sec = elf_numsections (abfd);
  for (count = 1; count < num_sec; count++)
    {
      i_shdrp[count]->sh_name
	= _bfd_elf_strtab_offset (elf_shstrtab (abfd),
				  i_shdrp[count]->sh_name);
      if (bed->elf_backend_section_processing)
	if (!(*bed->elf_backend_section_processing) (abfd, i_shdrp[count]))
	  return FALSE;
      if (i_shdrp[count]->contents)
	{
	  bfd_size_type amt = i_shdrp[count]->sh_size;

	  if (bfd_seek (abfd, i_shdrp[count]->sh_offset, SEEK_SET) != 0
	      || bfd_bwrite (i_shdrp[count]->contents, amt, abfd) != amt)
	    return FALSE;
	}
    }

  /* Write out the section header names.  */
  t = elf_tdata (abfd);
  if (elf_shstrtab (abfd) != NULL
      && (bfd_seek (abfd, t->shstrtab_hdr.sh_offset, SEEK_SET) != 0
	  || !_bfd_elf_strtab_emit (abfd, elf_shstrtab (abfd))))
    return FALSE;

  if (bed->elf_backend_final_write_processing)
    (*bed->elf_backend_final_write_processing) (abfd, elf_linker (abfd));

  if (!bed->s->write_shdrs_and_ehdr (abfd))
    return FALSE;

  /* This is last since write_shdrs_and_ehdr can touch i_shdrp[0].  */
  if (t->o->build_id.after_write_object_contents != NULL)
    return (*t->o->build_id.after_write_object_contents) (abfd);

  return TRUE;
}

bfd_boolean
_bfd_elf_write_corefile_contents (bfd *abfd)
{
  /* Hopefully this can be done just like an object file.  */
  return _bfd_elf_write_object_contents (abfd);
}

/* Given a section, search the header to find them.  */

unsigned int
_bfd_elf_section_from_bfd_section (bfd *abfd, struct bfd_section *asect)
{
  const struct elf_backend_data *bed;
  unsigned int sec_index;

  if (elf_section_data (asect) != NULL
      && elf_section_data (asect)->this_idx != 0)
    return elf_section_data (asect)->this_idx;

  if (bfd_is_abs_section (asect))
    sec_index = SHN_ABS;
  else if (bfd_is_com_section (asect))
    sec_index = SHN_COMMON;
  else if (bfd_is_und_section (asect))
    sec_index = SHN_UNDEF;
  else
    sec_index = SHN_BAD;

  bed = get_elf_backend_data (abfd);
  if (bed->elf_backend_section_from_bfd_section)
    {
      int retval = sec_index;

      if ((*bed->elf_backend_section_from_bfd_section) (abfd, asect, &retval))
	return retval;
    }

  if (sec_index == SHN_BAD)
    bfd_set_error (bfd_error_nonrepresentable_section);

  return sec_index;
}

/* Given a BFD symbol, return the index in the ELF symbol table, or -1
   on error.  */

int
_bfd_elf_symbol_from_bfd_symbol (bfd *abfd, asymbol **asym_ptr_ptr)
{
  asymbol *asym_ptr = *asym_ptr_ptr;
  int idx;
  flagword flags = asym_ptr->flags;

  /* When gas creates relocations against local labels, it creates its
     own symbol for the section, but does put the symbol into the
     symbol chain, so udata is 0.  When the linker is generating
     relocatable output, this section symbol may be for one of the
     input sections rather than the output section.  */
  if (asym_ptr->udata.i == 0
      && (flags & BSF_SECTION_SYM)
      && asym_ptr->section)
    {
      asection *sec;
      int indx;

      sec = asym_ptr->section;
      if (sec->owner != abfd && sec->output_section != NULL)
	sec = sec->output_section;
      if (sec->owner == abfd
	  && (indx = sec->index) < elf_num_section_syms (abfd)
	  && elf_section_syms (abfd)[indx] != NULL)
	asym_ptr->udata.i = elf_section_syms (abfd)[indx]->udata.i;
    }

  idx = asym_ptr->udata.i;

  if (idx == 0)
    {
      /* This case can occur when using --strip-symbol on a symbol
	 which is used in a relocation entry.  */
      _bfd_error_handler
	/* xgettext:c-format */
	(_("%pB: symbol `%s' required but not present"),
	 abfd, bfd_asymbol_name (asym_ptr));
      bfd_set_error (bfd_error_no_symbols);
      return -1;
    }

#if DEBUG & 4
  {
    fprintf (stderr,
	     "elf_symbol_from_bfd_symbol 0x%.8lx, name = %s, sym num = %d, flags = 0x%.8x\n",
	     (long) asym_ptr, asym_ptr->name, idx, flags);
    fflush (stderr);
  }
#endif

  return idx;
}

/* Copy private BFD data.  This copies or rewrites ELF program header
   information.  */

static bfd_boolean
copy_private_bfd_data (bfd *ibfd ATTRIBUTE_UNUSED, bfd *obfd ATTRIBUTE_UNUSED)
{
  return TRUE;
}

/* Initialize private output section information from input section.  */

bfd_boolean
_bfd_elf_init_private_section_data (bfd *ibfd,
				    asection *isec,
				    bfd *obfd,
				    asection *osec,
				    struct bfd_link_info *link_info)

{
  Elf_Internal_Shdr *ihdr, *ohdr;
  bfd_boolean final_link = (link_info != NULL
			    && !bfd_link_relocatable (link_info));

  if (ibfd->xvec->flavour != bfd_target_elf_flavour
      || obfd->xvec->flavour != bfd_target_elf_flavour)
    return TRUE;

  BFD_ASSERT (elf_section_data (osec) != NULL);

  /* For objcopy and relocatable link, don't copy the output ELF
     section type from input if the output BFD section flags have been
     set to something different.  For a final link allow some flags
     that the linker clears to differ.  */
  if (elf_section_type (osec) == SHT_NULL
      && (osec->flags == isec->flags
	  || (final_link
	      && ((osec->flags ^ isec->flags)
		  & ~(SEC_LINK_ONCE | SEC_LINK_DUPLICATES | SEC_RELOC)) == 0)))
    elf_section_type (osec) = elf_section_type (isec);

  /* FIXME: Is this correct for all OS/PROC specific flags?  */
  elf_section_flags (osec) |= (elf_section_flags (isec)
			       & (SHF_MASKOS | SHF_MASKPROC));

  /* Copy sh_info from input for mbind section.  */
  if (elf_section_flags (isec) & SHF_GNU_MBIND)
    elf_section_data (osec)->this_hdr.sh_info
      = elf_section_data (isec)->this_hdr.sh_info;

  /* Set things up for objcopy and relocatable link.  The output
     SHT_GROUP section will have its elf_next_in_group pointing back
     to the input group members.  Ignore linker created group section.
     See elfNN_ia64_object_p in elfxx-ia64.c.  */
  if ((link_info == NULL
       || !link_info->resolve_section_groups)
      && (elf_sec_group (isec) == NULL
	  || (elf_sec_group (isec)->flags & SEC_LINKER_CREATED) == 0))
    {
      if (elf_section_flags (isec) & SHF_GROUP)
	elf_section_flags (osec) |= SHF_GROUP;
      elf_next_in_group (osec) = elf_next_in_group (isec);
      elf_section_data (osec)->group = elf_section_data (isec)->group;
    }

  /* If not decompress, preserve SHF_COMPRESSED.  */
  if (!final_link && (ibfd->flags & BFD_DECOMPRESS) == 0)
    elf_section_flags (osec) |= (elf_section_flags (isec)
				 & SHF_COMPRESSED);

  ihdr = &elf_section_data (isec)->this_hdr;

  /* We need to handle elf_linked_to_section for SHF_LINK_ORDER. We
     don't use the output section of the linked-to section since it
     may be NULL at this point.  */
  if ((ihdr->sh_flags & SHF_LINK_ORDER) != 0)
    {
      ohdr = &elf_section_data (osec)->this_hdr;
      ohdr->sh_flags |= SHF_LINK_ORDER;
      elf_linked_to_section (osec) = elf_linked_to_section (isec);
    }

  osec->use_rela_p = isec->use_rela_p;

  return TRUE;
}

/* Copy private section information.  This copies over the entsize
   field, and sometimes the info field.  */

bfd_boolean
_bfd_elf_copy_private_section_data (bfd *ibfd,
				    asection *isec,
				    bfd *obfd,
				    asection *osec)
{
  Elf_Internal_Shdr *ihdr, *ohdr;

  if (ibfd->xvec->flavour != bfd_target_elf_flavour
      || obfd->xvec->flavour != bfd_target_elf_flavour)
    return TRUE;

  ihdr = &elf_section_data (isec)->this_hdr;
  ohdr = &elf_section_data (osec)->this_hdr;

  ohdr->sh_entsize = ihdr->sh_entsize;

  if (ihdr->sh_type == SHT_SYMTAB
      || ihdr->sh_type == SHT_DYNSYM
      || ihdr->sh_type == SHT_GNU_verneed
      || ihdr->sh_type == SHT_GNU_verdef)
    ohdr->sh_info = ihdr->sh_info;

  return _bfd_elf_init_private_section_data (ibfd, isec, obfd, osec,
					     NULL);
}

/* Look at all the SHT_GROUP sections in IBFD, making any adjustments
   necessary if we are removing either the SHT_GROUP section or any of
   the group member sections.  DISCARDED is the value that a section's
   output_section has if the section will be discarded, NULL when this
   function is called from objcopy, bfd_abs_section_ptr when called
   from the linker.  */

bfd_boolean
_bfd_elf_fixup_group_sections (bfd *ibfd, asection *discarded)
{
  asection *isec;

  for (isec = ibfd->sections; isec != NULL; isec = isec->next)
    if (elf_section_type (isec) == SHT_GROUP)
      {
	asection *first = elf_next_in_group (isec);
	asection *s = first;
	bfd_size_type removed = 0;

	while (s != NULL)
	  {
	    /* If this member section is being output but the
	       SHT_GROUP section is not, then clear the group info
	       set up by _bfd_elf_copy_private_section_data.  */
	    if (s->output_section != discarded
		&& isec->output_section == discarded)
	      {
		elf_section_flags (s->output_section) &= ~SHF_GROUP;
		elf_group_name (s->output_section) = NULL;
	      }
	    /* Conversely, if the member section is not being output
	       but the SHT_GROUP section is, then adjust its size.  */
	    else if (s->output_section == discarded
		     && isec->output_section != discarded)
	      {
		struct bfd_elf_section_data *elf_sec = elf_section_data (s);
		removed += 4;
		if (elf_sec->rel.hdr != NULL
		    && (elf_sec->rel.hdr->sh_flags & SHF_GROUP) != 0)
		  removed += 4;
		if (elf_sec->rela.hdr != NULL
		    && (elf_sec->rela.hdr->sh_flags & SHF_GROUP) != 0)
		  removed += 4;
	      }
	    s = elf_next_in_group (s);
	    if (s == first)
	      break;
	  }
	if (removed != 0)
	  {
	    if (discarded != NULL)
	      {
		/* If we've been called for ld -r, then we need to
		   adjust the input section size.  */
		if (isec->rawsize == 0)
		  isec->rawsize = isec->size;
		isec->size = isec->rawsize - removed;
		if (isec->size <= 4)
		  {
		    isec->size = 0;
		    isec->flags |= SEC_EXCLUDE;
		  }
	      }
	    else
	      {
		/* Adjust the output section size when called from
		   objcopy. */
		isec->output_section->size -= removed;
		if (isec->output_section->size <= 4)
		  {
		    isec->output_section->size = 0;
		    isec->output_section->flags |= SEC_EXCLUDE;
		  }
	      }
	  }
      }

  return TRUE;
}

/* Copy private header information.  */

bfd_boolean
_bfd_elf_copy_private_header_data (bfd *ibfd, bfd *obfd)
{
  if (bfd_get_flavour (ibfd) != bfd_target_elf_flavour
      || bfd_get_flavour (obfd) != bfd_target_elf_flavour)
    return TRUE;

  /* Copy over private BFD data if it has not already been copied.
     This must be done here, rather than in the copy_private_bfd_data
     entry point, because the latter is called after the section
     contents have been set, which means that the program headers have
     already been worked out.  */
  if (elf_seg_map (obfd) == NULL && elf_tdata (ibfd)->phdr != NULL)
    {
      if (! copy_private_bfd_data (ibfd, obfd))
	return FALSE;
    }

  return _bfd_elf_fixup_group_sections (ibfd, NULL);
}

/* Copy private symbol information.  If this symbol is in a section
   which we did not map into a BFD section, try to map the section
   index correctly.  We use special macro definitions for the mapped
   section indices; these definitions are interpreted by the
   swap_out_syms function.  */

#define MAP_ONESYMTAB (SHN_HIOS + 1)
#define MAP_DYNSYMTAB (SHN_HIOS + 2)
#define MAP_STRTAB    (SHN_HIOS + 3)
#define MAP_SHSTRTAB  (SHN_HIOS + 4)
#define MAP_SYM_SHNDX (SHN_HIOS + 5)

bfd_boolean
_bfd_elf_copy_private_symbol_data (bfd *ibfd ATTRIBUTE_UNUSED,
                   asymbol *isymarg ATTRIBUTE_UNUSED,
                   bfd *obfd ATTRIBUTE_UNUSED,
                   asymbol *osymarg ATTRIBUTE_UNUSED)
{
  return TRUE;
}

/* Return the number of bytes required to hold the symtab vector.

   Note that we base it on the count plus 1, since we will null terminate
   the vector allocated based on this size.  However, the ELF symbol table
   always has a dummy entry as symbol #0, so it ends up even.  */

long
_bfd_elf_get_symtab_upper_bound (bfd *abfd)
{
  bfd_size_type symcount;
  long symtab_size;
  Elf_Internal_Shdr *hdr = &elf_tdata (abfd)->symtab_hdr;

  symcount = hdr->sh_size / sizeof (Elf64_External_Sym);
  if (symcount >= LONG_MAX / sizeof (asymbol *))
    {
      bfd_set_error (bfd_error_file_too_big);
      return -1;
    }
  symtab_size = (symcount + 1) * (sizeof (asymbol *));
  if (symcount > 0)
    symtab_size -= sizeof (asymbol *);

  return symtab_size;
}

long
_bfd_elf_get_dynamic_symtab_upper_bound (bfd *abfd)
{
  bfd_size_type symcount;
  long symtab_size;
  Elf_Internal_Shdr *hdr = &elf_tdata (abfd)->dynsymtab_hdr;

  if (elf_dynsymtab (abfd) == 0)
    {
      bfd_set_error (bfd_error_invalid_operation);
      return -1;
    }

  symcount = hdr->sh_size / sizeof (Elf64_External_Sym);
  if (symcount >= LONG_MAX / sizeof (asymbol *))
    {
      bfd_set_error (bfd_error_file_too_big);
      return -1;
    }
  symtab_size = (symcount + 1) * (sizeof (asymbol *));
  if (symcount > 0)
    symtab_size -= sizeof (asymbol *);

  return symtab_size;
}

long
_bfd_elf_get_reloc_upper_bound (bfd *abfd ATTRIBUTE_UNUSED,
				sec_ptr asect)
{
  return (asect->reloc_count + 1) * sizeof (arelent *);
}

/* Canonicalize the relocs.  */

long
_bfd_elf_canonicalize_reloc (bfd *abfd,
			     sec_ptr section,
			     arelent **relptr,
			     asymbol **symbols)
{
  arelent *tblptr;
  unsigned int i;
  const struct elf_backend_data *bed = get_elf_backend_data (abfd);

  if (! bed->s->slurp_reloc_table (abfd, section, symbols, FALSE))
    return -1;

  tblptr = section->relocation;
  for (i = 0; i < section->reloc_count; i++)
    *relptr++ = tblptr++;

  *relptr = NULL;

  return section->reloc_count;
}

long
elf_slurp_symbol_table (bfd *abfd, asymbol **symptrs, bfd_boolean dynamic);

long
_bfd_elf_canonicalize_symtab (bfd *abfd, asymbol **allocation)
{
  long symcount = elf_slurp_symbol_table (abfd, allocation, FALSE);

  if (symcount >= 0)
    bfd_get_symcount (abfd) = symcount;
  return symcount;
}

long
_bfd_elf_canonicalize_dynamic_symtab (bfd *abfd,
				      asymbol **allocation)
{
  long symcount = elf_slurp_symbol_table (abfd, allocation, TRUE);

  if (symcount >= 0)
    bfd_get_dynamic_symcount (abfd) = symcount;
  return symcount;
}

/* Return the size required for the dynamic reloc entries.  Any loadable
   section that was actually installed in the BFD, and has type SHT_REL
   or SHT_RELA, and uses the dynamic symbol table, is considered to be a
   dynamic reloc section.  */

long
_bfd_elf_get_dynamic_reloc_upper_bound (bfd *abfd)
{
  bfd_size_type count;
  asection *s;

  if (elf_dynsymtab (abfd) == 0)
    {
      bfd_set_error (bfd_error_invalid_operation);
      return -1;
    }

  count = 1;
  for (s = abfd->sections; s != NULL; s = s->next)
    if (elf_section_data (s)->this_hdr.sh_link == elf_dynsymtab (abfd)
	&& (elf_section_data (s)->this_hdr.sh_type == SHT_REL
	    || elf_section_data (s)->this_hdr.sh_type == SHT_RELA))
      {
	count += s->size / elf_section_data (s)->this_hdr.sh_entsize;
	if (count > LONG_MAX / sizeof (arelent *))
	  {
	    bfd_set_error (bfd_error_file_too_big);
	    return -1;
	  }
      }
  return count * sizeof (arelent *);
}

/* Canonicalize the dynamic relocation entries.  Note that we return the
   dynamic relocations as a single block, although they are actually
   associated with particular sections; the interface, which was
   designed for SunOS style shared libraries, expects that there is only
   one set of dynamic relocs.  Any loadable section that was actually
   installed in the BFD, and has type SHT_REL or SHT_RELA, and uses the
   dynamic symbol table, is considered to be a dynamic reloc section.  */

long
_bfd_elf_canonicalize_dynamic_reloc (bfd *abfd,
				     arelent **storage,
				     asymbol **syms)
{
  bfd_boolean (*slurp_relocs) (bfd *, asection *, asymbol **, bfd_boolean);
  asection *s;
  long ret;

  if (elf_dynsymtab (abfd) == 0)
    {
      bfd_set_error (bfd_error_invalid_operation);
      return -1;
    }

  slurp_relocs = get_elf_backend_data (abfd)->s->slurp_reloc_table;
  ret = 0;
  for (s = abfd->sections; s != NULL; s = s->next)
    {
      if (elf_section_data (s)->this_hdr.sh_link == elf_dynsymtab (abfd)
	  && (elf_section_data (s)->this_hdr.sh_type == SHT_REL
	      || elf_section_data (s)->this_hdr.sh_type == SHT_RELA))
	{
	  arelent *p;
	  long count, i;

	  if (! (*slurp_relocs) (abfd, s, syms, TRUE))
	    return -1;
	  count = s->size / elf_section_data (s)->this_hdr.sh_entsize;
	  p = s->relocation;
	  for (i = 0; i < count; i++)
	    *storage++ = p++;
	  ret += count;
	}
    }

  *storage = NULL;

  return ret;
}

/* Read in the version information.  */

bfd_boolean
_bfd_elf_slurp_version_tables (bfd *abfd ATTRIBUTE_UNUSED, bfd_boolean default_imported_symver ATTRIBUTE_UNUSED)
{
  return TRUE;
}

asymbol *
_bfd_elf_make_empty_symbol (bfd *abfd)
{
  elf_symbol_type *newsym;

  newsym = (elf_symbol_type *) bfd_zalloc (abfd, sizeof * newsym);
  if (!newsym)
    return NULL;
  newsym->symbol.the_bfd = abfd;
  return &newsym->symbol;
}

void
_bfd_elf_get_symbol_info (bfd *abfd ATTRIBUTE_UNUSED,
			  asymbol *symbol,
			  symbol_info *ret)
{
  bfd_symbol_info (symbol, ret);
}

/* Return whether a symbol name implies a local symbol.  Most targets
   use this function for the is_local_label_name entry point, but some
   override it.  */

bfd_boolean
_bfd_elf_is_local_label_name (bfd *abfd ATTRIBUTE_UNUSED,
			      const char *name)
{
  /* Normal local symbols start with ``.L''.  */
  if (name[0] == '.' && name[1] == 'L')
    return TRUE;

  /* At least some SVR4 compilers (e.g., UnixWare 2.1 cc) generate
     DWARF debugging symbols starting with ``..''.  */
  if (name[0] == '.' && name[1] == '.')
    return TRUE;

  /* gcc will sometimes generate symbols beginning with ``_.L_'' when
     emitting DWARF debugging output.  I suspect this is actually a
     small bug in gcc (it calls ASM_OUTPUT_LABEL when it should call
     ASM_GENERATE_INTERNAL_LABEL, and this causes the leading
     underscore to be emitted on some ELF targets).  For ease of use,
     we treat such symbols as local.  */
  if (name[0] == '_' && name[1] == '.' && name[2] == 'L' && name[3] == '_')
    return TRUE;

  /* Treat assembler generated fake symbols, dollar local labels and
     forward-backward labels (aka local labels) as locals.
     These labels have the form:

       L0^A.*				       (fake symbols)

       [.]?L[0123456789]+{^A|^B}[0123456789]*  (local labels)

     Versions which start with .L will have already been matched above,
     so we only need to match the rest.  */
  if (name[0] == 'L' && isdigit (name[1]))
    {
      bfd_boolean ret = FALSE;
      const char * p;
      char c;

      for (p = name + 2; (c = *p); p++)
	{
	  if (c == 1 || c == 2)
	    {
	      if (c == 1 && p == name + 2)
		/* A fake symbol.  */
		return TRUE;

	      /* FIXME: We are being paranoid here and treating symbols like
		 L0^Bfoo as if there were non-local, on the grounds that the
		 assembler will never generate them.  But can any symbol
		 containing an ASCII value in the range 1-31 ever be anything
		 other than some kind of local ?  */
	      ret = TRUE;
	    }

      if (! isdigit (c))
	    {
	      ret = FALSE;
	      break;
	    }
	}
      return ret;
    }

  return FALSE;
}

alent *
_bfd_elf_get_lineno (bfd *abfd ATTRIBUTE_UNUSED,
		     asymbol *symbol ATTRIBUTE_UNUSED)
{
  abort ();
  return NULL;
}

bfd_boolean
_bfd_elf_set_arch_mach (bfd *abfd,
			enum bfd_architecture arch,
			unsigned long machine)
{
  /* If this isn't the right architecture for this backend, and this
     isn't the generic backend, fail.  */
  if (arch != get_elf_backend_data (abfd)->arch
      && arch != bfd_arch_unknown
      && get_elf_backend_data (abfd)->arch != bfd_arch_unknown)
    return FALSE;

  return bfd_default_set_arch_mach (abfd, arch, machine);
}

/* Find the nearest line to a particular section and offset,
   for error reporting.  */

bfd_boolean
_bfd_elf_find_nearest_line (bfd *abfd,
			    asymbol **symbols,
			    asection *section,
			    bfd_vma offset,
			    const char **filename_ptr,
			    const char **functionname_ptr,
			    unsigned int *line_ptr,
			    unsigned int *discriminator_ptr)
{
  if (_bfd_dwarf2_find_nearest_line (abfd, symbols, NULL, section, offset,
				     filename_ptr, functionname_ptr,
				     line_ptr, discriminator_ptr,
				     dwarf_debug_sections, 0,
				     &elf_tdata (abfd)->dwarf2_find_line_info))
    {
      if (!*functionname_ptr)
	_bfd_elf_find_function (abfd, symbols, section, offset,
				*filename_ptr ? NULL : filename_ptr,
				functionname_ptr);
      return TRUE;
    }

  if ((*functionname_ptr || *line_ptr))
    return TRUE;

  if (symbols == NULL)
    return FALSE;

  if (! _bfd_elf_find_function (abfd, symbols, section, offset,
				filename_ptr, functionname_ptr))
    return FALSE;

  *line_ptr = 0;
  return TRUE;
}

/* Find the line for a symbol.  */

bfd_boolean
_bfd_elf_find_line (bfd *abfd, asymbol **symbols, asymbol *symbol,
		    const char **filename_ptr, unsigned int *line_ptr)
{
  return _bfd_dwarf2_find_nearest_line (abfd, symbols, symbol, NULL, 0,
					filename_ptr, NULL, line_ptr, NULL,
					dwarf_debug_sections, 0,
					&elf_tdata (abfd)->dwarf2_find_line_info);
}

/* After a call to bfd_find_nearest_line, successive calls to
   bfd_find_inliner_info can be used to get source information about
   each level of function inlining that terminated at the address
   passed to bfd_find_nearest_line.  Currently this is only supported
   for DWARF2 with appropriate DWARF3 extensions. */

bfd_boolean
_bfd_elf_find_inliner_info (bfd *abfd,
			    const char **filename_ptr,
			    const char **functionname_ptr,
			    unsigned int *line_ptr)
{
  bfd_boolean found;
  found = _bfd_dwarf2_find_inliner_info (abfd, filename_ptr,
					 functionname_ptr, line_ptr,
					 & elf_tdata (abfd)->dwarf2_find_line_info);
  return found;
}

int
_bfd_elf_sizeof_headers (bfd *abfd, struct bfd_link_info *info)
{
  const struct elf_backend_data *bed = get_elf_backend_data (abfd);
  int ret = bed->s->sizeof_ehdr;

  if (!bfd_link_relocatable (info))
    {
      bfd_size_type phdr_size = elf_program_header_size (abfd);

      if (phdr_size == (bfd_size_type) -1)
	{
	  struct elf_segment_map *m;

	  phdr_size = 0;
	  for (m = elf_seg_map (abfd); m != NULL; m = m->next)
	    phdr_size += bed->s->sizeof_phdr;

	  if (phdr_size == 0)
	    phdr_size = get_program_header_size (abfd, info);
	}

      elf_program_header_size (abfd) = phdr_size;
      ret += phdr_size;
    }

  return ret;
}

bfd_boolean
_bfd_elf_set_section_contents (bfd *abfd,
			       sec_ptr section,
			       const void *location,
			       file_ptr offset,
			       bfd_size_type count)
{
  Elf_Internal_Shdr *hdr;
  file_ptr pos;

  if (! abfd->output_has_begun
      && ! _bfd_elf_compute_section_file_positions (abfd, NULL))
    return FALSE;

  if (!count)
    return TRUE;

  hdr = &elf_section_data (section)->this_hdr;
  if (hdr->sh_offset == (file_ptr) -1)
    {
      /* We must compress this section.  Write output to the buffer.  */
      unsigned char *contents = hdr->contents;
      if ((offset + count) > hdr->sh_size
	  || (section->flags & SEC_ELF_COMPRESS) == 0
	  || contents == NULL)
	abort ();
      memcpy (contents + offset, location, count);
      return TRUE;
    }
  pos = hdr->sh_offset + offset;
  if (bfd_seek (abfd, pos, SEEK_SET) != 0
      || bfd_bwrite (location, count, abfd) != count)
    return FALSE;

  return TRUE;
}

bfd_boolean
_bfd_elf_no_info_to_howto (bfd *abfd ATTRIBUTE_UNUSED,
			   arelent *cache_ptr ATTRIBUTE_UNUSED,
			   Elf_Internal_Rela *dst ATTRIBUTE_UNUSED)
{
  abort ();
  return FALSE;
}

/* Try to convert a non-ELF reloc into an ELF one.  */

bfd_boolean
_bfd_elf_validate_reloc (bfd *abfd, arelent *areloc)
{
  /* Check whether we really have an ELF howto.  */

  if ((*areloc->sym_ptr_ptr)->the_bfd->xvec != abfd->xvec)
    {
      bfd_reloc_code_real_type code;
      reloc_howto_type *howto;

      /* Alien reloc: Try to determine its type to replace it with an
	 equivalent ELF reloc.  */

      if (areloc->howto->pc_relative)
	{
	  switch (areloc->howto->bitsize)
	    {
	    case 8:
	      code = BFD_RELOC_8_PCREL;
	      break;
	    case 12:
	      code = BFD_RELOC_12_PCREL;
	      break;
	    case 16:
	      code = BFD_RELOC_16_PCREL;
	      break;
	    case 24:
	      code = BFD_RELOC_24_PCREL;
	      break;
	    case 32:
	      code = BFD_RELOC_32_PCREL;
	      break;
	    case 64:
	      code = BFD_RELOC_64_PCREL;
	      break;
	    default:
	      goto fail;
	    }

	  howto = bfd_reloc_type_lookup (abfd, code);

	  if (areloc->howto->pcrel_offset != howto->pcrel_offset)
	    {
	      if (howto->pcrel_offset)
		areloc->addend += areloc->address;
	      else
		areloc->addend -= areloc->address; /* addend is unsigned!! */
	    }
	}
      else
	{
	  switch (areloc->howto->bitsize)
	    {
	    case 8:
	      code = BFD_RELOC_8;
	      break;
	    case 14:
	      code = BFD_RELOC_14;
	      break;
	    case 16:
	      code = BFD_RELOC_16;
	      break;
	    case 26:
	      code = BFD_RELOC_26;
	      break;
	    case 32:
	      code = BFD_RELOC_32;
	      break;
	    case 64:
	      code = BFD_RELOC_64;
	      break;
	    default:
	      goto fail;
	    }

	  howto = bfd_reloc_type_lookup (abfd, code);
	}

      if (howto)
	areloc->howto = howto;
      else
	goto fail;
    }

  return TRUE;

 fail:
  /* xgettext:c-format */
  _bfd_error_handler (_("%pB: %s unsupported"),
		      abfd, areloc->howto->name);
  bfd_set_error (bfd_error_bad_value);
  return FALSE;
}

bfd_boolean
_bfd_elf_close_and_cleanup (bfd *abfd)
{
  struct elf_obj_tdata *tdata = elf_tdata (abfd);
  if (bfd_get_format (abfd) == bfd_object && tdata != NULL)
    {
      if (elf_tdata (abfd)->o != NULL && elf_shstrtab (abfd) != NULL)
	_bfd_elf_strtab_free (elf_shstrtab (abfd));
      _bfd_dwarf2_cleanup_debug_info (abfd, &tdata->dwarf2_find_line_info);
    }

  return _bfd_generic_close_and_cleanup (abfd);
}

/* For Rel targets, we encode meaningful data for BFD_RELOC_VTABLE_ENTRY
   in the relocation's offset.  Thus we cannot allow any sort of sanity
   range-checking to interfere.  There is nothing else to do in processing
   this reloc.  */

bfd_reloc_status_type
_bfd_elf_rel_vtable_reloc_fn
  (bfd *abfd ATTRIBUTE_UNUSED, arelent *re ATTRIBUTE_UNUSED,
   struct bfd_symbol *symbol ATTRIBUTE_UNUSED,
   void *data ATTRIBUTE_UNUSED, asection *is ATTRIBUTE_UNUSED,
   bfd *obfd ATTRIBUTE_UNUSED, char **errmsg ATTRIBUTE_UNUSED)
{
  return bfd_reloc_ok;
}

/* Elf core file support.  Much of this only works on native
   toolchains, since we rely on knowing the
   machine-dependent procfs structure in order to pick
   out details about the corefile.  */

#ifdef HAVE_SYS_PROCFS_H
/* Needed for new procfs interface on sparc-solaris.  */
# define _STRUCTURED_PROC 1
# include <sys/procfs.h>
#endif

/* Return a PID that identifies a "thread" for threaded cores, or the
   PID of the main process for non-threaded cores.  */

static int
elfcore_make_pid (bfd *abfd)
{
  int pid;

  pid = elf_tdata (abfd)->core->lwpid;
  if (pid == 0)
    pid = elf_tdata (abfd)->core->pid;

  return pid;
}

/* If there isn't a section called NAME, make one, using
   data from SECT.  Note, this function will generate a
   reference to NAME, so you shouldn't deallocate or
   overwrite it.  */

static bfd_boolean
elfcore_maybe_make_sect (bfd *abfd, char *name, asection *sect)
{
  asection *sect2;

  if (bfd_get_section_by_name (abfd, name) != NULL)
    return TRUE;

  sect2 = bfd_make_section_with_flags (abfd, name, sect->flags);
  if (sect2 == NULL)
    return FALSE;

  sect2->size = sect->size;
  sect2->filepos = sect->filepos;
  sect2->alignment_power = sect->alignment_power;
  return TRUE;
}

/* Create a pseudosection containing SIZE bytes at FILEPOS.  This
   actually creates up to two pseudosections:
   - For the single-threaded case, a section named NAME, unless
     such a section already exists.
   - For the multi-threaded case, a section named "NAME/PID", where
     PID is elfcore_make_pid (abfd).
   Both pseudosections have identical contents.  */
bfd_boolean
_bfd_elfcore_make_pseudosection (bfd *abfd,
				 char *name,
				 size_t size,
				 ufile_ptr filepos)
{
  char buf[100];
  char *threaded_name;
  size_t len;
  asection *sect;

  /* Build the section name.  */

  sprintf (buf, "%s/%d", name, elfcore_make_pid (abfd));
  len = strlen (buf) + 1;
  threaded_name = (char *) bfd_alloc (abfd, len);
  if (threaded_name == NULL)
    return FALSE;
  memcpy (threaded_name, buf, len);

  sect = bfd_make_section_anyway_with_flags (abfd, threaded_name,
					     SEC_HAS_CONTENTS);
  if (sect == NULL)
    return FALSE;
  sect->size = size;
  sect->filepos = filepos;
  sect->alignment_power = 2;

  return elfcore_maybe_make_sect (abfd, name, sect);
}

/* prstatus_t exists on:
     solaris 2.5+
     linux 2.[01] + glibc
     unixware 4.2
*/

#if defined (HAVE_PRSTATUS_T)

static bfd_boolean
elfcore_grok_prstatus (bfd *abfd, Elf_Internal_Note *note)
{
  size_t size;
  int offset;

  if (note->descsz == sizeof (prstatus_t))
    {
      prstatus_t prstat;

      size = sizeof (prstat.pr_reg);
      offset   = offsetof (prstatus_t, pr_reg);
      memcpy (&prstat, note->descdata, sizeof (prstat));

      /* Do not overwrite the core signal if it
	 has already been set by another thread.  */
      if (elf_tdata (abfd)->core->signal == 0)
	elf_tdata (abfd)->core->signal = prstat.pr_cursig;
      if (elf_tdata (abfd)->core->pid == 0)
	elf_tdata (abfd)->core->pid = prstat.pr_pid;

      /* pr_who exists on:
	 solaris 2.5+
	 unixware 4.2
	 pr_who doesn't exist on:
	 linux 2.[01]
	 */
#if defined (HAVE_PRSTATUS_T_PR_WHO)
      elf_tdata (abfd)->core->lwpid = prstat.pr_who;
#else
      elf_tdata (abfd)->core->lwpid = prstat.pr_pid;
#endif
    }
#if defined (HAVE_PRSTATUS32_T)
  else if (note->descsz == sizeof (prstatus32_t))
    {
      /* 64-bit host, 32-bit corefile */
      prstatus32_t prstat;

      size = sizeof (prstat.pr_reg);
      offset   = offsetof (prstatus32_t, pr_reg);
      memcpy (&prstat, note->descdata, sizeof (prstat));

      /* Do not overwrite the core signal if it
	 has already been set by another thread.  */
      if (elf_tdata (abfd)->core->signal == 0)
	elf_tdata (abfd)->core->signal = prstat.pr_cursig;
      if (elf_tdata (abfd)->core->pid == 0)
	elf_tdata (abfd)->core->pid = prstat.pr_pid;

      /* pr_who exists on:
	 solaris 2.5+
	 unixware 4.2
	 pr_who doesn't exist on:
	 linux 2.[01]
	 */
#if defined (HAVE_PRSTATUS32_T_PR_WHO)
      elf_tdata (abfd)->core->lwpid = prstat.pr_who;
#else
      elf_tdata (abfd)->core->lwpid = prstat.pr_pid;
#endif
    }
#endif /* HAVE_PRSTATUS32_T */
  else
    {
      /* Fail - we don't know how to handle any other
	 note size (ie. data object type).  */
      return TRUE;
    }

  /* Make a ".reg/999" section and a ".reg" section.  */
  return _bfd_elfcore_make_pseudosection (abfd, ".reg",
					  size, note->descpos + offset);
}
#endif /* defined (HAVE_PRSTATUS_T) */

/* Create a pseudosection containing the exact contents of NOTE.  */
static bfd_boolean
elfcore_make_note_pseudosection (bfd *abfd,
				 char *name,
				 Elf_Internal_Note *note)
{
  return _bfd_elfcore_make_pseudosection (abfd, name,
					  note->descsz, note->descpos);
}

/* There isn't a consistent prfpregset_t across platforms,
   but it doesn't matter, because we don't have to pick this
   data structure apart.  */

static bfd_boolean
elfcore_grok_prfpreg (bfd *abfd, Elf_Internal_Note *note)
{
  return elfcore_make_note_pseudosection (abfd, ".reg2", note);
}

/* Linux dumps the Intel SSE regs in a note named "LINUX" with a note
   type of NT_PRXFPREG.  Just include the whole note's contents
   literally.  */

static bfd_boolean
elfcore_grok_prxfpreg (bfd *abfd, Elf_Internal_Note *note)
{
  return elfcore_make_note_pseudosection (abfd, ".reg-xfp", note);
}

/* Linux dumps the Intel XSAVE extended state in a note named "LINUX"
   with a note type of NT_X86_XSTATE.  Just include the whole note's
   contents literally.  */

static bfd_boolean
elfcore_grok_xstatereg (bfd *abfd, Elf_Internal_Note *note)
{
  return elfcore_make_note_pseudosection (abfd, ".reg-xstate", note);
}

static bfd_boolean
elfcore_grok_ppc_vmx (bfd *abfd, Elf_Internal_Note *note)
{
  return elfcore_make_note_pseudosection (abfd, ".reg-ppc-vmx", note);
}

static bfd_boolean
elfcore_grok_ppc_vsx (bfd *abfd, Elf_Internal_Note *note)
{
  return elfcore_make_note_pseudosection (abfd, ".reg-ppc-vsx", note);
}

static bfd_boolean
elfcore_grok_ppc_tar (bfd *abfd, Elf_Internal_Note *note)
{
  return elfcore_make_note_pseudosection (abfd, ".reg-ppc-tar", note);
}

static bfd_boolean
elfcore_grok_ppc_ppr (bfd *abfd, Elf_Internal_Note *note)
{
  return elfcore_make_note_pseudosection (abfd, ".reg-ppc-ppr", note);
}

static bfd_boolean
elfcore_grok_ppc_dscr (bfd *abfd, Elf_Internal_Note *note)
{
  return elfcore_make_note_pseudosection (abfd, ".reg-ppc-dscr", note);
}

static bfd_boolean
elfcore_grok_ppc_ebb (bfd *abfd, Elf_Internal_Note *note)
{
  return elfcore_make_note_pseudosection (abfd, ".reg-ppc-ebb", note);
}

static bfd_boolean
elfcore_grok_ppc_pmu (bfd *abfd, Elf_Internal_Note *note)
{
  return elfcore_make_note_pseudosection (abfd, ".reg-ppc-pmu", note);
}

static bfd_boolean
elfcore_grok_ppc_tm_cgpr (bfd *abfd, Elf_Internal_Note *note)
{
  return elfcore_make_note_pseudosection (abfd, ".reg-ppc-tm-cgpr", note);
}

static bfd_boolean
elfcore_grok_ppc_tm_cfpr (bfd *abfd, Elf_Internal_Note *note)
{
  return elfcore_make_note_pseudosection (abfd, ".reg-ppc-tm-cfpr", note);
}

static bfd_boolean
elfcore_grok_ppc_tm_cvmx (bfd *abfd, Elf_Internal_Note *note)
{
  return elfcore_make_note_pseudosection (abfd, ".reg-ppc-tm-cvmx", note);
}

static bfd_boolean
elfcore_grok_ppc_tm_cvsx (bfd *abfd, Elf_Internal_Note *note)
{
  return elfcore_make_note_pseudosection (abfd, ".reg-ppc-tm-cvsx", note);
}

static bfd_boolean
elfcore_grok_ppc_tm_spr (bfd *abfd, Elf_Internal_Note *note)
{
  return elfcore_make_note_pseudosection (abfd, ".reg-ppc-tm-spr", note);
}

static bfd_boolean
elfcore_grok_ppc_tm_ctar (bfd *abfd, Elf_Internal_Note *note)
{
  return elfcore_make_note_pseudosection (abfd, ".reg-ppc-tm-ctar", note);
}

static bfd_boolean
elfcore_grok_ppc_tm_cppr (bfd *abfd, Elf_Internal_Note *note)
{
  return elfcore_make_note_pseudosection (abfd, ".reg-ppc-tm-cppr", note);
}

static bfd_boolean
elfcore_grok_ppc_tm_cdscr (bfd *abfd, Elf_Internal_Note *note)
{
  return elfcore_make_note_pseudosection (abfd, ".reg-ppc-tm-cdscr", note);
}

static bfd_boolean
elfcore_grok_s390_high_gprs (bfd *abfd, Elf_Internal_Note *note)
{
  return elfcore_make_note_pseudosection (abfd, ".reg-s390-high-gprs", note);
}

static bfd_boolean
elfcore_grok_s390_timer (bfd *abfd, Elf_Internal_Note *note)
{
  return elfcore_make_note_pseudosection (abfd, ".reg-s390-timer", note);
}

static bfd_boolean
elfcore_grok_s390_todcmp (bfd *abfd, Elf_Internal_Note *note)
{
  return elfcore_make_note_pseudosection (abfd, ".reg-s390-todcmp", note);
}

static bfd_boolean
elfcore_grok_s390_todpreg (bfd *abfd, Elf_Internal_Note *note)
{
  return elfcore_make_note_pseudosection (abfd, ".reg-s390-todpreg", note);
}

static bfd_boolean
elfcore_grok_s390_ctrs (bfd *abfd, Elf_Internal_Note *note)
{
  return elfcore_make_note_pseudosection (abfd, ".reg-s390-ctrs", note);
}

static bfd_boolean
elfcore_grok_s390_prefix (bfd *abfd, Elf_Internal_Note *note)
{
  return elfcore_make_note_pseudosection (abfd, ".reg-s390-prefix", note);
}

static bfd_boolean
elfcore_grok_s390_last_break (bfd *abfd, Elf_Internal_Note *note)
{
  return elfcore_make_note_pseudosection (abfd, ".reg-s390-last-break", note);
}

static bfd_boolean
elfcore_grok_s390_system_call (bfd *abfd, Elf_Internal_Note *note)
{
  return elfcore_make_note_pseudosection (abfd, ".reg-s390-system-call", note);
}

static bfd_boolean
elfcore_grok_s390_tdb (bfd *abfd, Elf_Internal_Note *note)
{
  return elfcore_make_note_pseudosection (abfd, ".reg-s390-tdb", note);
}

static bfd_boolean
elfcore_grok_s390_vxrs_low (bfd *abfd, Elf_Internal_Note *note)
{
  return elfcore_make_note_pseudosection (abfd, ".reg-s390-vxrs-low", note);
}

static bfd_boolean
elfcore_grok_s390_vxrs_high (bfd *abfd, Elf_Internal_Note *note)
{
  return elfcore_make_note_pseudosection (abfd, ".reg-s390-vxrs-high", note);
}

static bfd_boolean
elfcore_grok_s390_gs_cb (bfd *abfd, Elf_Internal_Note *note)
{
  return elfcore_make_note_pseudosection (abfd, ".reg-s390-gs-cb", note);
}

static bfd_boolean
elfcore_grok_s390_gs_bc (bfd *abfd, Elf_Internal_Note *note)
{
  return elfcore_make_note_pseudosection (abfd, ".reg-s390-gs-bc", note);
}

static bfd_boolean
elfcore_grok_arm_vfp (bfd *abfd, Elf_Internal_Note *note)
{
  return elfcore_make_note_pseudosection (abfd, ".reg-arm-vfp", note);
}

static bfd_boolean
elfcore_grok_aarch_tls (bfd *abfd, Elf_Internal_Note *note)
{
  return elfcore_make_note_pseudosection (abfd, ".reg-aarch-tls", note);
}

static bfd_boolean
elfcore_grok_aarch_hw_break (bfd *abfd, Elf_Internal_Note *note)
{
  return elfcore_make_note_pseudosection (abfd, ".reg-aarch-hw-break", note);
}

static bfd_boolean
elfcore_grok_aarch_hw_watch (bfd *abfd, Elf_Internal_Note *note)
{
  return elfcore_make_note_pseudosection (abfd, ".reg-aarch-hw-watch", note);
}

static bfd_boolean
elfcore_grok_aarch_sve (bfd *abfd, Elf_Internal_Note *note)
{
  return elfcore_make_note_pseudosection (abfd, ".reg-aarch-sve", note);
}

#if defined (HAVE_PRPSINFO_T)
typedef prpsinfo_t   elfcore_psinfo_t;
#if defined (HAVE_PRPSINFO32_T)		/* Sparc64 cross Sparc32 */
typedef prpsinfo32_t elfcore_psinfo32_t;
#endif
#endif

#if defined (HAVE_PSINFO_T)
typedef psinfo_t   elfcore_psinfo_t;
#if defined (HAVE_PSINFO32_T)		/* Sparc64 cross Sparc32 */
typedef psinfo32_t elfcore_psinfo32_t;
#endif
#endif

/* return a malloc'ed copy of a string at START which is at
   most MAX bytes long, possibly without a terminating '\0'.
   the copy will always have a terminating '\0'.  */

char *
_bfd_elfcore_strndup (bfd *abfd, char *start, size_t max)
{
  char *dups;
  char *end = (char *) memchr (start, '\0', max);
  size_t len;

  if (end == NULL)
    len = max;
  else
    len = end - start;

  dups = (char *) bfd_alloc (abfd, len + 1);
  if (dups == NULL)
    return NULL;

  memcpy (dups, start, len);
  dups[len] = '\0';

  return dups;
}

#if defined (HAVE_PRPSINFO_T) || defined (HAVE_PSINFO_T)
static bfd_boolean
elfcore_grok_psinfo (bfd *abfd, Elf_Internal_Note *note)
{
  if (note->descsz == sizeof (elfcore_psinfo_t))
    {
      elfcore_psinfo_t psinfo;

      memcpy (&psinfo, note->descdata, sizeof (psinfo));

#if defined (HAVE_PSINFO_T_PR_PID) || defined (HAVE_PRPSINFO_T_PR_PID)
      elf_tdata (abfd)->core->pid = psinfo.pr_pid;
#endif
      elf_tdata (abfd)->core->program
	= _bfd_elfcore_strndup (abfd, psinfo.pr_fname,
				sizeof (psinfo.pr_fname));

      elf_tdata (abfd)->core->command
	= _bfd_elfcore_strndup (abfd, psinfo.pr_psargs,
				sizeof (psinfo.pr_psargs));
    }
#if defined (HAVE_PRPSINFO32_T) || defined (HAVE_PSINFO32_T)
  else if (note->descsz == sizeof (elfcore_psinfo32_t))
    {
      /* 64-bit host, 32-bit corefile */
      elfcore_psinfo32_t psinfo;

      memcpy (&psinfo, note->descdata, sizeof (psinfo));

#if defined (HAVE_PSINFO32_T_PR_PID) || defined (HAVE_PRPSINFO32_T_PR_PID)
      elf_tdata (abfd)->core->pid = psinfo.pr_pid;
#endif
      elf_tdata (abfd)->core->program
	= _bfd_elfcore_strndup (abfd, psinfo.pr_fname,
				sizeof (psinfo.pr_fname));

      elf_tdata (abfd)->core->command
	= _bfd_elfcore_strndup (abfd, psinfo.pr_psargs,
				sizeof (psinfo.pr_psargs));
    }
#endif

  else
    {
      /* Fail - we don't know how to handle any other
	 note size (ie. data object type).  */
      return TRUE;
    }

  /* Note that for some reason, a spurious space is tacked
     onto the end of the args in some (at least one anyway)
     implementations, so strip it off if it exists.  */

  {
    char *command = elf_tdata (abfd)->core->command;
    int n = strlen (command);

    if (0 < n && command[n - 1] == ' ')
      command[n - 1] = '\0';
  }

  return TRUE;
}
#endif /* defined (HAVE_PRPSINFO_T) || defined (HAVE_PSINFO_T) */

#if defined (HAVE_PSTATUS_T)
static bfd_boolean
elfcore_grok_pstatus (bfd *abfd, Elf_Internal_Note *note)
{
  if (note->descsz == sizeof (pstatus_t)
#if defined (HAVE_PXSTATUS_T)
      || note->descsz == sizeof (pxstatus_t)
#endif
      )
    {
      pstatus_t pstat;

      memcpy (&pstat, note->descdata, sizeof (pstat));

      elf_tdata (abfd)->core->pid = pstat.pr_pid;
    }
#if defined (HAVE_PSTATUS32_T)
  else if (note->descsz == sizeof (pstatus32_t))
    {
      /* 64-bit host, 32-bit corefile */
      pstatus32_t pstat;

      memcpy (&pstat, note->descdata, sizeof (pstat));

      elf_tdata (abfd)->core->pid = pstat.pr_pid;
    }
#endif
  /* Could grab some more details from the "representative"
     lwpstatus_t in pstat.pr_lwp, but we'll catch it all in an
     NT_LWPSTATUS note, presumably.  */

  return TRUE;
}
#endif /* defined (HAVE_PSTATUS_T) */

#if defined (HAVE_LWPSTATUS_T)
static bfd_boolean
elfcore_grok_lwpstatus (bfd *abfd, Elf_Internal_Note *note)
{
  lwpstatus_t lwpstat;
  char buf[100];
  char *name;
  size_t len;
  asection *sect;

  if (note->descsz != sizeof (lwpstat)
#if defined (HAVE_LWPXSTATUS_T)
      && note->descsz != sizeof (lwpxstatus_t)
#endif
      )
    return TRUE;

  memcpy (&lwpstat, note->descdata, sizeof (lwpstat));

  elf_tdata (abfd)->core->lwpid = lwpstat.pr_lwpid;
  /* Do not overwrite the core signal if it has already been set by
     another thread.  */
  if (elf_tdata (abfd)->core->signal == 0)
    elf_tdata (abfd)->core->signal = lwpstat.pr_cursig;

  /* Make a ".reg/999" section.  */

  sprintf (buf, ".reg/%d", elfcore_make_pid (abfd));
  len = strlen (buf) + 1;
  name = bfd_alloc (abfd, len);
  if (name == NULL)
    return FALSE;
  memcpy (name, buf, len);

  sect = bfd_make_section_anyway_with_flags (abfd, name, SEC_HAS_CONTENTS);
  if (sect == NULL)
    return FALSE;

#if defined (HAVE_LWPSTATUS_T_PR_CONTEXT)
  sect->size = sizeof (lwpstat.pr_context.uc_mcontext.gregs);
  sect->filepos = note->descpos
    + offsetof (lwpstatus_t, pr_context.uc_mcontext.gregs);
#endif

#if defined (HAVE_LWPSTATUS_T_PR_REG)
  sect->size = sizeof (lwpstat.pr_reg);
  sect->filepos = note->descpos + offsetof (lwpstatus_t, pr_reg);
#endif

  sect->alignment_power = 2;

  if (!elfcore_maybe_make_sect (abfd, ".reg", sect))
    return FALSE;

  /* Make a ".reg2/999" section */

  sprintf (buf, ".reg2/%d", elfcore_make_pid (abfd));
  len = strlen (buf) + 1;
  name = bfd_alloc (abfd, len);
  if (name == NULL)
    return FALSE;
  memcpy (name, buf, len);

  sect = bfd_make_section_anyway_with_flags (abfd, name, SEC_HAS_CONTENTS);
  if (sect == NULL)
    return FALSE;

#if defined (HAVE_LWPSTATUS_T_PR_CONTEXT)
  sect->size = sizeof (lwpstat.pr_context.uc_mcontext.fpregs);
  sect->filepos = note->descpos
    + offsetof (lwpstatus_t, pr_context.uc_mcontext.fpregs);
#endif

#if defined (HAVE_LWPSTATUS_T_PR_FPREG)
  sect->size = sizeof (lwpstat.pr_fpreg);
  sect->filepos = note->descpos + offsetof (lwpstatus_t, pr_fpreg);
#endif

  sect->alignment_power = 2;

  return elfcore_maybe_make_sect (abfd, ".reg2", sect);
}
#endif /* defined (HAVE_LWPSTATUS_T) */

static bfd_boolean
elfcore_grok_win32pstatus (bfd *abfd, Elf_Internal_Note *note)
{
  char buf[30];
  char *name;
  size_t len;
  asection *sect;
  int type;
  int is_active_thread;
  bfd_vma base_addr;

  if (note->descsz < 728)
    return TRUE;

  if (! CONST_STRNEQ (note->namedata, "win32"))
    return TRUE;

  type = bfd_get_32 (abfd, note->descdata);

  switch (type)
    {
    case 1 /* NOTE_INFO_PROCESS */:
      /* FIXME: need to add ->core->command.  */
      /* process_info.pid */
      elf_tdata (abfd)->core->pid = bfd_get_32 (abfd, note->descdata + 8);
      /* process_info.signal */
      elf_tdata (abfd)->core->signal = bfd_get_32 (abfd, note->descdata + 12);
      break;

    case 2 /* NOTE_INFO_THREAD */:
      /* Make a ".reg/999" section.  */
      /* thread_info.tid */
      sprintf (buf, ".reg/%ld", (long) bfd_get_32 (abfd, note->descdata + 8));

      len = strlen (buf) + 1;
      name = (char *) bfd_alloc (abfd, len);
      if (name == NULL)
	return FALSE;

      memcpy (name, buf, len);

      sect = bfd_make_section_anyway_with_flags (abfd, name, SEC_HAS_CONTENTS);
      if (sect == NULL)
	return FALSE;

      /* sizeof (thread_info.thread_context) */
      sect->size = 716;
      /* offsetof (thread_info.thread_context) */
      sect->filepos = note->descpos + 12;
      sect->alignment_power = 2;

      /* thread_info.is_active_thread */
      is_active_thread = bfd_get_32 (abfd, note->descdata + 8);

      if (is_active_thread)
	if (! elfcore_maybe_make_sect (abfd, ".reg", sect))
	  return FALSE;
      break;

    case 3 /* NOTE_INFO_MODULE */:
      /* Make a ".module/xxxxxxxx" section.  */
      /* module_info.base_address */
      base_addr = bfd_get_32 (abfd, note->descdata + 4);
      sprintf (buf, ".module/%08lx", (unsigned long) base_addr);

      len = strlen (buf) + 1;
      name = (char *) bfd_alloc (abfd, len);
      if (name == NULL)
	return FALSE;

      memcpy (name, buf, len);

      sect = bfd_make_section_anyway_with_flags (abfd, name, SEC_HAS_CONTENTS);

      if (sect == NULL)
	return FALSE;

      sect->size = note->descsz;
      sect->filepos = note->descpos;
      sect->alignment_power = 2;
      break;

    default:
      return TRUE;
    }

  return TRUE;
}

static bfd_boolean
elfcore_grok_note (bfd *abfd, Elf_Internal_Note *note)
{
  const struct elf_backend_data *bed = get_elf_backend_data (abfd);

  switch (note->type)
    {
    default:
      return TRUE;

    case NT_PRSTATUS:
      if (bed->elf_backend_grok_prstatus)
	if ((*bed->elf_backend_grok_prstatus) (abfd, note))
	  return TRUE;
#if defined (HAVE_PRSTATUS_T)
      return elfcore_grok_prstatus (abfd, note);
#else
      return TRUE;
#endif

#if defined (HAVE_PSTATUS_T)
    case NT_PSTATUS:
      return elfcore_grok_pstatus (abfd, note);
#endif

#if defined (HAVE_LWPSTATUS_T)
    case NT_LWPSTATUS:
      return elfcore_grok_lwpstatus (abfd, note);
#endif

    case NT_FPREGSET:		/* FIXME: rename to NT_PRFPREG */
      return elfcore_grok_prfpreg (abfd, note);

    case NT_WIN32PSTATUS:
      return elfcore_grok_win32pstatus (abfd, note);

    case NT_PRXFPREG:		/* Linux SSE extension */
      if (note->namesz == 6
	  && strcmp (note->namedata, "LINUX") == 0)
	return elfcore_grok_prxfpreg (abfd, note);
      else
	return TRUE;

    case NT_X86_XSTATE:		/* Linux XSAVE extension */
      if (note->namesz == 6
	  && strcmp (note->namedata, "LINUX") == 0)
	return elfcore_grok_xstatereg (abfd, note);
      else
	return TRUE;

    case NT_PPC_VMX:
      if (note->namesz == 6
	  && strcmp (note->namedata, "LINUX") == 0)
	return elfcore_grok_ppc_vmx (abfd, note);
      else
	return TRUE;

    case NT_PPC_VSX:
      if (note->namesz == 6
	  && strcmp (note->namedata, "LINUX") == 0)
	return elfcore_grok_ppc_vsx (abfd, note);
      else
	return TRUE;

    case NT_PPC_TAR:
      if (note->namesz == 6
          && strcmp (note->namedata, "LINUX") == 0)
        return elfcore_grok_ppc_tar (abfd, note);
      else
        return TRUE;

    case NT_PPC_PPR:
      if (note->namesz == 6
          && strcmp (note->namedata, "LINUX") == 0)
        return elfcore_grok_ppc_ppr (abfd, note);
      else
        return TRUE;

    case NT_PPC_DSCR:
      if (note->namesz == 6
          && strcmp (note->namedata, "LINUX") == 0)
        return elfcore_grok_ppc_dscr (abfd, note);
      else
        return TRUE;

    case NT_PPC_EBB:
      if (note->namesz == 6
          && strcmp (note->namedata, "LINUX") == 0)
        return elfcore_grok_ppc_ebb (abfd, note);
      else
        return TRUE;

    case NT_PPC_PMU:
      if (note->namesz == 6
          && strcmp (note->namedata, "LINUX") == 0)
        return elfcore_grok_ppc_pmu (abfd, note);
      else
        return TRUE;

    case NT_PPC_TM_CGPR:
      if (note->namesz == 6
          && strcmp (note->namedata, "LINUX") == 0)
        return elfcore_grok_ppc_tm_cgpr (abfd, note);
      else
        return TRUE;

    case NT_PPC_TM_CFPR:
      if (note->namesz == 6
          && strcmp (note->namedata, "LINUX") == 0)
        return elfcore_grok_ppc_tm_cfpr (abfd, note);
      else
        return TRUE;

    case NT_PPC_TM_CVMX:
      if (note->namesz == 6
          && strcmp (note->namedata, "LINUX") == 0)
        return elfcore_grok_ppc_tm_cvmx (abfd, note);
      else
        return TRUE;

    case NT_PPC_TM_CVSX:
      if (note->namesz == 6
          && strcmp (note->namedata, "LINUX") == 0)
        return elfcore_grok_ppc_tm_cvsx (abfd, note);
      else
        return TRUE;

    case NT_PPC_TM_SPR:
      if (note->namesz == 6
          && strcmp (note->namedata, "LINUX") == 0)
        return elfcore_grok_ppc_tm_spr (abfd, note);
      else
        return TRUE;

    case NT_PPC_TM_CTAR:
      if (note->namesz == 6
          && strcmp (note->namedata, "LINUX") == 0)
        return elfcore_grok_ppc_tm_ctar (abfd, note);
      else
        return TRUE;

    case NT_PPC_TM_CPPR:
      if (note->namesz == 6
          && strcmp (note->namedata, "LINUX") == 0)
        return elfcore_grok_ppc_tm_cppr (abfd, note);
      else
        return TRUE;

    case NT_PPC_TM_CDSCR:
      if (note->namesz == 6
          && strcmp (note->namedata, "LINUX") == 0)
        return elfcore_grok_ppc_tm_cdscr (abfd, note);
      else
        return TRUE;

    case NT_S390_HIGH_GPRS:
      if (note->namesz == 6
	  && strcmp (note->namedata, "LINUX") == 0)
	return elfcore_grok_s390_high_gprs (abfd, note);
      else
	return TRUE;

    case NT_S390_TIMER:
      if (note->namesz == 6
	  && strcmp (note->namedata, "LINUX") == 0)
	return elfcore_grok_s390_timer (abfd, note);
      else
	return TRUE;

    case NT_S390_TODCMP:
      if (note->namesz == 6
	  && strcmp (note->namedata, "LINUX") == 0)
	return elfcore_grok_s390_todcmp (abfd, note);
      else
	return TRUE;

    case NT_S390_TODPREG:
      if (note->namesz == 6
	  && strcmp (note->namedata, "LINUX") == 0)
	return elfcore_grok_s390_todpreg (abfd, note);
      else
	return TRUE;

    case NT_S390_CTRS:
      if (note->namesz == 6
	  && strcmp (note->namedata, "LINUX") == 0)
	return elfcore_grok_s390_ctrs (abfd, note);
      else
	return TRUE;

    case NT_S390_PREFIX:
      if (note->namesz == 6
	  && strcmp (note->namedata, "LINUX") == 0)
	return elfcore_grok_s390_prefix (abfd, note);
      else
	return TRUE;

    case NT_S390_LAST_BREAK:
      if (note->namesz == 6
	  && strcmp (note->namedata, "LINUX") == 0)
	return elfcore_grok_s390_last_break (abfd, note);
      else
	return TRUE;

    case NT_S390_SYSTEM_CALL:
      if (note->namesz == 6
	  && strcmp (note->namedata, "LINUX") == 0)
	return elfcore_grok_s390_system_call (abfd, note);
      else
	return TRUE;

    case NT_S390_TDB:
      if (note->namesz == 6
	  && strcmp (note->namedata, "LINUX") == 0)
	return elfcore_grok_s390_tdb (abfd, note);
      else
	return TRUE;

    case NT_S390_VXRS_LOW:
      if (note->namesz == 6
	  && strcmp (note->namedata, "LINUX") == 0)
	return elfcore_grok_s390_vxrs_low (abfd, note);
      else
	return TRUE;

    case NT_S390_VXRS_HIGH:
      if (note->namesz == 6
	  && strcmp (note->namedata, "LINUX") == 0)
	return elfcore_grok_s390_vxrs_high (abfd, note);
      else
	return TRUE;

    case NT_S390_GS_CB:
      if (note->namesz == 6
	  && strcmp (note->namedata, "LINUX") == 0)
	return elfcore_grok_s390_gs_cb (abfd, note);
      else
	return TRUE;

    case NT_S390_GS_BC:
      if (note->namesz == 6
	  && strcmp (note->namedata, "LINUX") == 0)
	return elfcore_grok_s390_gs_bc (abfd, note);
      else
	return TRUE;

    case NT_ARM_VFP:
      if (note->namesz == 6
	  && strcmp (note->namedata, "LINUX") == 0)
	return elfcore_grok_arm_vfp (abfd, note);
      else
	return TRUE;

    case NT_ARM_TLS:
      if (note->namesz == 6
	  && strcmp (note->namedata, "LINUX") == 0)
	return elfcore_grok_aarch_tls (abfd, note);
      else
	return TRUE;

    case NT_ARM_HW_BREAK:
      if (note->namesz == 6
	  && strcmp (note->namedata, "LINUX") == 0)
	return elfcore_grok_aarch_hw_break (abfd, note);
      else
	return TRUE;

    case NT_ARM_HW_WATCH:
      if (note->namesz == 6
	  && strcmp (note->namedata, "LINUX") == 0)
	return elfcore_grok_aarch_hw_watch (abfd, note);
      else
	return TRUE;

    case NT_ARM_SVE:
      if (note->namesz == 6
	  && strcmp (note->namedata, "LINUX") == 0)
	return elfcore_grok_aarch_sve (abfd, note);
      else
	return TRUE;

    case NT_PRPSINFO:
    case NT_PSINFO:
      if (bed->elf_backend_grok_psinfo)
	if ((*bed->elf_backend_grok_psinfo) (abfd, note))
	  return TRUE;
#if defined (HAVE_PRPSINFO_T) || defined (HAVE_PSINFO_T)
      return elfcore_grok_psinfo (abfd, note);
#else
      return TRUE;
#endif

    case NT_AUXV:
      {
	asection *sect = bfd_make_section_anyway_with_flags (abfd, ".auxv",
							     SEC_HAS_CONTENTS);

	if (sect == NULL)
	  return FALSE;
	sect->size = note->descsz;
	sect->filepos = note->descpos;
	sect->alignment_power = 1 + bfd_get_arch_size (abfd) / 32;

	return TRUE;
      }

    case NT_FILE:
      return elfcore_make_note_pseudosection (abfd, ".note.linuxcore.file",
					      note);

    case NT_SIGINFO:
      return elfcore_make_note_pseudosection (abfd, ".note.linuxcore.siginfo",
					      note);

    }
}

static bfd_boolean
elfobj_grok_gnu_build_id (bfd *abfd, Elf_Internal_Note *note)
{
  struct bfd_build_id* build_id;

  if (note->descsz == 0)
    return FALSE;

  build_id = bfd_alloc (abfd, sizeof (struct bfd_build_id) - 1 + note->descsz);
  if (build_id == NULL)
    return FALSE;

  build_id->size = note->descsz;
  memcpy (build_id->data, note->descdata, note->descsz);
  abfd->build_id = build_id;

  return TRUE;
}

static bfd_boolean
elfobj_grok_gnu_note (bfd *abfd, Elf_Internal_Note *note)
{
  switch (note->type)
    {
    default:
      return TRUE;

    case NT_GNU_PROPERTY_TYPE_0:
      return _bfd_elf_parse_gnu_properties (abfd, note);

    case NT_GNU_BUILD_ID:
      return elfobj_grok_gnu_build_id (abfd, note);
    }
}

static bfd_boolean
elfobj_grok_stapsdt_note_1 (bfd *abfd, Elf_Internal_Note *note)
{
  struct sdt_note *cur =
    (struct sdt_note *) bfd_alloc (abfd, sizeof (struct sdt_note)
				   + note->descsz);

  cur->next = (struct sdt_note *) (elf_tdata (abfd))->sdt_note_head;
  cur->size = (bfd_size_type) note->descsz;
  memcpy (cur->data, note->descdata, note->descsz);

  elf_tdata (abfd)->sdt_note_head = cur;

  return TRUE;
}

static bfd_boolean
elfobj_grok_stapsdt_note (bfd *abfd, Elf_Internal_Note *note)
{
  switch (note->type)
    {
    case NT_STAPSDT:
      return elfobj_grok_stapsdt_note_1 (abfd, note);

    default:
      return TRUE;
    }
}

static bfd_boolean
elfcore_grok_freebsd_psinfo (bfd *abfd, Elf_Internal_Note *note)
{
  size_t offset;

  switch (elf_elfheader (abfd)->e_ident[EI_CLASS])
    {
    case ELFCLASS32:
      if (note->descsz < 108)
	return FALSE;
      break;

    case ELFCLASS64:
      if (note->descsz < 120)
	return FALSE;
      break;

    default:
      return FALSE;
    }

  /* Check for version 1 in pr_version.  */
  if (bfd_h_get_32 (abfd, (bfd_byte *) note->descdata) != 1)
    return FALSE;

  offset = 4;

  /* Skip over pr_psinfosz. */
  if (elf_elfheader (abfd)->e_ident[EI_CLASS] == ELFCLASS32)
    offset += 4;
  else
    {
      offset += 4;	/* Padding before pr_psinfosz. */
      offset += 8;
    }

  /* pr_fname is PRFNAMESZ (16) + 1 bytes in size.  */
  elf_tdata (abfd)->core->program
    = _bfd_elfcore_strndup (abfd, note->descdata + offset, 17);
  offset += 17;

  /* pr_psargs is PRARGSZ (80) + 1 bytes in size.  */
  elf_tdata (abfd)->core->command
    = _bfd_elfcore_strndup (abfd, note->descdata + offset, 81);
  offset += 81;

  /* Padding before pr_pid.  */
  offset += 2;

  /* The pr_pid field was added in version "1a".  */
  if (note->descsz < offset + 4)
    return TRUE;

  elf_tdata (abfd)->core->pid
    = bfd_h_get_32 (abfd, (bfd_byte *) note->descdata + offset);

  return TRUE;
}

static bfd_boolean
elfcore_grok_freebsd_prstatus (bfd *abfd, Elf_Internal_Note *note)
{
  size_t offset;
  size_t size;
  size_t min_size;

  /* Compute offset of pr_getregsz, skipping over pr_statussz.
     Also compute minimum size of this note.  */
  switch (elf_elfheader (abfd)->e_ident[EI_CLASS])
    {
    case ELFCLASS32:
      offset = 4 + 4;
      min_size = offset + (4 * 2) + 4 + 4 + 4;
      break;

    case ELFCLASS64:
      offset = 4 + 4 + 8;	/* Includes padding before pr_statussz.  */
      min_size = offset + (8 * 2) + 4 + 4 + 4 + 4;
      break;

    default:
      return FALSE;
    }

  if (note->descsz < min_size)
    return FALSE;

  /* Check for version 1 in pr_version.  */
  if (bfd_h_get_32 (abfd, (bfd_byte *) note->descdata) != 1)
    return FALSE;

  /* Extract size of pr_reg from pr_gregsetsz.  */
  /* Skip over pr_gregsetsz and pr_fpregsetsz.  */
  if (elf_elfheader (abfd)->e_ident[EI_CLASS] == ELFCLASS32)
    {
      size = bfd_h_get_32 (abfd, (bfd_byte *) note->descdata + offset);
      offset += 4 * 2;
    }
  else
    {
      size = bfd_h_get_64 (abfd, (bfd_byte *) note->descdata + offset);
      offset += 8 * 2;
    }

  /* Skip over pr_osreldate.  */
  offset += 4;

  /* Read signal from pr_cursig.  */
  if (elf_tdata (abfd)->core->signal == 0)
    elf_tdata (abfd)->core->signal
      = bfd_h_get_32 (abfd, (bfd_byte *) note->descdata + offset);
  offset += 4;

  /* Read TID from pr_pid.  */
  elf_tdata (abfd)->core->lwpid
      = bfd_h_get_32 (abfd, (bfd_byte *) note->descdata + offset);
  offset += 4;

  /* Padding before pr_reg.  */
  if (elf_elfheader (abfd)->e_ident[EI_CLASS] == ELFCLASS64)
    offset += 4;

  /* Make sure that there is enough data remaining in the note.  */
  if ((note->descsz - offset) < size)
    return FALSE;

  /* Make a ".reg/999" section and a ".reg" section.  */
  return _bfd_elfcore_make_pseudosection (abfd, ".reg",
					  size, note->descpos + offset);
}

static bfd_boolean
elfcore_grok_freebsd_note (bfd *abfd, Elf_Internal_Note *note)
{
  const struct elf_backend_data *bed = get_elf_backend_data (abfd);

  switch (note->type)
    {
    case NT_PRSTATUS:
      if (bed->elf_backend_grok_freebsd_prstatus)
	if ((*bed->elf_backend_grok_freebsd_prstatus) (abfd, note))
	  return TRUE;
      return elfcore_grok_freebsd_prstatus (abfd, note);

    case NT_FPREGSET:
      return elfcore_grok_prfpreg (abfd, note);

    case NT_PRPSINFO:
      return elfcore_grok_freebsd_psinfo (abfd, note);

    case NT_FREEBSD_THRMISC:
      if (note->namesz == 8)
	return elfcore_make_note_pseudosection (abfd, ".thrmisc", note);
      else
	return TRUE;

    case NT_FREEBSD_PROCSTAT_PROC:
      return elfcore_make_note_pseudosection (abfd, ".note.freebsdcore.proc",
					      note);

    case NT_FREEBSD_PROCSTAT_FILES:
      return elfcore_make_note_pseudosection (abfd, ".note.freebsdcore.files",
					      note);

    case NT_FREEBSD_PROCSTAT_VMMAP:
      return elfcore_make_note_pseudosection (abfd, ".note.freebsdcore.vmmap",
					      note);

    case NT_FREEBSD_PROCSTAT_AUXV:
      {
	asection *sect = bfd_make_section_anyway_with_flags (abfd, ".auxv",
							     SEC_HAS_CONTENTS);

	if (sect == NULL)
	  return FALSE;
	sect->size = note->descsz - 4;
	sect->filepos = note->descpos + 4;
	sect->alignment_power = 1 + bfd_get_arch_size (abfd) / 32;

	return TRUE;
      }

    case NT_X86_XSTATE:
      if (note->namesz == 8)
	return elfcore_grok_xstatereg (abfd, note);
      else
	return TRUE;

    case NT_FREEBSD_PTLWPINFO:
      return elfcore_make_note_pseudosection (abfd, ".note.freebsdcore.lwpinfo",
					      note);

    case NT_ARM_VFP:
      return elfcore_grok_arm_vfp (abfd, note);

    default:
      return TRUE;
    }
}

static bfd_boolean
elfcore_netbsd_get_lwpid (Elf_Internal_Note *note, int *lwpidp)
{
  char *cp;

  cp = strchr (note->namedata, '@');
  if (cp != NULL)
    {
      *lwpidp = atoi(cp + 1);
      return TRUE;
    }
  return FALSE;
}

static bfd_boolean
elfcore_grok_netbsd_procinfo (bfd *abfd, Elf_Internal_Note *note)
{
  if (note->descsz <= 0x7c + 31)
    return FALSE;

  /* Signal number at offset 0x08. */
  elf_tdata (abfd)->core->signal
    = bfd_h_get_32 (abfd, (bfd_byte *) note->descdata + 0x08);

  /* Process ID at offset 0x50. */
  elf_tdata (abfd)->core->pid
    = bfd_h_get_32 (abfd, (bfd_byte *) note->descdata + 0x50);

  /* Command name at 0x7c (max 32 bytes, including nul). */
  elf_tdata (abfd)->core->command
    = _bfd_elfcore_strndup (abfd, note->descdata + 0x7c, 31);

  return elfcore_make_note_pseudosection (abfd, ".note.netbsdcore.procinfo",
					  note);
}

static bfd_boolean
elfcore_grok_netbsd_note (bfd *abfd, Elf_Internal_Note *note)
{
  int lwp;

  if (elfcore_netbsd_get_lwpid (note, &lwp))
    elf_tdata (abfd)->core->lwpid = lwp;

  if (note->type == NT_NETBSDCORE_PROCINFO)
    {
      /* NetBSD-specific core "procinfo".  Note that we expect to
	 find this note before any of the others, which is fine,
	 since the kernel writes this note out first when it
	 creates a core file.  */

      return elfcore_grok_netbsd_procinfo (abfd, note);
    }

  /* As of Jan 2002 there are no other machine-independent notes
     defined for NetBSD core files.  If the note type is less
     than the start of the machine-dependent note types, we don't
     understand it.  */

  if (note->type < NT_NETBSDCORE_FIRSTMACH)
    return TRUE;


  switch (bfd_get_arch (abfd))
    {
      /* On the Alpha, SPARC (32-bit and 64-bit), PT_GETREGS == mach+0 and
	 PT_GETFPREGS == mach+2.  */

    case bfd_arch_alpha:
    case bfd_arch_sparc:
      switch (note->type)
	{
	case NT_NETBSDCORE_FIRSTMACH+0:
	  return elfcore_make_note_pseudosection (abfd, ".reg", note);

	case NT_NETBSDCORE_FIRSTMACH+2:
	  return elfcore_make_note_pseudosection (abfd, ".reg2", note);

	default:
	  return TRUE;
	}

      /* On all other arch's, PT_GETREGS == mach+1 and
	 PT_GETFPREGS == mach+3.  */

    default:
      switch (note->type)
	{
	case NT_NETBSDCORE_FIRSTMACH+1:
	  return elfcore_make_note_pseudosection (abfd, ".reg", note);

	case NT_NETBSDCORE_FIRSTMACH+3:
	  return elfcore_make_note_pseudosection (abfd, ".reg2", note);

	default:
	  return TRUE;
	}
    }
    /* NOTREACHED */
}

static bfd_boolean
elfcore_grok_openbsd_procinfo (bfd *abfd, Elf_Internal_Note *note)
{
  if (note->descsz <= 0x48 + 31)
    return FALSE;

  /* Signal number at offset 0x08. */
  elf_tdata (abfd)->core->signal
    = bfd_h_get_32 (abfd, (bfd_byte *) note->descdata + 0x08);

  /* Process ID at offset 0x20. */
  elf_tdata (abfd)->core->pid
    = bfd_h_get_32 (abfd, (bfd_byte *) note->descdata + 0x20);

  /* Command name at 0x48 (max 32 bytes, including nul). */
  elf_tdata (abfd)->core->command
    = _bfd_elfcore_strndup (abfd, note->descdata + 0x48, 31);

  return TRUE;
}

static bfd_boolean
elfcore_grok_openbsd_note (bfd *abfd, Elf_Internal_Note *note)
{
  if (note->type == NT_OPENBSD_PROCINFO)
    return elfcore_grok_openbsd_procinfo (abfd, note);

  if (note->type == NT_OPENBSD_REGS)
    return elfcore_make_note_pseudosection (abfd, ".reg", note);

  if (note->type == NT_OPENBSD_FPREGS)
    return elfcore_make_note_pseudosection (abfd, ".reg2", note);

  if (note->type == NT_OPENBSD_XFPREGS)
    return elfcore_make_note_pseudosection (abfd, ".reg-xfp", note);

  if (note->type == NT_OPENBSD_AUXV)
    {
      asection *sect = bfd_make_section_anyway_with_flags (abfd, ".auxv",
							   SEC_HAS_CONTENTS);

      if (sect == NULL)
	return FALSE;
      sect->size = note->descsz;
      sect->filepos = note->descpos;
      sect->alignment_power = 1 + bfd_get_arch_size (abfd) / 32;

      return TRUE;
    }

  if (note->type == NT_OPENBSD_WCOOKIE)
    {
      asection *sect = bfd_make_section_anyway_with_flags (abfd, ".wcookie",
							   SEC_HAS_CONTENTS);

      if (sect == NULL)
	return FALSE;
      sect->size = note->descsz;
      sect->filepos = note->descpos;
      sect->alignment_power = 1 + bfd_get_arch_size (abfd) / 32;

      return TRUE;
    }

  return TRUE;
}

static bfd_boolean
elfcore_grok_nto_status (bfd *abfd, Elf_Internal_Note *note, long *tid)
{
  void *ddata = note->descdata;
  char buf[100];
  char *name;
  asection *sect;
  short sig;
  unsigned flags;

  if (note->descsz < 16)
    return FALSE;

  /* nto_procfs_status 'pid' field is at offset 0.  */
  elf_tdata (abfd)->core->pid = bfd_get_32 (abfd, (bfd_byte *) ddata);

  /* nto_procfs_status 'tid' field is at offset 4.  Pass it back.  */
  *tid = bfd_get_32 (abfd, (bfd_byte *) ddata + 4);

  /* nto_procfs_status 'flags' field is at offset 8.  */
  flags = bfd_get_32 (abfd, (bfd_byte *) ddata + 8);

  /* nto_procfs_status 'what' field is at offset 14.  */
  if ((sig = bfd_get_16 (abfd, (bfd_byte *) ddata + 14)) > 0)
    {
      elf_tdata (abfd)->core->signal = sig;
      elf_tdata (abfd)->core->lwpid = *tid;
    }

  /* _DEBUG_FLAG_CURTID (current thread) is 0x80.  Some cores
     do not come from signals so we make sure we set the current
     thread just in case.  */
  if (flags & 0x00000080)
    elf_tdata (abfd)->core->lwpid = *tid;

  /* Make a ".qnx_core_status/%d" section.  */
  sprintf (buf, ".qnx_core_status/%ld", *tid);

  name = (char *) bfd_alloc (abfd, strlen (buf) + 1);
  if (name == NULL)
    return FALSE;
  strcpy (name, buf);

  sect = bfd_make_section_anyway_with_flags (abfd, name, SEC_HAS_CONTENTS);
  if (sect == NULL)
    return FALSE;

  sect->size		= note->descsz;
  sect->filepos		= note->descpos;
  sect->alignment_power = 2;

  return (elfcore_maybe_make_sect (abfd, ".qnx_core_status", sect));
}

static bfd_boolean
elfcore_grok_nto_regs (bfd *abfd,
		       Elf_Internal_Note *note,
		       long tid,
		       char *base)
{
  char buf[100];
  char *name;
  asection *sect;

  /* Make a "(base)/%d" section.  */
  sprintf (buf, "%s/%ld", base, tid);

  name = (char *) bfd_alloc (abfd, strlen (buf) + 1);
  if (name == NULL)
    return FALSE;
  strcpy (name, buf);

  sect = bfd_make_section_anyway_with_flags (abfd, name, SEC_HAS_CONTENTS);
  if (sect == NULL)
    return FALSE;

  sect->size		= note->descsz;
  sect->filepos		= note->descpos;
  sect->alignment_power = 2;

  /* This is the current thread.  */
  if (elf_tdata (abfd)->core->lwpid == tid)
    return elfcore_maybe_make_sect (abfd, base, sect);

  return TRUE;
}

#define BFD_QNT_CORE_INFO	7
#define BFD_QNT_CORE_STATUS	8
#define BFD_QNT_CORE_GREG	9
#define BFD_QNT_CORE_FPREG	10

static bfd_boolean
elfcore_grok_nto_note (bfd *abfd, Elf_Internal_Note *note)
{
  /* Every GREG section has a STATUS section before it.  Store the
     tid from the previous call to pass down to the next gregs
     function.  */
  static long tid = 1;

  switch (note->type)
    {
    case BFD_QNT_CORE_INFO:
      return elfcore_make_note_pseudosection (abfd, ".qnx_core_info", note);
    case BFD_QNT_CORE_STATUS:
      return elfcore_grok_nto_status (abfd, note, &tid);
    case BFD_QNT_CORE_GREG:
      return elfcore_grok_nto_regs (abfd, note, tid, ".reg");
    case BFD_QNT_CORE_FPREG:
      return elfcore_grok_nto_regs (abfd, note, tid, ".reg2");
    default:
      return TRUE;
    }
}

static bfd_boolean
elfcore_grok_spu_note (bfd *abfd, Elf_Internal_Note *note)
{
  char *name;
  asection *sect;
  size_t len;

  /* Use note name as section name.  */
  len = note->namesz;
  name = (char *) bfd_alloc (abfd, len);
  if (name == NULL)
    return FALSE;
  memcpy (name, note->namedata, len);
  name[len - 1] = '\0';

  sect = bfd_make_section_anyway_with_flags (abfd, name, SEC_HAS_CONTENTS);
  if (sect == NULL)
    return FALSE;

  sect->size		= note->descsz;
  sect->filepos		= note->descpos;
  sect->alignment_power = 1;

  return TRUE;
}

/* Function: elfcore_write_note

   Inputs:
     buffer to hold note, and current size of buffer
     name of note
     type of note
     data for note
     size of data for note

   Writes note to end of buffer.  ELF64 notes are written exactly as
   for ELF32, despite the current (as of 2006) ELF gabi specifying
   that they ought to have 8-byte namesz and descsz field, and have
   8-byte alignment.  Other writers, eg. Linux kernel, do the same.

   Return:
   Pointer to realloc'd buffer, *BUFSIZ updated.  */

char *
elfcore_write_note (bfd *abfd,
		    char *buf,
		    int *bufsiz,
		    const char *name,
		    int type,
		    const void *input,
		    int size)
{
  Elf_External_Note *xnp;
  size_t namesz;
  size_t newspace;
  char *dest;

  namesz = 0;
  if (name != NULL)
    namesz = strlen (name) + 1;

  newspace = 12 + ((namesz + 3) & -4) + ((size + 3) & -4);

  buf = (char *) realloc (buf, *bufsiz + newspace);
  if (buf == NULL)
    return buf;
  dest = buf + *bufsiz;
  *bufsiz += newspace;
  xnp = (Elf_External_Note *) dest;
  H_PUT_32 (abfd, namesz, xnp->namesz);
  H_PUT_32 (abfd, size, xnp->descsz);
  H_PUT_32 (abfd, type, xnp->type);
  dest = xnp->name;
  if (name != NULL)
    {
      memcpy (dest, name, namesz);
      dest += namesz;
      while (namesz & 3)
	{
	  *dest++ = '\0';
	  ++namesz;
	}
    }
  memcpy (dest, input, size);
  dest += size;
  while (size & 3)
    {
      *dest++ = '\0';
      ++size;
    }
  return buf;
}

/* gcc-8 warns (*) on all the strncpy calls in this function about
   possible string truncation.  The "truncation" is not a bug.  We
   have an external representation of structs with fields that are not
   necessarily NULL terminated and corresponding internal
   representation fields that are one larger so that they can always
   be NULL terminated.
   gcc versions between 4.2 and 4.6 do not allow pragma control of
   diagnostics inside functions, giving a hard error if you try to use
   the finer control available with later versions.
   gcc prior to 4.2 warns about diagnostic push and pop.
   gcc-5, gcc-6 and gcc-7 warn that -Wstringop-truncation is unknown,
   unless you also add #pragma GCC diagnostic ignored "-Wpragma".
   (*) Depending on your system header files!  */
#if GCC_VERSION >= 8000
# pragma GCC diagnostic push
# pragma GCC diagnostic ignored "-Wstringop-truncation"
#endif
char *
elfcore_write_prpsinfo (bfd  *abfd,
			char *buf,
			int  *bufsiz,
			const char *fname,
			const char *psargs)
{
  const struct elf_backend_data *bed = get_elf_backend_data (abfd);

  if (bed->elf_backend_write_core_note != NULL)
    {
      char *ret;
      ret = (*bed->elf_backend_write_core_note) (abfd, buf, bufsiz,
						 NT_PRPSINFO, fname, psargs);
      if (ret != NULL)
	return ret;
    }

#if defined (HAVE_PRPSINFO_T) || defined (HAVE_PSINFO_T)
# if defined (HAVE_PRPSINFO32_T) || defined (HAVE_PSINFO32_T)
  if (bed->s->elfclass == ELFCLASS32)
    {
#  if defined (HAVE_PSINFO32_T)
      psinfo32_t data;
      int note_type = NT_PSINFO;
#  else
      prpsinfo32_t data;
      int note_type = NT_PRPSINFO;
#  endif

      memset (&data, 0, sizeof (data));
      strncpy (data.pr_fname, fname, sizeof (data.pr_fname));
      strncpy (data.pr_psargs, psargs, sizeof (data.pr_psargs));
      return elfcore_write_note (abfd, buf, bufsiz,
				 "CORE", note_type, &data, sizeof (data));
    }
  else
# endif
    {
# if defined (HAVE_PSINFO_T)
      psinfo_t data;
      int note_type = NT_PSINFO;
# else
      prpsinfo_t data;
      int note_type = NT_PRPSINFO;
# endif

      memset (&data, 0, sizeof (data));
      strncpy (data.pr_fname, fname, sizeof (data.pr_fname));
      strncpy (data.pr_psargs, psargs, sizeof (data.pr_psargs));
      return elfcore_write_note (abfd, buf, bufsiz,
				 "CORE", note_type, &data, sizeof (data));
    }
#endif	/* PSINFO_T or PRPSINFO_T */

  free (buf);
  return NULL;
}
#if GCC_VERSION >= 8000
# pragma GCC diagnostic pop
#endif

char *
elfcore_write_linux_prpsinfo32
  (bfd *abfd, char *buf, int *bufsiz,
   const struct elf_internal_linux_prpsinfo *prpsinfo)
{
  if (get_elf_backend_data (abfd)->linux_prpsinfo32_ugid16)
    {
      struct elf_external_linux_prpsinfo32_ugid16 data;

      swap_linux_prpsinfo32_ugid16_out (abfd, prpsinfo, &data);
      return elfcore_write_note (abfd, buf, bufsiz, "CORE", NT_PRPSINFO,
				 &data, sizeof (data));
    }
  else
    {
      struct elf_external_linux_prpsinfo32_ugid32 data;

      swap_linux_prpsinfo32_ugid32_out (abfd, prpsinfo, &data);
      return elfcore_write_note (abfd, buf, bufsiz, "CORE", NT_PRPSINFO,
				 &data, sizeof (data));
    }
}

char *
elfcore_write_linux_prpsinfo64
  (bfd *abfd, char *buf, int *bufsiz,
   const struct elf_internal_linux_prpsinfo *prpsinfo)
{
  if (get_elf_backend_data (abfd)->linux_prpsinfo64_ugid16)
    {
      struct elf_external_linux_prpsinfo64_ugid16 data;

      swap_linux_prpsinfo64_ugid16_out (abfd, prpsinfo, &data);
      return elfcore_write_note (abfd, buf, bufsiz,
				 "CORE", NT_PRPSINFO, &data, sizeof (data));
    }
  else
    {
      struct elf_external_linux_prpsinfo64_ugid32 data;

      swap_linux_prpsinfo64_ugid32_out (abfd, prpsinfo, &data);
      return elfcore_write_note (abfd, buf, bufsiz,
				 "CORE", NT_PRPSINFO, &data, sizeof (data));
    }
}

char *
elfcore_write_prstatus (bfd *abfd,
			char *buf,
			int *bufsiz,
			long pid,
			int cursig,
			const void *gregs)
{
  const struct elf_backend_data *bed = get_elf_backend_data (abfd);

  if (bed->elf_backend_write_core_note != NULL)
    {
      char *ret;
      ret = (*bed->elf_backend_write_core_note) (abfd, buf, bufsiz,
						 NT_PRSTATUS,
						 pid, cursig, gregs);
      if (ret != NULL)
	return ret;
    }

#if defined (HAVE_PRSTATUS_T)
#if defined (HAVE_PRSTATUS32_T)
  if (bed->s->elfclass == ELFCLASS32)
    {
      prstatus32_t prstat;

      memset (&prstat, 0, sizeof (prstat));
      prstat.pr_pid = pid;
      prstat.pr_cursig = cursig;
      memcpy (&prstat.pr_reg, gregs, sizeof (prstat.pr_reg));
      return elfcore_write_note (abfd, buf, bufsiz, "CORE",
				 NT_PRSTATUS, &prstat, sizeof (prstat));
    }
  else
#endif
    {
      prstatus_t prstat;

      memset (&prstat, 0, sizeof (prstat));
      prstat.pr_pid = pid;
      prstat.pr_cursig = cursig;
      memcpy (&prstat.pr_reg, gregs, sizeof (prstat.pr_reg));
      return elfcore_write_note (abfd, buf, bufsiz, "CORE",
				 NT_PRSTATUS, &prstat, sizeof (prstat));
    }
#endif /* HAVE_PRSTATUS_T */

  free (buf);
  return NULL;
}

#if defined (HAVE_LWPSTATUS_T)
char *
elfcore_write_lwpstatus (bfd *abfd,
			 char *buf,
			 int *bufsiz,
			 long pid,
			 int cursig,
			 const void *gregs)
{
  lwpstatus_t lwpstat;
  const char *note_name = "CORE";

  memset (&lwpstat, 0, sizeof (lwpstat));
  lwpstat.pr_lwpid  = pid >> 16;
  lwpstat.pr_cursig = cursig;
#if defined (HAVE_LWPSTATUS_T_PR_REG)
  memcpy (&lwpstat.pr_reg, gregs, sizeof (lwpstat.pr_reg));
#elif defined (HAVE_LWPSTATUS_T_PR_CONTEXT)
#if !defined(gregs)
  memcpy (lwpstat.pr_context.uc_mcontext.gregs,
	  gregs, sizeof (lwpstat.pr_context.uc_mcontext.gregs));
#else
  memcpy (lwpstat.pr_context.uc_mcontext.__gregs,
	  gregs, sizeof (lwpstat.pr_context.uc_mcontext.__gregs));
#endif
#endif
  return elfcore_write_note (abfd, buf, bufsiz, note_name,
			     NT_LWPSTATUS, &lwpstat, sizeof (lwpstat));
}
#endif /* HAVE_LWPSTATUS_T */

#if defined (HAVE_PSTATUS_T)
char *
elfcore_write_pstatus (bfd *abfd,
		       char *buf,
		       int *bufsiz,
		       long pid,
		       int cursig ATTRIBUTE_UNUSED,
		       const void *gregs ATTRIBUTE_UNUSED)
{
  const char *note_name = "CORE";
#if defined (HAVE_PSTATUS32_T)
  const struct elf_backend_data *bed = get_elf_backend_data (abfd);

  if (bed->s->elfclass == ELFCLASS32)
    {
      pstatus32_t pstat;

      memset (&pstat, 0, sizeof (pstat));
      pstat.pr_pid = pid & 0xffff;
      buf = elfcore_write_note (abfd, buf, bufsiz, note_name,
				NT_PSTATUS, &pstat, sizeof (pstat));
      return buf;
    }
  else
#endif
    {
      pstatus_t pstat;

      memset (&pstat, 0, sizeof (pstat));
      pstat.pr_pid = pid & 0xffff;
      buf = elfcore_write_note (abfd, buf, bufsiz, note_name,
				NT_PSTATUS, &pstat, sizeof (pstat));
      return buf;
    }
}
#endif /* HAVE_PSTATUS_T */

char *
elfcore_write_prfpreg (bfd *abfd,
		       char *buf,
		       int *bufsiz,
		       const void *fpregs,
		       int size)
{
  const char *note_name = "CORE";
  return elfcore_write_note (abfd, buf, bufsiz,
			     note_name, NT_FPREGSET, fpregs, size);
}

char *
elfcore_write_prxfpreg (bfd *abfd,
			char *buf,
			int *bufsiz,
			const void *xfpregs,
			int size)
{
  char *note_name = "LINUX";
  return elfcore_write_note (abfd, buf, bufsiz,
			     note_name, NT_PRXFPREG, xfpregs, size);
}

char *
elfcore_write_xstatereg (bfd *abfd, char *buf, int *bufsiz,
			 const void *xfpregs, int size)
{
  char *note_name;
  if (get_elf_backend_data (abfd)->elf_osabi == ELFOSABI_FREEBSD)
    note_name = "FreeBSD";
  else
    note_name = "LINUX";
  return elfcore_write_note (abfd, buf, bufsiz,
			     note_name, NT_X86_XSTATE, xfpregs, size);
}

char *
elfcore_write_ppc_vmx (bfd *abfd,
		       char *buf,
		       int *bufsiz,
		       const void *ppc_vmx,
		       int size)
{
  char *note_name = "LINUX";
  return elfcore_write_note (abfd, buf, bufsiz,
			     note_name, NT_PPC_VMX, ppc_vmx, size);
}

char *
elfcore_write_ppc_vsx (bfd *abfd,
		       char *buf,
		       int *bufsiz,
		       const void *ppc_vsx,
		       int size)
{
  char *note_name = "LINUX";
  return elfcore_write_note (abfd, buf, bufsiz,
			     note_name, NT_PPC_VSX, ppc_vsx, size);
}

char *
elfcore_write_ppc_tar (bfd *abfd,
                       char *buf,
                       int *bufsiz,
                       const void *ppc_tar,
                       int size)
{
  char *note_name = "LINUX";
  return elfcore_write_note (abfd, buf, bufsiz,
                             note_name, NT_PPC_TAR, ppc_tar, size);
}

char *
elfcore_write_ppc_ppr (bfd *abfd,
                       char *buf,
                       int *bufsiz,
                       const void *ppc_ppr,
                       int size)
{
  char *note_name = "LINUX";
  return elfcore_write_note (abfd, buf, bufsiz,
                             note_name, NT_PPC_PPR, ppc_ppr, size);
}

char *
elfcore_write_ppc_dscr (bfd *abfd,
                        char *buf,
                        int *bufsiz,
                        const void *ppc_dscr,
                        int size)
{
  char *note_name = "LINUX";
  return elfcore_write_note (abfd, buf, bufsiz,
                             note_name, NT_PPC_DSCR, ppc_dscr, size);
}

char *
elfcore_write_ppc_ebb (bfd *abfd,
                       char *buf,
                       int *bufsiz,
                       const void *ppc_ebb,
                       int size)
{
  char *note_name = "LINUX";
  return elfcore_write_note (abfd, buf, bufsiz,
                             note_name, NT_PPC_EBB, ppc_ebb, size);
}

char *
elfcore_write_ppc_pmu (bfd *abfd,
                       char *buf,
                       int *bufsiz,
                       const void *ppc_pmu,
                       int size)
{
  char *note_name = "LINUX";
  return elfcore_write_note (abfd, buf, bufsiz,
                             note_name, NT_PPC_PMU, ppc_pmu, size);
}

char *
elfcore_write_ppc_tm_cgpr (bfd *abfd,
                           char *buf,
                           int *bufsiz,
                           const void *ppc_tm_cgpr,
                           int size)
{
  char *note_name = "LINUX";
  return elfcore_write_note (abfd, buf, bufsiz,
                             note_name, NT_PPC_TM_CGPR, ppc_tm_cgpr, size);
}

char *
elfcore_write_ppc_tm_cfpr (bfd *abfd,
                           char *buf,
                           int *bufsiz,
                           const void *ppc_tm_cfpr,
                           int size)
{
  char *note_name = "LINUX";
  return elfcore_write_note (abfd, buf, bufsiz,
                             note_name, NT_PPC_TM_CFPR, ppc_tm_cfpr, size);
}

char *
elfcore_write_ppc_tm_cvmx (bfd *abfd,
                           char *buf,
                           int *bufsiz,
                           const void *ppc_tm_cvmx,
                           int size)
{
  char *note_name = "LINUX";
  return elfcore_write_note (abfd, buf, bufsiz,
                             note_name, NT_PPC_TM_CVMX, ppc_tm_cvmx, size);
}

char *
elfcore_write_ppc_tm_cvsx (bfd *abfd,
                           char *buf,
                           int *bufsiz,
                           const void *ppc_tm_cvsx,
                           int size)
{
  char *note_name = "LINUX";
  return elfcore_write_note (abfd, buf, bufsiz,
                             note_name, NT_PPC_TM_CVSX, ppc_tm_cvsx, size);
}

char *
elfcore_write_ppc_tm_spr (bfd *abfd,
                          char *buf,
                          int *bufsiz,
                          const void *ppc_tm_spr,
                          int size)
{
  char *note_name = "LINUX";
  return elfcore_write_note (abfd, buf, bufsiz,
                             note_name, NT_PPC_TM_SPR, ppc_tm_spr, size);
}

char *
elfcore_write_ppc_tm_ctar (bfd *abfd,
                           char *buf,
                           int *bufsiz,
                           const void *ppc_tm_ctar,
                           int size)
{
  char *note_name = "LINUX";
  return elfcore_write_note (abfd, buf, bufsiz,
                             note_name, NT_PPC_TM_CTAR, ppc_tm_ctar, size);
}

char *
elfcore_write_ppc_tm_cppr (bfd *abfd,
                           char *buf,
                           int *bufsiz,
                           const void *ppc_tm_cppr,
                           int size)
{
  char *note_name = "LINUX";
  return elfcore_write_note (abfd, buf, bufsiz,
                             note_name, NT_PPC_TM_CPPR, ppc_tm_cppr, size);
}

char *
elfcore_write_ppc_tm_cdscr (bfd *abfd,
                            char *buf,
                            int *bufsiz,
                            const void *ppc_tm_cdscr,
                            int size)
{
  char *note_name = "LINUX";
  return elfcore_write_note (abfd, buf, bufsiz,
                             note_name, NT_PPC_TM_CDSCR, ppc_tm_cdscr, size);
}

static char *
elfcore_write_s390_high_gprs (bfd *abfd,
			      char *buf,
			      int *bufsiz,
			      const void *s390_high_gprs,
			      int size)
{
  char *note_name = "LINUX";
  return elfcore_write_note (abfd, buf, bufsiz,
			     note_name, NT_S390_HIGH_GPRS,
			     s390_high_gprs, size);
}

char *
elfcore_write_s390_timer (bfd *abfd,
			  char *buf,
			  int *bufsiz,
			  const void *s390_timer,
			  int size)
{
  char *note_name = "LINUX";
  return elfcore_write_note (abfd, buf, bufsiz,
			     note_name, NT_S390_TIMER, s390_timer, size);
}

char *
elfcore_write_s390_todcmp (bfd *abfd,
			   char *buf,
			   int *bufsiz,
			   const void *s390_todcmp,
			   int size)
{
  char *note_name = "LINUX";
  return elfcore_write_note (abfd, buf, bufsiz,
			     note_name, NT_S390_TODCMP, s390_todcmp, size);
}

char *
elfcore_write_s390_todpreg (bfd *abfd,
			    char *buf,
			    int *bufsiz,
			    const void *s390_todpreg,
			    int size)
{
  char *note_name = "LINUX";
  return elfcore_write_note (abfd, buf, bufsiz,
			     note_name, NT_S390_TODPREG, s390_todpreg, size);
}

char *
elfcore_write_s390_ctrs (bfd *abfd,
			 char *buf,
			 int *bufsiz,
			 const void *s390_ctrs,
			 int size)
{
  char *note_name = "LINUX";
  return elfcore_write_note (abfd, buf, bufsiz,
			     note_name, NT_S390_CTRS, s390_ctrs, size);
}

char *
elfcore_write_s390_prefix (bfd *abfd,
			   char *buf,
			   int *bufsiz,
			   const void *s390_prefix,
			   int size)
{
  char *note_name = "LINUX";
  return elfcore_write_note (abfd, buf, bufsiz,
			     note_name, NT_S390_PREFIX, s390_prefix, size);
}

char *
elfcore_write_s390_last_break (bfd *abfd,
			       char *buf,
			       int *bufsiz,
			       const void *s390_last_break,
			       int size)
{
  char *note_name = "LINUX";
  return elfcore_write_note (abfd, buf, bufsiz,
			     note_name, NT_S390_LAST_BREAK,
			     s390_last_break, size);
}

char *
elfcore_write_s390_system_call (bfd *abfd,
				char *buf,
				int *bufsiz,
				const void *s390_system_call,
				int size)
{
  char *note_name = "LINUX";
  return elfcore_write_note (abfd, buf, bufsiz,
			     note_name, NT_S390_SYSTEM_CALL,
			     s390_system_call, size);
}

char *
elfcore_write_s390_tdb (bfd *abfd,
			char *buf,
			int *bufsiz,
			const void *s390_tdb,
			int size)
{
  char *note_name = "LINUX";
  return elfcore_write_note (abfd, buf, bufsiz,
			     note_name, NT_S390_TDB, s390_tdb, size);
}

char *
elfcore_write_s390_vxrs_low (bfd *abfd,
			     char *buf,
			     int *bufsiz,
			     const void *s390_vxrs_low,
			     int size)
{
  char *note_name = "LINUX";
  return elfcore_write_note (abfd, buf, bufsiz,
			     note_name, NT_S390_VXRS_LOW, s390_vxrs_low, size);
}

char *
elfcore_write_s390_vxrs_high (bfd *abfd,
			     char *buf,
			     int *bufsiz,
			     const void *s390_vxrs_high,
			     int size)
{
  char *note_name = "LINUX";
  return elfcore_write_note (abfd, buf, bufsiz,
			     note_name, NT_S390_VXRS_HIGH,
			     s390_vxrs_high, size);
}

char *
elfcore_write_s390_gs_cb (bfd *abfd,
			  char *buf,
			  int *bufsiz,
			  const void *s390_gs_cb,
			  int size)
{
  char *note_name = "LINUX";
  return elfcore_write_note (abfd, buf, bufsiz,
			     note_name, NT_S390_GS_CB,
			     s390_gs_cb, size);
}

char *
elfcore_write_s390_gs_bc (bfd *abfd,
			  char *buf,
			  int *bufsiz,
			  const void *s390_gs_bc,
			  int size)
{
  char *note_name = "LINUX";
  return elfcore_write_note (abfd, buf, bufsiz,
			     note_name, NT_S390_GS_BC,
			     s390_gs_bc, size);
}

char *
elfcore_write_arm_vfp (bfd *abfd,
		       char *buf,
		       int *bufsiz,
		       const void *arm_vfp,
		       int size)
{
  char *note_name = "LINUX";
  return elfcore_write_note (abfd, buf, bufsiz,
			     note_name, NT_ARM_VFP, arm_vfp, size);
}

char *
elfcore_write_aarch_tls (bfd *abfd,
		       char *buf,
		       int *bufsiz,
		       const void *aarch_tls,
		       int size)
{
  char *note_name = "LINUX";
  return elfcore_write_note (abfd, buf, bufsiz,
			     note_name, NT_ARM_TLS, aarch_tls, size);
}

char *
elfcore_write_aarch_hw_break (bfd *abfd,
			    char *buf,
			    int *bufsiz,
			    const void *aarch_hw_break,
			    int size)
{
  char *note_name = "LINUX";
  return elfcore_write_note (abfd, buf, bufsiz,
			     note_name, NT_ARM_HW_BREAK, aarch_hw_break, size);
}

char *
elfcore_write_aarch_hw_watch (bfd *abfd,
			    char *buf,
			    int *bufsiz,
			    const void *aarch_hw_watch,
			    int size)
{
  char *note_name = "LINUX";
  return elfcore_write_note (abfd, buf, bufsiz,
			     note_name, NT_ARM_HW_WATCH, aarch_hw_watch, size);
}

char *
elfcore_write_aarch_sve (bfd *abfd,
			 char *buf,
			 int *bufsiz,
			 const void *aarch_sve,
			 int size)
{
  char *note_name = "LINUX";
  return elfcore_write_note (abfd, buf, bufsiz,
			     note_name, NT_ARM_SVE, aarch_sve, size);
}

char *
elfcore_write_register_note (bfd *abfd,
			     char *buf,
			     int *bufsiz,
			     const char *section,
			     const void *data,
			     int size)
{
  if (strcmp (section, ".reg2") == 0)
    return elfcore_write_prfpreg (abfd, buf, bufsiz, data, size);
  if (strcmp (section, ".reg-xfp") == 0)
    return elfcore_write_prxfpreg (abfd, buf, bufsiz, data, size);
  if (strcmp (section, ".reg-xstate") == 0)
    return elfcore_write_xstatereg (abfd, buf, bufsiz, data, size);
  if (strcmp (section, ".reg-ppc-vmx") == 0)
    return elfcore_write_ppc_vmx (abfd, buf, bufsiz, data, size);
  if (strcmp (section, ".reg-ppc-vsx") == 0)
    return elfcore_write_ppc_vsx (abfd, buf, bufsiz, data, size);
  if (strcmp (section, ".reg-ppc-tar") == 0)
    return elfcore_write_ppc_tar (abfd, buf, bufsiz, data, size);
  if (strcmp (section, ".reg-ppc-ppr") == 0)
    return elfcore_write_ppc_ppr (abfd, buf, bufsiz, data, size);
  if (strcmp (section, ".reg-ppc-dscr") == 0)
    return elfcore_write_ppc_dscr (abfd, buf, bufsiz, data, size);
  if (strcmp (section, ".reg-ppc-ebb") == 0)
    return elfcore_write_ppc_ebb (abfd, buf, bufsiz, data, size);
  if (strcmp (section, ".reg-ppc-pmu") == 0)
    return elfcore_write_ppc_pmu (abfd, buf, bufsiz, data, size);
  if (strcmp (section, ".reg-ppc-tm-cgpr") == 0)
    return elfcore_write_ppc_tm_cgpr (abfd, buf, bufsiz, data, size);
  if (strcmp (section, ".reg-ppc-tm-cfpr") == 0)
    return elfcore_write_ppc_tm_cfpr (abfd, buf, bufsiz, data, size);
  if (strcmp (section, ".reg-ppc-tm-cvmx") == 0)
    return elfcore_write_ppc_tm_cvmx (abfd, buf, bufsiz, data, size);
  if (strcmp (section, ".reg-ppc-tm-cvsx") == 0)
    return elfcore_write_ppc_tm_cvsx (abfd, buf, bufsiz, data, size);
  if (strcmp (section, ".reg-ppc-tm-spr") == 0)
    return elfcore_write_ppc_tm_spr (abfd, buf, bufsiz, data, size);
  if (strcmp (section, ".reg-ppc-tm-ctar") == 0)
    return elfcore_write_ppc_tm_ctar (abfd, buf, bufsiz, data, size);
  if (strcmp (section, ".reg-ppc-tm-cppr") == 0)
    return elfcore_write_ppc_tm_cppr (abfd, buf, bufsiz, data, size);
  if (strcmp (section, ".reg-ppc-tm-cdscr") == 0)
    return elfcore_write_ppc_tm_cdscr (abfd, buf, bufsiz, data, size);
  if (strcmp (section, ".reg-s390-high-gprs") == 0)
    return elfcore_write_s390_high_gprs (abfd, buf, bufsiz, data, size);
  if (strcmp (section, ".reg-s390-timer") == 0)
    return elfcore_write_s390_timer (abfd, buf, bufsiz, data, size);
  if (strcmp (section, ".reg-s390-todcmp") == 0)
    return elfcore_write_s390_todcmp (abfd, buf, bufsiz, data, size);
  if (strcmp (section, ".reg-s390-todpreg") == 0)
    return elfcore_write_s390_todpreg (abfd, buf, bufsiz, data, size);
  if (strcmp (section, ".reg-s390-ctrs") == 0)
    return elfcore_write_s390_ctrs (abfd, buf, bufsiz, data, size);
  if (strcmp (section, ".reg-s390-prefix") == 0)
    return elfcore_write_s390_prefix (abfd, buf, bufsiz, data, size);
  if (strcmp (section, ".reg-s390-last-break") == 0)
    return elfcore_write_s390_last_break (abfd, buf, bufsiz, data, size);
  if (strcmp (section, ".reg-s390-system-call") == 0)
    return elfcore_write_s390_system_call (abfd, buf, bufsiz, data, size);
  if (strcmp (section, ".reg-s390-tdb") == 0)
    return elfcore_write_s390_tdb (abfd, buf, bufsiz, data, size);
  if (strcmp (section, ".reg-s390-vxrs-low") == 0)
    return elfcore_write_s390_vxrs_low (abfd, buf, bufsiz, data, size);
  if (strcmp (section, ".reg-s390-vxrs-high") == 0)
    return elfcore_write_s390_vxrs_high (abfd, buf, bufsiz, data, size);
  if (strcmp (section, ".reg-s390-gs-cb") == 0)
    return elfcore_write_s390_gs_cb (abfd, buf, bufsiz, data, size);
  if (strcmp (section, ".reg-s390-gs-bc") == 0)
    return elfcore_write_s390_gs_bc (abfd, buf, bufsiz, data, size);
  if (strcmp (section, ".reg-arm-vfp") == 0)
    return elfcore_write_arm_vfp (abfd, buf, bufsiz, data, size);
  if (strcmp (section, ".reg-aarch-tls") == 0)
    return elfcore_write_aarch_tls (abfd, buf, bufsiz, data, size);
  if (strcmp (section, ".reg-aarch-hw-break") == 0)
    return elfcore_write_aarch_hw_break (abfd, buf, bufsiz, data, size);
  if (strcmp (section, ".reg-aarch-hw-watch") == 0)
    return elfcore_write_aarch_hw_watch (abfd, buf, bufsiz, data, size);
  if (strcmp (section, ".reg-aarch-sve") == 0)
    return elfcore_write_aarch_sve (abfd, buf, bufsiz, data, size);
  return NULL;
}

static bfd_boolean
elf_parse_notes (bfd *abfd, char *buf, size_t size, file_ptr offset,
		 size_t align)
{
  char *p;

  /* NB: CORE PT_NOTE segments may have p_align values of 0 or 1.
     gABI specifies that PT_NOTE alignment should be aligned to 4
     bytes for 32-bit objects and to 8 bytes for 64-bit objects.  If
     align is less than 4, we use 4 byte alignment.   */
  if (align < 4)
    align = 4;
  if (align != 4 && align != 8)
    return FALSE;

  p = buf;
  while (p < buf + size)
    {
      Elf_External_Note *xnp = (Elf_External_Note *) p;
      Elf_Internal_Note in;

      if (offsetof (Elf_External_Note, name) > buf - p + size)
	return FALSE;

      in.type = H_GET_32 (abfd, xnp->type);

      in.namesz = H_GET_32 (abfd, xnp->namesz);
      in.namedata = xnp->name;
      if (in.namesz > buf - in.namedata + size)
	return FALSE;

      in.descsz = H_GET_32 (abfd, xnp->descsz);
      in.descdata = p + ELF_NOTE_DESC_OFFSET (in.namesz, align);
      in.descpos = offset + (in.descdata - buf);
      if (in.descsz != 0
	  && (in.descdata >= buf + size
	      || in.descsz > buf - in.descdata + size))
	return FALSE;

      switch (bfd_get_format (abfd))
	{
	default:
	  return TRUE;

	case bfd_core:
	  {
#define GROKER_ELEMENT(S,F) {S, sizeof (S) - 1, F}
	    struct
	    {
	      const char * string;
	      size_t len;
	      bfd_boolean (* func)(bfd *, Elf_Internal_Note *);
	    }
	    grokers[] =
	    {
	      GROKER_ELEMENT ("", elfcore_grok_note),
	      GROKER_ELEMENT ("FreeBSD", elfcore_grok_freebsd_note),
	      GROKER_ELEMENT ("NetBSD-CORE", elfcore_grok_netbsd_note),
	      GROKER_ELEMENT ( "OpenBSD", elfcore_grok_openbsd_note),
	      GROKER_ELEMENT ("QNX", elfcore_grok_nto_note),
	      GROKER_ELEMENT ("SPU/", elfcore_grok_spu_note)
	    };
#undef GROKER_ELEMENT
	    int i;

	    for (i = ARRAY_SIZE (grokers); i--;)
	      {
		if (in.namesz >= grokers[i].len
		    && strncmp (in.namedata, grokers[i].string,
				grokers[i].len) == 0)
		  {
		    if (! grokers[i].func (abfd, & in))
		      return FALSE;
		    break;
		  }
	      }
	    break;
	  }

	case bfd_object:
	  if (in.namesz == sizeof "GNU" && strcmp (in.namedata, "GNU") == 0)
	    {
	      if (! elfobj_grok_gnu_note (abfd, &in))
		return FALSE;
	    }
	  else if (in.namesz == sizeof "stapsdt"
		   && strcmp (in.namedata, "stapsdt") == 0)
	    {
	      if (! elfobj_grok_stapsdt_note (abfd, &in))
		return FALSE;
	    }
	  break;
	}

      p += ELF_NOTE_NEXT_OFFSET (in.namesz, in.descsz, align);
    }

  return TRUE;
}

static bfd_boolean
elf_read_notes (bfd *abfd, file_ptr offset, bfd_size_type size,
		size_t align)
{
  char *buf;

  if (size == 0 || (size + 1) == 0)
    return TRUE;

  if (bfd_seek (abfd, offset, SEEK_SET) != 0)
    return FALSE;

  buf = (char *) bfd_malloc (size + 1);
  if (buf == NULL)
    return FALSE;

  /* PR 17512: file: ec08f814
     0-termintate the buffer so that string searches will not overflow.  */
  buf[size] = 0;

  if (bfd_bread (buf, size, abfd) != size
      || !elf_parse_notes (abfd, buf, size, offset, align))
    {
      free (buf);
      return FALSE;
    }

  free (buf);
  return TRUE;
}

/* Providing external access to the ELF program header table.  */

/* Return an upper bound on the number of bytes required to store a
   copy of ABFD's program header table entries.  Return -1 if an error
   occurs; bfd_get_error will return an appropriate code.  */

long
bfd_get_elf_phdr_upper_bound (bfd *abfd)
{
  if (abfd->xvec->flavour != bfd_target_elf_flavour)
    {
      bfd_set_error (bfd_error_wrong_format);
      return -1;
    }

  return elf_elfheader (abfd)->e_phnum * sizeof (Elf_Internal_Phdr);
}

/* Copy ABFD's program header table entries to *PHDRS.  The entries
   will be stored as an array of Elf_Internal_Phdr structures, as
   defined in include/elf/internal.h.  To find out how large the
   buffer needs to be, call bfd_get_elf_phdr_upper_bound.

   Return the number of program header table entries read, or -1 if an
   error occurs; bfd_get_error will return an appropriate code.  */

int
bfd_get_elf_phdrs (bfd *abfd, void *phdrs)
{
  int num_phdrs;

  if (abfd->xvec->flavour != bfd_target_elf_flavour)
    {
      bfd_set_error (bfd_error_wrong_format);
      return -1;
    }

  num_phdrs = elf_elfheader (abfd)->e_phnum;
  if (num_phdrs != 0)
    memcpy (phdrs, elf_tdata (abfd)->phdr,
	    num_phdrs * sizeof (Elf_Internal_Phdr));

  return num_phdrs;
}

enum elf_reloc_type_class
_bfd_elf_reloc_type_class (const struct bfd_link_info *info ATTRIBUTE_UNUSED,
			   const asection *rel_sec ATTRIBUTE_UNUSED,
			   const Elf_Internal_Rela *rela ATTRIBUTE_UNUSED)
{
  return reloc_class_normal;
}

/* For RELA architectures, return the relocation value for a
   relocation against a local symbol.  */

bfd_vma
_bfd_elf_rela_local_sym (bfd *abfd ATTRIBUTE_UNUSED,
             Elf_Internal_Sym *sym ATTRIBUTE_UNUSED,
             asection **psec ATTRIBUTE_UNUSED,
             Elf_Internal_Rela *rel ATTRIBUTE_UNUSED)
{
  return 0;
}

bfd_vma
_bfd_elf_rel_local_sym (bfd *abfd ATTRIBUTE_UNUSED,
            Elf_Internal_Sym *sym ATTRIBUTE_UNUSED,
            asection **psec ATTRIBUTE_UNUSED,
            bfd_vma addend ATTRIBUTE_UNUSED)
{
  return 0;
}

/* Adjust an address within a section.  Given OFFSET within SEC, return
   the new offset within the section, based upon changes made to the
   section.  Returns -1 if the offset is now invalid.
   The offset (in abnd out) is in target sized bytes, however big a
   byte may be.  */

bfd_vma
_bfd_elf_section_offset (bfd *abfd,
			 struct bfd_link_info *info,
			 asection *sec,
			 bfd_vma offset)
{
  switch (sec->sec_info_type)
    {
    case SEC_INFO_TYPE_EH_FRAME:
      return _bfd_elf_eh_frame_section_offset (abfd, info, sec, offset);

    default:
      if ((sec->flags & SEC_ELF_REVERSE_COPY) != 0)
	{
	  /* Reverse the offset.  */
	  const struct elf_backend_data *bed = get_elf_backend_data (abfd);
	  bfd_size_type address_size = bed->s->arch_size / 8;

	  /* address_size and sec->size are in octets.  Convert
	     to bytes before subtracting the original offset.  */
	  offset = (sec->size - address_size) / bfd_octets_per_byte (abfd) - offset;
	}
      return offset;
    }
}

/* Create a new BFD as if by bfd_openr.  Rather than opening a file,
   reconstruct an ELF file by reading the segments out of remote memory
   based on the ELF file header at EHDR_VMA and the ELF program headers it
   points to.  If not null, *LOADBASEP is filled in with the difference
   between the VMAs from which the segments were read, and the VMAs the
   file headers (and hence BFD's idea of each section's VMA) put them at.

   The function TARGET_READ_MEMORY is called to copy LEN bytes from the
   remote memory at target address VMA into the local buffer at MYADDR; it
   should return zero on success or an `errno' code on failure.  TEMPL must
   be a BFD for an ELF target with the word size and byte order found in
   the remote memory.  */

bfd *
bfd_elf_bfd_from_remote_memory
  (bfd *templ,
   bfd_vma ehdr_vma,
   bfd_size_type size,
   bfd_vma *loadbasep,
   int (*target_read_memory) (bfd_vma, bfd_byte *, bfd_size_type))
{
  return (*get_elf_backend_data (templ)->elf_backend_bfd_from_remote_memory)
    (templ, ehdr_vma, size, loadbasep, target_read_memory);
}

long
_bfd_elf_get_synthetic_symtab (bfd *abfd ATTRIBUTE_UNUSED,
			       long symcount ATTRIBUTE_UNUSED,
			       asymbol **syms ATTRIBUTE_UNUSED,
                   long dynsymcount ATTRIBUTE_UNUSED,
                   asymbol **dynsyms ATTRIBUTE_UNUSED,
                   asymbol **ret ATTRIBUTE_UNUSED)
{
  return -1;
}

/* It is only used by x86-64 so far.
   ??? This repeats *COM* id of zero.  sec->id is supposed to be unique,
   but current usage would allow all of _bfd_std_section to be zero.  */
static const asymbol lcomm_sym
  = GLOBAL_SYM_INIT ("LARGE_COMMON", &_bfd_elf_large_com_section);
asection _bfd_elf_large_com_section
  = BFD_FAKE_SECTION (_bfd_elf_large_com_section, &lcomm_sym,
		      "LARGE_COMMON", 0, SEC_IS_COMMON);

void
_bfd_elf_post_process_headers (bfd * abfd,
			       struct bfd_link_info * link_info ATTRIBUTE_UNUSED)
{
  Elf_Internal_Ehdr * i_ehdrp;	/* ELF file header, internal form.  */

  i_ehdrp = elf_elfheader (abfd);

  i_ehdrp->e_ident[EI_OSABI] = get_elf_backend_data (abfd)->elf_osabi;

  /* To make things simpler for the loader on Linux systems we set the
     osabi field to ELFOSABI_GNU if the binary contains symbols of
     the STT_GNU_IFUNC type or STB_GNU_UNIQUE binding.  */
  if (i_ehdrp->e_ident[EI_OSABI] == ELFOSABI_NONE
      && elf_tdata (abfd)->has_gnu_symbols)
    i_ehdrp->e_ident[EI_OSABI] = ELFOSABI_GNU;
}


/* Return TRUE for ELF symbol types that represent functions.
   This is the default version of this function, which is sufficient for
   most targets.  It returns true if TYPE is STT_FUNC or STT_GNU_IFUNC.  */

bfd_boolean
_bfd_elf_is_function_type (unsigned int type)
{
  return (type == STT_FUNC
	  || type == STT_GNU_IFUNC);
}

/* If the ELF symbol SYM might be a function in SEC, return the
   function size and set *CODE_OFF to the function's entry point,
   otherwise return zero.  */

bfd_size_type
_bfd_elf_maybe_function_sym (const asymbol *sym, asection *sec,
			     bfd_vma *code_off)
{
  bfd_size_type size;

  if ((sym->flags & (BSF_SECTION_SYM | BSF_FILE | BSF_OBJECT
		     | BSF_THREAD_LOCAL | BSF_RELC | BSF_SRELC)) != 0
      || sym->section != sec)
    return 0;

  *code_off = sym->value;
  size = 0;
  if (!(sym->flags & BSF_SYNTHETIC))
    size = ((elf_symbol_type *) sym)->internal_elf_sym.st_size;
  if (size == 0)
    size = 1;
  return size;
}
