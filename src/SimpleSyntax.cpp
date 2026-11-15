//SimpleSyntax.cpp
//Author: Bombo
//16.03.2017
//Синтаксический анализатор

#include"SimpleSyntax.h"

//***************************************SimpleSyntax****************************************//

SimpleSyntax::SimpleSyntax()
{
    psvFirst = NULL;
    psfFirst= NULL;

    pbnflFirstLexem = NULL;
    nLastNumber = 0;

    pbnfrFirstRule = NULL;
    pbnfrLastRule = NULL;
    fAssemblyRule = false;

    ppmMatrix = NULL;

    return;
}

SimpleSyntax::SimpleSyntax(char const * const in_pszGrammar)
{
    psvFirst = NULL;
    psfFirst = NULL;

    pbnflFirstLexem = NULL;
    nLastNumber = 0;

    pbnfrFirstRule = NULL;
    pbnfrLastRule = NULL;
    fAssemblyRule = false;

    ppmMatrix = NULL;

    PrepareAnalyzer(in_pszGrammar);
}

SimpleSyntax::~SimpleSyntax()
{
    BNFLexem *pbnflListLexem;
    BNFRule *pbnfrListRule;

    ClearSemanticFunctions();

    if(pbnflFirstLexem != NULL)
    {
      pbnflListLexem = pbnflFirstLexem;
      while(pbnflFirstLexem != NULL)
      {
        pbnflFirstLexem = pbnflFirstLexem->pbnflNext;
        delete pbnflListLexem;
        pbnflListLexem = pbnflFirstLexem;
      }
    }

    if(pbnfrFirstRule != NULL)
    {
      pbnfrListRule = pbnfrFirstRule;
      while(pbnfrFirstRule != NULL)
      {
        pbnfrFirstRule = pbnfrFirstRule->pbnfrNext;
        delete pbnfrListRule;
        pbnfrListRule = pbnfrFirstRule;
      }
    }

    if(ppmMatrix != NULL)
      delete ppmMatrix;

    return;
}

//it's a main method of this class
StackValue *SimpleSyntax::AnalyzeNext(void)
{
    ScanResult *psrResult;
    StackValue *psvReturn;

    psvReturn = NULL;
    while(scScaner.NextScan(&psrResult))
    {
      psvReturn = PushLexem(psrResult);
      AddAnalyzedValue(&psvReturn);
      return psvReturn;
    }
    psvReturn = PushLexem(psrResult);
    AddAnalyzedValue(&psvReturn);
    psrResult = new ScanResult("#");
    psrResult->AddValue("#", 1);
    psvReturn = PushLexem(psrResult);
    AddAnalyzedValue(&psvReturn);
    return psvReturn;
}

void SimpleSyntax::CleanStack(void)
{
    StackValue *psvList;
    ScanResult *psrResult;

    psvList = psvFirst;
    while(psvFirst != NULL)
    {
      psvFirst = psvFirst->psvNext;
      delete psvList;
      psvList = psvFirst;
    }
    psrResult = new ScanResult("#");
    psrResult->AddValue("#", 1);
    psvList = PushLexem(psrResult);
    delete psvList;
}

StackValue *SimpleSyntax::PushLexem(ScanResult *in_psrResult)
{
    StackValue *psvValue, *psvAnalyzedValue;

    //do not add empty strings and commentaries
    if(in_psrResult == NULL || strncmp(in_psrResult->pszName,"COMMENT\t",8) == 0)
      return NULL;

    psvValue = new StackValue();
    //copy scan result
    psvValue->srResult = *in_psrResult;
    //find and set lexem number
    psvValue->nLexemNumber = GetLexemNumber(in_psrResult);

    //set precedence type
    if(ppmMatrix != NULL && psvFirst != NULL)
      psvValue->nPrecedenceType = ppmMatrix->tmPrecedenceMatrix[(psvFirst->nLexemNumber-1)*nLastNumber + psvValue->nLexemNumber-1];
    else
      psvValue->nPrecedenceType = -1;

//    fprintf(stderr, "%s, %d, type=%d\n", in_psrResult->pszName, psvValue->nLexemNumber, psvValue->nPrecedenceType);

    psvValue->psvNext = psvFirst;
    psvFirst = psvValue;

    //don't process '#' symbol
    if(strncmp(psvFirst->srResult.pszName, "#", 1) == 0)
      return NULL;

    psvAnalyzedValue = AnalyzePrecedenceType();
//    psvValue->PrintNext();

    return psvAnalyzedValue;
}

StackValue *SimpleSyntax::AnalyzePrecedenceType(void)
{
  BNFRule bnfrRule;
  BNFRule const *pbnfrRule;
  BNFLexem const *pbnflLexem;
  StackValue svLast, svValue;
  StackValue *psvValue, *psvReturn = NULL;

//  PrintStack();

  //analyze precedence type
  switch(psvFirst->nPrecedenceType)
  {
    case ERRORX:
      break;
    case UNDEFINEDX:
      break;
    case WITHINX:
      return NULL;
    case FIRSTX:
      return NULL;
    case LASTX:
      //remember the last value (the first-value in this stack)
      PopLexem(svLast);
      psvValue = new StackValue();
      //find the rule
      for(;;)
      {
        switch(PopLexem(svValue))
        {
          case WITHINX:
            //update rule
            bnfrRule.AssembleRule(svValue.nLexemNumber);
            psvValue->AddRuleValue(&svValue);
            continue;
          case FIRSTX:
            //update rule
            bnfrRule.AssembleRule(svValue.nLexemNumber);
            bnfrRule.ReverseRule();
            //find rule
            pbnfrRule = FindRule(bnfrRule.pRule, bnfrRule.nLength);
            //push new lexem
            if(pbnfrRule == NULL)
              break;
            pbnflLexem = GetLexem(pbnfrRule->nNumberOfRuleLexem);
            psvValue->AddRuleValue(&svValue);
            psvValue->nLexemNumber = pbnfrRule->nNumberOfRuleLexem;
            psvValue->psvNext = psvFirst;
            psvValue->SetFunctionName(pbnfrRule->pszFunctionName);

            psvValue->srResult.AddName(pbnflLexem->pValue);
            psvValue->srResult.AddValue(pbnflLexem->pValue, pbnflLexem->nLength);

            psvValue->nPrecedenceType = ppmMatrix->tmPrecedenceMatrix[pbnflLexem->nNumberOfLexem-1 + (psvFirst->nLexemNumber - 1)*nLastNumber];
            psvFirst = psvValue;
            //init return stack value
            psvReturn = new StackValue(psvValue);
            //fprintf(stderr, "SimpleSyntax::AnalyzePrecedenceType() : %s, %d, %s, type=%d\n", psvReturn->srResult.pszName, psvReturn->nLexemNumber, psvReturn->srResult.pbValue, psvReturn->nPrecedenceType); //debug!!!
            //clear in-stack value
            psvValue->ClearRuleValues();
            //fprintf(stderr, "SimpleSyntax::AnalyzePrecedenceType() : %s, %d, %s, type=%d\n", psvFirst->srResult.pszName, psvFirst->nLexemNumber, psvFirst->srResult.pbValue, psvFirst->nPrecedenceType); //debug!!!
            //get back the first value (with type update)
            psvValue = new StackValue();
            *psvValue = svLast;
            psvValue->nPrecedenceType = ppmMatrix->tmPrecedenceMatrix[svLast.nLexemNumber - 1 + (psvFirst->nLexemNumber - 1)*nLastNumber];
            psvValue->psvNext = psvFirst;
            psvFirst = psvValue;
            //fprintf(stderr, "\nSimpleSyntax::AnalyzePrecedenceType() : %s, %d, %s, type=%d\n", psvFirst->srResult.pszName, psvFirst->nLexemNumber, psvFirst->srResult.pbValue, psvFirst->nPrecedenceType); //debug!!!
//            psvValue = psvFirst->psvNext;
//            PrintStack();
//            psvReturn->PrintNext(); //debug!!!
            CallSemanticFunction(psvReturn);
            return psvReturn; //return stack value
        }
        if(strncmp(svValue.srResult.pszName, "GOAL", 4) == 0)
        {
          return NULL;
        }
        else
        {
          PrintStack();
          fprintf(stderr, "SimpleSyntax::AnalyzePrecedenceType() ERROR! : wrong lexem (%s, '%s', type=%d)!\n", svValue.srResult.pszName, svValue.srResult.pbValue, svValue.nPrecedenceType);
          if(psvReturn == NULL)
            psvReturn = new StackValue();
          psvReturn->srResult.AddName("ERROR\t");
          return psvReturn;
        }
        break;
      }
      break;
    default:
      break;
  }
  return NULL;
}

