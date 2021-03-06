dnl# -*- sh -*-
dnl# Process this file with autoconf to produce a configure script.
dnl#
dnl#Copyright (c) 1998 Michal Vitecek <fuf@fuf.sh.cvut.cz>
dnl#Copyright (c) 2001,1998 Sasha Vasko <sasha at aftercode.net>
dnl#Copyright (c) 1998 Juergen Sawinski <Juergen.Sawinski@urz.uni-heidelberg.de>
dnl#Copyright (c) 1997 1998 Guylhem AZNAR <guylhem@oeil.qc.ca>

AC_PREREQ([2.69])
AC_INIT([libAfterBase],[1.14],[as-bugs@afterstep.org],[libAfterBase.tar])
AC_CONFIG_SRCDIR([afterbase_config.h.in])

version=$PACKAGE_VERSION
version_major="1"
lib_name="LIBAFTERBASE"
lib_file_name=$PACKAGE_NAME


AC_CONFIG_HEADERS([config.h:config.h.in])

dnl# Setup command-line options

dnl# we need the standard prefixes defined early
test "x$prefix" = xNONE && prefix=$ac_default_prefix
test "x$exec_prefix" = xNONE && exec_prefix="$prefix"
eval "mandir=$mandir"
eval "libdir=$libdir"
eval "includedir=$includedir"

dnl# compile-time switches
AC_ARG_ENABLE([i18n],		[  --enable-i18n           support I18N [no] ],[enable_i18n=$enableval],[enable_i18n="no"])
AC_ARG_ENABLE([xlocale],		[  --enable-xlocale        using X_LOCALE [no] ],[enable_xlocale=$enableval],[enable_xlocale="no"])
AC_ARG_WITH([locale],		[  --with-locale=LOCALE    locale name you want to use ],[with_locale=$withval],[with_locale=""])
AC_ARG_ENABLE([staticlibs],   [  --enable-staticlibs     enable linking to libAfterBase statically [yes] ],[enable_staticlibs=$enableval],[enable_staticlibs="yes"])
AC_ARG_ENABLE([sharedlibs],   [  --enable-sharedlibs     enable linking to libAfterBase dynamically [no] ],[enable_sharedlibs=$enableval],[enable_sharedlibs="no"])

AC_ARG_ENABLE([gdb],		[  --enable-gdb            add gdb symbols (-g) (for debugging) [no] ],[enable_gdb=$enableval],[enable_gdb="no"])
AC_ARG_ENABLE([warn],		[  --enable-warn           turn on more compiler warnings (for debugging) [no] ],[enable_warn=$enableval],[enable_warn="no"])
AC_ARG_ENABLE([gprof],		[  --enable-gprof          add gprof symbols (-pg) (for debugging) [no] ],[enable_gprof=$enableval],[enable_gprof="no"])
AC_ARG_ENABLE([audit],		[  --enable-audit          memory usage audit (for debugging) [no] ],[enable_audit=$enableval],[enable_audit="no"])
AC_ARG_ENABLE([trace],		[  --enable-trace          some function calls tracing (see include/trace.h) (for debugging) [no] ], [enable_trace=$enableval], [enable_trace="no"])
AC_ARG_ENABLE([trace-x],		[  --enable-trace-x        trace X calls (runtime option) (for debugging) [no] ],[enable_trace_x=$enableval],[enable_trace_x="no"])

dnl# Check for compiler tools

AC_PROG_AWK
AC_PROG_CC
AM_PROG_CC_C_O
AC_PROG_CPP
AC_PROG_CXX
AC_PROG_LN_S
AC_PROG_MAKE_SET
AC_EXEEXT
dnl#  AC_SUBST(CC)
dnl#  AC_SUBST(GCC)
AC_SUBST(CFLAGS)

AC_DIAGNOSE([obsolete],[AC_CYGWIN is obsolete: use AC_CANONICAL_HOST and check if $host_os
matches *cygwin*])dnl
AC_CANONICAL_HOST
case $host_os in
  *cygwin* ) CYGWIN=yes;;
	 * ) CYGWIN=no;;
esac


