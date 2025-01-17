#ifndef _SM_SSWR_ORGANMGR_ORGANWEBHANDLER
#define _SM_SSWR_ORGANMGR_ORGANWEBHANDLER
#include "Data/ArrayListDbl.h"
#include "Data/Int32Map.h"
#include "Data/Int64Map.h"
#include "Data/UInt32Map.h"
#include "Data/RandomOS.h"
#include "Data/StringUTF8Map.h"
#include "DB/DBTool.h"
#include "IO/ConfigFile.h"
#include "IO/LogTool.h"
#include "Media/CS/CSConverter.h"
#include "Media/Resizer/LanczosResizerLR_C32.h"
#include "Net/SocketFactory.h"
#include "Net/WebServer/MemoryWebSessionManager.h"
#include "Net/WebServer/WebListener.h"
#include "Net/WebServer/WebServiceHandler.h"
#include "Parser/ParserList.h"
#include "Sync/RWMutex.h"
#include "Text/Locale.h"

namespace SSWR
{
	namespace OrganMgr
	{
		class OrganWebHandler : public Net::WebServer::WebServiceHandler
		{
		private:
			typedef enum
			{
				POT_UNKNOWN,
				POT_USERFILE,
				POT_GROUP,
				POT_SPECIES
			} PickObjType;

			typedef enum
			{
				GF_NONE = 0,
				GF_ADMIN_ONLY = 1
			} GroupFlags;

			typedef enum
			{
				SF_NONE = 0,
				SF_HAS_MYPHOTO = 1,
				SF_HAS_WEBPHOTO = 8
			} SpeciesFlags;

			typedef struct
			{
				Int32 bookId;
				Int32 speciesId;
				const UTF8Char *dispName;
			} BookSpInfo;

			typedef struct
			{
				Int32 id;
				const UTF8Char *title;
				const UTF8Char *author;
				const UTF8Char *press;
				Int64 publishDate;
				const UTF8Char *url;

				Data::ArrayList<BookSpInfo*> *species;
			} BookInfo;

			typedef struct
			{
				Int32 id;
				Int32 parentId;
				const UTF8Char *cname;
				const UTF8Char *ename;
				Double lat;
				Double lon;
				Int32 cateId;
				Int32 locType;
			} LocationInfo;

			typedef struct
			{
				Int64 fromDate;
				Int64 toDate;
				Int32 locId;
				Int32 cateId;
			} TripInfo;
			
			typedef struct
			{
				Int32 id;
				Int32 fileType;
				const UTF8Char *oriFileName;
				Int64 fileTimeTicks;
				Double lat;
				Double lon;
				Int32 webuserId;
				Int32 speciesId;
				Int64 captureTimeTicks;
				const UTF8Char *dataFileName;
				UInt32 crcVal;
				Int32 rotType;
				Int32 prevUpdated;
				Double cropLeft;
				Double cropTop;
				Double cropRight;
				Double cropBottom;
				const UTF8Char *descript;
				const UTF8Char *location;
			} UserFileInfo;

			typedef struct
			{
				Int32 id;
				Int32 crcVal;
				const UTF8Char *imgUrl;
				const UTF8Char *srcUrl;
				const UTF8Char *location;
				Int32 prevUpdated;
				Double cropLeft;
				Double cropTop;
				Double cropRight;
				Double cropBottom;
			} WebFileInfo;

			typedef struct
			{
				Int32 id;
				const UTF8Char *userName;
				const UTF8Char *pwd;
				const UTF8Char *watermark;
				Int32 userType;
				Data::ArrayListInt64 *userFileIndex;
				Data::ArrayList<UserFileInfo*> *userFileObj;
				Data::Int32Map<Data::Int64Map<TripInfo*>*> *tripCates;
				Int32 unorganSpId;
			} WebUserInfo;

