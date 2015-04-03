/*
 * wcx_data.c
 *
 * Thanks php-yaf
 *
 */

#include "php.h"
#include "Zend/zend_interfaces.h"

#include "wcx_data.h"

zend_class_entry *wcx_data_class_entry;

#ifdef HAVE_SPL
extern PHPAPI zend_class_entry *spl_ce_Countable;
#endif

/** {{{ ARG_INFO
 */
ZEND_BEGIN_ARG_INFO_EX(wcx_data_void_arginfo, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(wcx_data_get_arginfo, 0, 0, 0)
	ZEND_ARG_INFO(0, name)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(wcx_data_rget_arginfo, 0, 0, 1)
	ZEND_ARG_INFO(0, name)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(wcx_data_set_arginfo, 0, 0, 2)
	ZEND_ARG_INFO(0, name)
	ZEND_ARG_INFO(0, value)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(wcx_data_isset_arginfo, 0, 0, 1)
	ZEND_ARG_INFO(0, name)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(wcx_data_unset_arginfo, 0, 0, 1)
	ZEND_ARG_INFO(0, name)
ZEND_END_ARG_INFO()
/* }}} */

zval * wcx_data_instance(zval *this_ptr, zval *values, zval *readonly TSRMLS_DC) {
	zval *instance;

	switch (Z_TYPE_P(values)) {
		case IS_ARRAY:
			if (this_ptr) {
				instance = this_ptr;
			} else {
				MAKE_STD_ZVAL(instance);
				object_init_ex(instance, wcx_data_class_entry);
			}
			zend_update_property(wcx_data_class_entry, instance, ZEND_STRL(WCX_DATA_PROPERT_NAME), values TSRMLS_CC);
			if (readonly) {
				convert_to_boolean(readonly);
				zend_update_property_bool(wcx_data_class_entry, instance, ZEND_STRL(WCX_DATA_PROPERT_NAME_READONLY), Z_BVAL_P(readonly) TSRMLS_CC);
			}
			return instance;
		break;
		default:
			php_error_docref(NULL TSRMLS_CC, E_ERROR, "Invalid parameters provided, must be an array");
			return NULL;
	}
}

zval * wcx_data_format(zval *instance, zval **ppzval TSRMLS_DC) {
	zval *readonly, *ret;
	readonly = zend_read_property(wcx_data_class_entry, instance, ZEND_STRL(WCX_DATA_PROPERT_NAME_READONLY), 1 TSRMLS_CC);
	ret = wcx_data_instance(NULL, *ppzval, readonly TSRMLS_CC);

	return ret;
}

/**
 *
 * {{{ proto public WcxData::__construct(mixed $array, string $readonly)
 */
PHP_METHOD(wcx_data, __construct) {
	zval *values, *readonly = NULL;
	int num_args = ZEND_NUM_ARGS();

	if (0 == num_args) {
		MAKE_STD_ZVAL(values);
		array_init(values);
	} else {
		if (zend_parse_parameters(num_args TSRMLS_CC, "z|z", &values, &readonly) == FAILURE) {
			return;
		}
	}

	wcx_data_instance(getThis(), values, readonly TSRMLS_CC);
}
/** }}} */

/*
 *
 * {{{ proto public WcxData::get(string $name = NULL, mixed $default_value = NULL)
 */
PHP_METHOD(wcx_data, get) {
	zval *ret, **ppzval, *default_value = NULL;
	char *name;
	uint len = 0;
	int num_args = ZEND_NUM_ARGS();

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|sz", &name, &len, &default_value) == FAILURE) {
		return;
	}

	if (0 == num_args) {
		RETURN_ZVAL(getThis(), 1, 0);
	}

	zval *properties;
	HashTable *hash;
	long lval;
	double dval;

	properties = zend_read_property(wcx_data_class_entry, getThis(), ZEND_STRL(WCX_DATA_PROPERT_NAME), 1 TSRMLS_CC);
	hash  = Z_ARRVAL_P(properties);

	if (is_numeric_string(name, len, &lval, &dval, 0) != IS_LONG) {
		if (zend_hash_find(hash, name, len + 1, (void **) &ppzval) == FAILURE) {
			goto GET_RETURN;
		}
	} else {
		if (zend_hash_index_find(hash, lval, (void **) &ppzval) == FAILURE) {
			goto GET_RETURN;
		}
	}

	if (Z_TYPE_PP(ppzval) == IS_ARRAY) {
		if ((ret = wcx_data_format(getThis(), ppzval TSRMLS_CC))) {
			RETURN_ZVAL(ret, 1, 1);
		} else {
			goto GET_RETURN;
		}
	} else {
		RETURN_ZVAL(*ppzval, 1, 0);
	}

GET_RETURN: {
	if (1 == num_args || Z_TYPE_P(default_value) == IS_NULL) {
		RETURN_NULL();
	} else {
		if (Z_TYPE_P(default_value) == IS_ARRAY
				&& (ret = wcx_data_format(getThis(), &default_value TSRMLS_CC))) {
			RETURN_ZVAL(ret, 1, 1);
		}

		RETURN_ZVAL(default_value, 1, 0);
	}
}
}
/* }}} */

