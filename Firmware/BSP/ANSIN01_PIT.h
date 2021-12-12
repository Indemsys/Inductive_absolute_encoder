#ifndef ANSIN01_PIT_H
#define ANSIN01_PIT_H


void     Init_PIT_module(void);
void     Init_PIT0(uint32_t period);
uint32_t Get_PIT0_curr_val(void);
uint32_t Get_PIT0_load_val(void);

#endif // ANSIN01_PIT_H