			typedef struct
			{
				Int32 speciesId;
				const UTF8Char *engName;
				const UTF8Char *chiName;
				const UTF8Char *sciName;
				Int32 groupId;
				const UTF8Char *descript;
				const UTF8Char *dirName;
				const UTF8Char *photo;
				const UTF8Char *idKey;
				Int32 cateId;
				SpeciesFlags flags;
				Int32 photoId;
				Int32 photoWId;

				Data::ArrayList<BookSpInfo*> *books;
				Data::ArrayList<UserFileInfo*> *files;
				Data::Int32Map<WebFileInfo*> *wfiles;
			} SpeciesInfo;

			struct GroupInfo
			{
				Int32 id;
				Int32 groupType;
				const UTF8Char *engName;
				const UTF8Char *chiName;
				const UTF8Char *descript;
				Int32 parentId;
				Int32 photoGroup;
				Int32 photoSpecies;
				const UTF8Char *idKey;
				Int32 cateId;
				GroupFlags flags;

				UOSInt photoCount;
				UOSInt myPhotoCount;
				UOSInt totalCount;
				SpeciesInfo *photoSpObj;
				Data::ArrayList<SpeciesInfo*> *species;
				Data::ArrayList<GroupInfo *> *groups;
			};

			typedef struct
			{
				Int32 id;
				const UTF8Char *chiName;
				const UTF8Char *engName;
			} GroupTypeInfo;

			typedef struct
			{
				Int32 cateId;
				const UTF8Char *chiName;
				const UTF8Char *dirName;
				const UTF8Char *srcDir;
				Int32 flags;
				Data::Int32Map<GroupTypeInfo *> *groupTypes;
				Data::ArrayList<GroupInfo*> *groups;
			} CategoryInfo;

			typedef struct
			{
				UInt32 scnWidth;
				Bool isMobile;
				WebUserInfo *user;
				Data::ArrayListInt32 *pickObjs;
				PickObjType pickObjType;
			} RequestEnv;

		private:
			Data::RandomOS *random;
			DB::DBTool *db;
			const UTF8Char *imageDir;
			const UTF8Char *cacheDir;
			const UTF8Char *dataDir;
			Int32 unorganizedGroupId;
			UInt32 scnSize;
			const UTF8Char *reloadPwd;
			Net::SocketFactory *sockf;
			IO::LogTool *log;
			Net::WebServer::WebListener *listener;
			Sync::Mutex *parserMut;
			Parser::ParserList *parsers;
			Sync::Mutex *resizerMut;
			Media::Resizer::LanczosResizerLR_C32 *resizerLR;
			Sync::Mutex *csconvMut;
			Media::CS::CSConverter *csconv;
			UInt32 csconvFCC;
			UInt32 csconvBpp;
			Media::PixelFormat csconvPF;
			Media::ColorProfile *csconvColor;

			Media::ColorManager *colorMgr;
			Media::ColorManagerSess *colorSess;
			Media::DrawEngine *eng;
			Net::WebServer::MemoryWebSessionManager *sessMgr;

			Data::StringUTF8Map<CategoryInfo*> *cateSMap;
			Data::Int32Map<CategoryInfo*> *cateMap;

			Sync::RWMutex *dataMut;
			Data::Int32Map<SpeciesInfo*> *spMap;
			Data::StringUTF8Map<SpeciesInfo*> *spNameMap;
			Data::Int32Map<GroupInfo*> *groupMap;
			Data::Int32Map<BookInfo*> *bookMap;
			Data::Int32Map<WebUserInfo*> *userMap;
			Data::StringUTF8Map<WebUserInfo*> *userNameMap;
			Data::Int32Map<UserFileInfo*> *userFileMap;
			Data::UInt32Map<IO::ConfigFile*> *langMap;
			Data::Int32Map<LocationInfo*> *locMap;
			Text::Locale *locale;

			void LoadLangs();
			void LoadCategory();
			void LoadSpecies();
			void LoadGroups();
			void LoadBooks();
			void LoadUsers();
			void LoadLocations();
			void FreeSpecies();
			void FreeGroups();
			void FreeGroup(GroupInfo *group);
			void FreeBooks();
			void FreeUsers();
			void ClearUsers();
			void UserFilePrevUpdated(UserFileInfo *userFile);
			void WebFilePrevUpdated(WebFileInfo *userFile);

