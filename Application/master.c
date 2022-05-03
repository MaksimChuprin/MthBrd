#include "defines.h"

// порядок бит в конфигурации преобразователя
#define   CH4_ON            0
#define   O2_ON             1
#define   CO2_ON            2
#define   CO_ON             3
#define   NH3_ON            4
#define   H2S_ON            5
#define   NO2_ON            6
#define   SO2_ON            7

// global
const U8 gas[8]                = { O2_ON, CO2_ON, CH4_ON, CO_ON, H2S_ON, NH3_ON, NO2_ON, SO2_ON };              // порядок индикации в LED приборе (МАГ-6-С)
const F32 treshArray[8][2]     = { 0.75, 1.75, 19, 17, 0.5, 1.5, 20, 100, 20, 60, 10, 30, 2, 10, 10, 30 };      // пороги
const U8  treshTypeArray[8][2] = { 1, 1, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 };                            // вехний/нижний
const U8  treshWghtArray[8][2] = { 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1 };                            // внимание/тревога

// local
static U8                         Uart1Buffer[U1BUFLEN];
static U8                         Rx1Counter;
const  U8                         command_array[11] = "$0001RR0000";

// ecalc
static Flo32 e_calc(Flo32 t) 
{
  Flo32   x1, x2, x3, y1, y2, y3, s1, s2, s3, s4, a, b, c;
  Int8S   i;
    
  if(t >= 0)  return (2.719325e-8*t*t*t*t*t + 2.8791907e-6*t*t*t*t + 2.5602536e-4*t*t*t + 1.475077e-2*t*t + 4.377191e-1*t + 6.1225);

  i =  ceil(t);
  x1=   i;  y1= E_TAB[-i];
  x2= --i;  y2= E_TAB[-i];
  x3= --i;  y3= E_TAB[-i];
        
  s1= (y3-y1)/(x3-x1);
  s2= (y3-y2)/(x3-x2);
  s3= (x3*x3-x1*x1)/(x3-x1);
  s4= (x3*x3-x2*x2)/(x3-x2);
  a=  (s1-s2)/(s3-s4);
  b=  s1 - a*s3;
  c=  y1 - a*x1*x1 - b*x1;
        
  return (a*t*t + b*t + c);
}

static Flo32 tcalc(Flo32 e) 
{  
  Flo32   tt, ee;
  Flo32   t1= -100., t2= 100.;

  for(;;)
  {
    tt = (t1 + t2)/2;
    ee = e_calc(tt);

    if(e >= ee) t1 = tt;
           else t2 = tt;

    if((t2 - t1) < 0.05) return tt;
  }
}

// % -> e
static Flo32 ep_Calc(Flo32 Hum, Flo32 Temp)
{
  return Hum * e_calc(Temp) / 100.;
}


static void LimitsCheck(U8 i)
{  
  F32   up_Err, low_Err;
  
  // cher error limits & arrange limit range
  if(ErrorArray[i] & (CONNECT_ERR | ASSIGNED_ERR | COMPLEX_ERR)) return;
  
  ErrorArray[i] &= ~(LOWLIMIT_ERR + UPLIMIT_ERR); 
  low_Err        =  MeasureAttribute[i].LowLimit  -  MeasureAttribute[i].ErrLow / 100. * fabs(MeasureAttribute[i].UpLimit - MeasureAttribute[i].LowLimit);
  up_Err         =  MeasureAttribute[i].UpLimit   +  MeasureAttribute[i].ErrUp / 100.  * fabs(MeasureAttribute[i].UpLimit - MeasureAttribute[i].LowLimit);
  
  if(MainArray[i] < low_Err)  ErrorArray[i]|= LOWLIMIT_ERR;      
  if(MainArray[i] > up_Err) ErrorArray[i]|= UPLIMIT_ERR;      
      
  if(MainArray[i] < MeasureAttribute[i].LowLimit)           MainArray[i] = MeasureAttribute[i].LowLimit;
  else  if(MainArray[i] > MeasureAttribute[i].UpLimit)      MainArray[i] = MeasureAttribute[i].UpLimit;
}

static Boolean CheckFuncParam(U8 calcNum)
{  
  pU8         param_pnt;
  U8          k;
  
  /* нет вычислений */
  if(CalcConfig[calcNum].FuncNum == 0) 
  {
    ErrorArray[calcNum + CALC_BASE] =  ASSIGNED_ERR;
    return 1;
  }
  
  param_pnt = CalcConfig[calcNum].InputMeasureParamNum;
  
  for(k= 0; k < 5; k++)
  {
    if(param_pnt[k] == 0xff)  
    {
      if(k == 0)
      {
        ErrorArray[calcNum + CALC_BASE]=  ASSIGNED_ERR;
        return 1;
      }      
      else break;
    }
      
    if( ErrorArray[param_pnt[k]] && !(CalcConfig[calcNum].ErrorAllowed & (1 << k)) )
    {
      if( ErrorArray[param_pnt[k]] == CONNECT_ERR )     ErrorArray[calcNum + CALC_BASE] = CONNECT_ERR;
      else                                              ErrorArray[calcNum + CALC_BASE] = COMPLEX_ERR;
      return 1;
    }
  }
  
  ErrorArray[calcNum + CALC_BASE] =  NO_ERR;
  return 0;
}

