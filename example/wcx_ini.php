<?php

$ini_config = wcx_ini('/Users/venkman/data/soft/wcx/example/config.ini', 'dev');
print_r($ini_config);
print_r($ini_config['database']['master']);
