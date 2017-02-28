#ifndef ARDUINO_SLACKBOT_H
#define ARDUINO_SLACKBOT_H

#include <Arduino.h>
#include <Hash.h>
#include <WebSocketsClient.h>
#include <ArduinoJson.h>
#include <ESP8266HTTPClient.h>
#include "slre.h"
#include <functional>
#include <mem.h>

//#define DEBUG

#ifdef DEBUG
 #define PRINT(x) Serial.print(x)
 #define PRINTLN(x) Serial.println(x)
#define PRINTF(x) Serial.printf(x)
 #define PRINTF(x,y) Serial.printf(x,y)
#else
 #define PRINT(x)
 #define PRINTLN(x)
 #define PRINTF(x) Serial.printf(x)
 #define PRINTF(x,y) 
#endif

extern "C" void webSocketEvent(WStype_t type, uint8_t *payload, size_t len) __attribute__ ((signal));

class ArduinoSlackBot {
 
public: 

  friend void webSocketEvent(WStype_t type, uint8_t *payload, size_t len); 

  ArduinoSlackBot();

  void begin(const char *token,const char *id); //setup bot with slack token and bot id
  bool connect(); //connect to slack (handled by begin)
  void update(); //update bot ping and reconnect if needed
  void replyMsg(const char *msg); //reply to channel
  void sendMsg(const char *channel,const char *msg); //send msg to a channel
  void hears(const char *regex, std::function<void()> callback); //setup bot command and response
  void fails(const char *failMsg); //setup default message if bot doesn't understand

  
private: 

  void sendPing(); //ping to keep connection alive
  void parseResponse(char *payload); //parse json to find message
  void parseCmds(); //match messsage with commands and responses

  String slackToken;
  const char * slackSSLFingerprint = "AC 95 5A 58 B8 4E 0B CD B3 97 D2 88 68 F5 CA C1 0A 81 E3 6E"; //You can use https://www.grc.com/fingerprints.htm to get fingerprint for "slack.com"
  const char* WORD_SEPERATORS = "., \"'()[]<>;:-+&?!\n\t";
  bool connected = false;
  unsigned long lastPing = 0;
  long nextCmdId = 1;

   //struct of bot queries and response functions
  struct queryResp {
   const char *query; 
   std::function<void()> resp;
  };
 
  int qrListLength; 
  queryResp* qrList[50]; //list of queries and responses, 50 max 

  static ArduinoSlackBot* ptrBot; //static ptr to EspSlackBot class for the webSocketEvent handler
  WebSocketsClient webSocket; //websocket

  const char *failMsg; //fail message
  const char* botID; //your unqiue bot id, neccessary for only answering when mentioned
  

  //slack message struct
  struct message { 
    const char* type;
    const char* channel;
    const char* user;
    const char* text;
    const char* timestamp;
    const char* team;
  } slackMsg;

}; 
 
#endif
