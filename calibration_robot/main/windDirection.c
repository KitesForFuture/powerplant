
#define windAverageConstBig		0.9999
#define windAverageConstSmall	0.0001

float windY = 0;
float windZ = 0;

void updateWindDirection(){
	if(rot2 > 0.9){ // kite flies horizontally like a plane
		float inverseNorm = 1.0/sqrt(rot3*rot3 + rot6*rot6); // = sqrt(1-rot2*rot2); // horizontal components of x-axis
		windY = windAverageConstBig*windY - windAverageConstSmall*rot3*inverseNorm;
		windZ = windAverageConstBig*windZ - windAverageConstSmall*rot6*inverseNorm;
	/*
	}else if(rot2 < -0.9){
		float inverseNorm = 1.0/sqrt(rot3*rot3 + rot6*rot6); // = sqrt(1-rot2*rot2);
		windY = windAverageConstBig*windY + windAverageConstSmall*rot3*inverseNorm;
		windZ = windAverageConstBig*windZ + windAverageConstSmall*rot6*inverseNorm;
	*/
	}else{
		float inverseNorm = 1.0/sqrt(rot5*rot5 + rot8*rot8); // = sqrt(1-rot2*rot2); // horizontal components of z-axis
		windY = windAverageConstBig*windY + windAverageConstSmall*rot5*inverseNorm;
		windZ = windAverageConstBig*windZ + windAverageConstSmall*rot8*inverseNorm;
	}
	//TODO normalize!
}
