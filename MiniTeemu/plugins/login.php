<?php
/**
 * login.php - L�hett�� login komennon
 */
if( $init==true ) {
    $plugin->addRule('code', "451");
    $plugin->addRule('break', true);
    return;
}

$plugin->irc->trace("451; Rekister�idy ensin.");
$plugin->irc->login();
?>