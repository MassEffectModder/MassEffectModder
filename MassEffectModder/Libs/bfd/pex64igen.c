/* Support for the generic parts of PE/PEI; the common executable parts.
   Copyright (C) 1995-2019 Free Software Foundation, Inc.
   Written by Cygnus Solutions.

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


/* Most of this hacked by Steve Chamberlain <sac@cygnus.com>.

   PE/PEI rearrangement (and code added): Donn Terry
					  Softway Systems, Inc.  */

/* Hey look, some documentation [and in a place you expect to find it]!

   The main reference for the pei format is "Microsoft Portable Executable
   and Common Object File Format Specification 4.1".  Get it if you need to
   do some serious hacking on this code.

   Another reference:
   "Peering Inside the PE: A Tour of the Win32 Portable Executable
   File Format", MSJ 1994, Volume 9.

   The *sole* difference between the pe format and the pei format is that the
   latter has an MSDOS 2.0 .exe header on the front that prints the message
   "This app must be run under Windows." (or some such).
   (FIXME: Whether that statement is *really* true or not is unknown.
   Are there more subtle differences between pe and pei formats?
   For now assume there aren't.  If you find one, then for God sakes
   document it here!)

   The Microsoft docs use the word "image" instead of "executable" because
   the former can also refer to a DLL (shared library).  Confusion can arise
   because the `i' in `pei' also refers to "image".  The `pe' format can
   also create images (i.e. executables), it's just that to run on a win32
   system you need to use the pei format.

   FIXME: Please add more docs here so the next poor fool that has to hack
   on this code has a chance of getting something accomplished without
   wasting too much time.  */

/* This expands into COFF_WITH_pe, COFF_WITH_pep, or COFF_WITH_pex64
   depending on whether we're compiling for straight PE or PE+.  */
#define COFF_WITH_pex64

#include "sysdep.h"
#include "bfd.h"
#include "libbfd.h"
#include "coff/internal.h"
#include "bfdver.h"
#include "libiberty.h"
#ifdef HAVE_WCHAR_H
#include <wchar.h>
#endif
#ifdef HAVE_WCTYPE_H
#include <wctype.h>
#endif

/* NOTE: it's strange to be including an architecture specific header
   in what's supposed to be general (to PE/PEI) code.  However, that's
   where the definitions are, and they don't vary per architecture
   within PE/PEI, so we get them from there.  FIXME: The lack of
   variance is an assumption which may prove to be incorrect if new
   PE/PEI targets are created.  */
# include "coff/x86_64.h"

#include "coff/pe.h"
#include "libcoff.h"
#include "libpei.h"

#if defined COFF_WITH_pep || defined COFF_WITH_pex64
# undef AOUTSZ
# define AOUTSZ		PEPAOUTSZ
# define PEAOUTHDR	PEPAOUTHDR
#endif

#define HighBitSet(val)      ((val) & 0x80000000)
#define SetHighBit(val)      ((val) | 0x80000000)
#define WithoutHighBit(val)  ((val) & 0x7fffffff)

/* FIXME: This file has various tests of POWERPC_LE_PE.  Those tests
   worked when the code was in peicode.h, but no longer work now that
   the code is in peigen.c.  PowerPC NT is said to be dead.  If
   anybody wants to revive the code, you will have to figure out how
   to handle those issues.  */

