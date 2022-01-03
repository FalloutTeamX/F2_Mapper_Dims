//////////////////////////////////////////////////////////////////////
// CFrmSet Class
//////////////////////////////////////////////////////////////////////

#include "frmset.h"
#include "mdi.h"
#include "frame.h"
#include "main.h"
#include "utilites.h"
#include "log.h"
#include "lists.h"
#include "macros.h"
//---------------------------------------------------------------------------
CFrmSet::CFrmSet(void)
{
   lError = true;
   pUtil = frmMDI->pUtil;
   pLog = frmMDI->pLog;
   for (int i = 0; i < 8; i++)
   {
      if (i == critter_ID)
         pFRM[i] = new CFrame[(frmMDI->pLstFiles->pFRMlst[i]->Count + 1) * 216];
      else
         pFRM[i] = new CFrame[frmMDI->pLstFiles->pFRMlst[i]->Count + 1];
   }
   ClearLocals();
   lError = false;
}
//---------------------------------------------------------------------------
void CFrmSet::ClearLocals(void)
{
   int i, j;
   for (i = 0; i < 7; i++)
   {
      for (j = 0; j < frmMDI->pLstFiles->pFRMlst[i]->Count + 1; j++)
      {
         pFRM[i][j].bLocal = false;
      }
      nFrmCount[i] = 0;
   }
}
//---------------------------------------------------------------------------
void CFrmSet::LoadLocalFRMs(void)
{
   pObjSet = frmEnv->pObjSet;
   pLstFiles = frmMDI->pLstFiles;
   int x, y;
   DWORD nDir;
   WORD nFrmID, nProID;
   BYTE nObjType;
   frmMDI->iPos = 0;
   BYTE *pObj, *pObjChild;

//-------------------LOAD INTERFACE--------------------
   LoadFRM("exitgrid.frm", intrface_ID, EG_blockID, true);
   LoadFRM("saiblock.frm", intrface_ID, SAI_blockID, true);
   LoadFRM("wallblockF.frm", intrface_ID, wall_see_blockID, true);      //flags thru
   LoadFRM("objblock.frm", intrface_ID, obj_blockID, true);
   LoadFRM("light.frm", intrface_ID, light_blockID, true);
   LoadFRM("scrblock.frm", intrface_ID, scroll_blockID, true);
   LoadFRM("selfblock.frm", intrface_ID, obj_self_blockID, true);           //"art\\intrface\\msef002.frm"
   LoadFRM("wallblock.frm", intrface_ID, wall_blockID, true);
   LoadFRM("objblockF.frm", intrface_ID, obj_see_blockID, true);        //flags thru
   LoadFRM("selfblockF.frm", intrface_ID, obj_thru_blockID, true);      //flags thru
   LoadFRM("HEX.frm", intrface_ID, hex_ID, true);
   LoadFRM("objhex.frm", intrface_ID, obj_hexID, true);
   LoadFRM("cursorhex.frm", intrface_ID, cursor_hexID, true);

//-------------------LOAD LOCAL TILES--------------------
   for (y = 0; y < frmEnv->pMap->TilesSizeY; y++)
   {
      for (x = 0; x < frmEnv->pMap->TilesSizeX; x++)
      {
         nFrmID = frmEnv->pTileSet->GetFloorID(x, y);
         //nFrmID &= 0x0FFF;   //��������� ����������� �� 4095
         LoadFRM(pUtil->GetFRMFileName(tile_ID,
                          pLstFiles->pFRMlst[tile_ID]->Strings[nFrmID]),
                                                         tile_ID, nFrmID, true);
         nFrmID = frmEnv->pTileSet->GetRoofID(x, y);
         //nFrmID &= 0x0FFF;   //��������� ����������� �� 4095
         LoadFRM(pUtil->GetFRMFileName(tile_ID,
                          pLstFiles->pFRMlst[tile_ID]->Strings[nFrmID]),
                                                         tile_ID, nFrmID, true);
         frmMDI->iPos++;
         Application->ProcessMessages();
      }
   }
//-------------------LOAD LOCAL OBJECTS--------------------
   DWORD nObjNum, nLevel, nChildCount = 0;
   String filename;

   nLevel = 0;
   do
   {
      pObj = pObjSet->GetFirstObj(&nObjNum, nLevel);
      while (pObj)
      {
         pObjSet->GetObjFrm(&nObjType, &nFrmID);
         filename = pUtil->GetFRMFileName(nObjType,
                                 pLstFiles->pFRMlst[nObjType]->Strings[nFrmID]);
         if (nObjType == critter_ID) // Critters type
               GetCritterFName(&filename, pObjSet->GetObjFrmDW(), &nFrmID);
         LoadFRM(filename, nObjType, nFrmID, true);
         frmMDI->iPos++;
         Application->ProcessMessages();
         pObj = pObjSet->GetNextObj(&nObjNum, &nChildCount, nLevel);
      }
   } while (++nLevel < frmEnv->pMap->Levels);
}
//---------------------------------------------------------------------------
void CFrmSet::LoadFRM(String sFile, BYTE nFrmType, WORD nFrmID, bool bLocal)
{
   //�������� ��������� �� FRM, ���� ����� ������ ������
   CFrame *l_pFRM = &pFRM[nFrmType][nFrmID];
   //����������� ������� FRM ���� ������ FRM �� ���, �� ����� ���������
   nFrmCount[nFrmType] += !l_pFRM->bLocal && bLocal ? 1 : 0;
   //��������� ��� �� FRM ���������, ��� ������������� ������ ��� ���������
   l_pFRM->bLocal = l_pFRM->bLocal ? l_pFRM->bLocal : bLocal;
   //��������� FRM �� ����������, ���� ��� ���������� �� ����� �� �-���
   if (l_pFRM->GetBMP()) return;

   ULONG i; // ������������ ��� �������� ��������
   DWORD a_width, bmpsize, frmsize, dwPtr, dwFramesBufSize, dwPixDataSize;
   WORD nFrames, width, height;

   // ������� ������� FRM ���� ��� ������
   HANDLE h_frm = pUtil->OpenFileX(sFile);
   // ���� �������� ������, �� ����� �� �-���
   if (h_frm == INVALID_HANDLE_VALUE)
   {
      pLog->LogX("Cannot open file \"" + sFile + "\"");
      return;
   }
   //������ �������� ��������� �� �������� 0x08 �� ������ �����
   pUtil->SetFilePointerX(h_frm, 0x08, FILE_BEGIN);
   //��������� 2 ����� (���-�� ������ ��� ������ �����������(����� ���� = 1))
   pUtil->ReadFileX(h_frm, &nFrames, 2, &i);
   l_pFRM->nFrames = pUtil->GetW(&nFrames);
   //��������� 12 ���� (WORD * 6 : �������� �� � ������������ ����������� �����)
   pUtil->ReadFileX(h_frm, &doffX, 12, &i);
   //��������� 12 ���� (WORD * 6 : �������� �� � ������������ ����������� �����)
   pUtil->ReadFileX(h_frm, &doffY, 12, &i);
   //��������� 24 ����� (DWORD * 6 : �������� � ����� �� ������ �������)
   pUtil->ReadFileX(h_frm, &dwDirectionOffset, 24, &i);
   //��������� 4 ����� (DWORD ������ ������� �������)
   pUtil->ReadFileX(h_frm, &dwFramesBufSize, 4, &i);
   pUtil->ReverseDW(&dwFramesBufSize);
   //������� ����� � ��������� ���� ������� �������, ��� ������ ����� �� �-���
   BYTE *framesdata;
   if ((framesdata = (BYTE *)malloc(dwFramesBufSize)) == NULL)
      return;
   pUtil->ReadFileX(h_frm, framesdata, dwFramesBufSize, &i);
   //��������� ���� �� �������������
   pUtil->CloseHandleX(h_frm);
   //�������, ������� �������� ����������� ����� FRM ����
   l_pFRM->nDirTotal = 1 + (pUtil->GetDW(&dwDirectionOffset[1]) != 0) +
                           (pUtil->GetDW(&dwDirectionOffset[2]) != 0) +
                           (pUtil->GetDW(&dwDirectionOffset[3]) != 0) +
                           (pUtil->GetDW(&dwDirectionOffset[4]) != 0) +
                           (pUtil->GetDW(&dwDirectionOffset[5]) != 0);
   //���������� �������� ��� ������
   l_pFRM->PrepareFrames();
   WORD HeightMAX = 0;
   WORD HeightSpr = 0;
   WORD WidthSpr = 0;
   WORD WidthSprMAX = 0;
   //�������� ���� �����������
   for (int nDir = 0; nDir < l_pFRM->nDirTotal; nDir++)
   {
      //�������� �������� ��� ������� �����������
      dwPtr = pUtil->GetDW(&dwDirectionOffset[nDir]);
      //�������� �������� ����� �� � � � ��� ������� �����������
      l_pFRM->doffX[nDir] = pUtil->GetW((WORD *)&doffX[nDir]);
      l_pFRM->doffY[nDir] = pUtil->GetW((WORD *)&doffY[nDir]);

      //�������� ���� ������
      for (int nFrame = 0; nFrame < l_pFRM->nFrames; nFrame++)
      {
         //� ��������� ���������� ����� � �������
         l_pFRM->sprX[nDir][nFrame] = WidthSpr;
         //� ��������� ���������� ����� � �������
         l_pFRM->sprY[nDir][nFrame] = HeightSpr;
         //�������� ������ �����
         width = pUtil->GetW((WORD *)(framesdata + dwPtr));
         l_pFRM->width[nDir][nFrame] = width;
         //���������� ������ ������
         WidthSpr += width;
         //�������� ������ �����
         height = pUtil->GetW((WORD *)(framesdata + dwPtr + 2));
         l_pFRM->height[nDir][nFrame] = height;
         //����������� ������������ ������ ������ �� ������ �����������
         HeightMAX = max(HeightMAX, height);
         //�������� ������ ������� � ��������� ��� ������� �����
         dwPixDataSize = pUtil->GetDW((DWORD *)(framesdata + dwPtr + 4));
         //�������� �������� �� � ������������ ����������� �����
         l_pFRM->foffX[nDir][nFrame] =
                                  pUtil->GetW((WORD *)(framesdata + dwPtr + 8));
         //�������� �������� �� � ������������ ����������� �����
         l_pFRM->foffY[nDir][nFrame] =
                                 pUtil->GetW((WORD *)(framesdata + dwPtr + 10));
         try { //��������� ���� � �������������� � HBITMAP
         l_pFRM->LoadData(nDir, nFrame, framesdata + dwPtr + 12);   //TODO: ��� ��� ������ ��� ��� � ������� LoadData
         } catch (Exception &exception) {
            break;
         }
         //����������� �������� �� ������ ����� ��� ��������� ����������
         dwPtr += (12 + dwPixDataSize);
      }
      //���������� ������ ��� ������� �����������
      HeightSpr += HeightMAX;
      //������� ������������ ������ ����� ������ �����������
      WidthSprMAX = max(WidthSprMAX, WidthSpr);
      //�������� ����������
      WidthSpr = 0;
      HeightMAX = 0;
   }
   l_pFRM->BuildFrames(WidthSprMAX, HeightSpr);
   l_pFRM->FileName = ExtractFileName(sFile);
   if (l_pFRM->pBMP)
   {
      pLog->LogX("Load FRM file \"" + sFile +
                 "\" directions: " + String(l_pFRM->nDirTotal) +
                 " frames: " + String(l_pFRM->nFrames));
   }
   else
   {
      pLog->LogX("Failed to init FRM file \"" + sFile + "\"");
   }
   free (framesdata);
}
//---------------------------------------------------------------------------
void CFrmSet::FreeUpFRM(CFrame *l_pFRM)
{
   if (l_pFRM->bLocal) return;
   l_pFRM->FreeUp();
}
//---------------------------------------------------------------------------
void CFrmSet::GetCritterFName(String* filename, DWORD frmPID, WORD *frmID)
{
   int NewIndex = -1;
   try {
      // extract index B from line A,B,C in critters.lst:
      //   A: [FRM name (6 chars)],
      //   B: [index in CRITTERS.LST - used to reference Aimed Shot screen],
      //   C: [Whether Run animations are included]
      int CommaPos = filename->Pos(",");

      String NewIndexAsStr = filename->SubString(CommaPos + 1,
                                             filename->Length() - CommaPos);
      CommaPos = NewIndexAsStr.Pos(",");

      if (CommaPos)
      {
         NewIndexAsStr = NewIndexAsStr.SubString(1, CommaPos - 1);
         NewIndex = NewIndexAsStr.ToInt();
      }
   } catch(EConvertError&) {
      Application->MessageBox("Bad string in critters.lst\n"
                              "Object will be ignored",
                              "Mapper",
                              MB_ICONEXCLAMATION | MB_OK);
     *filename = "";
     return;
   }

   DWORD Index = frmPID & 0x00000FFF;
   DWORD ID1   = (frmPID & 0x0000F000) >> 12;
   DWORD ID2   = (frmPID & 0x00FF0000) >> 16;
   DWORD ID3   = (frmPID & 0x70000000) >> 28;

   // ��������� �������
   if (ID2 == 0x1B || ID2 == 0x1D ||
       ID2 == 0x1E || ID2 == 0x37 ||
       ID2 == 0x39 || ID2 == 0x3A ||
       ID2 == 0x21 || ID2 == 0x40)
   {
      Index = NewIndex;
      *filename = pUtil->GetFRMFileName(critter_ID,
                                pLstFiles->pFRMlst[critter_ID]->Strings[Index]);
//       *filename = pLstFiles->pFRMlst[critter_ID]->Strings[Index];
   }

   int CommaPos = filename->Pos(",");

   if (CommaPos)
        filename->SetLength(CommaPos - 1);

   // ��������� ���������
   char Suffix1;
   char Suffix2;
   if (!getSuffixes(ID1, ID2, Suffix1, Suffix2))
   {
      Application->MessageBox("Bad FRM PID\n"
                              "Object will be ignored",
                              "Mapper",
                              MB_ICONEXCLAMATION | MB_OK);
     *filename = "";
     return;
   }

   // ��������� ������� ����� �����
   String FileExt = "";
   FileExt += Suffix1;
   FileExt += Suffix2;
   FileExt += ".fr";
   FileExt += (ID3) ? char('0' + ID3 - 1) : ('m');
   DWORD SuffIndex = pUtil->GetIndexBySuffix(FileExt);
   if (SuffIndex)
   {
      *frmID *= SuffIndex;
   }
   else
   {
      Application->MessageBox("Bad suffix\n"
                              "Object will be ignored",
                              "Mapper",
                              MB_ICONEXCLAMATION | MB_OK);
   }
   *filename += FileExt;
}
//------------------------------------------------------------------------------
bool CFrmSet::getSuffixes(DWORD ID1, DWORD ID2, char& Suffix1, char& Suffix2)
{
  if (ID1 >= 0x0B)
     return false;

  if (ID2 >= 0x26 && ID2 <= 0x2F) {
     Suffix2 = char(ID2) + 0x3D;

     if (ID1 == 0)
        return false;

     Suffix1 = char(ID1) + 'c';

     return true;
  }

  if (ID2 == 0x24) {
     Suffix2 = 'h';
     Suffix1 = 'c';

     return true;
  }

  if (ID2 == 0x25) {
     Suffix2 = 'j';
     Suffix1 = 'c';

     return true;
  }

  if (ID2 == 0x40) {
     Suffix2 = 'a';
     Suffix1 = 'n';

     return true;
  }

  if (ID2 >= 0x30) {
     Suffix2 = char(ID2) + 0x31;
     Suffix1 = 'r';

     return true;
  }

  if (ID2 >= 0x14) {
     Suffix2 = char(ID2) + 0x4D;
     Suffix1 = 'b';

     return true;
  }

  if (ID2 == 0x12) {
     if (ID1 == 0x01) {
        Suffix1 = 'd';
        Suffix2 = 'm';

        return true;
     }

     if (ID1 == 0x04) {
        Suffix1 = 'g';
        Suffix2 = 'm';

        return true;
     }

     Suffix1 = 'a';
     Suffix2 = 's';

     return true;
  }

  if (ID2 == 0x0D) {
     if (ID1 > 0x00) {
        Suffix1 = char(ID1) + 'c';
        Suffix2 = 'e';

        return true;
     }

     Suffix1 = 'a';
     Suffix2 = 'n';

     return true;
  }

  Suffix2 = char(ID2) + 'a';

  if (ID2 <= 1 && ID1 > 0){
     Suffix1 = char(ID1) + 'c';

     return true;
  }

  Suffix1 = 'a';

  return true;
}
//---------------------------------------------------------------------------
CFrmSet::~CFrmSet()
{
   for (int i = 0; i < 8; i++)
      if (pFRM[i]) delete[] pFRM[i];
}
//---------------------------------------------------------------------------
