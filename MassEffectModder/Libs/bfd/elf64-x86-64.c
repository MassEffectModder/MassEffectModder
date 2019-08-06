/* X86-64 specific support for ELF
   Copyright (C) 2000-2019 Free Software Foundation, Inc.
   Contributed by Jan Hubicka <jh@suse.cz>.

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

#include "elfxx-x86.h"
#include "dwarf2.h"
#include "libiberty.h"

#include "opcode/i386.h"
#include "elf/x86-64.h"

#ifdef CORE_HEADER
#include <stdarg.h>
#include CORE_HEADER
#endif

/* In case we're on a 32-bit machine, construct a 64-bit "-1" value.  */
#define MINUS_ONE (~ (bfd_vma) 0)

/* Since both 32-bit and 64-bit x86-64 encode relocation type in the
   identical manner, we use ELF32_R_TYPE instead of ELF64_R_TYPE to get
   relocation type.  We also use ELF_ST_TYPE instead of ELF64_ST_TYPE
   since they are the same.  */

/* The relocation "howto" table.  Order of fields:
   type, rightshift, size, bitsize, pc_relative, bitpos, complain_on_overflow,
   special_function, name, partial_inplace, src_mask, dst_mask, pcrel_offset.  */
static reloc_howto_type x86_64_elf_howto_table[] =
{
  HOWTO(R_X86_64_NONE, 0, 3, 0, FALSE, 0, complain_overflow_dont,
	bfd_elf_generic_reloc, "R_X86_64_NONE",	FALSE, 0x00000000, 0x00000000,
	FALSE),
  HOWTO(R_X86_64_64, 0, 4, 64, FALSE, 0, complain_overflow_bitfield,
	bfd_elf_generic_reloc, "R_X86_64_64", FALSE, MINUS_ONE, MINUS_ONE,
	FALSE),
  HOWTO(R_X86_64_PC32, 0, 2, 32, TRUE, 0, complain_overflow_signed,
	bfd_elf_generic_reloc, "R_X86_64_PC32", FALSE, 0xffffffff, 0xffffffff,
	TRUE),
  HOWTO(R_X86_64_GOT32, 0, 2, 32, FALSE, 0, complain_overflow_signed,
	bfd_elf_generic_reloc, "R_X86_64_GOT32", FALSE, 0xffffffff, 0xffffffff,
	FALSE),
  HOWTO(R_X86_64_PLT32, 0, 2, 32, TRUE, 0, complain_overflow_signed,
	bfd_elf_generic_reloc, "R_X86_64_PLT32", FALSE, 0xffffffff, 0xffffffff,
	TRUE),
  HOWTO(R_X86_64_COPY, 0, 2, 32, FALSE, 0, complain_overflow_bitfield,
	bfd_elf_generic_reloc, "R_X86_64_COPY", FALSE, 0xffffffff, 0xffffffff,
	FALSE),
  HOWTO(R_X86_64_GLOB_DAT, 0, 4, 64, FALSE, 0, complain_overflow_bitfield,
	bfd_elf_generic_reloc, "R_X86_64_GLOB_DAT", FALSE, MINUS_ONE,
	MINUS_ONE, FALSE),
  HOWTO(R_X86_64_JUMP_SLOT, 0, 4, 64, FALSE, 0, complain_overflow_bitfield,
	bfd_elf_generic_reloc, "R_X86_64_JUMP_SLOT", FALSE, MINUS_ONE,
	MINUS_ONE, FALSE),
  HOWTO(R_X86_64_RELATIVE, 0, 4, 64, FALSE, 0, complain_overflow_bitfield,
	bfd_elf_generic_reloc, "R_X86_64_RELATIVE", FALSE, MINUS_ONE,
	MINUS_ONE, FALSE),
  HOWTO(R_X86_64_GOTPCREL, 0, 2, 32, TRUE, 0, complain_overflow_signed,
	bfd_elf_generic_reloc, "R_X86_64_GOTPCREL", FALSE, 0xffffffff,
	0xffffffff, TRUE),
  HOWTO(R_X86_64_32, 0, 2, 32, FALSE, 0, complain_overflow_unsigned,
	bfd_elf_generic_reloc, "R_X86_64_32", FALSE, 0xffffffff, 0xffffffff,
	FALSE),
  HOWTO(R_X86_64_32S, 0, 2, 32, FALSE, 0, complain_overflow_signed,
	bfd_elf_generic_reloc, "R_X86_64_32S", FALSE, 0xffffffff, 0xffffffff,
	FALSE),
  HOWTO(R_X86_64_16, 0, 1, 16, FALSE, 0, complain_overflow_bitfield,
	bfd_elf_generic_reloc, "R_X86_64_16", FALSE, 0xffff, 0xffff, FALSE),
  HOWTO(R_X86_64_PC16,0, 1, 16, TRUE, 0, complain_overflow_bitfield,
	bfd_elf_generic_reloc, "R_X86_64_PC16", FALSE, 0xffff, 0xffff, TRUE),
  HOWTO(R_X86_64_8, 0, 0, 8, FALSE, 0, complain_overflow_bitfield,
	bfd_elf_generic_reloc, "R_X86_64_8", FALSE, 0xff, 0xff, FALSE),
  HOWTO(R_X86_64_PC8, 0, 0, 8, TRUE, 0, complain_overflow_signed,
	bfd_elf_generic_reloc, "R_X86_64_PC8", FALSE, 0xff, 0xff, TRUE),
  HOWTO(R_X86_64_DTPMOD64, 0, 4, 64, FALSE, 0, complain_overflow_bitfield,
	bfd_elf_generic_reloc, "R_X86_64_DTPMOD64", FALSE, MINUS_ONE,
	MINUS_ONE, FALSE),
  HOWTO(R_X86_64_DTPOFF64, 0, 4, 64, FALSE, 0, complain_overflow_bitfield,
	bfd_elf_generic_reloc, "R_X86_64_DTPOFF64", FALSE, MINUS_ONE,
	MINUS_ONE, FALSE),
  HOWTO(R_X86_64_TPOFF64, 0, 4, 64, FALSE, 0, complain_overflow_bitfield,
	bfd_elf_generic_reloc, "R_X86_64_TPOFF64", FALSE, MINUS_ONE,
	MINUS_ONE, FALSE),
  HOWTO(R_X86_64_TLSGD, 0, 2, 32, TRUE, 0, complain_overflow_signed,
	bfd_elf_generic_reloc, "R_X86_64_TLSGD", FALSE, 0xffffffff,
	0xffffffff, TRUE),
  HOWTO(R_X86_64_TLSLD, 0, 2, 32, TRUE, 0, complain_overflow_signed,
	bfd_elf_generic_reloc, "R_X86_64_TLSLD", FALSE, 0xffffffff,
	0xffffffff, TRUE),
  HOWTO(R_X86_64_DTPOFF32, 0, 2, 32, FALSE, 0, complain_overflow_signed,
	bfd_elf_generic_reloc, "R_X86_64_DTPOFF32", FALSE, 0xffffffff,
	0xffffffff, FALSE),
  HOWTO(R_X86_64_GOTTPOFF, 0, 2, 32, TRUE, 0, complain_overflow_signed,
	bfd_elf_generic_reloc, "R_X86_64_GOTTPOFF", FALSE, 0xffffffff,
	0xffffffff, TRUE),
  HOWTO(R_X86_64_TPOFF32, 0, 2, 32, FALSE, 0, complain_overflow_signed,
	bfd_elf_generic_reloc, "R_X86_64_TPOFF32", FALSE, 0xffffffff,
	0xffffffff, FALSE),
  HOWTO(R_X86_64_PC64, 0, 4, 64, TRUE, 0, complain_overflow_bitfield,
	bfd_elf_generic_reloc, "R_X86_64_PC64", FALSE, MINUS_ONE, MINUS_ONE,
	TRUE),
  HOWTO(R_X86_64_GOTOFF64, 0, 4, 64, FALSE, 0, complain_overflow_bitfield,
	bfd_elf_generic_reloc, "R_X86_64_GOTOFF64",
	FALSE, MINUS_ONE, MINUS_ONE, FALSE),
  HOWTO(R_X86_64_GOTPC32, 0, 2, 32, TRUE, 0, complain_overflow_signed,
	bfd_elf_generic_reloc, "R_X86_64_GOTPC32",
	FALSE, 0xffffffff, 0xffffffff, TRUE),
  HOWTO(R_X86_64_GOT64, 0, 4, 64, FALSE, 0, complain_overflow_signed,
	bfd_elf_generic_reloc, "R_X86_64_GOT64", FALSE, MINUS_ONE, MINUS_ONE,
	FALSE),
  HOWTO(R_X86_64_GOTPCREL64, 0, 4, 64, TRUE, 0, complain_overflow_signed,
	bfd_elf_generic_reloc, "R_X86_64_GOTPCREL64", FALSE, MINUS_ONE,
	MINUS_ONE, TRUE),
  HOWTO(R_X86_64_GOTPC64, 0, 4, 64, TRUE, 0, complain_overflow_signed,
	bfd_elf_generic_reloc, "R_X86_64_GOTPC64",
	FALSE, MINUS_ONE, MINUS_ONE, TRUE),
  HOWTO(R_X86_64_GOTPLT64, 0, 4, 64, FALSE, 0, complain_overflow_signed,
	bfd_elf_generic_reloc, "R_X86_64_GOTPLT64", FALSE, MINUS_ONE,
	MINUS_ONE, FALSE),
  HOWTO(R_X86_64_PLTOFF64, 0, 4, 64, FALSE, 0, complain_overflow_signed,
	bfd_elf_generic_reloc, "R_X86_64_PLTOFF64", FALSE, MINUS_ONE,
	MINUS_ONE, FALSE),
  HOWTO(R_X86_64_SIZE32, 0, 2, 32, FALSE, 0, complain_overflow_unsigned,
	bfd_elf_generic_reloc, "R_X86_64_SIZE32", FALSE, 0xffffffff, 0xffffffff,
	FALSE),
  HOWTO(R_X86_64_SIZE64, 0, 4, 64, FALSE, 0, complain_overflow_unsigned,
	bfd_elf_generic_reloc, "R_X86_64_SIZE64", FALSE, MINUS_ONE, MINUS_ONE,
	FALSE),
  HOWTO(R_X86_64_GOTPC32_TLSDESC, 0, 2, 32, TRUE, 0,
	complain_overflow_bitfield, bfd_elf_generic_reloc,
	"R_X86_64_GOTPC32_TLSDESC",
	FALSE, 0xffffffff, 0xffffffff, TRUE),
  HOWTO(R_X86_64_TLSDESC_CALL, 0, 0, 0, FALSE, 0,
	complain_overflow_dont, bfd_elf_generic_reloc,
	"R_X86_64_TLSDESC_CALL",
	FALSE, 0, 0, FALSE),
  HOWTO(R_X86_64_TLSDESC, 0, 4, 64, FALSE, 0,
	complain_overflow_bitfield, bfd_elf_generic_reloc,
	"R_X86_64_TLSDESC",
	FALSE, MINUS_ONE, MINUS_ONE, FALSE),
  HOWTO(R_X86_64_IRELATIVE, 0, 4, 64, FALSE, 0, complain_overflow_bitfield,
	bfd_elf_generic_reloc, "R_X86_64_IRELATIVE", FALSE, MINUS_ONE,
	MINUS_ONE, FALSE),
  HOWTO(R_X86_64_RELATIVE64, 0, 4, 64, FALSE, 0, complain_overflow_bitfield,
	bfd_elf_generic_reloc, "R_X86_64_RELATIVE64", FALSE, MINUS_ONE,
	MINUS_ONE, FALSE),
  HOWTO(R_X86_64_PC32_BND, 0, 2, 32, TRUE, 0, complain_overflow_signed,
	bfd_elf_generic_reloc, "R_X86_64_PC32_BND", FALSE, 0xffffffff, 0xffffffff,
	TRUE),
  HOWTO(R_X86_64_PLT32_BND, 0, 2, 32, TRUE, 0, complain_overflow_signed,
	bfd_elf_generic_reloc, "R_X86_64_PLT32_BND", FALSE, 0xffffffff, 0xffffffff,
	TRUE),
  HOWTO(R_X86_64_GOTPCRELX, 0, 2, 32, TRUE, 0, complain_overflow_signed,
	bfd_elf_generic_reloc, "R_X86_64_GOTPCRELX", FALSE, 0xffffffff,
	0xffffffff, TRUE),
  HOWTO(R_X86_64_REX_GOTPCRELX, 0, 2, 32, TRUE, 0, complain_overflow_signed,
	bfd_elf_generic_reloc, "R_X86_64_REX_GOTPCRELX", FALSE, 0xffffffff,
	0xffffffff, TRUE),

  /* We have a gap in the reloc numbers here.
     R_X86_64_standard counts the number up to this point, and
     R_X86_64_vt_offset is the value to subtract from a reloc type of
     R_X86_64_GNU_VT* to form an index into this table.  */
#define R_X86_64_standard (R_X86_64_REX_GOTPCRELX + 1)
#define R_X86_64_vt_offset (R_X86_64_GNU_VTINHERIT - R_X86_64_standard)

/* GNU extension to record C++ vtable hierarchy.  */
  HOWTO (R_X86_64_GNU_VTINHERIT, 0, 4, 0, FALSE, 0, complain_overflow_dont,
	 NULL, "R_X86_64_GNU_VTINHERIT", FALSE, 0, 0, FALSE),

/* GNU extension to record C++ vtable member usage.  */
  HOWTO (R_X86_64_GNU_VTENTRY, 0, 4, 0, FALSE, 0, complain_overflow_dont,
	 _bfd_elf_rel_vtable_reloc_fn, "R_X86_64_GNU_VTENTRY", FALSE, 0, 0,
	 FALSE),

/* Use complain_overflow_bitfield on R_X86_64_32 for x32.  */
  HOWTO(R_X86_64_32, 0, 2, 32, FALSE, 0, complain_overflow_bitfield,
	bfd_elf_generic_reloc, "R_X86_64_32", FALSE, 0xffffffff, 0xffffffff,
	FALSE)
};

/* Set if a relocation is converted from a GOTPCREL relocation.  */
#define R_X86_64_converted_reloc_bit (1 << 7)

#define X86_PCREL_TYPE_P(TYPE)		\
  (   ((TYPE) == R_X86_64_PC8)		\
   || ((TYPE) == R_X86_64_PC16)		\
   || ((TYPE) == R_X86_64_PC32)		\
   || ((TYPE) == R_X86_64_PC32_BND)	\
   || ((TYPE) == R_X86_64_PC64))

#define X86_SIZE_TYPE_P(TYPE)		\
  ((TYPE) == R_X86_64_SIZE32 || (TYPE) == R_X86_64_SIZE64)

/* Map BFD relocs to the x86_64 elf relocs.  */
struct elf_reloc_map
{
  bfd_reloc_code_real_type bfd_reloc_val;
  unsigned char elf_reloc_val;
};

