//===============================INCLUDE LIBRARIES==============================
#include "Timer.h"  
#include <SoftwareSerial.h>
//================================DECLARE VARIABLES=============================
Timer t, t1, t2; //Timers that will be used for launching functions at specific intervals
int once;//when a breach is detected send only one SMS
String trustedNo = "+40747160083";//This is the phone number that the system will send or receive SMS from
String password = "1234567890";//Password for ARM and DISARM
int stare = 0;//State of the system
//================================BUTON=========================================
 int buttonState = 0;
#define buttonPin 10 
//====================================MAGNETIC contact==========================
//contact magnetic
const int contact = 9;     // the number of the pushbutton pin
// variables will change:
int contactState = 0;         // variable for reading the pushbutton status 
//==================================IR LASER====================================
// set pin numbers:
const int laser = 13;     // the number of the pushbutton pin
// variables will change:
int laserState = 0;         // variable for reading the pushbutton status
//==============================a-gsm variables=================================
#define powerPIN     7//Arduino Digital pin used to power up / power down the modem
#define resetPIN     6//Arduino Digital pin used to reset the modem 
#define statusPIN    5//Arduino Digital pin used to monitor if modem is powered 
int state = 0, i = 0, powerState = 0, ready4SMS = 0, ready4Voice = 0;
#define BUFFDSIZE 240
SoftwareSerial SSerial(2, 3);  //RX==>2 ,TX soft==>3
char ch;
char buffd[BUFFDSIZE];
char o_buffd[BUFFDSIZE];
int noSMS = 0, totSMS = 0;
char readBuffer[200];
///==============================LED-RED-GREEN==================================
#define READY 11
#define ARMED 12
int count, n = 0;//Various variables
void setup() {
//================================DEFINE IN-OUT-PORTS===========================
//=======================================RED-BUTTON=============================
  pinMode(buttonPin, INPUT); 
//=================================MAGNETIC CONTACT=============================
   pinMode(contact, INPUT);
   digitalWrite(contact, HIGH); // turn on input pin's pull-up resistor 
//===============================LED-RED-GREEN==================================
    pinMode(READY, OUTPUT);
    pinMode(ARMED, OUTPUT);
//=====================================IR-LASER=================================
   pinMode(laser, INPUT);
  digitalWrite(laser, LOW); // turn on input pin's pull-up resistor 
//Timers initialization
  t.every(250, alarmSMS);
  t1.every(50, lightEffect);
  t2.every(100, sensor);
//============================= a-gsm setup=====================================
  SSerial.begin(9600);
  Serial.begin(57600);//1ms
  clearSSerial();
  clearSerial();
  delay(10);
  pinMode(resetPIN, OUTPUT);
  digitalWrite(resetPIN, LOW);
  pinMode(powerPIN, OUTPUT);
  pinMode(statusPIN, INPUT);
  digitalWrite(powerPIN, LOW);
  delay(100);
  Serial.flush();
  SSerial.flush();
  if (!digitalRead(statusPIN)) restartMODEM();
  clearBUFFD();
  ready4SMS = 0;
  ready4Voice = 0;
   Serial.println("SMSAlarm ready");//PUT ON SERIAL THE MESSAGE
//==============================================================================
}

void loop() {
  t.update();//Check for SMS (larmSMS function)
  t1.update();//Light(LED) effect(lightEffect function)
  t2.update();//Check sensor status(sensor function)
}

