
int n;
float value,floorwetness=0, uvligth=0, whiteligth=0, infraredligth=0, ph=0;
 


void setup() {
  
  Serial.begin(9600);

}

void loop() {

  if(Serial.available()){
   String cadena = Serial.readString();
   int pyc = cadena.indexOf(';');
   n= cadena.substring(0,pyc).toInt();
   value=cadena.substring(pyc+1).toFloat();
   delay(10);

  
      if (n==-1){
        getFloorWetness();
      }
      else if (n==-2){
        getPh();
      }
      else if (n==-3){
        getInfraredLigth();
      }
      else if (n==-4){
        getUvLigth();
      }
      else if (n==-5){
        getWhiteLigth();
      }
      else if (n==-11){
        setFloorWetness(value);
      }
      else if (n==-12){
        setPh(value);
      }
      else if (n==-13){
        setInfraredLigth(value);
      }
      else if (n==-14){
        setUvLigth(value);
      }
      else if (n==-15){
        setWhiteLigth(value);
      }
      else{}
    
    
    
  }

}


void getFloorWetness(){
  delay(1);
  Serial.println(floorwetness);
  delay(1);
  }

void getPh(){
  delay(1);
  Serial.println(ph);
  delay(1);
  }

 void getInfraredLigth(){
  delay(1);
  Serial.println(infraredligth);
  delay(1);
  }

 void getUvLigth(){
  delay(1);
  Serial.println(uvligth);
  delay(1);
  }

  void getWhiteLigth(){
  delay(1);
  Serial.println(whiteligth);
  delay(1);
  }

  void setFloorWetness(float value){
  
    floorwetness=value;
  
  }

  void setPh(float value){
  
    ph=value;
 
  }

  void setInfraredLigth(float value){
 
    infraredligth=value;
 
  }

  void setUvLigth(float value){
 
   uvligth=value;
 
  }

  void setWhiteLigth(float value){
  
   whiteligth=value;
  
  }


  
