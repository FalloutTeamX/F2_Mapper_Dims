//---------------------------------------------------------------------------
#ifndef infoH
#define infoH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
//---------------------------------------------------------------------------
class TfrmInfo : public TForm
{
__published:	// IDE-managed Components
   TMemo *Memo;
private:	// User declarations
public:		// User declarations
   __fastcall TfrmInfo(TComponent* Owner);
   void InfoShow(String info);
};
//---------------------------------------------------------------------------
extern PACKAGE TfrmInfo *frmInfo;
//---------------------------------------------------------------------------
#endif
