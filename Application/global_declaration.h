void                                Superviser_Task(void);
void                                StatisticManager(void);
void                                ControlManager(void);

void                                hexbuffer_2_binbuffer(pInt8U hexBuffer,pInt8U binBuffer,Int16U nBytes,Int16U Pointer);
void                                binbuffer_2_hexbuffer(pInt8U hexBuffer,pInt8U binBuffer,Int16U nBytes,Int16U Pointer);
void                                binbuffer_2_binbuffer(pInt8U pBufferSource,pInt8U pBufferDestination,Int16U nBytes);
Int16U                              hex_to_int(Int8U pos,pInt8U pBuffer);
Int16U                              char_to_hex(Int8U cnum);
Int8U                               hex_to_char(Int8U pos,pInt8U pBuffer);
Int8U                               check_sum8(pInt8U pBuffer, Int16U len);
Int32U                              check_sum32(pInt32U pBuffer,Int32U Size);

U8                                  TFT_init(void);
