#pragma once
#include "freertos/queue.h"

class StringQueue {
private:
  QueueHandle_t stringQueue;

public:
  StringQueue(size_t size = 10) {
    stringQueue = xQueueCreate(size, sizeof(String *));
  }

  ~StringQueue() {
    String *str_ptr;
    while(xQueueReceive(stringQueue, &str_ptr, 0)) {
      delete str_ptr;
    }
    vQueueDelete(stringQueue);
  }

  void push(const String& str) {
    String *str_ptr = new String(str);
    xQueueSend(stringQueue, &str_ptr, portMAX_DELAY);
  }

  String shift() {
    String *str_ptr;
    if(xQueueReceive(stringQueue, &str_ptr, portMAX_DELAY)) {
      String str = *str_ptr;
      delete str_ptr;
      return str;
    }
    return String();
  }

  bool isEmpty() const {
    return uxQueueMessagesWaiting(stringQueue) == 0;
  }

  size_t size() const {
    return uxQueueMessagesWaiting(stringQueue);
  }
};

