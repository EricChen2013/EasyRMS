/*
	Copyright (c) 2013-2015 EasyDarwin.ORG.  All rights reserved.
	Github: https://github.com/EasyDarwin
	WEChat: EasyDarwin
	Website: http://www.easydarwin.org
*/
/*
    File:       EasyRecordSession.cpp
    Contains:   Implementation of object defined in EasyRecordSession.h. 
*/
#include "EasyRecordSession.h"
#include "SocketUtils.h"
#include "EventContext.h"
#include "OSMemory.h"
#include "OS.h"
#include "atomic.h"
#include "QTSSModuleUtils.h"
#include <errno.h>
#include "QTSServerInterface.h"

#ifndef __Win32__
    #include <unistd.h>
#endif

// PREFS
static UInt32                   sDefaultM3U8Version					= 3; 
static Bool16                   sDefaultAllowCache					= false; 
static UInt32                   sDefaultTargetDuration				= 4;
static UInt32                   sDefaultPlaylistCapacity			= 4;
static char*					sDefaultHTTPRootDir					= "http://www.easydarwin.org/";
static char*					sDefaultOSSBucketName				= "easydarwin-easyrms-bucket";
static char*					sDefaultOSSEndpoint					= "oss-cn-hangzhou.aliyuncs.com";
static UInt32					sDefaultOSSPort						= 80;
static char*					sDefaultOSSAccessKeyID				= "ayO28eQpxOntWuzV";
static char*					sDefaultOSSAccessKeySecret			= "MJQD5mE27JCTIwBdrbofmSPjgDoAkG";
static UInt32					sDefaultRecordDuration				= 10;
static char*					sDefaultLocalRecordPath				= "Record";
static UInt32					sDefaultRecordToWhere				= RECORD_TYPE_FILE;

UInt32                          EasyRecordSession::sM3U8Version			= 3;
Bool16                          EasyRecordSession::sAllowCache			= false;
UInt32                          EasyRecordSession::sTargetDuration		= 4;
UInt32                          EasyRecordSession::sPlaylistCapacity	= 4;
char*							EasyRecordSession::sHTTPRootDir			= NULL;
char*							EasyRecordSession::sOSSBucketName		= NULL;
char*							EasyRecordSession::sOSSEndpoint			= NULL;
UInt32							EasyRecordSession::sOSSPort				= 80;
char*							EasyRecordSession::sOSSAccessKeyID		= NULL;
char*							EasyRecordSession::sOSSAccessKeySecret	= NULL;
UInt32							EasyRecordSession::sRecordDuration		= 10;
char*							EasyRecordSession::sLocalRecordPath		= NULL;
UInt32							EasyRecordSession::sRecordToWhere		= RECORD_TYPE_FILE;

void EasyRecordSession::Initialize(QTSS_ModulePrefsObject inPrefs)
{
	delete [] sHTTPRootDir;
    sHTTPRootDir = QTSSModuleUtils::GetStringAttribute(inPrefs, "HTTP_ROOT_DIR", sDefaultHTTPRootDir);

	QTSSModuleUtils::GetAttribute(inPrefs, "M3U8_VERSION", qtssAttrDataTypeUInt32,
							  &EasyRecordSession::sM3U8Version, &sDefaultM3U8Version, sizeof(sDefaultM3U8Version));

	QTSSModuleUtils::GetAttribute(inPrefs, "ALLOW_CACHE", qtssAttrDataTypeBool16,
							  &EasyRecordSession::sAllowCache, &sDefaultAllowCache, sizeof(sDefaultAllowCache));

	QTSSModuleUtils::GetAttribute(inPrefs, "TARGET_DURATION", qtssAttrDataTypeUInt32,
							  &EasyRecordSession::sTargetDuration, &sDefaultTargetDuration, sizeof(sDefaultTargetDuration));

	QTSSModuleUtils::GetAttribute(inPrefs, "PLAYLIST_CAPACITY", qtssAttrDataTypeUInt32,
							  &EasyRecordSession::sPlaylistCapacity, &sDefaultPlaylistCapacity, sizeof(sDefaultPlaylistCapacity));

	QTSSModuleUtils::GetAttribute(inPrefs, "record_duration", qtssAttrDataTypeUInt32,
							  &EasyRecordSession::sRecordDuration, &sDefaultRecordDuration, sizeof(sDefaultRecordDuration));
	
	delete [] sLocalRecordPath;
	sLocalRecordPath = QTSSModuleUtils::GetStringAttribute(inPrefs, "local_record_path", sDefaultLocalRecordPath);

	QTSSModuleUtils::GetAttribute(inPrefs, "record_to_where", qtssAttrDataTypeUInt32,
							  &EasyRecordSession::sRecordToWhere, &sDefaultRecordToWhere, sizeof(sDefaultRecordToWhere));
	
	delete [] sOSSBucketName;
	sOSSBucketName = QTSSModuleUtils::GetStringAttribute(inPrefs, "oss_bucket_name", sDefaultOSSBucketName);

	delete [] sOSSEndpoint;
	sOSSEndpoint = QTSSModuleUtils::GetStringAttribute(inPrefs, "oss_endpoint", sDefaultOSSEndpoint);

	QTSSModuleUtils::GetAttribute(inPrefs, "oss_port", qtssAttrDataTypeUInt32,
		&EasyRecordSession::sOSSPort, &sDefaultOSSPort, sizeof(sDefaultOSSPort));

	delete [] sOSSAccessKeyID;
	sOSSAccessKeyID = QTSSModuleUtils::GetStringAttribute(inPrefs, "oss_access_key_id", sDefaultOSSAccessKeyID);

	delete [] sOSSAccessKeySecret;
	sOSSAccessKeySecret = QTSSModuleUtils::GetStringAttribute(inPrefs, "oss_access_key_secret", sDefaultOSSAccessKeySecret);

	if(sRecordToWhere == RECORD_TYPE_OSS)
	{
		EasyRecord_OSS_Initialize(sOSSBucketName, sOSSEndpoint, sOSSPort, sOSSAccessKeyID, sOSSAccessKeySecret);
	}
	EasyRecord_SetRecordType((ENUM_RECORD_TYPE)sRecordToWhere);
}

