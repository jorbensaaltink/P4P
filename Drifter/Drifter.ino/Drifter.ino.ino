/*
 Web Server
 A simple web server that shows the value of the analog input pins.
 using an Arduino Wiznet Ethernet shield.

 Circuit:
 * Ethernet shield attached to pins 10, 11, 12, 13
 * Analog inputs attached to pins A0 through A5 (optional)

 created 18 Dec 2009 by David A. Mellis modified 9 Apr 2012, by Tom Igoe

 modified nov. 2015, bij Sibbele Oosterhaven (support GET-variables to set/unset digital pins)
 */

//Onderstaande regels worden gebruikt om relatief veel tekst te verwerken. Aangezien de Arduino maar weinig intern geheugen heeft (1 KB)
//worden deze teksen opgeslagen en verwerkt vanuit het programmageheugen. Je wordt niet geacht dit te begrijpen (maar dat mag wel).
//----------
const char cs0[] PROGMEM = "<strong>Opdracht 20 van het vak embedded systems 1</strong>"; 
const char cs1[] PROGMEM = "Dit voorbeeld is gebaseerd op het script in Voorbeelden->Ethernet->Webserver";
const char cs2[] PROGMEM = "De website die hier getoond wordt is dynamische gemaakt door de sensorwaarde";
const char cs3[] PROGMEM = "van kanaal 0 toe te voegen (refresh page, handmatig of automatisch).";
const char cs4[] PROGMEM = "<strong>Hoofdopdracht:</strong>";
const char cs5[] PROGMEM = "De site moet worden aangevuld met de mogelijkheid om variabelen mee te geven aan de server.";
const char cs6[] PROGMEM = "Dit kan op twee manier: met GET-variabelen, via de URL (192.168.1.3/?p8=1)";
const char cs7[] PROGMEM = "of, bij gebruik van Forms, met POST-variabelen, die niet zichtbaar zijn in de URL.";
const char* const string_table[] PROGMEM = {cs0, cs1, cs2, cs3, cs4, cs5, cs6, cs7};
char buffer[100];  
//----------

//Defines
#define maxLength     50  // string length
#define sensorPin     0   // on channel A0 
#define ledPin        8  
#define infoPin       9  



//Includes
#include <SPI.h>
#include <Ethernet.h>

// Enter a MAC address and IP address for your controller below. The IP address will be dependent on your local network:
//byte mac[] = { 0x40, 0x6C, 0x8F, 0x36, 0x84, 0x8A }; 
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };   // Ethernet adapter shield S. Oosterhaven
IPAddress ip(192, 168, 0, 70);

// Initialize the Ethernet server library (port 80 is default for HTTP):
EthernetServer server(80);

String httpHeader;           // = String(maxLength);
int arg = 0, val = 0;        // to store get/post variables from the URL (argument and value, http:\\192.168.1.3\website?p8=1)
bool command = false;
int aanuit = 1;


void setup() {
  
   //Init I/O-pins
   DDRD = 0xFC;              // p7..p2: output
   DDRB = 0x3F;              // p14,p15: input, p13..p8: output
   pinMode(ledPin, OUTPUT);
   pinMode(infoPin, OUTPUT);
   
   //Default states
   digitalWrite(ledPin, LOW);
   digitalWrite(infoPin, LOW);
  
   // Open serial communications and wait for port to open:
   Serial.begin(9600);
   while (!Serial) {
     ; // wait for serial port to connect. Needed for Leonardo only
   }

   // Start the Ethernet connection and the server:
   //Try to get an IP address from the DHCP server
   // if DHCP fails, use static address
   if (Ethernet.begin(mac) == 0) {
     Serial.println("No DHCP");
     Ethernet.begin(mac, ip);
   }
  
   //Start the ethernet server and give some debug info
   server.begin();
   Serial.println("Embedded Webserver");
   Serial.println("Ethernetboard connected (pins 10, 11, 12, 13 and SPI)");
   Serial.print("Ledpin at pin "); Serial.println(ledPin);
   Serial.print("Server is at "); Serial.println(Ethernet.localIP());
}


