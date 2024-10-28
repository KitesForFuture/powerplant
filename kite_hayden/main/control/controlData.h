#ifndef CONTROLS_CONTROL_DATA
#define CONTROLS_CONTROL_DATA

struct _ControlData {
	float rudder;
	float line_tension;
};
typedef struct _ControlData ControlData;

void initControlData(ControlData* controlData, float rudder, float line_tension);

#endif
