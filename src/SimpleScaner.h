//SimpleScaner.h
//Author: Bombo
//14.03.2017
//Сканер (лексический анализатор)

#pragma once

#include<fcntl.h>
#include<stdio.h>
#include<string.h>
#include<unistd.h>
#include<sys/stat.h>
#include<sys/types.h>

//ED end
//ER error
//UD undefined
//LT letter
//DG digit
//LD letter-digit (scan only)
//DL delimiter
//QU quotes
//SQ single quote
//SL slash
//BL blank
enum CharacterType {ED = -2, ER = -1, UD = 0, LT, DG, LD, DL, QU, SQ, SL, BL};

const CharacterType pBuiltInCharTable[256] = {
//      0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F
/* 0*/ ED,UD,UD,UD,UD,UD,UD,UD,UD,BL,BL,UD,UD,BL,UD,UD,
/* 1*/ UD,UD,UD,UD,UD,UD,UD,UD,UD,UD,UD,UD,UD,UD,UD,UD,
/* 2*/ BL,DL,QU,UD,UD,DL,DL,SQ,DL,DL,DL,DL,DL,DL,DL,SL,
/* 3*/ DG,DG,DG,DG,DG,DG,DG,DG,DG,DG,DL,DL,DL,DL,DL,DL,
/* 4*/ UD,LT,LT,LT,LT,LT,LT,LT,LT,LT,LT,LT,LT,LT,LT,LT,
/* 5*/ LT,LT,LT,LT,LT,LT,LT,LT,LT,LT,LT,DL,UD,DL,UD,LT,
/* 6*/ UD,LT,LT,LT,LT,LT,LT,LT,LT,LT,LT,LT,LT,LT,LT,LT,
/* 7*/ LT,LT,LT,LT,LT,LT,LT,LT,LT,LT,LT,DL,DL,DL,UD,UD,

/* 8*/ UD,UD,UD,UD,UD,UD,UD,UD,UD,UD,UD,UD,UD,UD,UD,UD,
/* 9*/ UD,UD,UD,UD,UD,UD,UD,UD,UD,UD,UD,UD,UD,UD,UD,UD,
/* A*/ UD,UD,UD,UD,UD,UD,UD,UD,UD,UD,UD,UD,UD,UD,UD,UD,
/* B*/ UD,UD,UD,UD,UD,UD,UD,UD,UD,UD,UD,UD,UD,UD,UD,UD,
/* C*/ UD,UD,UD,UD,UD,UD,UD,UD,UD,UD,UD,UD,UD,UD,UD,UD,
/* D*/ UD,UD,UD,UD,UD,UD,UD,UD,UD,UD,UD,UD,UD,UD,UD,UD,
/* E*/ UD,UD,UD,UD,UD,UD,UD,UD,UD,UD,UD,UD,UD,UD,UD,UD,
/* F*/ UD,UD,UD,UD,UD,UD,UD,UD,UD,UD,UD,UD,UD,UD,UD,UD
};

//scan result structure for syntax analyser
struct ScanResult
{
    char *pszName; //result name
    char *pbValue; //result value
    long lSize; //result length

    ScanResult();
    ScanResult(char const * const in_pszName);
    ScanResult(const ScanResult &in_srResult);
    ScanResult(const ScanResult *in_psrResult);
    ScanResult(char const * const in_pszName, char const * const in_pbValue, long in_lSize);
    ~ScanResult();

    ScanResult& operator =(const ScanResult& in_srResult);
    ScanResult& operator =(const ScanResult *in_psrResult);

    void AddName(char const * const in_pszName);
    void AddValue(char const * const in_pbValue, long in_lSize);

    bool NameCmp(char const * const in_pszName);

    void Print(void);
};

//auxiliary structures (SingleSymbol, SymbolsList) for string creation
struct SingleSymbol
{
    char chSymbol; //single symbol value from generated string
    CharacterType ctType; //a type of this symbol

    SingleSymbol *psNext;

    SingleSymbol();
    SingleSymbol(char in_chSymbol);
    SingleSymbol(char in_chSymbol, CharacterType in_ctType);
};

//the list of all known symbols from grammar
struct SymbolsList
{
    SingleSymbol *psFirst; //generated string

    SymbolsList();
    SymbolsList(char in_chSymbol);
    SymbolsList(char in_chSymbol, CharacterType in_ctType);
    ~SymbolsList();

    void AddSymbol(char in_chSymbol);
    void AddSymbol(char in_chSymbol, CharacterType in_ctType);
    void AddSymbol(SingleSymbol const * const in_psSymbol);
    void Clean(void); //delete all current list
    char *GetString(void);
    void SetResultValue(ScanResult * const out_psrResult);
};

class SimpleScaner
{
protected:
    char *pszFile; //path to file with code
    int fdFile; //file descriptor

    char *pszBuff; //some buffer with code
    long lIndex; //current position in buffer
    long lBuffLen; //buffer length

    CharacterType const * pCharTable; //character table by default
    CharacterType *pAltCharTable; //alternative char table (terminal symbols, 16x16=256 massive)

public:
    //the last scan result
    ScanResult *psrLastScanResult;

public:
    SimpleScaner();
    ~SimpleScaner();

    //open file with some code
    bool OpenFile(char const * const in_pszFile);
    //read buffer with some code
    bool ReadBuff(char const * const in_pszBuff);

    //read alternative char table from file
    bool ReadNewTable(char const * const in_szPathToTable);

    //the main method of this class
    virtual ScanResult *Scan(void);
    //real scan
    bool NextScan(ScanResult **out_psrResult);
    //scan subfunction
    CharacterType GetCharType(bool in_fIncrementIndex);
    void GetCharType(SingleSymbol * const out_psSymbol, bool in_fIncrementIndex);

    //increment current position
    void IncrementPosition(void);
    void DecrementPosition(void);

    //debug scan
    void MakeScan(void);

    //print buffer
    void PrintBuff(void);
};
