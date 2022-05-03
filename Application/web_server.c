#include "defines.h"

// WEB server!
void proceedWEB(SOCKTYPE pOutput, pU8 buffer)
{
  if (WebServerConfig.Disabled) return;

  if (strncmp(buffer, "GET ", 4) != 0) 
  {
    return;
  }
    
  buffer[3] = '_';
  char* c = strchr(buffer, 0x20);
  if ((!c) || (c - &buffer[4] > 24)) return; 
  
  char request[32];
  
  memset(request, 0x00, 32);
  memcpy(request, &buffer[4], c - &buffer[4]);

  if ((strcmp(request, "/") == 0) || (strcmp(request, "index.html") == 0))
  {
    WS_IndexPage(pOutput, buffer);
  }
  else if (strcmp(request, "/?statistic=get") == 0)
  {
    WS_GetStatistic(pOutput, buffer);
  }
  else if (strcmp(request, "/?statistic=reset") == 0)
  {
    WS_ResetStatistic(pOutput);
  }
  else
  {
    _IP_WEBS_SendString(pOutput, "HTTP/1.0 404 Not Found");
    _IP_WEBS_SendString(pOutput, ""); // break before data
  }
}

void WS_IndexPage(SOCKTYPE pOutput,  pU8 buffer)
{
  _IP_WEBS_SendString(pOutput, "HTTP/1.0 200 OK");  
  _IP_WEBS_SendString(pOutput, "Content-Type: text/html");
  _IP_WEBS_SendString(pOutput, "");
  
  _IP_WEBS_SendString(pOutput, "<html>");
  _IP_WEBS_SendString(pOutput, "<head>");
  _IP_WEBS_SendString(pOutput, "<meta charset=\"windows-1251\">");
  if (WebServerConfig.AutoRefreshPeriod != 0)
  {
    sprintf(buffer, "<meta http-equiv=\"Refresh\" content=\"%d\">", WebServerConfig.AutoRefreshPeriod);
    _IP_WEBS_SendString(pOutput, buffer);
  }
    
  sprintf(buffer, "<title>%s</title>", DeviceName);
  _IP_WEBS_SendString(pOutput, buffer);
  
  WS_SendCSS(pOutput);
  
  _IP_WEBS_SendString(pOutput, "</head>");
  _IP_WEBS_SendString(pOutput, "<body>");

  WS_SendHeader(pOutput, buffer);
  
  WS_SendChannels(pOutput, buffer);
  
  WS_SendControl(pOutput, buffer);
  
  WS_SendFooter(pOutput, buffer);
  
  _IP_WEBS_SendString(pOutput, "</body>");
  _IP_WEBS_SendString(pOutput, "</html>");
}

void WS_GetStatistic(SOCKTYPE pOutput, pU8 buffer)
{
  // headers
  _IP_WEBS_SendString(pOutput, "HTTP/1.0 200 OK");  
  _IP_WEBS_SendString(pOutput, "Content-Type: text/plain");
  
  sprintf(buffer, "Content-Disposition: attachment; filename=\"%s [%s].csv\"", DeviceName, SerialNumber);
  _IP_WEBS_SendString(pOutput, buffer);
  
  _IP_WEBS_SendString(pOutput, "Accept-Ranges: bytes");
  _IP_WEBS_SendString(pOutput, "Cache-Control: private");
  _IP_WEBS_SendString(pOutput, "Pragma: private");
  _IP_WEBS_SendString(pOutput, "Expires: Mon, 26 Jul 1997 05:00:00 GMT");
  _IP_WEBS_SendString(pOutput, ""); // break before data
  
  SendStatisticHeaderForWS(pOutput, buffer);
  
  for (int i = 0, count = (WriteStatAddr-SD_STATISTIC_OFFSET) / 0x600; i < count; i++)
  {
    SendStatisticRecordForWS(i, pOutput, buffer);
  }
  _IP_WEBS_SendString(pOutput, ""); // break before data
}

