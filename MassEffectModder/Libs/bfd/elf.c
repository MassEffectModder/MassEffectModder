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

/* Swap in a Versym structure.  */

void
_bfd_elf_swap_versym_in (bfd *abfd ATTRIBUTE_UNUSED,
             const Elf_External_Versym *src ATTRIBUTE_UNUSED,
             Elf_Internal_Versym *dst ATTRIBUTE_UNUSED)
{
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
		return FALSE;
	    }
	}
      else
	/* For objdump, don't rename the section.  For objcopy, delay
	   section rename to elf_fake_sections.  */
	newsect->flags |= SEC_ELF_RENAME;
    }

  return TRUE;
}

bfd_reloc_status_type
bfd_elf_generic_reloc (bfd *abfd ATTRIBUTE_UNUSED,
               arelent *reloc_entry ATTRIBUTE_UNUSED,
               asymbol *symbol ATTRIBUTE_UNUSED,
               void *data ATTRIBUTE_UNUSED,
               asection *input_section ATTRIBUTE_UNUSED,
               bfd *output_bfd ATTRIBUTE_UNUSED,
               char **error_message ATTRIBUTE_UNUSED)
{
  return bfd_reloc_continue;
}

bfd_boolean
_bfd_elf_copy_private_bfd_data (bfd *ibfd ATTRIBUTE_UNUSED, bfd *obfd ATTRIBUTE_UNUSED)
{
  return TRUE;
}

bfd_boolean
_bfd_elf_print_private_bfd_data (bfd *abfd ATTRIBUTE_UNUSED, void *farg ATTRIBUTE_UNUSED)
{
  return TRUE;
}

const char *
_bfd_elf_get_symbol_version_string (bfd *abfd ATTRIBUTE_UNUSED, asymbol *symbol ATTRIBUTE_UNUSED,
                    bfd_boolean *hidden ATTRIBUTE_UNUSED)
{
  return NULL;
}

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

/* Given an ELF section number, retrieve the corresponding BFD
   section.  */

asection *
bfd_section_from_elf_index (bfd *abfd, unsigned int sec_index)
{
  if (sec_index >= elf_numsections (abfd))
    return NULL;
  return elf_elfsections (abfd)[sec_index]->bfd_section;
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

  return _bfd_generic_new_section_hook (abfd, sec);
}

bfd_boolean
_bfd_elf_init_private_section_data (bfd *ibfd ATTRIBUTE_UNUSED,
                    asection *isec ATTRIBUTE_UNUSED,
                    bfd *obfd ATTRIBUTE_UNUSED,
                    asection *osec ATTRIBUTE_UNUSED,
                    struct bfd_link_info *link_info ATTRIBUTE_UNUSED)

{
  return TRUE;
}

bfd_boolean
_bfd_elf_copy_private_section_data (bfd *ibfd ATTRIBUTE_UNUSED,
                    asection *isec ATTRIBUTE_UNUSED,
                    bfd *obfd ATTRIBUTE_UNUSED,
                    asection *osec ATTRIBUTE_UNUSED)
{
  return TRUE;
}

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

bfd_boolean
_bfd_elf_find_line (bfd *abfd ATTRIBUTE_UNUSED, asymbol **symbols ATTRIBUTE_UNUSED, asymbol *symbol ATTRIBUTE_UNUSED,
            const char **filename_ptr ATTRIBUTE_UNUSED, unsigned int *line_ptr ATTRIBUTE_UNUSED)
{
    return FALSE;
}

bfd_boolean
_bfd_elf_find_inliner_info (bfd *abfd ATTRIBUTE_UNUSED,
                const char **filename_ptr ATTRIBUTE_UNUSED,
                const char **functionname_ptr ATTRIBUTE_UNUSED,
                unsigned int *line_ptr ATTRIBUTE_UNUSED)
{
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