/** {{{ proto public WcxData::to_array(void)
*/
PHP_METHOD(wcx_data, to_array) {
	zval *properties = zend_read_property(wcx_data_class_entry, getThis(), ZEND_STRL(WCX_DATA_PROPERT_NAME), 1 TSRMLS_CC);
	RETURN_ZVAL(properties, 1, 0);
}
/* }}} */

/** {{{ proto public WcxData::set($name, $value)
*/
PHP_METHOD(wcx_data, set) {
	zval *readonly = zend_read_property(wcx_data_class_entry, getThis(), ZEND_STRL(WCX_DATA_PROPERT_NAME_READONLY), 1 TSRMLS_CC);

	if (!Z_BVAL_P(readonly)) {
		zval *name, *value, *props;
		if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "zz", &name, &value) == FAILURE) {
			return;
		}

		if (Z_TYPE_P(name) != IS_STRING || !Z_STRLEN_P(name)) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "Expect a string key name");
			RETURN_FALSE;
		}

		Z_ADDREF_P(value);
		props = zend_read_property(wcx_data_class_entry, getThis(), ZEND_STRL(WCX_DATA_PROPERT_NAME), 1 TSRMLS_CC);
		if (zend_hash_update(Z_ARRVAL_P(props), Z_STRVAL_P(name), Z_STRLEN_P(name) + 1, (void **)&value, sizeof(zval*), NULL) == SUCCESS) {
			RETURN_TRUE;
		} else {
			Z_DELREF_P(value);
		}
	}

	RETURN_FALSE;
}
/* }}} */

/** {{{ proto public WcxData::__isset($name)
*/
PHP_METHOD(wcx_data, __isset) {
	char *name;
	uint len;
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &name, &len) == FAILURE) {
		return;
	} else {
		zval *prop = zend_read_property(wcx_data_class_entry, getThis(), ZEND_STRL(WCX_DATA_PROPERT_NAME), 1 TSRMLS_CC);
		RETURN_BOOL(zend_hash_exists(Z_ARRVAL_P(prop), name, len + 1));
	}
}
/* }}} */

/** {{{ proto public WcxData::offsetUnset($index)
*/
PHP_METHOD(wcx_data, offsetUnset) {
	zval *readonly = zend_read_property(wcx_data_class_entry, getThis(), ZEND_STRL(WCX_DATA_PROPERT_NAME_READONLY), 1 TSRMLS_CC);

	if (!Z_BVAL_P(readonly)) {
		zval *name, *props;
		if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &name) == FAILURE) {
			return;
		}

		if (Z_TYPE_P(name) != IS_STRING || !Z_STRLEN_P(name)) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "Expect a string key name");
			RETURN_FALSE;
		}

		props = zend_read_property(wcx_data_class_entry, getThis(), ZEND_STRL(WCX_DATA_PROPERT_NAME), 1 TSRMLS_CC);
		if (zend_hash_del(Z_ARRVAL_P(props), Z_STRVAL_P(name), Z_STRLEN_P(name) + 1) == SUCCESS) {
			RETURN_TRUE;
		}
	}

	RETURN_FALSE;
}
/* }}} */

