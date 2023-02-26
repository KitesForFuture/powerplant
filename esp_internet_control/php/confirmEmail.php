<?php

$user = $_GET["user"];
$code = $_GET["code"];
$con = new mysqli("server.de", "username_w", "pw_w", "accounts");

//check if code coincides
$sql="SELECT `emailConfirmed` FROM `users` WHERE `username`='".$user."';";
$result = mysqli_query($con,$sql);

$result2 = mysqli_fetch_assoc($result);

if($result2['emailConfirmed'] == $code){
	//update emailConfirmed in users DB
	$sql="UPDATE users SET emailConfirmed = 0 WHERE `username`='".$user."';";
	//mysqli_query($con,$sql);
	if ($con->query($sql) === TRUE){
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
						<h1 style=\"font-family: Monospace; text-align:center; margin-bottom:10%\">yippie</h1>
						<div class = \"div2\">
						  	Email ist jetzt best&auml;tigt. Gehe zum <a href=\"https://www.kitesforfuture.de/control/login.php\" target=\"_top\" class=\"class1\">Login</a>
						</div>
					</div>
				</body></html>";
	}else{
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
					</style>
				</head>
				<body>
					<div class=\"div1\">
						<h1 style=\"font-family: Monospace; text-align:center; margin-bottom:10%\">...oops</h1>
						<div class = \"div2\">
						  	Unsere Datenbank spinnt scheinbar gerade. Sorry. Schick mir am Besten eine Email an benjamin@kitesforfuture.de
						</div>
					</div>
				</body></html>";
	}
}else if ($result2['emailConfirmed']==0){
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
						<h1 style=\"font-family: Monospace; text-align:center; margin-bottom:10%\">fertig!</h1>
						<div class = \"div2\">
						  	Deine Email ist bereits best&auml;tigt. Gehe zum <a href=\"https://www.kitesforfuture.de/control/login.php\" target=\"_top\" class=\"class1\">Login</a>
						</div>
					</div>
				</body></html>";
}else{
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
					</style>
				</head>
				<body>
					<div class=\"div1\">
						<h1 style=\"font-family: Monospace; text-align:center; margin-bottom:10%\">...oops</h1>
						<div class = \"div2\">
						  	Emailcode stimmt nicht mit dem Code in unserer Datenbank überein.
						</div>
					</div>
				</body></html>";
}

?>
