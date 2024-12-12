#include <SimpleFOC.h>
#include <Wire.h>

MagneticSensorSPI sensor = MagneticSensorSPI(AS5048_SPI, 10); // AS5048 sensor, Chip select pin 10
BLDCMotor motor = BLDCMotor(11); // GM3506 motor with 11 pole pairs
BLDCDriver3PWM driver = BLDCDriver3PWM(6, 5, 3, 4); // PWM pins 6, 5, 3. Enable pin 4.

const float step_increment = 0.05; // Step size for incremental movement
float target_angle = 0.0; 
float desired_angle = 0.0; 
String incoming_data = "";
unsigned long last_debug_time = 0;
const unsigned long debug_interval = 1000; // Debug every 1 second

const int POT_PIN = A2;              // Potentiometer connected to analog pin A2
float current_resistance = 0.0;       // Store the current resistance value
unsigned long last_resistance_time = 0;
const unsigned long RESISTANCE_INTERVAL = 50; // Read resistance every 50ms

unsigned long start_time = 0;
float m_velocity = 0;
float position_error = 0;

void setup() {
  SPI.begin();
  pinMode(4, OUTPUT);
  digitalWrite(10, HIGH);
  pinMode(7, OUTPUT);
  digitalWrite(7, LOW);

  pinMode(POT_PIN, INPUT);           // Set potentiometer pin as input

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
  Wire.onRequest(sendData);          // Register request handler
  Serial.begin(115200);
  start_time = millis();
  // Print CSV header
  Serial.println("time,desired_angle,current_angle,m_velocity,position_error,pot_resistance");
  //Serial.println("Slave motor ready.");
}

void loop() {
  motor.loopFOC();
  
  if (abs(desired_angle - target_angle) > step_increment) {
    target_angle += (desired_angle > target_angle) ? step_increment : -step_increment;
  } else {
    target_angle = desired_angle;
  }
  
  motor.move(target_angle);

  if (millis() - last_debug_time > debug_interval) {
    m_velocity = sensor.getVelocity();
    position_error = desired_angle - sensor.getPreciseAngle();
    
    // CSV format output
    Serial.print((millis() - start_time) / 1000.0, 3);
    Serial.print(",");
    Serial.print(desired_angle, 3);
    Serial.print(",");
    Serial.print(sensor.getPreciseAngle(), 3);
    Serial.print(",");
    Serial.print(m_velocity, 3);
    Serial.print(",");
    Serial.print(position_error, 3);
    Serial.print(",");
    Serial.println(current_resistance, 3);
    last_debug_time = millis();
  }
}

void receiveData(int byte_count) {
  while (Wire.available()) {
    char c = Wire.read();
    if (c == ';') {
      if (incoming_data.startsWith("A")) {
        desired_angle = incoming_data.substring(1).toFloat();
        //Serial.print("Received Angle: ");
        //Serial.println(desired_angle, 2);
      }
      incoming_data = "";
    } else {
      incoming_data += c;
    }
  }
}

void sendData() {
  // Read and map potentiometer value (0-1023) to resistance range (0-5.00)
  float resistance = analogRead(POT_PIN) * (5.0 / 1023.0);
  
  // Format and send the resistance data
  String resistance_str = "C" + String(resistance, 2) + ";";
  Wire.write(resistance_str.c_str());
  current_resistance=resistance;
  //Serial.print("Sent resistance: ");
  //Serial.println(resistance_str);
}

float readResistance() {
  // Read and map potentiometer value (0-1023) to resistance range (0-5.00)
  return analogRead(POT_PIN) * (5.0 / 1023.0);
}
