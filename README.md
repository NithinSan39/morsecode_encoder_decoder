# STM32 Morse Code Encoder & Decoder

This application note contains an explanation and complete C-code implementation for a two-part Morse code communication system:
* **Data Transmission (Encoder):** Translating standard English text received via PC UART into precisely timed hardware LED blinks.
* **Data Reception (Decoder):** Measuring raw human input via a tactile button to decode Morse code back into English text using non-blocking hardware timers.

## Table of Contents
1. [Abbreviations](#abbreviations)
2. [General about Morse Code Timing](#general-about-morse-code-timing)
3. [The Encoder (Transmission)](#the-encoder-transmission)
4. [The Decoder (Reception)](#the-decoder-reception)
5. [Hardware Setup & Pinout](#hardware-setup--pinout)
6. [How to use this repository](#how-to-use-this-repository)

## Abbreviations
* **GPIO:** General-Purpose Input/Output
* **UART:** Universal Asynchronous Receiver-Transmitter
* **TX:** Transmit
* **RX:** Receive
* **ITU:** International Telecommunication Union (Standardizer of Morse Code)
* **HAL:** Hardware Abstraction Layer

## General about Morse Code Timing
Standard ITU Morse code relies on strict relative timing. The fundamental unit of time is the duration of a single **dot**. Every other element is mathematically derived from this base unit:
* **Dot (`.`):** 1 Time Unit
* **Dash (`-`):** 3 Time Units
* **Inter-element Gap:** 1 Time Unit (Space between dots/dashes within a letter)
* **Short Gap:** 3 Time Units (Space between letters)
* **Medium Gap:** 7 Time Units (Space between words)

For the sake of this application, the base unit is defined as `200ms` for human-readable execution.

## The Encoder (Transmission)
The encoder application translates ASCII characters into Morse code sequences. 
* Application receives full string arrays via UART RX interrupts from a PC terminal.
* It parses the string, comparing each character against a predefined multidimensional `MORSE_MAP` array.
* **Execution:** The LED toggling utilizes active-blocking execution (`HAL_Delay`). This forces strict adherence to ITU timing standards by halting the CPU precisely for the required duration of each dot or dash.

## The Decoder (Reception)
Unlike the encoder, the decoder relies on measuring unpredictable human input. 
* **Input handling:** It monitors an active-low GPIO pin connected to a tactile button or wire.
* **Debouncing:** Implements a 50ms software delay to filter out mechanical switch bounce.
* **Non-blocking measurement:** Uses the STM32 SysTick timer (`HAL_GetTick()`) to act as a stopwatch. 
  * If the button hold duration is `< 400ms`, it registers a **Dot**.
  * If the duration is `> 400ms`, it registers a **Dash**.
* **Timeout Detection:** Because the application does not know when a user has finished typing a letter, it continuously polls the idle time. If the line remains idle for `> 2000ms`, the application seals the buffer, compares it against the `MORSE_MAP`, and transmits the decoded character via UART TX back to the PC.

## Hardware Setup & Pinout
This project was developed for the STM32F1xx series (specifically the STM32F103C8T6 "Blue Pill"). 

| Component | STM32 Pin | Function |
| :--- | :--- | :--- |
| **LED (Morse Output)** | `PA3` | Output: Flashes the encoded dots and dashes |
| **PC UART (TX)** | `PA9` | Output: Sends decoded text and UI prompts to PC |
| **PC UART (RX)** | `PA10` | Input: Receives text from PC to encode |
| **Button/Wire** | `PA0` | Input (Pull-Up): Tactile input for decoding |

*(Note: The UART terminal should be configured to a Baud Rate of 115200, 8 Data Bits, 1 Stop Bit, No Parity).*

## How to use this repository
1. Clone the repository to your local machine:
   `git clone https://github.com/NithinSan39/morsecode_encoder_decoder.git`
2. Open **STM32CubeIDE**.
3. Go to `File` > `Open Projects from File System...` and select the cloned directory.
4. Open the `.ioc` file to review the hardware configuration, or compile and flash the `main.c` file directly to your board.
