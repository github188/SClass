#ifndef _SM_SSWR_AVIREAD_AVIRHTTPCLIENTFORM
#define _SM_SSWR_AVIREAD_AVIRHTTPCLIENTFORM
#include "Data/ArrayList.h"
#include "IO/MemoryStream.h"
#include "SSWR/AVIRead/AVIRCore.h"
#include "UI/GUIButton.h"
#include "UI/GUICheckBox.h"
#include "UI/GUIComboBox.h"
#include "UI/GUIForm.h"
#include "UI/GUIGroupBox.h"
#include "UI/GUIHSplitter.h"
#include "UI/GUILabel.h"
#include "UI/GUIListView.h"
#include "UI/GUIPanel.h"
#include "UI/GUITabControl.h"
#include "UI/GUITabPage.h"
#include "UI/GUITextBox.h"

namespace SSWR
{
	namespace AVIRead
	{
		class AVIRHTTPClientForm : public UI::GUIForm
		{
		private:
			typedef struct
			{
				const UTF8Char *name;
				const UTF8Char *value;
			} ParamValue;

			typedef struct
			{
				const UTF8Char *name;
				const UTF8Char *value;
				const UTF8Char *domain;
				const UTF8Char *path;
				Int64 expireTime;
				Bool secure;
			} HTTPCookie;
			
			
		private:
			SSWR::AVIRead::AVIRCore *core;

			UI::GUITabControl *tcMain;

			UI::GUITabPage *tpRequest;
			UI::GUIPanel *pnlRequest;
			UI::GUILabel *lblURL;
			UI::GUITextBox *txtURL;
			UI::GUICheckBox *chkNoShutdown;
			UI::GUILabel *lblMethod;
			UI::GUIComboBox *cboMethod;
			UI::GUICheckBox *chkOSClient;
			UI::GUIButton *btnUserAgent;
			UI::GUILabel *lblUserAgent;
			UI::GUILabel *lblUserName;
			UI::GUITextBox *txtUserName;
			UI::GUILabel *lblPassword;
			UI::GUITextBox *txtPassword;
			UI::GUILabel *lblFileUpload;
			UI::GUITextBox *txtFileFormName;
			UI::GUIButton *btnFileSelect;
			UI::GUIButton *btnFileClear;
			UI::GUILabel *lblFileStatus;
			UI::GUILabel *lblDataStr;
			UI::GUITextBox *txtDataStr;
			UI::GUIButton *btnDataStr;
			UI::GUILabel *lblPostFormat;
			UI::GUIComboBox *cboPostFormat;
			UI::GUIButton *btnRequest;
			UI::GUIListView *lvReqData;

			UI::GUITabPage *tpResponse;
			UI::GUIPanel *pnlResponse;
			UI::GUILabel *lblReqURL;
			UI::GUITextBox *txtReqURL;
			UI::GUILabel *lblSvrIP;
			UI::GUITextBox *txtSvrIP;
			UI::GUILabel *lblTimeDNS;
			UI::GUITextBox *txtTimeDNS;
			UI::GUILabel *lblTimeConn;
			UI::GUITextBox *txtTimeConn;
			UI::GUILabel *lblTimeSendHdr;
			UI::GUITextBox *txtTimeSendHdr;
			UI::GUILabel *lblTimeResp;
			UI::GUITextBox *txtTimeResp;
			UI::GUILabel *lblTimeTotal;
			UI::GUITextBox *txtTimeTotal;
			UI::GUILabel *lblRespStatus;
			UI::GUITextBox *txtRespStatus;
			UI::GUILabel *lblRespSize;
			UI::GUITextBox *txtRespSize;
			UI::GUIListView *lvHeaders;
			UI::GUIPanel *pnlControl;
			UI::GUIButton *btnSave;
			UI::GUIButton *btnView;

			UI::GUITabPage *tpCookie;
			UI::GUIListView *lvCookie;

			Net::SocketFactory *sockf;
			Bool threadRunning;
			Bool threadToStop;
			Sync::Event *threadEvt;
			const UTF8Char *reqURL;
			const UTF8Char *reqBody;
			const UTF8Char *reqBodyType;
			const UTF8Char *reqUserName;
			const UTF8Char *reqPassword;
			UOSInt reqBodyLen;
			const Char *reqMeth;
			Bool reqOSClient;
			Bool noShutdown;
			const UTF8Char *userAgent;

			Bool respChanged;
			Net::SocketUtil::AddressInfo respSvrAddr;
			Double respTimeDNS;
			Double respTimeConn;
			Double respTimeReq;
			Double respTimeResp;
			Double respTimeTotal;
			UInt64 respSize;
			Int32 respStatus;
			const UTF8Char *respReqURL;
			Data::ArrayList<const UTF8Char*> *respHeaders;
			Sync::Mutex *respMut;
			const UTF8Char *respContType;
			IO::MemoryStream *respData;
			Data::ArrayList<ParamValue*> *params;
			Data::ArrayList<HTTPCookie *> *cookieList;
			Sync::Mutex *cookieMut;
			Data::ArrayList<const UTF8Char*> *fileList;

			static void __stdcall OnUserAgentClicked(void *userObj);
			static void __stdcall OnRequestClicked(void *userObj);
			static void __stdcall OnSaveClicked(void *userObj);
			static void __stdcall OnViewClicked(void *userObj);
			static void __stdcall OnDataStrClicked(void *userObj);
			static void __stdcall OnFileSelectClicked(void *userObj);
			static void __stdcall OnFileClearClicked(void *userObj);
			static UInt32 __stdcall ProcessThread(void *userObj);
			static void __stdcall OnTimerTick(void *userObj);
			void ClearHeaders();
			void ClearParams();
			void ClearCookie();
			void ClearFiles();
			HTTPCookie *SetCookie(const UTF8Char *cookieStr, const UTF8Char *reqURL);
		public:
			AVIRHTTPClientForm(UI::GUIClientControl *parent, UI::GUICore *ui, SSWR::AVIRead::AVIRCore *core);
			virtual ~AVIRHTTPClientForm();

			virtual void OnMonitorChanged();
		};
	}
}
#endif