static Boolean IsCalcDisplParam(U8 calcNum)
{
  for(U8 i = 0; i < GetDisplayChannelsCount(); i++)
  {
    for (U8 j = 0; j < GetDisplayChannelParamsCount(i); j++)
    {
      U8 calc_config_index = DisplayConfig[i].DisplayParam[j] - CALC_BASE;
      if( calc_config_index == calcNum ) return 1;
    }
  }
  return 0;
}


static void  CalculationChanels(void)
{      
  F32         x, f_var, p1, p2, temp, min, max, low, high;
  F32         inputMeasureParam[5], inputConstantParam[5];
  U8          i, k, j, l;
  Int8U       index;
  
  // вычисляем не индицируемые параметры 
  for(i= 0; i < 128; i++)
  {    
    if( IsCalcDisplParam(i) ) continue;
    
    if( CheckFuncParam(i)   ) continue; // проверка
    
    /*  извлекаем входные параметры */
    for( k = 0; k < 5; k++)
    {
      inputMeasureParam[k]=    MainArray[CalcConfig[i].InputMeasureParamNum[k]];
      j= CalcConfig[i].InputConstantParamNum[k];
      inputConstantParam[k]=   DisplayConfig[j/12].Constant[j%12];
    }
      
    switch(CalcConfig[i].FuncNum)
    {
                                                                  
      case  CALC_POLY:
                      x= inputMeasureParam[0];
                      f_var= inputConstantParam[0];
                      for(k= 1; k < 5; k++)
                      {
                        j= CalcConfig[i].InputConstantParamNum[k];
                        if(j == 0xff)  break;
                        f_var += x * inputConstantParam[k];                        
                        x *= inputMeasureParam[0];
                      }
                      MainArray[i + CALC_BASE]= f_var;
                      LimitsCheck(i + CALC_BASE);
                      break;
                                        
      // функция не определена
      default:        ErrorArray[i + CALC_BASE] =  ASSIGNED_ERR;
                      continue;
    }   
  }

  // вычисляемые индицируемые параметры 
  
  // для пересчёта влажности по давлению с учётом возможности использованиея датчиков/констант
  //(DisplayConfig[CurrentDisplay].ParamConfig[index] & PARAM_PRESSURE1_SOURCE) ? "Датчик" : "Конст."
  //(DisplayConfig[CurrentDisplay].ParamConfig[index] & PARAM_PRESSURE2_SOURCE) ? "Датчик" : "Конст."
  //Таким образом:
  //1. (Int8U)DisplayConfig[].Constant[] - индекс параметра давления в MeasureAttribute
  //2. DisplayConfig[].Constant[] - значение константы
  for(i = 0; i < GetDisplayChannelsCount(); i++)
  {
    for (j = 0; j < GetDisplayChannelParamsCount(i); j++)
    {
      Int8U measure_attribute = DisplayConfig[i].DisplayParam[j];
      Int8U calc_config_index = measure_attribute - CALC_BASE;
  
      // если параметр не вычисляемый
      if (measure_attribute < CALC_BASE) continue;
      
      // если некорректные аргументы или присутствуют ошибки
      if(CheckFuncParam(calc_config_index)) continue;
      
      //  извлекаем входные параметры 
      for(k = 0; k < 5; k++)
      {
        inputMeasureParam[k] = MainArray[CalcConfig[calc_config_index].InputMeasureParamNum[k]];
        l = CalcConfig[calc_config_index].InputConstantParamNum[k];
        inputConstantParam[k] = DisplayConfig[l/12].Constant[l%12];
      }
      
      switch(CalcConfig[calc_config_index].FuncNum)
      {
        //   М0 - влажность, М1 - температура, М2 - Р1 точка измерения, М3 - Р2 точка пересчета, С0 - Р1 точка измерения, С1 - Р2 точка пересчета 
        case 4:         ErrorArray[measure_attribute] = 0;
                        
                        f_var = e_calc(inputMeasureParam[0]);
      
                        if (DisplayConfig[i].ParamConfig[j] & PARAM_PRESSURE1_SOURCE)
                        {
                          ErrorArray[measure_attribute] = ErrorArray[(Int8U)inputConstantParam[0]];
                          p1 = MainArray[(Int8U)inputConstantParam[0]];
                        }
                        else
                          p1 = inputConstantParam[0];

                        if (DisplayConfig[i].ParamConfig[j] & PARAM_PRESSURE2_SOURCE)
                        {
                          ErrorArray[measure_attribute] |= ErrorArray[(Int8U)inputConstantParam[1]];
                          p2 = MainArray[(Int8U)inputConstantParam[1]];
                        }
                        else
                          p2 = inputConstantParam[1];

                        MainArray[measure_attribute] = tcalc(f_var * p2 / p1);                                                                      // Cp -> Cp
                        LimitsCheck(measure_attribute);
                        break;
                        
        case 5:         ErrorArray[measure_attribute] = 0;                  // Cp -> mg/m3
                        
                        MainArray[measure_attribute] = 217000. * e_calc(inputMeasureParam[0])/(273.15 + inputMeasureParam[1]);                      
                        LimitsCheck(measure_attribute);
                        break;

        case 6:         ErrorArray[measure_attribute] = 0;
                  
                        if (DisplayConfig[i].ParamConfig[j] & PARAM_PRESSURE1_SOURCE)
                        {
                          ErrorArray[measure_attribute] = ErrorArray[(Int8U)inputConstantParam[0]];
                          p1 = MainArray[(Int8U)inputConstantParam[0]];
                        }
                        else
                          p1 = inputConstantParam[0];

                        if (DisplayConfig[i].ParamConfig[j] & PARAM_PRESSURE2_SOURCE)
                        {
                          ErrorArray[measure_attribute] |= ErrorArray[(Int8U)inputConstantParam[1]];
                          p2 = MainArray[(Int8U)inputConstantParam[1]];
                        }
                        else
                          p2 = inputConstantParam[1];
                        
                        MainArray[measure_attribute] = p2 / p1 * e_calc(inputMeasureParam[0])/e_calc(inputMeasureParam[1]) * 100.;                  // Cp -> %
                        LimitsCheck(measure_attribute);
                        break;
  
        case 7:         ErrorArray[measure_attribute] = 0;
                    
                        if (DisplayConfig[i].ParamConfig[j] & PARAM_PRESSURE1_SOURCE)
                        {
                          ErrorArray[measure_attribute] = ErrorArray[(Int8U)inputConstantParam[0]];
                          p1 = MainArray[(Int8U)inputConstantParam[0]];
                        }
                        else
                          p1 = inputConstantParam[0];

                        if (DisplayConfig[i].ParamConfig[j] & PARAM_PRESSURE2_SOURCE)
                        {
                          ErrorArray[measure_attribute] |= ErrorArray[(Int8U)inputConstantParam[1]];
                          p2 = MainArray[(Int8U)inputConstantParam[1]];
                        }
                        else
                          p2 = inputConstantParam[1];
                        
                        f_var= p2 / p1 * e_calc(inputMeasureParam[0]);
                        MainArray[measure_attribute] = f_var / (1013.25 * p2 - f_var) * 1e6;                                                        // Cp -> ppm
                        LimitsCheck(measure_attribute);
                        break;
                        
        // Давление из тока
        case 8:         ErrorArray[measure_attribute] = 0;

                        //out current
                        index = (Int8U)inputConstantParam[1];
                        if (index > 2) index = 0;
                        
                        if (index == 0)
                        {
                          low = 0.0;
                          high = 0.5;
                        }
                        else if (index == 1)
                        {
                          low = 0.0;
                          high = 2.0;
                        }
                        else if (index == 2)
                        {
                          if (inputMeasureParam[0] < 0.36)
                          {
                            ErrorArray[measure_attribute] = CONNECT_ERR;
                            break;
                          }
                          
                          low = 0.4;
                          high = 2.0;
                        }
                        
                        //range
                        min = inputConstantParam[2];
                        max = inputConstantParam[3];
                        
                        //pressure calculation (linear interpolation) [кПа]
                        temp = min + (max - min) * ((inputMeasureParam[0] - low)/(high - low));
                        
                        //кПа -> атм
                        temp *= 0.00986923267;
                        
                        //type (если избыточное, добавить 1)
                        if ( (Int8U)inputConstantParam[0] == 1) temp += 1;
                        
                        //filter
                        MainArray[measure_attribute] += 0.1 * (temp - MainArray[measure_attribute]);
                        
                        //set limits (в атм)
                        MeasureAttribute[measure_attribute].LowLimit = min * 0.00986923267;
                        MeasureAttribute[measure_attribute].UpLimit = max * 0.00986923267;
                        
                        LimitsCheck(measure_attribute);
                        break;
                        
        // % -> % (с давлением)
        case 9:         ErrorArray[measure_attribute] = 0;
                        
                        f_var = e_calc(inputMeasureParam[0]);
      
                        if (DisplayConfig[i].ParamConfig[j] & PARAM_PRESSURE1_SOURCE)
                        {
                          ErrorArray[measure_attribute] = ErrorArray[(Int8U)inputConstantParam[0]];
                          p1 = MainArray[(Int8U)inputConstantParam[0]];
                        }
                        else
                          p1 = inputConstantParam[0];

                        if (DisplayConfig[i].ParamConfig[j] & PARAM_PRESSURE2_SOURCE)
                        {
                          ErrorArray[measure_attribute] |= ErrorArray[(Int8U)inputConstantParam[1]];
                          p2 = MainArray[(Int8U)inputConstantParam[1]];
                        }
                        else
                          p2 = inputConstantParam[1];

                        MainArray[measure_attribute] = inputMeasureParam[0] * p2 / p1;
                        LimitsCheck(measure_attribute);
                        break;          

        // % -> г/м3 (с давлением)
        case 10:         ErrorArray[measure_attribute] = 0;
                        
                        //f_var = e_calc(inputMeasureParam[0]);
                        f_var = ep_Calc(inputMeasureParam[0], inputMeasureParam[1]);
      
                        if (DisplayConfig[i].ParamConfig[j] & PARAM_PRESSURE1_SOURCE)
                        {
                          ErrorArray[measure_attribute] = ErrorArray[(Int8U)inputConstantParam[0]];
                          p1 = MainArray[(Int8U)inputConstantParam[0]];
                        }
                        else
                          p1 = inputConstantParam[0];

                        if (DisplayConfig[i].ParamConfig[j] & PARAM_PRESSURE2_SOURCE)
                        {
                          ErrorArray[measure_attribute] |= ErrorArray[(Int8U)inputConstantParam[1]];
                          p2 = MainArray[(Int8U)inputConstantParam[1]];
                        }
                        else
                          p2 = inputConstantParam[1];

                        MainArray[measure_attribute] = 217. * (f_var * p2 / p1) / (273.15 + inputMeasureParam[1]);
                        LimitsCheck(measure_attribute);
                        break;       

        // % -> °Cтр (с давлением)
        case 11:         ErrorArray[measure_attribute] = 0;
                        
                        //f_var = e_calc(inputMeasureParam[0]);
                        f_var = ep_Calc(inputMeasureParam[0], inputMeasureParam[1]);
      
                        if (DisplayConfig[i].ParamConfig[j] & PARAM_PRESSURE1_SOURCE)
                        {
                          ErrorArray[measure_attribute] = ErrorArray[(Int8U)inputConstantParam[0]];
                          p1 = MainArray[(Int8U)inputConstantParam[0]];
                        }
                        else
                          p1 = inputConstantParam[0];

                        if (DisplayConfig[i].ParamConfig[j] & PARAM_PRESSURE2_SOURCE)
                        {
                          ErrorArray[measure_attribute] |= ErrorArray[(Int8U)inputConstantParam[1]];
                          p2 = MainArray[(Int8U)inputConstantParam[1]];
                        }
                        else
                          p2 = inputConstantParam[1];

                        MainArray[measure_attribute] = tcalc(f_var * p2 / p1);
                        LimitsCheck(measure_attribute);
                        break;

        // % -> ppm (с давлением)
        case 12:        ErrorArray[measure_attribute] = 0;
                        
                        f_var = e_calc(inputMeasureParam[0]);
                        f_var = ep_Calc(inputMeasureParam[0], inputMeasureParam[1]);
                        
                        if (DisplayConfig[i].ParamConfig[j] & PARAM_PRESSURE1_SOURCE)
                        {
                          ErrorArray[measure_attribute] = ErrorArray[(Int8U)inputConstantParam[0]];
                          p1 = MainArray[(Int8U)inputConstantParam[0]];
                        }
                        else
                          p1 = inputConstantParam[0];

                        if (DisplayConfig[i].ParamConfig[j] & PARAM_PRESSURE2_SOURCE)
                        {
                          ErrorArray[measure_attribute] |= ErrorArray[(Int8U)inputConstantParam[1]];
                          p2 = MainArray[(Int8U)inputConstantParam[1]];
                        }
                        else
                          p2 = inputConstantParam[1];

                        x = (f_var * p2 / p1);
                        MainArray[measure_attribute] = x / (1013.25  * p2 - x) * 1e6;
                        LimitsCheck(measure_attribute);
                        break;

        // Расход [ТТМ-2 Т]
        case 13:
                      p1 = inputConstantParam[0];
                      p2 = inputConstantParam[1];
                      if (DisplayConfig[i].ParamConfig[j] & PARAM_FLOW_CUT_TYPE)
                        MainArray[measure_attribute] = inputMeasureParam[0] * p1 * p2 * 0.0036; // расчитать расход м3/ч = V * a * b * 3600 (с -> ч) / 1е6 (мм -> м)
                      else
                        MainArray[measure_attribute] = inputMeasureParam[0] * p1 * p1 * 0.0028274333882; // расчитать расход м3/ч = V * D * D * 3,14 * 3600 / 4 / 1е6 (мм -> м)
                      //if (Time % 2 == 0)
                      //  MainArray[measure_attribute] = 9999;
                      //else
                      //  MainArray[measure_attribute] = 9;
                      LimitsCheck(measure_attribute);
                      break;

        // об % кислорода пересчет с учетом давления                
        case 17:        if (DisplayConfig[i].ParamConfig[j] & PARAM_PRESSURE1_SOURCE)
                        {
                          ErrorArray[measure_attribute] = ErrorArray[(Int8U)inputConstantParam[0]];
                          p1 = MainArray[(Int8U)inputConstantParam[0]];
                        }
                        else
                          p1 = inputConstantParam[0];

                        MainArray[measure_attribute] = inputMeasureParam[0] * (0.98 / p1);
                        LimitsCheck(measure_attribute);
                        break;
      // об % кислорода пересчет в г/м3        
      case 18:          if (DisplayConfig[i].ParamConfig[j] & PARAM_PRESSURE1_SOURCE)
                        {
                          ErrorArray[measure_attribute] = ErrorArray[(Int8U)inputConstantParam[0]];
                          p1 = MainArray[(Int8U)inputConstantParam[0]];
                        }
                        else
                          p1 = inputConstantParam[0];

                        if (DisplayConfig[i].ParamConfig[j] & PARAM_PRESSURE2_SOURCE)
                        {
                          ErrorArray[measure_attribute] |= ErrorArray[(Int8U)inputConstantParam[1]];
                          p2 = MainArray[(Int8U)inputConstantParam[1]];
                        }
                        else
                          p2 = inputConstantParam[1];
                        
                        MainArray[measure_attribute] =  inputMeasureParam[0] * 6.637* (0.98 / p1);
                        LimitsCheck(measure_attribute);
                        break;                        
                        
        // об % CO2 пересчет с учетом давления                
        case 23:        if (DisplayConfig[i].ParamConfig[j] & PARAM_PRESSURE1_SOURCE)
                        {
                          ErrorArray[measure_attribute] = ErrorArray[(Int8U)inputConstantParam[0]];
                          p1 = MainArray[(Int8U)inputConstantParam[0]];
                        }
                        else
                          p1 = inputConstantParam[0];

                        if (DisplayConfig[i].ParamConfig[j] & PARAM_PRESSURE2_SOURCE)
                        {
                          ErrorArray[measure_attribute] |= ErrorArray[(Int8U)inputConstantParam[1]];
                          p2 = MainArray[(Int8U)inputConstantParam[1]];
                        }
                        else
                          p2 = inputConstantParam[1];
                        
                        MainArray[measure_attribute] = inputMeasureParam[0] * (0.98 / p1);
                        LimitsCheck(measure_attribute);
                        break;
      // СО мг/м3 пересчет в ppm
      case 20:          MainArray[measure_attribute] =  inputMeasureParam[0] * 0.858;
                        LimitsCheck(measure_attribute);
                        break;

      // об % CO2 пересчет в мг/м3        
      case 21:          if (DisplayConfig[i].ParamConfig[j] & PARAM_PRESSURE1_SOURCE)
                        {
                          ErrorArray[measure_attribute] = ErrorArray[(Int8U)inputConstantParam[0]];
                          p1 = MainArray[(Int8U)inputConstantParam[0]];
                        }
                        else
                          p1 = inputConstantParam[0];

                        if (DisplayConfig[i].ParamConfig[j] & PARAM_PRESSURE2_SOURCE)
                        {
                          ErrorArray[measure_attribute] |= ErrorArray[(Int8U)inputConstantParam[1]];
                          p2 = MainArray[(Int8U)inputConstantParam[1]];
                        }
                        else
                          p2 = inputConstantParam[1];
                        
                        MainArray[measure_attribute] =  inputMeasureParam[0] * 18.257* (0.98 / p1);
                        LimitsCheck(measure_attribute);
                        break;                        
      }
    }
  }
}

