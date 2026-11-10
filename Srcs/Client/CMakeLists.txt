cmake_minimum_required(VERSION 3.10.2)
project("WM2Mobile")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsigned-char -fms-extensions -fno-autolink -ferror-limit=0 -Wno-pragma-pack -Wno-invalid-token-paste -Wno-register")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsigned-char -fms-extensions -fno-autolink -ferror-limit=0 -Wno-pragma-pack -Wno-invalid-token-paste")

add_definitions(-DENABLE_OPENGL)
add_definitions(-DENABLE_MOBILE_CONTROLS)
add_definitions(-DLOCALE_SERVICE_EUROPE)
add_definitions(-DBUILDING_GRANNY_STATIC=1)
add_definitions(-DCRYPTOPP_DISABLE_ASM -D_LIBCPP_ENABLE_CXX17_REMOVED_FEATURES=1 -D_LIBCPP_ENABLE_CXX17_REMOVED_UNARY_BINARY_FUNCTION=1)

if(ANDROID)
    add_definitions(-DANDROID -D__ANDROID__)
elseif(APPLE)
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/android_compat)
    add_definitions(-DANDROID)
endif()

# 1. Build Granny 3D from Extern/granny/source
add_subdirectory(
    ${CMAKE_CURRENT_SOURCE_DIR}/../Extern/granny/source
    ${CMAKE_CURRENT_BINARY_DIR}/granny2_build
)

if(TARGET granny2_static)
    target_compile_definitions(granny2_static PRIVATE CONTAINER_INTPTR=intptr_t)
    if(ANDROID OR APPLE)
        # Remove Windows winxx include path if present on Android target
        get_target_property(GRANNY_INCLUDES granny2_static INCLUDE_DIRECTORIES)
        if(GRANNY_INCLUDES)
            list(FILTER GRANNY_INCLUDES EXCLUDE REGEX "winxx")
            set_target_properties(granny2_static PROPERTIES INCLUDE_DIRECTORIES "${GRANNY_INCLUDES}")
        endif()
    endif()
endif()

