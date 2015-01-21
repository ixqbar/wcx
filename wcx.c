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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/php_rand.h"
#include "ext/standard/info.h"
#include "ext/standard/crc32.h"
#include "php_wcx.h"

#include "aes.c"
#include "zlib.c"

/* If you declare any globals in php_wcx.h uncomment this:
ZEND_DECLARE_MODULE_GLOBALS(wcx)
*/

/* True global resources - no need for thread safety here */
static int le_wcx;

ZEND_BEGIN_ARG_INFO_EX(arg_info_wcx_encrypt, 0, 0, 0)
	ZEND_ARG_INFO(0, encrypt_str)
	ZEND_ARG_INFO(0, encrypt_key)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arg_info_wcx_decrypt, 0, 0, 0)
	ZEND_ARG_INFO(0, decrypt_str)
	ZEND_ARG_INFO(0, decrypt_key)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arg_info_wcx_array_rand, 0, 0, 0)
	ZEND_ARG_INFO(0, arr)
	ZEND_ARG_INFO(0, arr_num)
ZEND_END_ARG_INFO()

/* {{{ wcx_functions[]
 *
 * Every user visible function must have an entry in wcx_functions[].
 */
const zend_function_entry wcx_functions[] = {
	PHP_FE(wcx_encrypt,	   arg_info_wcx_encrypt)
	PHP_FE(wcx_decrypt,	   arg_info_wcx_decrypt)
	PHP_FE(wcx_array_rand, arg_info_wcx_array_rand)
	PHP_FE_END	/* Must be the last line in wcx_functions[] */
};
/* }}} */

/* {{{ wcx_module_entry
 */
zend_module_entry wcx_module_entry = {
#if ZEND_MODULE_API_NO >= 20010901
	STANDARD_MODULE_HEADER,
#endif
	"wcx",
	wcx_functions,
	PHP_MINIT(wcx),
	PHP_MSHUTDOWN(wcx),
	PHP_RINIT(wcx),		/* Replace with NULL if there's nothing to do at request start */
	PHP_RSHUTDOWN(wcx),	/* Replace with NULL if there's nothing to do at request end */
	PHP_MINFO(wcx),
#if ZEND_MODULE_API_NO >= 20010901
	PHP_WCX_VERSION,
#endif
	STANDARD_MODULE_PROPERTIES
};
/* }}} */

#ifdef COMPILE_DL_WCX
ZEND_GET_MODULE(wcx)
#endif

/* {{{ PHP_INI
 */
/* Remove comments and fill if you need to have entries in php.ini
PHP_INI_BEGIN()
    STD_PHP_INI_ENTRY("wcx.global_value",      "42", PHP_INI_ALL, OnUpdateLong, global_value, zend_wcx_globals, wcx_globals)
    STD_PHP_INI_ENTRY("wcx.global_string", "foobar", PHP_INI_ALL, OnUpdateString, global_string, zend_wcx_globals, wcx_globals)
PHP_INI_END()
*/
/* }}} */

/* {{{ php_wcx_init_globals
 */
/* Uncomment this function if you have INI entries
static void php_wcx_init_globals(zend_wcx_globals *wcx_globals)
{
	wcx_globals->global_value = 0;
	wcx_globals->global_string = NULL;
}
*/
/* }}} */

/* {{{ PHP_MINIT_FUNCTION
 */
