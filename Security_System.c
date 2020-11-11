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
#define RESET_BUTTON_MASK BUTTON_1_MASK
#define MODE_BUTTON_MASK  BUTTON_0_MASK

// Masks for colors of each onboard led
#define LED_0_BLUE_MASK   0b000001
#define LED_0_GREEN_MASK  0b000010
#define LED_0_RED_MASK    0b000100
#define LED_1_BLUE_MASK   0b001000
#define LED_1_GREEN_MASK  0b010000
#define LED_1_RED_MASK    0b100000
#define LED_0_PURPLE_MASK LED_0_BLUE_MASK  | LED_0_RED_MASK
#define LED_0_YELLOW_MASK LED_0_GREEN_MASK | LED_0_RED_MASK

// Masks for peripheral addresses
#define KEYPAD_BASE_ADDR        0x43c00000
#define ON_BOARD_PUSH_BASE_ADDR 0x43c10000
#define SEVEN_SEGMENT_BASE_ADDR 0x43c20000
#define RGB_LED_BASE_ADDR       0x43c30000

// An enum to define the operating modes (states) of the program
typedef enum
{
    MODE_1_CHECK_CODE = 0x1,
    MODE_2_SET_CODE   = 0x2,
    MODE_3_GET_CODE   = 0x3
} Mode;
#define DEFAULT_MODE MODE_1_CHECK_CODE  // CDL=> Moved to be below definition

// The current mode of the program
Mode currentMode;

// Toggles the current mode of operation
void toggleMode();

// Sets the current mode of operation
void setMode(Mode mode);

// Sets mode LED color for current mode of operation
void setModeLED();

// Determines if reset button has been pressed
bool isResetButtonPressed();

// Determines if mode button has been pressed
bool isModeButtonPressed();

// Determines if keypad has been pressed
bool isKeypadPressed();

// Add a new keypad entry to code
void addNewKeypadEntry();

// Displays current pin entry to seven segment display
void displayCurrentEntry();

// Checks if code is equal to MASTER_CODE
bool checkForMasterCode();

// Checks if code is already existing
bool checkForExistingCode();

// Stores newly entered pin number
void storeNewPin();

// Remove stored pin number
void removePin();

#define CODE_LENGTH 4
#define MAX_NUM_STORED_CODES 100

// Empty code for all digits blank (Note: Blank code is denoted by 0xF)
const uint8_t BLANK_CODE[CODE_LENGTH] = {0xF,0xF,0xF,0xF};  // CDL=> Needed?

// Master code for system (cannot be changed)
const uint8_t MASTER_CODE[CODE_LENGTH] = {0,0,0,0};

// Location to store valid codes
uint8_t storedCodes[MAX_NUM_STORED_CODES][CODE_LENGTH];