void WS_ResetStatistic(SOCKTYPE pOutput)
{
  ConfigWord    &= ~STATISCTIC_OVERFLOW;
  InvConfigWord  = ~ConfigWord;
  WriteStatAddr  = SD_STATISTIC_OFFSET;
  
  OS_Use(&SemaRAM);
    SAVE_CONFIG(ConfigWord);
    SAVE_CONFIG(InvConfigWord);
  OS_Unuse(&SemaRAM);
  
  _IP_WEBS_SendString(pOutput, "<script>");
  _IP_WEBS_SendString(pOutput, "window.location.replace('/');");
  _IP_WEBS_SendString(pOutput, "</script>");
}

void WS_SendHeader(SOCKTYPE pOutput, pU8 buffer)
{
  _IP_WEBS_SendString(pOutput, "<div class=\"header_block\">");
  _IP_WEBS_SendString(pOutput, "<table class=\"header_table\">");
  _IP_WEBS_SendString(pOutput, "<tbody>");
  _IP_WEBS_SendString(pOutput, "<tr>");
  _IP_WEBS_SendString(pOutput, "<td><a href=\"http://www.eksis.ru\"><img class=\"logo\" src=\"http://www.eksis.ru/bitrix/templates/eksis/images/logo.png\" height=\"86\" width=\"214\" alt=\"ЭКСИС\"></a></td>");
  _IP_WEBS_SendString(pOutput, "<td align=\"center\">");
  sprintf(buffer, "%s [%s]", DeviceName, SerialNumber);
  _IP_WEBS_SendString(pOutput, buffer);
  _IP_WEBS_SendString(pOutput, "<br>");
  int   m, d, h, mi, s, y;
  ParseDate( Time, &s, &mi, &h, &d, &m, &y);
  sprintf(buffer, "Текущее время в приборе: %02d:%02d:%02d %02d.%02d.%4d", h, mi ,s, d, m, y);
  _IP_WEBS_SendString(pOutput, buffer);
  _IP_WEBS_SendString(pOutput, "</td>");
  _IP_WEBS_SendString(pOutput, "</tr>");
  _IP_WEBS_SendString(pOutput, "</tbody>");
  _IP_WEBS_SendString(pOutput, "</table>");
  _IP_WEBS_SendString(pOutput, "</div>");
}

void WS_SendChannels(SOCKTYPE pOutput, pU8 buffer)
{
  Int8U current;
  Int8U count = GetDisplayChannelsCount();
  
  _IP_WEBS_SendString(pOutput, "<table cellpadding=\"10\" cellspacing=\"10\" align=\"center\">");
  _IP_WEBS_SendString(pOutput, "<tr>");
  for (int i = 0; i < count; i++)
  {
    if (i % 4 == 0)
    {
      _IP_WEBS_SendString(pOutput, "<tr>");
      current = i / 4;
    }
    
    _IP_WEBS_SendString(pOutput, "<td>");
      WS_SendChannel(pOutput, i, buffer);
    _IP_WEBS_SendString(pOutput, "</td>");
    
    if ((i / 4) != current) _IP_WEBS_SendString(pOutput, "<tr>");
  }
  _IP_WEBS_SendString(pOutput, "</tr>");
  _IP_WEBS_SendString(pOutput, "</table>");
}

