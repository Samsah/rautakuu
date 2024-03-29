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
	
	
	
	// Action Statistics
	
	// Addon Created by Rufus (rufus@nonstuff.de)
	
	$db->query("SELECT name FROM hlstats_Games WHERE code='$game'");
	if ($db->num_rows() < 1) error("No such game '$game'.");
	
	list($gamename) = $db->fetch_row();
	$db->free_result();
	
	pageHeader(
		array($gamename, "Action Statistics"),
		array($gamename=>"%s?game=$game", "Action Statistics"=>"")
	);


	$tblPlayerActions = new Table(
		array(
			new TableColumn(
				"description",
				"Action",
				"width=45&link=" . urlencode("mode=actioninfo&amp;action=%k&amp;game=$game")
			),
			new TableColumn(
				"obj_count",
				"Achieved",
				"width=25&align=right&append=+times"
			),
			new TableColumn(
				"obj_bonus",
				"Skill Bonus",
				"width=25&align=right"
			)
		),
		"code",
		"obj_count",
		"description",
		true,
		9999,
		"obj_page",
		"obj_sort",
		"obj_sortorder"
	);

	$db->query("
		SELECT
			COUNT(*)
		FROM
			hlstats_Actions, hlstats_Events_PlayerActions
		WHERE
			hlstats_Events_PlayerActions.actionId = hlstats_Actions.id
			AND hlstats_Actions.game='$game'
	");
	
	list($totalactions) = $db->fetch_row();
	
	$result = $db->query("
		SELECT
			hlstats_Actions.code,
			hlstats_Actions.description,
			COUNT(hlstats_Events_PlayerActions.id) AS obj_count,
			hlstats_Actions.reward_player AS obj_bonus
		FROM
			hlstats_Actions, hlstats_Events_PlayerActions, hlstats_Players
		WHERE
			hlstats_Events_PlayerActions.playerId = hlstats_Players.playerId
			AND hlstats_Players.game='$game'
			AND hlstats_Events_PlayerActions.actionId = hlstats_Actions.id
			AND hlstats_Actions.game='$game'
		GROUP BY
			hlstats_Actions.id
		ORDER BY
			$tblPlayerActions->sort $tblPlayerActions->sortorder,
			$tblPlayerActions->sort2 $tblPlayerActions->sortorder
	");
?>
<table width="90%" align="center" border=0 cellspacing=0 cellpadding=0>

<tr>
	<td width="50%"><?php echo $g_options["font_normal"]; ?>From a total of <b><?php echo $totalactions; ?></b> actions (Last <?php echo DELETEDAYS; ?> Days)<?php echo $g_options["fontend_normal"]; ?></td>
	<td width="50%" align="right"><?php echo $g_options["font_normal"]; ?>Back to <a href="<?php echo $g_options["scripturl"] . "?game=$game"; ?>"><?php echo $gamename; ?></a><?php echo $g_options["fontend_normal"]; ?></td>
</tr>

</table><p>
<?
	$tblPlayerActions->draw($result, $db->num_rows($result), 90);
?>
