/* x86 specific support for ELF
   Copyright (C) 2017-2019 Free Software Foundation, Inc.

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
#include "objalloc.h"
#include "elf/i386.h"
#include "elf/x86-64.h"

/* The name of the dynamic interpreter.  This is put in the .interp
   section.  */

#define ELF32_DYNAMIC_INTERPRETER "/usr/lib/libc.so.1"
#define ELF64_DYNAMIC_INTERPRETER "/lib/ld64.so.1"
#define ELFX32_DYNAMIC_INTERPRETER "/lib/ldx32.so.1"

bfd_boolean
_bfd_x86_elf_mkobject (bfd *abfd)
{
  return bfd_elf_allocate_object (abfd,
				  sizeof (struct elf_x86_obj_tdata),
                  X86_64_ELF_DATA);
}

/* _TLS_MODULE_BASE_ needs to be treated especially when linking
   executables.  Rather than setting it to the beginning of the TLS
   section, we have to set it to the end.    This function may be called
   multiple times, it is idempotent.  */

void
_bfd_x86_elf_set_tls_module_base (struct bfd_link_info *info)
{
  struct elf_x86_link_hash_table *htab;
  struct bfd_link_hash_entry *base;
  const struct elf_backend_data *bed;

  if (!bfd_link_executable (info))
    return;

  bed = get_elf_backend_data (info->output_bfd);
  htab = elf_x86_hash_table (info, bed->target_id);
  if (htab == NULL)
    return;

  base = htab->tls_module_base;
  if (base == NULL)
    return;

  base->u.def.value = htab->elf.tls_size;
}

/* Return the base VMA address which should be subtracted from real addresses
   when resolving @dtpoff relocation.
   This is PT_TLS segment p_vaddr.  */

bfd_vma
_bfd_x86_elf_dtpoff_base (struct bfd_link_info *info)
{
  /* If tls_sec is NULL, we should have signalled an error already.  */
  if (elf_hash_table (info)->tls_sec == NULL)
    return 0;
  return elf_hash_table (info)->tls_sec->vma;
}

/* Find and/or create a hash entry for local symbol.  */

struct elf_link_hash_entry *
_bfd_elf_x86_get_local_sym_hash (struct elf_x86_link_hash_table *htab,
				 bfd *abfd, const Elf_Internal_Rela *rel,
				 bfd_boolean create)
{
  struct elf_x86_link_hash_entry e, *ret;
  asection *sec = abfd->sections;
  hashval_t h = ELF_LOCAL_SYMBOL_HASH (sec->id,
				       htab->r_sym (rel->r_info));
  void **slot;

  e.elf.indx = sec->id;
  e.elf.dynstr_index = htab->r_sym (rel->r_info);
  slot = htab_find_slot_with_hash (htab->loc_hash_table, &e, h,
				   create ? INSERT : NO_INSERT);

  if (!slot)
    return NULL;

  if (*slot)
    {
      ret = (struct elf_x86_link_hash_entry *) *slot;
      return &ret->elf;
    }

  ret = (struct elf_x86_link_hash_entry *)
	objalloc_alloc ((struct objalloc *) htab->loc_hash_memory,
			sizeof (struct elf_x86_link_hash_entry));
  if (ret)
    {
      memset (ret, 0, sizeof (*ret));
      ret->elf.indx = sec->id;
      ret->elf.dynstr_index = htab->r_sym (rel->r_info);
      ret->elf.dynindx = -1;
      ret->plt_got.offset = (bfd_vma) -1;
      *slot = ret;
    }
  return &ret->elf;
}

/* Create an entry in a x86 ELF linker hash table.  NB: THIS MUST BE IN
   SYNC WITH _bfd_elf_link_hash_newfunc.  */

struct bfd_hash_entry *
_bfd_x86_elf_link_hash_newfunc (struct bfd_hash_entry *entry ATTRIBUTE_UNUSED,
                struct bfd_hash_table *table ATTRIBUTE_UNUSED,
                const char *string ATTRIBUTE_UNUSED)
{
  return NULL;
}