void
_bfd_pex64i_swap_sym_in (bfd * abfd, void * ext1, void * in1)
{
  SYMENT *ext = (SYMENT *) ext1;
  struct internal_syment *in = (struct internal_syment *) in1;

  if (ext->e.e_name[0] == 0)
    {
      in->_n._n_n._n_zeroes = 0;
      in->_n._n_n._n_offset = H_GET_32 (abfd, ext->e.e.e_offset);
    }
  else
    memcpy (in->_n._n_name, ext->e.e_name, SYMNMLEN);

  in->n_value = H_GET_32 (abfd, ext->e_value);
  in->n_scnum = (short) H_GET_16 (abfd, ext->e_scnum);

  if (sizeof (ext->e_type) == 2)
    in->n_type = H_GET_16 (abfd, ext->e_type);
  else
    in->n_type = H_GET_32 (abfd, ext->e_type);

  in->n_sclass = H_GET_8 (abfd, ext->e_sclass);
  in->n_numaux = H_GET_8 (abfd, ext->e_numaux);

#ifndef STRICT_PE_FORMAT
  /* This is for Gnu-created DLLs.  */

  /* The section symbols for the .idata$ sections have class 0x68
     (C_SECTION), which MS documentation indicates is a section
     symbol.  Unfortunately, the value field in the symbol is simply a
     copy of the .idata section's flags rather than something useful.
     When these symbols are encountered, change the value to 0 so that
     they will be handled somewhat correctly in the bfd code.  */
  if (in->n_sclass == C_SECTION)
    {
      char namebuf[SYMNMLEN + 1];
      const char *name = NULL;

      in->n_value = 0x0;

      /* Create synthetic empty sections as needed.  DJ */
      if (in->n_scnum == 0)
	{
	  asection *sec;

	  name = _bfd_coff_internal_syment_name (abfd, in, namebuf);
	  if (name == NULL)
	    {
	      _bfd_error_handler (_("%pB: unable to find name for empty section"),
				  abfd);
	      bfd_set_error (bfd_error_invalid_target);
	      return;
	    }

	  sec = bfd_get_section_by_name (abfd, name);
	  if (sec != NULL)
	    in->n_scnum = sec->target_index;
	}

      if (in->n_scnum == 0)
	{
	  int unused_section_number = 0;
	  asection *sec;
	  flagword flags;

	  for (sec = abfd->sections; sec; sec = sec->next)
	    if (unused_section_number <= sec->target_index)
	      unused_section_number = sec->target_index + 1;

	  if (name == namebuf)
	    {
	      name = (const char *) bfd_alloc (abfd, strlen (namebuf) + 1);
	      if (name == NULL)
		{
		  _bfd_error_handler (_("%pB: out of memory creating name for empty section"),
				      abfd);
		  return;
		}
	      strcpy ((char *) name, namebuf);
	    }

	  flags = SEC_HAS_CONTENTS | SEC_ALLOC | SEC_DATA | SEC_LOAD;
	  sec = bfd_make_section_anyway_with_flags (abfd, name, flags);
	  if (sec == NULL)
	    {
	      _bfd_error_handler (_("%pB: unable to create fake empty section"),
				  abfd);
	      return;
	    }

	  sec->vma = 0;
	  sec->lma = 0;
	  sec->size = 0;
	  sec->filepos = 0;
	  sec->rel_filepos = 0;
	  sec->reloc_count = 0;
	  sec->line_filepos = 0;
	  sec->lineno_count = 0;
	  sec->userdata = NULL;
	  sec->next = NULL;
	  sec->alignment_power = 2;

	  sec->target_index = unused_section_number;

	  in->n_scnum = unused_section_number;
	}
      in->n_sclass = C_STAT;
    }
#endif

#ifdef coff_swap_sym_in_hook
  /* This won't work in peigen.c, but since it's for PPC PE, it's not
     worth fixing.  */
  coff_swap_sym_in_hook (abfd, ext1, in1);
#endif
}

unsigned int
_bfd_pex64i_swap_sym_out (bfd * abfd ATTRIBUTE_UNUSED,
                          void * inp ATTRIBUTE_UNUSED,
                          void * extp ATTRIBUTE_UNUSED)
{
  return SYMESZ;
}

