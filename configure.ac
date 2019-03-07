AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_SRCDIR([src/main.c])

AC_INIT([LANScanner], [1.0.2])
AM_INIT_AUTOMAKE([foreign])

AC_PROG_CC

AX_CHECK_COMPILE_FLAG([-Wall], [AX_APPEND_FLAG([-Wall])],
                      AC_MSG_WARN([-Wall not supported by C compiler]))
AX_CHECK_COMPILE_FLAG([-Wpedantic], [AX_APPEND_FLAG([-Wpedantic])],
                      AC_MSG_WARN([-Wpedantic not supported by C compiler]))
AX_CHECK_COMPILE_FLAG([-Werror], [AX_APPEND_FLAG([-Werror])],
                      AC_MSG_WARN([-Werror not supported by C compiler]))

AC_OUTPUT(Makefile src/Makefile)