			void CalcGroupCount(GroupInfo *group);
			void GetGroupSpecies(GroupInfo *group, Data::StringUTF8Map<SpeciesInfo*> *spMap, WebUserInfo *user);
			void SearchInGroup(GroupInfo *group, const UTF8Char *searchStr, Data::ArrayListDbl *speciesIndice, Data::ArrayList<SpeciesInfo*> *speciesObjs, Data::ArrayListDbl *groupIndice, Data::ArrayList<GroupInfo*> *groupObjs, WebUserInfo *user);
			Bool GroupIsAdmin(GroupInfo *group);
			UTF8Char *PasswordEnc(UTF8Char *buff, const UTF8Char *pwd);
			Bool BookFileExist(BookInfo *book);
			Bool UserGPSGetPos(Int32 userId, Data::DateTime *t, Double *lat, Double *lon);

			//LockWrite dataMut before calling
			Int32 SpeciesAdd(const UTF8Char *engName, const UTF8Char *chiName, const UTF8Char *sciName, Int32 groupId, const UTF8Char *description, const UTF8Char *dirName, const UTF8Char *idKey, Int32 cateId);
			Bool SpeciesSetPhotoId(Int32 speciesId, Int32 photoId);
			Bool SpeciesSetFlags(Int32 speciesId, SpeciesFlags flags);
			Bool SpeciesMove(Int32 speciesId, Int32 groupId, Int32 cateId);
			Bool SpeciesModify(Int32 speciesId, const UTF8Char *engName, const UTF8Char *chiName, const UTF8Char *sciName, const UTF8Char *description, const UTF8Char *dirName);
			Int32 UserfileAdd(Int32 userId, Int32 spId, const UTF8Char *fileName, const UInt8 *fileCont, UOSInt fileSize);
			Bool UserfileMove(Int32 userfileId, Int32 speciesId, Int32 cateId);
			Bool UserfileUpdateDesc(Int32 userfileId, const UTF8Char *descr);
			Bool UserfileUpdateRotType(Int32 userfileId, Int32 rotType);
			Bool SpeciesBookIsExist(const UTF8Char *speciesName, Text::StringBuilderUTF *bookNameOut);
			Int32 GroupAdd(const UTF8Char* engName, const UTF8Char *chiName, Int32 parentId, const UTF8Char *descr, Int32 groupTypeId, Int32 cateId, GroupFlags flags);
			Bool GroupModify(Int32 id, const UTF8Char *engName, const UTF8Char *chiName, const UTF8Char *descr, Int32 groupTypeId, GroupFlags flags);
			Bool GroupDelete(Int32 id);
			Bool GroupMove(Int32 groupId, Int32 destGroupId, Int32 cateId);
			Bool GroupAddCounts(Int32 groupId, UOSInt totalCount, UOSInt photoCount, UOSInt myPhotoCount);
			Bool GroupSetPhotoSpecies(Int32 groupId, Int32 photoSpeciesId);
			Bool GroupSetPhotoGroup(Int32 groupId, Int32 photoGroupId);

