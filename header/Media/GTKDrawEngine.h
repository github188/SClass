#ifndef _SM_MEDIA_GDIENGINE
#define _SM_MEDIA_GDIENGINE
#include "Media/DrawEngine.h"
#include "Media/ABlend/AlphaBlend8_C8.h"

namespace Media
{
	class GTKImage;

	class GTKDrawEngine : public Media::DrawEngine
	{
	public:
		Media::ABlend::AlphaBlend8_C8 *iab;
	public:
		GTKDrawEngine();
		virtual ~GTKDrawEngine();

		virtual DrawImage *CreateImage32(UOSInt width, UOSInt height, Media::AlphaType atype);
		DrawImage *CreateImageScn(void *cr, OSInt left, OSInt top, OSInt right, OSInt bottom);
		virtual DrawImage *LoadImage(const UTF8Char *fileName);
		virtual DrawImage *LoadImageStream(IO::SeekableStream *stm);
		virtual DrawImage *ConvImage(Media::Image *img);
		virtual DrawImage *CloneImage(DrawImage *img);
		virtual Bool DeleteImage(DrawImage *img);
	};

	class GTKDrawFont : public DrawFont
	{
	private:
		const UTF8Char *fontName;
		Double fontHeight;
		OSInt fontSlant;
		OSInt fontWeight;
	public:
		GTKDrawFont(const UTF8Char *fontName, Double fontHeight, Media::DrawEngine::DrawFontStyle drawFontStyle);
		GTKDrawFont(const UTF8Char *fontName, Double fontHeight, OSInt fontSlant, OSInt fontWeight);
		~GTKDrawFont();

		void Init(void *cr);
		const UTF8Char *GetFontName();
		Double GetHeight();
		OSInt GetFontWeight();
		OSInt GetFontSlant();
	};

	class GTKDrawPen : public DrawPen
	{
	private:
		Int32 oriColor;
		Double rV;
		Double gV;
		Double bV;
		Double aV;
		Double thick;

	public:
		GTKDrawPen(Int32 oriColor, Double thick);
		virtual ~GTKDrawPen();

		virtual Double GetThick();

		void Init(void *cr);
	};

	class GTKDrawBrush : public DrawBrush
	{
	private:
		Int32 oriColor;
		Double rV;
		Double gV;
		Double bV;
		Double aV;

	public:
		GTKDrawBrush(Int32 oriColor);
		~GTKDrawBrush();

		void Init(void *cr);
		Int32 GetOriColor();
	};

	class GTKDrawImage : public DrawImage, public Image
	{
	private:
		GTKDrawEngine *eng;
		void *surface; //cairo_surface_t *
		void *cr; //cairo_t *
		OSInt left;
		OSInt top;

	public:
		GTKDrawImage(GTKDrawEngine *eng, void *surface, void *cr, OSInt left, OSInt top, OSInt width, OSInt height, Int32 bitCount, Media::AlphaType atype);
		virtual ~GTKDrawImage();

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
		virtual OSInt GetImgBpl();
		virtual Media::EXIFData *GetEXIF();
		virtual Media::PixelFormat GetPixelFormat();

