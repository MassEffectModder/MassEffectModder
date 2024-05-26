/* config.h.  Generated from config.h.in by configure.  */
/* config.h.in.  Generated from configure.ac by autoheader.  */

/* Define to 1 if you have the <dlfcn.h> header file. */
#define HAVE_DLFCN_H 1

/* Define to 1 if you have the 'feenableexcept' function. */
/* #undef HAVE_FEENABLEEXCEPT */

/* Define to 1 if you have the <inttypes.h> header file. */
#define HAVE_INTTYPES_H 1

/* Define to 1 if you have the 'm' library (-lm). */
/* #undef HAVE_LIBM */

/* Define to 1 if you have the 'z' library (-lz). */
#define HAVE_LIBZ 1

/* Define to 1 if you have the 'pow' function. */
#define HAVE_POW 1

/* Define to 1 if you have the <stdint.h> header file. */
#define HAVE_STDINT_H 1

/* Define to 1 if you have the <stdio.h> header file. */
#define HAVE_STDIO_H 1

/* Define to 1 if you have the <stdlib.h> header file. */
#define HAVE_STDLIB_H 1

/* Define to 1 if you have the <strings.h> header file. */
#define HAVE_STRINGS_H 1

/* Define to 1 if you have the <string.h> header file. */
#define HAVE_STRING_H 1

/* Define to 1 if you have the <sys/stat.h> header file. */
#define HAVE_SYS_STAT_H 1

/* Define to 1 if you have the <sys/types.h> header file. */
#define HAVE_SYS_TYPES_H 1

/* Define to 1 if you have the <unistd.h> header file. */
#define HAVE_UNISTD_H 1

/* Define to the sub-directory where libtool stores uninstalled libraries. */
#define LT_OBJDIR ".libs/"

/* Name of package */
#define PACKAGE "libpng"

/* Define to the address where bug reports for this package should be sent. */
#define PACKAGE_BUGREPORT "png-mng-implement@lists.sourceforge.net"

/* Define to the full name of this package. */
#define PACKAGE_NAME "libpng"

/* Define to the full name and version of this package. */
#define PACKAGE_STRING "libpng 1.6.43"

/* Define to the one symbol short name of this package. */
#define PACKAGE_TARNAME "libpng"

/* Define to the home page for this package. */
#define PACKAGE_URL ""

/* Define to the version of this package. */
#define PACKAGE_VERSION "1.6.43"

/* Turn on ARM Neon optimizations at run-time */
/* #undef PNG_ARM_NEON_API_SUPPORTED */

/* Check for ARM Neon support at run-time */
/* #undef PNG_ARM_NEON_CHECK_SUPPORTED */

/* Enable ARM Neon optimizations */
#if defined(__aarch64__)
#define PNG_ARM_NEON_OPT 2
#endif

/* Enable Intel SSE optimizations */
#if defined(__x86_64__)
#define PNG_INTEL_SSE_OPT 1
#endif

/* Enable LOONGARCH LSX optimizations */
/* #undef PNG_LOONGARCH_LSX_OPT */

/* Turn on MIPS MMI optimizations at run-time */
/* #undef PNG_MIPS_MMI_API_SUPPORTED */

/* Check for MIPS MMI support at run-time */
/* #undef PNG_MIPS_MMI_CHECK_SUPPORTED */

/* Enable MIPS MMI optimizations */
/* #undef PNG_MIPS_MMI_OPT */

/* Turn on MIPS MSA optimizations at run-time */
/* #undef PNG_MIPS_MSA_API_SUPPORTED */

/* Check for MIPS MSA support at run-time */
/* #undef PNG_MIPS_MSA_CHECK_SUPPORTED */

/* Enable MIPS MSA optimizations */
/* #undef PNG_MIPS_MSA_OPT */

/* Turn on POWERPC VSX optimizations at run-time */
/* #undef PNG_POWERPC_VSX_API_SUPPORTED */

/* Check for POWERPC VSX support at run-time */
/* #undef PNG_POWERPC_VSX_CHECK_SUPPORTED */

/* Enable POWERPC VSX optimizations */
/* #undef PNG_POWERPC_VSX_OPT */

/* Define to 1 if all of the C89 standard headers exist (not just the ones
   required in a freestanding environment). This macro is provided for
   backward compatibility; new code need not use it. */
#define STDC_HEADERS 1

/* Define to 1 if your <sys/time.h> declares 'struct tm'. */
/* #undef TM_IN_SYS_TIME */

/* Version number of package */
#define VERSION "1.6.43"

/* Define to the equivalent of the C99 'restrict' keyword, or to
   nothing if this is not supported.  Do not define if restrict is
   supported only directly.  */
#define restrict __restrict__
/* Work around a bug in older versions of Sun C++, which did not
   #define __restrict__ or support _Restrict or __restrict__
   even though the corresponding Sun C compiler ended up with
   "#define restrict _Restrict" or "#define restrict __restrict__"
   in the previous line.  This workaround can be removed once
   we assume Oracle Developer Studio 12.5 (2016) or later.  */
#if defined __SUNPRO_CC && !defined __RESTRICT && !defined __restrict__
# define _Restrict
# define __restrict__
#endif
