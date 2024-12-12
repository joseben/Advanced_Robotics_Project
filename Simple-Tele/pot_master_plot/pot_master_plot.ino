#include <SimpleFOC.h>
#include <Wire.h>

MagneticSensorSPI sensor = MagneticSensorSPI(AS5048_SPI, 10);
BLDCMotor motor = BLDCMotor(11);
BLDCDriver3PWM driver = BLDCDriver3PWM(6, 5, 3, 4);

const float change_threshold = 0.1;
float last_sent_angle = 0.0;
unsigned long last_debug_time = 0;
unsigned long retry_interval = 500; // Retry every 500 ms on error
unsigned long last_retry_time = 0;
bool i2c_error = false;

String resistance_buffer = "";
float received_resistance = 0.0;
const float RESISTANCE_FACTOR = 0.5;  // Adjust this to change resistance strength

const float MAX_VOLTAGE = 6.0;  // Maximum voltage to apply
const float MIN_VOLTAGE = 0.0;  // Minimum voltage
const float VOLTAGE_MULTIPLIER = 1.0; // Adjust this to change resistance intensity

float previous_voltage = 0;
const float SMOOTHING_FACTOR = 0.1;  // Adjust 0.05-0.2 for different smoothing levels
const float VELOCITY_THRESHOLD = 0.2; // Increased threshold
const float MAX_VOLTAGE_CHANGE = 0.5; // Maximum allowed voltage change per update

unsigned long start_time = 0;
float applied_voltage = 0;

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
  
  motor.controller = MotionControlType::torque; // Change to torque control
  motor.voltage_limit = MAX_VOLTAGE; // Set voltage limit for safety
  
  Wire.begin();
  Serial.begin(115200);
  Wire.setClock(50000); // Set I2C clock to 50 kHz for more stability
  //Serial.println("Master setup complete");
  start_time = millis();
  // Print CSV header
  Serial.println("time,master_angle,received_resistance,applied_voltage");
}

void loop() {
  motor.loopFOC(); // Important for torque control
  
  sensor.update();
  float precise_angle = sensor.getPreciseAngle();

  if (abs(precise_angle - last_sent_angle) >= change_threshold) {
    Wire.beginTransmission(8);
    Wire.print("A");
    Wire.print(precise_angle, 2);
    Wire.print(";");
    if (Wire.endTransmission() != 0) {
      Serial.println("Error: I2C transmission failed!");
      i2c_error = true;
      last_retry_time = millis();
    } else {
      //Serial.print("Sent angle: ");
      //Serial.println(precise_angle, 2);
      last_sent_angle = precise_angle;
      i2c_error = false;
    }
  }


    if (i2c_error && millis() - last_retry_time > retry_interval) {
        //Serial.println("Retrying I2C transmission...");
        last_retry_time = millis();
        i2c_error = false;
    }
    
  if (millis() - last_debug_time > 1000) {
    //Serial.print("Current Angle: ");
    //Serial.println(precise_angle, 2);
    last_debug_time = millis();
  }

  // Request resistance data from slave
  requestResistanceData();

    delay(100);

    Serial.print((millis() - start_time) / 1000.0, 3);
    Serial.print(",");
    Serial.print(sensor.getPreciseAngle(), 3);
    Serial.print(",");
    Serial.print(received_resistance, 3);
    Serial.print(",");
    Serial.println(previous_voltage, 3);
}

unsigned long i2c_start_time = millis();

void requestResistanceData() {
  Wire.requestFrom(8, 32);  // Request up to 32 bytes from slave
  
  resistance_buffer = "";
  while (Wire.available()) {
    char c = Wire.read();
    if (c == ';') {
      if (resistance_buffer.startsWith("C")) {
        received_resistance = resistance_buffer.substring(1).toFloat();
        //Serial.print("Resistance: ");
        //Serial.println(received_resistance, 2);
        
        applyResistance(received_resistance);
      }
      resistance_buffer = "";
    } else {
      resistance_buffer += c;
    }
  }
}

void applyResistance(float resistance) {
  // Map resistance to voltage range
  float target_voltage = resistance * VOLTAGE_MULTIPLIER;
  target_voltage = constrain(target_voltage, MIN_VOLTAGE, MAX_VOLTAGE);
  
  // Get current velocity with averaging
  float velocity = sensor.getVelocity();
  
  // Smooth voltage transitions
  float voltage_change = target_voltage - previous_voltage;
  voltage_change = constrain(voltage_change, -MAX_VOLTAGE_CHANGE, MAX_VOLTAGE_CHANGE);
  float smoothed_voltage = previous_voltage + (voltage_change * SMOOTHING_FACTOR);
  
  if (abs(velocity) > VELOCITY_THRESHOLD) {
    float direction = velocity > 0 ? -1 : 1;
    motor.move(smoothed_voltage * direction);
  } else {
    // Gradual force reduction when nearly stopped
    motor.move(smoothed_voltage * 0.5);
  }
  
  previous_voltage = smoothed_voltage;
}