void WS_SendChannel(SOCKTYPE pOutput, Int8U display_channel_index, pU8 buffer)
{
  Int8U count = GetDisplayChannelParamsCount(display_channel_index);

  _IP_WEBS_SendString(pOutput, "<table class=\"bl_chan\" width=\"100%%\" border=\"0\" cellpadding=\"0\" cellspacing=\"0\" align=\"center\">");
  _IP_WEBS_SendString(pOutput, "<tbody>");
  _IP_WEBS_SendString(pOutput, "<tr class=\"row\">");
  _IP_WEBS_SendString(pOutput, "<td class=\"col top\">");
  sprintf(buffer, "<div class=\"circle_channel num_chan\">%d</div>", display_channel_index+1);
  _IP_WEBS_SendString(pOutput, buffer);
  _IP_WEBS_SendString(pOutput, "</td>");
  _IP_WEBS_SendString(pOutput, "<td class=\"col\">");
  _IP_WEBS_SendString(pOutput, "<table>");
  _IP_WEBS_SendString(pOutput, "<tbody>");

  for (int i = 0; i < count; i++)
  {
    WS_SendParameter(pOutput, display_channel_index, i, buffer);
  }

  _IP_WEBS_SendString(pOutput, "</tbody>");
  _IP_WEBS_SendString(pOutput, "</table>");
  _IP_WEBS_SendString(pOutput, "</td>");
  _IP_WEBS_SendString(pOutput, "</tr>");
  _IP_WEBS_SendString(pOutput, "</tbody>");
  _IP_WEBS_SendString(pOutput, "</table>");
}

void WS_SendParameter(SOCKTYPE pOutput, Int8U display_channel_index, Int8U dispay_param_index, pU8 buffer)
{
  char param_name[32];
  char param_value[32];
  char param_units[32];
  Int8U red;
  
  GetParamName(display_channel_index, dispay_param_index, param_name);  
  GetParamValueForWS(display_channel_index, dispay_param_index, param_value, &red);
  GetParamUnitsForWS(display_channel_index, dispay_param_index, param_units);

  _IP_WEBS_SendString(pOutput, "<tr class=\"row\">");
  _IP_WEBS_SendString(pOutput, "<td class=\"col\">");
  sprintf(buffer, "<div class=\"param_name\">%s</div>", param_name);
  _IP_WEBS_SendString(pOutput, buffer);
  _IP_WEBS_SendString(pOutput, "</td>");
  _IP_WEBS_SendString(pOutput, "<td class=\"col less\">");
  if (red)
    sprintf(buffer, "<div class=\"circle_value_red\">%s</div>", param_value);
  else
    sprintf(buffer, "<div class=\"circle_value\">%s</div>", param_value);
  
  _IP_WEBS_SendString(pOutput, buffer);
  _IP_WEBS_SendString(pOutput, "</td>");
  _IP_WEBS_SendString(pOutput, "<td class=\"col less\">");
  sprintf(buffer, "<div class=\"circle_units\">%s</div>", param_units);
  _IP_WEBS_SendString(pOutput, buffer);
  _IP_WEBS_SendString(pOutput, "</td>");
  _IP_WEBS_SendString(pOutput, "</tr>");
}