//this function returns "NOTHING\t" value if not rule detected
void SimpleSyntax::AddAnalyzedValue(StackValue ** const in_ppsvValue)
{
  StackValue *psvTemp;

  if(in_ppsvValue == NULL)
    return;

  psvTemp = AnalyzePrecedenceType();
  if(psvTemp == NULL)
  {
    if((*in_ppsvValue) == NULL)
    {
      (*in_ppsvValue) = new StackValue();
      (*in_ppsvValue)->srResult.AddName("NOTHING\t");
      (*in_ppsvValue)->srResult.AddValue("NOTHING", 7);
    }
    return;
  }
  else
  {
    if((*in_ppsvValue) == NULL)
    {
      (*in_ppsvValue) = new StackValue();
      (*in_ppsvValue) = psvTemp;
    }
    else
    {
      (*in_ppsvValue)->AddNextValue(psvTemp);
    }
  }
  //find other rules
  while( (psvTemp = AnalyzePrecedenceType()) != NULL )
  {
    (*in_ppsvValue)->AddNextValue(psvTemp);
  }
}

int SimpleSyntax::PopLexem(StackValue &out_psvValue)
{
    StackValue *psvList;

    if(psvFirst == NULL)
      return ERRORX;

    out_psvValue = psvFirst;
    psvList = psvFirst;
    psvFirst = psvFirst->psvNext;
    delete psvList;

    return out_psvValue.nPrecedenceType;
}

void SimpleSyntax::PrepareAnalyzer(char const * const in_pszGrammar)
{
    int nLexemIndex;
    ScanResult *psrResult;
    SimpleGrammar *psgScaner;

    if(in_pszGrammar == NULL || strlen(in_pszGrammar) == 0)
      return;

    //clear current rule in syntax analizer
    ClearAllRules();

    psgScaner = new SimpleGrammar();
    psgScaner->ReadBuff(in_pszGrammar);
//    psgScaner->PrintBuff();

    while(psgScaner->NextScan(&psrResult))
    {
      if(psrResult->NameCmp("RULE_NAME\t") || psrResult->NameCmp("LEXEM\t"))
      {
        nLexemIndex = AddLexem(psrResult->pbValue, psrResult->lSize);
        if(fAssemblyRule)
          pbnfrLastRule->AssembleRule(nLexemIndex);
        else
          AddRule(nLexemIndex, NULL, 0);
        continue;
      }

      if(psrResult->NameCmp("DELIMITER\t"))
      {
        nLexemIndex = AddLexem(psrResult->pbValue, psrResult->lSize);
        if(fAssemblyRule)
          pbnfrLastRule->AssembleRule(nLexemIndex);
        continue;
      }

      if(psrResult->NameCmp("RULE\t"))
      {
        fAssemblyRule = true;
        continue;
      }

      if(psrResult->NameCmp("RULE_END\t"))
      {
        fAssemblyRule = false;
        continue;
      }

      if(psrResult->NameCmp("NEXT\t"))
      {
        AddRule(pbnfrLastRule->nNumberOfRuleLexem, NULL, 0);
        fAssemblyRule = true;
        continue;
      }

      delete psrResult;
    }

    delete psgScaner;

    //prepared lexems
    AddLexem("#", 1);

    NoLeftRecursion();

//    PrintAllLexems();
//    PrintAllRules();

    InitPrecedenceMatrix();

    return;
}

void SimpleSyntax::PrepareFunctionsNames(const char * const in_pszFunctions)
{
    int nLength;
    SimpleSyntax *psFunctions;
    BNFLexem const *pbnflLexem;
    BNFRule *pbnfrList, *pbnfrThisList;

    if(pbnfrFirstRule == NULL)
      return;

    psFunctions = new SimpleSyntax();
    psFunctions->PrepareAnalyzer(in_pszFunctions);

    pbnfrList = psFunctions->pbnfrFirstRule;
    pbnfrThisList = pbnfrFirstRule;
    while(pbnfrList != NULL && pbnfrThisList != NULL)
    {
      if(pbnfrList->nLength > 0 && pbnfrList->pRule != NULL)
      {
        pbnflLexem = psFunctions->GetLexem(pbnfrList->pRule[0]);
        if(pbnflLexem != NULL && pbnflLexem->nLength > 0 && pbnflLexem->pValue != NULL)
        {
          nLength = strlen(pbnflLexem->pValue);
          if(pbnfrThisList->pszFunctionName != NULL)
            delete [] pbnfrThisList->pszFunctionName;
          //set function name for a rule
          pbnfrThisList->pszFunctionName = new char[nLength + 1];
          memset(pbnfrThisList->pszFunctionName, 0, nLength + 1);
          strncpy(pbnfrThisList->pszFunctionName, pbnflLexem->pValue, nLength);
        }
      }
      pbnfrThisList = pbnfrThisList->pbnfrNext;
      pbnfrList = pbnfrList->pbnfrNext;
    }

    delete psFunctions;
}

void SimpleSyntax::PrepareCodeFromBuff(const char * const in_pszBuff)
{
  scScaner.ReadBuff(in_pszBuff);
}

void SimpleSyntax::PrepareCodeFromFile(const char * const in_pszFile)
{
  scScaner.OpenFile(in_pszFile);
}

void SimpleSyntax::NoLeftRecursion(void)
{
  char *pNewValue;
  const BNFLexem *pbnflLexemInfo;
  int nIndex, nNewLength, nLexemIndex;
  BNFRule *pbnfrList, *pbnfrUpdateList;
  BNFLexem *pbnflUpdatedLexems, *pbnflNewLexem, *pbnflList;

  //preinit
  pbnflUpdatedLexems = NULL;

  //no left recursion
  pbnfrList = pbnfrFirstRule;
  while(pbnfrList != NULL)
  {
    if(pbnfrList->nLength > 0 && pbnfrList->pRule[0] == pbnfrList->nNumberOfRuleLexem)
    {
      //check if this lexem is already updated
      pbnflList = pbnflUpdatedLexems;
      while(pbnflList != NULL)
      {
        if(pbnflList->nNumberOfLexem == pbnfrList->nNumberOfRuleLexem)
        {
          pbnfrList = pbnfrList->pbnfrNext;
          continue;
        }
        pbnflList = pbnflList->pbnflNext;
      }
      //get old lexem
      pbnflLexemInfo = GetLexem(pbnfrList->nNumberOfRuleLexem);
      if(pbnflLexemInfo == NULL)
      {
        pbnfrList = pbnfrList->pbnfrNext;
        continue;
      }
      //create new value for new lexem
      nNewLength = pbnflLexemInfo->nLength + 2;
      pNewValue = new char[nNewLength];
      memset(pNewValue, 0, nNewLength);
      for(nIndex = 0; nIndex < pbnflLexemInfo->nLength; ++nIndex)
      {
        pNewValue[nIndex] = pbnflLexemInfo->pValue[nIndex];
      }
      pNewValue[pbnflLexemInfo->nLength] = '_';
      pNewValue[pbnflLexemInfo->nLength + 1] = '1';
      //create new lexem
      nLexemIndex = AddLexem(pNewValue, nNewLength);

      //add old lexem to new temporary list
      pbnflNewLexem = new BNFLexem(nLexemIndex, pNewValue, nNewLength);
      if(pbnflUpdatedLexems == NULL)
      {
        pbnflUpdatedLexems = pbnflNewLexem;
      }
      else
      {
        pbnflList = pbnflUpdatedLexems;
        while(pbnflList->pbnflNext != NULL)
        {
          pbnflList = pbnflList->pbnflNext;
        }
        pbnflList->pbnflNext = pbnflNewLexem;
      }

      //add new rule "oldLexem::=newLexem"
      AddRule(pbnflLexemInfo->nNumberOfLexem, &(nLexemIndex), 1);

      //swap old lexem and new lexem
      pbnfrUpdateList = pbnfrFirstRule;
      while(pbnfrUpdateList != NULL)
      {
        if(pbnfrUpdateList->nNumberOfRuleLexem == pbnflLexemInfo->nNumberOfLexem &&
           pbnfrUpdateList->nLength > 0 &&
           pbnfrUpdateList->pRule != NULL &&
           pbnfrUpdateList->pRule[0] != nLexemIndex)
        {
          for(nIndex = 0; nIndex < pbnfrUpdateList->nLength; ++nIndex)
          {
            if(pbnfrUpdateList->pRule[nIndex] == pbnfrUpdateList->nNumberOfRuleLexem)
              pbnfrUpdateList->pRule[nIndex] = nLexemIndex;
          }
          pbnfrUpdateList->nNumberOfRuleLexem = nLexemIndex;
        }
        pbnfrUpdateList = pbnfrUpdateList->pbnfrNext;
      }
    }
    pbnfrList = pbnfrList->pbnfrNext;
  }
}

