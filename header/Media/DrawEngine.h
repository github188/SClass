#ifndef _SM_MEDIA_DRAWENGINE
#define _SM_MEDIA_DRAWENGINE
#include "IO/SeekableStream.h"
#include "Media/Image.h"

namespace Media
{
	class DrawBrush
	{
	protected:
		DrawBrush()
		{
		}
	};

	class DrawPen
	{
	protected:
		DrawPen()
		{
		}

	public:
		virtual Double GetThick() = 0;
	};

	class DrawFont
	{
	protected:
		DrawFont()
		{
		}
	};

	class DrawImage;

	class DrawEngine
	{
	public:
		typedef enum
		{
			DRAW_POS_TOPLEFT,
			DRAW_POS_TOPCENTER,
			DRAW_POS_TOPRIGHT,
			DRAW_POS_CENTERLEFT,
			DRAW_POS_CENTER,
			DRAW_POS_CENTERRIGHT,
			DRAW_POS_BOTTOMLEFT,
			DRAW_POS_BOTTOMCENTER,
			DRAW_POS_BOTTOMRIGHT
		} DrawPos;

		typedef enum
		{
			DFS_NORMAL = 0,
			DFS_BOLD = 1,
			DFS_ITALIC = 2,
			DFS_ANTIALIAS = 8
		} DrawFontStyle;

	public:
		virtual ~DrawEngine(){};

		virtual DrawImage *CreateImage32(UOSInt width, UOSInt height, Media::AlphaType atype) = 0;
		virtual DrawImage *LoadImage(const UTF8Char *fileName) = 0;
		virtual DrawImage *LoadImageStream(IO::SeekableStream *stm) = 0;
		virtual DrawImage *ConvImage(Media::Image *img) = 0;
		virtual DrawImage *CloneImage(DrawImage *img) = 0;
		virtual Bool DeleteImage(DrawImage *img) = 0;
	};

	class DrawImage
	{
	public:
		virtual UOSInt GetWidth() = 0;
		virtual UOSInt GetHeight() = 0;
		virtual UInt32 GetBitCount() = 0;
		virtual ColorProfile *GetColorProfile() = 0;
		virtual void SetColorProfile(const ColorProfile *color) = 0;
		virtual Media::AlphaType GetAlphaType() = 0;
		virtual void SetAlphaType(Media::AlphaType atype) = 0;
		virtual Double GetHDPI() = 0;
		virtual Double GetVDPI() = 0;
		virtual void SetHDPI(Double dpi) = 0;
		virtual void SetVDPI(Double dpi) = 0;
		virtual UInt8 *GetImgBits(Bool *revOrder) = 0;
		virtual void GetImgBitsEnd(Bool modified) = 0;
		virtual OSInt GetImgBpl() = 0;
		virtual Media::EXIFData *GetEXIF() = 0;
		virtual Media::PixelFormat GetPixelFormat() = 0;