void WS_SendControl(SOCKTYPE pOutput, pU8 buffer)
{
  Int8U count = GetOutChannelsCount();
  
  _IP_WEBS_SendString(pOutput, "<table class=\"header_table\" width=\"100%%\" border=\"0\" cellpadding=\"0\" cellspacing=\"0\" align=\"center\">");
  _IP_WEBS_SendString(pOutput, "<tbody>");
  _IP_WEBS_SendString(pOutput, "<tr class=\"row\">");
  _IP_WEBS_SendString(pOutput, "<td class=\"col\">");
  _IP_WEBS_SendString(pOutput, "<table class=\"control_block\" border=\"0\" cellpadding=\"0\" cellspacing=\"0\" align=\"center\">");
  _IP_WEBS_SendString(pOutput, "<tbody>");
  _IP_WEBS_SendString(pOutput, "<tr class=\"row\">");
  
  for (int i = 0; i < count; i++)
  {
    if (OutConfig[i].ChanelType == RELAY_TYPE)
    {
      _IP_WEBS_SendString(pOutput, "<td class=\"col\">");
      if ((RelayOuts >> i) & 1)
        sprintf(buffer, "<a title=\"Состояние %d реле: ЗАМКНУТО\"><div class=\"circle_green\">%d</div></a>", i+1, i+1);
      else
        sprintf(buffer, "<a title=\"Состояние %d реле: РАЗОМКНУТО\"><div class=\"circle_red\">%d</div></a>", i+1, i+1);
      _IP_WEBS_SendString(pOutput, buffer);
      _IP_WEBS_SendString(pOutput, "</td>");
    }
    
    if ((OutConfig[i].ChanelType >= ANALOG_TYPE_05) && (OutConfig[i].ChanelType <= ANALOG_TYPE_420))
    {
      _IP_WEBS_SendString(pOutput, "<td class=\"col\">");
      sprintf(buffer, "<a title=\"Сила тока (мА) %d аналогового выхода\"><div class=\"circle_blue\">%.1f</div></a>", i+1, OutCurrents[i]);
      _IP_WEBS_SendString(pOutput, buffer);
      _IP_WEBS_SendString(pOutput, "</td>");
    }

  }
  _IP_WEBS_SendString(pOutput, "</tr>");
  _IP_WEBS_SendString(pOutput, "</tbody>");
  _IP_WEBS_SendString(pOutput, "</table>");
  _IP_WEBS_SendString(pOutput, "</td>");
  _IP_WEBS_SendString(pOutput, "</tr>");
  _IP_WEBS_SendString(pOutput, "</tbody>");
  _IP_WEBS_SendString(pOutput, "</table>");
}

void WS_SendFooter(SOCKTYPE pOutput, pU8 buffer)
{
  _IP_WEBS_SendString(pOutput, "<table class=\"footer_table\" align=\"center\">");
  _IP_WEBS_SendString(pOutput, "<tbody>");
  _IP_WEBS_SendString(pOutput, "<tr>");
  _IP_WEBS_SendString(pOutput, "<td>");
  _IP_WEBS_SendString(pOutput, "<div class=\"stat_block\">");
  _IP_WEBS_SendString(pOutput, "<table>");
  _IP_WEBS_SendString(pOutput, "<tbody>");
  _IP_WEBS_SendString(pOutput, "<tr>");
  sprintf(buffer, "<td align=\"center\" colspan=\"2\"><input type=\"button\" class=\"download_stat_button\" value=\"Загрузить статистику (записей: %d)\" onclick=\"window.open('?statistic=get');\"/></td>", (WriteStatAddr-SD_STATISTIC_OFFSET)/0x600);
  _IP_WEBS_SendString(pOutput, buffer);
  _IP_WEBS_SendString(pOutput, "</tr>");
  _IP_WEBS_SendString(pOutput, "<tr>");
  if (!WebServerConfig.HideStatisticReset) _IP_WEBS_SendString(pOutput, "<td align=\"center\" colspan=\"2\"><input type=\"button\" class=\"reset_stat_button\" value=\"Сбросить статистику\" onclick=\"if (confirm('Вы уверены, что хотите сбросить статистику?')) {window.location.replace('?statistic=reset');}\"/></td>");
  _IP_WEBS_SendString(pOutput, "</tr>");
  _IP_WEBS_SendString(pOutput, "</tbody>");
  _IP_WEBS_SendString(pOutput, "</table>");
  _IP_WEBS_SendString(pOutput, "</div>");
  _IP_WEBS_SendString(pOutput, "</td>");
  _IP_WEBS_SendString(pOutput, "</tr>");
  _IP_WEBS_SendString(pOutput, "</tbody>");
  _IP_WEBS_SendString(pOutput, "</table>");
}

