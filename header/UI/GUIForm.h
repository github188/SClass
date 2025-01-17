#ifndef _SM_UI_GUIFORM
#define _SM_UI_GUIFORM
#include "UI/GUICore.h"
#include "UI/GUIClientControl.h"
#include "UI/GUIMainMenu.h"

namespace UI
{
	class GUITimer;
	class GUIButton;
	class GUIIcon;

	typedef void (__stdcall *FileEvent)(void *userObj, const UTF8Char **files, UOSInt nFiles);
	typedef void (__stdcall *MenuEvent)(void *userObj, UInt16 cmdId);

	class GUIForm : public GUIClientControl
	{
	public:
		typedef enum
		{
			FS_NORMAL,
			FS_MAXIMIZED,
			FS_MINIMIZED
		} FormState;

		typedef enum
		{
			DR_UNKNOWN,
			DR_OK,
			DR_CANCEL
		} DialogResult;

		typedef enum
		{
			CR_USER
		} CloseReason;
		typedef Bool (__stdcall *FormClosingEvent)(void *userObj, CloseReason reason); //true to cancel
		typedef void (__stdcall *FormClosedEvent)(void *userObj, UI::GUIForm *frm);
		typedef void (__stdcall *KeyEvent)(void *userObj, UOSInt keyCode, Bool extendedKey);

	private:
		Bool virtualMode;
		Data::ArrayList<FormClosedEvent> *closeHandlers;
		Data::ArrayList<void*> *closeHandlersObj;
		Data::ArrayList<FileEvent> *dropFileHandlers;
		Data::ArrayList<void*> *dropFileHandlersObj;
		Data::ArrayList<MenuEvent> *menuClickedHandlers;
		Data::ArrayList<void*> *menuClickedHandlersObj;
		Data::ArrayList<KeyEvent> *keyDownHandlers;
		Data::ArrayList<void*> *keyDownHandlersObj;
		Data::ArrayList<KeyEvent> *keyUpHandlers;
		Data::ArrayList<void*> *keyUpHandlersObj;
		Data::ArrayList<GUITimer*> *timers;
		FormClosingEvent closingHdlr;
		void *closingHdlrObj;
		Bool exitOnClose;
		UInt32 nextTmrId;
		UI::GUIMainMenu *menu;
		UI::GUIButton *okBtn;
		UI::GUIButton *cancelBtn;

		Bool fs;
		UInt32 fsStyle;
		Int32 fsX;
		Int32 fsY;
		Int32 fsW;
		Int32 fsH;

		UI::GUIForm *currDialog;
		Bool isDialog;
		DialogResult dialogResult;
		void *hAcc;

	private:
		static Int32 useCnt;
		static OSInt __stdcall FormWndProc(void *hWnd, UInt32 msg, UOSInt wParam, OSInt lParam);
		static void Init(void *hInst);
		static void Deinit(void *hInst);

		void UpdateHAcc();
		GUIForm(void *hWnd);
	public:
		static GUIForm *FindForm(const UTF8Char *formName);

	public:
		GUIForm(UI::GUIClientControl *parent, Double initW, Double initH, UI::GUICore *ui);
		virtual ~GUIForm();

		void SetFormState(FormState fs);
		DialogResult ShowDialog(UI::GUIForm *owner);
		void SetDialogResult(DialogResult dr);
		void ShowTitleBar(Bool show);
		void SetAlwaysOnTop(Bool alwaysOnTop);

		void MakeActive();
		void MakeForeground();
		virtual void Close();
		virtual void SetText(const UTF8Char *text);
		virtual void GetSizeP(UOSInt *width, UOSInt *height);
		virtual void SetExitOnClose(Bool exitOnClose);
		virtual void SetNoResize(Bool noResize);
		virtual UI::GUITimer *AddTimer(UInt32 interval, UI::UIEvent handler, void *userObj);
		virtual void RemoveTimer(UI::GUITimer *tmr);
		virtual void SetMenu(UI::GUIMainMenu *menu);
		virtual UI::GUIMainMenu *GetMenu();
		virtual void UpdateMenu();
		virtual void SetDefaultButton(UI::GUIButton *btn);
		virtual void SetCancelButton(UI::GUIButton *btn);

		virtual void GetClientSize(Double *w, Double *h);
		virtual Bool IsChildVisible();

		virtual const UTF8Char *GetObjectClass();
		virtual OSInt OnNotify(UInt32 code, void *lParam);
		virtual void OnSizeChanged(Bool updateScn);
		virtual Bool OnPaint();
		void OnDropFiles(void *hDrop);
		virtual void EventMenuClicked(UInt16 cmdId);
		void SetSmallIcon(UI::GUIIcon *icon);
		void SetLargeIcon(UI::GUIIcon *icon);
		void ShowMouseCursor(Bool toShow);

		virtual void HandleFormClosed(FormClosedEvent handler, void *userObj);
		virtual void HandleDropFiles(FileEvent handler, void *userObj);
		virtual void HandleMenuClicked(MenuEvent handler, void *userObj);
		virtual void HandleKeyDown(KeyEvent handler, void *userObj);
		virtual void HandleKeyUp(KeyEvent handler, void *userObj);
		void SetClosingHandler(FormClosingEvent handler, void *userObj);
		virtual void SetDPI(Double hdpi, Double ddpi);

		void EventClosed();
		void EventTimer(UOSInt tmrId);
		virtual void OnFocus();
		virtual void OnFocusLost();
		virtual void OnDisplaySizeChange(UOSInt dispWidth, UOSInt dispHeight);
		void OnFileDrop(const UTF8Char **files, UOSInt nFiles);

		void ToFullScn();
		void FromFullScn();
		UI::GUICore *GetUI();
	};
}
#endif
