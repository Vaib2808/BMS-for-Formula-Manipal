   case 3: // Start Cell ADC Measurement
      wakeup_sleep(TOTAL_IC);
      LTC6813_adcv(ADC_CONVERSION_MODE,ADC_DCP,CELL_CH_TO_CONVERT);
      conv_time = LTC6813_pollAdc();
      print_conv_time(conv_time);
      break;
    
    case 4: // Read Cell Voltage Registers
      wakeup_sleep(TOTAL_IC);
      error = LTC6813_rdcv(SEL_ALL_REG,TOTAL_IC,BMS_IC); // Set to read back all cell voltage registers 
      check_error(error);
      print_cells(DATALOG_DISABLED);
      break;

    case 10: //Start Combined Cell Voltage and Sum of cells
      wakeup_sleep(TOTAL_IC);
      LTC6813_adcvsc(ADC_CONVERSION_MODE,ADC_DCP);
      conv_time = LTC6813_pollAdc();
      print_conv_time(conv_time);
      wakeup_idle(TOTAL_IC);
      error = LTC6813_rdcv(SEL_ALL_REG, TOTAL_IC,BMS_IC); // Set to read back all cell voltage registers
      check_error(error);
      print_cells(DATALOG_DISABLED);
      wakeup_idle(TOTAL_IC);
      error = LTC6813_rdstat(SEL_REG_A,TOTAL_IC,BMS_IC); // Set to read back stat register A
      check_error(error);
      print_sumofcells();
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


void print_sumofcells(void)
{
  for (int current_ic =0 ; current_ic < TOTAL_IC; current_ic++)
  {
    Serial.print(F(" IC "));
    Serial.print(current_ic+1,DEC);
    Serial.print(F(" SOC:"));
    Serial.print(BMS_IC[current_ic].stat.stat_codes[0]*0.0001*30,4);
    Serial.print(F(","));
  }
  Serial.println("\n");
}

