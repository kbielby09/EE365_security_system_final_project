/* -----------------------------------------------------------------------------
 * Filename     : Security_System.c
 * Author(s)    : Kyle Bielby, Chris Lloyd (Team 1)
 * Class        : EE365 (Final Project)
 * Due Date     : 2020-11-23
 * Target Board : Cora Z7-10
 * Description  : Multi-mode simple security system.
 *
 *                This file comprises
 *                the software for a final project for
 *                EE365 (Advanced Digital Logic Design) at Clarkson University.
 *
 *                At a high level, this system is used for verifying 4 digit
 *                passcodes (0-9) mimicking some sort of authentication system.
 *                There is also functionality to store and remove passcodes.
 *
 *                It has three core modes (indicated by onboard LED_0):
 *                <> MODE_1_CHECK_CODE (Led color: Blue)
 *                  -> Allows a user to enter a passcode and provides feedback
 *                     indicating whether the passcode is valid.
 *
 *                <> MODE_2_SET_CODE (Led color: Yellow)
 *                  -> Allows a user to enter a passcode and provides feedback
 *                     indicating whether the passcode was stored.
 *
 *                <> MODE_3_REMOVE_CODE (Led color: Purple)
 *                  -> Allows a user to enter a passcode and provides feedback
 *                     indicating whether the passcode was removed.
 *
 *                To indicate whether an operation completed successfully
 *                or not, an onboard pushbutton will flash either green or red.
 *
 *                Digit Input is through a matrix keypad being controlled in
 *                firmware. This provides a stream of 4-bit data indicating
 *                what button is pressed (0-9 only) with all other keys and
 *                no key pressed indicated by 0xF.
 *
 *                Passcode output is through a 4-digit seven segment display
 *                also being controlled in firmware. To drive the display, a
 *                16-bit number is written to the display register with
 *                the 4 nibbles corresponding to the 4 digits. Once again,
 *                0-9 only with 0xF being a blank digit.
 *
 * -------------------------------------------------------------------------- */

// Includes
#include <stdio.h>
#include <stdbool.h>
#include "xil_cache.h"
#include "keypad_binary_slave.h"
#include "seven_segment_display_slave.h"
#include "axilab_slave_button.h"
#include "axilab_slave_led.h"
#include "xil_io.h"

// Masks for onboard push buttons
#define BUTTON_0_MASK 1
#define BUTTON_1_MASK 2
#define RESET_BUTTON_MASK BUTTON_1_MASK
#define MODE_BUTTON_MASK  BUTTON_0_MASK

// Masks for individual colors of each onboard led
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
#define ONBOARD_PUSH_BASE_ADDR  0x43c10000
#define SEVEN_SEGMENT_BASE_ADDR 0x43c20000
#define RGB_LEDS_BASE_ADDR      0x43c30000

/*******************************************************************************
 * Mode related functionality
 ******************************************************************************/

// An enum to define the operating modes (states) of the program
typedef enum
{
    MODE_1_CHECK_CODE  = 0x1,
    MODE_2_SET_CODE    = 0x2,
    MODE_3_REMOVE_CODE = 0x3
} Mode;
#define DEFAULT_MODE MODE_1_CHECK_CODE

// The current mode of the program
Mode currentMode;

// Toggles the current mode of operation
void toggleMode();

// Sets the current mode of operation
void setMode(Mode mode);

/*******************************************************************************
 * Passcode related functionality
 ******************************************************************************/

#define PASSCODE_LENGTH 4
#define MAX_NUM_STORED_PASSCODES 100

// Master passcode for system (cannot be changed)
const uint8_t MASTER_PASSCODE[PASSCODE_LENGTH] = {0,0,0,0};

// Location to store valid passcodes
uint8_t storedPasscodes[MAX_NUM_STORED_PASSCODES][PASSCODE_LENGTH];
uint8_t currentStoredPasscodesIndex;

// A location to store the current keypad entry (0xF results in a blank digit)
uint8_t currentPasscode[PASSCODE_LENGTH];
uint8_t currentPasscodeIndex;

// Clears and resets storedPasscodes
void resetStoredPasscodes();

// Clears and resets currentPasscode
void resetCurrentPasscode();

// Adds the current passcode to storedPasscodes
bool storePasscode(uint8_t passcode[]);

// Removes the current passcode from storedPasscodes
bool removePasscode(uint8_t passcode[]);

// Add a digit to currentPasscode
bool storeCurrentPasscodeDigit(uint8_t digitData);

// Checks if passcode is equal to MASTER_PASSCODE
bool isMasterPasscode(uint8_t passcode[]);

// Checks if passcode exists in storedPasscodes
bool isExistingPasscode(uint8_t passcode[]);

// Checks if storedPasscodes is full
bool isStoredPasscodesFull();

// Checks if currentPasscode is complete
bool isCurrentPasscodeComplete();

