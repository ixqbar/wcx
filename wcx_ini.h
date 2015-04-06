/*
 * wcx_ini.h
 *
 * Thanks php-yaf
 */

#ifndef EXT_WCX_WCX_INI_H_
#define EXT_WCX_WCX_INI_H_

#define WCX_CONFIG_INI_PARSING_START   0
#define WCX_CONFIG_INI_PARSING_PROCESS 1
#define WCX_CONFIG_INI_PARSING_END     2

void wcx_config_ini_parser_cb(zval *key, zval *value, zval *index, int callback_type, zval *arr TSRMLS_DC);

#endif /* EXT_WCX_WCX_INI_H_ */
