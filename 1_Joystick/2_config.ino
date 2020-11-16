#include <EEPROM.h> //Save config & calibration 
//EEPROM only ~100,000 write cycles
String pin[] = {"Dir_Pin_x","Pulse_Pin_x", "Dir_Pin_y","Pulse_Pin_y","Dir_Pin_z","Pulse_Pin_z","Upper_Lim_x","Lower_Lim_x","Upper_Lim_y","Lower_Lim_y","Upper_Lim_z","Lower_Lim_z"};

char no; //char store for answer
int sum; //int store for memory check
int cache; //individual val of EEPROM
String tem; 
int rom_save[13]; //Pin storage
int count_down=0; 
bool check;

void axis_rev(){ //Joystick side save config of reverse axis only
  String axis[] = {"x ","y ","z "};
  
  Serial.println("Current axis reverse setting:");
  
  for (int i =0; i<3; i++){
    cache = bitRead(EEPROM[14],i);
    Serial.print(axis[i]);
    Serial.println(cache);
  }
  Serial.println("Continue with these setting? y/n");
  tem = Serial.readStringUntil('\n');
  rep:
  while(!Serial.available()){}
  no = Serial.read();
    
  if (no =='n'){
    cache =0;
    for (int i =0; i<3; i++){
      tem = Serial.readStringUntil('\n');
      re_ax:
      Serial.println(axis[i]);
      Serial.println("Reverse the axis? y/n");

      while(!Serial.available()){}
      no = Serial.read();
      if (no == 'y'){
        bitWrite(cache,i,1);
        }
      else if (no !='n'){
        Serial.println("Invalid input, try again. y/n");
        goto re_ax;
        }
    }
    EEPROM.write(14,cache); 
  }
    
  else if(no!='y') {
    Serial.println("Invalid input, try again. y/n");
    goto rep;
  }
  
  for (int i =0; i<3; i++){
    if (bitRead(EEPROM[14],i) ==1){
      rev[i] = true;
    }
  }
  tem = Serial.readStringUntil('\n');
}


void read_write(){ //Function for transmitting to motor side
  axis_rev();
  count_down = 0;
  send_again:
  Serial.println("Sending to motor side...");
  
  if (not(nrf24.write(&rom_save,sizeof(rom_save)))){
    delay(1000);
    count_down ++;
    
    if(count_down >19){
      Serial.println("Motor side lose response. Switch to last config.");
      return;
    }
    goto send_again;    
  }
  
  Serial.println("Tranmitted");
}



void int_write(int pos, int num){ //Write int(2bytes) to EEPROM
  EEPROM.write(pos,num >> 8);
  EEPROM.write(pos+1, num & 0xFF);
}
int int_read(int pos){ //Read int(2bytes) from EEPROM
  byte b_1 = EEPROM.read(pos);
  return (b_1 << 8) + EEPROM.read(pos+1);
}

void mem_wipe(){ //Clear all data in EEPROM (Not recommend)
  Serial.println("Memory Wipe? y/n");
  while(!Serial.available()){}
  no = Serial.read();
  if (no == 'y'){
    for (int i = 0; i <EEPROM.length(); i++){
      EEPROM.update(i,0);
    }
    Serial.println("Wipe successful");
  }
}

void mem_config(){ //Set pin number for signal output
  Serial.println("Type corresponding Pin no. to config");
  sum = 0;
  tem = Serial.readStringUntil('\n');
  
  for (int i=0; i < 12; i++){
    Serial.println(pin[i]);
    while(!Serial.available()){}
    tem = Serial.readStringUntil('\n');
    cache = tem.toInt();
    Serial.println(cache);
    sum += cache;
    EEPROM[i] = cache;
    rom_save[i] = cache;
    delay(300);
  }
  
  //Serial.println(sum);
  int_write(12, sum); // Checking byte
  rom_save[12] = sum;
  
  read_write();
  
  Serial.println("Config finished");
  for (int i=0; i < 12; i++){
    Serial.print(pin[i]); 
    Serial.print(" ");
    Serial.println(EEPROM.read(i));
  }
}

void check_mem(){ //Main function for memory checking and config
  nrf24.enableAckPayload();
  Serial.println("Memory checking...");
  sum = 0;
  check = false;
  unsigned long time_count = millis();
  nrf24.startListening();
  while(not nrf24.available()){ //timeout, assume motor side has config itself. config axis rev only.
    if(millis() > time_count + 20000){ //20s bf timeout
      Serial.println("Timeout. Motor side will use latest modified config."); 
      return axis_rev();
    }
  }
  nrf24.read(&rom_save,sizeof(rom_save));
  nrf24.stopListening();

  for (int i=0; i < 12; i++){ //Check if motor side has same config as joystick side
    if (rom_save[i] != EEPROM.read(i)){
      check = true; //Not sync
    }
    sum += rom_save[i];
  }
  
  nrf24.writeAckPayload(1,&rom_save,sizeof(rom_save));
  nrf24.flush_tx();
  if (sum == int_read(12) and sum != 0 and not check){ //If no error
    Serial.println("Config synchronized");
    sync:
    Serial.println("Previous config found.");
    for (int i=0; i < 12; i++){
      rom_save[i] = EEPROM.read(i);
      Serial.print(pin[i]); 
      Serial.print(" ");
      Serial.println(rom_save[i]);
    }
    Serial.println("Continue with previous config? y/n");
    re:
    while(!Serial.available()){}
    no = Serial.read();
    if (no == 'y'){
      read_write();
      return; 
    }
    else if (no =='n'){
      mem_config();
      return; 
    }
    else{
      Serial.println("Invalid input, try again. y/n");
      goto re;
    }
  }
  else if(not check){
    Serial.println("Config corrupted or not set up. Config now");
    mem_config();
  }
  else{
    Serial.println("Config not synchronize.");
    goto sync;
  }
}
