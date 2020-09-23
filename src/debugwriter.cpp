#include "debugwriter.h"
#include "config.h"

#ifdef __ANDROID__
#include <android/log.h>
extern "C" {
	void writeDebugJNI(char* message);
}
#endif

Debug::~Debug()
{
#ifdef __ANDROID__
    std::string message = buf.str();
#ifdef INI_ENCODING
    convertIfNotValidUTF8("", message);
#endif
    __android_log_write(ANDROID_LOG_DEBUG, "mkxp", message.c_str());
    int l = message.length();
    char chars[l + 1];
    strcpy(chars, message.c_str());
    writeDebugJNI(chars);
#else
    std::cerr << buf.str() << std::endl;
#endif
}