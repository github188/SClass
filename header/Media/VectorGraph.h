#ifndef _SM_MEDIA_VECTORGRAPH
#define _SM_MEDIA_VECTORGRAPH
#include "Data/ArrayList.h"
#include "Math/Vector2D.h"
#include "Math/Unit/Distance.h"
#include "Media/DrawEngine.h"

namespace Media
{
	class VectorGraph : public Media::DrawImage
	{
	private:
		class VectorPenStyle : public Media::DrawPen
		{
		private:
			OSInt index;
			Int32 color;
			Double thick;
			UInt8 *pattern;
			OSInt nPattern;

		public:
			VectorPenStyle(OSInt index, Int32 color, Double thick, UInt8 *pattern, OSInt nPattern);
			virtual ~VectorPenStyle();
			
			virtual Double GetThick();
			Bool IsSame(Int32 color, Double thick, UInt8 *pattern, OSInt nPattern);
			OSInt GetIndex();
			Media::DrawPen *CreateDrawPen(Double oriDPI, Media::DrawImage *dimg);
		};

		class VectorFontStyle : public Media::DrawFont
		{
		private:
			OSInt index;
			const WChar *name;
			Double height;
			Media::DrawEngine::DrawFontStyle fontStyle;
			Int32 codePage;

		public:
			VectorFontStyle(OSInt index, const WChar *name, Double height, Media::DrawEngine::DrawFontStyle fontStyle, Int32 codePage);
			virtual ~VectorFontStyle();
			
			Bool IsSame(const WChar *name, Double height, Media::DrawEngine::DrawFontStyle fontStyle, Int32 codePage);
			OSInt GetIndex();
			Double GetHeight();
			const WChar *GetName();
			Media::DrawEngine::DrawFontStyle GetStyle();
			Int32 GetCodePage();
			Media::DrawFont *CreateDrawFont(Double oriDPI, Media::DrawImage *dimg);
		};

		class VectorBrushStyle : public Media::DrawBrush
		{
		private:
			OSInt index;
			Int32 color;

		public:
			VectorBrushStyle(OSInt index, Int32 color);
			virtual ~VectorBrushStyle();

			Bool IsSame(Int32 color);
			OSInt GetIndex();
			Media::DrawBrush *CreateDrawBrush(Double oriDPI, Media::DrawImage *dimg);
		};

		typedef struct
		{
			VectorPenStyle *pen;
			VectorFontStyle *font;
			VectorBrushStyle *brush;
		} VectorStyles;

	private:
		Media::DrawEngine *refEng;
		Media::ColorProfile *colorProfile;
		Int32 srid;
		Double width;
		Double height;
		Math::Unit::Distance::DistanceUnit unit;
		Data::ArrayList<VectorPenStyle*> *penStyles;
		Data::ArrayList<VectorFontStyle*> *fontStyles;
		Data::ArrayList<VectorBrushStyle*> *brushStyles;

		Data::ArrayList<VectorStyles*> *itemStyle;
		Data::ArrayList<Math::Vector2D *> *items;
		Media::DrawEngine::DrawPos align;
		
	public:
		VectorGraph(Int32 srid, Double visibleWidth, Double visibleHeight, Math::Unit::Distance::DistanceUnit unit, Media::DrawEngine *refEng, Media::ColorProfile *colorProfile);
		virtual ~VectorGraph();

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

