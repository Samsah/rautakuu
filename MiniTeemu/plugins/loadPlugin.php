<?php
/**
 * loadPlugin.php - Lataa uuden pluginin
 */
if( $init==true ) {
    $plugin->addRule('code',   "PRIVMSG");
    $plugin->addRule('prefix', "MiniMe, loadPlugin ");
    $plugin->addRule('nick',   "IsoTeemu");
    $plugin->addRule('break',  true);
    return;
}

$pluginName = trim(substr($plugin->line->msg, strrpos($plugin->line->msg, " ")+1));

if(isset($plugin->irc->irc_data->triggers->plugins[$pluginName])) {
    $plugin->irc->message("{$plugin->line->nick}: Plugin {$pluginName} on jo rekister�ity. k�yt� reloadPlugini�");
    return false;
}

if(!$plugin->irc->irc_data->triggers->registerPlugin($pluginName)) {
    $plugin->irc->message("{$plugin->line->nick}: Virhe ladattaessa plugini� {$pluginName}");
    return false;
}
$plugin->irc->message("{$plugin->line->nick}: Plugin {$pluginName} ladattiin onnistuneesti");
return true;

?>