/* RTSPClient��ȡ���ݺ�ص����ϲ� */
int Easy_APICALL __RTSPClientCallBack( int _chid, int *_chPtr, int _mediatype, char *pbuf, RTSP_FRAME_INFO *frameinfo)
{
	EasyRecordSession* pHLSSession = (EasyRecordSession *)_chPtr;

	if (NULL == pHLSSession)	return -1;

	//Ͷ�ݵ������Ӧ��EasyRecordSession���д���
	pHLSSession->ProcessData(_chid, _mediatype, pbuf, frameinfo);

	return 0;
}

EasyRecordSession::EasyRecordSession(StrPtrLen* inSessionID)
:   fQueueElem(),
	fRTSPClientHandle(NULL),
	fRecordHandle(NULL),
	tsTimeStampMSsec(0),
	fPlayTime(0),
    fTotalPlayTime(0),
	fLastStatPlayTime(0),
	fLastStatBitrate(0),
	fNumPacketsReceived(0),
	fLastNumPacketsReceived(0),
	fNumBytesReceived(0),
	fLastNumBytesReceived(0),
	fTimeoutTask(NULL, 60*1000)
{
    fTimeoutTask.SetTask(this);
    fQueueElem.SetEnclosingObject(this);

    if (inSessionID != NULL)
    {
        fHLSSessionID.Ptr = NEW char[inSessionID->Len + 1];
        ::memcpy(fHLSSessionID.Ptr, inSessionID->Ptr, inSessionID->Len);
		fHLSSessionID.Ptr[inSessionID->Len] = '\0';
        fHLSSessionID.Len = inSessionID->Len;
        fRef.Set(fHLSSessionID, this);
    }

	fHLSURL[0] = '\0';
	fSourceURL[0] = '\0';

	this->Signal(Task::kStartEvent);
}


EasyRecordSession::~EasyRecordSession()
{
	HLSSessionRelease();
    fHLSSessionID.Delete();

    if (this->GetRef()->GetRefCount() == 0)
    {   
        qtss_printf("EasyRecordSession::~EasyRecordSession() UnRegister and delete session =%p refcount=%"_U32BITARG_"\n", GetRef(), GetRef()->GetRefCount() ) ;       
        QTSServerInterface::GetServer()->GetRecordSessionMap()->UnRegister(GetRef());
    }
}

SInt64 EasyRecordSession::Run()
{
    EventFlags theEvents = this->GetEvents();
	OSRefTable* sHLSSessionMap =  QTSServerInterface::GetServer()->GetRecordSessionMap();
	OSMutexLocker locker (sHLSSessionMap->GetMutex());

	if (theEvents & Task::kKillEvent)
    {
        return -1;
    }

	if (theEvents & Task::kTimeoutEvent)
    {
		char msgStr[2048] = { 0 };
		qtss_snprintf(msgStr, sizeof(msgStr), "EasyRecordSession::Run Timeout SessionID=%s", fHLSSessionID.Ptr);
		QTSServerInterface::LogError(qtssMessageVerbosity, msgStr);

		return -1;
    }

	//ͳ������
	{
		SInt64 curTime = OS::Milliseconds();

		UInt64 bytesReceived = fNumBytesReceived - fLastNumBytesReceived;
		UInt64 durationTime	= curTime - fLastStatPlayTime;

		if(durationTime)
			fLastStatBitrate = (bytesReceived*1000)/(durationTime);

		fLastNumBytesReceived = fNumBytesReceived;
		fLastStatPlayTime = curTime;

	}

    return 2000;
}

