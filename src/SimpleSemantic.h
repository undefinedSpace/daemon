//SimpleSemantic.h
//Author: Bombo
//17.04.2017
//Основа для семантического анализатора

#pragma once

#include<stdio.h>
#include<stdlib.h>
#include<mysql/mysql.h>

#include"DirSnapshot.h"
#include"SimpleSyntax.h"

enum ResultType {RESULT_ERROR=0,FIRST_REQUEST,SECOND_REQUEST};

struct SemanticStack
{
    char *pszType;
    char *pbValue;
    int nLength;

    SemanticStack *pseNext;

    SemanticStack();
    SemanticStack(char const * const in_pszType, char const * const in_pbValue, int in_nLength);
    ~SemanticStack();

    SemanticStack& operator =(const SemanticStack * const in_pseValue);
    SemanticStack& operator =(const SemanticStack& in_pseValue);
};

struct DaemonFileTree
{
    DaemonFileType ftType;
    ResultOfCompare rocEvent;
    char *pszName;
    char *pszInode;
    char *pszUuid;
    char *pszTime;
    char *pszCrc;
    char *pszParent;
    char *pszRootToken;
    char *pszOldUuid;

    DaemonFileTree *pdftNext;
    DaemonFileTree *pdftPrev;
    DaemonFileTree *pdftUpper;
    DaemonFileTree *pdftBush;

    DaemonFileTree();
    ~DaemonFileTree();

    DaemonFileTree& operator =(DaemonFileTree *in_pdftFile);
    DaemonFileTree& operator =(DaemonFileTree& in_dftFile);

    void SetValue(char const * const in_pszType, char const * const in_pbValue, int in_nLength);
};

class SemanticTree
{
    DaemonFileTree *pdftFirst;
    DaemonFileTree *pdftCurrent;

    char *pszAddr;
    MYSQL mysql;
    MYSQL_RES *res;
    MYSQL_ROW row;
    int num;
    ResultType rtType;

public:
    SemanticTree();
    ~SemanticTree();

    void AddValue(char const * const in_pszType, char const * const in_pbValue, int in_nLength);
    void GetValue(DaemonFileTree &out_dftValue);

    void AddNext(void);
    void AddBush(void);
    void SubBush(void);

    void GoNext(void);
    void GoPrev(void);
    void GoBush(void);
    void GoUpper(void);

    void SetParent(void);

    bool SendRequest(char const * const in_pszRequest);
    bool RecvRequest(char const * const in_pszRequest);
    bool RecvRequest(char const * const in_pszRequest, char const * const in_pszRequest2);
    bool RecvRequest(char const * const in_pszRquest, char const * const in_pszRequest2, char const * const in_pszRequest3);
    bool NextRow(void);
    void SendTree(void);

    void SetAddr(char const * const in_pszAddr);

    void PrintTree(void);
};

class DaemonSemanticData
{
//  SimpleSemantic seSemantic;
    SimpleSyntax sAnalyzer;

public:
  DaemonSemanticData();
  ~DaemonSemanticData();

  int Analyze(const char * const in_pszAddr, char const * const in_pszBuff);
};

class SimpleSemantic
{
    //semantic result stack
    SemanticStack *pseFirst;

public:
    //file tree
    SemanticTree stTree;

public:
    SimpleSemantic();
    ~SimpleSemantic();

    void PushValue(SemanticStack &in_seValue);
    void PushValue(char const * const in_pszType, char const * const in_pbValue, int in_nLength);
    void PopValue(SemanticStack& out_seValue);

    void PrintStack(void);
};

//======================================
struct ProjectData
{
    int nId;
    int nIdItem;
    int nIdServer;
    char *szPath;
    time_t ttStartTime;
    time_t ttStopTime;

    ProjectData(int in_nId, int in_nIdItem, int in_nIdServer, char const * const in_szPath, time_t in_ttStartTime, time_t in_ttStopTime);
    ~ProjectData();

    ProjectData *ppdNext;
};

class ProjectDataList
{
    ProjectData *ppdFirst;

public:
    ProjectDataList();
    ProjectDataList(ProjectData * const in_ppdFirst);
    ~ProjectDataList();

    void AddProject(ProjectData * const in_ppdProject);
    int GetIdItemByIdProject(int in_nIdProject);
};

struct ProjectFile
{
    int nId;
    int nInode;
    int nType;
    ProjectFile *ppfNext;

    ProjectFile(int in_nId, int in_nInode, int in_nType);
};

class ProjectFileList
{
    int nCount;
    int nMaxInode;
    char *szFilesInodeList;
    ProjectFile *ppfFirst;

public:
    ProjectFileList();
    ~ProjectFileList();

    void AddFile(ProjectFile * const ppfFile);
    void Clean(void);
    char const * const InitInodeList(void);
};
