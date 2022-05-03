#include "defines.h"

// статические переменные
static  sd_flags_struct   SD_Flags;

// SPI_XMIT_COMMAND 
void spi_xmit_command(U8 command, U32 data, U8 crc)
{
  U8      i, full_command[6];
	  
  full_command[0] = command;
  full_command[5] = crc;

  // load data to command
  for(i= 4; i > 0; i--)
  {
    full_command[i] = data & 0xff;
    data            = data >> 8;
  }
  // send 
  for(i= 0; i < 6; i++) shift_byte_spi3(full_command[i]);			
} 

// SD_COMMAND_RESPONSE 
void sd_command_response(U8 response_type, pU8 response)
{ 		
  for(U8 i= 0; i < response_type; i++)  
      response[i] = shift_byte_spi3(DUMMY_DATA);
}

// check if present
U8 sd_check(void)
{  
  U8    i;
  U8    sdBuffer[16];
    
  SET_SPI3_300k;  
  SD_CS_HIGH;
  for(i = 0; i < 10; i++) shift_byte_spi3(0xff);                     // 80 тактов "1" 
  SD_CS_LOW;
  
  spi_xmit_command(GO_IDLE_STATE, STUFF_BITS, INITIAL_CRC);	     // CMD0 
  for(i = 0;;)
  {
    sd_command_response(R1_RESPONSE, sdBuffer); 
    if(sdBuffer[0] == IN_IDLE_STATE) break; 
    if( ++i == 5 ) // 250 ms
    {
      errors_flags.sd_not_present = 1;
      break;
    }
    OS_Delay(50);
  }
  
  shift_byte_spi3(DUMMY_DATA);
  SD_CS_HIGH;
  SET_SPI3_10M;
  
  return ((i >= 5) ? 1 : 0);
}

// SD_INITIALIZATION 
U8 sd_initialization(void)
{
  U8    sdBuffer[16];
  if(errors_flags.sd_not_present) return 1;
  
  system_flags.SDHC     =   0;
  
  OS_Use( &SemaSPI3 );  
  SET_SPI3_300k;
  
  SD_CS_HIGH;
  for(U8 i = 0; i < 10; i++) shift_byte_spi3(0xff);                     // 80 тактов "1" 
  SD_CS_LOW;
  
  spi_xmit_command(GO_IDLE_STATE, STUFF_BITS, INITIAL_CRC);	        // CMD0
  START_SD_TIMER(250); // 250 ms
  for(;;)
  {
    sd_command_response(R1_RESPONSE, sdBuffer);
    if(sdBuffer[0] == IN_IDLE_STATE) break; 
    if( SD_Flags.sd_timeout )
    {
      SET_SPI3_10M;
      SD_CS_HIGH;
      OS_Unuse( &SemaSPI3 );
      return 1;
    }
  }
  shift_byte_spi3(DUMMY_DATA);                                          // 8 clock
  
  spi_xmit_command(SEND_IF_COND, INTERFACE_COND, INTERFACE_COND_CRC);	// CMD8
  START_SD_TIMER(250); // 250 ms
  for(U8 i = 0; ; i++)
  {
    sd_command_response(R1_RESPONSE, sdBuffer);
    if((sdBuffer[0] == IN_IDLE_STATE) || (sdBuffer[0] == ILLEGAL_COMMAND_IDLE_STATE)) break; 
    if( SD_Flags.sd_timeout )
    {      
      SD_CS_HIGH;
      OS_Unuse( &SemaSPI3 );
      SET_SPI3_10M;
      return 1;
    }
  }
  
  U8  init_res;  
  //  check for version
  if(sdBuffer[0] == ILLEGAL_COMMAND_IDLE_STATE) 
  {
        shift_byte_spi3(DUMMY_DATA);    // 8 clock
        init_res = sd_version1_initialization();
  }
  else  init_res = sd_version2_initialization();
    
  SD_CS_HIGH;
  OS_Unuse( &SemaSPI3 );
  SET_SPI3_10M;
  
  return init_res;
}

