/*
 * After getting this all going, I realised it's easier to detect via the TV's USB 5V...
 */

const int CTRL_PIN = 3;
const int OFF_DELAY_SEC = 180; // seconds to wait before turning amp off
const int LOOP_DELAY = 100; // ms between samples

const int OFF_DELAY = OFF_DELAY_SEC * (1000/LOOP_DELAY); 
int tAmpOff = OFF_DELAY;
bool ampOn = false;

// the setup routine runs once when you press reset:
void setup() {
  // initialize serial communication at 9600 bits per second:
  //Serial.begin(9600);
  tAmpOff = OFF_DELAY; 
  ampOn = false;
  pinMode(CTRL_PIN, OUTPUT);
}

const int RC5_PER = 889; // half-bit period (microseconds)
const int CA_RC5_SYS = 16; // Cambridge Audio system code
const int CA_CMD_AMPON    = 14;
const int CA_CMD_AMPOFF   = 15;

void txManchBit(bool b)
{
  digitalWrite(CTRL_PIN, b ? LOW : HIGH);
  delayMicroseconds(RC5_PER);
  digitalWrite(CTRL_PIN, b ? HIGH : LOW);
  delayMicroseconds(RC5_PER);
}

void txRC5(int sys, int cmd)
{
  const int RC5_START = 0b110;
  int msg = ((RC5_START & 0b111) << 11) | ((sys & 0b11111) << 6) | (cmd & 0b111111);
  //Serial.println("RC5 message:");
  //Serial.println(msg, BIN);
  
  for( int i=13; i>=0; i-- )
  { // transmit bits, from MSB to LSB
    txManchBit(msg & (1<<i));
    //Serial.println(1<<i, BIN);
    //Serial.println( (msg & (1<<i)), BIN);
  }
  // return output to idle state
  digitalWrite(CTRL_PIN, LOW);
  
}

// the loop routine runs over and over again forever:
void loop() 
{
  // read the input on analog pins 0 & 1:
  int a0Value = analogRead(A0);
  int a1Value = analogRead(A1);
  
  if((a0Value>10) or (a1Value>10))
  { // audio found
    tAmpOff = OFF_DELAY;
    /*
    Serial.println("I found an audio! ampOn=");
    Serial.println(ampOn);
    Serial.println("tAmpOff=");
    Serial.println(tAmpOff);
    Serial.println("OFF_DELAY=");
    Serial.println(OFF_DELAY); //*/
    if(false == ampOn)
    {
      txRC5(CA_RC5_SYS, CA_CMD_AMPON);
      ampOn = true;
    }
  }
  else 
  { // no audio found
    if(ampOn)
    {
      if(tAmpOff > 0)
      {
        tAmpOff -= 1;
        //Serial.println("Time remaining before shutdown:");
        //Serial.println(tAmpOff);
      }
      else
      { // time to turn off
        txRC5(CA_RC5_SYS, CA_CMD_AMPOFF);
        ampOn = false;
      }
    }
  }
  
  delay(LOOP_DELAY);        // delay in between reads for stability
}
