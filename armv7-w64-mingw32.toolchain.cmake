cmake_minimum_required( VERSION 3.2 )

if( CMAKE_TOOLCHAIN_FILE )
  # touch toolchain variable to suppress "unused variable" warning
endif()

set( CMAKE_SYSTEM_NAME Windows )

set(MXE "~/mxe-SHARED" CACHE PATH "mxe-SHARED")
set(TRIPLET armv7-w64-mingw32)

set( CMAKE_SYSTEM_PROCESSOR "armv7" )
set( CMAKE_C_COMPILER   "${MXE}/usr/bin/${TRIPLET}-clang"     CACHE PATH "C compiler" )
set( CMAKE_CXX_COMPILER "${MXE}/usr/bin/${TRIPLET}-clang++"     CACHE PATH "C++ compiler" )
set( CMAKE_ASM_COMPILER "${MXE}/usr/bin/${TRIPLET}-clang"     CACHE PATH "assembler" )
set( CMAKE_STRIP        "${MXE}/usr/bin/${TRIPLET}-strip"   CACHE PATH "strip" )
set( CMAKE_AR           "${MXE}/usr/bin/${TRIPLET}-ar"      CACHE PATH "archive" )
set( CMAKE_LINKER       "${MXE}/usr/bin/${TRIPLET}-ld"      CACHE PATH "linker" )
set( CMAKE_NM           "${MXE}/usr/bin/${TRIPLET}-nm"      CACHE PATH "nm" )
set( CMAKE_OBJCOPY      "${MXE}/usr/bin/${TRIPLET}-objcopy" CACHE PATH "objcopy" )
set( CMAKE_OBJDUMP      "${MXE}/usr/bin/${TRIPLET}-objdump" CACHE PATH "objdump" )
set( CMAKE_RANLIB       "${MXE}/usr/bin/${TRIPLET}-ranlib"  CACHE PATH "ranlib" )

# where is the target environment
set( CMAKE_FIND_ROOT_PATH "${MXE}/usr/bin" "${MXE}/${TRIPLET}" "${CMAKE_INSTALL_PREFIX}" "${CMAKE_INSTALL_PREFIX}/share" )
set( CMAKE_INSTALL_PREFIX "${MXE}/usr/${TRIPLET}" CACHE PATH "default install path" )
set( PKG_CONFIG_EXECUTABLE "${MXE}/usr/bin/${TRIPLET}-pkg-config" )

add_compile_definitions(WINRT)

set(WINRT True)