		virtual Bool DrawLine(Double x1, Double y1, Double x2, Double y2, DrawPen *p);
		virtual Bool DrawPolylineI(Int32 *points, UOSInt nPoints, DrawPen *p);
		virtual Bool DrawPolygonI(Int32 *points, UOSInt nPoints, DrawPen *p, DrawBrush *b);
		virtual Bool DrawPolyPolygonI(Int32 *points, UInt32 *pointCnt, UOSInt nPointCnt, DrawPen *p, DrawBrush *b);
		virtual Bool DrawPolyline(Double *points, UOSInt nPoints, DrawPen *p);
		virtual Bool DrawPolygon(Double *points, UOSInt nPoints, DrawPen *p, DrawBrush *b);
		virtual Bool DrawPolyPolygon(Double *points, UInt32 *pointCnt, UOSInt nPointCnt, DrawPen *p, DrawBrush *b);
		virtual Bool DrawRect(Double x, Double y, Double w, Double h, DrawPen *p, DrawBrush *b);
		virtual Bool DrawEllipse(Double tlx, Double tly, Double w, Double h, DrawPen *p, DrawBrush *b);
		virtual Bool DrawStringUTF8(Double tlx, Double tly, const UTF8Char *str, DrawFont *f, DrawBrush *b);
		virtual Bool DrawString(Double tlx, Double tly, const WChar *str, DrawFont *f, DrawBrush *p);
		virtual Bool DrawStringRotUTF8(Double centX, Double centY, const UTF8Char *str, DrawFont *f, DrawBrush *p, Double angleDegree);
		virtual Bool DrawStringRot(Double centX, Double centY, const WChar *str, DrawFont *f, DrawBrush *p, Double angleDegree);
		virtual Bool DrawStringBUTF8(Double tlx, Double tly, const UTF8Char *str, DrawFont *f, DrawBrush *p, OSInt buffSize);
		virtual Bool DrawStringB(Double tlx, Double tly, const WChar *str, DrawFont *f, DrawBrush *p, OSInt buffSize);
		virtual Bool DrawStringRotBUTF8(Double centX, Double centY, const UTF8Char *str, DrawFont *f, DrawBrush *p, Double angleDegree, OSInt buffSize);
		virtual Bool DrawStringRotB(Double centX, Double centY, const WChar *str, DrawFont *f, DrawBrush *p, Double angleDegree, OSInt buffSize);
		virtual Bool DrawImagePt(DrawImage *img, Double tlx, Double tly);
		virtual Bool DrawImagePt2(Media::StaticImage *img, Double tlx, Double tly);
		virtual Bool DrawImagePt3(DrawImage *img, Double destX, Double destY, Double srcX, Double srcY, Double srcW, Double srcH); /////////////////////////////

		virtual DrawPen *NewPenARGB(Int32 color, Double thick, UInt8 *pattern, OSInt nPattern);
		virtual DrawBrush *NewBrushARGB(Int32 color);
		virtual DrawFont *NewFontA(const Char *name, Int16 pxSize, Media::DrawEngine::DrawFontStyle fontStyle);
		virtual DrawFont *NewFontW(const WChar *name, Int16 pxSize, Media::DrawEngine::DrawFontStyle fontStyle);
		virtual DrawFont *NewFontHUTF8(const UTF8Char *name, Double height, Media::DrawEngine::DrawFontStyle fontStyle, Int32 codePage);
		virtual DrawFont *NewFontH(const WChar *name, Double height, Media::DrawEngine::DrawFontStyle fontStyle, Int32 codePage);
		virtual DrawFont *CloneFont(Media::DrawFont *f);
		virtual void DelPen(DrawPen *p);
		virtual void DelBrush(DrawBrush *b);
		virtual void DelFont(DrawFont *f);

		virtual Bool GetTextSizeUTF8(DrawFont *fnt, const UTF8Char *txt, OSInt txtLen, Double *sz);
		virtual Bool GetTextSize(DrawFont *fnt, const WChar *txt, OSInt txtLen, Double *sz);
		virtual void SetTextAlign(Media::DrawEngine::DrawPos pos);
		virtual void GetStringBoundW(Int32 *pos, OSInt centX, OSInt centY, const WChar *str, DrawFont *f, OSInt *drawX, OSInt *drawY);
		virtual void GetStringBoundRotW(Int32 *pos, Double centX, Double centY, const WChar *str, DrawFont *f, Double angleDegree, OSInt *drawX, OSInt *drawY);
		virtual void CopyBits(OSInt x, OSInt y, void *imgPtr, OSInt bpl, OSInt width, OSInt height);

		virtual Media::StaticImage *ToStaticImage();
		virtual Int32 SavePng(IO::SeekableStream *stm);
		virtual Int32 SaveGIF(IO::SeekableStream *stm);
		virtual Int32 SaveJPG(IO::SeekableStream *stm);

		Double GetVisibleWidthMM();
		Double GetVisibleHeightMM();
		void DrawTo(Media::DrawImage *dimg, Int32 *imgDurMS);
	};
}
#endif