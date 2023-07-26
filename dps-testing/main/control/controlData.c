#include "controlData.h"

void initControlData(ControlData* controlData, float left_prop, float right_prop, float left_elevon, float right_elevon, float brake, float rudder, float line_tension){
	controlData->left_prop = left_prop;
	controlData->right_prop = right_prop;
	controlData->left_elevon = left_elevon;
	controlData->right_elevon = right_elevon;
	controlData->brake = brake;
	controlData->rudder = rudder;
	controlData->line_tension = line_tension;
}
