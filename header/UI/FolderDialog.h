#ifndef _SM_UI_FOLDERDIALOG
#define _SM_UI_FOLDERDIALOG
#include "IO/Registry.h"

namespace UI
{
	class FolderDialog
	{
	private:
		IO::Registry *reg;
		const UTF8Char *dirName;
		const UTF8Char *message;

		static Int32 __stdcall BrowseCB(void *hwnd, UInt32 uMsg, OSInt lParam, OSInt lpData);
	public:
		FolderDialog(const WChar *compName, const WChar *appName, const WChar *dialogName);
		~FolderDialog();

		void SetFolder(const UTF8Char *dirName);
		const UTF8Char *GetFolder();
		void SetMessage(const UTF8Char *message);

		Bool ShowDialog(void *ownerHandle);
	};
};
#endif
