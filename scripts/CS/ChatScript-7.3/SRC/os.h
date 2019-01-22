#ifndef _OSH_
#define _OSH_

#ifdef INFORMATION
Copyright (C) 2011-2017 by Bruce Wilcox

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
#endif

#define SAFE_BUFFER_MARGIN 2000

// EXCEPTION/ERROR
// error recovery
#define SERVER_RECOVERY 4
extern jmp_buf scriptJump[5];
extern int jumpIndex;

void JumpBack();
void myexit(char* msg, int code = 4);
void mystart(char* msg);

#define NORMALFILES 0
#define MONGOFILES 1
#define POSTGRESFILES 2
extern bool logged;
extern int filesystemOverride;
#define MAX_GLOBAL 600
extern char* ruleDepth[MAX_GLOBAL];
extern char* nameDepth[MAX_GLOBAL];
extern char* tagDepth[MAX_GLOBAL][25];

#define RECORD_SIZE 4000

// MEMORY SYSTEM
extern char* releaseStackDepth[MAX_GLOBAL];
extern unsigned int maxBufferLimit;
extern unsigned int maxReleaseStackGap;
extern unsigned int maxBufferSize;
extern unsigned int maxBufferUsed;	
extern unsigned int bufferIndex;
extern unsigned int baseBufferIndex;
extern unsigned int overflowIndex;
extern char* buffers;
extern bool showmem;
extern unsigned long maxHeapBytes;
extern char* heapBase;
extern char* heapFree;
extern char* stackFree;
extern char* stackStart;
extern char* heapEnd;
extern unsigned long minHeapAvailable;
extern int loglimit;

char* Index2Heap(unsigned int offset);
inline unsigned int Heap2Index(char* str) {return (!str) ? 0 : (unsigned int)(heapBase - str);}

// MEMORY SYSTEM
void ResetBuffers();
char* AllocateBuffer(char*name = "");
void FreeBuffer(char*name = "");
void CloseBuffers();
char* AllocateStack(char* word, size_t len = 0, bool localvar = false);
void ReleaseInfiniteStack();
void ReleaseStack(char* word);
char* InfiniteStack(char*& limit,char* caller);
char* InfiniteStack64(char*& limit,char* caller);
void CompleteBindStack64(int n,char* base);
void CompleteBindStack();
bool AllocateStackSlot(char* variable);
char* RestoreStackSlot(char* variable,char* slot);
char* AllocateHeap(char* word,size_t len = 0,int bytes= 1,bool clear = false,bool purelocal = false);
bool PreallocateHeap(size_t len);
void ProtectNL(char* buffer);
void InitStackHeap();
void FreeStackHeap();
bool KeyReady();
bool InHeap(char* ptr);
bool InStack(char* ptr);

// FILE SYSTEM
int MakeDirectory(char* directory);
void EncryptInit(char* params);
void DecryptInit(char* params);
void EncryptRestart();
extern unsigned int currentFileLine;
extern unsigned int maxFileLine;
extern char currentFilename[MAX_WORD_SIZE];
int FClose(FILE* file);
void InitFileSystem(char* untouchedPath,char* readablePath,char* writeablePath);
void C_Directories(char* x);
void ResetEncryptTags();
void StartFile(const char* name);
int FileSize(FILE* in,char* buffer,size_t allowedSize);
void FileDelete(const char* filename);
FILE* FopenStaticReadOnly(const char* name);
FILE* FopenReadOnly(const char* name);
FILE* FopenReadNormal(char* name);
FILE* FopenReadWritten(const char* name);
FILE* FopenBinaryWrite(const char* name); // only for binary data files
FILE* FopenUTF8Write(const char* filename);
FILE* FopenUTF8WriteAppend(const char* filename, const char* flags = "ab");
typedef void (*FILEWALK)(char* name, uint64 flag);
void CopyFile2File(const char* newname,const char* oldname,bool autoNumber);
bool LogEndedCleanly();

