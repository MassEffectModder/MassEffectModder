/* BFD back-end for Intel 386 PE IMAGE COFF files.
   Copyright (C) 2006-2019 Free Software Foundation, Inc.

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
   MA 02110-1301, USA.

   Written by Kai Tietz, OneVision Software GmbH&CoKg.  */

#include "sysdep.h"
#include "bfd.h"

#define TARGET_SYM		x86_64_pei_vec
#define TARGET_NAME		"pei-x86-64"
#define COFF_IMAGE_WITH_PE
#define COFF_WITH_PE
#define COFF_WITH_pex64
#define PCRELOFFSET		TRUE
#if defined (USE_MINGW64_LEADING_UNDERSCORES)
#define TARGET_UNDERSCORE	'_'
#else
#define TARGET_UNDERSCORE	0
#endif
/* Long section names not allowed in executable images, only object files.  */
#define COFF_LONG_SECTION_NAMES 0
#define COFF_SUPPORT_GNU_LINKONCE
#define COFF_LONG_FILENAMES
#define PDATA_ROW_SIZE	(3 * 4)

#define COFF_SECTION_ALIGNMENT_ENTRIES \
{ COFF_SECTION_NAME_EXACT_MATCH (".bss"), \
  COFF_ALIGNMENT_FIELD_EMPTY, COFF_ALIGNMENT_FIELD_EMPTY, 4 }, \
{ COFF_SECTION_NAME_PARTIAL_MATCH (".data"), \
  COFF_ALIGNMENT_FIELD_EMPTY, COFF_ALIGNMENT_FIELD_EMPTY, 4 }, \
{ COFF_SECTION_NAME_PARTIAL_MATCH (".rdata"), \
  COFF_ALIGNMENT_FIELD_EMPTY, COFF_ALIGNMENT_FIELD_EMPTY, 4 }, \
{ COFF_SECTION_NAME_PARTIAL_MATCH (".text"), \
  COFF_ALIGNMENT_FIELD_EMPTY, COFF_ALIGNMENT_FIELD_EMPTY, 4 }, \
{ COFF_SECTION_NAME_PARTIAL_MATCH (".idata"), \
  COFF_ALIGNMENT_FIELD_EMPTY, COFF_ALIGNMENT_FIELD_EMPTY, 2 }, \
{ COFF_SECTION_NAME_EXACT_MATCH (".pdata"), \
  COFF_ALIGNMENT_FIELD_EMPTY, COFF_ALIGNMENT_FIELD_EMPTY, 2 }, \
{ COFF_SECTION_NAME_PARTIAL_MATCH (".debug"), \
  COFF_ALIGNMENT_FIELD_EMPTY, COFF_ALIGNMENT_FIELD_EMPTY, 0 }, \
{ COFF_SECTION_NAME_PARTIAL_MATCH (".gnu.linkonce.wi."), \
  COFF_ALIGNMENT_FIELD_EMPTY, COFF_ALIGNMENT_FIELD_EMPTY, 0 }

/* Note we have to make sure not to include headers twice.
   Not all headers are wrapped in #ifdef guards, so we define
   PEI_HEADERS to prevent double including in coff-x86_64.c  */
#define PEI_HEADERS
#include "sysdep.h"
#include "bfd.h"
#include "libbfd.h"
#include "coff/x86_64.h"
#include "coff/internal.h"
#include "coff/pe.h"
#include "libcoff.h"
#include "libpei.h"
#include "libiberty.h"

#undef AOUTSZ
#define AOUTSZ		PEPAOUTSZ
#define PEAOUTHDR	PEPAOUTHDR

#define BADMAG(x) AMD64BADMAG(x)

#ifdef COFF_WITH_pex64
# undef  AOUTSZ
# define AOUTSZ		PEPAOUTSZ
# define PEAOUTHDR	PEPAOUTHDR
#endif

#define COFF_DEFAULT_SECTION_ALIGNMENT_POWER (2)

/* The page size is a guess based on ELF.  */

#define COFF_PAGE_SIZE 0x1000

/* Return TRUE if this relocation should appear in the output .reloc
   section.  */

static bfd_boolean
in_reloc_p (bfd *abfd ATTRIBUTE_UNUSED, reloc_howto_type *howto)
{
  return ! howto->pc_relative && howto->type != R_AMD64_IMAGEBASE
     && howto->type != R_AMD64_SECREL;
}

#define I386  1			/* Customize coffcode.h */
#define AMD64 1

/* For 386 COFF a STYP_NOLOAD | STYP_BSS section is part of a shared
   library.  On some other COFF targets STYP_BSS is normally
   STYP_NOLOAD.  */
#define BSS_NOLOAD_IS_SHARED_LIBRARY

/* Compute the addend of a reloc.  If the reloc is to a common symbol,
   the object file contains the value of the common symbol.  By the
   time this is called, the linker may be using a different symbol
   from a different object file with a different value.  Therefore, we
   hack wildly to locate the original symbol from this file so that we
   can make the correct adjustment.  This macro sets coffsym to the
   symbol from the original file, and uses it to set the addend value
   correctly.  If this is not a common symbol, the usual addend
   calculation is done, except that an additional tweak is needed for
   PC relative relocs.
   FIXME: This macro refers to symbols and asect; these are from the
   calling function, not the macro arguments.  */

/* We use the special COFF backend linker.  For normal AMD64 COFF, we
   can use the generic relocate_section routine.  For PE, we need our
   own routine.  */

#ifdef TARGET_UNDERSCORE

/* If amd64 gcc uses underscores for symbol names, then it does not use
   a leading dot for local labels, so if TARGET_UNDERSCORE is defined
   we treat all symbols starting with L as local.  */

static bfd_boolean
coff_amd64_is_local_label_name (bfd *abfd, const char *name)
{
  if (name[0] == 'L')
    return TRUE;

  return _bfd_coff_is_local_label_name (abfd, name);
}

#define coff_bfd_is_local_label_name coff_amd64_is_local_label_name

#endif /* TARGET_UNDERSCORE */

#ifndef bfd_pe_print_pdata
#define bfd_pe_print_pdata   NULL
#endif

bfd_boolean
_bfd_pex64i_final_link_postscript (bfd * abfd ATTRIBUTE_UNUSED,
                                struct coff_final_link_info *pfinfo ATTRIBUTE_UNUSED)
{
  return TRUE;
}

#include "libpei.h"

static bfd_boolean (*pe_saved_coff_bfd_copy_private_bfd_data) (bfd *, bfd *) =
#ifndef coff_bfd_copy_private_bfd_data
     NULL;
#else
     coff_bfd_copy_private_bfd_data;
#undef coff_bfd_copy_private_bfd_data
#endif

static bfd_boolean		       pe_bfd_copy_private_bfd_data (bfd *, bfd *);
#define coff_bfd_copy_private_bfd_data pe_bfd_copy_private_bfd_data

#define coff_mkobject	   pe_mkobject
#define coff_mkobject_hook pe_mkobject_hook

/* This structure contains static variables used by the ILF code.  */
typedef asection * asection_ptr;

typedef struct
{
  bfd *			abfd;
  bfd_byte *		data;
  struct bfd_in_memory * bim;
  unsigned short	magic;

  arelent *		reltab;
  unsigned int		relcount;

  coff_symbol_type *	sym_cache;
  coff_symbol_type *	sym_ptr;
  unsigned int		sym_index;

  unsigned int *	sym_table;
  unsigned int *	table_ptr;

  combined_entry_type * native_syms;
  combined_entry_type * native_ptr;

  coff_symbol_type **	sym_ptr_table;
  coff_symbol_type **	sym_ptr_ptr;

  unsigned int		sec_index;

  char *		string_table;
  char *		string_ptr;
  char *		end_string_ptr;

  SYMENT *		esym_table;
  SYMENT *		esym_ptr;

  struct internal_reloc * int_reltab;
}
pe_ILF_vars;

const bfd_target *coff_real_object_p
  (bfd *, unsigned, struct internal_filehdr *, struct internal_aouthdr *);

#ifndef NO_COFF_RELOCS
static void
coff_swap_reloc_in (bfd * abfd, void * src, void * dst)
{
  RELOC *reloc_src = (RELOC *) src;
  struct internal_reloc *reloc_dst = (struct internal_reloc *) dst;

  reloc_dst->r_vaddr  = H_GET_32 (abfd, reloc_src->r_vaddr);
  reloc_dst->r_symndx = H_GET_S32 (abfd, reloc_src->r_symndx);
  reloc_dst->r_type   = H_GET_16 (abfd, reloc_src->r_type);
#ifdef SWAP_IN_RELOC_OFFSET
  reloc_dst->r_offset = SWAP_IN_RELOC_OFFSET (abfd, reloc_src->r_offset);
#endif
}

static unsigned int
coff_swap_reloc_out (bfd * abfd, void * src, void * dst)
{
  struct internal_reloc *reloc_src = (struct internal_reloc *) src;
  struct external_reloc *reloc_dst = (struct external_reloc *) dst;

  H_PUT_32 (abfd, reloc_src->r_vaddr, reloc_dst->r_vaddr);
  H_PUT_32 (abfd, reloc_src->r_symndx, reloc_dst->r_symndx);
  H_PUT_16 (abfd, reloc_src->r_type, reloc_dst->r_type);

#ifdef SWAP_OUT_RELOC_OFFSET
  SWAP_OUT_RELOC_OFFSET (abfd, reloc_src->r_offset, reloc_dst->r_offset);
#endif
#ifdef SWAP_OUT_RELOC_EXTRA
  SWAP_OUT_RELOC_EXTRA (abfd, reloc_src, reloc_dst);
#endif
  return RELSZ;
}
#endif /* not NO_COFF_RELOCS */

#ifdef COFF_IMAGE_WITH_PE
#undef FILHDR
#define FILHDR struct external_PEI_IMAGE_hdr
#endif

static void
coff_swap_filehdr_in (bfd * abfd, void * src, void * dst)
{
  FILHDR *filehdr_src = (FILHDR *) src;
  struct internal_filehdr *filehdr_dst = (struct internal_filehdr *) dst;

  filehdr_dst->f_magic  = H_GET_16 (abfd, filehdr_src->f_magic);
  filehdr_dst->f_nscns  = H_GET_16 (abfd, filehdr_src->f_nscns);
  filehdr_dst->f_timdat = H_GET_32 (abfd, filehdr_src->f_timdat);
  filehdr_dst->f_nsyms  = H_GET_32 (abfd, filehdr_src->f_nsyms);
  filehdr_dst->f_flags  = H_GET_16 (abfd, filehdr_src->f_flags);
  filehdr_dst->f_symptr = H_GET_32 (abfd, filehdr_src->f_symptr);

  /* Other people's tools sometimes generate headers with an nsyms but
     a zero symptr.  */
  if (filehdr_dst->f_nsyms != 0 && filehdr_dst->f_symptr == 0)
    {
      filehdr_dst->f_nsyms = 0;
      filehdr_dst->f_flags |= F_LSYMS;
    }

  filehdr_dst->f_opthdr = H_GET_16 (abfd, filehdr_src-> f_opthdr);
}

# define coff_swap_filehdr_out _bfd_XXi_only_swap_filehdr_out

static void
coff_swap_scnhdr_in (bfd * abfd, void * ext, void * in)
{
  SCNHDR *scnhdr_ext = (SCNHDR *) ext;
  struct internal_scnhdr *scnhdr_int = (struct internal_scnhdr *) in;

  memcpy (scnhdr_int->s_name, scnhdr_ext->s_name, sizeof (scnhdr_int->s_name));

  scnhdr_int->s_vaddr   = GET_SCNHDR_VADDR (abfd, scnhdr_ext->s_vaddr);
  scnhdr_int->s_paddr   = GET_SCNHDR_PADDR (abfd, scnhdr_ext->s_paddr);
  scnhdr_int->s_size    = GET_SCNHDR_SIZE (abfd, scnhdr_ext->s_size);
  scnhdr_int->s_scnptr  = GET_SCNHDR_SCNPTR (abfd, scnhdr_ext->s_scnptr);
  scnhdr_int->s_relptr  = GET_SCNHDR_RELPTR (abfd, scnhdr_ext->s_relptr);
  scnhdr_int->s_lnnoptr = GET_SCNHDR_LNNOPTR (abfd, scnhdr_ext->s_lnnoptr);
  scnhdr_int->s_flags   = H_GET_32 (abfd, scnhdr_ext->s_flags);

  /* MS handles overflow of line numbers by carrying into the reloc
     field (it appears).  Since it's supposed to be zero for PE
     *IMAGE* format, that's safe.  This is still a bit iffy.  */
  scnhdr_int->s_nlnno = (H_GET_16 (abfd, scnhdr_ext->s_nlnno)
             + (H_GET_16 (abfd, scnhdr_ext->s_nreloc) << 16));
  scnhdr_int->s_nreloc = 0;

  if (scnhdr_int->s_vaddr != 0)
    {
      scnhdr_int->s_vaddr += pe_data (abfd)->pe_opthdr.ImageBase;
      /* Do not cut upper 32-bits for 64-bit vma.  */
#ifndef COFF_WITH_pex64
      scnhdr_int->s_vaddr &= 0xffffffff;
#endif
    }

#ifndef COFF_NO_HACK_SCNHDR_SIZE
  /* If this section holds uninitialized data and is from an object file
     or from an executable image that has not initialized the field,
     or if the image is an executable file and the physical size is padded,
     use the virtual size (stored in s_paddr) instead.  */
  if (scnhdr_int->s_paddr > 0
      && (((scnhdr_int->s_flags & IMAGE_SCN_CNT_UNINITIALIZED_DATA) != 0
       && (! bfd_pei_p (abfd) || scnhdr_int->s_size == 0))
      || (bfd_pei_p (abfd) && (scnhdr_int->s_size > scnhdr_int->s_paddr))))
  /* This code used to set scnhdr_int->s_paddr to 0.  However,
     coff_set_alignment_hook stores s_paddr in virt_size, which
     only works if it correctly holds the virtual size of the
     section.  */
    scnhdr_int->s_size = scnhdr_int->s_paddr;
#endif
}

static bfd_boolean
pe_mkobject (bfd * abfd)
{
  pe_data_type *pe;
  bfd_size_type amt = sizeof (pe_data_type);

  abfd->tdata.pe_obj_data = (struct pe_tdata *) bfd_zalloc (abfd, amt);

  if (abfd->tdata.pe_obj_data == 0)
    return FALSE;

  pe = pe_data (abfd);

  pe->coff.pe = 1;

  /* in_reloc_p is architecture dependent.  */
  pe->in_reloc_p = in_reloc_p;

  memset (& pe->pe_opthdr, 0, sizeof pe->pe_opthdr);
  return TRUE;
}

/* Create the COFF backend specific information.  */

static void *
pe_mkobject_hook (bfd * abfd,
          void * filehdr,
          void * aouthdr ATTRIBUTE_UNUSED)
{
  struct internal_filehdr *internal_f = (struct internal_filehdr *) filehdr;
  pe_data_type *pe;

  if (! pe_mkobject (abfd))
    return NULL;

  pe = pe_data (abfd);
  pe->coff.sym_filepos = internal_f->f_symptr;
  /* These members communicate important constants about the symbol
     table to GDB's symbol-reading code.  These `constants'
     unfortunately vary among coff implementations...  */
  pe->coff.local_n_btmask = N_BTMASK;
  pe->coff.local_n_btshft = N_BTSHFT;
  pe->coff.local_n_tmask = N_TMASK;
  pe->coff.local_n_tshift = N_TSHIFT;
  pe->coff.local_symesz = SYMESZ;
  pe->coff.local_auxesz = AUXESZ;
  pe->coff.local_linesz = LINESZ;

  pe->coff.timestamp = internal_f->f_timdat;

  obj_raw_syment_count (abfd) =
    obj_conv_table_size (abfd) =
      internal_f->f_nsyms;

  pe->real_flags = internal_f->f_flags;

  if ((internal_f->f_flags & F_DLL) != 0)
    pe->dll = 1;

  if ((internal_f->f_flags & IMAGE_FILE_DEBUG_STRIPPED) == 0)
    abfd->flags |= HAS_DEBUG;

  if (aouthdr)
    pe->pe_opthdr = ((struct internal_aouthdr *) aouthdr)->pe;

  return (void *) pe;
}

