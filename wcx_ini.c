/*
 * wcx_ini.c
 *
 * Thanks php-yaf
 */

#include "php.h"
#include "php_ini.h"
#include "wcx_ini.h"
#include "php_wcx.h"

static inline void wcx_deep_copy_section(zval *dst, zval *src TSRMLS_DC) {
	zval **ppzval, **dstppzval, *value;
	HashTable *ht;
	ulong idx;
	char *key;
	uint key_len;

	ht = Z_ARRVAL_P(src);
	for(zend_hash_internal_pointer_reset(ht);
			zend_hash_has_more_elements(ht) == SUCCESS;
			zend_hash_move_forward(ht)) {
		if (zend_hash_get_current_data(ht, (void **)&ppzval) == FAILURE) {
			continue;
		}

		switch (zend_hash_get_current_key_ex(ht, &key, &key_len, &idx, 0, NULL)) {
			case HASH_KEY_IS_STRING:
				if (Z_TYPE_PP(ppzval) == IS_ARRAY
						&& zend_hash_find(Z_ARRVAL_P(dst), key, key_len, (void **)&dstppzval) == SUCCESS
						&& Z_TYPE_PP(dstppzval) == IS_ARRAY) {
					MAKE_STD_ZVAL(value);
					array_init(value);
					wcx_deep_copy_section(value, *dstppzval TSRMLS_CC);
					wcx_deep_copy_section(value, *ppzval TSRMLS_CC);
				} else {
					value = *ppzval;
					Z_ADDREF_P(value);
				}
				zend_hash_update(Z_ARRVAL_P(dst), key, key_len, (void *)&value, sizeof(zval *), NULL);
				break;
			case HASH_KEY_IS_LONG:
				if (Z_TYPE_PP(ppzval) == IS_ARRAY
						&& zend_hash_index_find(Z_ARRVAL_P(dst), idx, (void **)&dstppzval) == SUCCESS
						&& Z_TYPE_PP(dstppzval) == IS_ARRAY) {
					MAKE_STD_ZVAL(value);
					array_init(value);
					wcx_deep_copy_section(value, *dstppzval TSRMLS_CC);
					wcx_deep_copy_section(value, *ppzval TSRMLS_CC);
				} else {
					value = *ppzval;
					Z_ADDREF_P(value);
				}
				zend_hash_index_update(Z_ARRVAL_P(dst), idx, (void *)&value, sizeof(zval *), NULL);
				break;
			case HASH_KEY_NON_EXISTENT:
				break;
		}
	}
}

static void wcx_config_ini_simple_parser_cb(zval *key, zval *value, zval *index, int callback_type, zval *arr TSRMLS_DC) {
	zval *element;
	switch (callback_type) {
		case ZEND_INI_PARSER_ENTRY:
			{
				char *skey, *seg, *ptr;
				zval **ppzval, *dst;

				if (!value) {
					break;
				}

				dst = arr;
				skey = estrndup(Z_STRVAL_P(key), Z_STRLEN_P(key));
				if ((seg = php_strtok_r(skey, ".", &ptr))) {
					do {
					    char *real_key = seg;
						seg = php_strtok_r(NULL, ".", &ptr);
						if (zend_symtable_find(Z_ARRVAL_P(dst), real_key, strlen(real_key) + 1, (void **) &ppzval) == FAILURE) {
							if (seg) {
								zval *tmp;
							    MAKE_STD_ZVAL(tmp);
								array_init(tmp);
								zend_symtable_update(Z_ARRVAL_P(dst),
										real_key, strlen(real_key) + 1, (void **)&tmp, sizeof(zval *), (void **)&ppzval);
							} else {
							    MAKE_STD_ZVAL(element);
								ZVAL_ZVAL(element, value, 1, 0);
								zend_symtable_update(Z_ARRVAL_P(dst),
										real_key, strlen(real_key) + 1, (void **)&element, sizeof(zval *), NULL);
								break;
							}
						} else {
							SEPARATE_ZVAL(ppzval);
							if (IS_ARRAY != Z_TYPE_PP(ppzval)) {
								if (seg) {
									zval *tmp;
									MAKE_STD_ZVAL(tmp);
									array_init(tmp);
									zend_symtable_update(Z_ARRVAL_P(dst),
											real_key, strlen(real_key) + 1, (void **)&tmp, sizeof(zval *), (void **)&ppzval);
								} else {
									MAKE_STD_ZVAL(element);
									ZVAL_ZVAL(element, value, 1, 0);
									zend_symtable_update(Z_ARRVAL_P(dst),
											real_key, strlen(real_key) + 1, (void **)&element, sizeof(zval *), NULL);
								}
							}
						}
						dst = *ppzval;
					} while (seg);
				}
				efree(skey);
			}
			break;

		case ZEND_INI_PARSER_POP_ENTRY:
			{
				zval *hash, **find_hash, *dst;

				if (!value) {
					break;
				}

				if (!(Z_STRLEN_P(key) > 1 && Z_STRVAL_P(key)[0] == '0')
						&& is_numeric_string(Z_STRVAL_P(key), Z_STRLEN_P(key), NULL, NULL, 0) == IS_LONG) {
					ulong skey = (ulong)zend_atol(Z_STRVAL_P(key), Z_STRLEN_P(key));
					if (zend_hash_index_find(Z_ARRVAL_P(arr), skey, (void **) &find_hash) == FAILURE) {
						MAKE_STD_ZVAL(hash);
						array_init(hash);
						zend_hash_index_update(Z_ARRVAL_P(arr), skey, &hash, sizeof(zval *), NULL);
					} else {
						hash = *find_hash;
					}
				} else {
					char *seg, *ptr;
					char *skey = estrndup(Z_STRVAL_P(key), Z_STRLEN_P(key));

					dst = arr;
					if ((seg = php_strtok_r(skey, ".", &ptr))) {
						while (seg) {
							if (zend_symtable_find(Z_ARRVAL_P(dst), seg, strlen(seg) + 1, (void **) &find_hash) == FAILURE) {
								MAKE_STD_ZVAL(hash);
								array_init(hash);
								zend_symtable_update(Z_ARRVAL_P(dst),
										seg, strlen(seg) + 1, (void **)&hash, sizeof(zval *), (void **)&find_hash);
							}
							dst = *find_hash;
							seg = php_strtok_r(NULL, ".", &ptr);
						}
						hash = dst;
					} else {
						if (zend_symtable_find(Z_ARRVAL_P(dst), seg, strlen(seg) + 1, (void **)&find_hash) == FAILURE) {
							MAKE_STD_ZVAL(hash);
							array_init(hash);
							zend_symtable_update(Z_ARRVAL_P(dst), seg, strlen(seg) + 1, (void **)&hash, sizeof(zval *), NULL);
						} else {
							hash = *find_hash;
						}
					}
					efree(skey);
				}

				if (Z_TYPE_P(hash) != IS_ARRAY) {
					zval_dtor(hash);
					INIT_PZVAL(hash);
					array_init(hash);
				}

				MAKE_STD_ZVAL(element);
				ZVAL_ZVAL(element, value, 1, 0);

				if (index && Z_STRLEN_P(index) > 0) {
					add_assoc_zval_ex(hash, Z_STRVAL_P(index), Z_STRLEN_P(index) + 1, element);
				} else {
					add_next_index_zval(hash, element);
				}
			}
			break;

		case ZEND_INI_PARSER_SECTION:
			break;
	}
}

