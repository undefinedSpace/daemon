//SimpleGrammar.h
//Author: Bombo
//20.03.2017
//Конструктор внутреннего представления грамматик

#pragma once

#include"SimpleScaner.h"

const CharacterType pGrammTable[256] = {
//      0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F
/* 0*/ UD,UD,UD,UD,UD,UD,UD,UD,UD,BL,DL,UD,UD,BL,UD,UD,
/* 1*/ UD,UD,UD,UD,UD,UD,UD,UD,UD,UD,UD,UD,UD,UD,UD,UD,
/* 2*/ BL,UD,DL,DL,UD,UD,UD,DL,DL,DL,DL,DL,DL,DL,DL,SL,
/* 3*/ DG,DG,DG,DG,DG,DG,DG,DG,DG,DG,DL,DL,DL,DL,DL,UD,
/* 4*/ UD,LT,LT,LT,LT,LT,LT,LT,LT,LT,LT,LT,LT,LT,LT,LT,
/* 5*/ LT,LT,LT,LT,LT,LT,LT,LT,LT,LT,UD,DL,UD,DL,UD,LT,
/* 6*/ UD,LT,LT,LT,LT,LT,LT,LT,LT,LT,LT,LT,LT,LT,LT,LT,
/* 7*/ LT,LT,LT,LT,LT,LT,LT,LT,LT,LT,UD,DL,DL,DL,UD,UD,

/* 8*/ UD,UD,UD,UD,UD,UD,UD,UD,UD,UD,UD,UD,UD,UD,UD,UD,
/* 9*/ UD,UD,UD,UD,UD,UD,UD,UD,UD,UD,UD,UD,UD,UD,UD,UD,
/* A*/ UD,UD,UD,UD,UD,UD,UD,UD,LT,UD,UD,UD,UD,UD,UD,UD,
/* B*/ UD,UD,UD,UD,UD,UD,UD,UD,LT,UD,UD,UD,UD,UD,UD,UD,
/* C*/ LT,LT,LT,LT,LT,LT,LT,LT,LT,LT,LT,LT,LT,LT,LT,LT,
/* D*/ LT,LT,LT,LT,LT,LT,LT,LT,LT,LT,LT,LT,LT,LT,LT,LT,
/* E*/ LT,LT,LT,LT,LT,LT,LT,LT,LT,LT,LT,LT,LT,LT,LT,LT,
/* F*/ LT,LT,LT,LT,LT,LT,LT,LT,LT,LT,LT,LT,LT,LT,LT,LT
};

class SimpleGrammar : public SimpleScaner
{
public:
    SimpleGrammar();
    virtual ~SimpleGrammar();

    ScanResult *Scan(void);
};