/* Copy any private info we understand from the input bfd
   to the output bfd.  */

static bfd_boolean
pe_bfd_copy_private_bfd_data (bfd *ibfd, bfd *obfd)
{
  /* PR binutils/716: Copy the large address aware flag.
     XXX: Should we be copying other flags or other fields in the pe_data()
     structure ?  */
  if (pe_data (obfd) != NULL
      && pe_data (ibfd) != NULL
      && pe_data (ibfd)->real_flags & IMAGE_FILE_LARGE_ADDRESS_AWARE)
    pe_data (obfd)->real_flags |= IMAGE_FILE_LARGE_ADDRESS_AWARE;

  if (!_bfd_XX_bfd_copy_private_bfd_data_common (ibfd, obfd))
    return FALSE;

  if (pe_saved_coff_bfd_copy_private_bfd_data)
    return pe_saved_coff_bfd_copy_private_bfd_data (ibfd, obfd);

  return TRUE;
}

#define coff_bfd_copy_private_section_data \
  _bfd_XX_bfd_copy_private_section_data

#define coff_get_symbol_info _bfd_nosymbols_get_symbol_info

/* Code to handle Microsoft's Image Library Format.
   Also known as LINK6 format.
   Documentation about this format can be found at:

   http://msdn.microsoft.com/library/specs/pecoff_section8.htm  */

/* The following constants specify the sizes of the various data
   structures that we have to create in order to build a bfd describing
   an ILF object file.  The final "+ 1" in the definitions of SIZEOF_IDATA6
   and SIZEOF_IDATA7 below is to allow for the possibility that we might
   need a padding byte in order to ensure 16 bit alignment for the section's
   contents.

   The value for SIZEOF_ILF_STRINGS is computed as follows:

      There will be NUM_ILF_SECTIONS section symbols.  Allow 9 characters
      per symbol for their names (longest section name is .idata$x).

      There will be two symbols for the imported value, one the symbol name
      and one with _imp__ prefixed.  Allowing for the terminating nul's this
      is strlen (symbol_name) * 2 + 8 + 21 + strlen (source_dll).

      The strings in the string table must start STRING__SIZE_SIZE bytes into
      the table in order to for the string lookup code in coffgen/coffcode to
      work.  */
#define NUM_ILF_RELOCS		8
#define NUM_ILF_SECTIONS	6
#define NUM_ILF_SYMS		(2 + NUM_ILF_SECTIONS)

#define SIZEOF_ILF_SYMS		 (NUM_ILF_SYMS * sizeof (* vars.sym_cache))
#define SIZEOF_ILF_SYM_TABLE	 (NUM_ILF_SYMS * sizeof (* vars.sym_table))
#define SIZEOF_ILF_NATIVE_SYMS	 (NUM_ILF_SYMS * sizeof (* vars.native_syms))
#define SIZEOF_ILF_SYM_PTR_TABLE (NUM_ILF_SYMS * sizeof (* vars.sym_ptr_table))
#define SIZEOF_ILF_EXT_SYMS	 (NUM_ILF_SYMS * sizeof (* vars.esym_table))
#define SIZEOF_ILF_RELOCS	 (NUM_ILF_RELOCS * sizeof (* vars.reltab))
#define SIZEOF_ILF_INT_RELOCS	 (NUM_ILF_RELOCS * sizeof (* vars.int_reltab))
#define SIZEOF_ILF_STRINGS	 (strlen (symbol_name) * 2 + 8 \
                    + 21 + strlen (source_dll) \
                    + NUM_ILF_SECTIONS * 9 \
                    + STRING_SIZE_SIZE)
#define SIZEOF_IDATA2		(5 * 4)

/* For PEx64 idata4 & 5 have thumb size of 8 bytes.  */
#ifdef COFF_WITH_pex64
#define SIZEOF_IDATA4		(2 * 4)
#define SIZEOF_IDATA5		(2 * 4)
#else
#define SIZEOF_IDATA4		(1 * 4)
#define SIZEOF_IDATA5		(1 * 4)
#endif

#define SIZEOF_IDATA6		(2 + strlen (symbol_name) + 1 + 1)
#define SIZEOF_IDATA7		(strlen (source_dll) + 1 + 1)
#define SIZEOF_ILF_SECTIONS	(NUM_ILF_SECTIONS * sizeof (struct coff_section_tdata))

#define ILF_DATA_SIZE				\
    + SIZEOF_ILF_SYMS				\
    + SIZEOF_ILF_SYM_TABLE			\
    + SIZEOF_ILF_NATIVE_SYMS			\
    + SIZEOF_ILF_SYM_PTR_TABLE			\
    + SIZEOF_ILF_EXT_SYMS			\
    + SIZEOF_ILF_RELOCS				\
    + SIZEOF_ILF_INT_RELOCS			\
    + SIZEOF_ILF_STRINGS			\
    + SIZEOF_IDATA2				\
    + SIZEOF_IDATA4				\
    + SIZEOF_IDATA5				\
    + SIZEOF_IDATA6				\
    + SIZEOF_IDATA7				\
    + SIZEOF_ILF_SECTIONS			\
    + MAX_TEXT_SECTION_SIZE

/* Create an empty relocation against the given symbol.  */

static void
pe_ILF_make_a_symbol_reloc (pe_ILF_vars *		vars,
                bfd_vma			address,
                bfd_reloc_code_real_type	reloc,
                struct bfd_symbol **	sym,
                unsigned int		sym_index)
{
  arelent * entry;
  struct internal_reloc * internal;

  entry = vars->reltab + vars->relcount;
  internal = vars->int_reltab + vars->relcount;

  entry->address     = address;
  entry->addend      = 0;
  entry->howto       = bfd_reloc_type_lookup (vars->abfd, reloc);
  entry->sym_ptr_ptr = sym;

  internal->r_vaddr  = address;
  internal->r_symndx = sym_index;
  internal->r_type   = entry->howto->type;

  vars->relcount ++;

  BFD_ASSERT (vars->relcount <= NUM_ILF_RELOCS);
}

/* Create an empty relocation against the given section.  */

static void
pe_ILF_make_a_reloc (pe_ILF_vars *	       vars,
             bfd_vma		       address,
             bfd_reloc_code_real_type  reloc,
             asection_ptr	       sec)
{
  pe_ILF_make_a_symbol_reloc (vars, address, reloc, sec->symbol_ptr_ptr,
                  coff_section_data (vars->abfd, sec)->i);
}

/* Move the queued relocs into the given section.  */

static void
pe_ILF_save_relocs (pe_ILF_vars * vars,
            asection_ptr  sec)
{
  /* Make sure that there is somewhere to store the internal relocs.  */
  if (coff_section_data (vars->abfd, sec) == NULL)
    /* We should probably return an error indication here.  */
    abort ();

  coff_section_data (vars->abfd, sec)->relocs = vars->int_reltab;
  coff_section_data (vars->abfd, sec)->keep_relocs = TRUE;

  sec->relocation  = vars->reltab;
  sec->reloc_count = vars->relcount;
  sec->flags      |= SEC_RELOC;

  vars->reltab     += vars->relcount;
  vars->int_reltab += vars->relcount;
  vars->relcount   = 0;

  BFD_ASSERT ((bfd_byte *) vars->int_reltab < (bfd_byte *) vars->string_table);
}

/* Create a global symbol and add it to the relevant tables.  */

static void
pe_ILF_make_a_symbol (pe_ILF_vars *  vars,
              const char *   prefix,
              const char *   symbol_name,
              asection_ptr   section,
              flagword       extra_flags)
{
  coff_symbol_type * sym;
  combined_entry_type * ent;
  SYMENT * esym;
  unsigned short sclass;

  if (extra_flags & BSF_LOCAL)
    sclass = C_STAT;
  else
    sclass = C_EXT;

#ifdef THUMBPEMAGIC
  if (vars->magic == THUMBPEMAGIC)
    {
      if (extra_flags & BSF_FUNCTION)
    sclass = C_THUMBEXTFUNC;
      else if (extra_flags & BSF_LOCAL)
    sclass = C_THUMBSTAT;
      else
    sclass = C_THUMBEXT;
    }
#endif

  BFD_ASSERT (vars->sym_index < NUM_ILF_SYMS);

  sym = vars->sym_ptr;
  ent = vars->native_ptr;
  esym = vars->esym_ptr;

  /* Copy the symbol's name into the string table.  */
  sprintf (vars->string_ptr, "%s%s", prefix, symbol_name);

  if (section == NULL)
    section = bfd_und_section_ptr;

  /* Initialise the external symbol.  */
  H_PUT_32 (vars->abfd, vars->string_ptr - vars->string_table,
        esym->e.e.e_offset);
  H_PUT_16 (vars->abfd, section->target_index, esym->e_scnum);
  esym->e_sclass[0] = sclass;

  /* The following initialisations are unnecessary - the memory is
     zero initialised.  They are just kept here as reminders.  */

  /* Initialise the internal symbol structure.  */
  ent->u.syment.n_sclass	  = sclass;
  ent->u.syment.n_scnum		  = section->target_index;
  ent->u.syment._n._n_n._n_offset = (bfd_hostptr_t) sym;
  ent->is_sym = TRUE;

  sym->symbol.the_bfd = vars->abfd;
  sym->symbol.name    = vars->string_ptr;
  sym->symbol.flags   = BSF_EXPORT | BSF_GLOBAL | extra_flags;
  sym->symbol.section = section;
  sym->native	      = ent;

  * vars->table_ptr = vars->sym_index;
  * vars->sym_ptr_ptr = sym;

  /* Adjust pointers for the next symbol.  */
  vars->sym_index ++;
  vars->sym_ptr ++;
  vars->sym_ptr_ptr ++;
  vars->table_ptr ++;
  vars->native_ptr ++;
  vars->esym_ptr ++;
  vars->string_ptr += strlen (symbol_name) + strlen (prefix) + 1;

  BFD_ASSERT (vars->string_ptr < vars->end_string_ptr);
}

/* Create a section.  */

static asection_ptr
pe_ILF_make_a_section (pe_ILF_vars * vars,
               const char *  name,
               unsigned int  size,
               flagword      extra_flags)
{
  asection_ptr sec;
  flagword     flags;

  sec = bfd_make_section_old_way (vars->abfd, name);
  if (sec == NULL)
    return NULL;

  flags = SEC_HAS_CONTENTS | SEC_ALLOC | SEC_LOAD | SEC_KEEP | SEC_IN_MEMORY;

  bfd_set_section_flags (vars->abfd, sec, flags | extra_flags);

  (void) bfd_set_section_alignment (vars->abfd, sec, 2);

  /* Check that we will not run out of space.  */
  BFD_ASSERT (vars->data + size < vars->bim->buffer + vars->bim->size);

  /* Set the section size and contents.  The actual
     contents are filled in by our parent.  */
  bfd_set_section_size (vars->abfd, sec, (bfd_size_type) size);
  sec->contents = vars->data;
  sec->target_index = vars->sec_index ++;

  /* Advance data pointer in the vars structure.  */
  vars->data += size;

  /* Skip the padding byte if it was not needed.
     The logic here is that if the string length is odd,
     then the entire string length, including the null byte,
     is even and so the extra, padding byte, is not needed.  */
  if (size & 1)
    vars->data --;

# if (GCC_VERSION >= 3000)
  /* PR 18758: See note in pe_ILF_buid_a_bfd.  We must make sure that we
     preserve host alignment requirements.  We test 'size' rather than
     vars.data as we cannot perform binary arithmetic on pointers.  We assume
     that vars.data was sufficiently aligned upon entry to this function.
     The BFD_ASSERTs in this functions will warn us if we run out of room,
     but we should already have enough padding built in to ILF_DATA_SIZE.  */
  {
    unsigned int alignment = __alignof__ (struct coff_section_tdata);

    if (size & (alignment - 1))
      vars->data += alignment - (size & (alignment - 1));
  }
#endif
  /* Create a coff_section_tdata structure for our use.  */
  sec->used_by_bfd = (struct coff_section_tdata *) vars->data;
  vars->data += sizeof (struct coff_section_tdata);

  BFD_ASSERT (vars->data <= vars->bim->buffer + vars->bim->size);

  /* Create a symbol to refer to this section.  */
  pe_ILF_make_a_symbol (vars, "", name, sec, BSF_LOCAL);

  /* Cache the index to the symbol in the coff_section_data structure.  */
  coff_section_data (vars->abfd, sec)->i = vars->sym_index - 1;

  return sec;
}

/* This structure contains the code that goes into the .text section
   in order to perform a jump into the DLL lookup table.  The entries
   in the table are index by the magic number used to represent the
   machine type in the PE file.  The contents of the data[] arrays in
   these entries are stolen from the jtab[] arrays in ld/pe-dll.c.
   The SIZE field says how many bytes in the DATA array are actually
   used.  The OFFSET field says where in the data array the address
   of the .idata$5 section should be placed.  */
#define MAX_TEXT_SECTION_SIZE 32

typedef struct
{
  unsigned short magic;
  unsigned char  data[MAX_TEXT_SECTION_SIZE];
  unsigned int   size;
  unsigned int   offset;
}
jump_table;

static jump_table jtab[] =
{
#ifdef I386MAGIC
  { I386MAGIC,
    { 0xff, 0x25, 0x00, 0x00, 0x00, 0x00, 0x90, 0x90 },
    8, 2
  },
#endif

#ifdef AMD64MAGIC
  { AMD64MAGIC,
    { 0xff, 0x25, 0x00, 0x00, 0x00, 0x00, 0x90, 0x90 },
    8, 2
  },
#endif

#ifdef  MC68MAGIC
  { MC68MAGIC,
    { /* XXX fill me in */ },
    0, 0
  },
#endif

#ifdef  MIPS_ARCH_MAGIC_WINCE
  { MIPS_ARCH_MAGIC_WINCE,
    { 0x00, 0x00, 0x08, 0x3c, 0x00, 0x00, 0x08, 0x8d,
      0x08, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00 },
    16, 0
  },
#endif

#ifdef  SH_ARCH_MAGIC_WINCE
  { SH_ARCH_MAGIC_WINCE,
    { 0x01, 0xd0, 0x02, 0x60, 0x2b, 0x40,
      0x09, 0x00, 0x00, 0x00, 0x00, 0x00 },
    12, 8
  },
#endif

#ifdef  ARMPEMAGIC
  { ARMPEMAGIC,
    { 0x00, 0xc0, 0x9f, 0xe5, 0x00, 0xf0,
      0x9c, 0xe5, 0x00, 0x00, 0x00, 0x00},
    12, 8
  },
#endif

#ifdef  THUMBPEMAGIC
  { THUMBPEMAGIC,
    { 0x40, 0xb4, 0x02, 0x4e, 0x36, 0x68, 0xb4, 0x46,
      0x40, 0xbc, 0x60, 0x47, 0x00, 0x00, 0x00, 0x00 },
    16, 12
  },
#endif
  { 0, { 0 }, 0, 0 }
};