			Net::WebServer::IWebSession *ParseRequestEnv(Net::WebServer::IWebRequest *req, Net::WebServer::IWebResponse *resp, RequestEnv *env, Bool keepSess);
			static Bool __stdcall SvcPhoto(Net::WebServer::IWebRequest *req, Net::WebServer::IWebResponse *resp, const UTF8Char *subReq, Net::WebServer::WebServiceHandler *parent);
			static Bool __stdcall SvcPhotoDown(Net::WebServer::IWebRequest *req, Net::WebServer::IWebResponse *resp, const UTF8Char *subReq, Net::WebServer::WebServiceHandler *parent);
			static Bool __stdcall SvcGroup(Net::WebServer::IWebRequest *req, Net::WebServer::IWebResponse *resp, const UTF8Char *subReq, Net::WebServer::WebServiceHandler *parent);
			static Bool __stdcall SvcGroupMod(Net::WebServer::IWebRequest *req, Net::WebServer::IWebResponse *resp, const UTF8Char *subReq, Net::WebServer::WebServiceHandler *parent);
			static Bool __stdcall SvcSpecies(Net::WebServer::IWebRequest *req, Net::WebServer::IWebResponse *resp, const UTF8Char *subReq, Net::WebServer::WebServiceHandler *parent);
			static Bool __stdcall SvcSpeciesMod(Net::WebServer::IWebRequest *req, Net::WebServer::IWebResponse *resp, const UTF8Char *subReq, Net::WebServer::WebServiceHandler *parent);
			static Bool __stdcall SvcList(Net::WebServer::IWebRequest *req, Net::WebServer::IWebResponse *resp, const UTF8Char *subReq, Net::WebServer::WebServiceHandler *parent);
			static Bool __stdcall SvcPhotoDetail(Net::WebServer::IWebRequest *req, Net::WebServer::IWebResponse *resp, const UTF8Char *subReq, Net::WebServer::WebServiceHandler *parent);
			static Bool __stdcall SvcPhotoDetailD(Net::WebServer::IWebRequest *req, Net::WebServer::IWebResponse *resp, const UTF8Char *subReq, Net::WebServer::WebServiceHandler *parent);
			static Bool __stdcall SvcPhotoYear(Net::WebServer::IWebRequest *req, Net::WebServer::IWebResponse *resp, const UTF8Char *subReq, Net::WebServer::WebServiceHandler *parent);
			static Bool __stdcall SvcPhotoDay(Net::WebServer::IWebRequest *req, Net::WebServer::IWebResponse *resp, const UTF8Char *subReq, Net::WebServer::WebServiceHandler *parent);
			static Bool __stdcall SvcPhotoUpload(Net::WebServer::IWebRequest *req, Net::WebServer::IWebResponse *resp, const UTF8Char *subReq, Net::WebServer::WebServiceHandler *parent);
			static Bool __stdcall SvcSearchInside(Net::WebServer::IWebRequest *req, Net::WebServer::IWebResponse *resp, const UTF8Char *subReq, Net::WebServer::WebServiceHandler *parent);
			static Bool __stdcall SvcSearchInsideMoreS(Net::WebServer::IWebRequest *req, Net::WebServer::IWebResponse *resp, const UTF8Char *subReq, Net::WebServer::WebServiceHandler *parent);
			static Bool __stdcall SvcSearchInsideMoreG(Net::WebServer::IWebRequest *req, Net::WebServer::IWebResponse *resp, const UTF8Char *subReq, Net::WebServer::WebServiceHandler *parent);
			static Bool __stdcall SvcBookList(Net::WebServer::IWebRequest *req, Net::WebServer::IWebResponse *resp, const UTF8Char *subReq, Net::WebServer::WebServiceHandler *parent);
			static Bool __stdcall SvcBook(Net::WebServer::IWebRequest *req, Net::WebServer::IWebResponse *resp, const UTF8Char *subReq, Net::WebServer::WebServiceHandler *parent);
			static Bool __stdcall SvcBookView(Net::WebServer::IWebRequest *req, Net::WebServer::IWebResponse *resp, const UTF8Char *subReq, Net::WebServer::WebServiceHandler *parent);
			static Bool __stdcall SvcLogin(Net::WebServer::IWebRequest *req, Net::WebServer::IWebResponse *resp, const UTF8Char *subReq, Net::WebServer::WebServiceHandler *parent);
			static Bool __stdcall SvcLogout(Net::WebServer::IWebRequest *req, Net::WebServer::IWebResponse *resp, const UTF8Char *subReq, Net::WebServer::WebServiceHandler *parent);
			static Bool __stdcall SvcReload(Net::WebServer::IWebRequest *req, Net::WebServer::IWebResponse *resp, const UTF8Char *subReq, Net::WebServer::WebServiceHandler *parent);
			static Bool __stdcall SvcRestart(Net::WebServer::IWebRequest *req, Net::WebServer::IWebResponse *resp, const UTF8Char *subReq, Net::WebServer::WebServiceHandler *parent);
			static Bool __stdcall SvcIndex(Net::WebServer::IWebRequest *req, Net::WebServer::IWebResponse *resp, const UTF8Char *subReq, Net::WebServer::WebServiceHandler *parent);
			static Bool __stdcall SvcCate(Net::WebServer::IWebRequest *req, Net::WebServer::IWebResponse *resp, const UTF8Char *subReq, Net::WebServer::WebServiceHandler *parent);
			static Bool __stdcall SvcFavicon(Net::WebServer::IWebRequest *req, Net::WebServer::IWebResponse *resp, const UTF8Char *subReq, Net::WebServer::WebServiceHandler *parent);

