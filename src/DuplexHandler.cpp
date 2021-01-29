/**
 * @file DuplexHandler.cpp
 * @date 20.06.2020
 * @author Grandeur Technologies
 *
 * Copyright (c) 2019 Grandeur Technologies LLP. All rights reserved.
 * This file is part of the Arduino SDK for Grandeur.
 *
 */

#include "DuplexHandler.h"
#include "arduinoWebSockets/WebSockets.h"

/* Create client */
WebSocketsClient client;

// Init ping counter
unsigned long millisCounterForPing = 0;

// Init status handler and variable
short DuplexHandler::_status = DISCONNECTED;
void (*DuplexHandler::_connectionCallback)(bool) = [](bool connectionEventHandler) {};

// Define a queue
EventQueue DuplexHandler::_queue;

// Create a new event table and subscriptions
EventTable DuplexHandler::_eventsTable;
EventTable DuplexHandler::_subscriptions;

/* Deifne event handler */
void duplexEventHandler(WStype_t eventType, uint8_t* packet, size_t length);

DuplexHandler::DuplexHandler(Config config) {
  // Constructor
  _query = "/?type=device&apiKey=" + config.apiKey;
  _token = config.token;
}

DuplexHandler::DuplexHandler() {
  // Overload the contructor
  _query = "/?type=device";
  _token = "";
}

void DuplexHandler::init(void) {
  // Setting up event handler
  client.onEvent(&duplexEventHandler);

  // Scheduling reconnect every 5 seconds if it disconnects
  client.setReconnectInterval(5000);

  // Opening up the connection
  client.beginSSL(GRANDEUR_URL, GRANDEUR_PORT, _query, GRANDEUR_FINGERPRINT, "node");

  // Set auth header
  char tokenArray[_token.length() + 1];
  _token.toCharArray(tokenArray, _token.length() + 1);
  client.setAuthorization(tokenArray);
}

void DuplexHandler::onConnectionEvent(void connectionEventHandler(bool)) {
  // Event handler for connection
  _connectionCallback = connectionEventHandler;
}

short DuplexHandler::getStatus() {
  // Return status
  return _status;
}

void DuplexHandler::loop(bool valve) {
  // Give duplex time to execute
  if(valve) {
    // If valve is true => valve is open
    if(millis() - millisCounterForPing >= PING_INTERVAL) {
        // Ping Grandeur if PING_INTERVAL milliseconds have passed
        millisCounterForPing = millis();
        ping();
    }

    // Running duplex loop
    client.loop();
  }
}

// Define the handle function
void DuplexHandler::handle(EventID id, EventKey key, EventPayload payload, Callback callback) {
  // Check connection status
  if(_status != CONNECTED) {
    return ;
  }

  // Create packet
  char packet[PACKET_SIZE];

  // Saving callback to eventsTable
  _eventsTable.insert(key, id, callback);
  // _eventsTable.print();
  // _subscriptions.print();

  // Formatting the packet
  snprintf(packet, PACKET_SIZE, "{\"header\": {\"id\": %lu, \"task\": \"%s\"}, \"payload\": %s}", id, key.c_str(), payload.c_str());
  
  // Sending to server
  client.sendTXT(packet);
}

void DuplexHandler::send(const char* task, const char* payload, Callback callback) {
  // Generate packet id
  gID packetID = micros();

  // Check connection status
  if(_status != CONNECTED) {
    // Append the packet to queue
    _queue.push(packetID, task, payload, callback);
    // _queue.print();
    return ;
  }

  // Create packet
  char packet[PACKET_SIZE];

  // Saving callback to eventsTable
  _eventsTable.insert(task, packetID, callback);
  // _eventsTable.print();
  // _subscriptions.print();

  // Formatting the packet
  snprintf(packet, PACKET_SIZE, "{\"header\": {\"id\": %lu, \"task\": \"%s\"}, \"payload\": %s}", packetID, task, payload);
  
  // Sending to server
  client.sendTXT(packet);
}

void DuplexHandler::unsubscribe(gID id, const char* payload) {
  // Generate an id
  gID packetID = micros();

  // Remove callback from subscriptions table
  _subscriptions.remove(id);
  // _eventsTable.print();
  // _subscriptions.print();

  // Push unsub to queue
  _queue.push(packetID, "/topic/unsubscribe", payload, NULL);

  // and remove subscription packet from queue
  _queue.remove(id);
  // _queue.print();
  
  // Check connection status
  if(_status != CONNECTED) {
    // Don't proceed if we aren't
    return ;
  }

  // Create packet
  char packet[PACKET_SIZE];

  // Saving callback to eventsTable
  _eventsTable.insert("/topic/unsubscribe", packetID, NULL);
  // _eventsTable.print();
  // _subscriptions.print();

  // Formatting the packet
  snprintf(packet, PACKET_SIZE, "{\"header\": {\"id\": %lu, \"task\": \"%s\"}, \"payload\": %s}", packetID, "/topic/unsubscribe", payload);
  
  // Sending to server
  client.sendTXT(packet);
}