#ifndef NUM_ENTRIES
#define NUM_ENTRIES(a) (sizeof (a) / sizeof (a)[0])
#endif

/* Build a full BFD from the information supplied in a ILF object.  */

static bfd_boolean
pe_ILF_build_a_bfd (bfd *	    abfd,
            unsigned int    magic,
            char *	    symbol_name,
            char *	    source_dll,
            unsigned int    ordinal,
            unsigned int    types)
{
  bfd_byte *		   ptr;
  pe_ILF_vars		   vars;
  struct internal_filehdr  internal_f;
  unsigned int		   import_type;
  unsigned int		   import_name_type;
  asection_ptr		   id4, id5, id6 = NULL, text = NULL;
  coff_symbol_type **	   imp_sym;
  unsigned int		   imp_index;

  /* Decode and verify the types field of the ILF structure.  */
  import_type = types & 0x3;
  import_name_type = (types & 0x1c) >> 2;

  switch (import_type)
    {
    case IMPORT_CODE:
    case IMPORT_DATA:
      break;

    case IMPORT_CONST:
      /* XXX code yet to be written.  */
      /* xgettext:c-format */
      _bfd_error_handler (_("%pB: unhandled import type; %x"),
              abfd, import_type);
      return FALSE;

    default:
      /* xgettext:c-format */
      _bfd_error_handler (_("%pB: unrecognized import type; %x"),
              abfd, import_type);
      return FALSE;
    }

  switch (import_name_type)
    {
    case IMPORT_ORDINAL:
    case IMPORT_NAME:
    case IMPORT_NAME_NOPREFIX:
    case IMPORT_NAME_UNDECORATE:
      break;

    default:
      /* xgettext:c-format */
      _bfd_error_handler (_("%pB: unrecognized import name type; %x"),
              abfd, import_name_type);
      return FALSE;
    }

  /* Initialise local variables.

     Note these are kept in a structure rather than being
     declared as statics since bfd frowns on global variables.

     We are going to construct the contents of the BFD in memory,
     so allocate all the space that we will need right now.  */
  vars.bim
    = (struct bfd_in_memory *) bfd_malloc ((bfd_size_type) sizeof (*vars.bim));
  if (vars.bim == NULL)
    return FALSE;

  ptr = (bfd_byte *) bfd_zmalloc ((bfd_size_type) ILF_DATA_SIZE);
  vars.bim->buffer = ptr;
  vars.bim->size   = ILF_DATA_SIZE;
  if (ptr == NULL)
    goto error_return;

  /* Initialise the pointers to regions of the memory and the
     other contents of the pe_ILF_vars structure as well.  */
  vars.sym_cache = (coff_symbol_type *) ptr;
  vars.sym_ptr   = (coff_symbol_type *) ptr;
  vars.sym_index = 0;
  ptr += SIZEOF_ILF_SYMS;

  vars.sym_table = (unsigned int *) ptr;
  vars.table_ptr = (unsigned int *) ptr;
  ptr += SIZEOF_ILF_SYM_TABLE;

  vars.native_syms = (combined_entry_type *) ptr;
  vars.native_ptr  = (combined_entry_type *) ptr;
  ptr += SIZEOF_ILF_NATIVE_SYMS;

  vars.sym_ptr_table = (coff_symbol_type **) ptr;
  vars.sym_ptr_ptr   = (coff_symbol_type **) ptr;
  ptr += SIZEOF_ILF_SYM_PTR_TABLE;

  vars.esym_table = (SYMENT *) ptr;
  vars.esym_ptr   = (SYMENT *) ptr;
  ptr += SIZEOF_ILF_EXT_SYMS;

  vars.reltab   = (arelent *) ptr;
  vars.relcount = 0;
  ptr += SIZEOF_ILF_RELOCS;

  vars.int_reltab  = (struct internal_reloc *) ptr;
  ptr += SIZEOF_ILF_INT_RELOCS;

  vars.string_table = (char *) ptr;
  vars.string_ptr   = (char *) ptr + STRING_SIZE_SIZE;
  ptr += SIZEOF_ILF_STRINGS;
  vars.end_string_ptr = (char *) ptr;

  /* The remaining space in bim->buffer is used
     by the pe_ILF_make_a_section() function.  */
# if (GCC_VERSION >= 3000)
  /* PR 18758: Make sure that the data area is sufficiently aligned for
     pointers on the host.  __alignof__ is a gcc extension, hence the test
     above.  For other compilers we will have to assume that the alignment is
     unimportant, or else extra code can be added here and in
     pe_ILF_make_a_section.

     Note - we cannot test 'ptr' directly as it is illegal to perform binary
     arithmetic on pointers, but we know that the strings section is the only
     one that might end on an unaligned boundary.  */
  {
    unsigned int alignment = __alignof__ (char *);

    if (SIZEOF_ILF_STRINGS & (alignment - 1))
      ptr += alignment - (SIZEOF_ILF_STRINGS & (alignment - 1));
  }
#endif

  vars.data = ptr;
  vars.abfd = abfd;
  vars.sec_index = 0;
  vars.magic = magic;

  /* Create the initial .idata$<n> sections:
     [.idata$2:  Import Directory Table -- not needed]
     .idata$4:  Import Lookup Table
     .idata$5:  Import Address Table

     Note we do not create a .idata$3 section as this is
     created for us by the linker script.  */
  id4 = pe_ILF_make_a_section (& vars, ".idata$4", SIZEOF_IDATA4, 0);
  id5 = pe_ILF_make_a_section (& vars, ".idata$5", SIZEOF_IDATA5, 0);
  if (id4 == NULL || id5 == NULL)
    goto error_return;

  /* Fill in the contents of these sections.  */
  if (import_name_type == IMPORT_ORDINAL)
    {
      if (ordinal == 0)
    /* See PR 20907 for a reproducer.  */
    goto error_return;

#ifdef COFF_WITH_pex64
      ((unsigned int *) id4->contents)[0] = ordinal;
      ((unsigned int *) id4->contents)[1] = 0x80000000;
      ((unsigned int *) id5->contents)[0] = ordinal;
      ((unsigned int *) id5->contents)[1] = 0x80000000;
#else
      * (unsigned int *) id4->contents = ordinal | 0x80000000;
      * (unsigned int *) id5->contents = ordinal | 0x80000000;
#endif
    }
  else
    {
      char * symbol;
      unsigned int len;

      /* Create .idata$6 - the Hint Name Table.  */
      id6 = pe_ILF_make_a_section (& vars, ".idata$6", SIZEOF_IDATA6, 0);
      if (id6 == NULL)
    goto error_return;

      /* If necessary, trim the import symbol name.  */
      symbol = symbol_name;

      /* As used by MS compiler, '_', '@', and '?' are alternative
     forms of USER_LABEL_PREFIX, with '?' for c++ mangled names,
     '@' used for fastcall (in C),  '_' everywhere else.  Only one
     of these is used for a symbol.  We strip this leading char for
     IMPORT_NAME_NOPREFIX and IMPORT_NAME_UNDECORATE as per the
     PE COFF 6.0 spec (section 8.3, Import Name Type).  */

      if (import_name_type != IMPORT_NAME)
    {
      char c = symbol[0];

      /* Check that we don't remove for targets with empty
         USER_LABEL_PREFIX the leading underscore.  */
      if ((c == '_' && abfd->xvec->symbol_leading_char != 0)
          || c == '@' || c == '?')
        symbol++;
    }

      len = strlen (symbol);
      if (import_name_type == IMPORT_NAME_UNDECORATE)
    {
      /* Truncate at the first '@'.  */
      char *at = strchr (symbol, '@');

      if (at != NULL)
        len = at - symbol;
    }

      id6->contents[0] = ordinal & 0xff;
      id6->contents[1] = ordinal >> 8;

      memcpy ((char *) id6->contents + 2, symbol, len);
      id6->contents[len + 2] = '\0';
    }

  if (import_name_type != IMPORT_ORDINAL)
    {
      pe_ILF_make_a_reloc (&vars, (bfd_vma) 0, BFD_RELOC_RVA, id6);
      pe_ILF_save_relocs (&vars, id4);

      pe_ILF_make_a_reloc (&vars, (bfd_vma) 0, BFD_RELOC_RVA, id6);
      pe_ILF_save_relocs (&vars, id5);
    }

  /* Create an import symbol.  */
  pe_ILF_make_a_symbol (& vars, "__imp_", symbol_name, id5, 0);
  imp_sym   = vars.sym_ptr_ptr - 1;
  imp_index = vars.sym_index - 1;

  /* Create extra sections depending upon the type of import we are dealing with.  */
  switch (import_type)
    {
      int i;

    case IMPORT_CODE:
      /* CODE functions are special, in that they get a trampoline that
     jumps to the main import symbol.  Create a .text section to hold it.
     First we need to look up its contents in the jump table.  */
      for (i = NUM_ENTRIES (jtab); i--;)
    {
      if (jtab[i].size == 0)
        continue;
      if (jtab[i].magic == magic)
        break;
    }
      /* If we did not find a matching entry something is wrong.  */
      if (i < 0)
    abort ();

      /* Create the .text section.  */
      text = pe_ILF_make_a_section (& vars, ".text", jtab[i].size, SEC_CODE);
      if (text == NULL)
    goto error_return;

      /* Copy in the jump code.  */
      memcpy (text->contents, jtab[i].data, jtab[i].size);

      /* Create a reloc for the data in the text section.  */
#ifdef MIPS_ARCH_MAGIC_WINCE
      if (magic == MIPS_ARCH_MAGIC_WINCE)
    {
      pe_ILF_make_a_symbol_reloc (&vars, (bfd_vma) 0, BFD_RELOC_HI16_S,
                      (struct bfd_symbol **) imp_sym,
                      imp_index);
      pe_ILF_make_a_reloc (&vars, (bfd_vma) 0, BFD_RELOC_LO16, text);
      pe_ILF_make_a_symbol_reloc (&vars, (bfd_vma) 4, BFD_RELOC_LO16,
                      (struct bfd_symbol **) imp_sym,
                      imp_index);
    }
      else
#endif
#ifdef AMD64MAGIC
      if (magic == AMD64MAGIC)
    {
      pe_ILF_make_a_symbol_reloc (&vars, (bfd_vma) jtab[i].offset,
                      BFD_RELOC_32_PCREL, (asymbol **) imp_sym,
                      imp_index);
    }
      else
#endif
    pe_ILF_make_a_symbol_reloc (&vars, (bfd_vma) jtab[i].offset,
                    BFD_RELOC_32, (asymbol **) imp_sym,
                    imp_index);

      pe_ILF_save_relocs (& vars, text);
      break;

    case IMPORT_DATA:
      break;

    default:
      /* XXX code not yet written.  */
      abort ();
    }

  /* Initialise the bfd.  */
  memset (& internal_f, 0, sizeof (internal_f));

  internal_f.f_magic  = magic;
  internal_f.f_symptr = 0;
  internal_f.f_nsyms  = 0;
  internal_f.f_flags  = F_AR32WR | F_LNNO; /* XXX is this correct ?  */

  if (   ! bfd_set_start_address (abfd, (bfd_vma) 0)
      || ! bfd_coff_set_arch_mach_hook (abfd, & internal_f))
    goto error_return;

  if (bfd_coff_mkobject_hook (abfd, (void *) & internal_f, NULL) == NULL)
    goto error_return;

  coff_data (abfd)->pe = 1;
#ifdef THUMBPEMAGIC
  if (vars.magic == THUMBPEMAGIC)
    /* Stop some linker warnings about thumb code not supporting interworking.  */
    coff_data (abfd)->flags |= F_INTERWORK | F_INTERWORK_SET;
#endif

  /* Switch from file contents to memory contents.  */
  bfd_cache_close (abfd);

  abfd->iostream = (void *) vars.bim;
  abfd->flags |= BFD_IN_MEMORY /* | HAS_LOCALS */;
  abfd->iovec = &_bfd_memory_iovec;
  abfd->where = 0;
  abfd->origin = 0;
  obj_sym_filepos (abfd) = 0;

  /* Now create a symbol describing the imported value.  */
  switch (import_type)
    {
    case IMPORT_CODE:
      pe_ILF_make_a_symbol (& vars, "", symbol_name, text,
                BSF_NOT_AT_END | BSF_FUNCTION);

      break;

    case IMPORT_DATA:
      /* Nothing to do here.  */
      break;

    default:
      /* XXX code not yet written.  */
      abort ();
    }

  /* Create an import symbol for the DLL, without the .dll suffix.  */
  ptr = (bfd_byte *) strrchr (source_dll, '.');
  if (ptr)
    * ptr = 0;
  pe_ILF_make_a_symbol (& vars, "__IMPORT_DESCRIPTOR_", source_dll, NULL, 0);
  if (ptr)
    * ptr = '.';

  /* Point the bfd at the symbol table.  */
  obj_symbols (abfd) = vars.sym_cache;
  bfd_get_symcount (abfd) = vars.sym_index;

  obj_raw_syments (abfd) = vars.native_syms;
  obj_raw_syment_count (abfd) = vars.sym_index;

  obj_coff_external_syms (abfd) = (void *) vars.esym_table;
  obj_coff_keep_syms (abfd) = TRUE;

  obj_convert (abfd) = vars.sym_table;
  obj_conv_table_size (abfd) = vars.sym_index;

  obj_coff_strings (abfd) = vars.string_table;
  obj_coff_keep_strings (abfd) = TRUE;

  abfd->flags |= HAS_SYMS;

  return TRUE;

 error_return:
  if (vars.bim->buffer != NULL)
    free (vars.bim->buffer);
  free (vars.bim);
  return FALSE;
}

/* We have detected a Image Library Format archive element.
   Decode the element and return the appropriate target.  */

