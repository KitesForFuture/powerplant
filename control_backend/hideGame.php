<?php
// You'd put this code at the top of any "protected" page you create

// Always start this first
session_start();
if ( isset( $_SESSION['user_id'] ) && isset($_SESSION['user_group']) &&  $_SESSION['user_group'] == 'lehrer' ) {

	$servername = "dedivirt2155.your-server.de";
	$login = "gamesuser";
	$password = "rP5Tg4NpYFcMDHk6";
	$database = "gamesdb";
	
	$con = mysqli_connect($servername,$login,$password,$database);
	if (!$con) {
		die('Could not connect: ' . mysqli_error($con));
	}
	
	mysqli_select_db($con,$database);
	
	//check if you own this game
	$sql="SELECT `owner` FROM `games` WHERE `game`='".$_GET["game"]."';";
	$result = mysqli_query($con,$sql);
	$result2 = mysqli_fetch_assoc($result);
	if($result2['owner'] == $_SESSION['username']){
		$sql = "UPDATE `games` SET `hidden`=".$_GET["hidden"]." WHERE `game`='".$_GET["game"]."';";
		$result = mysqli_query($con,$sql);
		header("Location: https://www.isomathe.de/lehrer.php");
	}else{
		echo "Only its owner is allowed to hide/unhide this game.";
	}
}else{
	echo "Nur Lehrer erlaubt hier.";
}

?>
