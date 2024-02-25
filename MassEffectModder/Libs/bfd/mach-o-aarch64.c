/* AArch-64 Mach-O support for BFD.
   Copyright (C) 2015-2019 Free Software Foundation, Inc.

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
#include "libiberty.h"
#include "mach-o.h"
#include "mach-o/arm64.h"

#define bfd_mach_o_object_p bfd_mach_o_arm64_object_p
#define bfd_mach_o_core_p bfd_mach_o_arm64_core_p
#define bfd_mach_o_mkobject bfd_mach_o_arm64_mkobject

#define bfd_mach_o_canonicalize_one_reloc \
  bfd_mach_o_arm64_canonicalize_one_reloc
#define bfd_mach_o_swap_reloc_out NULL

#define bfd_mach_o_bfd_reloc_type_lookup bfd_mach_o_arm64_bfd_reloc_type_lookup
#define bfd_mach_o_bfd_reloc_name_lookup bfd_mach_o_arm64_bfd_reloc_name_lookup

#define bfd_mach_o_print_thread NULL
#define bfd_mach_o_tgt_seg_table NULL
#define bfd_mach_o_section_type_valid_for_tgt NULL

static const bfd_target *
bfd_mach_o_arm64_object_p (bfd *abfd)
{
  return bfd_mach_o_header_p (abfd, 0, 0, BFD_MACH_O_CPU_TYPE_ARM64);
}

/* In case we're on a 32-bit machine, construct a 64-bit "-1" value.  */
#define MINUS_ONE (~ (bfd_vma) 0)

static reloc_howto_type arm64_howto_table[]=
{
  /* 0 */
  HOWTO (BFD_RELOC_64, 0, 4, 64, FALSE, 0,
	 complain_overflow_bitfield,
	 NULL, "64",
	 FALSE, MINUS_ONE, MINUS_ONE, FALSE),
  HOWTO (BFD_RELOC_32, 0, 2, 32, FALSE, 0,
	 complain_overflow_bitfield,
	 NULL, "32",
	 FALSE, 0xffffffff, 0xffffffff, FALSE),
  HOWTO (BFD_RELOC_16, 0, 1, 16, FALSE, 0,
	 complain_overflow_bitfield,
	 NULL, "16",
	 FALSE, 0xffff, 0xffff, FALSE),
  HOWTO (BFD_RELOC_8, 0, 0, 8, FALSE, 0,
	 complain_overflow_bitfield,
	 NULL, "8",
	 FALSE, 0xff, 0xff, FALSE),
  /* 4 */
  HOWTO (BFD_RELOC_64_PCREL, 0, 4, 64, TRUE, 0,
	 complain_overflow_bitfield,
	 NULL, "DISP64",
	 FALSE, MINUS_ONE, MINUS_ONE, TRUE),
  HOWTO (BFD_RELOC_32_PCREL, 0, 2, 32, TRUE, 0,
	 complain_overflow_bitfield,
	 NULL, "DISP32",
	 FALSE, 0xffffffff, 0xffffffff, TRUE),
  HOWTO (BFD_RELOC_16_PCREL, 0, 1, 16, TRUE, 0,
	 complain_overflow_bitfield,
	 NULL, "DISP16",
	 FALSE, 0xffff, 0xffff, TRUE),
  HOWTO (BFD_RELOC_AARCH64_CALL26, 0, 2, 26, TRUE, 0,
	 complain_overflow_bitfield,
	 NULL, "BRANCH26",
	 FALSE, 0x03ffffff, 0x03ffffff, TRUE),
  /* 8 */
  HOWTO (BFD_RELOC_AARCH64_ADR_HI21_PCREL, 12, 2, 21, TRUE, 0,
	 complain_overflow_signed,
	 NULL, "PAGE21",
	 FALSE, 0x1fffff, 0x1fffff, TRUE),
  HOWTO (BFD_RELOC_AARCH64_LDST16_LO12, 1, 2, 12, TRUE, 0,
	 complain_overflow_signed,
	 NULL, "PGOFF12",
	 FALSE, 0xffe, 0xffe, TRUE),
  HOWTO (BFD_RELOC_MACH_O_ARM64_ADDEND, 0, 2, 32, FALSE, 0,
	 complain_overflow_signed,
	 NULL, "ADDEND",
	 FALSE, 0xffffffff, 0xffffffff, FALSE),
  HOWTO (BFD_RELOC_MACH_O_SUBTRACTOR32, 0, 2, 32, FALSE, 0,
	 complain_overflow_bitfield,
	 NULL, "SUBTRACTOR32",
	 FALSE, 0xffffffff, 0xffffffff, FALSE),
  /* 12 */
  HOWTO (BFD_RELOC_MACH_O_SUBTRACTOR64, 0, 4, 64, FALSE, 0,
	 complain_overflow_bitfield,
	 NULL, "SUBTRACTOR64",
	 FALSE, MINUS_ONE, MINUS_ONE, FALSE),
  HOWTO (BFD_RELOC_MACH_O_ARM64_GOT_LOAD_PAGE21, 12, 2, 21, TRUE, 0,
	 complain_overflow_signed,
	 NULL, "GOT_LD_PG21",
	 FALSE, 0x1fffff, 0x1fffff, TRUE),
  HOWTO (BFD_RELOC_MACH_O_ARM64_GOT_LOAD_PAGEOFF12, 1, 2, 12, TRUE, 0,
	 complain_overflow_signed,
	 NULL, "GOT_LD_PGOFF12",
	 FALSE, 0xffe, 0xffe, TRUE),
  HOWTO (BFD_RELOC_MACH_O_ARM64_POINTER_TO_GOT, 0, 2, 32, TRUE, 0,
	 complain_overflow_bitfield,
	 NULL, "PTR_TO_GOT",
	 FALSE, 0xffffffff, 0xffffffff, TRUE),
};

