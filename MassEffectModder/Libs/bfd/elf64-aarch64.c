/* AArch64-specific support for 64-bit ELF.
   Copyright (C) 2009-2019 Free Software Foundation, Inc.
   Contributed by ARM Ltd.

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
   along with this program; see the file COPYING3. If not,
   see <http://www.gnu.org/licenses/>.  */

/* Notes on implementation:

  Thread Local Store (TLS)

  Overview:

  The implementation currently supports both traditional TLS and TLS
  descriptors, but only general dynamic (GD).

  For traditional TLS the assembler will present us with code
  fragments of the form:

  adrp x0, :tlsgd:foo
			   R_AARCH64_TLSGD_ADR_PAGE21(foo)
  add  x0, :tlsgd_lo12:foo
			   R_AARCH64_TLSGD_ADD_LO12_NC(foo)
  bl   __tls_get_addr
  nop

  For TLS descriptors the assembler will present us with code
  fragments of the form:

  adrp	x0, :tlsdesc:foo		      R_AARCH64_TLSDESC_ADR_PAGE21(foo)
  ldr	x1, [x0, #:tlsdesc_lo12:foo]	      R_AARCH64_TLSDESC_LD64_LO12(foo)
  add	x0, x0, #:tlsdesc_lo12:foo	      R_AARCH64_TLSDESC_ADD_LO12(foo)
  .tlsdesccall foo
  blr	x1				      R_AARCH64_TLSDESC_CALL(foo)

  The relocations R_AARCH64_TLSGD_{ADR_PREL21,ADD_LO12_NC} against foo
  indicate that foo is thread local and should be accessed via the
  traditional TLS mechanims.

  The relocations R_AARCH64_TLSDESC_{ADR_PAGE21,LD64_LO12_NC,ADD_LO12_NC}
  against foo indicate that 'foo' is thread local and should be accessed
  via a TLS descriptor mechanism.

  The precise instruction sequence is only relevant from the
  perspective of linker relaxation which is currently not implemented.

  The static linker must detect that 'foo' is a TLS object and
  allocate a double GOT entry. The GOT entry must be created for both
  global and local TLS symbols. Note that this is different to none
  TLS local objects which do not need a GOT entry.

  In the traditional TLS mechanism, the double GOT entry is used to
  provide the tls_index structure, containing module and offset
  entries. The static linker places the relocation R_AARCH64_TLS_DTPMOD
  on the module entry. The loader will subsequently fixup this
  relocation with the module identity.

  For global traditional TLS symbols the static linker places an
  R_AARCH64_TLS_DTPREL relocation on the offset entry. The loader
  will subsequently fixup the offset. For local TLS symbols the static
  linker fixes up offset.

  In the TLS descriptor mechanism the double GOT entry is used to
  provide the descriptor. The static linker places the relocation
  R_AARCH64_TLSDESC on the first GOT slot. The loader will
  subsequently fix this up.

  Implementation:

  The handling of TLS symbols is implemented across a number of
  different backend functions. The following is a top level view of
  what processing is performed where.

  The TLS implementation maintains state information for each TLS
  symbol. The state information for local and global symbols is kept
  in different places. Global symbols use generic BFD structures while
  local symbols use backend specific structures that are allocated and
  maintained entirely by the backend.

  The flow:

  elf64_aarch64_check_relocs()

  This function is invoked for each relocation.

  The TLS relocations R_AARCH64_TLSGD_{ADR_PREL21,ADD_LO12_NC} and
  R_AARCH64_TLSDESC_{ADR_PAGE21,LD64_LO12_NC,ADD_LO12_NC} are
  spotted. One time creation of local symbol data structures are
  created when the first local symbol is seen.

  The reference count for a symbol is incremented.  The GOT type for
  each symbol is marked as general dynamic.

  elf64_aarch64_allocate_dynrelocs ()

  For each global with positive reference count we allocate a double
  GOT slot. For a traditional TLS symbol we allocate space for two
  relocation entries on the GOT, for a TLS descriptor symbol we
  allocate space for one relocation on the slot. Record the GOT offset
  for this symbol.

  elf64_aarch64_size_dynamic_sections ()

  Iterate all input BFDS, look for in the local symbol data structure
  constructed earlier for local TLS symbols and allocate them double
  GOT slots along with space for a single GOT relocation. Update the
  local symbol structure to record the GOT offset allocated.

  elf64_aarch64_relocate_section ()

  Calls elf64_aarch64_final_link_relocate ()

  Emit the relevant TLS relocations against the GOT for each TLS
  symbol. For local TLS symbols emit the GOT offset directly. The GOT
  relocations are emitted once the first time a TLS symbol is
  encountered. The implementation uses the LSB of the GOT offset to
  flag that the relevant GOT relocations for a symbol have been
  emitted. All of the TLS code that uses the GOT offset needs to take
  care to mask out this flag bit before using the offset.

  elf64_aarch64_final_link_relocate ()

  Fixup the R_AARCH64_TLSGD_{ADR_PREL21, ADD_LO12_NC} relocations.  */

#include "sysdep.h"
#include "bfd.h"
#include "libiberty.h"
#include "libbfd.h"
#include "elf-bfd.h"
#include "bfdlink.h"
#include "objalloc.h"
#include "elfxx-aarch64.h"

#define ARCH_SIZE	64

