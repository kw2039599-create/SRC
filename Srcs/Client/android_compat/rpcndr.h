#pragma once

#ifndef __ANDROID__
  #error "This rpcndr.h is only for Android!"
#endif

#include <windows.h>
#include <rpc.h>

#ifndef __RPCNDR_H_VERSION__
#define __RPCNDR_H_VERSION__ 500
#endif

#ifndef __RPCNDR_H__
#define __RPCNDR_H__

#define RPC_DS_MAJOR_VERSION 6
#define RPC_DS_MINOR_VERSION 0

typedef unsigned char boolean;
#ifndef __midl_proxy
#define __midl_proxy
#endif

#ifndef __RPC_USER
#define __RPC_USER
#endif

#ifndef __RPC_STUB
#define __RPC_STUB
#endif

#ifndef __RPC_FAR
#define __RPC_FAR
#endif

#ifndef RPC_IF_HANDLE
typedef void* RPC_IF_HANDLE;
#endif

#ifdef __cplusplus
extern "C" {
#endif

void * MIDL_user_allocate(size_t len);
void   MIDL_user_free(void * ptr);

#ifdef __cplusplus
}
#endif

#ifndef MIDL_INTERFACE
#define MIDL_INTERFACE(x) struct
#endif

#endif // __RPCNDR_H__