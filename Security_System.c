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
        else if (keypad key is pressed)
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
    setModeLED();

    // Reset the current code
    currentKeypadEntry = BLANK_CODE;
    currentKeypadEntryIndex = 0;
}