#if ARCH_SIZE == 64
#define AARCH64_R(NAME)		R_AARCH64_ ## NAME
#define AARCH64_R_STR(NAME)	"R_AARCH64_" #NAME
#define HOWTO64(...)		HOWTO (__VA_ARGS__)
#define HOWTO32(...)		EMPTY_HOWTO (0)
#define LOG_FILE_ALIGN	3
#define BFD_RELOC_AARCH64_TLSDESC_LD64_LO12_NC BFD_RELOC_AARCH64_TLSDESC_LD64_LO12
#endif

#define IS_AARCH64_TLS_RELOC(R_TYPE)				\
  ((R_TYPE) == BFD_RELOC_AARCH64_TLSGD_ADD_LO12_NC		\
   || (R_TYPE) == BFD_RELOC_AARCH64_TLSGD_ADR_PAGE21		\
   || (R_TYPE) == BFD_RELOC_AARCH64_TLSGD_ADR_PREL21		\
   || (R_TYPE) == BFD_RELOC_AARCH64_TLSGD_MOVW_G0_NC		\
   || (R_TYPE) == BFD_RELOC_AARCH64_TLSGD_MOVW_G1		\
   || (R_TYPE) == BFD_RELOC_AARCH64_TLSIE_ADR_GOTTPREL_PAGE21	\
   || (R_TYPE) == BFD_RELOC_AARCH64_TLSIE_LD32_GOTTPREL_LO12_NC	\
   || (R_TYPE) == BFD_RELOC_AARCH64_TLSIE_LD64_GOTTPREL_LO12_NC	\
   || (R_TYPE) == BFD_RELOC_AARCH64_TLSIE_LD_GOTTPREL_PREL19	\
   || (R_TYPE) == BFD_RELOC_AARCH64_TLSIE_MOVW_GOTTPREL_G0_NC	\
   || (R_TYPE) == BFD_RELOC_AARCH64_TLSIE_MOVW_GOTTPREL_G1	\
   || (R_TYPE) == BFD_RELOC_AARCH64_TLSLD_ADD_DTPREL_HI12	\
   || (R_TYPE) == BFD_RELOC_AARCH64_TLSLD_ADD_DTPREL_LO12	\
   || (R_TYPE) == BFD_RELOC_AARCH64_TLSLD_ADD_DTPREL_LO12_NC	\
   || (R_TYPE) == BFD_RELOC_AARCH64_TLSLD_ADD_LO12_NC		\
   || (R_TYPE) == BFD_RELOC_AARCH64_TLSLD_ADR_PAGE21		\
   || (R_TYPE) == BFD_RELOC_AARCH64_TLSLD_ADR_PREL21		\
   || (R_TYPE) == BFD_RELOC_AARCH64_TLSLD_LDST16_DTPREL_LO12	\
   || (R_TYPE) == BFD_RELOC_AARCH64_TLSLD_LDST16_DTPREL_LO12_NC	\
   || (R_TYPE) == BFD_RELOC_AARCH64_TLSLD_LDST32_DTPREL_LO12	\
   || (R_TYPE) == BFD_RELOC_AARCH64_TLSLD_LDST32_DTPREL_LO12_NC	\
   || (R_TYPE) == BFD_RELOC_AARCH64_TLSLD_LDST64_DTPREL_LO12	\
   || (R_TYPE) == BFD_RELOC_AARCH64_TLSLD_LDST64_DTPREL_LO12_NC	\
   || (R_TYPE) == BFD_RELOC_AARCH64_TLSLD_LDST8_DTPREL_LO12	\
   || (R_TYPE) == BFD_RELOC_AARCH64_TLSLD_LDST8_DTPREL_LO12_NC	\
   || (R_TYPE) == BFD_RELOC_AARCH64_TLSLD_MOVW_DTPREL_G0	\
   || (R_TYPE) == BFD_RELOC_AARCH64_TLSLD_MOVW_DTPREL_G0_NC	\
   || (R_TYPE) == BFD_RELOC_AARCH64_TLSLD_MOVW_DTPREL_G1	\
   || (R_TYPE) == BFD_RELOC_AARCH64_TLSLD_MOVW_DTPREL_G1_NC	\
   || (R_TYPE) == BFD_RELOC_AARCH64_TLSLD_MOVW_DTPREL_G2	\
   || (R_TYPE) == BFD_RELOC_AARCH64_TLSLE_ADD_TPREL_HI12	\
   || (R_TYPE) == BFD_RELOC_AARCH64_TLSLE_ADD_TPREL_LO12	\
   || (R_TYPE) == BFD_RELOC_AARCH64_TLSLE_ADD_TPREL_LO12_NC	\
   || (R_TYPE) == BFD_RELOC_AARCH64_TLSLE_LDST16_TPREL_LO12	\
   || (R_TYPE) == BFD_RELOC_AARCH64_TLSLE_LDST16_TPREL_LO12_NC	\
   || (R_TYPE) == BFD_RELOC_AARCH64_TLSLE_LDST32_TPREL_LO12	\
   || (R_TYPE) == BFD_RELOC_AARCH64_TLSLE_LDST32_TPREL_LO12_NC	\
   || (R_TYPE) == BFD_RELOC_AARCH64_TLSLE_LDST64_TPREL_LO12	\
   || (R_TYPE) == BFD_RELOC_AARCH64_TLSLE_LDST64_TPREL_LO12_NC	\
   || (R_TYPE) == BFD_RELOC_AARCH64_TLSLE_LDST8_TPREL_LO12	\
   || (R_TYPE) == BFD_RELOC_AARCH64_TLSLE_LDST8_TPREL_LO12_NC	\
   || (R_TYPE) == BFD_RELOC_AARCH64_TLSLE_MOVW_TPREL_G0		\
   || (R_TYPE) == BFD_RELOC_AARCH64_TLSLE_MOVW_TPREL_G0_NC	\
   || (R_TYPE) == BFD_RELOC_AARCH64_TLSLE_MOVW_TPREL_G1		\
   || (R_TYPE) == BFD_RELOC_AARCH64_TLSLE_MOVW_TPREL_G1_NC	\
   || (R_TYPE) == BFD_RELOC_AARCH64_TLSLE_MOVW_TPREL_G2		\
   || (R_TYPE) == BFD_RELOC_AARCH64_TLS_DTPMOD			\
   || (R_TYPE) == BFD_RELOC_AARCH64_TLS_DTPREL			\
   || (R_TYPE) == BFD_RELOC_AARCH64_TLS_TPREL			\
   || IS_AARCH64_TLSDESC_RELOC ((R_TYPE)))

