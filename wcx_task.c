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

#include <stdio.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <signal.h>

#include "php.h"
#include "php_ini.h"
#include "php_globals.h"
#include "ext/standard/info.h"
#include "ext/standard/php_var.h"
#include "ext/standard/php_smart_str.h"

#include "wcx_task.h"
#include "php_wcx.h"

int task_process_running = 1;

void task_process_halt(int signal) {
	WCX_TASK_DEBUG_LOG("catch signal %d\n", signal);
	task_process_running = 0;
}

void wcx_task_execute(wcx_task_message *wtm, zval *closure) {
	zval *tmp = NULL;
	zval *muuid = NULL;

	MAKE_STD_ZVAL(tmp);
	MAKE_STD_ZVAL(muuid);
	ZVAL_STRINGL(muuid, wtm->mtext + wtm->mtask_len, wtm->muuid_len, 1);

	const unsigned char *p = (const unsigned char *) wtm->mtext;

	php_unserialize_data_t var_hash;
	PHP_VAR_UNSERIALIZE_INIT(var_hash);
	if (php_var_unserialize(&tmp,
			&p,
			p + wtm->mtask_len,
			&var_hash TSRMLS_CC)) {
		zval **params[2];
		params[0] = &muuid;
		params[1] = &tmp;
		zval *retval = NULL;
		if (SUCCESS == call_user_function_ex(CG(function_table), NULL, closure, &retval, 2, params, 0, NULL TSRMLS_CC)) {
			if (NULL != retval) {
				zval_ptr_dtor(&retval);
			}
		}
	} else {
		WCX_TASK_DEBUG_LOG("unserialize failed\n");
	}

	zval_dtor(tmp);
	FREE_ZVAL(tmp);

	zval_dtor(muuid);
	FREE_ZVAL(muuid);

	PHP_VAR_UNSERIALIZE_DESTROY(var_hash);
}

