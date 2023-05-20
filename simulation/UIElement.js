"use strict";

class UIElement{
	
	constructor(id){
		this.id = id;
	}
	
	getHTML(){
		return "\n\
			<tr>\n\
				<td>\n\
					<p id=\""+this.id+"_displayValue\"></p>\n\
				</td>\n\
			</tr>\n\
			";
	}
	
	init(){
	
	}
	
	setString(str){
		document.getElementById(""+this.id+"_displayValue").innerHTML = str;
	}
}
