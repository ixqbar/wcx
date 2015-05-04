<?php

var_dump(wcx_str_rand(0));
var_dump(wcx_str_rand(5));
var_dump(wcx_str_rand(26));
var_dump(wcx_str_rand(27));

echo PHP_EOL;

var_dump(wcx_str_rand(5, true));
var_dump(wcx_str_rand(26, true));
var_dump(wcx_str_rand(30, true));

echo PHP_EOL;

var_dump(wcx_str_rand(10, "ok12"));
var_dump(wcx_str_rand(30, "ok12"));