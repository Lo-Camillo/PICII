#include <ESP8266WiFi.h>
#include <Servo.h>

// Enter your wifi network name and Wifi Password
const char* ssid = "Hotspot";
const char* password = "lohayne123";

// Set web server port number to 80
WiFiServer server(80);

// Variable to store the HTTP request
String header;

Servo s; 
int pos; 

// These variables store current output state of LED
String alarm = "off";
String button = "off";
String blinks = "off";
int buttonpressed = 0;
int check = 0;

// Assign output variables to GPIO pins
const int pinLED = 4;
const int pinBuzzer = 5;
const int pinServer = 2;
const int pushButton = 14;

// Current time
unsigned long currentTime = millis();
// Previous time
unsigned long previousTime = 0; 
// Define timeout time in milliseconds (example: 2000ms = 2s)
const long timeoutTime = 2000;

// Define a function to blink the LED and sound the alarm
void triggerAlarm(){
  digitalWrite(pinLED, HIGH);
  digitalWrite(pinBuzzer, HIGH);
  delay(1000);
  digitalWrite(pinLED, LOW);
  digitalWrite(pinBuzzer, LOW);
  delay(1000); 
  }

void setup() {
Serial.begin(115200);
// Initialize the output variables as outputs
pinMode(pinLED, OUTPUT);
pinMode(pinBuzzer, OUTPUT);
pinMode(pinServer,OUTPUT);
pinMode(pushButton, INPUT);
// Set outputs to LOW
digitalWrite(pinLED, LOW);
digitalWrite(pinBuzzer, LOW);
digitalWrite(pinServer, LOW);

s.attach(pinServer);
s.write(0); 

// Connect to Wi-Fi network with SSID and password
Serial.print("Connecting to ");
Serial.println(ssid);
WiFi.begin(ssid, password);
while (WiFi.status() != WL_CONNECTED) {
delay(500);
Serial.print(".");
}
// Print local IP address and start web server
Serial.println("");
Serial.println("WiFi connected.");
Serial.println("IP address: ");
Serial.println(WiFi.localIP());
server.begin();
}

void loop(){
WiFiClient client = server.available(); // Listen for incoming clients

if (client) { // If a new client connects,
Serial.println("New Client."); // print a message out in the serial port
String currentLine = ""; // make a String to hold incoming data from the client
currentTime = millis();
previousTime = currentTime;
while (client.connected() && currentTime - previousTime <= timeoutTime) { // loop while the client's connected
currentTime = millis(); 
if (client.available()) { // if there's bytes to read from the client,
char c = client.read(); // read a byte, then
Serial.write(c); // print it out the serial monitor
header += c;
if (c == '\n') { // if the byte is a newline character
// if the current line is blank, you got two newline characters in a row.
// that's the end of the client HTTP request, so send a response:
if (currentLine.length() == 0) {
// HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
// and a content-type so the client knows what's coming, then a blank line:
client.println("HTTP/1.1 200 OK");
client.println("Content-type:text/html");
client.println("Connection: close");
client.println();

// turns the GPIOs on and off
if (header.indexOf("GET /alarm/on") >= 0) {
alarm = "on";
} else if (header.indexOf("GET /alarm/off") >= 0) {
alarm = "off";
}
else if (header.indexOf("GET /pacientok") >= 0) {
alarm = "off";
buttonpressed = 0;
check = 0;
}

// Display the HTML web page
client.println("<!DOCTYPE html><html>");
client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
client.println("<link rel=\"icon\" href=\"data:,\">");
// CSS to style the on/off buttons 
client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
client.println(".buttonRed { background-color: #ff0000; border: none; color: white; padding: 16px 40px; border-radius: 60%;");
client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
client.println(".buttonOff { background-color: #77878A; border: none; color: white; padding: 16px 40px; border-radius: 70%;");
client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}</style></head>");

// Web Page Heading
client.println("<h1>Porta Medicamentos Digital</h1>");

if(alarm=="off" and !buttonpressed){
  client.println("<body><p> Avise que esta na hora de tomar os medicamentos! </p>");
  client.println("<p><a href=\"/alarm/on\"><button class=\"button buttonOff\">ATIVAR</button></a></p>");
  
}else if (alarm == "on" and !buttonpressed and check == 0){
  client.print(("<body>\n"));
  client.print(("<meta http-equiv=\"refresh\" content=\"3\";>")); 
  client.println("<p> Esperando paciente... </p>");
  client.println("<p><a href=\"/alarm/off\"><button class=\"button buttonRed\">ATIVADO</button></a></p>");
}

if (buttonpressed == 1){
  client.println("<body><p> Paciente ativou! </p>");
  client.println("<p><a href=\"/pacientok\"><button class=\"button buttonRed\">VOLTAR</button></a></p>");
  alarm = "off";
}
client.println("</body></html>");

// The HTTP response ends with another blank line
client.println();
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
// Clear the header variable
header = "";
// Close the connection
client.stop();
Serial.println("Client disconnected.");
Serial.println("");
}

int touchState = digitalRead(pushButton); // read new state

//Serial.print("ButtonState: ");
//Serial.println(touchState);

if (touchState == HIGH and alarm == "on") {
    digitalWrite(pinLED, LOW);
    digitalWrite(pinBuzzer, LOW);
    alarm = "off";
    buttonpressed = 1;
    for(pos = 0; pos <= 180; pos++){ 
    s.write(pos);
    //delay(15); 
  }
}
else if (touchState == LOW and alarm == "on") {
  triggerAlarm();
}
}
