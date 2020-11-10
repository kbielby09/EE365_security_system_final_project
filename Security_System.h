/*  Security_System.h
 *  Authors: Kyle Bielby, Christopher Lloyd
 *  Date Created: 11/10/2020
 *  Description: This header file contains the function headers used in the
 *  Security_System.c main file.
 */

 // TODO place macros in header file

/*
 *  This funciton changes the mode of operation when the mode button is
 *  is pressed
 *
 *  Returns: void
 */
void toggleMode();

/*
 *  Sets the mode of operation and changes the corresponding on board LED
 *
 *  Returns: void
 */
void setMode(Mode mode);

/*
 *  Sets LED color for mode of operation
 *
 *  Returns: void
 */
void setModeLED();

/*
 *  Determines if reset button has been pressed
 *
 *  Returns:
 * - bool: returns true if reset button register contains a value other than
 *         default
 */
bool isResetButtonPressed();

/*
 *  Determines if mode button has been pressed
 *
 *  Returns:
 * - bool: returns true if mode button register contains a value other than the
 *         default returns false if mode button register contains default values
 */
bool isModeButtonPressed();

/*
 *  Determines if keypad has been pressed
 *
 *  Returns:
 * - bool: returns true if keypad register contains a value that is non empty
 *         returns false if empty
 */
bool isKeypadPressed();

/*
 *  add a new keypad entry to current keypad entry
 *
 *  Returns: void
 */
void addNewKeypadEntry();

/*
 *  displays current pin entry to seven segment display
 *
 *  Returns: void
 */
void displayCurrentEntry();

/*
 *  checks if code is equal to master code
 *
 *  Returns:
 *  - bool: returns true if entered code is the equal to the master code
 *          otherwise returns false
 */
bool checkForMasterCode();

/*
 *  checks if code is already stored in memory
 *
 *  Returns:
 *  - bool: returns true if enterd pin is already present in memory otherwise
 *          returns false if entered pin is not contained in memory
 */
bool checkForExistingCode();

/*
 *  stores newly entered pin number
 *
 *  Returns: void
 */
void storeNewPin();

/*
 *  Sets the mode of operation and changes the corresponding on board LED
 *
 *  Returns: void
 */
void removePin();
