// Include SoftwareSerial and Wifi library
#include <SoftwareSerial.h>
#include <RingBuf.h>
#include <Servo.h>
#include "WiFiEsp.h"
#include <math.h>

#define HOUR (60 * 60 * 1000L)
#define MINUTE (60 * 1000L)



///////////////////////////////////////////////////////////
// Declaration of time from web and special time variables.
// Some initialization.
// Time only gets gotten if a client has set special feeding times.
// The functions for time getting is used only for this.
///////////////////////////////////////////////////////////

char timeServer[] = "www.worldclockapi.com";

// This is the hour and minute of day gotten from web.
int hourOfDay = 0;
int minuteOfDay = 0;

// Setting boolean
bool setToFeedSpecialTimes = false;

// User information about which hours to feed.
bool feedingTheseHours[24];

// If we are supposed to feed @12 and @13, then,
// when feeding @12, we set nextPossibleSpecialTime
// to 13, such that feeding does not get triggered by
// values of 12.
bool nextPossibleSpecialTime = 0;
int lastFeed = -1;

// THEN: what time of day was recorded, in milliseconds of a day (12h, 00m -> 43200000ms)
// NOW: What the time is now. 
// These only get updated if it has been set by a client that the device should
// feed using a set amount of hours.
long int THEN;
long int NOW;

// Lasdt time gotten time.
long int lastTimeGottenTime = 0;
// Time is not gotten from the beginning.
bool timeGotten = false;

// A simple function.
void resetFeedingHours(){
  for(int i = 0; i < 24; i++) {
   feedingTheseHours[i] = false;
  }
}








///////////////////////////////////////////////////////////
// Settings and variables for feeding per interval.
///////////////////////////////////////////////////////////

// Setting booleans.
bool setToFeedPerHour = false;
bool setToFeedPerMinute = false;
// Holds millisecond record of the last time we fed by interval setting.
// Will be used to know how 
unsigned long lastTimeFedByInterval = 0L;

// Every this interval amount. One or the other is used.
int everyThisHours;
int everyThisMinutes;








///////////////////////////////////////////////////////////
// Servo stuff.
///////////////////////////////////////////////////////////

Servo servo_9;

void feed(){
  Serial.println("FEEEEEEEEEEEEEEEEEEEEEEDING");
  pinMode(4, OUTPUT);
  servo_9.write(10);
  delay(1000);
  servo_9.write(120);
  delay(1000);
  pinMode(4, INPUT);
}








///////////////////////////////////////////////////////////
// Wifi stuff.
///////////////////////////////////////////////////////////

// Create WiFi module object on GPIO pin 6 (RX) and 7 (TX)
SoftwareSerial Serial1(6, 7);

// Declare and initialise global arrays for WiFi settings
char ssid[] = "Johan";
char pass[] = "johan123";

// Declare and initialise variable for radio status 
int status = WL_IDLE_STATUS;

// Create a web server on port 80
WiFiEspServer server(80);

// Used to make easy sense of characters gotten by HTTP.
RingBuffer buf(8);








///////////////////////////////////////////////////////////
// One time per session setup.
///////////////////////////////////////////////////////////

void setup() {

  resetFeedingHours();
  servo_9.attach(4);
  // Set servo to a good position.
  servo_9.write(120);
  delay(2000);
  pinMode(4, INPUT);
        
  // Initialize serial for debugging
  Serial.begin(115200);
  
  // Initialize serial for ESP module
  Serial1.begin(9600);
  
  // Initialize ESP module
  WiFi.init(&Serial1);

  // Check for the presence of the shield
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("WiFi shield not present");
    // Don't continue
    while (true);
  }
  
  // Attempt to connect to WiFi network
  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    
    // Connect to WPA/WPA2 network
    status = WiFi.begin(ssid, pass);
  }

  Serial.println("You're connected to the network");
  printWifiStatus();

  // Start the web server
  server.begin();
}








///////////////////////////////////////////////////////////
// Serves the website to a client.
///////////////////////////////////////////////////////////

