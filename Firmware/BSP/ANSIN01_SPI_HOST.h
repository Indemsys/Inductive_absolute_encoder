#ifndef __ANSIN01_HOST_SPI_H
  #define __ANSIN01_HOST_SPI_H

#define HOSTSPI_PACKET_WAIT_ERROR  0 
#define HOSTSPI_PACKET_RECEIVED    1 
#define HOSTSPI_TX_DATA_READY       2 

void     Host_SPI_init(void);
void     Host_SPI_set_data_ready(void);
uint32_t Host_SPI_wait_data_ready(uint32_t timeout);
uint32_t Host_SPI_wait_packet(void *buf, uint32_t sz, uint32_t timeout);
uint32_t HOST_SPI_send_packet(void *buf, uint32_t sz, uint32_t timeout);

#endif // 



