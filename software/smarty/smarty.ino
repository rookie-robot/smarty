/*
 *
 *    This sketch is for the smarty rover
 *
 *    ----------
 *    Copyright (C) 2022 - PRESENT  Zhengyu Peng
 *    Website: https://zpeng.me
 *
 *    `                      `
 *    -:.                  -#:
 *    -//:.              -###:
 *    -////:.          -#####:
 *    -/:.://:.      -###++##:
 *    ..   `://:-  -###+. :##:
 *           `:/+####+.   :##:
 *    .::::::::/+###.     :##:
 *    .////-----+##:    `:###:
 *     `-//:.   :##:  `:###/.
 *       `-//:. :##:`:###/.
 *         `-//:+######/.
 *           `-/+####/.
 *             `+##+.
 *              :##:
 *              :##:
 *              :##:
 *              :##:
 *              :##:
 *               .+:
 *
 */

#include <WiFi.h>
#include <WiFiUdp.h>

// #include <ESP32Servo.h>

/**
 * @brief WiFi parameters
 */
// set WiFi credentials
#ifndef APSSID
#define APSSID "smartyrobot"
#define APPSK "smartyrobot"
#endif
const char *ssid = APSSID;
const char *password = APPSK;

char packetBuffer[4096 + 1];   // buffer to hold incoming packet
unsigned int localPort = 1234; // local port to listen on

WiFiUDP Udp;

const int MID = 0;
const int MIN = -255;
const int MAX = 255;

const int center_var_x = 1890;
const int center_var_y = 1872;
const float scale_x_upper = (float)(MAX - MID) / (4096.0 - (float)center_var_x);
const float scale_x_lower = (float)(MAX - MID) / ((float)center_var_x);
const float scale_y_upper = (float)(MAX - MID) / (4096.0 - (float)center_var_y);
const float scale_y_lower = (float)(MAX - MID) / ((float)center_var_y);

int x_val = center_var_x;
int y_val = center_var_y;

float main_dutycycle = 0;
float offset_dutycycle = 0;

int left_duty;
int right_duty;

/**
 * @brief Motor pins
 */
#define PIN_R1 27 // left motor pin 1
#define PIN_R2 26 // left motor pin 2
#define PIN_L1 33 // right motor pin 1
#define PIN_L2 25 // right motor pin 2

/**
 * @brief PWM channels
 */
#define PWM_R1 0 // left motor pin 1
#define PWM_R2 1 // left motor pin 2
#define PWM_L1 2 // right motor pin 1
#define PWM_L2 3 // right motor pin 2

const int pwmFreq = 5000; // PWM frequency
const int pwmResolution = 8; // PWM resolution

int right_speed = 0;
int left_speed = 0;

bool rover;

int str_idx;
String inputString = "";

/**
 * @brief Set the right motor speed
 * 
 * @param speed motor speed, -255 ~ 255
 */
void set_right_motor(int speed)
{
  if (speed >= 0)
  {
    ledcWrite(PWM_R1, speed);
    ledcWrite(PWM_R2, 0);
  }
  else
  {
    ledcWrite(PWM_R1, 0);
    ledcWrite(PWM_R2, -speed);
  }
}

/**
 * @brief Set the left motor speed
 * 
 * @param speed motor speed, -255 ~ 255
 */
void set_left_motor(int speed)
{
  if (speed >= 0)
  {
    ledcWrite(PWM_L1, speed);
    ledcWrite(PWM_L2, 0);
  }
  else
  {
    ledcWrite(PWM_L1, 0);
    ledcWrite(PWM_L2, -speed);
  }
}

/**
 * @brief Initial setup
 * 
 */
