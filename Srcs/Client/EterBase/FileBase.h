#ifndef __INC_ETERBASE_FILEBASE_H__
#define __INC_ETERBASE_FILEBASE_H__

#include <windows.h>

#ifndef MAX_PATH
#define MAX_PATH 260
#endif
#ifndef _MAX_PATH
#define _MAX_PATH 260
#endif
#ifndef _MAX_DRIVE
#define _MAX_DRIVE 3
#endif
#ifndef _MAX_DIR
#define _MAX_DIR 256
#endif
#ifndef _MAX_FNAME
#define _MAX_FNAME 256
#endif
#ifndef _MAX_EXT
#define _MAX_EXT 256
#endif

class CFileBase
{
	public:
		enum EFileMode
		{
			FILEMODE_READ = (1 << 0),
			FILEMODE_WRITE = (1 << 1)
		};

		CFileBase();
		virtual	~CFileBase();

		void			Destroy();
		void			Close();

		BOOL			Create(const char* filename, EFileMode mode);
		DWORD			Size();
		void			SeekCur(DWORD size);
		void			Seek(DWORD offset);
		DWORD			GetPosition();

		virtual BOOL	Write(const void* src, int bytes);
		BOOL			Read(void* dest, int bytes);

		char*			GetFileName();
		BOOL			IsNull();

	protected:
		int				m_mode;
		char			m_filename[MAX_PATH+1];
		HANDLE			m_hFile;
		DWORD			m_dwSize;
};

#endif
//martysama0134's fb0d8d5d0566583187a94f9a823b2075
