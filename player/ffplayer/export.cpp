#include "stdafx.h"

#include "Player.h"

int32_t WINAPI CreateObject(const GUID & guid, void** ppInterface)
{
    if (guid == IDL_IPLAYER)
    {
        auto p = new Player;
        *ppInterface = (void* )p;
    }

    return 0;
}