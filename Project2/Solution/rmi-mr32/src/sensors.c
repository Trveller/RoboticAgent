#include <math.h>
#include "sensors.h"
#include "config_constants.h"

SensorReadings sensor_sensorReadings;

void readBeaconSensor();
void readGroundSensor();
void calculate_extra_values();

void sensors_init(){
	enableObstSens();
	enableGroundSens();
	sensor_sensorReadings.last_position_index = -1;
}
void sensors_finish(){
	disableObstSens();
	disableGroundSens();
}

void refresh_sensorReadings(){	
	readAnalogSensors();				// Fill in "analogSensors" structure
	sensor_sensorReadings.obstacleSensor.front = analogSensors.obstSensFront;
	sensor_sensorReadings.obstacleSensor.left = analogSensors.obstSensLeft;
	sensor_sensorReadings.obstacleSensor.right = analogSensors.obstSensRight;
	sensor_sensorReadings.batteryVoltage = analogSensors.batteryVoltage;
	readGroundSensor();	// Read ground sensor
	getRobotPos(&sensor_sensorReadings.positionSensor.x, &sensor_sensorReadings.positionSensor.y, &sensor_sensorReadings.positionSensor.t);  //Read position
	readBeaconSensor();
	
	calculate_extra_values();
	/*
	printf("x=%5.3f, y=%5.3f, t=%5.3f, relAngle=%5.3f , startX=%5.3f, startY=%5.3f \n", 
	sensor_sensorReadings.positionSensor.x, sensor_sensorReadings.positionSensor.y, sensor_sensorReadings.positionSensor.t, 
	sensor_sensorReadings.startingPosition.relative_direction, sensor_sensorReadings.startingPosition.x, sensor_sensorReadings.startingPosition.y );
	//printf("\n");
	
	printf("Obst_left=%03d, Obst_front=%03d, Obst_right=%03d, Bat_voltage=%03d, Ground_sens=%d, Beacon_visible=%d, Ground_sensors=", sensor_sensorReadings.obstacleSensor.left,
					sensor_sensorReadings.obstacleSensor.front, sensor_sensorReadings.obstacleSensor.right, sensor_sensorReadings.batteryVoltage, sensor_sensorReadings.groundSensor,
					sensor_sensorReadings.beaconSensor.isVisible);
	printInt(sensor_sensorReadings.groundSensor, 2 | 5 << 16);	// System call
	printf("\n");	
	
	printf("Beacon_visible=%d, Beacon_direction=%03d", sensor_sensorReadings.beaconSensor.isVisible, sensor_sensorReadings.beaconSensor.direction);
	printf("\n");
	*/
}


void readGroundSensor(){
	sensor_sensorReadings.groundSensor = readLineSensors(0);
	sensor_sensorReadings.atBeaconArea = (sensor_sensorReadings.groundSensor > 0);
}

#define BEACON_SENSOR_SERVO_LIMIT 15
#define BEACON_SENSOR_SERVO_BUFFER 4

int beacon_servo_pos = 0;
int beacon_servo_dir = LEFT;
int beacon_servo_counter = 0;
int beacon_servo_sensor_limit_left = -BEACON_SENSOR_SERVO_LIMIT;
int beacon_servo_sensor_limit_right = BEACON_SENSOR_SERVO_LIMIT;

void readBeaconSensor(){
	if(readBeaconSens()){
		sensor_sensorReadings.beaconSensor.isVisible = true;
		sensor_sensorReadings.beaconSensor.relative_direction = (beacon_servo_pos/15.0*M_PI);//normalizing angle in radians
		beacon_servo_counter = 0;
		beacon_servo_sensor_limit_left = ((beacon_servo_pos - BEACON_SENSOR_SERVO_BUFFER) >= -BEACON_SENSOR_SERVO_LIMIT) ? (beacon_servo_pos - BEACON_SENSOR_SERVO_BUFFER) : -BEACON_SENSOR_SERVO_LIMIT;
		beacon_servo_sensor_limit_right = ((beacon_servo_pos + BEACON_SENSOR_SERVO_BUFFER) <= BEACON_SENSOR_SERVO_LIMIT) ? (beacon_servo_pos + BEACON_SENSOR_SERVO_BUFFER) :  BEACON_SENSOR_SERVO_LIMIT;
	}
	if(sensor_sensorReadings.beaconSensor.isVisible == true && beacon_servo_counter > (BEACON_SENSOR_SERVO_BUFFER*3)){
		beacon_servo_sensor_limit_left = -BEACON_SENSOR_SERVO_LIMIT;
		beacon_servo_sensor_limit_right = BEACON_SENSOR_SERVO_LIMIT;
		sensor_sensorReadings.beaconSensor.isVisible = false;
		beacon_servo_counter = 0;
	}else{
		beacon_servo_counter++;
	}
	if(beacon_servo_pos <= beacon_servo_sensor_limit_left || beacon_servo_pos >= beacon_servo_sensor_limit_right){
		beacon_servo_dir = !beacon_servo_dir;//change direction
	}
	if(beacon_servo_dir == LEFT){
		beacon_servo_pos--;
	}else{
		beacon_servo_pos++;
	}	
	setServoPos(beacon_servo_pos);
}

int calculate_extra_values_counter = 0;
void calculate_extra_values(){
	if(sensor_sensorReadings.startingPosition.x == 0.0 && sensor_sensorReadings.startingPosition.y == 0.0 && sensor_sensorReadings.startingPosition.t == 0.0)
	{
		sensor_sensorReadings.startingPosition.x =  sensor_sensorReadings.positionSensor.x;
		sensor_sensorReadings.startingPosition.y =  sensor_sensorReadings.positionSensor.y;
		sensor_sensorReadings.startingPosition.t =  sensor_sensorReadings.positionSensor.t;
	}
	sensor_sensorReadings.beaconSensor.apsolute_direction = (sensor_sensorReadings.beaconSensor.relative_direction + sensor_sensorReadings.positionSensor.t);
	
	double dx = (sensor_sensorReadings.startingPosition.x - sensor_sensorReadings.positionSensor.x);
	double dy = (sensor_sensorReadings.startingPosition.y - sensor_sensorReadings.positionSensor.y);
	double angle = atan2(dy, dx) - sensor_sensorReadings.positionSensor.t;
	angle = (angle>M_PI) ? angle-2*M_PI : angle;
	angle = (angle<-M_PI) ? angle+2*M_PI : angle;
	sensor_sensorReadings.startingPosition.relative_direction = angle;	
	
	if(calculate_extra_values_counter%50 == 0)
	{
		sensor_sensorReadings.last_position_index++;
		if(sensor_sensorReadings.last_position_index>=MAX_NUM_POSITIONS)
			sensor_sensorReadings.last_position_index=MAX_NUM_POSITIONS-1;
		calculate_extra_values_counter = calculate_extra_values_counter%50;
		sensor_sensorReadings.positionsHistory[sensor_sensorReadings.last_position_index].x = sensor_sensorReadings.positionSensor.x;
		sensor_sensorReadings.positionsHistory[sensor_sensorReadings.last_position_index].y = sensor_sensorReadings.positionSensor.y;
		sensor_sensorReadings.positionsHistory[sensor_sensorReadings.last_position_index].t = sensor_sensorReadings.positionSensor.t;		
	}	
	calculate_extra_values_counter++;
}

SensorReadings get_sensorReadings(){
	return sensor_sensorReadings;
}
SensorReadings get_new_sensorReadings(){
	refresh_sensorReadings();
	return sensor_sensorReadings;
}

