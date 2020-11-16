//Main Function(Motor side remote)
//Motor side has to operate faster than joystick side sending
#include <EEPROM.h> //Save config & calibration 
#include <RF24.h> //RF communication
#include <printf.h>

int Dir_Pin[3];     //Direction pin, High or Low
int Pulse_Pin[3];   //Pulse for motor movement
int Upper_Lim[3];
int Lower_Lim[3];
int input[3];       
int delay_time= 350; //Total delay = delay_time *2
int sensor_read[2];

RF24 nrf24(3,2); //CE, CSN pin, define nrf module
byte addresses[][6] = {"00001","00002"}; //addresses for nrf module

bool dir[3],in_rev[3];
float linspace[3],cur_step[3];

void setup() {
  Serial.begin(9600);
  //Serial.println("wewewewewew");
  printf_begin();
  //nrf24 init
  nrf24.begin();
  Serial.print(nrf24.isChipConnected());
  nrf24.setPALevel(RF24_PA_LOW);
  nrf24.printDetails();
  nrf24.openWritingPipe(addresses[1]);
  nrf24.openReadingPipe(1,addresses[0]);
  
  nrf24.stopListening();
  
  setting();
  //testing();
  nrf24.startListening();
}

void loop() {
  
  if (nrf24.available()) {
    nrf24.read(&input, sizeof(input));
    
     
      for (int i=0; i<3; i++){ //data pre process
        cur_step[i] = 0;

        
        Serial.print(input[i]);
        Serial.print(" ");
        if (digitalRead(Upper_Lim[i]) == true or digitalRead(Lower_Lim[i]) == true){
          Serial.println("Limit reached");
        }
        
        
        
        if (in_rev[i]){ //Reverse limit detection
          sensor_read[1] = digitalRead(Upper_Lim[i]);
          sensor_read[0] = digitalRead(Lower_Lim[i]);
        }
        else{
          sensor_read[0] = digitalRead(Upper_Lim[i]);
          sensor_read[1] = digitalRead(Lower_Lim[i]);
        }
        

        
        if (input[i]<0 and sensor_read[0]!= true){ //Set direction and check if motor reaching limit
          dir[i] = true;
        }
        else if (sensor_read[1] != true){
          dir[i] = false;
        }
        else {dir[i] = true;} //if (input[i] > 0 and sensor_read[1] == true)
        
        input[i] = abs(input[i]);
        
        if (dir[i]){ //Direction set
          digitalWrite(Dir_Pin[i],LOW);
        }
        else{
          digitalWrite(Dir_Pin[i],HIGH);
        }
        
        linspace[i] = 128/(input[i]-1); //Linspace for smooth motor movement
        cur_step[i] += linspace[i];
        
      }
      Serial.println();
      
      
      for (int i =1; i<=128; i++){ //Each package move for 128ms
        for (int j=0; j<3; j++){
          if( i == round(cur_step[j])){
              cur_step[j] += linspace[j];
              digitalWrite(Pulse_Pin[j],HIGH);
          }
        }
        delayMicroseconds(delay_time); 
        for (int j=0; j<3; j++){digitalWrite(Pulse_Pin[j],LOW);}
        delayMicroseconds(delay_time);
      }
    }
    
  
  
}
