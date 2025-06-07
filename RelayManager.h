

#pragma once

// GPIO numbers for relay switches
#define RELAY_REVERSE_RX_TX               12
#define RELAY_USE_RTS_CTS_OR_DTR_DSR      16
#define RELAY_REVERSE_RTS_CTS_OR_DTR_DSR  17

class RelayManager;
extern RelayManager relayManager;

class RelayManager {

public:

  void start() {
    setupGPIOs();
  }

  void setupGPIOs() {
    pinMode(RELAY_REVERSE_RX_TX, OUTPUT);
    pinMode(RELAY_USE_RTS_CTS_OR_DTR_DSR, OUTPUT);
    pinMode(RELAY_REVERSE_RTS_CTS_OR_DTR_DSR, OUTPUT);
  }

  // NOTE: This brute-force sets the relays by going HI->LO on the pins and then setting the wanted state.
  //       The capacitors might be empty, and going HI makes sure to charge them. We need to have them
  //       charged to make sure that the relays can switch to the 'off' position. Subsequently going LO
  //       on the GPIO will make all relays reliably switch to the 'off' position.
  //       Now all relays are reliably in the 'off' state and we can reliably switch the relays that we
  //       want to switch to the 'on' state.
  //       This takes a little more power than necessary, and makes the relays 'click' more than necessary,
  //       every time that we change their state. Actually, we really only have to do the HI->LO after
  //       power-on, as that is the only time that we don't know what the actual state of the relays
  //       is. We could reset them once after power-on, keep track of their state, and only switch the
  //       relays as are necessary to switch. But well, that would be more prone to errors. And the
  //       clicking sounds interesting. ;)

  void setRelays(bool nullModem, bool dtrDsr) {

    resetRelays();    
    // Wait a little time for the relays to have actually switched.
    vTaskDelay(150 / portTICK_PERIOD_MS);

    if (nullModem) {
      digitalWrite(RELAY_REVERSE_RX_TX, HIGH);
      digitalWrite(RELAY_REVERSE_RTS_CTS_OR_DTR_DSR, HIGH);
    }
  
    if (dtrDsr == 1) {
      digitalWrite(RELAY_USE_RTS_CTS_OR_DTR_DSR, HIGH);
    }
  }

  void resetRelays() {

    // Reset all to off by going hi->lo on all. We need a little time after going
    // HIGH to wait for the relay to have switched, and to charge the capactor.
  
    digitalWrite(RELAY_REVERSE_RX_TX, HIGH);
    digitalWrite(RELAY_USE_RTS_CTS_OR_DTR_DSR, HIGH);
    digitalWrite(RELAY_REVERSE_RTS_CTS_OR_DTR_DSR, HIGH);
    
    vTaskDelay(150 / portTICK_PERIOD_MS);
    
    digitalWrite(RELAY_REVERSE_RX_TX, LOW);
    digitalWrite(RELAY_USE_RTS_CTS_OR_DTR_DSR, LOW);
    digitalWrite(RELAY_REVERSE_RTS_CTS_OR_DTR_DSR, LOW);
  }
};

RelayManager relayManager;