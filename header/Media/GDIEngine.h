#ifndef _SM_MEDIA_GDIENGINE
#define _SM_MEDIA_GDIENGINE
#include "Media/DrawEngine.h"
#include "Media/ImageAlphaBlend.h"
#include "Sync/Mutex.h"

namespace Media
{
	class GDIImage;

	class GDIEngine : public Media::DrawEngine
	{
	public:
		Media::ImageAlphaBlend *iab;
	private:
		void *hdc;
		void *hdcScreen;
		void *hpenBlack;
		void *hbrushWhite;
		Sync::Mutex *gdiMut;
#ifndef _WIN32_WCE
		void *gdiplusStartupInput;
		UInt32 gdiplusToken;
#endif

	public:
		GDIEngine();
		virtual ~GDIEngine();

		virtual DrawImage *CreateImage32(UOSInt width, UOSInt height, Media::AlphaType atype);
		GDIImage *CreateImage24(UOSInt width, UOSInt height);
		DrawImage *CreateImageScn(void *hdc, OSInt left, OSInt top, OSInt right, OSInt bottom);
		virtual DrawImage *LoadImage(const UTF8Char *fileName);
		virtual DrawImage *LoadImageStream(IO::SeekableStream *stm);
		virtual DrawImage *ConvImage(Media::Image *img);
		virtual DrawImage *CloneImage(DrawImage *img);
		virtual Bool DeleteImage(DrawImage *img);
		void *GetBlackPen();
		void *GetWhiteBrush();
	};

	class GDIBrush : public DrawBrush
	{
	public:
		void *hbrush;
		DrawImage *img;
		UInt32 color;
		UInt32 oriColor;

	public:
		GDIBrush(void *hbrush, UInt32 oriColor, DrawImage *img);
		virtual ~GDIBrush();
	};

	class GDIPen : public DrawPen
	{
	public:
		void *hpen;
		UInt32 *pattern;
		UOSInt nPattern;
		DrawImage *img;
		Double thick;
		UInt32 oriColor;
	public:
		GDIPen(void *hpen, UInt32 *pattern, UOSInt nPattern, DrawImage *img, Double thick, UInt32 oriColor);
		virtual ~GDIPen();

		Double GetThick();
	};

	class GDIFont : public DrawFont
	{
	private:
		DrawImage *img;
		void *hdc;
		const WChar *fontName;
		Double ptSize;
		Media::DrawEngine::DrawFontStyle style;
		Int32 codePage;
		
	public:
		Int32 pxSize;
		void *hfont;

		GDIFont(void *hdc, const Char *fontName, Double ptSize, Media::DrawEngine::DrawFontStyle style, DrawImage *img, Int32 codePage);
		GDIFont(void *hdc, const WChar *fontName, Double ptSize, Media::DrawEngine::DrawFontStyle style, DrawImage *img, Int32 codePage);
		virtual ~GDIFont();

		const WChar *GetNameW();
		Double GetPointSize();
		Media::DrawEngine::DrawFontStyle GetFontStyle();
		Int32 GetCodePage();
	};

	class GDIImage : public DrawImage, public Image
	{
	private:
		GDIEngine *eng;
		UOSInt width;
		UOSInt height;
		UInt32 bitCount;
		Media::DrawEngine::DrawPos strAlign;

		DrawBrush *currBrush;
		DrawFont *currFont;
		DrawPen *currPen;

		OSInt left;
		OSInt top;
	public:
		void *hBmp;
		void *bmpBits;
		void *hdcBmp;

		GDIImage(GDIEngine *eng, OSInt left, OSInt top, UOSInt width, UOSInt height, UInt32 bitCount, void *hBmp, void *bmpBits, void *hdcBmp, Media::AlphaType atype);
		virtual ~GDIImage();

		virtual UOSInt GetWidth();
		virtual UOSInt GetHeight();
		virtual UInt32 GetBitCount();
		virtual ColorProfile *GetColorProfile();
		virtual void SetColorProfile(const ColorProfile *color);
		virtual Media::AlphaType GetAlphaType();
		virtual void SetAlphaType(Media::AlphaType atype);
		virtual Double GetHDPI();
		virtual Double GetVDPI();
		virtual void SetHDPI(Double dpi);
		virtual void SetVDPI(Double dpi);
		virtual UInt8 *GetImgBits(Bool *revOrder);
		virtual void GetImgBitsEnd(Bool modified);
		virtual UOSInt GetImgBpl();
		virtual Media::EXIFData *GetEXIF();
		virtual Media::PixelFormat GetPixelFormat();

