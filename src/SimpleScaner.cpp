//SimpleScaner.cpp
//Author: Bombo
//14.03.2017
//Сканер (лексический анализатор)

#include"SimpleScaner.h"

//*****************************************SimpleScaner******************************************//

SimpleScaner::SimpleScaner()
{
    pszFile = NULL;
    fdFile = -1;
    pszBuff = NULL;
    lIndex = -1;
    lBuffLen = 0;
    pCharTable = pBuiltInCharTable;
    pAltCharTable = NULL;
    psrLastScanResult = NULL;
}

SimpleScaner::~SimpleScaner()
{
    if(pszFile != NULL)
        delete [] pszFile;
    if(fdFile >= 0)
        close(fdFile);
    if(pszBuff != NULL)
        delete [] pszBuff;
    if(pAltCharTable != NULL)
      delete [] pAltCharTable;
    if(psrLastScanResult != NULL)
    {
      delete psrLastScanResult;
      psrLastScanResult = NULL;
    }
}

//this method does not modify alternative char table
bool SimpleScaner::OpenFile(char const * const in_pszFile)
{
    size_t stLen;

    if(pszFile != NULL)
    {
        delete [] pszFile;
        pszFile = NULL;
    }
    if(fdFile >= 0)
    {
        close(fdFile);
        fdFile = -1;
    }
    if(pszBuff != NULL)
    {
        delete [] pszBuff;
        pszBuff = NULL;
        lIndex = -1;
        lBuffLen = 0;
    }

    if(in_pszFile == NULL || (stLen = strlen(in_pszFile)) == 0)
      return false;
    //copy file name, open file
    if( (fdFile = open(in_pszFile, O_RDONLY)) >= 0 )
    {
      pszFile = new char[stLen + 1];
      memset(pszFile, 0, stLen);
      strncpy(pszFile, in_pszFile, stLen);
    }
    else
      return false;

    return true;
}

//this method does not modify alternative char table
bool SimpleScaner::ReadBuff(char const * const in_pszBuff)
{
    size_t stLen;

    if(pszFile != NULL)
    {
        delete [] pszFile;
        pszFile = NULL;
    }
    if(fdFile >= 0)
    {
        close(fdFile);
        fdFile = -1;
    }
    if(pszBuff != NULL)
    {
        delete [] pszBuff;
        pszBuff = NULL;
        lIndex = -1;
        lBuffLen = 0;
    }

    //check buff
    if( in_pszBuff == NULL || (stLen = strlen(in_pszBuff)) == 0 )
        return false;

    //copy buff
    pszBuff = new char[stLen + 1];
    memset(pszBuff, 0, stLen + 1);
    strncpy(pszBuff, in_pszBuff, stLen);
    lBuffLen = stLen;
    lIndex = -1;

    return true;
}

//create alternative char table
bool SimpleScaner::ReadNewTable(char const * const in_szPathToTable)
{
    if(in_szPathToTable == NULL || strlen(in_szPathToTable) == 0)
      return false;

    //read alternative character-types table from file
    //...

    return true;
}

