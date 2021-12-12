#ifndef HOST_CHANNEL_H
#define HOST_CHANNEL_H



enum T_parser_state
{
  ps_IDLE, ps_CMD, ps_LEN, ps_DATA, ps_CRC
};

enum T_commands
{
  cmd_READ, cmd_Write, cmd_Operation
};

#define HOST_CHAN_MAX_PACK_LEN  (64)  
                                                      
                                                       

typedef struct 
{
  uint8_t         packet[HOST_CHAN_MAX_PACK_LEN];
  uint32_t        pack_len;   
  uint8_t         crc;
  
  uint8_t         var_indx;
  uint8_t         *pvar;
  uint8_t         var_sz; 

  uint32_t        host_rx_pack_cnt; 
  uint32_t        host_tx_pack_cnt; 
  uint32_t        host_errtx_pack_cnt; 

} T_host_channel_cbl;


void Task_Host_channel(void *handle);
void Host_ch_set_ready_sig(void);

#endif // HOST_CHANNEL_H