#define IS_AARCH64_TLS_RELAX_RELOC(R_TYPE)			\
  ((R_TYPE) == BFD_RELOC_AARCH64_TLSDESC_ADD			\
   || (R_TYPE) == BFD_RELOC_AARCH64_TLSDESC_ADD_LO12		\
   || (R_TYPE) == BFD_RELOC_AARCH64_TLSDESC_ADR_PAGE21		\
   || (R_TYPE) == BFD_RELOC_AARCH64_TLSDESC_ADR_PREL21		\
   || (R_TYPE) == BFD_RELOC_AARCH64_TLSDESC_CALL		\
   || (R_TYPE) == BFD_RELOC_AARCH64_TLSDESC_LD_PREL19		\
   || (R_TYPE) == BFD_RELOC_AARCH64_TLSDESC_LD64_LO12_NC	\
   || (R_TYPE) == BFD_RELOC_AARCH64_TLSDESC_LDR			\
   || (R_TYPE) == BFD_RELOC_AARCH64_TLSDESC_OFF_G0_NC		\
   || (R_TYPE) == BFD_RELOC_AARCH64_TLSDESC_OFF_G1		\
   || (R_TYPE) == BFD_RELOC_AARCH64_TLSDESC_LDR			\
   || (R_TYPE) == BFD_RELOC_AARCH64_TLSGD_ADR_PAGE21		\
   || (R_TYPE) == BFD_RELOC_AARCH64_TLSGD_ADR_PREL21		\
   || (R_TYPE) == BFD_RELOC_AARCH64_TLSGD_ADD_LO12_NC		\
   || (R_TYPE) == BFD_RELOC_AARCH64_TLSGD_MOVW_G0_NC		\
   || (R_TYPE) == BFD_RELOC_AARCH64_TLSGD_MOVW_G1		\
   || (R_TYPE) == BFD_RELOC_AARCH64_TLSIE_ADR_GOTTPREL_PAGE21	\
   || (R_TYPE) == BFD_RELOC_AARCH64_TLSIE_LD_GOTTPREL_PREL19	\
   || (R_TYPE) == BFD_RELOC_AARCH64_TLSIE_LD64_GOTTPREL_LO12_NC	\
   || (R_TYPE) == BFD_RELOC_AARCH64_TLSLD_ADD_LO12_NC		\
   || (R_TYPE) == BFD_RELOC_AARCH64_TLSLD_ADR_PAGE21		\
   || (R_TYPE) == BFD_RELOC_AARCH64_TLSLD_ADR_PREL21)

#define IS_AARCH64_TLSDESC_RELOC(R_TYPE)			\
  ((R_TYPE) == BFD_RELOC_AARCH64_TLSDESC			\
   || (R_TYPE) == BFD_RELOC_AARCH64_TLSDESC_ADD			\
   || (R_TYPE) == BFD_RELOC_AARCH64_TLSDESC_ADD_LO12		\
   || (R_TYPE) == BFD_RELOC_AARCH64_TLSDESC_ADR_PAGE21		\
   || (R_TYPE) == BFD_RELOC_AARCH64_TLSDESC_ADR_PREL21		\
   || (R_TYPE) == BFD_RELOC_AARCH64_TLSDESC_CALL		\
   || (R_TYPE) == BFD_RELOC_AARCH64_TLSDESC_LD32_LO12_NC	\
   || (R_TYPE) == BFD_RELOC_AARCH64_TLSDESC_LD64_LO12		\
   || (R_TYPE) == BFD_RELOC_AARCH64_TLSDESC_LDR			\
   || (R_TYPE) == BFD_RELOC_AARCH64_TLSDESC_LD_PREL19		\
   || (R_TYPE) == BFD_RELOC_AARCH64_TLSDESC_OFF_G0_NC		\
   || (R_TYPE) == BFD_RELOC_AARCH64_TLSDESC_OFF_G1)

#define ELIMINATE_COPY_RELOCS 1

/* Return size of a relocation entry.  HTAB is the bfd's
   elf_aarch64_link_hash_entry.  */
#define RELOC_SIZE(HTAB) (sizeof (Elf64_External_Rela))

/* GOT Entry size - 8 bytes in ELF64 and 4 bytes in ELF32.  */
#define GOT_ENTRY_SIZE			(ARCH_SIZE / 8)
#define PLT_ENTRY_SIZE			(32)
#define PLT_SMALL_ENTRY_SIZE		(16)
#define PLT_TLSDESC_ENTRY_SIZE		(32)

/* Encoding of the nop instruction.  */
#define INSN_NOP 0xd503201f

