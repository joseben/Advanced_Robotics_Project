#include <SimpleFOC.h>

// Master hardware setup
MagneticSensorSPI sensor = MagneticSensorSPI(AS5048_SPI, 10); // AS5048 sensor, Chip select pin 10
BLDCMotor motor = BLDCMotor(11); // GM3506 motor with 11 pole pairs
BLDCDriver3PWM driver = BLDCDriver3PWM(6, 5, 3, 4); // PWM pins 6, 5, 3. Enable pin 4.

const float change_threshold = 0.1; // Threshold for angle change
float last_sent_angle = 0.0; // Keep track of the last sent angle

void setup() {
  SPI.begin();
  pinMode(4, OUTPUT);
  digitalWrite(10, HIGH);

  pinMode(7, OUTPUT);
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

  Serial.begin(115200);
  Serial.println("Master motor ready.");
}

void loop() {
  sensor.update();

  float precise_angle = sensor.getPreciseAngle(); // Get precise angle reading

  // Check if the change in angle exceeds the threshold
  if (abs(precise_angle - last_sent_angle) >= change_threshold) {
    Serial.print("A");
    Serial.print(precise_angle, 2); // Send the angle with 2 decimal places
    Serial.println(";");
    last_sent_angle = precise_angle; // Update the last sent angle
  }

  delay(10); // Reduce communication frequency
}
