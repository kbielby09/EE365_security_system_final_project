--------------------------------------------------------------------------------
-- Filename     : seven_segment_display_driver.vhd
-- Author(s)    : Kyle Bielby, Chris Lloyd (Team 1)
-- Class        : EE365 (Final Project)
-- Due Date     : 2020-11-23
-- Target Board : Cora Z7-10
-- Entity       : seven_segment_display_driver
-- Description  : Driver code to display a 4 digit number (0-9) on a 4 digit
--                seven segment display. Uses a commond cathode display meaning
--                data needs to be refreshed to each digit at a fast rate to
--                trick the eye.
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
entity seven_segment_display_driver is
port
(
  -- Clocks & Resets
  I_CLK_125MHZ     : in std_logic;

  -- Input four digit number to display
  -- bits 0-3:   digit 1
  -- bits 4-7:   digit 2
  -- bits 8-11:  digit 3
  -- bits 12-15: digit 4
  I_DISPLAY_DATA   : in std_logic_vector(15 downto 0);

  -- Display digit control
  -- | DIGIT 4 | DIGIT 3 | DIGIT 2 | DIGIT 1 |
  O_DIGIT_SELECT   : out std_logic_vector(3 downto 0);

  -- seven segment display segment selection port
  -- bit 0: segment A
  -- bit 1: segment B
  -- bit 2: segment C
  -- bit 3: segment D
  -- bit 4: segment E
  -- bit 5: segment F
  -- bit 6: segment G
  O_SEGMENT_SELECT : out std_logic_vector(6 downto 0)
);
end seven_segment_display_driver;

--------------------------------
--  Architecture Declaration  --
--------------------------------
architecture rtl of seven_segment_display_driver is

  -------------
  -- SIGNALS --
  -------------

  -- Counter and enable for digit refresh
  signal s_digit_refresh_counter : unsigned(16 downto 0) := "00000000000000000";
  signal s_digit_enable          : std_logic;

  -- FSM for digit position when refreshing display
  type T_REFRESHED_DIGIT is (DIGIT_1_STATE,
                             DIGIT_2_STATE,
                             DIGIT_3_STATE,
                             DIGIT_4_STATE);

  -- Current digit that is illuminated
  signal s_current_digit         : T_REFRESHED_DIGIT;

  -- Value of the digit being displayed
  signal s_digit_value           : std_logic_vector(3 downto 0);

