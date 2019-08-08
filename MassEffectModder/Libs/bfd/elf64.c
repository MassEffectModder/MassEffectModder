/* ELF executable support for BFD.
   Copyright (C) 1991-2019 Free Software Foundation, Inc.

   Written by Fred Fish @ Cygnus Support, from information published
   in "UNIX System V Release 4, Programmers Guide: ANSI C and
   Programming Support Tools".  Sufficient support for gdb.

   Rewritten by Mark Eichin @ Cygnus Support, from information
   published in "System V Application Binary Interface", chapters 4
   and 5, as well as the various "Processor Supplement" documents
   derived from it. Added support for assembler and other object file
   utilities.  Further work done by Ken Raeburn (Cygnus Support), Michael
   Meissner (Open Software Foundation), and Peter Hoogenboom (University
   of Utah) to finish and extend this.

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


/* Problems and other issues to resolve.

   (1)	BFD expects there to be some fixed number of "sections" in
	the object file.  I.E. there is a "section_count" variable in the
	bfd structure which contains the number of sections.  However, ELF
	supports multiple "views" of a file.  In particular, with current
	implementations, executable files typically have two tables, a
	program header table and a section header table, both of which
	partition the executable.

	In ELF-speak, the "linking view" of the file uses the section header
	table to access "sections" within the file, and the "execution view"
	uses the program header table to access "segments" within the file.
	"Segments" typically may contain all the data from one or more
	"sections".

	Note that the section header table is optional in ELF executables,
	but it is this information that is most useful to gdb.  If the
	section header table is missing, then gdb should probably try
	to make do with the program header table.  (FIXME)

   (2)  The code in this file is compiled twice, once in 32-bit mode and
	once in 64-bit mode.  More of it should be made size-independent
	and moved into elf.c.

   (3)	ELF section symbols are handled rather sloppily now.  This should
	be cleaned up, and ELF section symbols reconciled with BFD section
	symbols.

   (4)  We need a published spec for 64-bit ELF.  We've got some stuff here
	that we're using for SPARC V9 64-bit chips, but don't assume that
	it's cast in stone.
 */

#define ARCH_SIZE		64

#include "sysdep.h"
#include "bfd.h"
#include "libiberty.h"
#include "bfdlink.h"
#include "libbfd.h"
#include "elf-bfd.h"
#include "libiberty.h"

/* Renaming structures, typedefs, macros and functions to be size-specific.  */
#define Elf_External_Ehdr	NAME(Elf,External_Ehdr)
#define Elf_External_Sym	NAME(Elf,External_Sym)
#define Elf_External_Shdr	NAME(Elf,External_Shdr)
#define Elf_External_Phdr	NAME(Elf,External_Phdr)
#define Elf_External_Rel	NAME(Elf,External_Rel)
#define Elf_External_Rela	NAME(Elf,External_Rela)
#define Elf_External_Dyn	NAME(Elf,External_Dyn)

#define elf_core_file_failing_command	NAME(bfd_elf,core_file_failing_command)
#define elf_core_file_failing_signal	NAME(bfd_elf,core_file_failing_signal)
#define elf_core_file_matches_executable_p \
  NAME(bfd_elf,core_file_matches_executable_p)
#define elf_core_file_pid		NAME(bfd_elf,core_file_pid)
#define elf_object_p			NAME(bfd_elf,object_p)
#define elf_core_file_p			NAME(bfd_elf,core_file_p)
#define elf_get_symtab_upper_bound	NAME(bfd_elf,get_symtab_upper_bound)
#define elf_get_dynamic_symtab_upper_bound \
  NAME(bfd_elf,get_dynamic_symtab_upper_bound)
#define elf_swap_reloc_in		NAME(bfd_elf,swap_reloc_in)
#define elf_swap_reloca_in		NAME(bfd_elf,swap_reloca_in)
#define elf_swap_reloc_out		NAME(bfd_elf,swap_reloc_out)
#define elf_swap_reloca_out		NAME(bfd_elf,swap_reloca_out)
//#define elf_swap_symbol_in		NAME(bfd_elf,swap_symbol_in)
#define elf_swap_symbol_out		NAME(bfd_elf,swap_symbol_out)
#define elf_swap_phdr_in		NAME(bfd_elf,swap_phdr_in)
#define elf_swap_phdr_out		NAME(bfd_elf,swap_phdr_out)
#define elf_swap_dyn_in			NAME(bfd_elf,swap_dyn_in)
#define elf_swap_dyn_out		NAME(bfd_elf,swap_dyn_out)
#define elf_get_reloc_upper_bound	NAME(bfd_elf,get_reloc_upper_bound)
#define elf_canonicalize_reloc		NAME(bfd_elf,canonicalize_reloc)
//#define elf_slurp_symbol_table		NAME(bfd_elf,slurp_symbol_table)
#define elf_canonicalize_symtab		NAME(bfd_elf,canonicalize_symtab)
#define elf_canonicalize_dynamic_symtab \
  NAME(bfd_elf,canonicalize_dynamic_symtab)
