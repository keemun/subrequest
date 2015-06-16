/*
  +----------------------------------------------------------------------+
  | PHP Version 5                                                        |
  +----------------------------------------------------------------------+
  | Copyright (c) 1997-2010 The PHP Group                                |
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

/* $Id: header 297205 2010-03-30 21:09:07Z johannes $ */

#ifndef PHP_SUBREQUEST_H
#define PHP_SUBREQUEST_H

extern zend_module_entry subrequest_module_entry;
#define phpext_subrequest_ptr &subrequest_module_entry

#ifdef PHP_WIN32
#define PHP_SUBREQUEST_API __declspec(dllexport)
#else
#define PHP_SUBREQUEST_API
#endif

#ifdef ZTS
#include "TSRM.h"
#endif

PHP_MINIT_FUNCTION(subrequest);
PHP_MSHUTDOWN_FUNCTION(subrequest);
PHP_RINIT_FUNCTION(subrequest);
PHP_RSHUTDOWN_FUNCTION(subrequest);
PHP_MINFO_FUNCTION(subrequest);

PHP_FUNCTION(subreq_set_domain);
PHP_FUNCTION(subreq_set_param);
PHP_FUNCTION(subrequest);
PHP_FUNCTION(subreq_close);


#ifdef ZTS
#define SUBREQUEST_G(v) TSRMG(subrequest_globals_id, zend_subrequest_globals *, v)
#else
#define SUBREQUEST_G(v) (subrequest_globals.v)
#endif

#endif


/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
