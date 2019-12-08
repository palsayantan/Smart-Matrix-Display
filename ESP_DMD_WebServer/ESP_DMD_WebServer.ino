#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>

#include <SPI.h>
#include <DMD2.h>
#include <fonts/Arial14.h>

const char MAIN_page[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
<body>
<center>
<h1>Matrix Display Controller<h1>
<form action="/action_page">
  Enter Text :<br><br>
  <input type="text" name="msg" value="Enter your text here">
  <br><br>
  <input type="submit" value="Submit">
</form> 
</center>
</body>
</html>
)=====";

//SSID and Password of your WiFi router
const char* ssid = "Sayantan";
const char* password = "sayantan";

ESP8266WebServer server(80); //Server on port 80

// Set Width and Hight to the number of displays you have
const int WIDTH = 1;
const int HIGHT = 1;

const uint8_t *FONT = Arial14;

/*
  Pin Defination
   nOE - GPIO5
   A - GPIO16
   B - GPIO2
   CLK - GPIO14
   SCLK - GPIO12
   r/Data - GPIO13
 */

SPIDMD dmd(WIDTH , HIGHT, 5, 16, 2, 12);  // DMD controls the entire display
DMD_TextBox box(dmd);  // "box" provides a text box to automatically write to/scroll the display
char MESSAGE[255]; //Store the message

void handleRoot() {
 String s = MAIN_page; //Read HTML contents
 server.send(200, "text/html", s); //Send web page
}

void handleForm() {
  String msg = server.arg("msg"); 
  int msg_len = msg.length() + 1; 
  MESSAGE[msg_len];
  msg.toCharArray(MESSAGE, msg_len);

  String s = MAIN_page;
  server.send(200, "text/html", s); 
  Scroll();
}

void Scroll(){
  const char *next = MESSAGE;
  while(*next) {
    Serial.print(*next);
    box.print(*next);
    delay(200);
    next++;
  }
}

void setup(void){
  Serial.begin(115200);
  
  WiFi.begin(ssid, password);     //Connect to your WiFi router
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  //If connection successful show IP address in serial monitor
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println("WiFi");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());  //IP address assigned to your ESP

  server.on("/", handleRoot);      //Which routine to handle at root location
  server.on("/action_page", handleForm); //form action is handled here

  server.begin();                  //Start server
  Serial.println("HTTP server started");
  dmd.setBrightness(255);
  dmd.selectFont(FONT);
  dmd.begin();
}

void loop(void){
  server.handleClient();          //Handle client requests
}