void
_bfd_pex64i_swap_aux_in (bfd *	abfd,
		      void *	ext1,
		      int       type,
		      int       in_class,
		      int	indx ATTRIBUTE_UNUSED,
		      int	numaux ATTRIBUTE_UNUSED,
		      void *	in1)
{
  AUXENT *ext = (AUXENT *) ext1;
  union internal_auxent *in = (union internal_auxent *) in1;

  /* PR 17521: Make sure that all fields in the aux structure
     are initialised.  */
  memset (in, 0, sizeof * in);
  switch (in_class)
    {
    case C_FILE:
      if (ext->x_file.x_fname[0] == 0)
	{
	  in->x_file.x_n.x_zeroes = 0;
	  in->x_file.x_n.x_offset = H_GET_32 (abfd, ext->x_file.x_n.x_offset);
	}
      else
	memcpy (in->x_file.x_fname, ext->x_file.x_fname, FILNMLEN);
      return;

    case C_STAT:
    case C_LEAFSTAT:
    case C_HIDDEN:
      if (type == T_NULL)
	{
	  in->x_scn.x_scnlen = GET_SCN_SCNLEN (abfd, ext);
	  in->x_scn.x_nreloc = GET_SCN_NRELOC (abfd, ext);
	  in->x_scn.x_nlinno = GET_SCN_NLINNO (abfd, ext);
	  in->x_scn.x_checksum = H_GET_32 (abfd, ext->x_scn.x_checksum);
	  in->x_scn.x_associated = H_GET_16 (abfd, ext->x_scn.x_associated);
	  in->x_scn.x_comdat = H_GET_8 (abfd, ext->x_scn.x_comdat);
	  return;
	}
      break;
    }

  in->x_sym.x_tagndx.l = H_GET_32 (abfd, ext->x_sym.x_tagndx);
  in->x_sym.x_tvndx = H_GET_16 (abfd, ext->x_sym.x_tvndx);

  if (in_class == C_BLOCK || in_class == C_FCN || ISFCN (type)
      || ISTAG (in_class))
    {
      in->x_sym.x_fcnary.x_fcn.x_lnnoptr = GET_FCN_LNNOPTR (abfd, ext);
      in->x_sym.x_fcnary.x_fcn.x_endndx.l = GET_FCN_ENDNDX (abfd, ext);
    }
  else
    {
      in->x_sym.x_fcnary.x_ary.x_dimen[0] =
	H_GET_16 (abfd, ext->x_sym.x_fcnary.x_ary.x_dimen[0]);
      in->x_sym.x_fcnary.x_ary.x_dimen[1] =
	H_GET_16 (abfd, ext->x_sym.x_fcnary.x_ary.x_dimen[1]);
      in->x_sym.x_fcnary.x_ary.x_dimen[2] =
	H_GET_16 (abfd, ext->x_sym.x_fcnary.x_ary.x_dimen[2]);
      in->x_sym.x_fcnary.x_ary.x_dimen[3] =
	H_GET_16 (abfd, ext->x_sym.x_fcnary.x_ary.x_dimen[3]);
    }

  if (ISFCN (type))
    {
      in->x_sym.x_misc.x_fsize = H_GET_32 (abfd, ext->x_sym.x_misc.x_fsize);
    }
  else
    {
      in->x_sym.x_misc.x_lnsz.x_lnno = GET_LNSZ_LNNO (abfd, ext);
      in->x_sym.x_misc.x_lnsz.x_size = GET_LNSZ_SIZE (abfd, ext);
    }
}

unsigned int
_bfd_pex64i_swap_aux_out (bfd *  abfd ATTRIBUTE_UNUSED,
               void * inp ATTRIBUTE_UNUSED,
               int    type ATTRIBUTE_UNUSED,
               int    in_class ATTRIBUTE_UNUSED,
		       int    indx ATTRIBUTE_UNUSED,
		       int    numaux ATTRIBUTE_UNUSED,
               void * extp ATTRIBUTE_UNUSED)
{
  return AUXESZ;
}

void
_bfd_pex64i_swap_lineno_in (bfd * abfd, void * ext1, void * in1)
{
  LINENO *ext = (LINENO *) ext1;
  struct internal_lineno *in = (struct internal_lineno *) in1;

  in->l_addr.l_symndx = H_GET_32 (abfd, ext->l_addr.l_symndx);
  in->l_lnno = GET_LINENO_LNNO (abfd, ext);
}

unsigned int
_bfd_pex64i_swap_lineno_out (bfd * abfd ATTRIBUTE_UNUSED,
                             void * inp ATTRIBUTE_UNUSED,
                             void * outp ATTRIBUTE_UNUSED)
{
  return LINESZ;
}