# 2. Build Crypto++ from Extern/cryptopp
file(GLOB CRYPTOPP_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/../Extern/cryptopp/*.cpp")
list(FILTER CRYPTOPP_SOURCES EXCLUDE REGEX ".*(test|bench|validat|fipstest|dlltest|adhoc).*\\.cpp$")
if(NOT ANDROID_ABI MATCHES "arm" AND NOT APPLE)
    list(FILTER CRYPTOPP_SOURCES EXCLUDE REGEX ".*(_neon|_armv4).*\\.cpp$")
endif()
add_library(cryptopp_static STATIC ${CRYPTOPP_SOURCES})
target_include_directories(cryptopp_static PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../Extern/cryptopp)

# 3. Build Python 2.7 from Extern/Python-2.7.18
set(PYTHON_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../Extern/Python-2.7.18")
if(EXISTS "${PYTHON_SRC_DIR}/Python-2.7.18/Python")
    set(PYTHON_SRC_DIR "${PYTHON_SRC_DIR}/Python-2.7.18")
endif()

set(PY_GEN_DIR "${CMAKE_CURRENT_BINARY_DIR}/python27_gen")
file(MAKE_DIRECTORY "${PY_GEN_DIR}")

# Generate Android NDK pyconfig.h for Python 2.7
file(WRITE "${PY_GEN_DIR}/pyconfig.h" "
#ifndef Py_PYCONFIG_H
#define Py_PYCONFIG_H

#define HAVE_PROTOTYPES 1
#define STDC_HEADERS 1
#define HAVE_STDARG_PROTOTYPES 1
#define HAVE_UNISTD_H 1
#define HAVE_SYS_TYPES_H 1
#define HAVE_SYS_STAT_H 1
#define HAVE_SYS_TIME_H 1
#define HAVE_SYS_WAIT_H 1
#define HAVE_SYS_SELECT_H 1
#define HAVE_DIRENT_H 1
#define HAVE_FCNTL_H 1
#define HAVE_SIGNAL_H 1
#define HAVE_STDINT_H 1
#define HAVE_STDDEF_H 1
#define HAVE_STDLIB_H 1
#define HAVE_STRING_H 1
#define HAVE_MEMORY_H 1
#define HAVE_INTTYPES_H 1
#define HAVE_LIMITS_H 1
#define HAVE_LOCALE_H 1
#define HAVE_ERRNO_H 1
#define HAVE_UTIME_H 1
#define HAVE_PTHREAD_H 1
#define HAVE_DLFCN_H 1
#define HAVE_WCHAR_H 1
// FixiOSSystem
#ifdef __APPLE__
#include <stdlib.h>
#undef system
#define system(cmd) (-1)
#endif

#define _REENTRANT 1
#define WITH_THREAD 1
#define _POSIX_THREADS 1
#define WITH_DOC_STRINGS 1
#define WITH_PYMALLOC 1
#define Py_USING_UNICODE 1
#define Py_UNICODE_SIZE 2
#define PY_UNICODE_TYPE unsigned short
#define Py_UNICODE_TYPE unsigned short
#define DOUBLE_IS_LITTLE_ENDIAN_IEEE754 1

#define HAVE_SSIZE_T 1
#define HAVE_UINTPTR_T 1
#define HAVE_LONG_LONG 1
#define HAVE_UINT32_T 1
#define HAVE_UINT64_T 1
#define HAVE_INT32_T 1
#define HAVE_INT64_T 1
#define PY_UINT32_T uint32_t
#define PY_UINT64_T uint64_t
#define PY_INT32_T int32_t
#define PY_INT64_T int64_t
#define PY_FORMAT_LONG_LONG \"ll\"
#define PY_FORMAT_SIZE_T \"z\"
#define HAVE_DEV_URANDOM 1
#define HAVE_TZNAME 1
#define HAVE_GETTIMEOFDAY 1
#define HAVE_SELECT 1
#define HAVE_STAT 1
#define HAVE_FSTAT 1
#define HAVE_LSTAT 1
#define HAVE_SYMLINK 1
#define HAVE_READLINK 1
#define HAVE_GETCWD 1
#define HAVE_STRDUP 1
#define HAVE_STRERROR 1
#define HAVE_MEMMOVE 1
#define HAVE_HYPOT 1
#define HAVE_COPYSIGN 1
#define HAVE_ROUND 1
#define HAVE_ACOSH 1
#define HAVE_ASINH 1
#define HAVE_ATANH 1
#define HAVE_ERF 1
#define HAVE_ERFC 1
#define HAVE_EXPM1 1
#define HAVE_FINITE 1
#define HAVE_GAMMA 1
#define HAVE_LGAMMA 1
#define HAVE_LOG1P 1
#define HAVE_DECL_ISINF 1
#define HAVE_DECL_ISNAN 1
#define HAVE_DECL_ISFINITE 1
#define TIME_WITH_SYS_TIME 1
#define SYS_SELECT_WITH_SYS_TIME 1

#define SIZEOF_SHORT 2
#define SIZEOF_INT 4
#define SIZEOF_PID_T 4
#define SIZEOF_WCHAR_T 4
#define SIZEOF_LONG_LONG 8
#define SIZEOF_FLOAT 4
#define SIZEOF_DOUBLE 8
#define SIZEOF_FPOS_T 8

#if defined(__LP64__) || defined(_LP64) || defined(__aarch64__)
#define SIZEOF_LONG 8
#define SIZEOF_VOID_P 8
#define SIZEOF_SIZE_T 8
#define SIZEOF_TIME_T 8
#define SIZEOF_OFF_T 8
#define SIZEOF_PTHREAD_T 8
#else
#define SIZEOF_LONG 4
#define SIZEOF_VOID_P 4
#define SIZEOF_SIZE_T 4
#define SIZEOF_TIME_T 4
#define SIZEOF_OFF_T 4
#define SIZEOF_PTHREAD_T 4
#endif

#endif /* Py_PYCONFIG_H */
")

file(REMOVE_RECURSE "${PY_GEN_DIR}/Python27")
file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/../Extern/include/Python27/" DESTINATION "${PY_GEN_DIR}/Python27")
file(COPY "${PY_GEN_DIR}/pyconfig.h" DESTINATION "${PY_GEN_DIR}/Python27")
file(READ "${PYTHON_SRC_DIR}/Objects/classobject.c" CLASSOBJ_CONTENT)
string(REPLACE "_PyObject_GC_UNTRACK(inst);" "if (_Py_AS_GC(inst)->gc.gc_refs == _PyGC_REFS_UNTRACKED) return;\n    _PyObject_GC_UNTRACK(inst);" CLASSOBJ_CONTENT "${CLASSOBJ_CONTENT}")
file(WRITE "${PYTHON_SRC_DIR}/Objects/classobject.c" "${CLASSOBJ_CONTENT}")

file(READ "${CMAKE_CURRENT_SOURCE_DIR}/../Extern/include/Python27/Python.h" PY_H_CONTENT)
file(WRITE "${PY_GEN_DIR}/Python27/Python.h" "${PY_H_CONTENT}
#if defined(__cplusplus) && (defined(__LP64__) || defined(_LP64) || defined(__aarch64__))
inline int PyObject_AsCharBuffer(PyObject *obj, const char **buffer, int *buffer_len) {
    Py_ssize_t tmp_len = buffer_len ? *buffer_len : 0;
    int res = PyObject_AsCharBuffer(obj, buffer, &tmp_len);
    if (buffer_len) *buffer_len = (int)tmp_len;
    return res;
}
inline int PyObject_AsReadBuffer(PyObject *obj, const void **buffer, int *buffer_len) {
    Py_ssize_t tmp_len = buffer_len ? *buffer_len : 0;
    int res = PyObject_AsReadBuffer(obj, buffer, &tmp_len);
    if (buffer_len) *buffer_len = (int)tmp_len;
    return res;
}
inline int PyObject_AsWriteBuffer(PyObject *obj, void **buffer, int *buffer_len) {
    Py_ssize_t tmp_len = buffer_len ? *buffer_len : 0;
    int res = PyObject_AsWriteBuffer(obj, buffer, &tmp_len);
    if (buffer_len) *buffer_len = (int)tmp_len;
    return res;
}
inline int PyString_AsStringAndSize(PyObject *obj, char **s, int *len) {
    Py_ssize_t tmp_len = len ? *len : 0;
    int res = PyString_AsStringAndSize(obj, s, &tmp_len);
    if (len) *len = (int)tmp_len;
    return res;
}
inline int PyDict_Next(PyObject *mp, int *pos, PyObject **key, PyObject **value) {
    Py_ssize_t tmp_pos = pos ? *pos : 0;
    int res = PyDict_Next(mp, &tmp_pos, key, value);
    if (pos) *pos = (int)tmp_pos;
    return res;
}
static inline PyObject* _Py_BuildValue_Orig(const char* format, ...) {
    va_list va;
    va_start(va, format);
    PyObject* retval = Py_VaBuildValue((char*)format, va);
    va_end(va);
    return retval;
}
template <typename T>
inline PyObject* Py_BuildValue(const char* format, T* ptr) {
    if (format && (format[0] == 'i' || format[0] == 'l') && format[1] == '\\0') {
        return PyLong_FromVoidPtr((void*)ptr);
    }
    return _Py_BuildValue_Orig(format, ptr);
}
#endif
")

# Generate config.c for built-in modules and 32/64-bit Py_InitModule4 bridge
file(WRITE "${PY_GEN_DIR}/config.c" "
#include \"Python.h\"

#ifdef __cplusplus
extern \"C\" {
#endif

extern void initposix(void);
extern void initposixpath(void);
extern void initerrno(void);
extern void init_sre(void);
extern void init_codecs(void);
extern void init_weakref(void);
extern void initzipimport(void);
extern void init_symtable(void);
extern void initxxsubtype(void);
extern void initmath(void);
extern void inittime(void);
extern void initoperator(void);
extern void init_random(void);
extern void init_struct(void);
extern void inititertools(void);
extern void initstrop(void);
extern void initarray(void);
extern void initcmath(void);
extern void initcStringIO(void);
extern void initcPickle(void);
extern void initbinascii(void);
extern void init_functools(void);
extern void init_collections(void);
extern void initthread(void);
extern void initsignal(void);
extern void initzlib(void);
extern void PyMarshal_Init(void);
extern void initimp(void);
extern void initgc(void);
extern void _PyWarnings_Init(void);

struct _inittab _PyImport_Inittab[] = {
    {\"posix\", initposix},
    {\"posixpath\", initposixpath},
    {\"errno\", initerrno},
    {\"_sre\", init_sre},
    {\"_codecs\", init_codecs},
    {\"_weakref\", init_weakref},
    {\"zipimport\", initzipimport},
    {\"_symtable\", init_symtable},
    {\"xxsubtype\", initxxsubtype},
    {\"math\", initmath},
    {\"time\", inittime},
    {\"operator\", initoperator},
    {\"_random\", init_random},
    {\"_struct\", init_struct},
    {\"itertools\", inititertools},
    {\"strop\", initstrop},
    {\"array\", initarray},
    {\"cmath\", initcmath},
    {\"cStringIO\", initcStringIO},
    {\"cPickle\", initcPickle},
    {\"binascii\", initbinascii},
    {\"_functools\", init_functools},
    {\"_collections\", init_collections},
    {\"thread\", initthread},
    {\"signal\", initsignal},
    {\"zlib\", initzlib},
    {\"marshal\", PyMarshal_Init},
    {\"imp\", initimp},
    {\"__main__\", NULL},
    {\"__builtin__\", NULL},
    {\"sys\", NULL},
    {\"exceptions\", NULL},
    {\"gc\", initgc},
    {\"_warnings\", _PyWarnings_Init},
    {0, 0}
};

const char* Py_GetBuildInfo(void) { return \"2.7.18 (Android)\"; }
const char* _Py_svnversion(void) { return \"Unversioned directory\"; }
const char* _Py_hgversion(void) { return \"\"; }
const char* _Py_hgidentifier(void) { return \"default\"; }
char* Py_GetPath(void) { return (char*)\"lib\"; }
char* Py_GetPrefix(void) { return (char*)\"\"; }
char* Py_GetExecPrefix(void) { return (char*)\"\"; }
char* Py_GetProgramFullPath(void) { return (char*)\"\"; }

#ifdef Py_InitModule4
#undef Py_InitModule4
PyAPI_FUNC(PyObject *) Py_InitModule4(const char *name, PyMethodDef *methods, const char *doc, PyObject *self, int apiver) {
    return Py_InitModule4_64(name, methods, doc, self, apiver);
}
#endif

#ifdef __cplusplus
}
#endif
")

set(PYTHON_CORE_SOURCES
    ${PY_GEN_DIR}/config.c
    ${PYTHON_SRC_DIR}/Python/_warnings.c
    ${PYTHON_SRC_DIR}/Python/Python-ast.c
    ${PYTHON_SRC_DIR}/Python/asdl.c
    ${PYTHON_SRC_DIR}/Python/ast.c
    ${PYTHON_SRC_DIR}/Python/bltinmodule.c
    ${PYTHON_SRC_DIR}/Python/ceval.c
    ${PYTHON_SRC_DIR}/Python/compile.c
    ${PYTHON_SRC_DIR}/Python/codecs.c
    ${PYTHON_SRC_DIR}/Python/dtoa.c
    ${PYTHON_SRC_DIR}/Python/dynload_stub.c
    ${PYTHON_SRC_DIR}/Python/errors.c
    ${PYTHON_SRC_DIR}/Python/formatter_string.c
    ${PYTHON_SRC_DIR}/Python/formatter_unicode.c
    ${PYTHON_SRC_DIR}/Python/frozen.c
    ${PYTHON_SRC_DIR}/Python/frozenmain.c
    ${PYTHON_SRC_DIR}/Python/future.c
    ${PYTHON_SRC_DIR}/Python/getargs.c
    ${PYTHON_SRC_DIR}/Python/getcompiler.c
    ${PYTHON_SRC_DIR}/Python/getcopyright.c
    ${PYTHON_SRC_DIR}/Python/getopt.c
    ${PYTHON_SRC_DIR}/Python/getplatform.c
    ${PYTHON_SRC_DIR}/Python/getversion.c
    ${PYTHON_SRC_DIR}/Python/graminit.c
    ${PYTHON_SRC_DIR}/Python/import.c
    ${PYTHON_SRC_DIR}/Python/importdl.c
    ${PYTHON_SRC_DIR}/Python/marshal.c
    ${PYTHON_SRC_DIR}/Python/modsupport.c
    ${PYTHON_SRC_DIR}/Python/mysnprintf.c
    ${PYTHON_SRC_DIR}/Python/mystrtoul.c
    ${PYTHON_SRC_DIR}/Python/peephole.c
    ${PYTHON_SRC_DIR}/Python/pyarena.c
    ${PYTHON_SRC_DIR}/Python/pyctype.c
    ${PYTHON_SRC_DIR}/Python/pyfpe.c
    ${PYTHON_SRC_DIR}/Python/pymath.c
    ${PYTHON_SRC_DIR}/Python/pystate.c
    ${PYTHON_SRC_DIR}/Python/pystrcmp.c
    ${PYTHON_SRC_DIR}/Python/pystrtod.c
    ${PYTHON_SRC_DIR}/Python/pythonrun.c
    ${PYTHON_SRC_DIR}/Python/random.c
    ${PYTHON_SRC_DIR}/Python/structmember.c
    ${PYTHON_SRC_DIR}/Python/symtable.c
    ${PYTHON_SRC_DIR}/Python/sysmodule.c
    ${PYTHON_SRC_DIR}/Python/thread.c
    ${PYTHON_SRC_DIR}/Python/traceback.c
    ${PYTHON_SRC_DIR}/Objects/abstract.c
    ${PYTHON_SRC_DIR}/Objects/boolobject.c
    ${PYTHON_SRC_DIR}/Objects/bufferobject.c
    ${PYTHON_SRC_DIR}/Objects/bytearrayobject.c
    ${PYTHON_SRC_DIR}/Objects/bytes_methods.c
    ${PYTHON_SRC_DIR}/Objects/capsule.c
    ${PYTHON_SRC_DIR}/Objects/cellobject.c
    ${PYTHON_SRC_DIR}/Objects/classobject.c
    ${PYTHON_SRC_DIR}/Objects/cobject.c
    ${PYTHON_SRC_DIR}/Objects/codeobject.c
    ${PYTHON_SRC_DIR}/Objects/complexobject.c
    ${PYTHON_SRC_DIR}/Objects/descrobject.c
    ${PYTHON_SRC_DIR}/Objects/dictobject.c
    ${PYTHON_SRC_DIR}/Objects/enumobject.c
    ${PYTHON_SRC_DIR}/Objects/exceptions.c
    ${PYTHON_SRC_DIR}/Objects/fileobject.c
    ${PYTHON_SRC_DIR}/Objects/floatobject.c
    ${PYTHON_SRC_DIR}/Objects/frameobject.c
    ${PYTHON_SRC_DIR}/Objects/funcobject.c
    ${PYTHON_SRC_DIR}/Objects/genobject.c
    ${PYTHON_SRC_DIR}/Objects/intobject.c
    ${PYTHON_SRC_DIR}/Objects/iterobject.c
    ${PYTHON_SRC_DIR}/Objects/listobject.c
    ${PYTHON_SRC_DIR}/Objects/longobject.c
    ${PYTHON_SRC_DIR}/Objects/memoryobject.c
    ${PYTHON_SRC_DIR}/Objects/methodobject.c
    ${PYTHON_SRC_DIR}/Objects/moduleobject.c
    ${PYTHON_SRC_DIR}/Objects/object.c
    ${PYTHON_SRC_DIR}/Objects/obmalloc.c
    ${PYTHON_SRC_DIR}/Objects/rangeobject.c
    ${PYTHON_SRC_DIR}/Objects/setobject.c
    ${PYTHON_SRC_DIR}/Objects/sliceobject.c
    ${PYTHON_SRC_DIR}/Objects/stringobject.c
    ${PYTHON_SRC_DIR}/Objects/structseq.c
    ${PYTHON_SRC_DIR}/Objects/tupleobject.c
    ${PYTHON_SRC_DIR}/Objects/typeobject.c
    ${PYTHON_SRC_DIR}/Objects/unicodectype.c
    ${PYTHON_SRC_DIR}/Objects/unicodeobject.c
    ${PYTHON_SRC_DIR}/Objects/weakrefobject.c
    ${PYTHON_SRC_DIR}/Parser/acceler.c
    ${PYTHON_SRC_DIR}/Parser/bitset.c
    ${PYTHON_SRC_DIR}/Parser/firstsets.c
    ${PYTHON_SRC_DIR}/Parser/grammar.c
    ${PYTHON_SRC_DIR}/Parser/grammar1.c
    ${PYTHON_SRC_DIR}/Parser/listnode.c
    ${PYTHON_SRC_DIR}/Parser/metagrammar.c
    ${PYTHON_SRC_DIR}/Parser/myreadline.c
    ${PYTHON_SRC_DIR}/Parser/node.c
    ${PYTHON_SRC_DIR}/Parser/parser.c
    ${PYTHON_SRC_DIR}/Parser/parsetok.c
    ${PYTHON_SRC_DIR}/Parser/pgen.c
    ${PYTHON_SRC_DIR}/Parser/tokenizer.c
    ${PYTHON_SRC_DIR}/Modules/gcmodule.c
    ${PYTHON_SRC_DIR}/Modules/main.c
    ${PYTHON_SRC_DIR}/Modules/posixmodule.c
    ${PYTHON_SRC_DIR}/Modules/errnomodule.c
    ${PYTHON_SRC_DIR}/Modules/_sre.c
    ${PYTHON_SRC_DIR}/Modules/_codecsmodule.c
    ${PYTHON_SRC_DIR}/Modules/_weakref.c
    ${PYTHON_SRC_DIR}/Modules/zipimport.c
    ${PYTHON_SRC_DIR}/Modules/symtablemodule.c
    ${PYTHON_SRC_DIR}/Modules/xxsubtype.c
    ${PYTHON_SRC_DIR}/Modules/mathmodule.c
    ${PYTHON_SRC_DIR}/Modules/_math.c
    ${PYTHON_SRC_DIR}/Modules/timemodule.c
    ${PYTHON_SRC_DIR}/Modules/operator.c
    ${PYTHON_SRC_DIR}/Modules/_randommodule.c
    ${PYTHON_SRC_DIR}/Modules/_struct.c
    ${PYTHON_SRC_DIR}/Modules/itertoolsmodule.c
    ${PYTHON_SRC_DIR}/Modules/stropmodule.c
    ${PYTHON_SRC_DIR}/Modules/arraymodule.c
    ${PYTHON_SRC_DIR}/Modules/cmathmodule.c
    ${PYTHON_SRC_DIR}/Modules/cStringIO.c
    ${PYTHON_SRC_DIR}/Modules/cPickle.c
    ${PYTHON_SRC_DIR}/Modules/binascii.c
    ${PYTHON_SRC_DIR}/Modules/_functoolsmodule.c
    ${PYTHON_SRC_DIR}/Modules/_collectionsmodule.c
    ${PYTHON_SRC_DIR}/Modules/threadmodule.c
    ${PYTHON_SRC_DIR}/Modules/signalmodule.c
    ${PYTHON_SRC_DIR}/Modules/zlibmodule.c
)

add_library(python27_static STATIC ${PYTHON_CORE_SOURCES})
target_include_directories(python27_static PRIVATE
    "${PY_GEN_DIR}"
    "${PYTHON_SRC_DIR}/Include"
    "${PYTHON_SRC_DIR}/Python"
    "${PYTHON_SRC_DIR}/Modules"
)
target_compile_definitions(python27_static PRIVATE Py_BUILD_CORE PLATFORM="android")
target_compile_options(python27_static PRIVATE -w -fwrapv -Wno-implicit-function-declaration -Wno-int-conversion -Wno-incompatible-function-pointer-types)

if(ANDROID)
    # Include Android NDK sources for cpu-features.h
    target_include_directories(cryptopp_static PRIVATE ${ANDROID_NDK}/sources/android/cpufeatures)

    # Find and link the NDK cpufeatures static library
    add_library(cpufeatures STATIC ${ANDROID_NDK}/sources/android/cpufeatures/cpu-features.c)
    target_link_libraries(cryptopp_static PRIVATE cpufeatures)
endif()


file(GLOB ETER_BASE_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/EterBase/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/EterBase/*.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/EterBase/Poly/*.cpp"
)
file(GLOB ETER_LIB_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/EterLib/*.cpp")
file(GLOB ETER_IMAGE_LIB_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/EterImageLib/*.cpp")
file(GLOB GAME_LIB_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/GameLib/*.cpp")
list(FILTER GAME_LIB_SOURCES EXCLUDE REGEX ".*(CRenderTarget|RenderTargetManager)\\.cpp$")

file(GLOB ETER_LIB_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/EterLib/*.cpp")
list(FILTER ETER_LIB_SOURCES EXCLUDE REGEX ".*(CRenderTarget|RenderTargetManager)\\.cpp$")

# Grab all the UserInterface files FIRST and ensure VoiceChat.cpp is included
file(GLOB USER_INTERFACE_SOURCES 
    "${CMAKE_CURRENT_SOURCE_DIR}/UserInterface/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/UserInterface/VoiceChat.cpp"
)
# Only filter out general backups and old files
list(FILTER USER_INTERFACE_SOURCES EXCLUDE REGEX ".*(Copy|old|bak)$")

if(APPLE)
    list(APPEND USER_INTERFACE_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/android_compat/IOSBridge.mm")
endif()

file(GLOB ETER_PACK_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/EterPack/*.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/EterPack/*.c")
file(GLOB ETER_GRN_LIB_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/EterGrnLib/*.cpp")
file(GLOB EFFECT_LIB_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/EffectLib/*.cpp")
file(GLOB ETER_PYTHON_LIB_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/EterPythonLib/*.cpp")
file(GLOB SCRIPT_LIB_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/ScriptLib/*.cpp")
file(GLOB SPHERE_LIB_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/SphereLib/*.cpp")
file(GLOB MILES_LIB_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/MilesLib/*.cpp")
file(GLOB SPEED_TREE_LIB_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/SpeedTreeLib/*.cpp")
file(GLOB PR_TERRAIN_LIB_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/PRTerrainLib/*.cpp")
file(GLOB ETER_LOCALE_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/EterLocale/*.cpp")

set(METIN2_ALL_SOURCES
    ${USER_INTERFACE_SOURCES}
    ${ETER_BASE_SOURCES}
    ${ETER_LIB_SOURCES}
    ${ETER_IMAGE_LIB_SOURCES}
    ${GAME_LIB_SOURCES}
    ${ETER_PACK_SOURCES}
    ${ETER_GRN_LIB_SOURCES}
    ${EFFECT_LIB_SOURCES}
    ${ETER_PYTHON_LIB_SOURCES}
    ${SCRIPT_LIB_SOURCES}
    ${SPHERE_LIB_SOURCES}
    ${MILES_LIB_SOURCES}
    ${SPEED_TREE_LIB_SOURCES}
    ${PR_TERRAIN_LIB_SOURCES}
    ${ETER_LOCALE_SOURCES}
)

if(APPLE)
    add_executable(metin2_mobile MACOSX_BUNDLE ${METIN2_ALL_SOURCES})
        set_target_properties(metin2_mobile PROPERTIES
        OUTPUT_NAME "WM2Mobile"
        MACOSX_BUNDLE_GUI_IDENTIFIER "com.wm2.mobile"
        XCODE_ATTRIBUTE_PRODUCT_BUNDLE_IDENTIFIER "com.wm2.mobile"
        MACOSX_BUNDLE_BUNDLE_NAME "WM2 Mobile"
        XCODE_ATTRIBUTE_INFOPLIST_KEY_CFBundleDisplayName "WM2 Mobile"
        MACOSX_BUNDLE_SHORT_VERSION_STRING "1.0.0"
        MACOSX_BUNDLE_BUNDLE_VERSION "1"
        XCODE_ATTRIBUTE_MARKETING_VERSION "1.0.0"
        XCODE_ATTRIBUTE_CURRENT_PROJECT_VERSION "1"
        XCODE_ATTRIBUTE_TARGETED_DEVICE_FAMILY "1,2"
        XCODE_ATTRIBUTE_INFOPLIST_KEY_UIFileSharingEnabled YES
        XCODE_ATTRIBUTE_INFOPLIST_KEY_LSSupportsOpeningDocumentsInPlace YES
        XCODE_ATTRIBUTE_INFOPLIST_KEY_UIRequiresFullScreen YES
        XCODE_ATTRIBUTE_INFOPLIST_KEY_ITSAppUsesNonExemptEncryption NO
        "XCODE_ATTRIBUTE_INFOPLIST_KEY_UISupportedInterfaceOrientations[sdk=iphone*]" "UIInterfaceOrientationLandscapeLeft UIInterfaceOrientationLandscapeRight"
        "XCODE_ATTRIBUTE_INFOPLIST_KEY_UISupportedInterfaceOrientations[sdk=ipad*]" "UIInterfaceOrientationLandscapeLeft UIInterfaceOrientationLandscapeRight"
    )
else()
    add_library(metin2_mobile SHARED ${METIN2_ALL_SOURCES})
endif()

target_include_directories(metin2_mobile PRIVATE
    "${PY_GEN_DIR}"
    "${PY_GEN_DIR}/Python27"
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/android_compat
    ${CMAKE_CURRENT_SOURCE_DIR}/../Extern/granny/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../Extern/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../Extern/cryptopp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Common
    ${CMAKE_CURRENT_SOURCE_DIR}/../common
    ${CMAKE_CURRENT_SOURCE_DIR}/EterBase
    ${CMAKE_CURRENT_SOURCE_DIR}/EterLib
    ${CMAKE_CURRENT_SOURCE_DIR}/EterPythonLib
    ${CMAKE_CURRENT_SOURCE_DIR}/GameLib
    ${CMAKE_CURRENT_SOURCE_DIR}/UserInterface
    ${CMAKE_CURRENT_SOURCE_DIR}/MilesLib
    ${CMAKE_CURRENT_SOURCE_DIR}/EffectLib
    ${CMAKE_CURRENT_SOURCE_DIR}/SphereLib
    ${CMAKE_CURRENT_SOURCE_DIR}/EterPack
    ${CMAKE_CURRENT_SOURCE_DIR}/EterGrnLib
    ${CMAKE_CURRENT_SOURCE_DIR}/EterImageLib
    ${CMAKE_CURRENT_SOURCE_DIR}/SpeedTreeLib
    ${CMAKE_CURRENT_SOURCE_DIR}/PRTerrainLib
    ${CMAKE_CURRENT_SOURCE_DIR}/ScriptLib
    ${CMAKE_CURRENT_SOURCE_DIR}/EterLocale
    ${CMAKE_CURRENT_SOURCE_DIR}/CWebBrowser
)

if(ANDROID OR APPLE)
    # -----------------------------------------------------------------------------
    # Linker Fix: Ignore Windows #pragma comment(lib, ...) directives
    # -----------------------------------------------------------------------------
    set(DUMMY_LIB_DIR "${CMAKE_BINARY_DIR}/dummy_windows_libs")
    file(MAKE_DIRECTORY "${DUMMY_LIB_DIR}")

    set(WINDOWS_LIBS 
        winmm.lib imm32.lib uuid.lib version.lib imagehlp.lib 
        ws2_32.lib wsock32.lib d3d8.lib d3dx8.lib ddraw.lib 
        dxguid.lib dinput8.lib dsound.lib strmiids.lib
        kernel32.lib user32.lib gdi32.lib advapi32.lib
        ole32.lib oleaut32.lib winspool.lib comdlg32.lib
        shell32.lib xmllite.lib shlwapi.lib discord_rpc_r.lib
        devil.lib granny2.9.12.0.lib mss32.lib oldnames.lib
        SpeedTreeRT.lib dmoguids.lib voice_chat_core_Release_Win32
        voice_chat_core_Release_Win32.lib cryptlib-Release.lib lzo2.lib
        libjpeg-ST.lib libjpeg.lib
    )

    foreach(WIN_LIB ${WINDOWS_LIBS})
        file(WRITE "${DUMMY_LIB_DIR}/${WIN_LIB}" "")
    endforeach()

    target_link_directories(metin2_mobile PRIVATE "${DUMMY_LIB_DIR}")
endif()

if(ANDROID)
    find_library(log-lib log)
    find_library(android-lib android)
    find_library(gles3-lib GLESv3)
    find_library(egl-lib EGL)

    target_link_libraries(metin2_mobile
        granny2_static
        cryptopp_static
        python27_static
        ${log-lib}
        ${android-lib}
        ${gles3-lib}
        ${egl-lib}
        z
        m
    )
    target_link_options(metin2_mobile PRIVATE "-Wl,-error-limit=0" "-Wl,--export-dynamic")
elseif(APPLE)
    target_link_libraries(metin2_mobile
        granny2_static
        cryptopp_static
        python27_static
        "-framework Foundation"
        "-framework UIKit"
        "-framework GLKit"
        "-framework OpenGLES"
        "-framework CoreGraphics"
        "-framework ImageIO"
        "-framework AVFoundation"
        "-Wl,-no_fixup_chains"
        z
        m
    )
endif()
if(APPLE)
    target_compile_definitions(metin2_mobile PRIVATE __ANDROID__=1)
endif()