// A location to store the current keypad entry
uint8_t currentKeypadEntry[CODE_LENGTH];
uint8_t currentKeypadEntryIndex; // CDL=> Used or needed?

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
    // Initialization of UART for print functionality
    init_platform();

    // Initialize stored codes to null values of 0xF
    for (int i = 0; i < MAX_NUM_STORED_CODES; i++)
    {
    	// storedCodes[i] = BLANK_CODE; // CDL=> Would this work? Better way to clear codes?
        storedCodes[i][0] = 0xF;
    	storedCodes[i][1] = 0xF;
    	storedCodes[i][2] = 0xF;
    	storedCodes[i][3] = 0xF;
    }

    // Set the current mode to default mode (check mode)
    setMode(DEFAULT_MODE);

    // Hex value of key press
    uint8_t keyHex;

    while (1) // Main program execution loop
    {
        if (isResetButtonPressed())
        {
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

            for(int i = 0; i < 100000000; i++){}  // delay  // CDL=> Make explicit delay function

        }
        else if (isKeypadPressed())
        {
        	if (currentKeypadEntry[3] != 0xF) {
        		currentKeypadEntry[0] = BLANK_CODE[0];
        		currentKeypadEntry[1] = BLANK_CODE[1];
        		currentKeypadEntry[2] = BLANK_CODE[2];
        		currentKeypadEntry[3] = BLANK_CODE[3];
        	}  // CDL=> Explain this logic better? Unclear at first glance

        	for(int i = 0; i < 15000000; i++){}  // CDL=> Make explicit delay function

            // Add to currentKeyPadEntry
            addNewKeypadEntry();

            // Set seven segment display to currentKeyPadEntry
            displayCurrentEntry();  // CDL=> Maybe just called when entry is updated in addNewKeypadEntry since should
                                    // only happen when that updates?

            // Check if four digits have been entered
            if (currentKeypadEntry[0] != 0xF && currentKeypadEntry[1] != 0xF &&
                currentKeypadEntry[2] != 0xF && currentKeypadEntry[3] != 0xF)  // CDL=> Maybe use an index instead?
            {
               // CDL=> LED logic should be abstracted out into led function (flashStatusLED(COLOR (green or red)) with
               // built in case statement to | with current mode led)
               // make it go (on, delay, off, delay, on, delay, off, delay) (0.5-1 second total time to be noticeable)
               switch (currentMode)
               {
                    case MODE_1_CHECK_CODE:
                        if (checkForMasterCode() | checkForExistingCode())
                        {
                            // Flash green status led
                            AXILAB_SLAVE_LED_mWriteReg(RGB_LED_BASE_ADDR, 0,
                                                        LED_0_BLUE_MASK |
                                                        LED_1_GREEN_MASK);

                            for(int i = 0; i < 10000000; i++){}  // delay // CDL=> Make explicit delay function

                            AXILAB_SLAVE_LED_mWriteReg(RGB_LED_BASE_ADDR, 0,
                                                        LED_0_BLUE_MASK);
                        }
                        else
                        {
                            // Flash red status led
                            AXILAB_SLAVE_LED_mWriteReg(RGB_LED_BASE_ADDR, 0,
                                                        LED_0_BLUE_MASK |
                                                        LED_1_RED_MASK);
                            for(int i = 0; i < 10000000; i++){} // CDL=> Make explicit delay function
                            AXILAB_SLAVE_LED_mWriteReg(RGB_LED_BASE_ADDR, 0,
                                                        LED_0_BLUE_MASK);
                        }
                        break;
                    case MODE_2_SET_CODE:
                        if (!checkForMasterCode() && !checkForExistingCode())
                        {
                            // Flash green status led
                            AXILAB_SLAVE_LED_mWriteReg(RGB_LED_BASE_ADDR, 0,
                                                       LED_0_YELLOW_MASK |
                                                       LED_1_GREEN_MASK);
                            for(int i = 0; i < 10000000; i++){}  // delay // CDL=> Make explicit delay function

                            AXILAB_SLAVE_LED_mWriteReg(RGB_LED_BASE_ADDR, 0,
                                                       LED_0_YELLOW_MASK);

                            // Add code to stored codes
                            storeNewPin();
                        }
                        else
                        {
                            // Flash red status led
                            AXILAB_SLAVE_LED_mWriteReg(RGB_LED_BASE_ADDR, 0,
                                                       LED_0_YELLOW_MASK |
                                                       LED_1_RED_MASK);

                            for(int i = 0; i < 10000000; i++){}  // delay // CDL=> Make explicit delay function

                            AXILAB_SLAVE_LED_mWriteReg(RGB_LED_BASE_ADDR, 0,
                                                       LED_0_YELLOW_MASK);
                        }
                        break;
                    case MODE_3_GET_CODE:
                        if (!checkForMasterCode() && checkForExistingCode())
                        {
                            // Flash green status led
                            AXILAB_SLAVE_LED_mWriteReg(RGB_LED_BASE_ADDR, 0,
                                                       LED_0_PURPLE_MASK |
                                                       LED_1_GREEN_MASK);

                            for(int i = 0; i < 10000000; i++){}  // delay // CDL=> Make explicit delay function

                            AXILAB_SLAVE_LED_mWriteReg(RGB_LED_BASE_ADDR, 0,
                                                       LED_0_PURPLE_MASK);

                            // Remove entered code from stored codes
                            removePin();
                        }
                        else
                        {
                            // Flash red status led
                            AXILAB_SLAVE_LED_mWriteReg(RGB_LED_BASE_ADDR, 0,
                                                       LED_0_PURPLE_MASK |
                                                       LED_1_RED_MASK);

                            for(int i = 0; i < 10000000; i++){}  // delay // CDL=> Make explicit delay function

                            AXILAB_SLAVE_LED_mWriteReg(RGB_LED_BASE_ADDR, 0,
                                                       LED_0_PURPLE_MASK);
                        }
                        break;
                    default:
                        break;
               }
            }
        }

    }

    // Shutdown UART interface
    cleanup_platform();

    // Return with no errors
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
            setMode(MODE_2_SET_CODE);    // Change mode to check mode
            break;
        case MODE_2_SET_CODE:
            setMode(MODE_3_GET_CODE);    // Change mode to set mode
            break;
        case MODE_3_GET_CODE:
            setMode(MODE_1_CHECK_CODE);  // Change mode to remove mode
            break;
        default:
            setMode(DEFAULT_MODE);
            break;
    }
}

