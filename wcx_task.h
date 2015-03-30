/*
 * wcx_task.h
 *
 *  Created on: Feb 15, 2015
 *      Author: venkman
 */

#ifndef WCX_TASK_H_
#define WCX_TASK_H_

#include "wcx_list.h"
#include <semaphore.h>

#define WCX_TASK_MESSAGE_MAX_LEN       2048
#define WCX_TASK_MESSAGE_OPT_IS_ADD    1
#define WCX_TASK_MESSAGE_OPT_IS_DELETE 2
#define WCX_TASK_PROPERT_PROCESS_NAME  "_process"
#define WCX_TASK_PROPERT_INTERVAL_NAME "_interval"
#define WCX_TASK_DEBUG_LOG(fmt, args...) if (1 == INI_INT("wcx.debug")) { php_printf(fmt, ##args); }
#define WCX_TASK_PTR() (wcx_task_ptr *)WCX_G(wcx_task_ptr)
#define WCX_TASK_LOCK() { \
	wcx_task_ptr *tpr = (wcx_task_ptr *)WCX_G(wcx_task_ptr);\
	if (tpr) { \
		sem_wait(&tpr->info->slock); \
	} \
}
#define WCX_TASK_UNLOCK() { \
	wcx_task_ptr *tpr = (wcx_task_ptr *)WCX_G(wcx_task_ptr);\
	if (tpr) { \
		sem_post(&tpr->info->slock); \
	} \
}
#define WCX_LOCK() { \
	wcx_task_ptr *tpr = (wcx_task_ptr *)WCX_G(wcx_task_ptr);\
	if (tpr) { \
		sem_wait(&tpr->info->ulock); \
	} \
}
#define WCX_UNLOCK() { \
	wcx_task_ptr *tpr = (wcx_task_ptr *)WCX_G(wcx_task_ptr);\
	if (tpr) { \
		sem_post(&tpr->info->ulock); \
	} \
}

typedef struct {
	zend_object zo;
	void *cls_obj;
} wcx_task_object;

static zend_class_entry *wcx_task_class_entry;
static zend_object_handlers wcx_task_object_handlers;

typedef struct _wcx_task_message {
	long mtype;
	long mtime;
	long mopt;
	int muuid_len;
	int mtask_len;
	char mtext[];
} wcx_task_message;

typedef struct _wcx_task_info {
	int init;
	int running;
	long qnum;
	sem_t slock;
	sem_t ulock;
} wcx_task_info;

typedef struct _wcx_task_ptr {
	int mid;
	wcx_task_info *info;
	list *task;
} wcx_task_ptr;

void register_wcx_task_class(TSRMLS_D);
wcx_task_ptr * wcx_task_init();
void wcx_task_release(wcx_task_ptr *);

PHP_METHOD(wcx_task, __construct);
PHP_METHOD(wcx_task, interval);
PHP_METHOD(wcx_task, process);
PHP_METHOD(wcx_task, run);

#endif /* WCX_TASK_H_ */
