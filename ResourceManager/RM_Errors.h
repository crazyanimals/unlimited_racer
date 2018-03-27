#pragma once

#include "..\Globals\ErrorHandler.h"

#define BT_OK                       ERRNOERROR
#define BT_NODE_CREATION_FAILED     ERROUTOFMEMORY
#define BT_STRANGE_CREATION_ERROR   ERRGENERIC
#define BT_NODE_NOT_FOUND           ERRNOTFOUND
#define RM_OK                       ERRNOERROR
#define RM_RESOURCE_NOT_FOUND       ERRNOTFOUND
#define RM_INVALID_RESOURCE_TYPE    ERRINVALIDRESOURCETYPE
#define RC_OK                       ERRNOERROR

#define HE(code, location, param) { ErrorHandler.HandleError(code, location, param); return code; }