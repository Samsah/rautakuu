<?php

class irc_data {

    // K�sitelt�v� data
    var $data;

    // Ylij��nyt osa
    var $leftOver = "";

    // Rivi objectit
    var $lines = array();

    // Kokonais rivien m��r�
    var $i=0;

    // Viimeksi tarkastettu rivi
    var $lastExecLine=0;

    function append($rawdata) {
        // Muutetaan windows enterit UNIX enteriksi.
        $this->data = str_replace("\r", '', $rawdata);
        $this->leftOver = "";

        // Jos dataa j�� yli, yritett�n se ottaa talteen.
        if(( $strrpos = strrpos($data ,"\n")) !== false ) {
            $this->leftOver = substr($this->data, $strrpos+1);
            irc::trace("LEFTOVER DATA: {$this->leftOver}");

            // Otetaan oleellinen osa, ja j�tet��n yli��nyt osa pois.
            $this->data = substr($this->data, 0, $strrpos);
        }

        // Hajotettaan data riveiksi.
        $lines = explode("\n", $this->data );

        while( count( $lines ) > 0 ) {
            $this->lines[$this->i] =& new irc_data_line( array_shift($lines) );
            // Line ei ollut hyv�ksytt�v�. Pudotetaan se.
            if(!$this->lines[$this->i]->valid()) {
                unset($this->lines[$this->i]);
                continue;
            }

            $this->i++;
        }

        // Huono koodi. Parempi cleanup tapa t�ytyy tehd�.
        if(($offrows = $this->numLines()) > 500 ) {
            irc::trace("Yli 500 rivin m��r� t�yttynyt.");
            $this->lines = array_slice($this->lines, $offrows-500);
        }
    }

    function numLines() {
        return count( $lines );
    }

    /**
     * Ajaa triggerit
     */
    function runTriggers() {
        irc::trace("lastExecLine: {$this->lastExecLine} i: {$this->i} ");
        global $irc_triggers;
        while( $this->lastExecLine < $this->i ) {
            $irc_triggers->event($this->lines[$this->lastExecLine]);
            $this->lastExecLine++;
        }
    }

    /**
     * palauttaa ylij��neen osan.
     * @param $nuke h�vitet��nk� ylij��nyt osa.
     */
    function getLeftOvers($nuke=true) {
        if(empty($this->leftOver)) return;
        $data = $this->leftOver;
        if( $nuke == true ) {
            $this->leftOver = "";
        }
        return $data;
    }

    /**
     * T�m� palauttaa viimeisen rivin.
     */
    function getLastLine() {
        if( is_array($this->lines) && count($this->lines) > 0 ) {
            return $this->lines[count($this->lines)-1]->getLine();
        } else {
            return "";
        }
    }
}

class irc_data_line {

    var $data;
    var $valid;

    var $nick;
    var $from;
    var $code;
    var $host;
    var $ident;
    var $channel;

    var $msg;

    var $ping = false;

    function irc_data_line($line) {
        irc::trace("Saatu dataa:\n<< \"{$line}\"");
        if( substr($line,0,6) != "PING :" && $this->valid($line) ) {
            $this->data = substr($line, 1);
        } else {
            $this->data = $line;
        }

        $this->parseLine();
    }

    function valid($line=null) {
        if( $line === null ) $line = $this->data;
        if(!empty( $this->valid )) return $this->valid;
        if( substr($line,0,6) == "PING :" ) {
            $this->valid = true;
        } elseif( substr( $line, 0, 1 ) == ":" ) {
            $this->valid = true;
        } else {
            $this->valid = false;
        }
        return $this->valid;
    }

    function parseLine() {

        if( substr($this->data,0,6) == "PING :") {
            $this->ping = true;
            return;
        }

        $exs = explode(" ", $this->data);
        $poe = strpos( $exs[0], "!" );
        $poa = strpos( $exs[0], "@" );
        $poc = strpos( $this->data, ":" );

        $this->code    = $exs[1];

        $this->from    = $exs[0];
        $this->nick    = substr($exs[0], 0, $poe);
        $this->host    = substr($exs[0], $poa+1);
        $this->ident   = substr($exs[0], $poe+1, ($poa-$poe)-1);
        $this->channel = $exs[4];

        $this->msg     = trim(substr($this->data, $poc+1));
    }

    function getLine() {
        return $this->data;
    }

    function get ($what) {
        if(isset($this->$what)) {
            return $this->$what;
        } else {
            irc::trace("Ei {$what}:ia tiedossa");
            return false;
        }
    }
}

define("IRC_TRACE_ECHO", 0);
define("IRC_TRACE_SEND", 1);