void SendStatisticHeaderForWS(SOCKTYPE pOutput, pU8 buffer)
{
  U32     channels_count = GetDisplayChannelsCount();
  U32     parameters_count;
  char    buffer_1[64];
  
  sprintf(buffer, "Дата/Время;");
  send_sock_cyclone(pOutput, buffer);
      
  for (U32 i = 0; i < channels_count; i++)
  {
    parameters_count = GetDisplayChannelParamsCount(i);    
    for (U32 j = 0 ; j < parameters_count; j++)
    {
      GetParamTypeAndUnits(i, j, buffer_1);
      sprintf(buffer, "[К%.2d]%s;", i + 1, buffer_1);
        strdel_(buffer, '^');
          strdel_(buffer, 'v');
      send_sock_cyclone(pOutput, buffer);
    }
  }
  _IP_WEBS_SendString(pOutput, "");
}

void SendStatisticRecordForWS(int record_index, SOCKTYPE pOutput, pU8 buffer)
{
  U32     index, decimal;
  U32     parameters_count;
  U32     channels_count = GetDisplayChannelsCount();
  int     m, d, h, mi, s,  y;
  char    st[16];
  
  errors_flags.sd_fail |= read_SDCard(buffer, SD_STATISTIC_OFFSET + record_index * 0x600 + 0x400, 0x200);
  
  ParseDate( *(Int32U*)&buffer[0x564-0x400], &s, &mi, &h, &d, &m, &y);
  sprintf(buffer, "%.2d.%.2d.%.4d %.2d:%.2d:%.2d;", d, m, y, h, mi, s);
  send_sock_cyclone(pOutput, buffer);
  
  errors_flags.sd_fail |= read_SDCard(buffer, SD_STATISTIC_OFFSET + record_index * 0x600, 0x400);  
  
  for (int i = 0; i < channels_count; i++)
  {
    parameters_count = GetDisplayChannelParamsCount(i);    
    for (int j = 0 ; j < parameters_count; j++)
    {
      index   = DisplayConfig[i].DisplayParam[j];
      decimal = MeasureAttribute[index].DecPoint;
      
      sprintf(st, "%.*f", decimal, *(pF32)&buffer[index * 4]);
      
      char* c;
      c = strchr(st, '.');
      
      while (c)
      {
        *c = ',';
         c = strchr(st, '.'); // заменить . на ,
      }      
      sprintf(st, "%s;", st);
      send_sock_cyclone(pOutput, st);
    }
  }
  _IP_WEBS_SendString(pOutput, "");
}

