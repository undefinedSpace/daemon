//SimpleSemantic.cpp
//Author: Bombo
//17.04.2017
//Основа для семантического анализатора

#include"SimpleSemantic.h"
#include <set>
#include <string>

namespace {
bool ValidUuid(const char *value)
{
  if(value == NULL || strlen(value) != 36) return false;
  for(int i = 0; i < 36; ++i)
    if(i == 8 || i == 13 || i == 18 || i == 23)
    {
      if(value[i] != '-') return false;
    }
    else if(!((value[i] >= '0' && value[i] <= '9') ||
              (value[i] >= 'a' && value[i] <= 'f')))
      return false;
  return true;
}

bool Decimal(const char *value)
{
  if(value == NULL || !*value) return false;
  for(; *value; ++value)
    if(*value < '0' || *value > '9') return false;
  return true;
}
}

SimpleSemantic::SimpleSemantic()
{
    pseFirst = NULL;
}

SimpleSemantic::~SimpleSemantic()
{
    SemanticStack *pseList;
    //SimpleSemantic *psList;

    while(pseFirst != NULL)
    {
        pseList = pseFirst->pseNext;
        delete pseFirst;
        pseFirst = pseList;
    }
}

void SimpleSemantic::PushValue(SemanticStack &in_seValue)
{
    PushValue(in_seValue.pszType, in_seValue.pbValue, in_seValue.nLength);
}

void SimpleSemantic::PushValue(const char * const in_pszType, const char * const in_pbValue, int in_nLength)
{
    SemanticStack *pseTemp;

    pseTemp = new SemanticStack(in_pszType, in_pbValue, in_nLength);
    if(pseFirst == NULL)
    {
        pseFirst = pseTemp;
    }
    else
    {
        pseTemp->pseNext = pseFirst;
        pseFirst = pseTemp;
    }
//    PrintStack(); //debug!!!
}

void SimpleSemantic::PopValue(SemanticStack& out_seValue)
{
    SemanticStack *pseTemp;

    if(pseFirst == NULL)
        return;
    out_seValue = pseFirst;
    pseTemp = pseFirst;
    pseFirst = pseFirst->pseNext;
    delete pseTemp;
//    PrintStack(); //debug!!!
}

void SimpleSemantic::PrintStack(void)
{
    SemanticStack *pseList;

    pseList = pseFirst;
    while(pseList != NULL)
    {
      fprintf(stderr, "(%s;%s) ", pseList->pszType, pseList->pbValue);
      pseList = pseList->pseNext;
    }
    fprintf(stderr, "\n");
}

//********************************************SemanticStack*******************************************//

SemanticStack::SemanticStack()
{
    pszType = NULL;
    pbValue = NULL;
    nLength = 0;

    pseNext = NULL;
}

SemanticStack::SemanticStack(const char * const in_pszType, const char * const in_pbValue, int in_nLength)
{
    int nStrLen;

    pszType = NULL;
    pbValue = NULL;
    nLength = 0;
    pseNext = NULL;

    if(in_pszType == NULL || in_pbValue == NULL || in_nLength <= 0)
        return;
    nStrLen = strlen(in_pszType);
    if(nStrLen <= 0)
        return;
    pszType = new char[nStrLen+1];
    memset(pszType, 0, nStrLen+1);
    strncpy(pszType, in_pszType, nStrLen);

    nLength = in_nLength;
    pbValue = new char[in_nLength+1];
    memset(pbValue, 0, nLength+1);
    memcpy(pbValue, in_pbValue, nLength);
}

SemanticStack::~SemanticStack()
{
    if(pszType != NULL)
        delete [] pszType;
    if(pbValue != NULL)
        delete [] pbValue;
}

SemanticStack& SemanticStack::operator =(SemanticStack const * const in_pseValue)
{
    int nStrLen;

    if(in_pseValue->pszType == NULL || in_pseValue->pbValue == NULL || in_pseValue->nLength <= 0)
        return *this;

    if(pszType != NULL)
    {
        delete [] pszType;
        pszType = NULL;
    }
    if(pbValue != NULL)
    {
        delete [] pbValue;
        pbValue = NULL;
    }
    nLength = 0;
    pseNext = NULL;

    nStrLen = strlen(in_pseValue->pszType);
    pszType = new char[nStrLen+1];
    memset(pszType, 0, nStrLen+1);
    strncpy(pszType, in_pseValue->pszType, nStrLen);

    pbValue = new char[in_pseValue->nLength+1];
    memset(pbValue, 0, in_pseValue->nLength+1);
    memcpy(pbValue, in_pseValue->pbValue, in_pseValue->nLength);

    nLength = in_pseValue->nLength;

    return *this;
}

SemanticStack& SemanticStack::operator =(const SemanticStack &in_pseValue)
{
    *this = &in_pseValue;
    return *this;
}

//************************************************DaemonFileTree**************************************//

DaemonFileTree::DaemonFileTree()
{
    ftType = IS_NOTAFILE;
    rocEvent = IS_EMPTY;
    pszName = NULL;
    pszInode = NULL;
    pszUuid = NULL;
    pszTime = NULL;
    pszCrc = NULL;
    pszParent = NULL;
    pszRootToken = NULL;
    pszOldUuid = NULL;

    pdftNext = NULL;
    pdftPrev = NULL;
    pdftUpper = NULL;
    pdftBush = NULL;
}

DaemonFileTree::~DaemonFileTree()
{
    DaemonFileTree *pdftTemp;

    if(pszName != NULL)
        delete [] pszName;
    if(pszInode != NULL)
        delete [] pszInode;
    if(pszUuid != NULL)
        delete [] pszUuid;
    if(pszTime != NULL)
        delete [] pszTime;
    if(pszCrc != NULL)
        delete [] pszCrc;
    if(pszParent != NULL)
        delete [] pszParent;
    if(pszRootToken != NULL)
        delete [] pszRootToken;
    if(pszOldUuid != NULL)
        delete [] pszOldUuid;

    while(pdftNext != NULL)
    {
      pdftTemp = pdftNext;
      while(pdftTemp->pdftNext != NULL)
      {
        pdftTemp = pdftTemp->pdftNext;
      }
      if(pdftTemp->pdftNext != NULL)
      {
        delete pdftTemp->pdftNext;
        pdftTemp->pdftNext = NULL;
      }
      else
      {
        delete pdftNext;
        pdftNext = NULL;
      }
    }

    if(pdftBush != NULL)
      delete pdftBush;
}

DaemonFileTree& DaemonFileTree::operator =(DaemonFileTree *in_pdftFile)
{
    int nLength;

    ftType = in_pdftFile->ftType;
    rocEvent = in_pdftFile->rocEvent;
    pdftNext = NULL;
    pdftBush = NULL;
    pdftPrev = NULL;
    pdftUpper = NULL;

    if(pszName != NULL)
    {
        delete [] pszName;
        pszName = NULL;
    }
    if(pszInode != NULL)
    {
        delete [] pszInode;
        pszInode = NULL;
    }
    if(pszUuid != NULL)
    {
        delete [] pszUuid;
        pszUuid = NULL;
    }
    if(pszTime != NULL)
    {
        delete [] pszTime;
        pszTime = NULL;
    }
    if(pszCrc != NULL)
    {
        delete [] pszCrc;
        pszCrc = NULL;
    }
    if(pszParent != NULL)
    {
        delete [] pszParent;
        pszParent = NULL;
    }
    if(pszRootToken != NULL)
    {
        delete [] pszRootToken;
        pszRootToken = NULL;
    }
    if(pszOldUuid != NULL)
    {
        delete [] pszOldUuid;
        pszOldUuid = NULL;
    }
    if(in_pdftFile->pszName != NULL && (nLength = strlen(in_pdftFile->pszName)) > 0)
    {
        pszName = new char[nLength + 1];
        memset(pszName, 0, nLength + 1);
        strncpy(pszName, in_pdftFile->pszName, nLength);
    }

    if(in_pdftFile->pszInode != NULL && (nLength = strlen(in_pdftFile->pszInode)) > 0)
    {
        pszInode = new char[nLength + 1];
        memset(pszInode, 0, nLength + 1);
        strncpy(pszInode, in_pdftFile->pszInode, nLength);
    }

    if(in_pdftFile->pszUuid != NULL && (nLength = strlen(in_pdftFile->pszUuid)) > 0)
    {
        pszUuid = new char[nLength + 1];
        memset(pszUuid, 0, nLength + 1);
        strncpy(pszUuid, in_pdftFile->pszUuid, nLength);
    }

    if(in_pdftFile->pszTime != NULL && (nLength = strlen(in_pdftFile->pszTime)) > 0)
    {
        pszName = new char[nLength + 1];
        memset(pszTime, 0, nLength + 1);
        strncpy(pszTime, in_pdftFile->pszTime, nLength);
    }

    if(in_pdftFile->pszCrc != NULL && (nLength = strlen(in_pdftFile->pszCrc)) > 0)
    {
        pszCrc = new char[nLength + 1];
        memset(pszCrc, 0, nLength + 1);
        strncpy(pszCrc, in_pdftFile->pszCrc, nLength);
    }

    if(in_pdftFile->pszParent != NULL && (nLength = strlen(in_pdftFile->pszParent)) > 0)
    {
        pszParent = new char[nLength + 1];
        memset(pszParent, 0, nLength + 1);
        strncpy(pszParent, in_pdftFile->pszParent, nLength);
    }

    if(in_pdftFile->pszRootToken != NULL && (nLength = strlen(in_pdftFile->pszRootToken)) > 0)
    {
        pszRootToken = new char[nLength + 1];
        memset(pszRootToken, 0, nLength + 1);
        strncpy(pszRootToken, in_pdftFile->pszRootToken, nLength);
    }
    if(in_pdftFile->pszOldUuid != NULL && (nLength = strlen(in_pdftFile->pszOldUuid)) > 0)
    {
        pszOldUuid = new char[nLength + 1];
        memcpy(pszOldUuid, in_pdftFile->pszOldUuid, nLength + 1);
    }

    return *this;
}