/** {{{ proto public WcxData::count($name)
*/
PHP_METHOD(wcx_data, count) {
	zval *prop = zend_read_property(wcx_data_class_entry, getThis(), ZEND_STRL(WCX_DATA_PROPERT_NAME), 1 TSRMLS_CC);
	RETURN_LONG(zend_hash_num_elements(Z_ARRVAL_P(prop)));
}
/* }}} */

/** {{{ proto public WcxData::rewind(void)
*/
PHP_METHOD(wcx_data, rewind) {
	zval *prop = zend_read_property(wcx_data_class_entry, getThis(), ZEND_STRL(WCX_DATA_PROPERT_NAME), 1 TSRMLS_CC);
	zend_hash_internal_pointer_reset(Z_ARRVAL_P(prop));
}
/* }}} */

/** {{{ proto public WcxData::current(void)
*/
PHP_METHOD(wcx_data, current) {
	zval *prop, **ppzval, *ret;

	prop = zend_read_property(wcx_data_class_entry, getThis(), ZEND_STRL(WCX_DATA_PROPERT_NAME), 1 TSRMLS_CC);
	if (zend_hash_get_current_data(Z_ARRVAL_P(prop), (void **)&ppzval) == FAILURE) {
		RETURN_FALSE;
	}

	if (Z_TYPE_PP(ppzval) == IS_ARRAY) {
		if ((ret = wcx_data_format(getThis(), ppzval TSRMLS_CC))) {
			RETURN_ZVAL(ret, 1, 1);
		} else {
			RETURN_NULL();
		}
	} else {
		RETURN_ZVAL(*ppzval, 1, 0);
	}
}
/* }}} */

/** {{{ proto public WcxData::key(void)
*/
PHP_METHOD(wcx_data, key) {
	zval *prop;
	char *string;
	ulong index;

	prop = zend_read_property(wcx_data_class_entry, getThis(), ZEND_STRL(WCX_DATA_PROPERT_NAME), 1 TSRMLS_CC);
	zend_hash_get_current_key(Z_ARRVAL_P(prop), &string, &index, 0);
	switch(zend_hash_get_current_key_type(Z_ARRVAL_P(prop))) {
		case HASH_KEY_IS_LONG:
			RETURN_LONG(index);
			break;
		case HASH_KEY_IS_STRING:
			RETURN_STRING(string, 1);
			break;
		default:
			RETURN_FALSE;
	}
}
/* }}} */

/** {{{ proto public WcxData::next(void)
*/
PHP_METHOD(wcx_data, next) {
	zval *prop = zend_read_property(wcx_data_class_entry, getThis(), ZEND_STRL(WCX_DATA_PROPERT_NAME), 1 TSRMLS_CC);
	zend_hash_move_forward(Z_ARRVAL_P(prop));
	RETURN_TRUE;
}
/* }}} */

/** {{{ proto public WcxData::valid(void)
*/
PHP_METHOD(wcx_data, valid) {
	zval *prop = zend_read_property(wcx_data_class_entry, getThis(), ZEND_STRL(WCX_DATA_PROPERT_NAME), 1 TSRMLS_CC);
	RETURN_BOOL(zend_hash_has_more_elements(Z_ARRVAL_P(prop)) == SUCCESS);
}
/* }}} */

/** {{{ proto public WcxData::readonly(void)
*/
PHP_METHOD(wcx_data, readonly) {
	zval *readonly = zend_read_property(wcx_data_class_entry, getThis(), ZEND_STRL(WCX_DATA_PROPERT_NAME_READONLY), 1 TSRMLS_CC);
	RETURN_BOOL(Z_LVAL_P(readonly));
}
/* }}} */

