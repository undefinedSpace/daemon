//DirSnapshot.cpp
//Author: Bombo
//12.01.2017
//Класс DirSnapshot является "слепком" каталога

#include"DirSnapshot.h"
#include"SomeDirectory.h"
#include"RootMonitor.h"

/*************************************DirSnapshot**********************************/

DirSnapshot::DirSnapshot()
{
    pfdFirst = new FileData();
}

//функция не подходит для создания конечного слепка директории, т.к.
//ненайденным директориям не задаются родительские
DirSnapshot::DirSnapshot(char const * const in_pName)
{
    DIR *dFd;
    FileData *pfdFile;
    struct dirent *pdeData;

    pfdFirst = new FileData();

    //тут должен быть автоматически создан весь слепок
    //путём чтения текущей директории
    //параметр in_pName - имя этой директории

    //создаём список файлов (слепок)
    if(in_pName == NULL)
	return;

/*
    dFd = opendir(in_pName);

    if(dFd == NULL)
	return;

    pdeData = readdir(dFd);

    while(pdeData != NULL)
    {
	//исключаем "." и ".."
	if( !((strlen(pdeData->d_name) == 1 && strncmp(pdeData->d_name, ".", 1) == 0) ||
	    (strlen(pdeData->d_name) == 2 && strncmp(pdeData->d_name, "..", 2) == 0)) )
	{
	    pfdFile = AddFile(pdeData->d_name, true); //сразу вычисляем хэш

	    if(pfdFile != NULL)
		fprintf(stderr, "1:%s\t%s  \n", (pfdFile->nType==IS_DIRECTORY)?("DIR"):(""), pdeData->d_name); //отладка!!!

	    //т.к. этим конструктором создаётся слепок исключительно для сравнения с другими,
	    //отслежывать найденные в нём директории не требуется
	}
	pdeData = readdir(dFd);
    }
*/
}

DirSnapshot::DirSnapshot(FileData * const in_pfdParent)
{
    DIR *dFd;
    char *pPath;
    FileData *pfdFile;
    struct dirent *pdeData;

    pfdFirst = new FileData();

    //тут должен быть автоматически создан весь слепок
    //путём чтения текущей директории
    //параметр in_pName - имя этой директории

/*
    //создаём список файлов (слепок)
    dFd = opendir(in_pfdParent->pName);
    if(dFd < 0)
	return;

    pdeData = readdir(dFd);
    while(pdeData != NULL)
    {
	//исключаем "." и ".."
	if( !((strlen(pdeData->d_name) == 1 && strncmp(pdeData->d_name, ".", 1) == 0) ||
	    (strlen(pdeData->d_name) == 2 && strncmp(pdeData->d_name, "..", 2) == 0)) )
	{
	    pfdFile = AddFile(pdeData->d_name, true); //сразу вычисляем хэш

	    if(pfdFile != NULL)
		fprintf(stderr, "2:%s\t%s  \n", (pfdFile->nType==IS_DIRECTORY)?("DIR"):(""), pdeData->d_name); //отладка!!!

	    //если это директория - добавляем в список
	    //ищем полный путь к директории
//	    pPath = in_pfdParent->GetFullPath();
	    if(pPath != NULL)
	    {
//		fprintf(stderr, "path: %s\n", pPath); //отладка!!!
//		delete [] pPath;
	    }
	}
	pdeData = readdir(dFd);
    }
*/
}