//the main function
//the result of this method is needed to be deleted outside
ScanResult *SimpleScaner::Scan(void)
{
    SingleSymbol sType, sTypePrev;
    SymbolsList slSymbols;

    if(psrLastScanResult != NULL)
    {
      delete psrLastScanResult;
      psrLastScanResult = NULL;
    }

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
//{ED = -2, ER = -1, UD = 0, LT, DG, LD, DL, QU, SQ, SL, BL};
    GetCharType(&sType, false);
    for(;;)
    {
      switch(sType.ctType)
      {
        case ED:
          //return END symbol
          psrLastScanResult = new ScanResult("END\t");
          psrLastScanResult->AddValue("END", 3);
          return psrLastScanResult;
          break;
        case ER:
          //return ERR symbol
          break;
        case UD:
          slSymbols.AddSymbol(&sType);
          psrLastScanResult = new ScanResult("UNKNOWN\t");
          slSymbols.SetResultValue(psrLastScanResult);
          IncrementPosition();
          fprintf(stderr, "SimpleScaner::Scan(), unknown symbol: '%c'\n", sType.chSymbol);
          _exit(0);
          //return UNK symbol
          return psrLastScanResult;
        case LT:
          //collect all LT+LD to string, while not DL or something else
          do {
            //add new symbol to temporary list
            slSymbols.AddSymbol(&sType);
            //get next symbol+type
            GetCharType(&sType, true);
          }
          while(sType.ctType == LT || sType.ctType == DG); //while letter or digit
          //get WRD type
          //...
          //set WRD to output
          psrLastScanResult = new ScanResult("LEXEM\t");
          //copy string to output result
          slSymbols.SetResultValue(psrLastScanResult);
          return psrLastScanResult;
        case DG:
          //check of the next symbol is DG; in yes case, continue, in no - out
          do {
            //add new digit to temporary list
            slSymbols.AddSymbol(&sType);
            //get next symbol
            GetCharType(&sType, true);
          }
          while(sType.ctType == DG);
          //get WRD type
          //...
          //set WRD to output
          psrLastScanResult = new ScanResult("DIGIT\t");
          //copy string to output result
          slSymbols.SetResultValue(psrLastScanResult);
          return psrLastScanResult;
        case LD:
          break;
        case DL:
          //send this DLM value
          //get WRD type
          //...
          //set WRD type to output
          psrLastScanResult = new ScanResult("DELIMITER\t");
          //copy string to output result
          slSymbols.AddSymbol(&sType);
          //index increment
          IncrementPosition();
          //copy value to output
          slSymbols.SetResultValue(psrLastScanResult);
          return psrLastScanResult;
        case QU: //quotes
          //check if the next symbol is quotes; yes - out, no - continue
          //get next symbol
          GetCharType(&sType, true);
          sTypePrev.chSymbol = sType.chSymbol;
          sTypePrev.ctType = sType.ctType;
          while( !(sType.ctType == QU && sTypePrev.chSymbol != '\\') )
          {
            //save current symbol
            slSymbols.AddSymbol(&sType);
            //save previous symbol
            sTypePrev.chSymbol = sType.chSymbol;
            sTypePrev.ctType = sType.ctType;
            //get next symbol
            GetCharType(&sType, true);
          }
          //save string
          psrLastScanResult = new ScanResult("DSTRING\t");
          slSymbols.SetResultValue(psrLastScanResult);
          IncrementPosition();
          return psrLastScanResult;
        case SQ: //single quote
          //check if the next symbol is single quote; yes - out, no - continue
          //get next symbol
          GetCharType(&sType, true);
          sTypePrev.chSymbol = sType.chSymbol;
          sTypePrev.ctType = sType.ctType;
          while( !(sType.ctType == SQ && sTypePrev.chSymbol != '\\') )
          {
            //save current symbol
            slSymbols.AddSymbol(&sType);
            //save prev symbol
            sTypePrev.chSymbol = sType.chSymbol;
            sTypePrev.ctType = sType.ctType;
            //get next symbol
            GetCharType(&sType, true);
          }
          //save string
          psrLastScanResult = new ScanResult("SSTRING\t");
          slSymbols.SetResultValue(psrLastScanResult);
          IncrementPosition();
          return psrLastScanResult;
        case SL:
          //check if the next symbol is '/' or '*'
          //if yes - skip all symbols until '\n' or "*/" is met
          GetCharType(&sType, true);

          // on // ...
          if(sType.chSymbol == '/')
          {
            GetCharType(&sType, true);
            while(sType.chSymbol != '\n' && sType.chSymbol != '\r')
            {
              slSymbols.AddSymbol(&sType);
              GetCharType(&sType, true);
            }
            psrLastScanResult = new ScanResult("COMMENT\t");
            slSymbols.SetResultValue(psrLastScanResult);
            IncrementPosition();
            return psrLastScanResult;
          }

          // on /* ... */
          if(sType.chSymbol == '*')
          {
            sTypePrev.chSymbol = '\0';
            sTypePrev.ctType = UD;
            GetCharType(&sType, true);
            if(sType.chSymbol != '*')
              slSymbols.AddSymbol(&sType);
            while( !(sType.chSymbol == '/' && sTypePrev.chSymbol == '*') )
            {
              sTypePrev.chSymbol = sType.chSymbol;
              sTypePrev.ctType = sType.ctType;
              GetCharType(&sType, true);
              if(sType.chSymbol == '*' || (sType.chSymbol == '/' && sTypePrev.chSymbol == '*'))
                continue;
              if(sTypePrev.chSymbol == '*')
                slSymbols.AddSymbol(&sTypePrev);
              slSymbols.AddSymbol(&sType);
            }
            psrLastScanResult = new ScanResult("COMMENT\t");
            slSymbols.SetResultValue(psrLastScanResult);
            IncrementPosition();
            return psrLastScanResult;
          }

          //if no - send '/' DLM value
          psrLastScanResult = new ScanResult("DELIMITER\t");
          psrLastScanResult->AddValue("/", 1);
          return psrLastScanResult;
        case BL:
          //continue scanning until not blank symbol is met
          GetCharType(&sType, true);
          continue;
      }
      break;
    }

//    psrLastScanResult->AddName("ERROR");
//    return psrLastScanResult; //error: wrong semantic symbol
    if(psrLastScanResult != NULL)
    {
      delete psrLastScanResult;
      psrLastScanResult = NULL;
    }

    return NULL;
}

