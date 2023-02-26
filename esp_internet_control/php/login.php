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
		
		// Grab user data from the database using the user_id
		// Let them access the "logged in only" pages
		if ( isset( $_SESSION['user_id'] ) ) {
			if(isset($_SESSION['user_group']) && $_SESSION['user_group'] == 'viewer'){
				header("Location: http://www.kitesforfuture.de/control/viewer.php");
			}else if(isset($_SESSION['user_group']) && $_SESSION['user_group'] == 'controller'){
				header("Location: http://www.kitesforfuture.de/control/controller.php");
			}else{
				echo "Nicht genügend Rechte, um fortzufahren.";
				// remove all session variables
				session_unset();
				// destroy the session
				session_destroy();
			}
			
		} else {
			if ( ! empty( $_POST ) ) {
				if ( isset( $_POST['username'] ) && isset( $_POST['password'] ) ) {
					// Getting submitted user data from database
					$sanitized_username = filter_var($_POST['username'], FILTER_SANITIZE_EMAIL);
					$sanitized_password = filter_var($_POST['password'], FILTER_SANITIZE_EMAIL);
					
					$con = new mysqli("server.de", "username_r", "pw_r", "accounts");
					$stmt = $con->prepare("SELECT * FROM users WHERE username = ?");
					$stmt->bind_param('s', $sanitized_username);
					$stmt->execute();
					$result = $stmt->get_result();
					$user = $result->fetch_object();
					
					// Verify user password and set $_SESSION
					//if ( password_verify( $sanitized_password, $user->password ) ) {
					if ( sha1($sanitized_password."https://www.kitesforfuture.de") == $user->password ) {
						//checking emailVerification status
						if($user->emailConfirmed == 0){
							//only allow testers to proceed
							$_SESSION['user_id'] = $user->ID;
							$_SESSION['user_group'] = $user->user_group;
							$_SESSION['username'] = $user->username;
							if($user->user_group == 'viewer'){
								header("Location: http://www.kitesforfuture.de/control/viewer.php");
							}else if($user->user_group == 'controller'){
								header("Location: http://www.kitesforfuture.de/control/controller.php");
							}else{
								echo "Sorry. Momentan d&uuml;rfen sich nur Viewer und Controller anmelden.";
								// remove all session variables
								session_unset();
								// destroy the session
								session_destroy();
							}
						}else{
							echo "Bitte best&auml;tige erst deine Email. Gehe dazu in dein Emailpostfach.";
						}
						
					}else{
						echo "Falsches Passwort für Nutzer ".$sanitized_username;
					}
				}
			} else {
				// Show choice --Login or Register--
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
							  
							  transform: translate(-50%, 0);
							  
							  opacity: 0.9;
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
							<input type=\"text\" name=\"username\" placeholder=\"...dein Nutzername\" required>
							
							<input type=\"password\" name=\"password\" placeholder=\"...dein Passwort\" required>
							
							<input type=\"submit\" value=\"Login\" class=\"submitClass\">
						  </form>
						  
						  <div class = \"div2\">
						  	Noch keinen Account? Dann einfach hier
						  	<br>
						  	<a href=\"https://www.kitesforfuture.de/control/register.php\" target=\"_top\" class=\"class1\">registrieren</a>
						  </div>
						  <a style=\"background-color: gray;\" href=\"https://www.kitesforfuture.de/control\" target=\"_top\" class=\"class1\">&#x21E6;</a>
						  
						</div>
						
					</body></html>";
			}
		}
		?>


