#ifndef CMD_H__
#define CMD_H__

#define CMD_INPUT_BUFFER_SIZE 32

typedef struct {
    char buffer[CMD_INPUT_BUFFER_SIZE];
    uint8_t pos;
} cmd_input_t;

void cmd_init(void);

void pending_cmd(void);

#endif
