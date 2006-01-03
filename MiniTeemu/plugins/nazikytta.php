<?php
/**
 * nazikytta.php -- tarkastaa onko k�ytt�j� away nickin vaihdon yhteydess�.
 */
if( $init==true ) {
    $plugin->addRule('code', "NICK");
    return;
}
$exp = time()+10;
$toban = substr($plugin->line->from, strpos($plugin->line->from, "~")+1);
// Luodaan uusi plugin.
// TODO: vitun ��li�m�ist�. keksi jotain fiksumpaa

$code = '
if( $init==true ) {
    $plugin->addRule("code", "301");
    $plugin->addRule("nick", "'.$plugin->line->msg.'");
    $plugin->addRule("expire", "'.$exp.'");
    return;
}

//print_r($plugin->line);

irc::trace("Away scripti havaittu '.$plugin->line->msg.'");
//$plugin->irc->send("MODE '.$plugin->line->channel.' +b *!*'.$toban.'");

$plugin->irc->send("KICK '.$plugin->line->channel.' '.$plugin->line->msg.' :http://rautakuu.org/drupal/RautakuuIrc #4");

$plugin->expire();
';

// Rekister�id��n plugini uutena
$plugin->irc->irc_data->triggers->newPlugin($code);

// L�hetet��n whois kysely
$plugin->irc->send("WHOIS {$plugin->line->msg}");

?>
