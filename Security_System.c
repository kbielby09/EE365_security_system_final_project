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
#defibe LED_1_PURPLE_MASK LED_1_BLUE_MASK | LED_1_RED_MASK
#defibe LED_1_YELLOW_MASK LED_1_GREEN_MASK | LED_1_RED_MASK

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

// Determines if button has reset button has been pressed
bool isResetButtonPressed();

// Get keypad entries
void getKeypadEntries();

#define CODE_LENGTH 4
#define MAX_NUM_STORED_CODES 100

// Empty code for all digits blank
const uint8_t BLANK_CODE[CODE_LENGTH] = {0xF,0xF,0xF,0xF};

// Master code for system (cannot be changed)
const uint8_t MASTER_CODE[CODE_LENGTH] = {0,0,0,0};

// The current mode of the program
modes currentMode;

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

    // Set the current mode
    setMode(DEFAULT_MODE);

    // Hex value of key press
    uint8_t keyHex;

    while (1) // Main program execution loop
    {
        if (resetbtn is pressed)
        {
            // Set the current mode
            setMode(DEFAULT_MODE);

            // Clear any stored codes // CDL=> Move to method?
            memset(storedCodes, 0, sizeof(storedCodes[0][0]) * MAX_NUM_STORED_CODES * CODE_LENGTH);
        }
        else if (modeToggleBtn is pressed)
        {
            // Toggle the current mode and reset code
            toggleMode();
        }
        else if (keypad key is pressed) {

        }
        {

            // add to currentKeyPadEntry
            // set seven segment display to currentKeyPadEntry

            if (currentKeyPadEntry is complete)
            {
                switch (currentMode)
                {
                    case checkState:
                        // Set led1 (mode led) to color of currentMode
                        if (code == masterCode or code in codes[])
                        {
                            // flash green leds
                        }
                        else
                        {
                            // flash red leds
                        }
                        break;
                    case setState:
                        // Set led1 (mode led) to color of currentMode
                        if (code != masterCode and code not in codes[])
                        {
                            // flash green leds and add code to codes[]
                        }
                        else
                        {
                            // flash red leds
                        }
                        break;
                    case deleteState:
                        // Set led1 (mode led) to color of currentMode
                        if (code != masterCode and code in codes[])
                        {
                            // flash green leds and remove code from codes[]
                        }
                        else
                        {
                            // flash red leds
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
    // Set the current mode
    currentMode = mode

    // Set the mode LED to the current mode color
    setModeLED(mode);

    // Reset the current code
    currentKeypadEntry = BLANK_CODE;
    currentKeypadEntryIndex = 0;
}

/*
 * This function sets the current mode, changes mode led, and
 * resets current code.
 *
 * Return: None (void)
 */
void setModeLED() {
  switch (currentMode) {
    case MODE_1_CHECK_CODE:
        AXILAB_SLAVE_LED_mWriteReg(RGB_LED_BASE_ADDR, 0, LED_1_BLUE_MASK);
    case MODE_2_SET_CODE:
        AXILAB_SLAVE_LED_mWriteReg(RGB_LED_BASE_ADDR, 0, LED_1_YELLOW_MASK);
    case MODE_3_GET_CODE:
        AXILAB_SLAVE_LED_mWriteReg(RGB_LED_BASE_ADDR, 0, LED_1_PURPLE_MASK);
    default:
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