DaemonFileTree& DaemonFileTree::operator =(DaemonFileTree& in_dftFile)
{
    *this = &in_dftFile;
    return *this;
}

void DaemonFileTree::SetValue(const char * const in_pszType, const char * const in_pbValue, int in_nLength)
{
    //int nLength;

    if(in_pszType == NULL || strlen(in_pszType) <= 0)
        return;

    if(in_pbValue == NULL || strlen(in_pbValue) <= 0 || in_nLength <= 0)
        return;

    if(strcmp(in_pszType, "type") == 0)
    {
        if(strcmp(in_pbValue, "file") == 0)
        {
            ftType = IS_FILE;
            return;
        }
        if(strcmp(in_pbValue, "folder") == 0)
        {
            ftType = IS_DIRECTORY;
            return;
        }
        if(strcmp(in_pbValue, "link") == 0)
        {
            ftType = IS_LINK;
            return;
        }
        return;
    }
    if(strcmp(in_pszType, "event") == 0)
    {
        if(strcmp(in_pbValue, "IS_EQUAL") == 0)
        {
            rocEvent = IS_EQUAL;
            return;
        }
        if(strcmp(in_pbValue, "INIT_PROJECT") == 0)
        {
            rocEvent = INIT_PROJECT;
            return;
        }
        if(strcmp(in_pbValue, "IS_DELETED") == 0)
        {
            rocEvent = IS_DELETED;
            return;
        }
        if(strcmp(in_pbValue, "IS_CREATED") == 0)
        {
            rocEvent = IS_CREATED;
            return;
        }
        if(strcmp(in_pbValue, "IS_REPLACED") == 0)
        {
            rocEvent = IS_REPLACED;
            return;
        }
        if(strcmp(in_pbValue, "NEW_NAME") == 0)
        {
            rocEvent = NEW_NAME;
            return;
        }
        if(strcmp(in_pbValue, "NEW_TIME") == 0)
        {
            rocEvent = NEW_TIME;
            return;
        }
        if(strcmp(in_pbValue, "NEW_HASH") == 0)
        {
            rocEvent = NEW_HASH;
            return;
        }
        if(strcmp(in_pbValue, "IS_EMPTY") == 0)
        {
            rocEvent = IS_EMPTY;
            return;
        }
        return;
    }

    if(strcmp(in_pszType, "name") == 0)
    {
        if(pszName != NULL)
            delete [] pszName;
        pszName = new char[in_nLength+1];
        memset(pszName, 0, in_nLength+1);
        strncpy(pszName, in_pbValue, in_nLength);
        return;
    }
    if(strcmp(in_pszType, "inode") == 0)
    {
        if(pszInode != NULL)
            delete [] pszInode;
        pszInode = new char[in_nLength+1];
        memset(pszInode, 0, in_nLength+1);
        strncpy(pszInode, in_pbValue, in_nLength);
        return;
    }
    if(strcmp(in_pszType, "uuid") == 0)
    {
        if(pszUuid != NULL)
            delete [] pszUuid;
        pszUuid = new char[in_nLength+1];
        memset(pszUuid, 0, in_nLength+1);
        strncpy(pszUuid, in_pbValue, in_nLength);
        return;
    }
    if(strcmp(in_pszType, "old_uuid") == 0)
    {
        delete [] pszOldUuid;
        pszOldUuid = new char[in_nLength+1];
        memcpy(pszOldUuid, in_pbValue, in_nLength);
        pszOldUuid[in_nLength] = '\0';
        return;
    }
    if(strcmp(in_pszType, "time") == 0)
    {
        if(pszTime != NULL)
            delete [] pszTime;
        pszTime = new char[in_nLength+1];
        memset(pszTime, 0, in_nLength+1);
        strncpy(pszTime, in_pbValue, in_nLength);
        return;
    }
    if(strcmp(in_pszType, "crc") == 0)
    {
        if(pszCrc != NULL)
            delete [] pszCrc;
        pszCrc = new char[in_nLength+1];
        memset(pszCrc, 0, in_nLength+1);
        strncpy(pszCrc, in_pbValue, in_nLength);
        return;
    }
    if(strcmp(in_pszType, "parent") == 0)
    {
        if(pszParent != NULL)
            delete [] pszParent;
        pszParent = new char[in_nLength+1];
        memset(pszParent, 0, in_nLength+1);
        strncpy(pszParent, in_pbValue, in_nLength);
        return;
    }
    if(strcmp(in_pszType, "token") == 0)
    {
        if(pszRootToken != NULL)
            delete [] pszRootToken;
        pszRootToken = new char[in_nLength+1];
        memset(pszRootToken, 0, in_nLength+1);
        strncpy(pszRootToken, in_pbValue, in_nLength);
        return;
    }
}

//*************************************************SemanticTree***************************************//

SemanticTree::SemanticTree()
{
    pdftFirst = NULL;
    pdftCurrent = NULL;
    pszAddr = NULL;
    res = NULL;
    row = NULL;
    num = 0;
}

SemanticTree::~SemanticTree()
{
  if(pdftFirst != NULL)
  {
    delete pdftFirst;
    pdftCurrent = NULL;
    pdftFirst = NULL;
  }
  if(pszAddr != NULL)
     delete [] pszAddr;
  if(res != NULL)
     mysql_free_result(res);
}

void SemanticTree::AddValue(const char * const in_pszType, const char * const in_pbValue, int in_nLength)
{
  if(pdftFirst == NULL)
    pdftFirst = pdftCurrent = new DaemonFileTree();

  if(pdftCurrent != NULL)
    pdftCurrent->SetValue(in_pszType, in_pbValue, in_nLength);
}

void SemanticTree::GetValue(DaemonFileTree &out_dftValue)
{
  if(pdftCurrent != NULL)
    out_dftValue = pdftCurrent;
}

//add next file
void SemanticTree::AddNext(void)
{
  DaemonFileTree *pdftTemp;

  pdftTemp = new DaemonFileTree();

  if(pdftFirst == NULL)
  {
    pdftFirst = pdftCurrent = pdftTemp;
    return;
  }

  //move current position as LIFO
  pdftCurrent->pdftNext = pdftTemp;
  pdftTemp->pdftPrev = pdftCurrent;
  pdftTemp->pdftUpper = pdftCurrent->pdftUpper;
  pdftCurrent = pdftTemp;
}

//add bush and set it as current file
void SemanticTree::AddBush(void)
{
  if(pdftCurrent == NULL)
  {
    if(pdftFirst != NULL)
      delete pdftFirst;
    pdftFirst = pdftCurrent = new DaemonFileTree();
    return;
  }

  if(pdftCurrent->pdftBush != NULL)
    delete pdftCurrent->pdftBush;
  pdftCurrent->pdftBush = new DaemonFileTree();
  pdftCurrent->pdftBush->pdftUpper = pdftCurrent;
}