static void CheckThresholds(void)
{
  for (U16 i = 0; i < GetDisplayChannelsCount(); i++)
  {
    for (U16 j = 0; j < GetDisplayChannelParamsCount(i); j++)
    {
      if( ErrorArray[DisplayConfig[i].DisplayParam[j]] ) DisplayConfig[i].TreshConfig[j*2].Status = DisplayConfig[i].TreshConfig[j*2+1].Status = 0; // если ошибки - нет реакции на пороги
      else
      {
        if (DisplayConfig[i].TreshConfig[j*2].Up)
        {
          DisplayConfig[i].TreshConfig[j*2].Status = MainArray[DisplayConfig[i].DisplayParam[j]] > DisplayConfig[i].Treshold[j*2];
        }
        else
        {
          DisplayConfig[i].TreshConfig[j*2].Status = MainArray[DisplayConfig[i].DisplayParam[j]] < DisplayConfig[i].Treshold[j*2];
        }
      
        if (DisplayConfig[i].TreshConfig[j*2+1].Up)
        {
          DisplayConfig[i].TreshConfig[j*2+1].Status = MainArray[DisplayConfig[i].DisplayParam[j]] > DisplayConfig[i].Treshold[j*2+1];
        }
        else
        {
          DisplayConfig[i].TreshConfig[j*2+1].Status = MainArray[DisplayConfig[i].DisplayParam[j]] < DisplayConfig[i].Treshold[j*2+1];
        }
      }
    }
  }
}