class irc {

    // Serveri, jolle liityt��n
    var $server     = "port80.se.quakenet.org";

    // Serverin portti.
    var $port       = "6667";

    // Kanavan nimi, jolle liityt��n.
    var $channel    = "#rautakuu";

    // Botin henk. koht. tietoja.
    var $botRName   = "Mini Me, completes me!";
    var $botNick    = "MiniTeemu";
    var $botUName   = "rautakuu";

    // Kuinka monta kertaa yritet��n yhdist�� ennen kuin annetaan periksi.
    var $tries      = 5;

    // Yhteys resurssi
    var $_connection;

    // Kuinka kauan odotetaan tapahtumahorisonttia?
    var $_delay     = 100;

    // IRC data container
    var $irc_data;

    // Bufferi joka sis�lt�� kaiken saamamme datan.
    var $_loggedin  = false;

    // Trace ajuri
    var $traceDrv   = IRC_TRACE_ECHO;

    /**
     * PHP5 constructori. Kutsuu PHP4 constructorin
     */
    function __construct($config=null) {
        $this->irc($config);
    }

    /**
     * PHP4 constructor
     * @param $config asetukset
     */
    function irc( $config = null ) {
        if( $config != null && is_array( $config )) {
            if( $config['server'] )     $this->server   = $config['channel'];
            if( $config['channel'] )    $this->channel  = $config['channel'];
            if( $config['port'] )       $this->port     = $config['port'];
        }

        // Huuhdellaan v�litt�m�sti
        ob_implicit_flush(true);

        // Ei aikarajaa
        @set_time_limit(0);

        // PHP4 __destructori emulaatio
        if( version_compare( phpversion(), "5.0.0", "<") == 1 ){
            register_shutdown_function(array(&$this,"__destruct"));
        }

        // Signal handlerit
        //pcntl_signal(SIGTERM, array(&$this,"_SigTerm"));
        //pcntl_signal(SIGKILL, array(&$this,"_SigKill"));

    }

    // Hoitaa viestin l�hetyksen IRC serverille
    function send( $msg ) {
        if( $this->_state() === false ) {
            $this->trace("Yhteytt� ei ole");
            return false;
        }
        if(! fwrite($this->_connection, $msg."\r\n")) {

            $this->trace("Viestin l�hetys ep�onnistui \"{$msg}\"");
            return false;
        }
        $this->trace("Viesti l�hetetty\n>> \"{$msg}\"");
        return true;
    }

    /**
     * Yhdist�� palvelimelle
     */
    function connect() {
        // Yritet��n loopata yhteytt�
        $i = 0;
        while( $i < $this->tries ) {

            $i++;
            $this->trace("Yritet��n yhdist�� #".($i));
            $this->_connection = fsockopen($this->server, $this->port, $errno, $errstr);

            if( $this->_connection === false ) {
                $this->trace("Ei saatu yhdistetty� palvelimelle \"{$this->server}:{$this->port}\". Syy: {$errstr} ({$errno})");
                continue;
            } else {
                break;
            }
        }
        if( $this->_connection === false ) {
            $this->trace("Yhteytt� ei saatu muodostettua");
            return false;
        }

        socket_set_blocking($this->_connection, false);
    }

    /**
     * Kirjaudu palvelimelle
     */
    function login($usermode=0) {
        $this->send("NICK ".$this->botNick);
        $this->send("USER ".$this->botUName." ".$usermode." * :".$this->botRName);
        $this->loggedin = true;
    }

    /**
     * Liity kanavalle
     */
    function join($channel=NULL) {
        if( $channel !== NULL ) {
            $this->channel = $channel;
        }
        $this->send("JOIN ".$this->channel);
    }

    /**
     * Kuuntelee kanavaa.
     */
    function listen() {

        if(!isset($this->irc_data)) {
            $this->irc_data =& new irc_data();
        }

        // Niin kauan kuin me olemme yhteydess�, kuuntele.
        while( $this->_state() ) {
            // Nukutaan hetki
            usleep( $this->_delay*1000 );

            // Jos ei kirjauduttu, kirjaudu.
            if($this->loggedin == false) {
                $this->login();
            }

            $rawdata = trim(fread($this->_connection, 10240));

            // Littet��n edellinen ylij��nyt data nykyiseen dataan
            $rawdata = trim($this->irc_data->getLeftOvers()).$rawdata;

            if(!empty( $rawdata )) {

                // Liitet��n data ja otetaan ylij��nyt data talteen
                $this->irc_data->append( $rawdata );

                $this->irc_data->runTriggers();
            }
        }
    }