#define elf_get_synthetic_symtab \
  NAME(bfd_elf,get_synthetic_symtab)
#define elf_make_empty_symbol		NAME(bfd_elf,make_empty_symbol)
#define elf_get_symbol_info		NAME(bfd_elf,get_symbol_info)
#define elf_get_lineno			NAME(bfd_elf,get_lineno)
#define elf_set_arch_mach		NAME(bfd_elf,set_arch_mach)
#define elf_find_nearest_line		NAME(bfd_elf,find_nearest_line)
#define elf_sizeof_headers		NAME(bfd_elf,sizeof_headers)
#define elf_set_section_contents	NAME(bfd_elf,set_section_contents)
#define elf_no_info_to_howto		NAME(bfd_elf,no_info_to_howto)
#define elf_no_info_to_howto_rel	NAME(bfd_elf,no_info_to_howto_rel)
#define elf_find_section		NAME(bfd_elf,find_section)
#define elf_write_shdrs_and_ehdr	NAME(bfd_elf,write_shdrs_and_ehdr)
#define elf_write_out_phdrs		NAME(bfd_elf,write_out_phdrs)
#define elf_checksum_contents		NAME(bfd_elf,checksum_contents)
#define elf_write_relocs		NAME(bfd_elf,write_relocs)
#define elf_slurp_reloc_table		NAME(bfd_elf,slurp_reloc_table)

#if ARCH_SIZE == 64
#define ELF_R_INFO(X,Y)	ELF64_R_INFO(X,Y)
#define ELF_R_SYM(X)	ELF64_R_SYM(X)
#define ELF_R_TYPE(X)	ELF64_R_TYPE(X)
#define ELFCLASS	ELFCLASS64
#define FILE_ALIGN	8
#define LOG_FILE_ALIGN	3
#endif

/* Structure swapping routines */

/* Should perhaps use put_offset, put_word, etc.  For now, the two versions
   can be handled by explicitly specifying 32 bits or "the long type".  */
#if ARCH_SIZE == 64
#define H_PUT_WORD		H_PUT_64
#define H_PUT_SIGNED_WORD	H_PUT_S64
#define H_GET_WORD		H_GET_64
#define H_GET_SIGNED_WORD	H_GET_S64
#endif

/* Translate an ELF symbol in external format into an ELF symbol in internal
   format.  */

bfd_boolean
elf_swap_symbol_in (bfd *abfd,
		    const void *psrc,
		    const void *pshn,
		    Elf_Internal_Sym *dst)
{
  const Elf_External_Sym *src = (const Elf_External_Sym *) psrc;
  const Elf_External_Sym_Shndx *shndx = (const Elf_External_Sym_Shndx *) pshn;

  dst->st_name = H_GET_32 (abfd, src->st_name);
  dst->st_value = H_GET_WORD (abfd, src->st_value);
  dst->st_size = H_GET_WORD (abfd, src->st_size);
  dst->st_info = H_GET_8 (abfd, src->st_info);
  dst->st_other = H_GET_8 (abfd, src->st_other);
  dst->st_shndx = H_GET_16 (abfd, src->st_shndx);
  if (dst->st_shndx == (SHN_XINDEX & 0xffff))
    {
      if (shndx == NULL)
	return FALSE;
      dst->st_shndx = H_GET_32 (abfd, shndx->est_shndx);
    }
  else if (dst->st_shndx >= (SHN_LORESERVE & 0xffff))
    dst->st_shndx += SHN_LORESERVE - (SHN_LORESERVE & 0xffff);
  dst->st_target_internal = 0;
  return TRUE;
}

/* Translate an ELF symbol in internal format into an ELF symbol in external
   format.  */

/* Translate an ELF file header in external format into an ELF file header in
   internal format.  */

