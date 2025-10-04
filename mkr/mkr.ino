#include <WiFiNINA.h>

const char* ssid = "";          // sostituire con ssid
const char* password = "";      // sostituire con la password

WiFiServer server(5005);

void setup() {
  Serial.begin(9600);
  Serial1.begin(9600);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connessione...");
  }

  Serial.print("IP: ");
  Serial.println(WiFi.localIP());

  server.begin();
}

void loop() {
  WiFiClient client = server.available();
  if (client) {
    String msg = client.readStringUntil('\n');
    msg.trim();

    if (msg.length() > 0) {
      Serial.println("Ricevuto: " + msg);
      Serial1.print("#" + msg + ";\n");
    }

    client.println("OK: " + msg);
    client.stop();
  }
}