int SimpleSyntax::AddLexem(char const * const in_pValue, int in_nLength)
{
    BNFLexem const *pbnflLexem;
    BNFLexem *pbnflListLexem, *pbnflNewLexem;

    if(in_pValue == NULL || in_nLength <= 0)
      return -1;

    //check if this lexem is already exist
    if((pbnflLexem = GetLexem(in_pValue, in_nLength)) != NULL)
      return pbnflLexem->nNumberOfLexem;

    nLastNumber++;
    pbnflNewLexem = new BNFLexem(nLastNumber, in_pValue, in_nLength);

    if(pbnflFirstLexem == NULL)
    {
      pbnflFirstLexem = pbnflNewLexem;
      return nLastNumber;
    }

    pbnflListLexem = pbnflFirstLexem;
    while(pbnflListLexem->pbnflNext != NULL)
    {
      pbnflListLexem = pbnflListLexem->pbnflNext;
    }
    pbnflListLexem->pbnflNext = pbnflNewLexem;
    return nLastNumber;
}

BNFLexem const *SimpleSyntax::GetLexem(const char * const in_pValue, int in_nLength)
{
    BNFLexem *pbnflListLexem;

    if(pbnflFirstLexem == NULL || in_pValue == NULL || in_nLength <= 0)
      return NULL;

    pbnflListLexem = pbnflFirstLexem;
    while(pbnflListLexem != NULL)
    {
      if(in_nLength == pbnflListLexem->nLength && memcmp(in_pValue, pbnflListLexem->pValue, in_nLength) == 0)
        return pbnflListLexem;
      pbnflListLexem = pbnflListLexem->pbnflNext;
    }
    return NULL;
}

BNFLexem const *SimpleSyntax::GetLexem(int in_nNumber)
{
    BNFLexem const *pbnflListLexem;

    if(pbnflFirstLexem == NULL)
      return NULL;

    pbnflListLexem = pbnflFirstLexem;
    while(pbnflListLexem != NULL)
    {
      if(pbnflListLexem->nNumberOfLexem == in_nNumber)
        return pbnflListLexem;
      pbnflListLexem = pbnflListLexem->pbnflNext;
    }
    return NULL;
}

int SimpleSyntax::GetLexemNumber(ScanResult const * const in_psrResult)
{
    BNFLexem const *pbnflLexem;

    if(in_psrResult == NULL)
      return -1;

    pbnflLexem = GetLexem(in_psrResult->pbValue, in_psrResult->lSize);
    if(pbnflLexem != NULL)
    {
      return pbnflLexem->nNumberOfLexem;
    }
    else
    {
      pbnflLexem = GetLexem(in_psrResult->pszName, strlen(in_psrResult->pszName)-1);
      if(pbnflLexem != NULL)
        return pbnflLexem->nNumberOfLexem;
      else
        return -1;
    }
}

void SimpleSyntax::SetNonTerminal(int in_nNumber)
{
    BNFLexem *pbnflList;

    pbnflList = pbnflFirstLexem;
    while(pbnflList != NULL)
    {
      if(pbnflList->nNumberOfLexem == in_nNumber)
      {
        pbnflList->fTerminal = false;
        return;
      }
      pbnflList = pbnflList->pbnflNext;
    }
}

bool SimpleSyntax::AddRule(int in_nNumber, int *in_pRule, int in_nLength)
{
    BNFRule const *pbnfrList;
    BNFRule *pbnfrRule, *pbnfrListRule;

    if( (pbnfrList = FindRule(in_pRule, in_nLength)) != NULL )
    {
      fprintf(stderr, "error: Rule duplicate for \"%s\"\n", GetLexem(pbnfrList->nNumberOfRuleLexem)->pValue);
      return false;
    }

    pbnfrRule = new BNFRule(in_nNumber, in_pRule, in_nLength);
    SetNonTerminal(in_nNumber);

    if(pbnfrFirstRule == NULL)
    {
      pbnfrFirstRule = pbnfrLastRule = pbnfrRule;
    }
    else
    {
      if(pbnfrLastRule == NULL)
      {
        pbnfrListRule = pbnfrFirstRule;
        while(pbnfrListRule->pbnfrNext != NULL)
        {
          pbnfrListRule = pbnfrListRule->pbnfrNext;
        }
        pbnfrListRule->pbnfrNext = pbnfrLastRule = pbnfrRule;
      }
      else
      {
        pbnfrLastRule->pbnfrNext = pbnfrRule;
        pbnfrLastRule = pbnfrRule;
      }
    }

    return true;
}

BNFRule const *SimpleSyntax::FindRule(int const * const in_pRule, int in_nLength)
{
    BNFRule *pbnfrListRule;

    if(pbnfrFirstRule == NULL || in_pRule == NULL || in_nLength == 0)
      return NULL;

    pbnfrListRule = pbnfrFirstRule;
    while(pbnfrListRule != NULL)
    {
      if(in_nLength == pbnfrListRule->nLength && memcmp(in_pRule, pbnfrListRule->pRule, in_nLength) == 0)
        return pbnfrListRule;
      pbnfrListRule = pbnfrListRule->pbnfrNext;
    }
    return NULL;
}

int SimpleSyntax::GetNumberByRule(int const * const in_pRule, int in_nLength)
{
    BNFRule const *pbnfrRule;

    pbnfrRule = FindRule(in_pRule, in_nLength);
    if(pbnfrRule == NULL)
      return -1;
    else
      return pbnfrRule->nNumberOfRuleLexem;
}

void SimpleSyntax::ClearAllRules(void)
{
    BNFRule *pbnfrListRule;

    pbnfrLastRule = NULL;
    pbnfrListRule = pbnfrFirstRule;
    while(pbnfrFirstRule != NULL)
    {
      pbnfrFirstRule = pbnfrFirstRule->pbnfrNext;
      delete pbnfrListRule;
      pbnfrListRule = pbnfrFirstRule;
    }
}

void SimpleSyntax::SetSemanticFunctions(SemanticFunctions *in_psfFunctions)
{
    SemanticFunctions *psfList;

    ClearSemanticFunctions();
    psfList = in_psfFunctions;
    while(psfList->pszFunctionName != NULL && psfList->pfSemanticFunction != NULL)
    {
      AddSemanticFunction(psfList->pszFunctionName, psfList->pfSemanticFunction);
      psfList++;
    }
}

void SimpleSyntax::AddSemanticFunction(const char * const in_pszFunctionName, void (*pfSemanticFunction)(StackValue *))
{
    SemanticFunctions *psfList, *psfTemp;

    psfTemp = new SemanticFunctions(in_pszFunctionName, pfSemanticFunction);
    if(psfFirst == NULL)
    {
      psfFirst = psfTemp;
      return;
    }

    psfList = psfFirst;
    while(psfList->psfNext != NULL)
    {
      psfList = psfList->psfNext;
    }
    psfList->psfNext = psfTemp;
}

void SimpleSyntax::CallSemanticFunction(StackValue *in_psvValue)
{
    SemanticFunctions *psfList;
    StackValue *psvStart;

    if(in_psvValue == NULL || in_psvValue->pszFunctionName == NULL)
      return;

    psvStart = in_psvValue;

    psfList = psfFirst;
    while(psfList != NULL)
    {
      if(psvStart->pszFunctionName != NULL && strcmp(psvStart->pszFunctionName, psfList->pszFunctionName) == 0)
      {
        psfList->pfSemanticFunction(psvStart);
        psvStart = psvStart->psvNext;
        if(psvStart != NULL)
        {
          psfList = psfFirst;
          continue;
        }
        return;
      }
      psfList = psfList->psfNext;
    }
}

void SimpleSyntax::ClearSemanticFunctions(void)
{
    SemanticFunctions *psfList;

    while(psfFirst != NULL)
    {
      psfList = psfFirst->psfNext;
      delete psfFirst;
      psfFirst = psfList;
    }
}

bool SimpleSyntax::InitPrecedenceMatrix(void)
{
    if(ppmMatrix != NULL)
      delete ppmMatrix;

    //getting space for precedence matrix
    ppmMatrix = new PrecedenceMatrix(pbnflFirstLexem, pbnfrFirstRule);

    //init matrix
    return ppmMatrix->BuildMatrix(false);
}

void SimpleSyntax::PrintAllLexems(void)
{
    BNFLexem *pbnflListLexem;

    if(pbnflFirstLexem == NULL)
    {
      fprintf(stderr, "\nNO LEXEM IS FOUND!\n");
      return;
    }

    pbnflListLexem = pbnflFirstLexem;
    while(pbnflListLexem != NULL)
    {
//      fprintf(stderr, "%d, %s, %s\n", pbnflListLexem->nNumber, pbnflListLexem->pValue, pbnflListLexem->fTerminal?"terminal":"non-terminal");
      fprintf(stderr, "%d, %s\n", pbnflListLexem->nNumberOfLexem, pbnflListLexem->pValue);
      pbnflListLexem = pbnflListLexem->pbnflNext;
    }
    fprintf(stderr,"\n");

    return;
}

