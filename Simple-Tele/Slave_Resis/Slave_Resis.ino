#include <SimpleFOC.h>
#include <Wire.h>

MagneticSensorSPI sensor = MagneticSensorSPI(AS5048_SPI, 10); // AS5048 sensor, Chip select pin 10
BLDCMotor motor = BLDCMotor(11); // GM3506 motor with 11 pole pairs
BLDCDriver3PWM driver = BLDCDriver3PWM(6, 5, 3, 4); // PWM pins 6, 5, 3. Enable pin 4.

const float step_increment = 0.05;
float target_angle = 0.0; 
float desired_angle = 0.0; 
String incoming_data = "";
unsigned long last_debug_time = 0;
const unsigned long debug_interval = 1000;
unsigned long resistance_start_time = 0;
bool resistance_applied = false;
float simulated_current = 5; // Simulated current (A)

void setup() {
  SPI.begin();
  pinMode(4, OUTPUT);
  digitalWrite(10, HIGH);
  pinMode(A2, INPUT);

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

  Wire.begin(8); // Slave address
  Wire.onReceive(receiveData);
  Wire.onRequest(sendData); // Add this to send data back to master
  Serial.begin(115200);
  Serial.println("Slave motor ready.");
  resistance_start_time = millis();
}

void loop() {
  motor.loopFOC();
  
  if (abs(desired_angle - target_angle) > step_increment) {
    target_angle += (desired_angle > target_angle) ? step_increment : -step_increment;
  } else {
    target_angle = desired_angle;
  }

  motor.move(target_angle);

  if (millis() - resistance_start_time >= 10000 && !resistance_applied) {
    Serial.println("Simulated resistance applied, sending current data.");
    resistance_applied = true;
  }

  if (millis() - last_debug_time > debug_interval) {
    Serial.print("Target Angle: ");
    Serial.print(target_angle, 2);
    Serial.print("\t Desired Angle: ");
    Serial.println(desired_angle, 2);
    last_debug_time = millis();
  }
}

void receiveData(int byte_count) {
  while (Wire.available()) {
    char c = Wire.read();
    if (c == ';') {
      if (incoming_data.startsWith("A")) {
        desired_angle = incoming_data.substring(1).toFloat();
        Serial.print("Received Angle: ");
        Serial.println(desired_angle, 2);
      }
      incoming_data = "";
    } else {
      incoming_data += c;
    }
  }
}

void sendData() {
  simulated_current = analogRead(A2);
  if (resistance_applied) {
    Wire.write((byte*)&simulated_current, sizeof(simulated_current));
    Serial.print("Sent Current: ");
    Serial.println(simulated_current);
  }
}