			void ResponsePhoto(Net::WebServer::IWebRequest *req, Net::WebServer::IWebResponse *resp, WebUserInfo *user, Bool isMobile, Int32 speciesId, Int32 cateId, UInt32 imgWidth, UInt32 imgHeight, const UTF8Char *fileName);
			void ResponsePhotoId(Net::WebServer::IWebRequest *req, Net::WebServer::IWebResponse *resp, WebUserInfo *user, Bool isMobile, Int32 speciesId, Int32 cateId, UInt32 imgWidth, UInt32 imgHeight, Int32 photoId);
			void ResponsePhotoWId(Net::WebServer::IWebRequest *req, Net::WebServer::IWebResponse *resp, WebUserInfo *user, Bool isMobile, Int32 speciesId, Int32 cateId, UInt32 imgWidth, UInt32 imgHeight, Int32 photoWId);

			void WriteHeaderPart1(IO::Writer *writer, const UTF8Char *title, Bool isMobile);
			void WriteHeaderPart2(IO::Writer *writer, WebUserInfo *user, const UTF8Char *onLoadFunc);
			void WriteHeader(IO::Writer *writer, const UTF8Char *title, WebUserInfo *user, Bool isMobile);
			void WriteFooter(IO::Writer *writer);
			void WriteLocator(IO::Writer *writer, GroupInfo *group, CategoryInfo *cate);
			void WriteLocatorText(IO::Writer *writer, GroupInfo *group, CategoryInfo *cate);
			void WriteGroupTable(IO::Writer *writer, Data::ArrayList<GroupInfo *> *groupList, UInt32 scnWidth, Bool showSelect);
			void WriteSpeciesTable(IO::Writer *writer, Data::ArrayList<SpeciesInfo *> *spList, UInt32 scnWidth, Int32 cateId, Bool showSelect);
			void WritePickObjs(IO::Writer *writer, RequestEnv *env, const UTF8Char *url);

			static Bool __stdcall OnSessionDel(Net::WebServer::IWebSession* sess, void *userObj);
			static Bool __stdcall OnSessionCheck(Net::WebServer::IWebSession* sess, void *userObj);

			IO::ConfigFile *LangGet(Net::WebServer::IWebRequest *req);
			static const UTF8Char *LangGetValue(IO::ConfigFile *lang, const UTF8Char *name);
		public:
			OrganWebHandler(Net::SocketFactory *sockf, IO::LogTool *log, DB::DBTool *db, const UTF8Char *imageDir, UInt16 port, const UTF8Char *cacheDir, const UTF8Char *dataDir, UInt32 scnSize, const UTF8Char *reloadPwd, Int32 unorganizedGroupId, Media::DrawEngine *eng);
			virtual ~OrganWebHandler();

			Bool IsError();

//			virtual void WebRequest(Net::WebServer::IWebRequest *req, Net::WebServer::IWebResponse *resp);

			void Reload();
			void Restart();
		};
	}
}
#endif
