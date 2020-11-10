/* -----------------------------------------------------------------------------
 * Filename     : hex_keypad_driver.vhd
 * Author(s)    : Kyle Bielby, Chris Lloyd (Team 1)
 * Class        : EE365 (Final Project)
 * Due Date     : 2020-11-23
 * Target Board : Cora Z7-10
 * Description  : Multi-mode simple security system.
 *                CDL=> Elaborate on design (modes, codes, I/O etc..)
 * -------------------------------------------------------------------------- */

// Includes
#include <stdio.h>
#include <stdbool.h>
#include "platform.h"
#include "xil_printf.h"
#include "xparameters.h"
#include "xil_cache.h"
#include "keypad_binary_slave.h"
#include "seven_segment_display_slave.h"
#include "axilab_slave_button.h"
#include "axilab_slave_led.h"
#include "xil_io.h"

// Masks for on board push buttons
#define BUTTON_0_MASK 1
#define BUTTON_1_MASK 2
#define RESET_BUTTON_MASK BUTTON_1_MASK  //redefinition
#define MODE_BUTTON_MASK  BUTTON_0_MASK

// Masks for rgb leds of on board leds
#define LED_0_BLUE_MASK   0b000001
#define LED_0_GREEN_MASK  0b000010
#define LED_0_RED_MASK    0b000100
#define LED_1_BLUE_MASK   0b001000
#define LED_1_GREEN_MASK  0b010000
#define LED_1_RED_MASK    0b100000
#define LED_0_PURPLE_MASK LED_0_BLUE_MASK | LED_0_RED_MASK
#define LED_0_YELLOW_MASK LED_0_GREEN_MASK | LED_0_RED_MASK

// Masks for peripheral addresses
#define RGB_LED_BASE_ADDR       0x43c30000
#define ON_BOARD_PUSH_BASE_ADDR 0x43c10000
#define SEVEN_SEGMENT_BASE_ADDR 0x43c20000
#define KEYPAD_BASE_ADDR        0x43c00000

// An enum to define the operating modes (states) of the program
#define DEFAULT_MODE MODE_1_CHECK_CODE
typedef enum
{
    MODE_1_CHECK_CODE = 0x1,
    MODE_2_SET_CODE   = 0x2,
    MODE_3_GET_CODE   = 0x3
} Mode;

void toggleMode();
void setMode(Mode mode);

// Sets LED color for mode of operation
void setModeLED();

// Determines if reset button has been pressed
bool isResetButtonPressed();

// Determines if mode button has been pressed
bool isModeButtonPressed();

// Determines if keypad has been pressed
bool isKeypadPressed();

// add a new keypad entry to code
void addNewKeypadEntry();

// displays current pin entry to
void displayCurrentEntry();

// checks if code is equal to MASTER_CODE
bool checkForMasterCode();

// checks if code is already existing
bool checkForExistingCode();

// stores newly entered pin number
void storeNewPin();

#define CODE_LENGTH 4
#define MAX_NUM_STORED_CODES 100

// Empty code for all digits blank
const uint8_t BLANK_CODE[CODE_LENGTH] = {0xF,0xF,0xF,0xF};

// Master code for system (cannot be changed)
const uint8_t MASTER_CODE[CODE_LENGTH] = {0,0,0,0};

// The current mode of the program
Mode currentMode;

// Location to store valid codes
uint8_t storedCodes[MAX_NUM_STORED_CODES][CODE_LENGTH];

// A location to store the current keypad entry (all 0xF meaning digits are off)
uint8_t currentKeypadEntry[CODE_LENGTH];
uint8_t currentKeypadEntryIndex; // CDL=> You are here: get index working

/*
 * This function is the main function of the project.
 *
 * Arguments: None
 *
 * Return:
 * - (int): Integer status of function exit. (0 -> good, anything else -> error)
 */
