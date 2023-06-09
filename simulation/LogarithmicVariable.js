"use strict";

class LogarithmicVariable extends UIElement{
	
	constructor(id, displayName, initialValue, factor, unit){
		super(id);
		this.name = name;
		this.initialValue = initialValue;
		this.currentValue = initialValue;
		this.factor = factor;
		this.displayName = displayName;
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
					<button type=\"button\" id=\"" + this.id + "_plus\" style=\"color: black;\">+</button>\n\
					<button type=\"button\" id=\"" + this.id + "_minus\" style=\"color: black;\">-</button>\n\
				</td>\n\
			</tr>\n\
			";
	}
	
	init(){
		this.setValue(this.initialValue);
		let that = this;
		this.getPlusElement().onclick = function(){
			that.setValue(that.currentValue*that.factor);
			that.oninput(that.currentValue);
		};
		this.getMinusElement().onclick = function(){
			that.setValue(that.currentValue/that.factor);
			that.oninput(that.currentValue);
		};
	}
	
	setValue(value){
		this.currentValue = value;
		this.setString(this.displayName + ": " + (this.currentValue).toFixed(2) + this.unit);
	}
	
	getPlusElement(){
		return document.getElementById("" + this.id + "_plus");
	}
	
	getMinusElement(){
		return document.getElementById("" + this.id + "_minus");
	}
}
