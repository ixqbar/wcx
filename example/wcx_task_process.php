<?php
/**
 *
 */

$wcx_task_handle = new WcxTask();
$wcx_task_handle->process(function($task){
   var_dump($task);
});

$wcx_task_handle->run();