DirSnapshot::DirSnapshot(void * const in_psdParent, bool in_fMakeHash, bool in_fUpdateDirList)
{
    DIR *dFd;
    int nFd;
    char *pPath = NULL;
    FileData *pfdFile, *pfdData;
    struct dirent *pdeData;
    SomeDirectory *psdParent, *psdAdd;

    pfdFirst = new FileData();

    //тут должен быть автоматически создан весь слепок
    //путём чтения текущей директории
    //параметр in_pName - имя этой директории

    //создаём список файлов (слепок)

    //получаем переданный параметр - родительскую директорию
    psdParent = (SomeDirectory *)in_psdParent;

    //небольшая проверка
    if(psdParent == NULL || ((pfdData = psdParent->GetFileData())==NULL) || pfdData->nType != IS_DIRECTORY)
    {
	fprintf(stderr, "DirSnapshot::DirSnapshot() Can not create snapshot (file \"%s\" is not a directory)\n", (pfdData==NULL)?"NULL!!!":pfdData->pName);
	return;
    }

    pPath = psdParent->GetFullPath();

    if(in_fUpdateDirList)
    {
      nFd = psdParent->GetDirFd();
      //если директория ещё не открыта - открываем
      if(nFd == -1)
      {
//  	fprintf(stderr, "DirSnapshot::DirSnapshot() открывается этот файл: pPath=\"%s\"\n", pPath); //отладка!!!
	nFd = open(pPath, O_RDONLY);
	switch(errno)
	{
	  case EACCES:
	    psdParent->GetFileData()->nType = IS_NOTAFILE;
	    break;
	}

// 	if(nFd == -1)
// 	  perror("DirSnapshot::DirSnapshot() open directory");

	  //учесть при инкапсуляции (!)
	psdParent->GetFileData()->nDirFd = nFd;
      }
//       fprintf(stderr, "DirSnapshot::DirSnapshot() nFd=%d\n", nFd); //отладка!!!
      dFd = fdopendir(nFd);
      if(dFd == NULL)
      {
// 	fprintf(stderr, "DirSnapshot::DirSnapshot() dFd == NULL\n");
	if(pPath != NULL)
	  delete [] pPath;
	return;
      }
    }
    else
    {
      //тут не требуется переоткрывать директорию, чтобы получить обновлённые данные о ней (!)
      //т.к. эта часть кода выполняется для создания временного списка уже давно открытой директории (!)
      
      //получаем дескриптор директории, для которой собираемся создать временный слепок
      nFd = psdParent->GetDirFd();
      dFd = fdopendir(nFd);
      if(dFd == NULL)
      {
	fprintf(stderr, "DirSnapshot::DirSnapshot() : This directory is closed!!!\n");
	return;
      }
      rewinddir(dFd); //вот что надо было сделать!
    }

    pdeData = readdir(dFd);

    while(pdeData != NULL)
    {
	//исключаем "." и ".."
	if( !((strlen(pdeData->d_name) == 1 && strncmp(pdeData->d_name, ".", 1) == 0) ||
	    (strlen(pdeData->d_name) == 2 && strncmp(pdeData->d_name, "..", 2) == 0)) )
	{
	    pfdFile = AddFile(pdeData->d_name, pPath, in_fMakeHash); //сразу вычисляем хэш, если задано

//  	    if(in_fUpdateDirList)
//  	      fprintf(stderr, "DirSnapshot::DirSnapshot() 3:path: %s, %s\t%s, fd=%d, inode=%d\n", pPath, (pfdFile->nType==IS_DIRECTORY)?("DIR"):(""), pdeData->d_name, pfdFile->nDirFd, (int)pfdFile->stData.st_ino); //отладка!!!

	    if(pfdFile == NULL)
	    {
		pdeData = readdir(dFd);
		continue;
	    }

	    //если это директория - добавляем в список открытых дескрипторов
	    //добавление происходит только при первичном создании слепка
	    if(in_fUpdateDirList && pPath != NULL && pfdFile->nType == IS_DIRECTORY)
	    {
		//возможно, создание этого объекта следует перенести в pdlList->AddQueueElement() (?)
		psdAdd = new SomeDirectory(pfdFile, psdParent, false);
		//непосредственно добавление
		pthread_mutex_lock(&(RootMonitor::mDescListMutex));
		if(RootMonitor::pdlList != NULL)
		    RootMonitor::pdlList->AddQueueElement(psdAdd);
		pthread_mutex_unlock(&(RootMonitor::mDescListMutex));
	    }
	}
	pdeData = readdir(dFd);
    }

    if(pPath != NULL)
        delete [] pPath;

    //освобождение мьютекса потока обработчика списка директорий (?)
    //возможно, лучше это делать после завершения данного метода
    //для избежания рекурсии
    //...
}

