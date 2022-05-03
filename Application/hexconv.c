#include "defines.h"

// hex conversions
Int8U  hex_to_char(Int8U pos,pInt8U pBuffer)
{
     Int8U  a,b;
     
     a= pBuffer[pos] - 48;
     if(a>9) a-= 7;
           
     b= pBuffer[pos+1] - 48;
     if(b>9) b-= 7;
           
     return((a<<4) | b);
}

Int16U   hex_to_int(Int8U pos,pInt8U pBuffer)
{
     return ((hex_to_char(pos,pBuffer)<<8) + hex_to_char(pos+2,pBuffer));
}

Int16U   char_to_hex(Int8U cnum)
{
      Int8U  a,b;
      
      a= (cnum & 0x0f);
      b= (cnum & 0xf0)>>4;
      
      if(a>9) a+= 55;
          else a+= 48;
          
      if(b>9) b+= 55;
          else b+= 48;
          
      return(a | (b<<8));
}

// check sum 
Int8U check_sum8(pInt8U pBuffer, Int16U len)
{     
  Int8U crc= 0;
  
  for(Int16U i= 0; i<len; i++) crc+= pBuffer[i];
  return crc;
}

void  hexbuffer_2_binbuffer(pInt8U hexBuffer,pInt8U binBuffer,Int16U nBytes,Int16U Pointer)
{
  for(Int16U i=0;i<nBytes;i++,Pointer+= 2) binBuffer[i]= hex_to_char(Pointer,hexBuffer);
}

void  binbuffer_2_hexbuffer(pInt8U hexBuffer,pInt8U binBuffer,Int16U nBytes,Int16U Pointer)
{
  for(Int16U i=0,codedbyte;i<nBytes;i++) 
  {
    codedbyte=  char_to_hex(binBuffer[i]);
    hexBuffer[Pointer++]= codedbyte>>8;
    hexBuffer[Pointer++]= codedbyte;
  }
}

void  binbuffer_2_binbuffer(pInt8U pBufferSource,pInt8U pBufferDestination,Int16U nBytes)
{
  for(Int16U i=0;i<nBytes;i++) pBufferDestination[i]= pBufferSource[i];
}

Int32U check_sum32(pInt32U pBuffer,Int32U Size)
{
  Int32U    crc,i;
  
  for(i= 0,crc= 0,Size/= 4; i<Size; i++)  crc+= pBuffer[i];
  return crc;
}
