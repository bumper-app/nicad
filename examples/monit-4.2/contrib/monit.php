<?php
//Script to forward a request on a web server to the Monit webserver.
//Allows the Monit information screen to be viewed remotely, without 
//leaving an additional port open on the server.
//
//Place this script on your webserver.
//Use a password protected area if you like.
//Set the configuration information below to match your monit installation. 
//With this script you are accessing monit from the localhost, so you don't
//need to allow external access through your firewall.
//
//Requires PHP, and a working webserver!
//Copyright 2003 GNU General Public License Version 2
//Author David Fletcher, <david@megapico.co.uk>

//Configuration - set this to match the information in /etc/monitrc
//Set information about the monit server address, port and login details.
$addr = 'localhost';
$port = 2812;
$path = '/';
$user = 'admin';
$pass = 'monit';
$timeout = 30;
//End of configuration

$urlHandle = fsockopen($addr, $port, $errno, $errstr, $timeout);
socket_set_timeout($urlHandle, $timeout);

$urlString = "GET $path HTTP/1.0\r\nHost: $addr\r\nConnection: Keep-Alive\r\nUser-Agent: MonitPHP\r\n";
$urlString .= "Authorization: Basic ".base64_encode("$user:$pass")."\r\n";
$urlString .= "\r\n";
      
fputs($urlHandle, $urlString);
$response = fgets($urlHandle);

$endHeader = false;                           // Strip initial header information
while ( !$endHeader){
  if (fgets($urlHandle) == "\r\n")
    $endHeader = true;
}

$info = '';                                  // Generate a string to send to the screen

while (!feof($urlHandle)) {
  $info.=fgets($urlHandle);
}

fclose ($urlHandle);

echo $info;

?>