/*******************************************************************************
 * Onboard LED related functionality
 ******************************************************************************/

// Sets the leds
void setLEDS(uint8_t ledData);

// Sets mode LED color for current mode of operation
void setModeLED();

// Flashes the status led a certain color
void flashStatusLED(uint8_t statusColor);

/*******************************************************************************
 * Miscellaneous functionality
 ******************************************************************************/
bool previousResetButtonState = false;

// Determines if reset button is pressed
bool isResetButtonPressed();

// Determines if reset button was released
bool isResetButtonReleased();

// Determines if mode button has been pressed
bool isModeButtonPressed();

// Determines if keypad has been pressed
bool isKeypadPressed();

// Gets the current keypad key pressed
uint8_t getKeypadValue();

// Displays code to seven segment display
void displayPasscode(uint8_t passcode[]);

// Delay for ms milliseconds
void delayMS(uint16_t ms);

// Reset the system
void resetSystem();

// Clear all outputs
void clearOutputs();

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
    // Reset passcodes and current mode
    resetSystem();

    while (true)  // Main program execution loop
    {
        if (isResetButtonPressed())  // Is reset button being held down?
        {
            clearOutputs();  // Clear all outputs
        }

        if (isResetButtonReleased())  // Is reset button being released (falling edge)?
        {
            resetSystem();                     // Reset passcodes and mode
            delayMS(250);                      // Delay program
            flashStatusLED(LED_1_GREEN_MASK);  // Flash green status led
        }
        else if (isModeButtonPressed())  // Is mode button being pressed?
        {
            toggleMode();  // Toggle the current mode and reset passcode
            delayMS(500);  // Delay 500 ms
        }
        else if (isKeypadPressed())  // Is a key on keypad being pressed?
        {
            // Add to currentPasscode
            storeCurrentPasscodeDigit(getKeypadValue());

            // Delay program to prevent same press being registered constantly
            delayMS(450);

            // Check if full passcode has been entered
            if (isCurrentPasscodeComplete())
            {
               switch (currentMode)
               {
                    case MODE_1_CHECK_CODE:
                        if (isMasterPasscode(currentPasscode) ||
                            isExistingPasscode(currentPasscode))
                        {
                            // Flash green status led
                            // (Indicating passcode valid)
                            flashStatusLED(LED_1_GREEN_MASK);
                        }
                        else
                        {
                            // Flash red status led
                            // (Indicating passcode invalid)
                            flashStatusLED(LED_1_RED_MASK);
                        }
                        break;
                    case MODE_2_SET_CODE:
                        if (!isMasterPasscode(currentPasscode) &&
                            !isExistingPasscode(currentPasscode) &&
                            !isStoredPasscodesFull())
                        {
                            // Flash green status led
                            // (Indicating passcode stored)
                            flashStatusLED(LED_1_GREEN_MASK);
                            storePasscode(currentPasscode);
                        }
                        else
                        {
                            // Flash red status led
                            // (Indicating passcode not stored)
                            flashStatusLED(LED_1_RED_MASK);
                        }
                        break;
                    case MODE_3_REMOVE_CODE:
                        if (!isMasterPasscode(currentPasscode) &&
                            isExistingPasscode(currentPasscode))
                        {
                            // Flash green status led
                            // (Indicating passcode removed)
                            flashStatusLED(LED_1_GREEN_MASK);
                            removePasscode(currentPasscode);
                        }
                        else
                        {
                            // Flash red status led
                            // (Indicating passcode not removed)
                            flashStatusLED(LED_1_RED_MASK);
                        }
                        break;
                    default:
                        break;
               }
               resetCurrentPasscode();  // Reset current passcode
            }
        }
    }

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
            setMode(MODE_3_REMOVE_CODE);    // Change mode to set mode
            break;
        case MODE_3_REMOVE_CODE:
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

    // Reset currentPasscode to null values of 0xF
    resetCurrentPasscode();
}

/*
 * This function resets storedPasscodes.
 *
 * Return: None (void)
 */
void resetStoredPasscodes()
{
    // Clear any stored passcodes and reset index
    memset(storedPasscodes, 0xF, sizeof(storedPasscodes[0][0]) *
                                 MAX_NUM_STORED_PASSCODES * PASSCODE_LENGTH);
    currentStoredPasscodesIndex = 0;
}

/*
 * This function resets currentPasscode.
 *
 * Return: None (void)
 */
void resetCurrentPasscode()
{
    // Clear current passcode
    memset(currentPasscode, 0xF, sizeof(currentPasscode[0]) * PASSCODE_LENGTH);
    currentPasscodeIndex = 0;

    // Display the current passcode to the seven segment display
    displayPasscode(currentPasscode);
}

/*
 * This function stores passcode to storedPasscodes.
 *
 * Param: passcode: The passcode to store.
 * Return: (bool): Passcode stored successfully?
 */