if test "x$FROM_AFTERSTEP_CONFIGURE" = "xyes"; then
dnl# AfterSTep already checked everything for us

	dnl#  AC_PROG_INSTALL
    AC_SUBST(INSTALL)
    AC_SUBST(INSTALL_PROGRAM)
    AC_SUBST(INSTALL_DATA)
    AC_SUBST(INSTALL_LIB)
    AC_SUBST(RANLIB)
    AC_SUBST(MV)
    AC_SUBST(CP)
    AC_SUBST(MKDIR)
    AC_SUBST(PERL)
    AC_SUBST(FIND)
    AC_SUBST(XARGS)
    AC_SUBST(LDCONFIG)
    AC_SUBST(LEX)
    AC_SUBST(LEXLIB)
    AC_SUBST(YACC)
	if test "x$GCC" = "xyes" -a "x$CYGWIN" = "xno" ; then
      if test -z "`echo $CFLAGS | grep '\-fPIC' 2> /dev/null`" ; then
		CFLAGS="$CFLAGS -fPIC"
	  fi
	  LDFLAGS="$LDFLAGS -dynamic"
	fi

else

dnl# we are alone - have to do all the checks ourselves :
	dnl# Add -Wall
	if test "x$GCC" = "xyes"; then
	  if test -z "`echo $CFLAGS | grep '\-Wall' 2> /dev/null`" ; then
  		CFLAGS="$CFLAGS -Wall"
	  fi
	fi

	dnl# Add -fPIC for IA64 compilation

	if test "x$GCC" = "xyes" -a "x$CYGWIN" = "xno" ; then
      if test -z "`echo $CFLAGS | grep '\-fPIC' 2> /dev/null`" ; then
		CFLAGS="$CFLAGS -fPIC"
	  fi
	  LDFLAGS="$LDFLAGS -dynamic"
	fi

	dnl# Add -g
	if test "x$enable_gdb" = "xyes"; then
  		if ! test -n "`echo $CFLAGS' ' | grep '\-g ' 2> /dev/null`" ; then
			CFLAGS="$CFLAGS -g"
  		fi
	else
  		DIRTY_CFLAGS="$CFLAGS"
  		CFLAGS="-DNO_DEBUG_OUTPUT"
  		for f in $DIRTY_CFLAGS; do 
        		if test "x$f" != "x-g"; then
	   		CFLAGS="$CFLAGS $f"
			fi	 
  		done
	fi

	dnl# Add -pg
	if test "x$enable_gprof" = "xyes"; then
	  CFLAGS="$CFLAGS -pg"
	  LDFLAGS="$LDFLAGS -pg"
	fi
	dnl# Add lots more warnings
	if test "x$enable_warn" = "xyes"; then
	  CFLAGS="$CFLAGS -Wuninitialized -Wwrite-strings -Wcast-qual -Wbad-function-cast -Wpointer-arith -Wstrict-prototypes -Wmissing-prototypes -Wmissing-declarations -Wnested-externs -Wconversion -Wcomment -Wcast-align -Winline -Wshadow -Wredundant-decls"
	fi

	AC_PROG_INSTALL
	dnl# if the path to the install program is relative, make it absolute
	currdir=`pwd`
	INSTALL=`echo $INSTALL|sed -e "s@\^\.\.@${currdir}@" -e "s@^autoconf@${currdir}/autoconf@"`
	INSTALL_PROGRAM=`echo $INSTALL_PROGRAM|sed -e "s@\^\.\.@${currdir}@" -e "s@^autoconf@${currdir}/autoconf@"`
	INSTALL_DATA=`echo $INSTALL_DATA|sed -e "s@\^\.\.@${currdir}@" -e "s@^autoconf@${currdir}/autoconf@"`
	INSTALL_LIB=`echo $INSTALL_LIB|sed -e "s@\^\.\.@${currdir}@" -e "s@^autoconf@${currdir}/autoconf@"`

	AC_PROG_LEX
	AC_PROG_RANLIB
	AC_PROG_YACC
	AC_PATH_PROG([MV], [mv], [mv])
	export MV
	AC_PATH_PROG([CP], [cp], [cp])
	export CP
	AC_PATH_PROG([MKDIR], [mkdir], [mkdir])
	export MKDIR
	AC_PATH_PROG([PERL], [perl], [perl])
	export PERL
	AC_PATH_PROG([FIND], [find], [find])
	export FIND
	AC_PATH_PROG([XARGS], [xargs], [xargs])
	export XARGS
	AC_PATH_PROG([LDCONFIG], [ldconfig], [ldconfig], [$PATH:/sbin:/usr/local/sbin])
	export LDCONFIG
