//RootMonitor.cpp
//Author: Bombo
//12.01.2017
//Класс RootMonitor отвечает за один отдельный проект для наблюдения

#include"RootMonitor.h"

DescriptorsList *RootMonitor::pdlList = NULL;
DescriptorsQueue *RootMonitor::pdqQueue = NULL;
pthread_mutex_t RootMonitor::mDescListMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t RootMonitor::mDescQueueMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t RootMonitor::mDirThreadMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t RootMonitor::mDescThreadMutex = PTHREAD_MUTEX_INITIALIZER;

RootMonitor::RootMonitor()
{
    psdRootDirectory = NULL;
    pdlList = NULL;
    pjsFirst = NULL;
    ulLastSessionNumber = 0L;
    ulRegularSessionNumber = 4096L;
    pszServerURL = NULL;
    sSocket = -1;
}

RootMonitor::RootMonitor(char * const pRootPath)
{
    pjsFirst = NULL;
    ulLastSessionNumber = 0L;
    ulRegularSessionNumber = 4096L;
    pszServerURL = NULL;
    sSocket = -1;

    if(pRootPath == NULL)
    {
	psdRootDirectory = NULL;
	pdlList = NULL;
	return;
    }

    if(pdqQueue == NULL)
	pdqQueue = new DescriptorsQueue();

    //создаём описание корневой директории
    psdRootDirectory = new SomeDirectory(pRootPath, NULL);

    //создаём первый список событий (инициализирующий)
    AddChange(INIT_SERVICE, ulLastSessionNumber, psdRootDirectory->GetFileData(), INIT_PROJECT, -1);

    //открываем корневую директорию и добавляем полученный дескриптор в список открытых
    //этот список существует для упрощения поиска директории по её дескриптору
    if(pdlList == NULL)
    {
	pthread_mutex_lock(&mDescListMutex);
	pdlList = new DescriptorsList(psdRootDirectory);
	pthread_mutex_unlock(&mDescListMutex);
    }
    else
    {
	pthread_mutex_lock(&mDescListMutex);
	pdlList->AddQueueElement(psdRootDirectory);
	pthread_mutex_unlock(&mDescListMutex);
    }
}

RootMonitor::RootMonitor(FileData * const in_pfdData)
{
    pjsFirst = NULL;
    ulLastSessionNumber = 0L;
    ulRegularSessionNumber = 4096L;
    pszServerURL = NULL;
    sSocket = -1;

    if(in_pfdData == NULL)
    {
	psdRootDirectory = NULL;
	pdlList = NULL;
	return;
    }

    if(pdqQueue == NULL)
	pdqQueue = new DescriptorsQueue();

    //создаём описание корневой директории
    psdRootDirectory = new SomeDirectory(in_pfdData, NULL, true);

    //создаём первый список событий (инициализирующий)
    AddChange(INIT_SERVICE, ulLastSessionNumber, psdRootDirectory->GetFileData(), INIT_PROJECT, -1);

    //открываем корневую директорию и добавляем полученный дескриптор в список открытых
    if(pdlList == NULL)
    {
	pthread_mutex_lock(&mDescListMutex);
	pdlList = new DescriptorsList(psdRootDirectory);
	pthread_mutex_unlock(&mDescListMutex);
    }
    else
    {
	pthread_mutex_lock(&mDescListMutex);
	pdlList->AddQueueElement(psdRootDirectory);
	pthread_mutex_unlock(&mDescListMutex);
    }
}

RootMonitor::RootMonitor(SomeDirectory * const in_psdRootDirectory)
{
    pjsFirst = NULL;
    ulLastSessionNumber = 0L;
    ulRegularSessionNumber = 4096L;
    pszServerURL = NULL;
    sSocket = -1;

    if(in_psdRootDirectory == NULL)
    {
	psdRootDirectory = NULL;
	pdlList = NULL;
	return;
    }

    if(pdqQueue == NULL)
	pdqQueue = new DescriptorsQueue();

    //инициализируем ссылку на описание корневой директории отслеживаемого проекта
    psdRootDirectory = in_psdRootDirectory;

    //создаём первый список событий (инициализирующий)
    AddChange(INIT_SERVICE, ulLastSessionNumber, psdRootDirectory->GetFileData(), INIT_PROJECT, -1);

    //открываем корневую директорию и добавляем полученный дескриптор в список открытых
    if(pdlList == NULL)
    {
	pthread_mutex_lock(&mDescListMutex);
	pdlList = new DescriptorsList(in_psdRootDirectory);
	pthread_mutex_unlock(&mDescListMutex);
    }
    else
    {
	pthread_mutex_lock(&mDescListMutex);
	pdlList->AddQueueElement(in_psdRootDirectory);
	pthread_mutex_unlock(&mDescListMutex);
    }
}

RootMonitor::~RootMonitor()
{
    //т.к. эти данные теперь статические, их не следует удалять
//    if(pdlList != NULL)
//        delete pdlList;
//    if(psdRootDirectory != NULL)
//        delete psdRootDirectory;
    DeleteJSONServices();
    if(pszServerURL != NULL)
      delete [] pszServerURL;
    if(sSocket != -1)
      close(sSocket);
}

