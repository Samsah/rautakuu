<?php
/**
 * vote.php - K�ynnist�� voten jostain
 */

static $vote;

if( $init==true ) {
    $plugin->addRule('code',   "PRIVMSG");
    $plugin->addRule('prefix', "vote ");

    if(!isset($vote)) $vote = array();
    $vote['ongoing'] = false;
    return;
}

$voteCmd = trim(substr($plugin->line->msg, 5));

/* Onko vanha vote jo umpeutunut? */
switch( $voteCmd ) {

    case "result" :
    case "tulos"  :
        if ($vote['ongoing'] == true && $vote['time'] > time()) {
            $plugin->irc->message("{$plugin->line->nick}: ��nestys ei ole viel� p��ttynyt.");
            break;
        }
        if( $vote['ongoing'] == true ) $vote['ongoing'] = false;
        $plugin->irc->message("{$plugin->line->nick}: ��nestys oli: \"".$vote['topic']."\"");
        if(($yes = count($vote['votes']['yes'])) > ($no = count($vote['votes']['no']))) {
            $plugin->irc->message("��nestyksen tulos: Kyll� voitti ��nin ".$yes."/".($yes+$no).".");
        } else {
            $plugin->irc->message("��nestyksen tulos: Ei voitti ��nin ".$no."/".($yes+$no).".");
        }
        break;

    case "no" :
    case "ei" :
    case "0"  :
    case "n"  :
    case "N"  :
    case "e"  :
    case "E"  :
        if($vote['ongoing'] != true) {
            $plugin->irc->message("{$plugin->line->nick}: ��nestyksi� ei ole menossa.");
            break;
        }
        if(isset($vote['votes']['no'][$plugin->line->nick]) || isset($vote['votes']['yes'][$plugin->line->nick])) {
            $plugin->irc->message("{$plugin->line->nick}: Olet jo ��nest�nyt.");
            break;
        }
        $vote['votes']['no'][$plugin->line->nick] = true;
        break;

    case "yes"   :
    case "kyll�" :
    case "1"     :
    case "Y"     :
    case "K"     :
    case "k"     :
        if($vote['ongoing'] != true) {
            $plugin->irc->message("{$plugin->line->nick}: ��nestyksi� ei ole menossa.");
            break;
        }
        if(isset($vote['votes']['no'][$plugin->line->nick]) || isset($vote['votes']['yes'][$plugin->line->nick])) {
            $plugin->irc->message("{$plugin->line->nick}: Olet jo ��nest�nyt.");
            break;
        }
        $vote['votes']['yes'][$plugin->line->nick] = true;
        break;

    default :
        if($vote['ongoing'] == true) {
            $plugin->irc->message("{$plugin->line->nick}: Tuntematon ��nestys komento. ��nest� joko kyll� tai ei.");
            break;
        }
        if(substr($voteCmd, -1) == "?") {
            $vote['ongoing']      = true;
            $vote['topic']        = $voteCmd;
            $vote['votes']        = array();
            $vote['votes']['yes'] = array();
            $vote['votes']['no']  = array();
            $vote['time']         = (time()+180);
            $plugin->irc->message("Uusi ��nestys: {$voteCmd}");
        } else {
            $plugin->irc->message("{$plugin->line->nick}: Tuntematon komento.");
        }
        break;
}

if($vote['ongoing'] == true && $vote['time'] < time()) {
    $plugin->irc->message("{$plugin->line->nick}: Vote on umpeutunut. Kiitoksia osallistuneille.");
    if(($yes = count($vote['votes']['yes'])) > ($no = count($vote['votes']['no']))) {
        $plugin->irc->message("��nestyksen tulos: Kyll� voitti ��nin ".$yes."/".($yes+$no).".");
    } else {
        $plugin->irc->message("��nestyksen tulos: Ei voitti ��nin ".$no."/".($yes+$no).".");
    }
    $vote['ongoing'] = false;
}

?>