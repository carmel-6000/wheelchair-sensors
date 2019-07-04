
#include <NewPing.h> // For Ultrasonic library
#include "LedControl.h"

///////////////////////////////////////////////////////
//                 DEFINITIONS                       //
///////////////////////////////////////////////////////
// I/O Pins
#define TRIGGER_PIN_A            2
#define ECHO_PIN_A               3
#define TRIGGER_PIN_B            4
#define ECHO_PIN_B               5
#define TRIGGER_PIN_C            6
#define ECHO_PIN_C               7
#define TRIGGER_PIN_D            8
#define ECHO_PIN_D               9
#define LIGHT                    A5

//ledbar:
//DIN 12
//CS  10
//CLK 11



// General
#define MAX_DISTANCE           400  // [cm]
#define DISTANCE_LEVEL_A        50//50  // [cm]
#define DISTANCE_LEVEL_B        40  // [cm]
#define DISTANCE_LEVEL_C        30  // [cm]
#define DISTANCE_LEVEL_D        20  // [cm]
#define DISTANCE_LEVEL_E        20  // [cm]
#define DISPLAY_DIST_GAP         5  // [cm]
#define SERIAL_COM_RATE     115200
#define SONAR_SAMPLE_TIME       50 // [mSec]
#define SONAR_NOF_SAMPLES        3
#define MODE_LEVEL_DEF           5
#define MODE_LEVEL_A             4
#define MODE_LEVEL_B             3
#define MODE_LEVEL_C             2
#define MODE_LEVEL_D             1
#define MODE_LEVEL_E             0


///////////////////////////////////////////////////////
//           GLOBAL VARIABLES DEFINITION             //
///////////////////////////////////////////////////////
NewPing sonar_a(TRIGGER_PIN_A, ECHO_PIN_A, MAX_DISTANCE);
NewPing sonar_b(TRIGGER_PIN_B, ECHO_PIN_B, MAX_DISTANCE);
NewPing sonar_c(TRIGGER_PIN_C, ECHO_PIN_C, MAX_DISTANCE);
NewPing sonar_d(TRIGGER_PIN_D, ECHO_PIN_D, MAX_DISTANCE);

LedControl lc = LedControl(12, 11, 10, 1);


int cycle_cnt;
int sample_id;
int dist_a[SONAR_NOF_SAMPLES];
int dist_a_avg;
int dist_b[SONAR_NOF_SAMPLES];
int dist_b_avg;
int dist_c[SONAR_NOF_SAMPLES];
int dist_c_avg;
int dist_d[SONAR_NOF_SAMPLES];
int dist_d_avg;
int dist_min;
int display_number;
int remainder;
int upper_display_number;
int lower_display_number;
int mode_level;
unsigned long CurTime;
int DispalyEnableFlag;
int RightSideOutOfRangeFlag,
    LeftSideOutOfRangeFlag;
bool ShowPowerUpDisplay = true;
int dist_min_r = 0;
int dist_min_l = 0;

///////////////////////////////////////////////////////
//                SETUP()                            //
///////////////////////////////////////////////////////
void setup()
{
  Serial.begin(SERIAL_COM_RATE);

  //wake up the MAX72XX from power-saving mode
  lc.shutdown(0, false);
  //set a medium brightness for the Leds
  lc.setIntensity(0, 8);

  cycle_cnt = 0;
  DispalyEnableFlag = 1;
  pinMode(LIGHT, OUTPUT);
}