// SD_VERSION1_INITIALIZATION 
U8 sd_version1_initialization(void)
{
  U8    sdBuffer[16];
  
  START_SD_TIMER(1200);
  for(;;)
  {		
    spi_xmit_command(APP_CMD, STUFF_BITS, DUMMY_CRC);           // CMD55
    do
    {
      sd_command_response(R1_RESPONSE, sdBuffer);
      if( SD_Flags.sd_timeout )
      {
        return 1;
      }
    }
    while((sdBuffer[0] != IN_IDLE_STATE) && (sdBuffer[0] != SUCCESS));
    shift_byte_spi3(DUMMY_DATA);   // 8 clock
   		
    //Transmit SEND OP COND command   								
    spi_xmit_command(SD_SEND_OP_COND, STUFF_BITS, DUMMY_CRC);	// ACMD41
    do
    {
      sd_command_response(R1_RESPONSE,sdBuffer);
      if( SD_Flags.sd_timeout )
      {
        return 1;
      }
    }
    while((sdBuffer[0] != IN_IDLE_STATE) && (sdBuffer[0] != SUCCESS));    
    shift_byte_spi3(DUMMY_DATA);   // 8 clock
    
    // analize
    if(sdBuffer[0] == SUCCESS) break;
  }
  
  // Transmit SET_BLOCKLEN command 
  spi_xmit_command(SET_BLOCKLEN, BLOCK_LENGTH, DUMMY_CRC);    // CMD16
  do
  {
    sd_command_response(R1_RESPONSE,sdBuffer);
    if( SD_Flags.sd_timeout )
    {
        return 1;
    }
  }
  while(sdBuffer[0] != SUCCESS);
  shift_byte_spi3(DUMMY_DATA);   // 8 clock
  
  SD_Flags.Version = 0;
  return 0;
}

//###################### SD_VERSION2_INITIALIZATION ##########################
U8 sd_version2_initialization(void)
{   
  U8    sdBuffer[16];
  
  // read R7 response
  sd_command_response(R7_RESPONSE, sdBuffer);
  shift_byte_spi3(DUMMY_DATA);  // 8 clock
  
  // check pattern
  if(sdBuffer[3] != 0xaa)
  {
    return 1;
  }
  
  START_SD_TIMER(1200);    
  for(;;)
  {		
    spi_xmit_command(APP_CMD, STUFF_BITS, DUMMY_CRC);           // CMD55
    do
    {
      sd_command_response(R1_RESPONSE,sdBuffer);
      if( SD_Flags.sd_timeout )
      {
        return 1;
      }
    }
    while((sdBuffer[0] != IN_IDLE_STATE) && (sdBuffer[0] != SUCCESS)); 
    shift_byte_spi3(DUMMY_DATA);      // 8 clock
   								
    spi_xmit_command(SD_SEND_OP_COND, VER2_OP_COND, DUMMY_CRC);	// ACMD41
    do
    {
      sd_command_response(R1_RESPONSE,sdBuffer);
      if( SD_Flags.sd_timeout )
      {
        return 1;
      }
    }
    while((sdBuffer[0] != IN_IDLE_STATE) && (sdBuffer[0] != SUCCESS));
    shift_byte_spi3(DUMMY_DATA);      // 8 clock
    
    // analize
    if(sdBuffer[0] == SUCCESS) break;
  }
  
  spi_xmit_command(READ_OCR, STUFF_BITS, DUMMY_CRC);	// CMD58
  do
  {
    sd_command_response(R1_RESPONSE, sdBuffer);
    if( SD_Flags.sd_timeout )
    {
      return 1;
    }
  }
  while(sdBuffer[0] != SUCCESS);
  sd_command_response(R3_RESPONSE,sdBuffer);
  shift_byte_spi3(DUMMY_DATA);   // 8 clock
  
  //Check if card is High Capacity
  if(sdBuffer[0] & HIGH_CAPACITY)  system_flags.SDHC = 1; 
  
  SD_Flags.Version = 1; 
  return 0;
}

//  ###########################  SD_WRITE_BLOCK  ###########################  
U8 sd_write_block(U32 sector, U8 * buffer)
{
  U16   i;
  U8    error_code = 0;
  U8    sdBuffer[16];  
  
  OS_Use( &SemaSPI3 );  
  SD_CS_LOW;
  
  //Transmit WRITE BLOCK command
  spi_xmit_command(WRITE_BLOCK, sector, DUMMY_CRC);   //CMD24
  START_SD_TIMER(200);
  do
  {
    sd_command_response(R1_RESPONSE, sdBuffer);
    if( SD_Flags.sd_timeout )
    {
      SD_CS_HIGH;  
      OS_Unuse( &SemaSPI3 );
      return 1;
    }
  } while(sdBuffer[0] != SUCCESS);
  shift_byte_spi3(DUMMY_DATA);  // 8 clock
	
  // start token + write block 512 
  shift_byte_spi3(START_BLOCK);	     
  for(i = 0; i < 512; i++)    shift_byte_spi3(*buffer++);

  //Transmit CRC
  shift_byte_spi3(DUMMY_DATA);
  shift_byte_spi3(DUMMY_DATA);

  //After data write, card with send a response stating if the card was accepted
  START_SD_TIMER(200);
  do
  {
    sd_command_response(R1_RESPONSE, sdBuffer);	
    sdBuffer[0] &=  0x0F;		//Mask response to test for errors

    //If there where errors writing data, branch to error function
    if((sdBuffer[0] == CRC_ERROR) || (sdBuffer[0] == WRITE_ERROR) || SD_Flags.sd_timeout)
    {
      SD_CS_HIGH;  
      OS_Unuse( &SemaSPI3 );
      return 1;
    }
  } while(sdBuffer[0] != DATA_ACCEPTED);

  // Card will respond with the DATA OUT line pulled low if the card is still busy
  // erasing. Continue checking DATA OUT line until line is released high.
  START_SD_TIMER(4000);
  do
  {
    sd_command_response(R1_RESPONSE, sdBuffer);
    if( SD_Flags.sd_timeout )
    {
      error_code = 1;
      break;
    }
  } while(sdBuffer[0] != DUMMY_DATA);
  
  SD_CS_HIGH;  
  OS_Unuse( &SemaSPI3 );  

  return error_code;
}