int main(void)
{
    // Initialization
    init_platform();

    // initialize stored codes
    for(int i = 0; i < MAX_NUM_STORED_CODES; i++) {
    	storedCodes[i][0] = 0xF;
    	storedCodes[i][1] = 0xF;
    	storedCodes[i][2] = 0xF;
    	storedCodes[i][3] = 0xF;
    }

    // Set the current mode
    setMode(0x1);

    // Hex value of key press
    uint8_t keyHex;

    while (1) // Main program execution loop
    {
        if (isResetButtonPressed())
        {
        	print("Reset Button was pressed");
            // Set the current mode
            setMode(DEFAULT_MODE);

            // Clear any stored codes // CDL=> Move to method?
            memset(storedCodes, 0, sizeof(storedCodes[0][0]) *
                   MAX_NUM_STORED_CODES * CODE_LENGTH);
        }
        else if (isModeButtonPressed())
        {
            // Toggle the current mode and reset code
            toggleMode();
        }
        else if (isKeypadPressed())
        {
        	if(currentKeypadEntry[3] != 15) {
        		currentKeypadEntry[0] = BLANK_CODE[0];
        		currentKeypadEntry[1] = BLANK_CODE[1];
        		currentKeypadEntry[2] = BLANK_CODE[2];
        		currentKeypadEntry[3] = BLANK_CODE[3];
        	}

        	for(int i = 0; i < 15000000; i++){}
            // add to currentKeyPadEntry
            addNewKeypadEntry();

            // set seven segment display to currentKeyPadEntry
            displayCurrentEntry();

            if (currentKeypadEntry[0] != 15 && currentKeypadEntry[1] != 15 &&  // NOTE could potentially create a function for this
                currentKeypadEntry[2] != 15 && currentKeypadEntry[3] != 15)
            {
               switch (currentMode)
               {
                   case MODE_1_CHECK_CODE:
                       // Set led1 (mode led) to color of currentMode
                       if (currentKeypadEntry[0] == MASTER_CODE[0] && currentKeypadEntry[1] == MASTER_CODE[1] && currentKeypadEntry[2] == MASTER_CODE[2] && currentKeypadEntry[3] == MASTER_CODE[3])
                       {
                    	   // flash green led
                    	   AXILAB_SLAVE_LED_mWriteReg(0x43c30000, 0, LED_0_BLUE_MASK | LED_1_GREEN_MASK);
                    	   for(int i = 0; i < 10000000; i++){}
                    	   AXILAB_SLAVE_LED_mWriteReg(0x43c30000, 0, LED_0_BLUE_MASK);
                       }
                       else
                       {
                           // flash red leds
                    	   AXILAB_SLAVE_LED_mWriteReg(0x43c30000, 0, LED_0_BLUE_MASK | LED_1_RED_MASK);
                           for(int i = 0; i < 10000000; i++){}
                    	   AXILAB_SLAVE_LED_mWriteReg(0x43c30000, 0, LED_0_BLUE_MASK);
                       }
                       break;
                   case MODE_2_SET_CODE:
                       // Set led1 (mode led) to color of currentMode
                       if (!checkForMasterCode() && !checkForExistingCode())
                       {
                           // flash green leds and add code to codes[]
                    	   AXILAB_SLAVE_LED_mWriteReg(0x43c30000, 0, LED_0_YELLOW_MASK | LED_1_GREEN_MASK);
                    	   for(int i = 0; i < 10000000; i++){}
                    	   AXILAB_SLAVE_LED_mWriteReg(0x43c30000, 0, LED_0_YELLOW_MASK);
                           printf("current pin: %d%d%d%d", currentKeypadEntry[0], currentKeypadEntry[1], currentKeypadEntry[2], currentKeypadEntry[3]);
                    	   storeNewPin();
                    	   printf("Stored Code: %d%d%d%d", storedCodes[0][0], storedCodes[0][1], storedCodes[0][2], storedCodes[0][3]);
                       }
                       else
                       {
                           // flash red leds
                    	   AXILAB_SLAVE_LED_mWriteReg(0x43c30000, 0, LED_0_YELLOW_MASK | LED_1_RED_MASK);
                    	   for(int i = 0; i < 10000000; i++){}
                    	   AXILAB_SLAVE_LED_mWriteReg(0x43c30000, 0, LED_0_YELLOW_MASK);
                       }
                       break;
                   case MODE_3_GET_CODE:
                       printf("current mode: %d", currentMode);

                       if (!checkForMasterCode() && checkForExistingCode())
                       {
                           // flash green leds and remove code from codes[]
                    	   AXILAB_SLAVE_LED_mWriteReg(0x43c30000, 0, LED_0_PURPLE_MASK | LED_1_GREEN_MASK);
                    	   for(int i = 0; i < 10000000; i++){}
                    	   AXILAB_SLAVE_LED_mWriteReg(0x43c30000, 0, LED_0_PURPLE_MASK);
                       }
                       else
                       {
                           // flash red leds
                    	   AXILAB_SLAVE_LED_mWriteReg(0x43c30000, 0, LED_0_PURPLE_MASK | LED_1_RED_MASK);
                    	   for(int i = 0; i < 10000000; i++){}
                    	   AXILAB_SLAVE_LED_mWriteReg(0x43c30000, 0, LED_0_PURPLE_MASK);
                       }
                       break;
                   default:
                       break;
               }
                // clear currentKeyPadEntry and set reset seven seg display
            }
        }

    }
    cleanup_platform();
    return 0;
}