///////////////////////////////////////////////////////
//                 LOOP()                            //
///////////////////////////////////////////////////////
void loop()
{
  // Variable Initialization
  sample_id = (cycle_cnt % SONAR_NOF_SAMPLES);
  dist_a[sample_id] = sonar_a.ping_cm();
  dist_b[sample_id] = sonar_b.ping_cm();
  dist_c[sample_id] = sonar_c.ping_cm();
  dist_d[sample_id] = sonar_d.ping_cm();

  if ( ShowPowerUpDisplay == true )
  {
    for (int index = 0; index < 8; index++)
    {
      lc.clearDisplay(0);
      lc.setRow(0, index, 255);
      lc.setColumn(0, index, 255);
      delay(500);
    }
    digitalWrite(LIGHT, 0);
    ShowPowerUpDisplay = false;
  }
  else
  {


    // Check for Out of Range - Right & Left Sides
    if ( (dist_a[sample_id] == 0) && (dist_b[sample_id] == 0) )
    {
      RightSideOutOfRangeFlag = 1;
      Serial.println("####### Right Side - Out Of Range ######");
    }
    else
    {
      RightSideOutOfRangeFlag = 0;
    }

    if ( (dist_c[sample_id] == 0) && (dist_d[sample_id] == 0) )
    {
      LeftSideOutOfRangeFlag = 1;
      Serial.println("####### Left Side - Out Of Range ######");
    }
    else
    {
      LeftSideOutOfRangeFlag = 0;
    }

    // Calculate average distance samples and call dispaly and beep functions
    if (sample_id == 0 )
    {
      // ------------------------------------
      // Right Sensor Data & Display handler
      // ------------------------------------
      dist_a_avg = (dist_a[0] + dist_a[1] + dist_a[2]) / 3;
      dist_b_avg =  (dist_b[0] + dist_b[1] + dist_b[2]) / 3;
      dist_min = min(dist_a_avg, dist_b_avg);
      if (dist_min < 35) {
        dist_min_r = 1;
      }
      else {
        dist_min_r = 0;
      }
      if (dist_min_r == 1 || dist_min_l == 1) {
        digitalWrite(LIGHT, 1);
      }
      else {
        digitalWrite(LIGHT, 0);
      }

      // Calculate display number
      remainder = dist_min % DISPLAY_DIST_GAP;
      lower_display_number = dist_min - remainder;
      upper_display_number = lower_display_number + DISPLAY_DIST_GAP;
      if (remainder <= (DISPLAY_DIST_GAP / 2 ) )
      {
        display_number = lower_display_number;
      }
      else
      {
        display_number = upper_display_number;
      }

      // Set level mode
      mode_level = MODE_LEVEL_DEF;
      if ( display_number < DISTANCE_LEVEL_E )
      {
        mode_level = MODE_LEVEL_E;
      }
      else if ( display_number < DISTANCE_LEVEL_D )
      {
        mode_level = MODE_LEVEL_D;
      }
      else if ( display_number < DISTANCE_LEVEL_C )
      {
        mode_level = MODE_LEVEL_C;
      }
      else if ( display_number < DISTANCE_LEVEL_B )
      {
        mode_level = MODE_LEVEL_B;
      }
      else if ( display_number < DISTANCE_LEVEL_A )
      {
        mode_level = MODE_LEVEL_A;
      }

      if ( DispalyEnableFlag == 0  || RightSideOutOfRangeFlag == 1 )
      {
        mode_level = MODE_LEVEL_DEF;
      }

      // Print to serial port
      Serial.print(dist_a_avg);
      Serial.println(" a cm");
      Serial.print(dist_b_avg);
      Serial.println(" b cm");
      Serial.println("----------");
      Serial.print(dist_min);
      Serial.println(" cm min");
      Serial.println("----------");
      Serial.print("Display Number: ");
      Serial.println(display_number);
      Serial.println("----------");
      Serial.print("Mode: ");
      Serial.println(mode_level);
      Serial.println("----------");

      // Set Display LED behavior according to mode level
      switch (mode_level)
      {
        case MODE_LEVEL_A:
          lc.setColumn(0, 7, 255);
          lc.setColumn(0, 6, 0);
          lc.setColumn(0, 5, 0);
          lc.setColumn(0, 4, 0);
          break;

        case MODE_LEVEL_B:
          lc.setColumn(0, 7, 255);
          lc.setColumn(0, 6, 255);
          lc.setColumn(0, 5, 0);
          lc.setColumn(0, 4, 0);
          break;

        case MODE_LEVEL_C:
          lc.setColumn(0, 7, 255);
          lc.setColumn(0, 6, 255);
          lc.setColumn(0, 5, 255);
          lc.setColumn(0, 4, 0);
          break;

        case MODE_LEVEL_D:
          lc.setColumn(0, 7, 255);
          lc.setColumn(0, 6, 255);
          lc.setColumn(0, 5, 255);
          lc.setColumn(0, 4, 255);
          break;

        case MODE_LEVEL_E:
          lc.setColumn(0, 7, 255);
          lc.setColumn(0, 6, 255);
          lc.setColumn(0, 5, 255);
          lc.setColumn(0, 4, 255);
          break;

        default:
          lc.setColumn(0, 7, 0);
          lc.setColumn(0, 6, 0);
          lc.setColumn(0, 5, 0);
          lc.setColumn(0, 4, 0);
          break;
      } // switch (mode_level)

      // ------------------------------------
      // Left Sensor Data & Display handler
      // ------------------------------------
      dist_c_avg =  (dist_c[0] + dist_c[1] + dist_c[2]) / 3;
      dist_d_avg =  (dist_d[0] + dist_d[1] + dist_d[2]) / 3;
      dist_min = min(dist_c_avg, dist_d_avg);

      if (dist_min < 35) {
        dist_min_l = 1;
      }
      else {
        dist_min_l = 0;
      }
      if (dist_min_r == 1 || dist_min_l == 1) {
        digitalWrite(LIGHT, 1);
      }
      else {
        digitalWrite(LIGHT, 0);
      }

      // Calculate display number
      remainder = dist_min % DISPLAY_DIST_GAP;
      lower_display_number = dist_min - remainder;
      upper_display_number = lower_display_number + DISPLAY_DIST_GAP;
      if (remainder <= (DISPLAY_DIST_GAP / 2 ) )
      {
        display_number = lower_display_number;
      }
      else
      {
        display_number = upper_display_number;
      }

      // Set level mode
      mode_level = MODE_LEVEL_DEF;
      if ( display_number < DISTANCE_LEVEL_E )
      {
        mode_level = MODE_LEVEL_E;
      }
      else if ( display_number < DISTANCE_LEVEL_D )
      {
        mode_level = MODE_LEVEL_D;
      }
      else if ( display_number < DISTANCE_LEVEL_C )
      {
        mode_level = MODE_LEVEL_C;
      }
      else if ( display_number < DISTANCE_LEVEL_B )
      {
        mode_level = MODE_LEVEL_B;
      }
      else if ( display_number < DISTANCE_LEVEL_A )
      {
        mode_level = MODE_LEVEL_A;
      }

      if ( DispalyEnableFlag == 0 || LeftSideOutOfRangeFlag == 1)
      {
        mode_level = MODE_LEVEL_DEF;
      }

      // Print to serial port
      Serial.print(dist_c_avg);
      Serial.println(" c cm");
      Serial.print(dist_d_avg);
      Serial.println(" d cm");
      Serial.println("----------");
      Serial.print(dist_min);
      Serial.println(" cm min");
      Serial.println("----------");
      Serial.print("Display Number: ");
      Serial.println(display_number);
      Serial.println("----------");
      Serial.print("Mode: ");
      Serial.println(mode_level);
      Serial.println("----------");

      // Set Display LED behavior according to mode level
      switch (mode_level)
      {
        case MODE_LEVEL_A:
          lc.setColumn(0, 0, 255);
          lc.setColumn(0, 1, 0);
          lc.setColumn(0, 2, 0);
          lc.setColumn(0, 3, 0);
          break;

        case MODE_LEVEL_B:
          lc.setColumn(0, 0, 255);
          lc.setColumn(0, 1, 255);
          lc.setColumn(0, 2, 0);
          lc.setColumn(0, 3, 0);
          break;

        case MODE_LEVEL_C:
          lc.setColumn(0, 0, 255);
          lc.setColumn(0, 1, 255);
          lc.setColumn(0, 2, 255);
          lc.setColumn(0, 3, 0);
          break;

        case MODE_LEVEL_D:
          lc.setColumn(0, 0, 255);
          lc.setColumn(0, 1, 255);
          lc.setColumn(0, 2, 255);
          lc.setColumn(0, 3, 255);
          break;

        case MODE_LEVEL_E:
          lc.setColumn(0, 0, 255);
          lc.setColumn(0, 1, 255);
          lc.setColumn(0, 2, 255);
          lc.setColumn(0, 3, 255);
          break;

        default:
          lc.setColumn(0, 0, 0);
          lc.setColumn(0, 1, 0);
          lc.setColumn(0, 2, 0);
          lc.setColumn(0, 3, 0);
          break;
      } // switch (mode_level)
    } // if (sample_id == 0 )

    delay(SONAR_SAMPLE_TIME);
    cycle_cnt++;
  }
} // void loop()
