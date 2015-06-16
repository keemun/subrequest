dnl $Id$
dnl config.m4 for extension subrequest

dnl Comments in this file start with the string 'dnl'.
dnl Remove where necessary. This file will not work
dnl without editing.

dnl If your extension references something external, use with:

dnl PHP_ARG_WITH(subrequest, for subrequest support,
dnl Make sure that the comment is aligned:
dnl [  --with-subrequest             Include subrequest support])

dnl Otherwise use enable:

PHP_ARG_ENABLE(subrequest, whether to enable subrequest support,
dnl Make sure that the comment is aligned:
[  --enable-subrequest           Enable subrequest support])

if test "$PHP_SUBREQUEST" != "no"; then
  dnl Write more examples of tests here...

  dnl # --with-subrequest -> check with-path
  dnl SEARCH_PATH="/usr/local /usr"     # you might want to change this
  dnl SEARCH_FOR="/include/subrequest.h"  # you most likely want to change this
  dnl if test -r $PHP_SUBREQUEST/$SEARCH_FOR; then # path given as parameter
  dnl   SUBREQUEST_DIR=$PHP_SUBREQUEST
  dnl else # search default path list
  dnl   AC_MSG_CHECKING([for subrequest files in default path])
  dnl   for i in $SEARCH_PATH ; do
  dnl     if test -r $i/$SEARCH_FOR; then
  dnl       SUBREQUEST_DIR=$i
  dnl       AC_MSG_RESULT(found in $i)
  dnl     fi
  dnl   done
  dnl fi
  dnl
  dnl if test -z "$SUBREQUEST_DIR"; then
  dnl   AC_MSG_RESULT([not found])
  dnl   AC_MSG_ERROR([Please reinstall the subrequest distribution])
  dnl fi

  dnl # --with-subrequest -> add include path
  dnl PHP_ADD_INCLUDE($SUBREQUEST_DIR/include)

  dnl # --with-subrequest -> check for lib and symbol presence
  dnl LIBNAME=subrequest # you may want to change this
  dnl LIBSYMBOL=subrequest # you most likely want to change this 

  dnl PHP_CHECK_LIBRARY($LIBNAME,$LIBSYMBOL,
  dnl [
  dnl   PHP_ADD_LIBRARY_WITH_PATH($LIBNAME, $SUBREQUEST_DIR/lib, SUBREQUEST_SHARED_LIBADD)
  dnl   AC_DEFINE(HAVE_SUBREQUESTLIB,1,[ ])
  dnl ],[
  dnl   AC_MSG_ERROR([wrong subrequest lib version or lib not found])
  dnl ],[
  dnl   -L$SUBREQUEST_DIR/lib -lm -ldl
  dnl ])
  dnl
  dnl PHP_SUBST(SUBREQUEST_SHARED_LIBADD)

  PHP_NEW_EXTENSION(subrequest, subrequest.c connection.c, $ext_shared)
fi
