#include "Stdafx.h"
#include "Math/Math.h"
#include "SSWR/AVIRead/AVIRLineSelector.h"

void __stdcall SSWR::AVIRead::AVIRLineSelector::OnResized(void *userObj)
{
	SSWR::AVIRead::AVIRLineSelector *me = (SSWR::AVIRead::AVIRLineSelector*)userObj;
	me->UpdateLineStyles();
}

void SSWR::AVIRead::AVIRLineSelector::OnDraw(Media::DrawImage *img)
{
	UOSInt defVal = this->env->GetDefLineStyle();
	UOSInt currPos = this->GetVScrollPos();
	UOSInt i = 0;
	UOSInt j = this->env->GetLineStyleCount();
	UTF8Char sbuff[256];

	UOSInt w = img->GetWidth();;
	UOSInt h = img->GetHeight();
	Double hdpi = this->GetHDPI();
	Double ddpi = this->GetDDPI();
	UInt32 itemH = (UInt32)Math::Double2Int32(22 * hdpi / ddpi);
	UInt32 itemTH = (UInt32)Math::Double2Int32(24 * hdpi / ddpi);
	UInt32 itemL = (UInt32)Math::Double2Int32(5 * hdpi / ddpi);
	UInt32 itemW = (UInt32)Math::Double2Int32(120 * hdpi / ddpi);
	Media::DrawEngine *deng = this->core->GetDrawEngine();
	Media::DrawImage *tmpBmp;
	if (w >= itemW)
	{
		tmpBmp = deng->CreateImage32((UInt32)Math::Double2Int32(110 * hdpi / ddpi), itemH, Media::AT_NO_ALPHA);
	}
	else if (w >= 10)
	{
		tmpBmp = deng->CreateImage32(w - 10, itemH, Media::AT_NO_ALPHA);
	}
	else
	{
		tmpBmp = deng->CreateImage32(w, itemH, Media::AT_NO_ALPHA);
	}
	tmpBmp->SetHDPI(hdpi / ddpi * 96.0);
	tmpBmp->SetVDPI(hdpi / ddpi * 96.0);

	Media::DrawBrush *bWhite = img->NewBrushARGB(this->colorConv->ConvRGB8(0xffffffff));
	Media::DrawBrush *bBlack = img->NewBrushARGB(this->colorConv->ConvRGB8(0xff000000));
	while (currPos < j && i < h)
	{
		if (currPos == defVal)
		{
			Media::DrawBrush *bDef = img->NewBrushARGB(this->colorConv->ConvRGB8(0xffffffc0));
			img->DrawRect(0, Math::UOSInt2Double(i), Math::UOSInt2Double(w), Math::UOSInt2Double(itemTH), 0, bDef);
			img->DelBrush(bDef);
		}
		else
		{
			img->DrawRect(0, Math::UOSInt2Double(i), Math::UOSInt2Double(w), itemTH, 0, bWhite);
		}
		this->core->GenLineStylePreview(tmpBmp, deng, this->env, currPos, this->colorConv);
		if (currPos == this->currLineStyle)
		{
			Media::DrawBrush *bRed = img->NewBrushARGB(this->colorConv->ConvRGB8(0xffff0000));
			img->DrawRect(0, Math::UOSInt2Double(i), itemW, itemTH, 0, bRed);
			img->DelBrush(bRed);
		}
		if (w >= itemW)
		{
			img->DrawImagePt(tmpBmp, itemL, Math::UOSInt2Double(i + 1));
		}
		else if (w >= 10)
		{
			img->DrawImagePt(tmpBmp, 5, Math::UOSInt2Double(i + 1));
		}
		else
		{
			img->DrawImagePt(tmpBmp, 0, Math::UOSInt2Double(i + 1));
		}
		sbuff[0] = 0;
		this->env->GetLineStyleName(currPos, sbuff);
		if (sbuff[0])
		{
			Media::DrawFont *fnt = this->CreateDrawFont(img);
			img->DrawString(itemW, Math::UOSInt2Double(i + 1), sbuff, fnt, bBlack);
			img->DelFont(fnt);
		}

		i += itemTH;
		currPos++;
	}
	if (i < h)
	{
		img->DrawRect(0, Math::UOSInt2Double(i), Math::UOSInt2Double(w), Math::UOSInt2Double(h - i), 0, bWhite);
	}
	img->DelBrush(bWhite);
	img->DelBrush(bBlack);
	deng->DeleteImage(tmpBmp);
}

