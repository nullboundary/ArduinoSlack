# ArduinoSlack

Run a slack bot on your esp8266 wifi hardware. Useful for triggering digital I/O or reading sensors from your slack channel. 

***
# Functions

#### begin

```C++
void begin(const char *token,const char *id);
```
Begin setups the bot with slack token and bot id

#### connect

```C++
bool connect();
```
Connect connects to slack (handled by begin)

#### update

```C++
void update();
```
update bot ping and reconnect if needed

#### replyMsg

```C++
void replyMsg(const char *msg);
```
reply to channel

#### sendMsg

```C++
void sendMsg(const char *channel,const char *msg);
```
send msg to a channel

#### hears

```C++
void hears(const char *regex, std::function<void()> callback);
```
parse bot command and response

#### fails

```C++
void fails(const char *failMsg);
```
setup default message if bot doesn't understand

***
# Example:
```C++
  /****************************************************/
void botCommands() {

  //syntax for bot.hears(regex, reponse function). Try it with a C++11 lambda 
  bot.hears("hello|hi|yo|howdy",[](){ 
    bot.replyMsg("hey hey!"); 
  });

  //try a more complex regex and a longer response function
  bot.hears("(get|read).*(a0|sensor|air.*quality)",readAirSensor);
  
  //if the bot doesn't understand (match) reply with a default message
  bot.fails("Sorry I didn't get that.");

}

/****************************************************/
void readAirSensor() {
    int sensorValue = analogRead(A0); //read analog input

    String valueMsg = String("Ok, the current the value is: "); //format the message
    valueMsg = valueMsg + String(sensorValue);

    bot.replyMsg(valueMsg.c_str()); //reply only takes c style strings
}
```
***
