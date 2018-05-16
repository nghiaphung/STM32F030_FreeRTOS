/*
 * @filename: timer.h
 * @date: 14/5/2018
 */
#ifndef TIMER_H
#define TIMER_H

void SysTimer_Init(void);
int CreateSWTimer(uint32_t interval, void (*Callback)(void*), void* param);
int RunSWTimer(int id);
void HaltSWTimer(int id);
int DeleteSWTimer(int id);

#endif /* TIMER_H */
