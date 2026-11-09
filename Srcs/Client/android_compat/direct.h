#pragma once

#ifndef __ANDROID__
  #error "This direct.h compatibility header is only for Android!"
#endif

#include <sys/stat.h>
#include <unistd.h>

#ifndef _mkdir
#define _mkdir(path) mkdir(path, 0777)
#endif

#ifndef _rmdir
#define _rmdir rmdir
#endif

#ifndef _getcwd
#define _getcwd getcwd
#endif