/* Note: this is for Arduino Yun only !  */
/*                                       */
/* This is for setting the wind direction and speed
/* on my Cape Code (R) brand windspeed and direction
/* analog dials. A set of 8 digital relays drives the
/* direction indicators, which are simply neon NE-2 
/* lamps in a compass rose. These are powered by 
/* household current gated through the relays.  The 
/* windspeed indicator is wired through a 10k(?) 
/* resistor to pin 10, and then the analog PWM function
/* of the Arduino pulses quickly enough to look like
/* a steady current.  I put the output of the resistor
/* directly to the wires leading out of the enclosure
/* and not through the capacitors and other stuff that 
/* typically filter/reduce power to the dial.
/* 
/* The Yun kind of sucks as a web client, and this uses
/* the Yun stack to ingest HTTP.  I then chop up the
/* string and get the "direction" and "speed" values 
/* to feed to the subroutines.  I can parse either 
/* numeric values for compass direction of the wind, or
/* letter values (N, NW, E, etc.) 
/*
/* It's not a very error-resistant system, so treat it
/* gently. Use only on a protected network.


/* jtodd@loligo.com 2015 */

#include <Bridge.h>
#include <YunServer.h>
#include <YunClient.h>

/* Comment following line of code if serial debug is not required */
#define DEBUG

/* Use this for storing the millisecond counter for when we last got an update */
unsigned long lastTime;
unsigned long countup;

/* Pin is used for which pin is active, indicating what light is lit */
static int pin;

/* Windspeed is what the current windspeed is, in mph */
static int windspeed;


/* EthernetServer server(80); */
YunServer server;


/* This function collects data from the port 80 request, and hands back a reply */
void process(YunClient client) {
  // read the command
  String GETBuffer = client.readString();  
  {
    client.println(F("200 - OK"));
    /*
    client.println(F("You sent a command - valid or invalid - no error checking here, folks!"));
    client.println(F(" Format: http://x.y.z.a/arduino/direction=<compass>&speed=<speed-in-mph>"));
    client.println(F(" Compass is one of: N, NE, E, SE, S, SW, W, NW"));
    client.println(F(" Speed is a non-decimal positive integer 0-100"));
    client.println(F(" Commands need to be sent every 60 seconds or N will blink as an error indicator."));
    client.println(F(" Not specifying one or the other will leave previous setting intact."));
    client.println(F(" (c)2015 John Todd jtodd@loligo.com")); */
    client.stop();
    /* now that we have the value, parse it out with update_values function */
    update_values(GETBuffer);
    /* and after we've parsed the data, go and change the lights and dial using display_values */
    display_values();
  }

}


  

