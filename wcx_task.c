/*
 * wcx_task.c
 *
 * @author ixqbar@gmail.com
 *
 * Example:
 *
 * $wcx_task_handle = new WcxTask();
 * $wcx_task_handle->process(function($task){});;
 * $wcx_task_handle->run();
 *
 */
//wcx_task class

#include "php.h"

typedef struct {
	zend_object zo;
	void *cls_obj;
} wcx_task_object;

PHP_METHOD(wcx_task, __construct);
PHP_METHOD(wcx_task, process);
PHP_METHOD(wcx_task, run);

static zend_class_entry *wcx_task_class_entry;
zend_object_handlers wcx_task_object_handlers;

static zend_function_entry wcx_task_methods[] = {
	PHP_ME(wcx_task, __construct, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
	PHP_ME(wcx_task, process,     NULL, ZEND_ACC_PUBLIC)
	PHP_ME(wcx_task, run,         NULL, ZEND_ACC_PUBLIC)
	{NULL, NULL, NULL}
};

static void wcx_task_free_storage(void *object TSRMLS_DC) {
	wcx_task_object *obj = (wcx_task_object *)object;
	if (obj->zo.properties) {
		zend_hash_destroy(obj->zo.properties);
		FREE_HASHTABLE(obj->zo.properties);
	}
	zend_object_std_dtor(object TSRMLS_CC);
	efree(object);
}

/**
 * @link https://wiki.php.net/internals/engine/objects
 */
static zend_object_value wcx_task_object_create(zend_class_entry *class_type TSRMLS_DC) {
	zend_object_value retval;
	wcx_task_object *obj;

	obj = emalloc(sizeof(wcx_task_object));
	memset(obj, 0, sizeof(wcx_task_object));
	obj->zo.ce = class_type;

	zend_object_std_init((zend_object *) obj, class_type TSRMLS_CC);
#if PHP_VERSION_ID < 50399
    zend_hash_copy(obj->zo.properties, &(class_type->default_properties), (copy_ctor_func_t) zval_add_ref, NULL, sizeof(zval*));
#else
    object_properties_init((zend_object*)obj, class_type);
#endif

	retval.handle   = zend_objects_store_put(obj,
			(zend_objects_store_dtor_t) zend_objects_destroy_object,
			(zend_objects_free_object_storage_t) wcx_task_free_storage,
			NULL TSRMLS_CC);
	retval.handlers = (zend_object_handlers *) &wcx_task_object_handlers;

	return retval;
}

PHP_METHOD(wcx_task, __construct) {
	zval *this;
	this = getThis();

	wcx_task_object *wcx_task_obj = (wcx_task_object *)zend_object_store_get_object(this TSRMLS_CC);
	wcx_task_obj->cls_obj = (void *)this;

	RETURN_TRUE;
}

PHP_METHOD(wcx_task, process) {
	zval *callback;
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &callback) == FAILURE) {
		RETURN_FALSE;
	}

	zend_update_property(wcx_task_class_entry, getThis(), "process", sizeof("process") - 1, callback TSRMLS_CC);

	RETURN_TRUE;
}

PHP_METHOD(wcx_task, run) {
	zval *closure = zend_read_property(wcx_task_class_entry, getThis(), "process", sizeof("process") - 1, 0 TSRMLS_CC);
	if (!closure) {
		php_error_docref(NULL TSRMLS_CC, E_ERROR, "Please define your callback function");
	}

	char *func_name = NULL;
	if (0 == zend_is_callable(closure, 0, &func_name TSRMLS_CC)) {
		php_error_docref(NULL TSRMLS_CC, E_ERROR, "Callback function '%s' is not callable", func_name);
		efree(func_name);
		RETURN_FALSE;
	}
	efree(func_name);

	zval *retval;
	zval **params[1];

	//juest for test
	zval *arr;
	MAKE_STD_ZVAL(arr);
	array_init(arr);
	add_assoc_string(arr, "result", "success", 1);
	params[0] = &arr;

	if (call_user_function_ex(CG(function_table), NULL, closure, &retval, 1, params, 0, NULL TSRMLS_CC) == SUCCESS ) {
		if (retval) {
			zval_ptr_dtor(&retval);
		}
	}

	zval_ptr_dtor(&arr);

	RETURN_TRUE;
}

void register_wcx_task_class(TSRMLS_D) {
	zend_class_entry ce;
	INIT_CLASS_ENTRY(ce, "WcxTask", wcx_task_methods);
	ce.create_object = wcx_task_object_create;
	memcpy(&wcx_task_object_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
	wcx_task_object_handlers.clone_obj = NULL;
	wcx_task_class_entry = zend_register_internal_class(&ce TSRMLS_CC);
	zend_declare_property_null(wcx_task_class_entry, "process", sizeof("process") - 1, ZEND_ACC_PUBLIC TSRMLS_CC);
}
