//DescriptorsList.cpp
//Author: Bombo
//13.01.2017
//Класс DescriptorsList содержит список всех открытых дескрипторов

#include"DescriptorsList.h"
#include"RootMonitor.h"
#include <vector>

/*********************************DescriptorsList*********************************/

DescriptorsList::DescriptorsList()
{
    //инициализация блокировки списка директорий
    mListMutex = PTHREAD_MUTEX_INITIALIZER;
    //пустой первый элемент
    pthread_mutex_lock(&mListMutex);
    pdleFirst = NULL;
    pthread_mutex_unlock(&mListMutex);
}

DescriptorsList::DescriptorsList(SomeDirectory *in_psdRootDirectory)
{
    //инициализация блокировки списка директорий
    mListMutex = PTHREAD_MUTEX_INITIALIZER;
    pthread_mutex_unlock(&mListMutex);
    //создаём список директорий с первым элементом
    pthread_mutex_lock(&mListMutex);
    pdleFirst = new DirListElement(in_psdRootDirectory, NULL);
    pthread_mutex_unlock(&mListMutex);
}

DescriptorsList::DescriptorsList(PFILEDATA in_pfdData)
{
    //инициализация блокировки списка директорий
    mListMutex = PTHREAD_MUTEX_INITIALIZER;
    pthread_mutex_unlock(&mListMutex);
    //создаём список директорий с первым элементом
    pthread_mutex_lock(&mListMutex);
    pdleFirst = new DirListElement(in_pfdData, NULL, NULL);
    pthread_mutex_unlock(&mListMutex);
}

DescriptorsList::~DescriptorsList()
{
    //удаляем ссписок директорий
    DirListElement *pdleList, *pdleDel;
    if(pdleFirst == NULL)
	return;
    pthread_mutex_lock(&mListMutex);
    pdleList = pdleFirst->pdleNext;
    while(pdleList != NULL)
    {
        pdleDel = pdleList;
        pdleList = pdleList->pdleNext;
        delete pdleDel;
    }
    delete pdleFirst;
    pdleFirst = NULL;
    pthread_mutex_unlock(&mListMutex);
}

//перед вызовом метода ОБЯЗАТЕЛЬНО следует блокировать mDescListMutex
//после вызова - освобождать
void DescriptorsList::AddQueueElement(SomeDirectory * const in_psdPtr)
{
    DirListElement *pdleList;

    if(in_psdPtr == NULL)
	return;

    //добавляем элемент списка
    if(pdleFirst == NULL)
    {
        pthread_mutex_lock(&mListMutex);
        //если список пуст - назначаем первый элемент
        pdleFirst = new DirListElement(in_psdPtr, NULL);
        pthread_mutex_unlock(&mListMutex);
        return;
    }

    pthread_mutex_lock(&mListMutex);
    //ищем конец списка
    pdleList = pdleFirst;
    while(pdleList->pdleNext != NULL)
    {
        pdleList = pdleList->pdleNext;
    }

    //добавляем директорию в конец списка
    pdleList->pdleNext = new DirListElement(in_psdPtr, pdleList);
    pthread_mutex_unlock(&mListMutex);
}

void DescriptorsList::SubQueueElement(SomeDirectory const * const in_psdPtr)
{
    DirListElement *pdleList;

    //если список пуст - выходим
    if(pdleFirst == NULL)
	return;

    pthread_mutex_lock(&mListMutex);
    //ищем заданный элемент
    pdleList = pdleFirst;
    while(pdleList != NULL)
    {
        if(pdleList->psdDirectory == in_psdPtr)
        {
            //удаляем найденный элемент из списка
            if(pdleFirst == pdleList)
              pdleFirst = pdleList->pdleNext;
            delete pdleList;
            pthread_mutex_unlock(&mListMutex);
            return;
        }
    }
    pthread_mutex_unlock(&mListMutex);
}