QTSS_Error EasyRecordSession::ProcessData(int _chid, int mediatype, char *pbuf, RTSP_FRAME_INFO *frameinfo)
{
	if(NULL == fRecordHandle) return QTSS_Unimplemented;
	
	TryCreateNewRecord();
	
	if ((mediatype == EASY_SDK_VIDEO_FRAME_FLAG) || (mediatype == EASY_SDK_AUDIO_FRAME_FLAG))
	{
		fNumPacketsReceived++;
		fNumBytesReceived += frameinfo->length;
	}

	if (mediatype == EASY_SDK_VIDEO_FRAME_FLAG)
	{
		unsigned long long llPTS = (frameinfo->timestamp_sec%1000000)*1000 + frameinfo->timestamp_usec/1000;	

		//printf("Get %s Video \tLen:%d \ttm:%u.%u \t%u\n",frameinfo->type==EASY_SDK_VIDEO_FRAME_I?"I":"P", frameinfo->length, frameinfo->timestamp_sec, frameinfo->timestamp_usec, llPTS);

		unsigned int uiFrameType = 0;
		if (frameinfo->type == EASY_SDK_VIDEO_FRAME_I)
		{
			uiFrameType = TS_TYPE_PES_VIDEO_I_FRAME;
		}
		else if (frameinfo->type == EASY_SDK_VIDEO_FRAME_P)
		{
			uiFrameType = TS_TYPE_PES_VIDEO_P_FRAME;
		}
		else
		{
			return QTSS_OutOfState;
		}

		EasyRecord_VideoMux(fRecordHandle, uiFrameType, (unsigned char*)pbuf, frameinfo->length, llPTS*90, llPTS*90, llPTS*90);
	}
	else if (mediatype == EASY_SDK_AUDIO_FRAME_FLAG)
	{

		unsigned long long llPTS = (frameinfo->timestamp_sec%1000000)*1000 + frameinfo->timestamp_usec/1000;	

		//printf("Get Audio \tLen:%d \ttm:%u.%u \t%u\n", frameinfo->length, frameinfo->timestamp_sec, frameinfo->timestamp_usec, llPTS);

		if (frameinfo->codec == EASY_SDK_AUDIO_CODEC_AAC)
		{
			EasyRecord_AudioMux(fRecordHandle, (unsigned char*)pbuf, frameinfo->length, llPTS*90, llPTS*90);
		}
	}
	else if (mediatype == EASY_SDK_EVENT_FRAME_FLAG)
	{
		if (NULL == pbuf && NULL == frameinfo)
		{
			printf("Connecting:%s ...\n", fHLSSessionID.Ptr);
		}
		else if (NULL!=frameinfo && frameinfo->type==0xF1)
		{
			printf("Lose Packet:%s ...\n", fHLSSessionID.Ptr);
		}
	}

	return QTSS_NoErr;
}