DirSnapshot::~DirSnapshot()
{
    SnapshotComparison *pscList;

    //если список файлов пуст - выходим
    if(pfdFirst == NULL)
	return;

    //удаляем элементы списка
    while(pfdFirst->pfdNext != NULL)
    {
	delete pfdFirst->pfdNext;
    }
    //удаляем первый элемент списка
    delete pfdFirst;
    pfdFirst = NULL;

    //удаляем результат сравнения слепков (если он есть)
    if(pscFirst != NULL)
    {
      pscList = GetResult();
      while(pscList != NULL)
      {
	delete pscList;
	pscList = GetResult();
      }
      delete pscFirst;
    }
}

//добавляем файл в список
FileData *DirSnapshot::AddFile(char const * const in_pName, char *in_pPath, bool in_fCalcHash)
{
    struct FileData *pfdList;
    char *pFullPath;
    size_t stName, stPath;
    struct stat sBuff;

    //если имя файла указано неверно - выходим
    if(in_pName == NULL || strlen(in_pName) <= 0)
	return NULL;

    //если список ещё пуст
    if(pfdFirst == NULL)
	pfdFirst = new FileData();

    //ищем последний элемент списка
    pfdList = pfdFirst;
    while(pfdList->pfdNext != NULL)
    {
	pfdList = pfdList->pfdNext;
    }

    //проверяем, есть ли доступ к файлу
    stName = strlen(in_pName);
    if(in_pPath != NULL)
      stPath = strlen(in_pPath);
    else
      stPath = 0;
    pFullPath = new char[stName + stPath + 2]; // 1 для '/' и 1 для '\0'
    memset(pFullPath, 0, stName+stPath+2);
    if(stPath > 0)
    {
      strncpy(pFullPath, in_pPath, stPath+stName+1);
      strncat(pFullPath, "/", stPath+stName+1);
    }
    strncat(pFullPath, in_pName, stPath+stName+1);

    //при успехе доступа к файлу добавляем его в список
    if(stat(pFullPath, &sBuff) == 0)
    {
      //добавляем файл в конец списка
      pfdList->pfdNext = new FileData(in_pName, in_pPath, pfdList, in_fCalcHash);
      delete [] pFullPath;
      return (pfdList->pfdNext);
    }
    else
    {
//       fprintf(stderr, "DirSnapshot::AddFile() : \"%s\"\n", pFullPath); //отладка!!!
      delete [] pFullPath;
      return NULL;
    }
}

//добавляем файл в список
FileData *DirSnapshot::AddFile(FileData const * const in_pfdFile, char const * const in_pPath, bool in_fCalcHash)
{
    struct FileData *pfdList;
    FileData *pfdCopy;

    //если список ещё пуст
    if(pfdFirst == NULL)
	pfdFirst = new FileData();

    //проверка
    if(in_pPath == NULL)
      return NULL;

    //ищем последний элемент списка
    pfdList = pfdFirst;
    while(pfdList->pfdNext != NULL)
    {
	pfdList = pfdList->pfdNext;
    }

    //тута надо сделать копию (!)
    pfdCopy = new FileData(in_pfdFile, in_pPath, true);
	 
    //добавляем файл в конец списка
    pfdList->pfdNext = pfdCopy;

    pfdCopy->pfdPrev = pfdList;
    pfdCopy->pfdNext = NULL;

    return pfdList->pfdNext;
}

//удаляем файл из слепка
//можно удалять по одному только имени, т.к. два файла с одинаковыми именами в одной
//директории находиться не могут
void DirSnapshot::SubFile(char const * const in_pName)
{
    struct FileData *pfdList;

    if(pfdFirst == NULL)
	return;

    if(in_pName == NULL || strlen(in_pName) <= 0)
	return;

    //ищем файл в списке по указанному имени
    pfdList = pfdFirst;
    while(pfdList != NULL && strcmp(pfdList->pName, in_pName) != 0)
    {
	pfdList = pfdList->pfdNext;
    }

    //если такой файл не найден в списке - выходим
    if(pfdList == NULL)
	return;

    //удаляем файл из списка
    delete pfdList;
}