void update_values(String GETBuffer) //get http and parse it and update the values
{
  byte startIndex = 0;
  byte endIndex = 0;
  String temp = "xxxxxxxxxx";

  /* making case insensitive */
  GETBuffer.toUpperCase();
  /* adding '&' at the end of Buffer: simplifying parsing */
  startIndex = GETBuffer.indexOf(F("?")); //better get index of HTTP... will implement in next version.
  startIndex = GETBuffer.indexOf(F(" "), startIndex);
  GETBuffer[startIndex] = '&';

  startIndex = 0;

#ifdef DEBUG
  Serial.println(GETBuffer);
#endif

    startIndex = GETBuffer.indexOf(F("DIRECTION"), startIndex);
    endIndex = GETBuffer.indexOf(F("="), startIndex);
    temp = GETBuffer.substring(startIndex + 9, endIndex); //direction
    startIndex = endIndex;
    endIndex = GETBuffer.indexOf(F("&"), startIndex);
    temp = GETBuffer.substring(startIndex + 1, endIndex);

    /* get rid of trailing whitespace if direction on a line by itself */
    temp.trim();
    
#ifdef DEBUG
  Serial.println(F("Value of temp is "));
  Serial.println(temp);
#endif

    /* Note that since compasses don't divide up into integers, I favored */
    /* the ordinal points (n, s, e, w) and gave them 1 degree wider angle */
    /* in each direction.                                                 */
    if (temp.equals(F("N")) || (temp.toInt() >=0 && temp.toInt() <= 23) || (temp.toInt() >=337 && temp.toInt() <=360))
    {
      pin = 2;;
    }
    else if (temp.equals(F("NE")) ||  (temp.toInt() >=24 && temp.toInt() <= 66))
    {
      pin = 3;
    }
    else if (temp.equals(F("E")) ||  (temp.toInt() >=67 && temp.toInt() <= 113))
    {
      pin = 4;
    }
    else if (temp.equals(F("SE")) ||  (temp.toInt() >=114 && temp.toInt() <= 156))
    {
      pin = 5;
    }
    else if (temp.equals(F("S")) ||  (temp.toInt() >=157 && temp.toInt() <= 203))
    {
      pin = 6;
    }
    else if (temp.equals(F("SW")) ||  (temp.toInt() >=204 && temp.toInt() <= 245))
    {
      pin = 7;
    }
    else if (temp.equals(F("W")) ||  (temp.toInt() >=246 && temp.toInt() <= 293))
    {
      pin = 8;
    }
    else if (temp.equals(F("NW")) ||  (temp.toInt() >=294 && temp.toInt() <= 336))
    {
      pin = 9;
    }

    
 
    
#ifdef DEBUG
  Serial.println(F("Value of pin is "));
  Serial.println(pin);
#endif    
      


// Setting speed values
  startIndex = 0;
  startIndex = GETBuffer.indexOf(F("SPEED="), startIndex);
  if (startIndex < GETBuffer.length()) //don't exceed buffer size more than 254 bytes
  {
    startIndex = endIndex;
    endIndex = GETBuffer.indexOf(F(" "), startIndex);
    temp = GETBuffer.substring(startIndex + 7, endIndex); //speed value
        
    windspeed = (byte)temp.toInt();
    if (windspeed < 1) {
       windspeed = 0;
    }
    if (windspeed > 99) {
       windspeed = 99;
    }
    
    
#ifdef DEBUG
  Serial.println(F("Value of windspeed is "));
  Serial.println(windspeed);
#endif    
      
  }
  
}


/* Now, actually take the values and push them to the ports for action */
void display_values() {
  int needle;
  
#ifdef DEBUG
  Serial.println(F("We're in display_values function with pin and windspeed set to "));
  Serial.println(pin);
  Serial.println(windspeed);
#endif    
        
  /* Flush all lights to off if direction pin value is set is non-zero to prepare for new light setting */
  if (pin > 0) {
  for (byte i = 2; i < 10; i++)
   {
    digitalWrite(i, LOW);
   }
    
   /* Now turn on the one we want */ 
  digitalWrite(pin, HIGH);
   } 
  
  /* Set wind speed dial if non-zero */
  if (windspeed > -1) {
    /* Values are 0-100 on mph and 0-255 on analog values, so do ratio */
    needle=int(windspeed*2.55);
    
#ifdef DEBUG
  Serial.println(F("Needle is set to "));
  Serial.println(needle);
#endif    
    
    
    analogWrite(10,needle);
  }
}



void loop () {
  YunClient client = server.accept();
 
  if (client) {
    lastTime = millis();
    process(client);
    client.stop();
  }

  countup=millis()-lastTime;
  delay(1000);

  /* If we've waited more than 60 seconds for an update, blink North as an error mode */
  if (countup > 60000) {
    analogWrite(10,0);
    digitalWrite(2, HIGH);
    delay(1000);
    digitalWrite(2, LOW);
  }
}


void setup() {
#ifdef DEBUG
  Serial.begin(9600);
#endif
  Bridge.begin();
  server.listenOnLocalhost();
  server.begin();
  Serial.println(F("Starting main loop now"));

  // set output mode and turn off all lights
  for (byte i = 2; i < 10; i++)
  {
    pinMode(i, OUTPUT);
    digitalWrite(i, LOW);
  }

  // cycle the lights in a circle (boot-up indication)
  for (byte i = 2; i < 10; i++)
  {
    digitalWrite(i, HIGH);
    delay(200);
    digitalWrite(i, LOW);
  }

  /*  cycle the needle gauge
    for(byte i=0;i<255;i =  i + 1)
      {
      analogWrite (10, i);
      delay(100);
      }
      analogWrite(10,0);
  */

}