void website(WiFiEspClient* client){
  // Send a standard HTTP response header
  client->print(F("<!DOCTYPE HTML>\r\n"));
  client->print(F("<html>\r\n"));
  client->print(F("<head>\r\n"));
  client->print(F("<link rel=\"stylesheet\" type=\"text/css\" href=\"http://mrryyi.github.io/mandles/css/main.css\">\r\n"));
  client->print(F("<link href=\"about:blank\" rel=\"shortcut icon\">\r\n"));
  client->print(F("</head>\r\n"));
  client->print(F("<body>\r\n"));
  client->print(F("<div class=\"f d\">\r\n"));
  client->print(F("<div id=\"pr\"></div>\r\n"));
  client->print(F("<a href=\"/FED\" id=\"btn-mandle\">FEED<br>MANDLES</button></a>\r\n"));
  client->print(F("</div>\r\n"));
  client->print(F("<div class=\"f s\">\r\n"));
  client->print(F("<form> Feed every this hours: <input type=\"number\" name=\"EVH\" maxlength=\"2\" min=\"1\" max=\"36\"><input type=\"submit\" value=\"CONFIRM\"></form>\r\n"));
  client->print(F("</div>\r\n"));
  client->print(F("<div class=\"f s\">\r\n"));
  client->print(F("<form> Feed every this minutes: <input type=\"number\" name=\"EVM\" maxlength=\"2\" min=\"1\" max=\"99999999\"><input type=\"submit\" value=\"CONFIRM\"></form>\r\n"));
  client->print(F("</div>\r\n"));
  client->print(F("<div class=\"f\">\r\n"));
  client->print(F("<form>\r\n"));
  client->print(F("<input type=\"submit\" name=\"s\" value=\"CONFIRM\"/>\r\n"));
  client->print(F("<p>Feed these hours: </p>\r\n"));
  client->print(F("<input type=\"checkbox\" name=\"h00\"><label>00:00:00</label><br/>\r\n"));
  client->print(F("<input type=\"checkbox\" name=\"h01\"><label>01:00:00</label><br/>\r\n"));
  client->print(F("<input type=\"checkbox\" name=\"h02\"><label>02:00:00</label><br/>\r\n"));
  client->print(F("<input type=\"checkbox\" name=\"h03\"><label>03:00:00</label><br/>\r\n"));
  client->print(F("<input type=\"checkbox\" name=\"h04\"><label>04:00:00</label><br/>\r\n"));
  client->print(F("<input type=\"checkbox\" name=\"h05\"><label>05:00:00</label><br/>\r\n"));
  client->print(F("<input type=\"checkbox\" name=\"h06\"><label>06:00:00</label><br/>\r\n"));
  client->print(F("<input type=\"checkbox\" name=\"h07\"><label>07:00:00</label><br/>\r\n"));
  client->print(F("<input type=\"checkbox\" name=\"h08\"><label>08:00:00</label><br/>\r\n"));
  client->print(F("<input type=\"checkbox\" name=\"h09\"><label>09:00:00</label><br/>\r\n"));
  client->print(F("<input type=\"checkbox\" name=\"h10\"><label>09:00:00</label><br/>\r\n"));
  client->print(F("<input type=\"checkbox\" name=\"h11\"><label>11:00:00</label><br/>\r\n"));
  client->print(F("<input type=\"checkbox\" name=\"h12\"><label>12:00:00</label><br/>\r\n"));
  client->print(F("<input type=\"checkbox\" name=\"h13\"><label>13:00:00</label><br/>\r\n"));
  client->print(F("<input type=\"checkbox\" name=\"h14\"><label>14:00:00</label><br/>\r\n"));
  client->print(F("<input type=\"checkbox\" name=\"h15\"><label>15:00:00</label><br/>\r\n"));
  client->print(F("<input type=\"checkbox\" name=\"h16\"><label>16:00:00</label><br/>\r\n"));
  client->print(F("<input type=\"checkbox\" name=\"h17\"><label>17:00:00</label><br/>\r\n"));
  client->print(F("<input type=\"checkbox\" name=\"h18\"><label>18:00:00</label><br/>\r\n"));
  client->print(F("<input type=\"checkbox\" name=\"h19\"><label>19:00:00</label><br/>\r\n"));
  client->print(F("<input type=\"checkbox\" name=\"h20\"><label>20:00:00</label><br/>\r\n"));
  client->print(F("<input type=\"checkbox\" name=\"h21\"><label>21:00:00</label><br/>\r\n"));
  client->print(F("<input type=\"checkbox\" name=\"h22\"><label>22:00:00</label><br/>\r\n"));
  client->print(F("<input type=\"checkbox\" name=\"h23\"><label>23:00:00</label><br/>\r\n"));
  client->print(F("</form>\r\n"));
  client->print(F("</div>\r\n"));
  client->print(F("<div id=\"a\"></div>\r\n"));
  client->print(F("</body>\r\n"));
  client->print(F("</html>\r\n"));
  
}