/*
 * This function toggles the current mode.
 *
 * Return: None (void)
 */
void toggleMode()
{
    // Toggle the current mode
    switch (currentMode)
    {
        case MODE_1_CHECK_CODE:
            setMode(MODE_2_SET_CODE);
            break;
        case MODE_2_SET_CODE:
            setMode(MODE_3_GET_CODE);
            break;
        case MODE_3_GET_CODE:
            setMode(MODE_1_CHECK_CODE);
            break;
        default:
            setMode(DEFAULT_MODE);
            break;
    }
}

/*
 * This function sets the current mode, changes mode led, and
 * resets current code.
 *
 * Return: None (void)
 */
void setMode(Mode mode)
{
	print("Setting mode\n");
    // Set the current mode
    currentMode = mode;

    // Set the mode LED to the current mode color
    setModeLED();

    // Reset the current code
    currentKeypadEntry[0] = BLANK_CODE[0];
    currentKeypadEntry[1] = BLANK_CODE[1];
    currentKeypadEntry[2] = BLANK_CODE[2];
    currentKeypadEntry[3] = BLANK_CODE[3];
    currentKeypadEntryIndex = 0;
}

/*
 * This function sets the LED color for the current mode
 *
 * Return: None (void)
 */
void setModeLED() {
  switch (currentMode) {
    case MODE_1_CHECK_CODE:
        AXILAB_SLAVE_LED_mWriteReg(RGB_LED_BASE_ADDR, 0, LED_0_BLUE_MASK);
        break;
    case MODE_2_SET_CODE:
        AXILAB_SLAVE_LED_mWriteReg(RGB_LED_BASE_ADDR, 0, LED_0_YELLOW_MASK);
        break;
    case MODE_3_GET_CODE:
        AXILAB_SLAVE_LED_mWriteReg(RGB_LED_BASE_ADDR, 0, LED_0_PURPLE_MASK);
        break;
    default:
    	break;
  }
}

/*
 * This function determines if the reset button has been pressed
 *
 * Return: bool: true if reset button has been pressed,
 * false if reset button has not been pressed
 */
bool isResetButtonPressed() {
  if(AXILAB_SLAVE_BUTTON_mReadReg(ON_BOARD_PUSH_BASE_ADDR, 0) &
     RESET_BUTTON_MASK) {
      return true;
  }
  return false;
}

/*
 * This function determines if the mode button has been pressed
 *
 * Return: bool: true if mode button has been pressed,
 * false if mode button has not been pressed
 */
