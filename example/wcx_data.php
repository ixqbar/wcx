<?php
$data = array('name' => array('foo', 'bar'));
$wcx_data_handle = new WcxData($data);
print_r($wcx_data_handle->to_array());
print_r($wcx_data_handle->get('name')->to_array());
echo $wcx_data_handle->get('name')->get(0);
var_dump($wcx_data_handle->get('none'));
var_dump($wcx_data_handle->get('none', [1,2,3]));