begin

  ------------------------------------------------------------------------------
  -- Process Name     : DIGIT_EN_CNTR
  -- Sensitivity List : I_CLK_125MHZ    : 125 MHz global clock
  -- Useful Outputs   : s_digit_enable : Enable line to allow state to change
  --                    in DIGIT_STATE_MACHINE process
  --                    (active high enable logic)
  -- Description      : Counter to delay the time between setting each digit.
  --                    This refresh rate is fast enough to trick the eye to
  --                    think all digits are set at the same time. Every 0x1FFFF
  --                    clock ticks (1.05 ms), s_digit_enable gets driven high.
  ------------------------------------------------------------------------------
  DIGIT_EN_CNTR: process (I_CLK_125MHZ)
  begin
    if (rising_edge(I_CLK_125MHZ)) then
      s_digit_refresh_counter  <= s_digit_refresh_counter + 1;

      -- Rollover case at max count 0x1FFFF (1.05 ms)
      if (s_digit_refresh_counter = "11111111111111111") then
        s_digit_enable         <= '1';
      else
        s_digit_enable         <= '0';
      end if;
    end if;
  end process DIGIT_EN_CNTR;
  ------------------------------------------------------------------------------

  ------------------------------------------------------------------------------
  -- Process Name     : DIGIT_STATE_MACHINE
  -- Sensitivity List : I_CLK_125MHZ    : 125 MHz global clock
  -- Useful Outputs   : s_current_digit : Current digit selected, used in
  --                                      DIGIT_DATA_CONTROL process to set each
  --                                      digit's segments.
  -- Description      : State machine to control which digit is currently
  --                    selected. This is used to know which digit to update.
  ------------------------------------------------------------------------------
  DIGIT_STATE_MACHINE: process (I_CLK_125MHZ)
  begin
    if (rising_edge(I_CLK_125MHZ)) then
      if (s_digit_enable = '1') then
        case s_current_digit is
          when DIGIT_1_STATE =>
            s_current_digit <= DIGIT_2_STATE;

          when DIGIT_2_STATE =>
            s_current_digit <= DIGIT_3_STATE;

          when DIGIT_3_STATE =>
            s_current_digit <= DIGIT_4_STATE;

          when DIGIT_4_STATE =>
            s_current_digit <= DIGIT_1_STATE;

          -- Error condition, should never occur
          when others =>
            s_current_digit <= s_current_digit;
        end case;
      else
        s_current_digit     <= s_current_digit;
      end if;
    end if;
  end process DIGIT_STATE_MACHINE;
  ------------------------------------------------------------------------------

  ------------------------------------------------------------------------------
  -- Process Name     : DIGIT_SELECTED_CONTROL
  -- Sensitivity List : I_CLK_125MHZ    : 125 MHz global clock
  -- Useful Outputs   : O_DIGIT_SELECT  : Current digit enabled to receive
  --                                      segment data.
  -- Description      : Controls the currently enabled digit output.
  ------------------------------------------------------------------------------
  DIGIT_SELECTED_CONTROL: process (I_CLK_125MHZ)
  begin
    if (rising_edge(I_CLK_125MHZ)) then
      case s_current_digit is
        when DIGIT_1_STATE =>
          O_DIGIT_SELECT <= "0001";

        when DIGIT_2_STATE =>
          O_DIGIT_SELECT <= "0010";

        when DIGIT_3_STATE =>
          O_DIGIT_SELECT <= "0100";

        when DIGIT_4_STATE =>
          O_DIGIT_SELECT <= "1000";

        -- Error condition, should never occur
        when others =>
          O_DIGIT_SELECT <= (others => '0');
      end case;
    end if;
  end process DIGIT_SELECTED_CONTROL;
  ------------------------------------------------------------------------------

  ------------------------------------------------------------------------------
  -- Process Name     : GET_DIGIT_VALUE
  -- Sensitivity List : I_CLK_125MHZ    : 125 MHz global clock
  -- Useful Outputs   : s_digit_value   : Segment data of current digit from
  --                                      slice of input data.
  -- Description      : Gets the segment data by slicing the input data into the
  --                    current nibble of data.
  ------------------------------------------------------------------------------
  GET_DIGIT_VALUE: process (I_CLK_125MHZ)
  begin
    if (rising_edge(I_CLK_125MHZ)) then
      case s_current_digit is  -- Note: Select next digit to control NOT current
        when DIGIT_4_STATE =>
          s_digit_value <= I_DISPLAY_DATA(3 downto 0);

        when DIGIT_1_STATE =>
          s_digit_value <= I_DISPLAY_DATA(7 downto 4);

        when DIGIT_2_STATE =>
          s_digit_value <= I_DISPLAY_DATA(11 downto 8);

        when DIGIT_3_STATE =>
          s_digit_value <= I_DISPLAY_DATA(15 downto 12);

        -- Error condition, should never occur
        when others =>
          s_digit_value <= (others => '0');
      end case;
    end if;
  end process GET_DIGIT_VALUE;
  ------------------------------------------------------------------------------

  ------------------------------------------------------------------------------
  -- Process Name     : DISPLAY_DIGIT
  -- Sensitivity List : I_CLK_125MHZ     : 125 MHz global clock
  -- Useful Outputs   : O_SEGMENT_SELECT : Segment data of current digit.
  -- Description      : Sets the current segment data to be displayed.
  ------------------------------------------------------------------------------
  DISPLAY_DIGIT: process (I_CLK_125MHZ)
  begin
    if (rising_edge(I_CLK_125MHZ)) then
      case s_digit_value is
        when "0000" =>                    -- 0
          O_SEGMENT_SELECT <= "0000000";
        when "0001" =>                    -- 1
          O_SEGMENT_SELECT <= "0000110";
        when "0010" =>                    -- 2
          O_SEGMENT_SELECT <= "1011011";
        when "0011" =>                    -- 3
          O_SEGMENT_SELECT <= "1001111";
        when "0100" =>                    -- 4
          O_SEGMENT_SELECT <= "1100110";
        when "0101" =>                    -- 5
          O_SEGMENT_SELECT <= "1101101";
        when "0110" =>                    -- 6
          O_SEGMENT_SELECT <= "1111101";
        when "0111" =>                    -- 7
          O_SEGMENT_SELECT <= "0000111";
        when "1000" =>                    -- 8
          O_SEGMENT_SELECT <= "1111111";
        when "1001" =>                    -- 9
          O_SEGMENT_SELECT <= "1100111";
        when others =>                    -- Blank
          O_SEGMENT_SELECT <= (others => '0');
      end case;
    end if;
  end process DISPLAY_DIGIT;
  ------------------------------------------------------------------------------
end architecture rtl;