void WS_SendCSS(SOCKTYPE pOutput)
{
  _IP_WEBS_SendString(pOutput, "<style type=\"text/css\">");
  
  _IP_WEBS_SendString(pOutput, "body {");
  _IP_WEBS_SendString(pOutput, "min-width: 600px");
  _IP_WEBS_SendString(pOutput, "font-family: Geneva, Arial, Helvetica, sans-serif;");
  _IP_WEBS_SendString(pOutput, "font-style: normal;");
  _IP_WEBS_SendString(pOutput, "}");
  
  _IP_WEBS_SendString(pOutput, ".header_table{");
  _IP_WEBS_SendString(pOutput, "width: 100%;");
  _IP_WEBS_SendString(pOutput, "}");
  
  _IP_WEBS_SendString(pOutput, ".header_block{");
  _IP_WEBS_SendString(pOutput, "background-color: #f2f2f2;");
  _IP_WEBS_SendString(pOutput, "color:#414141;");
  _IP_WEBS_SendString(pOutput, "margin: 0px;");
  _IP_WEBS_SendString(pOutput, "border-radius: 24px;");
  _IP_WEBS_SendString(pOutput, "border: 0px solid #d6d6d6;");
  _IP_WEBS_SendString(pOutput, "box-shadow: 0 0 15px grey;");
  _IP_WEBS_SendString(pOutput, "}");

  _IP_WEBS_SendString(pOutput, ".footer_block{");
  _IP_WEBS_SendString(pOutput, "background-color: #f2f2f2;");
  _IP_WEBS_SendString(pOutput, "color:#414141;");
  _IP_WEBS_SendString(pOutput, "border-radius: 24px;");
  _IP_WEBS_SendString(pOutput, "border: 0px solid #d6d6d6;");
  _IP_WEBS_SendString(pOutput, "box-shadow: 0 0 15px grey;");
  _IP_WEBS_SendString(pOutput, "}");
  
  _IP_WEBS_SendString(pOutput, ".logo {");
  _IP_WEBS_SendString(pOutput, "padding: 20px 20px 20px 20px;");
  _IP_WEBS_SendString(pOutput, "display: block;");
  _IP_WEBS_SendString(pOutput, "float: left;");
  _IP_WEBS_SendString(pOutput, "}");
  
  _IP_WEBS_SendString(pOutput, ".less{");
  _IP_WEBS_SendString(pOutput, "margin: 0px;");
  _IP_WEBS_SendString(pOutput, "padding: 0px;");
  _IP_WEBS_SendString(pOutput, "}");
  
  _IP_WEBS_SendString(pOutput, ".col{");
  _IP_WEBS_SendString(pOutput, "margin: 0px;");
  _IP_WEBS_SendString(pOutput, "padding: 0px;");
  _IP_WEBS_SendString(pOutput, "}");
  
  _IP_WEBS_SendString(pOutput, ".top {");
  _IP_WEBS_SendString(pOutput, "padding-top: 4px;");
  _IP_WEBS_SendString(pOutput, "vertical-align: top;");
  _IP_WEBS_SendString(pOutput, "}");

  _IP_WEBS_SendString(pOutput, ".circle_channel, .circle_value, .circle_value_red, .circle_units, .circle_green, .circle_red, .circle_blue  {");
  _IP_WEBS_SendString(pOutput, "vertical-align: center;");
  _IP_WEBS_SendString(pOutput, "text-align: center;");
  _IP_WEBS_SendString(pOutput, "width: 56px;");
  _IP_WEBS_SendString(pOutput, "height: 56px;");
  _IP_WEBS_SendString(pOutput, "border-radius: 39px;");
  _IP_WEBS_SendString(pOutput, "line-height: 56px;");
  _IP_WEBS_SendString(pOutput, "text-align: center;");
  _IP_WEBS_SendString(pOutput, "font-size: 16px;");
  _IP_WEBS_SendString(pOutput, "}");
  
  _IP_WEBS_SendString(pOutput, ".circle_channel {");
  _IP_WEBS_SendString(pOutput, "background: #04859D;");
  _IP_WEBS_SendString(pOutput, "color: #FFFFFF;");
  _IP_WEBS_SendString(pOutput, "}");
  
  _IP_WEBS_SendString(pOutput, ".circle_value {");
  _IP_WEBS_SendString(pOutput, "background: #FFFFFF;");
  _IP_WEBS_SendString(pOutput, "}");
  
  _IP_WEBS_SendString(pOutput, ".circle_value_red {");
  _IP_WEBS_SendString(pOutput, "background: #FFFFFF;");
  _IP_WEBS_SendString(pOutput, "color: #FF0000;");
  _IP_WEBS_SendString(pOutput, "}");
  
  _IP_WEBS_SendString(pOutput, ".circle_units {");
  _IP_WEBS_SendString(pOutput, "background: #71AD2B;");
  _IP_WEBS_SendString(pOutput, "color: #FFFFFF;");
  _IP_WEBS_SendString(pOutput, "}");
  
  _IP_WEBS_SendString(pOutput, ".circle_red {");
  _IP_WEBS_SendString(pOutput, "background: #FF9773;");
  _IP_WEBS_SendString(pOutput, "}");
  
  _IP_WEBS_SendString(pOutput, ".circle_blue {");
  _IP_WEBS_SendString(pOutput, "background: #04859D;");
  _IP_WEBS_SendString(pOutput, "color: white;");
  _IP_WEBS_SendString(pOutput, "}");
  
  _IP_WEBS_SendString(pOutput, ".circle_green {");
  _IP_WEBS_SendString(pOutput, "background: #71AD2B;");
  _IP_WEBS_SendString(pOutput, "}");
  
  _IP_WEBS_SendString(pOutput, ".param_name {");
  _IP_WEBS_SendString(pOutput, "text-align: right;");
  _IP_WEBS_SendString(pOutput, "height: 80px;");
  _IP_WEBS_SendString(pOutput, "line-height: 80px;");
  _IP_WEBS_SendString(pOutput, "vertical-align: center;");
  _IP_WEBS_SendString(pOutput, "text-align: left;");
  _IP_WEBS_SendString(pOutput, "font-size: 16px;");
  _IP_WEBS_SendString(pOutput, "border: 0px solid #d6d6d6;");
  _IP_WEBS_SendString(pOutput, "color: #414141;");
  _IP_WEBS_SendString(pOutput, "margin-right: 20px;");
  _IP_WEBS_SendString(pOutput, "}");

  _IP_WEBS_SendString(pOutput, ".measure_caption {");
  _IP_WEBS_SendString(pOutput, "text-align: center;");
  _IP_WEBS_SendString(pOutput, "font-size: 22px;");
  _IP_WEBS_SendString(pOutput, "}");
  
  _IP_WEBS_SendString(pOutput, ".control_caption {");
  _IP_WEBS_SendString(pOutput, "text-align: center;");
  _IP_WEBS_SendString(pOutput, "font-size: 22px;");
  _IP_WEBS_SendString(pOutput, "}");
  
  _IP_WEBS_SendString(pOutput, ".bl_chan{");
  _IP_WEBS_SendString(pOutput, "background-color: #f2f2f2;");
  _IP_WEBS_SendString(pOutput, "color:#000000;");
  _IP_WEBS_SendString(pOutput, "border-radius: 24px;");
  _IP_WEBS_SendString(pOutput, "border: 0px solid #d6d6d6;");
  _IP_WEBS_SendString(pOutput, "box-shadow: 0 0 15px grey;");
  _IP_WEBS_SendString(pOutput, "}");

  _IP_WEBS_SendString(pOutput, ".num_chan {");
  _IP_WEBS_SendString(pOutput, "font-size: 40px;");
  _IP_WEBS_SendString(pOutput, "display: block;");
  _IP_WEBS_SendString(pOutput, "}");
  
  _IP_WEBS_SendString(pOutput, ".stat_block {");
  _IP_WEBS_SendString(pOutput, "height: 130px;");
  _IP_WEBS_SendString(pOutput, "background-color: #FFFFFF;");
  _IP_WEBS_SendString(pOutput, "border-radius: 24px;");
  _IP_WEBS_SendString(pOutput, "border: 1px solid #d6d6d6;");
  _IP_WEBS_SendString(pOutput, "}");

  _IP_WEBS_SendString(pOutput, ".input{");
  _IP_WEBS_SendString(pOutput, "height: 40px;");
  _IP_WEBS_SendString(pOutput, "border: 1px solid #d6d6d6;");
  _IP_WEBS_SendString(pOutput, "border-top-left-radius: 24px;");
  _IP_WEBS_SendString(pOutput, "border-top-right-radius: 24px;");
  _IP_WEBS_SendString(pOutput, "border-bottom-right-radius: 24px;");
  _IP_WEBS_SendString(pOutput, "border-bottom-left-radius: 24px;");
  _IP_WEBS_SendString(pOutput, "margin: 10px;");
  _IP_WEBS_SendString(pOutput, "text-align: center;");
  _IP_WEBS_SendString(pOutput, "font-size:22px;");
  _IP_WEBS_SendString(pOutput, "}");

  _IP_WEBS_SendString(pOutput, ".download_stat_button{");
  _IP_WEBS_SendString(pOutput, "text-align: center;");
  _IP_WEBS_SendString(pOutput, "display: inline-block;");
  _IP_WEBS_SendString(pOutput, "width: auto;");
  _IP_WEBS_SendString(pOutput, "background-color: #FFFFFF;");
  _IP_WEBS_SendString(pOutput, "border-top-left-radius: 24px;");
  _IP_WEBS_SendString(pOutput, "border-top-right-radius: 24px;");
  _IP_WEBS_SendString(pOutput, "border-bottom-right-radius: 24px;");
  _IP_WEBS_SendString(pOutput, "border-bottom-left-radius: 24px;");
  _IP_WEBS_SendString(pOutput, "padding: 5px 20px;");
  _IP_WEBS_SendString(pOutput, "border: 3px solid #175e8c;");
  _IP_WEBS_SendString(pOutput, "color:#175e8c;");
  _IP_WEBS_SendString(pOutput, "font-size:22px;");
  _IP_WEBS_SendString(pOutput, "text-align:center;");
  _IP_WEBS_SendString(pOutput, "text-decoration:none;");
  _IP_WEBS_SendString(pOutput, "font-weight:bold;");
  _IP_WEBS_SendString(pOutput, "cursor: pointer;");
  _IP_WEBS_SendString(pOutput, "}");

  _IP_WEBS_SendString(pOutput, "input.download_stat_button[type=\"button\"]:hover {");
  _IP_WEBS_SendString(pOutput, "text-decoration: none;");
  _IP_WEBS_SendString(pOutput, "color: #FFFFFF;");
  _IP_WEBS_SendString(pOutput, "background-color: #175e8c;");
  _IP_WEBS_SendString(pOutput, "}");

  _IP_WEBS_SendString(pOutput, ".reset_stat_button{");
  _IP_WEBS_SendString(pOutput, "text-align: center;");
  _IP_WEBS_SendString(pOutput, "display: inline-block;");
  _IP_WEBS_SendString(pOutput, "width: auto;");
  _IP_WEBS_SendString(pOutput, "background-color: #FFFFFF;");
  _IP_WEBS_SendString(pOutput, "border-top-left-radius: 24px;");
  _IP_WEBS_SendString(pOutput, "border-top-right-radius: 24px;");
  _IP_WEBS_SendString(pOutput, "border-bottom-right-radius: 24px;");
  _IP_WEBS_SendString(pOutput, "border-bottom-left-radius: 24px;");
  _IP_WEBS_SendString(pOutput, "padding: 5px 20px;");
  _IP_WEBS_SendString(pOutput, "border: 3px solid #FF0000;");
  _IP_WEBS_SendString(pOutput, "color:#FF0000;");
  _IP_WEBS_SendString(pOutput, "font-size:22px;");
  _IP_WEBS_SendString(pOutput, "text-align:center;");
  _IP_WEBS_SendString(pOutput, "text-decoration:none;");
  _IP_WEBS_SendString(pOutput, "font-weight:bold;");
  _IP_WEBS_SendString(pOutput, "cursor: pointer;");
  _IP_WEBS_SendString(pOutput, "}");

  _IP_WEBS_SendString(pOutput, "input.reset_stat_button[type=\"button\"]:hover {");
  _IP_WEBS_SendString(pOutput, "text-decoration: none;");
  _IP_WEBS_SendString(pOutput, "color: #FFFFFF;");
  _IP_WEBS_SendString(pOutput, "background-color: #FF0000;");
  _IP_WEBS_SendString(pOutput, "}");

  _IP_WEBS_SendString(pOutput, ".control_block{");
  _IP_WEBS_SendString(pOutput, "background-color: #f2f2f2;");
  _IP_WEBS_SendString(pOutput, "color:#414141;");
  _IP_WEBS_SendString(pOutput, "border-radius: 24px;");
  _IP_WEBS_SendString(pOutput, "border: 0px solid #d6d6d6;");
  _IP_WEBS_SendString(pOutput, "box-shadow: 0 0 15px grey;");
  _IP_WEBS_SendString(pOutput, "}");
  
  _IP_WEBS_SendString(pOutput, "</style>");
}
