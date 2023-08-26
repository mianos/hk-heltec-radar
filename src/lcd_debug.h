#pragma once

class LcdDebugStream : public Stream {
private:
  ScrollingText& _scroller;
  static const size_t _bufferSize = 128;
  char _buffer[_bufferSize];
  size_t _bufferIndex = 0;

  void flushBuffer() {
    if (_bufferIndex > 0) {
      _buffer[_bufferIndex] = '\0';
      _scroller.taf("%s", _buffer);
      _bufferIndex = 0;
    }
    _scroller.force();
  }
public:
  LcdDebugStream(ScrollingText& scroller) : _scroller(scroller) {}

  size_t write(uint8_t data) {
    if (data == '\n') {
      flushBuffer();
    } else {
      _buffer[_bufferIndex++] = data;
      if (_bufferIndex >= _bufferSize - 1) {
        flushBuffer();
      }
    }
    return 1;
  }

  size_t write(const uint8_t *buffer, size_t size) {
    for (size_t i = 0; i < size; i++) {
      write(buffer[i]);
    }
    return size;
  }

  void flush() { flushBuffer(); }
  int available() { return 0; }
  int read() { return -1; }
  int peek() { return -1; }
};

