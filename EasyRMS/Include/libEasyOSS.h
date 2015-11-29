#ifndef LIB_EASY_OSS_H
#define LIB_EASY_OSS_H

#include <stdio.h>
#include <string.h>

#ifdef _WIN32
#define EASYOSS_API  __declspec(dllexport)
#define EASYOSS_APICALL  __stdcall
#else
#define EASYOSS_API
#define EASYOSS_APICALL 
#endif

enum EASYOSS_OPEN_MODE
{
	EASYOSS_MODE_READ,
	EASYOSS_MODE_WRITE,
	EASYOSS_MODE_READ_WRITE
};

typedef void* EasyOSS_Handle;

#define EASYOSS_MAX_URL_LEN 1024

typedef struct _EasyOSS_URL
{
	char url[EASYOSS_MAX_URL_LEN];
	_EasyOSS_URL()
	{
		memset(url, 0, EASYOSS_MAX_URL_LEN);
	}
}EasyOSS_URL;

EASYOSS_API int EASYOSS_APICALL EasyOSS_Initialize(const char* bucket_name, const char* oss_endpoint, size_t oss_port, const char* access_key_id, const char* access_key_secret);

EASYOSS_API EasyOSS_Handle EASYOSS_APICALL EasyOSS_Open(const char* object_name, EASYOSS_OPEN_MODE mode);

EASYOSS_API int EASYOSS_APICALL EasyOSS_Write(EasyOSS_Handle handle, const void* data, size_t size);

EASYOSS_API int EASYOSS_APICALL EasyOSS_Read(EasyOSS_Handle handle, void* data, size_t size);

EASYOSS_API unsigned long EASYOSS_APICALL EasyOSS_Size(EasyOSS_Handle handle);

EASYOSS_API int EASYOSS_APICALL EasyOSS_Delete(const char* object_name);

EASYOSS_API int EASYOSS_APICALL EasyOSS_Close(EasyOSS_Handle handle);

EASYOSS_API int EASYOSS_APICALL EasyOSS_List(const char* name, const char* begin/*yyyymmddhhmmss*/, const char* end/*yyyymmddhhmmss*/, EasyOSS_URL *result, size_t result_size);

EASYOSS_API void EASYOSS_APICALL EasyOSS_Deinitialize();

#endif

