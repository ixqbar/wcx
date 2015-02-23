/*
  +----------------------------------------------------------------------+
  | PHP Version 5                                                        |
  +----------------------------------------------------------------------+
  | Copyright (c) 1997-2013 The PHP Group                                |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Author:                                                              |
  +----------------------------------------------------------------------+
*/

/* $Id$ */

#ifndef PHP_WCX_H
#define PHP_WCX_H

extern zend_module_entry wcx_module_entry;
#define phpext_wcx_ptr &wcx_module_entry

#define PHP_WCX_VERSION "0.2.1"

#ifdef PHP_WIN32
#	define PHP_WCX_API __declspec(dllexport)
#elif defined(__GNUC__) && __GNUC__ >= 4
#	define PHP_WCX_API __attribute__ ((visibility("default")))
#else
#	define PHP_WCX_API
#endif

#ifdef ZTS
#include "TSRM.h"
#endif

PHP_MINIT_FUNCTION(wcx);
PHP_MSHUTDOWN_FUNCTION(wcx);
PHP_RINIT_FUNCTION(wcx);
PHP_RSHUTDOWN_FUNCTION(wcx);
PHP_MINFO_FUNCTION(wcx);

PHP_FUNCTION(wcx_version);
PHP_FUNCTION(wcx_encrypt);
PHP_FUNCTION(wcx_decrypt);
PHP_FUNCTION(wcx_array_rand);
PHP_FUNCTION(wcx_bet);
PHP_FUNCTION(wcx_lock);
PHP_FUNCTION(wcx_unlock);
PHP_FUNCTION(wcx_task_info);
PHP_FUNCTION(wcx_task_clear);
PHP_FUNCTION(wcx_task_delete);
PHP_FUNCTION(wcx_task_post);

ZEND_BEGIN_MODULE_GLOBALS(wcx)
	long wcx_task_running;
	void *wcx_task_ptr;
ZEND_END_MODULE_GLOBALS(wcx)

ZEND_DECLARE_MODULE_GLOBALS(wcx)

#ifdef ZTS
#define WCX_G(v) TSRMG(wcx_globals_id, zend_wcx_globals *, v)
#else
#define WCX_G(v) (wcx_globals.v)
#endif

#endif	/* PHP_WCX_H */


/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
