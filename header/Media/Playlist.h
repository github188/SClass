#ifndef _SM_MEDIA_PLAYLIST
#define _SM_MEDIA_PLAYLIST
#include "IO/ParsedObject.h"
#include "Media/IMediaPlayer.h"
#include "Media/MediaFile.h"
#include "Parser/ParserList.h"

namespace Media
{
	class Playlist : public IO::ParsedObject, public Media::IPBControl
	{
	private:
		typedef struct
		{
			const UTF8Char *title;
			const UTF8Char *artist;
			const UTF8Char *fileName;
			Int32 timeStart;
			Int32 timeEnd;
		} PlaylistEntry;

		Data::ArrayList<PlaylistEntry*> *entries;
		Parser::ParserList *parsers;
		Media::IMediaPlayer *player;
		Media::MediaFile *currFile;
		Bool playing;

		static void __stdcall OnPBEnd(void *userObj);
		void FreeEntry(PlaylistEntry* ent);
	public:
		Playlist(const UTF8Char *sourceName, Parser::ParserList *parsers);
		virtual ~Playlist();

		virtual IO::ParsedObject::ParserType GetParserType();
		Bool AddFile(const UTF8Char *fileName);
		Bool RemoveEntry(OSInt index);
		Bool AppendPlaylist(Media::Playlist *playlist);
		void ClearFiles();

		OSInt GetCount();
		const UTF8Char *GetTitle(OSInt index);
		const UTF8Char *GetArtist(OSInt index);
		const UTF8Char *GetFileName(OSInt index);
		Int32 GetTimeStart(OSInt index);
		Int32 GetTimeEnd(OSInt index);

		void SetPlayer(Media::IMediaPlayer *player);
		Bool OpenItem(OSInt index);
		
		virtual Bool IsPlaying();
		virtual Bool StartPlayback();
		virtual Bool StopPlayback();
		virtual Bool PrevChapter();
		virtual Bool NextChapter();
		virtual Int32 GetCurrTime();
	};
};
#endif