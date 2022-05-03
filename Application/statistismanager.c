#include "defines.h"
#define CURRENT_STEP      ProgramStatus[chanelNum].CurrentStep

/*  управление программами */
void  program_step(Int8U chanelNum)
{ 
  // start program
  OS_Use( &SemaRAM);
  if(ProgramStatus[chanelNum].ProgStatus == START_PROGRAM)
  {
     ProgramStatus[chanelNum].ProgStatus=   WORK_PROGRAM;
     ProgramStatus[chanelNum].CurrentStep=  OutConfig[chanelNum].StartStep;
     ProgramStatus[chanelNum].SetPass=      1;
     ProgramTimer[chanelNum]=               0;
     MainArray[CONTROL_BASE + chanelNum]=   Program[CURRENT_STEP].Param;
     OutConfig[chanelNum].ProgramStatus=    ProgramStatus[chanelNum].data;
     SAVE_CONFIG( OutConfig[chanelNum].ProgramStatus);
  }
  
  if(!ProgramStatus[chanelNum].SetPass)
  {
    if(Program[CURRENT_STEP].SetlTime == ProgramTimer[chanelNum])
    {
      ProgramStatus[chanelNum].SetPass=     1;
      ProgramTimer[chanelNum]=              0;
      MainArray[CONTROL_BASE + chanelNum]=  Program[CURRENT_STEP].Param;
      OutConfig[chanelNum].ProgramStatus= ProgramStatus[chanelNum].data;
      SAVE_CONFIG( OutConfig[chanelNum].ProgramStatus);
    }
    else MainArray[CONTROL_BASE + chanelNum]+= StepShift[chanelNum];
  }
  else
  {
     if(Program[CURRENT_STEP].HoldTime == ProgramTimer[chanelNum])
     {
       if(ProgramStatus[chanelNum].CurrentStep == OutConfig[chanelNum].StopStep)
       {
         ProgramStatus[chanelNum].ProgFinished= 1;
         switch(OutConfig[chanelNum].EndProgCond)
         {
           case ENDP_STOP:  ProgramStatus[chanelNum].ProgStatus= STOP_PROGRAM;
                            OutConfig[chanelNum].ProgramStatus= ProgramStatus[chanelNum].data;
                            return;
                            
           case ENDP_KEEP_LAST:  
                            ProgramTimer[chanelNum]= 0;
                            OutConfig[chanelNum].ProgramStatus= ProgramStatus[chanelNum].data;
                            return;
           
           case ENDP_RESTART:
                            ProgramStatus[chanelNum].ProgStatus= START_PROGRAM;
                            OutConfig[chanelNum].ProgramStatus= ProgramStatus[chanelNum].data;
                            return;
                            
           case ENDP_LOADDEF:
                            ProgramTimer[chanelNum]= 0;
                            MainArray[CONTROL_BASE + chanelNum]= OutConfig[chanelNum].DefParam;
                            OutConfig[chanelNum].ProgramStatus= ProgramStatus[chanelNum].data;
                            return;
         }
       }
       else 
       {
         ProgramStatus[chanelNum].CurrentStep++;
         ProgramStatus[chanelNum].SetPass=      0;
         ProgramTimer[chanelNum]=               0;
         if(Program[CURRENT_STEP].SetlTime)
           StepShift[chanelNum]= (Program[CURRENT_STEP].Param - MainArray[CONTROL_BASE + chanelNum])/Program[CURRENT_STEP].SetlTime;
          else 
          {
            MainArray[CONTROL_BASE + chanelNum]= Program[CURRENT_STEP].Param;
            ProgramStatus[chanelNum].SetPass=    1;
          }         
         OutConfig[chanelNum].ProgramStatus= ProgramStatus[chanelNum].data;
       }
       SAVE_CONFIG( OutConfig[chanelNum].ProgramStatus);
     }
  }
  OS_Unuse( &SemaRAM);
  ProgramTimer[chanelNum]++;
}

#define LOGIC_TR1   BIT0
#define LOGIC_TR2   BIT1
#define LOGIC_ERR   BIT2

