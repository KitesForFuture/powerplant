<?php
		// You'd put this code at the top of any "protected" page you create
		//echo $_POST['gamename'];
		
		// Always start this first
		session_start();
		
		if ($_SERVER['HTTP_HOST'] == "kitesforfuture.de")
		{
		   $url = "https://www." . $_SERVER['HTTP_HOST'] . $_SERVER['REQUEST_URI'];
		   header("Location: $url");
		}
		
		if ( ! empty( $_POST ) ) {
			if ( isset( $_POST['username'] ) && isset( $_POST['email'] ) && isset( $_POST['password'] ) ) {
			
			
				$sanitized_username = filter_var($_POST['username'], FILTER_SANITIZE_EMAIL);
				$sanitized_email = filter_var($_POST['email'], FILTER_SANITIZE_EMAIL);
				$sanitized_password = filter_var($_POST['password'], FILTER_SANITIZE_EMAIL);
				
				if((filter_var($sanitized_email, FILTER_VALIDATE_EMAIL) && $sanitized_email == $_POST['email']) || $sanitized_email == ""){
					if($sanitized_password == $_POST['password']){
						$con = new mysqli("server.de", "username_w", "pw_w", "accounts");
						$sql="SELECT `username` FROM `users` WHERE `username`='".$sanitized_username."';";
						$result = mysqli_query($con,$sql);
						$result2 = mysqli_fetch_assoc($result);
						
						//username not yet present in DB
						if($result2 == NULL){
							//create new user
							
							//create random number for email verification:
							$email_verification_number = mt_rand(2,999999);
							
							$hashed_and_salted_pw = sha1($sanitized_password."https://www.kitesforfuture.de");
							if($sanitized_email == ""){
								$email_verification_number = 0;
							}
							$sql = "INSERT INTO users (username, password, email, kontostand, registration_date, emailConfirmed) VALUES (\"".$sanitized_username."\",\"".$hashed_and_salted_pw."\",\"".$sanitized_email."\",0, 0, ".$email_verification_number.");";
							
							//add line for this user in Achievements table
							//$sql2 = "INSERT INTO userAchievements (username) VALUES (\"".$sanitized_username."\");";
							
							if ($con->query($sql) === TRUE) {
								
								if($sanitized_email == ""){
									
									$con = new mysqli("server.de", "username_r", "pw_r", "accounts");
									$stmt = $con->prepare("SELECT * FROM users WHERE username = ?");
									$stmt->bind_param('s', $sanitized_username);
									$stmt->execute();
									$result = $stmt->get_result();
									$user = $result->fetch_object();
									
									$_SESSION['user_id'] = $user->ID;
									$_SESSION['user_group'] = $user->user_group;
									$_SESSION['username'] = $user->username;
									header("Location: https://www.kitesforfuture.de/control/control.php");
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
							background-color: #f2f2f2;
							margin: 0px;
							padding:0px;
							height:100%;
							width: 100%;
							background-image: url('background.jpg');
							background-size: cover;
							background-position: center;
						}

							.div1 {
							  font-family: Arial, Helvetica, sans-serif;
							  border-radius: 5px;
							  background-color: #f2f2f2;
							  padding: 1vw;
							  
							  position: absolute;
							  font-size: 5vmin;
							  font-weight: 600;
							  
							  top: 10%;
							  left: 50%;
							  
							  transform: translate(-50%, 0)
							}
							.homelink {
							  text-decoration: underline;
							  color: black;
							  text-align: center;
							}
							.div2 {
							  font-weight: 300;
							  
							  text-align: center
							  
							}
					</style>
				</head>
				<body>
					<div class=\"div1\">
						<div class = \"div2\">
						  	Jetzt Emails checken, auch den <i>Spam-Ordner</i>!
						</div>
					</div>
				</body></html>";
								//send verification email
								exec("./mail_confirmation.bash ".$sanitized_email." ".$sanitized_username." ".$email_verification_number);
								exec("./mail_notification.bash ".$sanitized_email." ".$sanitized_username);
							} else {
								//echo "Error: " . $sql . "<br>" . $con->error;
								echo "Error. Bitte versuch's nochmal: <a href=\"https://www.kitesforfuture.de/control/register.php\" target=\"_top\">registrieren</a>";
							}
							$con->close();
							
						}else{
							echo "Den Benutzernamen gibt's leider schon. Bitte versuch's nochmal: <a href=\"https://www.kitesforfuture.de/control/register.php\" target=\"_top\">registrieren</a>";
						}
					}else{
						echo "Das Passwort darf nur Buchstaben, Zahlen und !#$%&'*+-=?^_`{|}~@.[] enthalten. Bitte versuch's nochmal: <a href=\"https://www.kitesforfuture.de/control/register.php\" target=\"_top\">registrieren</a>";
					}
				}else{
					echo "Email ist keine email. Bitte versuch's nochmal: <a href=\"https://www.kitesforfuture.de/control/register.php\" target=\"_top\">registrieren</a>";
				}
			}
		} else {
			// Show Registering Form
			echo "<html><head>
					<meta http-equiv=\"Cache-Control\" content=\"no-cache, no-store, must-revalidate\" />
					<meta http-equiv=\"Pragma\" content=\"no-cache\" />
					<meta http-equiv=\"Expires\" content=\"0\" />
					<meta charset=\"utf-8\">
					<meta name=\"viewport\" >
					<style>
						body {
							font-family: Monospace;
							background-color: #f2f2f2;
							margin: 0px;
							padding:0px;
							height:100%;
							width: 100%;
							background-image: url('background.jpg');
							background-size: cover;
							background-position: center;
						}
						
						input{
							  width: 100%;
							  padding: 12px 20px;
							  margin: 8px 0;
							  display: inline-block;
							  border: 1px solid #ccc;
							  border-radius: 4px;
							  box-sizing: border-box;
							  font-size: 4vmin;
							  box-shadow:none;
							}

							input:-moz-placeholder,
							
							input:focus {
							  box-shadow:none !important;
							}

							input[type=submit] {
							  width: 100%;
							  background-color: #f44336;
							  color: white;
							  padding: 14px 20px;
							  margin: 10 0%;
							  border: none;
							  border-radius: 4px;
							  cursor: pointer;
							  font-size: 5vmin;
							  font-weight: 600;
							}

							input[type=submit]:hover {
							  background-color: red;
							}

							.div1 {
							  font-family: Arial, Helvetica, sans-serif;
							  border-radius: 5px;
							  background-color: #f2f2f2;
							  padding: 1vw;
							  
							  position: absolute;
							  font-size: 5vmin;
							  font-weight: 600;
							  
							  top: 10%;
							  left: 50%;
							  
							  transform: translate(-50%, 0)
							}
							
							.div2 {
							  font-weight: 300;
							  
							  text-align: center
							  
							}
							
							.bottom {
							  margin-top:10%;
							  font-size: 4vmin;
							  text-align: right
							}
							.homelink {
							  text-decoration: none;
							  color: black;
							  text-align: center;
							  margin-bottom: 10%;
							}
							a.class1:link, a.class1:visited {
							  background-color: #f44336;
							  color: white;
							  text-align: center;
							  text-decoration: none;
							}

							a.class1:hover, a.class1:active {
							  background-color: red;
							}
							
							a.class1 {
							  width: 100%;
				  			  padding: 14px 20px;
							  margin: 8px 0;
							  display: inline-block;
							  border: none;
							  border-radius: 4px;
							  box-shadow: none;
							  box-sizing: border-box;
							  cursor: pointer;
							  font-size: 5vmin;
							  font-weight: 600;
							  box-shadow: none;
							}
					</style>
				</head>
				<body>
					<div class=\"div1\">
						<h1 style=\"text-align: center;\"><a href=\"https://www.kitesforfuture.de/control\" target=\"_top\" class=\"homelink\">kitesforfuture/control</a></h1>
						<form action=\"\" method=\"post\">
							<div style=\"font-size: 3vmin;font-weight: 200;\">Irgendein Loginname:</div>
							<input id=\"username\" type=\"text\" name=\"username\" placeholder=\"Loginname\" required>
							<div style=\"font-size: 3vmin;font-weight: 200;\" id=\"usernameTaken\"></div>
							<div style=\"font-size: 3vmin;font-weight: 200;\">Neues Passwort:</div>
							<input type=\"password\" name=\"password\" placeholder=\"Passwort\" required>
							<div style=\"font-size: 3vmin;font-weight: 200;\">Email:</div>
							<input type=\"text\" name=\"email\" placeholder=\"Email\">
							<input type=\"submit\" value=\"Registrieren\">
						</form>
						<a style=\"background-color: gray;\" href=\"https://www.kitesforfuture.de/control/login.php\" target=\"_top\" class=\"class1\">&#x21E6;</a>
						<!-- Wir schicken dir keine Werbe-Emails und speichern dein Passwort nur als \"salted hash\". -->
					</div>
					 <script>
						var input = document.getElementById(\"username\");
						var usernameTaken = document.getElementById(\"usernameTaken\");
						input.addEventListener(\"change\", function(e){
							console.log(e); console.log(e.target.value);
							
							let xmlhttp;
							//RETRIEVE STUFF FROM DATABASE:
							if (window.XMLHttpRequest) {
								// code for IE7+, Firefox, Chrome, Opera, Safari
								xmlhttp = new XMLHttpRequest();
							} else {
								// code for IE6, IE5
								xmlhttp = new ActiveXObject(\"Microsoft.XMLHTTP\");
							}
							xmlhttp.onreadystatechange = function() {
								//console.log(this);
								if (this.readyState == 4 && this.status == 200) {
									document.body.classList.remove('busy-cursor');
									console.log(this);
									if(this.responseText.substring(0, 1) == \"0\"){
										usernameTaken.innerHTML = \"<font color=\\\"green\\\">&#9989;</font>\";
									}else{
										usernameTaken.innerHTML = \"<font color=\\\"red\\\">Dieser Loginname ist schon vergeben :(</font>\";
									}
								}
							};
							
							xmlhttp.open(\"GET\",\"checkUsername.php?username=\" + e.target.value,true);
							
							//This is required to disable caching
							xmlhttp.setRequestHeader('cache-control', 'max-age=0, private, must-revalidate');
							
							xmlhttp.send();
							
							
						});
					</script> 
				</body></html>";
		}
		?>


