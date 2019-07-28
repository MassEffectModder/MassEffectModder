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

#ifndef PCRELOFFSET
#define PCRELOFFSET TRUE
#endif

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

#include "peicode.h"

#define STRING_SIZE_SIZE 4

#define DOT_DEBUG	".debug"
#define DOT_ZDEBUG	".zdebug"
#define GNU_LINKONCE_WI ".gnu.linkonce.wi."
#define GNU_LINKONCE_WT ".gnu.linkonce.wt."
#define DOT_RELOC	".reloc"

#if defined (COFF_LONG_SECTION_NAMES)
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
#else /* !defined (COFF_LONG_SECTION_NAMES) */
/* If long section names are not supported, this stub disallows any
   attempt to enable them at run-time.  */
#define COFF_LONG_SECTION_NAMES_SETTER   bfd_coff_set_long_section_names_disallowed
#endif /* defined (COFF_LONG_SECTION_NAMES) */

/* Define a macro that can be used to initialise both the fields relating
   to long section names in the backend data struct simultaneously.  */
#if COFF_ENABLE_LONG_SECTION_NAMES
#define COFF_DEFAULT_LONG_SECTION_NAMES  (TRUE), COFF_LONG_SECTION_NAMES_SETTER
#else /* !COFF_ENABLE_LONG_SECTION_NAMES */
#define COFF_DEFAULT_LONG_SECTION_NAMES  (FALSE), COFF_LONG_SECTION_NAMES_SETTER
#endif /* COFF_ENABLE_LONG_SECTION_NAMES */

#if defined (COFF_LONG_SECTION_NAMES)
static bfd_boolean bfd_coff_set_long_section_names_allowed
  (bfd *, int);
#else /* !defined (COFF_LONG_SECTION_NAMES) */
static bfd_boolean bfd_coff_set_long_section_names_disallowed
  (bfd *, int);
#endif /* defined (COFF_LONG_SECTION_NAMES) */
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
static bfd_boolean coff_set_flags
  (bfd *, unsigned int *, unsigned short *);
static bfd_boolean coff_set_arch_mach
  (bfd *, enum bfd_architecture, unsigned long) ATTRIBUTE_UNUSED;
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
#ifndef coff_mkobject_hook
static void * coff_mkobject_hook
  (bfd *, void *,  void *);
#endif
#ifdef COFF_WITH_PE
static flagword handle_COMDAT
  (bfd *, flagword, void *, const char *, asection *);
#endif
#ifdef TICOFF
static bfd_boolean ticoff0_bad_format_hook
  (bfd *, void * );
static bfd_boolean ticoff1_bad_format_hook
  (bfd *, void * );
#endif

/* void warning(); */

#if defined (COFF_LONG_SECTION_NAMES)
static bfd_boolean
bfd_coff_set_long_section_names_allowed (bfd *abfd, int enable)
{
  coff_backend_info (abfd)->_bfd_coff_long_section_names = enable;
  return TRUE;
}
#else /* !defined (COFF_LONG_SECTION_NAMES) */
static bfd_boolean
bfd_coff_set_long_section_names_disallowed (bfd *abfd, int enable)
{
  (void) abfd;
  (void) enable;
  return FALSE;
}
#endif /* defined (COFF_LONG_SECTION_NAMES) */

/* Return a word with STYP_* (scnhdr.s_flags) flags set to represent
   the incoming SEC_* flags.  The inverse of this function is
   styp_to_sec_flags().  NOTE: If you add to/change this routine, you
   should probably mirror the changes in styp_to_sec_flags().  */

#ifndef COFF_WITH_PE

/* Macros for setting debugging flags.  */

#ifdef STYP_DEBUG
#define STYP_XCOFF_DEBUG STYP_DEBUG
#else
#define STYP_XCOFF_DEBUG STYP_INFO
#endif

#ifdef COFF_ALIGN_IN_S_FLAGS
#define STYP_DEBUG_INFO STYP_DSECT
#else
#define STYP_DEBUG_INFO STYP_INFO
#endif

#endif /* COFF_WITH_PE */

/* Return a word with SEC_* flags set to represent the incoming STYP_*
   flags (from scnhdr.s_flags).  The inverse of this function is
   sec_to_styp_flags().  NOTE: If you add to/change this routine, you
   should probably mirror the changes in sec_to_styp_flags().  */