gID DuplexHandler::subscribe(const char* event, const char* payload, Callback updateHandler) {
  // Generate an id
  gID packetID = micros();

  // Saving callback to subscriptions Table
  _subscriptions.insert(event, packetID, updateHandler);
  // _eventsTable.print();
  // _subscriptions.print();

  // Append the packet to queue because in case of queue
  // the packet will always be queue either we are connected
  // or disconnected
  // This is being done to handle case where we were connected
  // the subscribed to some events and got disconnected
  _queue.push(packetID, "/topic/subscribe", payload, NULL);
  // _queue.print();
  
  // Check connection status
  if(_status != CONNECTED) {
    // Don't proceed if we aren't
    // but return packet id
    return packetID;
  }

  // Create packet
  char packet[PACKET_SIZE];

  // Saving callback to eventsTable
  _eventsTable.insert("/topic/subscribe", packetID, NULL);
  // _eventsTable.print();
  // _subscriptions.print();

  // Formatting the packet
  snprintf(packet, PACKET_SIZE, "{\"header\": {\"id\": %lu, \"task\": \"%s\"}, \"payload\": %s}", packetID, "/topic/subscribe", payload);
  
  // Sending to server
  client.sendTXT(packet);

  // Return packet id
  return packetID;
}

void DuplexHandler::ping() {
  // Ping handler
  if(_status == CONNECTED) {
    // Generate packet
    char packet[PING_PACKET_SIZE];

    // Create id
    gID packetID = millis();

    // Saving callback to eventsTable
    _eventsTable.insert("ping", packetID, NULL);

    // Formatting the packet
    snprintf(packet, PING_PACKET_SIZE, "{\"header\": {\"id\": %lu, \"task\": \"ping\"}}", packetID);

    // Sending to server
    client.sendTXT(packet);
  }
}

/** This function handles all the cloud events.
 * @param eventType: The type of event that has occurred.
 * @param packet: The packet corresponding to the event.
 * @param length: The size of the @param packet.
*/
void duplexEventHandler(WStype_t eventType, uint8_t* packet, size_t length) {

  // Switch over event type
  switch(eventType) {
    case WStype_CONNECTED:
      // When duplex connection opens
      DuplexHandler::_status = CONNECTED;

      // Generate callback
      DuplexHandler::_connectionCallback(DuplexHandler::_status);

      // Resetting ping millis counter
      millisCounterForPing = millis();

      // Handle the queued events
      DuplexHandler::_queue.forEach(DuplexHandler::handle);

      // Then send 
      return ;

    case WStype_DISCONNECTED:
      // When duplex connection closes
      DuplexHandler::_status = DISCONNECTED;

      // Clear event table
      DuplexHandler::_eventsTable.clear();

      // Generate connection event
      return DuplexHandler::_connectionCallback(DuplexHandler::_status);

    case WStype_TEXT:
      // Serial.printf("%s\n", packet);

      // When a duplex message is received.
      Var messageObject = JSON.parse((char*) packet);

      // Handle parsing errors
      if (JSON.typeof(messageObject) == "undefined") {
        // Just for internal errors of Arduino_JSON
        // if the parsing fails.
        DEBUG_GRANDEUR("Parsing input failed!");
        return;
      }

      // If it is an update instead of response to a task
      if(messageObject["header"]["task"] == "update") {
        // Formulate the key
        std::string event((const char*) messageObject["payload"]["event"]);
        std::string path((const char*) messageObject["payload"]["path"]);

        // Handle the backward compatibility case
        if (event == "deviceParms" || event == "deviceSummary") event = "data";

        // Emit the event 
        DuplexHandler::_subscriptions.emit(event + "/" + path, messageObject["payload"]["update"], messageObject["payload"]["path"]);
        
        // Return
        return;
      }
      // Just return if it is unpair event
      else if(messageObject["header"]["task"] == "unpair") {
        return ;
      }

      // Fetching event callback function from the events Table
      Callback callback = DuplexHandler::_eventsTable.findAndRemove((gID) messageObject["header"]["id"]);
      
      // DuplexHandler::_eventsTable.print();
      // DuplexHandler::_subscriptions.print();

      // Remove the packet if it was queued
      // because the ack has been received
      if (messageObject["header"]["task"] == "/topic/subscribe");
      else {
        // but if it is not of subscribe type
        DuplexHandler::_queue.remove((long) messageObject["header"]["id"]);
        // DuplexHandler::_queue.print();
      }

      // If not found then simply return
      if(!callback) return;

      // Or otherwise resolve the event
      return callback(messageObject["payload"]);
  }
}

