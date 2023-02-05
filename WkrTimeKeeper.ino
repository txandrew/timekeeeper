#include "LedControl.h"
#include "Dictionary.h"
#include <time.h>
#include <string.h>

LedControl lc=LedControl(12,10,11,1);
#define BLUE 3
#define GREEN 5
#define RED 6

// unsigned long delaytime1=5000;
// unsigned long delaytime2=250;

int int_effort;
double int_effort_start;
double int_effort_resume;
double int_session_spent;
double time_session;
int int_running;
Dictionary<int, double> dict_TimeSpent;
Dictionary<int, double> dict_TimeStart;
Dictionary<int, bool> dict_Col_Lights;

//Interval is an hour
double int_interval = 3600000; //3600000 for one hour /  1000 = 1 second;
//Interval is a 8th of a minute
double int_interval2 = 450000;
//Interval is a 15 minute
double int_quarter = 90000;


void setup() {
  /*
   The MAX72XX is in power-saving mode on startup,
   we have to do a wakeup call
   */
  
  Serial.begin(9600);
  lc.shutdown(0,false);
  /* Set the brightness to a medium values */
  lc.setIntensity(0,8);
  /* and clear the display */
  lc.clearDisplay(0);

  pinMode(RED, OUTPUT); 
  pinMode(GREEN, OUTPUT);
  pinMode(BLUE, OUTPUT);
  digitalWrite(RED, HIGH);
  digitalWrite(GREEN, LOW);
  digitalWrite(BLUE, LOW);

  pinMode(7,INPUT_PULLUP);
  pinMode(8,INPUT_PULLUP);
  pinMode(9,INPUT_PULLUP);

  clearBoard();
  time_session = millis();
  int_effort = 0;
  int_effort_start = millis();
  int_running = 1;
}

