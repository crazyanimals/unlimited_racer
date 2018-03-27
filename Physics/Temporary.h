/******************************************************

	TEMPORARY FILE

	contains:

	constants - later they will be loaded
	from files

	structure - temporary structures for testing
	purposes only

*******************************************************/

typedef struct _tmpPhMap
{
	int data;
} CPhMap;

typedef struct _CARDATA
{
	float data[6];
} CARDATA;

template<class T> struct _object
{
	int ID;
	T * object;
};

typedef struct _keysData
{
	DWORD a[6];
} _KeysData;