/* Compute a hash of a local hash entry.  We use elf_link_hash_entry
  for local symbol so that we can handle local STT_GNU_IFUNC symbols
  as global symbol.  We reuse indx and dynstr_index for local symbol
  hash since they aren't used by global symbols in this backend.  */

hashval_t
_bfd_x86_elf_local_htab_hash (const void *ptr)
{
  struct elf_link_hash_entry *h
    = (struct elf_link_hash_entry *) ptr;
  return ELF_LOCAL_SYMBOL_HASH (h->indx, h->dynstr_index);
}

/* Compare local hash entries.  */

int
_bfd_x86_elf_local_htab_eq (const void *ptr1, const void *ptr2)
{
  struct elf_link_hash_entry *h1
     = (struct elf_link_hash_entry *) ptr1;
  struct elf_link_hash_entry *h2
    = (struct elf_link_hash_entry *) ptr2;

  return h1->indx == h2->indx && h1->dynstr_index == h2->dynstr_index;
}

/* Destroy an x86 ELF linker hash table.  */

/* Sort relocs into address order.  */

int
_bfd_x86_elf_compare_relocs (const void *ap, const void *bp)
{
  const arelent *a = * (const arelent **) ap;
  const arelent *b = * (const arelent **) bp;

  if (a->address > b->address)
    return 1;
  else if (a->address < b->address)
    return -1;
  else
    return 0;
}

bfd_boolean
_bfd_x86_elf_link_check_relocs (bfd *abfd ATTRIBUTE_UNUSED, struct bfd_link_info *info ATTRIBUTE_UNUSED)
{
  return TRUE;
}

/* Set the sizes of the dynamic sections.  */

bfd_boolean
_bfd_x86_elf_size_dynamic_sections (bfd *output_bfd ATTRIBUTE_UNUSED,
                    struct bfd_link_info *info ATTRIBUTE_UNUSED)
{
  return TRUE;
}

/* Finish up the x86 dynamic sections.  */

struct elf_x86_link_hash_table *
_bfd_x86_elf_finish_dynamic_sections (bfd *output_bfd,
				      struct bfd_link_info *info)
{
  struct elf_x86_link_hash_table *htab;
  const struct elf_backend_data *bed;
  bfd *dynobj;
  asection *sdyn;
  bfd_byte *dyncon, *dynconend;
  bfd_size_type sizeof_dyn;

  bed = get_elf_backend_data (output_bfd);
  htab = elf_x86_hash_table (info, bed->target_id);
  if (htab == NULL)
    return htab;

  dynobj = htab->elf.dynobj;
  sdyn = bfd_get_linker_section (dynobj, ".dynamic");

  /* GOT is always created in setup_gnu_properties.  But it may not be
     needed.  .got.plt section may be needed for static IFUNC.  */
  if (htab->elf.sgotplt && htab->elf.sgotplt->size > 0)
    {
      bfd_vma dynamic_addr;

      if (bfd_is_abs_section (htab->elf.sgotplt->output_section))
	{
	  _bfd_error_handler
	    (_("discarded output section: `%pA'"), htab->elf.sgotplt);
	  return NULL;
	}

      elf_section_data (htab->elf.sgotplt->output_section)->this_hdr.sh_entsize
	= htab->got_entry_size;

      dynamic_addr = (sdyn == NULL
		      ? (bfd_vma) 0
		      : sdyn->output_section->vma + sdyn->output_offset);

      /* Set the first entry in the global offset table to the address
	 of the dynamic section.  Write GOT[1] and GOT[2], needed for
	 the dynamic linker.  */
      if (htab->got_entry_size == 8)
	{
	  bfd_put_64 (output_bfd, dynamic_addr,
		      htab->elf.sgotplt->contents);
	  bfd_put_64 (output_bfd, (bfd_vma) 0,
		      htab->elf.sgotplt->contents + 8);
	  bfd_put_64 (output_bfd, (bfd_vma) 0,
		      htab->elf.sgotplt->contents + 8*2);
	}
      else
	{
	  bfd_put_32 (output_bfd, dynamic_addr,
		      htab->elf.sgotplt->contents);
	  bfd_put_32 (output_bfd, 0,
		      htab->elf.sgotplt->contents + 4);
	  bfd_put_32 (output_bfd, 0,
		      htab->elf.sgotplt->contents + 4*2);
	}
    }