void SimpleSyntax::PrintAllRules(void)
{
    int nNumberIndex;
    BNFRule *pbnfrListRule;

    if(pbnfrFirstRule == NULL)
      return;

    pbnfrListRule = pbnfrFirstRule;
    while(pbnfrListRule != NULL)
    {
      fprintf(stderr, "%d::=", pbnfrListRule->nNumberOfRuleLexem);
      for(nNumberIndex = 0; nNumberIndex < pbnfrListRule->nLength; ++nNumberIndex)
      {
        fprintf(stderr, "%d,", pbnfrListRule->pRule[nNumberIndex]);
      }
      fprintf(stderr, "\n");
      pbnfrListRule = pbnfrListRule->pbnfrNext;
    }
    fprintf(stderr, "\n");
    return;
}

void SimpleSyntax::PrintBuff(void)
{
    scScaner.PrintBuff();
}

void SimpleSyntax::MakeScan(void)
{
    scScaner.MakeScan();
}

void SimpleSyntax::PrintStack(void)
{
    StackValue *psvList, *psvLast;

    psvLast = NULL;
    while(psvLast != psvFirst)
    {
      psvList = psvFirst;
      while(psvList->psvNext != psvLast)
      {
        psvList = psvList->psvNext;
      }
      if(psvList->srResult.pbValue == NULL)
        fprintf(stderr, " <%s> ", psvList->srResult.pszName);
      else
        fprintf(stderr, "%s", psvList->srResult.pbValue);
      psvLast = psvList;
    }

    fprintf(stderr, "\n");
}

//****************************************StackValue*****************************************//

StackValue::StackValue()
{
    pszFunctionName = NULL;
    psvNext = NULL;
    psvRuleValues = NULL;
}

StackValue::StackValue(const StackValue * const in_psvValue)
{
  pszFunctionName = NULL;
  psvRuleValues = NULL;
  *this = in_psvValue;
}

StackValue::StackValue(const StackValue &in_svValue)
{
  pszFunctionName = NULL;
  psvRuleValues = NULL;
  *this = &in_svValue;
}

StackValue::~StackValue()
{
  ClearRuleValues();
}

const StackValue& StackValue::operator =(StackValue const * const in_psvValue)
{
    int nLength;
    StackValue *psvList, *psvTemp, *psvRule;

    if(in_psvValue == NULL)
      return *this;

    psvNext = NULL;
    srResult = in_psvValue->srResult;
    nLexemNumber = in_psvValue->nLexemNumber;
    nPrecedenceType = in_psvValue->nPrecedenceType;

    if(pszFunctionName != NULL)
    {
      delete [] pszFunctionName;
      pszFunctionName = NULL;
    }

    nLength = 0;
    if(in_psvValue->pszFunctionName != NULL)
      nLength = strlen(in_psvValue->pszFunctionName);
    if(nLength > 0)
    {
      pszFunctionName = new char[nLength+1];
      memset(pszFunctionName, 0, nLength+1);
      strncpy(pszFunctionName, in_psvValue->pszFunctionName, nLength);
    }

    //free old stack values
    if(psvRuleValues != NULL)
    {
      while(psvRuleValues != NULL)
      {
        psvTemp = psvRuleValues->psvNext;
        delete psvRuleValues;
        psvRuleValues = psvTemp;
      }
    }

    //init the list of rule values (for sending to semantic analyzer)
    psvList = in_psvValue->psvRuleValues;
    while(psvList != NULL)
    {
      psvTemp = new StackValue();
      psvTemp->srResult = psvList->srResult;
      psvTemp->nLexemNumber = psvList->nLexemNumber;
      psvTemp->nPrecedenceType = psvList->nPrecedenceType;
      psvTemp->psvNext = NULL;

      if(psvRuleValues == NULL)
      {
          psvRuleValues = psvTemp;
      }
      else
      {
          psvRule = psvRuleValues;
          while(psvRule->psvNext != NULL)
          {
            psvRule = psvRule->psvNext;
          }
          psvRule->psvNext = psvTemp;
      }

      psvList = psvList->psvNext;
    }

    return *this;
}

const StackValue& StackValue::operator =(const StackValue& in_svValue)
{
    *this = &in_svValue;

    return *this;
}

void StackValue::AddRuleValue(StackValue *in_psvValue)
{
  StackValue *psvTemp, *psvList;

  psvTemp = new StackValue();
  *psvTemp = in_psvValue;

  if(psvRuleValues == NULL)
  {
    psvRuleValues = psvTemp;
  }
  else
  {
    psvList = psvRuleValues;
    psvRuleValues = psvTemp;
    psvTemp->psvNext = psvList;
  }
}

void StackValue::SetFunctionName(const char * const in_pszFunctionName)
{
  int nLength;

  if(pszFunctionName != NULL)
  {
    delete [] pszFunctionName;
    pszFunctionName = NULL;
  }
  if(in_pszFunctionName != NULL)
  {
    nLength = strlen(in_pszFunctionName);
    pszFunctionName = new char[nLength+1];
    memset(pszFunctionName, 0, nLength+1);
    strncpy(pszFunctionName, in_pszFunctionName, nLength);
  }
}

void StackValue::ClearRuleValues(void)
{
  StackValue *psvList;

  psvList = psvRuleValues;
  while(psvRuleValues != NULL)
  {
    psvList = psvList->psvNext;
    delete psvRuleValues;
    psvRuleValues = psvList;
  }
}

void StackValue::AddNextValue(StackValue * const in_psvValue)
{
  StackValue *psvList;
  if(in_psvValue == NULL)
    return;

  if(psvNext == NULL)
  {
    psvNext = in_psvValue;
    return;
  }
  psvList = psvNext;
  while(psvList->psvNext != NULL)
  {
    psvList = psvList->psvNext;
  }
  psvList->psvNext = in_psvValue;
}

void StackValue::ClearNextValues(void)
{
  StackValue *psvList;

  if(psvNext == NULL)
    return;

  psvList = psvNext;
  while(psvNext != NULL)
  {
    psvList = psvList->psvNext;
    delete psvNext;
    psvNext = psvList;
  }
}

//this method find "GOAL" result
bool StackValue::NoGoal(const char * const in_pGoal)
{
  StackValue *psvList;

  if(in_pGoal == NULL)
    return true;

  if(srResult.pszName == NULL || strncmp(srResult.pszName, in_pGoal, strlen(in_pGoal)) == 0)
  {
    return false;
  }

  psvList = psvNext;
  while(psvList != NULL)
  {
    if(strncmp(psvList->srResult.pszName, in_pGoal, strlen(in_pGoal)) == 0)
      return false;
    psvList = psvList->psvNext;
  }

  return true;
}

void StackValue::PrintNext(void)
{
  char szOut[128];
  int nIndex, nCount;
  StackValue *psvList, *psvRules;

  psvList = this;
  while(psvList != NULL)
  {
    memset(szOut, 0, sizeof(szOut));
    if(psvList->pszFunctionName != NULL)
      snprintf(szOut, sizeof(szOut), "<%s>, type=%d, %s():", psvList->srResult.pszName, psvList->nPrecedenceType, psvList->pszFunctionName);
    else
      snprintf(szOut, sizeof(szOut), "<%s>, type=%d:", psvList->srResult.pszName, psvList->nPrecedenceType);
    fprintf(stderr, "%s", szOut);
    nCount = (int)(6-((strlen(szOut))/8));
//    fprintf(stderr, "\n%s, len=%ld, nCount=%d\n", szOut, strlen(szOut), nCount);
    for(nIndex = 0; nIndex < nCount; ++nIndex)
    {
      fprintf(stderr, "\t");
    }
    psvRules = psvList->psvRuleValues;
    while(psvRules != NULL)
    {
      if(psvRules->srResult.pbValue != NULL)
        fprintf(stderr, "'%s'", psvRules->srResult.pbValue);
      else
        fprintf(stderr, "<%s>", psvRules->srResult.pszName);
      psvRules = psvRules->psvNext;
    }
    fprintf(stderr, "\n");
    psvList = psvList->psvNext;
  }
}

//*****************************************BNFLexem******************************************//

BNFLexem::BNFLexem()
{
    nNumberOfLexem = -1;
    pValue = NULL;
    nLength = 0;
    fTerminal = true;

    pbnflNext = NULL;
}

