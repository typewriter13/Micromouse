#ifndef IR_SENSOR_H
#define IR_SENSOR_H

#define NUM_IR_SENSORS 5

void ir_sensors_init(void);
void ir_sensors_read(int values[NUM_IR_SENSORS]);

#endif
