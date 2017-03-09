//JSONParser.cpp
//Author: Bombo
//09.03.2017
//Класс JSONParser производит проверку и обработку запросов в формате JSON

#include"JSONParser.h"

//************************************JSONParser*********************************************//
JSONParser::JSONParser()
{
  pcJSONRequest = NULL;
  stStartPointer = -1;
  stEndPointer = -1;
  pjfFirst = NULL;
}

JSONParser::JSONParser(char * const in_pcJSONRequest)
{
  size_t stLen;

  if(in_pcJSONRequest == NULL || (stLen = strlen(in_pcJSONRequest)) == 0)
  {
    pcJSONRequest = NULL;
    return;
  }

  pcJSONRequest = new char[stLen+1];
  memset(pcJSONRequest, 0, stLen + 1);
  strncpy(pcJSONRequest, in_pcJSONRequest, stLen);

  stStartPointer = 0;
  stEndPointer = stLen-1;

  pjfFirst = NULL;
}

JSONParser::~JSONParser()
{
  JSONField *pjfList;

  if(pcJSONRequest != NULL)
  {
    delete [] pcJSONRequest;
  }

  pjfList = pjfFirst;
  while(pjfList != NULL)
  {
    pjfList = pjfFirst->pjfNext;
    delete pjfFirst;
    pjfFirst = pjfList;
  }
}

bool JSONParser::CheckRequest(void)
{
  int nIndex;
  long lBraceCounter, lBracketCounter, lQuotesCounter;

  //    pcJSONRequest = new char[128]; //отладка!!!
  //    memset(pcJSONRequest, 0, 128); //отладка!!!
  //    strncpy(pcJSONRequest, "[{\"\"}\"]\"]", 127); //отладка!!!
  //    fprintf(stderr, "JSONParser::CheckRequest() : Start debug:\n%s\n", pcJSONRequest); //отладка!!!
  //    stStartPointer = 0; //отладка!!!
  //    stEndPointer = (long) strlen((char *)pcJSONRequest); //отладка!!!

  if(pcJSONRequest == NULL || strlen(pcJSONRequest) == 0 || stStartPointer < 0 || stEndPointer < 0)
    return false;

  lBraceCounter = lBracketCounter = lQuotesCounter = 0;
  for(nIndex = stStartPointer; nIndex < stEndPointer; ++nIndex)
  {
    switch(pcJSONRequest[nIndex])
    {
      case '[':
      {
        if(lQuotesCounter == 0)
          lBracketCounter++;
        break;
      }
      case ']':
      {
        if(lQuotesCounter == 0)
          lBracketCounter--;
        break;
      }
      case '\"':
      {
        if(nIndex > 0 && pcJSONRequest[nIndex-1] != '\\')
          lQuotesCounter ^= 1;
        break;
      }
      case '{':
      {
        if(lQuotesCounter == 0)
          lBraceCounter++;
        break;
      }
      case '}':
      {
        if(lQuotesCounter == 0)
          lBraceCounter--;
        break;
      }
    }
  }
  if(lQuotesCounter == 0 && lBraceCounter == 0 && lBracketCounter == 0)
    return true;

  fprintf(stderr, "JSONParser::CheckRequest() : Wrong JSON format!!!\n%s\n", pcJSONRequest);
  return false;
}

void JSONParser::InitFields(void)
{
  if(!CheckRequest())
    return;

  //получаем имена и значения полей для текущей записи в JSON-запросе
  //для этого анализируем запись между stStartPointer и stEndPointer
  //...
}

//метод возвращает true, если ещё остались необработанные поля
bool JSONParser::ParseUndefined(void)
{
  //получаем первую запись из списка полей
  //...

  //обрабатываем запись в зависимости от значения её имени
  //...

  //если в списке есть ещё поля - возвращаем true
  //...

  return false;
}

//метод обрабатывает все записи в запросе (при помощи обработчика полей каждой записи, ParseUndefined())
void JSONParser::ParseAllItems(void)
{
  return;
}

//*************************************JSONField*********************************************//

JSONField::JSONField(void)
{
  pcName = NULL;
  pcValue = NULL;
  pjfNext = NULL;
}

JSONField::JSONField(char * const in_pcName, char * const in_pcValue)
{
  size_t stLen;

  if(in_pcName == NULL || (stLen = strlen(in_pcName)) == 0)
  {
    pcName = NULL;
    pcValue = NULL;
    pjfNext = NULL;
    return;
  }
  pcName = new char[stLen + 1];
  memset(pcName, 0, stLen + 1);
  strncpy(pcName, in_pcName, stLen);

  if(in_pcValue == NULL || (stLen = strlen(in_pcValue)) == 0)
  {
    pcValue = new char[2];
    memset(pcValue, 0, 2);
  }
  else
  {
    pcValue = new char[stLen + 1];
    memset(pcValue, 0, stLen + 1);
    strncpy(pcValue, in_pcValue, stLen);
  }

  pjfNext = NULL;
}

JSONField::~JSONField()
{
  if(pcName != NULL)
    delete [] pcName;
  if(pcValue != NULL)
    delete [] pcValue;
}
