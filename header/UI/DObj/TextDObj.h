#ifndef _SM_UI_DOBJ_TEXTDOBJ
#define _SM_UI_DOBJ_TEXTDOBJ
#include "UI/DObj/DirectObject.h"

namespace UI
{
	namespace DObj
	{
		class TextDObj : public DirectObject
		{
		public:
			typedef enum
			{
				TA_LEFT,
				TA_CENTER,
				TA_RIGHT
			} TextAlign;
		private:
			Media::DrawEngine *deng;
			const WChar *txt;
			const WChar *fontName;
			Double fontSize;
			Media::DrawEngine::DrawFontStyle fontStyle;
			TextAlign talign;
			Int32 fontColor;
			Int32 codePage;
			OSInt width;
			OSInt height;
			Double lineHeight;
			Int32 currPage;
			Bool pageChg;
			Data::ArrayList<const WChar *> *lines;

		public:
			TextDObj(Media::DrawEngine *deng, const WChar *txt, const WChar *fontName, Double fontSize, Media::DrawEngine::DrawFontStyle fontStyle, Int32 fontColor, OSInt left, OSInt top, OSInt width, OSInt height, Int32 codePage);
			virtual ~TextDObj();

			virtual Bool IsChanged();
			virtual Bool DoEvents();
			virtual void DrawObject(Media::DrawImage *dimg);

			virtual Bool IsObject(OSInt x, OSInt y);
			//virtual System::Windows::Forms::Cursor ^GetCursor() override;
			virtual void OnMouseDown();
			virtual void OnMouseUp();
			virtual void OnMouseClick();

			void SetTextAlign(TextAlign talign);
			OSInt GetPageCount();
			Int32 GetCurrPage();
			void SetCurrPage(Int32 currPage);
		};
	}
}
#endif