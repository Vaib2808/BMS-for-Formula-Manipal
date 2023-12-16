#include <Arduino.h>
#include <stdint.h>
#include <SPI.h>
#include "Linduino.h"
#include "LT_SPI.h"
#include "UserInterface.h"
#include "LTC681x.h"
#include "LTC6813.h"
#include <FlexCAN_T4.h>
#include "arduino_freertos.h"
#include "avr/pgmspace.h"
#include <semphr.h>

#define ENABLED 1
#define DISABLED 0
#define DATALOG_ENABLED 1
#define DATALOG_DISABLED 0
#define PWM 1
#define SCTL 2
FlexCAN_T4<CAN1, RX_SIZE_256, TX_SIZE_16> can1;
CAN_message_t msg;
CAN_message_t receivedMsg;
char buf[40];
String checks;
int8_t  dataTherm[10];
int value1, adcValue;
unsigned long value3;
SemaphoreHandle_t adcMutex;
bool adcValueReady = false;
QueueHandle_t canQueue;


const uint8_t TOTAL_IC = 5;//!< Number of ICs in the daisy chain

/********************************************************************
 ADC Command Configurations. See LTC681x.h for options
*********************************************************************/
const uint8_t ADC_OPT = ADC_OPT_DISABLED; //!< ADC Mode option bit
const uint8_t ADC_CONVERSION_MODE =MD_7KHZ_3KHZ; //!< ADC Mode
const uint8_t ADC_DCP = DCP_DISABLED; //!< Discharge Permitted 
const uint8_t CELL_CH_TO_CONVERT =CELL_CH_ALL; //!< Channel Selection for ADC conversion
const uint8_t AUX_CH_TO_CONVERT = AUX_CH_ALL; //!< Channel Selection for ADC conversion
const uint8_t STAT_CH_TO_CONVERT = STAT_CH_ALL; //!< Channel Selection for ADC conversion
const uint8_t SEL_ALL_REG = REG_ALL; //!< Register Selection 
const uint8_t SEL_REG_A = REG_1; //!< Register Selection 
const uint8_t SEL_REG_B = REG_2; //!< Register Selection 

const uint16_t MEASUREMENT_LOOP_TIME = 500; //!< Loop Time in milliseconds(ms)

//Under Voltage and Over Voltage Thresholds
const uint16_t OV_THRESHOLD = 42000; //!< Over voltage threshold ADC Code. LSB = 0.0001 ---(4.1V)
const uint16_t UV_THRESHOLD = 25000; //!< Under voltage threshold ADC Code. LSB = 0.0001 ---(3V)

//Loop Measurement Setup. These Variables are ENABLED or DISABLED. Remember ALL CAPS
const uint8_t WRITE_CONFIG = DISABLED;  //!< This is to ENABLED or DISABLED writing into to configuration registers in a continuous loop
const uint8_t READ_CONFIG = DISABLED; //!< This is to ENABLED or DISABLED reading the configuration registers in a continuous loop
const uint8_t MEASURE_CELL = ENABLED; //!< This is to ENABLED or DISABLED measuring the cell voltages in a continuous loop
const uint8_t MEASURE_AUX = DISABLED; //!< This is to ENABLED or DISABLED reading the auxiliary registers in a continuous loop
const uint8_t MEASURE_STAT = DISABLED; //!< This is to ENABLED or DISABLED reading the status registers in a continuous loop
const uint8_t PRINT_PEC = DISABLED; //!< This is to ENABLED or DISABLED printing the PEC Error Count in a continuous loop

cell_asic BMS_IC[TOTAL_IC]; //!< Global Battery Variable

