//SimpleSyntax.h
//Author: Bombo
//16.03.2017
//Синтаксический анализатор

#pragma once

#include"SimpleScaner.h"
#include"SimpleGrammar.h"

enum {ERRORX=-1, UNDEFINEDX=0, WITHINX=1, FIRSTX=2, LASTX=4, WRONGX};

struct BNFLexem
{
    int nNumberOfLexem; //number of lexem
    char *pValue; //the value of this lexem
    int nLength; //the length of current value
    bool fTerminal; //flag if this symbol is a terminal or not

    BNFLexem *pbnflNext;

    BNFLexem();
    BNFLexem(int in_nNumber, char const * const in_pValue, int in_nLength);
    ~BNFLexem();
};

struct BNFRule
{
    int nNumberOfRuleLexem; //the number of current lexem

    int *pRule; //some rule for this lexem
    int nLength; //the lenth of current rule
    char *pszFunctionName; //semantic function name

    BNFRule *pbnfrNext;

    BNFRule();
    BNFRule(int in_nNumber, int const * const in_pRule, int in_nLength);
    ~BNFRule();

    //add new lexem to this rule
    void AssembleRule(int in_nNumber);
    //sub old lexem from this rule
    void DisassembleRule(int in_nNumber);
    //reverse rule value
    void ReverseRule(void);
};

struct TransitionMatrix
{
    int nDimentionX;
    int nDimentionY;
    int *pMatrix;
    char *pchColumnNamesX;
    char *pchColumnNamesY;

    TransitionMatrix();
    TransitionMatrix(int in_nQuadOne);
    TransitionMatrix(int in_nDimentionX, int in_nDimentionY);
    TransitionMatrix(TransitionMatrix const * const in_ptmMatrix);
    TransitionMatrix(const TransitionMatrix& in_tmMatrix);
    ~TransitionMatrix();

    TransitionMatrix& operator=(const TransitionMatrix& in_tmMatrix);
    TransitionMatrix& operator|=(const TransitionMatrix& in_tmMatrix);
    TransitionMatrix& operator&=(const TransitionMatrix& in_tmMatrix);
    TransitionMatrix& operator/=(const TransitionMatrix& in_tmMatrix);
    TransitionMatrix operator|(const TransitionMatrix& in_tmMatrix);
    TransitionMatrix operator&(const TransitionMatrix& in_tmMatrix);
    TransitionMatrix operator/(const TransitionMatrix& in_tmMatrix);

    TransitionMatrix& operator*=(int in_nDigit);
    TransitionMatrix operator*(int in_nDigit);
    TransitionMatrix& operator+=(const TransitionMatrix& in_tmMatrix);
    TransitionMatrix operator+(const TransitionMatrix& in_tmMatrix);

    int &operator [](int in_nIndex);

    void SetNames(char *in_pchColumnNamesX, char *in_pchColumnNamesY);

    void Print(char const * const in_pch0, char const * const in_pch1);
};

struct PrecedenceMatrix
{
    TransitionMatrix tmPrecedenceMatrix; //matrix

    BNFRule *pbnfrIncludes; // =
    BNFRule *pbnfrHeads; // <
    BNFRule *pbnfrTails; // >

    TransitionMatrix tmIncludes;
    TransitionMatrix tmHeads;
    TransitionMatrix tmTails;

    BNFLexem const *pbnflLexems;
    int nElements;
    BNFRule const *pbnfrRules;

public:
    PrecedenceMatrix();
    PrecedenceMatrix(BNFLexem const * const in_pbnflLexems, BNFRule const * const in_pbnfrRules);
    ~PrecedenceMatrix();

    bool BuildMatrix(bool in_fPrint);

    bool MakeIncludes(void);
    bool MakeHeads(void);
    bool MakeTails(void);
    void AddBounds(void); //add # values to result matrix

    //add LAST
    bool AddNumber(BNFRule **in_pbnfrPtr, int in_nUpdatingRuleNumber, int in_nIncludeNumber);
    //del LAST (for some non-terminal)
    bool SubNumber(BNFRule **in_pbnfrPtr, int in_nUpdatingRuleNumber, int in_nIncludeNumber);

    //check if the symbol is terminal or not
    bool IsTerminal(int in_nNumber);

    int GetLexemNumber(char const * const in_pValue, int in_nLength);

