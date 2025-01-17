#ifndef _SM_SSWR_AVIREAD_AVIRTCPSPDCLIFORM
#define _SM_SSWR_AVIREAD_AVIRTCPSPDCLIFORM
#include "Manage/HiResClock.h"
#include "Net/TCPClient.h"
#include "SSWR/AVIRead/AVIRCore.h"
#include "UI/GUIButton.h"
#include "UI/GUIForm.h"
#include "UI/GUILabel.h"
#include "UI/GUITextBox.h"

namespace SSWR
{
	namespace AVIRead
	{
		class AVIRTCPSpdCliForm : public UI::GUIForm
		{
		private:
			SSWR::AVIRead::AVIRCore *core;
			UInt64 recvSize;
			UInt64 sendSize;
			Double lastTime;
			UInt64 lastRecvSize;
			UInt64 lastSendSize;
			Manage::HiResClock *clk;
			Net::TCPClient *cli;
			Sync::Mutex *cliMut;
			Bool connected;

			Bool procRunning;
			Bool recvRunning;
			Bool toStop;
			Sync::Event *mainEvt;
			Sync::Event *recvEvt;
			Sync::Event *procEvt;

			UI::GUILabel *lblHost;
			UI::GUITextBox *txtHost;
			UI::GUILabel *lblPort;
			UI::GUITextBox *txtPort;
			UI::GUIButton *btnConn;

			UI::GUILabel *lblSendSpeed;
			UI::GUITextBox *txtSendSpeed;
			UI::GUILabel *lblRecvSpeed;
			UI::GUITextBox *txtRecvSpeed;

			static void __stdcall OnConnClick(void *userObj);
			static void __stdcall OnTimerTick(void *userObj);
			static UInt32 __stdcall ProcThread(void *userObj);
			static UInt32 __stdcall RecvThread(void *userObj);
		public:
			AVIRTCPSpdCliForm(UI::GUIClientControl *parent, UI::GUICore *ui, SSWR::AVIRead::AVIRCore *core);
			virtual ~AVIRTCPSpdCliForm();

			virtual void OnMonitorChanged();
		};
	};
};
#endif
