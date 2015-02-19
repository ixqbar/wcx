<?php

$result = wcx_task_post(array(1,2,3));
var_dump($result);

$result = wcx_task_post(123456, time(), "1234");
var_dump($result);

$result = wcx_task_post('a789', time());
var_dump($result);

$to_delete_uuid = '';
$i = 1;
while ($i < 10) {
	$result = wcx_task_post("abd" . $i, time() + 10);
	var_dump($result);
	var_dump(wcx_task_info());
	if ($i == 3) {
	    $to_delete_uuid = $result;
	}
	$i++;
}

var_dump(wcx_task_delete($to_delete_uuid));
