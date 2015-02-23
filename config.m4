dnl $Id$
dnl config.m4 for extension wcx

dnl Comments in this file start with the string 'dnl'.
dnl Remove where necessary. This file will not work
dnl without editing.

dnl If your extension references something external, use with:

dnl PHP_ARG_WITH(wcx, for wcx support,
dnl Make sure that the comment is aligned:
dnl [  --with-wcx             Include wcx support])

dnl Otherwise use enable:

PHP_ARG_ENABLE(wcx, whether to enable wcx support,
Make sure that the comment is aligned:
[  --enable-wcx           Enable wcx support])

if test "$PHP_WCX" != "no"; then
  dnl Write more examples of tests here...

  dnl # --with-wcx -> check with-path
  dnl SEARCH_PATH="/usr/local /usr"     # you might want to change this
  dnl SEARCH_FOR="/include/wcx.h"  # you most likely want to change this
  dnl if test -r $PHP_WCX/$SEARCH_FOR; then # path given as parameter
  dnl   WCX_DIR=$PHP_WCX
  dnl else # search default path list
  dnl   AC_MSG_CHECKING([for wcx files in default path])
  dnl   for i in $SEARCH_PATH ; do
  dnl     if test -r $i/$SEARCH_FOR; then
  dnl       WCX_DIR=$i
  dnl       AC_MSG_RESULT(found in $i)
  dnl     fi
  dnl   done
  dnl fi
  dnl
  dnl if test -z "$WCX_DIR"; then
  dnl   AC_MSG_RESULT([not found])
  dnl   AC_MSG_ERROR([Please reinstall the wcx distribution])
  dnl fi

  dnl # --with-wcx -> add include path
  dnl PHP_ADD_INCLUDE($WCX_DIR/include)

  dnl # --with-wcx -> check for lib and symbol presence
  dnl LIBNAME=wcx # you may want to change this
  dnl LIBSYMBOL=wcx # you most likely want to change this 

  dnl PHP_CHECK_LIBRARY($LIBNAME,$LIBSYMBOL,
  dnl [
  dnl   PHP_ADD_LIBRARY_WITH_PATH($LIBNAME, $WCX_DIR/lib, WCX_SHARED_LIBADD)
  dnl   AC_DEFINE(HAVE_WCXLIB,1,[ ])
  dnl ],[
  dnl   AC_MSG_ERROR([wrong wcx lib version or lib not found])
  dnl ],[
  dnl   -L$WCX_DIR/lib -lm
  dnl ])
  dnl
  dnl PHP_SUBST(WCX_SHARED_LIBADD)

  PHP_NEW_EXTENSION(wcx,wcx_list.c wcx_task.c wcx.c,$ext_shared)
fi
