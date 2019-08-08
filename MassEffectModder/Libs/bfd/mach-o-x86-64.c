/* Intel x86-64 Mach-O support for BFD.
   Copyright (C) 2010-2019 Free Software Foundation, Inc.

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
#include "mach-o/x86-64.h"

#define bfd_mach_o_object_p bfd_mach_o_x86_64_object_p
#define bfd_mach_o_core_p bfd_mach_o_x86_64_core_p
#define bfd_mach_o_mkobject bfd_mach_o_x86_64_mkobject

static const bfd_target *
bfd_mach_o_x86_64_object_p (bfd *abfd)
{
  return bfd_mach_o_header_p (abfd, 0, 0, BFD_MACH_O_CPU_TYPE_X86_64);
}

/* In case we're on a 32-bit machine, construct a 64-bit "-1" value.  */
#define MINUS_ONE (~ (bfd_vma) 0)

static reloc_howto_type x86_64_howto_table[]=
{
  /* 0 */
  HOWTO(BFD_RELOC_64, 0, 4, 64, FALSE, 0,
	complain_overflow_bitfield,
	NULL, "64",
	FALSE, MINUS_ONE, MINUS_ONE, FALSE),
  HOWTO(BFD_RELOC_32, 0, 2, 32, FALSE, 0,
	complain_overflow_bitfield,
	NULL, "32",
	FALSE, 0xffffffff, 0xffffffff, FALSE),
  HOWTO(BFD_RELOC_32_PCREL, 0, 2, 32, TRUE, 0,
	complain_overflow_bitfield,
	NULL, "DISP32",
	FALSE, 0xffffffff, 0xffffffff, TRUE),
  HOWTO(BFD_RELOC_MACH_O_X86_64_PCREL32_1, 0, 2, 32, TRUE, 0,
	complain_overflow_bitfield,
	NULL, "DISP32_1",
	FALSE, 0xffffffff, 0xffffffff, TRUE),
  /* 4 */
  HOWTO(BFD_RELOC_MACH_O_X86_64_PCREL32_2, 0, 2, 32, TRUE, 0,
	complain_overflow_bitfield,
	NULL, "DISP32_2",
	FALSE, 0xffffffff, 0xffffffff, TRUE),
  HOWTO(BFD_RELOC_MACH_O_X86_64_PCREL32_4, 0, 2, 32, TRUE, 0,
	complain_overflow_bitfield,
	NULL, "DISP32_4",
	FALSE, 0xffffffff, 0xffffffff, TRUE),
  HOWTO(BFD_RELOC_MACH_O_X86_64_BRANCH32, 0, 2, 32, TRUE, 0,
	complain_overflow_bitfield,
	NULL, "BRANCH32",
	FALSE, 0xffffffff, 0xffffffff, TRUE),
  HOWTO(BFD_RELOC_MACH_O_X86_64_GOT_LOAD, 0, 2, 32, TRUE, 0,
	complain_overflow_bitfield,
	NULL, "GOT_LOAD",
	FALSE, 0xffffffff, 0xffffffff, TRUE),
  /* 8 */
  HOWTO(BFD_RELOC_MACH_O_SUBTRACTOR32, 0, 2, 32, FALSE, 0,
	complain_overflow_bitfield,
	NULL, "SUBTRACTOR32",
	FALSE, 0xffffffff, 0xffffffff, FALSE),
  HOWTO(BFD_RELOC_MACH_O_SUBTRACTOR64, 0, 4, 64, FALSE, 0,
	complain_overflow_bitfield,
	NULL, "SUBTRACTOR64",
	FALSE, MINUS_ONE, MINUS_ONE, FALSE),
  HOWTO(BFD_RELOC_MACH_O_X86_64_GOT, 0, 2, 32, TRUE, 0,
	complain_overflow_bitfield,
	NULL, "GOT",
	FALSE, 0xffffffff, 0xffffffff, TRUE),
  HOWTO(BFD_RELOC_MACH_O_X86_64_BRANCH8, 0, 0, 8, TRUE, 0,
	complain_overflow_bitfield,
	NULL, "BRANCH8",
	FALSE, 0xff, 0xff, TRUE),
  /* 12 */
  HOWTO(BFD_RELOC_MACH_O_X86_64_TLV, 0, 2, 32, TRUE, 0,
	complain_overflow_bitfield,
	NULL, "TLV",
	FALSE, 0xffffffff, 0xffffffff, TRUE),
};