//delete a bush of current position
void SemanticTree::SubBush(void)
{
  if(pdftCurrent == NULL)
    return;

  delete pdftCurrent->pdftBush;
  pdftCurrent->pdftBush = NULL;
}

void SemanticTree::GoNext(void)
{
  if(pdftCurrent == NULL)
    return;
  if(pdftCurrent->pdftNext != NULL)
    pdftCurrent = pdftCurrent->pdftNext;
}

void SemanticTree::GoPrev(void)
{
  if(pdftCurrent == NULL)
    return;
  if(pdftCurrent->pdftPrev != NULL)
    pdftCurrent = pdftCurrent->pdftPrev;
}

void SemanticTree::GoBush(void)
{
  if(pdftCurrent == NULL)
    return;
  if(pdftCurrent->pdftBush != NULL)
    pdftCurrent = pdftCurrent->pdftBush;
}

void SemanticTree::GoUpper(void)
{
  if(pdftCurrent == NULL)
    return;
  if(pdftCurrent->pdftUpper != NULL)
    pdftCurrent = pdftCurrent->pdftUpper;
}

void SemanticTree::SetParent(void)
{
  int nLength;
  DaemonFileTree *pdftList;

  if(pdftCurrent != NULL)
  {
    if(pdftCurrent->pdftBush != NULL && pdftCurrent->pszUuid != NULL)
    {
      nLength = strlen(pdftCurrent->pszUuid);
      if(nLength <= 0)
        return;
      pdftList = pdftCurrent->pdftBush;
      while(pdftList != NULL)
      {
        if(pdftList->pszParent != NULL)
          delete [] pdftList->pszParent;
        pdftList->pszParent = new char[nLength + 1];
        memset(pdftList->pszParent, 0, nLength + 1);
        strncpy(pdftList->pszParent, pdftCurrent->pszUuid, nLength);
        pdftList = pdftList->pdftNext;
      }
    }
  }
}

bool SemanticTree::SendRequest(char const * const in_pszRequest)
{
  if(mysql_query(&mysql, in_pszRequest) != 0)
  {
    fprintf(stderr, "SemanticTree::SendRequest() mysql_query(%s) error: %s\n", in_pszRequest, mysql_error(&mysql));
    mysql_close(&mysql);
    return true;
  }
  return false;
}

bool SemanticTree::RecvRequest(const char * const in_pszRequest)
{
  if(res != NULL)
  {
    mysql_free_result(res);
    res = NULL;
  }

  if(mysql_query(&mysql, in_pszRequest) != 0)
  {
    fprintf(stderr, "SemanticTree::RecvRequest() mysql_query(%s) error: %s\n", in_pszRequest, mysql_error(&mysql));
    mysql_close(&mysql);
    return true;
  }
  res = mysql_store_result(&mysql);
  num = mysql_affected_rows(&mysql);
  return false;
}

bool SemanticTree::RecvRequest(char const * const in_pszRequest, char const * const in_pszRequest2)
{
  rtType = FIRST_REQUEST;
  if(RecvRequest(in_pszRequest))
    return true;
  if(num == 0)
  {
    rtType = SECOND_REQUEST;
    if(SendRequest(in_pszRequest2))
    {
      //fprintf(stderr, "%s\n", in_pszRequest2); //debug!!!
      return true;
    }
    return RecvRequest(in_pszRequest);
  }
  return false;
}

bool SemanticTree::RecvRequest(const char * const in_pszRequest, const char * const in_pszRequest2, const char * const in_pszRequest3)
{
  rtType = FIRST_REQUEST;
  if(RecvRequest(in_pszRequest))
    return true;
  if(num == 0)
  {
    rtType = SECOND_REQUEST;
    if(SendRequest(in_pszRequest2) || SendRequest(in_pszRequest3))
      return true;
    return RecvRequest(in_pszRequest);
  }
  return false;
}

bool SemanticTree::NextRow(void)
{
  if(res == NULL)
    return false;
  if(num == 0)
  {
    mysql_free_result(res);
    res = NULL;
    return false;
  }
  num--;
  row = mysql_fetch_row(res);
  if(row == NULL)
  {
    if(res != NULL)
    {
      mysql_free_result(res);
      res = NULL;
    }
    return false;
  }
  return true;
}