/*  управление релейными каналами */
void switch_control(Int8U chanelNum)
{
  Flo32   err;
  Int16U  pattern;
  Int8U   input_param, logic_out, dispConfNum, paramNumd;
  
  pattern=      1 << OutConfig[chanelNum].ChanelNum;
  dispConfNum=  OutConfig[chanelNum].Input >> 3;
  paramNumd=    OutConfig[chanelNum].Input & 7;
  input_param=  DisplayConfig[dispConfNum].DisplayParam[paramNumd];
  
  ProgramStatus[chanelNum].ControlFail= 0;
  
  switch(OutConfig[chanelNum].CntrAttr)
  {
    case  OFF_CONTROL:  if(ManualControl & (1<<chanelNum))  RelayOuts |=  pattern; 
                        else                                RelayOuts &= ~pattern; 
                        break;
                        
    case  LOGIC_CONTROL:
                        logic_out= (ErrorArray[input_param] && (OutConfig[chanelNum].LogicUse & LOGIC_ERR))
                                      || (DisplayConfig[dispConfNum].TreshConfig[paramNumd*2].Status && (OutConfig[chanelNum].LogicUse & LOGIC_TR1))
                                          || (DisplayConfig[dispConfNum].TreshConfig[paramNumd*2 + 1].Status && (OutConfig[chanelNum].LogicUse & LOGIC_TR2));
                        logic_out= OutConfig[chanelNum].ControlType ? ~logic_out : logic_out;   // инверсия
                        
                        if(logic_out) RelayOuts|=  pattern;
                        else          RelayOuts&= ~pattern;                        
                        break;
                        
    case  HEST_CONTROL: 
                        if(ErrorArray[input_param]) 
                        {
                          RelayOuts&= ~pattern;
                          ProgramStatus[chanelNum].ControlFail= 1;
                          break;
                        }                        
                                                                                                
                        if(OutConfig[chanelNum].ProgUse)
                        {
                          if((ProgramStatus[chanelNum].ProgStatus == PAUSE_PROGRAM) || (ProgramStatus[chanelNum].ProgStatus == STOP_PROGRAM))
                          {
                            RelayOuts&= ~pattern;
                            break;
                          }
                          program_step(chanelNum);
                        }
                        else MainArray[CONTROL_BASE + chanelNum]= OutConfig[chanelNum].DefParam;
                        
                        err= MainArray[input_param] - MainArray[CONTROL_BASE + chanelNum];
                        if(OutConfig[chanelNum].ControlType)  err= -err;                      // инверсия
                          
                        if(RelayOuts & pattern)  err-= OutConfig[chanelNum].Hesteresis;
                        else                     err+= OutConfig[chanelNum].Hesteresis;
                        
                        if(err < 0)   RelayOuts|=  pattern;
                        else          RelayOuts&= ~pattern;
                        break;                       
  }
}

/*  управление токовыми каналами */

void analog_control(Int8U chanelNum)
{
  Flo32   voltage_out;
  Int8U   input_param, logic_out, dispConfNum, paramNumd;
  
  dispConfNum=  OutConfig[chanelNum].Input >> 3;
  paramNumd=    OutConfig[chanelNum].Input & 7;
  input_param=  DisplayConfig[dispConfNum].DisplayParam[paramNumd];
  
  ProgramStatus[chanelNum].ControlFail= 0;
  
  switch(OutConfig[chanelNum].CntrAttr)
  {
    case  OFF_CONTROL:  if(ManualControl & (1 << chanelNum))
                          switch(OutConfig[chanelNum].ChanelType)
                          {
                            case ANALOG_TYPE_05:    voltage_out= .5;   break;
                            
                            case ANALOG_TYPE_020:   
                            case ANALOG_TYPE_420:   voltage_out= 2.0;  break;
                          }
                        else voltage_out= 0;
                        break;
                        
    case  LOGIC_CONTROL:
                        logic_out= (ErrorArray[input_param] && (OutConfig[chanelNum].LogicUse & LOGIC_ERR))
                                      || (DisplayConfig[dispConfNum].TreshConfig[paramNumd*2].Status && (OutConfig[chanelNum].LogicUse & LOGIC_TR1))
                                          || (DisplayConfig[dispConfNum].TreshConfig[paramNumd*2 + 1].Status && (OutConfig[chanelNum].LogicUse & LOGIC_TR2));
                        logic_out= OutConfig[chanelNum].ControlType ? ~logic_out : logic_out;   // инверсия

                        if(logic_out)
                          switch(OutConfig[chanelNum].ChanelType)
                          {
                            case ANALOG_TYPE_05:    voltage_out= .5;   break;
                            
                            case ANALOG_TYPE_020:   
                            case ANALOG_TYPE_420:   voltage_out= 2.0;  break;
                          }
                        else
                          switch(OutConfig[chanelNum].ChanelType)
                          {
                            case ANALOG_TYPE_05:    
                            case ANALOG_TYPE_020:   voltage_out= .0;   break;
                               
                            case ANALOG_TYPE_420:   voltage_out= .4;   break;
                          }
                        break;
                        
    case  LINE_CONTROL:
                        if(ErrorArray[input_param]) 
                        {
                          voltage_out= .0;
                          break;
                        }                 
                        
                        voltage_out= (MainArray[input_param] - AnalogConfig[chanelNum][0]) / (AnalogConfig[chanelNum][1] - AnalogConfig[chanelNum][0]);
                        if(voltage_out > 1.)       voltage_out= 1.;
                        else if(voltage_out < .0)  voltage_out= .0;
                        
                        switch(OutConfig[chanelNum].ChanelType)
                        {
                          case ANALOG_TYPE_05:    voltage_out= 0.5 * voltage_out; break;
                          
                          case ANALOG_TYPE_020:   voltage_out= 2.0 * voltage_out; break;
                          
                          case ANALOG_TYPE_420:   voltage_out= 1.6 * voltage_out + .4;  break;
                        }
                        break;
                        
  default:             voltage_out = .0;                        
  }  
  AnalogArray[OutConfig[chanelNum].ChanelNum]=  voltage_out / Uref * 4095 + .5;  // расчет кода ЦАП
}

void  ControlManager(void)
{    
  for(U8 i = 0; i < MAX_OUTS; i++)
  {
    switch(OutConfig[i].ChanelType)
    {
    case NO_CONTROL:      continue;
    
    case RELAY_TYPE:      switch_control(i); 
                          break;
    
    case ANALOG_TYPE_05:  
    case ANALOG_TYPE_020:  
    case ANALOG_TYPE_420:              
                          analog_control(i); 
                          break;
    }
  }
}