//This function is responsable to check for SMS and change the state of the system
void alarmSMS() {
//============VERIFY IF RED BUTTON IS PRESSED===================================
	if(buttonState==HIGH)//RED BUTTON IS PRESSED
  {
      if (stare == 0) {/IF THE SYSTEM IS IN READY STATE AND ARIVED A NEW SMS-PUT SYSTEM IN ARMED STATE
        stare = 1;//SET SYSTEM STATE TO ARMED
       Serial.println("ARMED");//PUT ON SERIAL THE MESSAGE
        sendSMS("+40755936728", "ARMED");//SEND SMS
       
  
      }
      else if (stare == 1) {//IF SYSTEM IS IN ARMED STATE AND ARIVED A NEW SMS-PUT SYSTEM IN READY STATE
        stare = 0;//SET SYSTEM STATE TO READY
        Serial.println("DISARMED");//PUT ON SERIAL THE MESSAGE
        sendSMS("+40755936728", "DISARMED");//SEND SMS
      }
      else if (stare == 2) {//IF SYSTEM IS IN ALARM STATE AND ARIVED A NEW SMS-PUT SYSTEM IN ARMED STATE
        stare = 1;//SET SYSTEM STATE TO ARMED
        Serial.println("ALARM CANCEL BUT STILL ARMED");//PUT ON SERIAL THE MESSAGE
        sendSMS("+40755936728", "ALARM CANCEL BUT STILL ARMED"); //SEND SMS       
      }

  }
//==================VERIFY IF NEW SMS IS ARIVED=================================

  listSMS();
  int cnt;
  cnt = noSMS;
  while (cnt > 0) {//VERIFY IF IS A NEW SMS
    readSMS(cnt);

    if ((trustedNo == getValue(buffd, ',', 1).substring(1, 13)) && (password == getValue(buffd, ',', 4))) {//VERIFY IF IS THE TRUSTED NUMBER AND PASSWORD
      if((contactState==HIGH)||(laserState == LOW))//VERIFY IF ANY CONTACT IS IN MODE (NOT NORMAL)
      {
         sendSMS("+40747160083", "One of sensor is Opened.Can not Arm!");//SEND SMS
      }
      else
      {
      if (stare == 0) {//IF THE SYSTEM IS IN READY STATE AND ARIVED A NEW SMS-PUT SYSTEM IN ARMED STATE
        stare = 1;//SET SYSTEM STATE TO ARMED
        Serial.println("ARMED");//PUT ON SERIAL THE MESSAGE
        sendSMS("+40747160083", "ARMED");//SEND SMS
      }
      else if (stare == 1) {//IF SYSTEM IS IN ARMED STATE AND ARIVED A NEW SMS-PUT SYSTEM IN READY STATE
        stare = 0;//SET SYSTEM STATE TO READY
        Serial.println("DISARMED");//PUT ON SERIAL THE MESSAGE
        sendSMS("+40747160083", "DISARMED");//SEND SMS
      }
      else if (stare == 2) {//IF SYSTEM IS IN ALARM STATE AND ARIVED A NEW SMS-PUT SYSTEM IN ARMED STATE
        stare = 1;//SET SYSTEM STATE TO ARMED
        Serial.println("ALARM CANCEL BUT STILL ARMED");//PUT ON SERIAL THE MESSAGE
        sendSMS("+40747160083", "ALARM CANCEL BUT STILL ARMED");  //SEND SMS      
      }

    }
    

    }
    deleteSMS(cnt);
    clearBUFFD();
    clearSSerial();
    cnt--;
  }
}
//Function responsable for the LED effects
void lightEffect() {
  if (stare == 0) {//IF SYSTEM IS IN READY STATE-PUT ON GREEN LED
   
      digitalWrite(READY, HIGH);//GREEN LIGTH WILL BE ON
      digitalWrite(ARMED, LOW);//RED LIGTH WILL BE OFF
   
  }
  else if (stare == 1)//IF SYSTEM IS IN ARM STATE-PUT ON RED LED
  { once = 0;
    
      digitalWrite(READY, LOW);//RED LIGTH WILL BE OFF
      digitalWrite(ARMED, HIGH);//RED LIGTH WILL BE ON
      
  }

  else if (stare == 2)//IF SYSTEM STATE IS ALARM-PUT ON BOTH OF LEDS
  {
      digitalWrite(READY, HIGH);//RED LIGTH WILL BE ON
      digitalWrite(ARMED, HIGH);//RED LIGTH WILL BE ON
      
           
  }
}
//Function that is continuously checking the sensor value and in case of trigger it will send an alert
void sensor()
{
//===================================IR-LASER===================================
  laserState = digitalRead(laser);
  if ((stare==1)&&(laserState == LOW)){//IF THE SYSTEM IS ARMED AND THE NOMAL STATE OF LASER CHANGED , PUT SYSTEM IN ALARM STATE
    stare = 2;//PUT SYSTEM IN ALARM STATE
    if (once == 0){//PUT SYSTEM IN ALARM STATE
      Serial.println("DOOR LASER DETECT PERSON!");//PUT ON SERIAL THE MESSAGE
      sendSMS("+40747160083", "DOOR LASER DETECT PERSON!");//SEND SMS
      t1.update();//Light effect//CHANGE LEDS FOR ALARM STATE(RED AND GREEN ON)
      once = 1;//ONE MESSAGE WAS SEND
    }
}
//================================MAGNETIC CONTACT===============================
  contactState = digitalRead(contact);
  if((stare==1)&&(contactState==HIGH))//IF THE SYSTEM IS ARMED AND THE NOMAL STATE OF CONTACT CHANGED , PUT SYSTEM IN ALARM STATE
         { stare = 2;//PUT SYSTEM IN ALARM STATE
          if (once == 0) {//PUT SYSTEM IN ALARM STATE
           Serial.println("WINDOW IS OPENED!");//PUT ON SERIAL THE MESSAGE
            sendSMS("+40747160083", "WINDOW IS OPENED!");//SEND SMS
      t1.update();//Light effect/Light effect//CHANGE LEDS FOR ALARM STATE(RED AND GREEN ON)
      once = 1;//ONE MESSAGE WAS SEND
    }
  
}
}
