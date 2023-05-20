"use strict";

class Collapsible extends UIElement{
	
	constructor(id, name){
		super(id);
		this.name = name;
		this.children = [];
	}
	
	getHTML(){
		let returnValue = "\n\
			<button type=\"button\" id=\"" + this.id + "\" class=\"collapsible\">"+this.name+"</button>\n\
			<div style=\"font-size: 10px; display: none;\">\n\
			  	<table style=\"font-size: 10px;\">\n\
			";
		
		for(let i = 0; i < this.children.length; i++){
			returnValue+= this.children[i].getHTML();
		}
		
		returnValue += "\n\
			  		\n\
			  	</table>\n\
			  	<hr>\n\
			</div>\n\
			";
		
		return returnValue;
	}
	
	add(element){
		this.children.push(element);
		return element;
	}
	
	init(){
		for(let i = 0; i < this.children.length; i++){
			this.children[i].init();
		}
		document.getElementById(this.id).addEventListener("click", function() {
					this.classList.toggle("active");
					var content = this.nextElementSibling;
					if (content.style.display === "block") {
					  content.style.display = "none";
					} else {
					  content.style.display = "block";
					}
				  });
	}
}
