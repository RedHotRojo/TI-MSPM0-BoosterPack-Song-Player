# TI LP-MSPM0G3507 + BoosterPack Song Player

ECE 3660: Intro to Embedded Computer Systems, Final Project by Kemal and Rohit

## Overview

The project tested our Embedded Computer Programming capabilities by playing a song on a buzzer and displaying the notes on an LCD screen.

- MCU Development Board: TI LP-MSPM0G3507 + TI BOOSTXL-EDUMKII (Boosterpack)
- Piezo Buzzer plays each note
- LCD displays current note
- 2 push buttons control play/resume
- OPT 3001 light sensor controls volume

## Description
- TI MSPM0G3507 programmed in C
- Hardware timer controls buzzer using interrupts
- Hardware timer period is adjusted to control the frequency of the note
- OPT 3001 light sensor value determines duty cycle of hardware timer to control volume
- Clock delay determines length of note
- LCD uses SPI
- Push buttons use interrupts to change FSM state (Start or Stop) - S1 starts, S2 stops
- Hardware timer duty cycle and period are set to 0 for rests and while stopped

## Software Architecture Block Diagram
<img width="1448" height="874" alt="Block Diagram" src="https://github.com/user-attachments/assets/605979cb-e71d-419a-bdac-1ebb3c4e2039" />

## [Full Presentation](https://drive.google.com/file/d/1MBlNggo2uPBZ0au3VSc-iq_8k6rNp2Mt/view?usp=sharing)
