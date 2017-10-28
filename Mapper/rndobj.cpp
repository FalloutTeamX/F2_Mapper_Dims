//////////////////////////////////////////////////////////////////////
// CRandomObj Class
//////////////////////////////////////////////////////////////////////
#include <vcl.h>
#pragma hdrstop

#include <stdlib.h>
#include "rndobj.h"

#pragma package(smart_init)
//---------------------------------------------------------------------------
CRandomObj::CRandomObj(String pidLine)
{
    TStringList *list = new TStringList();
    list->DelimitedText = Trim(pidLine);
    list->Delimiter = ',';
    this->count = list->Count;

    objPid = new int[count];

    for (int n = 0; n < count; n++)
    {
       objPid[n] = StrToInt(list->Strings[n]);
    }

    delete list;
}
//---------------------------------------------------------------------------
int CRandomObj::GetObjectID()
{
   return objPid[random(count)];
}
//---------------------------------------------------------------------------
CRandomObj::~CRandomObj()
{
    delete[] objPid;
}
//---------------------------------------------------------------------------

