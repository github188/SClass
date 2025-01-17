#ifndef _SM_SSWR_ORGANMGR_ORGANGROUPITEM
#define _SM_SSWR_ORGANMGR_ORGANGROUPITEM

namespace SSWR
{
	namespace OrganMgr
	{
		class OrganGroupItem
		{
		public:
			typedef enum
			{
				IT_GROUP,
				IT_SPECIES,
				IT_PARENT,
				IT_IMAGE
			} ItemType;

			virtual ~OrganGroupItem(){};

			virtual ItemType GetItemType() = 0;
			virtual UTF8Char *GetItemName(UTF8Char *buff) = 0;
			virtual UTF8Char *GetEngName(UTF8Char *buff) = 0;
			virtual OrganGroupItem *Clone() = 0;
		};
	}
}
#endif
