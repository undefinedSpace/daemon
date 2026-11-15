//undefined by Bombo
//10.01.2017

// Filesystem events are read from fanotify; directory snapshots determine changes.

#include<stdio.h>
#include<fcntl.h>
#include<unistd.h>
#include<string.h>
#include<dirent.h> //opendir(), readdir()
#include<stdlib.h>
#include<pthread.h>
#include<sys/time.h>
#include<sys/stat.h>
#include<sys/types.h>
#include<arpa/inet.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<sys/select.h>
#include<sys/resource.h>

#include"RootMonitor.h"
#include"SimpleSemantic.h"
#include"FileUuid.h"

#define PORT_NUMBER 44553
#define SOCKET_READ_SIZE 102400
#define MESSAGE_BUFF_SIZE 20480000

RootMonitor *rmProject;

void StartSomeThread(unsigned long in_ulStack, void *(* in_FunctionName)(void *), void *in_pvArg);
void *fanotify_thread(void *);
void *directory_thread(void *arg);
void *send_json_thread(void *arg);
void *server_thread(void *arg);

int main(int argc, char *argv[])
{
  int i;
  char filename[256];
  char rootUuid[37];
  struct stat st;
  rlimit r;
  JSONParser *jp;

  //запускаем сервер
  StartSomeThread(1024000L, server_thread, NULL);
  usleep(1000000);

  jp = new JSONParser();
//  fprintf(stderr, "DEBUG!!!\n");
  jp->CheckRequest();
//  return 0;

  //увеличиваем лимит открытых файлов для текущего процесса
  r.rlim_cur = 8192;
  r.rlim_max = 8192;
  if(setrlimit(RLIMIT_NOFILE, &r) < 0)
    perror("main(), setrlimit error:");
  getrlimit(RLIMIT_NOFILE, &r);
  fprintf(stderr, "main() : nofile limits: rlim_cur=%ld, rlim_max=%ld\n", r.rlim_cur, r.rlim_max);

  pthread_mutex_lock(&(RootMonitor::mSendJSONThreadMutex));

  //проверяем количество аргументов
  if(argc <= 2)
  {
    fprintf(stderr, "USAGE: %s <path to directory 1> ... <path to directory N> <server URL[:Port]>\n", argv[0]);
    return -1;
  }

  RootMonitor::watcher = new FanotifyWatcher();
  if(!RootMonitor::watcher->IsReady())
    return 1;

  for(i = 0; i < argc-2; ++i)
  {
    //обнуляем имя файла
    memset(filename, 0, sizeof(filename));
    //копируем имя файла
    strncpy(filename, argv[i+1], sizeof(filename));
    if(filename[0] != '/')
    {
      fprintf(stderr, "main() wrong path: The path to project directory must be started from root! (\"%s\")\n", filename);
      continue;
    }

    if(stat(filename, &st) != 0 || !S_ISDIR(st.st_mode) ||
       !EnsureFileUuid(filename, rootUuid))
    {
      fprintf(stderr, "Cannot monitor directory or store its UUID: %s\n", filename);
      continue;
    }
    //пытаемся открыть файл
    rmProject = new RootMonitor(filename);
    fprintf(stderr, "Project path: \"%s\", inode=%ld, mode=%d, DIR=%d\n", filename, st.st_ino, st.st_mode & S_IFDIR, S_IFDIR);
    break;
  }

  if(rmProject == NULL)
    return 1;

  fprintf(stderr, "Server URL: %s\n", argv[argc-1]);
  rmProject->SetServerURL(argv[argc-1]);

  //запускаем поток обработки списка найденных директорий
  StartSomeThread(102400L, directory_thread, NULL);

  // Read filesystem events only after the initial directory scan has started.
  StartSomeThread(102400L, fanotify_thread, NULL);

  //запускаем поток отправки JSON-сообщений серверу
  StartSomeThread(102400L, send_json_thread, NULL);

  usleep(2000000); //отладка!!!
//  char *list; //отладка!!!
//  list = rmProject->GetJSON(rmProject->GetLastSessionNumber()); //отладка!!!
//  fprintf(stderr, "%s\n", (list==NULL)?"NULL":list); //отладка!!!
//  if(list != NULL) //отладка!!!
//    delete [] list; //отладка!!!

  rmProject->SendChangesToServer(); //отладка!!!

  for(;;)
  {
    usleep(700000);
  }

  return 0;
}