#define aarch64_compute_jump_table_size(htab)		\
  (((htab)->root.srelplt == NULL) ? 0			\
   : (htab)->root.srelplt->reloc_count * GOT_ENTRY_SIZE)

#define elf_info_to_howto		elf64_aarch64_info_to_howto
#define elf_info_to_howto_rel		elf64_aarch64_info_to_howto

#define AARCH64_ELF_ABI_VERSION		0

/* In case we're on a 32-bit machine, construct a 64-bit "-1" value.  */
#define ALL_ONES (~ (bfd_vma) 0)

#define TARGET_LITTLE_SYM		aarch64_elf64_le_vec
#define TARGET_LITTLE_NAME		"elf64-littleaarch64"

/* The linker script knows the section names for placement.
   The entry_names are used to do simple name mangling on the stubs.
   Given a function name, and its type, the stub can be found. The
   name can be changed. The only requirement is the %s be present.  */
#define STUB_ENTRY_NAME   "__%s_veneer"

/* The name of the dynamic interpreter.  This is put in the .interp
   section.  */
#define ELF_DYNAMIC_INTERPRETER     "/lib/ld.so.1"

#define AARCH64_MAX_FWD_BRANCH_OFFSET \
  (((1 << 25) - 1) << 2)
#define AARCH64_MAX_BWD_BRANCH_OFFSET \
  (-((1 << 25) << 2))

#define AARCH64_MAX_ADRP_IMM ((1 << 20) - 1)
#define AARCH64_MIN_ADRP_IMM (-(1 << 20))

/* Section name for stubs is the associated section name plus this
   string.  */
#define STUB_SUFFIX ".stub"

enum elf_aarch64_stub_type
{
  aarch64_stub_none,
  aarch64_stub_adrp_branch,
  aarch64_stub_long_branch,
  aarch64_stub_erratum_835769_veneer,
  aarch64_stub_erratum_843419_veneer,
};

struct elf_aarch64_stub_hash_entry
{
  /* Base hash table entry structure.  */
  struct bfd_hash_entry root;

  /* The stub section.  */
  asection *stub_sec;

  /* Offset within stub_sec of the beginning of this stub.  */
  bfd_vma stub_offset;

  /* Given the symbol's value and its section we can determine its final
     value when building the stubs (so the stub knows where to jump).  */
  bfd_vma target_value;
  asection *target_section;

  enum elf_aarch64_stub_type stub_type;

  /* The symbol table entry, if any, that this was derived from.  */
  struct elf_aarch64_link_hash_entry *h;

  /* Destination symbol type */
  unsigned char st_type;

  /* Where this stub is being called from, or, in the case of combined
     stub sections, the first input section in the group.  */
  asection *id_sec;

  /* The name for the local symbol at the start of this stub.  The
     stub name in the hash table has to be unique; this does not, so
     it can be friendlier.  */
  char *output_name;

  /* The instruction which caused this stub to be generated (only valid for
     erratum 835769 workaround stubs at present).  */
  uint32_t veneered_insn;

  /* In an erratum 843419 workaround stub, the ADRP instruction offset.  */
  bfd_vma adrp_offset;
};

/* Used to build a map of a section.  This is required for mixed-endian
   code/data.  */

typedef struct elf_elf_section_map
{
  bfd_vma vma;
  char type;
}
elf_aarch64_section_map;


typedef struct _aarch64_elf_section_data
{
  struct bfd_elf_section_data elf;
  unsigned int mapcount;
  unsigned int mapsize;
  elf_aarch64_section_map *map;
}
_aarch64_elf_section_data;

#define elf_aarch64_section_data(sec) \
  ((_aarch64_elf_section_data *) elf_section_data (sec))

/* The size of the thread control block which is defined to be two pointers.  */
#define TCB_SIZE	(ARCH_SIZE/8)*2

struct elf_aarch64_local_symbol
{
  unsigned int got_type;
  bfd_signed_vma got_refcount;
  bfd_vma got_offset;

  /* Offset of the GOTPLT entry reserved for the TLS descriptor. The
     offset is from the end of the jump table and reserved entries
     within the PLTGOT.

     The magic value (bfd_vma) -1 indicates that an offset has not be
     allocated.  */
  bfd_vma tlsdesc_got_jump_table_offset;
};

struct elf_aarch64_obj_tdata
{
  struct elf_obj_tdata root;

  /* local symbol descriptors */
  struct elf_aarch64_local_symbol *locals;

  /* Zero to warn when linking objects with incompatible enum sizes.  */
  int no_enum_size_warning;

  /* Zero to warn when linking objects with incompatible wchar_t sizes.  */
  int no_wchar_size_warning;
};

#define elf_aarch64_tdata(bfd)				\
  ((struct elf_aarch64_obj_tdata *) (bfd)->tdata.any)

#define elf_aarch64_locals(bfd) (elf_aarch64_tdata (bfd)->locals)

#define is_aarch64_elf(bfd)				\
  (bfd_get_flavour (bfd) == bfd_target_elf_flavour	\
   && elf_tdata (bfd) != NULL				\
   && elf_object_id (bfd) == AARCH64_ELF_DATA)

static bfd_boolean
elf64_aarch64_mkobject (bfd *abfd)
{
  return bfd_elf_allocate_object (abfd, sizeof (struct elf_aarch64_obj_tdata),
                  AARCH64_ELF_DATA);
}

#define elf_aarch64_hash_entry(ent) \
  ((struct elf_aarch64_link_hash_entry *)(ent))

#define GOT_UNKNOWN    0
#define GOT_NORMAL     1
#define GOT_TLS_GD     2
#define GOT_TLS_IE     4
#define GOT_TLSDESC_GD 8