//требуется переработать для поиска всех отличий сразу, а не по одному за вызов (!)
ResultOfCompare DirSnapshot::CompareSnapshots(DirSnapshot *in_pdsRemake, SnapshotComparison *out_pscResult, bool in_fHash)
{
  FileData *pfdResult;

//   fprintf(stderr, "DirSnapshot::CompareSnapshots() , список всех директорий:\n"); //отладка!!!
//   RootMonitor::pdlList->PrintList(); //отладка!!!
  //судя по всему, где-то не задаётся или сбрасывается родительская директория (i)
  //или удаляется родительская директория вместо дочерней (i)
  //или плохо удаляется директория из слепка, либо вообще не удаляется (i)
//   fprintf(stderr, "DirSnapshot::CompareSnapshots() , сравниваются слепки 1:\n"); //отладка!!!
//   PrintSnapshot(); //отладка!!!
//   fprintf(stderr, "DirSnapshot::CompareSnapshots() , 2:\n"); //отладка!!!
//   in_pdsRemake->PrintSnapshot(); //отладка!!!

  //если уже существует первоначальный слепок, и он пустой
  if(pfdFirst == NULL)
  {
    //если существует новый слепок, и он не пустой
    if(in_pdsRemake != NULL && in_pdsRemake->pfdFirst != NULL && in_pdsRemake->pfdFirst->pfdNext != NULL)
    {
      //смотрим, есть ли в старом слепке все файлы из нового
      pfdResult = IsDataIncluded(in_pdsRemake, this, in_fHash);
      //если найден файл, которого нет в старом слепке, но который присутствует в новом
      if(pfdResult != NULL)
      {
	//значит в этой папке был создан новый файл
	out_pscResult->rocResult = IS_CREATED;
	out_pscResult->pfdData = pfdResult;
	return IS_CREATED;
      }
      else
	return IS_EMPTY;
    }
    else
      return IS_EMPTY;
  }

  if(in_pdsRemake == NULL)
    return INPUT_IS_EMPTY;

  if(out_pscResult == NULL)
    return OUTPUT_IS_EMPTY;

  //смотрим, есть ли в новом слепке все файлы из старого слепка
  pfdResult = IsDataIncluded(this, in_pdsRemake, in_fHash);
  if(pfdResult != NULL)
  {
    //если какого-то файла нет в новом слепке, значит он был удалён из директории
    out_pscResult->rocResult = IS_DELETED;
    out_pscResult->pfdData = pfdResult;
    return IS_DELETED;
  }

  //смотрим, есть ли в старом слепке все файлы из нового слепка
  pfdResult = IsDataIncluded(in_pdsRemake, this, in_fHash);
  if(pfdResult != NULL)
  {
    //если в старом слепке какой-то файл не найден, значит он был создан в директории
    out_pscResult->rocResult = IS_CREATED;
    out_pscResult->pfdData = pfdResult;
    return IS_CREATED;
  }

  return IS_EQUAL;
}

