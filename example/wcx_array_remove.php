<?php

$a = 'hello';
$t = ["345", "34", 34, 'abc' => 34, $a => 34, [1, 3, 2, "3"]];
var_dump($t);

echo "to remove number 34" . PHP_EOL;
var_dump(wcx_array_remove($t, 34, true));
var_dump($t);

echo "to remove all 34" . PHP_EOL;
var_dump(wcx_array_remove($t, 34));
var_dump($t);

echo "to remove child string 3" . PHP_EOL;
var_dump(wcx_array_remove($t[3], "3", true));
var_dump($t);


var_dump($a);