fi

if test "x$enable_gdb" = "xno"; then
		DIRTY_CFLAGS="$CFLAGS"
  		CFLAGS="-O3"
  		for f in $DIRTY_CFLAGS; do 
       		if test "x$f" != "x-O2" -a "x$f" != "x-O3"; then
		   		CFLAGS="$CFLAGS $f"
			fi	 
  		done
fi


dnl# some standard checks :
dnl# let's check for endiannes of our box
AC_C_BIGENDIAN
AC_C_INLINE

dnl# Check for X :
dnl# check for X, if top level configure script did not do that yet :
if test "x$PATH_XTRA_CHECKED" != "xyes"; then
	AC_PATH_XTRA
	AC_PATH_X
else
  if test "x$have_x" != "xyes"; then
	  AC_DEFINE([X_DISPLAY_MISSING],[1],["No X Window System libraries available"])
  fi
fi

user_ldflags=$LDFLAGS

if test "x$have_x" = "xyes"; then
  x_libs="$X_LIBS $X_EXTRA_LIBS $X_PRE_LIBS"
  if test "x$user_ldflags" = "x"; then
  	full_x_libs=$x_libs
  else
	full_x_libs="$user_ldflags $x_libs"
  fi
  AC_CHECK_LIB([X11], [XOpenDisplay], [x_libs="-lX11 $x_libs"],[],[$full_x_libs])
fi

dnl# Checks for libraries.
dnl# FIXME: Replace `main' with a function in `-lc80':
AC_CHECK_LIB([c80], [main])

dnl# Check for headers
AC_FUNC_ALLOCA
AC_HEADER_STDC
AC_HEADER_DIRENT
AC_HEADER_TIME
AC_CHECK_HEADERS([execinfo.h fcntl.h limits.h link.h locale.h malloc.h stddef.h stdint.h stdio.h stdlib.h string.h sys/dirent.h sys/file.h sys/socket.h sys/systeminfo.h sys/time.h sys/wait.h unistd.h])

