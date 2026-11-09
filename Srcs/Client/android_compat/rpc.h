#pragma once

#ifndef __ANDROID__
  #error "This rpc.h is only for Android!"
#endif

#include <windows.h>

#ifndef __RPC_H__
#define __RPC_H__

typedef struct _RPC_MESSAGE *PRPC_MESSAGE;
typedef struct IRpcStubBuffer IRpcStubBuffer;
typedef struct IRpcChannelBuffer IRpcChannelBuffer;

#endif // __RPC_H__