void SemanticTree::SendTree(void)
{
  //bool fUpdate;
  //char const *szInodeList;
  ProjectFileList pflFiles;
  ProjectDataList pdlProjects;
  char szName[1024], szReverse[1024];
  DaemonFileTree *pdftTemp, *pdftList;
  char cmd[4096], cmd2[4096], cmd3[4096];
  int id_server, id_project, id_item, id_item_parent, nIndex, nIndexReverse, nType;
  std::set<std::string> seenUuids;
  bool initialSync = false;
  //int inode_server;

  //it's nessessary to do to escape a bug:
  pdftCurrent = NULL;

  if(!mysql_init(&mysql))
    perror("SemanticTree::SendTree() error");

  if(!mysql_real_connect(&mysql, "localhost", "test", "testtesttest", "undefinedSpace", 0, NULL, CLIENT_FOUND_ROWS))
  {
    fprintf(stderr, "SemanticTree::SendTree() error: %s\n", mysql_error(&mysql));
    mysql_close(&mysql);
    return;
  }

  fprintf(stderr, "DB is opened!\n"); //debug!!!

  id_server = -1;
  id_project = -1; //is got by token for IS_CREATED and IS_DELETED messages

  //find id_server
  if(pszAddr == NULL)
    {mysql_close(&mysql);return;}
  //snprintf(cmd, sizeof(cmd), "SELECT a.id,b.id_project FROM servers as a, accords as b,items as c WHERE a.ip=\"%s\" AND b.id_item=c.id AND c.inode=%s", pszAddr, pdftFirst->pszInode);
  snprintf(cmd, sizeof(cmd), "SELECT id FROM servers WHERE ip=\"%s\"", pszAddr);
  fprintf(stderr, "%s\n", cmd); //debug!!!
  snprintf(cmd2, sizeof(cmd2), "INSERT INTO servers(ip,hostname,token) VALUES(\"%s\",\"%s\",\"\")", pszAddr, pszAddr);
  if(RecvRequest(cmd, cmd2)) {fprintf(stderr, "SemanticTree::SendTree() 2 : id_server=%d\n", id_server);return;} //on error

  if(NextRow())
    {if(row[0] != NULL) id_server = atoi(row[0]);}
  else
    {mysql_close(&mysql);fprintf(stderr, "SemanticTree::SendTree() 2 : id_server=%d\n", id_server);return;}

  if(id_server < 0)
   {mysql_close(&mysql);return;}

  while(pdftFirst != NULL)
  {
    if(pdftFirst->rocEvent != IS_EMPTY &&
       (!ValidUuid(pdftFirst->pszUuid) || !Decimal(pdftFirst->pszInode) ||
        (pdftFirst->pszParent != NULL && !ValidUuid(pdftFirst->pszParent))))
    {
      fprintf(stderr, "Missing UUID in filesystem event\n");
      mysql_close(&mysql);
      return;
    }
    if(pdftFirst->pszUuid != NULL &&
       (pdftFirst->rocEvent == INIT_PROJECT || pdftFirst->rocEvent == IS_EQUAL))
      seenUuids.insert(pdftFirst->pszUuid);
    //fprintf(stderr, "event=%d\n", pdftFirst->rocEvent); //debug!!!
    //genarate SQL requests here
    switch((int)pdftFirst->rocEvent)
    {
      case INIT_PROJECT:
        initialSync = true;
        fprintf(stderr, "INIT_PROJECT message\n"); //debug!!!
	//TODO:
        //insert project folder
        //check if inode and name is NULL (!)
        //it is needed to be checked for update firstly (!)
        //...
        memset(szName, 0, sizeof(szName));
        memset(szReverse, 0, sizeof(szReverse));
        for(nIndex = strlen(pdftFirst->pszName)-1, nIndexReverse = 0; (nIndex >= 0) && (nIndexReverse < (int) sizeof(szReverse)); --nIndex, ++nIndexReverse)
        {
          if(pdftFirst->pszName[nIndex] == '/')
            break;
          szReverse[nIndexReverse] = pdftFirst->pszName[nIndex];
        }
        for(nIndex = 0, nIndexReverse = strlen(szReverse)-1; (nIndexReverse >= 0) && (nIndex < (int) sizeof(szName)); --nIndexReverse, ++nIndex)
        {
          memcpy(szName+nIndex, szReverse+nIndexReverse, 1);
        }
        snprintf(cmd, sizeof(cmd), "SELECT id FROM items WHERE id_type=0 AND id_server=%d AND uuid=\"%s\" AND deleted=0",id_server,pdftFirst->pszUuid);
        fprintf(stderr, "%s\n", cmd); //debug!!!
        snprintf(cmd2, sizeof(cmd2), "INSERT IGNORE INTO items(id_server,id_type,id_parent,uuid,inode,name,hash,time) VALUES(%d,%d,NULL,\"%s\",%s,\"%s\",0,NOW())",id_server,0,pdftFirst->pszUuid,pdftFirst->pszInode,szName);
        //update 'deleted' folder, if it is such
        //...
        if(RecvRequest(cmd, cmd2)) {fprintf(stderr, "INIT_PROJECT: no such root folder\n");mysql_close(&mysql);return;}

        if(rtType == SECOND_REQUEST)
        {
          //add "change"
          snprintf(cmd3, sizeof(cmd3), "\
INSERT INTO changes \
 SET id_item=(SELECT id FROM items WHERE uuid=\"%s\" AND deleted=0),\
id_event=(SELECT id FROM events WHERE description=\"%s\"),\
description='{\"name\":\"%s\"}',\
 time=NOW()",pdftFirst->pszUuid,"IS_CREATED",szName);
          if(SendRequest(cmd3)) {fprintf(stderr, "INIT_PROJECT: can not insert into changes\n");mysql_close(&mysql);return;}
          //use previous init of 'cmd'
          if(RecvRequest(cmd)) {fprintf(stderr, "INIT_PROJECT: no such root folder\n");mysql_close(&mysql);return;}
        }

        id_item = -1;

        //get project's item id
        if(NextRow())
          {if(row[0] != NULL) id_item = atoi(row[0]);}

        if(id_item < 0)
          {fprintf(stderr, "INIT_PROJECT: no root folder is found\n");mysql_close(&mysql);return;}
        snprintf(cmd2, sizeof(cmd2), "UPDATE items SET inode=%s WHERE id=%d AND uuid=\"%s\"", pdftFirst->pszInode, id_item, pdftFirst->pszUuid);
        if(SendRequest(cmd2)) return;

        //check if this project exists; create a new one if not:
        snprintf(cmd, sizeof(cmd), "SELECT id FROM projects WHERE path=\"%s\" AND id_server=%d",pdftFirst->pszName,id_server);
        fprintf(stderr, "%s\n", cmd); //debug!!!
        snprintf(cmd2, sizeof(cmd2), "INSERT INTO projects(path,id_server,id_item) VALUES(\"%s\",%d,%d)",pdftFirst->pszName,id_server,id_item);
        fprintf(stderr, "%s\n", cmd2); //debug!!!
        if(RecvRequest(cmd, cmd2)) {fprintf(stderr, "INIT_PROJECT: can not select/insert into projects\n");mysql_close(&mysql);return;}
        if(RecvRequest(cmd) || !NextRow() || row[0] == NULL)
          {fprintf(stderr, "INIT_PROJECT: no such project in DB\n");mysql_close(&mysql);return;}
        id_project = atoi(row[0]);
        snprintf(cmd2, sizeof(cmd2), "INSERT IGNORE INTO accords(id_item,id_project) VALUES(%d,%d)",id_item,id_project);
        if(SendRequest(cmd2)) {mysql_close(&mysql);return;}

        //select all projects with path collisions:
        //snprintf(cmd, sizeof(cmd), "SELECT id,id_item,id_server,time_start,time_stop,UNIX_TIMESTAMP(time_start),UNIX_TIMESTAMP(time_stop) FROM projects WHERE (path LIKE \"%s%\" OR \"%s\" LIKE CONCAT('',path,'%')) AND id_server=%d",pdftFirst->pszName,pdftFirst->pszName,id_server);
        snprintf(cmd, sizeof(cmd), "SELECT id,id_item,id_server,time_start,time_stop,UNIX_TIMESTAMP(time_start),UNIX_TIMESTAMP(time_stop) FROM projects WHERE \"%s\" LIKE CONCAT('',path,'%%') AND id_server=%d",pdftFirst->pszName,id_server);
        fprintf(stderr, "%s\n", cmd); //debug!!!
        if(RecvRequest(cmd)) {fprintf(stderr, "INIT_PROJECT: can not select projects\n");mysql_close(&mysql);return;}
        while(NextRow())
        {
          if(row[0] != NULL && row[1] != NULL && row[2] != NULL && row[3] != NULL && row[4] != NULL && row[5] != NULL && row[6] != NULL)
          {
            if(atol(row[5]) != atol(row[6])) //time_stop - time when the project is stopped
            {
              fprintf(stderr, "SemanticTree::SendTree() : Project \"%s\" is stopped at %s.\n", pdftFirst->pszName, (row[4]!=NULL)?row[4]:"0");
              continue;
            }
            //add projects data to list
            pdlProjects.AddProject(new ProjectData(atoi(row[0]),atoi(row[1]),atoi(row[2]),pdftFirst->pszName,atol(row[3]),atol(row[4])));
          }
          fprintf(stderr, "%s, %s, %s, %s, %s\n", row[0], row[1], row[2], row[3], row[4]); //debug!!!
        }

        //update token for project:
        if(pdftFirst->pszRootToken == NULL)
          {fprintf(stderr, "INIT_PROJECT: token = NULL\n");mysql_close(&mysql);return;}
        snprintf(cmd, sizeof(cmd), "UPDATE servers a,projects b SET a.token=\"%s\" WHERE b.path=\"%s\" AND b.id_server=a.id",pdftFirst->pszRootToken,pdftFirst->pszName);
        fprintf(stderr, "%s\n", cmd); //debug!!!
        if(SendRequest(cmd))
          {fprintf(stderr, "INIT_PROJECT: can not update project token\n");mysql_close(&mysql);return;}
/*
        //update project's accord table by upper-layer path projects
        //1. if no upper projects - exit (?)
        pflFiles.AddFile(new ProjectFile(id_item, atoi(pdftFirst->pszInode), 0)); // 0 - is folder
        //2. get all files from founded folder(s)
        szInodeList = pflFiles.InitInodeList();
        if(szInodeList != NULL)
          snprintf(cmd, sizeof(cmd), "SELECT id_type,inode FROM items WHERE id_parent IN (\"%s\")", szInodeList);
        fprintf(stderr, "%s\n", cmd); //debug!!!
        if(SendRequest(cmd))
          {fprintf(stderr, "INIT_PROJECT: can not get files by id_parent\n");mysql_close(&mysql);return;}
        //3. add founded files to accords (with current project)
        //4. get all folders from founded files
        //5. if no folders - exit
        //6. goto 2
        pflFiles.Clean();
*/
	//TODO:
        //0-6 or generate all accords data every request

        //get list of project's files
        //...
        //compare old and new lists to find files were deleted
        //...
        //add "change" events
        //...
        break;
      case IS_EQUAL:
        static int cnt; //debug!!!
        cnt++; //debug!!!
        fprintf(stderr, "IS_EQUAL message: count=%d, inode=%ld, %s\n", cnt, (unsigned long)atol(pdftFirst->pszInode), pdftFirst->pszName); //debug!!!

	//TODO:
        //save changes
        //check every field if it is NULL (!)
        //...
        if(id_project < 0)
          {mysql_close(&mysql);fprintf(stderr, "SemanticTree::SendTree() : id_project=%d\n", id_project);return;}

        if(pdftFirst->pszName == NULL || pdftFirst->pszInode == NULL || pdftFirst->pszParent == NULL)
            {mysql_close(&mysql);fprintf(stderr, "SemanticTree::SendTree() : wrong data\n");return;}

        //get parent item id
        snprintf(cmd, sizeof(cmd), "SELECT id FROM items WHERE id_server=%d AND uuid=\"%s\" AND deleted=0",id_server,pdftFirst->pszParent);
        if(RecvRequest(cmd)) {fprintf(stderr, "SemanticTree::SendTree() : Error: RecvRequest()");return;}
        id_item_parent = -1;

        //if no such parent
        if(NextRow()){if(row[0] != NULL) id_item_parent = atoi(row[0]);}

        if(id_item_parent < 0) break;

        // get/add an item
        snprintf(cmd, sizeof(cmd), "SELECT id,id_type,id_parent,inode,name,UNIX_TIMESTAMP(time),hash FROM items WHERE id_server=%d AND uuid=\"%s\" AND deleted=0", id_server, pdftFirst->pszUuid);
        snprintf(cmd2, sizeof(cmd2), "INSERT INTO items SET id_server=%d,id_type=%d,id_parent=%d,uuid=\"%s\",inode=%s,name=\"%s\",hash=%s,time=NOW()",
                 id_server,
                 pdftFirst->ftType==IS_DIRECTORY?0:1,
                 id_item_parent,
                 pdftFirst->pszUuid,
                 pdftFirst->pszInode,
                 pdftFirst->pszName,
//                 pdftFirst->pszTime?pdftFirst->pszTime:"0", //do somethint with time
                 pdftFirst->pszCrc?pdftFirst->pszCrc:"0");

        if(RecvRequest(cmd, cmd2)) {fprintf(stderr, "SemanticTree::SendTree() : Error: RecvRequest() 2");return;}
        if(NextRow())
        {
          if(row[0] == NULL || //id
             row[1] == NULL || //id_type
             row[4] == NULL || //name
             row[5] == NULL) //time
            break;

          id_item = atoi(row[0]);
          if(row[3] == NULL || strcmp(row[3], pdftFirst->pszInode) != 0)
          {
            snprintf(cmd2, sizeof(cmd2), "UPDATE items SET inode=%s WHERE id=%d", pdftFirst->pszInode, id_item);
            if(SendRequest(cmd2)) return;
          }
          //add new file to "change"
          if(rtType == SECOND_REQUEST)
          {
            snprintf(cmd3, sizeof(cmd3), "INSERT INTO changes SET id_item=%d,id_event=(SELECT id FROM events WHERE description=\"%s\"),description='{\"name\":\"%s\"}',time=NOW()",id_item,"IS_CREATED",pdftFirst->pszName);
            if(SendRequest(cmd3)) {fprintf(stderr, "SemanticTree::SendTree() : Error: SendRequest()");break;}
          }
          //save project->item to accords table
          snprintf(cmd2, sizeof(cmd2), "INSERT IGNORE INTO accords(id_item,id_project) VALUES(%d,%d)",id_item,id_project);
          if(SendRequest(cmd2)) {fprintf(stderr, "SemanticTree::SendTree() : Error: SendRequest() 2");break;}
          //compare old and new names of the item
          if(strcmp(row[4], pdftFirst->pszName) != 0)
          {
            //update item if it is changed since the last session
            snprintf(cmd2, sizeof(cmd2), "UPDATE items SET name=\"%s\" WHERE id=%d", pdftFirst->pszName, id_item);
            if(SendRequest(cmd2)) {fprintf(stderr, "SemanticTree::SendTree() : Error: SendRequest() 3");break;}
            //add new "change"
            snprintf(cmd2, sizeof(cmd2), "INSERT INTO changes SET id_item=%d,id_event=(SELECT id FROM events WHERE description=\"%s\"),description='{\"name\":\"%s\"}',time=NOW()",id_item,"NEW_NAME",row[4]);
            if(SendRequest(cmd2)) break; //on error
          }

          //compare old and new hash of the item
          if((row[6] != NULL && pdftFirst->pszCrc != NULL && atol(row[6]) != atol(pdftFirst->pszCrc)) ||
             (row[6] == NULL && pdftFirst->pszCrc != NULL && strcmp("0", pdftFirst->pszCrc) != 0) ||
             (row[6] != NULL && atol(row[6]) != 0 && pdftFirst->pszCrc == NULL))
          {
            //update item crc
            snprintf(cmd2, sizeof(cmd2), "UPDATE items SET hash=%ld WHERE id=%d", (pdftFirst->pszCrc==NULL)?0:atol(pdftFirst->pszCrc), id_item);
            if(SendRequest(cmd2)) {fprintf(stderr, "SemanticTree::SendTree() : Error: SendRequest() 4");break;} //on error
            //create "change"
            snprintf(cmd2, sizeof(cmd2), "INSERT INTO changes SET id_item=%d,id_event=(SELECT id FROM events WHERE description=\"%s\"),description='{\"crc\":\"%s\"}',time=NOW()",id_item,"NEW_HASH",(row[6]==NULL)?"0":row[6]);
            if(SendRequest(cmd2)) {fprintf(stderr, "SemanticTree::SendTree() : Error: SendRequest() 5");break;} //on error
          }

          //compare old and new parents of the item (if exists)
          if(row[2] != NULL && atoi(row[2]) != id_item_parent)
          {
            //update parent id
            snprintf(cmd2, sizeof(cmd2), "UPDATE items SET id_parent=%d WHERE id=%d", id_item_parent, id_item);
            if(SendRequest(cmd2)) {fprintf(stderr, "SemanticTree::SendTree() : Error: SendRequest() 6");break;} //on error
            //create "change"
            snprintf(cmd2, sizeof(cmd2), "INSERT INTO changes SET id_item=%d,id_event=(SELECT id FROM events WHERE description=\"%s\"),description='{\"parent\":\"%s\"}',time=NOW()",id_item,"NEW_PARENT",row[2]);
            if(SendRequest(cmd2)) {fprintf(stderr, "SemanticTree::SendTree() : Error: SendRequest() 7");break;} //on error
          }
          //is commented because new time always differs from the item's time
//          fprintf(stderr, "time: %s, %s\n", row[5], pdftFirst->pszTime); //debug!!!
//          //compare old and new time of the item
//          if(row[5] != NULL && pdftFirst->pszTime != NULL && atol(pdftFirst->pszTime) != atol(row[5]))
//          {
//              //update parent id
//              snprintf(cmd2, sizeof(cmd2), "UPDATE items SET time=%ld WHERE id=%d", atol(pfdftFirst->pszParent), id_item);
//              if(SendRequest(cmd2)) return; //on error
//              //create "change"
//              snprintf(cmd2, sizeof(cmd2), "INSERT INTO changes SET id_item=%d,id_event=(SELECT id FROM events WHERE description=\"%s\"),description='{\"time\":\"%s\"}',time=NOW()",id_item,"NEW_TIME",row[5]);
//              if(SendRequest(cmd2)) return; //on error
//          }
        }
        break;
      case IS_DELETED:
        fprintf(stderr, "IS_DELETED message\n"); //debug!!!
        //save "change" and mark this item as deleted
        snprintf(cmd2, sizeof(cmd2), "INSERT INTO changes(id_item,id_event,description,time) SELECT a.id,b.id,'{\"uuid\":\"%s\"}',NOW() FROM items a JOIN items p ON p.id=a.id_parent,events b WHERE p.uuid=\"%s\" AND a.uuid=\"%s\" AND a.deleted=0 AND a.id_server=%d AND b.description=\"%s\"",pdftFirst->pszUuid,pdftFirst->pszParent,pdftFirst->pszUuid,id_server,"IS_DELETED");
        if(SendRequest(cmd2)) {fprintf(stderr, "SemanticTree::SendTree() : Error: SendRequest() 12");return;} //on error
        snprintf(cmd2, sizeof(cmd2), "UPDATE items i JOIN items p ON p.id=i.id_parent SET i.deleted=1 WHERE i.id_server=%d AND i.uuid=\"%s\" AND p.uuid=\"%s\"",id_server,pdftFirst->pszUuid,pdftFirst->pszParent);
        if(SendRequest(cmd2)) {fprintf(stderr, "SemanticTree::SendTree() : Error: SendRequest() 13");return;} //on error
        break;
      case IS_REPLACED:
      {
        // Keep both physical objects. The single change belongs to the new
        // item and links its UUID to the old one in the description.
        if(id_server < 0 || pdftFirst->ftType != IS_FILE ||
           !ValidUuid(pdftFirst->pszUuid) || !ValidUuid(pdftFirst->pszOldUuid) ||
           !ValidUuid(pdftFirst->pszParent) || !Decimal(pdftFirst->pszInode) ||
           !Decimal(pdftFirst->pszCrc) || pdftFirst->pszName == NULL ||
           strcmp(pdftFirst->pszUuid, pdftFirst->pszOldUuid) == 0) break;
        std::string escapedName(strlen(pdftFirst->pszName) * 2 + 1, '\0');
        escapedName.resize(mysql_real_escape_string(&mysql, &escapedName[0],
                          pdftFirst->pszName, strlen(pdftFirst->pszName)));
        snprintf(cmd, sizeof(cmd),
                 "SELECT old.id,old.id_parent FROM items old JOIN items parent ON parent.id=old.id_parent "
                 "WHERE old.uuid='%s' AND old.id_server=%d AND old.deleted=0 "
                 "AND old.name='%s' AND parent.uuid='%s' AND parent.deleted=0",
                 pdftFirst->pszOldUuid,id_server,escapedName.c_str(),pdftFirst->pszParent);
        if(RecvRequest(cmd) || !NextRow()) break;
        const int oldId = atoi(row[0]);
        const int parentId = atoi(row[1]);
        snprintf(cmd, sizeof(cmd), "SELECT id FROM items WHERE uuid='%s'", pdftFirst->pszUuid);
        if(RecvRequest(cmd) || NextRow()) break;
        if(SendRequest("INSERT IGNORE INTO events(id,description) VALUES(16,'IS_REPLACED')")) break;
        if(SendRequest("START TRANSACTION")) break;
        snprintf(cmd, sizeof(cmd),
                 "INSERT INTO items(id_server,id_type,id_parent,uuid,inode,name,time,hash,deleted) "
                 "VALUES(%d,1,%d,'%s',%s,'%s',NOW(),%s,0)",
                 id_server,parentId,pdftFirst->pszUuid,pdftFirst->pszInode,
                 escapedName.c_str(),pdftFirst->pszCrc);
        bool ok = !SendRequest(cmd);
        const unsigned long long newId = ok ? mysql_insert_id(&mysql) : 0;
        if(ok)
        {
          snprintf(cmd, sizeof(cmd), "UPDATE items SET deleted=1 WHERE id=%d AND deleted=0", oldId);
          ok = !SendRequest(cmd) && mysql_affected_rows(&mysql) == 1;
        }
        if(ok)
        {
          snprintf(cmd, sizeof(cmd),
                   "INSERT IGNORE INTO accords(id_item,id_project) SELECT %llu,id_project FROM accords WHERE id_item=%d",
                   newId,oldId);
          ok = !SendRequest(cmd);
        }
        if(ok)
        {
          snprintf(cmd, sizeof(cmd),
                   "INSERT INTO changes(id_item,id_event,description,time) "
                   "VALUES(%llu,16,'{\"old_uuid\":\"%s\",\"new_uuid\":\"%s\"}',NOW())",
                   newId,pdftFirst->pszOldUuid,pdftFirst->pszUuid);
          ok = !SendRequest(cmd);
        }
        if(SendRequest(ok ? "COMMIT" : "ROLLBACK")) return;
        break;
      }
      case IS_CREATED:
        fprintf(stderr, "IS_CREATED message\n"); //debug!!!

        if(id_server < 0) {mysql_close(&mysql);return;}
        //create new item (don't forget to create a new "accords" row)
        switch(pdftFirst->ftType)
        {
            case IS_DIRECTORY:
                nType = 0;
                break;
            case IS_FILE:
                nType = 1;
                break;
            case IS_LINK:
                nType = 2;
                break;
            default:
                nType = -1;
        }
        if(nType < 0)
            {fprintf(stderr, "IS_CREATED: nType < 0\n");break;}
        if(pdftFirst->pszParent == NULL) break;
        snprintf(cmd, sizeof(cmd), "SELECT id FROM items WHERE id_server=%d AND uuid=\"%s\" AND deleted=0", id_server, pdftFirst->pszParent);
        if(RecvRequest(cmd) || !NextRow() || row[0] == NULL) break;
        id_item_parent = atoi(row[0]);
        // A move between directories retains the file UUID and inode.
        snprintf(cmd, sizeof(cmd), "SELECT id,id_parent,inode,name FROM items WHERE id_server=%d AND uuid=\"%s\"", id_server, pdftFirst->pszUuid);
        if(RecvRequest(cmd)) return;
        if(NextRow())
        {
          id_item = atoi(row[0]);
          if(row[2] == NULL || strcmp(row[2], pdftFirst->pszInode) != 0)
          {
            fprintf(stderr, "UUID collision with another inode: %s\n", pdftFirst->pszUuid);
            break;
          }
          const int oldParent = row[1] ? atoi(row[1]) : -1;
          const std::string oldName = row[3] ? row[3] : "";
          if(oldParent != id_item_parent)
          {
            snprintf(cmd2, sizeof(cmd2), "INSERT INTO changes(id_item,id_event,description,time) VALUES(%d,15,'{\"parent\":\"%d\"}',NOW())", id_item, oldParent);
            if(SendRequest(cmd2)) return;
          }
          if(oldName != pdftFirst->pszName)
          {
            snprintf(cmd2, sizeof(cmd2), "INSERT INTO changes(id_item,id_event,description,time) VALUES(%d,5,'{\"name\":\"%s\"}',NOW())", id_item, oldName.c_str());
            if(SendRequest(cmd2)) return;
          }
          snprintf(cmd2, sizeof(cmd2), "UPDATE items SET id_parent=%d,name=\"%s\",inode=%s,deleted=0 WHERE id=%d", id_item_parent, pdftFirst->pszName, pdftFirst->pszInode, id_item);
          if(SendRequest(cmd2)) return;
          snprintf(cmd2, sizeof(cmd2), "INSERT IGNORE INTO accords(id_item,id_project) SELECT %d,id_project FROM accords WHERE id_item=%d", id_item, id_item_parent);
          if(SendRequest(cmd2)) return;
          break;
        }
        snprintf(cmd2, sizeof(cmd2), "INSERT INTO items(id_server,id_type,id_parent,uuid,inode,name,time,hash,deleted) VALUES(%d,%d,%d,\"%s\",%s,\"%s\",NOW(),%s,0)",id_server,nType,id_item_parent,pdftFirst->pszUuid,pdftFirst->pszInode,pdftFirst->pszName,pdftFirst->pszCrc?pdftFirst->pszCrc:"0");
        if(SendRequest(cmd2)) {fprintf(stderr, "SemanticTree::SendTree() : Error: SendRequest() 10");return;} //on error

        snprintf(cmd2, sizeof(cmd2), "INSERT INTO changes(id_item,id_event,description,time) SELECT a.id,b.id,'{\"name\":\"%s\"}',NOW() FROM items a, events b WHERE a.uuid=\"%s\" AND a.deleted=0 AND a.id_server=%d AND b.description=\"%s\"",pdftFirst->pszName,pdftFirst->pszUuid,id_server,"IS_CREATED");
        if(SendRequest(cmd2)) {fprintf(stderr, "SemanticTree::SendTree() : Error: SendRequest() 11");return;} //on error

        // Find all projects linked to the parent, including nested projects.
        snprintf(cmd, sizeof(cmd), "INSERT IGNORE INTO accords(id_item,id_project) SELECT child.id,a.id_project FROM accords a JOIN items child ON child.uuid=\"%s\" AND child.id_server=%d WHERE a.id_item=%d", pdftFirst->pszUuid, id_server, id_item_parent);
        if(SendRequest(cmd)) {fprintf(stderr, "SemanticTree::SendTree() : Error: SendRequest() 11_1");return;} //on error

        break;
      case NEW_NAME:
        fprintf(stderr, "NEW_NAME message\n"); //debug!!!
        //create new "change" and change item name
        snprintf(cmd2, sizeof(cmd2), "INSERT INTO changes(id_item,id_event,description,time) SELECT a.id,b.id,CONCAT('{\"name\":\"',a.name,'\"}'),NOW() FROM items as a,events as b WHERE a.uuid=\"%s\" AND a.id_server=%d AND b.description=\"%s\"",pdftFirst->pszUuid,id_server,"NEW_NAME");
        if(SendRequest(cmd2)) {fprintf(stderr, "SemanticTree::SendTree() : Error: SendRequest() 14");return;} //on error

        snprintf(cmd2, sizeof(cmd2), "UPDATE items SET name=\"%s\",inode=%s WHERE id_server=%d AND uuid=\"%s\"",pdftFirst->pszName,pdftFirst->pszInode,id_server,pdftFirst->pszUuid);
        if(SendRequest(cmd2)) {fprintf(stderr, "SemanticTree::SendTree() : Error: SendRequest() 15");return;} //on error
        break;
      case NEW_TIME:
        fprintf(stderr, "NEW_TIME message\n"); //debug!!!
        //change item time and create new "change"
        //...
        break;
      case NEW_HASH:
        fprintf(stderr, "NEW_HASH message\n"); //debug!!!
        //change item hash and create new "change"
        snprintf(cmd2, sizeof(cmd2), "INSERT INTO changes(id_item,id_event,description,time) SELECT b.id,a.id,CONCAT('{\"crc\":\"',b.hash,'\"}'),NOW() FROM events a,items b WHERE a.description=\"%s\" AND b.uuid=\"%s\" AND b.deleted=0 AND b.id_server=%d","NEW_HASH",pdftFirst->pszUuid,id_server);
        if(SendRequest(cmd2)) {fprintf(stderr, "SemanticTree::SendTree() : Error: SendRequest() 8");return;} //on error
        snprintf(cmd2, sizeof(cmd2), "UPDATE items SET hash=%s,inode=%s WHERE uuid=\"%s\" AND deleted=0 AND id_server=%d",pdftFirst->pszCrc?pdftFirst->pszCrc:"0",pdftFirst->pszInode,pdftFirst->pszUuid,id_server);
        if(SendRequest(cmd2)) {fprintf(stderr, "SemanticTree::SendTree() : Error: SendRequest() 9");return;} //on error
        break;
      case IS_EMPTY:
        //fprintf(stderr, "IS_EMPTY message\n"); //debug!!!
        break;
    }

    //switch file (get next JSON note)
    pdftTemp = pdftFirst;
    if(pdftFirst != NULL && pdftFirst->pdftBush != NULL)
    {
      pdftFirst = pdftFirst->pdftBush;
      //append old_first->next to a new first
      pdftList = pdftFirst;
      while(pdftList->pdftNext != NULL)
      {
        pdftList = pdftList->pdftNext;
      }
      pdftList->pdftNext = pdftTemp->pdftNext;
    }
    else
    {
      pdftFirst = pdftFirst->pdftNext;
    }
    pdftTemp->pdftNext = NULL;
    pdftTemp->pdftBush = NULL;
    delete pdftTemp;
  }

  if(initialSync && id_project >= 0)
  {
    if(SendRequest("CREATE TEMPORARY TABLE seen_uuids (uuid CHAR(36) PRIMARY KEY)")) return;
    for(const std::string &uuid : seenUuids)
    {
      snprintf(cmd, sizeof(cmd), "INSERT INTO seen_uuids(uuid) VALUES(\"%s\")", uuid.c_str());
      if(SendRequest(cmd)) return;
    }
    snprintf(cmd, sizeof(cmd),
             "INSERT INTO changes(id_item,id_event,description,time) "
             "SELECT i.id,4,CONCAT('{\"uuid\":\"',i.uuid,'\"}'),NOW() "
             "FROM items i JOIN accords a ON a.id_item=i.id "
             "LEFT JOIN seen_uuids s ON s.uuid=i.uuid "
             "WHERE a.id_project=%d AND s.uuid IS NULL AND i.deleted=0", id_project);
    if(SendRequest(cmd)) return;
    snprintf(cmd, sizeof(cmd),
             "UPDATE items i JOIN accords a ON a.id_item=i.id "
             "LEFT JOIN seen_uuids s ON s.uuid=i.uuid "
             "SET i.deleted=1 WHERE a.id_project=%d AND s.uuid IS NULL AND i.deleted=0", id_project);
    if(SendRequest(cmd)) return;
  }

  if(res != NULL)
  {
    mysql_free_result(res);
    res = NULL;
  }
  mysql_close(&mysql);

  //fprintf(stderr, "SemanticTree::SendTree() end of the parsing."); //debug!!!
}

