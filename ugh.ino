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

void print_cells_over_CAN(uint8_t datalog_en) {
  CAN_message_t msg;

  for (int current_ic = 0; current_ic < TOTAL_IC; current_ic++) {
    if (datalog_en == 0) {
      msg.id = 0x100 + current_ic;
      msg.len = BMS_IC[0].ic_reg.cell_channels * 4;

      for (int i = 0; i < BMS_IC[0].ic_reg.cell_channels; i++) {
        float cellVoltage = BMS_IC[current_ic].cells.c_codes[i] * 0.0001;
        memcpy(&msg.buf[i * 4], &cellVoltage, sizeof(cellVoltage));
      }

      canBus.write(msg);
    } else {
      msg.id = 0x200; // Example ID for summary data
      msg.len = TOTAL_IC * BMS_IC[0].ic_reg.cell_channels * 4;

      int index = 0;
      for (int ic = 0; ic < TOTAL_IC; ic++) {
        for (int i = 0; i < BMS_IC[0].ic_reg.cell_channels; i++) {
          float cellVoltage = BMS_IC[ic].cells.c_codes[i] * 0.0001;
          memcpy(&msg.buf[index], &cellVoltage, sizeof(cellVoltage));
          index += 4;
        }
      }

      canBus.write(msg);
      break; // Send summary once and exit the loop
    }
  }
}


void print_cells(uint8_t datalog_en)
{
  for (int current_ic = 0 ; current_ic < TOTAL_IC; current_ic++)
  {
    if (datalog_en == 0)
    {
      Serial.print(" IC ");
      Serial.print(current_ic+1,DEC);
      Serial.print(", ");
      for (int i=0; i<BMS_IC[0].ic_reg.cell_channels; i++)
      {
        Serial.print(" C");
        Serial.print(i+1,DEC);
        Serial.print(":");
        Serial.print(BMS_IC[current_ic].cells.c_codes[i]*0.0001,4);
        Serial.print(",");
      }
      Serial.println();
    }
    else
    {
      Serial.print(" Cells, ");
      for (int i=0; i<BMS_IC[0].ic_reg.cell_channels; i++)
      {
        Serial.print(BMS_IC[current_ic].cells.c_codes[i]*0.0001,4);
        Serial.print(",");
      }
    }
  }
  Serial.println("\n");



void print_cells(uint8_t datalog_en) {
  CAN_message_t msg;

  for (int current_ic = 0; current_ic < TOTAL_IC; current_ic++) {
    if (datalog_en == 0) {
      Serial.print(" IC ");
      Serial.print(current_ic + 1, DEC);
      Serial.print(", ");
      for (int i = 0; i < BMS_IC[0].ic_reg.cell_channels; i++) {
        Serial.print(" C");
        Serial.print(i + 1, DEC);
        Serial.print(":");
        float cellVoltage = BMS_IC[current_ic].cells.c_codes[i] * 0.0001;
        Serial.print(cellVoltage, 4);
        Serial.print(",");
        // Construct CAN message
        msg.id = 0x100 + current_ic;
        msg.len = BMS_IC[current_ic].ic_reg.cell_channels * 4;
        memcpy(&msg.buf[i * 4], &cellVoltage, sizeof(cellVoltage));
        canBus.write(msg);
      }
      Serial.println();
    } else {
      for (int i = 0; i < BMS_IC[0].ic_reg.cell_channels; i++) {
        float cellVoltage = BMS_IC[current_ic].cells.c_codes[i] * 0.0001;
        Serial.print(cellVoltage, 4);
        Serial.print(",");
        // Construct CAN message
        msg.id = 0x200; // Example ID for summary data
        msg.len = TOTAL_IC * BMS_IC[current_ic].ic_reg.cell_channels * 4;
        int index = 0;
        for (int ic = 0; ic < TOTAL_IC; ic++) {
          for (int j = 0; j < BMS_IC[ic].ic_reg.cell_channels; j++) {
            float summaryCellVoltage = BMS_IC[ic].cells.c_codes[j] * 0.0001;
            memcpy(&msg.buf[index], &summaryCellVoltage, sizeof(summaryCellVoltage));
            index += 4;
          }
        }
        canBus.write(msg);
      }
    }
  }
  Serial.println("\n");
}

