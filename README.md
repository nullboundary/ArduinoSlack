# ArduinoSlack

Run a slack bot on your esp8266 wifi hardware. Useful for triggering digital I/O or reading sensors from your slack channel. 

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
