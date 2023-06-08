/**
 * @author Benjamin Kutschan
 */

"use strict";

var outTime = 3000;

function disableCache(xhr){
	xhr.setRequestHeader('cache-control', 'max-age=0, private, must-revalidate');
	/*xhr.setRequestHeader('cache-control', 'no-cache, must-revalidate, post-check=0, pre-check=0');
	xhr.setRequestHeader('cache-control', 'max-age=0');
	xhr.setRequestHeader('expires', '0');
	xhr.setRequestHeader('expires', 'Tue, 01 Jan 1980 1:00:00 GMT');
	xhr.setRequestHeader('pragma', 'no-cache');*/
}

function getData(filename, callback){
	
	fetch(filename)
	  .then((res) => res.text())
	  .then((text) => {
		callback(text);
	   })
	  .catch((e) => console.error(e));
}

//data is $_POST["x"] variable in PHP, filename can have ?y=abc&z=def at the end, $_GET["y"] and $_GET["z"] in PHP
function sendData(data, filename, callback){
	
	
	document.body.classList.add('busy-cursor');
	//SEND DATA TO SERVER
	//let data = JSON.stringify([this.box.id, this.no, box.id]);
	//console.log(data);
	
	var xmlhttp;
	if (window.XMLHttpRequest) {
	    // code for IE7+, Firefox, Chrome, Opera, Safari
	    xmlhttp = new XMLHttpRequest();
	} else {
	    // code for IE6, IE5
	    xmlhttp = new ActiveXObject("Microsoft.XMLHTTP");
	}
	xmlhttp.onreadystatechange = function() {
	    if (this.readyState == 4 && this.status != 200) {
	        //document.getElementById("txtHint").innerHTML = this.responseText;
			document.body.classList.remove('busy-cursor');
	    	alert("sending to server failed. Error code is " + this.status);
	    	//don't connect
	    }
	    if(this.readyState == 4 && this.status == 200){
	    	//write to server successful
			document.body.classList.remove('busy-cursor');
	    	callback();
	    }
	};
    
	xmlhttp.open("POST",filename,true);
	
	//This is required to disable caching
    disableCache(xmlhttp);
    
	xmlhttp.setRequestHeader("Content-type", "application/x-www-form-urlencoded");
	xmlhttp.send("x=" + data);
}