BNFLexem::BNFLexem(int in_nNumber, const char * const in_pValue, int in_nLength)
{
    int nIndex;
    pbnflNext = NULL;

    fTerminal = true;

    if(in_nNumber < 0 || in_pValue == NULL || in_nLength <= 0)
    {
        nNumberOfLexem = -1;
        pValue = NULL;
        nLength = 0;
        return;
    }

    nNumberOfLexem = in_nNumber;
    pValue = new char[in_nLength+1];
    memset(pValue, 0, in_nLength + 1);
    for(nIndex = 0; nIndex < in_nLength; ++nIndex)
    {
      pValue[nIndex] = in_pValue[nIndex];
    }
    nLength = in_nLength;
}

BNFLexem::~BNFLexem()
{
    if(pValue != NULL)
      delete [] pValue;
}

//******************************************BNFRule******************************************//

BNFRule::BNFRule()
{
    nNumberOfRuleLexem = -1;
    pRule = NULL;
    nLength = 0;
    pszFunctionName = NULL;

    pbnfrNext = NULL;
}

BNFRule::BNFRule(int in_nNumber, const int * const in_pRule, int in_nLength)
{
    int nIndex;
    pbnfrNext = NULL;
    pszFunctionName = NULL;

    if(in_nNumber < 0)
    {
      nNumberOfRuleLexem = -1;
      pRule = NULL;
      nLength = 0;
      return;
    }

    nNumberOfRuleLexem = in_nNumber;
    if(in_pRule != NULL && in_nLength > 0)
    {
      pRule = new int[in_nLength];
      for(nIndex = 0; nIndex < in_nLength; ++nIndex)
      {
        pRule[nIndex] = in_pRule[nIndex];
      }
      nLength = in_nLength;
    }
    else
    {
      pRule = NULL;
      nLength = 0;
    }
}

BNFRule::~BNFRule()
{
    if(pRule != NULL)
      delete [] pRule;
    if(pszFunctionName != NULL)
      delete [] pszFunctionName;
}

void BNFRule::AssembleRule(int in_nNumber)
{
    int *pNewRule;
    int nValueIndex;

    pNewRule = new int[nLength+1];
    if(pRule != NULL)
    {
      for(nValueIndex = 0; nValueIndex < nLength; ++nValueIndex)
      {
        pNewRule[nValueIndex] = pRule[nValueIndex];
      }
      delete pRule;
    }
    //add new lexem to the end of rule
    pNewRule[nLength] = in_nNumber;
    pRule = pNewRule;
    nLength++;
}

void BNFRule::DisassembleRule(int in_nNumber)
{
    int *pNewRule;
    int nValueIndex, nSubIndex;

    if(nLength > 1)
    {
      pNewRule = new int[nLength-1];
    }
    else
    {
      if(pRule != NULL && nLength == 1 && pRule[0] == in_nNumber)
      {
        delete pRule;
        pRule = NULL;
        nLength = 0;
      }
      return;
    }

    if(pRule != NULL)
    {
      //check
      for(nValueIndex = 0; nValueIndex < nLength; ++nValueIndex)
      {
        if(pRule[nValueIndex] == in_nNumber)
          break;
      }
      //if no such lexem in this rule
      if(nValueIndex >= nLength)
      {
        delete [] pNewRule;
        return;
      }

      for(nValueIndex = 0, nSubIndex = 0; nValueIndex < nLength; ++nValueIndex)
      {
        //sub old lexem
        if(pRule[nValueIndex] != in_nNumber)
        {
          pNewRule[nSubIndex] = pRule[nValueIndex];
          nSubIndex++;
        }
      }
      delete pRule;
    }
    pRule = pNewRule;
    nLength--;
}

void BNFRule::ReverseRule(void)
{
    int nIndex;
    char *pMassive;

    if(nLength <= 0)
      return;

    pMassive = new char[nLength];
    memset(pMassive, 0, nLength);
    for(nIndex = 0; nIndex < nLength; ++nIndex)
    {
      pMassive[nIndex] = pRule[nIndex];
    }
    for(nIndex = 0; nIndex < nLength; ++nIndex)
    {
      pRule[nIndex] = pMassive[nLength - nIndex - 1];
    }
    delete [] pMassive;
}

//**************************************TransitionMatrix*************************************//

TransitionMatrix::TransitionMatrix()
{
  nDimentionX = 1;
  nDimentionY = 1;
  pMatrix = new int[4];
  pchColumnNamesX = NULL;
  pchColumnNamesY = NULL;
}

TransitionMatrix::TransitionMatrix(int in_nQuadOne)
{
  int nIndex;

  if(in_nQuadOne > 1)
    nDimentionX = nDimentionY = in_nQuadOne;
  else
    nDimentionX = nDimentionY = 2;

  pMatrix = new int[nDimentionX*nDimentionY];
  memset(pMatrix, 0, sizeof(int)*nDimentionX*nDimentionY);
  pchColumnNamesX = NULL;
  pchColumnNamesY = NULL;

  for(nIndex = 0; nIndex < nDimentionX; ++nIndex)
    pMatrix[nIndex*nDimentionY + nIndex] = true;
}

TransitionMatrix::TransitionMatrix(int in_nDimentionX, int in_nDimentionY)
{
  int nSize;

  if(in_nDimentionX > 1)
    nDimentionX = in_nDimentionX;
  else
    nDimentionX = 1;

  if(in_nDimentionY > 1)
    nDimentionY = in_nDimentionY;
  else
    nDimentionY = 1;

  nSize = nDimentionX*nDimentionY;
  pMatrix = new int[nSize];
  memset(pMatrix, 0, nSize*sizeof(int));

  pchColumnNamesX = NULL;
  pchColumnNamesY = NULL;
}

TransitionMatrix::TransitionMatrix(TransitionMatrix const * const in_ptmMatrix)
{
  int nSize;

  if(in_ptmMatrix->nDimentionX > 1)
    nDimentionX = in_ptmMatrix->nDimentionX;
  else
    nDimentionX = 1;

  if(in_ptmMatrix->nDimentionY > 1)
    nDimentionY = in_ptmMatrix->nDimentionY;
  else
    nDimentionY = 1;

  pchColumnNamesX = NULL;
  pchColumnNamesY = NULL;

  nSize = nDimentionX*nDimentionY;
  pMatrix = new int[nSize];
  memcpy(pMatrix, in_ptmMatrix->pMatrix, nSize*sizeof(int));

  if(in_ptmMatrix->pchColumnNamesX != NULL)
  {
    pchColumnNamesX = new char[strlen(in_ptmMatrix->pchColumnNamesX)];
    strncpy(pchColumnNamesX, in_ptmMatrix->pchColumnNamesX, strlen(in_ptmMatrix->pchColumnNamesX));
  }

  if(in_ptmMatrix->pchColumnNamesY != NULL)
  {
    pchColumnNamesY = new char[strlen(in_ptmMatrix->pchColumnNamesY)];
    strncpy(pchColumnNamesY, in_ptmMatrix->pchColumnNamesY, strlen(in_ptmMatrix->pchColumnNamesY));
  }
}

TransitionMatrix::TransitionMatrix(const TransitionMatrix& in_tmMatrix)
{
  int nSize;

  pchColumnNamesX = NULL;
  pchColumnNamesY = NULL;

  if(in_tmMatrix.nDimentionX > 1)
    nDimentionX = in_tmMatrix.nDimentionX;
  else
    nDimentionX = 1;

  if(in_tmMatrix.nDimentionY > 1)
    nDimentionY = in_tmMatrix.nDimentionY;
  else
    nDimentionY = 1;

  nSize = nDimentionX*nDimentionY;
  pMatrix = new int[nSize];
  memcpy(pMatrix, in_tmMatrix.pMatrix, nSize*sizeof(int));

  if(in_tmMatrix.pchColumnNamesX != NULL)
  {
    nSize = strlen(in_tmMatrix.pchColumnNamesX);
    pchColumnNamesX = new char[nSize];
    strncpy(pchColumnNamesX, in_tmMatrix.pchColumnNamesX, nSize);
  }

  if(in_tmMatrix.pchColumnNamesY != NULL)
  {
    nSize = strlen(in_tmMatrix.pchColumnNamesY);
    pchColumnNamesY = new char[nSize];
    strncpy(pchColumnNamesY, in_tmMatrix.pchColumnNamesY, nSize);
  }
}

TransitionMatrix::~TransitionMatrix()
{
  if(pMatrix != NULL)
  {
    delete [] pMatrix;
    pMatrix = NULL;
  }
  if(pchColumnNamesX != NULL)
  {
    delete [] pchColumnNamesX;
    pchColumnNamesX = NULL;
  }
  if(pchColumnNamesY != NULL)
  {
    delete [] pchColumnNamesY;
    pchColumnNamesY = NULL;
  }
}