		virtual Bool DrawLine(Double x1, Double y1, Double x2, Double y2, DrawPen *p);
		virtual Bool DrawPolylineI(Int32 *points, UOSInt nPoints, DrawPen *p);
		virtual Bool DrawPolygonI(Int32 *points, UOSInt nPoints, DrawPen *p, DrawBrush *b);
		virtual Bool DrawPolyPolygonI(Int32 *points, UInt32 *pointCnt, UOSInt nPointCnt, DrawPen *p, DrawBrush *b);
		virtual Bool DrawPolyline(Double *points, UOSInt nPoints, DrawPen *p);
		virtual Bool DrawPolygon(Double *points, UOSInt nPoints, DrawPen *p, DrawBrush *b);
		virtual Bool DrawPolyPolygon(Double *points, UInt32 *pointCnt, UOSInt nPointCnt, DrawPen *p, DrawBrush *b);
		virtual Bool DrawRect(Double x, Double y, Double w, Double h, DrawPen *p, DrawBrush *b);
		virtual Bool DrawEllipse(Double tlx, Double tly, Double w, Double h, DrawPen *p, DrawBrush *b);
		virtual Bool DrawString(Double tlx, Double tly, const UTF8Char *str, DrawFont *f, DrawBrush *b);
		Bool DrawStringW(Double tlx, Double tly, const WChar *str, DrawFont *f, DrawBrush *p);
		virtual Bool DrawStringRot(Double centX, Double centY, const UTF8Char *str, DrawFont *f, DrawBrush *p, Double angleDegree);
		Bool DrawStringRotW(Double centX, Double centY, const WChar *str, DrawFont *f, DrawBrush *p, Double angleDegree);
		virtual Bool DrawStringB(Double tlx, Double tly, const UTF8Char *str, DrawFont *f, DrawBrush *p, UOSInt buffSize);
		Bool DrawStringBW(Double tlx, Double tly, const WChar *str, DrawFont *f, DrawBrush *p, UOSInt buffSize);
		virtual Bool DrawStringRotB(Double centX, Double centY, const UTF8Char *str, DrawFont *f, DrawBrush *p, Double angleDegree, UOSInt buffSize);
		Bool DrawStringRotBW(Double centX, Double centY, const WChar *str, DrawFont *f, DrawBrush *p, Double angleDegree, UOSInt buffSize);
		virtual Bool DrawImagePt(DrawImage *img, Double tlx, Double tly);
		virtual Bool DrawImagePt2(Media::StaticImage *img, Double tlx, Double tly);
		virtual Bool DrawImagePt3(DrawImage *img, Double destX, Double destY, Double srcX, Double srcY, Double srcW, Double srcH);
		Bool DrawImageRect(DrawImage *img, OSInt tlx, OSInt tly, OSInt brx, OSInt bry);

		virtual DrawPen *NewPenARGB(UInt32 color, Double thick, UInt8 *pattern, UOSInt nPattern);
		virtual DrawBrush *NewBrushARGB(UInt32 color);
		virtual DrawFont *NewFontPt(const UTF8Char *name, Double ptSize, Media::DrawEngine::DrawFontStyle fontStyle, UInt32 codePage);
		DrawFont *NewFontPtW(const WChar *name, Double ptSize, Media::DrawEngine::DrawFontStyle fontStyle, UInt32 codePage);
		virtual DrawFont *NewFontPx(const UTF8Char *name, Double pxSize, Media::DrawEngine::DrawFontStyle fontStyle, UInt32 codePage);
		DrawFont *NewFontPxW(const WChar *name, Double pxSize, Media::DrawEngine::DrawFontStyle fontStyle, UInt32 codePage);
		virtual DrawFont *CloneFont(Media::DrawFont *f);
		virtual void DelPen(DrawPen *p);
		virtual void DelBrush(DrawBrush *b);
		virtual void DelFont(DrawFont *f);

		virtual Bool GetTextSize(DrawFont *fnt, const UTF8Char *txt, Double *sz);
		virtual Bool GetTextSizeC(DrawFont *fnt, const UTF8Char *txt, UOSInt txtLen, Double *sz);
		Bool GetTextSize(DrawFont *fnt, const WChar *txt, OSInt txtLen, Double *sz);
		virtual void SetTextAlign(DrawEngine::DrawPos pos);
		virtual void GetStringBound(Int32 *pos, OSInt centX, OSInt centY, const UTF8Char *str, DrawFont *f, OSInt *drawX, OSInt *drawY);
		void GetStringBoundW(Int32 *pos, OSInt centX, OSInt centY, const WChar *str, DrawFont *f, OSInt *drawX, OSInt *drawY);
		virtual void GetStringBoundRot(Int32 *pos, Double centX, Double centY, const UTF8Char *str, DrawFont *f, Double angleDegree, OSInt *drawX, OSInt *drawY);
		void GetStringBoundRotW(Int32 *pos, Double centX, Double centY, const WChar *str, DrawFont *f, Double angleDegree, OSInt *drawX, OSInt *drawY);
		virtual void CopyBits(OSInt x, OSInt y, void *imgPtr, UOSInt bpl, UOSInt width, UOSInt height, Bool upsideDown);

		virtual Media::StaticImage *ToStaticImage();
		virtual UOSInt SavePng(IO::SeekableStream *stm);
		virtual UOSInt SaveGIF(IO::SeekableStream *stm);
		virtual UOSInt SaveJPG(IO::SeekableStream *stm);

		virtual Media::Image *Clone();
		virtual Media::Image::ImageType GetImageType();
		virtual void GetImageData(UInt8 *destBuff, OSInt left, OSInt top, UOSInt width, UOSInt height, UOSInt destBpl, Bool upsideDown);
		virtual Int32 GetPixel32(OSInt x, OSInt y);

		static void PolylineAccel(void *hdc, Int32 *points, UOSInt nPoints, OSInt ofstX, OSInt ofstY, OSInt width, OSInt height);
		static void PolygonAccel(void *hdc, Int32 *points, UOSInt nPoints, OSInt ofstX, OSInt ofstY, OSInt width, OSInt height, Int32 penWidth);
		static void PolyPolygonAccel(void *hdc, Int32 *points, UInt32 *pointCnt, UOSInt nPointCnt, OSInt ofstX, OSInt ofstY, OSInt width, OSInt height, Int32 penWidth);

		Bool IsOffScreen();
		Bool DrawRectN(OSInt x, OSInt y, OSInt w, OSInt h, DrawPen *p, DrawBrush *b);

		void *GetHDC();
		void SetImageAlpha(UInt8 alpha);
		void MulImageAlpha(Double val);
		void FillAlphaRect(OSInt left, OSInt top, OSInt width, OSInt height, UInt8 alpha);
	private:
		void *CreateGDIImage();
	};
}
#endif