/*
	����HLSֱ��Session
*/
QTSS_Error	EasyRecordSession::HLSSessionStart(char* rtspUrl, UInt32 inTimeout)
{
	QTSS_Error theErr = QTSS_NoErr;

	do{
		if(NULL == fRTSPClientHandle)
		{
			//����RTSPClient
			EasyRTSP_Init(&fRTSPClientHandle);

			if (NULL == fRTSPClientHandle)
			{
				theErr = QTSS_RequestFailed;
				break;
			}

			::sprintf(fSourceURL, "%s", rtspUrl);

			unsigned int mediaType = EASY_SDK_VIDEO_FRAME_FLAG | EASY_SDK_AUDIO_FRAME_FLAG;

			EasyRTSP_SetCallback(fRTSPClientHandle, __RTSPClientCallBack);
			EasyRTSP_OpenStream(fRTSPClientHandle, 0, rtspUrl,RTP_OVER_TCP, mediaType, 0, 0, this, 1000, 0);

			fPlayTime = fLastStatPlayTime = OS::Milliseconds();
			fNumPacketsReceived = fLastNumPacketsReceived = 0;
			fNumBytesReceived = fLastNumBytesReceived = 0;

		}

		if(NULL == fRecordHandle)
		{
			//����HLSSessioin Sink
			fRecordHandle = EasyRecord_Session_Create(sAllowCache, sM3U8Version);

			if (NULL == fRecordHandle)
			{
				theErr = QTSS_Unimplemented;
				break;
			}
			fLastRecordTime = 0;
			TryCreateNewRecord();
			//char subDir[QTSS_MAX_URL_LENGTH] = { 0 };
			//fLastRecordTime = time(NULL);
			//qtss_sprintf(subDir,"%s/%s/",fHLSSessionID.Ptr, TimeToString(fLastRecordTime));//Movies/deviceid/YYYYMMDDhhmmss/
			//EasyRecord_ResetStreamCache(fRecordHandle, "Movies/", subDir, fHLSSessionID.Ptr, sTargetDuration);

			//char msgStr[2048] = { 0 };
			//qtss_snprintf(msgStr, sizeof(msgStr), "EasyRecordSession::EasyHLS_ResetStreamCache SessionID=%s,movieFolder=%s,subDir=%s", fHLSSessionID.Ptr, "Movies/", subDir);
			//QTSServerInterface::LogError(qtssMessageVerbosity, msgStr);
			//		
			qtss_sprintf(fHLSURL, "%s%s/%s.m3u8", sHTTPRootDir, fHLSSessionID.Ptr, fHLSSessionID.Ptr);
		}
		
		fTimeoutTask.SetTimeout(inTimeout * 1000);
	}while(0);

	char msgStr[2048] = { 0 };
	qtss_snprintf(msgStr, sizeof(msgStr), "EasyRecordSession::HLSSessionStart SessionID=%s,url=%s,return=%d", fHLSSessionID.Ptr, rtspUrl, theErr);
	QTSServerInterface::LogError(qtssMessageVerbosity, msgStr);

	return theErr;
}

QTSS_Error	EasyRecordSession::HLSSessionRelease()
{
	qtss_printf("HLSSession Release....\n");
	
	//�ͷ�source
	if(fRTSPClientHandle)
	{
		EasyRTSP_CloseStream(fRTSPClientHandle);
		EasyRTSP_Deinit(&fRTSPClientHandle);
		fRTSPClientHandle = NULL;
		fSourceURL[0] = '\0';
	}

	//�ͷ�sink
	if(fRecordHandle)
	{
		EasyRecord_Session_Release(fRecordHandle);
		fRecordHandle = NULL;
		fHLSURL[0] = '\0';
 	}

	return QTSS_NoErr;
}

char* EasyRecordSession::GetHLSURL()
{
	return fHLSURL;
}

char* EasyRecordSession::GetSourceURL()
{
	return 	fSourceURL;
}

const char*	EasyRecordSession::TimeToString(UInt64 inTime)
{
	static char s[20];
	struct tm local;
	time_t t = inTime;

#ifdef _WIN32
	localtime_s(&local, &t);
#else
	localtime_r(&t, &local);
#endif

	memset(s, 0, 20);
	qtss_sprintf(s, "%04d%02d%02d%02d%02d%02d", local.tm_year + 1900, local.tm_mon + 1, local.tm_mday, \
			local.tm_hour, local.tm_min, local.tm_sec);

	return s;
}

bool EasyRecordSession::TryCreateNewRecord()
{
	if(fLastRecordTime != 0)
	{
		UInt64 now = time(NULL);		
		if(now - fLastRecordTime < sRecordDuration * 60)
		{
			return false;
		}
	}
	
	char rootDir[QTSS_MAX_URL_LENGTH] = { 0 };
	char subDir[QTSS_MAX_URL_LENGTH] = { 0 };
	
	if(sRecordToWhere == RECORD_TYPE_FILE)
	{
		qtss_sprintf(rootDir,"%s/%s/", sLocalRecordPath, fHLSSessionID.Ptr);
	}
	else
	{
		qtss_sprintf(rootDir,"%s/", fHLSSessionID.Ptr);
	}
	
	fLastRecordTime = time(NULL);
	qtss_sprintf(subDir,"%s/", TimeToString(fLastRecordTime));
	
	//EasyRecord_SetRecordType((ENUM_RECORD_TYPE)sRecordToWhere);
	EasyRecord_ResetStreamCache(fRecordHandle, rootDir, subDir, fHLSSessionID.Ptr, sTargetDuration);//deviceid/YYYYMMDDhhmmss/

	char msgStr[2048] = { 0 };
	qtss_snprintf(msgStr, sizeof(msgStr), "EasyRecordSession::EasyHLS_ResetStreamCache SessionID=%s,movieFolder=%s,subDir=%s", fHLSSessionID.Ptr, rootDir, subDir);
	QTSServerInterface::LogError(qtssMessageVerbosity, msgStr);
	
	return true;
}