static const bfd_target *
pe_ILF_object_p (bfd * abfd)
{
  bfd_byte	  buffer[14];
  bfd_byte *	  ptr;
  char *	  symbol_name;
  char *	  source_dll;
  unsigned int	  machine;
  bfd_size_type	  size;
  unsigned int	  ordinal;
  unsigned int	  types;
  unsigned int	  magic;

  /* Upon entry the first six bytes of the ILF header have
      already been read.  Now read the rest of the header.  */
  if (bfd_bread (buffer, (bfd_size_type) 14, abfd) != 14)
    return NULL;

  ptr = buffer;

  machine = H_GET_16 (abfd, ptr);
  ptr += 2;

  /* Check that the machine type is recognised.  */
  magic = 0;

  switch (machine)
    {
    case IMAGE_FILE_MACHINE_UNKNOWN:
    case IMAGE_FILE_MACHINE_ALPHA:
    case IMAGE_FILE_MACHINE_ALPHA64:
    case IMAGE_FILE_MACHINE_IA64:
      break;

    case IMAGE_FILE_MACHINE_I386:
#ifdef I386MAGIC
      magic = I386MAGIC;
#endif
      break;

    case IMAGE_FILE_MACHINE_AMD64:
#ifdef AMD64MAGIC
      magic = AMD64MAGIC;
#endif
      break;

    case IMAGE_FILE_MACHINE_R3000:
    case IMAGE_FILE_MACHINE_R4000:
    case IMAGE_FILE_MACHINE_R10000:

    case IMAGE_FILE_MACHINE_MIPS16:
    case IMAGE_FILE_MACHINE_MIPSFPU:
    case IMAGE_FILE_MACHINE_MIPSFPU16:
#ifdef MIPS_ARCH_MAGIC_WINCE
      magic = MIPS_ARCH_MAGIC_WINCE;
#endif
      break;

    case IMAGE_FILE_MACHINE_SH3:
    case IMAGE_FILE_MACHINE_SH4:
#ifdef SH_ARCH_MAGIC_WINCE
      magic = SH_ARCH_MAGIC_WINCE;
#endif
      break;

    case IMAGE_FILE_MACHINE_ARM:
#ifdef ARMPEMAGIC
      magic = ARMPEMAGIC;
#endif
      break;

    case IMAGE_FILE_MACHINE_THUMB:
#ifdef THUMBPEMAGIC
      {
    extern const bfd_target TARGET_LITTLE_SYM;

    if (abfd->xvec == & TARGET_LITTLE_SYM)
      magic = THUMBPEMAGIC;
      }
#endif
      break;

    case IMAGE_FILE_MACHINE_POWERPC:
      /* We no longer support PowerPC.  */
    default:
      _bfd_error_handler
    /* xgettext:c-format */
    (_("%pB: unrecognised machine type (0x%x)"
       " in Import Library Format archive"),
     abfd, machine);
      bfd_set_error (bfd_error_malformed_archive);

      return NULL;
      break;
    }

  if (magic == 0)
    {
      _bfd_error_handler
    /* xgettext:c-format */
    (_("%pB: recognised but unhandled machine type (0x%x)"
       " in Import Library Format archive"),
     abfd, machine);
      bfd_set_error (bfd_error_wrong_format);

      return NULL;
    }

  /* We do not bother to check the date.
     date = H_GET_32 (abfd, ptr);  */
  ptr += 4;

  size = H_GET_32 (abfd, ptr);
  ptr += 4;

  if (size == 0)
    {
      _bfd_error_handler
    (_("%pB: size field is zero in Import Library Format header"), abfd);
      bfd_set_error (bfd_error_malformed_archive);

      return NULL;
    }

  ordinal = H_GET_16 (abfd, ptr);
  ptr += 2;

  types = H_GET_16 (abfd, ptr);
  /* ptr += 2; */

  /* Now read in the two strings that follow.  */
  ptr = (bfd_byte *) bfd_alloc (abfd, size);
  if (ptr == NULL)
    return NULL;

  if (bfd_bread (ptr, size, abfd) != size)
    {
      bfd_release (abfd, ptr);
      return NULL;
    }

  symbol_name = (char *) ptr;
  /* See PR 20905 for an example of where the strnlen is necessary.  */
  source_dll  = symbol_name + strnlen (symbol_name, size - 1) + 1;

  /* Verify that the strings are null terminated.  */
  if (ptr[size - 1] != 0
      || (bfd_size_type) ((bfd_byte *) source_dll - ptr) >= size)
    {
      _bfd_error_handler
    (_("%pB: string not null terminated in ILF object file"), abfd);
      bfd_set_error (bfd_error_malformed_archive);
      bfd_release (abfd, ptr);
      return NULL;
    }

  /* Now construct the bfd.  */
  if (! pe_ILF_build_a_bfd (abfd, magic, symbol_name,
                source_dll, ordinal, types))
    {
      bfd_release (abfd, ptr);
      return NULL;
    }

  return abfd->xvec;
}

static void
pe_bfd_read_buildid (bfd *abfd)
{
  pe_data_type *pe = pe_data (abfd);
  struct internal_extra_pe_aouthdr *extra = &pe->pe_opthdr;
  asection *section;
  bfd_byte *data = 0;
  bfd_size_type dataoff;
  unsigned int i;
  bfd_vma addr = extra->DataDirectory[PE_DEBUG_DATA].VirtualAddress;
  bfd_size_type size = extra->DataDirectory[PE_DEBUG_DATA].Size;

  if (size == 0)
    return;

  addr += extra->ImageBase;

  /* Search for the section containing the DebugDirectory.  */
  for (section = abfd->sections; section != NULL; section = section->next)
    {
      if ((addr >= section->vma) && (addr < (section->vma + section->size)))
    break;
    }

  if (section == NULL)
    return;

  if (!(section->flags & SEC_HAS_CONTENTS))
    return;

  dataoff = addr - section->vma;

  /* PR 20605 and 22373: Make sure that the data is really there.
     Note - since we are dealing with unsigned quantities we have
     to be careful to check for potential overflows.  */
  if (dataoff >= section->size
      || size > section->size - dataoff)
    {
      _bfd_error_handler
    (_("%pB: error: debug data ends beyond end of debug directory"),
     abfd);
      return;
    }

  /* Read the whole section. */
  if (!bfd_malloc_and_get_section (abfd, section, &data))
    {
      if (data != NULL)
    free (data);
      return;
    }

  /* Search for a CodeView entry in the DebugDirectory */
  for (i = 0; i < size / sizeof (struct external_IMAGE_DEBUG_DIRECTORY); i++)
    {
      struct external_IMAGE_DEBUG_DIRECTORY *ext
    = &((struct external_IMAGE_DEBUG_DIRECTORY *)(data + dataoff))[i];
      struct internal_IMAGE_DEBUG_DIRECTORY idd;

      _bfd_XXi_swap_debugdir_in (abfd, ext, &idd);

      if (idd.Type == PE_IMAGE_DEBUG_TYPE_CODEVIEW)
    {
      char buffer[256 + 1];
      CODEVIEW_INFO *cvinfo = (CODEVIEW_INFO *) buffer;

      /*
        The debug entry doesn't have to have to be in a section, in which
        case AddressOfRawData is 0, so always use PointerToRawData.
      */
      if (_bfd_XXi_slurp_codeview_record (abfd,
                          (file_ptr) idd.PointerToRawData,
                          idd.SizeOfData, cvinfo))
        {
          struct bfd_build_id* build_id = bfd_alloc (abfd,
             sizeof (struct bfd_build_id) + cvinfo->SignatureLength);
          if (build_id)
        {
          build_id->size = cvinfo->SignatureLength;
          memcpy(build_id->data,  cvinfo->Signature,
             cvinfo->SignatureLength);
          abfd->build_id = build_id;
        }
        }
      break;
    }
    }
}

static const bfd_target *
pe_bfd_object_p (bfd * abfd)
{
  bfd_byte buffer[6];
  struct external_DOS_hdr dos_hdr;
  struct external_PEI_IMAGE_hdr image_hdr;
  struct internal_filehdr internal_f;
  struct internal_aouthdr internal_a;
  file_ptr opt_hdr_size;
  file_ptr offset;
  const bfd_target *result;

  /* Detect if this a Microsoft Import Library Format element.  */
  /* First read the beginning of the header.  */
  if (bfd_seek (abfd, (file_ptr) 0, SEEK_SET) != 0
      || bfd_bread (buffer, (bfd_size_type) 6, abfd) != 6)
    {
      if (bfd_get_error () != bfd_error_system_call)
    bfd_set_error (bfd_error_wrong_format);
      return NULL;
    }

  /* Then check the magic and the version (only 0 is supported).  */
  if (H_GET_32 (abfd, buffer) == 0xffff0000
      && H_GET_16 (abfd, buffer + 4) == 0)
    return pe_ILF_object_p (abfd);

  if (bfd_seek (abfd, (file_ptr) 0, SEEK_SET) != 0
      || bfd_bread (&dos_hdr, (bfd_size_type) sizeof (dos_hdr), abfd)
     != sizeof (dos_hdr))
    {
      if (bfd_get_error () != bfd_error_system_call)
    bfd_set_error (bfd_error_wrong_format);
      return NULL;
    }

  /* There are really two magic numbers involved; the magic number
     that says this is a NT executable (PEI) and the magic number that
     determines the architecture.  The former is IMAGE_DOS_SIGNATURE, stored in
     the e_magic field.  The latter is stored in the f_magic field.
     If the NT magic number isn't valid, the architecture magic number
     could be mimicked by some other field (specifically, the number
     of relocs in section 3).  Since this routine can only be called
     correctly for a PEI file, check the e_magic number here, and, if
     it doesn't match, clobber the f_magic number so that we don't get
     a false match.  */
  if (H_GET_16 (abfd, dos_hdr.e_magic) != IMAGE_DOS_SIGNATURE)
    {
      bfd_set_error (bfd_error_wrong_format);
      return NULL;
    }

  offset = H_GET_32 (abfd, dos_hdr.e_lfanew);
  if (bfd_seek (abfd, offset, SEEK_SET) != 0
      || (bfd_bread (&image_hdr, (bfd_size_type) sizeof (image_hdr), abfd)
      != sizeof (image_hdr)))
    {
      if (bfd_get_error () != bfd_error_system_call)
    bfd_set_error (bfd_error_wrong_format);
      return NULL;
    }

  if (H_GET_32 (abfd, image_hdr.nt_signature) != 0x4550)
    {
      bfd_set_error (bfd_error_wrong_format);
      return NULL;
    }

  /* Swap file header, so that we get the location for calling
     real_object_p.  */
  bfd_coff_swap_filehdr_in (abfd, &image_hdr, &internal_f);

  if (! bfd_coff_bad_format_hook (abfd, &internal_f)
      || internal_f.f_opthdr > bfd_coff_aoutsz (abfd))
    {
      bfd_set_error (bfd_error_wrong_format);
      return NULL;
    }

  /* Read the optional header, which has variable size.  */
  opt_hdr_size = internal_f.f_opthdr;

  if (opt_hdr_size != 0)
    {
      bfd_size_type amt = opt_hdr_size;
      void * opthdr;

      /* PR 17521 file: 230-131433-0.004.  */
      if (amt < sizeof (PEAOUTHDR))
    amt = sizeof (PEAOUTHDR);

      opthdr = bfd_zalloc (abfd, amt);
      if (opthdr == NULL)
    return NULL;
      if (bfd_bread (opthdr, opt_hdr_size, abfd)
      != (bfd_size_type) opt_hdr_size)
    return NULL;

      bfd_set_error (bfd_error_no_error);
      bfd_coff_swap_aouthdr_in (abfd, opthdr, & internal_a);
      if (bfd_get_error () != bfd_error_no_error)
    return NULL;
    }


  result = coff_real_object_p (abfd, internal_f.f_nscns, &internal_f,
                   (opt_hdr_size != 0
                ? &internal_a
                : (struct internal_aouthdr *) NULL));


  if (result)
    {
      /* Now the whole header has been processed, see if there is a build-id */
      pe_bfd_read_buildid(abfd);
    }

  return result;
}

#define coff_object_p pe_bfd_object_p

#define STRING_SIZE_SIZE 4

#define DOT_DEBUG	".debug"
#define DOT_ZDEBUG	".zdebug"
#define GNU_LINKONCE_WI ".gnu.linkonce.wi."
#define GNU_LINKONCE_WT ".gnu.linkonce.wt."
#define DOT_RELOC	".reloc"

/* Needed to expand the inputs to BLANKOR1TOODD.  */
#define COFFLONGSECTIONCATHELPER(x,y)    x ## y
/* If the input macro Y is blank or '1', return an odd number; if it is
   '0', return an even number.  Result undefined in all other cases.  */
#define BLANKOR1TOODD(y)		 COFFLONGSECTIONCATHELPER(1,y)
/* Defined to numerical 0 or 1 according to whether generation of long
   section names is disabled or enabled by default.  */
#define COFF_ENABLE_LONG_SECTION_NAMES   (BLANKOR1TOODD(COFF_LONG_SECTION_NAMES) & 1)
/* Where long section names are supported, we allow them to be enabled
   and disabled at runtime, so select an appropriate hook function for
   _bfd_coff_set_long_section_names.  */
#define COFF_LONG_SECTION_NAMES_SETTER   bfd_coff_set_long_section_names_allowed

/* Define a macro that can be used to initialise both the fields relating
   to long section names in the backend data struct simultaneously.  */
#define COFF_DEFAULT_LONG_SECTION_NAMES  (FALSE), COFF_LONG_SECTION_NAMES_SETTER

static bfd_boolean bfd_coff_set_long_section_names_allowed
  (bfd *, int);
static bfd_boolean styp_to_sec_flags
  (bfd *, void *, const char *, asection *, flagword *);
static bfd_boolean coff_bad_format_hook
  (bfd *, void *);
static void coff_set_custom_section_alignment
  (bfd *, asection *, const struct coff_section_alignment_entry *,
   const unsigned int);
static bfd_boolean coff_new_section_hook
  (bfd *, asection *);
static bfd_boolean coff_set_arch_mach_hook
  (bfd *, void *);
static bfd_boolean coff_compute_section_file_positions
  (bfd *);
static void * buy_and_read
  (bfd *, file_ptr, bfd_size_type);
static bfd_boolean coff_slurp_line_table
  (bfd *, asection *);
static bfd_boolean coff_slurp_symbol_table
  (bfd *);
static enum coff_symbol_classification coff_classify_symbol
  (bfd *, struct internal_syment *);

/* void warning(); */

static bfd_boolean
bfd_coff_set_long_section_names_allowed (bfd *abfd, int enable)
{
  coff_backend_info (abfd)->_bfd_coff_long_section_names = enable;
  return TRUE;
}

/* The PE version; see above for the general comments.

   Since to set the SEC_LINK_ONCE and associated flags, we have to
   look at the symbol table anyway, we return the symbol table index
   of the symbol being used as the COMDAT symbol.  This is admittedly
   ugly, but there's really nowhere else that we have access to the
   required information.  FIXME: Is the COMDAT symbol index used for
   any purpose other than objdump?  */