bool storePasscode(uint8_t passcode[])
{
    // Ensure storedPasscodes is not full
    if (isStoredPasscodesFull()) { return false; }

    // Add passcode and increment index
    storedPasscodes[currentStoredPasscodesIndex][0] = passcode[0];
    storedPasscodes[currentStoredPasscodesIndex][1] = passcode[1];
    storedPasscodes[currentStoredPasscodesIndex][2] = passcode[2];
    storedPasscodes[currentStoredPasscodesIndex][3] = passcode[3];
    currentStoredPasscodesIndex++;

    return true;
}

/*
 * This function removes passcode from storedPasscodes.
 *
 * Param: passcode: The passcode to remove.
 * Return: (bool): Passcode removed successfully?
 */
bool removePasscode(uint8_t passcode[])
{
    // Ensure passcode in storedPasscodes
    if (!isExistingPasscode(passcode)) { return false; }

    // Find passcode index
    int i = 0;
    for (; i < currentStoredPasscodesIndex; i++)
    {
        if ((passcode[0] == storedPasscodes[i][0]) &&
            (passcode[1] == storedPasscodes[i][1]) &&
            (passcode[2] == storedPasscodes[i][2]) &&
            (passcode[3] == storedPasscodes[i][3]))
        {
            break;
        }
    }

    // Shift all elements back and remove passcode
    for (; i < currentStoredPasscodesIndex - 1; i++)
    {
        storedPasscodes[i][0] = storedPasscodes[i + 1][0];
        storedPasscodes[i][1] = storedPasscodes[i + 1][1];
        storedPasscodes[i][2] = storedPasscodes[i + 1][2];
        storedPasscodes[i][3] = storedPasscodes[i + 1][3];
    }

    // Blank out last code
    currentStoredPasscodesIndex--;
    storedPasscodes[currentStoredPasscodesIndex][0] = 0xF;
    storedPasscodes[currentStoredPasscodesIndex][1] = 0xF;
    storedPasscodes[currentStoredPasscodesIndex][2] = 0xF;
    storedPasscodes[currentStoredPasscodesIndex][3] = 0xF;

    return true;
}

/*
 * This function stores a digit to currentPasscode.
 *
 * Param: digitData: The digit to store.
 * Return: (bool): Digit stored successfully?
 */
bool storeCurrentPasscodeDigit(uint8_t digitData)
{
    // Ensure passcode is not complete
    if (isCurrentPasscodeComplete()) { return false; }

    // Store passcode
    currentPasscode[currentPasscodeIndex++] = (digitData & 0xF);

    // Display the current passcode to the seven segment display
    displayPasscode(currentPasscode);

    return true;
}

/*
 * This function checks if passcode is equal to MASTER_PASSCODE.
 *
 * Param: passcode: The passcode to check.
 * Return: (bool): passcode equals MASTER_PASSCODE?
 */
bool isMasterPasscode(uint8_t passcode[])
{
    return ((passcode[0] == MASTER_PASSCODE[0]) &&
            (passcode[1] == MASTER_PASSCODE[1]) &&
            (passcode[2] == MASTER_PASSCODE[2]) &&
            (passcode[3] == MASTER_PASSCODE[3]));
}

/*
 * This function checks if passcode exists in storedPasscodes.
 *
 * Param: passcode: The passcode to check.
 * Return: (bool): passcode exists in storedPasscodes?
 */
bool isExistingPasscode(uint8_t passcode[])
{
    for (int i = 0; i < currentStoredPasscodesIndex; i++)
    {
        if ((passcode[0] == storedPasscodes[i][0]) &&
            (passcode[1] == storedPasscodes[i][1]) &&
            (passcode[2] == storedPasscodes[i][2]) &&
            (passcode[3] == storedPasscodes[i][3]))
        {
            return true;
        }
    }
    return false;
}

/*
 * This function checks if storedPasscodes is full.
 *
 * Return: (bool): storedPasscodes is full?
 */
bool isStoredPasscodesFull()
{
    return (currentStoredPasscodesIndex == MAX_NUM_STORED_PASSCODES);
}

/*
 * This function checks if currentPasscode is complete.
 *
 * Return: (bool): currentPasscode is complete?
 */
bool isCurrentPasscodeComplete()
{
    return (currentPasscodeIndex == PASSCODE_LENGTH);
}

/*
 * This function writes to the onboard leds.
 *
 * Param: ledData: Data to write to leds (lower 6 bits only).
 * Return: None (void)
 */
void setLEDS(uint8_t ledData)
{
    AXILAB_SLAVE_LED_mWriteReg(RGB_LEDS_BASE_ADDR, 0, (ledData & 0x3F));
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
        case MODE_1_CHECK_CODE:
            setLEDS(LED_0_BLUE_MASK);
            break;
        case MODE_2_SET_CODE:
            setLEDS(LED_0_YELLOW_MASK);
            break;
        case MODE_3_REMOVE_CODE:
            setLEDS(LED_0_PURPLE_MASK);
            break;
        default:
            break;
    }
}