static const struct elf_reloc_map x86_64_reloc_map[] =
{
  { BFD_RELOC_NONE,		R_X86_64_NONE, },
  { BFD_RELOC_64,		R_X86_64_64,   },
  { BFD_RELOC_32_PCREL,		R_X86_64_PC32, },
  { BFD_RELOC_X86_64_GOT32,	R_X86_64_GOT32,},
  { BFD_RELOC_X86_64_PLT32,	R_X86_64_PLT32,},
  { BFD_RELOC_X86_64_COPY,	R_X86_64_COPY, },
  { BFD_RELOC_X86_64_GLOB_DAT,	R_X86_64_GLOB_DAT, },
  { BFD_RELOC_X86_64_JUMP_SLOT, R_X86_64_JUMP_SLOT, },
  { BFD_RELOC_X86_64_RELATIVE,	R_X86_64_RELATIVE, },
  { BFD_RELOC_X86_64_GOTPCREL,	R_X86_64_GOTPCREL, },
  { BFD_RELOC_32,		R_X86_64_32, },
  { BFD_RELOC_X86_64_32S,	R_X86_64_32S, },
  { BFD_RELOC_16,		R_X86_64_16, },
  { BFD_RELOC_16_PCREL,		R_X86_64_PC16, },
  { BFD_RELOC_8,		R_X86_64_8, },
  { BFD_RELOC_8_PCREL,		R_X86_64_PC8, },
  { BFD_RELOC_X86_64_DTPMOD64,	R_X86_64_DTPMOD64, },
  { BFD_RELOC_X86_64_DTPOFF64,	R_X86_64_DTPOFF64, },
  { BFD_RELOC_X86_64_TPOFF64,	R_X86_64_TPOFF64, },
  { BFD_RELOC_X86_64_TLSGD,	R_X86_64_TLSGD, },
  { BFD_RELOC_X86_64_TLSLD,	R_X86_64_TLSLD, },
  { BFD_RELOC_X86_64_DTPOFF32,	R_X86_64_DTPOFF32, },
  { BFD_RELOC_X86_64_GOTTPOFF,	R_X86_64_GOTTPOFF, },
  { BFD_RELOC_X86_64_TPOFF32,	R_X86_64_TPOFF32, },
  { BFD_RELOC_64_PCREL,		R_X86_64_PC64, },
  { BFD_RELOC_X86_64_GOTOFF64,	R_X86_64_GOTOFF64, },
  { BFD_RELOC_X86_64_GOTPC32,	R_X86_64_GOTPC32, },
  { BFD_RELOC_X86_64_GOT64,	R_X86_64_GOT64, },
  { BFD_RELOC_X86_64_GOTPCREL64,R_X86_64_GOTPCREL64, },
  { BFD_RELOC_X86_64_GOTPC64,	R_X86_64_GOTPC64, },
  { BFD_RELOC_X86_64_GOTPLT64,	R_X86_64_GOTPLT64, },
  { BFD_RELOC_X86_64_PLTOFF64,	R_X86_64_PLTOFF64, },
  { BFD_RELOC_SIZE32,		R_X86_64_SIZE32, },
  { BFD_RELOC_SIZE64,		R_X86_64_SIZE64, },
  { BFD_RELOC_X86_64_GOTPC32_TLSDESC, R_X86_64_GOTPC32_TLSDESC, },
  { BFD_RELOC_X86_64_TLSDESC_CALL, R_X86_64_TLSDESC_CALL, },
  { BFD_RELOC_X86_64_TLSDESC,	R_X86_64_TLSDESC, },
  { BFD_RELOC_X86_64_IRELATIVE,	R_X86_64_IRELATIVE, },
  { BFD_RELOC_X86_64_PC32_BND,	R_X86_64_PC32_BND, },
  { BFD_RELOC_X86_64_PLT32_BND,	R_X86_64_PLT32_BND, },
  { BFD_RELOC_X86_64_GOTPCRELX, R_X86_64_GOTPCRELX, },
  { BFD_RELOC_X86_64_REX_GOTPCRELX, R_X86_64_REX_GOTPCRELX, },
  { BFD_RELOC_VTABLE_INHERIT,	R_X86_64_GNU_VTINHERIT, },
  { BFD_RELOC_VTABLE_ENTRY,	R_X86_64_GNU_VTENTRY, },
};

static reloc_howto_type *
elf_x86_64_rtype_to_howto (bfd *abfd, unsigned r_type)
{
  unsigned i;

  if (r_type == (unsigned int) R_X86_64_32)
    {
      if (ABI_64_P (abfd))
	i = r_type;
      else
	i = ARRAY_SIZE (x86_64_elf_howto_table) - 1;
    }
  else if (r_type < (unsigned int) R_X86_64_GNU_VTINHERIT
	   || r_type >= (unsigned int) R_X86_64_max)
    {
      if (r_type >= (unsigned int) R_X86_64_standard)
	{
	  /* xgettext:c-format */
	  _bfd_error_handler (_("%pB: unsupported relocation type %#x"),
			      abfd, r_type);
	  bfd_set_error (bfd_error_bad_value);
	  return NULL;
	}
      i = r_type;
    }
  else
    i = r_type - (unsigned int) R_X86_64_vt_offset;
  BFD_ASSERT (x86_64_elf_howto_table[i].type == r_type);
  return &x86_64_elf_howto_table[i];
}

/* Given a BFD reloc type, return a HOWTO structure.  */
static reloc_howto_type *
elf_x86_64_reloc_type_lookup (bfd *abfd,
			      bfd_reloc_code_real_type code)
{
  unsigned int i;

  for (i = 0; i < sizeof (x86_64_reloc_map) / sizeof (struct elf_reloc_map);
       i++)
    {
      if (x86_64_reloc_map[i].bfd_reloc_val == code)
	return elf_x86_64_rtype_to_howto (abfd,
					  x86_64_reloc_map[i].elf_reloc_val);
    }
  return NULL;
}

static reloc_howto_type *
elf_x86_64_reloc_name_lookup (bfd *abfd,
			      const char *r_name)
{
  unsigned int i;

  if (!ABI_64_P (abfd) && strcasecmp (r_name, "R_X86_64_32") == 0)
    {
      /* Get x32 R_X86_64_32.  */
      reloc_howto_type *reloc
	= &x86_64_elf_howto_table[ARRAY_SIZE (x86_64_elf_howto_table) - 1];
      BFD_ASSERT (reloc->type == (unsigned int) R_X86_64_32);
      return reloc;
    }

  for (i = 0; i < ARRAY_SIZE (x86_64_elf_howto_table); i++)
    if (x86_64_elf_howto_table[i].name != NULL
	&& strcasecmp (x86_64_elf_howto_table[i].name, r_name) == 0)
      return &x86_64_elf_howto_table[i];

  return NULL;
}

/* Given an x86_64 ELF reloc type, fill in an arelent structure.  */

static bfd_boolean
elf_x86_64_info_to_howto (bfd *abfd, arelent *cache_ptr,
			  Elf_Internal_Rela *dst)
{
  unsigned r_type;

  r_type = ELF32_R_TYPE (dst->r_info);
  cache_ptr->howto = elf_x86_64_rtype_to_howto (abfd, r_type);
  if (cache_ptr->howto == NULL)
    return FALSE;
  BFD_ASSERT (r_type == cache_ptr->howto->type || cache_ptr->howto->type == R_X86_64_NONE);
  return TRUE;
}

/* Support for core dump NOTE sections.  */
static bfd_boolean
elf_x86_64_grok_prstatus (bfd *abfd, Elf_Internal_Note *note)
{
  int offset;
  size_t size;

  switch (note->descsz)
    {
      default:
	return FALSE;

      case 296:		/* sizeof(istruct elf_prstatus) on Linux/x32 */
	/* pr_cursig */
	elf_tdata (abfd)->core->signal = bfd_get_16 (abfd, note->descdata + 12);

	/* pr_pid */
	elf_tdata (abfd)->core->lwpid = bfd_get_32 (abfd, note->descdata + 24);

	/* pr_reg */
	offset = 72;
	size = 216;

	break;

      case 336:		/* sizeof(istruct elf_prstatus) on Linux/x86_64 */
	/* pr_cursig */
	elf_tdata (abfd)->core->signal
	  = bfd_get_16 (abfd, note->descdata + 12);

	/* pr_pid */
	elf_tdata (abfd)->core->lwpid
	  = bfd_get_32 (abfd, note->descdata + 32);

	/* pr_reg */
	offset = 112;
	size = 216;

	break;
    }

  /* Make a ".reg/999" section.  */
  return _bfd_elfcore_make_pseudosection (abfd, ".reg",
					  size, note->descpos + offset);
}

