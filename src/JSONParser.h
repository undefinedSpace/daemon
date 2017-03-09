//JSONParser.h
//Author: Bombo
//09.03.2017
//Класс JSONParser производит проверку и обработку запросов в формате JSON

#pragma once

#include<stdio.h>
#include<string.h>

//поля текущей (обрабатываемой в данный момент) записи
struct JSONField
{
    char *pcName;
    char *pcValue;

    JSONField *pjfNext;

    JSONField(void);
    JSONField(char * const in_pcName, char * const in_pcValue);
    ~JSONField();
};

class JSONParser
{
    char *pcJSONRequest; //ссылка на массив с запросом в формате JSON
    long stStartPointer; //индекс начального элемента обрабатываемой области запроса
    long stEndPointer; //индекс конечного элемента обрабатываемой области запроса

    JSONField *pjfFirst; //массив полей текущей записи

public:
    JSONParser();
    JSONParser(char * const in_pcJSONRequest);
    ~JSONParser();

    //метод проверяет соответствие формату JSON
    bool CheckRequest(void);
    //метод получает список полей и их значения
    void InitFields(void);
    //метод обрабатывает поля запроса
    //возвращает true, если остались необработанные поля, и false, если пахать больше нечего.
    //имеет смысл в будущем сделать этот метод абстрактным и наследовать класс JSONParser (!)
    bool ParseUndefined(void);
    //выделение и обработка всех записей запроса
    void ParseAllItems(void);
};