void setup()
{
  Serial.begin(115200);

  WiFi.softAP(ssid, password);

  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);

  Udp.begin(localPort);

  // configure LED PWM functionalitites
  pinMode(PIN_L1, OUTPUT);
  pinMode(PIN_L2, OUTPUT);
  pinMode(PIN_R1, OUTPUT);
  pinMode(PIN_R2, OUTPUT);

  ledcSetup(PWM_L1, pwmFreq, pwmResolution);
  ledcAttachPin(PIN_L1, PWM_L1);

  ledcSetup(PWM_L2, pwmFreq, pwmResolution);
  ledcAttachPin(PIN_L2, PWM_L2);

  ledcSetup(PWM_R1, pwmFreq, pwmResolution);
  ledcAttachPin(PIN_R1, PWM_R1);

  ledcSetup(PWM_R2, pwmFreq, pwmResolution);
  ledcAttachPin(PIN_R2, PWM_R2);

  set_left_motor(0);
  set_right_motor(0);
}

/**
 * @brief Loop
 * 
 */
void loop()
{
  int packetSize = Udp.parsePacket();
  if (packetSize)
  {
    rover = false;
    // read the packet into packetBufffer
    int n = Udp.read(packetBuffer, 4096);
    packetBuffer[n] = 0;

    for (str_idx = 0; str_idx < n; str_idx++)
    {
      char inChar = packetBuffer[str_idx];
      //      Serial.print(inChar);
      //      Serial.print("\n");

      if (inChar != '\n' && inChar != ':')
      {
        // add it to the inputString:
        inputString += inChar;
      }
      else if (inChar == ':')
      {
        if (inputString[0] == 'X')
        {
          int sLength = inputString.length();
          String tempStr = inputString.substring(1, sLength);
          x_val = tempStr.toInt();
          Serial.print('X');
          Serial.print(x_val);
          Serial.print("\n");
          rover = true;
        }
        else if (inputString[0] == 'Y')
        {
          int sLength = inputString.length();
          String tempStr = inputString.substring(1, sLength);
          y_val = tempStr.toInt();
          Serial.print('Y');
          Serial.print(y_val);
          Serial.print("\n");
          rover = true;
        }
        else if (inputString[0] == 'L')
        {
          int sLength = inputString.length();
          String tempStr = inputString.substring(1, sLength);
          left_speed = tempStr.toInt();
          set_left_motor(left_speed);
          // rover = true;
        }
        else if (inputString[0] == 'R')
        {
          int sLength = inputString.length();
          String tempStr = inputString.substring(1, sLength);
          right_speed = tempStr.toInt();
          set_right_motor(right_speed);
          // rover = true;
        }

        inputString = "";
      }
    }
  }

  if (rover)
    {
      if (y_val >= center_var_y)
      {
        main_dutycycle = (float)(center_var_y - y_val) * scale_y_upper;
      }
      else
      {
        main_dutycycle = (float)(center_var_y - y_val) * scale_y_lower;
      }
  
      if (x_val >= center_var_x)
      {
        offset_dutycycle = (float)(center_var_x - x_val) * scale_x_upper;
      }
      else
      {
        offset_dutycycle = (float)(center_var_x - x_val) * scale_x_lower;
      }
  
      left_duty = round(-(main_dutycycle + offset_dutycycle) + MID);
      right_duty = round(-(main_dutycycle - offset_dutycycle) + MID);
  
      if (left_duty >= MIN && left_duty <= MAX)
      {
        set_left_motor(left_duty);
//        servo_l.writeMicroseconds(left_duty);
      }
      else if (left_duty < MIN)
      {
        set_left_motor(MIN);
//        servo_l.writeMicroseconds(MIN);
      }
      else if (left_duty > MAX)
      {
        set_left_motor(MAX);
//        servo_l.writeMicroseconds(MAX);
      }
  
      if (right_duty >= MIN && right_duty <= MAX)
      {
        set_right_motor(right_duty);
//        servo_r.writeMicroseconds(right_duty);
      }
      else if (right_duty < MIN)
      {
        set_right_motor(MIN);
//        servo_r.writeMicroseconds(MIN);
      }
      else if (right_duty > MAX)
      {
        set_right_motor(MAX);
//        servo_r.writeMicroseconds(MAX);
      }
    }
}