		virtual Bool DrawLine(Double x1, Double y1, Double x2, Double y2, DrawPen *p); ////////////////////////////////////
		virtual Bool DrawPolylineI(Int32 *points, UOSInt nPoints, DrawPen *p); ////////////////////////////////////
		virtual Bool DrawPolygonI(Int32 *points, UOSInt nPoints, DrawPen *p, DrawBrush *b); ////////////////////////////////////
		virtual Bool DrawPolyPolygonI(Int32 *points, UInt32 *pointCnt, UOSInt nPointCnt, DrawPen *p, DrawBrush *b); ////////////////////////////////////
		virtual Bool DrawPolyline(Double *points, UOSInt nPoints, DrawPen *p); ////////////////////////////////////
		virtual Bool DrawPolygon(Double *points, UOSInt nPoints, DrawPen *p, DrawBrush *b); ////////////////////////////////////
		virtual Bool DrawPolyPolygon(Double *points, UInt32 *pointCnt, UOSInt nPointCnt, DrawPen *p, DrawBrush *b); ////////////////////////////////////
		virtual Bool DrawRect(Double x, Double y, Double w, Double h, DrawPen *p, DrawBrush *b);
		virtual Bool DrawEllipse(Double tlx, Double tly, Double w, Double h, DrawPen *p, DrawBrush *b); ////////////////////////////////////
		virtual Bool DrawStringUTF8(Double tlx, Double tly, const UTF8Char *str, DrawFont *f, DrawBrush *p); ////////////////////////////////////
		virtual Bool DrawString(Double tlx, Double tly, const WChar *str, DrawFont *f, DrawBrush *p); ////////////////////////////////////
		virtual Bool DrawStringRotUTF8(Double centX, Double centY, const UTF8Char *str, DrawFont *f, DrawBrush *p, Double angleDegree); ////////////////////////////////////
		virtual Bool DrawStringRot(Double centX, Double centY, const WChar *str, DrawFont *f, DrawBrush *p, Double angleDegree); ////////////////////////////////////
		virtual Bool DrawStringBUTF8(Double tlx, Double tly, const UTF8Char *str, DrawFont *f, DrawBrush *p, OSInt buffSize); ////////////////////////////////////
		virtual Bool DrawStringB(Double tlx, Double tly, const WChar *str, DrawFont *f, DrawBrush *p, OSInt buffSize); ////////////////////////////////////
		virtual Bool DrawStringRotBUTF8(Double centX, Double centY, const UTF8Char *str, DrawFont *f, DrawBrush *p, Double angleDegree, OSInt buffSize); ////////////////////////////////////
		virtual Bool DrawStringRotB(Double centX, Double centY, const WChar *str, DrawFont *f, DrawBrush *p, Double angleDegree, OSInt buffSize); ////////////////////////////////////
		virtual Bool DrawImagePt(DrawImage *img, Double tlx, Double tly); ////////////////////////////////////
		virtual Bool DrawImagePt2(Media::StaticImage *img, Double tlx, Double tly); ////////////////////////////////////
		virtual Bool DrawImagePt3(DrawImage *img, Double destX, Double destY, Double srcX, Double srcY, Double srcW, Double srcH); ////////////////////////////////////

		virtual DrawPen *NewPenARGB(Int32 color, Double thick, UInt8 *pattern, OSInt nPattern);
		virtual DrawBrush *NewBrushARGB(Int32 color);
		virtual DrawFont *NewFontA(const Char *name, Int16 pxSize, Media::DrawEngine::DrawFontStyle fontStyle);
		virtual DrawFont *NewFontW(const WChar *name, Int16 pxSize, Media::DrawEngine::DrawFontStyle fontStyle);
		virtual DrawFont *NewFontHUTF8(const UTF8Char *name, Double height, Media::DrawEngine::DrawFontStyle fontStyle, Int32 codePage);
		virtual DrawFont *NewFontH(const WChar *name, Double height, Media::DrawEngine::DrawFontStyle fontStyle, Int32 codePage);
		virtual DrawFont *CloneFont(DrawFont *f);
		virtual void DelPen(DrawPen *p);
		virtual void DelBrush(DrawBrush *b);
		virtual void DelFont(DrawFont *f);

		virtual Bool GetTextSizeUTF8(DrawFont *fnt, const UTF8Char *txt, OSInt txtLen, Double *sz); ////////////////////////////////////
		virtual Bool GetTextSize(DrawFont *fnt, const WChar *txt, OSInt txtLen, Double *sz); ////////////////////////////////////
		virtual void SetTextAlign(DrawEngine::DrawPos pos); ////////////////////////////////////
		virtual void GetStringBoundW(Int32 *pos, OSInt centX, OSInt centY, const WChar *str, DrawFont *f, OSInt *drawX, OSInt *drawY); ////////////////////////////////////
		virtual void GetStringBoundRotW(Int32 *pos, Double centX, Double centY, const WChar *str, DrawFont *f, Double angleDegree, OSInt *drawX, OSInt *drawY); ////////////////////////////////////
		virtual void CopyBits(OSInt x, OSInt y, void *imgPtr, OSInt bpl, OSInt width, OSInt height); ////////////////////////////////////

		virtual Media::StaticImage *ToStaticImage(); ////////////////////////////////////
		virtual Int32 SavePng(IO::SeekableStream *stm);
		virtual Int32 SaveGIF(IO::SeekableStream *stm); ////////////////////////////////////
		virtual Int32 SaveJPG(IO::SeekableStream *stm);

		virtual Media::Image *Clone(); ////////////////////////////////////
		virtual Media::Image::ImageType GetImageType();
		virtual void GetImageData(UInt8 *destBuff, OSInt left, OSInt top, OSInt width, OSInt height, OSInt destBpl); ////////////////////////////////////
		virtual Int32 GetPixel32(OSInt x, OSInt y); ////////////////////////////////////

		void *GetSurface();
		void *GetCairo();
	};
}
#endif