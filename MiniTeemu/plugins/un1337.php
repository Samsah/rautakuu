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
    if ($pos = stristr($plugin->line->msg, $str)) {
        irc::trace("Teinixi� havaittu:{$str} in {$plugin->line->msg}[pos:$pos]");
        $plugin->send("KICK {$plugin->line->nick}: Teinixi� havaittu. Terveiset Chevylt�.");
    }
}

?>