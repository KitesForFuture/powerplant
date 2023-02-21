<?php
		// You'd put this code at the top of any "protected" page you create
		//echo $_POST['gamename'];
		
		// Always start this first
		session_start();
		
		if ( isset( $_SESSION['user_id'] ) && isset($_SESSION['user_group']) && $_SESSION['user_group'] == 'controller' ) {
			// Grab user data from the database using the user_id
			// Let them access the "logged in only" pages
			
			
			if ( ! empty( $_GET ) ) {
				if ( isset( $_GET['ID'] ) && isset( $_GET['command'] )) {
					//header("Location: https://www.isomathe.de/code/v6.11/commandKite.php?name=".$_POST['kitename']."&command=".$_POST['command']);
					
					//write command to db
					$servername = "dedivirt2155.your-server.de";
					$login = "dedivirk_2_w";
					$password = "J2jBvmM4rqr8S5Uf";
					$database = "kites_db";
					$table = "kites";
					
					$con = mysqli_connect($servername, $login, $password, $database);
					if (!$con) {
						die('Could not connect: ' . mysqli_error($con));
					}

					mysqli_select_db($con,$database);
					//write to DB
					$sql = "UPDATE `kites` SET `command`='".$_GET['command']."' WHERE `ID`='".$_GET['ID']."'";
					$result = mysqli_query($con,$sql);
				}
			}
			
			
			
			//get name of games:
			$servername = "dedivirt2155.your-server.de";
			$login = "dedivirk_2_r";
			$password = "j8PggCFXm6n1mAR8";
			$database = "kites_db";
			$table = "kites";
			
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
				echo "<td><a style=\"background-color: blue; color: white; text-decoration: none;\" href=\"controller.php?command=LAUNCH&ID=".$kiteID[$i]."\">LAND</a></td>";
			}else{
				echo "<td><a style=\"background-color: green; color: white; text-decoration: none;\" href=\"controller.php?command=LAND&ID=".$kiteID[$i]."\">LAUNCH</a></td>";
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
		} else {
			// TODO: Redirect them to the login page
			echo "Nothing to see here";
		}
		?>


