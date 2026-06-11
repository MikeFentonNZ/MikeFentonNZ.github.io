
/* RIGEL to ESP8266 Wemos D1 RX/TX test

DO NOT USE DELAY() for timing.
Instead, use timer variables in Loop()

Uses RGB LED to show receiving a command to control a connected device / sensor.
Send dummy data for TWO (2) sensors; blue light level and green light level.
Sensor values are out of 100%, except for
Air pressure (hPa - 900), distance (in centimetres), temperature ( value above 100 is -ve).
*/

// --- Declare global byte variables to retain their values across loop iterations

#define LED_ON HIGH
#define LED_OFF LOW
#define BLUE_LED D1
#define GREEN_LED D2
#define RED_LED D3

// --- Protocol Definition ---
const byte START_BYTE = 'R'; // 0x52 = 82 DEC
const byte END_BYTE = '\n';  // 0x0A = 10 DEC
const int BAUD_RATE = 38400;

// --- Data Structures ---
// Use structs to organize our data in a logic way.

// Structure for a sensor
struct Sensor {
  const char id; // The unique ASCII identifier (e.g., 't', 'h')
  byte value;    // The current reading (0-255)
};

// Structure for an output device
struct Output {
  const byte id; // The numeric ID (0-9)
  byte state;    // The current state (0-7)
};

//CHANGE OUTPUT DEVICE STATE - use Mod 10 eg,
// 10 = Switch is off, 11 = Switch is on. 
// 22 = motor is forward, 23 = motor is backward, 20 is motor is off
// 34 = LED is red, 35 = LED is green, 36 = LED is blue, 37 = LED is yellow,
// 38 = LED is white, 30 is LED is off.

// --- Device Configuration ---
// Define all our sensors and outputs here.
// This makes the code easy to expand and manage.

// COMMENT OUT SENSORS THAT ARE NOT USED!!!!!!!!
Sensor sensors[] = {
//  {'a', 0}, // Atmospheric (barometric) pressure above 900 hPa (ADD 900 to get original value)
//  {'b', 0}, // Blue colour sensor (raw ANALOG, use alarm min/max to make "digital")
//  {'c', 0}, // Contact / impact / break beam (DIGITAL)
//  {'d', 0}  // Distance sensor (HC-SR04) (raw ANALOG in cm (maximum 254 - 2.54 metres)
//  {'e', 0}, // Heading - compass point (1-12; N,NNE,NE,ENE,E,ESE,SE,SSE,S,SSW,SW,WSW,W,WNW,NW,NNW) 
//  {'g', 0}, // Green colour sensor (raw ANALOG, use alarm min/max to make "digital")
//  {'h', 0}  // Humidity (raw percentage)
//  {'i', 0}, // IR (raw ANALOG, use alarm min/max to make "digital")
//  {'l', 0}  // Ambient light sensor LDR (raw ANALOG, use alarm min/max to make "digital")
//  {'m', 0}, // Magnetic strength(raw ANALOG)
//  {'n', 0}, // Nuclear particle sensor 
//  {'o', 0}, // O2/respiration (raw ANALOG, use alarm min/max to make "digital")
//  {'p', 0}  // pulse/blood flow (raw ANALOG, use alarm min/max to make "digital")
//  {'r', 0}, // Red colour sensor (raw ANALOG, use alarm min/max to make "digital")
//  {'s', 0}, // sound level sensor (raw ANALOG, use alarm min/max to make "digital")
  {'t', 0},  // -ve temperature is above 100, positive temperature 0-100 (raw oC)
//  {'u', 0}, // UV colour sensor (raw ANALOG, use alarm min/max to make "digital")
//  {'x', 0}  // START timer
  {'y', 0} // Yellow colour sensor (raw ANALOG, use alarm min/max to make "digital") 
//  {'z', 0}, // STOP timer
};
const int SENSOR_COUNT = sizeof(sensors) / sizeof(Sensor);

