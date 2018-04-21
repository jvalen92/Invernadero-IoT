void setup() {
  Serial.begin(9600);
  }

void loop() {
  char c = Serial.read();
    if (c == 'H') {
        digitalWrite(LED_BUILTIN, HIGH);
	    Serial.print("Encendido");
	      } else if (c == 'L') {
	          digitalWrite(LED_BUILTIN, LOW);
		      Serial.print("Apagado");
		        } else {
			    Serial.print("Error");
			      }
			        delay(2000);
				}