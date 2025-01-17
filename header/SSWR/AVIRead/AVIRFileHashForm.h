#ifndef _SM_SSWR_AVIREAD_AVIRFILEHASHFORM
#define _SM_SSWR_AVIREAD_AVIRFiLEHASHFORM
#include "IO/FileCheck.h"
#include "SSWR/AVIRead/AVIRCore.h"
#include "UI/GUIButton.h"
#include "UI/GUIComboBox.h"
#include "UI/GUIForm.h"
#include "UI/GUILabel.h"
#include "UI/GUIListView.h"
#include "UI/GUIPanel.h"
#include "UI/GUIProgressBar.h"
#include "UI/GUIRealtimeLineChart.h"
#include "UI/GUITabControl.h"
#include "UI/GUITabPage.h"
#include "UI/GUITextBox.h"

namespace SSWR
{
	namespace AVIRead
	{
		class AVIRFileHashForm : public UI::GUIForm, public IO::IProgressHandler
		{
		private:
			typedef struct
			{
				const UTF8Char *fileName;
				Int32 status;
			} FileStatus;

		private:
			UI::GUIPanel *pnlCheckType;
			UI::GUILabel *lblCheckType;
			UI::GUIComboBox *cboCheckType;
			UI::GUITabControl *tcMain;
			UI::GUITabPage *tpStatus;
			UI::GUITabPage *tpTasks;
			UI::GUITabPage *tpFiles;

			UI::GUIPanel *pnlStatus;
			UI::GUIPanel *pnlStatusFile;
			UI::GUILabel *lblFile;
			UI::GUITextBox *txtFileName;
			UI::GUILabel *lblSpeed;
			UI::GUITextBox *txtSpeed;
			UI::GUILabel *lblTotalSize;
			UI::GUITextBox *txtTotalSize;
			UI::GUIProgressBar *prgFile;
			UI::GUIRealtimeLineChart *rlcSpeed;

			UI::GUIPanel *pnlTasks;
			UI::GUIButton *btnTasksClear;
			UI::GUIListView *lvTasks;

			UI::GUIListView *lvFiles;

			SSWR::AVIRead::AVIRCore *core;
			Sync::Event *fileEvt;
			Int32 threadStatus;
			Bool threadToStop;
			Data::ArrayList<FileStatus*> *fileList;
			Sync::Mutex *fileMut;
			Bool fileListChg;
			Data::DateTime *lastTimerTime;
			Sync::Mutex *readMut;
			const UTF8Char *progName;
			Bool progNameChg;
			UInt64 readSize;
			UInt64 totalRead;
			UInt64 progCount;
			UInt64 progCurr;
			UInt64 progLastCount;
			IO::FileCheck::CheckType currHashType;

			static void __stdcall OnFileDrop(void *userObj, const UTF8Char **files, UOSInt nFiles);
			static void __stdcall OnTimerTick(void *userObj);
			static void __stdcall OnCheckTypeChg(void *userObj);
			static UInt32 __stdcall HashThread(void *userObj);
			void AddFile(const UTF8Char *fileName);
			void UpdateUI();
		public:
			AVIRFileHashForm(UI::GUIClientControl *parent, UI::GUICore *ui, SSWR::AVIRead::AVIRCore *core);
			virtual ~AVIRFileHashForm();

			virtual void OnMonitorChanged();

			virtual void ProgressStart(const UTF8Char *name, UInt64 count);
			virtual void ProgressUpdate(UInt64 currCount, UInt64 newCount);
			virtual void ProgressEnd();
		};
	}
}
#endif