void DescriptorsList::SubQueueElement(int in_nDirFd)
{
    int nDirFd;
    DirListElement *pdleList;

    //если список пуст - выходим
    if(pdleFirst == NULL)
	return;

    pthread_mutex_lock(&mListMutex);
    pdleList = pdleFirst;
    while(pdleList != NULL)
    {
        nDirFd = pdleList->psdDirectory->GetDirFd();
//    	fprintf(stderr, "DescriptorsList::SubQueueElement() : %d, %d\n", (int)nDirFd, (int)in_nDirFd); //отладка!!!
        if(nDirFd == in_nDirFd)
        {
            if(RootMonitor::watcher != NULL)
              RootMonitor::watcher->RemoveDirectory(nDirFd);
            //удаляем элемент из очереди
            if(pdleFirst == pdleList)
              pdleFirst = pdleList->pdleNext;
            delete pdleList;
            pthread_mutex_unlock(&mListMutex);
            return;
        }
        pdleList = pdleList->pdleNext;
    }
    pthread_mutex_unlock(&mListMutex);
}

//переименовать директорию
void DescriptorsList::RenameQueueElement(PCFILEDATA const in_pfdNewData)
{
    DirListElement *pdleList;

    if(pdleFirst == NULL)
	return;

    pthread_mutex_lock(&mListMutex);
    pdleList = pdleFirst;
    while(pdleList != NULL)
    {
        if(strcmp(pdleList->psdDirectory->GetFileData()->szUuid, in_pfdNewData->szUuid) == 0)
        {
            //переименовываем директорию
            pdleList->psdDirectory->SetDirName(in_pfdNewData->pName);
            pthread_mutex_unlock(&mListMutex);
            return;
        }
        pdleList = pdleList->pdleNext;
    }
    pthread_mutex_unlock(&mListMutex);
}

//функция для отладки
int DescriptorsList::GetFd(void)
{
    static DirListElement *pdleLast;

    if(pdleLast == NULL)
    {
        pdleLast = pdleFirst;
        return pdleFirst->psdDirectory->GetDirFd();
    }

    pdleLast = pdleLast->pdleNext;

    if(pdleLast == NULL)
	return -1;

    return pdleLast->psdDirectory->GetDirFd();
}

//вывести список директорий
void DescriptorsList::PrintList(void)
{
    DirListElement *pdleList;
    PFILEDATA pfdData;

    //если список пуст - выходим
    if(pdleFirst == NULL)
    {
        fprintf(stderr, "\nDescriptorsList::PrintList() : DirectoryList is empty!\n\n");
        return;
    }

    fprintf(stderr, "\nDescriptorsList::PrintList() : DirectoryList: начало списка.\n");
    pthread_mutex_lock(&mListMutex);
    pdleList = pdleFirst;
    while(pdleList != NULL)
    {
        pfdData = pdleList->psdDirectory->GetFileData();
        if(pfdData != NULL)
          fprintf(stderr, "DescriptorsList::PrintList() : DirectoryList: файл %s, fd=%d, inode=%d.\n", pdleList->psdDirectory->GetDirName(), pdleList->psdDirectory->GetDirFd(), (int)pfdData->stData.st_ino);
        pdleList = pdleList->pdleNext;
    }
    fprintf(stderr, "DescriptorsList::PrintList() : DirectoryList: конец списка.\n\n");
    pthread_mutex_unlock(&mListMutex);
}

void DescriptorsList::UpdateList(void)
{
    DirListElement *pdleList;
    char *pPath = NULL;
    int nDirFd;
    PFILEDATA pfdData;

//     fprintf(stderr, "DescriptorsList::UpdateList() : start\n"); //отладка!!!
    if(pdleFirst == NULL)
    {
        fprintf(stderr, "DescriptorsList::UpdateList(): the list is empty!\n"); //отладка!!!
        return;
    }
//    pthread_mutex_lock(&mListMutex); (?)
    //ищем новые директории и обновляем их
    pdleList = pdleFirst;
    while(pdleList != NULL)
    {
        if(pdleList->psdDirectory == NULL)
        {
          fprintf(stderr, "DescriptorsList::UpdateList() Update: NULL!\n"); //отладка!!!
          continue;
        }
        pfdData = pdleList->psdDirectory->GetFileData();
        //создаём слепки для новых директорий в списке
        if(pdleList->psdDirectory != NULL && pdleList->psdDirectory->IsSnapshotNeeded() && pfdData != NULL)
        {
            pdleList->psdDirectory->MakeSnapshot();
            //проверяем, открыта уже директория или ещё нет
            nDirFd = pfdData->nDirFd;
            if(nDirFd == -1) //отладка!!!
            {
              pPath = pdleList->psdDirectory->GetFullPath();
              if(pPath != NULL)
              {
                //учесть при инкапсуляции (!)
                pfdData->nDirFd = open(pPath, O_RDONLY);
                delete [] pPath;
              }
            }
            nDirFd = pfdData->nDirFd;
            if(nDirFd >= 0)
            {
                if(RootMonitor::watcher != NULL &&
                   !RootMonitor::watcher->AddDirectory(pdleList->psdDirectory))
                    fprintf(stderr, "Failed to watch directory: %s\n", pfdData->pName);
            }
        }
        else
        {
//     	  if(pdleList->psdDirectory->IsSnapshotNeeded())
//     	    fprintf(stderr, "DescriptorsList::UpdateList() 3: no name or file data!\n");
        }
        pdleList = pdleList->pdleNext;
    }
//    pthread_mutex_unlock(&mListMutex); (?)
}

