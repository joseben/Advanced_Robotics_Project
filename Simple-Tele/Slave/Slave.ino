#include <SimpleFOC.h>

// Slave hardware setup
MagneticSensorSPI sensor = MagneticSensorSPI(AS5048_SPI, 10); // AS5048 sensor, Chip select pin 10
BLDCMotor motor = BLDCMotor(11); // GM3506 motor with 11 pole pairs
BLDCDriver3PWM driver = BLDCDriver3PWM(6, 5, 3, 4); // PWM pins 6, 5, 3. Enable pin 4.

// PID control gains
float Kp = 0.1, Ki = 2, Kd = 0; // Adjusted PID gains
float last_error = 0.0, integral = 0.0;

const float pid_threshold = 0.05; // Threshold for PID correction (radians)
const float decimal_precision = 0.01; // For two decimal places

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

  motor.controller = MotionControlType::angle;

  Serial.begin(115200);
  Serial.println("Slave motor ready.");
}

void loop() {
  motor.loopFOC();

  static float desired_angle = 0.0;
  static String input_buffer = ""; // Buffer to hold incoming data

  // Read data from Serial
  while (Serial.available()) {
    char incoming_char = Serial.read();

    if (incoming_char == 'A') {
      input_buffer = ""; // Start marker detected; clear buffer
    } else if (incoming_char == ';') {
      desired_angle = input_buffer.toFloat(); // End marker detected; process data
    } else {
      input_buffer += incoming_char; // Add character to buffer
    }
  }

  // Round current angle and desired angle to two decimal places
  float current_angle = round(sensor.getPreciseAngle() / decimal_precision) * decimal_precision;
  float desired_rounded = round(desired_angle / decimal_precision) * decimal_precision;

  // PID controller
  float error = desired_rounded - current_angle;
  integral += error * 0.01; // Assuming loop runs every 10 ms
  float derivative = (error - last_error) / 0.01;
  float pid_output = Kp * error + Ki * integral + Kd * derivative;

  last_error = error;

// Apply PID correction only if it exceeds the threshold
if (abs(pid_output) >= pid_threshold) {
    // Round PID output to 2 decimal places
    pid_output = round(pid_output / decimal_precision) * decimal_precision;
  Serial.print(", PID Output: ");
  Serial.println(pid_output, 2);

    // Update motor position target
    motor.move(current_angle + pid_output);
} else {
    // Optional: maintain current position if correction is below threshold
    motor.move(current_angle);
}

  // Monitor for debugging (optional)
  Serial.print("Desired: ");
  Serial.print(desired_rounded, 2); // Print rounded desired angle to 2 decimals
  Serial.print(", Current: ");
  Serial.println(current_angle, 2);   // Print rounded current angle to 2 decimals
  //Serial.print(", PID Output: ");
  //Serial.println(pid_output, 2);

  delay(5); // Reduce communication bottlenecks
}