//real scan
bool SimpleScaner::NextScan(ScanResult ** out_psrResult)
{
    if(out_psrResult != NULL)
      *out_psrResult = Scan();
    else
      Scan();

    if(psrLastScanResult == NULL)
      return false;

    if( (psrLastScanResult) != NULL && ((psrLastScanResult)->pszName != NULL && strncmp((psrLastScanResult)->pszName, "END\t", 4) != 0) )
      return true;

    return false;
}

CharacterType SimpleScaner::GetCharType(bool in_fIncrementIndex)
{
    CharacterType const *pcTablePtr;
    CharacterType ctType;
    char chOneChar;
    size_t stRet;

    ctType = ER;

    //characters table select
    if(pAltCharTable == NULL)
      pcTablePtr = pCharTable;
    else
      pcTablePtr = pAltCharTable;

    //check if the buffer is ready
    if(pszBuff != NULL && lBuffLen > 0)
    {
        //reading from buffer
        if(lIndex >= lBuffLen)
          return ED;
        if(in_fIncrementIndex)
          lIndex++;
        if(lIndex < 0)
          lIndex = 0;
        ctType = pcTablePtr[(int)pszBuff[lIndex]];
    }
    else if(fdFile >= 0)
    {
        //reading from file
        chOneChar = '\0';
        stRet = read(fdFile, &chOneChar, 1);
        if(stRet <= 0)
          return ED;
        if(!in_fIncrementIndex)
          lseek(fdFile, -1, SEEK_CUR);
        ctType = pcTablePtr[(int)chOneChar];
    }

    return ctType;
}

void SimpleScaner::GetCharType(SingleSymbol * const out_psSymbol, bool in_fIncrementIndex)
{
    CharacterType const *pcTablePtr;
    size_t stRet;

    if(out_psSymbol == NULL)
      return;

    out_psSymbol->chSymbol = -1;
    out_psSymbol->ctType = ER;

    //characters table select
    if(pAltCharTable == NULL)
      pcTablePtr = pCharTable;
    else
      pcTablePtr = pAltCharTable;

    //check if the buffer is ready
    if(pszBuff != NULL && lBuffLen > 0)
    {
      //reading for buffer
      if(lIndex >= lBuffLen)
      {
        out_psSymbol->ctType = ED;
        return;
      }
      if(in_fIncrementIndex)
        lIndex++;
      if(lIndex < 0)
        lIndex = 0;
      out_psSymbol->chSymbol = pszBuff[lIndex];
      out_psSymbol->ctType = pcTablePtr[(int)pszBuff[lIndex]];
    }
    else if(fdFile >= 0)
    {
      //reading from file
      out_psSymbol->chSymbol = '\0';
      stRet = read(fdFile, &(out_psSymbol->chSymbol), 1);
      if(stRet <= 0)
      {
        out_psSymbol->ctType = ED;
        return;
      }
      if(!in_fIncrementIndex)
        lseek(fdFile, -1, SEEK_CUR);
      out_psSymbol->ctType = pcTablePtr[(int) out_psSymbol->chSymbol];
    }
}

void SimpleScaner::IncrementPosition(void)
{
    if(pszBuff != NULL && lBuffLen > 0 && lIndex < lBuffLen)
    {
      lIndex++;
      if(lIndex < 0)
        lIndex = 0;
    }
    else if(fdFile >= 0)
    {
      lseek(fdFile, 1, SEEK_CUR);
    }
}