/*
 * This function sets the current mode, changes mode led, and
 * resets current entry code.
 *
 * Return: None (void)
 */
void setMode(Mode mode)
{
    // Set the current mode
    currentMode = mode;

    // Set the mode LED to the current mode color
    setModeLED();

    // Reset the current code // CDL=> Display afterwards
    currentKeypadEntry[0] = BLANK_CODE[0];  // CDL=> Move to clearEntry method
    currentKeypadEntry[1] = BLANK_CODE[1];
    currentKeypadEntry[2] = BLANK_CODE[2];
    currentKeypadEntry[3] = BLANK_CODE[3];
    currentKeypadEntryIndex = 0;
}

/*
 * This function sets the LED color for the current mode.
 *
 * Return: None (void)
 */
void setModeLED()
{
    switch (currentMode)
    {
        // CDL=> Abstract to setLEDs() method
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
 * This function determines if the reset button has been pressed.
 *
 * Return: bool: true if reset button has been pressed,
 *               false if reset button has not been pressed
 */
bool isResetButtonPressed()
{
    // CDL=> If statement not needed (simply return the value of the conditional)
    // return (AXILAB_SLAVE_BUTTON_mReadReg(ON_BOARD_PUSH_BASE_ADDR, 0) &
    //         RESET_BUTTON_MASK)
    if (AXILAB_SLAVE_BUTTON_mReadReg(ON_BOARD_PUSH_BASE_ADDR, 0) &
        RESET_BUTTON_MASK)
    {
        return true;
    }
    return false;
}

/*
 * This function determines if the mode button has been pressed.
 *
 * Return: bool: true if mode button has been pressed,
 *               false if mode button has not been pressed
 */
bool isModeButtonPressed()
{
    // CDL=> If statement not needed (simply return the value of the conditional)
    if (AXILAB_SLAVE_BUTTON_mReadReg(ON_BOARD_PUSH_BASE_ADDR, 0) &
        MODE_BUTTON_MASK)
    {
        return true;
    }
    return false;
}

/*
 * This function determines if the keypad button has been pressed.
 *
 * Return: bool: true if keypad button has been pressed,
 *               false if keypad button has not been pressed
 */
bool isKeypadPressed()
{
    // CDL=> Add check for any non 0-9 buttons or move to vhdl driver?
    // CDL=> If statement not needed (simply return the value of the conditional)
    if (KEYPAD_BINARY_SLAVE_mReadReg(KEYPAD_BASE_ADDR, 0) != 0xF)
    {
        return true;
    }
    return false;
}

/*
 * This function adds a new keypad entered digit to the currentKeypadEntry.
 *
 * Return: None (void)
 */
void addNewKeypadEntry()
{
    // CDL=> This whole method could be commentted better to describe what each step is doing
    u32 data1 = KEYPAD_BINARY_SLAVE_mReadReg(KEYPAD_BASE_ADDR, 0);
    // CDL=> Add check for any non 0-9 buttons or move to vhdl driver?
    if (data1 != 0xF && currentKeypadEntry[3] != 0xF)
    {
        currentKeypadEntry[0] = data1;
        currentKeypadEntry[1] = BLANK_CODE[1];
        currentKeypadEntry[2] = BLANK_CODE[2];
        currentKeypadEntry[3] = BLANK_CODE[3];
    }
    else
    {
        for (int i = 3; i >= 0; i--)
        {
            if (data1 != 0xF)
            {
                if (i == 0)
                {
                    currentKeypadEntry[0] = data1 & 0xF;
                }
                else
                {
                    currentKeypadEntry[i] = currentKeypadEntry[i - 1];
                }
            }

        }
    }
}

/*
 * This function displays the current entry code to the display.
 *
 * Return: None (void)
 */
void displayCurrentEntry()
{
    // Write the current entry code to the display
	SEVEN_SEGMENT_DISPLAY_SLAVE_mWriteReg(SEVEN_SEGMENT_BASE_ADDR, 0,
                                          currentKeypadEntry[3] << 12 |
                                          currentKeypadEntry[2] << 8 |
                                          currentKeypadEntry[1] << 4 |
                                          currentKeypadEntry[0]);
}

/*
 * This function checks if the current entry code is the master code.
 *
 * Return: bool: returns true if currentKeypadEntry is the master code,
 *              returns else otherwise
 */
bool checkForMasterCode()
{
	return ((currentKeypadEntry[0] == MASTER_CODE[0]) &&
            (currentKeypadEntry[1] == MASTER_CODE[1]) &&
            (currentKeypadEntry[2] == MASTER_CODE[2]) &&
            (currentKeypadEntry[3] == MASTER_CODE[3]));
}

/*
 * This function checks if the current entry code is a stored code.
 *
 * Return: bool: returns true if currentKeypadEntry is a stored code
 *               returns else otherwise
 */
bool checkForExistingCode()
{
	for (int i = 0; i < MAX_NUM_STORED_CODES; i++)
    {
		if ((currentKeypadEntry[0] == storedCodes[i][0]) &&
            (currentKeypadEntry[1] == storedCodes[i][1]) &&
            (currentKeypadEntry[2] == storedCodes[i][2]) &&
            (currentKeypadEntry[3] == storedCodes[i][3]))
        {
			return true;
		}
	}
	return false;
}

/*
 * This function stores a newly entered four digit pin.
 *
 * Return: None (void)
 */
void storeNewPin()
{
    // CDL=> This whole method could be commentted better to describe what each step is doing
    // CDL=> Could return a boolean in case array is full (index at max), or code in array
	if (currentKeypadEntry[3] != 0xF)
	{
		// CDL=> Why setting 0 index of stored codes?
        storedCodes[0][0] = currentKeypadEntry[0];
	    storedCodes[0][1] = currentKeypadEntry[1];
		storedCodes[0][2] = currentKeypadEntry[2];
		storedCodes[0][3] = currentKeypadEntry[3];
		for (int i = 0; i < MAX_NUM_STORED_CODES; i++)
		{
			if (storedCodes[i][0] == 0xF)
			{
				storedCodes[i][0] = currentKeypadEntry[0];
				storedCodes[i][1] = currentKeypadEntry[1];
				storedCodes[i][2] = currentKeypadEntry[2];
				storedCodes[i][3] = currentKeypadEntry[3];
				break;
			}
		}
	}
}

/*
 * This function removes a previously entered four digit pin.
 *
 * Return: None (void)
 */
void removePin()
{
    // CDL=> This whole method could be commentted better to describe what each step is doing
    // CDL=> Could return a boolean in case array is empty (index at 0), or code not in array
	for (int i = 0; i < MAX_NUM_STORED_CODES; i++)
    {
		if ((storedCodes[i][0] == currentKeypadEntry[0]) &&
            (storedCodes[i][1] == currentKeypadEntry[1]) &&
            (storedCodes[i][2] == currentKeypadEntry[2]) &&
            (storedCodes[i][3] == currentKeypadEntry[3]))
        {
			storedCodes[i][0] = 0xF;
		    storedCodes[i][1] = 0xF;
			storedCodes[i][2] = 0xF;
			storedCodes[i][3] = 0xF;
	    }
	}
}