// isr uart1
#pragma optimize = none
void UART1_ISR(void) 
{
  static U16    txCounter;
  static U8     lastSent;
  U8            inByte;
  
  OS_EnterInterrupt();
  
  if( USART1->SR & USART_SR_RXNE )
  {
    inByte = USART1->DR;
    if( USART1->CR1 & USART_CR1_RE )
    {
      if((inByte == '!') || (inByte == '?'))  Rx1Counter= 0;
      else  if((Rx1Counter == 0) && (inByte != '!') && (inByte != '?'))
      {
        OS_LeaveInterrupt(); 
        return; 
      }

      if(inByte != 0x0d)
      {
        Uart1Buffer[Rx1Counter++]=      inByte;
        if(Rx1Counter == U1BUFLEN)      
        { 
          CLEAR_BIT(USART1->CR1, USART_CR1_RE);
          OS_LeaveInterrupt(); 
          return; 
        }
      }
      else
      {
        CLEAR_BIT(USART1->CR1, USART_CR1_RE);
        OS_SignalEvent( U1_MESSAGE_IN, &OS_MASTER);        
      }
    }
  }
  
  if( (USART1->CR1 & USART_CR1_TCIE) && (USART1->SR & USART_SR_TC) )
  {
    if((lastSent == 0x0d) || (txCounter == U1BUFLEN))
    {
      U1RS485_OFF;
      lastSent   =             0;
      Rx1Counter = txCounter=  0;      
      CLEAR_BIT (USART1->CR1, USART_CR1_TCIE);
      SET_BIT   (USART1->CR1, USART_CR1_RE);      
    }
    else 
    {
      USART1->DR = Uart1Buffer[txCounter]; 
      lastSent  = Uart1Buffer[txCounter++];
    }
  }
  
  OS_LeaveInterrupt();
}

