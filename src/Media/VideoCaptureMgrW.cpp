#include "Stdafx.h"
#include "MyMemory.h"
#include "Media/VideoCaptureMgr.h"
#include "Media/DShow/DShowVideoCapture.h"

typedef struct
{
	Media::DShow::DShowVideoCaptureMgr *dshowMgr;
} ManagerData;

Media::VideoCaptureMgr::VideoCaptureMgr()
{
	ManagerData *data = MemAlloc(ManagerData, 1);
	NEW_CLASS(data->dshowMgr, Media::DShow::DShowVideoCaptureMgr());
	this->mgrData = data;
}

Media::VideoCaptureMgr::~VideoCaptureMgr()
{
	ManagerData *data = (ManagerData*)this->mgrData;;
	DEL_CLASS(data->dshowMgr);
	MemFree(data);
}

OSInt Media::VideoCaptureMgr::GetDeviceList(Data::ArrayList<DeviceInfo *> *devList)
{
	ManagerData *data = (ManagerData*)this->mgrData;
	DeviceInfo *devInfo;
	UTF8Char sbuff[512];
	OSInt ret = 0;
	OSInt i = 0;
	OSInt j = data->dshowMgr->GetDeviceCount();
	while (i < j)
	{
		devInfo = MemAlloc(DeviceInfo, 1);
		devInfo->devType = 0;
		devInfo->devId = i;
		data->dshowMgr->GetDeviceName(sbuff, i);
		devInfo->devName = Text::StrCopyNew(sbuff);
		devList->Add(devInfo);
		ret++;
		i++;
	}
	return ret;
}

void Media::VideoCaptureMgr::FreeDeviceList(Data::ArrayList<DeviceInfo *> *devList)
{
	DeviceInfo *devInfo;
	OSInt i = devList->GetCount();
	while (i-- > 0)
	{
		devInfo = devList->GetItem(i);
		SDEL_TEXT(devInfo->devName);
		MemFree(devInfo);
	}
}

Media::IVideoCapture *Media::VideoCaptureMgr::CreateDevice(Int32 devType, OSInt devId)
{
	ManagerData *data = (ManagerData*)this->mgrData;
	if (devType == 0)
	{
		return data->dshowMgr->GetDevice(devId);
	}
	else
	{
		return 0;
	}
}

const UTF8Char *Media::VideoCaptureMgr::GetDevTypeName(Int32 devType)
{
	switch (devType)
	{
	case 0:
		return (const UTF8Char*)"DShow";
	default:
		return (const UTF8Char*)"Unknown";
	}
}