    //debug output
    void PrintList(BNFRule const * const in_pbnfrPtr, char const * const in_pComment);
};

struct StackValue
{
    ScanResult srResult;
    int nLexemNumber;
    char *pszFunctionName;
    int nPrecedenceType;
    StackValue *psvRuleValues;

    StackValue *psvNext;

    StackValue();
    StackValue(StackValue const * const in_psvValue);
    StackValue(const StackValue& in_svValue);
    ~StackValue();

    const StackValue& operator =(StackValue const * const in_psvValue);
    const StackValue& operator =(const StackValue& in_svValue);

    void AddRuleValue(StackValue *in_psvValue);
    void SetFunctionName(char const * const in_pszFunctionName);
    void ClearRuleValues(void);
    void AddNextValue(StackValue * const in_psvValue);
    void ClearNextValues(void);

    //find some type of result
    bool NoGoal(char const * const in_pGoal);

    void PrintNext(void);
};

struct SemanticFunctions
{
    char *pszFunctionName;
    void (*pfSemanticFunction)(StackValue *);

    SemanticFunctions();
    SemanticFunctions(char const * const in_pszFunctionName, void (*in_pfSemanticFunction)(StackValue *));
    ~SemanticFunctions();

    SemanticFunctions *psfNext;
};

class SimpleSyntax
{
    StackValue *psvFirst; //the main stack of lexems from code for syntax analyzer
    SemanticFunctions *psfFirst; //all semantic functions

    BNFLexem *pbnflFirstLexem; //the list of all lexems
    int nLastNumber; //the last (maximum) number of lexem

    BNFRule *pbnfrFirstRule; //the list of all rules
    BNFRule *pbnfrLastRule; //the last rule in the list of all rules
    bool fAssemblyRule; //a location of the last found rule (right or left in grammar) (from "::=")

    PrecedenceMatrix *ppmMatrix; //the matrix of precedences NxNxNx2 (N is the length of Lexems list)

    SimpleScaner scScaner; //scaner

public:
    SimpleSyntax();
    SimpleSyntax(char const * const in_pszGrammar);
    ~SimpleSyntax();

    //the main method of this class
    //this method returns rule name (lexem value)
    StackValue *AnalyzeNext(void);
    void CleanStack(void);
    StackValue *PushLexem(ScanResult *in_psrResult);
    int PopLexem(StackValue& out_psvValue);
    StackValue *AnalyzePrecedenceType(void);
    void AddAnalyzedValue(StackValue ** const in_ppsvValue);

    //prepare syntax analizer
    void PrepareAnalyzer(char const * const in_pszGrammar);
    //prepare functions names
    void PrepareFunctionsNames(char const * const in_pszFunctions);
    //prepare code from buff
    void PrepareCodeFromBuff(char const * const in_pszBuff);
    //prepare code from file
    void PrepareCodeFromFile(char const * const in_pszFile);

    //add new lexem
    int AddLexem(char const * const in_pValue, int in_nLength);
    //get lexem
    BNFLexem const *GetLexem(char const * const in_pValue, int in_nLength);
    BNFLexem const *GetLexem(int in_nNumber);
    int GetLexemNumber(ScanResult const * const in_psrResult);
    void SetNonTerminal(int in_nNumber);

    //add new rule for some lexem
    bool AddRule(int in_nNumber, int *in_pRule, int in_nLength);
    //find lexem with this rule
    const BNFRule *FindRule(int const * const in_pRule, int in_nLength);
    //get lexem number by rule
    int GetNumberByRule(int const * const in_pRule, int in_nLength);
    //clear current rule list
    void ClearAllRules(void);

    //set semantic functions list
    void SetSemanticFunctions(SemanticFunctions *in_psfFunctions);
    //add semantic function
    void AddSemanticFunction(char const * const in_pszFunctionName, void (*pfSemanticFunction)(StackValue *));
    //call some semantic function
    void CallSemanticFunction(StackValue *in_psvValue);
    //clear semantic functions
    void ClearSemanticFunctions(void);

    //init precedence matrix
    bool InitPrecedenceMatrix(void);
    //no left recursion
    void NoLeftRecursion(void);

    //debug output
    void PrintAllLexems(void);
    void PrintAllRules(void);

    void PrintBuff(void);
    void MakeScan(void);

    void PrintStack(void);
};