const  U8     command_rd[11]  = "$0000RR0000";
const  U8     command_id[11]  = "$0000IG0000";
const  U8     command_rcw[11] = "$0000RI1000";
const  U8     command_p0[7]  = "$0000P0";
const  U8     command_p1[7]  = "$0000P1";
const  U8     command_st[7]  = "$0000ST";

// задача MASTER_Task
void MASTER_Task(void) 
{
  F32           f_var;  
  U32           masterWaitTime = 1, startLoopTime, timePass, interror, i;
  U16           data_adr, data_len;
  U16           id; 
  U8            inputCount, inLineSelect;//, base;
  volatile U8 base;
  U8            data_num, *command_str;
  U16           cw;
    
  // инициализация UART1
  __HAL_RCC_USART1_CLK_ENABLE();
  OS_ARM_InstallISRHandler(USART1_IRQn + 16, (OS_ISR_HANDLER*)UART1_ISR);
  OS_ARM_ISRSetPrio(USART1_IRQn + 16, 0xFE);
  OS_ARM_EnableISR(USART1_IRQn + 16);
  
  USART1->BRR=             select_speed(S9600, APB2);  // 9600
  USART1->CR1= USART1->CR2= USART1->CR3= USART1->GTPR= 0;
  SET_BIT(USART1->CR1, USART_CR1_UE | USART_CR1_TE | USART_CR1_RXNEIE);
      
  for(;;)
  {          
    OS_Delay(masterWaitTime);
    startLoopTime= OS_GetTime32();
    
    /* цикл опроса входов */
    for(inputCount = 0; inputCount < MAX_INPUT_COUNT; inputCount++)
    {              
      if(InputConfig[inputCount].ChanelAttr == INPUT_OFF) continue;

      // шифровка адреса коммутатора
      inLineSelect = (~InputConfig[inputCount].ComAdr & 0x07) + ((InputConfig[inputCount].ComAdr < 8) ? 0x08 : 0x10);  
      base         =   InputConfig[inputCount].StartAdr;

      // off all lines      
      SetComInputLine(0);
    
      switch(InputConfig[inputCount].ChanelAttr)
      {
        case  INPUT_I:
        case  INPUT_U:  SELECT_MUX_I;

                        SetComInputLine(inLineSelect);
                        OS_Delay(5);
                        
                        // start ADC12-1
                        ADC1_CONVERTION;
                        
                        OS_Use( &SemaRAM);
                        MainArray[base]     =  Uref * ADC1->JDR1 / 4095.;                        
                        MainArray[base + 1] =  Uref * ADC1->JDR2 / 4095.;
                        
                        ErrorArray[base]=      NO_ERR;
                        ErrorArray[base + 1]=  NO_ERR;                        
                        LimitsCheck(base);
                        LimitsCheck(base + 1);        
                        OS_Unuse( &SemaRAM);
                        continue;
      
        case  INPUT_DIGIT_1200:
                        USART1->BRR  =       select_speed(S1200, APB2);
                        f_var        =       8.32;
                        goto        digit_set;
                        
        case  INPUT_DIGIT_2400: 
                        USART1->BRR  =       select_speed(S2400, APB2);
                        f_var        =       4.16;
                        goto        digit_set;
                        
        case  INPUT_DIGIT_4800: 
                        USART1->BRR  =       select_speed(S4800, APB2);
                        f_var        =       2.08;
                        goto        digit_set;                        

        case  INPUT_DIGIT_19200: 
                        USART1->BRR  =       select_speed(S19200, APB2);
                        f_var        =       0.521;
                        goto        digit_set;                        
                        
        case  INPUT_DIGIT_38400: 
                        USART1->BRR  =       select_speed(S38400, APB2);
                        f_var        =       0.261;
                        goto        digit_set;                        
                        
        case  INPUT_DIGIT_9600:                    
        default:            
                        USART1->BRR  =       select_speed(S9600, APB2);
                        f_var        =       1.04;
                        
digit_set:              SELECT_MUX_DIG; 
                        SetComInputLine(inLineSelect); 
                        break;
      } // switch

send_message:
      /* create message */
      switch(CommandStatus[inputCount])
      {
        case GET_ID:          command_str =  (pU8)command_id;
                              data_adr    = 0;
                              data_num    = 2;
                              data_len    = 11;
                              break;
                              
        case GET_CONFIGWORD:  command_str =  (pU8)command_rcw;
                              data_adr    =  0x1000;
                              data_num    =  2;
                              data_len    = 11;
                              break;                              
                        
        case GET_ERR:         command_str =  (pU8)command_rd;
                              data_adr    =  InputConfig[inputCount].ErrorAdr;
                              data_num    =  4;
                              data_len    = 11;
                              break;
                        
        case GET_DATA:        command_str =  (pU8)command_rd;
                              data_adr    =  InputConfig[inputCount].DataAdr;
                              data_num    =  InputConfig[inputCount].InpNum * 4;  // float data
                              data_len    = 11;
                              break;
                              
        case START_PUMP:      command_str =  (pU8)command_p1;
                              data_adr    = 0;
                              data_num    = 0;
                              data_len    = 7;
                              break;
                              
        case STOP_PUMP:       command_str =  (pU8)command_p0;
                              data_adr    = 0;
                              data_num    = 0;
                              data_len    = 7;
                              break;
                              
        case STOP_TIMER:      command_str =  (pU8)command_st;
                              data_adr    = 0;
                              data_num    = 0;
                              data_len    = 7;
                              break;
                              
      }
      
      CLEAR_BIT(USART1->CR1, USART_CR1_RE);
      U1RS485_ON;
      OS_ClearEvents(NULL);
  
      /* create message */
      binbuffer_2_binbuffer(command_str, Uart1Buffer, data_len);
      
      /* load addres  */
      if( (InputConfig[inputCount].NetAdr == 255) || (InputConfig[inputCount].NetAdr == 0) )
      {
        Uart1Buffer[1] = Uart1Buffer[2] = Uart1Buffer[3] = Uart1Buffer[4] = 'F';
      }
      else binbuffer_2_hexbuffer(Uart1Buffer, &InputConfig[inputCount].NetAdr, 1, 3);
  
      /* load address of asked bytes */
      if(data_num)  
      {
        binbuffer_2_hexbuffer(Uart1Buffer, (pU8)((pU8)&data_adr + 1), 1, 7);
        binbuffer_2_hexbuffer(Uart1Buffer, (pU8)&data_adr, 1, 9); 
      }
      
      /* load num of asked bytes */
      if(data_num)  binbuffer_2_hexbuffer(Uart1Buffer, &data_num, 1, 11);
  
      /* check sum */
      i = check_sum8(Uart1Buffer, data_len+2);
      binbuffer_2_hexbuffer(Uart1Buffer, (pU8)&i, 1, data_len+2);  
      Uart1Buffer[data_len+4] =  0x0d;
  
      /* send message  */
      masterWaitTime      = 25 + (30 + data_num * 2) * f_var;  // calc max awaiting time: 25ms + N*speed_const ms
      OS_Delay(10); // 
      SET_BIT(USART1->CR1, USART_CR1_TCIE);
      
      /* wait for anwser */
      if(OS_WaitSingleEventTimed(U1_MESSAGE_IN, masterWaitTime))
      {       
        if( check_sum8(Uart1Buffer, Rx1Counter-2) != hex_to_char(Rx1Counter-2, Uart1Buffer) ) goto error_calc;
        ErrorCount[inputCount] = 0; 
        
        switch(CommandStatus[inputCount])
        {
          case GET_ID:    if(Uart1Buffer[0] != '?') 
                          {                            
                            hexbuffer_2_binbuffer(Uart1Buffer, (pU8)&id, 2, 7);                             
                            if(id != InputConfig[inputCount].Id) // check type
                            {                              
                              OS_Use( &SemaRAM);
                              for(i = 0; i < InputConfig[inputCount].InpNum; i++)  ErrorArray[base + i] = TYPE_ERR;
                              OS_Unuse( &SemaRAM);
                              break;  // stop asking till right type
                            }  
                          } 
                          
                          CommandStatus[inputCount] = GET_CONFIGWORD;                            
                          goto  send_message; 
                                                    
          case GET_CONFIGWORD:
                          hexbuffer_2_binbuffer(Uart1Buffer, (pU8)&cw, 2, 7);
                                                                                      
                          U8 changed = 0; 
                          if ( strstr(DeviceName,"МАГ") != NULL ) // Для МАГ-6 определение измеряемых преобразователем параметров
                          {
                             for (U8 j = 0, count = 0; j < 8; j++)  // 8 газов в МАГ
                             {
                               if ( cw & (1<<(gas[j] + 8)) ) // поиск измеряемых параметров в слове конфигурации
                               {
                                 if ( DisplayConfig[inputCount].DisplayParam[count] != (gas[j] + 8*inputCount) ) { changed = 1; break; } // не совпадает с текущей конфигурацией
                                 count++;
                               }
                             }
                             
                             U8 count = 0;
                             if ( changed ) // настройка новой конфигурации
                             {                                
                                for (U8 j = 0; j < 8; j++) // 8 газов в МАГ
                                {
                                 if ( cw & (1<<(gas[j] + 8)) ) // поиск измеряемых параметров в слове конфигурации
                                 {
                                    DisplayConfig[inputCount].DisplayParam[count]               = gas[j] + 8*inputCount;
                                    DisplayConfig[inputCount].ParamConfig[count]                = count | ((count<2) ? PARAM_ON_MAINSCREEN : 0); // позиция на экране (TFT)
                                    DisplayConfig[inputCount].Treshold[count*2]                 = treshArray     [gas[j]][0];
                                    DisplayConfig[inputCount].Treshold[count*2+1]               = treshArray     [gas[j]][1];
                                    DisplayConfig[inputCount].TreshConfig[count*2].Up           = treshTypeArray [gas[j]][0];
                                    DisplayConfig[inputCount].TreshConfig[count*2+1].Up         = treshTypeArray [gas[j]][1];
                                    DisplayConfig[inputCount].TreshConfig[count*2].Weight       = treshWghtArray [gas[j]][0];
                                    DisplayConfig[inputCount].TreshConfig[count*2+1].Weight     = treshWghtArray [gas[j]][1];
                                    count++;                              
                                 }
                                }
                                
                                for (U8 i = count; i < 6; i++) // до 6-ти параметров
                                  DisplayConfig[inputCount].DisplayParam[i] = 255; // отключить неиспользуемые параметры
                                
                                OS_Use(&SemaRAM);
                                  SAVE_CONFIG(DisplayConfig[inputCount]); // запись новой конфигурации
                                OS_Unuse(&SemaRAM);
                                system_flags.redraw_gui = 1;
                             }                                                                                       
                          }
                          
                          CommandStatus[inputCount] = GET_ERR; 
                          goto  send_message;

          case GET_ERR:   hexbuffer_2_binbuffer(Uart1Buffer, (pInt8U)&interror, data_num, 7); 
                          if(interror & InputConfig[inputCount].ErrorMask)
                          {
                            OS_Use( &SemaRAM); 
                              for(i = 0; i < InputConfig[inputCount].InpNum; i++)  ErrorArray[base + i] = INTERNAL_ERR;
                            OS_Unuse( &SemaRAM);
                            break; // 
                          }                          
                          
                          CommandStatus[inputCount] = GET_DATA;
                          goto  send_message;                                                                         
            
          case GET_DATA:  OS_Use( &SemaRAM); 
                            hexbuffer_2_binbuffer(Uart1Buffer, (pInt8U)&MainArray[base], data_num, 7); 
                            for(i = 0; i < InputConfig[inputCount].InpNum; i++)
                            { 
                              ErrorArray[base + i] = NO_ERR;  // clear old errors
                              LimitsCheck(base + i);          // set new ones...
                            }
                          OS_Unuse( &SemaRAM); 

                          CommandStatus[inputCount] = GET_ERR; // next ask errors
                          break;
                          
        case START_PUMP:  CommandStatus[inputCount] = GET_DATA;
                          goto  send_message; 
                              
        case STOP_PUMP:   CommandStatus[inputCount] = GET_DATA;
                          goto  send_message; 
                              
        case STOP_TIMER:  if (PumpStatus)
                            CommandStatus[inputCount] = START_PUMP; 
                          else
                            CommandStatus[inputCount] = STOP_PUMP;
                          goto  send_message; 
        }
      }
      else
      {
error_calc:
        CLEAR_BIT(USART1->CR1, USART_CR1_RE);
        if(ErrorCount[inputCount] >= MAX_ERROR_INPUT)
        {
          OS_Use( &SemaRAM); 
            for(i = 0; i < InputConfig[inputCount].InpNum; i++)  ErrorArray[base + i] = CONNECT_ERR;
          OS_Unuse( &SemaRAM);          
          CommandStatus[inputCount] = GET_ID;
        }
        else ++ErrorCount[inputCount];
      }
    } // for(; InputCount < MAX_INPUT_COUNT; InputCount++) 
    
    /* рассчет времени до следующего цикла опроса преобразователей */
    timePass = OS_GetTime32() - startLoopTime; 
    if(timePass < MinInputLoopTime)     masterWaitTime = MinInputLoopTime - timePass; 
    else                                masterWaitTime = 0; 
    
    /* вычисляемые каналы и пороги */
    OS_Use( &SemaRAM); 
      CalculationChanels(); 
      CheckThresholds();
    OS_Unuse( &SemaRAM); 
  } // for(;;) end of task
}