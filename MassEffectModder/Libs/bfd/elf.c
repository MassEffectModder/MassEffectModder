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

/* Swap in a Versym structure.  */

void
_bfd_elf_swap_versym_in (bfd *abfd,
             const Elf_External_Versym *src,
             Elf_Internal_Versym *dst)
{
  dst->vs_vers = H_GET_16 (abfd, src->vs_vers);
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
    case SHT_NOTE:		/* .note section.  */
    case SHT_DYNAMIC:	/* Dynamic linking information.  */
      goto success;

    case SHT_PROGBITS:		/* Normal section with contents.  */
    case SHT_NOBITS:		/* .bss section.  */
    case SHT_HASH:		/* .hash section.  */
    case SHT_INIT_ARRAY:	/* .init_array section.  */
    case SHT_FINI_ARRAY:	/* .fini_array section.  */
    case SHT_PREINIT_ARRAY:	/* .preinit_array section.  */
    case SHT_GNU_LIBLIST:	/* .gnu.liblist section.  */
    case SHT_GNU_HASH:		/* .gnu.hash section.  */
      ret = _bfd_elf_make_section_from_shdr (abfd, hdr, name, shindex);
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
    case SHT_GROUP:
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

/* Given a section, search the header to find them.  */

unsigned int
_bfd_elf_section_from_bfd_section (bfd *abfd ATTRIBUTE_UNUSED, struct bfd_section *asect ATTRIBUTE_UNUSED)
{
  return 0;
}

/* Given a BFD symbol, return the index in the ELF symbol table, or -1
   on error.  */

int
_bfd_elf_symbol_from_bfd_symbol (bfd *abfd ATTRIBUTE_UNUSED, asymbol **asym_ptr_ptr ATTRIBUTE_UNUSED)
{
  return -1;
}

/* Initialize private output section information from input section.  */

bfd_boolean
_bfd_elf_init_private_section_data (bfd *ibfd ATTRIBUTE_UNUSED,
                    asection *isec ATTRIBUTE_UNUSED,
                    bfd *obfd ATTRIBUTE_UNUSED,
                    asection *osec ATTRIBUTE_UNUSED,
                    struct bfd_link_info *link_info ATTRIBUTE_UNUSED)

{
  return TRUE;
}

/* Copy private section information.  This copies over the entsize
   field, and sometimes the info field.  */

bfd_boolean
_bfd_elf_copy_private_section_data (bfd *ibfd ATTRIBUTE_UNUSED,
                    asection *isec ATTRIBUTE_UNUSED,
                    bfd *obfd ATTRIBUTE_UNUSED,
                    asection *osec ATTRIBUTE_UNUSED)
{
  return TRUE;
}

/* Copy private header information.  */

bfd_boolean
_bfd_elf_copy_private_header_data (bfd *ibfd ATTRIBUTE_UNUSED, bfd *obfd ATTRIBUTE_UNUSED)
{
  return TRUE;
}

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
elf_slurp_symbol_table (bfd *abfd, asymbol **symptrs, bfd_boolean dynamic);

long
_bfd_elf_canonicalize_symtab (bfd *abfd, asymbol **allocation)
{
  long symcount = elf_slurp_symbol_table (abfd, allocation, FALSE);

  if (symcount >= 0)
    bfd_get_symcount (abfd) = symcount;
  return symcount;
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
              asymbol *symbol ATTRIBUTE_UNUSED,
              symbol_info *ret ATTRIBUTE_UNUSED)
{
}

/* Return whether a symbol name implies a local symbol.  Most targets
   use this function for the is_local_label_name entry point, but some
   override it.  */

bfd_boolean
_bfd_elf_is_local_label_name (bfd *abfd ATTRIBUTE_UNUSED,
                  const char *name ATTRIBUTE_UNUSED)
{
  return TRUE;
}

alent *
_bfd_elf_get_lineno (bfd *abfd ATTRIBUTE_UNUSED,
		     asymbol *symbol ATTRIBUTE_UNUSED)
{
  abort ();
  return NULL;
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
