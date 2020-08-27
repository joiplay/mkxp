#include "debugwriter.h"
#ifdef __ANDROID__
#include <android/log.h>
extern "C" {
	void writeDebugJNI(char* message);
}
#endif

Debug::~Debug()
{
#ifdef __ANDROID__
    __android_log_write(ANDROID_LOG_DEBUG, "mkxp", buf.str().c_str());
    int l = buf.str().length();
    char chars[l + 1];
    strcpy(chars, buf.str().c_str());
    writeDebugJNI(chars);
#else
    std::cerr << buf.str() << std::endl;
#endif
}