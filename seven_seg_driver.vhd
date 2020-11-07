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

  -- digit selection input
  DIGIT_VALUES_IN    : in std_logic_vector(15 downto 0);    -- gives the values of the digits to be illuminated
                                                            -- bits 0-3: digit 1; bits 4-7: digit 2, bits 8-11: digit 3
                                                            -- bits 12-15: digit 4

  -- seven segment display digit selection port
  GND_CTRL_VEC    : out std_logic_vector(3 downto 0);       -- if bit is 1 then digit is activated and if bit is 0 digit is inactive
                                                            -- bits 0-3: digit 1; bits 3-7: digit 2, bit 7: digit 4

  -- seven segment display segment selection port
  SEG_SELECT      : out std_logic_vector(6 downto 0)        -- if bit is 1 then segment is illuminated else if bit is 0 then segment is dim
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

  -- FSM for digit position when refreshing
  type REFRESHED_DIGIT is (DIGIT_1, DIGIT_2, DIGIT_3, DIGIT_4);

  -- stores digit this is currently illuminated
  signal current_digit     : REFRESHED_DIGIT;

  -- counter for digit refresh
  signal digit_refresh_counter : unsigned(16 downto 0) := "00000000000000000";

  -- digit selection signal
  signal digit_select          : std_logic_vector(3 downto 0);

  -- value of the digit being displayed
  signal digit_value           : std_logic_vector(3 downto 0);

  -- segment selection signal
  signal segment_select    : std_logic_vector(6 downto 0);


begin


------------------------------------------------------------------------------
  -- Process Name     : REFRESH_DIGITS
  -- Sensitivity List : I_CLK_100MHZ    : 100 MHz global clock
  --
  -- Useful Outputs   : segment_select : Gives the segment section that is to be illuminated
  --                  : digit_select : selects the digit to illuminate
  --                    (active high enable logic)
  -- Description      : illuminates the desired segment and digit that is to be displayed
  ------------------------------------------------------------------------------
  REFRESH_DIGITS : process (I_CLK_100MHZ, DIGIT_VALUES_IN)
  begin
      if (rising_edge(I_CLK_100MHZ)) then
          if(digit_refresh_counter = "11000011010100000") then
              case current_digit is
                  when DIGIT_1 =>
                      current_digit <= DIGIT_2;  -- set current digit to next digit
                      digit_select  <= "0010";    -- illuminate next digit (digit 2)
                      digit_value   <= DIGIT_VALUES_IN(7 downto 4);
              when DIGIT_2 =>
                  current_digit <= DIGIT_3;                        -- set current digit to next digit
                  digit_select <= "0100";                          -- illuminate next digit (digit 2)
                  digit_value <= DIGIT_VALUES_IN(11 downto 8);  -- set digit 3 values
              when DIGIT_3 =>
                  current_digit <= DIGIT_4;                      -- set current digit to next digit
                  digit_select <= "1000";                        -- illuminate next digit (digit 2)
                  digit_value <= DIGIT_VALUES_IN(15 downto 12);  -- get digit 4 values
              when DIGIT_4 =>
                  current_digit <= DIGIT_1;                    -- set current digit to next digit
                  digit_select <= "0001";                      -- illuminate next digit (digit 2)
                  digit_value <= DIGIT_VALUES_IN(3 downto 0);  -- set set of next digits to display
              when others =>
                  current_digit <= current_digit;
                  digit_value <= digit_value;
          end case;
              digit_refresh_counter <= (others => '0');  -- reset counter
          end if;

          digit_refresh_counter <= digit_refresh_counter + 1;
      end if;

  end process REFRESH_DIGITS;
  ------------------------------------------------------------------------------

  DISPLAY_DIGITS : process(I_CLK_100MHZ)
      begin
         if(rising_edge(I_CLK_100MHZ)) then
             case digit_value is
                 when "0000" =>
                     segment_select <= "0000000";
                 when "0001" =>
                     segment_select <= "0000110";
                 when "0010" =>
                     segment_select <= "1011011";
                 when "0011" =>
                     segment_select <= "1001111";
                 when "0100" =>
                     segment_select <= "1100110";
                 when "0101" =>
                     segment_select <= "1101101";
                 when "0110" =>
                     segment_select <= "1111101";
                 when "0111" =>
                     segment_select <= "0000111";
                 when "1000" =>
                     segment_select <= "1111111";
                 when "1001" =>
                     segment_select <= "1100111";
                 when others =>
                     segment_select <= segment_select;
             end case;
         end if;
  end process DISPLAY_DIGITS;

  -- send signals to output ports
  GND_CTRL_VEC <= digit_select;
  SEG_SELECT   <= segment_select;

end architecture rtl;