static bfd_boolean
bfd_mach_o_arm64_canonicalize_one_reloc (bfd *       abfd,
					 struct mach_o_reloc_info_external * raw,
					 arelent *   res,
					 asymbol **  syms,
					 arelent *   res_base ATTRIBUTE_UNUSED)
{
  bfd_mach_o_reloc_info reloc;

  res->address = bfd_get_32 (abfd, raw->r_address);
  if (res->address & BFD_MACH_O_SR_SCATTERED)
    {
      /* Only non-scattered relocations.  */
      return FALSE;
    }

  /* The value and info fields have to be extracted dependent on target
     endian-ness.  */
  bfd_mach_o_swap_in_non_scattered_reloc (abfd, &reloc, raw->r_symbolnum);

  if (reloc.r_type == BFD_MACH_O_ARM64_RELOC_ADDEND)
    {
      if (reloc.r_length == 2 && reloc.r_pcrel == 0)
	{
	  res->sym_ptr_ptr = bfd_abs_section_ptr->symbol_ptr_ptr;
	  res->addend = reloc.r_value;
	  res->howto = &arm64_howto_table[10];
	  return TRUE;
	}
      return FALSE;
    }

  if (!bfd_mach_o_canonicalize_non_scattered_reloc (abfd, &reloc, res, syms))
    return FALSE;

  switch (reloc.r_type)
    {
    case BFD_MACH_O_ARM64_RELOC_UNSIGNED:
      switch ((reloc.r_length << 1) | reloc.r_pcrel)
	{
	case 0: /* len = 0, pcrel = 0  */
	  res->howto = &arm64_howto_table[3];
	  return TRUE;
	case 2: /* len = 1, pcrel = 0  */
	  res->howto = &arm64_howto_table[2];
	  return TRUE;
	case 3: /* len = 1, pcrel = 1  */
	  res->howto = &arm64_howto_table[6];
	  return TRUE;
	case 4: /* len = 2, pcrel = 0  */
	  res->howto = &arm64_howto_table[1];
	  return TRUE;
	case 5: /* len = 2, pcrel = 1  */
	  res->howto = &arm64_howto_table[5];
	  return TRUE;
	case 6: /* len = 3, pcrel = 0  */
	  res->howto = &arm64_howto_table[0];
	  return TRUE;
	case 7: /* len = 3, pcrel = 1  */
	  res->howto = &arm64_howto_table[4];
	  return TRUE;
	default:
	  return FALSE;
	}
      break;
    case BFD_MACH_O_ARM64_RELOC_SUBTRACTOR:
      if (reloc.r_pcrel)
	return FALSE;
      switch (reloc.r_length)
	{
	case 2:
	  res->howto = &arm64_howto_table[11];
	  return TRUE;
	case 3:
	  res->howto = &arm64_howto_table[12];
	  return TRUE;
	default:
	  return FALSE;
	}
      break;
    case BFD_MACH_O_ARM64_RELOC_BRANCH26:
      if (reloc.r_length == 2 && reloc.r_pcrel == 1)
	{
	  res->howto = &arm64_howto_table[7];
	  return TRUE;
	}
      break;
    case BFD_MACH_O_ARM64_RELOC_PAGE21:
      if (reloc.r_length == 2 && reloc.r_pcrel == 1)
	{
	  res->howto = &arm64_howto_table[8];
	  return TRUE;
	}
      break;
    case BFD_MACH_O_ARM64_RELOC_PAGEOFF12:
      if (reloc.r_length == 2 && reloc.r_pcrel == 0)
	{
	  res->howto = &arm64_howto_table[9];
	  return TRUE;
	}
      break;
    case BFD_MACH_O_ARM64_RELOC_GOT_LOAD_PAGE21:
      if (reloc.r_length == 2 && reloc.r_pcrel == 1)
	{
	  res->howto = &arm64_howto_table[13];
	  return TRUE;
	}
      break;
    case BFD_MACH_O_ARM64_RELOC_GOT_LOAD_PAGEOFF12:
      if (reloc.r_length == 2 && reloc.r_pcrel == 0)
	{
	  res->howto = &arm64_howto_table[14];
	  return TRUE;
	}
      break;
    case BFD_MACH_O_ARM64_RELOC_POINTER_TO_GOT:
      if (reloc.r_length == 2 && reloc.r_pcrel == 1)
	{
	  res->howto = &arm64_howto_table[15];
	  return TRUE;
	}
      break;
    default:
      break;
    }
  return FALSE;
}

