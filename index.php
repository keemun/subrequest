<?php
echo time(),$_GET['action'],"\n";
if($_GET['action'] == 'test')
{
	sleep(2);
	echo 'aaaaaaaaaaaaaaaa';
}
elseif($_GET['action'] == 'test2')
{
	sleep(5);
	echo 'bbbbbbbbbbbbbbbb';
}
else
{
	sleep(8);
	echo 'cccccccccccccccc';
}
echo time(),$_GET['action'],"\n";
