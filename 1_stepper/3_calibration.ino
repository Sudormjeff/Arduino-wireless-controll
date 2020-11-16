void moving(bool limit[3]){
    for (int j =0; j<3; j++){
      if (not limit[j]){
        digitalWrite(Pulse_Pin[j],HIGH);
      }
    }
    delayMicroseconds(300);
    for (int j =0; j<3; j++){
      digitalWrite(Pulse_Pin[j],LOW);
    }
    delayMicroseconds(300);
}



void testing(){//Orientate upper and lower limit sensor and calibration
  
    int max_pos[3] ={0,0,0};
    bool limit[3];
    
    for (int i=0; i< 3; i++){
      digitalWrite(Dir_Pin[i],HIGH);
      limit[i] = false;
    }
    
    while(not limit[0] and not limit[1] and not limit[2]){
      moving(limit);
      for (int i=0; i< 3; i++){
        limit[i] = digitalRead(Upper_Lim[i]) or digitalRead(Lower_Lim[i]);
      }
    }
    
    byte lim_mem = 0;
    for (int i=0; i< 3; i++){
      digitalWrite(Dir_Pin[i],LOW);
      limit[i] = false;
      if (digitalRead(Upper_Lim[i]) == HIGH){ //determine the axis dir corr to dir pin
        in_rev[i] = true;
        bitWrite(lim_mem,i,1); //write to EEPROM to restore setting
      }
      else{
        in_rev[i] = false;
      }
    }
    
    EEPROM.update(14,lim_mem);
    while(not limit[0] and not limit[1] and not limit[2]){
      moving(limit);
      
      for (int i = 0; i<3; i++){
        limit[i] = digitalRead(Upper_Lim[i]) or digitalRead(Lower_Lim[i]);
        if (not limit[i]){
          max_pos[i] ++; //Determine range
        }
      }
      
    }
    
    for (int i=0; i< 3; i++){
      Serial.println(max_pos[i]);
      digitalWrite(Dir_Pin[i],HIGH);
      for (int j= max_pos[i]/2 ; j>0; j--){ //recenter
        digitalWrite(Pulse_Pin[i],HIGH);
        delayMicroseconds(300);
        digitalWrite(Pulse_Pin[i],LOW);
        delayMicroseconds(300);
      }
    }
  
  Serial.println("Calibration finished");


}
