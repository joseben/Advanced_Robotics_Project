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
  Wire.setClock(50000); // Set I2C clock to 50 kHz for more stability
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
    if (Wire.endTransmission() != 0) {
      Serial.println("Error: I2C transmission failed!");
      i2c_error = true;
      last_retry_time = millis();
    } else {
      Serial.print("Sent angle: ");
      Serial.println(precise_angle, 2);
      last_sent_angle = precise_angle;
      i2c_error = false;
    }
  }


  if (i2c_error) {
    Serial.println("Attempting I2C bus reset...s");
    resetI2CBus();
    i2c_error = false;
  }


  if (millis() - last_debug_time > 1000) {
    Serial.print("Current Angle: ");
    Serial.println(precise_angle, 2);
    last_debug_time = millis();
  }



  delay(100);
}

unsigned long i2c_start_time = millis();



void resetI2CBus() {
  Serial.println("Resetting I2C bus...");
  pinMode(SDA, INPUT_PULLUP);
  pinMode(SCL, INPUT_PULLUP);

  for (int i = 0; i < 9; i++) { // Clock SCL line to recover from stuck state
    pinMode(SCL, OUTPUT);
    digitalWrite(SCL, LOW);
    delayMicroseconds(5);
    digitalWrite(SCL, HIGH);
    delayMicroseconds(5);
  }

  // Generate STOP condition
  pinMode(SDA, OUTPUT);
  digitalWrite(SDA, LOW);
  delayMicroseconds(5);
  digitalWrite(SCL, HIGH);
  delayMicroseconds(5);
  digitalWrite(SDA, HIGH);

  Wire.begin();
  Serial.println("I2C bus reset complete.");
}
