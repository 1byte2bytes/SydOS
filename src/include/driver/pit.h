/*
 * File: pit.h
 * 
 * Copyright (c) 2017-2018 Sydney Erickson, John Davis
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef PIT_H
#define PIT_H

#include <main.h>

// PIT I/O ports.
enum {
    PIT_PORT_CHANNEL0               = 0x40, // Channel 0 port (read/write). Tied to IRQ0.
    PIT_PORT_CHANNEL1               = 0x41, // Channel 1 port (read/write). Tied to DRAM (doesn't exist now).
    PIT_PORT_CHANNEL2               = 0x42, // Channel 2 port (read/write). Tied to PC speaker.
    PIT_PORT_COMMAND                = 0x43  // Mode/command register (write only).
};

// PIT counters.
enum {
    PIT_CMD_COUNTER0                = 0x00, // Channel 0 (IRQ0).
    PIT_CMD_COUNTER1                = 0x40, // Channel 1 (DRAM).
    PIT_CMD_COUNTER2                = 0x80, // Channel 2 (PC speaker).
};

// PIT access modes.
enum {
    PIT_CMD_ACCESS_LATCH            = 0x00, // Latch count value command.
    PIT_CMD_ACCESS_LSBONLY          = 0x10, // Access mode: lobyte only.
    PIT_CMD_ACCESS_MSBONLY          = 0x20, // Access mode: hibyte only.
    PIT_CMD_ACCESS_DATA             = 0x30  // Access mode: lobyte/hibyte.
};

// PIT operating modes.
enum {
    PIT_CMD_MODE_TERMINALCOUNT      = 0x0, // Mode 0 (interrupt on terminal count).
    PIT_CMD_MODE_ONESHOT            = 0x2, // Mode 1 (hardware re-triggerable one-shot).
    PIT_CMD_MODE_RATEGEN            = 0x4, // Mode 2 (rate generator).
    PIT_CMD_MODE_SQUAREWAVEGEN      = 0x6, // Mode 3 (square wave generator).
    PIT_CMD_MODE_SOFTWARETRIG       = 0x8, // Mode 4 (software triggered strobe).
    PIT_CMD_MODE_HARDWARETRIG       = 0xA, // Mode 5 (hardware triggered strobe).
};

// PIT binary modes.
enum {
    PIT_CMD_BINCOUNT_BINARY         = 0x0, // 16-bit binary.
    PIT_CMD_BINCOUNT_BCD            = 0x1  // Four-digit BCD.
};

// PIT command masks.
enum {
    PIT_CMD_MASK_BINCOUNT           = 0x01,
    PIT_CMD_MASK_MODE               = 0x0E,
    PIT_CMD_MASK_ACCESS             = 0x30,
    PIT_CMD_MASK_COUNTER            = 0xC0
};

#define PIT_BASE_FREQ 1193182

extern void pit_startcounter(uint32_t freq, uint8_t counter, uint8_t mode);
extern void pit_init(void);

#endif
