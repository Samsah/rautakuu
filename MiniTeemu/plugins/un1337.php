<?php

if( $init==true ) {
    $plugin->addRule('code', 'PRIVMSG');
    return;
}

$teinix = array(
    'omg',
    'imac',
    'vinq',
    'parq',
    'lol',
    'itq',
    'stfu',
);

foreach($teinix as $str) {
    if (stristr($plugin->line->msg, $str)) {
        irc::trace("Teinixi� havaittu:{$str} in {$plugin->line->msg}");
        $plugin->irc->send("KICK {$plugin->line->channel} {$plugin->line->nick} : Teinixi� havaittu. Terveiset Chevylt�.");
    }
}

?>