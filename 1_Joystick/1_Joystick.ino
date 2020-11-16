//Main Function(Joystick side remote)
#include <EEPROM.h> //Save config & calibration 
#include <RF24.h> //RF communication
#include <printf.h>

RF24 nrf24(10,9); //CE, CSN pin, define nrf module
byte addresses[][6] = {"00001","00002"};

int sw[4] ; //sw: switch
int joyst_x = A0;
int joyst_y = A1;
int joyst_p = 6; //joystick press button

int val_x,val_y,val_z = 0,val; //val: cache 
int speed_motor = 0; //speed of motor, controlled by sw 2,4
int output[3]; //output to motor side, [0] = val_x, [1]= val_y, [2] = val_z

unsigned long sw_time[]={0,0,0,0,0},current_1,current_2; //last switch time recorded
int joyst_sw_delay = 200; 
int panel_sw_delay[] = {100,100,100,100};
bool rev[3] ={false,false,false}; //axis reverse
bool mode = true; //mode: two mode in total, logic operate in motor side

void setup() {

  //Initlize 
  Serial.begin(9600);
  printf_begin();
  pinMode(joyst_p, INPUT);
  
  for (int i=0; i<4; i++){
    sw[i] = i+2; //sw 1-4 in pin 2-5
    pinMode(sw[i],INPUT);
  }
  
  //nrf24 init
  nrf24.begin();
  Serial.print(nrf24.isChipConnected());
  nrf24.setPALevel(RF24_PA_LOW);
  nrf24.printDetails();
  nrf24.openWritingPipe(addresses[0]);
  nrf24.openReadingPipe(1,addresses[1]);
  check_mem(); //Used for config motor side, store I/O pin numbers
  nrf24.stopListening();
  
}

void loop() {
  if (Serial.available()){
    Serial.println("Config axis reverse");
    axis_rev();
  }
  
  current_1 = millis();
  for (int i =0; i<4; i++){
    val = digitalRead(sw[i]);
    if (val == 0 ){
        if (sw_time[i]+panel_sw_delay[i] < current_1){
          
          if (i == 0){
              val_z = z_out(speed_motor,val_z, true); //Accelerating z steps
          }
          else if (i == 1){
            if (speed_motor != 0){ //Decrease speed
              speed_motor = speed_motor - 16;
            }
          }
          else if (i == 2){
              val_z = z_out(speed_motor,val_z, false);
          }
          else{
            if (speed_motor != 112){ //Increase speed, max speed = 112 + 16 = 128
              speed_motor = speed_motor + 16;  
            }
          }
          
        }
      sw_time[i] = current_1;
    }
    
    else { //Reset to zero if not pushing switch
      if (i == 0 and val_z > 0){
        val_z = 0;
      }
      if (i ==2 and val_z < 0){
        val_z = 0;
      }
    }

  }
  
  current_2 = millis();
  if (digitalRead(joyst_p) == 0 && sw_time[4]+joyst_sw_delay < current_2){ //Change to const speed (ignore input)
    mode = not mode;
    sw_time[4] = current_2;
  }

  delay(100);
  
  val_x = x_y_out(joyst_x,speed_motor);
  //analogWrite(an_led[0],abs(val_x));

  val_y = x_y_out(joyst_y,speed_motor);
  //analogWrite(an_led[1],abs(val_y));

  if (mode){
    output[0] = val_x;
    output[1] = val_y;
    output[2] = val_z; 
  }

  for (int i =0; i<3; i++){
    if (rev[i] and mode){
      output[i] *=-1;
    }
    Serial.print(output[i]);
    Serial.print(" ");
  }
  Serial.println(" ");
  nrf24.write(&output,sizeof(output));
}
