<?php
			if ( ! empty( $_GET ) ) {
				if ( isset( $_GET['name'] ) && isset( $_GET['status'])  && isset( $_GET['line_length']) ) {
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
					
					//find kite in DB
					$sql="SELECT * FROM ".$table." WHERE `name`='".$_GET['name']."'";
					$result = mysqli_query($con,$sql);
					if($row = mysqli_fetch_assoc($result)){
						$kiteID = $row['ID'];
						$kiteCommand = $row['command'];
					}else{
						//else create it
						$sql = "INSERT INTO `kites`(`name`, `status`, `command`, `line_length`) VALUES ('".$_GET['name']."','".$_GET['status']."','LAND','".$_GET['line_length']."')";
						$result = mysqli_query($con,$sql);
						$sql="SELECT * FROM ".$table." WHERE `name`='".$_GET['name']."'";
						$result = mysqli_query($con,$sql);
						$row = mysqli_fetch_assoc($result);
						$kiteID = $row['ID'];
						$kiteCommand = $row['command'];
					}
					
					//write to DB
					$now = new DateTime("now");
					$timestring = $now->format('Y-m-d H:i:s');

					$sql = "UPDATE `kites` SET `status`='".$_GET['status']."',`line_length`='".$_GET['line_length']."',`datetime`='".$timestring."' WHERE `ID`='".$row['ID']."'";
					$result = mysqli_query($con,$sql);
					echo $kiteCommand;
				}
			}
?>