void SimpleScaner::DecrementPosition(void)
{
    if(pszBuff != NULL && lBuffLen > 0 && lIndex < lBuffLen && lIndex > 0)
    {
      lIndex--;
    }
    else if(fdFile >= 0)
    {
      lseek(fdFile, -1, SEEK_CUR);
    }
}

//scan (debugging)
void SimpleScaner::MakeScan(void)
{
    Scan();
    while( psrLastScanResult != NULL && (psrLastScanResult->pszName != NULL && strncmp(psrLastScanResult->pszName, "END\t", 4) != 0) )
    {
        psrLastScanResult->Print();
        Scan();
    }
}

void SimpleScaner::PrintBuff(void)
{
    if(pszBuff == NULL || strlen(pszBuff) == 0)
      return;

    fprintf(stderr, "%s\n\n", pszBuff);
}

//******************************************ScanResult*******************************************//

ScanResult::ScanResult()
{
    pszName = NULL;
    pbValue = NULL;
    lSize = 0;
}

ScanResult::ScanResult(const char * const in_pszName)
{
    pszName = NULL;
    pbValue = NULL;
    lSize = 0;

    AddName(in_pszName);
}

ScanResult::ScanResult(const ScanResult &in_srResult)
{
    AddName(in_srResult.pszName);
    AddValue(in_srResult.pbValue, in_srResult.lSize);
}

ScanResult::ScanResult(const ScanResult *in_psrResult)
{
    AddName(in_psrResult->pszName);
    AddValue(in_psrResult->pbValue, in_psrResult->lSize);
}

ScanResult::ScanResult(const char * const in_pszName, const char * const in_pbValue, long in_lSize)
{
    pszName = NULL;
    pbValue = NULL;
    lSize = 0;

    AddName(in_pszName);
    if(pszName != NULL)
      AddValue(in_pbValue, in_lSize);
}

ScanResult::~ScanResult()
{
    if(pszName != NULL)
    {
      delete [] pszName;
      pszName = NULL;
    }
    if(pbValue != NULL)
    {
      delete [] pbValue;
      pbValue = NULL;
    }
}

ScanResult& ScanResult::operator =(const ScanResult& in_srResult)
{
  AddName(in_srResult.pszName);
  AddValue(in_srResult.pbValue, in_srResult.lSize);

  return *this;
}

ScanResult& ScanResult::operator =(const ScanResult *in_psrResult)
{
  if(in_psrResult == NULL)
    return *this;

  *this = *in_psrResult;

  return *this;
}

void ScanResult::AddName(char const * const in_pszName)
{
    size_t stLen;

    if(pszName != NULL)
    {
      delete [] pszName;
      pszName = NULL;
    }

    if(in_pszName == NULL || (stLen = strlen(in_pszName)) == 0)
      return;

    pszName = new char[stLen + 1];
    memset(pszName, 0, stLen + 1);
    strncpy(pszName, in_pszName, stLen);
}

void ScanResult::AddValue(const char * const in_pbValue, long in_lSize)
{
    if(pbValue != NULL)
    {
      delete [] pbValue;
      pbValue = NULL;
    }
    lSize = 0;

    if(in_pbValue == NULL || in_lSize == 0)
      return;

    pbValue = new char[in_lSize + 1];
    memset(pbValue, 0, in_lSize + 1);
    memcpy(pbValue, in_pbValue, in_lSize);
    lSize = in_lSize;
}

bool ScanResult::NameCmp(const char * const in_pszName)
{
  int nNameIndex, nName1Len, nName2Len;

  if(in_pszName == NULL)
    return false;
  nName1Len = strlen(in_pszName);
  nName2Len = strlen(pszName);
  if(nName1Len != nName2Len)
    return false;
  for(nNameIndex = 0; nNameIndex < nName1Len; ++nNameIndex)
  {
    if(pszName[nNameIndex] != in_pszName[nNameIndex])
      return false;
  }
  return true;
}

void ScanResult::Print(void)
{
    if(pszName != NULL)
      fprintf(stderr, "%s\t", pszName);
    if(pbValue != NULL)
      fprintf(stderr, "%s\n", pbValue);
    else
      fprintf(stderr, "\n");
}

