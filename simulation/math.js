			"use strict";
			
			function clamp(a, min, max){
				return Math.min(Math.max(a, min), max);
			}
			
			function acos_clamp(number_more_or_less_between_one_and_minus_one){
				if(number_more_or_less_between_one_and_minus_one < -1)
					return Math.PI;
				else if(number_more_or_less_between_one_and_minus_one > 1)
					return 0;
				else
					return Math.acos(number_more_or_less_between_one_and_minus_one);
			}
			
			function asin_clamp(number_more_or_less_between_one_and_minus_one){
				if(number_more_or_less_between_one_and_minus_one < -1)
					return - Math.PI * 0.5;
				else if(number_more_or_less_between_one_and_minus_one > 1)
					return Math.PI * 0.5;
				else
					return Math.asin(number_more_or_less_between_one_and_minus_one);
			}
			
			function smallPow(x,a){
				let ret = 1;
				for(let i = 0; i < a; i++){
					ret *= x;
				}
				return ret;
			}
			
			function normalize(a){
				let norm = Math.sqrt(scalarProductOfMatrices(a,a));
				if(norm > 0.000000000000001){
					return scalarMult(1.0/norm, a);
				}else{
					return a;
				}
			}
			
			function vectorMultiply4(a,b){
				return [a[0]*b[0], a[0]*b[1], a[0]*b[2], a[0]*b[3],
						a[1]*b[0], a[1]*b[1], a[1]*b[2], a[1]*b[3],
						a[2]*b[0], a[2]*b[1], a[2]*b[2], a[2]*b[3],
						a[3]*b[0], a[3]*b[1], a[3]*b[2], a[3]*b[3]];
			}
			
			function vectorMultiply(a,b){
				let ret = [];
				for(let i = 0; i < a.length; i++){
					for(let j = 0; j < b.length; j++){
						ret[i*b.length + j] = a[i]*b[j];
					}
				}
				return ret;
			}
			
			function scalarProductOfMatrices(A,B){
				if(A.length != B.length){
					console.error("matrix dimensions don't match up");
				}
				let ret = 0;
				for(var i = 0; i < A.length; i++){
					ret += A[i]*B[i];
				}
				return ret;
			}
			
			function scalarMult(alpha, A){
				let ret = [];
				for(var i = 0; i < A.length; i++){
					ret[i] = alpha*A[i];
				}
				return ret;
			}
			
			function addMatrices(A, B){
				if(A.length != B.length){
					console.error("matrix dimensions don't match up");
				}
				let ret = [];
				for(var i = 0; i < A.length; i++){
					ret[i] = A[i]+B[i];
				}
				return ret;
			}
			
			function subtractMatrices(A, B){
				if(A.length != B.length){
					console.error("matrix dimensions don't match up");
				}
				let ret = [];
				for(var i = 0; i < A.length; i++){
					ret[i] = A[i]-B[i];
				}
				return ret;
			}