static flagword
handle_COMDAT (bfd * abfd,
           flagword sec_flags,
           void * hdr,
           const char *name,
           asection *section)
{
  struct internal_scnhdr *internal_s = (struct internal_scnhdr *) hdr;
  bfd_byte *esymstart, *esym, *esymend;
  int seen_state = 0;
  char *target_name = NULL;

  sec_flags |= SEC_LINK_ONCE;

  /* Unfortunately, the PE format stores essential information in
     the symbol table, of all places.  We need to extract that
     information now, so that objdump and the linker will know how
     to handle the section without worrying about the symbols.  We
     can't call slurp_symtab, because the linker doesn't want the
     swapped symbols.  */

  /* COMDAT sections are special.  The first symbol is the section
     symbol, which tells what kind of COMDAT section it is.  The
     second symbol is the "comdat symbol" - the one with the
     unique name.  GNU uses the section symbol for the unique
     name; MS uses ".text" for every comdat section.  Sigh.  - DJ */

  /* This is not mirrored in sec_to_styp_flags(), but there
     doesn't seem to be a need to, either, and it would at best be
     rather messy.  */

  if (! _bfd_coff_get_external_symbols (abfd))
    return sec_flags;

  esymstart = esym = (bfd_byte *) obj_coff_external_syms (abfd);
  esymend = esym + obj_raw_syment_count (abfd) * bfd_coff_symesz (abfd);

  while (esym < esymend)
    {
      struct internal_syment isym;
      char buf[SYMNMLEN + 1];
      const char *symname;

      bfd_coff_swap_sym_in (abfd, esym, & isym);

      BFD_ASSERT (sizeof (internal_s->s_name) <= SYMNMLEN);

      if (isym.n_scnum == section->target_index)
    {
      /* According to the MSVC documentation, the first
         TWO entries with the section # are both of
         interest to us.  The first one is the "section
         symbol" (section name).  The second is the comdat
         symbol name.  Here, we've found the first
         qualifying entry; we distinguish it from the
         second with a state flag.

         In the case of gas-generated (at least until that
         is fixed) .o files, it isn't necessarily the
         second one.  It may be some other later symbol.

         Since gas also doesn't follow MS conventions and
         emits the section similar to .text$<name>, where
         <something> is the name we're looking for, we
         distinguish the two as follows:

         If the section name is simply a section name (no
         $) we presume it's MS-generated, and look at
         precisely the second symbol for the comdat name.
         If the section name has a $, we assume it's
         gas-generated, and look for <something> (whatever
         follows the $) as the comdat symbol.  */

      /* All 3 branches use this.  */
      symname = _bfd_coff_internal_syment_name (abfd, &isym, buf);

      /* PR 17512 file: 078-11867-0.004  */
      if (symname == NULL)
        {
          _bfd_error_handler (_("%pB: unable to load COMDAT section name"),
                  abfd);
          break;
        }

      switch (seen_state)
        {
        case 0:
          {
        /* The first time we've seen the symbol.  */
        union internal_auxent aux;

        /* If it isn't the stuff we're expecting, die;
           The MS documentation is vague, but it
           appears that the second entry serves BOTH
           as the comdat symbol and the defining
           symbol record (either C_STAT or C_EXT,
           possibly with an aux entry with debug
           information if it's a function.)  It
           appears the only way to find the second one
           is to count.  (On Intel, they appear to be
           adjacent, but on Alpha, they have been
           found separated.)

           Here, we think we've found the first one,
           but there's some checking we can do to be
           sure.  */

        if (! ((isym.n_sclass == C_STAT
            || isym.n_sclass == C_EXT)
               && BTYPE (isym.n_type) == T_NULL
               && isym.n_value == 0))
          {
            /* Malformed input files can trigger this test.
               cf PR 21781.  */
            _bfd_error_handler (_("%pB: error: unexpected symbol '%s' in COMDAT section"),
                    abfd, symname);
            goto breakloop;
          }

        /* FIXME LATER: MSVC generates section names
           like .text for comdats.  Gas generates
           names like .text$foo__Fv (in the case of a
           function).  See comment above for more.  */

        if (isym.n_sclass == C_STAT && strcmp (name, symname) != 0)
          /* xgettext:c-format */
          _bfd_error_handler (_("%pB: warning: COMDAT symbol '%s'"
                    " does not match section name '%s'"),
                      abfd, symname, name);

        seen_state = 1;

        /* PR 17512: file: e2cfe54f.  */
        if (esym + bfd_coff_symesz (abfd) >= esymend)
          {
            /* xgettext:c-format */
            _bfd_error_handler (_("%pB: warning: no symbol for"
                      " section '%s' found"),
                    abfd, symname);
            break;
          }
        /* This is the section symbol.  */
        bfd_coff_swap_aux_in (abfd, (esym + bfd_coff_symesz (abfd)),
                      isym.n_type, isym.n_sclass,
                      0, isym.n_numaux, & aux);

        target_name = strchr (name, '$');
        if (target_name != NULL)
          {
            /* Gas mode.  */
            seen_state = 2;
            /* Skip the `$'.  */
            target_name += 1;
          }

        /* FIXME: Microsoft uses NODUPLICATES and
           ASSOCIATIVE, but gnu uses ANY and
           SAME_SIZE.  Unfortunately, gnu doesn't do
           the comdat symbols right.  So, until we can
           fix it to do the right thing, we are
           temporarily disabling comdats for the MS
           types (they're used in DLLs and C++, but we
           don't support *their* C++ libraries anyway
           - DJ.  */

        /* Cygwin does not follow the MS style, and
           uses ANY and SAME_SIZE where NODUPLICATES
           and ASSOCIATIVE should be used.  For
           Interix, we just do the right thing up
           front.  */

        switch (aux.x_scn.x_comdat)
          {
          case IMAGE_COMDAT_SELECT_NODUPLICATES:
#ifdef STRICT_PE_FORMAT
            sec_flags |= SEC_LINK_DUPLICATES_ONE_ONLY;
#else
            sec_flags &= ~SEC_LINK_ONCE;
#endif
            break;

          case IMAGE_COMDAT_SELECT_ANY:
            sec_flags |= SEC_LINK_DUPLICATES_DISCARD;
            break;

          case IMAGE_COMDAT_SELECT_SAME_SIZE:
            sec_flags |= SEC_LINK_DUPLICATES_SAME_SIZE;
            break;

          case IMAGE_COMDAT_SELECT_EXACT_MATCH:
            /* Not yet fully implemented ??? */
            sec_flags |= SEC_LINK_DUPLICATES_SAME_CONTENTS;
            break;

            /* debug$S gets this case; other
               implications ??? */

            /* There may be no symbol... we'll search
               the whole table... Is this the right
               place to play this game? Or should we do
               it when reading it in.  */
          case IMAGE_COMDAT_SELECT_ASSOCIATIVE:
#ifdef STRICT_PE_FORMAT
            /* FIXME: This is not currently implemented.  */
            sec_flags |= SEC_LINK_DUPLICATES_DISCARD;
#else
            sec_flags &= ~SEC_LINK_ONCE;
#endif
            break;

          default:  /* 0 means "no symbol" */
            /* debug$F gets this case; other
               implications ??? */
            sec_flags |= SEC_LINK_DUPLICATES_DISCARD;
            break;
          }
          }
          break;

        case 2:
          /* Gas mode: the first matching on partial name.  */

#ifndef TARGET_UNDERSCORE
#define TARGET_UNDERSCORE 0
#endif
          /* Is this the name we're looking for ?  */
          if (strcmp (target_name,
              symname + (TARGET_UNDERSCORE ? 1 : 0)) != 0)
        {
          /* Not the name we're looking for */
          esym += (isym.n_numaux + 1) * bfd_coff_symesz (abfd);
          continue;
        }
          /* Fall through.  */
        case 1:
          /* MSVC mode: the lexically second symbol (or
         drop through from the above).  */
          {
        char *newname;
        bfd_size_type amt;

        /* This must the second symbol with the
           section #.  It is the actual symbol name.
           Intel puts the two adjacent, but Alpha (at
           least) spreads them out.  */

        amt = sizeof (struct coff_comdat_info);
        coff_section_data (abfd, section)->comdat
          = (struct coff_comdat_info *) bfd_alloc (abfd, amt);
        if (coff_section_data (abfd, section)->comdat == NULL)
          abort ();

        coff_section_data (abfd, section)->comdat->symbol =
          (esym - esymstart) / bfd_coff_symesz (abfd);

        amt = strlen (symname) + 1;
        newname = (char *) bfd_alloc (abfd, amt);
        if (newname == NULL)
          abort ();

        strcpy (newname, symname);
        coff_section_data (abfd, section)->comdat->name
          = newname;
          }

          goto breakloop;
        }
    }

      esym += (isym.n_numaux + 1) * bfd_coff_symesz (abfd);
    }

 breakloop:
  return sec_flags;
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
           asection *section,
           flagword *flags_ptr)
{
  struct internal_scnhdr *internal_s = (struct internal_scnhdr *) hdr;
  unsigned long styp_flags = internal_s->s_flags;
  flagword sec_flags;
  bfd_boolean result = TRUE;
  bfd_boolean is_dbg = FALSE;

  if (CONST_STRNEQ (name, DOT_DEBUG)
      || CONST_STRNEQ (name, DOT_ZDEBUG)
#ifdef COFF_LONG_SECTION_NAMES
      || CONST_STRNEQ (name, GNU_LINKONCE_WI)
      || CONST_STRNEQ (name, GNU_LINKONCE_WT)
#endif
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
      /* COMDAT gets very special treatment.  */
      sec_flags = handle_COMDAT (abfd, sec_flags, hdr, name, section);
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

#if defined (COFF_LONG_SECTION_NAMES) && defined (COFF_SUPPORT_GNU_LINKONCE)
  /* As a GNU extension, if the name begins with .gnu.linkonce, we
     only link a single copy of the section.  This is used to support
     g++.  g++ will emit each template expansion in its own section.
     The symbols will be defined as weak, so that multiple definitions
     are permitted.  The GNU linker extension is to actually discard
     all but one of the sections.  */
  if (CONST_STRNEQ (name, ".gnu.linkonce"))
    sec_flags |= SEC_LINK_ONCE | SEC_LINK_DUPLICATES_DISCARD;
#endif

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

#ifdef RS6000COFF_C
  if (bfd_xcoff_text_align_power (abfd) != 0
      && strcmp (bfd_get_section_name (abfd, section), ".text") == 0)
    section->alignment_power = bfd_xcoff_text_align_power (abfd);
  else if (bfd_xcoff_data_align_power (abfd) != 0
      && strcmp (bfd_get_section_name (abfd, section), ".data") == 0)
    section->alignment_power = bfd_xcoff_data_align_power (abfd);
  else
    {
      int i;

      for (i = 0; i < XCOFF_DWSECT_NBR_NAMES; i++)
	if (strcmp (bfd_get_section_name (abfd, section),
		    xcoff_dwsect_names[i].name) == 0)
	  {
	    section->alignment_power = 0;
	    sclass = C_DWARF;
	    break;
	  }
    }
#endif

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

#ifdef COFF_ALIGN_IN_SECTION_HEADER

/* Set the alignment of a BFD section.  */

static void
coff_set_alignment_hook (bfd * abfd ATTRIBUTE_UNUSED,
			 asection * section,
			 void * scnhdr)
{
  struct internal_scnhdr *hdr = (struct internal_scnhdr *) scnhdr;
  unsigned int i;

#ifdef COFF_DECODE_ALIGNMENT
  i = COFF_DECODE_ALIGNMENT(hdr->s_flags);
#endif
  section->alignment_power = i;

#ifdef coff_set_section_load_page
  coff_set_section_load_page (section, hdr->s_page);
#endif
}

#else /* ! COFF_ALIGN_IN_SECTION_HEADER */
#ifdef COFF_WITH_PE

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

  /* Check for extended relocs.  */
  if (hdr->s_flags & IMAGE_SCN_LNK_NRELOC_OVFL)
    {
      struct external_reloc dst;
      struct internal_reloc n;
      file_ptr oldpos = bfd_tell (abfd);
      bfd_size_type relsz = bfd_coff_relsz (abfd);

      if (bfd_seek (abfd, (file_ptr) hdr->s_relptr, 0) != 0)
	return;
      if (bfd_bread (& dst, relsz, abfd) != relsz)
	return;

      coff_swap_reloc_in (abfd, &dst, &n);
      if (bfd_seek (abfd, oldpos, 0) != 0)
	return;
      section->reloc_count = hdr->s_nreloc = n.r_vaddr - 1;
      section->rel_filepos += relsz;
    }
  else if (hdr->s_nreloc == 0xffff)
    _bfd_error_handler
      (_("%pB: warning: claims to have 0xffff relocs, without overflow"),
       abfd);
}
#undef ALIGN_SET
#undef ELIFALIGN_SET

#endif /* ! COFF_WITH_PE */
#endif /* ! COFF_ALIGN_IN_SECTION_HEADER */

#ifndef coff_mkobject

static bfd_boolean
coff_mkobject (bfd * abfd)
{
  coff_data_type *coff;
  bfd_size_type amt = sizeof (coff_data_type);

  abfd->tdata.coff_obj_data = bfd_zalloc (abfd, amt);
  if (abfd->tdata.coff_obj_data == NULL)
    return FALSE;
  coff = coff_data (abfd);
  coff->symbols = NULL;
  coff->conversion_table = NULL;
  coff->raw_syments = NULL;
  coff->relocbase = 0;
  coff->local_toc_sym_map = 0;

/*  make_abs_section(abfd);*/

  return TRUE;
}
#endif

/* Create the COFF backend specific information.  */

#ifndef coff_mkobject_hook
static void *
coff_mkobject_hook (bfd * abfd,
		    void * filehdr,
		    void * aouthdr ATTRIBUTE_UNUSED)
{
  struct internal_filehdr *internal_f = (struct internal_filehdr *) filehdr;
  coff_data_type *coff;

  if (! coff_mkobject (abfd))
    return NULL;

  coff = coff_data (abfd);

  coff->sym_filepos = internal_f->f_symptr;

  /* These members communicate important constants about the symbol
     table to GDB's symbol-reading code.  These `constants'
     unfortunately vary among coff implementations...  */
  coff->local_n_btmask = N_BTMASK;
  coff->local_n_btshft = N_BTSHFT;
  coff->local_n_tmask = N_TMASK;
  coff->local_n_tshift = N_TSHIFT;
  coff->local_symesz = bfd_coff_symesz (abfd);
  coff->local_auxesz = bfd_coff_auxesz (abfd);
  coff->local_linesz = bfd_coff_linesz (abfd);

  coff->timestamp = internal_f->f_timdat;

  obj_raw_syment_count (abfd) =
    obj_conv_table_size (abfd) =
      internal_f->f_nsyms;

#ifdef RS6000COFF_C
  if ((internal_f->f_flags & F_SHROBJ) != 0)
    abfd->flags |= DYNAMIC;
  if (aouthdr != NULL && internal_f->f_opthdr >= bfd_coff_aoutsz (abfd))
    {
      struct internal_aouthdr *internal_a =
	(struct internal_aouthdr *) aouthdr;
      struct xcoff_tdata *xcoff;

      xcoff = xcoff_data (abfd);
# ifdef U803XTOCMAGIC
      xcoff->xcoff64 = internal_f->f_magic == U803XTOCMAGIC;
# else
      xcoff->xcoff64 = 0;
# endif
      xcoff->full_aouthdr = TRUE;
      xcoff->toc = internal_a->o_toc;
      xcoff->sntoc = internal_a->o_sntoc;
      xcoff->snentry = internal_a->o_snentry;
      bfd_xcoff_text_align_power (abfd) = internal_a->o_algntext;
      bfd_xcoff_data_align_power (abfd) = internal_a->o_algndata;
      xcoff->modtype = internal_a->o_modtype;
      xcoff->cputype = internal_a->o_cputype;
      xcoff->maxdata = internal_a->o_maxdata;
      xcoff->maxstack = internal_a->o_maxstack;
    }
#endif

#ifdef ARM
  /* Set the flags field from the COFF header read in.  */
  if (! _bfd_coff_arm_set_private_flags (abfd, internal_f->f_flags))
    coff->flags = 0;
#endif

#ifdef COFF_WITH_PE
  /* FIXME: I'm not sure this is ever executed, since peicode.h
     defines coff_mkobject_hook.  */
  if ((internal_f->f_flags & IMAGE_FILE_DEBUG_STRIPPED) == 0)
    abfd->flags |= HAS_DEBUG;
#endif

  if ((internal_f->f_flags & F_GO32STUB) != 0)
    {
      coff->go32stub = (char *) bfd_alloc (abfd, (bfd_size_type) GO32_STUBSIZE);
      if (coff->go32stub == NULL)
	return NULL;
    }
  if (coff->go32stub != NULL)
    memcpy (coff->go32stub, internal_f->go32stub, GO32_STUBSIZE);

  return coff;
}
#endif

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
#ifdef PPCMAGIC
    case PPCMAGIC:
      arch = bfd_arch_powerpc;
      break;
#endif
#ifdef I386MAGIC
    case I386MAGIC:
    case I386PTXMAGIC:
    case I386AIXMAGIC:		/* Danbury PS/2 AIX C Compiler.  */
    case LYNXCOFFMAGIC:
      arch = bfd_arch_i386;
      break;
#endif
#ifdef AMD64MAGIC
    case AMD64MAGIC:
      arch = bfd_arch_i386;
      machine = bfd_mach_x86_64;
      break;
#endif
#ifdef IA64MAGIC
    case IA64MAGIC:
      arch = bfd_arch_ia64;
      break;
#endif
#ifdef ARMMAGIC
    case ARMMAGIC:
    case ARMPEMAGIC:
    case THUMBPEMAGIC:
      arch = bfd_arch_arm;
      machine = bfd_arm_get_mach_from_notes (abfd, ARM_NOTE_SECTION);
      if (machine == bfd_mach_arm_unknown)
	{
	  switch (internal_f->f_flags & F_ARM_ARCHITECTURE_MASK)
	    {
	    case F_ARM_2:  machine = bfd_mach_arm_2;  break;
	    case F_ARM_2a: machine = bfd_mach_arm_2a; break;
	    case F_ARM_3:  machine = bfd_mach_arm_3;  break;
	    default:
	    case F_ARM_3M: machine = bfd_mach_arm_3M; break;
	    case F_ARM_4:  machine = bfd_mach_arm_4;  break;
	    case F_ARM_4T: machine = bfd_mach_arm_4T; break;
	      /* The COFF header does not have enough bits available
		 to cover all the different ARM architectures.  So
		 we interpret F_ARM_5, the highest flag value to mean
		 "the highest ARM architecture known to BFD" which is
		 currently the XScale.  */
	    case F_ARM_5:  machine = bfd_mach_arm_XScale;  break;
	    }
	}
      break;
#endif
#ifdef Z80MAGIC
    case Z80MAGIC:
      arch = bfd_arch_z80;
      switch (internal_f->f_flags & F_MACHMASK)
	{
	case 0:
	case bfd_mach_z80strict << 12:
	case bfd_mach_z80 << 12:
	case bfd_mach_z80full << 12:
	case bfd_mach_r800 << 12:
	  machine = ((unsigned)internal_f->f_flags & F_MACHMASK) >> 12;
	  break;
	default:
	  return FALSE;
	}
      break;
#endif
#ifdef Z8KMAGIC
    case Z8KMAGIC:
      arch = bfd_arch_z8k;
      switch (internal_f->f_flags & F_MACHMASK)
	{
	case F_Z8001:
	  machine = bfd_mach_z8001;
	  break;
	case F_Z8002:
	  machine = bfd_mach_z8002;
	  break;
	default:
	  return FALSE;
	}
      break;
#endif

#ifdef RS6000COFF_C
#ifdef XCOFF64
    case U64_TOCMAGIC:
    case U803XTOCMAGIC:
#else
    case U802ROMAGIC:
    case U802WRMAGIC:
    case U802TOCMAGIC:
#endif
      {
	int cputype;

	if (xcoff_data (abfd)->cputype != -1)
	  cputype = xcoff_data (abfd)->cputype & 0xff;
	else
	  {
	    /* We did not get a value from the a.out header.  If the
	       file has not been stripped, we may be able to get the
	       architecture information from the first symbol, if it
	       is a .file symbol.  */
	    if (obj_raw_syment_count (abfd) == 0)
	      cputype = 0;
	    else
	      {
		bfd_byte *buf;
		struct internal_syment sym;
		bfd_size_type amt = bfd_coff_symesz (abfd);

		buf = bfd_malloc (amt);
		if (buf == NULL)
		  return FALSE;
		if (bfd_seek (abfd, obj_sym_filepos (abfd), SEEK_SET) != 0
		    || bfd_bread (buf, amt, abfd) != amt)
		  {
		    free (buf);
		    return FALSE;
		  }
		bfd_coff_swap_sym_in (abfd, buf, & sym);
		if (sym.n_sclass == C_FILE)
		  cputype = sym.n_type & 0xff;
		else
		  cputype = 0;
		free (buf);
	      }
	  }

	/* FIXME: We don't handle all cases here.  */
	switch (cputype)
	  {
	  default:
	  case 0:
	    arch = bfd_xcoff_architecture (abfd);
	    machine = bfd_xcoff_machine (abfd);
	    break;

	  case 1:
	    arch = bfd_arch_powerpc;
	    machine = bfd_mach_ppc_601;
	    break;
	  case 2: /* 64 bit PowerPC */
	    arch = bfd_arch_powerpc;
	    machine = bfd_mach_ppc_620;
	    break;
	  case 3:
	    arch = bfd_arch_powerpc;
	    machine = bfd_mach_ppc;
	    break;
	  case 4:
	    arch = bfd_arch_rs6000;
	    machine = bfd_mach_rs6k;
	    break;
	  }
      }
      break;
#endif

#ifdef SH_ARCH_MAGIC_BIG
    case SH_ARCH_MAGIC_BIG:
    case SH_ARCH_MAGIC_LITTLE:
#ifdef COFF_WITH_PE
    case SH_ARCH_MAGIC_WINCE:
#endif
      arch = bfd_arch_sh;
      break;
#endif

#ifdef MIPS_ARCH_MAGIC_WINCE
    case MIPS_ARCH_MAGIC_WINCE:
      arch = bfd_arch_mips;
      break;
#endif

#ifdef SPARCMAGIC
    case SPARCMAGIC:
#ifdef LYNXCOFFMAGIC
    case LYNXCOFFMAGIC:
#endif
      arch = bfd_arch_sparc;
      break;
#endif

#ifdef TIC30MAGIC
    case TIC30MAGIC:
      arch = bfd_arch_tic30;
      break;
#endif

#ifdef TICOFF0MAGIC
#ifdef TICOFF_TARGET_ARCH
      /* This TI COFF section should be used by all new TI COFF v0 targets.  */
    case TICOFF0MAGIC:
      arch = TICOFF_TARGET_ARCH;
      machine = TICOFF_TARGET_MACHINE_GET (internal_f->f_flags);
      break;
#endif
#endif

#ifdef TICOFF1MAGIC
      /* This TI COFF section should be used by all new TI COFF v1/2 targets.  */
      /* TI COFF1 and COFF2 use the target_id field to specify which arch.  */
    case TICOFF1MAGIC:
    case TICOFF2MAGIC:
      switch (internal_f->f_target_id)
	{
#ifdef TI_TARGET_ID
	case TI_TARGET_ID:
	  arch = TICOFF_TARGET_ARCH;
	  machine = TICOFF_TARGET_MACHINE_GET (internal_f->f_flags);
	  break;
#endif
	default:
	  arch = bfd_arch_obscure;
	  _bfd_error_handler
	    (_("unrecognized TI COFF target id '0x%x'"),
	     internal_f->f_target_id);
	  break;
	}
      break;
#endif

#ifdef TIC80_ARCH_MAGIC
    case TIC80_ARCH_MAGIC:
      arch = bfd_arch_tic80;
      break;
#endif

#ifdef MCOREMAGIC
    case MCOREMAGIC:
      arch = bfd_arch_mcore;
      break;
#endif

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
#ifdef SYMNAME_IN_DEBUG
  return SYMNAME_IN_DEBUG (sym) != 0;
#else
  return FALSE;
#endif
}

