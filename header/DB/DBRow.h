#ifndef _SM_DB_DBROW
#define _SM_DB_DBROW
#include "Data/DateTime.h"
#include "Data/StringUTF8Map.h"
#include "DB/DBReader.h"
#include "DB/TableDef.h"

namespace DB
{
	class DBRow
	{
	public:
		typedef enum
		{
			DT_UNKNOWN,
			DT_INT64,
			DT_DATETIME,
			DT_STRING,
			DT_DOUBLE,
			DT_VECTOR,
			DT_BINARY
		} DataType;

	private:
		union FieldData
		{
			Int64 iVal;
			Data::DateTime *dt;
			const UTF8Char *str;
			Double dVal;
			Math::Vector2D *vec;
			UInt8 *bin;
		};

		struct Field
		{
			ColDef *def;
			Bool committedNull;
			FieldData committedData;
			Bool currentChanged;
			Bool currentNull;
			FieldData currentData;
		};
	private:
		TableDef *table;
		Data::StringUTF8Map<Field*> *dataMap;

		void FreeField(Field *field);
		DataType GetDataType(Field *field);

		Bool SetFieldNull(Field *field);
		Bool SetFieldStr(Field *field, const UTF8Char *strValue);
		Bool SetFieldInt64(Field *field, Int64 intValue);
		Bool SetFieldDouble(Field *field, Double dblValue);
		Bool SetFieldDate(Field *field, Data::DateTime *dt);
		Bool SetFieldVector(Field *field, Math::Vector2D *vec);
		Bool SetFieldBinary(Field *field, const UInt8 *buff, UOSInt buffSize);

		Bool IsFieldNull(Field *field);
		const UTF8Char *GetFieldStr(Field *field);
		Int64 GetFieldInt64(Field *field);
		Double GetFieldDouble(Field *field);
		Data::DateTime *GetFieldDate(Field *field);
		Math::Vector2D *GetFieldVector(Field *field);
		const UInt8 *GetFieldBinary(Field *field, UOSInt *buffSize);

	public:
		DBRow(TableDef *table);
		~DBRow();

		Bool SetByReader(DB::DBReader *r, Bool commit);

		DB::ColDef *GetFieldType(const UTF8Char *fieldName);
		DataType GetFieldDataType(const UTF8Char *fieldName);
		Bool SetValueNull(const UTF8Char *fieldName);
		Bool SetValueStr(const UTF8Char *fieldName, const UTF8Char *strValue);
		Bool SetValueInt64(const UTF8Char *fieldName, Int64 intValue);
		Bool SetValueDouble(const UTF8Char *fieldName, Double dblValue);
		Bool SetValueDate(const UTF8Char *fieldName, Data::DateTime *dt);
		Bool SetValueVector(const UTF8Char *fieldName, Math::Vector2D *vec);
		Bool SetValueBinary(const UTF8Char *fieldName, const UInt8 *buff, UOSInt buffSize);

		Bool IsNull(const UTF8Char *fieldName);
		const UTF8Char *GetValueStr(const UTF8Char *fieldName);
		Int64 GetValueInt64(const UTF8Char *fieldName);
		Double GetValueDouble(const UTF8Char *fieldName);
		Data::DateTime *GetValueDate(const UTF8Char *fieldName);
		Math::Vector2D *GetValueVector(const UTF8Char *fieldName);
		const UInt8 *GetValueBinary(const UTF8Char *fieldName, UOSInt *buffSize);

		void Commit();
		void Rollback();

		Bool GetSinglePKI64(Int64 *key);
		void ToString(Text::StringBuilderUTF *sb);
		void AppendTableName(Text::StringBuilderUTF *sb);
		void AppendVarNameForm(Text::StringBuilderUTF *sb, const UTF8Char *colName);

		TableDef *GetTableDef();
	};
}
#endif