void SemanticTree::SetAddr(const char * const in_pszAddr)
{
  int nLength;

  if(in_pszAddr == NULL || (nLength = strlen(in_pszAddr)) == 0)
    return;

  if(pszAddr != NULL)
    delete [] pszAddr;

  pszAddr = new char[nLength + 1];
  memset(pszAddr, 0, nLength + 1);
  strncpy(pszAddr, in_pszAddr, nLength);

  return;
}

void SemanticTree::PrintTree(void)
{
  DaemonFileTree *pdftList, *pdftTemp, *pdftLast;

  int cnt; //debug!!!
  cnt = 0; //debug!!!

  fprintf(stderr, "\n\n");
  pdftList = pdftFirst;
  while(pdftList != NULL)
  {
    fprintf(stderr, "%s, %s, %s\n", pdftList->pszName, pdftList->pszInode, pdftList->pszParent);
    pdftTemp = pdftList->pdftBush;
    while(pdftTemp != NULL)
    {
      fprintf(stderr, "\t%s, %s, %s\n", pdftTemp->pszName, pdftTemp->pszInode, pdftTemp->pszParent);
      pdftLast = pdftTemp->pdftBush;
      while(pdftLast != NULL)
      {
        fprintf(stderr, "\t\t%s, %s, %s\n", pdftLast->pszName, pdftLast->pszInode, pdftLast->pszParent);
        pdftLast = pdftLast->pdftNext;
        cnt++; //debug!!!
      }
      pdftTemp = pdftTemp->pdftNext;
      cnt++; //debug!!!
    }
    pdftList = pdftList->pdftNext;
    cnt++; //debug!!!
  }
  fprintf(stderr, "\n\n");
  fprintf(stderr, "cnt=%d\n",cnt); //debug!!!
}

