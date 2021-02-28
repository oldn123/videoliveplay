#include <windows.h>
#include <string>
using namespace std;

extern void * play(const string& inputPath, HWND h);

extern void close(void* p);

extern int onIdle(void* p);