#ifdef INFORMATION
Normally user topic files are saved on the local filesystem using fopen, fclose, fread, fwrite, fseek, ftell calls.
If you want to store them elsewhere, like in a database, you can pass into InitSystem a nonnull USERFILESYSTEM block.
The pattern of calls is that the system will do a create or open call. A nonzero value will be passed around, but it doesnt really matter
because you can assume only one "FILE" is in use at a time so you can manage that with globals outside of CS. 
For writing, the system will  create the file (it should be "binary utf8"), perform a single write,  and then close it.
For reading, the system will first open it, ask for the size of its contents, then read once and close it.
#endif

typedef FILE* (*UserFileCreate)(const char* name);
typedef FILE* (*UserFileOpen)(const char* name);
typedef int (*UserFileClose)(FILE*);
typedef size_t (*UserFileRead)(void* buffer,size_t size, size_t count, FILE* file);
typedef size_t (*UserFileWrite)(const void* buffer,size_t size, size_t count, FILE* file);
typedef int (*UserFileSize)(FILE* file, char* buffer, size_t allowedSize);
typedef void (*UserFileDelete)(const char* name);
typedef size_t (*UserFileDecrypt)(void* buffer,size_t size, size_t count, FILE* file,char* filekind);
typedef size_t (*UserFileEncrypt)(const void* buffer,size_t size, size_t count, FILE* file,char* filekind);

typedef struct USERFILESYSTEM //  how to access user topic data
{
	UserFileCreate userCreate;  // "wb" implied
	UserFileOpen userOpen; 
	UserFileClose userClose;
	UserFileRead userRead;
	UserFileWrite userWrite;
	UserFileDelete userDelete;
	UserFileEncrypt userEncrypt;
	UserFileDecrypt userDecrypt;

} USERFILESYSTEM;

extern USERFILESYSTEM userFileSystem;
void InitUserFiles();
void WalkDirectory(char* directory,FILEWALK function, uint64 flags);
size_t DecryptableFileRead(void* buffer,size_t size, size_t count, FILE* file,bool decrypt,char* filekind);
size_t EncryptableFileWrite(void* buffer,size_t size, size_t count, FILE* file,bool encrypt,char* filekind);
char* GetUserPath(char* name);

// TIME

#define SKIPWEEKDAY 4 // from gettimeinfo

char* GetTimeInfo(struct tm* ptm, bool nouser=false,bool utc=false);
char* GetMyTime(time_t curr);
void mylocaltime (const time_t * timer, struct tm* ptm);
void myctime(time_t * timer,char* buffer);

#ifdef __MACH__
void clock_get_mactime(struct timespec &ts);
#endif
clock_t ElapsedMilliseconds();
#ifndef WIN32
unsigned int GetFutureSeconds(unsigned int seconds);
#endif

// LOGGING
#define LOGGING_SET 1
#define LOGGING_NOT_SET 2

#define SERVERLOG 0
#define STDUSERLOG 1
#define STDDEBUGLOG 2
#define ECHOSTDTRACELOG 3
#define STDTRACELOG 4
#define STDTIMELOG 5

#define BADSCRIPTLOG 9
#define BUGLOG 10
#define STDTRACETABLOG 101
#define STDTRACEATTNLOG 201
#define STDTIMETABLOG 301

extern bool userEncrypt;
extern bool ltmEncrypt;
extern bool echo;
extern bool showDepth;
extern bool oob;
extern bool silent;
extern uint64 logCount;
extern char* testOutput;

#define ReportBug(...) { Bug(); Log(BUGLOG, __VA_ARGS__);}
#define DebugPrint(...) Log(STDDEBUGLOG, __VA_ARGS__)
extern char logFilename[MAX_WORD_SIZE];
extern bool logUpdated;
extern char* logmainbuffer; 
extern int logsize;
extern int outputsize;
extern char serverLogfileName[200];
extern int userLog;
extern int serverLog;
extern bool serverPreLog;
extern bool serverctrlz;

unsigned int Log(unsigned int spot,const char * fmt, ...);
void ChangeDepth(int value,char* where,bool nostackCutboack = false,char* code = NULL,FunctionResult result = NOPROBLEM_BIT);
void BugBacktrace(FILE* out);
void Bug();

// RANDOM NUMBERS

#define MAXRAND 256

extern unsigned int randIndex,oldRandIndex;

unsigned int random(unsigned int range);
uint64 Hashit(unsigned char * data, int len,bool & hasUpperCharacters, bool & hasUTF8Characters);

#endif