static bfd_boolean
styp_to_sec_flags (bfd *abfd,
           void * hdr,
           const char *name,
           asection *section ATTRIBUTE_UNUSED,
           flagword *flags_ptr)
{
  struct internal_scnhdr *internal_s = (struct internal_scnhdr *) hdr;
  unsigned long styp_flags = internal_s->s_flags;
  flagword sec_flags;
  bfd_boolean result = TRUE;
  bfd_boolean is_dbg = FALSE;

  if (CONST_STRNEQ (name, DOT_DEBUG)
      || CONST_STRNEQ (name, DOT_ZDEBUG)
      || CONST_STRNEQ (name, GNU_LINKONCE_WI)
      || CONST_STRNEQ (name, GNU_LINKONCE_WT)
      || CONST_STRNEQ (name, ".stab"))
    is_dbg = TRUE;
  /* Assume read only unless IMAGE_SCN_MEM_WRITE is specified.  */
  sec_flags = SEC_READONLY;

  /* If section disallows read, then set the NOREAD flag. */
  if ((styp_flags & IMAGE_SCN_MEM_READ) == 0)
    sec_flags |= SEC_COFF_NOREAD;

  /* Process each flag bit in styp_flags in turn.  */
  while (styp_flags)
    {
      unsigned long flag = styp_flags & - styp_flags;
      char * unhandled = NULL;

      styp_flags &= ~ flag;

      /* We infer from the distinct read/write/execute bits the settings
     of some of the bfd flags; the actual values, should we need them,
     are also in pei_section_data (abfd, section)->pe_flags.  */

      switch (flag)
    {
    case STYP_DSECT:
      unhandled = "STYP_DSECT";
      break;
    case STYP_GROUP:
      unhandled = "STYP_GROUP";
      break;
    case STYP_COPY:
      unhandled = "STYP_COPY";
      break;
    case STYP_OVER:
      unhandled = "STYP_OVER";
      break;
#ifdef SEC_NEVER_LOAD
    case STYP_NOLOAD:
      sec_flags |= SEC_NEVER_LOAD;
      break;
#endif
    case IMAGE_SCN_MEM_READ:
      sec_flags &= ~SEC_COFF_NOREAD;
      break;
    case IMAGE_SCN_TYPE_NO_PAD:
      /* Skip.  */
      break;
    case IMAGE_SCN_LNK_OTHER:
      unhandled = "IMAGE_SCN_LNK_OTHER";
      break;
    case IMAGE_SCN_MEM_NOT_CACHED:
      unhandled = "IMAGE_SCN_MEM_NOT_CACHED";
      break;
    case IMAGE_SCN_MEM_NOT_PAGED:
      /* Generate a warning message rather using the 'unhandled'
         variable as this will allow some .sys files generate by
         other toolchains to be processed.  See bugzilla issue 196.  */
      /* xgettext:c-format */
      _bfd_error_handler (_("%pB: warning: ignoring section flag"
                " %s in section %s"),
                  abfd, "IMAGE_SCN_MEM_NOT_PAGED", name);
      break;
    case IMAGE_SCN_MEM_EXECUTE:
      sec_flags |= SEC_CODE;
      break;
    case IMAGE_SCN_MEM_WRITE:
      sec_flags &= ~ SEC_READONLY;
      break;
    case IMAGE_SCN_MEM_DISCARDABLE:
      /* The MS PE spec says that debug sections are DISCARDABLE,
         but the presence of a DISCARDABLE flag does not necessarily
         mean that a given section contains debug information.  Thus
         we only set the SEC_DEBUGGING flag on sections that we
         recognise as containing debug information.  */
         if (is_dbg
#ifdef _COMMENT
          || strcmp (name, _COMMENT) == 0
#endif
          )
        {
          sec_flags |= SEC_DEBUGGING | SEC_READONLY;
        }
      break;
    case IMAGE_SCN_MEM_SHARED:
      sec_flags |= SEC_COFF_SHARED;
      break;
    case IMAGE_SCN_LNK_REMOVE:
      if (!is_dbg)
        sec_flags |= SEC_EXCLUDE;
      break;
    case IMAGE_SCN_CNT_CODE:
      sec_flags |= SEC_CODE | SEC_ALLOC | SEC_LOAD;
      break;
    case IMAGE_SCN_CNT_INITIALIZED_DATA:
      if (is_dbg)
        sec_flags |= SEC_DEBUGGING;
      else
        sec_flags |= SEC_DATA | SEC_ALLOC | SEC_LOAD;
      break;
    case IMAGE_SCN_CNT_UNINITIALIZED_DATA:
      sec_flags |= SEC_ALLOC;
      break;
    case IMAGE_SCN_LNK_INFO:
      /* We mark these as SEC_DEBUGGING, but only if COFF_PAGE_SIZE is
         defined.  coff_compute_section_file_positions uses
         COFF_PAGE_SIZE to ensure that the low order bits of the
         section VMA and the file offset match.  If we don't know
         COFF_PAGE_SIZE, we can't ensure the correct correspondence,
         and demand page loading of the file will fail.  */
#ifdef COFF_PAGE_SIZE
      sec_flags |= SEC_DEBUGGING;
#endif
      break;
    case IMAGE_SCN_LNK_COMDAT:
      break;
    default:
      /* Silently ignore for now.  */
      break;
    }

      /* If the section flag was not handled, report it here.  */
      if (unhandled != NULL)
    {
      _bfd_error_handler
        /* xgettext:c-format */
        (_("%pB (%s): section flag %s (%#lx) ignored"),
         abfd, name, unhandled, flag);
      result = FALSE;
    }
    }

  /* As a GNU extension, if the name begins with .gnu.linkonce, we
     only link a single copy of the section.  This is used to support
     g++.  g++ will emit each template expansion in its own section.
     The symbols will be defined as weak, so that multiple definitions
     are permitted.  The GNU linker extension is to actually discard
     all but one of the sections.  */
  if (CONST_STRNEQ (name, ".gnu.linkonce"))
    sec_flags |= SEC_LINK_ONCE | SEC_LINK_DUPLICATES_DISCARD;

  if (flags_ptr)
    * flags_ptr = sec_flags;

  return result;
}

#define	get_index(symbol)	((symbol)->udata.i)

/*
INTERNAL_DEFINITION
	bfd_coff_backend_data

CODE_FRAGMENT

.{* COFF symbol classifications.  *}
.
.enum coff_symbol_classification
.{
.  {* Global symbol.  *}
.  COFF_SYMBOL_GLOBAL,
.  {* Common symbol.  *}
.  COFF_SYMBOL_COMMON,
.  {* Undefined symbol.  *}
.  COFF_SYMBOL_UNDEFINED,
.  {* Local symbol.  *}
.  COFF_SYMBOL_LOCAL,
.  {* PE section symbol.  *}
.  COFF_SYMBOL_PE_SECTION
.};
.
.typedef asection * (*coff_gc_mark_hook_fn)
.  (asection *, struct bfd_link_info *, struct internal_reloc *,
.   struct coff_link_hash_entry *, struct internal_syment *);
.
Special entry points for gdb to swap in coff symbol table parts:
.typedef struct
.{
.  void (*_bfd_coff_swap_aux_in)
.    (bfd *, void *, int, int, int, int, void *);
.
.  void (*_bfd_coff_swap_sym_in)
.    (bfd *, void *, void *);
.
.  void (*_bfd_coff_swap_lineno_in)
.    (bfd *, void *, void *);
.
.  unsigned int (*_bfd_coff_swap_aux_out)
.    (bfd *, void *, int, int, int, int, void *);
.
.  unsigned int (*_bfd_coff_swap_sym_out)
.    (bfd *, void *, void *);
.
.  unsigned int (*_bfd_coff_swap_lineno_out)
.    (bfd *, void *, void *);
.
.  unsigned int (*_bfd_coff_swap_reloc_out)
.    (bfd *, void *, void *);
.
.  unsigned int (*_bfd_coff_swap_filehdr_out)
.    (bfd *, void *, void *);
.
.  unsigned int (*_bfd_coff_swap_aouthdr_out)
.    (bfd *, void *, void *);
.
.  unsigned int (*_bfd_coff_swap_scnhdr_out)
.    (bfd *, void *, void *);
.
.  unsigned int _bfd_filhsz;
.  unsigned int _bfd_aoutsz;
.  unsigned int _bfd_scnhsz;
.  unsigned int _bfd_symesz;
.  unsigned int _bfd_auxesz;
.  unsigned int _bfd_relsz;
.  unsigned int _bfd_linesz;
.  unsigned int _bfd_filnmlen;
.  bfd_boolean _bfd_coff_long_filenames;
.
.  bfd_boolean _bfd_coff_long_section_names;
.  bfd_boolean (*_bfd_coff_set_long_section_names)
.    (bfd *, int);
.
.  unsigned int _bfd_coff_default_section_alignment_power;
.  bfd_boolean _bfd_coff_force_symnames_in_strings;
.  unsigned int _bfd_coff_debug_string_prefix_length;
.  unsigned int _bfd_coff_max_nscns;
.
.  void (*_bfd_coff_swap_filehdr_in)
.    (bfd *, void *, void *);
.
.  void (*_bfd_coff_swap_aouthdr_in)
.    (bfd *, void *, void *);
.
.  void (*_bfd_coff_swap_scnhdr_in)
.    (bfd *, void *, void *);
.
.  void (*_bfd_coff_swap_reloc_in)
.    (bfd *abfd, void *, void *);
.
.  bfd_boolean (*_bfd_coff_bad_format_hook)
.    (bfd *, void *);
.
.  bfd_boolean (*_bfd_coff_set_arch_mach_hook)
.    (bfd *, void *);
.
.  void * (*_bfd_coff_mkobject_hook)
.    (bfd *, void *, void *);
.
.  bfd_boolean (*_bfd_styp_to_sec_flags_hook)
.    (bfd *, void *, const char *, asection *, flagword *);
.
.  void (*_bfd_set_alignment_hook)
.    (bfd *, asection *, void *);
.
.  bfd_boolean (*_bfd_coff_slurp_symbol_table)
.    (bfd *);
.
.  bfd_boolean (*_bfd_coff_symname_in_debug)
.    (bfd *, struct internal_syment *);
.
.  bfd_boolean (*_bfd_coff_pointerize_aux_hook)
.    (bfd *, combined_entry_type *, combined_entry_type *,
.     unsigned int, combined_entry_type *);
.
.  bfd_boolean (*_bfd_coff_print_aux)
.    (bfd *, FILE *, combined_entry_type *, combined_entry_type *,
.     combined_entry_type *, unsigned int);
.
.  void (*_bfd_coff_reloc16_extra_cases)
.    (bfd *, struct bfd_link_info *, struct bfd_link_order *, arelent *,
.     bfd_byte *, unsigned int *, unsigned int *);
.
.  int (*_bfd_coff_reloc16_estimate)
.    (bfd *, asection *, arelent *, unsigned int,
.     struct bfd_link_info *);
.
.  enum coff_symbol_classification (*_bfd_coff_classify_symbol)
.    (bfd *, struct internal_syment *);
.
.  bfd_boolean (*_bfd_coff_compute_section_file_positions)
.    (bfd *);
.
.  bfd_boolean (*_bfd_coff_start_final_link)
.    (bfd *, struct bfd_link_info *);
.
.  bfd_boolean (*_bfd_coff_relocate_section)
.    (bfd *, struct bfd_link_info *, bfd *, asection *, bfd_byte *,
.     struct internal_reloc *, struct internal_syment *, asection **);
.
.  reloc_howto_type *(*_bfd_coff_rtype_to_howto)
.    (bfd *, asection *, struct internal_reloc *,
.     struct coff_link_hash_entry *, struct internal_syment *, bfd_vma *);
.
.  bfd_boolean (*_bfd_coff_adjust_symndx)
.    (bfd *, struct bfd_link_info *, bfd *, asection *,
.     struct internal_reloc *, bfd_boolean *);
.
.  bfd_boolean (*_bfd_coff_link_add_one_symbol)
.    (struct bfd_link_info *, bfd *, const char *, flagword,
.     asection *, bfd_vma, const char *, bfd_boolean, bfd_boolean,
.     struct bfd_link_hash_entry **);
.
.  bfd_boolean (*_bfd_coff_link_output_has_begun)
.    (bfd *, struct coff_final_link_info *);
.
.  bfd_boolean (*_bfd_coff_final_link_postscript)
.    (bfd *, struct coff_final_link_info *);
.
.  bfd_boolean (*_bfd_coff_print_pdata)
.    (bfd *, void *);
.
.} bfd_coff_backend_data;
.
.#define coff_backend_info(abfd) \
.  ((bfd_coff_backend_data *) (abfd)->xvec->backend_data)
.
.#define bfd_coff_swap_aux_in(a,e,t,c,ind,num,i) \
.  ((coff_backend_info (a)->_bfd_coff_swap_aux_in) (a,e,t,c,ind,num,i))
.
.#define bfd_coff_swap_sym_in(a,e,i) \
.  ((coff_backend_info (a)->_bfd_coff_swap_sym_in) (a,e,i))
.
.#define bfd_coff_swap_lineno_in(a,e,i) \
.  ((coff_backend_info ( a)->_bfd_coff_swap_lineno_in) (a,e,i))
.
.#define bfd_coff_swap_reloc_out(abfd, i, o) \
.  ((coff_backend_info (abfd)->_bfd_coff_swap_reloc_out) (abfd, i, o))
.
.#define bfd_coff_swap_lineno_out(abfd, i, o) \
.  ((coff_backend_info (abfd)->_bfd_coff_swap_lineno_out) (abfd, i, o))
.
.#define bfd_coff_swap_aux_out(a,i,t,c,ind,num,o) \
.  ((coff_backend_info (a)->_bfd_coff_swap_aux_out) (a,i,t,c,ind,num,o))
.
.#define bfd_coff_swap_sym_out(abfd, i,o) \
.  ((coff_backend_info (abfd)->_bfd_coff_swap_sym_out) (abfd, i, o))
.
.#define bfd_coff_swap_scnhdr_out(abfd, i,o) \
.  ((coff_backend_info (abfd)->_bfd_coff_swap_scnhdr_out) (abfd, i, o))
.
.#define bfd_coff_swap_filehdr_out(abfd, i,o) \
.  ((coff_backend_info (abfd)->_bfd_coff_swap_filehdr_out) (abfd, i, o))
.
.#define bfd_coff_swap_aouthdr_out(abfd, i,o) \
.  ((coff_backend_info (abfd)->_bfd_coff_swap_aouthdr_out) (abfd, i, o))
.
.#define bfd_coff_filhsz(abfd) (coff_backend_info (abfd)->_bfd_filhsz)
.#define bfd_coff_aoutsz(abfd) (coff_backend_info (abfd)->_bfd_aoutsz)
.#define bfd_coff_scnhsz(abfd) (coff_backend_info (abfd)->_bfd_scnhsz)
.#define bfd_coff_symesz(abfd) (coff_backend_info (abfd)->_bfd_symesz)
.#define bfd_coff_auxesz(abfd) (coff_backend_info (abfd)->_bfd_auxesz)
.#define bfd_coff_relsz(abfd)  (coff_backend_info (abfd)->_bfd_relsz)
.#define bfd_coff_linesz(abfd) (coff_backend_info (abfd)->_bfd_linesz)
.#define bfd_coff_filnmlen(abfd) (coff_backend_info (abfd)->_bfd_filnmlen)
.#define bfd_coff_long_filenames(abfd) \
.  (coff_backend_info (abfd)->_bfd_coff_long_filenames)
.#define bfd_coff_long_section_names(abfd) \
.  (coff_backend_info (abfd)->_bfd_coff_long_section_names)
.#define bfd_coff_set_long_section_names(abfd, enable) \
.  ((coff_backend_info (abfd)->_bfd_coff_set_long_section_names) (abfd, enable))
.#define bfd_coff_default_section_alignment_power(abfd) \
.  (coff_backend_info (abfd)->_bfd_coff_default_section_alignment_power)
.#define bfd_coff_max_nscns(abfd) \
.  (coff_backend_info (abfd)->_bfd_coff_max_nscns)
.
.#define bfd_coff_swap_filehdr_in(abfd, i,o) \
.  ((coff_backend_info (abfd)->_bfd_coff_swap_filehdr_in) (abfd, i, o))
.
.#define bfd_coff_swap_aouthdr_in(abfd, i,o) \
.  ((coff_backend_info (abfd)->_bfd_coff_swap_aouthdr_in) (abfd, i, o))
.
.#define bfd_coff_swap_scnhdr_in(abfd, i,o) \
.  ((coff_backend_info (abfd)->_bfd_coff_swap_scnhdr_in) (abfd, i, o))
.
.#define bfd_coff_swap_reloc_in(abfd, i, o) \
.  ((coff_backend_info (abfd)->_bfd_coff_swap_reloc_in) (abfd, i, o))
.
.#define bfd_coff_bad_format_hook(abfd, filehdr) \
.  ((coff_backend_info (abfd)->_bfd_coff_bad_format_hook) (abfd, filehdr))
.
.#define bfd_coff_set_arch_mach_hook(abfd, filehdr)\
.  ((coff_backend_info (abfd)->_bfd_coff_set_arch_mach_hook) (abfd, filehdr))
.#define bfd_coff_mkobject_hook(abfd, filehdr, aouthdr)\
.  ((coff_backend_info (abfd)->_bfd_coff_mkobject_hook)\
.   (abfd, filehdr, aouthdr))
.
.#define bfd_coff_styp_to_sec_flags_hook(abfd, scnhdr, name, section, flags_ptr)\
.  ((coff_backend_info (abfd)->_bfd_styp_to_sec_flags_hook)\
.   (abfd, scnhdr, name, section, flags_ptr))
.
.#define bfd_coff_set_alignment_hook(abfd, sec, scnhdr)\
.  ((coff_backend_info (abfd)->_bfd_set_alignment_hook) (abfd, sec, scnhdr))
.
.#define bfd_coff_slurp_symbol_table(abfd)\
.  ((coff_backend_info (abfd)->_bfd_coff_slurp_symbol_table) (abfd))
.
.#define bfd_coff_symname_in_debug(abfd, sym)\
.  ((coff_backend_info (abfd)->_bfd_coff_symname_in_debug) (abfd, sym))
.
.#define bfd_coff_force_symnames_in_strings(abfd)\
.  (coff_backend_info (abfd)->_bfd_coff_force_symnames_in_strings)
.
.#define bfd_coff_debug_string_prefix_length(abfd)\
.  (coff_backend_info (abfd)->_bfd_coff_debug_string_prefix_length)
.
.#define bfd_coff_print_aux(abfd, file, base, symbol, aux, indaux)\
.  ((coff_backend_info (abfd)->_bfd_coff_print_aux)\
.   (abfd, file, base, symbol, aux, indaux))
.
.#define bfd_coff_reloc16_extra_cases(abfd, link_info, link_order,\
.				      reloc, data, src_ptr, dst_ptr)\
.  ((coff_backend_info (abfd)->_bfd_coff_reloc16_extra_cases)\
.   (abfd, link_info, link_order, reloc, data, src_ptr, dst_ptr))
.
.#define bfd_coff_reloc16_estimate(abfd, section, reloc, shrink, link_info)\
.  ((coff_backend_info (abfd)->_bfd_coff_reloc16_estimate)\
.   (abfd, section, reloc, shrink, link_info))
.
.#define bfd_coff_classify_symbol(abfd, sym)\
.  ((coff_backend_info (abfd)->_bfd_coff_classify_symbol)\
.   (abfd, sym))
.
.#define bfd_coff_compute_section_file_positions(abfd)\
.  ((coff_backend_info (abfd)->_bfd_coff_compute_section_file_positions)\
.   (abfd))
.
.#define bfd_coff_start_final_link(obfd, info)\
.  ((coff_backend_info (obfd)->_bfd_coff_start_final_link)\
.   (obfd, info))
.#define bfd_coff_relocate_section(obfd,info,ibfd,o,con,rel,isyms,secs)\
.  ((coff_backend_info (ibfd)->_bfd_coff_relocate_section)\
.   (obfd, info, ibfd, o, con, rel, isyms, secs))
.#define bfd_coff_rtype_to_howto(abfd, sec, rel, h, sym, addendp)\
.  ((coff_backend_info (abfd)->_bfd_coff_rtype_to_howto)\
.   (abfd, sec, rel, h, sym, addendp))
.#define bfd_coff_adjust_symndx(obfd, info, ibfd, sec, rel, adjustedp)\
.  ((coff_backend_info (abfd)->_bfd_coff_adjust_symndx)\
.   (obfd, info, ibfd, sec, rel, adjustedp))
.#define bfd_coff_link_add_one_symbol(info, abfd, name, flags, section,\
.				      value, string, cp, coll, hashp)\
.  ((coff_backend_info (abfd)->_bfd_coff_link_add_one_symbol)\
.   (info, abfd, name, flags, section, value, string, cp, coll, hashp))
.
.#define bfd_coff_link_output_has_begun(a,p) \
.  ((coff_backend_info (a)->_bfd_coff_link_output_has_begun) (a, p))
.#define bfd_coff_final_link_postscript(a,p) \
.  ((coff_backend_info (a)->_bfd_coff_final_link_postscript) (a, p))
.
.#define bfd_coff_have_print_pdata(a) \
.  (coff_backend_info (a)->_bfd_coff_print_pdata)
.#define bfd_coff_print_pdata(a,p) \
.  ((coff_backend_info (a)->_bfd_coff_print_pdata) (a, p))
.
.{* Macro: Returns true if the bfd is a PE executable as opposed to a
.   PE object file.  *}
.#define bfd_pei_p(abfd) \
.  (CONST_STRNEQ ((abfd)->xvec->name, "pei-"))
*/

