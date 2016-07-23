
#include "espSlackBot.h"

ArduinoSlackBot* ArduinoSlackBot::ptrBot = 0; 

ArduinoSlackBot::ArduinoSlackBot(){
  ptrBot = this;  //the ptr points to this object
  qrListLength = 0;
}

/********************************************************************
  setup the slackbot token and connect


********************************************************************/ 
void ArduinoSlackBot::begin(const char *token,const char *id){
  botID = id;
  slackToken = String(token);
  connected = connect();
  failMsg = "huh?"; //set a default fail incase user forgets to add in command list
}

/********************************************************************
  Update sends a keep alive ping and connects / reconnects to slack


********************************************************************/ 
void ArduinoSlackBot::update(){
 
  webSocket.loop();

  if (connected) {
    // Send ping every 5 seconds, to keep the connection alive
    if (millis() - lastPing > 5000) {
      sendPing();
      lastPing = millis();
    }
  } else {
    // Try to connect / reconnect to slack
    connected = connect();
    if (!connected) {
      delay(500);
    }
  }
}
/********************************************************************
  connect establishes a connection to the Slack RTM API


********************************************************************/
bool ArduinoSlackBot::connect() {
  // Step 1: Find WebSocket address via RTM API (https://api.slack.com/methods/rtm.start)
  HTTPClient http;
  String slackAddr = "https://slack.com/api/rtm.start?token="; 
  slackAddr = String(slackAddr + slackToken);
  PRINTLN(slackAddr);

  http.begin(slackAddr.c_str(), slackSSLFingerprint);
  int httpCode = http.GET();

  if (httpCode != HTTP_CODE_OK) {
    PRINTF("HTTP GET failed with code %d\n", httpCode);
    return false;
  }

  WiFiClient *client = http.getStreamPtr();
  client->find("wss:\\/\\/");
  String host = client->readStringUntil('\\');
  String path = client->readStringUntil('"');
  path.replace("\\/", "/");

  // Step 2: Open WebSocket connection and register event handler
  PRINTLN("WebSocket Host=" + host + " Path=" + path);
  webSocket.beginSSL(host, 443, path, "", "");
  webSocket.onEvent(webSocketEvent);
  return true;
}

/********************************************************************
  sendPing keeps the websocket connection alive by pinging slack
********************************************************************/
void ArduinoSlackBot::sendPing() {
  DynamicJsonBuffer jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
  root["type"] = "ping";
  root["id"] = nextCmdId++;
  String json;
  root.printTo(json);
  webSocket.sendTXT(json);
}

/********************************************************************
  replyMsg replies back to the same channel a message was recieved on


********************************************************************/
void ArduinoSlackBot::replyMsg(const char *msg) { 
  sendMsg(slackMsg.channel,msg); 
}

/********************************************************************
  sendMsg sends a message back to slack on a specific channel


********************************************************************/
void ArduinoSlackBot::sendMsg(const char *channel,const char *msg) {
  DynamicJsonBuffer jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
  root["type"] = "message";
  root["id"] = nextCmdId++;
  root["text"] = msg;
  root["channel"] = channel;
  String json;
  root.printTo(json);
  webSocket.sendTXT(json);
}
/********************************************************************
  parse the websocket json and look for a message type


********************************************************************/
void ArduinoSlackBot::parseResponse(char *payload) {
  DynamicJsonBuffer jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject(payload);

  if (root.success()) {
    
    if (root.containsKey("type")) {
      slackMsg.type = root["type"];

      PRINTLN(slackMsg.type);
      PRINT("free heap size:");
      PRINTLN(ESP.getFreeHeap());


      if (strcasecmp(slackMsg.type, "message") == 0) {
        slackMsg.channel = root["channel"];
        slackMsg.user = root["user"];
        slackMsg.text = root["text"];
        slackMsg.timestamp = root["ts"];
        slackMsg.team = root["team"];
        PRINTLN("parseCommands");
        parseCmds();
      }
    }
    

  } else {
    PRINTLN("parse fail");
  }
}
/********************************************************************
  parseCmds loops through a list of hear functions and tries to match queries 
  and execute reply response functions

********************************************************************/
void ArduinoSlackBot::parseCmds() {

  struct slre_cap matches[4];

  //you must reference the bot to get a response
  if (slre_match(botID, slackMsg.text, strlen(slackMsg.text), matches, 4, 0) > 0) {
    PRINTLN(ESP.getFreeHeap());

    for (int i=0; i <= qrListLength-1; i++) {
     
      //match the provided regex
      if (slre_match(qrList[i]->query, slackMsg.text, strlen(slackMsg.text), matches, 4, 0) > 0) {  
        PRINTLN(matches[0].ptr);
        qrList[i]->resp(); //execute provided bot response
        return; //stop matching if match is found
      } 
    }
    replyMsg(failMsg);
  }
  
}
/********************************************************************
  hears adds the query string and response function to a list 
  to be checked when a slack message is recieved. 

********************************************************************/
void ArduinoSlackBot::hears(const char *regex, std::function<void()> response) {

  queryResp* qr;
  qr = new queryResp; //allocate stuct
  
  //add regex and reponse function to struct
  qr->query = regex;
  qr->resp = response; 
  
  qrList[qrListLength] = qr; //add struct to list
  qrListLength++; //bump up the index

}
/********************************************************************
  fails adds a default message to be executed when the bot doesn't 
  understand a slack message

********************************************************************/
void ArduinoSlackBot::fails(const char *msg) {
  failMsg = msg;
}

/********************************************************************
  Called on each web socket event. Handles disconnection, and also
  incoming messages from slack.

********************************************************************/
  void webSocketEvent(WStype_t type, uint8_t *payload, size_t len) {
  switch (type) {
    case WStype_DISCONNECTED:
      PRINT("[WebSocket] Disconnected \n");
      ArduinoSlackBot::ptrBot->connected = false;
      break;

    case WStype_CONNECTED:
      PRINTF("[WebSocket] Connected to: %s\n", payload);
      ArduinoSlackBot::ptrBot->sendPing();
      break;

    case WStype_TEXT:
      PRINTF("[WebSocket] Message: %s\n", payload);
      ArduinoSlackBot::ptrBot->parseResponse((char*)payload);
      break;
  }
}