void
_bfd_pex64i_swap_aouthdr_in (bfd * abfd,
			  void * aouthdr_ext1,
			  void * aouthdr_int1)
{
  PEAOUTHDR * src = (PEAOUTHDR *) aouthdr_ext1;
  AOUTHDR * aouthdr_ext = (AOUTHDR *) aouthdr_ext1;
  struct internal_aouthdr *aouthdr_int
    = (struct internal_aouthdr *) aouthdr_int1;
  struct internal_extra_pe_aouthdr *a = &aouthdr_int->pe;

  aouthdr_int->magic = H_GET_16 (abfd, aouthdr_ext->magic);
  aouthdr_int->vstamp = H_GET_16 (abfd, aouthdr_ext->vstamp);
  aouthdr_int->tsize = GET_AOUTHDR_TSIZE (abfd, aouthdr_ext->tsize);
  aouthdr_int->dsize = GET_AOUTHDR_DSIZE (abfd, aouthdr_ext->dsize);
  aouthdr_int->bsize = GET_AOUTHDR_BSIZE (abfd, aouthdr_ext->bsize);
  aouthdr_int->entry = GET_AOUTHDR_ENTRY (abfd, aouthdr_ext->entry);
  aouthdr_int->text_start =
    GET_AOUTHDR_TEXT_START (abfd, aouthdr_ext->text_start);

#if !defined(COFF_WITH_pep) && !defined(COFF_WITH_pex64)
  /* PE32+ does not have data_start member!  */
  aouthdr_int->data_start =
    GET_AOUTHDR_DATA_START (abfd, aouthdr_ext->data_start);
  a->BaseOfData = aouthdr_int->data_start;
#endif

  a->Magic = aouthdr_int->magic;
  a->MajorLinkerVersion = H_GET_8 (abfd, aouthdr_ext->vstamp);
  a->MinorLinkerVersion = H_GET_8 (abfd, aouthdr_ext->vstamp + 1);
  a->SizeOfCode = aouthdr_int->tsize ;
  a->SizeOfInitializedData = aouthdr_int->dsize ;
  a->SizeOfUninitializedData = aouthdr_int->bsize ;
  a->AddressOfEntryPoint = aouthdr_int->entry;
  a->BaseOfCode = aouthdr_int->text_start;
  a->ImageBase = GET_OPTHDR_IMAGE_BASE (abfd, src->ImageBase);
  a->SectionAlignment = H_GET_32 (abfd, src->SectionAlignment);
  a->FileAlignment = H_GET_32 (abfd, src->FileAlignment);
  a->MajorOperatingSystemVersion =
    H_GET_16 (abfd, src->MajorOperatingSystemVersion);
  a->MinorOperatingSystemVersion =
    H_GET_16 (abfd, src->MinorOperatingSystemVersion);
  a->MajorImageVersion = H_GET_16 (abfd, src->MajorImageVersion);
  a->MinorImageVersion = H_GET_16 (abfd, src->MinorImageVersion);
  a->MajorSubsystemVersion = H_GET_16 (abfd, src->MajorSubsystemVersion);
  a->MinorSubsystemVersion = H_GET_16 (abfd, src->MinorSubsystemVersion);
  a->Reserved1 = H_GET_32 (abfd, src->Reserved1);
  a->SizeOfImage = H_GET_32 (abfd, src->SizeOfImage);
  a->SizeOfHeaders = H_GET_32 (abfd, src->SizeOfHeaders);
  a->CheckSum = H_GET_32 (abfd, src->CheckSum);
  a->Subsystem = H_GET_16 (abfd, src->Subsystem);
  a->DllCharacteristics = H_GET_16 (abfd, src->DllCharacteristics);
  a->SizeOfStackReserve =
    GET_OPTHDR_SIZE_OF_STACK_RESERVE (abfd, src->SizeOfStackReserve);
  a->SizeOfStackCommit =
    GET_OPTHDR_SIZE_OF_STACK_COMMIT (abfd, src->SizeOfStackCommit);
  a->SizeOfHeapReserve =
    GET_OPTHDR_SIZE_OF_HEAP_RESERVE (abfd, src->SizeOfHeapReserve);
  a->SizeOfHeapCommit =
    GET_OPTHDR_SIZE_OF_HEAP_COMMIT (abfd, src->SizeOfHeapCommit);
  a->LoaderFlags = H_GET_32 (abfd, src->LoaderFlags);
  a->NumberOfRvaAndSizes = H_GET_32 (abfd, src->NumberOfRvaAndSizes);

  {
    int idx;

    /* PR 17512: Corrupt PE binaries can cause seg-faults.  */
    if (a->NumberOfRvaAndSizes > IMAGE_NUMBEROF_DIRECTORY_ENTRIES)
      {
	/* xgettext:c-format */
	_bfd_error_handler
	  (_("%pB: aout header specifies an invalid number of data-directory entries: %ld"),
	   abfd, a->NumberOfRvaAndSizes);
	bfd_set_error (bfd_error_bad_value);

	/* Paranoia: If the number is corrupt, then assume that the
	   actual entries themselves might be corrupt as well.  */
	a->NumberOfRvaAndSizes = 0;
      }

    for (idx = 0; idx < a->NumberOfRvaAndSizes; idx++)
      {
	/* If data directory is empty, rva also should be 0.  */
	int size =
	  H_GET_32 (abfd, src->DataDirectory[idx][1]);

	a->DataDirectory[idx].Size = size;

	if (size)
	  a->DataDirectory[idx].VirtualAddress =
	    H_GET_32 (abfd, src->DataDirectory[idx][0]);
	else
	  a->DataDirectory[idx].VirtualAddress = 0;
      }

    while (idx < IMAGE_NUMBEROF_DIRECTORY_ENTRIES)
      {
	a->DataDirectory[idx].Size = 0;
	a->DataDirectory[idx].VirtualAddress = 0;
	idx ++;
      }
  }

  if (aouthdr_int->entry)
    {
      aouthdr_int->entry += a->ImageBase;
#if !defined(COFF_WITH_pep) && !defined(COFF_WITH_pex64)
      aouthdr_int->entry &= 0xffffffff;
#endif
    }

  if (aouthdr_int->tsize)
    {
      aouthdr_int->text_start += a->ImageBase;
#if !defined(COFF_WITH_pep) && !defined(COFF_WITH_pex64)
      aouthdr_int->text_start &= 0xffffffff;
#endif
    }

#if !defined(COFF_WITH_pep) && !defined(COFF_WITH_pex64)
  /* PE32+ does not have data_start member!  */
  if (aouthdr_int->dsize)
    {
      aouthdr_int->data_start += a->ImageBase;
      aouthdr_int->data_start &= 0xffffffff;
    }
#endif

}

