

class RingBuffer {

private:
  byte *buffer;
  
  int bufferSize;

  int lowMark;
  int highMark;

  int size;
  int head;
  int tail;

public:

  RingBuffer(int size, int lowWaterMark, int highWaterMark) {

    bufferSize = size;

    size = 0;
    head = 0;
    tail = 0;

    lowMark = lowWaterMark;
    highMark = highWaterMark;

    buffer = new byte[size];
  }

  int dataSize() {
    return size;
  }

  bool put(byte data, std::function<void(void)> onHighWater) {
    if (size == bufferSize) {
      // Buffer is full.
      return false;
    }

    buffer[tail] = data;
    
    tail++;
    size++;
    
    if (tail >= bufferSize) {
      tail = 0;
    }

    if (size == highMark) {
      onHighWater();
    }

    return true;
  }

  bool get(byte *data, std::function<void(void)> onLowWater) {
    if (size == 0 || data == NULL) {
      return false;
    }
    
    *data = buffer[head];

    head++;
    size--;

    if (head >= bufferSize) {
      head = 0;
    }

    if (size == lowMark) {
      onLowWater();
    }

    return true;
  }
};