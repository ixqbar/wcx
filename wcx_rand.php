<?php

print_r(wcx_array_rand(array('a' => 1, 'b' => 2, 'c' => 3, 'd' => 4), 2));

print_r(wcx_array_rand(array('a' => 1, 'b' => 2, 'c' => 3, 'd' => 4)));

print_r(wcx_array_rand(array(1, 'b' => 2, 'c' => 3, 'd' => 4)));

print_r(wcx_array_rand(array(1, 'b' => 2, 'c' => array(1, 2, array(1,2,3)), 'd' => 4), 2));