//SimpleGrammar.cpp
//Author: Bombo
//20.03.2017
//Конструктор внутреннего представления грамматик

#include"SimpleGrammar.h"

SimpleGrammar::SimpleGrammar()
{
    pCharTable = pGrammTable;
}

SimpleGrammar::~SimpleGrammar()
{
}

ScanResult *SimpleGrammar::Scan()
{
  SingleSymbol sType, sTypePrev, sTypeExtra;
  SymbolsList slSymbols, slSymbolsExtra;

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
    switch((int)sType.ctType)
    {
      case ED:
        //return END symbol
        psrLastScanResult = new ScanResult("END\t");
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
        fprintf(stderr, "SimpleGrammar::Scan(), unknown symbol: '%c'\n", sType.chSymbol);
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
        //check if this delimiter is special
        if(sType.chSymbol == ':')
        {
          sTypeExtra.chSymbol = sType.chSymbol;
          sTypeExtra.ctType = sType.ctType;

          slSymbolsExtra.Clean();
          slSymbolsExtra.AddSymbol(&sTypeExtra);

          GetCharType(&sTypeExtra, true);
          if(sTypeExtra.chSymbol == ':')
          {
            slSymbolsExtra.AddSymbol(&sTypeExtra);
            GetCharType(&sTypeExtra, true);
            if(sTypeExtra.chSymbol == '=')
            {
              psrLastScanResult = new ScanResult("RULE\t");
              slSymbolsExtra.AddSymbol(&sTypeExtra);
              slSymbolsExtra.SetResultValue(psrLastScanResult);
              IncrementPosition();
              return psrLastScanResult;
            }
            DecrementPosition();
          }
          DecrementPosition();
        }

        //get rule name (if it is so)
        if(sType.chSymbol == '<')
        {
          GetCharType(&sType, true);
          while(sType.chSymbol != '>')
          {
            slSymbols.AddSymbol(&sType);
            GetCharType(&sType, true);
          }
          psrLastScanResult = new ScanResult("RULE_NAME\t");
          slSymbols.SetResultValue(psrLastScanResult);
          IncrementPosition();
          return psrLastScanResult;
        }

        //get the end of rule (if it is so)
        if(sType.chSymbol == '\n')
        {
          psrLastScanResult = new ScanResult("RULE_END\t");
          slSymbols.AddSymbol(' ', DL);
          slSymbols.SetResultValue(psrLastScanResult);
          IncrementPosition();
          return psrLastScanResult;
        }

        //get next rule
        if(sType.chSymbol == '|')
        {
          psrLastScanResult = new ScanResult("NEXT\t");
          slSymbols.AddSymbol(&sType);
          slSymbols.SetResultValue(psrLastScanResult);
          IncrementPosition();
          return psrLastScanResult;
        }

        //send this DLM value
        //set WRD type to output
        psrLastScanResult = new ScanResult("DELIMITER\t");
        //copy string to output result
        slSymbols.AddSymbol(&sType);
        //index increment
        IncrementPosition();
        //copy value to output
        slSymbols.SetResultValue(psrLastScanResult);
        return psrLastScanResult;
//      case QU: //quotes
//        //check if the next symbol is quotes; yes - out, no - continue
//        //get next symbol
//        GetCharType(&sType, true);
//        sTypePrev.chSymbol = sType.chSymbol;
//        sTypePrev.ctType = sType.ctType;
//        while( !(sType.ctType == QU && sTypePrev.chSymbol != '\\') )
//        {
//          //save current symbol
//          slSymbols.AddSymbol(&sType);
//          //save previous symbol
//          sTypePrev.chSymbol = sType.chSymbol;
//          sTypePrev.ctType = sType.ctType;
//          //get next symbol
//          GetCharType(&sType, true);
//        }
//        //save string
//        slSymbols.SetResultValue(psrResult);
//        psrResult = new ScanResult("STRING\t");
//        IncrementPosition();
//        return psrResult;
//      case SQ: //single quote
//        //check if the next symbol is single quote; yes - out, no - continue
//        //get next symbol
//        GetCharType(&sType, true);
//        sTypePrev.chSymbol = sType.chSymbol;
//        sTypePrev.ctType = sType.ctType;
//        while( !(sType.ctType == SQ && sTypePrev.chSymbol != '\\') )
//        {
//          //save current symbol
//          slSymbols.AddSymbol(&sType);
//          //save prev symbol
//          sTypePrev.chSymbol = sType.chSymbol;
//          sTypePrev.ctType = sType.ctType;
//          //get next symbol
//          GetCharType(&sType, true);
//        }
//        //save string
//        slSymbols.SetResultValue(psrResult);
//        psrResult = new ScanResult("STRING\t");
//        IncrementPosition();
//        return psrResult;
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

//    psrLastScanResult = new ScanResult("ERROR");
//    return psrLastScanResult; //error: wrong semantic symbol

  if(psrLastScanResult != NULL)
  {
    delete psrLastScanResult;
    psrLastScanResult = NULL;
  }
  return NULL;
}
