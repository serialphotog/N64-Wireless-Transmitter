#ifndef __N64_H__
#define __N64_H__

// The pin that the controller interfaces with
#define N64_PIN 2

// Values for reading/writing from/to the controller
#define N64_HIGH DDRD &= ~0x04
#define N64_LOW DDRD |= 0x04
#define CONTROLLER_QUERY (PIND & 0x04)

// The delay for polling
#define POLLING_DELAY 25

// Stores the 8-byte values that we get from the controller
struct {
    unsigned char data1;
    unsigned char data2;
    char stickX;
    char stickY;
} ControllerStatus;

// Stores the raw controller data that we read
// Stores 1 received bit per byte
char rawControllerData[33];

// Checks if the start button is down
bool startDown()
{
    return ControllerStatus.data1 & 16;
}

// Checks if the Z button is down
bool zDown() 
{
    return ControllerStatus.data1 & 32;
}

// Checks if the B button is down
bool bDown()
{
    return ControllerStatus.data1 & 64;
}

// Checks if the A button is down
bool aDown()
{
    return ControllerStatus.data1 & 128;
}

// Checks if the left bumper button is down
bool lBumperDown()
{
    return ControllerStatus.data2 & 32;
}

// Checks if the right bumper button is down
bool rBumperDown()
{
    return ControllerStatus.data2 & 16;
}

// Checks if the C-Up button is down
bool cUpDown()
{
    return ControllerStatus.data2 & 0x08;
}

// Checks if the C-Down button is down
bool cDownDown()
{
    return ControllerStatus.data2 & 0x04;
}

// Checks if the C-Right button is down
bool cRightDown()
{
    return ControllerStatus.data2 & 0x01;
}

// Checks if the C-Left button is down
bool cLeftDown()
{
    return ControllerStatus.data2 & 0x02;
}

// Checks if the D-Up button is down
bool dUpDown()
{
    return ControllerStatus.data1 & 0x08;
}

// Checks if the D-Down button is down
bool dDownDown()
{
    return ControllerStatus.data1 & 0x04;
}

// Checks if the D-Right button is down
bool dRightDown()
{
    return ControllerStatus.data1 & 0x01;
}

// Checks if the D-Left button is down
bool dLeftDown()
{
    return ControllerStatus.data1 & 0x02;
}

#endif