void wcx_config_ini_parser_cb(zval *key, zval *value, zval *index, int callback_type, zval *arr TSRMLS_DC) {
	if (WCX_G(parsing_flag) == WCX_CONFIG_INI_PARSING_END) {
		return;
	}

	if (callback_type == ZEND_INI_PARSER_SECTION) {
		zval **parent;
		char *seg, *skey, *skey_orig;
		uint skey_len;

		if (WCX_G(parsing_flag) == WCX_CONFIG_INI_PARSING_PROCESS) {
			WCX_G(parsing_flag) = WCX_CONFIG_INI_PARSING_END;
			return;
		}

		skey_orig = skey = estrndup(Z_STRVAL_P(key), Z_STRLEN_P(key));
		skey_len = Z_STRLEN_P(key);
		while (*skey == ' ') {
			*(skey++) = '\0';
			skey_len--;
		}
		if (skey_len > 1) {
			seg = skey + skey_len - 1;
			while (*seg == ' ' || *seg == ':') {
				*(seg--) = '\0';
				skey_len--;
			}
		}

		MAKE_STD_ZVAL(WCX_G(active_ini_file_section));
		array_init(WCX_G(active_ini_file_section));

		if ((seg = strchr(skey, ':'))) {
			char *section, *p;

			if (seg > skey) {
				p = seg - 1;
				while (*p == ' ' || *p == ':') {
					*(p--) = '\0';
				}
			}

			while (*(seg) == ' ' || *(seg) == ':') {
				*(seg++) = '\0';
			}

			if ((section = strrchr(seg, ':'))) {
			    /* muilt-inherit */
				do {
					if (section > seg) {
						p = section - 1;
						while (*p == ' ' || *p == ':') {
							*(p--) = '\0';
						}
					}
					while (*(section) == ' ' || *(section) == ':') {
						*(section++) = '\0';
					}
					if (zend_symtable_find(Z_ARRVAL_P(arr), section, strlen(section) + 1, (void **)&parent) == SUCCESS) {
						wcx_deep_copy_section(WCX_G(active_ini_file_section), *parent TSRMLS_CC);
					}
				} while ((section = strrchr(seg, ':')));
			}

			if (zend_symtable_find(Z_ARRVAL_P(arr), seg, strlen(seg) + 1, (void **)&parent) == SUCCESS) {
				wcx_deep_copy_section(WCX_G(active_ini_file_section), *parent TSRMLS_CC);
			}
			skey_len = strlen(skey);
		}
		zend_symtable_update(Z_ARRVAL_P(arr), skey, skey_len + 1, &WCX_G(active_ini_file_section), sizeof(zval *), NULL);
		if (WCX_G(ini_wanted_section) && Z_STRLEN_P(WCX_G(ini_wanted_section)) == skey_len
				&& !strncasecmp(Z_STRVAL_P(WCX_G(ini_wanted_section)), skey, skey_len)) {
			WCX_G(parsing_flag) = WCX_CONFIG_INI_PARSING_PROCESS;
		}
		efree(skey_orig);
	} else if (value) {
		zval *active_arr;
		if (WCX_G(active_ini_file_section)) {
			active_arr = WCX_G(active_ini_file_section);
		} else {
			active_arr = arr;
		}
		wcx_config_ini_simple_parser_cb(key, value, index, callback_type, active_arr TSRMLS_CC);
	}
}