static bfd_boolean
bfd_mach_o_x86_64_canonicalize_one_reloc (bfd *       abfd,
					  struct mach_o_reloc_info_external * raw,
					  arelent *   res,
					  asymbol **  syms,
					  arelent *   res_base ATTRIBUTE_UNUSED)
{
  bfd_mach_o_reloc_info reloc;

  if (!bfd_mach_o_pre_canonicalize_one_reloc (abfd, raw, &reloc, res, syms))
    return FALSE;

  /* On x86-64, scattered relocs are not used.  */
  if (reloc.r_scattered)
    return FALSE;

  switch (reloc.r_type)
    {
    case BFD_MACH_O_X86_64_RELOC_UNSIGNED:
      if (reloc.r_pcrel)
	return FALSE;
      switch (reloc.r_length)
	{
	case 2:
	  res->howto = &x86_64_howto_table[1];
	  return TRUE;
	case 3:
	  res->howto = &x86_64_howto_table[0];
	  return TRUE;
	default:
	  return FALSE;
	}
    case BFD_MACH_O_X86_64_RELOC_SIGNED:
      if (reloc.r_length == 2 && reloc.r_pcrel)
	{
	  res->howto = &x86_64_howto_table[2];
	  return TRUE;
	}
      break;
    case BFD_MACH_O_X86_64_RELOC_BRANCH:
      if (!reloc.r_pcrel)
	return FALSE;
      switch (reloc.r_length)
	{
	case 2:
	  res->howto = &x86_64_howto_table[6];
	  return TRUE;
	default:
	  return FALSE;
	}
      break;
    case BFD_MACH_O_X86_64_RELOC_GOT_LOAD:
      if (reloc.r_length == 2 && reloc.r_pcrel && reloc.r_extern)
	{
	  res->howto = &x86_64_howto_table[7];
	  return TRUE;
	}
      break;
    case BFD_MACH_O_X86_64_RELOC_GOT:
      if (reloc.r_length == 2 && reloc.r_pcrel && reloc.r_extern)
	{
	  res->howto = &x86_64_howto_table[10];
	  return TRUE;
	}
      break;
    case BFD_MACH_O_X86_64_RELOC_SUBTRACTOR:
      if (reloc.r_pcrel)
	return FALSE;
      switch (reloc.r_length)
	{
	case 2:
	  res->howto = &x86_64_howto_table[8];
	  return TRUE;
	case 3:
	  res->howto = &x86_64_howto_table[9];
	  return TRUE;
	default:
	  return FALSE;
	}
      break;
    case BFD_MACH_O_X86_64_RELOC_SIGNED_1:
      if (reloc.r_length == 2 && reloc.r_pcrel)
	{
	  res->howto = &x86_64_howto_table[3];
	  return TRUE;
	}
      break;
    case BFD_MACH_O_X86_64_RELOC_SIGNED_2:
      if (reloc.r_length == 2 && reloc.r_pcrel)
	{
	  res->howto = &x86_64_howto_table[4];
	  return TRUE;
	}
      break;
    case BFD_MACH_O_X86_64_RELOC_SIGNED_4:
      if (reloc.r_length == 2 && reloc.r_pcrel)
	{
	  res->howto = &x86_64_howto_table[5];
	  return TRUE;
	}
      break;
    case BFD_MACH_O_X86_64_RELOC_TLV:
      if (reloc.r_length == 2 && reloc.r_pcrel && reloc.r_extern)
	{
	  res->howto = &x86_64_howto_table[12];
	  return TRUE;
	}
      break;
    default:
      return FALSE;
    }
  return FALSE;
}

