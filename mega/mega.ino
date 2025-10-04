String buffer = "";
bool reading = false;

void setup() {
  Serial.begin(9600);
  Serial1.begin(9600);
  Serial.println("\nMega pronto");
}

void loop() {
  while (Serial1.available()) {
    char c = Serial1.read();

    if (c == '#') {
      reading = true;
      buffer = "";
    } 
    else if (c == ';' && reading) {
      reading = false;
      buffer.trim();
      Serial.println("Mossa ricevuta: " + buffer);
      handleMove(buffer);
    } 
    else if (reading) {
      buffer += c;
    }
  }
}

void handleMove(String move) {
  Serial.println("Esegui mossa: " + move);
  // codice x dobot
  
}