Output outputs[] = {
  {3, 0},  // Output device 1 = RGB LED and it is off
  {4, 0},  // Output device 2 - fan and it is off
//  {3, 0}  // Output device 3
//  {4, 0}  // Output device 4
//  {5, 0}  // Output device 5
//  {6, 0}  // Output device 6
//  {7, 0}  // Output device 7
//  {8, 0}  // Output device 8
};
const int OUTPUT_COUNT = sizeof(outputs) / sizeof(Output);

// ---  GIVE VARIABLES INITIAL START UP VALUES
unsigned long startTime = 0;
unsigned long lastReceivedTime = 0;  
int counter = 0;

// --- Setup runs once only 
void setup() {
  Serial.begin(BAUD_RATE);  // start serial monitor and clear some space; cannot clear serial monitor in Arduino IDE
  // initialize digital pins
  pinMode(RED_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);
  pinMode(BLUE_LED, OUTPUT);
  analogWrite(RED_LED, 0);
  analogWrite(GREEN_LED, 0);
  analogWrite(BLUE_LED, 0);  // OFF
    // Initialize the built-in LED pin as an output
  pinMode(LED_BUILTIN, OUTPUT);
    // Turn OFF the inbuilt LED - represent FAN state ON or OFF
  digitalWrite(LED_BUILTIN, HIGH); // is actually OFF
  // Set the maximum time in milliseconds to wait for serial data.
  // This is set once here, as it doesn't need to be reset in every loop.
  Serial.setTimeout(100);
}

// --- Main loop runs forever
void loop() {
  receiveFromRIGEL();
  delay(1);
}

// =================================================================
// CORE PROTOCOL FUNCTIONS
// =================================================================

/*
 * Checks for incoming serial data and parses it according to the protocol.
 */
void receiveFromRIGEL() {
  // Check if there's any data in the buffer to start with
  if (Serial.available() == 0) {
    return;
  }
  if (Serial.read() != START_BYTE) {
    return; // Not a valid start, ignore this byte
  }
  // readBytes() blocks up to Serial.setTimeout() (100ms), so no delay() hacks
  // are needed while the rest of the packet trickles in.
  byte payloadLength;
  if (Serial.readBytes(&payloadLength, 1) != 1) {
    return; // length byte never arrived
  }
  // RIGEL never sends sensor data, ONLY output device states.
  // IMPORTANT: RIGEL's very FIRST packet after connecting is [82, 0, 10]
  // (length 0 \u2014 the PC doesn't know our outputs yet). We MUST still reply
  // to it, or RIGEL never learns our devices and the link never starts.
  if (payloadLength != OUTPUT_COUNT && payloadLength != 0) {
    // Length is not what we expect, packet is likely corrupt.
    return;
  }
  byte payload[16]; // protocol maximum is 8 output devices
  if (payloadLength > 0 && Serial.readBytes(payload, payloadLength) != payloadLength) {
    return; // payload incomplete
  }
  byte endByte;
  if (Serial.readBytes(&endByte, 1) != 1 || endByte != END_BYTE) {
    return; // Not a valid data packet
  }
  // Process the data (skip on the length-0 handshake packet)
  if (payloadLength == OUTPUT_COUNT) {
    processIncomingData(payload, payloadLength);
  }
  readAllSensors();
  sendToRIGEL();
}


/*
 * Processes a validated payload from the PC to update output states.
 * @param payload The byte array of data received.
 * @param length The length of the payload array.
 */
void processIncomingData(byte* payload, byte length) {
  // Check OUTPUT_COUNT = number of devices ESP or Picaxe has sent
  // Default back to OUTPUT_COUNT
  if (length == OUTPUT_COUNT) { 
   for (int i = 0; i < length; i++) {
     byte data = payload[i];
     byte outputId = data / 10;   // Integer division gives the tens place
     byte outputState = data % 10; // Modulo gives the ones place

     // Find the matching output device and update its state
     for (int j = 0; j < OUTPUT_COUNT; j++) {
       if (outputs[j].id == outputId) {
         outputs[j].state = outputState;

         switch (outputId) {
          case 1: // Switch
           {
            updateSwitch(outputState);
            break;
           } 
          case 2: // Simple motor or dual drive robot
           {
            updateMotor(outputState);
            break;
           } 
          case 3: // LED or RGB 
           {
            updateLEDRGB(outputState);
            break;
           } 
          case 4: // Fan
           {
            updateFan(outputState);
            break;
           } 
          case 5: // Heater
           {
            updateHeater(outputState);
            break;
           } 
          case 6:  // Vent
           {
            updateVent(outputState);
            break;
           } 
          case 7: // Pump
           {
            updatePump(outputState);
            break;
           } 
          case 8: // Valve
           {
            updateValve(outputState);
            break;
           }    
         }
       }  
     }
   }
  }
}