#define TARGET_NAME		aarch64_mach_o_vec
#define TARGET_STRING		"mach-o-arm64"
#define TARGET_ARCHITECTURE	bfd_arch_aarch64
#define TARGET_PAGESIZE		4096
#define TARGET_BIG_ENDIAN	0
#define TARGET_ARCHIVE		0
#define TARGET_PRIORITY		0


#define bfd_mach_o_bfd_free_cached_info		      _bfd_generic_bfd_free_cached_info
#define bfd_mach_o_get_section_contents_in_window     _bfd_generic_get_section_contents_in_window
#define bfd_mach_o_bfd_print_private_bfd_data	      bfd_mach_o_bfd_print_private_bfd_data
#define bfd_mach_o_bfd_is_target_special_symbol	      _bfd_bool_bfd_asymbol_false
#define bfd_mach_o_bfd_is_local_label_name	      bfd_generic_is_local_label_name
#define bfd_mach_o_get_lineno			      _bfd_nosymbols_get_lineno
#define bfd_mach_o_find_inliner_info		      _bfd_nosymbols_find_inliner_info
#define bfd_mach_o_get_symbol_version_string	      _bfd_nosymbols_get_symbol_version_string
#define bfd_mach_o_bfd_make_debug_symbol	      _bfd_nosymbols_bfd_make_debug_symbol
#define bfd_mach_o_read_minisymbols		      _bfd_generic_read_minisymbols
#define bfd_mach_o_minisymbol_to_symbol		      _bfd_generic_minisymbol_to_symbol
#define bfd_mach_o_bfd_get_relocated_section_contents bfd_generic_get_relocated_section_contents
#define bfd_mach_o_bfd_relax_section		      bfd_generic_relax_section
#define bfd_mach_o_bfd_link_hash_table_create	      _bfd_generic_link_hash_table_create
#define bfd_mach_o_bfd_link_add_symbols		      _bfd_generic_link_add_symbols
#define bfd_mach_o_bfd_link_just_syms		      _bfd_generic_link_just_syms
#define bfd_mach_o_bfd_copy_link_hash_symbol_type \
  _bfd_generic_copy_link_hash_symbol_type
