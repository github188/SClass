#ifndef _SM_UI_GUICORE
#define _SM_UI_GUICORE
#include "Media/DrawEngine.h"
#include "Media/MonitorMgr.h"

namespace UI
{
	typedef void (__stdcall *UIEvent)(void *userObj);

	class GUICore
	{
	public:
		typedef enum
		{
			DR_NONE,
			DR_LANDSCAPE,
			DR_PORT
		} DisplayRotation;
	public:
		GUICore() {};
		virtual ~GUICore(){};

		virtual void Run() = 0;
		virtual void ProcessMessages() = 0;
		virtual void WaitForMessages() = 0;
		virtual void Exit() = 0;
		virtual Media::DrawEngine *CreateDrawEngine() = 0;
		virtual Double GetMagnifyRatio(void *hMonitor) = 0;
		virtual void UseDevice(Bool useSystem, Bool useDisplay) = 0;
		virtual void SetNoDisplayOff(Bool noDispOff) = 0;
		virtual void DisplayOff() = 0;
		virtual void Suspend() = 0;
		virtual void GetDesktopSize(UOSInt *w, UOSInt *h) = 0;
		virtual void GetCursorPos(OSInt *x, OSInt *y) = 0;
		virtual void SetDisplayRotate(void *hMonitor, DisplayRotation rot) = 0;
		virtual void GetMonitorDPIs(void *hMonitor, Double *hdpi, Double *ddpi) = 0;
		virtual void SetMonitorMgr(Media::MonitorMgr *monMgr) = 0;
	};
}
#endif