PHP_MINIT_FUNCTION(wcx)
{
	/* If you have INI entries, uncomment these lines 
	REGISTER_INI_ENTRIES();
	*/
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION
 */
PHP_MSHUTDOWN_FUNCTION(wcx)
{
	/* uncomment this line if you have INI entries
	UNREGISTER_INI_ENTRIES();
	*/
	return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request start */
/* {{{ PHP_RINIT_FUNCTION
 */
PHP_RINIT_FUNCTION(wcx)
{
	return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request end */
/* {{{ PHP_RSHUTDOWN_FUNCTION
 */
PHP_RSHUTDOWN_FUNCTION(wcx)
{
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION
 */
PHP_MINFO_FUNCTION(wcx)
{
	php_info_print_table_start();
	php_info_print_table_header(2, "wcx support", "enabled");
	php_info_print_table_end();

	/* Remove comments if you have entries in php.ini
	DISPLAY_INI_ENTRIES();
	*/
}
/* }}} */


PHP_FUNCTION(wcx_encrypt)
{
	char *encrypt_data = NULL, *encrypt_key=NULL;
	int encrypt_data_len, encrypt_key_len;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ss", &encrypt_data, &encrypt_data_len, &encrypt_key, &encrypt_key_len) == FAILURE) {
		RETURN_NULL();
	}

	if (0 == encrypt_data_len
			|| 0 == encrypt_key_len) {
		RETURN_NULL();
	}

	int crc = 0^0xFFFFFFFF;
	int n = encrypt_data_len;

	while (n--) {
		crc = ((crc >> 8) & 0x00FFFFFF) ^ crc32tab[(crc ^ (*encrypt_data++)) & 0xFF ];
	}
	crc = crc^0xFFFFFFFF;

	//reset pointer to begin
	encrypt_data -= encrypt_data_len;

	//RETVAL_LONG(crc);

	//check to gzcompress
	int gzip = encrypt_data_len > 100 ? 1 : 0;

	char *aes_data;
	size_t aes_data_len = encrypt_data_len;

	if (gzip) {
		int level = -1;
		int encoding = 0x0f;

		if (SUCCESS != php_zlib_encode(encrypt_data, encrypt_data_len, &aes_data, &aes_data_len, encoding, level)) {
			RETURN_NULL();
		}
		//php_printf("php-extension gzcompress len=%zd\n", gzip_len);
	} else {
		aes_data = estrndup(encrypt_data, encrypt_data_len);
	}

	//RETURN_STRINGL(aes_data, aes_data_len, 0);

	//header
	char header[33];
	sprintf(header, "ok,%d,%lu,%u", gzip, aes_data_len, crc);
	size_t i;
	for (i = strlen(header); i < 32; i++) {
		header[i] = ' ';
	}
	header[32] = '\0';

	//rand iv
	char aes_iv[17];
	int j;
	for (j = 0; j < 16; j++) {
		int ch = rand() % 255;
		aes_iv[j] = (char)ch;
	}
	aes_iv[16] = '\0';

	int encrypt_times = (aes_data_len + 32) / 16 + 1;
	int paddings      = 16 - (aes_data_len + 32) % 16;
	int aes_buf_size  = (aes_data_len + 32) + paddings;

	char *result = (char *)emalloc(16 + sizeof(char) * aes_buf_size);
	memcpy(result, aes_iv, 16);

	char * aes_origin_buf = (char *)emalloc(sizeof(char) * aes_buf_size);
	memset(aes_origin_buf, 0, sizeof(char) * aes_buf_size);
	memcpy(aes_origin_buf, header, 32);
	memcpy(aes_origin_buf + 32, aes_data, aes_data_len);

	char * aes_finnal_buf = (char *)emalloc(sizeof(char) * aes_buf_size);
	memset(aes_finnal_buf, 0, sizeof(char) * aes_buf_size);

	char aes_key[17];
	memset(aes_key, ' ', sizeof(aes_key));
	if (encrypt_key_len < 16) {
		memcpy(aes_key, encrypt_key, encrypt_key_len);
	} else {
		memcpy(aes_key, encrypt_key, 16);
	}

	aes_context aes_ctx;
	aes_set_key((unsigned char *)aes_key, 16, &aes_ctx);

	if (SUCCESS != aes_cbc_encrypt((unsigned char *)aes_origin_buf, (unsigned char *)aes_finnal_buf, encrypt_times, (unsigned char *)aes_iv, &aes_ctx)) {
		efree(aes_data);
		efree(aes_origin_buf);
		efree(aes_finnal_buf);
		RETURN_NULL();
	}

	memcpy(result + 16, aes_finnal_buf, aes_buf_size);

	efree(aes_data);
	efree(aes_origin_buf);
	efree(aes_finnal_buf);

    RETVAL_STRINGL(result, 16 + aes_buf_size, 1);
    efree(result);
    return;
}


PHP_FUNCTION(wcx_decrypt)
{
	char *decrypt_data = NULL, *decrypt_key=NULL;
	int decrypt_data_len, decrypt_key_len;

	array_init(return_value);
	add_index_bool(return_value, 0, 1);
	add_index_string(return_value, 1, "", 1);

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ss", &decrypt_data, &decrypt_data_len, &decrypt_key, &decrypt_key_len) == FAILURE) {
		return;
	}

	if (decrypt_data_len < 48
			|| decrypt_data_len % 16 != 0
			|| 0 == decrypt_key_len) {
		return;
	}

	//iv
	char aes_iv[17];
	memcpy(aes_iv, decrypt_data, 16);
	aes_iv[16] = '\0';

	int aes_buf_size  = decrypt_data_len - 16;
	int decrypt_times = aes_buf_size / 16;

	char *aes_finnal_buf = (char *)emalloc(sizeof(char) * aes_buf_size);
	memset(aes_finnal_buf, 0, sizeof(char) * aes_buf_size);
	memcpy(aes_finnal_buf, decrypt_data + 16, sizeof(char) * aes_buf_size);

	char *aes_origin_buf = (char *)emalloc(sizeof(char) * aes_buf_size);
	memset(aes_origin_buf, 0, sizeof(char) * aes_buf_size);

	char aes_key[17];
	memset(aes_key, ' ', sizeof(aes_key));
	if (decrypt_key_len < 16) {
		memcpy(aes_key, decrypt_key, decrypt_key_len);
	} else {
		memcpy(aes_key, decrypt_key, 16);
	}

	aes_context aes_ctx;
	aes_set_key((unsigned char *)aes_key, 16, &aes_ctx);
	if (SUCCESS != aes_cbc_decrypt((unsigned char *)aes_finnal_buf, (unsigned char *)aes_origin_buf, decrypt_times, (unsigned char *)aes_iv, &aes_ctx)) {
		efree(aes_finnal_buf);
		efree(aes_origin_buf);
		return;
	}

	efree(aes_finnal_buf);

	char header[33];
	memcpy(header, aes_origin_buf, 32);
	header[32] = '\0';

	char* header_flags[4];
	char* flags = strtok(header, ",");
	int i;
	for (i = 0; i < 4; i++) {
		header_flags[i] = flags;
		flags = strtok(NULL, ",");
	}

	if (strcmp(header_flags[0], "ok") != 0) {
		efree(aes_origin_buf);
		return;
	}

	int is_zip = 0;
	if (strcmp(header_flags[1], "1") == 0) {
		is_zip = 1;
	}

	size_t origin_data_size = strtoul(header_flags[2], NULL, 10);

	char *origin_data;
	size_t origin_data_len = origin_data_size;

	if (is_zip) {
		int encoding = 0x0f;
		if (SUCCESS != php_zlib_decode(aes_origin_buf + 32, origin_data_size, &origin_data, &origin_data_len, encoding, 0)) {
			efree(aes_origin_buf);
			return;
		}
	} else {
		origin_data = estrndup(aes_origin_buf + 32, origin_data_len);
	}

	efree(aes_origin_buf);

	int crc = 0^0xFFFFFFFF;
	size_t n = origin_data_len;

	while (n--) {
		crc = ((crc >> 8) & 0x00FFFFFF) ^ crc32tab[(crc ^ (*origin_data++)) & 0xFF ];
	}
	crc = crc^0xFFFFFFFF;

	origin_data -= origin_data_len;

	if (crc != atoi(header_flags[3])) {
		efree(origin_data);
		return;
	}

	add_index_bool(return_value, 0, 0);
	add_index_stringl(return_value, 1, origin_data, origin_data_len, 0);
}


PHP_FUNCTION(wcx_array_rand)
{
	zval *input;
	long randval, num_req = 1;
	int num_avail, key_type, rand_num;
	char *string_key;
	uint string_key_len;
	ulong num_key;
	HashPosition pos;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "a|l", &input, &num_req) == FAILURE) {
		return;
	}

	num_avail = zend_hash_num_elements(Z_ARRVAL_P(input));

	if (ZEND_NUM_ARGS() > 1) {
		if (num_req <= 0 || num_req > num_avail) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "Second argument has to be between 1 and the number of elements in the array");
			return;
		}
	}

	rand_num = num_req;
	array_init_size(return_value, num_req);

	zval *temp;
	zval **entry;

	zend_hash_internal_pointer_reset_ex(Z_ARRVAL_P(input), &pos);
	while (num_req
		&& (key_type = zend_hash_get_current_key_ex(Z_ARRVAL_P(input), &string_key, &string_key_len, &num_key, 0, &pos)) != HASH_KEY_NON_EXISTENT
		&& zend_hash_get_current_data_ex(Z_ARRVAL_P(input), (void**)&entry, &pos) == SUCCESS) {
		randval = php_rand(TSRMLS_C);
		if ((double) (randval / (PHP_RAND_MAX + 1.0)) < (double) num_req / (double) num_avail) {
			zval_add_ref(entry);
			if (key_type == HASH_KEY_IS_STRING) {
				if (1 == rand_num) {
					add_index_stringl(return_value, 0, string_key, string_key_len - 1, 1);
					add_index_zval(return_value, 1, *entry);
				} else {
					MAKE_STD_ZVAL(temp);
					array_init_size(temp, 2);
					add_index_stringl(temp, 0, string_key, string_key_len - 1, 1);
					add_index_zval(temp, 1, *entry);

					add_next_index_zval(return_value, temp);
					temp = NULL;
				}
			} else {
				if (1 == rand_num) {
					add_index_long(return_value, 0, num_key);
					add_index_zval(return_value, 1, *entry);
				} else {
					MAKE_STD_ZVAL(temp);
					array_init_size(temp, 2);
					add_index_long(temp, 0, num_key);
					add_index_zval(temp, 1, *entry);

					add_next_index_zval(return_value, temp);
					temp = NULL;
				}
			}
			num_req--;
		}
		num_avail--;
		zend_hash_move_forward_ex(Z_ARRVAL_P(input), &pos);
	}
}

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
