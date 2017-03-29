//---------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include "about.h"
//---------------------------------------------------------------------
#pragma resource "*.dfm"
TAboutBox *AboutBox;
//---------------------------------------------------------------------
__fastcall TAboutBox::TAboutBox(TComponent* AOwner)
	: TForm(AOwner)
{
    DWORD tempHandle = 0;
    DWORD infoLen;
    VS_FIXEDFILEINFO* fileInfo;
    int Major, Minor, Release, Build;
    Major = Minor = Release = Build = 0;
    infoLen = GetFileVersionInfoSize(ParamStr(0).c_str(), &tempHandle);

    if (infoLen > 0)
    {
        char* pBuf = (char *) malloc(infoLen);
        unsigned int valueLen;

        GetFileVersionInfo(ParamStr(0).c_str(), 0, infoLen, pBuf);

        if (VerQueryValue(pBuf, "\\", (void **)&fileInfo, &valueLen))
        {
            Major =   (int)((fileInfo->dwFileVersionMS >> 16) & 0xffff);
            Minor =   (int)((fileInfo->dwFileVersionMS )      & 0xffff);
            Release = (int)((fileInfo->dwFileVersionLS >> 16) & 0xffff);
            Build =   (int)((fileInfo->dwFileVersionLS )      & 0xffff);
        }
    free(pBuf);
    }
    this->ProductName->Caption = "Dims Mapper2 v. "
        + IntToStr(Major) + "."
        + IntToStr(Minor) + "."
        + IntToStr(Release) + "."
        + IntToStr(Build) + ".";
}
//---------------------------------------------------------------------

