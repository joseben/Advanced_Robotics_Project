#include <SimpleFOC.h>

// Slave hardware setup
MagneticSensorSPI sensor = MagneticSensorSPI(AS5048_SPI, 10); // AS5048 sensor, Chip select pin 10
BLDCMotor motor = BLDCMotor(11); // GM3506 motor with 11 pole pairs
BLDCDriver3PWM driver = BLDCDriver3PWM(6, 5, 3, 4); // PWM pins 6, 5, 3. Enable pin 4.

const float step_increment = 0.001;  // Step size for incremental movement
float target_angle = 0.0;            // Holds the current target angle
float desired_angle = 0.0;           // Holds the desired angle sent from the master

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

  Serial.begin(115200);
  Serial.println("Slave motor ready.");
}

void loop() {
  motor.loopFOC(); // Run FOC algorithm

  // Check if data is available and parse the new desired angle
  if (Serial.available()) {
    float new_angle = Serial.parseFloat();
    if (new_angle != 0.0 || Serial.peek() == '0') { // Account for legitimate "0.0" values
      desired_angle = new_angle; // Update only with valid new data
    }
  }

  // Update target angle in small steps towards the desired angle
  if (abs(desired_angle - target_angle) > step_increment) {
    if (desired_angle > target_angle) {
      target_angle += step_increment; // Incrementally increase towards desired angle
    } else {
      target_angle -= step_increment; // Incrementally decrease towards desired angle
    }
  } else {
    target_angle = desired_angle; // Snap to desired angle when close enough
  }

  // Move motor to the updated target angle
  motor.move(target_angle);

  // Optional: Debugging
  Serial.print("Desired: ");
  Serial.print(desired_angle, 3);
  Serial.print(", Target: ");
  Serial.print(target_angle, 3);
  Serial.print(", Current: ");
  Serial.println(sensor.getPreciseAngle(), 3);

  //delay(10); // Reduce communication bottlenecks
}