static void
elf_swap_ehdr_in (bfd *abfd,
		  const Elf_External_Ehdr *src,
		  Elf_Internal_Ehdr *dst)
{
  memcpy (dst->e_ident, src->e_ident, EI_NIDENT);
  dst->e_type = H_GET_16 (abfd, src->e_type);
  dst->e_machine = H_GET_16 (abfd, src->e_machine);
  dst->e_version = H_GET_32 (abfd, src->e_version);
  dst->e_entry = H_GET_WORD (abfd, src->e_entry);
  dst->e_phoff = H_GET_WORD (abfd, src->e_phoff);
  dst->e_shoff = H_GET_WORD (abfd, src->e_shoff);
  dst->e_flags = H_GET_32 (abfd, src->e_flags);
  dst->e_ehsize = H_GET_16 (abfd, src->e_ehsize);
  dst->e_phentsize = H_GET_16 (abfd, src->e_phentsize);
  dst->e_phnum = H_GET_16 (abfd, src->e_phnum);
  dst->e_shentsize = H_GET_16 (abfd, src->e_shentsize);
  dst->e_shnum = H_GET_16 (abfd, src->e_shnum);
  dst->e_shstrndx = H_GET_16 (abfd, src->e_shstrndx);
}

/* Translate an ELF section header table entry in external format into an
   ELF section header table entry in internal format.  */

static void
elf_swap_shdr_in (bfd *abfd,
		  const Elf_External_Shdr *src,
		  Elf_Internal_Shdr *dst)
{
  dst->sh_name = H_GET_32 (abfd, src->sh_name);
  dst->sh_type = H_GET_32 (abfd, src->sh_type);
  dst->sh_flags = H_GET_WORD (abfd, src->sh_flags);
  dst->sh_addr = H_GET_WORD (abfd, src->sh_addr);
  dst->sh_offset = H_GET_WORD (abfd, src->sh_offset);
  dst->sh_size = H_GET_WORD (abfd, src->sh_size);
  /* PR 23657.  Check for invalid section size, in sections with contents.
     Note - we do not set an error value here because the contents
     of this particular section might not be needed by the consumer.  */
  if (dst->sh_type != SHT_NOBITS
      && dst->sh_size > bfd_get_file_size (abfd))
    _bfd_error_handler
      (_("warning: %pB has a corrupt section with a size (%" BFD_VMA_FMT "x) larger than the file size"),
       abfd, dst->sh_size);
  dst->sh_link = H_GET_32 (abfd, src->sh_link);
  dst->sh_info = H_GET_32 (abfd, src->sh_info);
  dst->sh_addralign = H_GET_WORD (abfd, src->sh_addralign);
  dst->sh_entsize = H_GET_WORD (abfd, src->sh_entsize);
  dst->bfd_section = NULL;
  dst->contents = NULL;
}

/* Translate an ELF section header table entry in internal format into an
   ELF section header table entry in external format.  */

/* Translate an ELF program header table entry in external format into an
   ELF program header table entry in internal format.  */

void
elf_swap_phdr_in (bfd *abfd,
		  const Elf_External_Phdr *src,
		  Elf_Internal_Phdr *dst)
{
  dst->p_type = H_GET_32 (abfd, src->p_type);
  dst->p_flags = H_GET_32 (abfd, src->p_flags);
  dst->p_offset = H_GET_WORD (abfd, src->p_offset);
  dst->p_vaddr = H_GET_WORD (abfd, src->p_vaddr);
  dst->p_paddr = H_GET_WORD (abfd, src->p_paddr);
  dst->p_filesz = H_GET_WORD (abfd, src->p_filesz);
  dst->p_memsz = H_GET_WORD (abfd, src->p_memsz);
  dst->p_align = H_GET_WORD (abfd, src->p_align);
}

/* ELF .o/exec file reading */

/* Begin processing a given object.

   First we validate the file by reading in the ELF header and checking
   the magic number.  */

static inline bfd_boolean
elf_file_p (Elf_External_Ehdr *x_ehdrp)
{
  return ((x_ehdrp->e_ident[EI_MAG0] == ELFMAG0)
	  && (x_ehdrp->e_ident[EI_MAG1] == ELFMAG1)
	  && (x_ehdrp->e_ident[EI_MAG2] == ELFMAG2)
	  && (x_ehdrp->e_ident[EI_MAG3] == ELFMAG3));
}