const zend_function_entry wcx_task_methods[] = {
	PHP_ME(wcx_task, __construct, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
	PHP_ME(wcx_task, process,     NULL, ZEND_ACC_PUBLIC)
	PHP_ME(wcx_task, run,         NULL, ZEND_ACC_PUBLIC)
	{NULL, NULL, NULL}
};

void wcx_task_free_storage(void *object TSRMLS_DC) {
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
zend_object_value wcx_task_object_create(zend_class_entry *class_type TSRMLS_DC) {
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

void register_wcx_task_class(TSRMLS_D) {
	zend_class_entry ce;
	INIT_CLASS_ENTRY(ce, "WcxTask", wcx_task_methods);
	ce.create_object = wcx_task_object_create;
	memcpy(&wcx_task_object_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
	wcx_task_object_handlers.clone_obj = NULL;
	wcx_task_class_entry = zend_register_internal_class(&ce TSRMLS_CC);

	zend_declare_property_null(wcx_task_class_entry, "process", sizeof("process") - 1, ZEND_ACC_PUBLIC TSRMLS_CC);
	zend_declare_property_long(wcx_task_class_entry, "debug", sizeof("debug") - 1, 0, ZEND_ACC_PUBLIC TSRMLS_CC);
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
	wcx_task_ptr *tpr = WCX_TASK_PTR();
	if (tpr == NULL || tpr->mid < 0) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "task init failed");
		RETURN_FALSE;
	}

	zval *closure = zend_read_property(wcx_task_class_entry, getThis(), "process", sizeof("process") - 1, 0 TSRMLS_CC);
	if (!closure) {
		php_error_docref(NULL TSRMLS_CC, E_ERROR, "please define your callback function");
		RETURN_FALSE;
	}

	char *func_name = NULL;
	if (0 == zend_is_callable(closure, 0, &func_name TSRMLS_CC)) {
		php_error_docref(NULL TSRMLS_CC, E_ERROR, "callback function '%s' is not callable", func_name);
		efree(func_name);
		RETURN_FALSE;
	}
	efree(func_name);

	struct sigaction act;
	act.sa_handler = task_process_halt;
	sigemptyset(&act.sa_mask);
	act.sa_flags = SA_RESETHAND;
	sigaction(SIGINT, &act, 0);

	long timestamp;
	int result = 0;
	wcx_task_message *queue_message = (wcx_task_message *)emalloc(sizeof(wcx_task_message) + WCX_TASK_MESSAGE_MAX_LEN);
	wcx_task_message *task_node_value, *task_head_node_value, *task_tail_node_value, *task_match_node_value;
	listNode *task_list_node = NULL;

	WCX_G(wcx_task_running) = 1;
	tpr->info->running += 1;

	while (tpr->info->running > 0
			&& task_process_running > 0) {
		WCX_TASK_DEBUG_LOG("task queue to lock\n");
		WCX_TASK_LOCK();
		WCX_TASK_DEBUG_LOG("task queue to locked\n");

		timestamp = (long)time(NULL);
#ifdef MSG_EXCEPT
		result = msgrcv(tpr->mid, queue_message, WCX_TASK_MESSAGE_MAX_LEN, 1, IPC_NOWAIT | MSG_NOERROR | MSG_EXCEPT);
#else
		result = msgrcv(tpr->mid, queue_message, WCX_TASK_MESSAGE_MAX_LEN, 1, IPC_NOWAIT | MSG_NOERROR);
#endif
		WCX_TASK_DEBUG_LOG("got queue len=%d\n", result);
		if (result > 0) {
			WCX_TASK_DEBUG_LOG("got message toal_len=%d, action=%ld, time=%ld, uuid_len=%d, task_len=%d, buffer=%s\n",
					result,
					queue_message->mopt,
					queue_message->mtime,
					queue_message->muuid_len,
					queue_message->mtask_len,
					queue_message->mtext);

			switch (queue_message->mopt) {
				case WCX_TASK_MESSAGE_OPT_IS_ADD:
					if (queue_message->mtime <= timestamp) {
						WCX_TASK_DEBUG_LOG("process message to execute\n");
						wcx_task_execute(queue_message, closure);
					} else {
						WCX_TASK_DEBUG_LOG("process message to crontab\n");
						//init node value
						task_node_value = malloc(sizeof(wcx_task_message) + queue_message->muuid_len + queue_message->mtask_len + 1);
						task_node_value->mtype     = 0;
						task_node_value->mopt      = 0;
						task_node_value->mtime     = queue_message->mtime;
						task_node_value->muuid_len = queue_message->muuid_len;
						task_node_value->mtask_len = queue_message->mtask_len;
						memcpy(task_node_value->mtext, queue_message->mtext, queue_message->muuid_len + queue_message->mtask_len);
						task_node_value->mtext[queue_message->muuid_len + queue_message->mtask_len] = '\0';

						if (tpr->task->head) {
							task_head_node_value = (wcx_task_message *)tpr->task->head->value;
							task_tail_node_value = (wcx_task_message *)tpr->task->tail->value;
							if (task_node_value->mtime < task_head_node_value->mtime) {
								listAddNodeHead(tpr->task, (void *)task_node_value);
							} else if (task_node_value->mtime >= task_tail_node_value->mtime) {
								listAddNodeTail(tpr->task, (void *)task_node_value);
							} else {
								task_list_node = tpr->task->head;
								while (NULL != task_list_node) {
									task_match_node_value = (wcx_task_message *)task_list_node->value;
									if (task_match_node_value->mtime > task_node_value->mtime) {
										listInsertNode(tpr->task, task_list_node, (void *)task_node_value, 0);
										break;
									}
									task_list_node = task_list_node->next;
								}
							}
						} else {
							listAddNodeHead(tpr->task, (void *)task_node_value);
						}
					}
					break;
				case WCX_TASK_MESSAGE_OPT_IS_DELETE:
					task_list_node = tpr->task->head;
					while (NULL != task_list_node) {
						task_node_value = (wcx_task_message *)task_list_node->value;
						if (0 == strncmp(task_node_value->mtext + task_node_value->mtask_len, queue_message->mtext, queue_message->muuid_len)) {
							WCX_TASK_DEBUG_LOG("process crontab to delete task=%s\n", queue_message->mtext);
							listDelNode(tpr->task, task_list_node);
						}
						task_list_node = task_list_node->next;
					}
					break;
			}

			queue_message->mtime = 0;
			memset(queue_message->mtext, 0,  queue_message->muuid_len + queue_message->mtask_len);
			queue_message->muuid_len = 0;
			queue_message->mtask_len = 0;

			tpr->info->qnum += 1;
			WCX_TASK_DEBUG_LOG("shared process num=%ld\n", tpr->info->qnum);
		}

		WCX_TASK_DEBUG_LOG("check crontab\n");
		//loop
		task_list_node = tpr->task->head;
		while (NULL != task_list_node) {
			task_node_value = (wcx_task_message *)task_list_node->value;
			if (task_node_value->mtime < timestamp) {
				WCX_TASK_DEBUG_LOG("process crontab to execute uuid_len=%d, task_len=%d, buffer=%s\n",
						task_node_value->muuid_len,
						task_node_value->mtask_len,
						task_node_value->mtext);
				wcx_task_execute(task_node_value, closure);
				listDelNode(tpr->task, task_list_node);
			} else {
				break;
			}
			task_list_node = task_list_node->next;
		}
		WCX_TASK_DEBUG_LOG("check crontab end\n");

		WCX_TASK_DEBUG_LOG("task queue to unlock\n");
		WCX_TASK_UNLOCK();
		WCX_TASK_DEBUG_LOG("task queue to unlocked\n");
		sleep(INI_INT("wcx.task_process_interval"));
	}

	efree(queue_message);

	msgctl(tpr->mid, IPC_RMID, 0);

	if (tpr->task) {
		listRelease(tpr->task);
		tpr->task = NULL;
	}

	RETURN_TRUE;
}

PHP_FUNCTION(wcx_task_post) {
	wcx_task_ptr *tpr = WCX_TASK_PTR();
	if (tpr == NULL || tpr->mid < 0) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "task init failed");
		RETURN_FALSE;
	}

	zval *message;
	long message_time = 0;
	char *user_message_uuid;
	int user_message_uuid_len = 0;
	int user_args_len = ZEND_NUM_ARGS();

	if (zend_parse_parameters(user_args_len TSRMLS_CC, "z|ls", &message, &message_time, &user_message_uuid, &user_message_uuid_len) == FAILURE) {
		RETURN_FALSE;
	}

	if (user_args_len < 2) {
		message_time = (long)time(NULL);
	}

	if (user_args_len < 3 || user_message_uuid_len == 0) {
		int sec, usec;
		struct timeval tv;

		gettimeofday((struct timeval *) &tv, (struct timezone *) NULL);
		sec = (int) tv.tv_sec;
		usec = (int) (tv.tv_usec % 0x100000);

		user_message_uuid_len = spprintf(&user_message_uuid, 0, "%08x%05x", sec, usec);
	}

	//@see sysvmsg.c
	smart_str msg_var = {0};
	php_serialize_data_t var_hash;

	PHP_VAR_SERIALIZE_INIT(var_hash);
	php_var_serialize(&msg_var, &message, &var_hash TSRMLS_CC);
	PHP_VAR_SERIALIZE_DESTROY(var_hash);

	//check size
	if (msg_var.len > WCX_TASK_MESSAGE_MAX_LEN - user_message_uuid_len) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "overflow max byte size %d", WCX_TASK_MESSAGE_MAX_LEN);
		RETURN_FALSE;
	}

	RETVAL_STRINGL(user_message_uuid, user_message_uuid_len,  1);

	int message_len = sizeof (wcx_task_message) + user_message_uuid_len + msg_var.len + 1;
	wcx_task_message *queue_message = emalloc(message_len);

	//message type must be > 0
	queue_message->mtype = 1;
	//message opt
	queue_message->mopt  = WCX_TASK_MESSAGE_OPT_IS_ADD;
	//message time
	queue_message->mtime = message_time;
	//message uuid len
	queue_message->muuid_len = user_message_uuid_len;
	//message data len
	queue_message->mtask_len = msg_var.len;
	//message
	memcpy(queue_message->mtext, msg_var.c, msg_var.len);
	memcpy(queue_message->mtext + msg_var.len, user_message_uuid, user_message_uuid_len);
	queue_message->mtext[user_message_uuid_len + msg_var.len] = '\0';

	WCX_TASK_DEBUG_LOG("task post total_len=%d, uuid_len=%d, uuid=%s, task_len=%d, buffer=%s\n",
			message_len,
			user_message_uuid_len,
			user_message_uuid,
			(int)msg_var.len,
			msg_var.c);

	smart_str_free(&msg_var);

	int result = msgsnd(tpr->mid, queue_message, message_len - sizeof (long), 0);

	efree(queue_message);

	if (user_args_len < 3) {
		efree(user_message_uuid);
	}

	if (result == -1) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "task post failed: %s", strerror(errno));
		RETURN_FALSE
	}

	return;
}