//если Subset является частью Set, возвращаем NULL
//если Subset больше Set, возвращается ссылка на файл, которого нету в Set
FileData *DirSnapshot::IsDataIncluded(DirSnapshot *in_pdsSubset, DirSnapshot *in_pdsSet, bool in_fHash)
{
  FileData *pfdListSubset, *pfdListSet;

  //перебираем все файлы первого слепка
  pfdListSubset = in_pdsSubset->pfdFirst->pfdNext;
  while(pfdListSubset != NULL)
  {
    //перебираем все файлы второго слепка
    pfdListSet = in_pdsSet->pfdFirst->pfdNext;
    while(pfdListSet != NULL)
    {
      //если имена файлов совпадает - переключаем файл первого слепка на следующий
//       if(strcmp(pfdListSubset->pName, pfdListSet->pName) == 0)
// 	break;
      //если совпадают inode (либо ulCrc) - переключаем файл первого слепка на следующий
      //нужно ещё добавить сравнение по именам+inode (для поиска переименований) и как-то сообщать о результате (!)
      if(in_fHash)
      {
	//сравниваем хэши файлов
	if( pfdListSubset->nType != IS_NOTAFILE && 
	    pfdListSet->nType != IS_NOTAFILE && 
	    pfdListSubset->ulCrc == pfdListSet->ulCrc )
	  break;
      }
      else
      {
	//сравниваем inode файлов
	if( pfdListSubset->nType != IS_NOTAFILE && 
	    pfdListSet->nType != IS_NOTAFILE && 
	    pfdListSubset->stData.st_ino == pfdListSet->stData.st_ino )
	  break;
      }
      pfdListSet = pfdListSet->pfdNext;
    }
    //если достигнут конец первого слепка, и какой-то файл из него не найден во втором слепке,
    //возвращаем ссылку на этот файл
    //имеет смысл создавать третий слепок для возврата найденных различий между слепками (чисто список файлов) (!)
    if(pfdListSet == NULL)
      return pfdListSubset;
    //берём следующий файл из первого слепка для поиска во втором слепке
    pfdListSubset = pfdListSubset->pfdNext;
  }

  return NULL;
}

void DirSnapshot::AddResult(FileData * const in_pfdFile, ResultOfCompare in_rocResult)
{
  SnapshotComparison *pscList;

  pthread_mutex_lock(&mComparisonResultList);
  if(pscFirst == NULL)
    pscFirst = new SnapshotComparison();

  pscList = pscFirst;
  while(pscList->pscNext != NULL)
  {
    pscList = pscList->pscNext;
  }

  pscList->pscNext = new SnapshotComparison(in_pfdFile, in_rocResult);
  pthread_mutex_unlock(&mComparisonResultList);
}

//после вызова функции и обработки результата требуется удалить объект, ссылка на который возвращается данным методом (!)
SnapshotComparison *DirSnapshot::GetResult(void)
{
  SnapshotComparison *pscRet;

  //если список не инициализирован или пустой - возвращаем NULL
  if(pscFirst == NULL || pscFirst->pscNext == NULL)
    return NULL;

  pthread_mutex_lock(&mComparisonResultList);
  //выносим первый элемент за пределы списка результатов сравнения двух слепков
  pscRet = pscFirst;
  pscFirst = pscFirst->pscNext;
  pscRet->pscNext = NULL;
  pthread_mutex_unlock(&mComparisonResultList);
  //возвращаем ссылку на вынесенный из списка результатов сравнения двух слепков элемент
  //после обработки требуется удалить этот объект (!)
  return pscRet;
}

//вывести содержимое директории
void DirSnapshot::PrintSnapshot(void)
{
  FileData *pfdList;

  pfdList = pfdFirst;
  while(pfdList != NULL)
  {
    fprintf(stderr, "DirSnapshot::PrintSnapshot() : %s\n", pfdList->pName);
    pfdList = pfdList->pfdNext;
  }
}

/****************************************FileData**********************************/

FileData::FileData()
{
    pName = new char[1];
    memset(pName, 0, sizeof(char));
    pSafeName = NULL;
    nType = IS_NOTAFILE;
    memset(&stData, 0, sizeof(struct stat));
    nDirFd = -1; //инициализация дескриптора как пустого
    //memset(szHash, 0, sizeof(szHash)); //обнуляем хэш файла
    ulCrc = crc32(0L, Z_NULL, 0);

    pfdNext = NULL;
    pfdPrev = NULL;
}

FileData::FileData(char const * const in_pName, char *in_pPath, struct FileData * const in_pfdPrev, bool in_fCalcHash)
{
    SetFileData(in_pName, in_pPath, in_fCalcHash);
    pfdPrev = in_pfdPrev;

    if(in_pfdPrev != NULL)
    {
	pfdNext = in_pfdPrev->pfdNext;
	if(in_pfdPrev->pfdNext != NULL)
	    in_pfdPrev->pfdNext->pfdPrev = this;
	in_pfdPrev->pfdNext = this;
    }
    else
    {
	pfdNext = NULL;
    }
}

