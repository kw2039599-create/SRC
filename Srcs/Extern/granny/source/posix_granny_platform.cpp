// ========================================================================
// POSIX / Android / iOS Platform Layer for Granny 2.9.12.0
// ========================================================================
#if !defined(_WIN32)

#include "granny_assert.h"
#include "granny_conversions.h"
#include "granny_crc.h"
#include "granny_file_operations.h"
#include "granny_file_reader.h"
#include "granny_file_writer.h"
#include "granny_limits.h"
#include "granny_log.h"
#include "granny_memory.h"
#include "granny_parameter_checking.h"
#include "granny_system_clock.h"
#include "granny_telemetry.h"
#include "granny_threads.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sched.h>

#include "granny_cpp_settings.h"
#include "granny_version.h"
CompileAssert(ProductMajorVersion == 2);
CompileAssert(ProductMinorVersion == 9);
CompileAssert(ProductBuildNumber  == 12);

#define SubsystemCode FileReadingLogMessage

USING_GRANNY_NAMESPACE;
BEGIN_GRANNY_NAMESPACE;

struct posix_file_reader
{
    file_reader Base;
    FILE*       FileHandle;
};

struct posix_file_writer
{
    file_writer Base;
    FILE*       FileHandle;
};

END_GRANNY_NAMESPACE;

#if DEBUG
bool GRANNY
DisplayAssertion(char const * const /*Expression*/,
                 char const * const /*File*/,
                 int32x const /*LineNumber*/,
                 char const * const /*Function*/,
                 bool* IgnoredPtr,
                 void* /*UserData*/)
{
    if (IgnoredPtr)
        *IgnoredPtr = true;
    return false;
}
#endif

void GRANNY
ThreadYieldToAny()
{
    sched_yield();
}

void *GRANNY
PlatformAllocate(uintaddrx Size)
{
    return malloc((size_t)Size);
}

void GRANNY
PlatformDeallocate(void *Memory)
{
    if (Memory)
        free(Memory);
}

char const *GRANNY
GetTemporaryDirectory(void)
{
    return "/tmp/";
}

void GRANNY
DeleteFile(char const *FileName)
{
    if (FileName)
        remove(FileName);
}

int32x GRANNY
GetFileSize(char const *FileName)
{
    if (!FileName)
        return 0;

    FILE* fp = fopen(FileName, "rb");
    if (!fp)
        return 0;

    fseek(fp, 0, SEEK_END);
    long sz = ftell(fp);
    fclose(fp);
    return (sz > 0) ? (int32x)sz : 0;
}

static CALLBACK_FN(void)
PosixCloseFileReader(file_reader* ReaderInit)
{
    posix_file_reader* Reader = (posix_file_reader*)ReaderInit;
    if (Reader)
    {
        if (Reader->FileHandle)
            fclose(Reader->FileHandle);
        Deallocate(Reader);
    }
}

static CALLBACK_FN(int32x)
PosixReadAtMost(file_reader* ReaderInit,
                int32x FilePosition,
                int32x UInt8Count,
                void *Buffer)
{
    CheckPointerNotNull(ReaderInit, return 0);
    posix_file_reader& Reader = *((posix_file_reader*)ReaderInit);
    if (!Reader.FileHandle)
        return 0;

    if (fseek(Reader.FileHandle, (long)FilePosition, SEEK_SET) != 0)
        return 0;

    size_t bytesRead = fread(Buffer, 1, (size_t)UInt8Count, Reader.FileHandle);
    return (int32x)bytesRead;
}

static CALLBACK_FN(bool)
PosixGetReaderSize(file_reader* ReaderInit, int32x* SizeVar)
{
    CheckPointerNotNull(ReaderInit, return false);
    CheckPointerNotNull(SizeVar, return false);
    posix_file_reader& Reader = *((posix_file_reader*)ReaderInit);
    if (!Reader.FileHandle)
        return false;

    long cur = ftell(Reader.FileHandle);
    fseek(Reader.FileHandle, 0, SEEK_END);
    long end = ftell(Reader.FileHandle);
    fseek(Reader.FileHandle, cur, SEEK_SET);
    if (end < 0)
        return false;

    *SizeVar = (int32x)end;
    return true;
}

CALLBACK_FN(file_reader*) GRANNY
CreatePlatformFileReaderInternal(char const *FileNameToOpen)
{
    if (!FileNameToOpen)
        return 0;

    FILE* fp = fopen(FileNameToOpen, "rb");
    if (!fp)
        return 0;

    posix_file_reader* Reader = Allocate(posix_file_reader, AllocationUnknown);
    if (Reader)
    {
        InitializeFileReader(PosixCloseFileReader,
                             PosixReadAtMost,
                             PosixGetReaderSize,
                             Reader->Base);
        Reader->FileHandle = fp;
        return (file_reader*)Reader;
    }

    fclose(fp);
    return 0;
}