//############################# SD_READ_SINGLE_BLOCK ################################
U8 sd_read_singleblock(U32 dataaddress, pU8 pBuffer, U32 numByte, U32 Shift)
{  
  U8    error_code = 0;
  U8    sdBuffer[16];  
  
  OS_Use( &SemaSPI3 );
  SD_CS_LOW;
  
  //Transmit READ SINGLE BLOCK command
  spi_xmit_command(READ_SINGLE_BLOCK, dataaddress, DUMMY_CRC);
  
  START_SD_TIMER(200);
  do
  {
    sd_command_response(R1_RESPONSE, sdBuffer);
    if( SD_Flags.sd_timeout )
    {
      SD_CS_HIGH;  
      OS_Unuse( &SemaSPI3 );
      return 1;
    }
  } while(sdBuffer[0] != SUCCESS);

  // wait START_BLOCK tokeb\n
  START_SD_TIMER(200);
  do
  {
    sd_command_response(R1_RESPONSE, sdBuffer);
     if( SD_Flags.sd_timeout )
    {
      SD_CS_HIGH;  
      OS_Unuse( &SemaSPI3 );
      return 1;
    }
  } while(sdBuffer[0] != START_BLOCK);
  
  for(U16 i = 0; i < 512; i++)  // 512 block
  {
    if( (i < Shift) || (i >= (Shift + numByte)) )   shift_byte_spi3(DUMMY_DATA);
    else                       pBuffer[i - Shift] = shift_byte_spi3(DUMMY_DATA);
  }
  // CRC + dummy
  shift_byte_spi3(DUMMY_DATA);
  shift_byte_spi3(DUMMY_DATA);
  shift_byte_spi3(DUMMY_DATA);
  
  SD_CS_HIGH;  
  OS_Unuse( &SemaSPI3 );

  return error_code;
}

//############################# SD_READ_MULTI_BLOCK ################################
U8 sd_read_multiblock(U32 dataaddress, pU8 pBuffer, U32 numBlock)
{  
  U8    error_code = 0;
  U8    sdBuffer[16];  
  
  OS_Use( &SemaSPI3 );
  SD_CS_LOW;
  
  //Transmit READ SINGLE BLOCK command
  spi_xmit_command(READ_MULTIPLE_BLOCK, dataaddress, DUMMY_CRC);
  
  START_SD_TIMER(200);
  do
  {
    sd_command_response(R1_RESPONSE, sdBuffer);
    if( SD_Flags.sd_timeout )
    {
      SD_CS_HIGH;  
      OS_Unuse( &SemaSPI3 );
      return 1;
    }
  } while(sdBuffer[0] != SUCCESS);

  /// reading blocks
  for(U32 i = 0; i < numBlock; i++)
  {
    // wait START_BLOCK tokeb\n
    START_SD_TIMER(200);
    do
    {
      sd_command_response(R1_RESPONSE, sdBuffer);
      if( SD_Flags.sd_timeout )
      {
        SD_CS_HIGH;  
        OS_Unuse( &SemaSPI3 );
        return 1;
      }
    } while(sdBuffer[0] != START_BLOCK);
  
    for(U32 i= 0; i < 512; i++)  // 512 block
    {
      *pBuffer = shift_byte_spi3(DUMMY_DATA);
      pBuffer++;
    }
    // CRC
    shift_byte_spi3(DUMMY_DATA);
    shift_byte_spi3(DUMMY_DATA);
  }
  
  //Transmit STOP_TRANSMISSION command
  spi_xmit_command(STOP_TRANSMISSION, STUFF_BITS, DUMMY_CRC);
  
  START_SD_TIMER(200);
  do
  {
    sd_command_response(R1_RESPONSE, sdBuffer);
    if( SD_Flags.sd_timeout )
    {
      error_code = 1;
      break;
    }
  } while(sdBuffer[0] != SUCCESS);
  
  shift_byte_spi3(DUMMY_DATA);
  SD_CS_HIGH;  
  OS_Unuse( &SemaSPI3 );

  return error_code;
}