#define GOT_TLS_GD_ANY_P(type)	((type & GOT_TLS_GD) || (type & GOT_TLSDESC_GD))

#undef PREV_SEC

#define AARCH64_BITS(x, pos, n) (((x) >> (pos)) & ((1 << (n)) - 1))

#define AARCH64_RT(insn) AARCH64_BITS (insn, 0, 5)
#define AARCH64_RT2(insn) AARCH64_BITS (insn, 10, 5)
#define AARCH64_RA(insn) AARCH64_BITS (insn, 10, 5)
#define AARCH64_RD(insn) AARCH64_BITS (insn, 0, 5)
#define AARCH64_RN(insn) AARCH64_BITS (insn, 5, 5)
#define AARCH64_RM(insn) AARCH64_BITS (insn, 16, 5)

#define AARCH64_MAC(insn) (((insn) & 0xff000000) == 0x9b000000)
#define AARCH64_BIT(insn, n) AARCH64_BITS (insn, n, 1)
#define AARCH64_OP31(insn) AARCH64_BITS (insn, 21, 3)
#define AARCH64_ZR 0x1f

/* All ld/st ops.  See C4-182 of the ARM ARM.  The encoding space for
   LD_PCREL, LDST_RO, LDST_UI and LDST_UIMM cover prefetch ops.  */

#define AARCH64_LD(insn) (AARCH64_BIT (insn, 22) == 1)
#define AARCH64_LDST(insn) (((insn) & 0x0a000000) == 0x08000000)
#define AARCH64_LDST_EX(insn) (((insn) & 0x3f000000) == 0x08000000)
#define AARCH64_LDST_PCREL(insn) (((insn) & 0x3b000000) == 0x18000000)
#define AARCH64_LDST_NAP(insn) (((insn) & 0x3b800000) == 0x28000000)
#define AARCH64_LDSTP_PI(insn) (((insn) & 0x3b800000) == 0x28800000)
#define AARCH64_LDSTP_O(insn) (((insn) & 0x3b800000) == 0x29000000)
#define AARCH64_LDSTP_PRE(insn) (((insn) & 0x3b800000) == 0x29800000)
#define AARCH64_LDST_UI(insn) (((insn) & 0x3b200c00) == 0x38000000)
#define AARCH64_LDST_PIIMM(insn) (((insn) & 0x3b200c00) == 0x38000400)
#define AARCH64_LDST_U(insn) (((insn) & 0x3b200c00) == 0x38000800)
#define AARCH64_LDST_PREIMM(insn) (((insn) & 0x3b200c00) == 0x38000c00)
#define AARCH64_LDST_RO(insn) (((insn) & 0x3b200c00) == 0x38200800)
#define AARCH64_LDST_UIMM(insn) (((insn) & 0x3b000000) == 0x39000000)
#define AARCH64_LDST_SIMD_M(insn) (((insn) & 0xbfbf0000) == 0x0c000000)
#define AARCH64_LDST_SIMD_M_PI(insn) (((insn) & 0xbfa00000) == 0x0c800000)
#define AARCH64_LDST_SIMD_S(insn) (((insn) & 0xbf9f0000) == 0x0d000000)
#define AARCH64_LDST_SIMD_S_PI(insn) (((insn) & 0xbf800000) == 0x0d800000)

/* Add an entry to the code/data map for section SEC.  */

static void
elf64_aarch64_section_map_add (asection *sec, char type, bfd_vma vma)
{
  struct _aarch64_elf_section_data *sec_data =
    elf_aarch64_section_data (sec);
  unsigned int newidx;

  if (sec_data->map == NULL)
    {
      sec_data->map = bfd_malloc (sizeof (elf_aarch64_section_map));
      sec_data->mapcount = 0;
      sec_data->mapsize = 1;
    }

  newidx = sec_data->mapcount++;

  if (sec_data->mapcount > sec_data->mapsize)
    {
      sec_data->mapsize *= 2;
      sec_data->map = bfd_realloc_or_free
    (sec_data->map, sec_data->mapsize * sizeof (elf_aarch64_section_map));
    }

  if (sec_data->map)
    {
      sec_data->map[newidx].vma = vma;
      sec_data->map[newidx].type = type;
    }
}


/* Initialise maps of insn/data for input BFDs.  */
void
bfd_elf64_aarch64_init_maps (bfd *abfd)
{
  Elf_Internal_Sym *isymbuf;
  Elf_Internal_Shdr *hdr;
  unsigned int i, localsyms;

  /* Make sure that we are dealing with an AArch64 elf binary.  */
  if (!is_aarch64_elf (abfd))
    return;

  if ((abfd->flags & DYNAMIC) != 0)
   return;

  hdr = &elf_symtab_hdr (abfd);
  localsyms = hdr->sh_info;

  /* Obtain a buffer full of symbols for this BFD. The hdr->sh_info field
     should contain the number of local symbols, which should come before any
     global symbols.  Mapping symbols are always local.  */
  isymbuf = bfd_elf_get_elf_syms (abfd, hdr, localsyms, 0, NULL, NULL, NULL);

  /* No internal symbols read?  Skip this BFD.  */
  if (isymbuf == NULL)
    return;

  for (i = 0; i < localsyms; i++)
    {
      Elf_Internal_Sym *isym = &isymbuf[i];
      asection *sec = bfd_section_from_elf_index (abfd, isym->st_shndx);
      const char *name;

      if (sec != NULL && ELF_ST_BIND (isym->st_info) == STB_LOCAL)
    {
      name = bfd_elf_string_from_elf_section (abfd,
                          hdr->sh_link,
                          isym->st_name);

      if (bfd_is_aarch64_special_symbol_name
          (name, BFD_AARCH64_SPECIAL_SYM_TYPE_MAP))
        elf64_aarch64_section_map_add (sec, name[1], isym->st_value);
    }
    }
}

