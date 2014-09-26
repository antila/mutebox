/**
 * @license The MIT License (MIT)
 * @see Based on the code from http://learn.adafruit.com/trinket-usb-volume-knob
 */

#include <TinyWireM.h>
#include <TinyAdafruit_MCP23017.h>
#include <TrinketHidCombo.h>

/*
MCP23017 Pinout:
                _________
    GPB0 <--> [|*1 \_/ 28|] <--> GPA7
    GPB1 <--> [| 2     27|] <--> GPA6
    GPB2 <--> [| 3     26|] <--> GPA5
    GPB3 <--> [| 4     25|] <--> GPA4
    GPB4 <--> [| 5     24|] <--> GPA3
    GPB5 <--> [| 6     23|] <--> GPA2
    GPB6 <--> [| 7     22|] <--> GPA1
    GPB7 <--> [| 8     21|] <--> GPA0
     VDD ---> [| 9     20|] ---> INTA
     VSS ---> [|10     19|] ---> INTB
      NC ---- [|11     18|] ---> RESET
     SCL ---> [|12     17|] <--- A2
     SDA <--> [|13     16|] <--- A1
      NC ---- [|14     15|] <--- A0
               -----------
*/

// Bank A
#define MCP_GPA0 0
#define MCP_GPA1 1
#define MCP_GPA2 2
#define MCP_GPA3 3
#define MCP_GPA4 4
#define MCP_GPA5 5
#define MCP_GPA6 6
#define MCP_GPA7 7

// Bank B
#define MCP_GPB0 8
#define MCP_GPB1 9
#define MCP_GPB2 10
#define MCP_GPB3 11
#define MCP_GPB4 12
#define MCP_GPB5 13
#define MCP_GPB6 14
#define MCP_GPB7 15

// Buttons
#define PIN_NEXT_SWITCH MCP_GPB1
#define PIN_PAUSE_SWITCH MCP_GPB2

// Rotary encoder inputs
#define PIN_ENCODER_A MCP_GPA6
#define PIN_ENCODER_B MCP_GPA5
#define PIN_ENCODER_SWITCH MCP_GPA4

//
static uint8_t enc_prev_pos   = 0;
static uint8_t enc_flags      = 0;
static char    pause_was_pressed = 0;
static char    next_was_pressed = 0;
static char    mute_was_pressed = 0;

Adafruit_MCP23017 mcp;

void setup()
{
    // Start the USB device engine and enumerate
    TrinketHidCombo.begin();

    setupMCP();
    setupRotaryEncoder();
}

void setupMCP()
{
    // Start MCP23017
    mcp.begin();
    
    // Set Bank A to output
    for ( int i = 0; i < 8; i++ ) {
        mcp.pinMode(i, OUTPUT);
    }

    // Set Bank B to input
    for ( int i = 8; i < 16; i++ ) {
        mcp.pinMode(i, INPUT);
        mcp.pullUp(i, HIGH);
    }
}

void setupRotaryEncoder() {
    // Get an initial reading on the encoder pins
    if (mcp.digitalRead(PIN_ENCODER_A) == LOW) {
        enc_prev_pos |= (1 << 0);
    }
    if (mcp.digitalRead(PIN_ENCODER_B) == LOW) {
        enc_prev_pos |= (1 << 1);
    }

}

void checkRotaryEncoder() {
    int8_t enc_action = 0; // 1 or -1 if moved, sign is direction

    // note: for better performance, the code will now use
    // direct port access techniques
    // http://www.arduino.cc/en/Reference/PortManipulation
    uint8_t enc_cur_pos = 0;
    // read in the encoder state first
    if (mcp.digitalRead(PIN_ENCODER_A) == LOW) {
        enc_cur_pos |= (1 << 0);
    }
    if (mcp.digitalRead(PIN_ENCODER_B) == LOW) {
        enc_cur_pos |= (1 << 1);
    }

    // if any rotation at all
    if (enc_cur_pos != enc_prev_pos) {
        if (enc_prev_pos == 0x00) {
            // this is the first edge
            if (enc_cur_pos == 0x01) {
                enc_flags |= (1 << 0);
            } else if (enc_cur_pos == 0x02) {
                enc_flags |= (1 << 1);
            }
        }

        if (enc_cur_pos == 0x03) {
            // this is when the encoder is in the middle of a "step"
            enc_flags |= (1 << 4);
        } else if (enc_cur_pos == 0x00) {
            // this is the final edge
            if (enc_prev_pos == 0x02) {
                enc_flags |= (1 << 2);
            } else if (enc_prev_pos == 0x01) {
                enc_flags |= (1 << 3);
            }

            // check the first and last edge
            // or maybe one edge is missing, if missing then require the middle state
            // this will reject bounces and false movements
            if (bit_is_set(enc_flags, 0) && (bit_is_set(enc_flags, 2) || bit_is_set(enc_flags, 4))) {
                enc_action = 1;
            } else if (bit_is_set(enc_flags, 2) && (bit_is_set(enc_flags, 0) || bit_is_set(enc_flags, 4))) {
                enc_action = 1;
            } else if (bit_is_set(enc_flags, 1) && (bit_is_set(enc_flags, 3) || bit_is_set(enc_flags, 4))) {
                enc_action = -1;
            } else if (bit_is_set(enc_flags, 3) && (bit_is_set(enc_flags, 1) || bit_is_set(enc_flags, 4))) {
                enc_action = -1;
            }

            enc_flags = 0; // reset for next time
        }
    }

    enc_prev_pos = enc_cur_pos;

    if (enc_action > 0) {
        TrinketHidCombo.pressMultimediaKey(MMKEY_VOL_UP);
    } else if (enc_action < 0) {
        TrinketHidCombo.pressMultimediaKey(MMKEY_VOL_DOWN);
    }

    TrinketHidCombo.poll(); // check if USB needs anything done
}

void checkButtons() {
    // remember that the switch is active-high
    if (mcp.digitalRead(PIN_ENCODER_SWITCH) == LOW) {
        // only on initial press, so the keystroke is not repeated while the button is held down
        if (mute_was_pressed == 0) {
            TrinketHidCombo.pressMultimediaKey(MMKEY_MUTE);
            delay(5); // debounce delay
        }
        mute_was_pressed = 1;
    } else {
        if (mute_was_pressed != 0) {
            delay(5); // debounce delay
        }
        mute_was_pressed = 0;
    }

    if (mcp.digitalRead(PIN_PAUSE_SWITCH) == LOW) {
        if (pause_was_pressed == 0) {
            TrinketHidCombo.pressMultimediaKey(MMKEY_PLAYPAUSE);
            delay(5); // debounce delay
        }
        pause_was_pressed = 1;
    } else {
        if (pause_was_pressed != 0) {
            delay(5);
        }
        pause_was_pressed = 0;
    }

    if (mcp.digitalRead(PIN_NEXT_SWITCH) == LOW) {
        if (next_was_pressed == 0) {
            TrinketHidCombo.pressMultimediaKey(MMKEY_SCAN_NEXT_TRACK);
            delay(5); // debounce delay
        }
        next_was_pressed = 1;
    } else {
        if (next_was_pressed != 0) {
            delay(5);
        }
        next_was_pressed = 0;
    }
}

void loop()
{
    checkButtons();
    checkRotaryEncoder();
}
