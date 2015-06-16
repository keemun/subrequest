<?php
$ret = subreq_set_domain('127.0.0.1', 9000);
var_dump($ret);
$ret = subreq_set_param(array(
"REQUEST_METHOD"=>"GET",
"SCRIPT_FILENAME"=>"/usr/share/nginx/html/index.php",
"QUERY_STRING"=>"action=test"
), "");
var_dump($ret);
$ret = subreq_set_param(array(
"REQUEST_METHOD"=>"GET",
"SCRIPT_FILENAME"=>"/usr/share/nginx/html/index.php",
"QUERY_STRING"=>"action=test2"
), "");
var_dump($ret);
$ret = subrequest();
var_dump($ret);
$ret = subrequest();
var_dump($ret);
$ret = subrequest();
var_dump($ret);