#define bfd_mach_o_bfd_final_link		      _bfd_generic_final_link
#define bfd_mach_o_bfd_link_split_section	      _bfd_generic_link_split_section
#define bfd_mach_o_bfd_link_check_relocs	      _bfd_generic_link_check_relocs
#define bfd_mach_o_bfd_merge_private_bfd_data	      _bfd_generic_bfd_merge_private_bfd_data
#define bfd_mach_o_bfd_set_private_flags	      bfd_mach_o_bfd_set_private_flags
#define bfd_mach_o_get_section_contents		      _bfd_generic_get_section_contents
#define bfd_mach_o_bfd_gc_sections		      bfd_generic_gc_sections
#define bfd_mach_o_bfd_lookup_section_flags	      bfd_generic_lookup_section_flags
#define bfd_mach_o_bfd_merge_sections		      bfd_generic_merge_sections
#define bfd_mach_o_bfd_is_group_section		      bfd_generic_is_group_section
#define bfd_mach_o_bfd_discard_group		      bfd_generic_discard_group
#define bfd_mach_o_section_already_linked	      _bfd_generic_section_already_linked
#define bfd_mach_o_bfd_define_common_symbol	      bfd_generic_define_common_symbol
#define bfd_mach_o_bfd_link_hide_symbol		      _bfd_generic_link_hide_symbol
#define bfd_mach_o_bfd_define_start_stop	      bfd_generic_define_start_stop
#define bfd_mach_o_bfd_copy_private_bfd_data	      _bfd_generic_bfd_copy_private_bfd_data
#define bfd_mach_o_core_file_matches_executable_p     generic_core_file_matches_executable_p
#define bfd_mach_o_core_file_pid		      _bfd_nocore_core_file_pid
#define bfd_mach_o_set_reloc			      _bfd_generic_set_reloc

#define bfd_mach_o_get_dynamic_symtab_upper_bound     bfd_mach_o_get_symtab_upper_bound
#define bfd_mach_o_canonicalize_dynamic_symtab	      bfd_mach_o_canonicalize_symtab

/* For Mach-O special archives.  */
#define bfd_mach_o_read_ar_hdr			  _bfd_noarchive_read_ar_hdr
#define bfd_mach_o_write_ar_hdr			  _bfd_noarchive_write_ar_hdr
#define bfd_mach_o_slurp_armap			  _bfd_noarchive_slurp_armap
#define bfd_mach_o_slurp_extended_name_table	  _bfd_noarchive_slurp_extended_name_table
#define bfd_mach_o_construct_extended_name_table  _bfd_noarchive_construct_extended_name_table
#define bfd_mach_o_truncate_arname		  _bfd_noarchive_truncate_arname
#define bfd_mach_o_write_armap			  _bfd_noarchive_write_armap
#define bfd_mach_o_get_elt_at_index		  _bfd_noarchive_get_elt_at_index
#define bfd_mach_o_update_armap_timestamp	  _bfd_noarchive_update_armap_timestamp

#define TARGET_NAME_BACKEND XCONCAT2(TARGET_NAME,_backend)

#ifndef TARGET_NAME
#error TARGET_NAME must be defined
#endif /* TARGET_NAME */

#ifndef TARGET_STRING
#error TARGET_STRING must be defined
#endif /* TARGET_STRING */

