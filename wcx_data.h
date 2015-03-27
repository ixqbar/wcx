/*
 * wcx_data.h
 *
 */

#ifndef WCX_DATA_H_
#define WCX_DATA_H_

#define	WCX_DATA_PROPERT_NAME "_config"
#define WCX_DATA_PROPERT_NAME_READONLY "_readonly"

extern zend_class_entry *wcx_data_class_entry;
void register_wcx_data_class(TSRMLS_D);

#endif /* WCX_DATA_H_ */