#ifdef RS6000COFF_C

#ifdef XCOFF64
#define FORCE_SYMNAMES_IN_STRINGS
#endif

/* Handle the csect auxent of a C_EXT, C_AIX_WEAKEXT or C_HIDEXT symbol.  */

static bfd_boolean
coff_pointerize_aux_hook (bfd *abfd ATTRIBUTE_UNUSED,
			  combined_entry_type *table_base,
			  combined_entry_type *symbol,
			  unsigned int indaux,
			  combined_entry_type *aux)
{
  BFD_ASSERT (symbol->is_sym);
  int n_sclass = symbol->u.syment.n_sclass;

  if (CSECT_SYM_P (n_sclass)
      && indaux + 1 == symbol->u.syment.n_numaux)
    {
      BFD_ASSERT (! aux->is_sym);
      if (SMTYP_SMTYP (aux->u.auxent.x_csect.x_smtyp) == XTY_LD)
	{
	  aux->u.auxent.x_csect.x_scnlen.p =
	    table_base + aux->u.auxent.x_csect.x_scnlen.l;
	  aux->fix_scnlen = 1;
	}

      /* Return TRUE to indicate that the caller should not do any
	 further work on this auxent.  */
      return TRUE;
    }

  /* Return FALSE to indicate that this auxent should be handled by
     the caller.  */
  return FALSE;
}