#ifndef TARGET_ARCHITECTURE
#error TARGET_ARCHITECTURE must be defined
#endif /* TARGET_ARCHITECTURE */

#ifndef TARGET_BIG_ENDIAN
#error TARGET_BIG_ENDIAN must be defined
#endif /* TARGET_BIG_ENDIAN */

#ifndef TARGET_ARCHIVE
#error TARGET_ARCHIVE must be defined
#endif /* TARGET_ARCHIVE */

#ifndef TARGET_PAGESIZE
#error TARGET_PAGESIZE must be defined
#endif

static const bfd_mach_o_backend_data TARGET_NAME_BACKEND =
{
  TARGET_ARCHITECTURE,
  TARGET_PAGESIZE,
  bfd_mach_o_canonicalize_one_reloc,
  bfd_mach_o_swap_reloc_out,
  bfd_mach_o_print_thread,
  bfd_mach_o_tgt_seg_table,
  bfd_mach_o_section_type_valid_for_tgt
};

const bfd_target TARGET_NAME =
{
  TARGET_STRING,		/* Name.  */
  bfd_target_mach_o_flavour,
  BFD_ENDIAN_LITTLE,		/* Target byte order.  */
  BFD_ENDIAN_LITTLE,		/* Target headers byte order.  */
  (HAS_RELOC | EXEC_P |		/* Object flags.  */
   HAS_LINENO | HAS_DEBUG |
   HAS_SYMS | HAS_LOCALS | DYNAMIC | WP_TEXT | D_PAGED),
  (SEC_CODE | SEC_DATA | SEC_ROM | SEC_HAS_CONTENTS
   | SEC_ALLOC | SEC_LOAD | SEC_RELOC),	/* Section flags.  */
  '_',				/* symbol_leading_char.  */
  ' ',				/* ar_pad_char.  */
  16,				/* ar_max_namelen.  */
  TARGET_PRIORITY,	/* match priority.  */

  bfd_getl64, bfd_getl_signed_64, bfd_putl64,
  bfd_getl32, bfd_getl_signed_32, bfd_putl32,
  bfd_getl16, bfd_getl_signed_16, bfd_putl16,	/* data */
  bfd_getl64, bfd_getl_signed_64, bfd_putl64,
  bfd_getl32, bfd_getl_signed_32, bfd_putl32,
  bfd_getl16, bfd_getl_signed_16, bfd_putl16,	/* hdrs */

  {				/* bfd_check_format.  */
    _bfd_dummy_target,
    bfd_mach_o_object_p,
    _bfd_dummy_target,
    _bfd_dummy_target
  },
  {				/* bfd_set_format.  */
    _bfd_bool_bfd_false_error,
    _bfd_bool_bfd_false_error,
    _bfd_bool_bfd_false_error,
    _bfd_bool_bfd_false_error,
  },
  {				/* bfd_write_contents.  */
    _bfd_bool_bfd_false_error,
    _bfd_bool_bfd_false_error,
    _bfd_bool_bfd_false_error,
    _bfd_bool_bfd_false_error,
  },

  BFD_JUMP_TABLE_GENERIC (bfd_mach_o),
  BFD_JUMP_TABLE_COPY (bfd_mach_o),
  BFD_JUMP_TABLE_CORE (_bfd_nocore),
  BFD_JUMP_TABLE_ARCHIVE (_bfd_noarchive),
  BFD_JUMP_TABLE_SYMBOLS (bfd_mach_o),
  BFD_JUMP_TABLE_RELOCS (_bfd_norelocs),
  BFD_JUMP_TABLE_WRITE (_bfd_nowrite),
  BFD_JUMP_TABLE_LINK (_bfd_nolink),
  BFD_JUMP_TABLE_DYNAMIC (_bfd_nodynamic),

  /* Alternative endian target.  */
  NULL,

  /* Back-end data.  */
  &TARGET_NAME_BACKEND
};