TransitionMatrix& TransitionMatrix::operator =(const TransitionMatrix& in_tmMatrix)
{
  int nSize, nIndexX, nIndexY;

  if(pMatrix != NULL)
    delete [] pMatrix;
  if(pchColumnNamesX != NULL)
  {
    delete [] pchColumnNamesX;
    pchColumnNamesX = NULL;
  }
  if(pchColumnNamesY != NULL)
  {
    delete [] pchColumnNamesY;
    pchColumnNamesY = NULL;
  }

  if(in_tmMatrix.nDimentionX > 1)
    nDimentionX = in_tmMatrix.nDimentionX;
  else
    nDimentionX = 1;

  if(in_tmMatrix.nDimentionY > 1)
    nDimentionY = in_tmMatrix.nDimentionY;
  else
    nDimentionY = 1;

  nSize = nDimentionX*nDimentionY;
  pMatrix = new int[nSize];
  for(nIndexX = 0; nIndexX < nDimentionX; ++nIndexX)
  {
    for(nIndexY = 0; nIndexY < nDimentionY; ++nIndexY)
    {
      pMatrix[nIndexX*nDimentionX + nIndexY] = in_tmMatrix.pMatrix[nIndexX*nDimentionX + nIndexY];
    }
  }

  if(in_tmMatrix.pchColumnNamesX != NULL)
  {
    nSize = strlen(in_tmMatrix.pchColumnNamesX);
    pchColumnNamesX = new char[nSize];
    strncpy(pchColumnNamesX, in_tmMatrix.pchColumnNamesX, nSize);
  }

  if(in_tmMatrix.pchColumnNamesY != NULL)
  {
    nSize = strlen(in_tmMatrix.pchColumnNamesY);
    pchColumnNamesY = new char[nSize];
    strncpy(pchColumnNamesY, in_tmMatrix.pchColumnNamesY, nSize);
  }

  return *this;
}

TransitionMatrix& TransitionMatrix::operator |=(const TransitionMatrix& in_tmMatrix)
{
  int nIndexX, nIndexY;
  TransitionMatrix tmMatrix(nDimentionX, nDimentionY);

  if(nDimentionX != in_tmMatrix.nDimentionX ||
     nDimentionY != in_tmMatrix.nDimentionY)
    return *this;

  for(nIndexX = 0; nIndexX < nDimentionX; ++nIndexX)
  {
    for(nIndexY = 0; nIndexY < nDimentionY; ++nIndexY)
    {
      tmMatrix.pMatrix[nIndexX*nDimentionY + nIndexY] = (bool)(this->pMatrix[nIndexX*nDimentionY + nIndexY]) || (in_tmMatrix.pMatrix[nIndexX*nDimentionY + nIndexY]);
    }
  }
  *this = tmMatrix;

  return *this;
}

TransitionMatrix& TransitionMatrix::operator &=(const TransitionMatrix& in_tmMatrix)
{
  int nIndex, nIndexNewX, nIndexNewY;
  TransitionMatrix *ptmMatrix;
  bool bRes;

  if(nDimentionY != in_tmMatrix.nDimentionX || nDimentionX == 0 || nDimentionY == 0 || in_tmMatrix.nDimentionY == 0)
    return *this;

  ptmMatrix = new TransitionMatrix(nDimentionX, in_tmMatrix.nDimentionY);

  for(nIndexNewX = 0; nIndexNewX < nDimentionX; ++nIndexNewX)
  {
    for(nIndexNewY = 0; nIndexNewY < in_tmMatrix.nDimentionY; ++nIndexNewY)
    {
      bRes = false;
      for(nIndex = 0; nIndex < nDimentionY; ++nIndex)
      {
        bRes = bRes || ((pMatrix[nIndexNewX*nDimentionY + nIndex])&&(in_tmMatrix.pMatrix[nIndex*nDimentionY + nIndexNewY]));
      }
      ptmMatrix->pMatrix[nIndexNewX*nDimentionY + nIndexNewY] = bRes;
    }
  }

  *this = *ptmMatrix;
  delete ptmMatrix;

  return *this;
}

TransitionMatrix& TransitionMatrix::operator /=(const TransitionMatrix& in_tmMatrix)
{
  int nIndexNewX, nIndexNewY;
  TransitionMatrix *ptmMatrix;
  //TransitionMatrix *ptmResMatrix;

  if(nDimentionY != in_tmMatrix.nDimentionY || nDimentionX == 0 || nDimentionY == 0 || in_tmMatrix.nDimentionX == 0 || in_tmMatrix.nDimentionY == 0)
    return *this;

  ptmMatrix = new TransitionMatrix(in_tmMatrix.nDimentionY, in_tmMatrix.nDimentionX);

  for(nIndexNewX = 0; nIndexNewX < in_tmMatrix.nDimentionX; ++nIndexNewX)
    for(nIndexNewY = 0; nIndexNewY < in_tmMatrix.nDimentionY; ++nIndexNewY)
      ptmMatrix->pMatrix[nIndexNewY*in_tmMatrix.nDimentionX + nIndexNewX] = in_tmMatrix.pMatrix[nIndexNewX*in_tmMatrix.nDimentionY + nIndexNewY];

  *this &= *ptmMatrix;
  delete ptmMatrix;

  return *this;
}

TransitionMatrix TransitionMatrix::operator|(const TransitionMatrix& in_tmMatrix)
{
  TransitionMatrix tmMatrix(this);

  tmMatrix |= in_tmMatrix;

  return TransitionMatrix(tmMatrix);
}

TransitionMatrix TransitionMatrix::operator &(const TransitionMatrix& in_tmMatrix)
{
  TransitionMatrix tmMatrix(this);

  tmMatrix &= in_tmMatrix;

  return TransitionMatrix(tmMatrix);
}

TransitionMatrix TransitionMatrix::operator /(const TransitionMatrix& in_tmMatrix)
{
  TransitionMatrix tmMatrix(this);

  tmMatrix /= in_tmMatrix;

  return TransitionMatrix(tmMatrix);
}

TransitionMatrix& TransitionMatrix::operator *=(int in_nDigit)
{
  int nIndexX, nIndexY;

  if(nDimentionX <= 0 || nDimentionY <= 0)
    return *this;

  for(nIndexX = 0; nIndexX < nDimentionX; ++nIndexX)
  {
    for(nIndexY = 0; nIndexY < nDimentionY; ++nIndexY)
    {
      pMatrix[nIndexY*nDimentionX+nIndexX] *= in_nDigit;
    }
  }

  return *this;
}

TransitionMatrix TransitionMatrix::operator *(int in_nDigit)
{
  TransitionMatrix tmMatrix(this);

  tmMatrix *= in_nDigit;
  return TransitionMatrix(tmMatrix);
}

TransitionMatrix& TransitionMatrix::operator +=(const TransitionMatrix& in_tmMatrix)
{
  int nIndexX, nIndexY;

  if(nDimentionX != in_tmMatrix.nDimentionX || nDimentionY != in_tmMatrix.nDimentionY)
    return *this;

  for(nIndexX = 0; nIndexX < nDimentionX; ++nIndexX)
  {
    for(nIndexY = 0; nIndexY < nDimentionY; ++nIndexY)
    {
      pMatrix[nIndexX*nDimentionY + nIndexY] += in_tmMatrix.pMatrix[nIndexX*nDimentionY + nIndexY];
    }
  }

  return *this;
}

TransitionMatrix TransitionMatrix::operator +(const TransitionMatrix& in_tmMatrix)
{
  TransitionMatrix tmMatrix(this);

  tmMatrix += in_tmMatrix;
  return TransitionMatrix(tmMatrix);
}

int& TransitionMatrix::operator [](int in_nIndex)
{
  if(in_nIndex >= nDimentionX*nDimentionY)
    return pMatrix[0];

  return pMatrix[in_nIndex];
}

void TransitionMatrix::Print(char const * const in_pch0, char const * const in_pch1)
{
  int nIndexX, nIndexY;

  fprintf(stderr, "   ");
  for(nIndexY = 0; nIndexY < nDimentionY; ++nIndexY)
  {
      fprintf(stderr, "%2d ", nIndexY+1);
  }

  fprintf(stderr, "\n");
  if(in_pch1 != NULL)
  {
    for(nIndexX = 0; nIndexX < nDimentionX; ++nIndexX)
    {
      fprintf(stderr, "%2d  ", nIndexX+1);
      for(nIndexY = 0; nIndexY < nDimentionY; ++nIndexY)
      {
        fprintf(stderr, "%s  ", (pMatrix[nIndexX*nDimentionY + nIndexY])?in_pch1:in_pch0);
      }
      fprintf(stderr, "\n");
    }
  }
  else
  {
    for(nIndexX = 0; nIndexX < nDimentionX; ++nIndexX)
    {
      fprintf(stderr, "%2d ", nIndexX+1);
      for(nIndexY = 0; nIndexY < nDimentionY; ++nIndexY)
      {
        if(pMatrix[nIndexX*nDimentionY + nIndexY] == 0)
          fprintf(stderr, "%2s ", in_pch0);
        else
          fprintf(stderr, "%2d ", pMatrix[nIndexX*nDimentionY + nIndexY]);
      }
      fprintf(stderr, "\n");
    }
  }
  fprintf(stderr, "\n");
}