PHP_FUNCTION(wcx_lock) {
	WCX_LOCK();
}

PHP_FUNCTION(wcx_unlock) {
	WCX_UNLOCK();
}

PHP_FUNCTION(wcx_task_info) {
	array_init(return_value);

	wcx_task_ptr *tpr = WCX_TASK_PTR();
	if (NULL == tpr) {
		return;
	}

	struct msqid_ds msg_stat;
	if (0 == msgctl(tpr->mid, IPC_STAT, &msg_stat)) {
		add_assoc_long(return_value, "no_process_queue_num", msg_stat.msg_qnum);
	}

	add_assoc_long(return_value, "process_queue_num", tpr->info->qnum);
	add_assoc_long(return_value, "task_crontab_num", tpr->task->len);
	add_assoc_long(return_value, "task_process_num", tpr->info->running);
}

PHP_FUNCTION(wcx_task_delete) {
	wcx_task_ptr *tpr = WCX_TASK_PTR();
	if (NULL == tpr) {
		RETURN_FALSE;
	}

	char *user_message_uuid;
	int user_message_uuid_len = 0;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &user_message_uuid, &user_message_uuid_len) == FAILURE) {
		RETURN_FALSE;
	}

	if (user_message_uuid_len <= 0) {
		RETURN_FALSE;
	}

	int message_len = sizeof (wcx_task_message) + user_message_uuid_len + 1;
	wcx_task_message *queue_message = emalloc(message_len);
	//message type must be > 0
	queue_message->mtype = 1;
	//message opt
	queue_message->mopt  = WCX_TASK_MESSAGE_OPT_IS_DELETE;
	//message time
	queue_message->mtime = (long)time(NULL);
	//message uuid len
	queue_message->muuid_len = user_message_uuid_len;
	//message task len
	queue_message->mtask_len = 0;
	//message data
	memcpy(queue_message->mtext, user_message_uuid, user_message_uuid_len);
	queue_message->mtext[user_message_uuid_len] = '\0';

	WCX_TASK_DEBUG_LOG("task delete total_len=%d, uuid_len=%d, uuid=%s, buffer=%s\n",
				message_len,
				user_message_uuid_len,
				user_message_uuid,
				queue_message->mtext);

	int result = msgsnd(tpr->mid, queue_message, message_len - sizeof (long), 0);
	efree(queue_message);
	if (result == -1) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "task delete failed: %s", strerror(errno));
		RETURN_FALSE
	}

	RETURN_TRUE;
}