void DescriptorsList::CompareAll(const char *token)
{
    std::vector<int> descriptors;
    for(DirListElement *element = pdleFirst; element != NULL; element = element->pdleNext)
      if(element->psdDirectory != NULL && !element->psdDirectory->IsSnapshotNeeded())
        descriptors.push_back(element->psdDirectory->GetDirFd());
    for(int fd : descriptors)
    {
      SomeDirectory *directory = GetDirectory(fd);
      if(directory != NULL && !directory->IsSnapshotNeeded())
        directory->CompareSnapshots(token);
    }
}

SomeDirectory *DescriptorsList::GetDirectory(int in_nFd)
{
    DirListElement *pdleDir;

    if(pdleFirst == NULL)
	return NULL;

    pdleDir = pdleFirst;
    while(pdleDir != NULL)
    {
        if(pdleDir->psdDirectory->GetDirFd() == in_nFd)
            return pdleDir->psdDirectory;
        pdleDir = pdleDir->pdleNext;
    }

    return NULL;
}

/*********************************DirListElement*********************************/

DIRLISTELEMENT::DirListElement()
{
    psdDirectory = NULL;
    pdleNext = NULL;
    pdlePrev = NULL;
}

DIRLISTELEMENT::DirListElement(SomeDirectory *in_psdDirectory, DirListElement * const in_pdlePrev)
{
    psdDirectory = in_psdDirectory;
    pdlePrev = in_pdlePrev;

//     fprintf(stderr, "DirListElement::DirListElement() : inode=%d\n", (int)(psdDirectory->GetFileData())->stData.st_ino); //отладка!!!
    //исключаем выпадение части элементов списка
    if(in_pdlePrev != NULL)
    {
        pdleNext = in_pdlePrev->pdleNext;
	//если за предшествующим элементом есть последующий, меняем у последующего предыдущий на this
        if(in_pdlePrev->pdleNext != NULL)
            in_pdlePrev->pdleNext->pdlePrev = this;
        in_pdlePrev->pdleNext = this;
    }
    else
    {
        pdleNext = NULL;
    }
}

DIRLISTELEMENT::DirListElement(PFILEDATA in_pfdData, SomeDirectory * const in_psdParent, DirListElement * const in_pdlePrev)
{
    //осторожно! Родительский каталог не ищется! Надо делать! Краш!!! TODO: //
    psdDirectory = new SomeDirectory(in_pfdData, in_psdParent, true); //снимок обязан присутствовать в элементе очереди
    pdlePrev = in_pdlePrev;
    //исключаем выпадение части элементов списка
    if(in_pdlePrev != NULL)
    {
        pdleNext = in_pdlePrev->pdleNext;
	//если за предшествующим элементом есть последующий, меняем у последующего предыдущий на this
        if(in_pdlePrev->pdleNext != NULL)
            in_pdlePrev->pdleNext->pdlePrev = this;
        in_pdlePrev->pdleNext = this;
    }
    else
    {
        pdleNext = NULL;
    }
}

DIRLISTELEMENT::~DirListElement()
{
    //полностью удаляем директорию
    //FileData удаляется из своего слепка автоматически
    delete psdDirectory;
    //обновляем очередь
    if(pdlePrev != NULL)
      pdlePrev->pdleNext = pdleNext;
    if(pdleNext != NULL)
      pdleNext->pdlePrev = pdlePrev;
}