static bfd_boolean
bfd_mach_o_x86_64_swap_reloc_out (arelent *rel, bfd_mach_o_reloc_info *rinfo)
{
  rinfo->r_address = rel->address;
  rinfo->r_scattered = 0;
  switch (rel->howto->type)
    {
    case BFD_RELOC_32:
      rinfo->r_type = BFD_MACH_O_X86_64_RELOC_UNSIGNED;
      rinfo->r_pcrel = 0;
      rinfo->r_length = 2;
      break;
    case BFD_RELOC_64:
      rinfo->r_type = BFD_MACH_O_X86_64_RELOC_UNSIGNED;
      rinfo->r_pcrel = 0;
      rinfo->r_length = 3;
      break;
    case BFD_RELOC_32_PCREL:
      rinfo->r_type = BFD_MACH_O_X86_64_RELOC_SIGNED;
      rinfo->r_pcrel = 1;
      rinfo->r_length = 2;
      break;
    case BFD_RELOC_MACH_O_X86_64_PCREL32_1:
      rinfo->r_type = BFD_MACH_O_X86_64_RELOC_SIGNED_1;
      rinfo->r_pcrel = 1;
      rinfo->r_length = 2;
      break;
    case BFD_RELOC_MACH_O_X86_64_PCREL32_2:
      rinfo->r_type = BFD_MACH_O_X86_64_RELOC_SIGNED_2;
      rinfo->r_pcrel = 1;
      rinfo->r_length = 2;
      break;
    case BFD_RELOC_MACH_O_X86_64_PCREL32_4:
      rinfo->r_type = BFD_MACH_O_X86_64_RELOC_SIGNED_4;
      rinfo->r_pcrel = 1;
      rinfo->r_length = 2;
      break;
    case BFD_RELOC_MACH_O_X86_64_BRANCH32:
      rinfo->r_type = BFD_MACH_O_X86_64_RELOC_BRANCH;
      rinfo->r_pcrel = 1;
      rinfo->r_length = 2;
      break;
    case BFD_RELOC_MACH_O_SUBTRACTOR32:
      rinfo->r_type = BFD_MACH_O_X86_64_RELOC_SUBTRACTOR;
      rinfo->r_pcrel = 0;
      rinfo->r_length = 2;
      break;
    case BFD_RELOC_MACH_O_SUBTRACTOR64:
      rinfo->r_type = BFD_MACH_O_X86_64_RELOC_SUBTRACTOR;
      rinfo->r_pcrel = 0;
      rinfo->r_length = 3;
      break;
    case BFD_RELOC_MACH_O_X86_64_GOT:
      rinfo->r_type = BFD_MACH_O_X86_64_RELOC_GOT;
      rinfo->r_pcrel = 1;
      rinfo->r_length = 2;
      break;
    case BFD_RELOC_MACH_O_X86_64_GOT_LOAD:
      rinfo->r_type = BFD_MACH_O_X86_64_RELOC_GOT_LOAD;
      rinfo->r_pcrel = 1;
      rinfo->r_length = 2;
      break;
    case BFD_RELOC_MACH_O_X86_64_TLV:
      rinfo->r_type = BFD_MACH_O_X86_64_RELOC_TLV;
      rinfo->r_pcrel = 1;
      rinfo->r_length = 2;
      break;
    default:
      return FALSE;
    }
  if ((*rel->sym_ptr_ptr)->flags & BSF_SECTION_SYM)
    {
      rinfo->r_extern = 0;
      rinfo->r_value =
	(*rel->sym_ptr_ptr)->section->output_section->target_index;
    }
  else
    {
      rinfo->r_extern = 1;
      rinfo->r_value = (*rel->sym_ptr_ptr)->udata.i;
    }
  return TRUE;
}

static bfd_boolean
bfd_mach_o_section_type_valid_for_x86_64 (unsigned long val)
{
  if (val == BFD_MACH_O_S_NON_LAZY_SYMBOL_POINTERS
      || val == BFD_MACH_O_S_LAZY_SYMBOL_POINTERS
      || val == BFD_MACH_O_S_SYMBOL_STUBS)
    return FALSE;
  return TRUE;
}

/* We want to bump the alignment of some sections.  */
static const mach_o_section_name_xlat text_section_names_xlat[] =
  {
    {	".eh_frame",				"__eh_frame",
	SEC_READONLY | SEC_DATA | SEC_LOAD,	BFD_MACH_O_S_COALESCED,
	BFD_MACH_O_S_ATTR_LIVE_SUPPORT
	| BFD_MACH_O_S_ATTR_STRIP_STATIC_SYMS
	| BFD_MACH_O_S_ATTR_NO_TOC,		3},
    { NULL, NULL, 0, 0, 0, 0}
  };

const mach_o_segment_name_xlat mach_o_x86_64_segsec_names_xlat[] =
  {
    { "__TEXT", text_section_names_xlat },
    { NULL, NULL }
  };

#define bfd_mach_o_canonicalize_one_reloc bfd_mach_o_x86_64_canonicalize_one_reloc
#define bfd_mach_o_swap_reloc_out bfd_mach_o_x86_64_swap_reloc_out

#define bfd_mach_o_bfd_reloc_type_lookup bfd_mach_o_x86_64_bfd_reloc_type_lookup
#define bfd_mach_o_bfd_reloc_name_lookup bfd_mach_o_x86_64_bfd_reloc_name_lookup
#define bfd_mach_o_print_thread NULL
#define bfd_mach_o_tgt_seg_table mach_o_x86_64_segsec_names_xlat
#define bfd_mach_o_section_type_valid_for_tgt bfd_mach_o_section_type_valid_for_x86_64

#define TARGET_NAME		x86_64_mach_o_vec
#define TARGET_STRING		"mach-o-x86-64"
#define TARGET_ARCHITECTURE	bfd_arch_i386
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