/* Check to see if the file associated with ABFD matches the target vector
   that ABFD points to.

   Note that we may be called several times with the same ABFD, but different
   target vectors, most of which will not match.  We have to avoid leaving
   any side effects in ABFD, or any data it points to (like tdata), if the
   file does not match the target vector.  */

const bfd_target *
elf_object_p (bfd *abfd)
{
  Elf_External_Ehdr x_ehdr;	/* Elf file header, external form */
  Elf_Internal_Ehdr *i_ehdrp;	/* Elf file header, internal form */
  Elf_External_Shdr x_shdr;	/* Section header table entry, external form */
  Elf_Internal_Shdr i_shdr;
  Elf_Internal_Shdr *i_shdrp;	/* Section header table, internal form */
  unsigned int shindex;
  asection *s;
  bfd_size_type amt;
  const bfd_target *target;

  /* Read in the ELF header in external format.  */

  if (bfd_bread (&x_ehdr, sizeof (x_ehdr), abfd) != sizeof (x_ehdr))
    {
      if (bfd_get_error () != bfd_error_system_call)
	goto got_wrong_format_error;
      else
	goto got_no_match;
    }

  /* Now check to see if we have a valid ELF file, and one that BFD can
     make use of.  The magic number must match, the address size ('class')
     and byte-swapping must match our XVEC entry, and it must have a
     section header table (FIXME: See comments re sections at top of this
     file).  */

  if (! elf_file_p (&x_ehdr)
      || x_ehdr.e_ident[EI_VERSION] != EV_CURRENT
      || x_ehdr.e_ident[EI_CLASS] != ELFCLASS)
    goto got_wrong_format_error;

  /* Check that file's byte order matches xvec's */
  switch (x_ehdr.e_ident[EI_DATA])
    {
    case ELFDATA2MSB:		/* Big-endian */
      if (! bfd_header_big_endian (abfd))
	goto got_wrong_format_error;
      break;
    case ELFDATA2LSB:		/* Little-endian */
      if (! bfd_header_little_endian (abfd))
	goto got_wrong_format_error;
      break;
    case ELFDATANONE:		/* No data encoding specified */
    default:			/* Unknown data encoding specified */
      goto got_wrong_format_error;
    }

  target = abfd->xvec;

  /* Allocate an instance of the elf_obj_tdata structure and hook it up to
     the tdata pointer in the bfd.  */

  if (! (*target->_bfd_set_format[bfd_object]) (abfd))
    goto got_no_match;

  /* Now that we know the byte order, swap in the rest of the header */
  i_ehdrp = elf_elfheader (abfd);
  elf_swap_ehdr_in (abfd, &x_ehdr, i_ehdrp);

  /* Reject ET_CORE (header indicates core file, not object file) */
  if (i_ehdrp->e_type == ET_CORE)
    goto got_wrong_format_error;

  /* If this is a relocatable file and there is no section header
     table, then we're hosed.  */
  if (i_ehdrp->e_shoff == 0 && i_ehdrp->e_type == ET_REL)
    goto got_wrong_format_error;

  /* As a simple sanity check, verify that what BFD thinks is the
     size of each section header table entry actually matches the size
     recorded in the file, but only if there are any sections.  */
  if (i_ehdrp->e_shentsize != sizeof (x_shdr) && i_ehdrp->e_shnum != 0)
    goto got_wrong_format_error;

  /* Further sanity check.  */
  if (i_ehdrp->e_shoff == 0 && i_ehdrp->e_shnum != 0)
    goto got_wrong_format_error;

  /* Check that the ELF e_machine field matches what this particular
     BFD format expects.  */
  if (EM_X86_64 != i_ehdrp->e_machine)
    goto got_wrong_format_error;

  if (i_ehdrp->e_type == ET_EXEC)
    abfd->flags |= EXEC_P;
  else if (i_ehdrp->e_type == ET_DYN)
    abfd->flags |= DYNAMIC;

  if (i_ehdrp->e_phnum > 0)
    abfd->flags |= D_PAGED;

  if (! bfd_default_set_arch_mach (abfd, bfd_arch_i386, 0))
    {
	goto got_no_match;
    }

  if (i_ehdrp->e_shoff != 0)
    {
      file_ptr where = (file_ptr) i_ehdrp->e_shoff;

      /* Seek to the section header table in the file.  */
      if (bfd_seek (abfd, where, SEEK_SET) != 0)
	goto got_no_match;

      /* Read the first section header at index 0, and convert to internal
	 form.  */
      if (bfd_bread (&x_shdr, sizeof x_shdr, abfd) != sizeof (x_shdr))
	goto got_no_match;
      elf_swap_shdr_in (abfd, &x_shdr, &i_shdr);

      /* If the section count is zero, the actual count is in the first
	 section header.  */
      if (i_ehdrp->e_shnum == SHN_UNDEF)
	{
	  i_ehdrp->e_shnum = i_shdr.sh_size;
	  if (i_ehdrp->e_shnum >= SHN_LORESERVE
	      || i_ehdrp->e_shnum != i_shdr.sh_size
	      || i_ehdrp->e_shnum  == 0)
	    goto got_wrong_format_error;
	}

      /* And similarly for the string table index.  */
      if (i_ehdrp->e_shstrndx == (SHN_XINDEX & 0xffff))
	{
	  i_ehdrp->e_shstrndx = i_shdr.sh_link;
	  if (i_ehdrp->e_shstrndx != i_shdr.sh_link)
	    goto got_wrong_format_error;
	}

      /* And program headers.  */
      if (i_ehdrp->e_phnum == PN_XNUM && i_shdr.sh_info != 0)
	{
	  i_ehdrp->e_phnum = i_shdr.sh_info;
	  if (i_ehdrp->e_phnum != i_shdr.sh_info)
	    goto got_wrong_format_error;
	}

      /* Sanity check that we can read all of the section headers.
	 It ought to be good enough to just read the last one.  */
      if (i_ehdrp->e_shnum != 1)
	{
	  /* Check that we don't have a totally silly number of sections.  */
	  if (i_ehdrp->e_shnum > (unsigned int) -1 / sizeof (x_shdr)
	      || i_ehdrp->e_shnum > (unsigned int) -1 / sizeof (i_shdr))
	    goto got_wrong_format_error;

	  where += (i_ehdrp->e_shnum - 1) * sizeof (x_shdr);
	  if ((bfd_size_type) where <= i_ehdrp->e_shoff)
	    goto got_wrong_format_error;

	  if (bfd_seek (abfd, where, SEEK_SET) != 0)
	    goto got_no_match;
	  if (bfd_bread (&x_shdr, sizeof x_shdr, abfd) != sizeof (x_shdr))
	    goto got_no_match;

	  /* Back to where we were.  */
	  where = i_ehdrp->e_shoff + sizeof (x_shdr);
	  if (bfd_seek (abfd, where, SEEK_SET) != 0)
	    goto got_no_match;
	}
    }

  /* Allocate space for a copy of the section header table in
     internal form.  */
  if (i_ehdrp->e_shnum != 0)
    {
      Elf_Internal_Shdr *shdrp;
      unsigned int num_sec;

      amt = sizeof (*i_shdrp) * (bfd_size_type) i_ehdrp->e_shnum;
      i_shdrp = (Elf_Internal_Shdr *) bfd_alloc (abfd, amt);
      if (!i_shdrp)
	goto got_no_match;
      num_sec = i_ehdrp->e_shnum;
      elf_numsections (abfd) = num_sec;
      amt = sizeof (i_shdrp) * num_sec;
      elf_elfsections (abfd) = (Elf_Internal_Shdr **) bfd_alloc (abfd, amt);
      if (!elf_elfsections (abfd))
	goto got_no_match;

      memcpy (i_shdrp, &i_shdr, sizeof (*i_shdrp));
      for (shdrp = i_shdrp, shindex = 0; shindex < num_sec; shindex++)
	elf_elfsections (abfd)[shindex] = shdrp++;

      /* Read in the rest of the section header table and convert it
	 to internal form.  */
      for (shindex = 1; shindex < i_ehdrp->e_shnum; shindex++)
	{
	  if (bfd_bread (&x_shdr, sizeof x_shdr, abfd) != sizeof (x_shdr))
	    goto got_no_match;
	  elf_swap_shdr_in (abfd, &x_shdr, i_shdrp + shindex);

	  /* Sanity check sh_link and sh_info.  */
	  if (i_shdrp[shindex].sh_link >= num_sec)
	    {
	      /* PR 10478: Accept Solaris binaries with a sh_link
		 field set to SHN_BEFORE or SHN_AFTER.  */
		  if (i_shdrp[shindex].sh_link == (SHN_LORESERVE & 0xffff) /* SHN_BEFORE */
		      || i_shdrp[shindex].sh_link == ((SHN_LORESERVE + 1) & 0xffff) /* SHN_AFTER */)
		    break;
	    }

	  if (((i_shdrp[shindex].sh_flags & SHF_INFO_LINK)
	       || i_shdrp[shindex].sh_type == SHT_RELA
	       || i_shdrp[shindex].sh_type == SHT_REL)
	      && i_shdrp[shindex].sh_info >= num_sec)
	    goto got_wrong_format_error;

	  /* If the section is loaded, but not page aligned, clear
	     D_PAGED.  */
	  if (i_shdrp[shindex].sh_size != 0
	      && (i_shdrp[shindex].sh_flags & SHF_ALLOC) != 0
	      && i_shdrp[shindex].sh_type != SHT_NOBITS
	      && (((i_shdrp[shindex].sh_addr - i_shdrp[shindex].sh_offset)
           % 0x1000)
		  != 0))
	    abfd->flags &= ~D_PAGED;
	}
    }

  /* A further sanity check.  */
  if (i_ehdrp->e_shnum != 0)
    {
      if (i_ehdrp->e_shstrndx >= elf_numsections (abfd))
	{
	  /* PR 2257:
	     We used to just goto got_wrong_format_error here
	     but there are binaries in existance for which this test
	     will prevent the binutils from working with them at all.
	     So we are kind, and reset the string index value to 0
	     so that at least some processing can be done.  */
	  i_ehdrp->e_shstrndx = SHN_UNDEF;
	  _bfd_error_handler
	    (_("warning: %pB has a corrupt string table index - ignoring"),
	     abfd);
	}
    }
  else if (i_ehdrp->e_shstrndx != SHN_UNDEF)
    goto got_wrong_format_error;

  /* Read in the program headers.  */
  if (i_ehdrp->e_phnum == 0)
    elf_tdata (abfd)->phdr = NULL;
  else
    {
      Elf_Internal_Phdr *i_phdr;
      unsigned int i;

      /* Check for a corrupt input file with an impossibly large number
	 of program headers.  */
      if (bfd_get_file_size (abfd) > 0
	  && i_ehdrp->e_phnum > bfd_get_file_size (abfd))
	goto got_no_match;
      amt = (bfd_size_type) i_ehdrp->e_phnum * sizeof (*i_phdr);
      elf_tdata (abfd)->phdr = (Elf_Internal_Phdr *) bfd_alloc (abfd, amt);
      if (elf_tdata (abfd)->phdr == NULL)
	goto got_no_match;
      if (bfd_seek (abfd, (file_ptr) i_ehdrp->e_phoff, SEEK_SET) != 0)
	goto got_no_match;
      i_phdr = elf_tdata (abfd)->phdr;
      for (i = 0; i < i_ehdrp->e_phnum; i++, i_phdr++)
	{
	  Elf_External_Phdr x_phdr;

	  if (bfd_bread (&x_phdr, sizeof x_phdr, abfd) != sizeof x_phdr)
	    goto got_no_match;
	  elf_swap_phdr_in (abfd, &x_phdr, i_phdr);
	}
    }

  if (i_ehdrp->e_shstrndx != 0 && i_ehdrp->e_shoff != 0)
    {
      unsigned int num_sec;

      /* Once all of the section headers have been read and converted, we
	 can start processing them.  Note that the first section header is
	 a dummy placeholder entry, so we ignore it.  */
      num_sec = elf_numsections (abfd);
      for (shindex = 1; shindex < num_sec; shindex++)
	if (!bfd_section_from_shdr (abfd, shindex))
	  goto got_no_match;

      /* Set up ELF sections for SHF_GROUP and SHF_LINK_ORDER.  */
      if (! _bfd_elf_setup_sections (abfd))
	goto got_wrong_format_error;
    }

  /* Remember the entry point specified in the ELF file header.  */
  bfd_set_start_address (abfd, i_ehdrp->e_entry);

  /* If we have created any reloc sections that are associated with
     debugging sections, mark the reloc sections as debugging as well.  */
  for (s = abfd->sections; s != NULL; s = s->next)
    {
      if ((elf_section_data (s)->this_hdr.sh_type == SHT_REL
	   || elf_section_data (s)->this_hdr.sh_type == SHT_RELA)
	  && elf_section_data (s)->this_hdr.sh_info > 0)
	{
	  unsigned long targ_index;
	  asection *targ_sec;

	  targ_index = elf_section_data (s)->this_hdr.sh_info;
	  targ_sec = bfd_section_from_elf_index (abfd, targ_index);
	  if (targ_sec != NULL
	      && (targ_sec->flags & SEC_DEBUGGING) != 0)
	    s->flags |= SEC_DEBUGGING;
	}
    }
  return target;

 got_wrong_format_error:
  bfd_set_error (bfd_error_wrong_format);

 got_no_match:
  return NULL;
}

