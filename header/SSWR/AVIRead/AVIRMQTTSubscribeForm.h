#ifndef _SM_SSWR_AVIREAD_AVIRMQTTSUBSCRIBEFORM
#define _SM_SSWR_AVIREAD_AVIRMQTTSUBSCRIBEFORM
#include "Data/StringUTF8Map.h"
#include "Net/MQTTClient.h"
#include "SSWR/AVIRead/AVIRCore.h"
#include "UI/GUIButton.h"
#include "UI/GUIForm.h"
#include "UI/GUIHSplitter.h"
#include "UI/GUILabel.h"
#include "UI/GUIListBox.h"
#include "UI/GUIListView.h"
#include "UI/GUIPanel.h"
#include "UI/GUIPictureBoxSimple.h"
#include "UI/GUITabControl.h"
#include "UI/GUITabPage.h"
#include "UI/GUITextBox.h"
#include "UI/GUIVSplitter.h"
#include "UI/ListBoxLogger.h"

namespace SSWR
{
	namespace AVIRead
	{
		class AVIRMQTTSubscribeForm : public UI::GUIForm
		{
		private:
			typedef struct
			{
				const UTF8Char *topic;
				UTF8Char *currValue;
				Bool updated;
				UOSInt recvCnt;
				Int64 lastRecvTime;
				Int64 dateList[256];
				Double valueList[256];
			} TopicStatus;
			
		private:
			SSWR::AVIRead::AVIRCore *core;
			Net::MQTTClient *client;
			IO::LogTool *log;
			UI::ListBoxLogger *logger;
			Sync::Mutex *topicMut;
			Data::StringUTF8Map<TopicStatus*> *topicMap;
			Bool topicListChanged;
			TopicStatus *currTopic;
			Media::DrawImage *dispImg;

			UI::GUITabControl *tcMain;

			UI::GUITabPage *tpStatus;
			UI::GUILabel *lblHost;
			UI::GUITextBox *txtHost;
			UI::GUILabel *lblPort;
			UI::GUITextBox *txtPort;
			UI::GUILabel *lblUsername;
			UI::GUITextBox *txtUsername;
			UI::GUILabel *lblPassword;
			UI::GUITextBox *txtPassword;
			UI::GUIButton *btnStart;

			UI::GUITabPage *tpSTopic;
			UI::GUILabel *lblSTopic;
			UI::GUITextBox *txtSTopic;
			UI::GUIButton *btnSTopic;
			UI::GUIListBox *lbSTopic;

			UI::GUITabPage *tpPublish;
			UI::GUILabel *lblPublishTopic;
			UI::GUITextBox *txtPublishTopic;
			UI::GUILabel *lblPublishMessage;
			UI::GUITextBox *txtPublishMessage;
			UI::GUIButton *btnPublish;

			UI::GUITabPage *tpTopic;
			UI::GUIListView *lvTopic;
			UI::GUIVSplitter *vspTopic;
			UI::GUIPictureBoxSimple *pbTopic;

			UI::GUITabPage *tpLog;
			UI::GUIListBox *lbLog;
			UI::GUITextBox *txtLog;

			static void __stdcall OnStartClicked(void *userObj);
			static void __stdcall OnSTopicClicked(void *userObj);
			static void __stdcall OnLogSelChg(void *userObj);
			static void __stdcall OnTopicSelChg(void *userObj);
			static void __stdcall OnPublishClicked(void *userObj);
			static void __stdcall OnPingTimerTick(void *userObj);
			static void __stdcall OnTimerTick(void *userObj);
			static void __stdcall OnPublishMessage(void *userObj, const UTF8Char *topic, const UInt8 *message, UOSInt msgSize);

			void UpdateTopicChart();
			void ServerStop();
		public:
			AVIRMQTTSubscribeForm(UI::GUIClientControl *parent, UI::GUICore *ui, SSWR::AVIRead::AVIRCore *core);
			virtual ~AVIRMQTTSubscribeForm();

			virtual void OnMonitorChanged();
		};
	}
}
#endif
