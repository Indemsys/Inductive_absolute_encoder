#ifndef OS_MISC_H
#define OS_MISC_H


void   Reset_system(void); 
void  _time_delay(uint32_t millisec);
void* _mem_alloc_zero(uint32_t size);
void  _mem_free(void *ptr);  

#endif // OS_MISC_H



