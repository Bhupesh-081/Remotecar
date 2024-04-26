#include <WiFi.h>

// Define the GPIO pins connected to the L298N motor driver
const int enA = 13;
const int in1 = 12;
const int in2 = 14;

// Define WiFi parameters
const char *ssid = "RemoteCar";
const char *password = "password";

WiFiServer server(80);

void setup() {
  // Initialize Serial communication
  Serial.begin(115200);
  delay(1000);

  // Set GPIO pins as output
  pinMode(enA, OUTPUT);
  pinMode(in1, OUTPUT);
  pinMode(in2, OUTPUT);

  // Connect to WiFi network
  Serial.println();
  Serial.println("Configuring access point...");
  WiFi.softAP(ssid, password);
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);

  server.begin();
}

void loop() {
  WiFiClient client = server.available(); // Check for incoming clients

  if (client) {
    Serial.println("New client connected");
    String currentLine = ""; // Make a String to hold incoming data from the client

    while (client.connected()) { // Loop while the client's connected
      if (client.available()) { // If there's bytes to read from the client,
        char c = client.read(); // read a byte
        Serial.write(c); // Print it out the serial monitor

        // If the HTTP request has ended
        if (c == '\n') {
          // If the current line is blank, you got two newline characters in a row
          // that's the end of the client HTTP request, so send a response
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();

            // Send the HTML web page
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<title>Remote Car Control</title>");
            client.println("<style>");
            client.println("body {font-family: Arial, sans-serif; text-align: center; background-color: #f0f0f0; margin-top: 50px;}");
            client.println("h1 {color: #333333;}");
            client.println(".button {display: inline-block; background-color: #4CAF50; border: none; color: white; padding: 15px 25px; text-align: center; font-size: 16px; margin: 10px; cursor: pointer; border-radius: 10px; text-decoration: none; box-shadow: 0 4px #357a38;}");
            client.println(".button:hover {background-color: #45a049;}");
            client.println(".button:active {background-color: #357a38; box-shadow: 0 2px #357a38; transform: translateY(2px);}");
            client.println("</style>");
            client.println("</head>");
            client.println("<body>");
            client.println("<h1>Remote Car Control</h1>");
            client.println("<a href=\"/?action=forward\" class=\"button\">Forward</a><br>");
            client.println("<a href=\"/?action=left\" class=\"button\">Left</a>");
            client.println("<a href=\"/?action=stop\" class=\"button\">Stop</a>");
            client.println("<a href=\"/?action=right\" class=\"button\">Right</a><br>");
            client.println("<a href=\"/?action=backward\" class=\"button\">Backward</a>");
            client.println("</body></html>");

            // Break out of the while loop
            break;
          } else { // if you got a newline, then clear currentLine
            currentLine = "";
          }
        } else if (c != '\r') { // if you got anything else but a carriage return character,
          currentLine += c; // add it to the end of the currentLine
        }
      }
    }
    // Print the command received from the webpage
    Serial.print("Received command: ");
    Serial.println(currentLine);

    // Close the connection
    client.stop();
    Serial.println("Client disconnected");
  }
}

// Function to move the car forward
void forward() {
  digitalWrite(in1, HIGH);
  digitalWrite(in2, LOW);
  analogWrite(enA, 255); // Set speed
}

// Function to move the car backward
void backward() {
  digitalWrite(in1, LOW);
  digitalWrite(in2, HIGH);
  analogWrite(enA, 255); // Set speed
}

// Function to turn the car left
void left() {
  digitalWrite(in1, LOW);
  digitalWrite(in2, LOW);
  analogWrite(enA, 255); // Set speed
}

// Function to turn the car right
void right() {
  digitalWrite(in1, HIGH);
  digitalWrite(in2, HIGH);
  analogWrite(enA, 255); // Set speed
}