void loop() {
  // listen for incoming clients 
  EthernetClient client = server.available(); 
  
  //Webpage part
  if (client) {
    Serial.println("New client connected");
    // an http request ends with a blank line
    boolean currentLineIsBlank = true;
    
    while (client.connected()) {
      if (client.available()) {
        //read characters from client in the HTTP header
        char c = client.read();
        //store characters to string
        if (httpHeader.length() < maxLength) httpHeader += c;  // don't need to store the whole Header
        //Serial.write(c);                                     // for dbug only
        
        // at end of the line (new line) and the line is blank, the http request has ended, so you can send a reply
        if (c == '\n' && currentLineIsBlank) {
          // client HTTP-request received
          int sensorValue = analogRead(sensorPin);
          httpHeader.replace(" HTTP/1.1", ";");                // clean Header, and put a ; behind (last) arguments
          httpHeader.trim();                                   // remove extra chars like space
          Serial.println(httpHeader);                          // first part of header, for debug only
             
          // send a standard http response header
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println("Connection: close");          // the connection will be closed after completion of the response
          //client.println("Refresh: 3");               // refresh the page automatically every 3 sec
          client.println();

          // Web gedeelte
          
          client.println("<!DOCTYPE HTML>");
          client.println("<HTML>");
          client.println("<HEAD><TITLE>Verwerking.. | Drifter </TITLE></HEAD>");
          client.println("<STYLE>");
          client.println("</STYLE>");
          client.println("<BODY>");


          
          client.println("<H4 style='color:DarkBlue'>Waddenzee Drifter Meetwaarden Verwerken..</H4>"); 
          client.println("Mocht u blijven haken op deze pagina klik dan <a href='http://localhost:55432/index.html'>hier</a>");
          // output the value of analog input pin A0      
          //client.print("Meet waarde die binnenkomt (op Analog kanaal "); client.print(sensorPin); client.print("): ");
          //client.print(sensorValue);
          
          //grab the commands from url
          client.println("</P>");
          Serial.print("argument: ");
          Serial.println(arg);
          Serial.print("value: ");
          Serial.println(val);
          if (parseHeader(httpHeader, arg, val)) {   // search for argument and value, eg. p8=1
              Serial.print("argument: ");
              Serial.println(arg);
              Serial.print("value: ");
              Serial.println(val);
              digitalWrite(arg, val);                // Recall: pins 10..13 used for the Ethernet shield
              //client.print("Apparaat aan pin ");client.print(arg); client.print(" = "); client.println(val);
          }
          else client.println("No IO-pins to control");
          client.println("</P>");

         
          
          client.print("<script type='text/javascript'> window.location = 'http://localhost:55432/index.cshtml");
          if (checkCommand(httpHeader))
          {
            client.print("?windsnelheid=");
            client.print(sensorValue);  
          }        
          client.print("' </script>");

          // end of website
          client.println("</BODY>");
          client.println("</HTML>");
          break;
        }
        
        if (c == '\n') {
          // you're starting a new line
          currentLineIsBlank = true;
        }
        else if (c != '\r') {
          // you've gotten a character on the current line
          currentLineIsBlank = false;
        }
      }
    }
    // give the web browser time to receive the data
    delay(1);
    // close the connection:
    client.stop();
    httpHeader = "";
    Serial.println("Client disconnected");
  }
}

// Kijk of er een command is geselecteerd
bool checkCommand(String header)
{
  int result = 0;
  String pinStatus = header.substring(header.indexOf("?p") + 2);
  
  if (pinStatus.substring(pinStatus.indexOf("=") + 2, pinStatus.indexOf(";")) == "2" || pinStatus.substring(pinStatus.indexOf("=") + 1, pinStatus.indexOf(";")) == "2")
    {
      result = 1;
    }
    else
    {
      result = 0;  
    }
    return result;
}

// GET-vars after "?"   192.168.1.3/?p8=1
// parse header. Argument starts with p (only p2 .. p9)

bool parseHeader(String header, int &a, int &v)
{
  int result = 0;
  String pinStatus = header.substring(header.indexOf("?p") + 2);
  Serial.print("pin status: ");
  Serial.println(pinStatus);
  Serial.println(pinStatus.substring(1, pinStatus.indexOf("=") - 1));
  Serial.println(pinStatus.substring(pinStatus.indexOf("=") + 1, pinStatus.indexOf(";")));
  Serial.print("header: ");
  Serial.println(header);
  if (pinStatus.substring(1, pinStatus.indexOf("=") - 1) == "8")
  {
    a = 8;
    if (pinStatus.substring(pinStatus.indexOf("=") + 1, pinStatus.indexOf(";")) == "1")
    {
      v = 1;
      result = 1;
    }
    if (pinStatus.substring(pinStatus.indexOf("=") + 1, pinStatus.indexOf(";")) == "0")
    {
      v = 0;
      result = 1;
    }
  }
  if (pinStatus.substring(1, pinStatus.indexOf("=") - 1) == "9")
  {
    a = 9;
    if (pinStatus.substring(pinStatus.indexOf("=") + 1, pinStatus.indexOf(";")) == "1")
    {
      v = 1;
      result = 1;
    }
    if (pinStatus.substring(pinStatus.indexOf("=") + 1, pinStatus.indexOf(";")) == "0")
    {
      v = 0;
      result = 1;
    }
  }

  Serial.print("arg: ");
  Serial.println(a);
  Serial.print("val: ");
  Serial.println(v);
  
  return result;
}


