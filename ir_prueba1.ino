/*
 * ir_prueba1.ino
 *
 *
 ************************************************************************************
 */
#define DECODE_NEC  
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
    
    IrReceiver.printIRResultShort(&Serial);  // Print complete received data in one line
    IrReceiver.printIRSendUsage(&Serial);    // Print the statement required to send this data

    if (currentIndex > 0 && millis() - startTime >= 10000) {
      Serial.println("EXCEDIO TIEMPO");
      currentIndex = 0;
    }

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