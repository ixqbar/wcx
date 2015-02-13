<?php
/**
 *
 */

$wcx_task_handle = new WcxTask();
$wcx_task_handle->process(function($task){
   print_r($task);
});

$wcx_task_handle->run();