#else
#define coff_pointerize_aux_hook 0
#endif /* ! RS6000COFF_C */

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

#ifdef TARG_AUX


/* AUX's ld wants relocations to be sorted.  */
static int
compare_arelent_ptr (const void * x, const void * y)
{
  const arelent **a = (const arelent **) x;
  const arelent **b = (const arelent **) y;
  bfd_size_type aadr = (*a)->address;
  bfd_size_type badr = (*b)->address;

  return (aadr < badr ? -1 : badr < aadr ? 1 : 0);
}

#endif /* TARG_AUX */


/* Set flags and magic number of a coff file from architecture and machine
   type.  Result is TRUE if we can represent the arch&type, FALSE if not.  */

static bfd_boolean
coff_set_flags (bfd * abfd,
		unsigned int *magicp ATTRIBUTE_UNUSED,
		unsigned short *flagsp ATTRIBUTE_UNUSED)
{
  switch (bfd_get_arch (abfd))
    {
#ifdef Z80MAGIC
    case bfd_arch_z80:
      *magicp = Z80MAGIC;
      switch (bfd_get_mach (abfd))
	{
	case 0:
	case bfd_mach_z80strict:
	case bfd_mach_z80:
	case bfd_mach_z80full:
	case bfd_mach_r800:
	  *flagsp = bfd_get_mach (abfd) << 12;
	  break;
	default:
	  return FALSE;
	}
      return TRUE;
#endif

#ifdef Z8KMAGIC
    case bfd_arch_z8k:
      *magicp = Z8KMAGIC;

      switch (bfd_get_mach (abfd))
	{
	case bfd_mach_z8001: *flagsp = F_Z8001;	break;
	case bfd_mach_z8002: *flagsp = F_Z8002;	break;
	default:	     return FALSE;
	}
      return TRUE;
#endif

#ifdef TIC30MAGIC
    case bfd_arch_tic30:
      *magicp = TIC30MAGIC;
      return TRUE;
#endif

#ifdef TICOFF_DEFAULT_MAGIC
    case TICOFF_TARGET_ARCH:
      /* If there's no indication of which version we want, use the default.  */
      if (!abfd->xvec )
	*magicp = TICOFF_DEFAULT_MAGIC;
      else
	{
	  /* We may want to output in a different COFF version.  */
	  switch (abfd->xvec->name[4])
	    {
	    case '0':
	      *magicp = TICOFF0MAGIC;
	      break;
	    case '1':
	      *magicp = TICOFF1MAGIC;
	      break;
	    case '2':
	      *magicp = TICOFF2MAGIC;
	      break;
	    default:
	      return FALSE;
	    }
	}
      TICOFF_TARGET_MACHINE_SET (flagsp, bfd_get_mach (abfd));
      return TRUE;
#endif

#ifdef TIC80_ARCH_MAGIC
    case bfd_arch_tic80:
      *magicp = TIC80_ARCH_MAGIC;
      return TRUE;
#endif

#ifdef ARMMAGIC
    case bfd_arch_arm:
#ifdef ARM_WINCE
      * magicp = ARMPEMAGIC;
#else
      * magicp = ARMMAGIC;
#endif
      * flagsp = 0;
      if (APCS_SET (abfd))
	{
	  if (APCS_26_FLAG (abfd))
	    * flagsp |= F_APCS26;

	  if (APCS_FLOAT_FLAG (abfd))
	    * flagsp |= F_APCS_FLOAT;

	  if (PIC_FLAG (abfd))
	    * flagsp |= F_PIC;
	}
      if (INTERWORK_SET (abfd) && INTERWORK_FLAG (abfd))
	* flagsp |= F_INTERWORK;
      switch (bfd_get_mach (abfd))
	{
	case bfd_mach_arm_2:  * flagsp |= F_ARM_2;  break;
	case bfd_mach_arm_2a: * flagsp |= F_ARM_2a; break;
	case bfd_mach_arm_3:  * flagsp |= F_ARM_3;  break;
	case bfd_mach_arm_3M: * flagsp |= F_ARM_3M; break;
	case bfd_mach_arm_4:  * flagsp |= F_ARM_4;  break;
	case bfd_mach_arm_4T: * flagsp |= F_ARM_4T; break;
	case bfd_mach_arm_5:  * flagsp |= F_ARM_5;  break;
	  /* FIXME: we do not have F_ARM vaues greater than F_ARM_5.
	     See also the comment in coff_set_arch_mach_hook().  */
	case bfd_mach_arm_5T: * flagsp |= F_ARM_5;  break;
	case bfd_mach_arm_5TE: * flagsp |= F_ARM_5; break;
	case bfd_mach_arm_XScale: * flagsp |= F_ARM_5; break;
	}
      return TRUE;
#endif

#ifdef PPCMAGIC
    case bfd_arch_powerpc:
      *magicp = PPCMAGIC;
      return TRUE;
#endif

#if defined(I386MAGIC) || defined(AMD64MAGIC)
    case bfd_arch_i386:
#if defined(I386MAGIC)
      *magicp = I386MAGIC;
#endif
#if defined LYNXOS
      /* Just overwrite the usual value if we're doing Lynx.  */
      *magicp = LYNXCOFFMAGIC;
#endif
#if defined AMD64MAGIC
      *magicp = AMD64MAGIC;
#endif
      return TRUE;
#endif

#ifdef IA64MAGIC
    case bfd_arch_ia64:
      *magicp = IA64MAGIC;
      return TRUE;
#endif

#ifdef SH_ARCH_MAGIC_BIG
    case bfd_arch_sh:
#ifdef COFF_IMAGE_WITH_PE
      *magicp = SH_ARCH_MAGIC_WINCE;
#else
      if (bfd_big_endian (abfd))
	*magicp = SH_ARCH_MAGIC_BIG;
      else
	*magicp = SH_ARCH_MAGIC_LITTLE;
#endif
      return TRUE;
#endif

#ifdef MIPS_ARCH_MAGIC_WINCE
    case bfd_arch_mips:
      *magicp = MIPS_ARCH_MAGIC_WINCE;
      return TRUE;
#endif

#ifdef SPARCMAGIC
    case bfd_arch_sparc:
      *magicp = SPARCMAGIC;
#ifdef LYNXOS
      /* Just overwrite the usual value if we're doing Lynx.  */
      *magicp = LYNXCOFFMAGIC;
#endif
      return TRUE;
#endif

#ifdef RS6000COFF_C
    case bfd_arch_rs6000:
#ifndef PPCMAGIC
    case bfd_arch_powerpc:
#endif
      BFD_ASSERT (bfd_get_flavour (abfd) == bfd_target_xcoff_flavour);
      *magicp = bfd_xcoff_magic_number (abfd);
      return TRUE;
#endif

#ifdef MCOREMAGIC
    case bfd_arch_mcore:
      * magicp = MCOREMAGIC;
      return TRUE;
#endif

    default:			/* Unknown architecture.  */
      /* Fall through to "return FALSE" below, to avoid
	 "statement never reached" errors on the one below.  */
      break;
    }

  return FALSE;
}

