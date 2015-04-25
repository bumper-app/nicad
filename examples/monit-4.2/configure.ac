# Process this file with autoconf to produce a configure script.
# Mark Ferlatte, <ferlatte@cryptio.net>
#
# $Id: configure.ac,v 1.74 2004/03/24 16:13:02 hauk Exp $

# autoconf requirement
AC_PREREQ([2.53])

# Init autoconf and automake
AC_INIT([monit], [4.2], [monit-general@nongnu.org])

# Package info
AC_REVISION([$Revision: 1.74 $])
AC_CONFIG_SRCDIR([monitor.c])

# ------------------------------------------------------------------------
# Programs 
# ------------------------------------------------------------------------

AC_PROG_CC
AC_PROG_GCC_TRADITIONAL
AC_PROG_INSTALL
AC_PROG_MAKE_SET
AC_PROG_LEX
	if test "$LEX" != flex; then
		AC_MSG_ERROR([monit requires flex, not lex])
	fi
AC_PROG_YACC

# ------------------------------------------------------------------------
# Libraries 
# ------------------------------------------------------------------------

# Check for libraries
AC_CHECK_LIB([socket], [socket])
AC_CHECK_LIB([inet], [socket])
AC_CHECK_LIB([nsl], [inet_addr])
AC_CHECK_LIB([resolv], [inet_aton])
AC_CHECK_LIB([crypt], [crypt])

# Wacky pthread madness
pthread_libs=""
AC_CHECK_LIB([pthread], 
	     [pthread_create],
	     [pthread_libs="-lpthread"])
AC_CHECK_LIB([c_r],
	     [pthread_create],
	     [pthread_libs="-pthread"])
if test "$pthread_libs"
then
	LIBS="$pthread_libs $LIBS"
fi

# ------------------------------------------------------------------------
# Header files 
# ------------------------------------------------------------------------

# Checks for header files.
AC_HEADER_STDC
AC_HEADER_SYS_WAIT
AC_HEADER_STAT
AC_HEADER_TIME
AC_CHECK_HEADERS([ \
	arpa/inet.h \
	asm/page.h \
	asm/param.h \
	ctype.h \
	crypt.h \
	errno.h \
	fcntl.h \
	getopt.h \
	glob.h \
	grp.h \
	kvm.h \
	limits.h \
	limits.h \
	loadavg.h \
	locale.h \
	machine/pmap.h \
	machine/vmparam.h \
	memory.h \
	mntent.h \
	netdb.h \
	net/if.h \
	netinet/in.h \
	netinet/ip.h \
	netinet/ip_icmp.h \
	procfs.h \
	procinfo.h \
	pthread.h \
	pwd.h \
	regex.h \
	setjmp.h \
	signal.h \
	stdarg.h \
	stdio.h \
	string.h \
	string.h \
	strings.h \
	stropts.h \
	sys/filio.h \
	sys/ioctl.h \
	sys/loadavg.h \
	sys/lock.h \
	sys/mnttab.h \
	sys/mount.h \
	sys/nlist.h \
	sys/param.h \
	sys/proc.h \
	sys/pstat.h \
	sys/resource.h \
	sys/resourcevar.h \
	sys/socket.h \
	sys/statvfs.h \
        sys/sysctl.h \
	sys/time.h \
	sys/types.h \
	sys/un.h \
	sys/user.h \
        sys/vmmeter.h \
	sys/vfs.h \
	syslog.h \
	unistd.h \
        uvm/uvm.h \
	uvm/uvm_object.h \
	uvm/uvm_map.h \
	uvm/uvm_pmap.h \
	vm/pmap.h \
        vm/vm.h \
	vm/vm_object.h \
	vm/vm_map.h \
	])

# Check for types.
AC_TYPE_MODE_T
AC_TYPE_PID_T
AC_TYPE_SIZE_T
AC_TYPE_PID_T
AC_TYPE_SIGNAL

# Check for structures.
AC_STRUCT_TM
AC_CHECK_MEMBERS([struct tm.tm_gmtoff])

# ------------------------------------------------------------------------
# Compiler 
# ------------------------------------------------------------------------

# Compiler characteristics
AC_C_CONST
AC_C_BIGENDIAN

# If the compiler is gcc, turn on warnings
if test "x$GCC" = "xyes"; then
	case " $CFLAGS " in
		*[\ \	]-Wall[\ \	]*)
			;;
		*)
			CFLAGS="$CFLAGS -Wall"
			;;
	esac
fi

# Checks for library functions.
AC_FUNC_ERROR_AT_LINE
AC_FUNC_FORK
AC_FUNC_MALLOC
AC_FUNC_STAT
AC_FUNC_STRFTIME
AC_CHECK_FUNCS(statfs)
AC_CHECK_FUNCS(statvfs)
AC_CHECK_FUNCS(setlocale)

# Check if we do need external GNU replacements
AC_FUNC_GETLOADAVG

jm_FUNC_GNU_STRFTIME