		virtual Bool DrawLine(Double x1, Double y1, Double x2, Double y2, DrawPen *p) = 0;
		virtual Bool DrawPolylineI(Int32 *points, UOSInt nPoints, DrawPen *p) = 0;
		virtual Bool DrawPolygonI(Int32 *points, UOSInt nPoints, DrawPen *p, DrawBrush *b) = 0;
		virtual Bool DrawPolyPolygonI(Int32 *points, UInt32 *pointCnt, UOSInt nPointCnt, DrawPen *p, DrawBrush *b) = 0;
		virtual Bool DrawPolyline(Double *points, UOSInt nPoints, DrawPen *p) = 0;
		virtual Bool DrawPolygon(Double *points, UOSInt nPoints, DrawPen *p, DrawBrush *b) = 0;
		virtual Bool DrawPolyPolygon(Double *points, UInt32 *pointCnt, UOSInt nPointCnt, DrawPen *p, DrawBrush *b) = 0;
		virtual Bool DrawRect(Double x, Double y, Double w, Double h, DrawPen *p, DrawBrush *b) = 0;
		virtual Bool DrawEllipse(Double tlx, Double tly, Double w, Double h, DrawPen *p, DrawBrush *b) = 0;
		virtual Bool DrawStringUTF8(Double tlx, Double tly, const UTF8Char *str, DrawFont *f, DrawBrush *b) = 0;
		virtual Bool DrawString(Double tlx, Double tly, const WChar *str, DrawFont *f, DrawBrush *b) = 0;
		virtual Bool DrawStringRotUTF8(Double centX, Double centY, const UTF8Char *str, DrawFont *f, DrawBrush *b, Double angleDegree) = 0;
		virtual Bool DrawStringRot(Double centX, Double centY, const WChar *str, DrawFont *f, DrawBrush *b, Double angleDegree) = 0;
		virtual Bool DrawStringBUTF8(Double tlx, Double tly, const UTF8Char *str, DrawFont *f, DrawBrush *b, OSInt buffSize) = 0;
		virtual Bool DrawStringB(Double tlx, Double tly, const WChar *str, DrawFont *f, DrawBrush *b, OSInt buffSize) = 0;
		virtual Bool DrawStringRotBUTF8(Double centX, Double centY, const UTF8Char *str, DrawFont *f, DrawBrush *b, Double angleDegree, OSInt buffSize) = 0;
		virtual Bool DrawStringRotB(Double centX, Double centY, const WChar *str, DrawFont *f, DrawBrush *b, Double angleDegree, OSInt buffSize) = 0;
		virtual Bool DrawImagePt(DrawImage *img, Double tlx, Double tly) = 0;
		virtual Bool DrawImagePt2(Media::StaticImage *img, Double tlx, Double tly) = 0;
		virtual Bool DrawImagePt3(DrawImage *img, Double destX, Double destY, Double srcX, Double srcY, Double srcW, Double srcH) = 0;

		virtual DrawPen *NewPenARGB(Int32 color, Double thick, UInt8 *pattern, OSInt nPattern) = 0;
		virtual DrawBrush *NewBrushARGB(Int32 color) = 0;
		virtual DrawFont *NewFontA(const Char *name, Int16 pxSize, Media::DrawEngine::DrawFontStyle fontStyle) = 0;
		virtual DrawFont *NewFontW(const WChar *name, Int16 pxSize, Media::DrawEngine::DrawFontStyle fontStyle) = 0;
		virtual DrawFont *NewFontHUTF8(const UTF8Char *name, Double height, Media::DrawEngine::DrawFontStyle fontStyle, Int32 codePage) = 0;
		virtual DrawFont *NewFontH(const WChar *name, Double height, Media::DrawEngine::DrawFontStyle fontStyle, Int32 codePage) = 0;
		virtual DrawFont *CloneFont(DrawFont *f) = 0;
		virtual void DelPen(DrawPen *p) = 0;
		virtual void DelBrush(DrawBrush *b) = 0;
		virtual void DelFont(DrawFont *f) = 0;

		virtual Bool GetTextSizeUTF8(DrawFont *fnt, const UTF8Char *txt, OSInt txtLen, Double *sz) = 0;
		virtual Bool GetTextSize(DrawFont *fnt, const WChar *txt, OSInt txtLen, Double *sz) = 0;
		virtual void SetTextAlign(Media::DrawEngine::DrawPos pos) = 0;
		virtual void GetStringBoundW(Int32 *pos, OSInt centX, OSInt centY, const WChar *str, DrawFont *f, OSInt *drawX, OSInt *drawY) = 0;
		virtual void GetStringBoundRotW(Int32 *pos, Double centX, Double centY, const WChar *str, DrawFont *f, Double angleDegree, OSInt *drawX, OSInt *drawY) = 0;
		virtual void CopyBits(OSInt x, OSInt y, void *imgPtr, OSInt bpl, OSInt width, OSInt height) = 0;

		virtual Media::StaticImage *ToStaticImage() = 0;
		virtual Int32 SavePng(IO::SeekableStream *stm) = 0;
		virtual Int32 SaveGIF(IO::SeekableStream *stm) = 0;
		virtual Int32 SaveJPG(IO::SeekableStream *stm) = 0;
	};
};
#endif