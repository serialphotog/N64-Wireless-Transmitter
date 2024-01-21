#include "n64.h"

void sendData(unsigned char* buffer, char length)
{
  char bits;

  // This is a precisely timed assembly routine
  // We have 16 cycles per microsecond. Most assembly ops
  // take 1 cycle, but some can take 2
  outer_loop:
  {
    bits = 8;
    inner_loop:
    {
      // Starting a bit, set the line low
      N64_LOW;

      if (*buffer >> 7)
      {
        // 1 bit
        // Remain low for 1us, then go high for 3us
        // Nop block #1
        asm volatile("nop\nnop\nnop\nnop\nnop\n");
        N64_HIGH;

        // Nop block #2
        // Wait for 2us to sync up
        asm volatile(
          "nop\nnop\nnop\nnop\nnop\n"
          "nop\nnop\nnop\nnop\nnop\n"
          "nop\nnop\nnop\nnop\nnop\n"
          "nop\nnop\nnop\nnop\nnop\n"
          "nop\nnop\nnop\nnop\nnop\n"
        );
      }
      else
      {
        // 0 Bit
        // Remains low for 3us, then goes high for 1us
        // Nop block #3
        asm volatile(
          "nop\nnop\nnop\nnop\nnop\n"
          "nop\nnop\nnop\nnop\nnop\n"
          "nop\nnop\nnop\nnop\nnop\n"
          "nop\nnop\nnop\nnop\nnop\n"
          "nop\nnop\nnop\nnop\nnop\n"
          "nop\nnop\nnop\nnop\nnop\n"
          "nop\nnop\nnop\nnop\nnop\n"
          "nop\n"
        );
        N64_HIGH;
      }

      // The line must remain high for exactly 16 more cycles, regardless of previous path
      bits--;
      if (bits != 0)
      {
        // Nop block #4
        asm volatile(
          "nop\nnop\nnop\nnop\nnop\n"
          "nop\nnop\nnop\nnop\n");
        
        // Rotate the bits
        *buffer <<= 1;
        goto inner_loop;
      }
    }

    length--;
    if (length != 0)
    {
      buffer++;
      goto outer_loop;
    }
  }

  asm volatile("nop\nnop\nnop\nnop\n");
  N64_LOW;

  // Wait 1us, 16 cycles, then raise the line
  // Nop block #6
  asm volatile(
    "nop\nnop\nnop\nnop\nnop\n"
    "nop\nnop\nnop\nnop\nnop\n"
    "nop\nnop\nnop\nnop\n"
  );
  N64_HIGH;
}

void translateRawData()
{
  int i;
  memset(&ControllerStatus, 0, sizeof(ControllerStatus));

  // bits: A, B, Z, Start, Dup, Ddown, Dleft, Dright
  for (i = 0; i < 8; i++)
  {
    ControllerStatus.data1 |= rawControllerData[i] ? (0x80 >> i) : 0;
  }

  // bits: 0, 0, L, R, Cup, Cdown, Cleft, Cright
  for (i = 0; i < 8; i++)
  {
    ControllerStatus.data2 |= rawControllerData[i + 8] ? (0x80 >> i) : 0;
  }

  // Joystick x value
  for (i = 0; i < 8; i++)
  {
    ControllerStatus.stickX |= rawControllerData[i + 16] ? (0x80 >> i) : 0;
  }

  // Joystick y value
  for (i = 0; i < 8; i++)
  {
    ControllerStatus.stickY |= rawControllerData[i + 24] ? (0x80 >> i) : 0;
  }
}

void getData()
{
  // Listen for the expected 8 bytes of data back from the controller and 
  // write it to the rawControllerData[], one bit per byte for speed reasons. 
  unsigned char timeout;
  char bitcount = 32;
  char* bitbin = rawControllerData;

  read_loop:
    // Wait for the line to go low
    timeout = 0x3f;
    while (CONTROLLER_QUERY) {
      if (!--timeout)
        return;
    }

    // Wait for ~2us and poll the line
    asm volatile(
      "nop\nnop\nnop\nnop\nnop\n"
      "nop\nnop\nnop\nnop\nnop\n"
      "nop\nnop\nnop\nnop\nnop\n"
      "nop\nnop\nnop\nnop\nnop\n"
      "nop\nnop\nnop\nnop\nnop\n"
      "nop\nnop\nnop\nnop\nnop\n"
    );

    *bitbin = CONTROLLER_QUERY;
    bitbin++;
    bitcount--;
    if (bitcount == 0)
      return;
    
    // Wait for the line to go high again
    timeout = 0x3f;
    while (!CONTROLLER_QUERY)
    {
      if (!--timeout)
        return;
    }

    goto read_loop;
}

void pollStatus()
{
  if (startDown())
    Serial.println("Start Pressed");
  if (zDown())
    Serial.println("Z Pressed");
  if (bDown())
    Serial.println("B Pressed");
  if (aDown())
    Serial.println("A Pressed");
  if (lBumperDown())
    Serial.println("Left Bumper Pressed");
  if (rBumperDown())
    Serial.println("Right Bumper Pressed");
  if (cUpDown())
    Serial.println("C-Up Pressed");
  if (cDownDown())
    Serial.println("C-Down Pressed");
  if (cLeftDown())
    Serial.println("C-Left Pressed");
  if (cRightDown())
    Serial.println("C-Right Pressed");
  if (dUpDown())
    Serial.println("D-Up Pressed");
  if (dDownDown())
    Serial.println("D-Down Pressed");
  if (dLeftDown())
    Serial.println("D-Left Pressed");
  if (dRightDown())
    Serial.println("D-Right Pressed");

  if (ControllerStatus.stickX > 0.0 || ControllerStatus.stickX < 0.0)
  {
    Serial.print("Stick X: ");
    Serial.println(ControllerStatus.stickX, DEC);
  } 

  if (ControllerStatus.stickY > 0.0 || ControllerStatus.stickY < 0.0)
  {
    Serial.print("Stick Y: ");
    Serial.println(ControllerStatus.stickY, DEC);
  }
}

void setup() {
  Serial.begin(9600);

  // Communicate with the N64 controller on this pin
  // Don't sent +5v to the controller
  digitalWrite(N64_PIN, LOW);
  pinMode(N64_PIN, INPUT);

  // Initialize the controller by sending a null byte
  unsigned char initialize = 0x00;
  noInterrupts();
  sendData(&initialize, 1);

  // We need to wait for the controller to quit sending it's response
  // We don't care what the response is at this stage
  Serial.println("Waiting for controller's response...");
  int x;
  for (x = 0; x < 64; x++)
  {
    if (!CONTROLLER_QUERY)
      x = 0;
  }
  Serial.println("Response received. Ready to begin polling.");

  // Query the controller's status so we can determine the 0 point of the joystick
  unsigned char command[] = { 0x01 };
  sendData(command, 1);
  getData();
  interrupts();
  translateRawData();
}

void loop() {
  // Command to send to the N64 controller.
  unsigned char command[] = { 0x01 };

  noInterrupts();
  sendData(command, 1);
  getData();
  interrupts();

  // Perform the data translation
  translateRawData();

  // Poll the controller's status
  pollStatus();

  delay(POLLING_DELAY);
}
