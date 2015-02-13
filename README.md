
more detail http://xingqiba.sinaapp.com

##functions
```
string wcx_encrypt(to_encrypt_string, to_encrypt_key)
array  wcx_decrypt(to_decrypt_string, to_encrtpt_key)
array  wcx_array_rand(to_rand_array,num)
bool   wcx_bet(rate_num)
```

##dev plan

task
```
wcx_task_info()
wcx_task_post($task, $expect_task_process_timestamp = 0, $task_uuid = '')
wcx_task_delete($task_uuid)
wcx_task_clear()
```

task process
```
$wcx_task_handle = new WcxTask();
$wcx_task_handle->process(function($task){
    //task process
});;
$wcx_task_handle->run();
```
