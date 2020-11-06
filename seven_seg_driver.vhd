--------------------------------------------------------------------------------
-- Filename     : seven_seg_driver.vhd
-- Author       : Kyle Bielby
-- Date Created : 2020-11-05
-- Last Revised : 2020-11-05
-- Project      : seven_seg_driver
-- Description  : driver code that displays digit on the seven segment display
--------------------------------------------------------------------------------
-- Todos:
--
--
--------------------------------------------------------------------------------

-----------------
--  Libraries  --
-----------------
library ieee;
  use ieee.std_logic_1164.all;
  use ieee.numeric_std.all;

--------------
--  Entity  --
--------------
entity seven_seg_driver is
port
(
  -- Clocks & Resets
  I_CLK_100MHZ    : in std_logic;
  I_SYSTEM_RST    : in std_logic;

  -- digit selection input
  DIGIT_IN        : in std_logic_vector(3 downto 0);    -- gives the digit to be illuminated
                                                              -- bit 0: digit 1; bit 1: digit 2, bit 3: digit

  -- digit selection input;
  DIGIT_VALUE_IN   : in std_logic_vector(3 downto 0);

  -- seven segment display digit selection port
  GND_CTRL_VEC    : out std_logic_vector(3 downto 0);    -- if bit is 1 then digit is activated and if bit is 0 digit is inactive
                                                         -- bit 0: digit 1; bit 1: digit 2, bit 3: digit 4

  -- seven segment display segment selection port
  SEG_SELECT      : out std_logic_vector(6 downto 0)    -- if bit is 1 then segment is illuminated else if bit is 0 then segment is dim
                                                        -- bit 0: sement A, bit 1: segment B, bit 2: segment C, bit 3: segment D,
                                                        -- bit 4: segment E, bit 5: segment F, bit 6: segment G
  );
end seven_seg_driver;


--------------------------------
--  Architecture Declaration  --
--------------------------------
architecture rtl of seven_seg_driver is

  -------------
  -- SIGNALS --
  -------------

  -- segment selection signal
  signal segment_select    : std_logic_vector(6 downto 0);

  -- digit selection signal
  signal digit_select      : std_logic_vector(3 downto 0);

begin
------------------------------------------------------------------------------
  -- Process Name     : KEYPAD_EN_CNTR
  -- Sensitivity List : I_CLK_100MHZ    : 100 MHz global clock
  --                    DIGIT_VALUE_IN  : digit value to be displayed
  --                    DIGIT_IN        : the digit number to be illuminated
  -- Useful Outputs   : segment_select : Gives the segment section that is to be illuminated
  --                  : digit_select : selects the digit to illuminate
  --                    (active high enable logic)
  -- Description      : illuminates the desired segment and digit that is to be displayed
  ------------------------------------------------------------------------------
  LIGHT_DIGIT : process (I_CLK_100MHZ, DIGIT_VALUE_IN, DIGIT_IN)
  begin
      if (rising_edge(I_CLK_100MHZ)) then
          -- check digit value
          digit_select <= DIGIT_IN;
          case DIGIT_VALUE_IN is
              when "0001" =>    -- digit is 1
                  segment_select <= "0000110";
              when "0010" =>    -- digit is 2
                  segment_select <= "1011011";
              when "0011" =>    -- digit is 3
                  segment_select <= "1001111";
              when "0100" =>    -- digit is 4
                  segment_select <= "1100110";
              when "0101" =>    -- digit is 5
                  segment_select <= "1101101";
              when "0110" =>    -- digit is 6
                  segment_select <= "1111101";
              when "0111" =>    -- digit is 7
                  segment_select <= "0000111";
              when "1000" =>    -- digit is 8
                  segment_select <= "11111111";
              when "1001" =>    -- digit is 9
                  segment_select <= "1100111";
          end case;

      end if;

  end process LIGHT_DIGIT;
  ------------------------------------------------------------------------------

  -- send signals to output ports
  GND_CTRL_VEC <= digit_select;
  SEG_SELECT   <= segment_select;

end architecture rtl;
