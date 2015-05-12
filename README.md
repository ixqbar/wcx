
[more features](https://github.com/jonnywang/wcx/tree/dev)

##ini配置
```
wcx.debug = 0                           是否打开调试，默认关闭
wcx.task_enabled = 0                    是否开启wcx_task_*系列函数及WcxTask类,默认关闭
wcx.task_queue_key = wcx_task_queue     wcx_task_*系列函数使用了消息队列和共享内存，其用于指定消息队列 可无需更改
wcx.task_data_key = wcx_task_data       wcx_task_*系列函数使用了消息队列和共享内存，其用于指定消息队列 可无需更改
wcx.task_process_interval = 1           task扫描间隔，单位秒  //@since v0.2.5 deprecated
```

##functions

###wcx_encrypt wcx_decrypt
```php
string wcx_encrypt($to_encrypt_string, $to_encrypt_key) //加密
array wcx_decrypt($to_decrypt_string, $to_encrtpt_key) //解密
```
* 内部实现为AES,php版本可参考example文件内代码

###wcx_array_rand
```php
array wcx_array_rand($to_rand_array,$num)  //数组随机(同时返回被随机到的key,value)
```

###wcx_bet
```php
bool wcx_bet($rate_num)
```

###wcx_lock wcx_unlock
```php
bool wcx_lock()
bool wcx_unlock()
```
* 依赖wcx.task_enabled＝1

###wcx_task_info wcx_task_post wcx_task_delete wcx_task_clear
```php
array  wcx_task_info()
string wcx_task_post($task, $expect_task_process_timestamp = 0, $task_uuid = '')
bool   wcx_task_delete($task_uuid)
bool   wcx_task_clear()
```
* 依赖wcx.task_enabled＝1

###WcxTask
```php
$wcx_task_handle = new WcxTask();
$wcx_task_handle->interval = 3;
$wcx_task_handle->process(function($task_uuid, $task_data){
    //系统每间隔wcx.task_process_interval秒扫描一次并检查
    //队列中task的执行时间点($expect_task_process_timestamp,
    //默认是立刻被执行)是否小于当前时间，小于则触发process，否则等待下一轮检测
});
$wcx_task_handle->run();
```

###wcx_ini
```php
array wcx_ini($ini_file_path, $section_name = '')
```
* 参照php-yaf中Yaf_Config_Ini代码实现

###WcxData
```php
class WcxData implements Iterator, ArrayAccess, Countable {
	protected array _config;
	protected array _readonly;
		
	public __construct($data, $readonly=true);
	public mixed get($name, $default_value);
	public mixed __get($name);
	public mixed __isset($name);
	public mixed __set($name, $value);
	public mixed set($name, $value);
	public mixed count();
	public mixed offsetGet($name);
	public mixed offsetSet($name, $value);
	public mixed offsetExists($name);
	public mixed offsetUnset($name);
	public void rewind();
	public mixed key();
	public mixed next();
	public mixed current();
	public boolean valid();
	public array to_array();
	public boolean readonly()
}
```
* 参照php-yaf中Yaf_Config_Abstract代码实现, get增加默认参数

###WcxData
```php
$data = array('name' => array('foo', 'bar'));
$wcx_data_handle = new WcxData($data);
print_r($wcx_data_handle->to_array());
print_r($wcx_data_handle->get('name')->to_array());
echo $wcx_data_handle->get('name')->get(0);
var_dump($wcx_data_handle->get('none'));
var_dump($wcx_data_handle->get('none', [1,2,3]));
var_dump($wcx_data_handle->get('none', [1,2,3])->to_array());
```

###wcx_str_rand
```php
string wcx_str_rand($to_rand_len, $not_rand_number = false)
string wcx_str_rand($to_rand_len, $to_rand_chars)
```

###wcx_array_remove
```php
int wcx_array_remove(&$to_remove_arr, $to_remove_arr_element[, $to_remove_arr_element_num|$to_remove_arr_element_by_strict_compare])
```
* wcx_array_remove($t, 3) 不区分类型删除所有3
* wcx_array_remove($t, 3, 1) 不区分类型删除3，删除总量为1
* wcx_array_remove($t, 3, true) 区分类型删除所有3

###wcx_geo_distance
```php
wcx_geo_distance($lat1d, $lon1d, $lat2d, $lon2d)
```

更多疑问请+qq群 233415606 or [website http://xingqiba.sinaapp.com](http://xingqiba.sinaapp.com)



