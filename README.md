# Multi-mode simple security system Implemented on a Cora Z7-10 Development Board.

This project includes the Software and Hardware for a final project for
EE365 (Advanced Digital Logic Design) at Clarkson University.

At a high level, this system is used for verifying 4 digit
passcodes (0-9) mimicking some sort of authentication system.
There is also functionality to store and remove passcodes.

It has three core modes (indicated by onboard LED_0):

- MODE_1_CHECK_CODE (Led color: Blue)
Allows a user to enter a passcode and provides feedback
      indicating whether the passcode is valid.

- MODE_2_SET_CODE (Led color: Yellow)
Allows a user to enter a passcode and provides feedback
      indicating whether the passcode was stored.

- MODE_3_REMOVE_CODE (Led color: Purple)
Allows a user to enter a passcode and provides feedback
      indicating whether the passcode was removed.

To indicate whether an operation completed successfully
or not, an onboard pushbutton will flash either green or red.

Digit Input is through a matrix keypad being controlled in
firmware. This provides a stream of 4-bit data indicating
what button is pressed (0-9 only) with all other keys and
no key pressed indicated by 0xF.

Passcode output is through a 4-digit seven segment display
also being controlled in firmware. To drive the display, a
16-bit number is written to the display register with
the 4 nibbles corresponding to the 4 digits. Once again,
0-9 only with 0xF being a blank digit.