unsigned int
_bfd_pex64i_swap_aouthdr_out (bfd * abfd ATTRIBUTE_UNUSED,
                              void * in ATTRIBUTE_UNUSED,
                              void * out ATTRIBUTE_UNUSED)
{
  return AOUTSZ;
}

unsigned int
_bfd_pex64i_only_swap_filehdr_out (bfd * abfd ATTRIBUTE_UNUSED,
                                   void * in ATTRIBUTE_UNUSED,
                                   void * out ATTRIBUTE_UNUSED)
{
  return FILHSZ;
}

unsigned int
_bfd_pex64_only_swap_filehdr_out (bfd * abfd ATTRIBUTE_UNUSED,
                                  void * in ATTRIBUTE_UNUSED,
                                  void * out ATTRIBUTE_UNUSED)
{
  return FILHSZ;
}

unsigned int
_bfd_pex64i_swap_scnhdr_out (bfd * abfd ATTRIBUTE_UNUSED,
                             void * in ATTRIBUTE_UNUSED,
                             void * out ATTRIBUTE_UNUSED)
{
  return SCNHSZ;
}

void
_bfd_pex64i_swap_debugdir_in (bfd * abfd, void * ext1, void * in1)
{
  struct external_IMAGE_DEBUG_DIRECTORY *ext = (struct external_IMAGE_DEBUG_DIRECTORY *) ext1;
  struct internal_IMAGE_DEBUG_DIRECTORY *in = (struct internal_IMAGE_DEBUG_DIRECTORY *) in1;

  in->Characteristics = H_GET_32(abfd, ext->Characteristics);
  in->TimeDateStamp = H_GET_32(abfd, ext->TimeDateStamp);
  in->MajorVersion = H_GET_16(abfd, ext->MajorVersion);
  in->MinorVersion = H_GET_16(abfd, ext->MinorVersion);
  in->Type = H_GET_32(abfd, ext->Type);
  in->SizeOfData = H_GET_32(abfd, ext->SizeOfData);
  in->AddressOfRawData = H_GET_32(abfd, ext->AddressOfRawData);
  in->PointerToRawData = H_GET_32(abfd, ext->PointerToRawData);
}