  if (!htab->elf.dynamic_sections_created)
    return htab;

  if (sdyn == NULL || htab->elf.sgot == NULL)
    abort ();

  sizeof_dyn = bed->s->sizeof_dyn;
  dyncon = sdyn->contents;
  dynconend = sdyn->contents + sdyn->size;
  for (; dyncon < dynconend; dyncon += sizeof_dyn)
    {
      Elf_Internal_Dyn dyn;
      asection *s;

      (*bed->s->swap_dyn_in) (dynobj, dyncon, &dyn);

      switch (dyn.d_tag)
	{
	default:
	  continue;

	case DT_PLTGOT:
	  s = htab->elf.sgotplt;
	  dyn.d_un.d_ptr = s->output_section->vma + s->output_offset;
	  break;

	case DT_JMPREL:
	  dyn.d_un.d_ptr = htab->elf.srelplt->output_section->vma;
	  break;

	case DT_PLTRELSZ:
	  s = htab->elf.srelplt->output_section;
	  dyn.d_un.d_val = s->size;
	  break;

	case DT_TLSDESC_PLT:
	  s = htab->elf.splt;
	  dyn.d_un.d_ptr = s->output_section->vma + s->output_offset
	    + htab->tlsdesc_plt;
	  break;

	case DT_TLSDESC_GOT:
	  s = htab->elf.sgot;
	  dyn.d_un.d_ptr = s->output_section->vma + s->output_offset
	    + htab->tlsdesc_got;
	  break;
	}

      (*bed->s->swap_dyn_out) (output_bfd, &dyn, dyncon);
    }

  if (htab->plt_got != NULL && htab->plt_got->size > 0)
    elf_section_data (htab->plt_got->output_section)
      ->this_hdr.sh_entsize = htab->non_lazy_plt->plt_entry_size;

  if (htab->plt_second != NULL && htab->plt_second->size > 0)
    elf_section_data (htab->plt_second->output_section)
      ->this_hdr.sh_entsize = htab->non_lazy_plt->plt_entry_size;

  /* Adjust .eh_frame for .plt section.  */
  if (htab->plt_eh_frame != NULL
      && htab->plt_eh_frame->contents != NULL)
    {
      if (htab->elf.splt != NULL
	  && htab->elf.splt->size != 0
	  && (htab->elf.splt->flags & SEC_EXCLUDE) == 0
	  && htab->elf.splt->output_section != NULL
	  && htab->plt_eh_frame->output_section != NULL)
	{
	  bfd_vma plt_start = htab->elf.splt->output_section->vma;
	  bfd_vma eh_frame_start = htab->plt_eh_frame->output_section->vma
				   + htab->plt_eh_frame->output_offset
				   + PLT_FDE_START_OFFSET;
	  bfd_put_signed_32 (dynobj, plt_start - eh_frame_start,
			     htab->plt_eh_frame->contents
			     + PLT_FDE_START_OFFSET);
	}

      if (htab->plt_eh_frame->sec_info_type == SEC_INFO_TYPE_EH_FRAME)
	{
	  if (! _bfd_elf_write_section_eh_frame (output_bfd, info,
						 htab->plt_eh_frame,
						 htab->plt_eh_frame->contents))
	    return NULL;
	}
    }