//############################# OPEN SD to read MULTI_BLOCK ################################
U8  open_SDCard(U32 dataaddress)
{
  U8    error_code = 0;
  U8    sdBuffer[16];  
  
  OS_Use( &SemaSPI3 );
  SD_CS_LOW;
  
  //Transmit READ SINGLE BLOCK command
  spi_xmit_command( READ_MULTIPLE_BLOCK, (system_flags.SDHC ? (dataaddress >> 9) : dataaddress), DUMMY_CRC);
  
  START_SD_TIMER(200);
  do
  {
    sd_command_response(R1_RESPONSE, sdBuffer);
    if( SD_Flags.sd_timeout )
    {
      error_code = 1;
      break;
    }
  } while(sdBuffer[0] != SUCCESS);
  
  return   error_code;
}

//############################# read MULTI_BLOCK without closing Card ################################
U8  read_opened_SDCard(pU8 buffer, U32 numBlock)
{
  U8    error_code = 0;
  U8    sdBuffer[16];  
  
  for(U32 i = 0; i < numBlock; i++)
  {
    // wait START_BLOCK tokeb\n
    START_SD_TIMER(200);
    do
    {
      sd_command_response(R1_RESPONSE, sdBuffer);
      if( SD_Flags.sd_timeout )
      {
        error_code = 1;
        break;
      }
    } while(sdBuffer[0] != START_BLOCK);
  
    for(U32 i= 0; i < 512; i++)  // 512 block
    {
      *buffer = shift_byte_spi3(DUMMY_DATA);
      buffer++;
    }
    // CRC
    shift_byte_spi3(DUMMY_DATA);
    shift_byte_spi3(DUMMY_DATA);
  }
  
  return   error_code;
} 

//############################# CLOSE SD after read MULTI_BLOCK ################################
U8  close_SDCard(void)
{
  U8    error_code = 0;
  U8    sdBuffer[16];  
  
  //Transmit STOP_TRANSMISSION command
  spi_xmit_command(STOP_TRANSMISSION, STUFF_BITS, DUMMY_CRC);
  
  START_SD_TIMER(200);
  do
  {
    sd_command_response(R1_RESPONSE, sdBuffer);
    if( SD_Flags.sd_timeout )
    {
      error_code = 1;
      break;
    }
  } while(sdBuffer[0] != SUCCESS);
  
  shift_byte_spi3(DUMMY_DATA);
  SD_CS_HIGH;  
  OS_Unuse( &SemaSPI3 );

  return error_code;
}

// чтение карты  ###########################  
U8  read_SDCard(void * readBuffer, U32 sdMemAdr, U32 numReadBytes)
{
  div_t   n;
  U16     insegAdr;
  U8      error_code;
  
  n = div(numReadBytes, 512);
  
  // кратные 512
  if(n.quot)
  {
    error_code = sd_read_multiblock((system_flags.SDHC ? (sdMemAdr >> 9) : sdMemAdr), (pU8)readBuffer, n.quot);
    if(error_code) return error_code;
    sdMemAdr += 512 * n.quot;
    readBuffer = (void *)((U32)readBuffer + 512 * n.quot);
  }
 
  if(n.rem)
  {
    // для чтения по UARTs, USB-hid
    insegAdr   = sdMemAdr & 0x1ff;
    sdMemAdr  &= 0xfffffe00;
    error_code = sd_read_singleblock((system_flags.SDHC ? (sdMemAdr >> 9) : sdMemAdr), (pU8)readBuffer, n.rem, insegAdr);
  }    
  return error_code;
}

// запись карты, sdMemAdr строго кратно 512, numWriteBytes строго кратно 512  ###########################  
U8  write_SDCard(void * writeBuffer, U32 sdMemAdr, U32 numWriteBytes)
{  
  div_t       n;
  U8          error_code = 1, i;

  n = div(numWriteBytes, 512);  
  for(i = 0; i < n.quot; i++)
  {
    error_code = sd_write_block((system_flags.SDHC ? (sdMemAdr >> 9) : sdMemAdr), (pU8)writeBuffer);
    if(error_code) return error_code;
    sdMemAdr += 512;
    writeBuffer = (void *)((U32)writeBuffer + 512);
  }
  
//  if(!n.quot) error_code = sd_write_block((system_flags.SDHC ? (sdMemAdr >> 9) : sdMemAdr), (pU8)writeBuffer);

  return error_code;
}

/* программный таймер for SD */
void SDTimerISR(void)
{  
  SD_Flags.sd_timeout = 1;
}