///////////////////////////////////////////////////////////
// Gets hours and seconds from the web. Returns successfullness.
///////////////////////////////////////////////////////////

bool getTimeFromWeb(){

  WiFiEspClient client;
  Serial.println("\nStarting connection to server...");
  // if you get a connection, report back via serial:
  if (client.connect("worldclockapi.com", 80)) {
    Serial.println("connected to server");
    // Make a HTTP request:
    client.println("GET /api/jsonp/cet/now?callback=mycallback HTTP/1.1");
    client.println("Host: worldclockapi.com");
    client.println("Connection: close");
    client.println();
  }

  // Safeguard, so that we can check if we even got values.
  hourOfDay = -1;
  minuteOfDay = -1;

  // Holder character for transferred data.
  char c;
  // Custom ringbuffer.
  char buf[6];

  // if there are incoming bytes available from the server, read them:
  bool trying = true;

  // Flag for correctness, set to false by a number of possible errors.
  bool correct = true;
  // The millisecond count of when we connected to the server.
  long int timeRequested = millis();

  // Attempts to find client availability.
  while(trying){
    // While client is available, read from it.
    while (client.available()) {
      trying = false;
      c = client.read();
      Serial.write(c);
      buf[0] = buf[1];
      buf[1] = buf[2];
      buf[2] = buf[3];
      buf[3] = buf[4];
      buf[4] = buf[5];
      buf[5] = c;

      // Now that we found the '+' in the "hh:mm+" format,
      // we can copy the previous values into our variables.
      if (buf[5] == '+'){
        hourOfDay = ((buf[0] - 48) * 10)  + (buf[1] - 48);
        minuteOfDay = ((buf[3] - 48) * 10)  + (buf[4] - 48);
      }
    } // End while client is available
    
    // Try for 30 seconds, then it will go into main loop,
    // having not gotten the desired information from the web
    // by returned value, and will go into getTimeFromWeb() again,
    // because the values of hourOfDay and minuteOfDay will be -1,
    // and will set correct to false.
    if(millis() - timeRequested > MINUTE/2){
      trying = false;
    }
  } // End while trying
  
  Serial.println();
  Serial.print(hourOfDay);
  Serial.print(":");
  Serial.println(minuteOfDay);

  // if the server's disconnected, stop the client:
  if (!client.connected()) {
    Serial.println();
    Serial.println("disconnecting from server.");
    client.stop();
  } // End if client is not connected.
  
  bool gotTime = false;
  // If the values are < 0 for either variable,
  // then we failed to get what the API was trying to tell us.
  if(hourOfDay < 0 || minuteOfDay < 0) {
    correct = false;
  }
  
  // Values should not exceed 23 and 59, respectivoli.
  if(hourOfDay > 23 || minuteOfDay > 59){
    correct = false;
  }
  if(correct){
    return true;
  }
  else{
    return false;
  }
}








// The following block will check if we need to feed,
// but only if we have gotten time from the web. (timeGotten)
// We need to feed if the arduino time since last time-gettening
// aligns with an hour that which is supposed to 

