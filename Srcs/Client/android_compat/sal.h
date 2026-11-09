#pragma once

#ifndef __ANDROID__
  #error "This sal.h is only for Android!"
#endif

// Stub for Microsoft Source Annotation Language (SAL) headers used by DirectXMath
#ifndef _SAL_H_
#define _SAL_H_

#define _In_
#define _In_z_
#define _In_reads_(s)
#define _In_reads_bytes_(s)
#define _In_reads_opt_(s)
#define _Out_
#define _Out_opt_
#define _Out_writes_(s)
#define _Out_writes_opt_(s)
#define _Out_writes_bytes_(s)
#define _Inout_
#define _Inout_opt_
#define _Optional_
#define _Reserved_
#define _Check_return_
#define _Success_(expr)
#define _Deref_out_
#define _Deref_in_out_

#endif // _SAL_H_