#ifndef _SM_SSWR_AVIREAD_AVIRPACKAGEFORM
#define _SM_SSWR_AVIREAD_AVIRPACKAGEFORM
#include "IO/PackageFile.h"
#include "SSWR/AVIRead/AVIRCore.h"
#include "UI/GUIForm.h"
#include "UI/GUILabel.h"
#include "UI/GUIListView.h"
#include "UI/GUIPanel.h"
#include "UI/GUIPopupMenu.h"
#include "UI/GUIProgressBar.h"
#include "UI/GUIRealtimeLineChart.h"
#include "UI/GUITabControl.h"
#include "UI/GUITabPage.h"
#include "UI/GUITextBox.h"
#include "UI/GUIVSplitter.h"

namespace SSWR
{
	namespace AVIRead
	{
		class AVIRPackageForm : public UI::GUIForm, public IO::IProgressHandler
		{
		private:
			typedef enum
			{
				AT_COPY,
				AT_MOVE,
				AT_DELETE,
				AT_COPYFAIL,
				AT_MOVEFAIL,
				AT_DELETEFAIL,
				AT_SUCCEED,
				AT_RETRYCOPY,
				AT_RETRYMOVE
			} ActionType;
		private:
			UI::GUIPanel *pnlCtrl;
			UI::GUITabControl *tcMain;
			UI::GUIProgressBar *prgStatus;

			UI::GUITabPage *tpFiles;
			UI::GUIListView *lvFiles;

			UI::GUITabPage *tpStatus;
			UI::GUIPanel *pnlStatus;
			UI::GUILabel *lblStatusFile;
			UI::GUITextBox *txtStatusFile;
			UI::GUIPanel *pnlStatusBNT;
			UI::GUILabel *lblStatusFileSize;
			UI::GUITextBox *txtStatusFileSize;
			UI::GUILabel *lblStatusCurrSize;
			UI::GUITextBox *txtStatusCurrSize;
			UI::GUILabel *lblStatusCurrSpeed;
			UI::GUITextBox *txtStatusCurrSpeed;
			UI::GUILabel *lblStatusTimeLeft;
			UI::GUITextBox *txtStatusTimeLeft;
			UI::GUIListView *lvStatus;
			UI::GUIVSplitter *vspStatus;
			UI::GUIRealtimeLineChart *rlcStatus;

			UI::GUITabPage *tpInfo;
			UI::GUITextBox *txtInfo;

			UI::GUIPopupMenu *mnuPopup;

			SSWR::AVIRead::AVIRCore *core;
			IO::PackageFile *packFile;

			Sync::Mutex *statusFileMut;
			Bool statusFileChg;
			const UTF8Char *statusFile;
			Int64 statusFileSize;
			Int64 statusDispSize;
			Double statusDispSpd;
			IO::ActiveStreamReader::BottleNeckType statusBNT;
			IO::ActiveStreamReader::BottleNeckType statusDispBNT;

			Sync::Mutex *fileMut;
			Data::ArrayList<const UTF8Char *> *fileNames;
			Data::ArrayList<ActionType> *fileAction;
			Bool statusChg;
			Bool threadRunning;
			Bool threadToStop;
			Sync::Event *threadEvt;

			Sync::Mutex *readMut;
			Int64 readTotal;
			Int64 readCurr;
			Int64 readLast;
			const UTF8Char *readCurrFile;
			Int64 readFileCnt;
			Data::DateTime *readLastTimer;
			Data::DateTime *readReadTime;

			Sync::Mutex *progMut;
			const UTF8Char *progName;
			Int64 progStartCnt;
			Bool progStarted;
			Int64 progUpdateCurr;
			Int64 progUpdateNew;
			Bool progUpdated;
			Bool progEnd;

			static UInt32 __stdcall ProcessThread(void *userObj);
			static void __stdcall OnTimerTick(void *userObj);
			static void __stdcall LVDblClick(void *userObj, OSInt index);
			static void __stdcall OnStatusDblClick(void *userObj, OSInt index);
			void DisplayPackFile(IO::PackageFile *packFile);
		public:
			AVIRPackageForm(UI::GUIClientControl *parent, UI::GUICore *ui, SSWR::AVIRead::AVIRCore *core, IO::PackageFile *packFile);
			virtual ~AVIRPackageForm();

			virtual void EventMenuClicked(UInt16 cmdId);
			virtual void OnMonitorChanged();

			virtual void ProgressStart(const UTF8Char *name, Int64 count);
			virtual void ProgressUpdate(Int64 currCount, Int64 newCount);
			virtual void ProgressEnd();
		};
	};
};
#endif