/*
	Copyright (c) 2013-2015 EasyDarwin.ORG.  All rights reserved.
	Github: https://github.com/EasyDarwin
	WEChat: EasyDarwin
	Website: http://www.easydarwin.org
*/
/*
    File:       EasyRecordSession.h
    Contains:   HLS
*/
#include "QTSS.h"
#include "OSRef.h"
#include "StrPtrLen.h"
#include "ResizeableStringFormatter.h"
#include "MyAssert.h"

#include "SourceInfo.h"
#include "OSArrayObjectDeleter.h"
#include "EasyRTSPClientAPI.h"
#include "EasyRecordAPI.h"

#include "TimeoutTask.h"

#ifndef __EASY_RECORD_SESSION__
#define __EASY_RECORD_SESSION__

#define QTSS_MAX_URL_LENGTH	512

class EasyRecordSession : public Task
{
    public:
        EasyRecordSession(StrPtrLen* inSourceID);
        virtual ~EasyRecordSession();
        
        //����ģ������
        static void Initialize(QTSS_ModulePrefsObject inPrefs);

		virtual SInt64	Run();
        // ACCESSORS

        OSRef*          GetRef()            { return &fRef; }
        OSQueueElem*    GetQueueElem()      { return &fQueueElem; }
	
        StrPtrLen*      GetSessionID()     { return &fHLSSessionID; }
		QTSS_Error		ProcessData(int _chid, int mediatype, char *pbuf, RTSP_FRAME_INFO *frameinfo);
		QTSS_Error		HLSSessionStart(char* rtspUrl, UInt32 inTimeout);
		QTSS_Error		HLSSessionRelease();
		char*			GetHLSURL();
		char*			GetSourceURL();

		void RefreshTimeout()	{ fTimeoutTask.RefreshTimeout(); }

		//ͳ��
		SInt64          GetTotalPlayTime()		const { return fTotalPlayTime; }
		SInt64			GetNumPacketsReceived() const { return fNumPacketsReceived; }
		SInt64			GetNumBytesReceived()	const { return fNumBytesReceived; }
		UInt32			GetLastStatBitrate()	const { return fLastStatBitrate; }

		const char*		TimeToString(UInt64 inTime);//return string with formatYYYYMMDDhhmmss
		bool			TryCreateNewRecord();
   
    private:

        //HLSSession�б���EasyRecordModule��sHLSSessionMapά��  
        OSRef       fRef;
        StrPtrLen   fHLSSessionID;
		char		fHLSURL[QTSS_MAX_URL_LENGTH];
		char		fSourceURL[QTSS_MAX_URL_LENGTH];
        OSQueueElem fQueueElem; 

		//RTSPClient Handle
		Easy_RTSP_Handle	fRTSPClientHandle;
		//HLS Handle
		Easy_Record_Handle fRecordHandle;
		
		//TS timestamp ms���Զ���ʱ���
		int tsTimeStampMSsec;

		static UInt32	sM3U8Version;
		static Bool16	sAllowCache;
		static UInt32	sTargetDuration;
		static UInt32	sPlaylistCapacity;
		static char*	sHTTPRootDir;
		static char*	sOSSBucketName;
		static char*	sOSSEndpoint;
		static UInt32	sOSSPort;
		static char*	sOSSAccessKeyID;
		static char*	sOSSAccessKeySecret;
		static UInt32	sRecordDuration;
		static char*	sLocalRecordPath;
		static UInt32	sRecordToWhere;	//0-OSS 1-local

		//ͳ��
		SInt64          fPlayTime;				//��ʼ��ʱ��
		SInt64			fLastStatPlayTime;		//��һ��ͳ�Ƶ�ʱ��

        SInt64          fTotalPlayTime;			//�ܹ�����ʱ��

        SInt64			fNumPacketsReceived;	//�յ������ݰ�������
		SInt64			fLastNumPacketsReceived;//��һ��ͳ���յ������ݰ�����

        SInt64			fNumBytesReceived;		//�յ�����������
        SInt64			fLastNumBytesReceived;	//��һ��ͳ���յ�����������

		UInt32			fLastStatBitrate;		//���һ��ͳ�Ƶõ��ı�����

		UInt64			fLastRecordTime;		//�ϴ�������¼���ʱ��

	protected:
		TimeoutTask		fTimeoutTask;
};

#endif

