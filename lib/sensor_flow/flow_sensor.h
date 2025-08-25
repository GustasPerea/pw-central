#pragma once
#include <stdint.h>

void flow_sensor_init(void);
float flow_sensor_get_increment(float interval_seconds); // returns liters measured during interval