bool isSpecialTime () {
  // Offset since gotten time from web, in milliseconds.
  long int SINCE = millis() - lastTimeGottenTime;
  // What time in milliseconds it is now

  //          THEN       SINCE      NOW
  // Example, 43200000 + 3600000 -> 46800000 
  //          12:00    + 1:00    -> 13:00
  NOW = THEN + SINCE;
  
  // nowHour represents the hour of the day.
  // Dividing the millisecond representation of NOW by how many
  // milliseconds is in an HOUR.
  // 13:00 in milliseconds / 3600000 -> 13
  // then using modulo (%) 24 to get numbers from 0 to 23.
  // 0 being midnight, 23 being the hour before midnight.
  
  int nowHour = (NOW / HOUR) % 24;

  double nHour= float(THEN + SINCE) / HOUR;
  double c = 24;
  nHour = fmod(float(NOW/HOUR), c);
  Serial.print(NOW);
  Serial.print(", ");
  Serial.print(nowHour);
  Serial.print(", ");
  Serial.print(nHour);
  Serial.print(", ");
  Serial.println(lastFeed);

  // Return true only if we're supposed to feed on this hour,
  // as well as that we haven't before (indicated by nextPossibleSpecialTime)
  if (feedingTheseHours[nowHour] == true && (nowHour >= ((lastFeed+1)%24))) {

    // Resets to 0 when NOW == 23 because (23+1) % 24 is 0.
    // This will ensure that the next possible special time is on a new day.
    // We interpret time as a clock that goes upward at a rate of one second per second
    // and resets on a new day.
    lastFeed = nowHour;
    return true;
  }
  
  return false;
}