    function part( $channel=NULL, $reason=NULL ) {
        if( $reason !== NULL ) {
            $reason = " :".$reason;
        }
        $this->send("PART $channel $reason");
    }

    function disconnect($msg="") {
        $this->send("QUIT $msg");
    }

    function pong($data) {
        $this->send("PONG ".$data);
    }

    /**
     * L�hett�� perinteisen viestin.
     * Voidaan kutsua staattisesti, jos $irc on on rekister�ity sivulla,
     * ja kenelle viesti on osoitettu on asetettu.
     */
    function message($msg, $to=null) {

        //irc::trace("Yritet��n l�hett�� viesti�: ".$message);

        global $irc;
        if(is_a($this, "irc")) {
            if($to === null) $to = $this->channel;
            $message = "PRIVMSG ".$to." :".$msg;
            $this->send($message);
        } elseif( is_a($irc, "irc")) {
            if($to === null) $to = $irc->channel;
            $message = "PRIVMSG ".$to." :".$msg;
            $irc->send($message);
        }
    }

    function trace($msg="") {
        if( function_exists( "debug_backtrace" )) {
            $tstack = debug_backtrace();
            if( $tstack[0]['function'] == "trace" ) $tstack = array_slice($tstack, 1);
            $msg = $tstack[0]['class'].$tstack[0]['type'].$tstack[0]['function']."[".$tstack[0]['line']."]: ".$msg;
        }
        /*
        switch( $this->traceDrv ) {
            case IRC_TRACE_SEND :
                $this->message($msg);
                break;
            case IRC_TRACE_ECHO :
            default :
                echo $msg."\n";
                break;
        }
        */
        echo $msg."\n";
    }

    /**
     * Palauttaa yhteyden tilan.
     * @return bool true jos yhteys auki, false jos kiinni
     */
    function _state() {
        if( is_resource( $this->_connection ) &&
            get_resource_type( $this->_connection ) == "stream" ) return true;
        return false;
    }

    function _SigTerm() {
        $this->disconnect("Joku terminoi minut");
        die();
    }

    function _SigKill() {
        $this->disconnect("Joku tappoi minut");
        die();
    }

    function __destruct() {
        if( $this->_state() ) {
            $this->disconnect();
        } else {
            $this->trace();
        }
    }
}

/**
 * base Luokka joka yritt�� hanskata mit� datalla tehd��n
 */
class irc_triggers_base {

    /**
     * Triggerit ja niiden s��nn�t
     */
    var $triggers;

    var $line;

    function irc_triggers_base() {
        // Rekister�id��n Ping vastaus.
        $this->registerTrigger(array('ping'  => true,
                                     'event' => array(&$this,'pong')));

        // liitty-ensin vastaus
        $this->registerTrigger(array('code'  => '451',
                                     'break' => true,
                                     'event' => array(&$this,'login')));

        // Welcome koodi.
        $this->registerTrigger(array('code'  => '001',
                                     'break' => true,
                                     'event' => array(&$this,'channel')));

        // Nick jo k�yt�ss�
        $this->registerTrigger(array('code'  => '433',
                                     'break' => true,
                                     'event' => array(&$this,'changeNick')));
    }

    function __construct() {
        $this->irc_triggers_base();
    }

    /**
     * Lis�� triggerin trigger kasaan
     * @param $rules triggerin s��nn�t
     * @param $offset -1 nostaa p��llim�iseksi, 0 seuraavaksi ja sit� suuremmat sijainnin
     */
    function registerTrigger( $rules, $offset=0 ) {
        if( $offset == 0 ) {
            $this->triggers[] = $rules;
        } else {
            $this->triggers[$offset] = $rules;
        }
    }


    function &triggerLine( &$line ) {
        $this->line =& $line;
    }

    function event( &$line ) {
        if( $line !== null ) {
            $this->triggerLine(&$line);
        }

        foreach($this->triggers as $trigger) {
            $valid = true;
            // Testaa s��nt�jen paikkaansa pit�vyyden.
            foreach( $trigger as $key => $param ) {
                if( $key == "event" ) continue;
                if( $key == "break" ) continue;

                if( $this->line->$key != $param ) {
                    //irc::trace("Avain $key ei vastaa arvoa $param");
                    $valid = false;
                    break;
                }
            }

            if ($valid == true) {
                // Katsotaan, voiko tapahtumaa suorittaa
                if( is_array( $trigger['event'] )) {
                    call_user_method($trigger['event'][1], $trigger['event'][0]);
                } else {
                    if(function_exists($trigger['event'])) {
                        call_user_method($trigger['event']);
                    } else {
                        irc::trace("Rekister�ity event ei kutsuttavissa: ".print_r($trigger['event'], 1));
                    }
                }
                // Katkaistaanko s��nt�ketjun tarkistus?
                if ( $trigger['break'] == true ) break;
            }
        }
    }


