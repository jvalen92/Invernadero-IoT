void setup() {
  Serial.begin(9600);
}

void loop() {
  int input = analogRead(A0);
  Serial.print(input);
  Serial.print(" -> ");
  Serial.print(moistureSEN0114(input));
  Serial.println("%");

  Serial.print(input);
  Serial.print(" -> ");
  Serial.print(moistureSEN0193(input));
  Serial.println("%");

  delay(5000);
}

float moistureSEN0193(int analogInput) {//This sensor is less sentitive than SEN0114
  int dry = 520, water = 260;//Calibrate depending on sensor, this values are the standards
  return (double)((dry - analogInput) * 100) / (dry - water);//Calculating percentage
}

float moistureSEN0114(int analogInput) {//This sensor is more sensitive than SEN0193
  int dry = 0, water = 950;//Calibrete depending on sensor, this values are the standards
  return (double)((analogInput - dry) * 100) / (water - dry);//Calculating percentage
}