void loop() {

  ///////////////////////////////////////////////////////////
  // Feeding at special times.
  ///////////////////////////////////////////////////////////

  // If set to feed at special times, do that logic.
  if (setToFeedSpecialTimes) {
    // Every 12 hours, try to get the time.
    // Even if arduino could keep time forever after given a base time,
    // it seems like a good idea to get the authentic real time. 🤔
    if (millis() - lastTimeGottenTime > HOUR*12) {
      timeGotten = false;
    }
    
    // timeGotten is initialized to false by declaration,
    // so we attempt as soon as the setting is set by a client,
    // and not an hour later (as above) which could cause several missed feeding.

    // If time is not gotten (from the web) then we will to get it.
    if(!timeGotten) {
        // Maybe we get the time.
        timeGotten = getTimeFromWeb();

        if(timeGotten){
          // WE GOT THE TIME!
          // Record when, in arduino time, that we got the time, so that we can
          // calculate new times based on the difference between milliseconds at this time
          // with milliseconds when we want a time.
          lastTimeGottenTime = millis();
          // THEN is the time of day that it is when we got the real time, in hours + minutes as milliseconds.
          THEN = hourOfDay*HOUR + minuteOfDay*MINUTE;
        } // End if we got the time.

        
    } // End if time is not gotten.
    
    // If time is gotten, it is okay to check if now is special.
    if(timeGotten){
      // If time is special, feed.
      if (isSpecialTime()){
        feed();
      } // End if time is special.
    } // End if time gotten.
  } // End if set to feed at special times.







  


  ///////////////////////////////////////////////////////////
  // Feeding at certain intervals.
  ///////////////////////////////////////////////////////////

  // If set to feed by a certain interval of hours.
  if(setToFeedPerHour){
    // Check if it is time yet
    if (millis() - lastTimeFedByInterval >= everyThisHours*HOUR) {
      // Record last time fed by interval method.
      lastTimeFedByInterval = millis();
      feed();
    }
  }
  // else check if set to feed by a certain interval of minutes.
  else if (setToFeedPerMinute) {
    // Check if it is time yet
    if (millis() - lastTimeFedByInterval >= everyThisMinutes*MINUTE) {
      // Record last time fed by interval method.
      lastTimeFedByInterval = millis();
      feed();
    }
  } // End if check setting







  
  ///////////////////////////////////////////////////////////
  // Some variables.
  ///////////////////////////////////////////////////////////

  // Indicates if the user has issued an instant feed by the site.
  bool instantFeed = false;
  
  // Listen for incoming clients
  WiFiEspClient client = server.available();



  
  ///////////////////////////////////////////////////////////
  // Web serving.
  ///////////////////////////////////////////////////////////

  // If we have a client, we enter the realm of web serving.
  if (client) {
    Serial.println("New client");
    
    // An HTTP request ends with a blank line
    boolean currentLineIsBlank = true;

    // If the stuff between GET and HTTP has been handled, we set this to true
    // as to not compromise 
    bool userRequestDone = false;
    
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        Serial.write(c);
        buf.push(c);

        
        
        ///////////////////////////////////////////////////////////
        // Check user requests. 
        ///////////////////////////////////////////////////////////
        if(!userRequestDone){

          
          // Unimportant business beyond this point.
          if (buf.endsWith("HTTP")) {
            userRequestDone = true;
          }
          
          ///////////////////////////////////////////////////////////
          // instantFeed.
          ///////////////////////////////////////////////////////////
          
          if(buf.endsWith("FED")) {
            instantFeed = true;
          } // End if buf ends with FED

          ///////////////////////////////////////////////////////////
          // Client setting special hours.
          ///////////////////////////////////////////////////////////
          
          if(buf.endsWith("CONFIRM&")) {
            resetFeedingHours();
            
            bool foundSpace = false;
            byte hour;
            
            do {
              c = client.read();
              buf.push(c);
              // Read the two digits after 'h' like 'h12' -> 12, to mean feed at 12:00 o clock.
              if(buf.endsWith("h")){
                c = client.read();
                hour = (c - 48) * 10;
                buf.push(c);
                c = client.read();
                hour += c - 48;
                buf.push(c);
                if(hour >= 0 && hour < 24){
                  feedingTheseHours[hour] = true;
                  Serial.println();
                  Serial.print("Feeding @");
                  Serial.println(hour);
                }
              }

              // Exit if we find a space character in the HTTP request, signaling
              // that there are no more client settings left.
              if(c == ' '){
                foundSpace = true;
              }
            } while (!foundSpace);
            
            setToFeedPerHour = false;
            setToFeedPerMinute = false;
            setToFeedSpecialTimes = true;
            instantFeed = false; // If user previously used instant feed and it shows up when unintended.
            userRequestDone = true;
          }

          ///////////////////////////////////////////////////////////
          // Set by hour.
          ///////////////////////////////////////////////////////////
         
          if (buf.endsWith("EVH=")){
          
            setToFeedPerHour = true;
            setToFeedPerMinute = false;
            setToFeedSpecialTimes = false;
            instantFeed = false; // If user previously used instant feed and it shows up when unintended.
            userRequestDone = true;
  
            c = client.read();
            Serial.write(c);
           
            everyThisHours = c - 48;
           
            c = client.read();
            Serial.write(c);
           
            if (isDigit(c)){
              everyThisHours *= 10;
              everyThisHours += c - 48;
            }
          } // end if buf ends with EVH=
         
          ///////////////////////////////////////////////////////////
          // Set by minute.
          ///////////////////////////////////////////////////////////
         
          if(buf.endsWith("EVM=")){
           
            setToFeedPerMinute = true;
            setToFeedPerHour = false;
            setToFeedSpecialTimes = false;
            instantFeed = false; // If user previously used instant feed and it shows up when unintended.
            userRequestDone = true;
            
            int i = 0;
            do {
              c = client.read();
              Serial.write(c);
              if (!isDigit(c)){
                break;
              }
              i++;
              everyThisMinutes *= 10;
              everyThisMinutes += c - 48;
            } while (i < 8);
            
            // We set this to the current arduino time so that we don't feed instantly.
            // Kind of like how you don't count the position you're in when you
            // move 6 moves in monopoly.
            lastTimeFedByInterval = millis();
          } // End if buf ends with EVM=
            
        } // End if user request not done.

        
        
        ///////////////////////////////////////////////////////////
        // Send website if user's request is done.
        ///////////////////////////////////////////////////////////
        
        // If you've gotten to the end of the line (received a newline
        // character) and the line is blank, the http request has ended,
        // so you can send a reply
        if (c == '\n' && currentLineIsBlank) {
            
          Serial.println("Sending response");
          website(&client);
          client.stop();
          break;
        }
        
        if (c == '\n') {
          currentLineIsBlank = true;
        }
        else if (c != '\r') {
          currentLineIsBlank = false;
        } // End checking if current line is blank.
      
      } // End if client is available. 
    } // End if client is connected.

    
    // Give the web browser time to receive the data
    delay(10);
    
  } // End if client is not stopped.


  ///////////////////////////////////////////////////////////
  // Feeds after the HTTP stuff is completed as to not cause delay and timeouts,
  // as the feeding mechanism utilizes delays in order to keep time in 
  // the 
  ///////////////////////////////////////////////////////////

  // If instantFeed, feed.
  // instantFeed will be set to false next loop().
  if( instantFeed ){
    feed();
  } // End if instantFeed

  
} // End loop



//  Prints the status of 
//    SSID        - of the wifi         (external)
//    IP Address  - of the wifi shield  (internal)

void printWifiStatus() {
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);
}