/*************************************************************************
 Set configuration register. Refer to the data sheet
**************************************************************************/
bool REFON = true; //!< Reference Powered Up Bit
bool ADCOPT = false; //!< ADC Mode option bit
bool GPIOBITS_A[5] = {false,false,true,true,true}; //!< GPIO Pin Control // Gpio 1,2,3,4,5
bool GPIOBITS_B[4] = {false,false,false,false}; //!< GPIO Pin Control // Gpio 6,7,8,9
uint16_t UV=UV_THRESHOLD; //!< Under voltage Comparison Voltage
uint16_t OV=OV_THRESHOLD; //!< Over voltage Comparison Voltage
bool DCCBITS_A[12] = {false,false,false,false,false,false,false,false,false,false,false,false}; //!< Discharge cell switch //Dcc 1,2,3,4,5,6,7,8,9,10,11,12
bool DCCBITS_B[7]= {false,false,false,false,false,false,false}; //!< Discharge cell switch //Dcc 0,13,14,15
bool DCTOBITS[4] = {true,false,true,false}; //!< Discharge time value //Dcto 0,1,2,3  // Programed for 4 min 
/*Ensure that Dcto bits are set according to the required discharge time. Refer to the data sheet */
bool FDRF = false; //!< Force Digital Redundancy Failure Bit
bool DTMEN = true; //!< Enable Discharge Timer Monitor
bool PSBITS[2]= {false,false}; //!< Digital Redundancy Path Selection//ps-0,1


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


static void task1(void*) {
    while (true) {
        if (adcValueReady) {
            xSemaphoreTake(adcMutex, portMAX_DELAY);
            int adcValue = value1;
            adcValueReady = false;
            xSemaphoreGive(adcMutex);

            msg.id = 1232;
            for (uint8_t i = 0; i < 8; i++) {
                msg.buf[i] = (uint8_t)((adcValue >> (i * 8)) & 0xFF);
            }
//            ::Serial.println(adcValue);
//            can1.write(msg);
//            sprintf(buf, "CAN sent with id: %lu \n", msg.id); // Use %lu for uint32_t
//            ::Serial.println(buf);
        }
        ::vTaskDelay(pdMS_TO_TICKS(500));
    }
}

static void task2(void*) {
    while (true) {
        xSemaphoreTake(adcMutex, portMAX_DELAY);
         value1 = arduino::analogRead(A1);
         int adcValue= value1;
            adcValueReady = true;
        xSemaphoreGive(adcMutex);
        checks= Serial.readString();
      if(checks == "Axx1"){
        ::Serial.print(adcValue);
        ::Serial.print(" Voltage: ");
        ::Serial.println(adcValue * 3.3 / 4095, arduino::DEC);
      }
        ::vTaskDelay(pdMS_TO_TICKS(20));
    }
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

FLASHMEM __attribute__((noinline)) void setup() {
    ::Serial.begin(115'200);
    ::pinMode(arduino::LED_BUILTIN, arduino::OUTPUT);
    ::digitalWriteFast(arduino::LED_BUILTIN, arduino::HIGH);
    ::pinMode(A1, arduino::INPUT);
    ::arduino::analogReadResolution(12);
    ::can1.begin();
     ::can1.enableFIFO();
     ::can1.enableFIFOInterrupt();
    ::can1.onReceive(canint);
    ::can1.setBaudRate(500000);
      
    adcMutex = xSemaphoreCreateMutex();
    canQueue = xQueueCreate(10, sizeof(CAN_message_t)); // Adjust the queue size as needed

    ::delay(5'000);

    if (CrashReport) {
        ::Serial.print(CrashReport);
        ::Serial.println();
        ::Serial.flush();
    }

    ::Serial.println(PSTR("\r\nBooting FreeRTOS kernel " tskKERNEL_VERSION_NUMBER ". Built by gcc " __VERSION__ " (newlib " _NEWLIB_VERSION ") on " __DATE__ ". ***\r\n"));

    ::xTaskCreate(task1, "task1", 512, nullptr, 3, nullptr); // Increase priority to 3
    ::xTaskCreate(task2, "task2", 512, nullptr, 2, nullptr);
//    ::xTaskCreate(task3, "task3", 512, nullptr, 1, nullptr); // Increase priority to 1

    ::Serial.println("setup(): starting scheduler...");
    ::Serial.flush();

    ::vTaskStartScheduler();
}

void loop() {
//  can1.events();
  }
//
//void FlexCAN_T4_userRxCallback(uint32_t id, uint8_t len, uint8_t* buf, uint8_t bus) {
//    receivedMsg.id = id;
//    receivedMsg.len = len;
//    memcpy(receivedMsg.buf, buf, len);
//    ::Serial.println("Can Recieved");
//    ::digitalWriteFast(arduino::LED_BUILTIN, arduino::LOW);
//    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
//    xQueueSendFromISR(canQueue, &receivedMsg, &xHigherPriorityTaskWoken);
//
////    if (xHigherPriorityTaskWoken != pdFALSE) {
////        portYIELD_FROM_ISR();
////    }
//}