static bfd_boolean
elf_x86_64_grok_psinfo (bfd *abfd, Elf_Internal_Note *note)
{
  switch (note->descsz)
    {
      default:
	return FALSE;

      case 124:		/* sizeof(struct elf_prpsinfo) on Linux/x32 */
	elf_tdata (abfd)->core->pid
	  = bfd_get_32 (abfd, note->descdata + 12);
	elf_tdata (abfd)->core->program
	  = _bfd_elfcore_strndup (abfd, note->descdata + 28, 16);
	elf_tdata (abfd)->core->command
	  = _bfd_elfcore_strndup (abfd, note->descdata + 44, 80);
	break;

      case 136:		/* sizeof(struct elf_prpsinfo) on Linux/x86_64 */
	elf_tdata (abfd)->core->pid
	  = bfd_get_32 (abfd, note->descdata + 24);
	elf_tdata (abfd)->core->program
	 = _bfd_elfcore_strndup (abfd, note->descdata + 40, 16);
	elf_tdata (abfd)->core->command
	 = _bfd_elfcore_strndup (abfd, note->descdata + 56, 80);
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

#ifdef CORE_HEADER
# if GCC_VERSION >= 8000
#  pragma GCC diagnostic push
#  pragma GCC diagnostic ignored "-Wstringop-truncation"
# endif
static char *
elf_x86_64_write_core_note (bfd *abfd, char *buf, int *bufsiz,
			    int note_type, ...)
{
  const struct elf_backend_data *bed = get_elf_backend_data (abfd);
  va_list ap;
  const char *fname, *psargs;
  long pid;
  int cursig;
  const void *gregs;

  switch (note_type)
    {
    default:
      return NULL;

    case NT_PRPSINFO:
      va_start (ap, note_type);
      fname = va_arg (ap, const char *);
      psargs = va_arg (ap, const char *);
      va_end (ap);

      if (bed->s->elfclass == ELFCLASS32)
	{
	  prpsinfo32_t data;
	  memset (&data, 0, sizeof (data));
	  strncpy (data.pr_fname, fname, sizeof (data.pr_fname));
	  strncpy (data.pr_psargs, psargs, sizeof (data.pr_psargs));
	  return elfcore_write_note (abfd, buf, bufsiz, "CORE", note_type,
				     &data, sizeof (data));
	}
      else
	{
	  prpsinfo64_t data;
	  memset (&data, 0, sizeof (data));
	  strncpy (data.pr_fname, fname, sizeof (data.pr_fname));
	  strncpy (data.pr_psargs, psargs, sizeof (data.pr_psargs));
	  return elfcore_write_note (abfd, buf, bufsiz, "CORE", note_type,
				     &data, sizeof (data));
	}
      /* NOTREACHED */

    case NT_PRSTATUS:
      va_start (ap, note_type);
      pid = va_arg (ap, long);
      cursig = va_arg (ap, int);
      gregs = va_arg (ap, const void *);
      va_end (ap);

      if (bed->s->elfclass == ELFCLASS32)
	{
	  if (bed->elf_machine_code == EM_X86_64)
	    {
	      prstatusx32_t prstat;
	      memset (&prstat, 0, sizeof (prstat));
	      prstat.pr_pid = pid;
	      prstat.pr_cursig = cursig;
	      memcpy (&prstat.pr_reg, gregs, sizeof (prstat.pr_reg));
	      return elfcore_write_note (abfd, buf, bufsiz, "CORE", note_type,
					 &prstat, sizeof (prstat));
	    }
	  else
	    {
	      prstatus32_t prstat;
	      memset (&prstat, 0, sizeof (prstat));
	      prstat.pr_pid = pid;
	      prstat.pr_cursig = cursig;
	      memcpy (&prstat.pr_reg, gregs, sizeof (prstat.pr_reg));
	      return elfcore_write_note (abfd, buf, bufsiz, "CORE", note_type,
					 &prstat, sizeof (prstat));
	    }
	}
      else
	{
	  prstatus64_t prstat;
	  memset (&prstat, 0, sizeof (prstat));
	  prstat.pr_pid = pid;
	  prstat.pr_cursig = cursig;
	  memcpy (&prstat.pr_reg, gregs, sizeof (prstat.pr_reg));
	  return elfcore_write_note (abfd, buf, bufsiz, "CORE", note_type,
				     &prstat, sizeof (prstat));
	}
    }
  /* NOTREACHED */
}
# if GCC_VERSION >= 8000
#  pragma GCC diagnostic pop
# endif
#endif

/* Functions for the x86-64 ELF linker.	 */

/* The size in bytes of an entry in the global offset table.  */

#define GOT_ENTRY_SIZE 8

/* The size in bytes of an entry in the lazy procedure linkage table.  */

#define LAZY_PLT_ENTRY_SIZE 16

/* The size in bytes of an entry in the non-lazy procedure linkage
   table.  */

#define NON_LAZY_PLT_ENTRY_SIZE 8

/* The first entry in a lazy procedure linkage table looks like this.
   See the SVR4 ABI i386 supplement and the x86-64 ABI to see how this
   works.  */

static const bfd_byte elf_x86_64_lazy_plt0_entry[LAZY_PLT_ENTRY_SIZE] =
{
  0xff, 0x35, 8, 0, 0, 0,	/* pushq GOT+8(%rip)  */
  0xff, 0x25, 16, 0, 0, 0,	/* jmpq *GOT+16(%rip) */
  0x0f, 0x1f, 0x40, 0x00	/* nopl 0(%rax)       */
};

/* Subsequent entries in a lazy procedure linkage table look like this.  */

static const bfd_byte elf_x86_64_lazy_plt_entry[LAZY_PLT_ENTRY_SIZE] =
{
  0xff, 0x25,	/* jmpq *name@GOTPC(%rip) */
  0, 0, 0, 0,	/* replaced with offset to this symbol in .got.	 */
  0x68,		/* pushq immediate */
  0, 0, 0, 0,	/* replaced with index into relocation table.  */
  0xe9,		/* jmp relative */
  0, 0, 0, 0	/* replaced with offset to start of .plt0.  */
};

/* The first entry in a lazy procedure linkage table with BND prefix
   like this.  */

static const bfd_byte elf_x86_64_lazy_bnd_plt0_entry[LAZY_PLT_ENTRY_SIZE] =
{
  0xff, 0x35, 8, 0, 0, 0,	  /* pushq GOT+8(%rip)	      */
  0xf2, 0xff, 0x25, 16, 0, 0, 0,  /* bnd jmpq *GOT+16(%rip)   */
  0x0f, 0x1f, 0			  /* nopl (%rax)	      */
};

/* Subsequent entries for branches with BND prefx in a lazy procedure
   linkage table look like this.  */

static const bfd_byte elf_x86_64_lazy_bnd_plt_entry[LAZY_PLT_ENTRY_SIZE] =
{
  0x68, 0, 0, 0, 0,		/* pushq immediate	      */
  0xf2, 0xe9, 0, 0, 0, 0,	/* bnd jmpq relative	      */
  0x0f, 0x1f, 0x44, 0, 0	/* nopl 0(%rax,%rax,1)	      */
};

/* The first entry in the IBT-enabled lazy procedure linkage table is the
   the same as the lazy PLT with BND prefix so that bound registers are
   preserved when control is passed to dynamic linker.  Subsequent
   entries for a IBT-enabled lazy procedure linkage table look like
   this.  */

static const bfd_byte elf_x86_64_lazy_ibt_plt_entry[LAZY_PLT_ENTRY_SIZE] =
{
  0xf3, 0x0f, 0x1e, 0xfa,	/* endbr64		      */
  0x68, 0, 0, 0, 0,		/* pushq immediate	      */
  0xf2, 0xe9, 0, 0, 0, 0,	/* bnd jmpq relative	      */
  0x90				/* nop			      */
};

/* The first entry in the x32 IBT-enabled lazy procedure linkage table
   is the same as the normal lazy PLT.  Subsequent entries for an
   x32 IBT-enabled lazy procedure linkage table look like this.  */

static const bfd_byte elf_x32_lazy_ibt_plt_entry[LAZY_PLT_ENTRY_SIZE] =
{
  0xf3, 0x0f, 0x1e, 0xfa,	/* endbr64		      */
  0x68, 0, 0, 0, 0,		/* pushq immediate	      */
  0xe9, 0, 0, 0, 0,		/* jmpq relative	      */
  0x66, 0x90			/* xchg %ax,%ax		      */
};

/* Entries in the non-lazey procedure linkage table look like this.  */

static const bfd_byte elf_x86_64_non_lazy_plt_entry[NON_LAZY_PLT_ENTRY_SIZE] =
{
  0xff, 0x25,	     /* jmpq *name@GOTPC(%rip)			      */
  0, 0, 0, 0,	     /* replaced with offset to this symbol in .got.  */
  0x66, 0x90	     /* xchg %ax,%ax				      */
};

/* Entries for branches with BND prefix in the non-lazey procedure
   linkage table look like this.  */

static const bfd_byte elf_x86_64_non_lazy_bnd_plt_entry[NON_LAZY_PLT_ENTRY_SIZE] =
{
  0xf2, 0xff, 0x25,  /* bnd jmpq *name@GOTPC(%rip)		      */
  0, 0, 0, 0,	     /* replaced with offset to this symbol in .got.  */
  0x90		     /* nop					      */
};

/* Entries for branches with IBT-enabled in the non-lazey procedure
   linkage table look like this.  They have the same size as the lazy
   PLT entry.  */

static const bfd_byte elf_x86_64_non_lazy_ibt_plt_entry[LAZY_PLT_ENTRY_SIZE] =
{
  0xf3, 0x0f, 0x1e, 0xfa,	/* endbr64		       */
  0xf2, 0xff, 0x25,		/* bnd jmpq *name@GOTPC(%rip)  */
  0, 0, 0, 0,  /* replaced with offset to this symbol in .got. */
  0x0f, 0x1f, 0x44, 0x00, 0x00	/* nopl 0x0(%rax,%rax,1)       */
};

/* Entries for branches with IBT-enabled in the x32 non-lazey procedure
   linkage table look like this.  They have the same size as the lazy
   PLT entry.  */

static const bfd_byte elf_x32_non_lazy_ibt_plt_entry[LAZY_PLT_ENTRY_SIZE] =
{
  0xf3, 0x0f, 0x1e, 0xfa,	     /* endbr64		       */
  0xff, 0x25,			     /* jmpq *name@GOTPC(%rip) */
  0, 0, 0, 0,  /* replaced with offset to this symbol in .got. */
  0x66, 0x0f, 0x1f, 0x44, 0x00, 0x00 /* nopw 0x0(%rax,%rax,1)  */
};

/* The TLSDESC entry in a lazy procedure linkage table.  */
static const bfd_byte elf_x86_64_tlsdesc_plt_entry[LAZY_PLT_ENTRY_SIZE] =
{
  0xf3, 0x0f, 0x1e, 0xfa,	     /* endbr64		       */
  0xff, 0x35, 8, 0, 0, 0,	     /* pushq GOT+8(%rip)	*/
  0xff, 0x25, 16, 0, 0, 0	     /* jmpq *GOT+TDG(%rip)	*/
};

/* .eh_frame covering the lazy .plt section.  */

static const bfd_byte elf_x86_64_eh_frame_lazy_plt[] =
{
  PLT_CIE_LENGTH, 0, 0, 0,	/* CIE length */
  0, 0, 0, 0,			/* CIE ID */
  1,				/* CIE version */
  'z', 'R', 0,			/* Augmentation string */
  1,				/* Code alignment factor */
  0x78,				/* Data alignment factor */
  16,				/* Return address column */
  1,				/* Augmentation size */
  DW_EH_PE_pcrel | DW_EH_PE_sdata4, /* FDE encoding */
  DW_CFA_def_cfa, 7, 8,		/* DW_CFA_def_cfa: r7 (rsp) ofs 8 */
  DW_CFA_offset + 16, 1,	/* DW_CFA_offset: r16 (rip) at cfa-8 */
  DW_CFA_nop, DW_CFA_nop,

  PLT_FDE_LENGTH, 0, 0, 0,	/* FDE length */
  PLT_CIE_LENGTH + 8, 0, 0, 0,	/* CIE pointer */
  0, 0, 0, 0,			/* R_X86_64_PC32 .plt goes here */
  0, 0, 0, 0,			/* .plt size goes here */
  0,				/* Augmentation size */
  DW_CFA_def_cfa_offset, 16,	/* DW_CFA_def_cfa_offset: 16 */
  DW_CFA_advance_loc + 6,	/* DW_CFA_advance_loc: 6 to __PLT__+6 */
  DW_CFA_def_cfa_offset, 24,	/* DW_CFA_def_cfa_offset: 24 */
  DW_CFA_advance_loc + 10,	/* DW_CFA_advance_loc: 10 to __PLT__+16 */
  DW_CFA_def_cfa_expression,	/* DW_CFA_def_cfa_expression */
  11,				/* Block length */
  DW_OP_breg7, 8,		/* DW_OP_breg7 (rsp): 8 */
  DW_OP_breg16, 0,		/* DW_OP_breg16 (rip): 0 */
  DW_OP_lit15, DW_OP_and, DW_OP_lit11, DW_OP_ge,
  DW_OP_lit3, DW_OP_shl, DW_OP_plus,
  DW_CFA_nop, DW_CFA_nop, DW_CFA_nop, DW_CFA_nop
};

/* .eh_frame covering the lazy BND .plt section.  */

static const bfd_byte elf_x86_64_eh_frame_lazy_bnd_plt[] =
{
  PLT_CIE_LENGTH, 0, 0, 0,	/* CIE length */
  0, 0, 0, 0,			/* CIE ID */
  1,				/* CIE version */
  'z', 'R', 0,			/* Augmentation string */
  1,				/* Code alignment factor */
  0x78,				/* Data alignment factor */
  16,				/* Return address column */
  1,				/* Augmentation size */
  DW_EH_PE_pcrel | DW_EH_PE_sdata4, /* FDE encoding */
  DW_CFA_def_cfa, 7, 8,		/* DW_CFA_def_cfa: r7 (rsp) ofs 8 */
  DW_CFA_offset + 16, 1,	/* DW_CFA_offset: r16 (rip) at cfa-8 */
  DW_CFA_nop, DW_CFA_nop,

  PLT_FDE_LENGTH, 0, 0, 0,	/* FDE length */
  PLT_CIE_LENGTH + 8, 0, 0, 0,	/* CIE pointer */
  0, 0, 0, 0,			/* R_X86_64_PC32 .plt goes here */
  0, 0, 0, 0,			/* .plt size goes here */
  0,				/* Augmentation size */
  DW_CFA_def_cfa_offset, 16,	/* DW_CFA_def_cfa_offset: 16 */
  DW_CFA_advance_loc + 6,	/* DW_CFA_advance_loc: 6 to __PLT__+6 */
  DW_CFA_def_cfa_offset, 24,	/* DW_CFA_def_cfa_offset: 24 */
  DW_CFA_advance_loc + 10,	/* DW_CFA_advance_loc: 10 to __PLT__+16 */
  DW_CFA_def_cfa_expression,	/* DW_CFA_def_cfa_expression */
  11,				/* Block length */
  DW_OP_breg7, 8,		/* DW_OP_breg7 (rsp): 8 */
  DW_OP_breg16, 0,		/* DW_OP_breg16 (rip): 0 */
  DW_OP_lit15, DW_OP_and, DW_OP_lit5, DW_OP_ge,
  DW_OP_lit3, DW_OP_shl, DW_OP_plus,
  DW_CFA_nop, DW_CFA_nop, DW_CFA_nop, DW_CFA_nop
};

/* .eh_frame covering the lazy .plt section with IBT-enabled.  */

static const bfd_byte elf_x86_64_eh_frame_lazy_ibt_plt[] =
{
  PLT_CIE_LENGTH, 0, 0, 0,	/* CIE length */
  0, 0, 0, 0,			/* CIE ID */
  1,				/* CIE version */
  'z', 'R', 0,			/* Augmentation string */
  1,				/* Code alignment factor */
  0x78,				/* Data alignment factor */
  16,				/* Return address column */
  1,				/* Augmentation size */
  DW_EH_PE_pcrel | DW_EH_PE_sdata4, /* FDE encoding */
  DW_CFA_def_cfa, 7, 8,		/* DW_CFA_def_cfa: r7 (rsp) ofs 8 */
  DW_CFA_offset + 16, 1,	/* DW_CFA_offset: r16 (rip) at cfa-8 */
  DW_CFA_nop, DW_CFA_nop,

  PLT_FDE_LENGTH, 0, 0, 0,	/* FDE length */
  PLT_CIE_LENGTH + 8, 0, 0, 0,	/* CIE pointer */
  0, 0, 0, 0,			/* R_X86_64_PC32 .plt goes here */
  0, 0, 0, 0,			/* .plt size goes here */
  0,				/* Augmentation size */
  DW_CFA_def_cfa_offset, 16,	/* DW_CFA_def_cfa_offset: 16 */
  DW_CFA_advance_loc + 6,	/* DW_CFA_advance_loc: 6 to __PLT__+6 */
  DW_CFA_def_cfa_offset, 24,	/* DW_CFA_def_cfa_offset: 24 */
  DW_CFA_advance_loc + 10,	/* DW_CFA_advance_loc: 10 to __PLT__+16 */
  DW_CFA_def_cfa_expression,	/* DW_CFA_def_cfa_expression */
  11,				/* Block length */
  DW_OP_breg7, 8,		/* DW_OP_breg7 (rsp): 8 */
  DW_OP_breg16, 0,		/* DW_OP_breg16 (rip): 0 */
  DW_OP_lit15, DW_OP_and, DW_OP_lit10, DW_OP_ge,
  DW_OP_lit3, DW_OP_shl, DW_OP_plus,
  DW_CFA_nop, DW_CFA_nop, DW_CFA_nop, DW_CFA_nop
};

/* .eh_frame covering the x32 lazy .plt section with IBT-enabled.  */

static const bfd_byte elf_x32_eh_frame_lazy_ibt_plt[] =
{
  PLT_CIE_LENGTH, 0, 0, 0,	/* CIE length */
  0, 0, 0, 0,			/* CIE ID */
  1,				/* CIE version */
  'z', 'R', 0,			/* Augmentation string */
  1,				/* Code alignment factor */
  0x78,				/* Data alignment factor */
  16,				/* Return address column */
  1,				/* Augmentation size */
  DW_EH_PE_pcrel | DW_EH_PE_sdata4, /* FDE encoding */
  DW_CFA_def_cfa, 7, 8,		/* DW_CFA_def_cfa: r7 (rsp) ofs 8 */
  DW_CFA_offset + 16, 1,	/* DW_CFA_offset: r16 (rip) at cfa-8 */
  DW_CFA_nop, DW_CFA_nop,

  PLT_FDE_LENGTH, 0, 0, 0,	/* FDE length */
  PLT_CIE_LENGTH + 8, 0, 0, 0,	/* CIE pointer */
  0, 0, 0, 0,			/* R_X86_64_PC32 .plt goes here */
  0, 0, 0, 0,			/* .plt size goes here */
  0,				/* Augmentation size */
  DW_CFA_def_cfa_offset, 16,	/* DW_CFA_def_cfa_offset: 16 */
  DW_CFA_advance_loc + 6,	/* DW_CFA_advance_loc: 6 to __PLT__+6 */
  DW_CFA_def_cfa_offset, 24,	/* DW_CFA_def_cfa_offset: 24 */
  DW_CFA_advance_loc + 10,	/* DW_CFA_advance_loc: 10 to __PLT__+16 */
  DW_CFA_def_cfa_expression,	/* DW_CFA_def_cfa_expression */
  11,				/* Block length */
  DW_OP_breg7, 8,		/* DW_OP_breg7 (rsp): 8 */
  DW_OP_breg16, 0,		/* DW_OP_breg16 (rip): 0 */
  DW_OP_lit15, DW_OP_and, DW_OP_lit9, DW_OP_ge,
  DW_OP_lit3, DW_OP_shl, DW_OP_plus,
  DW_CFA_nop, DW_CFA_nop, DW_CFA_nop, DW_CFA_nop
};

/* .eh_frame covering the non-lazy .plt section.  */

static const bfd_byte elf_x86_64_eh_frame_non_lazy_plt[] =
{
#define PLT_GOT_FDE_LENGTH		20
  PLT_CIE_LENGTH, 0, 0, 0,	/* CIE length */
  0, 0, 0, 0,			/* CIE ID */
  1,				/* CIE version */
  'z', 'R', 0,			/* Augmentation string */
  1,				/* Code alignment factor */
  0x78,				/* Data alignment factor */
  16,				/* Return address column */
  1,				/* Augmentation size */
  DW_EH_PE_pcrel | DW_EH_PE_sdata4, /* FDE encoding */
  DW_CFA_def_cfa, 7, 8,		/* DW_CFA_def_cfa: r7 (rsp) ofs 8 */
  DW_CFA_offset + 16, 1,	/* DW_CFA_offset: r16 (rip) at cfa-8 */
  DW_CFA_nop, DW_CFA_nop,

  PLT_GOT_FDE_LENGTH, 0, 0, 0,	/* FDE length */
  PLT_CIE_LENGTH + 8, 0, 0, 0,	/* CIE pointer */
  0, 0, 0, 0,			/* the start of non-lazy .plt goes here */
  0, 0, 0, 0,			/* non-lazy .plt size goes here */
  0,				/* Augmentation size */
  DW_CFA_nop, DW_CFA_nop, DW_CFA_nop, DW_CFA_nop,
  DW_CFA_nop, DW_CFA_nop, DW_CFA_nop
};

/* These are the standard parameters.  */
static const struct elf_x86_lazy_plt_layout elf_x86_64_lazy_plt =
  {
    elf_x86_64_lazy_plt0_entry,		/* plt0_entry */
    LAZY_PLT_ENTRY_SIZE,		/* plt0_entry_size */
    elf_x86_64_lazy_plt_entry,		/* plt_entry */
    LAZY_PLT_ENTRY_SIZE,		/* plt_entry_size */
    elf_x86_64_tlsdesc_plt_entry,	/* plt_tlsdesc_entry */
    LAZY_PLT_ENTRY_SIZE,		/* plt_tlsdesc_entry_size */
    6,					/* plt_tlsdesc_got1_offset */
    12,					/* plt_tlsdesc_got2_offset */
    10,					/* plt_tlsdesc_got1_insn_end */
    16,					/* plt_tlsdesc_got2_insn_end */
    2,					/* plt0_got1_offset */
    8,					/* plt0_got2_offset */
    12,					/* plt0_got2_insn_end */
    2,					/* plt_got_offset */
    7,					/* plt_reloc_offset */
    12,					/* plt_plt_offset */
    6,					/* plt_got_insn_size */
    LAZY_PLT_ENTRY_SIZE,		/* plt_plt_insn_end */
    6,					/* plt_lazy_offset */
    elf_x86_64_lazy_plt0_entry,		/* pic_plt0_entry */
    elf_x86_64_lazy_plt_entry,		/* pic_plt_entry */
    elf_x86_64_eh_frame_lazy_plt,	/* eh_frame_plt */
    sizeof (elf_x86_64_eh_frame_lazy_plt) /* eh_frame_plt_size */
  };

static const struct elf_x86_non_lazy_plt_layout elf_x86_64_non_lazy_plt =
  {
    elf_x86_64_non_lazy_plt_entry,	/* plt_entry */
    elf_x86_64_non_lazy_plt_entry,	/* pic_plt_entry */
    NON_LAZY_PLT_ENTRY_SIZE,		/* plt_entry_size */
    2,					/* plt_got_offset */
    6,					/* plt_got_insn_size */
    elf_x86_64_eh_frame_non_lazy_plt,	/* eh_frame_plt */
    sizeof (elf_x86_64_eh_frame_non_lazy_plt) /* eh_frame_plt_size */
  };

static const struct elf_x86_lazy_plt_layout elf_x86_64_lazy_bnd_plt =
  {
    elf_x86_64_lazy_bnd_plt0_entry,	/* plt0_entry */
    LAZY_PLT_ENTRY_SIZE,		/* plt0_entry_size */
    elf_x86_64_lazy_bnd_plt_entry,	/* plt_entry */
    LAZY_PLT_ENTRY_SIZE,		/* plt_entry_size */
    elf_x86_64_tlsdesc_plt_entry,	/* plt_tlsdesc_entry */
    LAZY_PLT_ENTRY_SIZE,		/* plt_tlsdesc_entry_size */
    6,					/* plt_tlsdesc_got1_offset */
    12,					/* plt_tlsdesc_got2_offset */
    10,					/* plt_tlsdesc_got1_insn_end */
    16,					/* plt_tlsdesc_got2_insn_end */
    2,					/* plt0_got1_offset */
    1+8,				/* plt0_got2_offset */
    1+12,				/* plt0_got2_insn_end */
    1+2,				/* plt_got_offset */
    1,					/* plt_reloc_offset */
    7,					/* plt_plt_offset */
    1+6,				/* plt_got_insn_size */
    11,					/* plt_plt_insn_end */
    0,					/* plt_lazy_offset */
    elf_x86_64_lazy_bnd_plt0_entry,	/* pic_plt0_entry */
    elf_x86_64_lazy_bnd_plt_entry,	/* pic_plt_entry */
    elf_x86_64_eh_frame_lazy_bnd_plt,	/* eh_frame_plt */
    sizeof (elf_x86_64_eh_frame_lazy_bnd_plt) /* eh_frame_plt_size */
  };

static const struct elf_x86_non_lazy_plt_layout elf_x86_64_non_lazy_bnd_plt =
  {
    elf_x86_64_non_lazy_bnd_plt_entry,	/* plt_entry */
    elf_x86_64_non_lazy_bnd_plt_entry,	/* pic_plt_entry */
    NON_LAZY_PLT_ENTRY_SIZE,		/* plt_entry_size */
    1+2,				/* plt_got_offset */
    1+6,				/* plt_got_insn_size */
    elf_x86_64_eh_frame_non_lazy_plt,	/* eh_frame_plt */
    sizeof (elf_x86_64_eh_frame_non_lazy_plt) /* eh_frame_plt_size */
  };

static const struct elf_x86_lazy_plt_layout elf_x86_64_lazy_ibt_plt =
  {
    elf_x86_64_lazy_bnd_plt0_entry,	/* plt0_entry */
    LAZY_PLT_ENTRY_SIZE,		/* plt0_entry_size */
    elf_x86_64_lazy_ibt_plt_entry,	/* plt_entry */
    LAZY_PLT_ENTRY_SIZE,		/* plt_entry_size */
    elf_x86_64_tlsdesc_plt_entry,	/* plt_tlsdesc_entry */
    LAZY_PLT_ENTRY_SIZE,		/* plt_tlsdesc_entry_size */
    6,					/* plt_tlsdesc_got1_offset */
    12,					/* plt_tlsdesc_got2_offset */
    10,					/* plt_tlsdesc_got1_insn_end */
    16,					/* plt_tlsdesc_got2_insn_end */
    2,					/* plt0_got1_offset */
    1+8,				/* plt0_got2_offset */
    1+12,				/* plt0_got2_insn_end */
    4+1+2,				/* plt_got_offset */
    4+1,				/* plt_reloc_offset */
    4+1+6,				/* plt_plt_offset */
    4+1+6,				/* plt_got_insn_size */
    4+1+5+5,				/* plt_plt_insn_end */
    0,					/* plt_lazy_offset */
    elf_x86_64_lazy_bnd_plt0_entry,	/* pic_plt0_entry */
    elf_x86_64_lazy_ibt_plt_entry,	/* pic_plt_entry */
    elf_x86_64_eh_frame_lazy_ibt_plt,	/* eh_frame_plt */
    sizeof (elf_x86_64_eh_frame_lazy_ibt_plt) /* eh_frame_plt_size */
  };

static const struct elf_x86_lazy_plt_layout elf_x32_lazy_ibt_plt =
  {
    elf_x86_64_lazy_plt0_entry,		/* plt0_entry */
    LAZY_PLT_ENTRY_SIZE,		/* plt0_entry_size */
    elf_x32_lazy_ibt_plt_entry,		/* plt_entry */
    LAZY_PLT_ENTRY_SIZE,		/* plt_entry_size */
    elf_x86_64_tlsdesc_plt_entry,	/* plt_tlsdesc_entry */
    LAZY_PLT_ENTRY_SIZE,		/* plt_tlsdesc_entry_size */
    6,					/* plt_tlsdesc_got1_offset */
    12,					/* plt_tlsdesc_got2_offset */
    10,					/* plt_tlsdesc_got1_insn_end */
    16,					/* plt_tlsdesc_got2_insn_end */
    2,					/* plt0_got1_offset */
    8,					/* plt0_got2_offset */
    12,					/* plt0_got2_insn_end */
    4+2,				/* plt_got_offset */
    4+1,				/* plt_reloc_offset */
    4+6,				/* plt_plt_offset */
    4+6,				/* plt_got_insn_size */
    4+5+5,				/* plt_plt_insn_end */
    0,					/* plt_lazy_offset */
    elf_x86_64_lazy_plt0_entry,		/* pic_plt0_entry */
    elf_x32_lazy_ibt_plt_entry,		/* pic_plt_entry */
    elf_x32_eh_frame_lazy_ibt_plt,	/* eh_frame_plt */
    sizeof (elf_x32_eh_frame_lazy_ibt_plt) /* eh_frame_plt_size */
  };

static const struct elf_x86_non_lazy_plt_layout elf_x86_64_non_lazy_ibt_plt =
  {
    elf_x86_64_non_lazy_ibt_plt_entry,	/* plt_entry */
    elf_x86_64_non_lazy_ibt_plt_entry,	/* pic_plt_entry */
    LAZY_PLT_ENTRY_SIZE,		/* plt_entry_size */
    4+1+2,				/* plt_got_offset */
    4+1+6,				/* plt_got_insn_size */
    elf_x86_64_eh_frame_non_lazy_plt,	/* eh_frame_plt */
    sizeof (elf_x86_64_eh_frame_non_lazy_plt) /* eh_frame_plt_size */
  };

static const struct elf_x86_non_lazy_plt_layout elf_x32_non_lazy_ibt_plt =
  {
    elf_x32_non_lazy_ibt_plt_entry,	/* plt_entry */
    elf_x32_non_lazy_ibt_plt_entry,	/* pic_plt_entry */
    LAZY_PLT_ENTRY_SIZE,		/* plt_entry_size */
    4+2,				/* plt_got_offset */
    4+6,				/* plt_got_insn_size */
    elf_x86_64_eh_frame_non_lazy_plt,	/* eh_frame_plt */
    sizeof (elf_x86_64_eh_frame_non_lazy_plt) /* eh_frame_plt_size */
  };

static const struct elf_x86_backend_data elf_x86_64_arch_bed =
  {
    is_normal				 /* os */
  };

#define	elf_backend_arch_data	&elf_x86_64_arch_bed

static bfd_boolean
elf64_x86_64_elf_object_p (bfd *abfd)
{
  /* Set the right machine number for an x86-64 elf64 file.  */
  bfd_default_set_arch_mach (abfd, bfd_arch_i386, bfd_mach_x86_64);
  return TRUE;
}

/* Return TRUE if the TLS access code sequence support transition
   from R_TYPE.  */

static bfd_boolean
elf_x86_64_check_tls_transition (bfd *abfd,
				 struct bfd_link_info *info,
				 asection *sec,
				 bfd_byte *contents,
				 Elf_Internal_Shdr *symtab_hdr,
				 struct elf_link_hash_entry **sym_hashes,
				 unsigned int r_type,
				 const Elf_Internal_Rela *rel,
				 const Elf_Internal_Rela *relend)
{
  unsigned int val;
  unsigned long r_symndx;
  bfd_boolean largepic = FALSE;
  struct elf_link_hash_entry *h;
  bfd_vma offset;
  struct elf_x86_link_hash_table *htab;
  bfd_byte *call;
  bfd_boolean indirect_call;

  htab = elf_x86_hash_table (info, X86_64_ELF_DATA);
  offset = rel->r_offset;
  switch (r_type)
    {
    case R_X86_64_TLSGD:
    case R_X86_64_TLSLD:
      if ((rel + 1) >= relend)
	return FALSE;

      if (r_type == R_X86_64_TLSGD)
	{
	  /* Check transition from GD access model.  For 64bit, only
		.byte 0x66; leaq foo@tlsgd(%rip), %rdi
		.word 0x6666; rex64; call __tls_get_addr@PLT
	     or
		.byte 0x66; leaq foo@tlsgd(%rip), %rdi
		.byte 0x66; rex64
		call *__tls_get_addr@GOTPCREL(%rip)
		which may be converted to
		addr32 call __tls_get_addr
	     can transit to different access model.  For 32bit, only
		leaq foo@tlsgd(%rip), %rdi
		.word 0x6666; rex64; call __tls_get_addr@PLT
	     or
		leaq foo@tlsgd(%rip), %rdi
		.byte 0x66; rex64
		call *__tls_get_addr@GOTPCREL(%rip)
		which may be converted to
		addr32 call __tls_get_addr
	     can transit to different access model.  For largepic,
	     we also support:
		leaq foo@tlsgd(%rip), %rdi
		movabsq $__tls_get_addr@pltoff, %rax
		addq $r15, %rax
		call *%rax
	     or
		leaq foo@tlsgd(%rip), %rdi
		movabsq $__tls_get_addr@pltoff, %rax
		addq $rbx, %rax
		call *%rax  */

	  static const unsigned char leaq[] = { 0x66, 0x48, 0x8d, 0x3d };

	  if ((offset + 12) > sec->size)
	    return FALSE;

	  call = contents + offset + 4;
	  if (call[0] != 0x66
	      || !((call[1] == 0x48
		    && call[2] == 0xff
		    && call[3] == 0x15)
		   || (call[1] == 0x48
		       && call[2] == 0x67
		       && call[3] == 0xe8)
		   || (call[1] == 0x66
		       && call[2] == 0x48
		       && call[3] == 0xe8)))
	    {
	      if (!ABI_64_P (abfd)
		  || (offset + 19) > sec->size
		  || offset < 3
		  || memcmp (call - 7, leaq + 1, 3) != 0
		  || memcmp (call, "\x48\xb8", 2) != 0
		  || call[11] != 0x01
		  || call[13] != 0xff
		  || call[14] != 0xd0
		  || !((call[10] == 0x48 && call[12] == 0xd8)
		       || (call[10] == 0x4c && call[12] == 0xf8)))
		return FALSE;
	      largepic = TRUE;
	    }
	  else if (ABI_64_P (abfd))
	    {
	      if (offset < 4
		  || memcmp (contents + offset - 4, leaq, 4) != 0)
		return FALSE;
	    }
	  else
	    {
	      if (offset < 3
		  || memcmp (contents + offset - 3, leaq + 1, 3) != 0)
		return FALSE;
	    }
	  indirect_call = call[2] == 0xff;
	}
      else
	{
	  /* Check transition from LD access model.  Only
		leaq foo@tlsld(%rip), %rdi;
		call __tls_get_addr@PLT
	     or
		leaq foo@tlsld(%rip), %rdi;
		call *__tls_get_addr@GOTPCREL(%rip)
		which may be converted to
		addr32 call __tls_get_addr
	     can transit to different access model.  For largepic
	     we also support:
		leaq foo@tlsld(%rip), %rdi
		movabsq $__tls_get_addr@pltoff, %rax
		addq $r15, %rax
		call *%rax
	     or
		leaq foo@tlsld(%rip), %rdi
		movabsq $__tls_get_addr@pltoff, %rax
		addq $rbx, %rax
		call *%rax  */

	  static const unsigned char lea[] = { 0x48, 0x8d, 0x3d };

	  if (offset < 3 || (offset + 9) > sec->size)
	    return FALSE;

	  if (memcmp (contents + offset - 3, lea, 3) != 0)
	    return FALSE;

	  call = contents + offset + 4;
	  if (!(call[0] == 0xe8
		|| (call[0] == 0xff && call[1] == 0x15)
		|| (call[0] == 0x67 && call[1] == 0xe8)))
	    {
	      if (!ABI_64_P (abfd)
		  || (offset + 19) > sec->size
		  || memcmp (call, "\x48\xb8", 2) != 0
		  || call[11] != 0x01
		  || call[13] != 0xff
		  || call[14] != 0xd0
		  || !((call[10] == 0x48 && call[12] == 0xd8)
		       || (call[10] == 0x4c && call[12] == 0xf8)))
		return FALSE;
	      largepic = TRUE;
	    }
	  indirect_call = call[0] == 0xff;
	}

      r_symndx = htab->r_sym (rel[1].r_info);
      if (r_symndx < symtab_hdr->sh_info)
	return FALSE;

      h = sym_hashes[r_symndx - symtab_hdr->sh_info];
      if (h == NULL
	  || !((struct elf_x86_link_hash_entry *) h)->tls_get_addr)
	return FALSE;
      else
	{
	  r_type = (ELF32_R_TYPE (rel[1].r_info)
		    & ~R_X86_64_converted_reloc_bit);
	  if (largepic)
	    return r_type == R_X86_64_PLTOFF64;
	  else if (indirect_call)
	    return r_type == R_X86_64_GOTPCRELX;
	  else
	    return (r_type == R_X86_64_PC32 || r_type == R_X86_64_PLT32);
	}

    case R_X86_64_GOTTPOFF:
      /* Check transition from IE access model:
		mov foo@gottpoff(%rip), %reg
		add foo@gottpoff(%rip), %reg
       */

      /* Check REX prefix first.  */
      if (offset >= 3 && (offset + 4) <= sec->size)
	{
	  val = bfd_get_8 (abfd, contents + offset - 3);
	  if (val != 0x48 && val != 0x4c)
	    {
	      /* X32 may have 0x44 REX prefix or no REX prefix.  */
	      if (ABI_64_P (abfd))
		return FALSE;
	    }
	}
      else
	{
	  /* X32 may not have any REX prefix.  */
	  if (ABI_64_P (abfd))
	    return FALSE;
	  if (offset < 2 || (offset + 3) > sec->size)
	    return FALSE;
	}

      val = bfd_get_8 (abfd, contents + offset - 2);
      if (val != 0x8b && val != 0x03)
	return FALSE;

      val = bfd_get_8 (abfd, contents + offset - 1);
      return (val & 0xc7) == 5;

    case R_X86_64_GOTPC32_TLSDESC:
      /* Check transition from GDesc access model:
		leaq x@tlsdesc(%rip), %rax

	 Make sure it's a leaq adding rip to a 32-bit offset
	 into any register, although it's probably almost always
	 going to be rax.  */

      if (offset < 3 || (offset + 4) > sec->size)
	return FALSE;

      val = bfd_get_8 (abfd, contents + offset - 3);
      if ((val & 0xfb) != 0x48)
	return FALSE;

      if (bfd_get_8 (abfd, contents + offset - 2) != 0x8d)
	return FALSE;

      val = bfd_get_8 (abfd, contents + offset - 1);
      return (val & 0xc7) == 0x05;

    case R_X86_64_TLSDESC_CALL:
      /* Check transition from GDesc access model:
		call *x@tlsdesc(%rax)
       */
      if (offset + 2 <= sec->size)
	{
	  /* Make sure that it's a call *x@tlsdesc(%rax).  */
	  call = contents + offset;
	  return call[0] == 0xff && call[1] == 0x10;
	}

      return FALSE;

    default:
      abort ();
    }
}

/* Return TRUE if the TLS access transition is OK or no transition
   will be performed.  Update R_TYPE if there is a transition.  */

static bfd_boolean
elf_x86_64_tls_transition (struct bfd_link_info *info, bfd *abfd,
			   asection *sec, bfd_byte *contents,
			   Elf_Internal_Shdr *symtab_hdr,
			   struct elf_link_hash_entry **sym_hashes,
			   unsigned int *r_type, int tls_type,
			   const Elf_Internal_Rela *rel,
			   const Elf_Internal_Rela *relend,
			   struct elf_link_hash_entry *h,
			   unsigned long r_symndx,
			   bfd_boolean from_relocate_section)
{
  unsigned int from_type = *r_type;
  unsigned int to_type = from_type;
  bfd_boolean check = TRUE;

  /* Skip TLS transition for functions.  */
  if (h != NULL
      && (h->type == STT_FUNC
	  || h->type == STT_GNU_IFUNC))
    return TRUE;

  switch (from_type)
    {
    case R_X86_64_TLSGD:
    case R_X86_64_GOTPC32_TLSDESC:
    case R_X86_64_TLSDESC_CALL:
    case R_X86_64_GOTTPOFF:
      if (bfd_link_executable (info))
	{
	  if (h == NULL)
	    to_type = R_X86_64_TPOFF32;
	  else
	    to_type = R_X86_64_GOTTPOFF;
	}

      /* When we are called from elf_x86_64_relocate_section, there may
	 be additional transitions based on TLS_TYPE.  */
      if (from_relocate_section)
	{
	  unsigned int new_to_type = to_type;

	  if (TLS_TRANSITION_IE_TO_LE_P (info, h, tls_type))
	    new_to_type = R_X86_64_TPOFF32;

	  if (to_type == R_X86_64_TLSGD
	      || to_type == R_X86_64_GOTPC32_TLSDESC
	      || to_type == R_X86_64_TLSDESC_CALL)
	    {
	      if (tls_type == GOT_TLS_IE)
		new_to_type = R_X86_64_GOTTPOFF;
	    }

	  /* We checked the transition before when we were called from
	     elf_x86_64_check_relocs.  We only want to check the new
	     transition which hasn't been checked before.  */
	  check = new_to_type != to_type && from_type == to_type;
	  to_type = new_to_type;
	}

      break;

    case R_X86_64_TLSLD:
      if (bfd_link_executable (info))
	to_type = R_X86_64_TPOFF32;
      break;

    default:
      return TRUE;
    }

  /* Return TRUE if there is no transition.  */
  if (from_type == to_type)
    return TRUE;

  /* Check if the transition can be performed.  */
  if (check
      && ! elf_x86_64_check_tls_transition (abfd, info, sec, contents,
					    symtab_hdr, sym_hashes,
					    from_type, rel, relend))
    {
      reloc_howto_type *from, *to;
      const char *name;

      from = elf_x86_64_rtype_to_howto (abfd, from_type);
      to = elf_x86_64_rtype_to_howto (abfd, to_type);

      if (from == NULL || to == NULL)
	return FALSE;

      if (h)
	name = h->root.root.string;
      else
	{
	  struct elf_x86_link_hash_table *htab;

	  htab = elf_x86_hash_table (info, X86_64_ELF_DATA);
	  if (htab == NULL)
	    name = "*unknown*";
	  else
	    {
	      Elf_Internal_Sym *isym;

	      isym = bfd_sym_from_r_symndx (&htab->sym_cache,
					    abfd, r_symndx);
	      name = bfd_elf_sym_name (abfd, symtab_hdr, isym, NULL);
	    }
	}

      _bfd_error_handler
	/* xgettext:c-format */
	(_("%pB: TLS transition from %s to %s against `%s' at %#" PRIx64
	   " in section `%pA' failed"),
	 abfd, from->name, to->name, name, (uint64_t) rel->r_offset, sec);
      bfd_set_error (bfd_error_bad_value);
      return FALSE;
    }

  *r_type = to_type;
  return TRUE;
}

/* Rename some of the generic section flags to better document how they
   are used here.  */
#define check_relocs_failed	sec_flg0

static bfd_boolean
elf_x86_64_need_pic (struct bfd_link_info *info,
		     bfd *input_bfd, asection *sec,
		     struct elf_link_hash_entry *h,
		     Elf_Internal_Shdr *symtab_hdr,
		     Elf_Internal_Sym *isym,
		     reloc_howto_type *howto)
{
  const char *v = "";
  const char *und = "";
  const char *pic = "";
  const char *object;

  const char *name;
  if (h)
    {
      name = h->root.root.string;
      switch (ELF_ST_VISIBILITY (h->other))
	{
	case STV_HIDDEN:
	  v = _("hidden symbol ");
	  break;
	case STV_INTERNAL:
	  v = _("internal symbol ");
	  break;
	case STV_PROTECTED:
	  v = _("protected symbol ");
	  break;
	default:
	  if (((struct elf_x86_link_hash_entry *) h)->def_protected)
	    v = _("protected symbol ");
	  else
	    v = _("symbol ");
	  pic = _("; recompile with -fPIC");
	  break;
	}

      if (!h->def_regular && !h->def_dynamic)
	und = _("undefined ");
    }
  else
    {
      name = bfd_elf_sym_name (input_bfd, symtab_hdr, isym, NULL);
      pic = _("; recompile with -fPIC");
    }

  if (bfd_link_dll (info))
    object = _("a shared object");
  else if (bfd_link_pie (info))
    object = _("a PIE object");
  else
    object = _("a PDE object");

  /* xgettext:c-format */
  _bfd_error_handler (_("%pB: relocation %s against %s%s`%s' can "
			"not be used when making %s%s"),
		      input_bfd, howto->name, und, v, name,
		      object, pic);
  bfd_set_error (bfd_error_bad_value);
  sec->check_relocs_failed = 1;
  return FALSE;
}

/* With the local symbol, foo, we convert
   mov foo@GOTPCREL(%rip), %reg
   to
   lea foo(%rip), %reg
   and convert
   call/jmp *foo@GOTPCREL(%rip)
   to
   nop call foo/jmp foo nop
   When PIC is false, convert
   test %reg, foo@GOTPCREL(%rip)
   to
   test $foo, %reg
   and convert
   binop foo@GOTPCREL(%rip), %reg
   to
   binop $foo, %reg
   where binop is one of adc, add, and, cmp, or, sbb, sub, xor
   instructions.  */

static bfd_boolean
elf_x86_64_convert_load_reloc (bfd *abfd,
			       bfd_byte *contents,
			       unsigned int *r_type_p,
			       Elf_Internal_Rela *irel,
			       struct elf_link_hash_entry *h,
			       bfd_boolean *converted,
			       struct bfd_link_info *link_info)
{
  return TRUE;
}

/* Look through the relocs for a section during the first phase, and
   calculate needed space in the global offset table, procedure
   linkage table, and dynamic reloc sections.  */

static bfd_boolean
elf_x86_64_check_relocs (bfd *abfd, struct bfd_link_info *info,
			 asection *sec,
			 const Elf_Internal_Rela *relocs)
{
  return TRUE;
}

/* Return the relocation value for @tpoff relocation
   if STT_TLS virtual address is ADDRESS.  */

static bfd_vma
elf_x86_64_tpoff (struct bfd_link_info *info, bfd_vma address)
{
  struct elf_link_hash_table *htab = elf_hash_table (info);
  const struct elf_backend_data *bed = get_elf_backend_data (info->output_bfd);
  bfd_vma static_tls_size;

  /* If tls_segment is NULL, we should have signalled an error already.  */
  if (htab->tls_sec == NULL)
    return 0;

  /* Consider special static TLS alignment requirements.  */
  static_tls_size = BFD_ALIGN (htab->tls_size, bed->static_tls_alignment);
  return address - static_tls_size - htab->tls_sec->vma;
}

/* Finish up dynamic symbol handling.  We set the contents of various
   dynamic sections here.  */

static bfd_boolean
elf_x86_64_finish_dynamic_symbol (bfd *output_bfd,
				  struct bfd_link_info *info,
				  struct elf_link_hash_entry *h,
				  Elf_Internal_Sym *sym)
{
  return TRUE;
}

/* Finish up local dynamic symbol handling.  We set the contents of
   various dynamic sections here.  */

static bfd_boolean
elf_x86_64_finish_local_dynamic_symbol (void **slot, void *inf)
{
  struct elf_link_hash_entry *h
    = (struct elf_link_hash_entry *) *slot;
  struct bfd_link_info *info
    = (struct bfd_link_info *) inf;

  return elf_x86_64_finish_dynamic_symbol (info->output_bfd,
					   info, h, NULL);
}

/* Finish up undefined weak symbol handling in PIE.  Fill its PLT entry
   here since undefined weak symbol may not be dynamic and may not be
   called for elf_x86_64_finish_dynamic_symbol.  */

static bfd_boolean
elf_x86_64_pie_finish_undefweak_symbol (struct bfd_hash_entry *bh,
					void *inf)
{
  struct elf_link_hash_entry *h = (struct elf_link_hash_entry *) bh;
  struct bfd_link_info *info = (struct bfd_link_info *) inf;

  if (h->root.type != bfd_link_hash_undefweak
      || h->dynindx != -1)
    return TRUE;

  return elf_x86_64_finish_dynamic_symbol (info->output_bfd,
					   info, h, NULL);
}

/* Used to decide how to sort relocs in an optimal manner for the
   dynamic linker, before writing them out.  */

static enum elf_reloc_type_class
elf_x86_64_reloc_type_class (const struct bfd_link_info *info,
			     const asection *rel_sec ATTRIBUTE_UNUSED,
			     const Elf_Internal_Rela *rela)
{
  bfd *abfd = info->output_bfd;
  const struct elf_backend_data *bed = get_elf_backend_data (abfd);
  struct elf_x86_link_hash_table *htab
    = elf_x86_hash_table (info, X86_64_ELF_DATA);

  if (htab->elf.dynsym != NULL
      && htab->elf.dynsym->contents != NULL)
    {
      /* Check relocation against STT_GNU_IFUNC symbol if there are
	 dynamic symbols.  */
      unsigned long r_symndx = htab->r_sym (rela->r_info);
      if (r_symndx != STN_UNDEF)
	{
	  Elf_Internal_Sym sym;
	  if (!bed->s->swap_symbol_in (abfd,
				       (htab->elf.dynsym->contents
					+ r_symndx * bed->s->sizeof_sym),
				       0, &sym))
	    abort ();

	  if (ELF_ST_TYPE (sym.st_info) == STT_GNU_IFUNC)
	    return reloc_class_ifunc;
	}
    }

  switch ((int) ELF32_R_TYPE (rela->r_info))
    {
    case R_X86_64_IRELATIVE:
      return reloc_class_ifunc;
    case R_X86_64_RELATIVE:
    case R_X86_64_RELATIVE64:
      return reloc_class_relative;
    case R_X86_64_JUMP_SLOT:
      return reloc_class_plt;
    case R_X86_64_COPY:
      return reloc_class_copy;
    default:
      return reloc_class_normal;
    }
}

/* Finish up the dynamic sections.  */

static bfd_boolean
elf_x86_64_finish_dynamic_sections (bfd *output_bfd,
				    struct bfd_link_info *info)
{
  struct elf_x86_link_hash_table *htab;

  htab = _bfd_x86_elf_finish_dynamic_sections (output_bfd, info);
  if (htab == NULL)
    return FALSE;

  if (! htab->elf.dynamic_sections_created)
    return TRUE;

  if (htab->elf.splt && htab->elf.splt->size > 0)
    {
      elf_section_data (htab->elf.splt->output_section)
	->this_hdr.sh_entsize = htab->plt.plt_entry_size;

      if (htab->plt.has_plt0)
	{
	  /* Fill in the special first entry in the procedure linkage
	     table.  */
	  memcpy (htab->elf.splt->contents,
		  htab->lazy_plt->plt0_entry,
		  htab->lazy_plt->plt0_entry_size);
	  /* Add offset for pushq GOT+8(%rip), since the instruction
	     uses 6 bytes subtract this value.  */
	  bfd_put_32 (output_bfd,
		      (htab->elf.sgotplt->output_section->vma
		       + htab->elf.sgotplt->output_offset
		       + 8
		       - htab->elf.splt->output_section->vma
		       - htab->elf.splt->output_offset
		       - 6),
		      (htab->elf.splt->contents
		       + htab->lazy_plt->plt0_got1_offset));
	  /* Add offset for the PC-relative instruction accessing
	     GOT+16, subtracting the offset to the end of that
	     instruction.  */
	  bfd_put_32 (output_bfd,
		      (htab->elf.sgotplt->output_section->vma
		       + htab->elf.sgotplt->output_offset
		       + 16
		       - htab->elf.splt->output_section->vma
		       - htab->elf.splt->output_offset
		       - htab->lazy_plt->plt0_got2_insn_end),
		      (htab->elf.splt->contents
		       + htab->lazy_plt->plt0_got2_offset));
	}

      if (htab->tlsdesc_plt)
	{
	  bfd_put_64 (output_bfd, (bfd_vma) 0,
		      htab->elf.sgot->contents + htab->tlsdesc_got);

	  memcpy (htab->elf.splt->contents + htab->tlsdesc_plt,
		  htab->lazy_plt->plt_tlsdesc_entry,
		  htab->lazy_plt->plt_tlsdesc_entry_size);

	  /* Add offset for pushq GOT+8(%rip), since ENDBR64 uses 4
	     bytes and the instruction uses 6 bytes, subtract these
	     values.  */
	  bfd_put_32 (output_bfd,
		      (htab->elf.sgotplt->output_section->vma
		       + htab->elf.sgotplt->output_offset
		       + 8
		       - htab->elf.splt->output_section->vma
		       - htab->elf.splt->output_offset
		       - htab->tlsdesc_plt
		       - htab->lazy_plt->plt_tlsdesc_got1_insn_end),
		      (htab->elf.splt->contents
		       + htab->tlsdesc_plt
		       + htab->lazy_plt->plt_tlsdesc_got1_offset));
	  /* Add offset for indirect branch via GOT+TDG, where TDG
	     stands for htab->tlsdesc_got, subtracting the offset
	     to the end of that instruction.  */
	  bfd_put_32 (output_bfd,
		      (htab->elf.sgot->output_section->vma
		       + htab->elf.sgot->output_offset
		       + htab->tlsdesc_got
		       - htab->elf.splt->output_section->vma
		       - htab->elf.splt->output_offset
		       - htab->tlsdesc_plt
		       - htab->lazy_plt->plt_tlsdesc_got2_insn_end),
		      (htab->elf.splt->contents
		       + htab->tlsdesc_plt
		       + htab->lazy_plt->plt_tlsdesc_got2_offset));
	}
    }

  /* Fill PLT entries for undefined weak symbols in PIE.  */
  if (bfd_link_pie (info))
    bfd_hash_traverse (&info->hash->table,
		       elf_x86_64_pie_finish_undefweak_symbol,
		       info);

  return TRUE;
}

/* Fill PLT/GOT entries and allocate dynamic relocations for local
   STT_GNU_IFUNC symbols, which aren't in the ELF linker hash table.
   It has to be done before elf_link_sort_relocs is called so that
   dynamic relocations are properly sorted.  */

static bfd_boolean
elf_x86_64_output_arch_local_syms
  (bfd *output_bfd ATTRIBUTE_UNUSED,
   struct bfd_link_info *info,
   void *flaginfo ATTRIBUTE_UNUSED,
   int (*func) (void *, const char *,
		Elf_Internal_Sym *,
		asection *,
		struct elf_link_hash_entry *) ATTRIBUTE_UNUSED)
{
  struct elf_x86_link_hash_table *htab
    = elf_x86_hash_table (info, X86_64_ELF_DATA);
  if (htab == NULL)
    return FALSE;

  /* Fill PLT and GOT entries for local STT_GNU_IFUNC symbols.  */
  htab_traverse (htab->loc_hash_table,
		 elf_x86_64_finish_local_dynamic_symbol,
		 info);

  return TRUE;
}

/* Forward declaration.  */
static const struct elf_x86_lazy_plt_layout elf_x86_64_nacl_plt;

/* Similar to _bfd_elf_get_synthetic_symtab.  Support PLTs with all
   dynamic relocations.   */

static long
elf_x86_64_get_synthetic_symtab (bfd *abfd,
				 long symcount ATTRIBUTE_UNUSED,
				 asymbol **syms ATTRIBUTE_UNUSED,
				 long dynsymcount,
				 asymbol **dynsyms,
				 asymbol **ret)
{
  long count, i, n;
  int j;
  bfd_byte *plt_contents;
  long relsize;
  const struct elf_x86_lazy_plt_layout *lazy_plt;
  const struct elf_x86_non_lazy_plt_layout *non_lazy_plt;
  const struct elf_x86_lazy_plt_layout *lazy_bnd_plt;
  const struct elf_x86_non_lazy_plt_layout *non_lazy_bnd_plt;
  const struct elf_x86_lazy_plt_layout *lazy_ibt_plt;
  const struct elf_x86_non_lazy_plt_layout *non_lazy_ibt_plt;
  asection *plt;
  enum elf_x86_plt_type plt_type;
  struct elf_x86_plt plts[] =
    {
      { ".plt", NULL, NULL, plt_unknown, 0, 0, 0, 0 },
      { ".plt.got", NULL, NULL, plt_non_lazy, 0, 0, 0, 0 },
      { ".plt.sec", NULL, NULL, plt_second, 0, 0, 0, 0 },
      { ".plt.bnd", NULL, NULL, plt_second, 0, 0, 0, 0 },
      { NULL, NULL, NULL, plt_non_lazy, 0, 0, 0, 0 }
    };

  *ret = NULL;

  if ((abfd->flags & (DYNAMIC | EXEC_P)) == 0)
    return 0;

  if (dynsymcount <= 0)
    return 0;

  relsize = bfd_get_dynamic_reloc_upper_bound (abfd);
  if (relsize <= 0)
    return -1;

  if (get_elf_x86_backend_data (abfd)->target_os != is_nacl)
    {
      lazy_plt = &elf_x86_64_lazy_plt;
      non_lazy_plt = &elf_x86_64_non_lazy_plt;
      lazy_bnd_plt = &elf_x86_64_lazy_bnd_plt;
      non_lazy_bnd_plt = &elf_x86_64_non_lazy_bnd_plt;
      if (ABI_64_P (abfd))
	{
	  lazy_ibt_plt = &elf_x86_64_lazy_ibt_plt;
	  non_lazy_ibt_plt = &elf_x86_64_non_lazy_ibt_plt;
	}
      else
	{
	  lazy_ibt_plt = &elf_x32_lazy_ibt_plt;
	  non_lazy_ibt_plt = &elf_x32_non_lazy_ibt_plt;
	}
    }
  else
    {
      lazy_plt = &elf_x86_64_nacl_plt;
      non_lazy_plt = NULL;
      lazy_bnd_plt = NULL;
      non_lazy_bnd_plt = NULL;
      lazy_ibt_plt = NULL;
      non_lazy_ibt_plt = NULL;
    }

  count = 0;
  for (j = 0; plts[j].name != NULL; j++)
    {
      plt = bfd_get_section_by_name (abfd, plts[j].name);
      if (plt == NULL || plt->size == 0)
	continue;

      /* Get the PLT section contents.  */
      plt_contents = (bfd_byte *) bfd_malloc (plt->size);
      if (plt_contents == NULL)
	break;
      if (!bfd_get_section_contents (abfd, (asection *) plt,
				     plt_contents, 0, plt->size))
	{
	  free (plt_contents);
	  break;
	}

      /* Check what kind of PLT it is.  */
      plt_type = plt_unknown;
      if (plts[j].type == plt_unknown
	  && (plt->size >= (lazy_plt->plt_entry_size
			    + lazy_plt->plt_entry_size)))
	{
	  /* Match lazy PLT first.  Need to check the first two
	     instructions.   */
	  if ((memcmp (plt_contents, lazy_plt->plt0_entry,
		       lazy_plt->plt0_got1_offset) == 0)
	      && (memcmp (plt_contents + 6, lazy_plt->plt0_entry + 6,
			  2) == 0))
	    plt_type = plt_lazy;
	  else if (lazy_bnd_plt != NULL
		   && (memcmp (plt_contents, lazy_bnd_plt->plt0_entry,
			       lazy_bnd_plt->plt0_got1_offset) == 0)
		   && (memcmp (plt_contents + 6,
			       lazy_bnd_plt->plt0_entry + 6, 3) == 0))
	    {
	      plt_type = plt_lazy | plt_second;
	      /* The fist entry in the lazy IBT PLT is the same as the
		 lazy BND PLT.  */
	      if ((memcmp (plt_contents + lazy_ibt_plt->plt_entry_size,
			   lazy_ibt_plt->plt_entry,
			   lazy_ibt_plt->plt_got_offset) == 0))
		lazy_plt = lazy_ibt_plt;
	      else
		lazy_plt = lazy_bnd_plt;
	    }
	}

      if (non_lazy_plt != NULL
	  && (plt_type == plt_unknown || plt_type == plt_non_lazy)
	  && plt->size >= non_lazy_plt->plt_entry_size)
	{
	  /* Match non-lazy PLT.  */
	  if (memcmp (plt_contents, non_lazy_plt->plt_entry,
		      non_lazy_plt->plt_got_offset) == 0)
	    plt_type = plt_non_lazy;
	}

      if (plt_type == plt_unknown || plt_type == plt_second)
	{
	  if (non_lazy_bnd_plt != NULL
	      && plt->size >= non_lazy_bnd_plt->plt_entry_size
	      && (memcmp (plt_contents, non_lazy_bnd_plt->plt_entry,
			  non_lazy_bnd_plt->plt_got_offset) == 0))
	    {
	      /* Match BND PLT.  */
	      plt_type = plt_second;
	      non_lazy_plt = non_lazy_bnd_plt;
	    }
	  else if (non_lazy_ibt_plt != NULL
		   && plt->size >= non_lazy_ibt_plt->plt_entry_size
		   && (memcmp (plt_contents,
			       non_lazy_ibt_plt->plt_entry,
			       non_lazy_ibt_plt->plt_got_offset) == 0))
	    {
	      /* Match IBT PLT.  */
	      plt_type = plt_second;
	      non_lazy_plt = non_lazy_ibt_plt;
	    }
	}

      if (plt_type == plt_unknown)
	{
	  free (plt_contents);
	  continue;
	}

      plts[j].sec = plt;
      plts[j].type = plt_type;

      if ((plt_type & plt_lazy))
	{
	  plts[j].plt_got_offset = lazy_plt->plt_got_offset;
	  plts[j].plt_got_insn_size = lazy_plt->plt_got_insn_size;
	  plts[j].plt_entry_size = lazy_plt->plt_entry_size;
	  /* Skip PLT0 in lazy PLT.  */
	  i = 1;
	}
      else
	{
	  plts[j].plt_got_offset = non_lazy_plt->plt_got_offset;
	  plts[j].plt_got_insn_size = non_lazy_plt->plt_got_insn_size;
	  plts[j].plt_entry_size = non_lazy_plt->plt_entry_size;
	  i = 0;
	}

      /* Skip lazy PLT when the second PLT is used.  */
      if (plt_type == (plt_lazy | plt_second))
	plts[j].count = 0;
      else
	{
	  n = plt->size / plts[j].plt_entry_size;
	  plts[j].count = n;
	  count += n - i;
	}

      plts[j].contents = plt_contents;
    }

  return _bfd_x86_elf_get_synthetic_symtab (abfd, count, relsize,
					    (bfd_vma) 0, plts, dynsyms,
					    ret);
}

/* Handle an x86-64 specific section when reading an object file.  This
   is called when elfcode.h finds a section with an unknown type.  */

static bfd_boolean
elf_x86_64_section_from_shdr (bfd *abfd, Elf_Internal_Shdr *hdr,
			      const char *name, int shindex)
{
  if (hdr->sh_type != SHT_X86_64_UNWIND)
    return FALSE;

  if (! _bfd_elf_make_section_from_shdr (abfd, hdr, name, shindex))
    return FALSE;

  return TRUE;
}

/* Hook called by the linker routine which adds symbols from an object
   file.  We use it to put SHN_X86_64_LCOMMON items in .lbss, instead
   of .bss.  */

static bfd_boolean
elf_x86_64_add_symbol_hook (bfd *abfd,
			    struct bfd_link_info *info ATTRIBUTE_UNUSED,
			    Elf_Internal_Sym *sym,
			    const char **namep ATTRIBUTE_UNUSED,
			    flagword *flagsp ATTRIBUTE_UNUSED,
			    asection **secp,
			    bfd_vma *valp)
{
  asection *lcomm;

  switch (sym->st_shndx)
    {
    case SHN_X86_64_LCOMMON:
      lcomm = bfd_get_section_by_name (abfd, "LARGE_COMMON");
      if (lcomm == NULL)
	{
	  lcomm = bfd_make_section_with_flags (abfd,
					       "LARGE_COMMON",
					       (SEC_ALLOC
						| SEC_IS_COMMON
						| SEC_LINKER_CREATED));
	  if (lcomm == NULL)
	    return FALSE;
	  elf_section_flags (lcomm) |= SHF_X86_64_LARGE;
	}
      *secp = lcomm;
      *valp = sym->st_size;
      return TRUE;
    }

  return TRUE;
}


/* Given a BFD section, try to locate the corresponding ELF section
   index.  */

static bfd_boolean
elf_x86_64_elf_section_from_bfd_section (bfd *abfd ATTRIBUTE_UNUSED,
					 asection *sec, int *index_return)
{
  if (sec == &_bfd_elf_large_com_section)
    {
      *index_return = SHN_X86_64_LCOMMON;
      return TRUE;
    }
  return FALSE;
}

/* Process a symbol.  */

static void
elf_x86_64_symbol_processing (bfd *abfd ATTRIBUTE_UNUSED,
			      asymbol *asym)
{
  elf_symbol_type *elfsym = (elf_symbol_type *) asym;

  switch (elfsym->internal_elf_sym.st_shndx)
    {
    case SHN_X86_64_LCOMMON:
      asym->section = &_bfd_elf_large_com_section;
      asym->value = elfsym->internal_elf_sym.st_size;
      /* Common symbol doesn't set BSF_GLOBAL.  */
      asym->flags &= ~BSF_GLOBAL;
      break;
    }
}

static bfd_boolean
elf_x86_64_common_definition (Elf_Internal_Sym *sym)
{
  return (sym->st_shndx == SHN_COMMON
	  || sym->st_shndx == SHN_X86_64_LCOMMON);
}

static unsigned int
elf_x86_64_common_section_index (asection *sec)
{
  if ((elf_section_flags (sec) & SHF_X86_64_LARGE) == 0)
    return SHN_COMMON;
  else
    return SHN_X86_64_LCOMMON;
}

static asection *
elf_x86_64_common_section (asection *sec)
{
  if ((elf_section_flags (sec) & SHF_X86_64_LARGE) == 0)
    return bfd_com_section_ptr;
  else
    return &_bfd_elf_large_com_section;
}

static bfd_boolean
elf_x86_64_merge_symbol (struct elf_link_hash_entry *h,
			 const Elf_Internal_Sym *sym,
			 asection **psec,
			 bfd_boolean newdef,
			 bfd_boolean olddef,
			 bfd *oldbfd,
			 const asection *oldsec)
{
  /* A normal common symbol and a large common symbol result in a
     normal common symbol.  We turn the large common symbol into a
     normal one.  */
  if (!olddef
      && h->root.type == bfd_link_hash_common
      && !newdef
      && bfd_is_com_section (*psec)
      && oldsec != *psec)
    {
      if (sym->st_shndx == SHN_COMMON
	  && (elf_section_flags (oldsec) & SHF_X86_64_LARGE) != 0)
	{
	  h->root.u.c.p->section
	    = bfd_make_section_old_way (oldbfd, "COMMON");
	  h->root.u.c.p->section->flags = SEC_ALLOC;
	}
      else if (sym->st_shndx == SHN_X86_64_LCOMMON
	       && (elf_section_flags (oldsec) & SHF_X86_64_LARGE) == 0)
	*psec = bfd_com_section_ptr;
    }

  return TRUE;
}

static int
elf_x86_64_additional_program_headers (bfd *abfd,
				       struct bfd_link_info *info ATTRIBUTE_UNUSED)
{
  asection *s;
  int count = 0;

  /* Check to see if we need a large readonly segment.  */
  s = bfd_get_section_by_name (abfd, ".lrodata");
  if (s && (s->flags & SEC_LOAD))
    count++;

  /* Check to see if we need a large data segment.  Since .lbss sections
     is placed right after the .bss section, there should be no need for
     a large data segment just because of .lbss.  */
  s = bfd_get_section_by_name (abfd, ".ldata");
  if (s && (s->flags & SEC_LOAD))
    count++;

  return count;
}

/* Return TRUE iff relocations for INPUT are compatible with OUTPUT. */

static bfd_boolean
elf_x86_64_relocs_compatible (const bfd_target *input,
			      const bfd_target *output)
{
  return NULL;
}

/* Set up x86-64 GNU properties.  Return the first relocatable ELF input
   with GNU properties if found.  Otherwise, return NULL.  */

static bfd *
elf_x86_64_link_setup_gnu_properties (struct bfd_link_info *info)
{
  return NULL;
}

static const struct bfd_elf_special_section
elf_x86_64_special_sections[]=
{
  { STRING_COMMA_LEN (".gnu.linkonce.lb"), -2, SHT_NOBITS,   SHF_ALLOC + SHF_WRITE + SHF_X86_64_LARGE},
  { STRING_COMMA_LEN (".gnu.linkonce.lr"), -2, SHT_PROGBITS, SHF_ALLOC + SHF_X86_64_LARGE},
  { STRING_COMMA_LEN (".gnu.linkonce.lt"), -2, SHT_PROGBITS, SHF_ALLOC + SHF_EXECINSTR + SHF_X86_64_LARGE},
  { STRING_COMMA_LEN (".lbss"),		   -2, SHT_NOBITS,   SHF_ALLOC + SHF_WRITE + SHF_X86_64_LARGE},
  { STRING_COMMA_LEN (".ldata"),	   -2, SHT_PROGBITS, SHF_ALLOC + SHF_WRITE + SHF_X86_64_LARGE},
  { STRING_COMMA_LEN (".lrodata"),	   -2, SHT_PROGBITS, SHF_ALLOC + SHF_X86_64_LARGE},
  { NULL,			0,	    0, 0,	     0 }
};

#define TARGET_LITTLE_SYM		    x86_64_elf64_vec
#define TARGET_LITTLE_NAME		    "elf64-x86-64"
#define ELF_ARCH			    bfd_arch_i386
#define ELF_TARGET_ID			    X86_64_ELF_DATA
#define ELF_MACHINE_CODE		    EM_X86_64
#if DEFAULT_LD_Z_SEPARATE_CODE
# define ELF_MAXPAGESIZE		    0x1000
#else
# define ELF_MAXPAGESIZE		    0x200000
#endif
#define ELF_MINPAGESIZE			    0x1000
#define ELF_COMMONPAGESIZE		    0x1000

#define elf_backend_can_gc_sections	    1
#define elf_backend_can_refcount	    1
#define elf_backend_want_got_plt	    1
#define elf_backend_plt_readonly	    1
#define elf_backend_want_plt_sym	    0
#define elf_backend_got_header_size	    (GOT_ENTRY_SIZE*3)
#define elf_backend_rela_normal		    1
#define elf_backend_plt_alignment	    4
#define elf_backend_extern_protected_data   1
#define elf_backend_caches_rawsize	    1
#define elf_backend_dtrel_excludes_plt	    1
#define elf_backend_want_dynrelro	    1

#define elf_info_to_howto		    elf_x86_64_info_to_howto

#define bfd_elf64_bfd_reloc_type_lookup	    elf_x86_64_reloc_type_lookup
#define bfd_elf64_bfd_reloc_name_lookup \
  elf_x86_64_reloc_name_lookup

#define elf_backend_relocs_compatible	    elf_x86_64_relocs_compatible
#define elf_backend_check_relocs	    elf_x86_64_check_relocs
#define elf_backend_create_dynamic_sections _bfd_elf_create_dynamic_sections
#define elf_backend_finish_dynamic_sections elf_x86_64_finish_dynamic_sections
#define elf_backend_finish_dynamic_symbol   elf_x86_64_finish_dynamic_symbol
#define elf_backend_output_arch_local_syms  elf_x86_64_output_arch_local_syms
#define elf_backend_grok_prstatus	    elf_x86_64_grok_prstatus
#define elf_backend_grok_psinfo		    elf_x86_64_grok_psinfo
#ifdef CORE_HEADER
#define elf_backend_write_core_note	    elf_x86_64_write_core_note
#endif
#define elf_backend_reloc_type_class	    elf_x86_64_reloc_type_class
#define elf_backend_relocate_section	    elf_x86_64_relocate_section
#define elf_backend_init_index_section	    _bfd_elf_init_1_index_section
#define elf_backend_object_p		    elf64_x86_64_elf_object_p
#define bfd_elf64_get_synthetic_symtab	    elf_x86_64_get_synthetic_symtab

#define elf_backend_section_from_shdr \
	elf_x86_64_section_from_shdr

#define elf_backend_section_from_bfd_section \
  elf_x86_64_elf_section_from_bfd_section
#define elf_backend_add_symbol_hook \
  elf_x86_64_add_symbol_hook
#define elf_backend_symbol_processing \
  elf_x86_64_symbol_processing
#define elf_backend_common_section_index \
  elf_x86_64_common_section_index
#define elf_backend_common_section \
  elf_x86_64_common_section
#define elf_backend_common_definition \
  elf_x86_64_common_definition
#define elf_backend_merge_symbol \
  elf_x86_64_merge_symbol
#define elf_backend_special_sections \
  elf_x86_64_special_sections
#define elf_backend_additional_program_headers \
  elf_x86_64_additional_program_headers
#define elf_backend_setup_gnu_properties \
  elf_x86_64_link_setup_gnu_properties
#define elf_backend_hide_symbol \
  _bfd_x86_elf_hide_symbol

#undef	elf64_bed
#define elf64_bed elf64_x86_64_bed

/* This structure contains everything that BFD knows about a target.
   It includes things like its byte order, name, what routines to call
   to do various operations, etc.  Every BFD points to a target structure
   with its "xvec" member.

   There are two such structures here:  one for big-endian machines and
   one for little-endian machines.   */

#ifndef bfd_elf64_close_and_cleanup
#define	bfd_elf64_close_and_cleanup _bfd_elf_close_and_cleanup
#endif
#ifndef bfd_elf64_bfd_free_cached_info
#define bfd_elf64_bfd_free_cached_info _bfd_free_cached_info
#endif
#ifndef bfd_elf64_get_section_contents
#define bfd_elf64_get_section_contents _bfd_generic_get_section_contents
#endif

#define bfd_elf64_canonicalize_dynamic_symtab \
  _bfd_elf_canonicalize_dynamic_symtab
#ifndef bfd_elf64_get_synthetic_symtab
#define bfd_elf64_get_synthetic_symtab \
  _bfd_elf_get_synthetic_symtab
#endif
#ifndef bfd_elf64_canonicalize_reloc
#define bfd_elf64_canonicalize_reloc	_bfd_elf_canonicalize_reloc
#endif
#ifndef bfd_elf64_set_reloc
#define bfd_elf64_set_reloc		_bfd_generic_set_reloc
#endif
#ifndef bfd_elf64_find_nearest_line
#define bfd_elf64_find_nearest_line	_bfd_elf_find_nearest_line
#endif
#ifndef bfd_elf64_find_line
#define bfd_elf64_find_line		_bfd_elf_find_line
#endif
#ifndef bfd_elf64_find_inliner_info
#define bfd_elf64_find_inliner_info	_bfd_elf_find_inliner_info
#endif
#define bfd_elf64_read_minisymbols	_bfd_elf_read_minisymbols
#define bfd_elf64_minisymbol_to_symbol	_bfd_elf_minisymbol_to_symbol
#define bfd_elf64_get_dynamic_symtab_upper_bound \
  _bfd_elf_get_dynamic_symtab_upper_bound
#define bfd_elf64_get_lineno		_bfd_elf_get_lineno
#ifndef bfd_elf64_get_reloc_upper_bound
#define bfd_elf64_get_reloc_upper_bound _bfd_elf_get_reloc_upper_bound
#endif
#ifndef bfd_elf64_get_symbol_info
#define bfd_elf64_get_symbol_info	_bfd_elf_get_symbol_info
#endif
#ifndef bfd_elf64_get_symbol_version_string
#define bfd_elf64_get_symbol_version_string \
  _bfd_elf_get_symbol_version_string
#endif
#define bfd_elf64_canonicalize_symtab	_bfd_elf_canonicalize_symtab
#define bfd_elf64_get_symtab_upper_bound _bfd_elf_get_symtab_upper_bound
#define bfd_elf64_make_empty_symbol	_bfd_elf_make_empty_symbol
#ifndef bfd_elf64_new_section_hook
#define bfd_elf64_new_section_hook	_bfd_elf_new_section_hook
#endif
#define bfd_elf64_set_arch_mach		_bfd_elf_set_arch_mach
#ifndef bfd_elf64_set_section_contents
#define bfd_elf64_set_section_contents	_bfd_elf_set_section_contents
#endif
#define bfd_elf64_sizeof_headers	_bfd_elf_sizeof_headers
#define bfd_elf64_write_object_contents _bfd_elf_write_object_contents
#define bfd_elf64_write_corefile_contents _bfd_elf_write_corefile_contents

#define bfd_elf64_get_section_contents_in_window \
  _bfd_generic_get_section_contents_in_window

#ifndef elf_backend_can_refcount
#define elf_backend_can_refcount 0
#endif
#ifndef elf_backend_want_got_plt
#define elf_backend_want_got_plt 0
#endif
#ifndef elf_backend_plt_readonly
#define elf_backend_plt_readonly 0
#endif
#ifndef elf_backend_want_plt_sym
#define elf_backend_want_plt_sym 0
#endif
#ifndef elf_backend_plt_not_loaded
#define elf_backend_plt_not_loaded 0
#endif
#ifndef elf_backend_plt_alignment
#define elf_backend_plt_alignment 2
#endif
#ifndef elf_backend_want_dynbss
#define elf_backend_want_dynbss 1
#endif
#ifndef elf_backend_want_dynrelro
#define elf_backend_want_dynrelro 0
#endif
#ifndef elf_backend_want_p_paddr_set_to_zero
#define elf_backend_want_p_paddr_set_to_zero 0
#endif
#ifndef elf_backend_no_page_alias
#define elf_backend_no_page_alias 0
#endif
#ifndef elf_backend_default_execstack
#define elf_backend_default_execstack 1
#endif
#ifndef elf_backend_caches_rawsize
#define elf_backend_caches_rawsize 0
#endif
#ifndef elf_backend_extern_protected_data
#define elf_backend_extern_protected_data 0
#endif
#ifndef elf_backend_always_renumber_dynsyms
#define elf_backend_always_renumber_dynsyms FALSE
#endif
#ifndef elf_backend_linux_prpsinfo32_ugid16
#define elf_backend_linux_prpsinfo32_ugid16 FALSE
#endif
#ifndef elf_backend_linux_prpsinfo64_ugid16
#define elf_backend_linux_prpsinfo64_ugid16 FALSE
#endif
#ifndef elf_backend_stack_align
#define elf_backend_stack_align 16
#endif
#ifndef elf_backend_strtab_flags
#define elf_backend_strtab_flags 0
#endif

#define bfd_elf64_bfd_debug_info_start		_bfd_void_bfd
#define bfd_elf64_bfd_debug_info_end		_bfd_void_bfd
#define bfd_elf64_bfd_debug_info_accumulate	_bfd_void_bfd_asection

#ifndef bfd_elf64_bfd_get_relocated_section_contents
#define bfd_elf64_bfd_get_relocated_section_contents \
  bfd_generic_get_relocated_section_contents
#endif

#ifndef bfd_elf64_bfd_relax_section
#define bfd_elf64_bfd_relax_section bfd_generic_relax_section
#endif

#ifndef elf_backend_can_gc_sections
#define elf_backend_can_gc_sections 0
#endif
#ifndef elf_backend_can_refcount
#define elf_backend_can_refcount 0
#endif
#ifndef elf_backend_want_got_sym
#define elf_backend_want_got_sym 1
#endif
#ifndef elf_backend_gc_keep
#define elf_backend_gc_keep		_bfd_elf_gc_keep
#endif
#ifndef elf_backend_gc_mark_dynamic_ref
#define elf_backend_gc_mark_dynamic_ref	bfd_elf_gc_mark_dynamic_ref_symbol
#endif
#ifndef elf_backend_gc_mark_hook
#define elf_backend_gc_mark_hook	_bfd_elf_gc_mark_hook
#endif
#ifndef elf_backend_gc_mark_extra_sections
#define elf_backend_gc_mark_extra_sections _bfd_elf_gc_mark_extra_sections
#endif
#ifndef bfd_elf64_bfd_gc_sections
#define bfd_elf64_bfd_gc_sections bfd_elf_gc_sections
#endif

#ifndef bfd_elf64_bfd_merge_sections
#define bfd_elf64_bfd_merge_sections \
  _bfd_elf_merge_sections
#endif

#ifndef bfd_elf64_bfd_is_group_section
#define bfd_elf64_bfd_is_group_section bfd_elf_is_group_section
#endif

#ifndef bfd_elf64_bfd_discard_group
#define bfd_elf64_bfd_discard_group bfd_generic_discard_group
#endif

#ifndef bfd_elf64_section_already_linked
#define bfd_elf64_section_already_linked \
  _bfd_elf_section_already_linked
#endif

#ifndef bfd_elf64_bfd_define_common_symbol
#define bfd_elf64_bfd_define_common_symbol bfd_generic_define_common_symbol
#endif

#ifndef bfd_elf64_bfd_link_hide_symbol
#define bfd_elf64_bfd_link_hide_symbol _bfd_elf_link_hide_symbol
#endif

#ifndef bfd_elf64_bfd_lookup_section_flags
#define bfd_elf64_bfd_lookup_section_flags bfd_elf_lookup_section_flags
#endif

#ifndef bfd_elf64_bfd_make_debug_symbol
#define bfd_elf64_bfd_make_debug_symbol _bfd_nosymbols_bfd_make_debug_symbol
#endif

#ifndef bfd_elf64_bfd_copy_private_symbol_data
#define bfd_elf64_bfd_copy_private_symbol_data \
  _bfd_elf_copy_private_symbol_data
#endif

#ifndef bfd_elf64_bfd_copy_private_section_data
#define bfd_elf64_bfd_copy_private_section_data \
  _bfd_elf_copy_private_section_data
#endif
#ifndef bfd_elf64_bfd_copy_private_header_data
#define bfd_elf64_bfd_copy_private_header_data \
  _bfd_elf_copy_private_header_data
#endif
#ifndef bfd_elf64_bfd_copy_private_bfd_data
#define bfd_elf64_bfd_copy_private_bfd_data \
  _bfd_elf_copy_private_bfd_data
#endif
#ifndef bfd_elf64_bfd_print_private_bfd_data
#define bfd_elf64_bfd_print_private_bfd_data \
  _bfd_elf_print_private_bfd_data
#endif
#ifndef bfd_elf64_bfd_merge_private_bfd_data
#define bfd_elf64_bfd_merge_private_bfd_data _bfd_bool_bfd_link_true
#endif
#ifndef bfd_elf64_bfd_set_private_flags
#define bfd_elf64_bfd_set_private_flags _bfd_bool_bfd_uint_true
#endif
#ifndef bfd_elf64_bfd_is_local_label_name
#define bfd_elf64_bfd_is_local_label_name _bfd_elf_is_local_label_name
#endif
#ifndef bfd_elf64_bfd_is_target_special_symbol
#define bfd_elf64_bfd_is_target_special_symbol _bfd_bool_bfd_asymbol_false
#endif

#ifndef bfd_elf64_get_dynamic_reloc_upper_bound
#define bfd_elf64_get_dynamic_reloc_upper_bound \
  _bfd_elf_get_dynamic_reloc_upper_bound
#endif
#ifndef bfd_elf64_canonicalize_dynamic_reloc
#define bfd_elf64_canonicalize_dynamic_reloc \
  _bfd_elf_canonicalize_dynamic_reloc
#endif

#ifdef elf_backend_relocate_section
#ifndef bfd_elf64_bfd_link_hash_table_create
#define bfd_elf64_bfd_link_hash_table_create _bfd_elf_link_hash_table_create
#endif
#ifndef bfd_elf64_bfd_copy_link_hash_symbol_type
#define bfd_elf64_bfd_copy_link_hash_symbol_type \
  _bfd_elf_copy_link_hash_symbol_type
#endif
#ifndef bfd_elf64_bfd_link_add_symbols
#define bfd_elf64_bfd_link_add_symbols	bfd_elf_link_add_symbols
#endif
#ifndef bfd_elf64_bfd_define_start_stop
#define bfd_elf64_bfd_define_start_stop bfd_elf_define_start_stop
#endif
#ifndef bfd_elf64_bfd_final_link
#define bfd_elf64_bfd_final_link	bfd_elf_final_link
#endif
#else /* ! defined (elf_backend_relocate_section) */
/* If no backend relocate_section routine, use the generic linker.
   Note - this will prevent the port from being able to use some of
   the other features of the ELF linker, because the generic hash structure
   does not have the fields needed by the ELF linker.  In particular it
   means that linking directly to S-records will not work.  */
#ifndef bfd_elf64_bfd_link_hash_table_create
#define bfd_elf64_bfd_link_hash_table_create \
  _bfd_generic_link_hash_table_create
#endif
#ifndef bfd_elf64_bfd_copy_link_hash_symbol_type
#define bfd_elf64_bfd_copy_link_hash_symbol_type \
  _bfd_generic_copy_link_hash_symbol_type
#endif
#ifndef bfd_elf64_bfd_link_add_symbols
#define bfd_elf64_bfd_link_add_symbols	_bfd_generic_link_add_symbols
#endif
#ifndef bfd_elf64_bfd_define_start_stop
#define bfd_elf64_bfd_define_start_stop bfd_generic_define_start_stop
#endif
#ifndef bfd_elf64_bfd_final_link
#define bfd_elf64_bfd_final_link	_bfd_generic_final_link
#endif
#endif /* ! defined (elf_backend_relocate_section) */

#ifndef bfd_elf64_bfd_link_just_syms
#define bfd_elf64_bfd_link_just_syms	_bfd_elf_link_just_syms
#endif

#ifndef bfd_elf64_bfd_link_split_section
#define bfd_elf64_bfd_link_split_section _bfd_generic_link_split_section
#endif

#ifndef bfd_elf64_bfd_link_check_relocs
#define bfd_elf64_bfd_link_check_relocs  _bfd_elf_link_check_relocs
#endif

#ifndef bfd_elf64_archive_p
#define bfd_elf64_archive_p bfd_generic_archive_p
#endif

#ifndef bfd_elf64_write_archive_contents
#define bfd_elf64_write_archive_contents _bfd_write_archive_contents
#endif

#ifndef bfd_elf64_mkobject
#define bfd_elf64_mkobject bfd_elf_make_object
#endif

#ifndef bfd_elf64_mkcorefile
#define bfd_elf64_mkcorefile bfd_elf_mkcorefile
#endif

#ifndef bfd_elf64_mkarchive
#define bfd_elf64_mkarchive _bfd_generic_mkarchive
#endif

#ifndef bfd_elf64_print_symbol
#define bfd_elf64_print_symbol bfd_elf_print_symbol
#endif

#ifndef elf_symbol_leading_char
#define elf_symbol_leading_char 0
#endif

#ifndef elf_info_to_howto
#define elf_info_to_howto NULL
#endif

#ifndef elf_info_to_howto_rel
#define elf_info_to_howto_rel NULL
#endif

#ifndef elf_backend_arch_data
#define elf_backend_arch_data NULL
#endif

#ifndef ELF_TARGET_ID
#define ELF_TARGET_ID	GENERIC_ELF_DATA
#endif

#ifndef ELF_OSABI
#define ELF_OSABI ELFOSABI_NONE
#endif

#ifndef ELF_MAXPAGESIZE
# error ELF_MAXPAGESIZE is not defined
#define ELF_MAXPAGESIZE 1
#endif

#ifndef ELF_COMMONPAGESIZE
#define ELF_COMMONPAGESIZE ELF_MAXPAGESIZE
#endif

#ifndef ELF_RELROPAGESIZE
#define ELF_RELROPAGESIZE ELF_COMMONPAGESIZE
#endif

#ifndef ELF_MINPAGESIZE
#define ELF_MINPAGESIZE ELF_COMMONPAGESIZE
#endif

#if ELF_COMMONPAGESIZE > ELF_MAXPAGESIZE
# error ELF_COMMONPAGESIZE > ELF_MAXPAGESIZE
#endif
#if ELF_RELROPAGESIZE > ELF_MAXPAGESIZE
# error ELF_RELROPAGESIZE > ELF_MAXPAGESIZE
#endif
#if ELF_MINPAGESIZE > ELF_COMMONPAGESIZE
# error ELF_MINPAGESIZE > ELF_COMMONPAGESIZE
#endif
#if ELF_MINPAGESIZE > ELF_RELROPAGESIZE
# error ELF_MINPAGESIZE > ELF_RELROPAGESIZE
#endif

#ifndef ELF_DYNAMIC_SEC_FLAGS
/* Note that we set the SEC_IN_MEMORY flag for these sections.  */
#define ELF_DYNAMIC_SEC_FLAGS			\
  (SEC_ALLOC | SEC_LOAD | SEC_HAS_CONTENTS	\
   | SEC_IN_MEMORY | SEC_LINKER_CREATED)
#endif

#ifndef elf_backend_collect
#define elf_backend_collect FALSE
#endif
#ifndef elf_backend_type_change_ok
#define elf_backend_type_change_ok FALSE
#endif

#ifndef elf_backend_sym_is_global
#define elf_backend_sym_is_global	0
#endif
#ifndef elf_backend_object_p
#define elf_backend_object_p		0
#endif
#ifndef elf_backend_symbol_processing
#define elf_backend_symbol_processing	0
#endif
#ifndef elf_backend_symbol_table_processing
#define elf_backend_symbol_table_processing	0
#endif
#ifndef elf_backend_get_symbol_type
#define elf_backend_get_symbol_type 0
#endif
#ifndef elf_backend_archive_symbol_lookup
#define elf_backend_archive_symbol_lookup _bfd_elf_archive_symbol_lookup
#endif
#ifndef elf_backend_name_local_section_symbols
#define elf_backend_name_local_section_symbols	0
#endif
#ifndef elf_backend_section_processing
#define elf_backend_section_processing	0
#endif
#ifndef elf_backend_section_from_shdr
#define elf_backend_section_from_shdr	_bfd_elf_make_section_from_shdr
#endif
#ifndef elf_backend_section_flags
#define elf_backend_section_flags	0
#endif
#ifndef elf_backend_get_sec_type_attr
#define elf_backend_get_sec_type_attr	_bfd_elf_get_sec_type_attr
#endif
#ifndef elf_backend_section_from_phdr
#define elf_backend_section_from_phdr	_bfd_elf_make_section_from_phdr
#endif
#ifndef elf_backend_fake_sections
#define elf_backend_fake_sections	0
#endif
#ifndef elf_backend_section_from_bfd_section
#define elf_backend_section_from_bfd_section	0
#endif
#ifndef elf_backend_add_symbol_hook
#define elf_backend_add_symbol_hook	0
#endif
#ifndef elf_backend_link_output_symbol_hook
#define elf_backend_link_output_symbol_hook 0
#endif
#ifndef elf_backend_create_dynamic_sections
#define elf_backend_create_dynamic_sections 0
#endif
#ifndef elf_backend_omit_section_dynsym
#define elf_backend_omit_section_dynsym _bfd_elf_omit_section_dynsym_default
#endif
#ifndef elf_backend_relocs_compatible
#define elf_backend_relocs_compatible _bfd_elf_default_relocs_compatible
#endif
#ifndef elf_backend_check_relocs
#define elf_backend_check_relocs	0
#endif
#ifndef elf_backend_check_directives
#define elf_backend_check_directives	0
#endif
#ifndef elf_backend_notice_as_needed
#define elf_backend_notice_as_needed	_bfd_elf_notice_as_needed
#endif
#ifndef elf_backend_adjust_dynamic_symbol
#define elf_backend_adjust_dynamic_symbol 0
#endif
#ifndef elf_backend_always_size_sections
#define elf_backend_always_size_sections 0
#endif
#ifndef elf_backend_size_dynamic_sections
#define elf_backend_size_dynamic_sections 0
#endif
#ifndef elf_backend_init_index_section
#define elf_backend_init_index_section _bfd_void_bfd_link
#endif
#ifndef elf_backend_relocate_section
#define elf_backend_relocate_section	0
#endif
#ifndef elf_backend_finish_dynamic_symbol
#define elf_backend_finish_dynamic_symbol	0
#endif
#ifndef elf_backend_finish_dynamic_sections
#define elf_backend_finish_dynamic_sections	0
#endif
#ifndef elf_backend_begin_write_processing
#define elf_backend_begin_write_processing	0
#endif
#ifndef elf_backend_final_write_processing
#define elf_backend_final_write_processing	0
#endif
#ifndef elf_backend_additional_program_headers
#define elf_backend_additional_program_headers	0
#endif
#ifndef elf_backend_modify_segment_map
#define elf_backend_modify_segment_map	0
#endif
#ifndef elf_backend_modify_program_headers
#define elf_backend_modify_program_headers	0
#endif
#ifndef elf_backend_allow_non_load_phdr
#define elf_backend_allow_non_load_phdr	0
#endif
#ifndef elf_backend_ecoff_debug_swap
#define elf_backend_ecoff_debug_swap	0
#endif
#ifndef elf_backend_bfd_from_remote_memory
#define elf_backend_bfd_from_remote_memory _bfd_elf64_bfd_from_remote_memory
#endif
#ifndef elf_backend_got_header_size
#define elf_backend_got_header_size	0
#endif
#ifndef elf_backend_got_elt_size
#define elf_backend_got_elt_size _bfd_elf_default_got_elt_size
#endif
#ifndef elf_backend_obj_attrs_vendor
#define elf_backend_obj_attrs_vendor		NULL
#endif
#ifndef elf_backend_obj_attrs_section
#define elf_backend_obj_attrs_section		NULL
#endif
#ifndef elf_backend_obj_attrs_arg_type
#define elf_backend_obj_attrs_arg_type		NULL
#endif
#ifndef elf_backend_obj_attrs_section_type
#define elf_backend_obj_attrs_section_type		SHT_GNU_ATTRIBUTES
#endif
#ifndef elf_backend_obj_attrs_order
#define elf_backend_obj_attrs_order		NULL
#endif
#ifndef elf_backend_obj_attrs_handle_unknown
#define elf_backend_obj_attrs_handle_unknown	NULL
#endif
#ifndef elf_backend_parse_gnu_properties
#define elf_backend_parse_gnu_properties	NULL
#endif
#ifndef elf_backend_merge_gnu_properties
#define elf_backend_merge_gnu_properties	NULL
#endif
#ifndef elf_backend_setup_gnu_properties
#define elf_backend_setup_gnu_properties	_bfd_elf_link_setup_gnu_properties
#endif
#ifndef elf_backend_fixup_gnu_properties
#define elf_backend_fixup_gnu_properties	NULL
#endif
#ifndef elf_backend_static_tls_alignment
#define elf_backend_static_tls_alignment	1
#endif
#ifndef elf_backend_post_process_headers
#define elf_backend_post_process_headers	_bfd_elf_post_process_headers
#endif
#ifndef elf_backend_print_symbol_all
#define elf_backend_print_symbol_all		NULL
#endif
#ifndef elf_backend_output_arch_local_syms
#define elf_backend_output_arch_local_syms	NULL
#endif
#ifndef elf_backend_output_arch_syms
#define elf_backend_output_arch_syms		NULL
#endif
#ifndef elf_backend_filter_implib_symbols
#define elf_backend_filter_implib_symbols	NULL
#endif
#ifndef elf_backend_copy_indirect_symbol
#define elf_backend_copy_indirect_symbol	_bfd_elf_link_hash_copy_indirect
#endif
#ifndef elf_backend_hide_symbol
#define elf_backend_hide_symbol			_bfd_elf_link_hash_hide_symbol
#endif
#ifndef elf_backend_fixup_symbol
#define elf_backend_fixup_symbol		NULL
#endif
#ifndef elf_backend_merge_symbol_attribute
#define elf_backend_merge_symbol_attribute	NULL
#endif
#ifndef elf_backend_get_target_dtag
#define elf_backend_get_target_dtag		NULL
#endif
#ifndef elf_backend_ignore_undef_symbol
#define elf_backend_ignore_undef_symbol		NULL
#endif
#ifndef elf_backend_emit_relocs
#define elf_backend_emit_relocs			_bfd_elf_link_output_relocs
#endif
#ifndef elf_backend_update_relocs
#define elf_backend_update_relocs		NULL
#endif
#ifndef elf_backend_count_relocs
#define elf_backend_count_relocs		NULL
#endif
#ifndef elf_backend_count_additional_relocs
#define elf_backend_count_additional_relocs	NULL
#endif
#ifndef elf_backend_sort_relocs_p
#define elf_backend_sort_relocs_p		NULL
#endif
#ifndef elf_backend_grok_prstatus
#define elf_backend_grok_prstatus		NULL
#endif
#ifndef elf_backend_grok_psinfo
#define elf_backend_grok_psinfo			NULL
#endif
#ifndef elf_backend_grok_freebsd_prstatus
#define elf_backend_grok_freebsd_prstatus	NULL
#endif
#ifndef elf_backend_write_core_note
#define elf_backend_write_core_note		NULL
#endif
#ifndef elf_backend_lookup_section_flags_hook
#define elf_backend_lookup_section_flags_hook	NULL
#endif
#ifndef elf_backend_reloc_type_class
#define elf_backend_reloc_type_class		_bfd_elf_reloc_type_class
#endif
#ifndef elf_backend_discard_info
#define elf_backend_discard_info		NULL
#endif
#ifndef elf_backend_ignore_discarded_relocs
#define elf_backend_ignore_discarded_relocs	NULL
#endif
#ifndef elf_backend_action_discarded
#define elf_backend_action_discarded _bfd_elf_default_action_discarded
#endif
#ifndef elf_backend_eh_frame_address_size
#define elf_backend_eh_frame_address_size _bfd_elf_eh_frame_address_size
#endif
#ifndef elf_backend_can_make_relative_eh_frame
#define elf_backend_can_make_relative_eh_frame	_bfd_elf_can_make_relative
#endif
#ifndef elf_backend_can_make_lsda_relative_eh_frame
#define elf_backend_can_make_lsda_relative_eh_frame	_bfd_elf_can_make_relative
#endif
#ifndef elf_backend_encode_eh_address
#define elf_backend_encode_eh_address		_bfd_elf_encode_eh_address
#endif
#ifndef elf_backend_write_section
#define elf_backend_write_section		NULL
#endif
#ifndef elf_backend_mips_irix_compat
#define elf_backend_mips_irix_compat		NULL
#endif
#ifndef elf_backend_mips_rtype_to_howto
#define elf_backend_mips_rtype_to_howto		NULL
#endif

/* Previously, backends could only use SHT_REL or SHT_RELA relocation
   sections, but not both.  They defined USE_REL to indicate SHT_REL
   sections, and left it undefined to indicated SHT_RELA sections.
   For backwards compatibility, we still support this usage.  */
#ifndef USE_REL
#define USE_REL 0
#endif

/* Use these in new code.  */
#ifndef elf_backend_may_use_rel_p
#define elf_backend_may_use_rel_p USE_REL
#endif
#ifndef elf_backend_may_use_rela_p
#define elf_backend_may_use_rela_p !USE_REL
#endif
#ifndef elf_backend_default_use_rela_p
#define elf_backend_default_use_rela_p !USE_REL
#endif
#ifndef elf_backend_rela_plts_and_copies_p
#define elf_backend_rela_plts_and_copies_p elf_backend_default_use_rela_p
#endif

#ifndef elf_backend_rela_normal
#define elf_backend_rela_normal 0
#endif

#ifndef elf_backend_dtrel_excludes_plt
#define elf_backend_dtrel_excludes_plt 0
#endif

#ifndef elf_backend_plt_sym_val
#define elf_backend_plt_sym_val NULL
#endif
#ifndef elf_backend_relplt_name
#define elf_backend_relplt_name NULL
#endif

#ifndef ELF_MACHINE_ALT1
#define ELF_MACHINE_ALT1 0
#endif

#ifndef ELF_MACHINE_ALT2
#define ELF_MACHINE_ALT2 0
#endif

#ifndef elf_backend_size_info
#define elf_backend_size_info _bfd_elf64_size_info
#endif

#ifndef elf_backend_special_sections
#define elf_backend_special_sections NULL
#endif

#ifndef elf_backend_sign_extend_vma
#define elf_backend_sign_extend_vma 0
#endif

#ifndef elf_backend_link_order_error_handler
#define elf_backend_link_order_error_handler _bfd_error_handler
#endif

#ifndef elf_backend_common_definition
#define elf_backend_common_definition _bfd_elf_common_definition
#endif

#ifndef elf_backend_common_section_index
#define elf_backend_common_section_index _bfd_elf_common_section_index
#endif

#ifndef elf_backend_common_section
#define elf_backend_common_section _bfd_elf_common_section
#endif

#ifndef elf_backend_merge_symbol
#define elf_backend_merge_symbol NULL
#endif

#ifndef elf_backend_hash_symbol
#define elf_backend_hash_symbol _bfd_elf_hash_symbol
#endif

#ifndef elf_backend_is_function_type
#define elf_backend_is_function_type _bfd_elf_is_function_type
#endif

#ifndef elf_backend_maybe_function_sym
#define elf_backend_maybe_function_sym _bfd_elf_maybe_function_sym
#endif

#ifndef elf_backend_get_reloc_section
#define elf_backend_get_reloc_section _bfd_elf_plt_get_reloc_section
#endif

#ifndef elf_backend_copy_special_section_fields
#define elf_backend_copy_special_section_fields NULL
#endif

#ifndef elf_backend_compact_eh_encoding
#define elf_backend_compact_eh_encoding NULL
#endif

#ifndef elf_backend_cant_unwind_opcode
#define elf_backend_cant_unwind_opcode 0
#endif

#ifndef elf_match_priority
#define elf_match_priority \
  (ELF_ARCH == bfd_arch_unknown ? 2 : ELF_OSABI == ELFOSABI_NONE ? 1 : 0)
#endif

extern const struct elf_size_info _bfd_elf64_size_info;

/* Forward declaration for use when initialising alternative_target field.  */
extern const bfd_target TARGET_LITTLE_SYM;

const bfd_target TARGET_LITTLE_SYM =
{
  /* name: identify kind of target */
  TARGET_LITTLE_NAME,

  /* flavour: general indication about file */
  bfd_target_elf_flavour,

  /* byteorder: data is little endian */
  BFD_ENDIAN_LITTLE,

  /* header_byteorder: header is also little endian */
  BFD_ENDIAN_LITTLE,

  /* object_flags: mask of all file flags */
  (HAS_RELOC | EXEC_P | HAS_LINENO | HAS_DEBUG | HAS_SYMS | HAS_LOCALS
   | DYNAMIC | WP_TEXT | D_PAGED | BFD_COMPRESS | BFD_DECOMPRESS
   | BFD_COMPRESS_GABI | BFD_CONVERT_ELF_COMMON | BFD_USE_ELF_STT_COMMON),

  /* section_flags: mask of all section flags */
  (SEC_HAS_CONTENTS | SEC_ALLOC | SEC_LOAD | SEC_RELOC | SEC_READONLY
   | SEC_CODE | SEC_DATA | SEC_DEBUGGING | SEC_EXCLUDE | SEC_SORT_ENTRIES
   | SEC_SMALL_DATA | SEC_MERGE | SEC_STRINGS | SEC_GROUP),

   /* leading_symbol_char: is the first char of a user symbol
      predictable, and if so what is it */
  elf_symbol_leading_char,

  /* ar_pad_char: pad character for filenames within an archive header
     FIXME:  this really has nothing to do with ELF, this is a characteristic
     of the archiver and/or os and should be independently tunable */
  '/',

  /* ar_max_namelen: maximum number of characters in an archive header
     FIXME:  this really has nothing to do with ELF, this is a characteristic
     of the archiver and should be independently tunable.  The System V ABI,
     Chapter 7 (Formats & Protocols), Archive section sets this as 15.  */
  15,

  elf_match_priority,

  /* Routines to byte-swap various sized integers from the data sections */
  bfd_getl64, bfd_getl_signed_64, bfd_putl64,
    bfd_getl32, bfd_getl_signed_32, bfd_putl32,
    bfd_getl16, bfd_getl_signed_16, bfd_putl16,

  /* Routines to byte-swap various sized integers from the file headers */
  bfd_getl64, bfd_getl_signed_64, bfd_putl64,
    bfd_getl32, bfd_getl_signed_32, bfd_putl32,
    bfd_getl16, bfd_getl_signed_16, bfd_putl16,

  /* bfd_check_format: check the format of a file being read */
  { _bfd_dummy_target,		/* unknown format */
    bfd_elf64_object_p,		/* assembler/linker output (object file) */
    _bfd_dummy_target,		/* unknown format */
    _bfd_dummy_target,		/* unknown format */
  },

  /* bfd_set_format: set the format of a file being written */
  { _bfd_bool_bfd_false_error,
    bfd_elf64_mkobject,
    _bfd_bool_bfd_false_error,
    _bfd_bool_bfd_false_error,
  },

  /* bfd_write_contents: write cached information into a file being written */
  { _bfd_bool_bfd_false_error,
    _bfd_bool_bfd_false_error,
    _bfd_bool_bfd_false_error,
    _bfd_bool_bfd_false_error,
  },

  BFD_JUMP_TABLE_GENERIC (bfd_elf64),
  BFD_JUMP_TABLE_COPY (bfd_elf64),
  BFD_JUMP_TABLE_CORE (_bfd_nocore),
  BFD_JUMP_TABLE_ARCHIVE (_bfd_noarchive),
  BFD_JUMP_TABLE_SYMBOLS (bfd_elf64),
  BFD_JUMP_TABLE_RELOCS (_bfd_norelocs),
  BFD_JUMP_TABLE_WRITE (_bfd_nowrite),
  BFD_JUMP_TABLE_LINK (_bfd_nolink),
  BFD_JUMP_TABLE_DYNAMIC (_bfd_nodynamic),

  /* Alternative endian target.  */
  NULL,

  /* backend_data: */
  NULL,
};