/* Structure to hold payload for _bfd_aarch64_erratum_843419_clear_stub,
   it is used to identify the stub information to reset.  */

struct erratum_843419_branch_to_stub_clear_data
{
  bfd_vma adrp_offset;
  asection *output_section;
};

/* Function to keep AArch64 specific flags in the ELF header.  */

static bfd_boolean
elf64_aarch64_set_private_flags (bfd *abfd, flagword flags)
{
  if (elf_flags_init (abfd) && elf_elfheader (abfd)->e_flags != flags)
    {
    }
  else
    {
      elf_elfheader (abfd)->e_flags = flags;
      elf_flags_init (abfd) = TRUE;
    }

  return TRUE;
}

/* Treat mapping symbols as special target symbols.  */

static bfd_boolean
elf64_aarch64_is_target_special_symbol (bfd *abfd ATTRIBUTE_UNUSED,
                    asymbol *sym)
{
  return bfd_is_aarch64_special_symbol_name (sym->name,
                         BFD_AARCH64_SPECIAL_SYM_TYPE_ANY);
}

/* This is a copy of elf_find_function () from elf.c except that
   AArch64 mapping symbols are ignored when looking for function names.  */

static bfd_boolean
aarch64_elf_find_function (bfd *abfd ATTRIBUTE_UNUSED,
               asymbol **symbols,
               asection *section,
               bfd_vma offset,
               const char **filename_ptr,
               const char **functionname_ptr)
{
  const char *filename = NULL;
  asymbol *func = NULL;
  bfd_vma low_func = 0;
  asymbol **p;

  for (p = symbols; *p != NULL; p++)
    {
      elf_symbol_type *q;

      q = (elf_symbol_type *) * p;

      switch (ELF_ST_TYPE (q->internal_elf_sym.st_info))
    {
    default:
      break;
    case STT_FILE:
      filename = bfd_asymbol_name (&q->symbol);
      break;
    case STT_FUNC:
    case STT_NOTYPE:
      /* Skip mapping symbols.  */
      if ((q->symbol.flags & BSF_LOCAL)
          && (bfd_is_aarch64_special_symbol_name
          (q->symbol.name, BFD_AARCH64_SPECIAL_SYM_TYPE_ANY)))
        continue;
      /* Fall through.  */
      if (bfd_get_section (&q->symbol) == section
          && q->symbol.value >= low_func && q->symbol.value <= offset)
        {
          func = (asymbol *) q;
          low_func = q->symbol.value;
        }
      break;
    }
    }

  if (func == NULL)
    return FALSE;

  if (filename_ptr)
    *filename_ptr = filename;
  if (functionname_ptr)
    *functionname_ptr = bfd_asymbol_name (func);

  return TRUE;
}


/* Find the nearest line to a particular section and offset, for error
   reporting.   This code is a duplicate of the code in elf.c, except
   that it uses aarch64_elf_find_function.  */

static bfd_boolean
elf64_aarch64_find_nearest_line (bfd *abfd,
                 asymbol **symbols,
                 asection *section,
                 bfd_vma offset,
                 const char **filename_ptr,
                 const char **functionname_ptr,
                 unsigned int *line_ptr,
                 unsigned int *discriminator_ptr)
{
  bfd_boolean found = FALSE;

  if (_bfd_dwarf2_find_nearest_line (abfd, symbols, NULL, section, offset,
                     filename_ptr, functionname_ptr,
                     line_ptr, discriminator_ptr,
                     dwarf_debug_sections, 0,
                     &elf_tdata (abfd)->dwarf2_find_line_info))
    {
      if (!*functionname_ptr)
    aarch64_elf_find_function (abfd, symbols, section, offset,
                   *filename_ptr ? NULL : filename_ptr,
                   functionname_ptr);

      return TRUE;
    }

  /* Skip _bfd_dwarf1_find_nearest_line since no known AArch64
     toolchain uses DWARF1.  */

  if (!_bfd_stab_section_find_nearest_line (abfd, symbols, section, offset,
                        &found, filename_ptr,
                        functionname_ptr, line_ptr,
                        &elf_tdata (abfd)->line_info))
    return FALSE;

  if (found && (*functionname_ptr || *line_ptr))
    return TRUE;

  if (symbols == NULL)
    return FALSE;

  if (!aarch64_elf_find_function (abfd, symbols, section, offset,
                  filename_ptr, functionname_ptr))
    return FALSE;

  *line_ptr = 0;
  return TRUE;
}

static bfd_boolean
elf64_aarch64_find_inliner_info (bfd *abfd,
                 const char **filename_ptr,
                 const char **functionname_ptr,
                 unsigned int *line_ptr)
{
  bfd_boolean found;
  found = _bfd_dwarf2_find_inliner_info
    (abfd, filename_ptr,
     functionname_ptr, line_ptr, &elf_tdata (abfd)->dwarf2_find_line_info);
  return found;
}

/* A structure used to record a list of sections, independently
   of the next and prev fields in the asection structure.  */
typedef struct section_list
{
  asection *sec;
  struct section_list *next;
  struct section_list *prev;
}
section_list;