  /* Adjust .eh_frame for .plt.got section.  */
  if (htab->plt_got_eh_frame != NULL
      && htab->plt_got_eh_frame->contents != NULL)
    {
      if (htab->plt_got != NULL
	  && htab->plt_got->size != 0
	  && (htab->plt_got->flags & SEC_EXCLUDE) == 0
	  && htab->plt_got->output_section != NULL
	  && htab->plt_got_eh_frame->output_section != NULL)
	{
	  bfd_vma plt_start = htab->plt_got->output_section->vma;
	  bfd_vma eh_frame_start = htab->plt_got_eh_frame->output_section->vma
				   + htab->plt_got_eh_frame->output_offset
				   + PLT_FDE_START_OFFSET;
	  bfd_put_signed_32 (dynobj, plt_start - eh_frame_start,
			     htab->plt_got_eh_frame->contents
			     + PLT_FDE_START_OFFSET);
	}
      if (htab->plt_got_eh_frame->sec_info_type == SEC_INFO_TYPE_EH_FRAME)
	{
	  if (! _bfd_elf_write_section_eh_frame (output_bfd, info,
						 htab->plt_got_eh_frame,
						 htab->plt_got_eh_frame->contents))
	    return NULL;
	}
    }

  /* Adjust .eh_frame for the second PLT section.  */
  if (htab->plt_second_eh_frame != NULL
      && htab->plt_second_eh_frame->contents != NULL)
    {
      if (htab->plt_second != NULL
	  && htab->plt_second->size != 0
	  && (htab->plt_second->flags & SEC_EXCLUDE) == 0
	  && htab->plt_second->output_section != NULL
	  && htab->plt_second_eh_frame->output_section != NULL)
	{
	  bfd_vma plt_start = htab->plt_second->output_section->vma;
	  bfd_vma eh_frame_start
	    = (htab->plt_second_eh_frame->output_section->vma
	       + htab->plt_second_eh_frame->output_offset
	       + PLT_FDE_START_OFFSET);
	  bfd_put_signed_32 (dynobj, plt_start - eh_frame_start,
			     htab->plt_second_eh_frame->contents
			     + PLT_FDE_START_OFFSET);
	}
      if (htab->plt_second_eh_frame->sec_info_type
	  == SEC_INFO_TYPE_EH_FRAME)
	{
	  if (! _bfd_elf_write_section_eh_frame (output_bfd, info,
						 htab->plt_second_eh_frame,
						 htab->plt_second_eh_frame->contents))
	    return NULL;
	}
    }

  if (htab->elf.sgot && htab->elf.sgot->size > 0)
    elf_section_data (htab->elf.sgot->output_section)->this_hdr.sh_entsize
      = htab->got_entry_size;

  return htab;
}


bfd_boolean
_bfd_x86_elf_always_size_sections (bfd *output_bfd ATTRIBUTE_UNUSED,
                   struct bfd_link_info *info ATTRIBUTE_UNUSED)
{
  return TRUE;
}

void
_bfd_x86_elf_merge_symbol_attribute (struct elf_link_hash_entry *h,
				     const Elf_Internal_Sym *isym,
				     bfd_boolean definition,
				     bfd_boolean dynamic ATTRIBUTE_UNUSED)
{
  if (definition)
    {
      struct elf_x86_link_hash_entry *eh
	= (struct elf_x86_link_hash_entry *) h;
      eh->def_protected = (ELF_ST_VISIBILITY (isym->st_other)
			   == STV_PROTECTED);
    }
}

/* Copy the extra info we tack onto an elf_link_hash_entry.  */

void
_bfd_x86_elf_copy_indirect_symbol (struct bfd_link_info *info ATTRIBUTE_UNUSED,
                   struct elf_link_hash_entry *dir ATTRIBUTE_UNUSED,
                   struct elf_link_hash_entry *ind ATTRIBUTE_UNUSED)
{
}

/* Remove undefined weak symbol from the dynamic symbol table if it
   is resolved to 0.   */

bfd_boolean
_bfd_x86_elf_fixup_symbol (struct bfd_link_info *info ATTRIBUTE_UNUSED,
               struct elf_link_hash_entry *h ATTRIBUTE_UNUSED)
{
  return TRUE;
}

/* Change the STT_GNU_IFUNC symbol defined in position-dependent
   executable into the normal function symbol and set its address
   to its PLT entry, which should be resolved by R_*_IRELATIVE at
   run-time.  */