//******************************************SingleSymbol*****************************************//

SingleSymbol::SingleSymbol()
{
    chSymbol = -1;
    ctType = UD;
    psNext = NULL;
}

SingleSymbol::SingleSymbol(char in_chSymbol)
{
    chSymbol = in_chSymbol;
    ctType = UD;
    psNext = NULL;
}

SingleSymbol::SingleSymbol(char in_chSymbol, CharacterType in_ctType)
{
    chSymbol = in_chSymbol;
    ctType = in_ctType;
    psNext = NULL;
}

//******************************************SymbolsList******************************************//

SymbolsList::SymbolsList()
{
    psFirst = NULL;
}

SymbolsList::SymbolsList(char in_chSymbol)
{
    psFirst = new SingleSymbol(in_chSymbol);
}

SymbolsList::SymbolsList(char in_chSymbol, CharacterType in_ctType)
{
    psFirst = new SingleSymbol(in_chSymbol, in_ctType);
}

SymbolsList::~SymbolsList()
{
    SingleSymbol *psList;

    if(psFirst != NULL)
    {
      psList = psFirst;
      while(psList != NULL)
      {
        psList = psFirst->psNext;
        delete psFirst;
        psFirst = psList;
      }
    }
}

void SymbolsList::AddSymbol(char in_chSymbol)
{
    SingleSymbol *psList;

    if(psFirst == NULL)
    {
      psFirst = new SingleSymbol(in_chSymbol);
      return;
    }

    psList = psFirst;
    while(psList->psNext != NULL)
    {
        psList = psList->psNext;
    }

    psList->psNext = new SingleSymbol(in_chSymbol);
}

void SymbolsList::AddSymbol(char in_chSymbol, CharacterType in_ctType)
{
    SingleSymbol *psList;

    if(psFirst == NULL)
    {
      psFirst= new SingleSymbol(in_chSymbol, in_ctType);
      return;
    }

    psList = psFirst;
    while(psList->psNext != NULL)
    {
        psList = psList->psNext;
    }

    psList->psNext = new SingleSymbol(in_chSymbol, in_ctType);
}

void SymbolsList::AddSymbol(SingleSymbol const * const in_psSymbol)
{
    AddSymbol(in_psSymbol->chSymbol, in_psSymbol->ctType);
}

void SymbolsList::Clean(void)
{
    SingleSymbol *psList;

    if(psFirst == NULL)
      return;
    psList = psFirst;
    while(psFirst != NULL)
    {
      psFirst = psFirst->psNext;
      delete psList;
      psList = psFirst;
    }
}

//the result of this method is needed to be deleted outside
char *SymbolsList::GetString(void)
{
    SingleSymbol *psList;
    char *pcResult;
    long lIndex;
    long lLen;

    if(psFirst == NULL)
      return NULL;

    lLen = 0;
    psList = psFirst;
    while(psList != NULL)
    {
      lLen++;
      psList = psList->psNext;
    }
    pcResult = new char[lLen + 1];
    if(pcResult == NULL)
      return NULL;
    memset(pcResult, 0, lLen + 1);
    lIndex = 0;
    psList = psFirst;
    while(psList != NULL && lIndex < lLen) //double check
    {
      pcResult[lIndex] = psList->chSymbol;
      lIndex++;
      psList = psFirst->psNext;
    }
    return pcResult;
}

void SymbolsList::SetResultValue(ScanResult * const out_psrResult)
{
    SingleSymbol *psList;
    char *pcResult;
    long lIndex;
    long lLen;


    if(psFirst == NULL || out_psrResult == NULL)
      return;

    lLen = 0;
    psList = psFirst;
    while(psList != NULL)
    {
      lLen++;
      psList = psList->psNext;
    }

    lIndex = 0;
    pcResult = new char[lLen + 1];
    if(pcResult == NULL)
      return;
    memset(pcResult, 0, lLen + 1);
    psList = psFirst;
    while(psList != NULL && lIndex < lLen)
    {
      pcResult[lIndex] = psList->chSymbol;
      lIndex++;
      psList = psList->psNext;
    }

    out_psrResult->AddValue(pcResult, lLen);
    delete [] pcResult;
}
