<?php

$ini_config = wcx_ini('./config.ini', 'dev');
print_r($ini_config);
print_r($ini_config['database']['master']);