long
elf_slurp_symbol_table (bfd *abfd, asymbol **symptrs, bfd_boolean dynamic)
{
  Elf_Internal_Shdr *hdr;
  Elf_Internal_Shdr *verhdr;
  unsigned long symcount;	/* Number of external ELF symbols */
  elf_symbol_type *sym;		/* Pointer to current bfd symbol */
  elf_symbol_type *symbase;	/* Buffer for generated bfd symbols */
  Elf_Internal_Sym *isym;
  Elf_Internal_Sym *isymend;
  Elf_Internal_Sym *isymbuf = NULL;
  Elf_External_Versym *xver;
  Elf_External_Versym *xverbuf = NULL;
  bfd_size_type amt;

  /* Read each raw ELF symbol, converting from external ELF form to
     internal ELF form, and then using the information to create a
     canonical bfd symbol table entry.

     Note that we allocate the initial bfd canonical symbol buffer
     based on a one-to-one mapping of the ELF symbols to canonical
     symbols.  We actually use all the ELF symbols, so there will be no
     space left over at the end.  When we have all the symbols, we
     build the caller's pointer vector.  */

  if (! dynamic)
    {
      hdr = &elf_tdata (abfd)->symtab_hdr;
      verhdr = NULL;
    }
  else
    {
      return -1;
    }

  symcount = hdr->sh_size / sizeof (Elf_External_Sym);
  if (symcount == 0)
    sym = symbase = NULL;
  else
    {
      isymbuf = bfd_elf_get_elf_syms (abfd, hdr, symcount, 0,
				      NULL, NULL, NULL);
      if (isymbuf == NULL)
	return -1;

      amt = symcount;
      amt *= sizeof (elf_symbol_type);
      symbase = (elf_symbol_type *) bfd_zalloc (abfd, amt);
      if (symbase == (elf_symbol_type *) NULL)
	goto error_return;

      /* Read the raw ELF version symbol information.  */
      if (verhdr != NULL
	  && verhdr->sh_size / sizeof (Elf_External_Versym) != symcount)
	{
	  _bfd_error_handler
	    /* xgettext:c-format */
	    (_("%pB: version count (%" PRId64 ")"
	       " does not match symbol count (%ld)"),
	     abfd,
	     (int64_t) (verhdr->sh_size / sizeof (Elf_External_Versym)),
	     symcount);

	  /* Slurp in the symbols without the version information,
	     since that is more helpful than just quitting.  */
	  verhdr = NULL;
	}

      if (verhdr != NULL)
	{
	  if (bfd_seek (abfd, verhdr->sh_offset, SEEK_SET) != 0)
	    goto error_return;

	  xverbuf = (Elf_External_Versym *) bfd_malloc (verhdr->sh_size);
	  if (xverbuf == NULL && verhdr->sh_size != 0)
	    goto error_return;

	  if (bfd_bread (xverbuf, verhdr->sh_size, abfd) != verhdr->sh_size)
	    goto error_return;
	}

      /* Skip first symbol, which is a null dummy.  */
      xver = xverbuf;
      if (xver != NULL)
	++xver;
      isymend = isymbuf + symcount;
      for (isym = isymbuf + 1, sym = symbase; isym < isymend; isym++, sym++)
	{
	  memcpy (&sym->internal_elf_sym, isym, sizeof (Elf_Internal_Sym));

	  sym->symbol.the_bfd = abfd;
	  sym->symbol.name = bfd_elf_sym_name (abfd, hdr, isym, NULL);
	  sym->symbol.value = isym->st_value;

	  if (isym->st_shndx == SHN_UNDEF)
	    {
	      sym->symbol.section = bfd_und_section_ptr;
	    }
	  else if (isym->st_shndx == SHN_ABS)
	    {
	      sym->symbol.section = bfd_abs_section_ptr;
	    }
	  else if (isym->st_shndx == SHN_COMMON)
	    {
	      sym->symbol.section = bfd_com_section_ptr;
	      if ((abfd->flags & BFD_PLUGIN) != 0)
		{
		  asection *xc = bfd_get_section_by_name (abfd, "COMMON");

		  if (xc == NULL)
		    {
		      flagword flags = (SEC_ALLOC | SEC_IS_COMMON | SEC_KEEP
					| SEC_EXCLUDE);
		      xc = bfd_make_section_with_flags (abfd, "COMMON", flags);
		      if (xc == NULL)
			goto error_return;
		    }
		  sym->symbol.section = xc;
		}
	      /* Elf puts the alignment into the `value' field, and
		 the size into the `size' field.  BFD wants to see the
		 size in the value field, and doesn't care (at the
		 moment) about the alignment.  */
	      sym->symbol.value = isym->st_size;
	    }
	  else
	    {
	      sym->symbol.section
		= bfd_section_from_elf_index (abfd, isym->st_shndx);
	      if (sym->symbol.section == NULL)
		{
		  /* This symbol is in a section for which we did not
		     create a BFD section.  Just use bfd_abs_section,
		     although it is wrong.  FIXME.  */
		  sym->symbol.section = bfd_abs_section_ptr;
		}
	    }

	  /* If this is a relocatable file, then the symbol value is
	     already section relative.  */
	  if ((abfd->flags & (EXEC_P | DYNAMIC)) != 0)
	    sym->symbol.value -= sym->symbol.section->vma;

	  switch (ELF_ST_BIND (isym->st_info))
	    {
	    case STB_LOCAL:
	      sym->symbol.flags |= BSF_LOCAL;
	      break;
	    case STB_GLOBAL:
	      if (isym->st_shndx != SHN_UNDEF && isym->st_shndx != SHN_COMMON)
		sym->symbol.flags |= BSF_GLOBAL;
	      break;
	    case STB_WEAK:
	      sym->symbol.flags |= BSF_WEAK;
	      break;
	    case STB_GNU_UNIQUE:
	      sym->symbol.flags |= BSF_GNU_UNIQUE;
	      break;
	    }

	  switch (ELF_ST_TYPE (isym->st_info))
	    {
	    case STT_SECTION:
	      sym->symbol.flags |= BSF_SECTION_SYM | BSF_DEBUGGING;
	      break;
	    case STT_FILE:
	      sym->symbol.flags |= BSF_FILE | BSF_DEBUGGING;
	      break;
	    case STT_FUNC:
	      sym->symbol.flags |= BSF_FUNCTION;
	      break;
	    case STT_COMMON:
	      /* FIXME: Do we have to put the size field into the value field
		 as we do with symbols in SHN_COMMON sections (see above) ?  */
	      sym->symbol.flags |= BSF_ELF_COMMON;
	      /* Fall through.  */
	    case STT_OBJECT:
	      sym->symbol.flags |= BSF_OBJECT;
	      break;
	    case STT_TLS:
	      sym->symbol.flags |= BSF_THREAD_LOCAL;
	      break;
	    case STT_RELC:
	      sym->symbol.flags |= BSF_RELC;
	      break;
	    case STT_SRELC:
	      sym->symbol.flags |= BSF_SRELC;
	      break;
	    case STT_GNU_IFUNC:
	      sym->symbol.flags |= BSF_GNU_INDIRECT_FUNCTION;
	      break;
	    }

	  if (dynamic)
	    sym->symbol.flags |= BSF_DYNAMIC;
	}
    }

  /* We rely on the zalloc to clear out the final symbol entry.  */

  symcount = sym - symbase;

  /* Fill in the user's symbol pointer vector if needed.  */
  if (symptrs)
    {
      long l = symcount;

      sym = symbase;
      while (l-- > 0)
	{
	  *symptrs++ = &sym->symbol;
	  sym++;
	}
      *symptrs = 0;		/* Final null pointer */
    }

  if (xverbuf != NULL)
    free (xverbuf);
  if (isymbuf != NULL && hdr->contents != (unsigned char *) isymbuf)
    free (isymbuf);
  return symcount;

error_return:
  if (xverbuf != NULL)
    free (xverbuf);
  if (isymbuf != NULL && hdr->contents != (unsigned char *) isymbuf)
    free (isymbuf);
  return -1;
}