/* See whether the magic number matches.  */

static bfd_boolean
coff_bad_format_hook (bfd * abfd ATTRIBUTE_UNUSED, void * filehdr)
{
  struct internal_filehdr *internal_f = (struct internal_filehdr *) filehdr;

  if (BADMAG (*internal_f))
    return FALSE;

  return TRUE;
}

/* Check whether this section uses an alignment other than the
   default.  */

static void
coff_set_custom_section_alignment (bfd *abfd ATTRIBUTE_UNUSED,
				   asection *section,
				   const struct coff_section_alignment_entry *alignment_table,
				   const unsigned int table_size)
{
  const unsigned int default_alignment = COFF_DEFAULT_SECTION_ALIGNMENT_POWER;
  unsigned int i;

  for (i = 0; i < table_size; ++i)
    {
      const char *secname = bfd_get_section_name (abfd, section);

      if (alignment_table[i].comparison_length == (unsigned int) -1
	  ? strcmp (alignment_table[i].name, secname) == 0
	  : strncmp (alignment_table[i].name, secname,
		     alignment_table[i].comparison_length) == 0)
	break;
    }
  if (i >= table_size)
    return;

  if (alignment_table[i].default_alignment_min != COFF_ALIGNMENT_FIELD_EMPTY
      && default_alignment < alignment_table[i].default_alignment_min)
    return;

  if (alignment_table[i].default_alignment_max != COFF_ALIGNMENT_FIELD_EMPTY
#if COFF_DEFAULT_SECTION_ALIGNMENT_POWER != 0
      && default_alignment > alignment_table[i].default_alignment_max
#endif
      )
    return;

  section->alignment_power = alignment_table[i].alignment_power;
}

/* Custom section alignment records.  */

static const struct coff_section_alignment_entry
coff_section_alignment_table[] =
{
#ifdef COFF_SECTION_ALIGNMENT_ENTRIES
  COFF_SECTION_ALIGNMENT_ENTRIES,
#endif
  /* There must not be any gaps between .stabstr sections.  */
  { COFF_SECTION_NAME_PARTIAL_MATCH (".stabstr"),
    1, COFF_ALIGNMENT_FIELD_EMPTY, 0 },
  /* The .stab section must be aligned to 2**2 at most, to avoid gaps.  */
  { COFF_SECTION_NAME_PARTIAL_MATCH (".stab"),
    3, COFF_ALIGNMENT_FIELD_EMPTY, 2 },
  /* Similarly for the .ctors and .dtors sections.  */
  { COFF_SECTION_NAME_EXACT_MATCH (".ctors"),
    3, COFF_ALIGNMENT_FIELD_EMPTY, 2 },
  { COFF_SECTION_NAME_EXACT_MATCH (".dtors"),
    3, COFF_ALIGNMENT_FIELD_EMPTY, 2 }
};

static const unsigned int coff_section_alignment_table_size =
  sizeof coff_section_alignment_table / sizeof coff_section_alignment_table[0];

/* Initialize a section structure with information peculiar to this
   particular implementation of COFF.  */

static bfd_boolean
coff_new_section_hook (bfd * abfd, asection * section)
{
  combined_entry_type *native;
  bfd_size_type amt;
  unsigned char sclass = C_STAT;

  section->alignment_power = COFF_DEFAULT_SECTION_ALIGNMENT_POWER;

  /* Set up the section symbol.  */
  if (!_bfd_generic_new_section_hook (abfd, section))
    return FALSE;

  /* Allocate aux records for section symbols, to store size and
     related info.

     @@ The 10 is a guess at a plausible maximum number of aux entries
     (but shouldn't be a constant).  */
  amt = sizeof (combined_entry_type) * 10;
  native = (combined_entry_type *) bfd_zalloc (abfd, amt);
  if (native == NULL)
    return FALSE;

  /* We don't need to set up n_name, n_value, or n_scnum in the native
     symbol information, since they'll be overridden by the BFD symbol
     anyhow.  However, we do need to set the type and storage class,
     in case this symbol winds up getting written out.  The value 0
     for n_numaux is already correct.  */

  native->is_sym = TRUE;
  native->u.syment.n_type = T_NULL;
  native->u.syment.n_sclass = sclass;

  coffsymbol (section->symbol)->native = native;

  coff_set_custom_section_alignment (abfd, section,
				     coff_section_alignment_table,
				     coff_section_alignment_table_size);

  return TRUE;
}

static void
coff_set_alignment_hook (bfd * abfd ATTRIBUTE_UNUSED,
			 asection * section,
			 void * scnhdr)
{
  struct internal_scnhdr *hdr = (struct internal_scnhdr *) scnhdr;
  bfd_size_type amt;
  unsigned int alignment_power_const
    = hdr->s_flags & IMAGE_SCN_ALIGN_POWER_BIT_MASK;

  switch (alignment_power_const)
    {
    case IMAGE_SCN_ALIGN_8192BYTES:
    case IMAGE_SCN_ALIGN_4096BYTES:
    case IMAGE_SCN_ALIGN_2048BYTES:
    case IMAGE_SCN_ALIGN_1024BYTES:
    case IMAGE_SCN_ALIGN_512BYTES:
    case IMAGE_SCN_ALIGN_256BYTES:
    case IMAGE_SCN_ALIGN_128BYTES:
    case IMAGE_SCN_ALIGN_64BYTES:
    case IMAGE_SCN_ALIGN_32BYTES:
    case IMAGE_SCN_ALIGN_16BYTES:
    case IMAGE_SCN_ALIGN_8BYTES:
    case IMAGE_SCN_ALIGN_4BYTES:
    case IMAGE_SCN_ALIGN_2BYTES:
    case IMAGE_SCN_ALIGN_1BYTES:
      section->alignment_power
	= IMAGE_SCN_ALIGN_POWER_NUM (alignment_power_const);
      break;
    default:
      break;
    }

  /* In a PE image file, the s_paddr field holds the virtual size of a
     section, while the s_size field holds the raw size.  We also keep
     the original section flag value, since not every bit can be
     mapped onto a generic BFD section bit.  */
  if (coff_section_data (abfd, section) == NULL)
    {
      amt = sizeof (struct coff_section_tdata);
      section->used_by_bfd = bfd_zalloc (abfd, amt);
      if (section->used_by_bfd == NULL)
	/* FIXME: Return error.  */
	abort ();
    }

  if (pei_section_data (abfd, section) == NULL)
    {
      amt = sizeof (struct pei_section_tdata);
      coff_section_data (abfd, section)->tdata = bfd_zalloc (abfd, amt);
      if (coff_section_data (abfd, section)->tdata == NULL)
	/* FIXME: Return error.  */
	abort ();
    }
  pei_section_data (abfd, section)->virt_size = hdr->s_paddr;
  pei_section_data (abfd, section)->pe_flags = hdr->s_flags;

  section->lma = hdr->s_vaddr;
}
#undef ALIGN_SET
#undef ELIFALIGN_SET

/* Determine the machine architecture and type.  FIXME: This is target
   dependent because the magic numbers are defined in the target
   dependent header files.  But there is no particular need for this.
   If the magic numbers were moved to a separate file, this function
   would be target independent and would also be much more successful
   at linking together COFF files for different architectures.  */

static bfd_boolean
coff_set_arch_mach_hook (bfd *abfd, void * filehdr)
{
  unsigned long machine;
  enum bfd_architecture arch;
  struct internal_filehdr *internal_f = (struct internal_filehdr *) filehdr;

  /* Zero selects the default machine for an arch.  */
  machine = 0;
  switch (internal_f->f_magic)
    {
    case AMD64MAGIC:
      arch = bfd_arch_i386;
      machine = bfd_mach_x86_64;
      break;

    default:			/* Unreadable input file type.  */
      arch = bfd_arch_obscure;
      break;
    }

  bfd_default_set_arch_mach (abfd, arch, machine);
  return TRUE;
}

static bfd_boolean
symname_in_debug_hook (bfd *abfd ATTRIBUTE_UNUSED,
		       struct internal_syment *sym ATTRIBUTE_UNUSED)
{
  return FALSE;
}

#define coff_pointerize_aux_hook 0

/* Print an aux entry.  This returns TRUE if it has printed it.  */

static bfd_boolean
coff_print_aux (bfd *abfd ATTRIBUTE_UNUSED,
        FILE *file ATTRIBUTE_UNUSED,
        combined_entry_type *table_base ATTRIBUTE_UNUSED,
        combined_entry_type *symbol ATTRIBUTE_UNUSED,
        combined_entry_type *aux ATTRIBUTE_UNUSED,
        unsigned int indaux ATTRIBUTE_UNUSED)
{
  /* Return FALSE to indicate that no special action was taken.  */
  return FALSE;
}

/*
SUBSUBSECTION
	Writing relocations

	To write relocations, the back end steps though the
	canonical relocation table and create an
	@code{internal_reloc}. The symbol index to use is removed from
	the @code{offset} field in the symbol table supplied.  The
	address comes directly from the sum of the section base
	address and the relocation offset; the type is dug directly
	from the howto field.  Then the @code{internal_reloc} is
	swapped into the shape of an @code{external_reloc} and written
	out to disk.

*/

/* Calculate the file position for each section.  */

static bfd_boolean
coff_compute_section_file_positions (bfd * abfd ATTRIBUTE_UNUSED)
{
  return TRUE;
}

static void *
buy_and_read (bfd *abfd, file_ptr where, bfd_size_type size)
{
  void * area = bfd_alloc (abfd, size);

  if (!area)
    return NULL;
  if (bfd_seek (abfd, where, SEEK_SET) != 0
      || bfd_bread (area, size, abfd) != size)
    return NULL;
  return area;
}

/*
SUBSUBSECTION
	Reading linenumbers

	Creating the linenumber table is done by reading in the entire
	coff linenumber table, and creating another table for internal use.

	A coff linenumber table is structured so that each function
	is marked as having a line number of 0. Each line within the
	function is an offset from the first line in the function. The
	base of the line number information for the table is stored in
	the symbol associated with the function.

	Note: The PE format uses line number 0 for a flag indicating a
	new source file.

	The information is copied from the external to the internal
	table, and each symbol which marks a function is marked by
	pointing its...

	How does this work ?
*/

static int
coff_sort_func_alent (const void * arg1, const void * arg2)
{
  const alent *al1 = *(const alent **) arg1;
  const alent *al2 = *(const alent **) arg2;
  const coff_symbol_type *s1 = (const coff_symbol_type *) (al1->u.sym);
  const coff_symbol_type *s2 = (const coff_symbol_type *) (al2->u.sym);

  if (s1 == NULL || s2 == NULL)
    return 0;
  if (s1->symbol.value < s2->symbol.value)
    return -1;
  else if (s1->symbol.value > s2->symbol.value)
    return 1;

  return 0;
}

