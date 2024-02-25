TEMPLATE = lib
CONFIG += staticlib

QT -= gui core

SOURCES += \
        kmp_affinity.cpp \
        kmp_alloc.cpp \
        kmp_atomic.cpp \
        kmp_barrier.cpp \
        kmp_cancel.cpp \
        kmp_csupport.cpp \
        kmp_debug.cpp \
        kmp_dispatch.cpp \
        kmp_environment.cpp \
        kmp_error.cpp \
        kmp_ftn_cdecl.cpp \
        kmp_ftn_extra.cpp \
        kmp_global.cpp \
        kmp_gsupport.cpp \
        kmp_i18n.cpp \
        kmp_io.cpp \
        kmp_lock.cpp \
        kmp_runtime.cpp \
        kmp_sched.cpp \
        kmp_settings.cpp \
        kmp_str.cpp \
        kmp_taskdeps.cpp \
        kmp_tasking.cpp \
        kmp_threadprivate.cpp \
        kmp_utility.cpp \
        kmp_version.cpp \
        kmp_wait_release.cpp \
        z_Linux_util.cpp \
        z_Linux_asm.S

HEADERS += \
        kmp.h \
        kmp_affinity.h \
        kmp_atomic.h \
        kmp_config.h \
        kmp_debug.h \
        kmp_dispatch.h \
        kmp_environment.h \
        kmp_error.h \
        kmp_ftn_entry.h \
        kmp_ftn_os.h \
        kmp_i18n.h \
        kmp_io.h \
        kmp_itt.h \
        kmp_lock.h \
        kmp_omp.h \
        kmp_os.h \
        kmp_platform.h \
        kmp_safe_c_api.h \
        kmp_settings.h \
        kmp_stats.h \
        kmp_str.h \
        kmp_taskdeps.h \
        kmp_version.h \
        kmp_wait_release.h \
        kmp_wrapper_getpid.h \
        kmp_wrapper_malloc.h \
        omp.h

macos {
    CONFIG(debug, debug|release) {
        contains(QT_ARCH, x86_64) {
            QMAKE_APPLE_DEVICE_ARCHS=x86_64h
        }
        contains(QT_ARCH, arm64) {
            QMAKE_APPLE_DEVICE_ARCHS=arm64
        }
    }
}

QMAKE_CXXFLAGS += -std=gnu++11 -O3 -DNDEBUG -D _GNU_SOURCE -D _REENTRANT \
                  -fno-exceptions -fno-rtti -w

QMAKE_CXXFLAGS_RELEASE -= -O2
QMAKE_CXXFLAGS_RELEASE += -g1
QMAKE_CXXFLAGS_DEBUG += -g