void
_bfd_x86_elf_link_fixup_ifunc_symbol (struct bfd_link_info *info,
				      struct elf_x86_link_hash_table *htab,
				      struct elf_link_hash_entry *h,
				      Elf_Internal_Sym *sym)
{
  if (bfd_link_pde (info)
      && h->def_regular
      && h->dynindx != -1
      && h->plt.offset != (bfd_vma) -1
      && h->type == STT_GNU_IFUNC
      && h->pointer_equality_needed)
    {
      asection *plt_s;
      bfd_vma plt_offset;
      bfd *output_bfd = info->output_bfd;

      if (htab->plt_second)
	{
	  struct elf_x86_link_hash_entry *eh
	    = (struct elf_x86_link_hash_entry *) h;

	  plt_s = htab->plt_second;
	  plt_offset = eh->plt_second.offset;
	}
      else
	{
	  plt_s = htab->elf.splt;
	  plt_offset = h->plt.offset;
	}

      sym->st_size = 0;
      sym->st_info = ELF_ST_INFO (ELF_ST_BIND (sym->st_info), STT_FUNC);
      sym->st_shndx
	= _bfd_elf_section_from_bfd_section (output_bfd,
					     plt_s->output_section);
      sym->st_value = (plt_s->output_section->vma
		       + plt_s->output_offset + plt_offset);
    }
}

/* Return TRUE if symbol should be hashed in the `.gnu.hash' section.  */

bfd_boolean
_bfd_x86_elf_hash_symbol (struct elf_link_hash_entry *h ATTRIBUTE_UNUSED)
{
  return TRUE;
}

/* Adjust a symbol defined by a dynamic object and referenced by a
   regular object.  The current definition is in some section of the
   dynamic object, but we're not including those sections.  We have to
   change the definition to something the rest of the link can
   understand.  */

bfd_boolean
_bfd_x86_elf_adjust_dynamic_symbol (struct bfd_link_info *info ATTRIBUTE_UNUSED,
                    struct elf_link_hash_entry *h ATTRIBUTE_UNUSED)
{
  return TRUE;
}

void
_bfd_x86_elf_hide_symbol (struct bfd_link_info *info ATTRIBUTE_UNUSED,
              struct elf_link_hash_entry *h ATTRIBUTE_UNUSED,
              bfd_boolean force_local ATTRIBUTE_UNUSED)
{
}

/* Return TRUE if a symbol is referenced locally.  It is similar to
   SYMBOL_REFERENCES_LOCAL, but it also checks version script.  It
   works in check_relocs.  */

bfd_boolean
_bfd_x86_elf_link_symbol_references_local (struct bfd_link_info *info ATTRIBUTE_UNUSED,
                       struct elf_link_hash_entry *h ATTRIBUTE_UNUSED)
{
  return FALSE;
}

/* Return the section that should be marked against GC for a given
   relocation.	*/

asection *
_bfd_x86_elf_gc_mark_hook (asection *sec ATTRIBUTE_UNUSED,
               struct bfd_link_info *info ATTRIBUTE_UNUSED,
               Elf_Internal_Rela *rel ATTRIBUTE_UNUSED,
               struct elf_link_hash_entry *h ATTRIBUTE_UNUSED,
               Elf_Internal_Sym *sym ATTRIBUTE_UNUSED)
{
  return NULL;
}

long
_bfd_x86_elf_get_synthetic_symtab (bfd *abfd ATTRIBUTE_UNUSED,
                   long count ATTRIBUTE_UNUSED,
                   long relsize ATTRIBUTE_UNUSED,
                   bfd_vma got_addr ATTRIBUTE_UNUSED,
                   struct elf_x86_plt plts[] ATTRIBUTE_UNUSED,
                   asymbol **dynsyms ATTRIBUTE_UNUSED,
                   asymbol **ret ATTRIBUTE_UNUSED)
{

  return -1;
}

/* Parse x86 GNU properties.  */