/** {{{ proto public WcxData::__destruct
*/
PHP_METHOD(wcx_data, __destruct) {
}
/* }}} */

/** {{{ proto private WcxData::__clone
*/
PHP_METHOD(wcx_data, __clone) {
}
/* }}} */

/** {{{ wcx_data_methods
*/
zend_function_entry wcx_data_methods[] = {
	PHP_ME(wcx_data, __construct, NULL, ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
	PHP_ME(wcx_data, __destruct, NULL, ZEND_ACC_PUBLIC|ZEND_ACC_DTOR)
	PHP_ME(wcx_data, __isset, wcx_data_isset_arginfo, ZEND_ACC_PUBLIC)
	PHP_ME(wcx_data, get, wcx_data_get_arginfo, ZEND_ACC_PUBLIC)
	PHP_ME(wcx_data, set, wcx_data_set_arginfo, ZEND_ACC_PUBLIC)
	PHP_ME(wcx_data, count, wcx_data_void_arginfo, ZEND_ACC_PUBLIC)
	PHP_ME(wcx_data, offsetUnset,	wcx_data_unset_arginfo, ZEND_ACC_PUBLIC)
	PHP_ME(wcx_data, rewind, wcx_data_void_arginfo, ZEND_ACC_PUBLIC)
	PHP_ME(wcx_data, current, wcx_data_void_arginfo, ZEND_ACC_PUBLIC)
	PHP_ME(wcx_data, next,	wcx_data_void_arginfo, ZEND_ACC_PUBLIC)
	PHP_ME(wcx_data, valid, wcx_data_void_arginfo, ZEND_ACC_PUBLIC)
	PHP_ME(wcx_data, key, wcx_data_void_arginfo, ZEND_ACC_PUBLIC)
	PHP_ME(wcx_data, readonly,	wcx_data_void_arginfo, ZEND_ACC_PUBLIC)
	PHP_ME(wcx_data, to_array, wcx_data_void_arginfo, ZEND_ACC_PUBLIC)
	PHP_MALIAS(wcx_data, __set, set, wcx_data_set_arginfo, ZEND_ACC_PUBLIC)
	PHP_MALIAS(wcx_data, __get, get, wcx_data_get_arginfo, ZEND_ACC_PUBLIC)
	PHP_MALIAS(wcx_data, offsetGet, get, wcx_data_rget_arginfo, ZEND_ACC_PUBLIC)
	PHP_MALIAS(wcx_data, offsetExists, __isset, wcx_data_isset_arginfo, ZEND_ACC_PUBLIC)
	PHP_MALIAS(wcx_data, offsetSet, set, wcx_data_set_arginfo, ZEND_ACC_PUBLIC)
	{NULL, NULL, NULL}
};
/* }}} */

void register_wcx_data_class(TSRMLS_D) {
	zend_class_entry ce;

	INIT_CLASS_ENTRY(ce, "WcxData", wcx_data_methods);
	wcx_data_class_entry = zend_register_internal_class(&ce TSRMLS_CC);

#ifdef HAVE_SPL
	zend_class_implements(wcx_data_class_entry TSRMLS_CC, 3, zend_ce_iterator, zend_ce_arrayaccess, spl_ce_Countable);
#else
	zend_class_implements(wcx_data_class_entry TSRMLS_CC, 2, zend_ce_iterator, zend_ce_arrayaccess);
#endif

	wcx_data_class_entry->ce_flags |= ZEND_ACC_FINAL_CLASS;

	zend_declare_property_null(wcx_data_class_entry, ZEND_STRL(WCX_DATA_PROPERT_NAME), ZEND_ACC_PROTECTED TSRMLS_CC);
	zend_declare_property_bool(wcx_data_class_entry, ZEND_STRL(WCX_DATA_PROPERT_NAME_READONLY), 0, ZEND_ACC_PROTECTED TSRMLS_CC);
}