//*****************************************ProjectData****************************************//

ProjectData::ProjectData(int in_nId, int in_nIdItem, int in_nIdServer, const char * const in_szPath, time_t in_ttStartTime, time_t in_ttStopTime)
{
    int nLen;

    nId = in_nId;
    (void)in_nIdItem;
    nIdServer = in_nIdServer;
    if(in_szPath != NULL)
    {
        nLen = strlen(in_szPath);
        szPath = new char[nLen+1];
        memset(szPath, 0, nLen+1);
        strncpy(szPath, in_szPath, nLen);
    }
    else
    {
        szPath = NULL;
    }
    ttStartTime = in_ttStartTime;
    ttStopTime = in_ttStopTime;

    ppdNext = NULL;
}

ProjectData::~ProjectData()
{
    if(szPath != NULL)
        delete [] szPath;
}

ProjectDataList::ProjectDataList()
{
    ppdFirst = NULL;
}

ProjectDataList::ProjectDataList(ProjectData * const in_ppdFirst)
{
    ppdFirst = in_ppdFirst;
}

ProjectDataList::~ProjectDataList()
{
    ProjectData *ppdList;

    while(ppdFirst != NULL)
    {
        ppdList = ppdFirst;
        ppdFirst = ppdFirst->ppdNext;
        delete ppdList;
    }
}