/* Unfortunately we need to keep a list of sections for which
   an _aarch64_elf_section_data structure has been allocated.  This
   is because it is possible for functions like elf64_aarch64_write_section
   to be called on a section which has had an elf_data_structure
   allocated for it (and so the used_by_bfd field is valid) but
   for which the AArch64 extended version of this structure - the
   _aarch64_elf_section_data structure - has not been allocated.  */
static section_list *sections_with_aarch64_elf_section_data = NULL;

static void
record_section_with_aarch64_elf_section_data (asection *sec)
{
  struct section_list *entry;

  entry = bfd_malloc (sizeof (*entry));
  if (entry == NULL)
    return;
  entry->sec = sec;
  entry->next = sections_with_aarch64_elf_section_data;
  entry->prev = NULL;
  if (entry->next != NULL)
    entry->next->prev = entry;
  sections_with_aarch64_elf_section_data = entry;
}

static struct section_list *
find_aarch64_elf_section_entry (asection *sec)
{
  struct section_list *entry;
  static struct section_list *last_entry = NULL;

  /* This is a short cut for the typical case where the sections are added
     to the sections_with_aarch64_elf_section_data list in forward order and
     then looked up here in backwards order.  This makes a real difference
     to the ld-srec/sec64k.exp linker test.  */
  entry = sections_with_aarch64_elf_section_data;
  if (last_entry != NULL)
    {
      if (last_entry->sec == sec)
    entry = last_entry;
      else if (last_entry->next != NULL && last_entry->next->sec == sec)
    entry = last_entry->next;
    }

  for (; entry; entry = entry->next)
    if (entry->sec == sec)
      break;

  if (entry)
    /* Record the entry prior to this one - it is the entry we are
       most likely to want to locate next time.  Also this way if we
       have been called from
       unrecord_section_with_aarch64_elf_section_data () we will not
       be caching a pointer that is about to be freed.  */
    last_entry = entry->prev;

  return entry;
}

static void
unrecord_section_with_aarch64_elf_section_data (asection *sec)
{
  struct section_list *entry;

  entry = find_aarch64_elf_section_entry (sec);

  if (entry)
    {
      if (entry->prev != NULL)
    entry->prev->next = entry->next;
      if (entry->next != NULL)
    entry->next->prev = entry->prev;
      if (entry == sections_with_aarch64_elf_section_data)
    sections_with_aarch64_elf_section_data = entry->next;
      free (entry);
    }
}


typedef struct
{
  void *finfo;
  struct bfd_link_info *info;
  asection *sec;
  int sec_shndx;
  int (*func) (void *, const char *, Elf_Internal_Sym *,
           asection *, struct elf_link_hash_entry *);
} output_arch_syminfo;

enum map_symbol_type
{
  AARCH64_MAP_INSN,
  AARCH64_MAP_DATA
};

/* Allocate target specific section data.  */

static bfd_boolean
elf64_aarch64_new_section_hook (bfd *abfd, asection *sec)
{
  if (!sec->used_by_bfd)
    {
      _aarch64_elf_section_data *sdata;
      bfd_size_type amt = sizeof (*sdata);

      sdata = bfd_zalloc (abfd, amt);
      if (sdata == NULL)
    return FALSE;
      sec->used_by_bfd = sdata;
    }

  record_section_with_aarch64_elf_section_data (sec);

  return _bfd_elf_new_section_hook (abfd, sec);
}


static void
unrecord_section_via_map_over_sections (bfd *abfd ATTRIBUTE_UNUSED,
                    asection *sec,
                    void *ignore ATTRIBUTE_UNUSED)
{
  unrecord_section_with_aarch64_elf_section_data (sec);
}

static bfd_boolean
elf64_aarch64_close_and_cleanup (bfd *abfd)
{
  if (abfd->sections)
    bfd_map_over_sections (abfd,
               unrecord_section_via_map_over_sections, NULL);

  return _bfd_elf_close_and_cleanup (abfd);
}

static bfd_boolean
elf64_aarch64_bfd_free_cached_info (bfd *abfd)
{
  if (abfd->sections)
    bfd_map_over_sections (abfd,
               unrecord_section_via_map_over_sections, NULL);

  return _bfd_free_cached_info (abfd);
}

#define ELF_ARCH			bfd_arch_aarch64
#define ELF_MACHINE_CODE		EM_AARCH64
#define ELF_MAXPAGESIZE			0x10000
#define ELF_MINPAGESIZE			0x1000
#define ELF_COMMONPAGESIZE		0x1000

#define bfd_elf64_close_and_cleanup		\
  elf64_aarch64_close_and_cleanup

#define bfd_elf64_bfd_free_cached_info		\
  elf64_aarch64_bfd_free_cached_info

#define bfd_elf64_bfd_is_target_special_symbol	\
  elf64_aarch64_is_target_special_symbol

#define bfd_elf64_bfd_link_hash_table_create	\
  elf64_aarch64_link_hash_table_create

#define bfd_elf64_bfd_reloc_type_lookup		\
  elf64_aarch64_reloc_type_lookup

#define bfd_elf64_bfd_reloc_name_lookup		\
  elf64_aarch64_reloc_name_lookup

#define bfd_elf64_bfd_set_private_flags		\
  elf64_aarch64_set_private_flags

#define bfd_elf64_find_inliner_info		\
  elf64_aarch64_find_inliner_info

#define bfd_elf64_find_nearest_line		\
  elf64_aarch64_find_nearest_line

#define bfd_elf64_mkobject			\
  elf64_aarch64_mkobject

