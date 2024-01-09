#include <FlexCAN_T4.h>
#include <Arduino.h>
#include <stdint.h>
#include <SPI.h>
#include "Linduino.h"
#include "LT_SPI.h"
#include "UserInterface.h"
#include "LTC681x.h"
#include "LTC6813.h"

#define ENABLED 1
#define DISABLED 0
#define DATALOG_ENABLED 1
#define DATALOG_DISABLED 0
FlexCAN_T4<CAN1, RX_SIZE_256, TX_SIZE_16> can1;
FlexCAN_T4<CAN2, RX_SIZE_256, TX_SIZE_16> can2;
CAN_message_t msg;
CAN_message_t receivedMsg;
char buf[40];
String checks;
int8_t  dataTherm[10];
int value1, adcValue;
unsigned long value3;
const uint8_t TOTAL_IC = 4;//!< Number of ICs in the daisy chain
//Under Voltage and Over Voltage Thresholds
const uint16_t OV_THRESHOLD = 41000; //!< Over voltage threshold ADC Code. LSB = 0.0001 ---(4.1V)
const uint16_t UV_THRESHOLD = 30000; //!< Under voltage threshold ADC Code. LSB = 0.0001 ---(3V)
const uint8_t MEASURE_CELL = ENABLED
cell_asic BMS_IC[TOTAL_IC];
uint16_t UV=UV_THRESHOLD; //!< Under voltage Comparison Voltage
uint16_t OV=OV_THRESHOLD; //!< Over voltage Comparison Voltage


void setup()
{
  Serial.begin(115200);
  quikeval_SPI_connect();
  spi_enable(SPI_CLOCK_DIV16); // This will set the Linduino to have a 1MHz Clock
  LTC6813_init_cfg(TOTAL_IC, BMS_IC);
  LTC6813_init_cfgb(TOTAL_IC,BMS_IC);
  for (uint8_t current_ic = 0; current_ic<TOTAL_IC;current_ic++) 
  {
    LTC6813_set_cfgr(current_ic,BMS_IC,REFON,ADCOPT,GPIOBITS_A,DCCBITS_A, DCTOBITS, UV, OV);
    LTC6813_set_cfgrb(current_ic,BMS_IC,FDRF,DTMEN,PSBITS,GPIOBITS_B,DCCBITS_B);   
  }   
  LTC6813_reset_crc_count(TOTAL_IC,BMS_IC);
  LTC6813_init_reg_limits(TOTAL_IC,BMS_IC);
  wakeup_sleep(TOTAL_IC);
}

void loop() {
            int8_t error = 0;
            wakeup_sleep(TOTAL_IC);
            error = LTC6813_rdcv(SEL_ALL_REG, TOTAL_IC, BMS_IC);
            check_error(error); 
            print_cells(DATALOG_DISABLED);
            print_cells(DATALOG_ENABLED);
            canint(receivedMsg);
            delay(1000);
}


void canint(const CAN_message_t &receivedMsg) {
//  can1.events();
            value3 = receivedMsg.id;
          if(receivedMsg.id == 0x1838F380){
            ::Serial.print("CAN ID:  ");
            ::Serial.print(receivedMsg.id,arduino:: HEX );
            ::Serial.print(" ");
//            dataTherm = recievedMs
            for(int k=0;k<8;k++){
             dataTherm[k] =  (int8_t)receivedMsg.buf[k];
              ::Serial.print(dataTherm[k]);
               ::Serial.print(" ");
              }
              ::Serial.println();
//            ::vTaskDelay(pdMS_TO_TICKS(30));
//        }
          }
}

void print_cells(uint8_t datalog_en) {
  CAN_message_t msg;

  for (int current_ic = 0; current_ic < TOTAL_IC; current_ic++) {
    if (datalog_en == 0) {
      msg.id = 0x100 + current_ic;
      msg.len = BMS_IC[current_ic].ic_reg.cell_channels * 4;

      for (int i = 0; i < BMS_IC[current_ic].ic_reg.cell_channels; i++) {
        float cellVoltage = BMS_IC[current_ic].cells.c_codes[i] * 0.0001;
        memcpy(&msg.buf[i * 4], &cellVoltage, sizeof(cellVoltage));
      }

      canBus.write(msg);
    } else {
      msg.id = 0x200; // Example ID for summary data
      msg.len = TOTAL_IC * BMS_IC[current_ic].ic_reg.cell_channels * 4;

      int index = 0;
      for (int ic = 0; ic < TOTAL_IC; ic++) {
        for (int i = 0; i < BMS_IC[ic].ic_reg.cell_channels; i++) {
          float cellVoltage = BMS_IC[ic].cells.c_codes[i] * 0.0001;
          memcpy(&msg.buf[index], &cellVoltage, sizeof(cellVoltage));
          index += 4;
        }
      }

      canBus.write(msg);
    }
  }
}


void check_error(int error)
{
  if (error == -1)
  {
    Serial.println(F("A PEC error was detected in the received data"));
  }
}