static bfd_boolean
coff_slurp_line_table (bfd *abfd, asection *asect)
{
  LINENO *native_lineno;
  alent *lineno_cache;
  bfd_size_type amt;
  unsigned int counter;
  alent *cache_ptr;
  bfd_vma prev_offset = 0;
  bfd_boolean ordered = TRUE;
  unsigned int nbr_func;
  LINENO *src;
  bfd_boolean have_func;
  bfd_boolean ret = TRUE;

  if (asect->lineno_count == 0)
    return TRUE;

  BFD_ASSERT (asect->lineno == NULL);

  if (asect->lineno_count > asect->size)
    {
      _bfd_error_handler
	(_("%pB: warning: line number count (%#lx) exceeds section size (%#lx)"),
	 abfd, (unsigned long) asect->lineno_count, (unsigned long) asect->size);
      return FALSE;
    }

  amt = ((bfd_size_type) asect->lineno_count + 1) * sizeof (alent);
  lineno_cache = (alent *) bfd_alloc (abfd, amt);
  if (lineno_cache == NULL)
    return FALSE;

  amt = (bfd_size_type) bfd_coff_linesz (abfd) * asect->lineno_count;
  native_lineno = (LINENO *) buy_and_read (abfd, asect->line_filepos, amt);
  if (native_lineno == NULL)
    {
      _bfd_error_handler
	(_("%pB: warning: line number table read failed"), abfd);
      bfd_release (abfd, lineno_cache);
      return FALSE;
    }

  cache_ptr = lineno_cache;
  asect->lineno = lineno_cache;
  src = native_lineno;
  nbr_func = 0;
  have_func = FALSE;

  for (counter = 0; counter < asect->lineno_count; counter++, src++)
    {
      struct internal_lineno dst;

      bfd_coff_swap_lineno_in (abfd, src, &dst);
      cache_ptr->line_number = dst.l_lnno;
      /* Appease memory checkers that get all excited about
	 uninitialised memory when copying alents if u.offset is
	 larger than u.sym.  (64-bit BFD on 32-bit host.)  */
      memset (&cache_ptr->u, 0, sizeof (cache_ptr->u));

      if (cache_ptr->line_number == 0)
	{
	  combined_entry_type * ent;
	  unsigned long symndx;
	  coff_symbol_type *sym;

	  have_func = FALSE;
	  symndx = dst.l_addr.l_symndx;
	  if (symndx >= obj_raw_syment_count (abfd))
	    {
	      _bfd_error_handler
		/* xgettext:c-format */
		(_("%pB: warning: illegal symbol index 0x%lx in line number entry %d"),
		 abfd, symndx, counter);
	      cache_ptr->line_number = -1;
	      ret = FALSE;
	      continue;
	    }

	  ent = obj_raw_syments (abfd) + symndx;
	  /* FIXME: We should not be casting between ints and
	     pointers like this.  */
	  if (! ent->is_sym)
	    {
	      _bfd_error_handler
		/* xgettext:c-format */
		(_("%pB: warning: illegal symbol index 0x%lx in line number entry %d"),
		 abfd, symndx, counter);
	      cache_ptr->line_number = -1;
	      ret = FALSE;
	      continue;
	    }
	  sym = (coff_symbol_type *) (ent->u.syment._n._n_n._n_zeroes);

	  /* PR 17512 file: 078-10659-0.004  */
	  if (sym < obj_symbols (abfd)
	      || sym >= obj_symbols (abfd) + bfd_get_symcount (abfd))
	    {
	      _bfd_error_handler
		/* xgettext:c-format */
		(_("%pB: warning: illegal symbol in line number entry %d"),
		 abfd, counter);
	      cache_ptr->line_number = -1;
	      ret = FALSE;
	      continue;
	    }

	  have_func = TRUE;
	  nbr_func++;
	  cache_ptr->u.sym = (asymbol *) sym;
	  if (sym->lineno != NULL)
	    _bfd_error_handler
	      /* xgettext:c-format */
	      (_("%pB: warning: duplicate line number information for `%s'"),
	       abfd, bfd_asymbol_name (&sym->symbol));

	  sym->lineno = cache_ptr;
	  if (sym->symbol.value < prev_offset)
	    ordered = FALSE;
	  prev_offset = sym->symbol.value;
	}
      else if (!have_func)
	/* Drop line information that has no associated function.
	   PR 17521: file: 078-10659-0.004.  */
	continue;
      else
	cache_ptr->u.offset = (dst.l_addr.l_paddr
			       - bfd_section_vma (abfd, asect));
      cache_ptr++;
    }

  asect->lineno_count = cache_ptr - lineno_cache;
  memset (cache_ptr, 0, sizeof (*cache_ptr));
  bfd_release (abfd, native_lineno);

  /* On some systems (eg AIX5.3) the lineno table may not be sorted.  */
  if (!ordered)
    {
      /* Sort the table.  */
      alent **func_table;
      alent *n_lineno_cache;

      /* Create a table of functions.  */
      func_table = (alent **) bfd_alloc (abfd, nbr_func * sizeof (alent *));
      if (func_table != NULL)
	{
	  alent **p = func_table;
	  unsigned int i;

	  for (i = 0; i < asect->lineno_count; i++)
	    if (lineno_cache[i].line_number == 0)
	      *p++ = &lineno_cache[i];

	  BFD_ASSERT ((unsigned int) (p - func_table) == nbr_func);

	  /* Sort by functions.  */
	  qsort (func_table, nbr_func, sizeof (alent *), coff_sort_func_alent);

	  /* Create the new sorted table.  */
	  amt = (bfd_size_type) asect->lineno_count * sizeof (alent);
	  n_lineno_cache = (alent *) bfd_alloc (abfd, amt);
	  if (n_lineno_cache != NULL)
	    {
	      alent *n_cache_ptr = n_lineno_cache;

	      for (i = 0; i < nbr_func; i++)
		{
		  coff_symbol_type *sym;
		  alent *old_ptr = func_table[i];

		  /* Update the function entry.  */
		  sym = (coff_symbol_type *) old_ptr->u.sym;
		  /* PR binutils/17512: Point the lineno to where
		     this entry will be after the memcpy below.  */
		  sym->lineno = lineno_cache + (n_cache_ptr - n_lineno_cache);
		  /* Copy the function and line number entries.  */
		  do
		    *n_cache_ptr++ = *old_ptr++;
		  while (old_ptr->line_number != 0);
		}
	      BFD_ASSERT ((bfd_size_type) (n_cache_ptr - n_lineno_cache) == (amt / sizeof (alent)));

	      memcpy (lineno_cache, n_lineno_cache, amt);
	    }
	  else
	    ret = FALSE;
	  bfd_release (abfd, func_table);
	}
      else
	ret = FALSE;
    }

  return ret;
}

/* Slurp in the symbol table, converting it to generic form.  Note
   that if coff_relocate_section is defined, the linker will read
   symbols via coff_link_add_symbols, rather than via this routine.  */

static bfd_boolean
coff_slurp_symbol_table (bfd * abfd)
{
  combined_entry_type *native_symbols;
  coff_symbol_type *cached_area;
  unsigned int *table_ptr;
  bfd_size_type amt;
  unsigned int number_of_symbols = 0;
  bfd_boolean ret = TRUE;

  if (obj_symbols (abfd))
    return TRUE;

  /* Read in the symbol table.  */
  if ((native_symbols = coff_get_normalized_symtab (abfd)) == NULL)
    return FALSE;

  /* Allocate enough room for all the symbols in cached form.  */
  amt = obj_raw_syment_count (abfd);
  amt *= sizeof (coff_symbol_type);
  cached_area = (coff_symbol_type *) bfd_alloc (abfd, amt);
  if (cached_area == NULL)
    return FALSE;

  amt = obj_raw_syment_count (abfd);
  amt *= sizeof (unsigned int);
  table_ptr = (unsigned int *) bfd_zalloc (abfd, amt);

  if (table_ptr == NULL)
    return FALSE;
  else
    {
      coff_symbol_type *dst = cached_area;
      unsigned int last_native_index = obj_raw_syment_count (abfd);
      unsigned int this_index = 0;

      while (this_index < last_native_index)
	{
	  combined_entry_type *src = native_symbols + this_index;
	  table_ptr[this_index] = number_of_symbols;

	  dst->symbol.the_bfd = abfd;
	  BFD_ASSERT (src->is_sym);
	  dst->symbol.name = (char *) (src->u.syment._n._n_n._n_offset);
	  /* We use the native name field to point to the cached field.  */
	  src->u.syment._n._n_n._n_zeroes = (bfd_hostptr_t) dst;
	  dst->symbol.section = coff_section_from_bfd_index (abfd,
						     src->u.syment.n_scnum);
	  dst->symbol.flags = 0;
	  /* PR 17512: file: 079-7098-0.001:0.1.  */
	  dst->symbol.value = 0;
	  dst->done_lineno = FALSE;

	  switch (src->u.syment.n_sclass)
	    {
	    case C_EXT:
	    case C_WEAKEXT:
#ifdef C_SYSTEM
	    case C_SYSTEM:	/* System Wide variable.  */
#endif
	    /* In PE, 0x68 (104) denotes a section symbol.  */
	    case C_SECTION:
	    /* In PE, 0x69 (105) denotes a weak external symbol.  */
	    case C_NT_WEAK:
	      switch (coff_classify_symbol (abfd, &src->u.syment))
		{
		case COFF_SYMBOL_GLOBAL:
		  dst->symbol.flags = BSF_EXPORT | BSF_GLOBAL;
		  /* PE sets the symbol to a value relative to the
		     start of the section.  */
		  dst->symbol.value = src->u.syment.n_value;
		  if (ISFCN ((src->u.syment.n_type)))
		    /* A function ext does not go at the end of a
		       file.  */
		    dst->symbol.flags |= BSF_NOT_AT_END | BSF_FUNCTION;
		  break;

		case COFF_SYMBOL_COMMON:
		  dst->symbol.section = bfd_com_section_ptr;
		  dst->symbol.value = src->u.syment.n_value;
		  break;

		case COFF_SYMBOL_UNDEFINED:
		  dst->symbol.section = bfd_und_section_ptr;
		  dst->symbol.value = 0;
		  break;

		case COFF_SYMBOL_PE_SECTION:
		  dst->symbol.flags |= BSF_EXPORT | BSF_SECTION_SYM;
		  dst->symbol.value = 0;
		  break;

		case COFF_SYMBOL_LOCAL:
		  dst->symbol.flags = BSF_LOCAL;
		  /* PE sets the symbol to a value relative to the
		     start of the section.  */
		  dst->symbol.value = src->u.syment.n_value;
		  if (ISFCN ((src->u.syment.n_type)))
		    dst->symbol.flags |= BSF_NOT_AT_END | BSF_FUNCTION;
		  break;
		}


	      if (src->u.syment.n_sclass == C_NT_WEAK)
		dst->symbol.flags |= BSF_WEAK;

	      if (src->u.syment.n_sclass == C_SECTION
		  && src->u.syment.n_scnum > 0)
		dst->symbol.flags = BSF_LOCAL;
	      if (src->u.syment.n_sclass == C_WEAKEXT
		  )
		dst->symbol.flags |= BSF_WEAK;

	      break;

	    case C_STAT:	 /* Static.  */
	    case C_LABEL:	 /* Label.  */
	      if (src->u.syment.n_scnum == N_DEBUG)
		dst->symbol.flags = BSF_DEBUGGING;
	      else
		dst->symbol.flags = BSF_LOCAL;

	      /* Base the value as an index from the base of the
		 section, if there is one.  */
	      if (dst->symbol.section)
		{
		  /* PE sets the symbol to a value relative to the
		     start of the section.  */
		  dst->symbol.value = src->u.syment.n_value;
		}
	      else
		dst->symbol.value = src->u.syment.n_value;
	      break;

	    case C_MOS:		/* Member of structure.  */
	    case C_EOS:		/* End of structure.  */
	    case C_REGPARM:	/* Register parameter.  */
	    case C_REG:		/* register variable.  */
	      /* C_AUTOARG conflicts with TI COFF C_UEXT.  */
	    case C_TPDEF:	/* Type definition.  */
	    case C_ARG:
	    case C_AUTO:	/* Automatic variable.  */
	    case C_FIELD:	/* Bit field.  */
	    case C_ENTAG:	/* Enumeration tag.  */
	    case C_MOE:		/* Member of enumeration.  */
	    case C_MOU:		/* Member of union.  */
	    case C_UNTAG:	/* Union tag.  */
	      dst->symbol.flags = BSF_DEBUGGING;
	      dst->symbol.value = (src->u.syment.n_value);
	      break;

	    case C_FILE:	/* File name.  */
	    case C_STRTAG:	/* Structure tag.  */
	      dst->symbol.flags = BSF_DEBUGGING;
	      dst->symbol.value = (src->u.syment.n_value);
	      break;

	    case C_BLOCK:	/* ".bb" or ".eb".  */
	    case C_FCN:		/* ".bf" or ".ef" (or PE ".lf").  */
	    case C_EFCN:	/* Physical end of function.  */
	      /* PE sets the symbol to a value relative to the start
		 of the section.  */
	      dst->symbol.value = src->u.syment.n_value;
	      if (strcmp (dst->symbol.name, ".bf") != 0)
		{
		  /* PE uses funny values for .ef and .lf; don't
		     relocate them.  */
		  dst->symbol.flags = BSF_DEBUGGING;
		}
	      else
		dst->symbol.flags = BSF_DEBUGGING | BSF_DEBUGGING_RELOC;
	      break;

	    case C_STATLAB:	/* Static load time label.  */
	      dst->symbol.value = src->u.syment.n_value;
	      dst->symbol.flags = BSF_GLOBAL;
	      break;

	    case C_NULL:
	      /* PE DLLs sometimes have zeroed out symbols for some
		 reason.  Just ignore them without a warning.  */
	      if (src->u.syment.n_type == 0
		  && src->u.syment.n_value == 0
		  && src->u.syment.n_scnum == 0)
		break;
	      /* Fall through.  */
	    case C_EXTDEF:	/* External definition.  */
	    case C_ULABEL:	/* Undefined label.  */
	    case C_USTATIC:	/* Undefined static.  */
	      /* New storage classes for TI COFF.  */
	    case C_EXTLAB:	/* External load time label.  */
	    default:
	      _bfd_error_handler
		/* xgettext:c-format */
		(_("%pB: unrecognized storage class %d for %s symbol `%s'"),
		 abfd, src->u.syment.n_sclass,
		 dst->symbol.section->name, dst->symbol.name);
	      ret = FALSE;
	      /* Fall through.  */
	    case C_HIDDEN:	/* Ext symbol in dmert public lib.  */
	      /* PR 20722: These symbols can also be generated by
		 building DLLs with --gc-sections enabled.  */
	      dst->symbol.flags = BSF_DEBUGGING;
	      dst->symbol.value = (src->u.syment.n_value);
	      break;
	    }

	  dst->native = src;
	  dst->symbol.udata.i = 0;
	  dst->lineno = NULL;

	  this_index += (src->u.syment.n_numaux) + 1;
	  dst++;
	  number_of_symbols++;
	}
    }

  obj_symbols (abfd) = cached_area;
  obj_raw_syments (abfd) = native_symbols;

  bfd_get_symcount (abfd) = number_of_symbols;
  obj_convert (abfd) = table_ptr;
  /* Slurp the line tables for each section too.  */
  {
    asection *p;

    p = abfd->sections;
    while (p)
      {
	if (! coff_slurp_line_table (abfd, p))
	  return FALSE;
	p = p->next;
      }
  }

  return ret;
}

/* Classify a COFF symbol.  A couple of targets have globally visible
   symbols which are not class C_EXT, and this handles those.  It also
   recognizes some special PE cases.  */

static enum coff_symbol_classification
coff_classify_symbol (bfd *abfd,
		      struct internal_syment *syment)
{
  /* FIXME: This partially duplicates the switch in
     coff_slurp_symbol_table.  */
  switch (syment->n_sclass)
    {
    case C_EXT:
    case C_WEAKEXT:
#ifdef C_SYSTEM
    case C_SYSTEM:
#endif
    case C_NT_WEAK:
      if (syment->n_scnum == 0)
	{
	  if (syment->n_value == 0)
	    return COFF_SYMBOL_UNDEFINED;
	  else
	    return COFF_SYMBOL_COMMON;
	}
      return COFF_SYMBOL_GLOBAL;

    default:
      break;
    }

