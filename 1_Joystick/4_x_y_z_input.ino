int x_y_out(int an_pin_no, int speed_motor){
  int dir = 1;
  val = analogRead(an_pin_no)-511;
  if (val <0){
    val = val - 1 ; 
    dir = -1;
  }
  val = abs(val);
  if (val < 64){ //Dead zone
    if (val < 8){ //Prevent random signal
      return 0; 
    }
    val = speed_motor/2; //constant mode
  }
  else{
    val = map(val,64,512,speed_motor/2,speed_motor+16);
  }
  return (val)*dir;
}

int z_out(int speed_motor,int val_z, bool add_or_minus){
    if ((val_z < 0 and add_or_minus) or (val_z > 0 and not add_or_minus)){
      return 0;
    }
    if (val_z == 0){
      val_z = 1;
    }
    else if (abs(val_z) < speed_motor ){
      if (abs(val_z) == 1){
        val_z = abs(val_z) +15;
      }
      else{
        val_z = abs(val_z) +16;
      }
    }
    else{
      val_z = speed_motor ;
      if (speed_motor ==0){
        val_z = speed_motor + 1;
      }
    }
    if (add_or_minus){
      return val_z;
    }
    return -abs(val_z);
}