enum elf_property_kind
_bfd_x86_elf_parse_gnu_properties (bfd *abfd, unsigned int type,
				   bfd_byte *ptr, unsigned int datasz)
{
  elf_property *prop;

  if (type == GNU_PROPERTY_X86_COMPAT_ISA_1_USED
      || type == GNU_PROPERTY_X86_COMPAT_ISA_1_NEEDED
      || (type >= GNU_PROPERTY_X86_UINT32_AND_LO
	  && type <= GNU_PROPERTY_X86_UINT32_AND_HI)
      || (type >= GNU_PROPERTY_X86_UINT32_OR_LO
	  && type <= GNU_PROPERTY_X86_UINT32_OR_HI)
      || (type >= GNU_PROPERTY_X86_UINT32_OR_AND_LO
	  && type <= GNU_PROPERTY_X86_UINT32_OR_AND_HI))
    {
      if (datasz != 4)
	{
	  _bfd_error_handler
	    (_("error: %pB: <corrupt x86 property (0x%x) size: 0x%x>"),
	     abfd, type, datasz);
	  return property_corrupt;
	}
      prop = _bfd_elf_get_property (abfd, type, datasz);
      prop->u.number |= bfd_h_get_32 (abfd, ptr);
      prop->pr_kind = property_number;
      return property_number;
    }

  return property_ignored;
}

/* Merge x86 GNU property BPROP with APROP.  If APROP isn't NULL,
   return TRUE if APROP is updated.  Otherwise, return TRUE if BPROP
   should be merged with ABFD.  */

bfd_boolean
_bfd_x86_elf_merge_gnu_properties (struct bfd_link_info *info,
				   bfd *abfd ATTRIBUTE_UNUSED,
				   elf_property *aprop,
				   elf_property *bprop)
{
  unsigned int number, features;
  bfd_boolean updated = FALSE;
  unsigned int pr_type = aprop != NULL ? aprop->pr_type : bprop->pr_type;

  if (pr_type == GNU_PROPERTY_X86_COMPAT_ISA_1_USED
      || (pr_type >= GNU_PROPERTY_X86_UINT32_OR_AND_LO
	  && pr_type <= GNU_PROPERTY_X86_UINT32_OR_AND_HI))
    {
      if (aprop == NULL || bprop == NULL)
	{
	  /* Only one of APROP and BPROP can be NULL.  */
	  if (aprop != NULL)
	    {
	      /* Remove this property since the other input file doesn't
		 have it.  */
	      aprop->pr_kind = property_remove;
	      updated = TRUE;
	    }
	}
      else
	{
	  number = aprop->u.number;
	  aprop->u.number = number | bprop->u.number;
	  updated = number != (unsigned int) aprop->u.number;
	}
      return updated;
    }
  else if (pr_type == GNU_PROPERTY_X86_COMPAT_ISA_1_NEEDED
	   || (pr_type >= GNU_PROPERTY_X86_UINT32_OR_LO
	       && pr_type <= GNU_PROPERTY_X86_UINT32_OR_HI))
    {
      if (aprop != NULL && bprop != NULL)
	{
	  number = aprop->u.number;
	  aprop->u.number = number | bprop->u.number;
	  /* Remove the property if all bits are empty.  */
	  if (aprop->u.number == 0)
	    {
	      aprop->pr_kind = property_remove;
	      updated = TRUE;
	    }
	  else
	    updated = number != (unsigned int) aprop->u.number;
	}
      else
	{
	  /* Only one of APROP and BPROP can be NULL.  */
	  if (aprop != NULL)
	    {
	      if (aprop->u.number == 0)
		{
		  /* Remove APROP if all bits are empty.  */
		  aprop->pr_kind = property_remove;
		  updated = TRUE;
		}
	    }
	  else
	    {
	      /* Return TRUE if APROP is NULL and all bits of BPROP
		 aren't empty to indicate that BPROP should be added
		 to ABFD.  */
	      updated = bprop->u.number != 0;
	    }
	}
      return updated;
    }
  else if (pr_type >= GNU_PROPERTY_X86_UINT32_AND_LO
	   && pr_type <= GNU_PROPERTY_X86_UINT32_AND_HI)
    {
      /* Only one of APROP and BPROP can be NULL:
	 1. APROP & BPROP when both APROP and BPROP aren't NULL.
	 2. If APROP is NULL, remove x86 feature.
	 3. Otherwise, do nothing.
       */
      if (aprop != NULL && bprop != NULL)
	{
	  features = 0;
	  if (info->ibt)
	    features = GNU_PROPERTY_X86_FEATURE_1_IBT;
	  if (info->shstk)
	    features |= GNU_PROPERTY_X86_FEATURE_1_SHSTK;
	  number = aprop->u.number;
	  /* Add GNU_PROPERTY_X86_FEATURE_1_IBT and
	     GNU_PROPERTY_X86_FEATURE_1_SHSTK.  */
	  aprop->u.number = (number & bprop->u.number) | features;
	  updated = number != (unsigned int) aprop->u.number;
	  /* Remove the property if all feature bits are cleared.  */
	  if (aprop->u.number == 0)
	    aprop->pr_kind = property_remove;
	}
      else
	{
	  features = 0;
	  if (info->ibt)
	    features = GNU_PROPERTY_X86_FEATURE_1_IBT;
	  if (info->shstk)
	    features |= GNU_PROPERTY_X86_FEATURE_1_SHSTK;
	  if (features)
	    {
	      /* Add GNU_PROPERTY_X86_FEATURE_1_IBT and
		 GNU_PROPERTY_X86_FEATURE_1_SHSTK.  */
	      if (aprop != NULL)
		{
		  number = aprop->u.number;
		  aprop->u.number = number | features;
		  updated = number != (unsigned int) aprop->u.number;
		}
	      else
		{
		  bprop->u.number |= features;
		  updated = TRUE;
		}
	    }
	  else if (aprop != NULL)
	    {
	      aprop->pr_kind = property_remove;
	      updated = TRUE;
	    }
	}
      return updated;
    }
  else
    {
      /* Never should happen.  */
      abort ();
    }

  return updated;
}