AC_CHECK_HEADERS([elf.h],[AC_CHECK_DECLS([ElfW],[],[],[#include <link.h>])
			AC_CHECK_MEMBERS([Elf32_Dyn.d_tag, Elf64_Dyn.d_tag],[],[],[#include <elf.h>])
			AC_CHECK_TYPES([Elf64_Addr, Elf32_Addr],[],[],[#include <elf.h>])])

dnl# this is needed for self diagnostic code :
AC_CHECK_FUNCS([backtrace_symbols_fd backtrace_symbols backtrace])

AC_CACHE_CHECK(for struct sigcontext, as_cv_struct_sigcontext,
[AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[#include <signal.h>
#include <asm/sigcontext.h>]], [[int main(){struct sigcontext sc; return (int)&sc;}]])],[as_cv_struct_sigcontext=yes],[as_cv_struct_sigcontext=no])])
if test "$as_cv_struct_sigcontext" = yes; then
  AC_DEFINE(HAVE_SIGCONTEXT,1,struct sigcontext is available)
fi

dnl# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADER_STDBOOL
AC_TYPE_MODE_T
AC_TYPE_OFF_T
AC_TYPE_PID_T
AC_TYPE_SIZE_T
AC_TYPE_UINT16_T
AC_CHECK_TYPES([ptrdiff_t])

dnl# Checks for functions
AC_FUNC_FORK
AC_FUNC_LSTAT_FOLLOWS_SLASHED_SYMLINK
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_FUNC_STRTOD
AC_FUNC_WAIT3
AC_CHECK_FUNCS([bzero getcwd gethostname gettimeofday isascii localeconv memmove memset mkdir putenv regcomp rmdir select setlocale socket sqrt strcasecmp strchr strdup strerror strncasecmp strrchr strstr strtol uname])

if test "x$enable_staticlibs" = "xyes"; then
   LIBPROG='$(LIB_STATIC)'
   LIBINSTALL="install.static"
   LIBUNINSTALL="uninstall.static"
else
   enable_sharedlibs="yes"
fi
if test "x$enable_sharedlibs" = "xyes"; then
	if test "x$CYGWIN" = "xyes" ; then
	   tmp='$(LIB_SHARED_CYG)'
	   tmp2='install.cyg'
	else
	   tmp='$(LIB_SHARED)'
	   tmp2='install.dyn'
	fi
   	LIBPROG="$LIBPROG $tmp"
   	LIBINSTALL="$LIBINSTALL $tmp2"
    LIBUNINSTALL="uninstall.static un$tmp2"
fi

dnl# define X_LOCALE.  need Linux, NetBSD and etc.

if test "x$enable_xlocale" = "xyes"; then
  DEFINE_XLOCALE="-DX_LOCALE"
fi

dnl# Check for safe min/max - there still are some retarded compilers out there :
AC_MSG_CHECKING(if we can use sign safe min/max macros)
AC_RUN_IFELSE([AC_LANG_SOURCE([[#define MIN(x,y) \
  		({ const typeof(x) _x = (x); const typeof(y) _y = (y);  (void) (&_x == &_y); \
     		 _x < _y ? _x : _y; })
                 int main(){return MIN(0,1);}]])],[AC_DEFINE(USE_SAFE_MINMAX,1,will use type-safe min/max macros) AC_MSG_RESULT(yes)],[AC_MSG_RESULT(no)],[])

dnl# Translate enable_* from "yes/no" to "1/0" (for configure.h)

if test "x$enable_i18n" = "xyes"; then AC_DEFINE([I18N],[1],[Localisation support]) fi
if test "x$enable_audit" = "xyes"; then AC_DEFINE([DEBUG_ALLOCS],[1],[All memory allocations are traced]) fi
if test "x$enable_trace" = "xyes"; then AC_DEFINE([DEBUG_TRACE],[1],[Select few function calls are traced]) fi
if test "x$enable_trace_x" = "xyes"; then AC_DEFINE([DEBUG_TRACE_X],[1],[Several Xlib calls are traced]) fi

dnl# Variable strings declaration

AC_SUBST(version)
AC_SUBST(version_major)
AC_SUBST(lib_name)
AC_SUBST(lib_file_name)
AC_SUBST(with_locale)
AC_SUBST(user_ldflags)
AC_SUBST(x_libs)
AC_SUBST(X_CFLAGS)
AC_SUBST(LIBPROG)
AC_SUBST(LIBINSTALL)
AC_SUBST(LIBUNINSTALL)
AC_SUBST(DEFINE_XLOCALE)
AC_SUBST(PERL)

if test "x$enable_audit" = "xyes"; then 
	DEBUG_ALLOCS_DEF="define"
	DEBUG_ALLOCS_VAL=1
else
	DEBUG_ALLOCS_DEF="undef"
	DEBUG_ALLOCS_VAL=
fi
if test "x$enable_trace" = "xyes"; then 
	DEBUG_TRACE_DEF="define"
	DEBUG_TRACE_VAL=1
else
	DEBUG_TRACE_DEF="undef"
	DEBUG_TRACE_VAL=
fi

if test "x$enable_trace_x" = "xyes"; then 
	DEBUG_TRACE_X_DEF="define"
	DEBUG_TRACE_X_VAL=1
else
	DEBUG_TRACE_X_DEF="undef"
	DEBUG_TRACE_X_VAL=
fi

if test "x$have_x" = "xyes"; then
	X_DISPLAY_DEF="define"
	X_DISPLAY_VAL=1
else
	X_DISPLAY_DEF="undef"
	X_DISPLAY_VAL=
fi

AC_SUBST(DEBUG_ALLOCS_DEF)
AC_SUBST(DEBUG_ALLOCS_VAL)
AC_SUBST(DEBUG_TRACE_DEF)
AC_SUBST(DEBUG_TRACE_VAL)
AC_SUBST(DEBUG_TRACE_X_DEF)
AC_SUBST(DEBUG_TRACE_X_VAL)
AC_SUBST(X_DISPLAY_DEF)
AC_SUBST(X_DISPLAY_VAL)

dnl# Common parts of the Makefile

AC_CONFIG_FILES([afterbase_config.h Makefile])
AC_OUTPUT
