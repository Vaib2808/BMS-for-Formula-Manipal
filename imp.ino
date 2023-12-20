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
