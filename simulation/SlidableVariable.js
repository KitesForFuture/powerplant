"use strict";

class SlidableVariable extends UIElement{
	
	constructor(id, displayName, min, max, initialValue, factor, digitsbehindComma, unit){
		super(id);
		this.name = name;
		this.min = min;
		this.max = max;
		this.initialValue = initialValue;
		this.currentValue = initialValue*factor;
		this.displayName = displayName;
		this.factor = factor;
		this.digitsbehindComma = digitsbehindComma;
		this.unit = unit;
		this.oninput = function(){};
	}
	
	getHTML(){
		return "\n\
			<tr>\n\
				<td>\n\
					<p id=\""+this.id+"_displayValue\">"+this.displayName+"</p>\n\
				</td>\n\
			</tr>\n\
			<tr>\n\
				<td>\n\
					<input type=\"range\" id=\"" + this.id + "_slider\" min=\""+this.min+"\" max=\""+this.max+"\" style=\"height: 5px;\">\n\
				</td>\n\
			</tr>\n\
			";
	}
	
	init(){
		this.getSliderElement().value = this.initialValue;
		
		let that = this;
		this.getSliderElement().oninput = function(){
			//console.log(parseFloat(that.getSliderElement().value)*this.factor);
			that.setString(that.displayName + ": " + (that.getSliderElement().value*that.factor).toFixed(that.digitsbehindComma) + that.unit);
			//console.log("value = " + this.value + "factor = " + this.factor);
			that.currentValue = this.value*that.factor;
			that.oninput(that.currentValue);
		};
	}
	
	setValue(value){
		this.getSliderElement().value = value/this.factor;
		this.currentValue = value;
		this.setString(this.displayName + ": " + (value).toFixed(this.digitsbehindComma) + this.unit);
	}
	
	getSliderElement(){
		return document.getElementById("" + this.id + "_slider");
	}
}
