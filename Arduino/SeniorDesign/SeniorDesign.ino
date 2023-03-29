#define TIMER_INTERRUPT_DEBUG     2
#define _TIMERINTERRUPT_LOGLEVEL_ 0

#define USE_TIMER_1 false
#define USE_TIMER_2 true

#include <TimerInterrupt.h>
#include <TimerOne.h>

int led = 8, receiverIn = A0, transmitterOut = 10;
uint8_t charDelay = 20, comma = 1, errorChance = 1;
int outputTest = 9;

int delayTime = 250;
bool delayEnable = false;

uint8_t incoming = 0, outgoing = 0, transmittedBits = 0, receivedBits = 0;
bool transmitFlag = false, receiveFlag = false, newTransmission = false, newReception = false;
bool o = false;

// Hardware Timer 1 frequency values:
int highFreq = 41; // 24KHz square wave
//int highFreq = 20; // 50KHz square wave
int lowFreq = 62; // 16KHz square wave
//int lowFreq = 40; // 25KHz square wave

// Hardware Timer 2 frequency value:
int interruptFreq = 20000;

void setup()
{
  pinMode(led, OUTPUT);
  pinMode(receiverIn, INPUT);
  pinMode(outputTest, OUTPUT);

  // Setup PWM pin based on 1MHz timer1 for transmitter:
  pinMode(transmitterOut, OUTPUT);
  Timer1.initialize(lowFreq);
  Timer1.pwm(transmitterOut, 512);

  // Setup Timer2 for transmit/receive operations:
  ITimer2.init();
  //ITimer2.attachInterrupt(interruptFreq, timerHandler);
  ITimer2.attachInterrupt(interruptFreq, interruptHandler);

  Serial.begin(9600);
}

void timerHandler()
{
  digitalWrite(led, o);
  o = !o;
}

void loop()
{
  // Read from PC
  if (Serial.available() > 0 && !newTransmission && transmittedBits == 0)
  {
    incoming = Serial.read();
    newTransmission = true;
  }

  if (transmitFlag)
  {
    transmitBit();
    transmitFlag = false;
  }

  if (receiveFlag)
  {
    receiveBit();
    receiveFlag = false;
  }//*/
}

void transmitBit()
{
  delayMicroseconds(charDelay);
  if(incoming == 0)
  {
    return;
  }

  if(newTransmission)
  {
    transmittedBits = 0;
  }

  if(newTransmission || transmittedBits < 1)
  {
    Timer1.setPeriod(highFreq);
    Timer1.setPwmDuty(transmitterOut, 512);

    digitalWrite(outputTest, true);

    doDelay();
  }
  else if (bitRead(incoming, transmittedBits-1) == 1)
  {
    Timer1.setPeriod(highFreq);
    Timer1.setPwmDuty(transmitterOut, 512);

    digitalWrite(outputTest, true);

    doDelay();
  }
  else
  {
    Timer1.setPeriod(lowFreq);
    Timer1.setPwmDuty(transmitterOut, 512);

    digitalWrite(outputTest, false);

    doDelay();
  }

  /*if(digitalRead(outputTest) == true) Serial.write(1);
  else Serial.write(0);//*/

  if (delayEnable)
  {
    digitalWrite(led, HIGH);
    doDelay();
    digitalWrite(led, LOW);
    doDelay();
  }

  if (transmittedBits < 8)
  {
    newTransmission = false;
    transmittedBits++;
  }
  else
  {
    //Serial.write(incoming);
    transmittedBits = 0;
    incoming = 0;

    Timer1.setPeriod(lowFreq);
    Timer1.setPwmDuty(transmitterOut, 512);

    digitalWrite(outputTest, false);
  }
}

void receiveBit()
{
  uint8_t votes = 5, carry;
  float holder = 0;

  for(int i=0; i<votes; i++)
  {
    if (analogRead(receiverIn) > 512)
    {
      holder += 1;
    }
    else
    {
      holder += 0;
    }
  }

  holder /= votes;

  if(!newReception)
  {
    if(holder > 0.5)
    {
      carry = 1;
      outgoing <<= 1;
      outgoing += carry;
    }
    else
    {
      carry = 0;
      outgoing <<= 1;
      outgoing += carry;
    } 
  }
  else
  {
    if(holder > 0.5)
    {
      carry = 1;
      //if(random(0,99) < errorChance) carry = 0;
      outgoing |= carry << receivedBits;
    }
    else
    {
      carry = 0;
      //if(random(0,99) < errorChance) carry = 1;
      outgoing |= carry << receivedBits;
    }
  }

  if(outgoing == comma && !newReception)
  {
    receivedBits = 0;
    outgoing = 0;
    newReception = true;
  }
  else if(newReception)
  {
    if (receivedBits < 7) receivedBits++;
    else
    {
      if(outgoing != 0)
      {
        Serial.write(outgoing);
      }

      receivedBits = 0;
      outgoing = 0;
      newReception = false;
    }    
  }
}

void interruptHandler()
{
  transmitFlag = true;
  receiveFlag = true;
}

void doDelay()
{
  if (delayEnable)
  {
    delay(delayTime);
  }
}
