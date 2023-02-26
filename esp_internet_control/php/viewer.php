<?php
		// You'd put this code at the top of any "protected" page you create
		
		// Always start this first
		
		session_start();
		
		if ( isset( $_SESSION['user_id'] ) ) {
			if(isset($_SESSION['user_group']) && $_SESSION['user_group'] == 'viewer'){
			
				
			
			//get name of games:
			$servername = "server.de";
			$login = "username_r";
			$password = "pw_r";
			$database = "kites_database";
			$table = "kites_table";
			
			$con = mysqli_connect($servername, $login, $password, $database);
			if (!$con) {
				die('Could not connect: ' . mysqli_error($con));
			}

			mysqli_select_db($con,$database);

			$sql="SELECT * FROM ".$table." WHERE 1";
			$result = mysqli_query($con,$sql);
			$i = 0;
			while($row = mysqli_fetch_assoc($result)){
				$kiteID[$i] = $row['ID'];
				$kiteName[$i] = $row['name'];
				$kiteStatus[$i] = $row['status'];
				$kiteStatusTime[$i] = $row['datetime'];
				$kiteCommand[$i] = $row['command'];
				$kiteLineLength[$i] = $row['line_length'];
				$i = $i+1;
			}
			
			
			
			
			
			echo "<html><head>
		<meta http-equiv=\"Cache-Control\" content=\"no-cache, no-store, must-revalidate\" />
		<meta http-equiv=\"Pragma\" content=\"no-cache\" />
		<meta http-equiv=\"Expires\" content=\"0\" />
		<meta charset=\"utf-8\">
		<meta name=\"viewport\" >
		<style>
			body {
				font-family: Monospace;
				background-color: #f0f0f0;
				
			}
			table {
			  font-family: arial, sans-serif;
			  border-collapse: collapse;
			  width: 100%;
			}

			td, th {
			  border: 1px solid #dddddd;
			  text-align: left;
			  padding: 8px;
			}

			tr:nth-child(even) {
			  background-color: #dddddd;
			}
		</style>
	</head>
	<body>
		<div id=\"div\">

		<table style=\"table-layout: auto;\">
			";
			
			//$date = new DateTime('2000-01-01');
			//$result = $date->format('Y-m-d H:i:s');
			
		while($i > 0){
			$i = $i - 1;
			echo "<tr><th>Name</th><th>Status</th><th>Current Command</th><th>Line Length</th></tr>";
			echo "<tr><td>".$kiteName[$i]."</td> ";
			$lastKiteStatusUpdated = new DateTime($kiteStatusTime[$i]);
			$lastKiteStatusUpdated->modify('+1 minutes');
			$now = new DateTime("now");
			if($lastKiteStatusUpdated > $now){
				echo "<td>".$kiteStatus[$i]."</td>";
			}else{
				echo "<td><font color=\"red\">offline</font> (last seen ".$kiteStatusTime[$i].")</td>";
			}
			if($kiteCommand[$i] == 'LAND'){
				echo "<td><div style=\"color: blue; text-decoration: none;\" href=\"controller.php?command=LAUNCH&ID=".$kiteID[$i]."\">LAND</div></td>";
			}else{
				echo "<td><div style=\"color: green; text-decoration: none;\" href=\"controller.php?command=LAND&ID=".$kiteID[$i]."\">LAUNCH</div></td>";
			}
			echo "<td>".$kiteLineLength[$i]."</td>";
			echo "</tr>";
		}
		echo"
		</table>
		

		</div>
		<a id = \"id_\" href=\"https://www.kitesforfuture.de/control/logout.php\" target=\"_top\">Logout</a>
		
		<script>
			setTimeout(() => {  location.reload(); }, 5000);
		</script>
		
	</body>
</html>";
			
			
			
			}else{
				echo "Nur Viwer d&uuml;rfen momentan hierher.";
			}
		
		} else {
			// TODO: Redirect them to the login page
			echo "Du musst dich erst einloggen: <a href=\"https://www.kitesforfuture.de/control/login.php\" target=\"_top\">Login</a>";
		}
		?>

