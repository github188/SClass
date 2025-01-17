#ifndef _SM_SSWR_AVIREAD_AVIRPROCINFOFORM
#define _SM_SSWR_AVIREAD_AVIRPROCINFOFORM

#include "Manage/HiResClock.h"
#include "Manage/SymbolResolver.h"
#include "SSWR/AVIRead/AVIRCore.h"
#include "UI/GUIButton.h"
#include "UI/GUICheckBox.h"
#include "UI/GUIForm.h"
#include "UI/GUIGroupBox.h"
#include "UI/GUIHSplitter.h"
#include "UI/GUILabel.h"
#include "UI/GUIListBox.h"
#include "UI/GUIListView.h"
#include "UI/GUIPanel.h"
#include "UI/GUIRealtimeLineChart.h"
#include "UI/GUITabControl.h"
#include "UI/GUITabPage.h"
#include "UI/GUITextBox.h"
#include "UI/GUIVSplitter.h"

namespace SSWR
{
	namespace AVIRead
	{
		class AVIRProcInfoForm : public UI::GUIForm
		{
		private:
			typedef struct
			{
				UInt32 procId;
				const UTF8Char *procName;
				UInt32 parentProcId;
				Bool found;
			} ProcessInfo;
		private:
			UI::GUITabControl *tcMain;
			UI::GUITabPage *tpSummary;
			UI::GUITabPage *tpDetail;

			UI::GUIPanel *pnlSummary;
			UI::GUIListView *lvSummary;

			UI::GUIListBox *lbDetail;
			UI::GUIHSplitter *hspDetail;
			UI::GUITabControl *tcDetail;
			UI::GUITabPage *tpDetInfo;
			UI::GUITabPage *tpDetModule;
			UI::GUITabPage *tpDetThread;
			UI::GUITabPage *tpDetHeap;
			UI::GUITabPage *tpDetChart;

			UI::GUILabel *lblDetProcId;
			UI::GUITextBox *txtDetProcId;
			UI::GUILabel *lblDetParentId;
			UI::GUITextBox *txtDetParentId;
			UI::GUILabel *lblDetName;
			UI::GUITextBox *txtDetName;
			UI::GUILabel *lblDetPath;
			UI::GUITextBox *txtDetPath;
			UI::GUILabel *lblDetPriority;
			UI::GUITextBox *txtDetPriority;
			
			UI::GUIPanel *pnlDetModule;
			UI::GUIButton *btnDetModule;
			UI::GUIListView *lvDetModule;

			UI::GUIPanel *pnlDetThread;
			UI::GUIButton *btnDetThread;
			UI::GUIListView *lvDetThread;

			UI::GUIPanel *pnlDetHeap;
			UI::GUIButton *btnDetHeap;
			UI::GUIListBox *lbDetHeap;
			UI::GUIHSplitter *hspDetHeap;
			UI::GUITextBox *txtDetHeap;
			UI::GUIListView *lvDetHeap;

			UI::GUIGroupBox *grpDetChartCPU;
			UI::GUIRealtimeLineChart *rlcDetChartCPU;
			UI::GUIVSplitter *vspDetChartCPU;
			UI::GUIGroupBox *grpDetChartPage;
			UI::GUIRealtimeLineChart *rlcDetChartPage;
			UI::GUIVSplitter *vspDetChartPage;
			UI::GUIGroupBox *grpDetChartCount;
			UI::GUIRealtimeLineChart *rlcDetChartCount;
			UI::GUIVSplitter *vspDetChartCount;
			UI::GUIGroupBox *grpDetChartWS;
			UI::GUIRealtimeLineChart *rlcDetChartWS;

			SSWR::AVIRead::AVIRCore *core;

			Data::ArrayListUInt32 *procIds;
			Data::ArrayList<ProcessInfo*> *procList;

			UOSInt currProc;
			Manage::Process *currProcObj;
			Manage::SymbolResolver *currProcRes;
			Manage::HiResClock *clk;
			Data::DateTime *lastUserTime;
			Data::DateTime *lastKernelTime;
			UOSInt threadCnt;

			static void __stdcall OnSumDblClicked(void *userObj, UOSInt index);
			static void __stdcall OnProcSelChg(void *userObj);
			static void __stdcall OnTimerTick(void *userObj);
			static void __stdcall OnTimerCPUTick(void *userObj);
			static void __stdcall OnDetModuleRefClicked(void *userObj);
			static void __stdcall OnDetThreadRefClicked(void *userObj);
			static void __stdcall OnDetThreadDblClicked(void *userObj, UOSInt index);
			static void __stdcall OnDetHeapRefClicked(void *userObj);
			static void __stdcall OnDetHeapSelChg(void *userObj);
			static void __stdcall OnDetHeapItemSelChg(void *userObj);

			void UpdateProcModules();
			void UpdateProcThreads();
			void UpdateProcHeaps();
			void UpdateProcHeapDetail(UInt32 heapId);
		public:
			AVIRProcInfoForm(UI::GUIClientControl *parent, UI::GUICore *ui, SSWR::AVIRead::AVIRCore *core);
			virtual ~AVIRProcInfoForm();

			virtual void OnMonitorChanged();
		};
	}
}
#endif