static bfd_boolean
coff_set_arch_mach (bfd * abfd,
		    enum bfd_architecture arch,
		    unsigned long machine)
{
  unsigned dummy1;
  unsigned short dummy2;

  if (! bfd_default_set_arch_mach (abfd, arch, machine))
    return FALSE;

  if (arch != bfd_arch_unknown
      && ! coff_set_flags (abfd, &dummy1, &dummy2))
    return FALSE;		/* We can't represent this type.  */

  return TRUE;			/* We're easy...  */
}

#ifdef COFF_IMAGE_WITH_PE

/* This is used to sort sections by VMA, as required by PE image
   files.  */

static int
sort_by_secaddr (const void * arg1, const void * arg2)
{
  const asection *a = *(const asection **) arg1;
  const asection *b = *(const asection **) arg2;

  if (a->vma < b->vma)
    return -1;
  else if (a->vma > b->vma)
    return 1;

  return 0;
}

#endif /* COFF_IMAGE_WITH_PE */

/* Calculate the file position for each section.  */

#define ALIGN_SECTIONS_IN_FILE
#if defined(TIC80COFF) || defined(TICOFF)
#undef ALIGN_SECTIONS_IN_FILE
#endif

static bfd_boolean
coff_compute_section_file_positions (bfd * abfd)
{
  asection *current;
  file_ptr sofar = bfd_coff_filhsz (abfd);
  bfd_boolean align_adjust;
  unsigned int target_index;
#ifdef ALIGN_SECTIONS_IN_FILE
  asection *previous = NULL;
  file_ptr old_sofar;
#endif

#ifdef COFF_IMAGE_WITH_PE
  int page_size;

  if (coff_data (abfd)->link_info
      || (pe_data (abfd) && pe_data (abfd)->pe_opthdr.FileAlignment))
    {
      page_size = pe_data (abfd)->pe_opthdr.FileAlignment;

      /* If no file alignment has been set, default to one.
     This repairs 'ld -r' for arm-wince-pe target.  */
      if (page_size == 0)
    page_size = 1;

      /* PR 17512: file: 0ac816d3.  */
      if (page_size < 0)
    {
      bfd_set_error (bfd_error_file_too_big);
      _bfd_error_handler
        /* xgettext:c-format */
        (_("%pB: page size is too large (0x%x)"), abfd, page_size);
      return FALSE;
    }
    }
  else
    page_size = PE_DEF_FILE_ALIGNMENT;
#else
#ifdef COFF_PAGE_SIZE
  int page_size = COFF_PAGE_SIZE;
#endif
#endif

#ifdef RS6000COFF_C
  /* On XCOFF, if we have symbols, set up the .debug section.  */
  if (bfd_get_symcount (abfd) > 0)
    {
      bfd_size_type sz;
      bfd_size_type i, symcount;
      asymbol **symp;

      sz = 0;
      symcount = bfd_get_symcount (abfd);
      for (symp = abfd->outsymbols, i = 0; i < symcount; symp++, i++)
    {
      coff_symbol_type *cf;

      cf = coff_symbol_from (*symp);
      if (cf != NULL
          && cf->native != NULL
          && cf->native->is_sym
          && SYMNAME_IN_DEBUG (&cf->native->u.syment))
        {
          size_t len;

          len = strlen (bfd_asymbol_name (*symp));
          if (len > SYMNMLEN || bfd_coff_force_symnames_in_strings (abfd))
        sz += len + 1 + bfd_coff_debug_string_prefix_length (abfd);
        }
    }
      if (sz > 0)
    {
      asection *dsec;

      dsec = bfd_make_section_old_way (abfd, DOT_DEBUG);
      if (dsec == NULL)
        abort ();
      dsec->size = sz;
      dsec->flags |= SEC_HAS_CONTENTS;
    }
    }
#endif

  if (bfd_get_start_address (abfd))
    /*  A start address may have been added to the original file. In this
    case it will need an optional header to record it.  */
    abfd->flags |= EXEC_P;

  if (abfd->flags & EXEC_P)
    sofar += bfd_coff_aoutsz (abfd);
#ifdef RS6000COFF_C
  else if (xcoff_data (abfd)->full_aouthdr)
    sofar += bfd_coff_aoutsz (abfd);
  else
    sofar += SMALL_AOUTSZ;
#endif

  sofar += abfd->section_count * bfd_coff_scnhsz (abfd);

#ifdef RS6000COFF_C
  /* XCOFF handles overflows in the reloc and line number count fields
     by allocating a new section header to hold the correct counts.  */
  for (current = abfd->sections; current != NULL; current = current->next)
    if (current->reloc_count >= 0xffff || current->lineno_count >= 0xffff)
      sofar += bfd_coff_scnhsz (abfd);
#endif

#ifdef COFF_IMAGE_WITH_PE
  {
    /* PE requires the sections to be in memory order when listed in
       the section headers.  It also does not like empty loadable
       sections.  The sections apparently do not have to be in the
       right order in the image file itself, but we do need to get the
       target_index values right.  */

    unsigned int count;
    asection **section_list;
    unsigned int i;
    bfd_size_type amt;

#ifdef COFF_PAGE_SIZE
    /* Clear D_PAGED if section alignment is smaller than
       COFF_PAGE_SIZE.  */
   if (pe_data (abfd)->pe_opthdr.SectionAlignment < COFF_PAGE_SIZE)
     abfd->flags &= ~D_PAGED;
#endif

    count = 0;
    for (current = abfd->sections; current != NULL; current = current->next)
      ++count;

    /* We allocate an extra cell to simplify the final loop.  */
    amt = sizeof (struct asection *) * (count + 1);
    section_list = (asection **) bfd_malloc (amt);
    if (section_list == NULL)
      return FALSE;

    i = 0;
    for (current = abfd->sections; current != NULL; current = current->next)
      {
    section_list[i] = current;
    ++i;
      }
    section_list[i] = NULL;

    qsort (section_list, count, sizeof (asection *), sort_by_secaddr);

    /* Rethread the linked list into sorted order; at the same time,
       assign target_index values.  */
    target_index = 1;
    abfd->sections = NULL;
    abfd->section_last = NULL;
    for (i = 0; i < count; i++)
      {
    current = section_list[i];
    bfd_section_list_append (abfd, current);

    /* Later, if the section has zero size, we'll be throwing it
       away, so we don't want to number it now.  Note that having
       a zero size and having real contents are different
       concepts: .bss has no contents, but (usually) non-zero
       size.  */
    if (current->size == 0)
      {
        /* Discard.  However, it still might have (valid) symbols
           in it, so arbitrarily set it to section 1 (indexing is
           1-based here; usually .text).  __end__ and other
           contents of .endsection really have this happen.
           FIXME: This seems somewhat dubious.  */
        current->target_index = 1;
      }
    else
      current->target_index = target_index++;
      }

    free (section_list);
  }
#else /* ! COFF_IMAGE_WITH_PE */
  {
    /* Set the target_index field.  */
    target_index = 1;
    for (current = abfd->sections; current != NULL; current = current->next)
      current->target_index = target_index++;
  }
#endif /* ! COFF_IMAGE_WITH_PE */

  if (target_index >= bfd_coff_max_nscns (abfd))
    {
      bfd_set_error (bfd_error_file_too_big);
      _bfd_error_handler
    /* xgettext:c-format */
    (_("%pB: too many sections (%d)"), abfd, target_index);
      return FALSE;
    }

  align_adjust = FALSE;
  for (current = abfd->sections;
       current != NULL;
       current = current->next)
    {
#ifdef COFF_IMAGE_WITH_PE
      /* With PE we have to pad each section to be a multiple of its
     page size too, and remember both sizes.  */
      if (coff_section_data (abfd, current) == NULL)
    {
      bfd_size_type amt = sizeof (struct coff_section_tdata);

      current->used_by_bfd = bfd_zalloc (abfd, amt);
      if (current->used_by_bfd == NULL)
        return FALSE;
    }
      if (pei_section_data (abfd, current) == NULL)
    {
      bfd_size_type amt = sizeof (struct pei_section_tdata);

      coff_section_data (abfd, current)->tdata = bfd_zalloc (abfd, amt);
      if (coff_section_data (abfd, current)->tdata == NULL)
        return FALSE;
    }
      if (pei_section_data (abfd, current)->virt_size == 0)
    pei_section_data (abfd, current)->virt_size = current->size;
#endif

      /* Only deal with sections which have contents.  */
      if (!(current->flags & SEC_HAS_CONTENTS))
    continue;

      current->rawsize = current->size;

#ifdef COFF_IMAGE_WITH_PE
      /* Make sure we skip empty sections in a PE image.  */
      if (current->size == 0)
    continue;
#endif

      /* Align the sections in the file to the same boundary on
     which they are aligned in virtual memory.  */
#ifdef ALIGN_SECTIONS_IN_FILE
      if ((abfd->flags & EXEC_P) != 0)
    {
      /* Make sure this section is aligned on the right boundary - by
         padding the previous section up if necessary.  */
      old_sofar = sofar;

      sofar = BFD_ALIGN (sofar, 1 << current->alignment_power);

#ifdef RS6000COFF_C
      /* Make sure the file offset and the vma of .text/.data are at the
         same page offset, so that the file can be mmap'ed without being
         relocated.  Failing that, AIX is able to load and execute the
         program, but it will be silently relocated (possible as
         executables are PIE).  But the relocation is slightly costly and
         complexify the use of addr2line or gdb.  So better to avoid it,
         like does the native linker.  Usually gnu ld makes sure that
         the vma of .text is the file offset so this issue shouldn't
         appear unless you are stripping such an executable.

         AIX loader checks the text section alignment of (vma - filepos),
         and the native linker doesn't try to align the text sections.
         For example:

         0 .text	     000054cc  10000128	 10000128  00000128  2**5
                 CONTENTS, ALLOC, LOAD, CODE
      */

      if (!strcmp (current->name, _TEXT)
          || !strcmp (current->name, _DATA))
        {
          bfd_vma align = 4096;
          bfd_vma sofar_off = sofar % align;
          bfd_vma vma_off = current->vma % align;

          if (vma_off > sofar_off)
        sofar += vma_off - sofar_off;
          else if (vma_off < sofar_off)
        sofar += align + vma_off - sofar_off;
        }
#endif
      if (previous != NULL)
        previous->size += sofar - old_sofar;
    }

#endif

      /* In demand paged files the low order bits of the file offset
     must match the low order bits of the virtual address.  */
#ifdef COFF_PAGE_SIZE
      if ((abfd->flags & D_PAGED) != 0
      && (current->flags & SEC_ALLOC) != 0)
    sofar += (current->vma - (bfd_vma) sofar) % page_size;
#endif
      current->filepos = sofar;

#ifdef COFF_IMAGE_WITH_PE
      /* Set the padded size.  */
      current->size = (current->size + page_size - 1) & -page_size;
#endif

      sofar += current->size;

#ifdef ALIGN_SECTIONS_IN_FILE
      /* Make sure that this section is of the right size too.  */
      if ((abfd->flags & EXEC_P) == 0)
    {
      bfd_size_type old_size;

      old_size = current->size;
      current->size = BFD_ALIGN (current->size,
                     1 << current->alignment_power);
      align_adjust = current->size != old_size;
      sofar += current->size - old_size;
    }
      else
    {
      old_sofar = sofar;
      sofar = BFD_ALIGN (sofar, 1 << current->alignment_power);
      align_adjust = sofar != old_sofar;
      current->size += sofar - old_sofar;
    }
#endif

#ifdef COFF_IMAGE_WITH_PE
      /* For PE we need to make sure we pad out to the aligned
     size, in case the caller only writes out data to the
     unaligned size.  */
      if (pei_section_data (abfd, current)->virt_size < current->size)
    align_adjust = TRUE;
#endif

#ifdef _LIB
      /* Force .lib sections to start at zero.  The vma is then
     incremented in coff_set_section_contents.  This is right for
     SVR3.2.  */
      if (strcmp (current->name, _LIB) == 0)
    (void) bfd_set_section_vma (abfd, current, 0);
#endif

#ifdef ALIGN_SECTIONS_IN_FILE
      previous = current;
#endif
    }

  /* It is now safe to write to the output file.  If we needed an
     alignment adjustment for the last section, then make sure that
     there is a byte at offset sofar.  If there are no symbols and no
     relocs, then nothing follows the last section.  If we don't force
     the last byte out, then the file may appear to be truncated.  */
  if (align_adjust)
    {
      bfd_byte b;

      b = 0;
      if (bfd_seek (abfd, sofar - 1, SEEK_SET) != 0
      || bfd_bwrite (&b, (bfd_size_type) 1, abfd) != 1)
    return FALSE;
    }

  /* Make sure the relocations are aligned.  We don't need to make
     sure that this byte exists, because it will only matter if there
     really are relocs.  */
  sofar = BFD_ALIGN (sofar, 1 << COFF_DEFAULT_SECTION_ALIGNMENT_POWER);

  obj_relocbase (abfd) = sofar;
  abfd->output_has_begun = TRUE;

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
#if defined ARM
	    case C_THUMBEXT:
	    case C_THUMBEXTFUNC:
#endif
#ifdef RS6000COFF_C
	    case C_HIDEXT:
#if ! defined _AIX52 && ! defined AIX_WEAK_SUPPORT
	    case C_AIX_WEAKEXT:
#endif
#endif
#ifdef C_SYSTEM
	    case C_SYSTEM:	/* System Wide variable.  */
#endif
#ifdef COFF_WITH_PE
	    /* In PE, 0x68 (104) denotes a section symbol.  */
	    case C_SECTION:
	    /* In PE, 0x69 (105) denotes a weak external symbol.  */
	    case C_NT_WEAK:
#endif
	      switch (coff_classify_symbol (abfd, &src->u.syment))
		{
		case COFF_SYMBOL_GLOBAL:
		  dst->symbol.flags = BSF_EXPORT | BSF_GLOBAL;
#if defined COFF_WITH_PE
		  /* PE sets the symbol to a value relative to the
		     start of the section.  */
		  dst->symbol.value = src->u.syment.n_value;
#else
		  dst->symbol.value = (src->u.syment.n_value
				       - dst->symbol.section->vma);
#endif
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
#if defined COFF_WITH_PE
		  /* PE sets the symbol to a value relative to the
		     start of the section.  */
		  dst->symbol.value = src->u.syment.n_value;
#else
		  dst->symbol.value = (src->u.syment.n_value
				       - dst->symbol.section->vma);
#endif
		  if (ISFCN ((src->u.syment.n_type)))
		    dst->symbol.flags |= BSF_NOT_AT_END | BSF_FUNCTION;
		  break;
		}

#ifdef RS6000COFF_C
	      /* A symbol with a csect entry should not go at the end.  */
	      if (src->u.syment.n_numaux > 0)
		dst->symbol.flags |= BSF_NOT_AT_END;
#endif

#ifdef COFF_WITH_PE
	      if (src->u.syment.n_sclass == C_NT_WEAK)
		dst->symbol.flags |= BSF_WEAK;

	      if (src->u.syment.n_sclass == C_SECTION
		  && src->u.syment.n_scnum > 0)
		dst->symbol.flags = BSF_LOCAL;
#endif
	      if (src->u.syment.n_sclass == C_WEAKEXT
#ifdef RS6000COFF_C
		  || src->u.syment.n_sclass == C_AIX_WEAKEXT
#endif
		  )
		dst->symbol.flags |= BSF_WEAK;

	      break;

	    case C_STAT:	 /* Static.  */
#if defined ARM
	    case C_THUMBSTAT:    /* Thumb static.  */
	    case C_THUMBLABEL:   /* Thumb label.  */
	    case C_THUMBSTATFUNC:/* Thumb static function.  */
#endif
#ifdef RS6000COFF_C
	    case C_DWARF:	 /* A label in a dwarf section.  */
	    case C_INFO:	 /* A label in a comment section.  */
#endif
	    case C_LABEL:	 /* Label.  */
	      if (src->u.syment.n_scnum == N_DEBUG)
		dst->symbol.flags = BSF_DEBUGGING;
	      else
		dst->symbol.flags = BSF_LOCAL;

	      /* Base the value as an index from the base of the
		 section, if there is one.  */
	      if (dst->symbol.section)
		{
#if defined COFF_WITH_PE
		  /* PE sets the symbol to a value relative to the
		     start of the section.  */
		  dst->symbol.value = src->u.syment.n_value;
#else
		  dst->symbol.value = (src->u.syment.n_value
				       - dst->symbol.section->vma);
#endif
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
#ifdef RS6000COFF_C
	    case C_GSYM:
	    case C_LSYM:
	    case C_PSYM:
	    case C_RSYM:
	    case C_RPSYM:
	    case C_STSYM:
	    case C_TCSYM:
	    case C_BCOMM:
	    case C_ECOML:
	    case C_ECOMM:
	    case C_DECL:
	    case C_ENTRY:
	    case C_FUN:
	    case C_ESTAT:
#endif
	      dst->symbol.flags = BSF_DEBUGGING;
	      dst->symbol.value = (src->u.syment.n_value);
	      break;

#ifdef RS6000COFF_C
	    case C_BINCL:	/* Beginning of include file.  */
	    case C_EINCL:	/* Ending of include file.  */
	      /* The value is actually a pointer into the line numbers
		 of the file.  We locate the line number entry, and
		 set the section to the section which contains it, and
		 the value to the index in that section.  */
	      {
		asection *sec;

		dst->symbol.flags = BSF_DEBUGGING;
		for (sec = abfd->sections; sec != NULL; sec = sec->next)
		  if (sec->line_filepos <= (file_ptr) src->u.syment.n_value
		      && ((file_ptr) (sec->line_filepos
				      + sec->lineno_count * bfd_coff_linesz (abfd))
			  > (file_ptr) src->u.syment.n_value))
		    break;
		if (sec == NULL)
		  dst->symbol.value = 0;
		else
		  {
		    dst->symbol.section = sec;
		    dst->symbol.value = ((src->u.syment.n_value
					  - sec->line_filepos)
					 / bfd_coff_linesz (abfd));
		    src->fix_line = 1;
		  }
	      }
	      break;

	    case C_BSTAT:
	      dst->symbol.flags = BSF_DEBUGGING;

	      /* The value is actually a symbol index.  Save a pointer
		 to the symbol instead of the index.  FIXME: This
		 should use a union.  */
	      src->u.syment.n_value =
		(long) (intptr_t) (native_symbols + src->u.syment.n_value);
	      dst->symbol.value = src->u.syment.n_value;
	      src->fix_value = 1;
	      break;
#endif

	    case C_BLOCK:	/* ".bb" or ".eb".  */
	    case C_FCN:		/* ".bf" or ".ef" (or PE ".lf").  */
	    case C_EFCN:	/* Physical end of function.  */
#if defined COFF_WITH_PE
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
#else
	      /* Base the value as an index from the base of the
		 section.  */
	      dst->symbol.flags = BSF_LOCAL;
	      dst->symbol.value = (src->u.syment.n_value
				   - dst->symbol.section->vma);
#endif
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
#ifdef RS6000COFF_C
	      /* XCOFF specific: deleted entry.  */
	      if (src->u.syment.n_value == C_NULL_VALUE)
		break;
#endif
	      /* Fall through.  */
	    case C_EXTDEF:	/* External definition.  */
	    case C_ULABEL:	/* Undefined label.  */
	    case C_USTATIC:	/* Undefined static.  */
#ifndef COFF_WITH_PE
	    /* C_LINE in regular coff is 0x68.  NT has taken over this storage
	       class to represent a section symbol.  */
	    case C_LINE:	/* line # reformatted as symbol table entry.  */
	      /* NT uses 0x67 for a weak symbol, not C_ALIAS.  */
	    case C_ALIAS:	/* Duplicate tag.  */
#endif
	      /* New storage classes for TI COFF.  */
#if defined(TIC80COFF) || defined(TICOFF)
	    case C_UEXT:	/* Tentative external definition.  */
#endif
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
#ifdef ARM
    case C_THUMBEXT:
    case C_THUMBEXTFUNC:
#endif
#ifdef C_SYSTEM
    case C_SYSTEM:
#endif
#ifdef COFF_WITH_PE
    case C_NT_WEAK:
#endif
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

#ifdef COFF_WITH_PE
  if (syment->n_sclass == C_STAT)
    {
      if (syment->n_scnum == 0)
	/* The Microsoft compiler sometimes generates these if a
	   small static function is inlined every time it is used.
	   The function is discarded, but the symbol table entry
	   remains.  */
	return COFF_SYMBOL_LOCAL;

#ifdef STRICT_PE_FORMAT
      /* This is correct for Microsoft generated objects, but it
	 breaks gas generated objects.  */
      if (syment->n_value == 0)
	{
	  asection *sec;
	  char * name;
	  char buf[SYMNMLEN + 1];

	  name = _bfd_coff_internal_syment_name (abfd, syment, buf)
	  sec = coff_section_from_bfd_index (abfd, syment->n_scnum);
	  if (sec != NULL && name != NULL
	      && (strcmp (bfd_get_section_name (abfd, sec), name) == 0))
	    return COFF_SYMBOL_PE_SECTION;
	}
#endif

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
#endif /* COFF_WITH_PE */

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
    amd64coff_object_p
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
