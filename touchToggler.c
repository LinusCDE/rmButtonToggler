/*
 * Press LEFT+RIGHT to block and unblock the touchscreen.
 */

#include <errno.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/uinput.h>

// Renamed copy of input_event from linux-kernel of the reMarkable
// Source: https://github.com/reMarkable/linux/blob/b82cb2adc32411c98ffc0db86cdd12858c8b39df/include/uapi/linux/input.h#L24
struct rm_input_event {
        struct timeval time;
        uint16_t type;
        uint16_t code;
        int32_t value;
};

// Source: https://github.com/canselcik/libremarkable/blob/master/src/input/gpio.rs
enum Button {
    LEFT,
    MIDDLE,
    RIGHT,
    POWER,
    WAKEUP
};

ssize_t codeToButton(uint16_t evCode) {
  switch(evCode) {
      case 105:
          return LEFT;
      case 102:
          return MIDDLE;
      case 106:
          return RIGHT;
      case 116:
          return POWER;
      case 143:
          return WAKEUP;
      default:
          fprintf(stderr, "FATAL: Unknown evCode %d !!!");
          exit(1);
          return -1;
  }
}

int main(int argc, char const *argv[])
{

    // Open file where input events for the wacom digitizer get received
    int touchInputFd = open("/dev/input/event1", O_WRONLY | O_NONBLOCK); // Even though we won't write
    FILE* gpioInputFp = gpioInputFp = fopen("/dev/input/event2", "r");

    if(gpioInputFp == NULL) {
        fprintf(stderr, "Failed to open gpio event file!\n");
        exit(1);
    }

    struct rm_input_event inputEvent; // Will contain the read data
    size_t inputEventSize = sizeof(inputEvent);
    //size_t timeSize = sizeof(inputEvent.time);
    //size_t packetSize = inputEventSize - timeSize; // Size of data that will get sent in packets
    size_t readBytes;

    uint8_t buttonState[5];
    buttonState[LEFT] = 0;
    buttonState[MIDDLE] = 0;
    buttonState[RIGHT] = 0;
    buttonState[POWER] = 0;
    buttonState[WAKEUP] = 0;

    /*bool*/uint8_t touchGrabbed = 0;

    while(1) {
        readBytes = fread((void*) &inputEvent, 1, inputEventSize, gpioInputFp);

        if(readBytes != inputEventSize) {
            // Failed to read required data. Should never happen.
            perror("Failed to read gpio event file");

            if(fclose(gpioInputFp) != 0)
                perror("Faild to close gpio event file");

            return 1;
        }

        if(inputEvent.type != 1)
            continue;

        int button = codeToButton(inputEvent.code);
        uint8_t isPressed = inputEvent.value ? 1 : 0;
        buttonState[button] = isPressed;

        if(buttonState[LEFT] && buttonState[RIGHT] && (button == LEFT || button == RIGHT)) {
            if(!touchGrabbed) {
                printf("Touchpad grabbed. Not input possible!\n");
                ioctl(touchInputFd, EVIOCGRAB, 1); // Block touch input
            } else {
                printf("Touchpad released. Input possible again.\n");
                ioctl(touchInputFd, EVIOCGRAB, 0); // Unblock touch input
            }

            touchGrabbed = !touchGrabbed;
        }

        // Debug:
        //printf("\nLEFT: %d\nMIDDLE: %d\nRIGHT: %d\nPOWER: %d\nWAKEUP: %d\n",
        //       buttonState[LEFT], buttonState[MIDDLE], buttonState[RIGHT], buttonState[POWER], buttonState[WAKEUP]);
        //printf("Type: %d, Code: %d, Value: %d\n", inputEvent.type, inputEvent.code, inputEvent.value);


    }


    return 0;
}