PHP_FUNCTION(wcx_task_clear) {
	wcx_task_ptr *tpr = WCX_TASK_PTR();
	if (NULL == tpr) {
		RETURN_FALSE;
	}

	WCX_TASK_LOCK();
	tpr->info->running = 0;
	WCX_TASK_UNLOCK();

	RETURN_TRUE;
}

wcx_task_ptr *wcx_task_init() {
	if (0 == INI_INT("wcx.task_enabled")) {
		return NULL;
	}

	wcx_task_ptr *tpr = malloc(sizeof (wcx_task_ptr));
	if (tpr == NULL) {
		return NULL;
	}

	//queue
	tpr->mid = msgget(ftok(INI_STR("wcx.task_queue_key"), 1), 0);
	if (tpr->mid < 0)	{
		tpr->mid = msgget(ftok(INI_STR("wcx.task_queue_key"), 1), IPC_CREAT | 0666);
		if (tpr->mid < 0) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "get system v queue message failed for %s : %s", INI_STR("wcx.task_queue_key"), strerror(errno));
			WCX_TASK_DEBUG_LOG("get system v queue message failed\n");
			return NULL;
		}
	}

	struct msqid_ds msg_stat;
	if (0 == msgctl(tpr->mid, IPC_STAT, &msg_stat)) {
		msg_stat.msg_qbytes = WCX_TASK_MESSAGE_MAX_LEN * WCX_TASK_MESSAGE_MAX_LEN;
		if (0 != msgctl(tpr->mid, IPC_SET, &msg_stat)) {
			WCX_TASK_DEBUG_LOG("init system v queue message max bytes failed\n");
		}
	} else {
		WCX_TASK_DEBUG_LOG("init system v queue message max bytes failed\n");
	}

	int shmid = shmget(ftok(INI_STR("wcx.task_data_key"), 1), sizeof (wcx_task_info), IPC_CREAT | 0666);
	if (-1 == shmid) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "get shared memory data failed for %s : %s", INI_STR("wcx.task_data_key"), strerror(errno));
		WCX_TASK_DEBUG_LOG("get shared memory data failed\n");
		return NULL;
	}
	void *shared_memory = shmat(shmid, NULL, 0);
	if ((void *)-1 == shared_memory) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "get shared memory data failed for %s : %s", INI_STR("wcx.task_data_key"), strerror(errno));
		WCX_TASK_DEBUG_LOG("get shared memory data failed\n");
		return NULL;
	}

	tpr->info = (wcx_task_info *)shared_memory;
	if (tpr->info->init <= 0) {
		WCX_TASK_DEBUG_LOG("shared memory data init\n");
		tpr->info->init = 1;
		sem_init(&tpr->info->slock, 1, 1);
		sem_init(&tpr->info->ulock, 1, 1);
	} else {
		WCX_TASK_DEBUG_LOG("shared memory data already init\n");
	}

	//list
	tpr->task = listCreate();
	listSetFreeMethod(tpr->task, free);

	return tpr;
}

void wcx_task_release(wcx_task_ptr *tpr) {
	if (tpr == NULL) {
		return;
	}

	if (tpr->info) {
		if (WCX_G(wcx_task_running)) {
			WCX_TASK_LOCK();
			tpr->info->running -= 1;
			if (tpr->info->running <= 0) {
				tpr->info->running = 0;
				tpr->info->qnum    = 0;
			}
			WCX_TASK_UNLOCK();
		}
		shmdt((void *)tpr->info);
	}

	if (tpr->task) {
		listRelease(tpr->task);
		tpr->task = NULL;
	}

	free(tpr);
	tpr = NULL;
}
