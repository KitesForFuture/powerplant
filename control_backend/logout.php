<?php
		// You'd put this code at the top of any "protected" page you create
		//echo $_POST['gamename'];
		
		// Always start this first
		session_start();
		
		session_destroy();
		header("Location: http://www.kitesforfuture.de/control");
?>


