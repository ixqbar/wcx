<?php
/**
 *
 */

$wcx_task_handle = new WcxTask();
$wcx_task_handle->process(function($task_uuid, $task_data){
   var_dump($task_uuid);
   var_dump($task_data);
});

$wcx_task_handle->run();