void loop() {


////////////////////////////////////////////////////
// Check For Button Actioins
////////////////////////////////////////////////////
  
  //Has the "Effort" button been pushed?
  if (digitalRead(7) == LOW) {
    dict_TimeSpent.set(int_effort,millis()-int_effort_start);
    // dict_TimeStart.set(int_effort,NULL);
    int_effort++;
    if (int_effort > 5) { int_effort = 0;} //Reset Loop
    int_effort_start = millis() - dict_TimeSpent.get(int_effort);

    //Wait for UpKey
    while ( digitalRead(7) == LOW) { delay(250);}
  }

  //Has the "Pause/Resume" button been pushed?
  if (digitalRead(8) == LOW) {
    if ( int_running == 1 )
    {
      int_effort_resume = int_effort;
      int_session_spent = millis() - time_session;
      dict_TimeSpent.set(int_effort,millis()-int_effort_start);  
      int_effort = -1;
      int_running = -1;
    }
    else
    {
      int_effort = int_effort_resume;
      time_session = millis() - int_session_spent;
      
      int_effort_start = millis() - dict_TimeSpent.get(int_effort);
      int_running = 1;
      
    }
    //Wait for UpKey
    while ( digitalRead(8) == LOW) { delay(250);}
  }

  //Has the "Reset" button been pushed?
  if (digitalRead(9) == LOW) {

    while ( digitalRead(9) == LOW ) { delay(250);}
    setup();
  }

  
////////////////////////////////////////////////////
// Control the RGB LED
////////////////////////////////////////////////////

  //If paused, blink
  if ( int_effort == -1) {
    if ( millis() % 1000 > 250 ) {
      analogWrite(RED, 255); analogWrite(GREEN,0  ); analogWrite(BLUE,0  );
    } else {
      analogWrite(RED, 0  ); analogWrite(GREEN,0  ); analogWrite(BLUE,0  );
    }
  }

  //Otherwise, set light to effort color
  if ( int_effort == 0) { analogWrite(RED, 255); analogWrite(GREEN,0  ); analogWrite(BLUE,0  );}
  if ( int_effort == 1) { analogWrite(RED,   0); analogWrite(GREEN,255); analogWrite(BLUE,0  );}
  if ( int_effort == 2) { analogWrite(RED,   0); analogWrite(GREEN,0  ); analogWrite(BLUE,255);}
  if ( int_effort == 3) { analogWrite(RED, 255); analogWrite(GREEN,255); analogWrite(BLUE,0  );}
  if ( int_effort == 4) { analogWrite(RED, 255); analogWrite(GREEN,0  ); analogWrite(BLUE,255  );}
  if ( int_effort == 5) { analogWrite(RED,   0); analogWrite(GREEN,255); analogWrite(BLUE,255);}

  
////////////////////////////////////////////////////
// Check the LED Grid
////////////////////////////////////////////////////
  
  //Only Update if Running
  if ( int_running == 1 ) {

    //Effort Time Keeper
    for ( int y = 0; y < 8; y++)
    {
      dict_Col_Lights.set(y,false);
    }
    // For X = 0 to 15, loop as long as X is less than the time spent on effort
    double dbl_effort_elapse = (millis() - int_effort_start) / int_interval;
    for (int x = 0; x < dbl_effort_elapse and x < 16  ; ++x )
    {
      if ( x < dbl_effort_elapse and x < 8) { dict_Col_Lights.set(x,true);}
      if ( x > 7 and x < dbl_effort_elapse) {dict_Col_Lights.set(x-8,false);}
      
      if ( x > dbl_effort_elapse - 1 )
      {
        //Light Blinker
        int int_effort_quarter = floor(fmod(millis() - int_effort_start,int_interval) / (int_interval / 4));
        if ( millis() % ((4-int_effort_quarter) * 1000) > ((4-int_effort_quarter) * 250 )) {
          dict_Col_Lights.set(x,true);
        } else {
          dict_Col_Lights.set(x,false);
        }
      }      
    }
    for ( int y = 0; y < 8; y++)
    {
      lc.setLed(0,int_effort,y,dict_Col_Lights.get(y));
    }


    ////////////////////////////
    //Session Time Keeper
    ////////////////////////////
    double dbl_session_elapse = (millis() - time_session) / int_interval;
    double dbl_session_elapse2 = (millis() - time_session) / int_interval2;
    
    
    //Clear Variable for Light Grid
    for ( int y = 0; y < 8; y++)
    {
      dict_Col_Lights.set(y,false);
    }    
    
    //For x is every hour worked thus far in the session
    for (int x = 0; x <= dbl_session_elapse and x < 16 ; ++x )
    {
      //For each hour worked less than 8 hours then turn light on
      if ( x < dbl_session_elapse and x < 8) { dict_Col_Lights.set(x,true);}
      //For each hour worked over 8 hours, then turn light off
      if ( x > 7 and x < dbl_session_elapse) { dict_Col_Lights.set(x-8,false);}
    }
    
    //Set lights in grid according to the variable
    for ( int y = 0; y < 8; y++)
    {
      lc.setLed(0,6,y,dict_Col_Lights.get(y));
    }
  
    ////////////////////////////
    //Session Time Keeper
    ////////////////////////////
    //Clear Variable for Light Grid
    for ( int y = 0; y < 8; y++)
    {
      dict_Col_Lights.set(y,false);
    }    
    
    //For x is every hour worked thus far in the session
    for (int x = 0; x <= fmod(dbl_session_elapse2, 8) ; ++x )
    {
      //For each hour worked less than 8 hours then turn light on
      if ( x < dbl_session_elapse2 ) { dict_Col_Lights.set(x,true);}
    }
    
    //Set lights in grid according to the variable
    for ( int y = 0; y < 8; y++ )
    {
      lc.setLed(0,7,y,dict_Col_Lights.get(y));
    }
  }
}
    
  

void clearBoard()
{
  for (int x = 0; x < 8; ++x)
  {
    for (int y = 0; y < 8; ++y)
    {
      lc.setLed(0,x,y,false);
    }
  }
}
