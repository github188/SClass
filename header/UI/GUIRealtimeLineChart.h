#ifndef _SM_UI_GUIREALTIMELINECHART
#define _SM_UI_GUIREALTIMELINECHART
#include "Media/DrawEngine.h"
#include "Sync/Mutex.h"
#include "UI/GUIControl.h"

namespace UI
{
	class GUIRealtimeLineChart : public GUIControl
	{
	private:
		static Int32 useCnt;

		Media::DrawEngine *eng;
		const UTF8Char *unit;
		OSInt lineCnt;
		OSInt sampleCnt;
		Int32 *lineColor;
		Int32 bgColor;
		Int32 fontColor;

		Bool valueChanged;
		Double *chartVal;
		OSInt chartOfst;
		Bool chartMaxChg;
		Double chartMax;
		Double chartMin;
		Sync::Mutex *chartMut;
		void *clsData;

	public:
		static OSInt __stdcall RLCWndProc(void *hWnd, UInt32 msg, UInt32 wParam, OSInt lParam);
	private:
		void OnPaint(Media::DrawImage *dimg);
		void Init(void *hInst);
		void Deinit(void *hInst);
	public:
		GUIRealtimeLineChart(GUICore *ui, UI::GUIClientControl *parent, Media::DrawEngine *eng, OSInt lineCnt, OSInt sampleCnt, Int32 updateInterval);
		virtual ~GUIRealtimeLineChart();

		virtual const UTF8Char *GetObjectClass();
		virtual OSInt OnNotify(Int32 code, void *lParam);

		void AddSample(Double *samples);
		void SetLineColor(OSInt lineId, Int32 lineColor);
		void SetUnit(const UTF8Char *unit);
		void SetBGColor(Int32 lineColor);
		void SetFontColor(Int32 fontColor);
		void ClearChart();

	};
};
#endif