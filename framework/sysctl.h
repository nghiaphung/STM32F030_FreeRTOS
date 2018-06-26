/*
 * @filename: sysctl.h
 * @date: 16/5/2018
 */
#ifndef SYSCTL_H
#define SYSCTL_H

/**
 * @brief system events
 */
typedef struct {
    uint8_t event;
    uint8_t param[4];
}sys_events_t;

void vSERIAL_CmdService(void *pvParam);
void vIO_QueryService(void *pvParam);

#endif /* SYSCTL_H */