EXTERNALS=""
if test "$jm_cv_func_working_gnu_strftime" = "no"; then
	# We do not have have a working strftime supporting %z
	EXTERNALS="$EXTERNALS external/strftime.c"
fi

AC_SUBST(EXTERNALS)

# Check for SOL_IP
AC_MSG_CHECKING(for SOL_IP)
AC_TRY_COMPILE([#include <netdb.h>], [
	int level = SOL_IP;
], [
	AC_MSG_RESULT(yes)
	AC_DEFINE([HAVE_SOL_IP], [1], [ SOL_IP defined])
], [
	AC_MSG_RESULT(no)
])

# ------------------------------------------------------------------------
# Paths 
# ------------------------------------------------------------------------

# Find the right directory to put the root-mode PID file in
AC_MSG_CHECKING([pid file location])
if test -d "/var/run"
then
	piddir="/var/run"
elif test -d "/etc"
then
	piddir="/etc"
fi

AC_DEFINE_UNQUOTED([PIDDIR], "$piddir",
	  [Define to the pid storage directory.])
AC_MSG_RESULT([$piddir])

# Test mounted filesystem description file
if test -f "/etc/mtab"
then
  AC_DEFINE([HAVE_MTAB], 1, [Define to 1 if you have /etc/mtab])
elif test -f "/etc/mnttab"; then
  AC_DEFINE([HAVE_MNTTAB], 1, [Define to 1 if you have /etc/mnttab])
fi

# ------------------------------------------------------------------------
# Architecture/OS detection
# ------------------------------------------------------------------------

# Backward compatibility until we get ride of arch settings
if test `uname` = "SunOS" 
then
   ARCH="SOLARIS"
   CFLAGS="$CFLAGS -D _REENTRANT -D_POSIX_PTHREAD_SEMANTICS"
elif test `uname` = "Linux" 
then
   ARCH="LINUX"
   CFLAGS="$CFLAGS -D _REENTRANT"
elif test `uname` = "HP-UX"
then
   ARCH="HPUX"
   CFLAGS="$CFLAGS -D _REENTRANT"
elif test `uname` = "OpenBSD"
then
   ARCH="OPENBSD"
   CFLAGS="$CFLAGS -D _REENTRANT"
   test_kvm="true"
elif test `uname` = "FreeBSD"
then
   ARCH="FREEBSD"
   CFLAGS="$CFLAGS -D _REENTRANT"
   test_kvm="true"
elif test `uname` = "Darwin"
then
   ARCH="DARWIN"
   CFLAGS="$CFLAGS -DREENTRANT -no-cpp-precomp -DNEED_SOCKLEN_T_DEFINED"
   test_kvm="true"
   LIBS="$LIBS -framework System -multiply_defined suppress"
elif test `uname` = "AIX"
then
   ARCH="AIX"
   CFLAGS="$CFLAGS -D _REENTRANT"
else
   AC_MSG_WARN([Architecture not supported: `uname`.])
   CFLAGS="$CFLAGS -D _REENTRANT"
   ARCH="UNKNOWN"
fi
AC_SUBST(ARCH)

# ------------------------------------------------------------------------
# Resource code
# ------------------------------------------------------------------------

# Check if we want to have resource support
AC_MSG_CHECKING([for resource support])
AC_ARG_WITH(resource,
    [  --without-resource      disable resouce monitoring (default: enabled)],
    [
        dnl Check the withvalue                
        if test "x$withval" = "xno" ; then
            use_resource=0
            AC_MSG_RESULT([disabled])
        fi
        if test "x$withval" = "xyes" ; then
            use_resource=1
            AC_MSG_RESULT([enabled])
        fi
    ],
    [
	use_resource=1
	AC_MSG_RESULT([enabled])
    ]
)

# Settings for resource support
if test "$use_resource" = 1; then
	# Test for kvm libs
	kvm_libs=""
	if test "$test_kvm"; then
        	AC_CHECK_LIB([kvm],
        	     [kvm_open],
        	     [kvm_libs="-lkvm"])
        	if test "$kvm_libs"
        	then
        	        LIBS="$LIBS $kvm_libs"
        	fi
	fi
	PROCARCH="$ARCH"
else
	PROCARCH="UNKNOWN"
fi

AC_SUBST(PROCARCH)

# ------------------------------------------------------------------------
# SSL Code
# ------------------------------------------------------------------------

# Check for ssl includes (take from the stunnel project)
checksslincldir() { :
    if test -f "$1/openssl/ssl.h"; then
        sslincldir="$1"
        return 0
    fi
    return 1
}

# Check for ssl libraries 
checkssllibdir() { :
    if test "(" -f "$1/libcrypto.so" -o -f "$1/libcrypto.a" -o -f "$1/libcrypto.dylib" ")"  -a \
            "(" -f "$1/libssl.so"    -o -f "$1/libssl.a"    -o -f "$1/libssl.dylib" ")" ; then
        ssllibdir="$1"
        return 0
    fi
    return 1
}