void ProjectDataList::AddProject(ProjectData * const in_ppdProject)
{
    ProjectData *ppdList;

    if(ppdFirst == NULL)
    {
        ppdFirst = in_ppdProject;
        return;
    }

    ppdList = ppdFirst;
    while(ppdList->ppdNext != NULL)
    {
        ppdList = ppdList->ppdNext;
    }
    ppdList->ppdNext = in_ppdProject;
}

int ProjectDataList::GetIdItemByIdProject(int in_nIdProject)
{
    ProjectData *ppdList;

    ppdList = ppdFirst;
    while(ppdList != NULL)
    {
        if(ppdList->nId == in_nIdProject)
            return ppdList->nIdItem;
    }
    return -1;
}

//********************************************ProjectFile(List)*******************************//

ProjectFile::ProjectFile(int in_nId, int in_nInode, int in_nType)
{
    nId = in_nId;
    nInode = in_nInode;
    nType = in_nType;
    ppfNext = NULL;
}

ProjectFileList::ProjectFileList()
{
    nCount = 0;
    nMaxInode = 0;
    szFilesInodeList = NULL;
    ppfFirst = NULL;
}

ProjectFileList::~ProjectFileList()
{
    ProjectFile *ppfTemp;

    while(ppfFirst != NULL)
    {
        ppfTemp = ppfFirst;
        ppfFirst = ppfFirst->ppfNext;
        delete ppfTemp;
    }
    if(szFilesInodeList != NULL)
        delete [] szFilesInodeList;
    nMaxInode = 0;
}