//конструктор копии
FileData::FileData(FileData const * const in_pfdFile, char const * const in_pPath, bool in_fCalcHash)
{
  size_t stSize;

  //обязательно обнуляем эти указатели
  pfdNext = NULL;
  pfdPrev = NULL;

  nDirFd = -1; //инициализация дескриптора как пустого
  
  if(in_pfdFile == NULL)
  {
    pName = new char[1];
    memset(pName, 0, sizeof(char));
    pSafeName = NULL;
    nType = IS_NOTAFILE;
    memset(&stData, 0, sizeof(struct stat));
    //memset(szHash, 0, sizeof(szHash)); //обнуляем хэш файла
    ulCrc = crc32(0L, Z_NULL, 0);
    return;    
  }

  if(in_pfdFile->pName != NULL)
  {
    stSize = strlen(in_pfdFile->pName);
    pName = new char[stSize+1];
    memset(pName, 0, stSize+1);
    strncpy(pName, in_pfdFile->pName, stSize);
  }
  else
  {
//     pName = new char[1];
//     memset(pName, 0, sizeof(char));
    pName = NULL;
  }
  pSafeName = NULL;
  nType = in_pfdFile->nType;
  memcpy(&stData, &(in_pfdFile->stData), sizeof(struct stat));
  nDirFd = in_pfdFile->nDirFd;
  if(in_fCalcHash)
  {
    CalcHash(in_pPath);
  }
  else
  {
    //memcpy(szHash, in_pfdFile->szHash, sizeof(szHash));
    ulCrc = in_pfdFile->ulCrc;
  }
}

FileData::~FileData()
{
    if(pfdPrev != NULL)
	pfdPrev->pfdNext = pfdNext;
    if(pfdNext != NULL)
	pfdNext->pfdPrev = pfdPrev;
    if(pName != NULL)
    {
// 	if(strcmp(pName, "123") == 0) //отладка!!!
// 	fprintf(stderr, "FileData::~FileData() : delete \"%s\"\n", pName); //отладка!!!

	delete [] pName;
	pName = NULL;
    }
    if(pSafeName != NULL)
    {
	delete [] pSafeName;
	pSafeName = NULL;
    }
}

void FileData::CalcHash(char const * const in_pPath)
{
    int nFd;
    char *pcBuff;
    ssize_t sstRead;
    char *pFullPath;
    const long BUFF_SIZE = 1048510;
    size_t stSize, stSizePath;

    ulCrc = crc32(0L, Z_NULL, 0);

    //обнуляем хэш
    //memset(szHash, 0, sizeof(szHash));

    //проверка
    if(nType != IS_FILE)
      return;

    if(in_pPath == NULL || pName == NULL || strlen(pName) == 0)
      return;

    //получаем полный путь к файлу
    stSize = strlen(in_pPath);
    stSizePath = strlen(pName);
    pFullPath = new char[stSize + stSizePath + 2];
    memset(pFullPath, 0, stSize + stSizePath + 2);
    strncpy(pFullPath, in_pPath, stSize + stSizePath + 1);
    strncat(pFullPath, "/", stSize + stSizePath + 1);
    strncat(pFullPath, pName, stSize + stSizePath + 1);
    
    nFd = open(pFullPath, O_RDONLY);
    if(pFullPath != NULL)
      delete [] pFullPath;
    if(nFd != -1)
    {
      pcBuff = new char[BUFF_SIZE]; //1M
      stSize = 0;
      while(stSize < stData.st_size)
      {
	memset(pcBuff, 0, BUFF_SIZE);
	stSizePath = read(nFd, pcBuff, BUFF_SIZE);
	stSize = stSize + stSizePath;
	//вычисляем хэш
	ulCrc = crc32(ulCrc, (const unsigned char *)pcBuff, stSizePath);
      }
      close(nFd);
      delete [] pcBuff;
    }
}

