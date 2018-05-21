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

#endif /* SYSCTL_H */