//**************************************PrecedenceMatrix*************************************//

PrecedenceMatrix::PrecedenceMatrix()
{
  pbnflLexems = NULL;
  nElements = 0;
  pbnfrRules = NULL;

  pbnfrIncludes = NULL;
  pbnfrHeads = NULL;
  pbnfrTails = NULL;
}

PrecedenceMatrix::PrecedenceMatrix(const BNFLexem * const in_pbnflLexems, const BNFRule * const in_pbnfrRules)
{
  BNFLexem const *pbnflList;

  pbnfrIncludes = pbnfrHeads = pbnfrTails = NULL;

  pbnflLexems = in_pbnflLexems;
  nElements = 0;
  if(in_pbnflLexems != NULL)
  {
    pbnflList = in_pbnflLexems;
    while(pbnflList != NULL)
    {
      nElements++;
      pbnflList = pbnflList->pbnflNext;
    }
  }

  pbnfrRules = in_pbnfrRules;
}

PrecedenceMatrix::~PrecedenceMatrix()
{
  BNFRule *pbnfrList;

  pbnfrList = pbnfrIncludes;
  while(pbnfrList != NULL)
  {
    pbnfrIncludes = pbnfrIncludes->pbnfrNext;
    delete pbnfrList;
    pbnfrList = pbnfrIncludes;
  }

  pbnfrList = pbnfrHeads;
  while(pbnfrList != NULL)
  {
    pbnfrHeads = pbnfrHeads->pbnfrNext;
    delete pbnfrList;
    pbnfrList = pbnfrHeads;
  }

  pbnfrList = pbnfrTails;
  while(pbnfrList != NULL)
  {
    pbnfrTails = pbnfrTails->pbnfrNext;
    delete pbnfrList;
    pbnfrList = pbnfrTails;
  }
}

bool PrecedenceMatrix::BuildMatrix(bool in_fPrint)
{
  bool fResult, fRet;
  TransitionMatrix tmEquiv, tmLess, tmMore, tmOne(nElements);

  if(pbnflLexems == NULL || pbnfrRules == NULL)
    return false;

  MakeIncludes();
  do {
    fResult = false;
    fRet = MakeHeads();
    fResult = fResult || fRet;
    fRet = MakeTails();
    fResult = fResult || fRet;
  }
  while(fResult);

//  PrintList(pbnfrIncludes, "Includes:\n");
//  tmIncludes.Print(".", NULL);

//  PrintList(pbnfrHeads, "Heads:\n");
//  tmHeads.Print(".", NULL);

//  PrintList(pbnfrTails, "Tails:\n");
//  tmTails.Print(".", NULL);

//  fprintf(stderr, "\n\n\n");

  //build matrix from includes, heads and tails
  tmEquiv = tmIncludes*WITHINX;
  tmLess = (tmIncludes & tmHeads)*FIRSTX;
  tmMore = (( tmOne / tmTails ) & tmIncludes & ( tmOne | tmHeads ))*LASTX;

//  fprintf(stderr, "Equivalent\n");
//  tmEquiv.Print(".", NULL);

//  fprintf(stderr, "\nLess\n");
//  tmLess.Print(".", NULL);

//  fprintf(stderr, "\nMore\n");
//  tmMore.Print(".", NULL);

  tmPrecedenceMatrix = tmEquiv + tmLess + tmMore;
  AddBounds();
  if(in_fPrint)
    tmPrecedenceMatrix.Print(".", NULL);

//  tmTails.Print(".", "1"); //отладка!!!

  return true;
}

bool PrecedenceMatrix::MakeIncludes()
{
  bool fResult, fRet;
  int nIndex;
  //int nInterIndex;
  BNFRule const *pbnfrList;
  //BNFRule *pbnfrInterList;

  if(pbnflLexems == NULL || pbnfrRules == NULL)
    return false;

  fResult = false;

  //find includes from all rules

  //init includes (it's ran just one time)
  if(pbnfrIncludes == NULL)
  {
    tmIncludes = TransitionMatrix(nElements, nElements);
    pbnfrList = pbnfrRules;
    while(pbnfrList != NULL)
    {
      for(nIndex = 0; nIndex < (pbnfrList->nLength - 1); ++nIndex)
      {
        tmIncludes[(int)(pbnfrList->pRule[nIndex]-1)*nElements + (int)(pbnfrList->pRule[nIndex+1]-1)] = true;
        fRet = AddNumber(&pbnfrIncludes, pbnfrList->pRule[nIndex], pbnfrList->pRule[nIndex+1]);
        fResult = fResult || fRet;
      }
      pbnfrList = pbnfrList->pbnfrNext;
    }
  }
  if(pbnfrIncludes == NULL)
    return false;

  return fResult;
}

bool PrecedenceMatrix::MakeHeads()
{
  bool fResult, fRet;
  int nIndex, nIndexInter;
  BNFRule const *pbnfrList, *pbnfrInterList;

  if(pbnflLexems == NULL || pbnfrRules == NULL)
    return false;

  fResult = false;

  //find heads from all rules

  //first init (find FIRST)
  if(pbnfrHeads == NULL)
  {
    tmHeads = TransitionMatrix(nElements, nElements);
    pbnfrList = pbnfrRules;
    while(pbnfrList != NULL)
    {
      if(pbnfrList->nLength > 0)
      {
        tmHeads[(pbnfrList->nNumberOfRuleLexem-1)*nElements+(pbnfrList->pRule[0])-1] = true;
        fRet = AddNumber(&pbnfrHeads, pbnfrList->nNumberOfRuleLexem, pbnfrList->pRule[0]);
        fResult = fResult || fRet;
      }
      pbnfrList = pbnfrList->pbnfrNext;
    }
  }

  //update (find FIRST+)
  pbnfrList = pbnfrHeads;
  while(pbnfrList != NULL)
  {
    for(nIndex = 0; nIndex < pbnfrList->nLength; ++nIndex)
    {
      pbnfrInterList = pbnfrHeads;
      while(pbnfrInterList != NULL)
      {
        if(pbnfrList->pRule[nIndex] == pbnfrInterList->nNumberOfRuleLexem)
        {
          for(nIndexInter = 0; nIndexInter < pbnfrInterList->nLength; ++nIndexInter)
          {
            tmHeads[(pbnfrList->nNumberOfRuleLexem-1)*(nElements)+(pbnfrInterList->pRule[nIndexInter]-1)] = true;
            fRet = AddNumber(&pbnfrHeads, pbnfrList->nNumberOfRuleLexem, pbnfrInterList->pRule[nIndexInter]);
            fResult = fResult || fRet;
          }
          break;
        }
        pbnfrInterList = pbnfrInterList->pbnfrNext;
      }
    }
    pbnfrList = pbnfrList->pbnfrNext;
  }

  return fResult;
}

bool PrecedenceMatrix::MakeTails()
{
  bool fResult, fRet;
  int nIndex, nInterIndex;
  BNFRule const *pbnfrList;
  BNFRule const *pbnfrInterList;

  if(pbnflLexems == NULL || pbnfrRules == NULL)
    return false;

  fResult = false;

  //find tails from all rules

  //first init (find LAST)
  if(pbnfrTails == NULL)
  {
    tmTails = TransitionMatrix(nElements, nElements);
    pbnfrList = pbnfrRules;
    while(pbnfrList != NULL)
    {
      if(pbnfrList->nLength > 0)
      {
        tmTails[(pbnfrList->nNumberOfRuleLexem-1)*(nElements)+(pbnfrList->pRule[pbnfrList->nLength-1]-1)] = true;
        fRet = AddNumber(&pbnfrTails, pbnfrList->nNumberOfRuleLexem, pbnfrList->pRule[pbnfrList->nLength-1]);
        fResult = fResult || fRet;
      }
      pbnfrList = pbnfrList->pbnfrNext;
    }
  }
  if(pbnfrTails == NULL)
    return false;

  //udpate (find LAST+)
  //tails rules enumeration
  pbnfrList = pbnfrTails;
  while(pbnfrList != NULL)
  {
    //tails rule value enumeration
    for(nIndex = 0; nIndex < pbnfrList->nLength; ++nIndex)
    {
      //tails rules enumeration
      pbnfrInterList = pbnfrTails;
      while(pbnfrInterList != NULL)
      {
        if(pbnfrInterList->nNumberOfRuleLexem != pbnfrList->nNumberOfRuleLexem &&
           pbnfrList->pRule[nIndex] == pbnfrInterList->nNumberOfRuleLexem)
        {
          //internal rule values enumeration
          for(nInterIndex = 0; nInterIndex < pbnfrInterList->nLength; ++nInterIndex)
          {
            tmTails[(pbnfrList->nNumberOfRuleLexem-1)*nElements+pbnfrInterList->pRule[nInterIndex]-1] = true;
            fRet = AddNumber(&pbnfrTails, pbnfrList->nNumberOfRuleLexem, pbnfrInterList->pRule[nInterIndex]);
            fResult = fResult || fRet;
          }
          break;
        }
        pbnfrInterList = pbnfrInterList->pbnfrNext;
      }
    }
    pbnfrList = pbnfrList->pbnfrNext;
  }

  return fResult;
}