    function pong() {
        global $irc;
        irc::trace("Ping? Pong! ".$this->line->getLine());
        $irc->pong(substr( $this->line->getLine(), 5));
    }

    function login() {
        global $irc;
        irc::trace("451; Rekister�idy ensin.");
        $irc->login();
    }

    function channel() {
        global $irc;
        irc::trace("Liityt��n kanavalle");
        $irc->join();
    }

    function changeNick() {
        global $irc;
        $nick =& $irc->botNick;

        $nickLchar = substr($nick, -1);

        if( is_int($nickLchar)) {
            $nickLchar++;
            $nick = substr($nick, 0, strlen($nick)-1).$nickLchar;
        } else {
            $nick .= 0;
        }

        irc::trace("Nick jo k�yt�ss�. Vaihdetaan se $nick");

        $irc->login();
    }
}

/**
 * Oma event trigger homma
 */
class irc_triggers extends irc_triggers_base {
    function irc_triggers() {
        $this->irc_triggers_base();

        // MOTD
        $this->registerTrigger(array('code'  => '372',
                                     'break' => true,
                                     'event' => array(&$this,'motd')));
    }

    function __construct() {
        $this->irc_triggers();
    }

    /**
     * Ker�� palvelimen MOTDn.
     */
    function motd() {
        static $motd;
        if(!isset($motd)) $motd = "";
        $motd .= $this->line->get("msg");
        return $motd;
    }

}

// Satunnaisia testifunctioita

class irc_triggers_test extends irc_triggers {
    function irc_triggers_test() {
        $this->irc_triggers();

        // MOTD
        $this->registerTrigger(array('code'  => 'PRIVMSG',
                                     'nick'  => 'IsoTeemu',
                                     'msg'   => 'MiniMe, uptime',
                                     'break' => true,
                                     'event' => array(&$this,'getUptime')));
        $this->registerTrigger(array('code'  => 'PRIVMSG',
                                     'nick'  => 'IsoTeemu',
                                     'msg'   => 'MiniMe, memusage',
                                     'break' => true,
                                     'event' => array(&$this,'getMemUsage')));

        $this->registerTrigger(array('code'  => 'PRIVMSG',
                                     'msg'   => 'MiniMe, disconnect',
                                     'nick'  => 'IsoTeemu',
                                     'break' => true,
                                     'event' => array(&$this,'disconnect')));

        $this->registerTrigger(array('code'  => 'PRIVMSG',
                                     'msg'   => 'fortune',
                                     'break' => true,
                                     'event' => array(&$this,'fortune')));
        $this->registerTrigger(array('code'  => 'PRIVMSG',
                                     'event' => array(&$this,'notMaster')));
        $this->registerTrigger(array('code'  => 'MODE',
                                     'event' => array(&$this,'didIGotOP')));
        $this->registerTrigger(array('code'  => 'JOIN',
                                     'event' => array(&$this,'opTeemu')));

    }

    function __construct() {
        $this->irc_triggers_test();
    }

    function getUptime() {
        global $irc;
        $irc->message(trim(exec("uptime")));
    }

    function getMemUsage() {
        global $irc;
        if(! function_exists("memory_get_usage")) {
            $irc->message("Herrani, en voi t�ytt�� pyynt��si");
            return false;
        }
        $irc->message(memory_get_usage());
    }

    function notMaster() {
        if(strstr($this->line->get("msg"), "MiniMe," )) {
            global $irc;
            $irc->message("Hyv� herra ".$this->line->get("nick").", et vaikuta is�nn�lt�ni, enk� suostu pyynt��si");
        }
    }

    function didIGotOP() {
        global $irc;

        if( strstr($this->line->getLine(), "+o ".$irc->botNick)) {
            $irc->message("Kiitos rakas ".$this->line->get("nick"));
        } elseif( strstr($this->line->getLine(), "-o ".$irc->botNick)) {
            $irc->message("Hei, tuo oli niinku tosi uncoolia!");
        }
    }

    function fortune() {
        global $irc;
        exec('fortune -s', $rows);
        foreach( $rows as $row ) {
            $irc->message($row);
        }
    }

    function opTeemu() {
        global $irc;
        if( $this->line->get("nick") == "IsoTeemu" ) {
            $irc->send("MODE ".$irc->channel." +o ".$this->line->get("nick"));
        }
    }