  if (syment->n_sclass == C_STAT)
    {
      if (syment->n_scnum == 0)
	/* The Microsoft compiler sometimes generates these if a
	   small static function is inlined every time it is used.
	   The function is discarded, but the symbol table entry
	   remains.  */
	return COFF_SYMBOL_LOCAL;

      return COFF_SYMBOL_LOCAL;
    }

  if (syment->n_sclass == C_SECTION)
    {
      /* In some cases in a DLL generated by the Microsoft linker, the
	 n_value field will contain garbage.  FIXME: This should
	 probably be handled by the swapping function instead.  */
      syment->n_value = 0;
      if (syment->n_scnum == 0)
	return COFF_SYMBOL_UNDEFINED;
      return COFF_SYMBOL_PE_SECTION;
    }

  /* If it is not a global symbol, we presume it is a local symbol.  */
  if (syment->n_scnum == 0)
    {
      char buf[SYMNMLEN + 1];

      _bfd_error_handler
	/* xgettext:c-format */
	(_("warning: %pB: local symbol `%s' has no section"),
	 abfd, _bfd_coff_internal_syment_name (abfd, syment, buf));
    }

  return COFF_SYMBOL_LOCAL;
}

#ifndef coff_rtype_to_howto
#define coff_rtype_to_howto NULL
#endif /* ! defined (coff_rtype_to_howto) */

#ifndef coff_set_reloc
#define coff_set_reloc _bfd_generic_set_reloc
#endif

#ifndef coff_reloc16_estimate
#define coff_reloc16_estimate dummy_reloc16_estimate

static int
dummy_reloc16_estimate (bfd *abfd ATTRIBUTE_UNUSED,
			asection *input_section ATTRIBUTE_UNUSED,
			arelent *reloc ATTRIBUTE_UNUSED,
			unsigned int shrink ATTRIBUTE_UNUSED,
			struct bfd_link_info *link_info ATTRIBUTE_UNUSED)
{
  abort ();
  return 0;
}

#endif

#ifndef coff_reloc16_extra_cases

#define coff_reloc16_extra_cases dummy_reloc16_extra_cases

/* This works even if abort is not declared in any header file.  */

static void
dummy_reloc16_extra_cases (bfd *abfd ATTRIBUTE_UNUSED,
			   struct bfd_link_info *link_info ATTRIBUTE_UNUSED,
			   struct bfd_link_order *link_order ATTRIBUTE_UNUSED,
			   arelent *reloc ATTRIBUTE_UNUSED,
			   bfd_byte *data ATTRIBUTE_UNUSED,
			   unsigned int *src_ptr ATTRIBUTE_UNUSED,
			   unsigned int *dst_ptr ATTRIBUTE_UNUSED)
{
  abort ();
}
#endif

/* If coff_relocate_section is defined, we can use the optimized COFF
   backend linker.  Otherwise we must continue to use the old linker.  */

#ifdef coff_relocate_section

#ifndef coff_bfd_link_hash_table_create
#define coff_bfd_link_hash_table_create _bfd_coff_link_hash_table_create
#endif
#ifndef coff_bfd_link_add_symbols
#define coff_bfd_link_add_symbols _bfd_coff_link_add_symbols
#endif
#ifndef coff_bfd_final_link
#define coff_bfd_final_link _bfd_coff_final_link
#endif

#else /* ! defined (coff_relocate_section) */

#define coff_relocate_section NULL
#ifndef coff_bfd_link_hash_table_create
#define coff_bfd_link_hash_table_create _bfd_generic_link_hash_table_create
#endif
#ifndef coff_bfd_link_add_symbols
#define coff_bfd_link_add_symbols _bfd_generic_link_add_symbols
#endif
#define coff_bfd_final_link _bfd_generic_final_link

#endif /* ! defined (coff_relocate_section) */

#define coff_bfd_link_just_syms      _bfd_generic_link_just_syms
#define coff_bfd_copy_link_hash_symbol_type \
  _bfd_generic_copy_link_hash_symbol_type
#define coff_bfd_link_split_section  _bfd_generic_link_split_section

#define coff_bfd_link_check_relocs   _bfd_generic_link_check_relocs

#ifndef coff_start_final_link
#define coff_start_final_link NULL
#endif

#ifndef coff_adjust_symndx
#define coff_adjust_symndx NULL
#endif

#ifndef coff_link_add_one_symbol
#define coff_link_add_one_symbol _bfd_generic_link_add_one_symbol
#endif

#ifndef coff_link_output_has_begun

static bfd_boolean
coff_link_output_has_begun (bfd * abfd,
			    struct coff_final_link_info * info ATTRIBUTE_UNUSED)
{
  return abfd->output_has_begun;
}
#endif

#ifndef coff_final_link_postscript

static bfd_boolean
coff_final_link_postscript (bfd * abfd ATTRIBUTE_UNUSED,
			    struct coff_final_link_info * pfinfo ATTRIBUTE_UNUSED)
{
  return TRUE;
}
#endif

#ifndef coff_SWAP_aux_in
#define coff_SWAP_aux_in coff_swap_aux_in
#endif
#ifndef coff_SWAP_sym_in
#define coff_SWAP_sym_in coff_swap_sym_in
#endif
#ifndef coff_SWAP_lineno_in
#define coff_SWAP_lineno_in coff_swap_lineno_in
#endif
#ifndef coff_SWAP_aux_out
#define coff_SWAP_aux_out coff_swap_aux_out
#endif
#ifndef coff_SWAP_sym_out
#define coff_SWAP_sym_out coff_swap_sym_out
#endif
#ifndef coff_SWAP_lineno_out
#define coff_SWAP_lineno_out coff_swap_lineno_out
#endif
#ifndef coff_SWAP_reloc_out
#define coff_SWAP_reloc_out coff_swap_reloc_out
#endif
#ifndef coff_SWAP_filehdr_out
#define coff_SWAP_filehdr_out coff_swap_filehdr_out
#endif
#ifndef coff_SWAP_aouthdr_out
#define coff_SWAP_aouthdr_out coff_swap_aouthdr_out
#endif
#ifndef coff_SWAP_scnhdr_out
#define coff_SWAP_scnhdr_out coff_swap_scnhdr_out
#endif
#ifndef coff_SWAP_reloc_in
#define coff_SWAP_reloc_in coff_swap_reloc_in
#endif
#ifndef coff_SWAP_filehdr_in
#define coff_SWAP_filehdr_in coff_swap_filehdr_in
#endif
#ifndef coff_SWAP_aouthdr_in
#define coff_SWAP_aouthdr_in coff_swap_aouthdr_in
#endif
#ifndef coff_SWAP_scnhdr_in
#define coff_SWAP_scnhdr_in coff_swap_scnhdr_in
#endif

static bfd_coff_backend_data bfd_coff_std_swap_table ATTRIBUTE_UNUSED =
{
  coff_SWAP_aux_in, coff_SWAP_sym_in, coff_SWAP_lineno_in,
  coff_SWAP_aux_out, coff_SWAP_sym_out,
  coff_SWAP_lineno_out, coff_SWAP_reloc_out,
  coff_SWAP_filehdr_out, coff_SWAP_aouthdr_out,
  coff_SWAP_scnhdr_out,
  FILHSZ, AOUTSZ, SCNHSZ, SYMESZ, AUXESZ, RELSZ, LINESZ, FILNMLEN,
#ifdef COFF_LONG_FILENAMES
  TRUE,
#else
  FALSE,
#endif
  COFF_DEFAULT_LONG_SECTION_NAMES,
  COFF_DEFAULT_SECTION_ALIGNMENT_POWER,
#ifdef COFF_FORCE_SYMBOLS_IN_STRINGS
  TRUE,
#else
  FALSE,
#endif
#ifdef COFF_DEBUG_STRING_WIDE_PREFIX
  4,
#else
  2,
#endif
  32768,
  coff_SWAP_filehdr_in, coff_SWAP_aouthdr_in, coff_SWAP_scnhdr_in,
  coff_SWAP_reloc_in, coff_bad_format_hook, coff_set_arch_mach_hook,
  coff_mkobject_hook, styp_to_sec_flags, coff_set_alignment_hook,
  coff_slurp_symbol_table, symname_in_debug_hook, coff_pointerize_aux_hook,
  coff_print_aux, coff_reloc16_extra_cases, coff_reloc16_estimate,
  coff_classify_symbol, coff_compute_section_file_positions,
  coff_start_final_link, coff_relocate_section, coff_rtype_to_howto,
  coff_adjust_symndx, coff_link_add_one_symbol,
  coff_link_output_has_begun, coff_final_link_postscript,
  bfd_pe_print_pdata
};


#ifndef coff_close_and_cleanup
#define coff_close_and_cleanup		    _bfd_generic_close_and_cleanup
#endif

#ifndef coff_bfd_free_cached_info
#define coff_bfd_free_cached_info	    _bfd_generic_bfd_free_cached_info
#endif

#ifndef coff_get_section_contents
#define coff_get_section_contents	    _bfd_generic_get_section_contents
#endif

#ifndef coff_bfd_copy_private_symbol_data
#define coff_bfd_copy_private_symbol_data   _bfd_generic_bfd_copy_private_symbol_data
#endif

#ifndef coff_bfd_copy_private_header_data
#define coff_bfd_copy_private_header_data   _bfd_generic_bfd_copy_private_header_data
#endif

#ifndef coff_bfd_copy_private_section_data
#define coff_bfd_copy_private_section_data  _bfd_generic_bfd_copy_private_section_data
#endif

#ifndef coff_bfd_copy_private_bfd_data
#define coff_bfd_copy_private_bfd_data      _bfd_generic_bfd_copy_private_bfd_data
#endif

#ifndef coff_bfd_merge_private_bfd_data
#define coff_bfd_merge_private_bfd_data     _bfd_generic_bfd_merge_private_bfd_data
#endif

#ifndef coff_bfd_set_private_flags
#define coff_bfd_set_private_flags	    _bfd_generic_bfd_set_private_flags
#endif

#ifndef coff_bfd_print_private_bfd_data
#define coff_bfd_print_private_bfd_data     _bfd_generic_bfd_print_private_bfd_data
#endif

#ifndef coff_bfd_is_local_label_name
#define coff_bfd_is_local_label_name	    _bfd_coff_is_local_label_name
#endif

#ifndef coff_bfd_is_target_special_symbol
#define coff_bfd_is_target_special_symbol   _bfd_bool_bfd_asymbol_false
#endif

#ifndef coff_read_minisymbols
#define coff_read_minisymbols		    _bfd_generic_read_minisymbols
#endif

#ifndef coff_minisymbol_to_symbol
#define coff_minisymbol_to_symbol	    _bfd_generic_minisymbol_to_symbol
#endif

/* The reloc lookup routine must be supplied by each individual COFF
   backend.  */
#ifndef coff_bfd_reloc_type_lookup
#define coff_bfd_reloc_type_lookup	    _bfd_norelocs_bfd_reloc_type_lookup
#endif
#ifndef coff_bfd_reloc_name_lookup
#define coff_bfd_reloc_name_lookup    _bfd_norelocs_bfd_reloc_name_lookup
#endif

#ifndef coff_bfd_get_relocated_section_contents
#define coff_bfd_get_relocated_section_contents \
  bfd_generic_get_relocated_section_contents
#endif

#ifndef coff_bfd_relax_section
#define coff_bfd_relax_section		    bfd_generic_relax_section
#endif

#ifndef coff_bfd_gc_sections
#define coff_bfd_gc_sections		    bfd_coff_gc_sections
#endif

#ifndef coff_bfd_lookup_section_flags
#define coff_bfd_lookup_section_flags	    bfd_generic_lookup_section_flags
#endif

#ifndef coff_bfd_merge_sections
#define coff_bfd_merge_sections		    bfd_generic_merge_sections
#endif

#ifndef coff_bfd_is_group_section
#define coff_bfd_is_group_section	    bfd_generic_is_group_section
#endif

#ifndef coff_bfd_discard_group
#define coff_bfd_discard_group		    bfd_generic_discard_group
#endif

#ifndef coff_section_already_linked
#define coff_section_already_linked \
  _bfd_coff_section_already_linked
#endif

#ifndef coff_bfd_define_common_symbol
#define coff_bfd_define_common_symbol	    bfd_generic_define_common_symbol
#endif

#ifndef coff_bfd_link_hide_symbol
#define coff_bfd_link_hide_symbol	    _bfd_generic_link_hide_symbol
#endif

#ifndef coff_bfd_define_start_stop
#define coff_bfd_define_start_stop	    bfd_generic_define_start_stop
#endif

#ifdef PE
#define amd64coff_object_p pe_bfd_object_p
#else
#define amd64coff_object_p coff_object_p
#endif

const bfd_target
  TARGET_SYM =
{
  TARGET_NAME,
  bfd_target_coff_flavour,
  BFD_ENDIAN_LITTLE,		/* Data byte order is little.  */
  BFD_ENDIAN_LITTLE,		/* Header byte order is little.  */

  (HAS_RELOC | EXEC_P		/* Object flags.  */
   | HAS_LINENO | HAS_DEBUG
   | HAS_SYMS | HAS_LOCALS | WP_TEXT | D_PAGED | BFD_COMPRESS | BFD_DECOMPRESS),

  (SEC_HAS_CONTENTS | SEC_ALLOC | SEC_LOAD | SEC_RELOC /* Section flags.  */
   | SEC_LINK_ONCE | SEC_LINK_DUPLICATES | SEC_READONLY | SEC_DEBUGGING
   | SEC_CODE | SEC_DATA | SEC_EXCLUDE ),

#ifdef TARGET_UNDERSCORE
  TARGET_UNDERSCORE,		/* Leading underscore.  */
#else
  0,				/* Leading underscore.  */
#endif
  '/',				/* Ar_pad_char.  */
  15,				/* Ar_max_namelen.  */
  0,				/* match priority.  */

  bfd_getl64, bfd_getl_signed_64, bfd_putl64,
     bfd_getl32, bfd_getl_signed_32, bfd_putl32,
     bfd_getl16, bfd_getl_signed_16, bfd_putl16, /* Data.  */
  bfd_getl64, bfd_getl_signed_64, bfd_putl64,
     bfd_getl32, bfd_getl_signed_32, bfd_putl32,
     bfd_getl16, bfd_getl_signed_16, bfd_putl16, /* Hdrs.  */

  /* Note that we allow an object file to be treated as a core file as well.  */
  {				/* bfd_check_format.  */
    _bfd_dummy_target,
    amd64coff_object_p,
    _bfd_dummy_target,
    _bfd_dummy_target
  },
  {				/* bfd_set_format.  */
    _bfd_bool_bfd_false_error,
    _bfd_bool_bfd_false_error,
    _bfd_bool_bfd_false_error,
    _bfd_bool_bfd_false_error
  },
  {				/* bfd_write_contents.  */
    _bfd_bool_bfd_false_error,
    _bfd_bool_bfd_false_error,
    _bfd_bool_bfd_false_error,
    _bfd_bool_bfd_false_error
  },

  BFD_JUMP_TABLE_GENERIC (coff),
  BFD_JUMP_TABLE_COPY (coff),
  BFD_JUMP_TABLE_CORE (_bfd_nocore),
  BFD_JUMP_TABLE_ARCHIVE (_bfd_noarchive),
  BFD_JUMP_TABLE_SYMBOLS (coff),
  BFD_JUMP_TABLE_RELOCS (_bfd_norelocs),
  BFD_JUMP_TABLE_WRITE (_bfd_nowrite),
  BFD_JUMP_TABLE_LINK (_bfd_nolink),
  BFD_JUMP_TABLE_DYNAMIC (_bfd_nodynamic),

  NULL,

  COFF_SWAP_TABLE
};
