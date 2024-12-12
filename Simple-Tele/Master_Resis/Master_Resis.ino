#include <SimpleFOC.h>
#include <Wire.h>

MagneticSensorSPI sensor = MagneticSensorSPI(AS5048_SPI, 10);
BLDCMotor motor = BLDCMotor(11);
BLDCDriver3PWM driver = BLDCDriver3PWM(6, 5, 3, 4);

const float change_threshold = 0.1;
float last_sent_angle = 0.0;
unsigned long last_debug_time = 0;
float received_current = 0.0;


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
  Wire.begin();
  Serial.begin(115200);
  Wire.setClock(50000); 
  Serial.println("Master setup complete");
}

void loop() {
  sensor.update();
  float precise_angle = sensor.getPreciseAngle();

  if (abs(precise_angle - last_sent_angle) >= change_threshold) {
    Wire.beginTransmission(8);
    Wire.print("A");
    Wire.print(precise_angle, 2);
    Wire.print(";");
    if (Wire.endTransmission() == 0) {
      Serial.print("Sent angle: ");
      Serial.println(precise_angle, 2);
      last_sent_angle = precise_angle;
    } else {
      Serial.println("Error: I2C transmission failed!");
    }
  }

  requestCurrentData();

  if (millis() - last_debug_time > 1000) {
    Serial.print("Current Angle: ");
    Serial.println(precise_angle, 2);
    Serial.print("Received Current: ");
    Serial.println(received_current);
    last_debug_time = millis();
  }
}

void requestCurrentData() {
  Wire.requestFrom(8, sizeof(received_current));
  if (Wire.available() == sizeof(received_current)) {
    Wire.readBytes((byte*)&received_current, sizeof(received_current));
    float resistance_torque = calculateResistance(received_current);
    motor.move(motor.shaft_angle - resistance_torque); // Apply resistance
  }
}

float calculateResistance(float current) {
  return current; //* 0.2; // Adjust resistance calculation factor as needed
}
