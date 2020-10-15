<?php
//$templ = file_get_contents('http://192.168.200.123:8247/');
//echo $templ;
$handle = fopen('http://192.168.200.123:8247','r');
stream_set_timeout($handle, 2);
if($handle==FALSE) {
 $handle=fopen('./fail1.html','r');
    }
$contents = stream_get_contents($handle);
fclose($handle);

$str=$contents;
$from="Teplota venkovni sever:";
$to="&deg;C";
$contents=getStringBetween($str,$from,$to);

function getStringBetween($str,$from,$to)
{
    $sub = substr($str, strpos($str,$from)+strlen($from),strlen($str));
    return substr($sub,0,strpos($sub,$to));
}
if($contents!=FALSE && $contents!=""){
    $contents=trim($contents);
    $contents=floatval($contents);
    //echo $contents;  //in contents is now float value - outside temperature - value in C grad

    $handle2 = fopen('http://butbn.tmep.cz/?skleniky_venku='.$contents,'r');
    $response=stream_get_contents($handle2);
    fclose($handle2);

    echo $response;
} //end of if
exit
?>