void RootMonitor::AddChange(ServiceType in_stType, unsigned long in_ulSessionNumber, FileData * const in_pfdFile, ResultOfCompare in_rocEvent, ino_t in_itParentInode)
{
  JSONService *pjsList, *pjsLast, *pjsBuff;

  if(pjsFirst == NULL)
    pjsFirst = new JSONService(in_stType, in_ulSessionNumber);

  pjsList = pjsLast = pjsFirst;
  while(pjsList != NULL)
  {
    pjsLast = pjsList;
    if( ((pjsLast->GetSessionNumber()) == in_ulSessionNumber) && ((pjsLast->GetType()) == in_stType) )
      break;
    pjsList = pjsList->GetNext();
  }

  //pjsLast указывает либо на найденный сервис, либо на последний сервис в списке
  if(pjsList == NULL)
  {
    pjsBuff = pjsLast->GetNext();
    pjsList = new JSONService(in_stType, in_ulSessionNumber);
    pjsLast->SetNext(pjsList);
    pjsList->SetNext(pjsBuff);
  }

  pjsList->AddChange(in_stType, in_pfdFile, in_rocEvent, in_itParentInode);
}

//добавить в очередь инициализирующее событие для данного проекта
void RootMonitor::AddInitChange(FileData * const in_pfdFile, ino_t in_itParentInode)
{
  AddChange(INIT_SERVICE, ulLastSessionNumber, in_pfdFile, IS_EQUAL, in_itParentInode);
}

//получить JSON конкретной сессии
char * const RootMonitor::GetJSON(unsigned long in_ulSessionNumber)
{
  JSONService *pjsList;

  if(pjsFirst == NULL)
    return NULL;

  pjsList = pjsFirst;
  while(pjsList != NULL)
  {
    if((pjsList->GetSessionNumber()) == in_ulSessionNumber)
      return (pjsList->GetJSON());
    pjsList = pjsList->GetNext();
  }

  return NULL;
}

unsigned long RootMonitor::GetLastSessionNumber(void)
{
  return ulLastSessionNumber;
}

unsigned long RootMonitor::GetRegularSessionNumber(void)
{
  return ulRegularSessionNumber;
}

void RootMonitor::IncRegularSessionNumber(void)
{
  ulRegularSessionNumber++;
}

//поменять/установить путь к корневой директории
int RootMonitor::SetRootPath(char const * const in_pNewRootPath)
{
    //останавливаем сопровождающие потоки
    //...

//    SetDirName(in_pNewRootPath); //функцию необходимо переделать, поэтому закомментировано

    //запускаем потоки
    //...

/*
    size_t stLen;
    int nNewRootFd;

    //если путь указан неверно
    if(in_pNewRootPath == NULL || (stLen = strlen(in_pNewRootPath)) <= 0)
    {
	return -1;
    }

    if((nNewRootFd = open(in_pNewRootPath, O_RDONLY)) < 0)
    {
	return -2;
    }

    //закрываем имеющийся дескриптор (если он открыт)
    if(nRootFd >= 0)
        close(nRootFd);
    //удаляем прежний путь
    if(pRootPath != NULL)
	delete [] pRootPath;
    //удаляем копию пути
    if(pSafeRootPath != NULL)
	delete [] pSafeRootPath;

    //задаём дескриптор
    nRootFd = nNewRootFd;
    //создаём новый путь
    pRootPath = new char[stLen+1];
    memset(pRootPath, 0, stLen+1);
    strncpy(pRootPath, in_pNewRootPath, stLen);
    //копируем путь
    pSafeRootPath = new char[stLen+1];
    memset(pSafeRootPath, 0, stLen+1);
    strncpy(pSafeRootPath, in_pNewRootPath, stLen);
*/
    return 0;
}

void RootMonitor::DeleteJSONServices(void)
{
    JSONService *pjsList, *pjsDel;

    pjsList = pjsDel = pjsFirst;
    while(pjsList != NULL)
    {
	pjsList = pjsList->GetNext();
	delete pjsDel;
	pjsDel = pjsList;
    }
    pjsFirst = NULL;
}

//вывести содержимое списка с конкретным номером сессии
void RootMonitor::PrintSession(unsigned long in_ulSessionNumber)
{
    JSONService *pjsList;
    
    pjsList = pjsFirst;
    while(pjsList != NULL)
    {
	if((pjsList->GetSessionNumber()) == in_ulSessionNumber)
	{
	    pjsList->PrintService();
	    return;
	} 
	pjsList = pjsList->GetNext();
    }
}

//вывести содержимое списка с конкретным номером сессии
void RootMonitor::PrintServices(void)
{
    JSONService *pjsList;
    
    pjsList = pjsFirst;
    while(pjsList != NULL)
    {
	pjsList->PrintService();
	pjsList = pjsList->GetNext();
    }
}