# Check if we want to have SSL
AC_MSG_CHECKING([for SSL support])
AC_ARG_WITH(ssl,
    [  --without-ssl           disable the use of ssl (default: enabled)],
    [
        dnl Check the withvalue                
        if test "x$withval" = "xno" ; then
            use_ssl=0
            AC_MSG_RESULT([disabled])
        fi
        if test "x$withval" = "xyes" ; then
            use_ssl=1
            AC_MSG_RESULT([enabled])
        fi
    ],
    [
	use_ssl=1
        AC_MSG_RESULT([enabled])
    ]
)


# Check for SSL directory (take from the stunnel project)
if test "$use_ssl" = "1"; then
    AC_ARG_WITH(ssl-dir,
        [  --with-ssl-dir=DIR       location of SSL installation],
        [
            dnl Check the specified location only
            for dir in "$withval" "$withval/include"; do
            	checksslincldir "$dir"
            done
            for dir in "$withval" "$withval/lib"; do
                checkssllibdir "$dir" && break 2
            done
        ]
    )

    AC_MSG_CHECKING([for SSL include directory])

    AC_ARG_WITH(ssl-incl-dir,
        [  --with-ssl-incl-dir=DIR  location of installed SSL include files],
        [
            dnl Check the specified location only
            checksslincldir "$withval"
        ],
        [
            if test -z "$sslincldir"; then	
                dnl Search default locations of SSL includes
                for maindir in /usr /usr/local /usr/lib /usr/pkg /var /opt; do
                    for dir in "$maindir/include"\
                               "$maindir/include/openssl"\ 
                               "$maindir/include/ssl"\
                               "$maindir/ssl/include"; do
                        checksslincldir $dir && break 2
                    done
                done
            fi
        ]
    )

    if test -z "$sslincldir"; then
        AC_MSG_RESULT([Not found])
        echo
        echo "Couldn't find your SSL header files."
        echo "Use --with-ssl-incl-dir option to fix this problem or disable"
        echo "the SSL support with --without-ssl"
        echo
        exit 1
    fi

    AC_MSG_RESULT([$sslincldir])

    AC_MSG_CHECKING([for SSL library directory])

    AC_ARG_WITH(ssl-lib-dir,
        [  --with-ssl-lib-dir=DIR   location of installed SSL library files],
        [
            dnl Check the specified location only
            checkssllibdir "$withval"
        ],
        [
            if test -z "$ssllibdir"; then	
                dnl Search default locations of SSL libraries
                for maindir in /usr /usr/local /usr/pkg /var /opt; do
                    for dir in $maindir $maindir/openssl $maindir/ssl \
                               $maindir/lib $maindir/lib/openssl \
                               $maindir/lib/ssl $maindir/ssl/lib; do
                        checkssllibdir $dir && break 2
                    done
                done
            fi
        ]
    )
    
    if test -z "$ssllibdir"; then
        AC_MSG_RESULT([Not found])
        echo
        echo "Couldn't find your SSL library files."
        echo "Use --with-ssl-lib-dir option to fix this problem or disable the"
        echo "SSL support with --without-ssl"
        echo
        exit 1
    fi
    
    AC_MSG_RESULT([$ssllibdir])

    AC_DEFINE([HAVE_OPENSSL], 1, [Define to 1 if you have openssl])
    AC_SUBST(sslincldir)
    AC_SUBST(ssllibdir)
fi

# Add SSL includes and libraries
if test "$sslincldir" -a "$ssllibdir"
then
    CFLAGS="$CFLAGS -I$sslincldir"
    LIBS="$LIBS -L$ssllibdir -lssl -lcrypto"	
    # Darwin already knows about ssldirs
    if test "x$ARCH" = "xDARWIN"; then
     LIBS="$LIBS -lssl -lcrypto"
    else
     CFLAGS="$CFLAGS -I$sslincldir"
     LIBS="$LIBS -L$ssllibdir -lssl -lcrypto"
    fi
    # Redhat 9 compilation fix:
    if test -f "/usr/kerberos/include/krb5.h"
    then
        CFLAGS="$CFLAGS -I/usr/kerberos/include"
    fi
fi

# ------------------------------------------------------------------------
# Outputs
# ------------------------------------------------------------------------

AC_CONFIG_HEADER([config.h])
AC_CONFIG_FILES([Makefile])
AC_OUTPUT

echo ""
echo "monit has been configured with the following options:"

echo "                Architecture: ${ARCH}"
if test "$use_ssl" = "1"; then
    echo "                 SSL support: enabled"
    echo "       SSL include directory: ${sslincldir}"
    echo "       SSL library directory: ${ssllibdir}"
else
    echo "                 SSL support: disabled"
fi
if test "$use_resource" = "1"; then
    echo "         resource monitoring: enabled"  
else
    echo "         resource monitoring: disabled"
fi
echo "               resource code: sysdep_$PROCARCH.c"
echo "              Compiler flags: ${CFLAGS}"
echo "                Linker flags: ${LIBS}"
echo "           pid file location: ${piddir}"
echo