void PrecedenceMatrix::AddBounds(void)
{
  int nIndex, nInterIndex;

  nInterIndex = GetLexemNumber("#", 1);
  if(nInterIndex > 0)
  {
    for(nIndex = 0; nIndex < nElements; ++nIndex)
    {
      if(nInterIndex != (nIndex+1))
        tmPrecedenceMatrix[nIndex*nElements + nInterIndex - 1] = LASTX;
    }
  }

  nInterIndex = GetLexemNumber("#", 1);
  if(nInterIndex > 0)
  {
    for(nIndex = 0; nIndex < nElements; ++nIndex)
    {
      if(nInterIndex != (nIndex+1))
        tmPrecedenceMatrix[(nInterIndex-1)*nElements + nIndex] = FIRSTX;
    }
  }
}

bool PrecedenceMatrix::AddNumber(BNFRule **in_pbnfrPtr, int in_nUpdatingRuleNumber, int in_nIncludeNumber)
{
  int *pNewRule;
  int nRuleIndex;
  int nIncludeNumber[1];
  BNFRule *pbnfrList, *pbnfrNewRule;

  nIncludeNumber[0] = in_nIncludeNumber;
  if(*in_pbnfrPtr == NULL)
  {
    pbnfrNewRule = new BNFRule(in_nUpdatingRuleNumber, nIncludeNumber, 1);
    *in_pbnfrPtr = pbnfrNewRule;
    return true;
  }

  pbnfrList = *in_pbnfrPtr;
  while(pbnfrList != NULL)
  {
    if(pbnfrList->nNumberOfRuleLexem == in_nUpdatingRuleNumber)
    {
      //if current rule is empty
      if(pbnfrList->pRule == NULL)
      {
        pbnfrList->pRule = new int[1];
        pbnfrList->pRule[0] = in_nIncludeNumber;
        return true;
      }
      //if current rule is not empty, try to find the dup
      for(nRuleIndex = 0; nRuleIndex < pbnfrList->nLength; ++nRuleIndex)
      {
        if(pbnfrList->pRule[nRuleIndex] == in_nIncludeNumber)
          return false;
      }
      //if there is no dup in current rule - add new index to one
      pNewRule = new int[pbnfrList->nLength + 1];
      for(nRuleIndex = 0; nRuleIndex < pbnfrList->nLength; ++nRuleIndex)
      {
        pNewRule[nRuleIndex] = pbnfrList->pRule[nRuleIndex];
      }
      pNewRule[pbnfrList->nLength] = in_nIncludeNumber;
      delete [] pbnfrList->pRule;
      pbnfrList->pRule = pNewRule;
      pbnfrList->nLength++;
      return true;
    }
    pbnfrList = pbnfrList->pbnfrNext;
  }
  //if there is no rule to update - add new one
  pbnfrList = *in_pbnfrPtr;
  while(pbnfrList->pbnfrNext != NULL)
  {
    pbnfrList = pbnfrList->pbnfrNext;
  }
  pbnfrNewRule = new BNFRule(in_nUpdatingRuleNumber, nIncludeNumber, 1);
  pbnfrList->pbnfrNext = pbnfrNewRule;

  return true;
}

bool PrecedenceMatrix::SubNumber(BNFRule **in_pbnfrPtr, int in_nUpdatingRuleNumber, int in_nIncludeNumber)
{
  int *pNewRule;
  //int nIncludeNumber[1];
  int nRuleIndex, nSubIndex;
  BNFRule *pbnfrList;

  //nIncludeNumber[0] = in_nIncludeNumber;
  if(*in_pbnfrPtr == NULL)
    return false;

  pbnfrList = *in_pbnfrPtr;
  while(pbnfrList != NULL)
  {
    if(pbnfrList->nNumberOfRuleLexem == in_nUpdatingRuleNumber)
    {
      //if current rule is empty
      if(pbnfrList->pRule == NULL || pbnfrList->nLength < 1)
      {
        return false;
      }
      //if the length is 1
      if(pbnfrList->nLength == 1)
      {
        if(pbnfrList->pRule[0] == in_nIncludeNumber)
        {
          delete pbnfrList->pRule;
          pbnfrList->pRule = NULL;
          pbnfrList->nLength = 0;
          return true;
        }
        return false;
      }

      //not empty
      for(nRuleIndex = 0; nRuleIndex < pbnfrList->nLength; ++nRuleIndex)
      {
        if(pbnfrList->pRule[nRuleIndex] == in_nIncludeNumber)
          break;
      }
      //if there is no in_nIncludeNumber lexem in this rule
      if(nRuleIndex >= pbnfrList->nLength)
        return false;
      pNewRule = new int[pbnfrList->nLength - 1];
      for(nRuleIndex = 0, nSubIndex = 0; nRuleIndex < pbnfrList->nLength; ++nRuleIndex)
      {
        if(pbnfrList->pRule[nRuleIndex] != in_nIncludeNumber)
        {
          pNewRule[nSubIndex] = pbnfrList->pRule[nRuleIndex];
          nSubIndex++;
        }
      }
      delete pbnfrList->pRule;
      pbnfrList->pRule = pNewRule;
      pbnfrList->nLength = pbnfrList->nLength - 1;
      return true;
    }
    pbnfrList = pbnfrList->pbnfrNext;
  }

  return false;
}

bool PrecedenceMatrix::IsTerminal(int in_nNumber)
{
  BNFLexem const *pbnflList;

  if(pbnflLexems == NULL)
    return false;

  pbnflList = pbnflLexems;
  while(pbnflList != NULL)
  {
    if(pbnflList->nNumberOfLexem == in_nNumber)
    {
      return pbnflList->fTerminal;
    }
    pbnflList = pbnflList->pbnflNext;
  }

  return true;
}

int PrecedenceMatrix::GetLexemNumber(char const * const in_pValue, int in_nLength)
{
  BNFLexem const *pbnflList;

  pbnflList = pbnflLexems;
  while(pbnflList != NULL)
  {
    if(in_nLength == pbnflList->nLength && memcmp(in_pValue, pbnflList->pValue, in_nLength) == 0)
      return pbnflList->nNumberOfLexem;
    pbnflList = pbnflList->pbnflNext;
  }
  return -1;
}

void PrecedenceMatrix::PrintList(BNFRule const * const in_pbnfrPtr, char const * const in_pComment)
{
  int nIndex;
  BNFRule const *pbnfrList;

  if(in_pComment != NULL)
    fprintf(stderr, "%s", in_pComment);
  pbnfrList = in_pbnfrPtr;
  while(pbnfrList != NULL)
  {
    fprintf(stderr, "%d: ", pbnfrList->nNumberOfRuleLexem);
    for(nIndex = 0; nIndex < pbnfrList->nLength; ++nIndex)
    {
      fprintf(stderr, "%d,", pbnfrList->pRule[nIndex]);
    }
    fprintf(stderr, "\n");
    pbnfrList = pbnfrList->pbnfrNext;
  }
  fprintf(stderr, "\n");
}

//*****************************************SemanticFunctions*******************************************//

SemanticFunctions::SemanticFunctions()
{
  pszFunctionName = NULL;
  pfSemanticFunction = NULL;

  psfNext = NULL;
}

SemanticFunctions::SemanticFunctions(const char * const in_pszFunctionName, void (*in_pfSemanticFunction)(StackValue *))
{
  int nLength;

  pszFunctionName = NULL;
  pfSemanticFunction = NULL;

  psfNext = NULL;

  if(in_pszFunctionName == NULL || in_pfSemanticFunction == NULL)
    return;

  nLength = strlen(in_pszFunctionName);
  if(nLength > 0)
  {
    pszFunctionName = new char[nLength+1];
    memset(pszFunctionName, 0, nLength+1);
    strncpy(pszFunctionName, in_pszFunctionName, nLength);

    pfSemanticFunction = in_pfSemanticFunction;
  }
}

SemanticFunctions::~SemanticFunctions()
{
  if(pszFunctionName != NULL)
  {
    delete [] pszFunctionName;
    pszFunctionName = NULL;
  }
}
