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
    'rofl',
    'munq',
);

$byes = array(
    'N�kemiin',
    'Ixudzan',
    'Ciao',
    'Farewell',
    'Adiau^',
    'Hyv�sti',
    'Au revoir',
    'Bonne journ�e',
    'Auf Wiedersehen',
    'Wiedersehen',
);

foreach($teinix as $str) {
    if (stristr($plugin->line->msg, $str)) {
        $bye=$byes[array_rand($byes)];
        irc::trace("Teinixi� havaittu:{$str} in {$plugin->line->msg}");
        $plugin->irc->send("KICK {$plugin->line->channel} {$plugin->line->nick} : Teinixi�. $bye");
    }
}

?>