/* Set up x86 GNU properties.  Return the first relocatable ELF input
   with GNU properties if found.  Otherwise, return NULL.  */

bfd *
_bfd_x86_elf_link_setup_gnu_properties
  (struct bfd_link_info *info ATTRIBUTE_UNUSED, struct elf_x86_init_table *init_table ATTRIBUTE_UNUSED)
{
  return NULL;
}

/* Fix up x86 GNU properties.  */

void
_bfd_x86_elf_link_fixup_gnu_properties
  (struct bfd_link_info *info ATTRIBUTE_UNUSED,
   elf_property_list **listp)
{
  elf_property_list *p;

  for (p = *listp; p; p = p->next)
    {
      unsigned int type = p->property.pr_type;
      if (type == GNU_PROPERTY_X86_COMPAT_ISA_1_USED
	  || type == GNU_PROPERTY_X86_COMPAT_ISA_1_NEEDED
	  || (type >= GNU_PROPERTY_X86_UINT32_AND_LO
	      && type <= GNU_PROPERTY_X86_UINT32_AND_HI)
	  || (type >= GNU_PROPERTY_X86_UINT32_OR_LO
	      && type <= GNU_PROPERTY_X86_UINT32_OR_HI)
	  || (type >= GNU_PROPERTY_X86_UINT32_OR_AND_LO
	      && type <= GNU_PROPERTY_X86_UINT32_OR_AND_HI))
	{
	  if (p->property.u.number == 0
	      && (type == GNU_PROPERTY_X86_COMPAT_ISA_1_NEEDED
		  || (type >= GNU_PROPERTY_X86_UINT32_AND_LO
		      && type <= GNU_PROPERTY_X86_UINT32_AND_HI)
		  || (type >= GNU_PROPERTY_X86_UINT32_OR_LO
		      && type <= GNU_PROPERTY_X86_UINT32_OR_HI)))
	    {
	      /* Remove empty property.  */
	      *listp = p->next;
	      continue;
	    }

	  listp = &p->next;
	}
      else if (type > GNU_PROPERTY_HIPROC)
	{
	  /* The property list is sorted in order of type.  */
	  break;
	}
    }
}