void StartSomeThread(unsigned long in_ulStack, void *(* in_FunctionName)(void *), void *in_pvArg)
{
  pthread_attr_t paAttr;
  pthread_t ptThread;

  pthread_attr_init(&paAttr);
  pthread_attr_setstacksize(&paAttr, in_ulStack);
  pthread_attr_setdetachstate(&paAttr, PTHREAD_CREATE_DETACHED);
  pthread_create(&ptThread, &paAttr, in_FunctionName, in_pvArg);
  pthread_attr_destroy(&paAttr);
}

void *fanotify_thread(void *)
{
  RootMonitor::watcher->Run();
  return NULL;
}

//поток обработки списка найденных директорий
void *directory_thread(void *arg)
{
  for(;;)
  {
    pthread_mutex_lock(&(RootMonitor::mScanMutex));
    if(RootMonitor::pdlList != NULL)
      RootMonitor::pdlList->UpdateList();
    pthread_mutex_unlock(&(RootMonitor::mScanMutex));
    usleep(100000);
  }
  pthread_exit(NULL);
}

void *send_json_thread(void *)
{
  for(;;)
  {
    pthread_mutex_lock(&(RootMonitor::mSendJSONThreadMutex));
    if(rmProject != NULL)
      rmProject->SendChangesToServer();
  }
  pthread_exit(NULL);
}

//основной поток сервера
void *server_thread(void *arg)
{
  struct sockaddr_in addr, addr_client;
  int sServerSocket, sInSocket;
  DaemonSemanticData dsdData;
  ssize_t sstSize, sstLen;
  char *pbBuff, *pbPtr;
  socklen_t slLen;
  int nValue;

  fprintf(stderr, "Start server.\n");
  sServerSocket = socket(AF_INET, SOCK_STREAM, 0);
  if(sServerSocket <= 0)
  {
    perror("server_thread() socket() error");
    pthread_exit(NULL);
  }
  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(PORT_NUMBER);

  nValue = 1;
  setsockopt(sServerSocket, SOL_SOCKET, SO_REUSEADDR, &nValue, sizeof(int));

  if(bind(sServerSocket, (struct sockaddr *) &addr, sizeof(addr)) < 0)
  {
    close(sServerSocket);
    perror("server_thread() bind() error");
    pthread_exit(NULL);
  }
  if(listen(sServerSocket, 50) < 0)
  {
    close(sServerSocket);
    perror("server_thread() listen() error");
    pthread_exit(NULL);
  }
  for(;;)
  {
    //fprintf(stderr, "ok!\n"); //debug!!!
    sInSocket = accept(sServerSocket, (struct sockaddr *)&addr_client, &slLen);
    if(sInSocket < 0)
    {
      perror("server_thread() accept() error");
      continue;
    }
    sstLen = 0L;
    pbBuff = new char[MESSAGE_BUFF_SIZE];
    memset(pbBuff, 0, MESSAGE_BUFF_SIZE);
//    sstSize = read(sInSocket, pbBuff+sstLen, 10240000);
    do {
      sstSize = read(sInSocket, pbBuff+sstLen, SOCKET_READ_SIZE);
      sstLen = sstSize + sstLen;
      fprintf(stderr, "read=%ld bytes, sstLen=%ld, SOCKET_READ_SIZE=%d, MESSAGE_BUFF_SIZE=%d\n", (long int)sstSize, sstLen, SOCKET_READ_SIZE, MESSAGE_BUFF_SIZE);
    }
    while(sstSize > 0 && sstSize <= SOCKET_READ_SIZE && sstLen < MESSAGE_BUFF_SIZE); //(?)
    close(sInSocket);
    //fprintf(stderr, "in_port=%d, buff:%s\n", htons(addr_client.sin_port), pbBuff); //отладка!!!
    pbPtr = strstr(pbBuff, "\r\n\r\n");
    if(pbPtr != NULL)
    {
      //fprintf(stderr, "%s\n", inet_ntoa(addr_client.sin_addr)); //debug!!!
      dsdData.Analyze(inet_ntoa(addr_client.sin_addr), pbPtr);
    }
    delete [] pbBuff;
    //fprintf(stderr, "clear\n"); //debug!!!
  }
  pthread_exit(NULL);
}
