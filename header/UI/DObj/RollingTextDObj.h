#ifndef _SM_UI_DOBJ_ROLLINGTEXTDOBJ
#define _SM_UI_DOBJ_ROLLINGTEXTDOBJ
#include "Data/DateTime.h"
#include "UI/DObj/DirectObject.h"

namespace UI
{
	namespace DObj
	{
		class RollingTextDObj : public DirectObject
		{
		private:
			Media::DrawEngine *deng;
			const WChar *txt;
			const WChar *fontName;
			Double fontSize;
			Int32 fontColor;
			Int32 codePage;
			UOSInt width;
			UOSInt height;
			Double lineHeight;
			Media::DrawImage *dimg;
			Double rollSpeed;
			OSInt lastRollPos;
			Data::DateTime *startTime;

			void UpdateBGImg();
		public:
			RollingTextDObj(Media::DrawEngine *deng, const WChar *txt, const WChar *fontName, Double fontSize, Int32 fontColor, OSInt left, OSInt top, OSInt width, OSInt height, Int32 codePage, Double rollSpeed);
			virtual ~RollingTextDObj();

			virtual Bool IsChanged();
			virtual Bool DoEvents();
			virtual void DrawObject(Media::DrawImage *dimg);

			virtual Bool IsObject(OSInt x, OSInt y);
			//virtual System::Windows::Forms::Cursor ^GetCursor() override;
			virtual void OnMouseDown();
			virtual void OnMouseUp();
			virtual void OnMouseClick();
		};
	}
}
#endif