    function tracePath() {
        global $irc;
        if( $irc->traceDrv == IRC_TRACE_ECHO ) {
            $irc->message("Vaihdetaan traceksi IRC_TRACE_SEND");
            $irc->traceDrv = IRC_TRACE_SEND;
        } elseif( $irc->traceDrv == IRC_TRACE_SEND ) {
            $irc->message("Vaihdetaan traceksi IRC_TRACE_ECHO");
            $irc->traceDrv = IRC_TRACE_ECHO;
        } else {
            $irc->message("Tuntematon trace ajuri");
        }
    }

    function disconnect() {
        global $irc;
        $irc->message("Bye!");
        $irc->disconnect();
    }

}

class irc_triggers_hlstats extends irc_triggers_test {

    var $db;

    function irc_triggers_hlstats() {
        $this->irc_triggers_test();

        $this->registerTrigger(array('code'  => 'PRIVMSG',
                                     'msg'   => 'skill',
                                     'break' => true,
                                     'event' => array(&$this,'skill')));

        $this->registerTrigger(array('code'  => 'PRIVMSG',
                                     'msg'   => 'top15',
                                     'break' => true,
                                     'event' => array(&$this,'top15')));

        $this->registerTrigger(array('code'  => 'PRIVMSG',
                                     'break' => true,
                                     'event' => array(&$this,'skillByName')));

    }

    function __construct() {
        $this->irc_triggers_hlstats();
    }

    /**
     * @todo
     */
    function skill() {
        global $irc;
        $skill = $this->_getSkill(addslashes($this->line->get("nick")));

       if( $skill === false ) {
            $irc->message("Anteeksi, mutten l�yt�nyt pisteit� nickillesi ".$this->line->get("nick").".");
        } else {
            $irc->message("Nickill� ".$this->line->get("nick")." on ".$skill." pistett�.");
        }
    }

    function skillbyname() {
        if(!strstr($this->line->get("msg"), "skill ")) return false;
        global $irc;

        list( $nick ) = sscanf($this->line->get("msg"), "skill %s");
        $nick = trim($nick);
        if(empty($nick)) return false;
        $skill = $this->_getSkill(addslashes($nick));

        if( $skill === false ) {
            $irc->message("Ei l�ytynyt pisteit� nickille ".$nick.".");
        } else {
            $irc->message("Nickin \"$nick\" pisteet on $skill.");
        }
    }

    function _getSkill($nick) {
        if(!$this->_db()) return false;
        $sql = sprintf("SELECT hlstats_Players.skill
                        FROM hlstats_Players
                        LEFT JOIN hlstats_PlayerNames ON
                        hlstats_PlayerNames.playerId = hlstats_Players.playerId
                        WHERE hlstats_Players.lastName LIKE '%s'
                        OR hlstats_PlayerNames.name LIKE '%s'
                        GROUP BY hlstats_Players.playerId
                        ORDER BY hlstats_PlayerNames.numuses DESC",
                        $nick, $nick);
        $res =& $this->db->query($sql);
        if (DB::isError($res)) {
            global $irc;
            $irc->message("Yritin suoritta hakua, mutta sain viestin: ".$res->getMessage());
            return false;
        } else {
            if( $res->numRows() < 1 ) {
                return false;
            } else {
                $entry = $res->fetchRow();
                $res->free();
                return $entry[0];
            }
        }
    }

    function _db() {
        global $irc;
        if(empty($this->db)) {
            include_once("DB.php");
            $this->db =& DB::Connect("mysql://cs:f9307fe00c@localhost/hlds");
        }
        if (DB::isError($this->db)) {
            $irc->message("Yritin yhdist�� tiedokantaan, mutta sain viestin: ".$this->db->getMessage());
            return false;
        }
        return true;
    }

    function top15() {
        if(!$this->_db()) return false;
        global $irc;
        $top15 =& $this->db->query("SELECT lastName
FROM hlstats_Players
WHERE game='cstrike'
AND hideranking=0
AND kills >= 1
ORDER BY skill desc, lastName ASC
LIMIT 0,15");
        if (DB::isError($top15)) {
            $irc->message("Virhe haettassa top15: ".$top->getMessage());
            return false;
        }
        $nicks = "";
        while( $row =& $top15->fetchRow()) {
            if( $nicks != "" ) $nicks .= ", ";
            $nicks .= $row[0];
        }
        $top15->free();
        $irc->message("Top15; $nicks");
    }
}

$irc_triggers =& new irc_triggers_hlstats();

$irc =& new irc();
$GLOBALS['irc'] =& $irc;

irc::trace("Yhdistet��n...");
$irc->connect();
irc::trace("Kuunnellaan...");
$irc->listen();


?>