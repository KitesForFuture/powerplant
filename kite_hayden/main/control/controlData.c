#include "controlData.h"

void initControlData(ControlData* controlData, float rudder, float line_tension){
	controlData->rudder = rudder;
	controlData->line_tension = line_tension;
}