void updateSwitch(byte device_state){

}

void updateMotor(byte device_state){

}

void updateLEDRGB(byte device_state){
// 0 = off, 1 = on, 4 = red, 5 = green, 6 = blue, 7 = yellow, 8 = white.
    switch (device_state) {
      case 0:
      {
        analogWrite(RED_LED, 0);
        analogWrite(GREEN_LED, 0);
        analogWrite(BLUE_LED, 0);  // OFF
        break;
      } 
      case 4:
      {
        analogWrite(RED_LED, 255);
        analogWrite(GREEN_LED, 0);
        analogWrite(BLUE_LED, 0);  // red
        break;
      } 
      case 5:
      {
        analogWrite(RED_LED, 0);
        analogWrite(GREEN_LED, 255);
        analogWrite(BLUE_LED, 0);  // green
        break;
      } 
      case 6:
      {
        analogWrite(RED_LED, 0);
        analogWrite(GREEN_LED, 0);
        analogWrite(BLUE_LED, 255);  // blue
        break;
      } 
      case 7:
      {
        analogWrite(RED_LED, 155);
        analogWrite(GREEN_LED,155);
        analogWrite(BLUE_LED,0);  // yellow
        break;
      } 
      case 8:
      {
        analogWrite(RED_LED, 255);
        analogWrite(GREEN_LED,255);
        analogWrite(BLUE_LED, 255);  // white
        break;
      } 
    } 
}

void updateFan(byte device_state){  // demonstration LED indicator
  if (device_state == 0) {
     digitalWrite(LED_BUILTIN, HIGH); // is actually OFF
  }else{
     digitalWrite(LED_BUILTIN, LOW); // is actually ON
  }
}


void updateHeater(byte device_state){

}

void updateVent(byte device_state){

}

void updatePump(byte device_state){

}

void updateValve(byte device_state){

}


/*
 * Reads all sensor values and updates the global structs.
 */
void readAllSensors() {
  for (int i = 0; i < SENSOR_COUNT; i++) {
    // --- YOUR SENSOR READING LOGIC HERE ---
    // Example:
    // if (sensors[i].id == 't') { sensors[i].value = readTemp(); }
    // For now, we use random values for demonstration.
    counter += 1;
    if (counter > 100) { counter = 0;}
    sensors[i].value = counter;
  }
}


/*
 * Assembles and sends the full data packet to RIGEL.
 */
void sendToRIGEL() {
  // The payload contains all sensor data (2 bytes each) and all output data (1 byte each).
  const byte payloadLength = (SENSOR_COUNT * 2) + OUTPUT_COUNT;
  byte payload[payloadLength];

  // Populate payload with sensor data
  int payloadIndex = 0;
  for (int i = 0; i < SENSOR_COUNT; i++) {
    payload[payloadIndex++] = sensors[i].id;
    payload[payloadIndex++] = sensors[i].value;
  }

  // Populate payload with output data
  for (int i = 0; i < OUTPUT_COUNT; i++) {
    // Encode output ID and state into a single byte
    byte encodedOutput = (outputs[i].id * 10) + outputs[i].state;
    payload[payloadIndex++] = encodedOutput;
  }

  // Send the complete packet over serial
  Serial.write(START_BYTE);
  Serial.write(payloadLength);
  Serial.write(payload, payloadLength);
  Serial.write(END_BYTE);
}

