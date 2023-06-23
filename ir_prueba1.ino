/*
 * SimpleReceiver.cpp
 *
 * Demonstrates receiving NEC IR codes with IRremote
 *
 *  This file is part of Arduino-IRremote https://github.com/Arduino-IRremote/Arduino-IRremote.
 *
 ************************************************************************************
 * MIT License
 *
 * Copyright (c) 2020-2022 Armin Joachimsmeyer
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is furnished
 * to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
 * PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
 * CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE
 * OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 ************************************************************************************
 */
#define DECODE_NEC  // Includes Apple and Onkyo

#include <Arduino.h>
#include "PinDefinitionsAndMore.h"  // Define macros for input and output pin etc.
#include <IRremote.hpp>

int currentIndex = 0;     // Índice del array en el que se almacenará el siguiente número
uint16_t inputArray[4];   // Array para almacenar los cuatro números ingresados
unsigned long startTime;  // Tiempo de inicio del ingreso
unsigned long previousTime = 0; // Almacena el tiempo del último valor leído
const unsigned long minIntervalTime = 500; // Intervalo mínimo entre repeticiones en milisegundos

void setup() {
  Serial.begin(115200);
  IrReceiver.begin(IR_RECEIVE_PIN, ENABLE_LED_FEEDBACK);
}

void loop() {

  if (IrReceiver.decode()) {
    Serial.println(IrReceiver.decodedIRData.decodedRawData, HEX);  // Print "old" raw data
    // USE NEW 3.x FUNCTIONS
    IrReceiver.printIRResultShort(&Serial);  // Print complete received data in one line
    IrReceiver.printIRSendUsage(&Serial);    // Print the statement required to send this data

    if (currentIndex > 0 && millis() - startTime >= 10000) {
      Serial.println("EXCEDIO TIEMPO");
      currentIndex = 0;
    }

    // Obtener el tiempo actual
    unsigned long actualTime = millis();

    // Verificar si ha pasado suficiente tiempo desde la última repetición
    if (actualTime - previousTime >= minIntervalTime) {
      // Se ha cumplido el intervalo mínimo, procesar el valor
      previousTime = actualTime;

      processDecodedIRData(IrReceiver.decodedIRData.command);
    }

    IrReceiver.resume();  // Enable receiving of the next value
  }
}

/**
 * 
 *
 */
void processDecodedIRData(uint16_t key) {
  
  if (key) {
    if (key == 0x45) {
      startTime = millis();  // Inicia el temporizador
      currentIndex = 0;

    } else if (key == 0x16 || key == 0xC || key == 0x18 || key == 0x5E || 
               key == 0x8 || key == 0x1C || key == 0x5A || key == 0x42 || 
               key == 0x52 || key == 0x4A) {
      
      if (currentIndex < 4) {
        inputArray[currentIndex] = key;
        
        Serial.print("IDX: ");
        Serial.print(currentIndex);
        Serial.print("KEY: ");
        Serial.println(key);
        currentIndex++;
      }  
      if (currentIndex == 4) {
        Serial.print("NUMS: ");
        for (int i = 0; i < 4; i++) {
          Serial.print(inputArray[i]);
          Serial.print("|");
        }
        Serial.println();
      }
      
    }  else {
      Serial.println("CHAR NO VALIDO");
      currentIndex = 0;
    }
  }
}