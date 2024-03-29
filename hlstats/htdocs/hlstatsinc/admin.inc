<?php
	/*
	 * HLstats - Real-time player and clan rankings and statistics for Half-Life
	 * http://sourceforge.net/projects/hlstats/
	 *
	 * Copyright (C) 2001  Simon Garner
	 *
	 * This program is free software; you can redistribute it and/or
	 * modify it under the terms of the GNU General Public License
	 * as published by the Free Software Foundation; either version 2
	 * of the License, or (at your option) any later version.
	 *
	 * This program is distributed in the hope that it will be useful,
	 * but WITHOUT ANY WARRANTY; without even the implied warranty of
	 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	 * GNU General Public License for more details.
	 *
	 * You should have received a copy of the GNU General Public License
	 * along with this program; if not, write to the Free Software
	 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
	 */

if(!defined("_HLSTATS")) die("Direct access denied.");

// Do _not_ cache thease pages.
$g_options['useCache'] = false;

global $game;
$resultGames = $db->query("
    SELECT
        code,
        name
    FROM
        hlstats_Games
    WHERE
        hidden='0'
    ORDER BY
        name ASC
    LIMIT 0,1
");

list($game) = $db->fetch_row($resultGames);

	class Auth
	{
		var $ok = false;
		var $error = false;

		var $username, $password, $savepass;
		var $sessionStart, $session;

		var $userdata = array();

		function Auth()
		{
			global $_POST, $_COOKIE;
			session_start();

			if ($_POST["authusername"])
			{
				$this->username = $_POST["authusername"];
				$this->password = $_POST["authpassword"];
				$this->savepass = $_POST["authsavepass"];
				$this->sessionStart = 0;

				# clear POST vars so as not to confuse the receiving page
				$_POST = array();

				$this->session = false;

				$this->checkPass();
			}
			elseif ($_COOKIE["authusername"])
			{
				$this->username 	= $_COOKIE["authusername"];
				$this->password 	= $_COOKIE["authpassword"];
				$this->savepass 	= $_COOKIE["authsavepass"];
				$this->sessionStart = $_COOKIE["authsessionStart"];

				$this->session = true;

				$this->checkPass();
			}
			else
			{
				$this->ok = false;
				$this->error = false;

				$this->session = false;

				$this->printAuth();
			}
		}

		function checkPass()
		{
			global $db;

			$db->query("
				SELECT
					*
				FROM
					hlstats_Users
				WHERE
					username='$this->username'
			");

			if ($db->num_rows() == 1)
			{
				// The username is OK

				$this->userdata = $db->fetch_array();
				$db->free_result();

                $db->query("SELECT MD5('$this->password')");
                list($authpass) = $db->fetch_row();
                $db->free_result();

                if ($authpass == $this->userdata["password"])
				{
					// The username and the password are OK

					$this->ok = true;
					$this->error = false;

					if ($this->sessionStart > (time()-3600))
					{
						// Valid session, update session time & display the page
						$this->doCookies();
						return true;
					}
					elseif ($this->sessionStart)
					{
						// A session exists but has expired
						if ($this->savepass)
						{
							// They selected 'Save my password' so we just
							// generate a new session and show the page.
							$this->doCookies();
							return true;
						}
						else
						{
							$this->ok = false;
							$this->error = "Your session has expired. Please try again.";
							$this->password = "";

							$this->printAuth();
							return false;
						}
					}
					elseif (!$this->session)
					{
						// No session and no cookies, but the user/pass was
						// POSTed, so we generate cookies.
						$this->doCookies();
						return true;
					}
					else
					{
						// No session, user/pass from a cookie, so we force auth
						$this->printAuth();
						return false;
					}
				}
				else
				{
					// The username is OK but the password is wrong

					$this->ok = false;
					if ($this->session)
					{
						// Cookie without 'Save my password' - not an error
						$this->error = false;
					}
					else
					{
						$this->error = "The password you supplied is incorrect.";
					}
					$this->password = "";
					$this->printAuth();
				}
			}
			else
			{
				// The username is wrong
				$this->ok = false;
				$this->error = "The username you supplied is not valid.";
				$this->printAuth();
			}
		}

		function doCookies()
		{
			setCookie("authusername", $this->username, time()+31536000, "", "", 0);

			if ($this->savepass)
			{
				setCookie("authpassword", $this->password, time()+31536000, "", "", 0);
			}
			else
			{
				setCookie("authpassword", $this->password, 0, "", "", 0);
			}
			setCookie("authsavepass", $this->savepass, time()+31536000, "", "", 0);
			setCookie("authsessionStart", time(), 0, "", "", 0);
		}

		function printAuth()
		{
			global $g_options;

			include(INCLUDE_PATH . "/adminauth.inc");
			exit();
		}
	}


	class AdminTask
	{
		var $title = "";
		var $acclevel = 0;
		var $type = "";
		var $description = "";

		function AdminTask ($title, $acclevel, $type="general", $description="")
		{
			$this->title = $title;
			$this->acclevel = $acclevel;
			$this->type = $type;
			$this->description = $description;
		}
	}


	class EditList
	{
		var $columns;
		var $keycol;
		var $table;
		var $icon;
		var $showid;

		var $errors;
		var $newerror;

		function EditList ($keycol, $table, $icon, $showid=true)
		{
			$this->keycol = $keycol;
			$this->table = $table;
			$this->icon = $icon;
			$this->showid = $showid;
		}

		function update ()
		{
			global $HTTP_POST_VARS, $db;

			$okcols = 0;
			foreach ($this->columns as $col)
			{
				$value = $HTTP_POST_VARS["new_$col->name"];

				if ($value != "")
				{
					if ($col->type == "ipaddress" &&
						!ereg("^[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}$", $value))
					{
						$this->errors[] = "Column '$col->title' requires a valid IP address for new row";
						$this->newerror = true;
						$okcols++;
					}
					else
					{
						if ($qcols) $qcols .= ", ";
						$qcols .= $col->name;

						if ($qvals) $qvals .= ", ";

						if ($col->type == "password")
						{
							$qvals .= "MD5('$value')";
						}
						else
						{
							$qvals .= "'$value'";
						}

						if ($col->type != "select" && $col->type != "hidden" && $value != $col->datasource)
							$okcols++;
					}
				}
				elseif ($col->required)
				{
					$this->errors[] = "Required column '$col->title' must have a value for new row";
					$this->newerror = true;
				}
			}

			if ($okcols > 0 && !$this->errors)
			{
				$db->query("
					INSERT INTO
						$this->table
						(
							$qcols
						)
					VALUES
					(
						$qvals
					)",
					false
				);

				if ($db->dberror())
				{
					$this->errors[] = "DB Error: " . $db->dberror();
				}
			}
			elseif ($okcols == 0)
			{
				$this->errors = array();
				$this->newerror = false;
			}

			if (!is_array($HTTP_POST_VARS["rows"]))
				return true;

			foreach ($HTTP_POST_VARS["rows"] as $row)
			{
				$row = stripslashes($row);

				if ($HTTP_POST_VARS[$row . "_delete"])
				{
					$db->query("
						DELETE FROM
							$this->table
						WHERE
							$this->keycol='" . addslashes($row) . "'
					");
				}
				else
				{
					$rowerror = false;

					$query = "UPDATE $this->table SET ";
					$i=0;
					foreach ($this->columns as $col)
					{
						$value = $HTTP_POST_VARS[$row . "_" . $col->name];

						if ($col->type == "password" && $value == "(encrypted)")
							continue;

						if ($value == "" && $col->required)
						{
							$this->errors[] = "Required column '$col->title' must have a value for row '$row'";
							$rowerror = true;
						}
						elseif ($col->type == "ipaddress" &&
						!ereg("^[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}$", $value))
						{
							$this->errors[] = "Column '$col->title' requires a valid IP address for row '$row'";
							$rowerror = true;
						}

						if ($i > 0) $query .= ", ";

						if ($col->type == "password")
						{
							$query .= $col->name . "=MD5('$value')";
						}
						else
						{
							$query .= $col->name . "='$value'";
						}
						$i++;
					}
					$query .= " WHERE $this->keycol='" . addslashes($row) . "'";

					if (!$rowerror)
					{
						$db->query($query);
					}
				}
			}

			if ($this->error())
			{
				return false;
			}
			else
			{
				return true;
			}
		}

		function draw ($result)
		{
			global $g_options, $HTTP_POST_VARS, $db;
?>
<table width="75%" border=0 cellspacing=0 cellpadding=0>

<tr valign="top" bgcolor="<?php echo $g_options["table_border"]; ?>">
	<td><table width="100%" border=0 cellspacing=1 cellpadding=4>

		<tr valign="bottom">
<?php
			echo "<td bgcolor=\"" . $g_options["table_head_bgcolor"] . "\"></td>";

			if ($this->showid)
			{
?>
			<td align="right" bgcolor="<?php echo $g_options["table_head_bgcolor"]; ?>"><?php echo $g_options["font_small"];
				echo "ID";
				echo $g_options["fontend_small"];
?></td>
<?php
			}

			foreach ($this->columns as $col)
			{
				if ($col->type == "hidden") continue;
				echo "<td bgcolor=\"" . $g_options["table_head_bgcolor"] . "\">"
					. $g_options["font_small"] . "<font color=\""
					. $g_options["table_head_text"] . "\">" . $col->title
					. "</font>" . $g_options["fontend_small"] . "</td>\n";
			}
?>
			<td align="center" bgcolor="<?php echo $g_options["table_head_bgcolor"]; ?>"><?php echo $g_options["font_small"];
				echo "Delete";
				echo $g_options["fontend_small"];
?></td>
		</tr>

<?php
			while ($rowdata = $db->fetch_array($result))
			{
				echo "\n<tr>\n";
				echo "<td align=\"center\" bgcolor=\"" . $g_options["table_bgcolor1"] . "\">";
				echo "<img src=\"" . $g_options["imgdir"] . "/$this->icon.gif\" width=16 height=16 border=0></td>\n";

				if ($this->showid)
				{
					echo "<td align=\"right\" bgcolor=\"" . $g_options["table_bgcolor2"] . "\">"
						. $g_options["font_small"] . $rowdata[$this->keycol] . $g_options["fontend_small"]
						. "</td>\n";
				}

				$this->drawfields($rowdata, false, false);
?>
<td align="center" bgcolor="<?php echo $g_options["table_bgcolor2"]; ?>"><input type="checkbox" name="<?php echo $rowdata[$this->keycol]; ?>_delete" value="1"></td>
<?php echo "</tr>\n\n";
			}
?>

<tr>
<?php
			echo "<td bgcolor=\"" . $g_options["table_bgcolor1"] . "\" align=\"center\">"
				. $g_options["font_small"] . "new" . $g_options["fontend_small"] . "</td>\n";

			if ($this->showid)
				echo "<td bgcolor=\"" . $g_options["table_bgcolor2"] . "\" align=\"right\">"
					. $g_options["font_small"] . "&nbsp;" . $g_options["fontend_small"] . "</td>\n";

			if ($this->newerror)
			{
				$this->drawfields($HTTP_POST_VARS, true, true);
			}
			else
			{
				$this->drawfields(array(), true);
			}

			echo "<td bgcolor=\"" . $g_options["table_bgcolor1"] . "\"></td>\n";
?>
</tr>

		</table></td>
</tr>

</table><p>
<?php
		}


		function drawfields ($rowdata=array(), $new=false, $stripslashes=false)
		{
			global $g_options, $db;

			$i=0;
			foreach ($this->columns as $col)
			{
				if ($new)
				{
					$keyval = "new";
					$rowdata[$col->name] = $rowdata["new_$col->name"];
					if ($stripslashes) $rowdata[$col->name] = stripslashes($rowdata[$col->name]);
				}
				else
				{
					$keyval = $rowdata[$this->keycol];
					if ($stripslashes) $keyval = stripslashes($keyval);
				}

				if ($col->type != "hidden")
				{
					echo "<td bgcolor=\"" . $g_options["table_bgcolor1"] . "\">";
				}

				if ($i == 0 && !$new)
				{
					echo "<input type=\"hidden\" name=\"rows[]\" value=\"" . htmlspecialchars($keyval) . "\">";
				}

				if ($col->maxlength < 1)
					$col->maxlength = "";

				switch ($col->type)
				{
					case "select":
						unset($coldata);

						if (ereg(";", $col->datasource))
						{
							// for manual datasource in format "key/value;key/value" or "key;key"
							foreach (explode(";", $col->datasource) as $v)
							{
								if (ereg("/", $v))
								{
									list($a, $b) = explode("/", $v);
									$coldata[$a] = $b;
								}
								else
								{
									$coldata[$v] = $v;
								}
							}
						}
						else
						{
							// for SQL datasource in format "table.column/keycolumn/where"
							list($col_table, $col_col) = explode(".", $col->datasource);
							list($col_col, $col_key, $col_where) = explode("/", $col_col);
							if ($col_where) $col_where = "WHERE $col_where";
							$col_result = $db->query("SELECT $col_key, $col_col FROM $col_table $col_where ORDER BY $col_key");
							$coldata = array();
							while (list($k, $v) = $db->fetch_row($col_result))
							{
								$coldata[$k] = $v;
							}
						}

						if ($col->width) $width = " style=\"width:" . $col->width*5 . "px\"";
						else $width = "";

						echo "<select name=\"" . $keyval . "_$col->name\"$width>\n";

						if (!$col->required)
						{
							echo "<option value=\"\">\n";
						}

						$gotcval = false;

						foreach ($coldata as $k=>$v)
						{
							if ($rowdata[$col->name] == $k)
							{
								$selected = " selected";
								$gotcval = true;
							}
							else
							{
								$selected = "";
							}

							echo "<option value=\"$k\"$selected>$v\n";
						}

						if (!$gotcval)
						{
							echo "<option value=\"" . $rowdata[$col->name] . "\" selected>"
								. $rowdata[$col->name] . "\n";
						}

						echo "</select>";
						break;

					case "checkbox":
						$selectedval = "1";
						$value = $rowdata[$col->name];

						if ($value == $selectedval) $selected = " checked";
						else $selected = "";

						echo "<center><input type=\"checkbox\" name=\"" . $keyval
							. "_$col->name\" value=\"$selectedval\"$selected></center>";
						break;

					case "hidden":
						echo "<input type=\"hidden\" name=\"" . $keyval
							. "_$col->name\" value=\"" . htmlspecialchars($col->datasource) . "\">";
						break;

					default:
						if ($col->datasource != "" && !isset($rowdata[$col->name]))
							$value = $col->datasource;
						else
							$value = $rowdata[$col->name];

						echo "<input type=\"text\" name=\"" . $keyval
							. "_$col->name\" size=$col->width "
							. "value=\"" . htmlspecialchars($value) . "\" class=\"textbox\""
							. " maxlength=\"$col->maxlength\">";
				}

				if ($col->type != "hidden")
				{
					echo "</td>\n";
				}

				$i++;
			}
		}

		function error()
		{
			if (is_array($this->errors))
			{
				return implode("<p>\n\n", $this->errors);
			}
			else
			{
				return false;
			}
		}
	}

	class EditListColumn
	{
		var $name;
		var $title;
		var $width;
		var $required;
		var $type;
		var $datasource;
		var $maxlength;

		function EditListColumn ($name, $title, $width=20, $required=false, $type="text", $datasource="", $maxlength=0)
		{
			$this->name = $name;
			$this->title = $title;
			$this->width = $width;
			$this->required = $required;
			$this->type = $type;
			$this->datasource = $datasource;
			$this->maxlength = intval($maxlength);
		}
	}



	class PropertyPage
	{
		var $table;
		var $keycol;
		var $keyval;
		var $propertygroups = array();

		function PropertyPage ($table, $keycol, $keyval, $groups)
		{
			$this->table  = $table;
			$this->keycol = $keycol;
			$this->keyval = $keyval;
			$this->propertygroups = $groups;
		}

		function draw ($data)
		{
			foreach ($this->propertygroups as $group)
			{
				$group->draw($data);
			}
		}

		function update ()
		{
			global $HTTP_POST_VARS, $db;

			$setstrings = array();
			foreach ($this->propertygroups as $group)
			{
				foreach ($group->properties as $prop)
				{
					$setstrings[] = $prop->name . "='" . $HTTP_POST_VARS[$prop->name] . "'";
				}
			}

			$db->query("
				UPDATE
					" . $this->table . "
				SET
					" . implode(",\n", $setstrings) . "
				WHERE
					" . $this->keycol . "='" . $this->keyval . "'
			");
		}
	}

	class PropertyPage_Group
	{
		var $title = "";
		var $properties = array();

		function PropertyPage_Group ($title, $properties)
		{
			$this->title = $title;
			$this->properties = $properties;
		}

		function draw ($data)
		{
			global $g_options;
?>
<b><?php echo $this->title; ?></b><br>
<table width="100%" border=0 cellspacing=0 cellpadding=0>

<tr valign="top" bgcolor="<?php echo $g_options["table_border"]; ?>">
	<td><table width="100%" border=0 cellspacing=1 cellpadding=4>
<?php
			foreach ($this->properties as $prop)
			{
				$prop->draw($data[$prop->name]);
			}
?>
		</table></td>
</tr>

</table><p>
<?php
		}
	}

	class PropertyPage_Property
	{
		var $name;
		var $title;
		var $type;

		function PropertyPage_Property ($name, $title, $type, $datasource="")
		{
			$this->name  = $name;
			$this->title = $title;
			$this->type  = $type;
			$this->datasource = $datasource;
		}

		function draw ($value)
		{
			global $g_options;
?>
<tr valign="middle">
	<td width="45%" bgcolor="<?php echo $g_options["table_bgcolor1"]; ?>"><?php
	echo $g_options["font_normal"];
	echo $this->title . ":";
	echo $g_options["fontend_normal"];
?></td>
	<td width="55%" bgcolor="<?php echo $g_options["table_bgcolor1"]; ?>"><?php
			switch ($this->type)
			{
				case "textarea":
					echo "<textarea name=\"$this->name\" cols=35 rows=4 wrap=\"virtual\">"
						. htmlspecialchars($value)
						. "</textarea>";
					break;

				case "select":
					// for manual datasource in format "key/value;key/value" or "key;key"
					foreach (explode(";", $this->datasource) as $v)
					{
						if (ereg("/", $v))
						{
							list($a, $b) = explode("/", $v);
							$coldata[$a] = $b;
						}
						else
						{
							$coldata[$v] = $v;
						}
					}

					echo getSelect($this->name, $coldata, $value);
					break;

				default:
					echo "<input type=\"text\" name=\"$this->name\" size=35 value=\""
						. htmlspecialchars($value)
						. "\" class=\"textbox\">";
					break;
			}
?></td>
</tr>
<?php
		}
	}


	function message ($icon, $msg)
	{
		global $g_options;
?>
		<table width="60%" border=0 cellspacing=0 cellpadding=0>

		<tr valign="top">
			<td width=40><img src="<?php echo $g_options["imgdir"] . "/$icon"; ?>.gif" width=16 height=16 border=0 hspace=5></td>
			<td width="100%"><?php
	echo $g_options["font_normal"];
	echo "<b>$msg</b>";
	echo $g_options["fontend_normal"];
?></td>
		</tr>

		</table><p>
<?php
	}




	$auth = new Auth;

	pageHeader(array("Admin"), array("Admin"=>""));

	$selTask = $HTTP_GET_VARS["task"];
	$selGame = $HTTP_GET_VARS["admingame"];
?>

<table width="90%" align="center" border=0 cellspacing=0 cellpadding=0>

<tr valign="top">
	<td><?php
	echo $g_options["font_normal"];

	// General Settings
	$admintasks["options"]			= new AdminTask("HLstats Options", 100);
	$admintasks["adminusers"]		= new AdminTask("Admin Users", 100);
	$admintasks["games"]			= new AdminTask("Games", 100);
	$admintasks["hostgroups"]		= new AdminTask("Host Groups", 100);
	$admintasks["clantags"]			= new AdminTask("Clan Tag Patterns", 80);
	$admintasks["plugins"]			= new AdminTask("Server Plugins", 80);

	// Game Settings
	$admintasks["servers"]			= new AdminTask("Servers", 100, "game");
	$admintasks["actions"]			= new AdminTask("Actions", 80, "game");
	$admintasks["teams"]			= new AdminTask("Teams", 80, "game");
	$admintasks["roles"]			= new AdminTask("Roles", 80, "game");
	$admintasks["weapons"]			= new AdminTask("Weapons", 80, "game");
	$admintasks["awards_weapons"]	= new AdminTask("Weapon Awards", 80, "game");
	$admintasks["awards_actions"]	= new AdminTask("Action Awards", 80, "game");

	// Tools
	$admintasks["tools_editdetails"] = new AdminTask("Edit Player or Clan Details", 80, "tool",
		"Edit a player or clan's profile information.");
	$admintasks["tools_adminevents"] = new AdminTask("Admin-Event History", 80, "tool",
		"View event history of logged Rcon commands and Admin Mod messages.");
	$admintasks["tools_ipstats"]	= new AdminTask("Host Statistics", 80, "tool",
		"See which ISPs your players are using.");
	$admintasks["tools_optimize"]	= new AdminTask("Optimize Database", 100, "tool",
		"This operation tells the MySQL server to clean up the database tables, optimizing them for better performance. It is recommended that you run this at least once a month.");
	$admintasks["tools_reset"]		= new AdminTask("Reset Statistics", 100, "tool", "Delete all players, clans and events from the database.");

	// Sub-Tools
	$admintasks["tools_editdetails_player"] = new AdminTask("Edit Player Details", 80, "subtool", "Edit a player's profile information.");
	$admintasks["tools_editdetails_clan"]   = new AdminTask("Edit Clan Details", 80, "subtool", "Edit a clan's profile information.");

	// Show Tool
	if ($admintasks[$selTask] && $admintasks[$selTask]->type == "tool" || $admintasks[$selTask]->type == "subtool")
	{
		$task = $admintasks[$selTask];
		$code = $selTask;
?>
&nbsp;<img src="<?php echo $g_options["imgdir"]; ?>/downarrow.gif" width=9 height=6 border=0 align="middle" alt="downarrow.gif"><b>&nbsp;<a href="<?php echo $g_options["scripturl"]; ?>?mode=admin">Tools</a></b><br>
<img src="<?php echo $g_options["imgdir"]; ?>/spacer.gif" width=1 height=8 border=0><br>

<?php
		include(INCLUDE_PATH . "/admintasks/$code.inc");
	}
	else
	{
		// General Settings
?>
&nbsp;<img src="<?php echo $g_options["imgdir"]; ?>/downarrow.gif" width=9 height=6 border=0 align="middle" alt="downarrow.gif"><b>&nbsp;General Settings</b><p>
<?php
		foreach ($admintasks as $code=>$task)
		{
			if ($auth->userdata["acclevel"] >= $task->acclevel && $task->type == "general")
			{
				if ($selTask == $code)
				{
?>
&nbsp;&nbsp;&nbsp;&nbsp;<img src="<?php echo $g_options["imgdir"]; ?>/downarrow.gif" width=9 height=6 border=0 align="middle" alt="downarrow.gif"><b>&nbsp;<a href="<?php echo $g_options["scripturl"]; ?>?mode=admin" name="<?php echo $code; ?>"><?php echo
$task->title; ?></a></b><p>

<form method="POST" action="<?php echo $g_options["scripturl"]; ?>?mode=admin&task=<?php echo $code; ?>#<?php echo $code; ?>">

<table width="100%" border=0 cellspacing=0 cellpadding=0>

<tr>
	<td width="10%">&nbsp;</td>
	<td width="90%"><?php
	echo $g_options["font_normal"];
	include(INCLUDE_PATH . "/admintasks/$code.inc");
	echo $g_options["fontend_normal"];
?></td>
</tr>

</table><br><br>
</form>
<?php
				}
				else
				{
?>
&nbsp;&nbsp;&nbsp;&nbsp;<img src="<?php echo $g_options["imgdir"]; ?>/rightarrow.gif" width=6 height=9 border=0 align="middle"
alt="rightarrow.gif"><b>&nbsp;<a href="<?php echo $g_options["scripturl"]; ?>?mode=admin&task=<?php echo $code; ?>#<?php echo $code;
?>"><?php echo $task->title; ?></a></b><p> <?php
				}
			}
		}


		// Game Settings
?>
&nbsp;<img src="<?php echo $g_options["imgdir"]; ?>/downarrow.gif" width=9 height=6 border=0 align="middle" alt="downarrow.gif"><b>&nbsp;Game Settings</b><p>
<?php
		$gamesresult = $db->query("
			SELECT
				name,
				code
			FROM
				hlstats_Games
		");

		while ($gamedata = $db->fetch_array($gamesresult))
		{
			$gamename = $gamedata["name"];
			$gamecode = $gamedata["code"];

			if ($gamecode == $selGame)
			{
?>
&nbsp;&nbsp;&nbsp;&nbsp;<img src="<?php echo $g_options["imgdir"]; ?>/downarrow.gif" width=9 height=6 border=0 align="middle" alt="downarrow.gif"><b>&nbsp;<a href="<?php echo $g_options["scripturl"]; ?>?mode=admin" name="game_<?php echo $gamecode; ?>"><?php echo $gamename;?></a></b> (<?php echo $gamecode; ?>)<p> <?php
				foreach ($admintasks as $code=>$task)
				{
					if ($auth->userdata["acclevel"] >= $task->acclevel && $task->type == "game")
					{
						if ($selTask == $code)
						{
?>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<img src="<?php echo $g_options["imgdir"]; ?>/downarrow.gif" width=9 height=6 border=0 align="middle" alt="downarrow.gif"><b>&nbsp;<a href="<?php echo $g_options["scripturl"]; ?>?mode=admin&admingame=<?php echo $gamecode; ?>" name="<?php echo $code; ?>"><?php echo $task->title; ?></a></b><p>

<form method="POST" action="<?php echo $g_options["scripturl"]; ?>?mode=admin&admingame=<?php echo $gamecode; ?>&task=<?php echo $code; ?>#<?php echo $code; ?>">

<table width="100%" border=0 cellspacing=0 cellpadding=0>

<tr>
	<td width="10%">&nbsp;</td>
	<td width="90%"><?php
	echo $g_options["font_normal"];
	include(INCLUDE_PATH . "/admintasks/$code.inc");
	echo $g_options["fontend_normal"];
?></td>
</tr>

</table><br><br>
</form>
<?php
						}
						else
						{
?>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<img src="<?php echo $g_options["imgdir"]; ?>/rightarrow.gif" width=6 height=9 border=0 align="middle" alt="downarrow.gif"><b>&nbsp;<a href="<?php echo $g_options["scripturl"]; ?>?mode=admin&admingame=<?php echo $gamecode; ?>&task=<?php echo $code; ?>#<?php echo $code; ?>"><?php echo $task->title; ?></a></b><p> <?php
						}
					}
				}
			}
			else
			{
?>
&nbsp;&nbsp;&nbsp;&nbsp;<img src="<?php echo $g_options["imgdir"]; ?>/rightarrow.gif" width=6 height=9 border=0 align="middle" alt="rightarrow.gif"><b>&nbsp;<a href="<?php echo $g_options["scripturl"]; ?>?mode=admin&admingame=<?php echo $gamecode; ?>#game_<?php echo $gamecode; ?>"><?php echo $gamename;?></a></b> (<?php echo $gamecode; ?>)<p> <?php
			}
		}
	}
	echo "</td>\n";

	if (!$selTask || !$admintasks[$selTask])
	{
		echo "<td width=\"50%\">";
		echo $g_options["font_normal"];
?>
&nbsp;<img src="<?php echo $g_options["imgdir"]; ?>/downarrow.gif" width=9 height=6 border=0 align="middle" alt="downarrow.gif"><b>&nbsp;Tools</b>

<ul>
<?php
	foreach ($admintasks as $code=>$task)
	{
		if ($auth->userdata["acclevel"] >= $task->acclevel && $task->type == "tool")
		{
?>	<li><b><a href="<?php echo $g_options["scripturl"]; ?>?mode=admin&task=<?php echo $code; ?>"><?php echo $task->title; ?></a></b><br>
		<?php echo $task->description; ?><p>
<?php
		}
	}
?>
</ul>
<?php
		echo $g_options["fontend_normal"];
		echo "</td>";
	}
?>
</tr>

</table>
