/*
 * Press LEFT+RIGHT to block and unblock all buttons.
 */

#include <errno.h>
#include <signal.h>
#include <stdint.h>
#include <stdbool.h>
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

typedef enum {
    LEFT,
    MIDDLE,
    RIGHT,
    POWER,
    WAKEUP
} Button;

// Codes as used by ev
typedef enum {
    CODE_LEFT = 105,
    CODE_MIDDLE = 102,
    CODE_RIGHT = 106,
    CODE_POWER = 116,
    CODE_WAKEUP = 143
} ButtonCode;

Button codeToButton(ButtonCode evCode) {
  // Source: https://github.com/canselcik/libremarkable/blob/master/src/input/gpio.rs
  switch(evCode) {
      case CODE_LEFT: return LEFT;
      case CODE_MIDDLE: return MIDDLE;
      case CODE_RIGHT: return RIGHT;
      case CODE_POWER: return POWER;
      case CODE_WAKEUP: return WAKEUP;
      default:
          fprintf(stderr, "FATAL: Unknown evCode %d in the context of a button!!!");
          exit(1);
          return -1; // Doesn't matter
  }
}

ButtonCode buttonToCode(Button button) {
  switch(button) {
      case LEFT: return CODE_LEFT;
      case MIDDLE: return CODE_MIDDLE;
      case RIGHT: return CODE_RIGHT;
      case POWER: return CODE_POWER;
      case WAKEUP: return CODE_WAKEUP;
      default:
          fprintf(stderr, "FATAL: Unknown Button enum value %d !!!");
          exit(1);
          return -1; // Doesn't matter
  }
}

int gpioInputFd;
bool buttonState[5];
bool blocking = false;


void setBlocking(bool newBlocking) {
    if(blocking == newBlocking)
        return;

    ioctl(gpioInputFd, EVIOCGRAB, newBlocking);
    if(newBlocking)
        printf("Buttons grabbed. Only filtered input allowed!\n");
    else
        printf("Touchpad released. All input allowed!\n");

    blocking = newBlocking;
}

int main(int argc, char const *argv[])
{
    // Init state
    buttonState[LEFT] = false;
    buttonState[MIDDLE] = false;
    buttonState[RIGHT] = false;
    buttonState[POWER] = false;
    buttonState[WAKEUP] = false;

    // Open file where input events for the hardware buttons (over gpio pins) get received
    gpioInputFd = open("/dev/input/event2", O_RDWR);

    if(gpioInputFd < 0) {
        perror("Failed to open gpio event file");
        exit(1);
    }

    struct rm_input_event inputEvent; // Will contain the read data
    size_t readBytes; // Read progress in case one event needs multiple reads.
    ssize_t result;

    while(1) {
        result = read(gpioInputFd, ((uint8_t*) &inputEvent) + readBytes, sizeof(inputEvent) - readBytes);

        if(result < 0) {
            // Failed to read required data. Should never happen.
            perror("Failed to read gpio event file");

            if(close(gpioInputFd) != 0)
                perror("Faild to close gpio event file");

            return 1;
        }

        readBytes += result;
        if(readBytes < sizeof(inputEvent))
            continue; // Event not fully read, yet.
        readBytes = 0; // Reset read progress.

        if(inputEvent.type != 1)
            continue;

        Button button = codeToButton(inputEvent.code);
        bool isPressed = inputEvent.value > 0;
        buttonState[button] = isPressed;
        //printf("Pressed: %d", isPressed);

        if(buttonState[LEFT] && buttonState[RIGHT] && (button == LEFT || button == RIGHT)) {
            setBlocking(!blocking);
        }

        // Debug:
        //printf("\nLEFT: %d\nMIDDLE: %d\nRIGHT: %d\nPOWER: %d\nWAKEUP: %d\n",
        //       buttonState[LEFT], buttonState[MIDDLE], buttonState[RIGHT], buttonState[POWER], buttonState[WAKEUP]);
        //printf("Type: %d, Code: %d, Value: %d\n", inputEvent.type, inputEvent.code, inputEvent.value);


    }


    return 0;
}