void ProjectFileList::AddFile(ProjectFile * const in_ppfFile)
{
    if(ppfFirst == NULL)
    {
        nCount = 1;
        nMaxInode = in_ppfFile->nInode;
        ppfFirst = in_ppfFile;
        return;
    }

    in_ppfFile->ppfNext = ppfFirst;
    ppfFirst = in_ppfFile;
    nCount++;
    if(in_ppfFile->nInode > nMaxInode)
        nMaxInode = in_ppfFile->nInode;
}

void ProjectFileList::Clean()
{
    ProjectFile *ppfTemp;

    while(ppfFirst != NULL)
    {
        ppfTemp = ppfFirst;
        ppfFirst = ppfFirst->ppfNext;
        delete ppfTemp;
    }
    nCount = 0;
    nMaxInode = 0;
}

const char * const ProjectFileList::InitInodeList()
{
    char strNumber[32];
    ProjectFile *ppfPtr;
    int nInode, nBytes, nIndex;

    if(szFilesInodeList != NULL)
    {
        delete [] szFilesInodeList;
        szFilesInodeList = NULL;
    }
    if(ppfFirst == NULL)
        return NULL;

    for(nInode = nMaxInode, nBytes = 0; nInode > 0; ++nBytes)
    {
        nInode = nInode / 10;
    }
    szFilesInodeList = new char[nCount*nBytes+1];
    memset(szFilesInodeList, 0, nCount*nBytes+1);

    for(ppfPtr = ppfFirst, nIndex = 0; nIndex < nCount && ppfPtr != NULL; ++nIndex)
    {
        if(ppfPtr == ppfFirst)
            snprintf(strNumber, sizeof(strNumber), "%d", ppfPtr->nInode);
        else
            snprintf(strNumber, sizeof(strNumber), ",%d", ppfPtr->nInode);
        strcat(szFilesInodeList, strNumber);
    }

    return szFilesInodeList;
}

//*****************************************DaemonSemanticData*********************************//

void JSONType(StackValue *in_psvValue);
void JSONStringValue(StackValue *in_psvValue);
void JSONDigit(StackValue *in_psvValue);
void JSONLexem(StackValue *in_psvValue);
void JSONSomeType(StackValue *in_psvValue);
void JSONSomeContent(StackValue *in_psvValue);
void JSONEmptyContent(StackValue *in_psvValue);
void JSONEndContent(StackValue *in_psvValue);
void JSONPackets(StackValue *in_psvValue);
void JSONSetParent(StackValue *in_psvValue);
void JSONSend(StackValue *in_psvValue);
void JSONPrint(StackValue *in_psvValue);

SimpleSemantic seSemantic;
SemanticTree stTree;
SemanticFunctions psfFunctions[] ={{"JSONType", JSONType},
                                  {"JSONStringValue", JSONStringValue},
                                  {"JSONDigit", JSONDigit},
                                  {"JSONLexem", JSONLexem},
                                  {"JSONSomeContent", JSONSomeContent},
                                  {"JSONEmptyContent", JSONEmptyContent},
                                  {"JSONEndContent", JSONEndContent},
                                  {"JSONSomeType", JSONSomeType},
                                  {"JSONPackets", JSONPackets},
                                  {"JSONSetParent", JSONSetParent},
                                  {"JSONSend", JSONSend},
                                  {"JSONPrint", JSONPrint},
                                  {NULL, NULL}};

DaemonSemanticData::DaemonSemanticData()
{
  char szSystemBuff[] = "\
<GOAL>::=<пакет>END|<начало массива><набор пакетов><конец массива>END\n\
<набор пакетов>::=<пакет>|<набор пакетов>,<пакет>\n\
<пакет>::={<список>}\n\
<список>::=<свойство>|<список>,<свойство>\n\
<свойство>::=<тип><значение свойства>|<массив>\n\
<массив>::=<тип><начало массива><набор пакетов><конец массива>|<тип><начало массива><конец массива>\n\
<значение свойства>::=\"<значение>\"|'<значение>'|DSTRING|SSTRING|<число>|<значение>\n\
<число>::=DIGIT\n\
<тип>::=\"<значение>\":|DSTRING:\n\
<значение>::=LEXEM\n\
<начало массива>::=[\n\
<конец массива>::=]\n\
";
  char szFunctions[] = "\n\
GOAL::=JSONSend|JSONSend\n\
<набор пакетов>::=JSONPackets|JSONPackets\n\
<пакет>::=JSONSetParent\n\
<список>::=|\n\
<свойство>::=JSONSomeType|\n\
<массив>::=|JSONEmptyContent\n\
<значение свойства>::=||JSONLexem|JSONLexem||JSONStringValue\n\
<число>::=JSONDigit\n\
<тип>::=JSONType|JSONType\n\
<значение>::=JSONLexem\n\
<начало массива>::=JSONSomeContent\n\
<конец массива>::=JSONEndContent\n\
";

  sAnalyzer.PrepareAnalyzer(szSystemBuff);
  sAnalyzer.SetSemanticFunctions(psfFunctions);
  sAnalyzer.PrepareFunctionsNames(szFunctions);
}

DaemonSemanticData::~DaemonSemanticData()
{
}

int DaemonSemanticData::Analyze(char const * const in_pszAddr, char const * const in_pszBuff)
{
  bool fBreak;
  StackValue *psvValue;

  if(in_pszBuff == NULL || strlen(in_pszBuff) == 0)
    return 1;

  stTree.SetAddr(in_pszAddr);
  sAnalyzer.PrepareCodeFromBuff(in_pszBuff);
//  sAnalyzer.PrintBuff(); //debug!!!
  sAnalyzer.CleanStack();

  //analyze code
  while((psvValue = sAnalyzer.AnalyzeNext()) != NULL)
  {
    fBreak = false;
    if(!(psvValue->NoGoal("GOAL") && psvValue->NoGoal("ERROR\t")))
      fBreak = true;
//    if(psvValue->NoGoal("NOTHING\t")) //debug!!!
//      psvValue->PrintNext(); //debug!!!
    psvValue->ClearNextValues();
    delete psvValue;
    if(fBreak)
      break;
  }

  fprintf(stderr, "\n");
  return 0;
}

void JSONType(StackValue *in_psvValue)
{
//      fprintf(stderr, "JSONType(), %s\n", in_psvValue->psvRuleValues->srResult.pbValue);
      seSemantic.PushValue("type", in_psvValue->psvRuleValues->srResult.pbValue, in_psvValue->psvRuleValues->srResult.lSize);
}

void JSONStringValue(StackValue *in_psvValue)
{
      SemanticStack seValue;
      seSemantic.PopValue(seValue);
      seSemantic.PushValue(seValue);
}

void JSONDigit(StackValue *in_psvValue)
{
      seSemantic.PushValue("digit", in_psvValue->psvRuleValues->srResult.pbValue, in_psvValue->psvRuleValues->srResult.lSize);
}

void JSONLexem(StackValue *in_psvValue)
{
      seSemantic.PushValue("lexem", in_psvValue->psvRuleValues->srResult.pbValue, in_psvValue->psvRuleValues->srResult.lSize);
}

void JSONSomeType(StackValue *in_psvValue)
{
      SemanticStack seValue1, seValue2;
      seSemantic.PopValue(seValue2);
      seSemantic.PopValue(seValue1);
      stTree.AddValue(seValue1.pbValue, seValue2.pbValue, seValue2.nLength);
}

void JSONSomeContent(StackValue *in_psvValue)
{
      stTree.AddBush();
      stTree.GoBush();
}

void JSONEmptyContent(StackValue *in_psvValue)
{
        stTree.SubBush();
}

void JSONEndContent(StackValue *in_psvValue)
{
      SemanticStack seValue;
      seSemantic.PopValue(seValue);
      stTree.GoUpper();
}

void JSONPackets(StackValue *in_psvValue)
{
      stTree.AddNext();
}

void JSONSetParent(StackValue *in_psvValue)
{
      stTree.SetParent();
      //stTree.PrintTree();

//      static int cnt; //debug!!!
//      cnt++; //debug!!!
//      fprintf(stderr, "JSONSetParent() : count=%d\n", cnt); //debug!!!
}

void JSONSend(StackValue *in_psvValue)
{
      //stTree.PrintTree(); //debug!!!
      stTree.SendTree();
}

void JSONPrint(StackValue *in_psvValue)
{
      stTree.PrintTree();
}