bool isModeButtonPressed() {
  if(AXILAB_SLAVE_BUTTON_mReadReg(ON_BOARD_PUSH_BASE_ADDR, 0) &
     MODE_BUTTON_MASK) {
      return true;
  }
  return false;
}

/*
 * This function determines if the keypad button has been pressed
 *
 * Return: bool: true if keypad button has been pressed,
 * false if keypad button has not been pressed
 */
bool isKeypadPressed() {
  if(KEYPAD_BINARY_SLAVE_mReadReg(KEYPAD_BASE_ADDR, 0) != 0xF) {
    return true;
  }
  return false;
}

/*
 * This function adds a new keypad entry to the set of pin numbers
 *
 * Return: None (void)
 */
void addNewKeypadEntry() {
  u32 data1 = KEYPAD_BINARY_SLAVE_mReadReg(0x43c00000, 0);
//  printf("data: %d", data1);
  if(data1 != 0xF && currentKeypadEntry[3] != 0xF) {
	  currentKeypadEntry[0] = data1;
	  currentKeypadEntry[1] = BLANK_CODE[1];
	  currentKeypadEntry[2] = BLANK_CODE[2];
	  currentKeypadEntry[3] = BLANK_CODE[3];
  }
  else {
	  for(int i = 3; i >= 0; i--) {
	       if(data1 != 15) {
	           if(i == 0) {
	               currentKeypadEntry[0] = data1 & 0xF;
	           }
	           else {
	               currentKeypadEntry[i] = currentKeypadEntry[i - 1];
	           }
	       }

	   }
  }

  printf("entries: %d%d%d%d\n", currentKeypadEntry[0], currentKeypadEntry[1], currentKeypadEntry[2], currentKeypadEntry[3]);
}

/*
 * This function adds a new keypad entry to the set of pin numbers
 *
 * Return: None (void)
 */
void displayCurrentEntry() {
	SEVEN_SEGMENT_DISPLAY_SLAVE_mWriteReg(0x43c20000, 0, currentKeypadEntry[3] << 12 | currentKeypadEntry[2] << 8 | currentKeypadEntry[1] << 4 | currentKeypadEntry[0]);
}

/*
 * This function adds a new keypad entry to the set of pin numbers
 *
 * Return: bool returns true if currentKeypadEntry is the master code
 */
bool checkForMasterCode() {
	return currentKeypadEntry[0] == MASTER_CODE[0] && currentKeypadEntry[1] == MASTER_CODE[1] && currentKeypadEntry[2] == MASTER_CODE[2] && currentKeypadEntry[3] == MASTER_CODE[3];
}

/*
 * This function checks if a
 *
 * Return: bool returns true if currentKeypadEntry is the master code
 */
bool checkForExistingCode()
{
	for(int i = 0; i < MAX_NUM_STORED_CODES; i++) {
		if(currentKeypadEntry[0] == storedCodes[i][0] && currentKeypadEntry[1] == storedCodes[i][1] && currentKeypadEntry[2] == storedCodes[i][2] && currentKeypadEntry[3] == storedCodes[i][3]) {
			print("existing code found\n");
			return true;
		}
	}
	print("Code not found\n");
	return false;
}

/*
 * This stores a newly entered four digit pin
 *
 * Return: None (void)
 */
void storeNewPin() {

	if(currentKeypadEntry[3] != 0xF)
	{
		storedCodes[0][0] = currentKeypadEntry[0];
	    storedCodes[0][1] = currentKeypadEntry[1];
		storedCodes[0][2] = currentKeypadEntry[2];
		storedCodes[0][3] = currentKeypadEntry[3];
		for(int i = 0; i < MAX_NUM_STORED_CODES; i++)
		{
			if(storedCodes[i][0] == 0xF)
			{
				print("stored new code\n");
				storedCodes[i][0] = currentKeypadEntry[0];
				storedCodes[i][1] = currentKeypadEntry[1];
				storedCodes[i][2] = currentKeypadEntry[2];
				storedCodes[i][3] = currentKeypadEntry[3];
				break;
			}
		}
	}
}
