//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "info.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TfrmInfo *frmInfo;
//---------------------------------------------------------------------------
__fastcall TfrmInfo::TfrmInfo(TComponent* Owner)
   : TForm(Owner)
{
}
//---------------------------------------------------------------------------
void TfrmInfo::InfoShow(String info)
{
   this->Memo->Text = info;
   this->Show();
}