CODEVIEW_INFO *
_bfd_pex64i_slurp_codeview_record (bfd * abfd, file_ptr where, unsigned long length, CODEVIEW_INFO *cvinfo)
{
  char buffer[256+1];

  if (bfd_seek (abfd, where, SEEK_SET) != 0)
    return NULL;

  if (bfd_bread (buffer, 256, abfd) < 4)
    return NULL;

  /* Ensure null termination of filename.  */
  buffer[256] = '\0';

  cvinfo->CVSignature = H_GET_32 (abfd, buffer);
  cvinfo->Age = 0;

  if ((cvinfo->CVSignature == CVINFO_PDB70_CVSIGNATURE)
      && (length > sizeof (CV_INFO_PDB70)))
    {
      CV_INFO_PDB70 *cvinfo70 = (CV_INFO_PDB70 *)(buffer);

      cvinfo->Age = H_GET_32(abfd, cvinfo70->Age);

      /* A GUID consists of 4,2,2 byte values in little-endian order, followed
	 by 8 single bytes.  Byte swap them so we can conveniently treat the GUID
	 as 16 bytes in big-endian order.  */
      bfd_putb32 (bfd_getl32 (cvinfo70->Signature), cvinfo->Signature);
      bfd_putb16 (bfd_getl16 (&(cvinfo70->Signature[4])), &(cvinfo->Signature[4]));
      bfd_putb16 (bfd_getl16 (&(cvinfo70->Signature[6])), &(cvinfo->Signature[6]));
      memcpy (&(cvinfo->Signature[8]), &(cvinfo70->Signature[8]), 8);

      cvinfo->SignatureLength = CV_INFO_SIGNATURE_LENGTH;
      // cvinfo->PdbFileName = cvinfo70->PdbFileName;

      return cvinfo;
    }
  else if ((cvinfo->CVSignature == CVINFO_PDB20_CVSIGNATURE)
	   && (length > sizeof (CV_INFO_PDB20)))
    {
      CV_INFO_PDB20 *cvinfo20 = (CV_INFO_PDB20 *)(buffer);
      cvinfo->Age = H_GET_32(abfd, cvinfo20->Age);
      memcpy (cvinfo->Signature, cvinfo20->Signature, 4);
      cvinfo->SignatureLength = 4;
      // cvinfo->PdbFileName = cvinfo20->PdbFileName;

      return cvinfo;
    }

  return NULL;
}

bfd_boolean
_bfd_pex64_print_ce_compressed_pdata (bfd * abfd ATTRIBUTE_UNUSED, void * vfile ATTRIBUTE_UNUSED)
{
  return TRUE;
}

bfd_boolean
_bfd_pex64_bfd_copy_private_bfd_data_common (bfd * ibfd ATTRIBUTE_UNUSED, bfd * obfd ATTRIBUTE_UNUSED)
{
  return TRUE;
}

bfd_boolean
_bfd_pex64_bfd_copy_private_section_data (bfd *ibfd ATTRIBUTE_UNUSED,
                       asection *isec ATTRIBUTE_UNUSED,
                       bfd *obfd ATTRIBUTE_UNUSED,
                       asection *osec ATTRIBUTE_UNUSED)
{
  return TRUE;
}

void
_bfd_pex64_get_symbol_info (bfd * abfd ATTRIBUTE_UNUSED,
                            asymbol *symbol ATTRIBUTE_UNUSED,
                            symbol_info *ret ATTRIBUTE_UNUSED)
{
}
