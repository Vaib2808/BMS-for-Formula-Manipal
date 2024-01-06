
    case 4: // Read Cell Voltage Registers
      wakeup_sleep(TOTAL_IC);
      error = LTC6813_rdcv(SEL_ALL_REG,TOTAL_IC,BMS_IC); // Set to read back all cell voltage registers 
      check_error(error);
      print_cells(DATALOG_DISABLED);
      break;

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