//задать имя файла и определить его тип
//по запросу можно сразу вычислить хэш
void FileData::SetFileData(char const * const in_pName, char *in_pPath, bool in_fCalcHash)
{
    char *pPath = NULL;
    size_t stLen;
    struct stat st;

    nDirFd = -1; //инициализация дескриптора как пустого

    if(in_pPath != NULL && in_pName != NULL)
    {
      stLen = strlen(in_pPath)+strlen(in_pName);
      pPath = new char[stLen+2];
      memset(pPath, 0, stLen+2);
      if(strlen(in_pPath) > 0)
      {
	memcpy(pPath, in_pPath, stLen);
	strncat(pPath, "/", stLen);
      }
      strncat(pPath, in_pName, stLen);
//       fprintf(stderr, "FileData::SetFileData() : %s\n", pPath); //отладка!!!
    }

    //если имя указано неверно - создаём пустой элемент списка (?)
    //возможно, придётся восстанавливать полный путь к файлу для корректного вызова stat() (!)
    if(in_pName == NULL || ((stLen = strlen(in_pName)) <= 0) || (stat(pPath, &st) < 0))
    {
	//осторожно, может сильно вырасти список пустышек (!)
	//возможно, стоит вызывать stat до создания объекта класса FileData
	perror("error in stat, FileData::SetFileData()");
 	fprintf(stderr, "Can not get stat FileData::SetFileData() : \"%s\" !\n", in_pName); //отладка!!!
	pName = new char[1];
	memset(pName, 0, sizeof(char));
	pSafeName = NULL;
	nType = IS_NOTAFILE;
	memset(&stData, 0, sizeof(struct stat));
	//memset(szHash, 0, sizeof(szHash)); //обнуляем хэш файла	
	ulCrc = crc32(0L, Z_NULL, 0);

	pfdNext = NULL;
	pfdPrev = NULL;
	
	if(pPath != NULL)
	  delete [] pPath;
	return;
    }

    //инициализация имени файла
    pName = new char[stLen+1];
    memset(pName, 0, stLen+1);
    strncpy(pName, in_pName, stLen);

    //инициализация имени файла
    pSafeName = new char[stLen+1];
    memset(pSafeName, 0, stLen+1);
    strncpy(pSafeName, in_pName, stLen);

    //описание файла
    memcpy(&stData, &st, sizeof(st));

//     fprintf(stderr, "FileData::SetFileData() : %s, inode=%d, %d\n", pPath, (int)st.st_ino, (int)stData.st_ino); //отладка!!!

    //хэш
    //memset(szHash, 0, sizeof(szHash));
    ulCrc = crc32(0L, Z_NULL, 0);

    //инициализация типа файла
    switch(st.st_mode & (S_IFDIR | S_IFREG | S_IFLNK))
    {
	case S_IFDIR:
	    nType = IS_DIRECTORY;
	    break;
	case S_IFREG:
	    nType = IS_FILE;
	    break;
	case S_IFLNK:
	    nType = IS_LINK;
	    break;
	default: nType = IS_NOTAFILE;
    }

    if(in_fCalcHash && nType == IS_FILE)
    {
	//получаем хэш файла, если это обычный файл
	CalcHash(in_pPath);
    }

    if(pPath != NULL)
      delete [] pPath;
}

char const * const FileData::GetName()
{
    return pName;
}

/****************************************SnapshotComparison*********************/

SnapshotComparison::SnapshotComparison()
{
    rocResult = IS_EMPTY;
    pfdData = NULL;
    pscNext = NULL;
}

SnapshotComparison::SnapshotComparison(FileData * const in_pfdFile, ResultOfCompare in_rocResult)
{
    rocResult = in_rocResult;
    pfdData = in_pfdFile;
    pscNext = NULL;
}

SnapshotComparison::~SnapshotComparison()
{
    rocResult = IS_EMPTY;
    pfdData = NULL;
}