open_file_reader_callback *GRANNY OpenFileReaderCallback = CreatePlatformFileReaderInternal;

static CALLBACK_FN(void)
PosixDeleteFileWriter(file_writer* WriterInit)
{
    posix_file_writer* Writer = (posix_file_writer*)WriterInit;
    if (Writer)
    {
        if (Writer->FileHandle)
            fclose(Writer->FileHandle);
        Deallocate(Writer);
    }
}

static CALLBACK_FN(int32x)
PosixSeekWriter(file_writer* WriterInit,
                int32x OffsetInUInt8s,
                int32x SeekType)
{
    CheckPointerNotNull(WriterInit, return 0);
    posix_file_writer& Writer = *((posix_file_writer*)WriterInit);
    if (!Writer.FileHandle)
        return 0;

    int whence = SEEK_SET;
    if (SeekType == SeekEnd) whence = SEEK_END;
    else if (SeekType == SeekCurrent) whence = SEEK_CUR;

    fseek(Writer.FileHandle, (long)OffsetInUInt8s, whence);
    return (int32x)ftell(Writer.FileHandle);
}

static CALLBACK_FN(bool)
PosixWrite(file_writer* WriterInit,
           int32x UInt8Count,
           void const *WritePointer)
{
    CheckPointerNotNull(WriterInit, return false);
    posix_file_writer& Writer = *((posix_file_writer*)WriterInit);
    if (!Writer.FileHandle)
        return false;

    if (Writer.Base.CRCing)
        AddToCRC32(Writer.Base.CRC, UInt8Count, WritePointer);

    size_t written = fwrite(WritePointer, 1, (size_t)UInt8Count, Writer.FileHandle);
    return ((int32x)written == UInt8Count);
}

static CALLBACK_FN(void)
PosixBeginWriterCRC(file_writer* WriterInit)
{
    CheckPointerNotNull(WriterInit, return);
    posix_file_writer& Writer = *((posix_file_writer*)WriterInit);
    Writer.Base.CRCing = true;
    BeginCRC32(Writer.Base.CRC);
}

static CALLBACK_FN(uint32)
PosixEndWriterCRC(file_writer* WriterInit)
{
    CheckPointerNotNull(WriterInit, return 0);
    posix_file_writer& Writer = *((posix_file_writer*)WriterInit);
    Writer.Base.CRCing = false;
    EndCRC32(Writer.Base.CRC);
    return Writer.Base.CRC;
}

CALLBACK_FN(file_writer*) GRANNY
CreatePlatformFileWriterInternal(char const *FileNameToOpen, bool EraseExisting)
{
    if (!FileNameToOpen)
        return 0;

    FILE* fp = fopen(FileNameToOpen, EraseExisting ? "wb" : "r+b");
    if (!fp && !EraseExisting)
        fp = fopen(FileNameToOpen, "wb");
    if (!fp)
        return 0;

    posix_file_writer* Writer = Allocate(posix_file_writer, AllocationUnknown);
    if (Writer)
    {
        InitializeFileWriter(PosixDeleteFileWriter,
                             PosixSeekWriter,
                             PosixWrite,
                             PosixBeginWriterCRC,
                             PosixEndWriterCRC,
                             Writer->Base);
        Writer->FileHandle = fp;
        return (file_writer*)Writer;
    }

    fclose(fp);
    return 0;
}

open_file_writer_callback *GRANNY OpenFileWriterCallback = CreatePlatformFileWriterInternal;

void GRANNY
RequeryTimerFrequency()
{
}

void GRANNY
GetSystemSeconds(system_clock* Result)
{
    CheckPointerNotNull(Result, return);
    struct timeval tv;
    gettimeofday(&tv, NULL);
    int64 CurrMicroseconds = ((int64)tv.tv_sec * 1000000LL) + (int64)tv.tv_usec;
    *((int64*)Result) = CurrMicroseconds;
}

real32 GRANNY
GetSecondsElapsed(system_clock const &StartClock,
                  system_clock const &EndClock)
{
    int64 const* Start = (int64 const*)&StartClock;
    int64 const* End   = (int64 const*)&EndClock;
    if (*Start < *End)
    {
        real64x const Difference = (real64x)(*End - *Start);
        return (real32)(Difference / 1000000.0);
    }
    return 0.0f;
}

#endif // !_WIN32