//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop
USEFORM("main.cpp", frmEnv);
USEFORM("pbar.cpp", frmPBar);
USEFORM("mdi.cpp", frmMDI);
USEFORM("properts.cpp", frmProperties);
USEFORM("config.cpp", frmConfig);
USEFORM("change.cpp", fmChange);
USEFORM("about.cpp", AboutBox);
//---------------------------------------------------------------------------
WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
        try
        {
                 Application->Initialize();
                 Application->Title = "Mapper";
                 Application->CreateForm(__classid(TfrmMDI), &frmMDI);
                 Application->CreateForm(__classid(TfrmProperties), &frmProperties);
                 Application->CreateForm(__classid(TfrmConfig), &frmConfig);
                 Application->CreateForm(__classid(TfmChange), &fmChange);
                 Application->Run();
        }
        catch (Exception &exception)
        {
                 Application->ShowException(&exception);
        }
        return 0;
}
//---------------------------------------------------------------------------




