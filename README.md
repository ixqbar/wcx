
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

###加密/解密(内部实现为AES,php版本可参考example文件内代码)
```
string wcx_encrypt(to_encrypt_string, to_encrypt_key)
array  wcx_decrypt(to_decrypt_string, to_encrtpt_key)
```

###数组随机(同时返回被随机到的key,value)
```
array  wcx_array_rand(to_rand_array,num)
```

###true or false
```
bool   wcx_bet(rate_num)
```

###加锁解锁，依赖wcx.task_enabled＝1
```
void   wcx_lock()
void   wcx_unlock()
```

###task，依赖wcx.task_enabled＝1
```
array  wcx_task_info()
string wcx_task_post($task, $expect_task_process_timestamp = 0, $task_uuid = '')
bool   wcx_task_delete($task_uuid)
bool   wcx_task_clear()
```

###WcxTask
```
$wcx_task_handle = new WcxTask();
//v0.2.5
$wcx_task_handle->interval = 3;
//v0.2.3
//$wcx_task_handle->process(function($task_data){
//@since v0.2.4
$wcx_task_handle->process(function($task_uuid, $task_data){
    //task process
    //使用wcx_task_post可投递$task到任务队列中，系统每间隔wcx.task_process_interval秒扫描一次并检查队列中task的执行时间点($expect_task_process_timestamp,默认是立刻被执行)是否小于当前时间，小于则触发process，否则等待下一轮检测
});
$wcx_task_handle->run();
```

###wcx_ini(参照php-yaf中Yaf_Config_Ini代码实现)
```
array wcx_ini(ini_file_path, section_name = '')
```

###WcxData(参照php-yaf中Yaf_Config_Abstract代码实现)

```
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

example
```
//@since v0.2.5
$data = array('name' => array('foo', 'bar'));
$wcx_data_handle = new WcxData($data);
print_r($wcx_data_handle->to_array());
print_r($wcx_data_handle->get('name')->to_array());
echo $wcx_data_handle->get('name')->get(0);
var_dump($wcx_data_handle->get('none'));
var_dump($wcx_data_handle->get('none', [1,2,3]));
var_dump($wcx_data_handle->get('none', [1,2,3])->to_array());
```

更多疑问请+qq群 233415606 or [website http://xingqiba.sinaapp.com](http://xingqiba.sinaapp.com)



