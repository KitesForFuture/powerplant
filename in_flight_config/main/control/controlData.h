#ifndef CONTROLS_CONTROL_DATA
#define CONTROLS_CONTROL_DATA

struct _ControlData {
	float left_prop;
	float right_prop;
	float left_elevon;
	float right_elevon;
	float brake;
	float line_tension;
};
typedef struct _ControlData ControlData;

void initControlData(ControlData* controlData, float left_prop, float right_prop, float left_elevon, float right_elevon, float brake, float line_tension);

#endif
