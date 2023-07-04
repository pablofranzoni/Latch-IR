/*
 * ir_prueba1.cpp
 *
 */
#define DECODE_NEC  // Includes Apple and Onkyo
#define START_CODE_DIG 99

#include <Arduino.h>
#include "PinDefinitionsAndMore.h"  // Define macros for input and output pin etc.
#include <IRremote.hpp>
#include <EEPROM.h>

// Dirección de memoria donde se almacena la variable de control
#define EEPROM_CONTROL_ADDR 0

// Dirección de memoria donde se almacenan los datos en la EEPROM
#define EEPROM_DATA_ADDR 1

struct Tabla {
  int hexvalue;
  int derecha;
};

int currentIndex = 0;     // Índice del array en el que se almacenará el siguiente número
uint16_t inputArray[4];   // Array para almacenar los cuatro números ingresados
unsigned long startTime;  // Tiempo de inicio del ingreso
unsigned long previousTime = 0; // Almacena el tiempo del último valor leído
const unsigned long minIntervalTime = 500; // Intervalo mínimo entre repeticiones en milisegundos

boolean compararDatos(int datos[]) {
  // Leer los datos almacenados en la EEPROM
  int storedData[3][4];
  EEPROM.get(EEPROM_DATA_ADDR, storedData);

  // Comparar los datos
  boolean coincidencia = true;
  int idx = datos[0];
  for (int j = 0; j < 4; j++) {
    if (datos[j] != storedData[idx][j]) {
      coincidencia = false;
      break;
    }
  }
  
  return coincidencia;
}

int obtenerValorDerecha(uint8_t value) {
  struct Tabla tabla[] = {
    {0x16, 0},
    {0x0C, 1},
    {0x18, 2},
    {0x5E, 3},
    {0x08, 4},
    {0x1C, 5},
    {0x5A, 6},
    {0x42, 7},
    {0x52, 8},
    {0x4A, 9},
    {0x45, START_CODE_DIG}
  };

  int tablaSize = sizeof(tabla) / sizeof(tabla[0]);
  
  for (int i = 0; i < tablaSize; i++) {
    if (tabla[i].hexvalue == value) {
      return tabla[i].derecha;
    }
  }
  
  return -1; // Valor por defecto si no se encuentra una coincidencia
}

void setup() {

  // Leer el estado de la variable de control
  byte control = EEPROM.read(EEPROM_CONTROL_ADDR);

  if (control == 0) {
    // Ejemplo de cómo grabar los datos en la EEPROM
    int data[3][4] = {
      {0, 9, 9, 1},    // No usado
      {1, 7, 4, 1},    // Salida 1 - Codigo 741
      {2, 8, 5, 3}     // Salida 2 - Codigo 853
    };
    EEPROM.put(EEPROM_DATA_ADDR, data);

    // Marcar la variable de control como grabada
    EEPROM.write(EEPROM_CONTROL_ADDR, 1);
  }

  Serial.begin(115200);
  IrReceiver.begin(IR_RECEIVE_PIN, ENABLE_LED_FEEDBACK);
}

void loop() {

  if (IrReceiver.decode()) {
    //Serial.println(IrReceiver.decodedIRData.decodedRawData, HEX);  // Print "old" raw data
    // USE NEW 3.x FUNCTIONS
    IrReceiver.printIRResultShort(&Serial);  // Print complete received data in one line
    //IrReceiver.printIRSendUsage(&Serial);    // Print the statement required to send this data

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

/*
 * 
 */
void processDecodedIRData(uint16_t key) {
  
  int digit = obtenerValorDerecha(key);

  if (digit) {
    if (digit == START_CODE_DIG) {
      startTime = millis();  // Inicia el temporizador
      currentIndex = 0;

    } else if (digit >= 0 && digit <= 9) {
      
      if (currentIndex < 4) {
        inputArray[currentIndex] = digit;
        
        Serial.print("IDX: ");
        Serial.print(currentIndex);
        Serial.print("DIGIT: ");
        Serial.println(digit);
        currentIndex++;
      }  
      if (currentIndex == 4) {
        Serial.print("NUMS: ");
        for (int i = 0; i < 4; i++) {
          Serial.print(inputArray[i]);
          Serial.print("|");
        }
        Serial.println();

        boolean resultado = compararDatos(inputArray);

        if (resultado) {
          Serial.print("Activa Salida: ");
          Serial.println(inputArray[0]);
        } else {
          Serial.println("Los datos no coinciden.");
        }

        currentIndex = 5;
      }
      
    }  else {
      Serial.println("CHAR NO VALIDO");
      currentIndex = 0;
    }
  }
}