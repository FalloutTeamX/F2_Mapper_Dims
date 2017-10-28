//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "change.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TfmChange *fmChange;
//---------------------------------------------------------------------------
__fastcall TfmChange::TfmChange(TComponent* Owner)
        : TForm(Owner)
{
   nOldValue = 1; 
}
//---------------------------------------------------------------------------
DWORD TfmChange::ChangeValue(DWORD nValueToChange)
{
   nOldValue = nValueToChange;   //nCurrentValue
   edValue->Text = nValueToChange;
   nCurrentValue = nValueToChange;
   UpDown1->Position = nCurrentValue;

   int result = this->ShowModal();
   return result == mrOk ? nCurrentValue : 0;
}
//---------------------------------------------------------------------------
void __fastcall TfmChange::FormCloseQuery(TObject *Sender, bool &CanClose)
{
   CanClose = true;
   try
   {
      nCurrentValue = edValue->Text.ToInt();
   }
   catch(Exception &exception)
   {
      nCurrentValue = nOldValue;
      edValue->Text = nOldValue;
      UpDown1->Position = nCurrentValue;
      CanClose = false;
   }
}
//---------------------------------------------------------------------------

