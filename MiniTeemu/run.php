#!/usr/bin/php -q
<?php

require_once("miniteemu.php");

$irc =& new irc();
$GLOBALS['irc'] =& $irc;

irc::trace("Yhdistet��n...");
$irc->connect();
irc::trace("Kuunnellaan...");
$irc->listen();



?>