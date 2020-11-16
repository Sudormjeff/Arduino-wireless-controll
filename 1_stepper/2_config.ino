int rom_save[13];

void setting(){ //pin config
  int temp=0;
  nrf24.enableAckPayload(); 
  for (int i =0; i<12; i++){
    rom_save[i]=EEPROM[i];
    Serial.println(EEPROM[i]);
  }
  send_again:
  if(not(nrf24.write(&rom_save,sizeof(rom_save)))){
    delay(1000);
    temp++;
    if (temp >9){ //Timeout
      return debug(); //Use internal storage as pin_no
    }
    goto send_again;
  }

  nrf24.startListening();
  while(not nrf24.available()){}
  nrf24.read(&rom_save,sizeof(rom_save));
  nrf24.stopListening();
  
  

  bool check = false;
  for (int i =0; i <12; i++){
    if (rom_save[i] != EEPROM[i]){
      check = true;
    }
  }

  if (check){modify();}
  else      { debug();}
  nrf24.writeAckPayload(1,&rom_save,sizeof(rom_save)); //resend package to tell joystick side start operation
  nrf24.flush_tx();
}
void modify(){ //If pin layout modified
  for (int i =0; i<3;i++){ 
    Dir_Pin[i]=rom_save[i*2];
    Pulse_Pin[i]=rom_save[i*2 +1];
    Upper_Lim[i]=rom_save[i*2 +6];
    Lower_Lim[i]=rom_save[i*2 +7];
    EEPROM.update(i*2,rom_save[i*2]);
    EEPROM.update(i*2 +1,rom_save[i*2 +1]);
    EEPROM.update(i*2 +6,rom_save[i*2 +6]);
    EEPROM.update(i*2 +7,rom_save[i*2 +7]);
    pinMode(Dir_Pin[i] ,OUTPUT);
    pinMode(Pulse_Pin[i] ,OUTPUT);
    pinMode(Upper_Lim[i] ,INPUT);
    pinMode(Lower_Lim[i] ,INPUT);
  }
  testing(); //motor calibration
}

void debug(){ //Bypass mem check if timeout, use last config
  
  for (int i =0; i<3;i++){
    in_rev[i] = bitRead(EEPROM[14],i);
    Dir_Pin[i]=EEPROM[i*2];
    Pulse_Pin[i]=EEPROM[i*2 +1];
    Upper_Lim[i]=EEPROM[i*2 +6];
    Lower_Lim[i]=EEPROM[i*2 +7];
    pinMode(Dir_Pin[i] ,OUTPUT);
    pinMode(Pulse_Pin[i] ,OUTPUT);
    pinMode(Upper_Lim[i] ,INPUT);
    pinMode(Lower_Lim[i] ,INPUT);
  }
}