#define bfd_elf64_new_section_hook		\
  elf64_aarch64_new_section_hook

#define elf_backend_adjust_dynamic_symbol	\
  0

#define elf_backend_always_size_sections	\
  0

#define elf_backend_check_relocs		\
  0

#define elf_backend_copy_indirect_symbol	\
  elf64_aarch64_copy_indirect_symbol

/* Create .dynbss, and .rela.bss sections in DYNOBJ, and set up shortcuts
   to them in our hash.  */
#define elf_backend_create_dynamic_sections	\
  elf64_aarch64_create_dynamic_sections

#define elf_backend_init_index_section		\
  _bfd_elf_init_2_index_sections

#define elf_backend_finish_dynamic_sections	\
  elf64_aarch64_finish_dynamic_sections

#define elf_backend_finish_dynamic_symbol	\
  elf64_aarch64_finish_dynamic_symbol

#define elf_backend_object_p			\
  elf64_aarch64_object_p

#define elf_backend_output_arch_local_syms	\
  elf64_aarch64_output_arch_local_syms

#define elf_backend_plt_sym_val			\
  elf64_aarch64_plt_sym_val

#define elf_backend_post_process_headers	\
  elf64_aarch64_post_process_headers

#define elf_backend_relocate_section		\
  elf64_aarch64_relocate_section

#define elf_backend_reloc_type_class		\
  elf64_aarch64_reloc_type_class

#define elf_backend_section_from_shdr		\
  elf64_aarch64_section_from_shdr

#define elf_backend_size_dynamic_sections	\
  elf64_aarch64_size_dynamic_sections

#define elf_backend_size_info			\
  elf64_aarch64_size_info

#define elf_backend_write_section		\
  elf64_aarch64_write_section

#define elf_backend_symbol_processing		\
  elf64_aarch64_backend_symbol_processing

#define elf_backend_can_refcount       1
#define elf_backend_can_gc_sections    1
#define elf_backend_plt_readonly       1
#define elf_backend_want_got_plt       1
#define elf_backend_want_plt_sym       0
#define elf_backend_want_dynrelro      1
#define elf_backend_may_use_rel_p      0
#define elf_backend_may_use_rela_p     1
#define elf_backend_default_use_rela_p 1
#define elf_backend_rela_normal	       1
#define elf_backend_dtrel_excludes_plt 1
#define elf_backend_got_header_size (GOT_ENTRY_SIZE * 3)
#define elf_backend_default_execstack  0
#define elf_backend_extern_protected_data 1
#define elf_backend_hash_symbol 1

#undef	elf_backend_obj_attrs_section
#define elf_backend_obj_attrs_section		".ARM.attributes"


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
#define bfd_elf64_find_line		_bfd_nosymbols_find_line
#endif
#ifndef bfd_elf64_find_inliner_info
#define bfd_elf64_find_inliner_info	_bfd_nosymbols_find_inliner_info
#endif
#define bfd_elf64_read_minisymbols	_bfd_elf_read_minisymbols
#define bfd_elf64_minisymbol_to_symbol	_bfd_elf_minisymbol_to_symbol
#define bfd_elf64_get_dynamic_symtab_upper_bound \
  _bfd_elf_get_dynamic_symtab_upper_bound
#define bfd_elf64_get_lineno		_bfd_nosymbols_get_lineno
#ifndef bfd_elf64_get_reloc_upper_bound
#define bfd_elf64_get_reloc_upper_bound _bfd_elf_get_reloc_upper_bound
#endif
#ifndef bfd_elf64_get_symbol_info
#define bfd_elf64_get_symbol_info	_bfd_nosymbols_get_symbol_info
#endif
#ifndef bfd_elf64_get_symbol_version_string
#define bfd_elf64_get_symbol_version_string \
  _bfd_nosymbols_get_symbol_version_string
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
  _bfd_generic_bfd_copy_private_symbol_data
#endif

#ifndef bfd_elf64_bfd_copy_private_section_data
#define bfd_elf64_bfd_copy_private_section_data \
  _bfd_generic_bfd_copy_private_section_data
#endif
#ifndef bfd_elf64_bfd_copy_private_header_data
#define bfd_elf64_bfd_copy_private_header_data \
  _bfd_generic_bfd_copy_private_header_data
#endif
#ifndef bfd_elf64_bfd_copy_private_bfd_data
#define bfd_elf64_bfd_copy_private_bfd_data \
  _bfd_generic_bfd_copy_private_bfd_data
#endif
#ifndef bfd_elf64_bfd_print_private_bfd_data
#define bfd_elf64_bfd_print_private_bfd_data \
  _bfd_generic_bfd_print_private_bfd_data
#endif
#ifndef bfd_elf64_bfd_merge_private_bfd_data
#define bfd_elf64_bfd_merge_private_bfd_data _bfd_bool_bfd_link_true
#endif
#ifndef bfd_elf64_bfd_set_private_flags
#define bfd_elf64_bfd_set_private_flags _bfd_bool_bfd_uint_true
#endif
#ifndef bfd_elf64_bfd_is_local_label_name
#define bfd_elf64_bfd_is_local_label_name bfd_generic_is_local_label_name
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
#define bfd_elf64_print_symbol _bfd_nosymbols_print_symbol
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
    _bfd_dummy_target		/* unknown format */
  },

  /* bfd_set_format: set the format of a file being written */
  { _bfd_bool_bfd_false_error,
    bfd_elf64_mkobject,
    _bfd_bool_bfd_false_error,
    _bfd_bool_bfd_false_error
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
  NULL
};
