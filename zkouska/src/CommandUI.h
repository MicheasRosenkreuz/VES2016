#ifndef COMMANDUI_H_INCLUDED
#define COMMANDUI_H_INCLUDED

#define CONSOLEPROMPT "-> "

void UIProcessCmd();

typedef struct
{
    uint8_t state;
    uint8_t on;
    uint16_t period;
    uint32_t ticks;
} LED_Status;

extern LED_Status led_Status[10];
#endif /* COMMANDUI_H_INCLUDED */
