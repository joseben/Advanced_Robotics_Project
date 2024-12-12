#include <SimpleFOC.h>

// Slave hardware setup
MagneticSensorSPI sensor = MagneticSensorSPI(AS5048_SPI, 10); // AS5048 sensor, Chip select pin 10
BLDCMotor motor = BLDCMotor(11); // GM3506 motor with 11 pole pairs
BLDCDriver3PWM driver = BLDCDriver3PWM(6, 5, 3, 4); // PWM pins 6, 5, 3. Enable pin 4.

const float movement_threshold = 0; // Threshold to avoid small oscillations
float desired_angle = 0.0;             // Start angle
float current_angle = 0.0;             // Holds the current angle
float increment = 0.01;                // Increment per update (adjust for slowness)
const float end_angle = 25.0;

unsigned long start_time = 0;         // Start time of the program
bool motor_active = false;            // Indicates when the motor should start moving

void setup() {
  SPI.begin();
  pinMode(4, OUTPUT);
  digitalWrite(10, HIGH);

  pinMode(7, OUTPUT); // Ground pin for SimpleFOC board
  digitalWrite(7, LOW);

  sensor.init();
  motor.linkSensor(&sensor);

  driver.voltage_power_supply = 12;
  driver.init();
  motor.linkDriver(&driver);

  motor.voltage_sensor_align = 3;
  motor.zero_electric_angle = 1.43;
  motor.sensor_direction = Direction::CW;

  motor.init();
  motor.initFOC();

  motor.controller = MotionControlType::angle; // Set motor to angle control mode

  start_time = millis(); // Record the start time
  Serial.begin(115200);
  Serial.println("Slave motor ready.");
}

void loop() {
  motor.loopFOC(); // Run FOC algorithm

  // Check if 5 seconds have passed
  if (!motor_active && millis() - start_time >= 10000) {
    motor_active = true; // Enable motor movement after 5 seconds
  }

  if (motor_active) {
    // Gradually increase the angle from start_angle to end_angle
    if (desired_angle < end_angle) {
      desired_angle += increment; // Increment angle
    }

    else if (desired_angle > end_angle) {
      desired_angle -= increment; // Increment angle
    }

    // Get the current motor position
    current_angle = sensor.getPreciseAngle();

    // Move motor only if the correction exceeds the threshold
    //if (abs(desired_angle - current_angle) >= movement_threshold) {
      motor.move(desired_angle); // Directly move to the desired angle
    //}
  }

  // Debugging
  Serial.print("Desired: ");
  Serial.print(desired_angle, 2);
  Serial.print(", Current: ");
  Serial.println(current_angle, 2);
}