/*
 * This function flashes the status led a certain color indicating the
 * status of an operation.
 *
 * Param: statusColor: The color to flash status led with.
 * Return: None (void)
 */
void flashStatusLED(uint8_t statusColor)
{
    // Ensure only led1 is being set
    statusColor = (statusColor & 0b111000);

    // Determine mode color
    uint8_t modeColor = 0;
    switch (currentMode)
    {
        case MODE_1_CHECK_CODE:
            modeColor = LED_0_BLUE_MASK;
            break;
        case MODE_2_SET_CODE:
            modeColor = LED_0_YELLOW_MASK;
            break;
        case MODE_3_REMOVE_CODE:
            modeColor = LED_0_PURPLE_MASK;
            break;
    }

    // Flash status led twice (total of 0.5 seconds)
    for (int i = 0; i < 2; i++)
    {
        setLEDS(modeColor | statusColor);  // Flash status led on
        delayMS(125);                      // Delay program
        setLEDS(modeColor);                // Flash status led off
        delayMS(125);                      // Delay program
    }
}

/*
 * This function determines if the reset button is being pressed.
 *
 * Return: (bool): Reset button is being pressed?
 */
bool isResetButtonPressed()
{
    return (AXILAB_SLAVE_BUTTON_mReadReg(ONBOARD_PUSH_BASE_ADDR, 0) &
            RESET_BUTTON_MASK);
}

/*
 * This function determines if the reset button has been released. This is
 * indicated by a falling edge on button state.
 *
 * Return: (bool): Reset button has been released?
 */
bool isResetButtonReleased()
{
    // Get whether the reset button was pressed
    bool currentResetButtonState = isResetButtonPressed();

    // Check if a falling edge has occurred
    bool fallingEdgeReset = (previousResetButtonState &&
                             !currentResetButtonState);

    // Set the previous state to the current state
    previousResetButtonState = currentResetButtonState;

    return fallingEdgeReset;
}

/*
 * This function determines if the mode button is being pressed.
 *
 * Return: (bool): Mode button is being pressed?
 */
bool isModeButtonPressed()
{
    return (AXILAB_SLAVE_BUTTON_mReadReg(ONBOARD_PUSH_BASE_ADDR, 0) &
            MODE_BUTTON_MASK);
}

/*
 * This function determines if a key on the keypad is being pressed.
 *
 * Return: (bool): Key on the keypad is being pressed?
 */
bool isKeypadPressed()
{
    return (KEYPAD_BINARY_SLAVE_mReadReg(KEYPAD_BASE_ADDR, 0) != 0xF);
}

/*
 * This function gets the keypad value that is being pressed.
 *
 * Return: (uint8_t): Digit value of keypad keypress.
 */
uint8_t getKeypadValue()
{
    // Ensure keypad is pressed
    if (!isKeypadPressed()) { return 0xF; }

    // Return value of key press
    return (KEYPAD_BINARY_SLAVE_mReadReg(KEYPAD_BASE_ADDR, 0) & 0xF);
}

/*
 * This function displays a passcode to the seven segment display.
 *
 * Param: passcode: The passcode to display.
 * Return: None (void)
 */
void displayPasscode(uint8_t passcode[])
{
    SEVEN_SEGMENT_DISPLAY_SLAVE_mWriteReg(SEVEN_SEGMENT_BASE_ADDR, 0,
                                          (passcode[0] << 12) |
                                          (passcode[1] << 8) |
                                          (passcode[2] << 4) |
                                          (passcode[3]));
}

/*
 * This function delays (blocking) by approximately (ms) milliseconds.
 *
 * Param: ms: The number of milliseconds to delay by.
 * Return: None (void)
 */
void delayMS(uint16_t ms)
{
    for (int i = 0; i < ms; i++)
    {
        for(int i = 0; i < 80000; i++) {}
    }
}

/*
 * This function clears all outputs including the onboard leds and seven
 * segment display.
 *
 * Return: None (void)
 */
void clearOutputs()
{
    setLEDS(0);
    uint8_t blankPasscode[] = {0xF, 0xF, 0xF, 0xF};
    displayPasscode(blankPasscode);
}

/*
 * This function resets the system by reseting the current and stored
 * passcode, and reseting the current mode.
 *
 * Return: None (void)
 */
void resetSystem()
{
    // Initialize storedPasscodes to null values of 0xF
    resetStoredPasscodes();

    // Initialize currentPasscode to null values of 0xF
    resetCurrentPasscode();

    // Initialize current mode to default mode
    setMode(DEFAULT_MODE);
}