void SSWR::AVIRead::AVIRLineSelector::OnMouseDown(OSInt scrollY, Int32 xPos, Int32 yPos, UI::GUIClientControl::MouseButton btn, KeyButton keys)
{
	OSInt i = scrollY + (yPos / Math::Double2Int32(24 * this->GetHDPI() / this->GetDDPI()));
	if (i >= (OSInt)this->env->GetLineStyleCount())
	{
		i = -1;
	}
	else
	{
		if (i != (OSInt)this->currLineStyle)
		{
			this->currLineStyle = (UOSInt)i;
			this->EventSelChg();
			this->Redraw();
		}
		if (btn == UI::GUIClientControl::MBTN_RIGHT && this->mnuLayers)
		{
			OSInt x;
			OSInt y;
			this->GetScreenPosP(&x, &y);
			this->mnuLayers->ShowMenu(this, x + xPos, y + yPos);
		}
	}
}

void SSWR::AVIRead::AVIRLineSelector::OnKeyDown(UInt32 keyCode)
{
}

SSWR::AVIRead::AVIRLineSelector::AVIRLineSelector(UI::GUICore *ui, UI::GUIClientControl *parent, SSWR::AVIRead::AVIRCore *core, Map::MapEnv *env, UOSInt initLineStyle, Media::ColorManagerSess *colorSess) : UI::GUICustomDrawVScroll(ui, parent, core->GetDrawEngine())
{
	this->core = core;
	this->env = env;
	this->colorSess = colorSess;
	this->currLineStyle = initLineStyle;
	this->mnuLayers = 0;
	Media::ColorProfile srcProfile(Media::ColorProfile::CPT_SRGB);
	Media::ColorProfile destProfile(Media::ColorProfile::CPT_PDISPLAY);
	NEW_CLASS(this->colorConv, Media::ColorConv(&srcProfile, &destProfile, this->colorSess));
	if (this->colorSess)
	{
		this->colorSess->AddHandler(this);
	}

	this->HandleSizeChanged(OnResized, this);

	this->UpdateLineStyles();
}

SSWR::AVIRead::AVIRLineSelector::~AVIRLineSelector()
{
	DEL_CLASS(this->colorConv);
	if (this->colorSess)
	{
		this->colorSess->RemoveHandler(this);
	}
}

void SSWR::AVIRead::AVIRLineSelector::YUVParamChanged(const Media::IColorHandler::YUVPARAM *yuvParam)
{
}

void SSWR::AVIRead::AVIRLineSelector::RGBParamChanged(const Media::IColorHandler::RGBPARAM2 *rgbParam)
{
	this->colorConv->RGBParamChanged(rgbParam);
	this->Redraw();
}

void SSWR::AVIRead::AVIRLineSelector::SetPopupMenu(UI::GUIPopupMenu *mnuLayers)
{
	this->mnuLayers = mnuLayers;
}

void SSWR::AVIRead::AVIRLineSelector::UpdateLineStyles()
{
	Double w;
	Double h;

	this->GetSize(&w, &h);
	UOSInt page = (UOSInt)(h / 24);
	if (page <= 0)
		page = 1;
	this->SetVScrollBar(0, this->env->GetLineStyleCount(), page);
}

UOSInt SSWR::AVIRead::AVIRLineSelector::GetSelectedLineStyle()
{
	return this->currLineStyle;
}
