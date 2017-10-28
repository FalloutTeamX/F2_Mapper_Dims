//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

//#include <stdlib.h>
#include "main.h"
#include "mdi.h"
#include "utilites.h"
#include "log.h"
#include "map.h"
#include "lists.h"
#include "frmset.h"
#include "frame.h"
#include "tileset.h"
#include "pal.h"
#include "macros.h"
#include "properts.h"
#include "msg.h"
#include "change.h"

//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"

#define SEL_NAV_SIZE_X 85

TfrmEnv *frmEnv;
int wform;
int hform;
int objSeletorCount;
int objInventarCount;

bool undoAvailable;
bool undoSelected;
int globalObjDir;

String subType[17] = {"[Armor]", "[Container]", "[Drug]", "[Weapon]", "[Ammo]", "[Misc]",
            "[Key]", "", "[Portal]", "[Stairs]", "[Elevator]", "[Ladder Down]",
            "[Ladder Up]", "[Generic]", "", "[Exit Grid]", "[Generic]" };

//---------------------------------------------------------------------------
__fastcall TfrmEnv::TfrmEnv(TComponent* Owner)
        : TForm(Owner)
{
   // Установка размера
   wform = frmMDI->mainWidth - 130;   //frmMDI->Width;
   hform = frmMDI->mainHeight - 240;  //frmMDI->Height;

   if (wform < 670)
       wform = 670;
   if (hform < 610)
       hform = 610;

   shp->Width = wform;
   shp->Height = hform;
   Shape3->Width = wform;
   imgMap->Height =  hform - 10;
   imgMap->Width  =  wform  - 10;

   // Миникарта сдвиг вправо
   MinimapPanel->Left = shp->BoundsRect.Right + 2;

   // Инвентарь сдвиг вправо
   InvenPanel->Left = shp->BoundsRect.Right + 2;
   InvenPanel->Height = shp->BoundsRect.Bottom - InvenPanel->Top;
   imgInv->Height = sbINV->Height;

   // Нижняя панель сдвиг вниз
   ObjectPanel->Top = shp->BoundsRect.Bottom;
   ObjectPanel->Width = InvenPanel->Left + InvenPanel->Width - 1;

   Label1->Left = Shape3->BoundsRect.Right - Label1->Width - 10;
   //---------------------------------------------------------------------------

   objSeletorCount = imgObj->Width / SEL_NAV_SIZE_X;
   if (objSeletorCount < 8)
      objSeletorCount = 8;

   blockOnTop = true;
   randomObject = NONE_SELECTED;
   CurMapObjSelect.nObjType = -1;

   pRndObj = frmMDI->RndObj;
   pUtil = frmMDI->pUtil;
   pLog = frmMDI->pLog;
   pFrmSet = frmMDI->pFrmSet;
   pProSet = frmMDI->pProSet;
   pLstFiles = frmMDI->pLstFiles;
   pMsg = frmMDI->pMsg;
   pPal = frmMDI->pPal;
   pMap = NULL;
   pTileSet = NULL;
   pObjSet = NULL;
   frmProp = frmMDI->frmProp;

   dds = NULL; dds2Map = NULL; dds2Nav = NULL; dds2Inv = NULL;
   ddcMap = NULL; ddcNav = NULL; ddcInv = NULL;

   panelTILE = (TStatusPanel *)sbar->Panels->Items[0];
   panelHEX = (TStatusPanel *)sbar->Panels->Items[1];
   panelObjCount = (TStatusPanel *)sbar->Panels->Items[2];
   panelObjSelected = (TStatusPanel *)sbar->Panels->Items[3];
   panelMSG = (TStatusPanel *)sbar->Panels->Items[4];

   pLog->LogX("Attach DDraw primary surface ...");
   AttachDDraw (this, &dds, 1);
   pLog->LogX("Attach DDraw map surface ...");
   AttachDDraw (imgMap, &dds2Map, 0);
   pLog->LogX("Attach DDraw navigator surface ...");
   AttachDDraw (imgObj, &dds2Nav, 0);
   pLog->LogX("Attach DDraw inventory surface ...");
   AttachDDraw (imgInv, &dds2Inv, 0);

   imgMap->Cursor = (TCursor)crHandCursor;
   OldMapWndProc = imgMap->WindowProc;
   imgMap->WindowProc = NewMapWndProc;
   OldNavWndProc = imgObj->WindowProc;
   imgObj->WindowProc = NewNavWndProc;
   OldInvWndProc = imgInv->WindowProc;
   imgInv->WindowProc = NewInvWndProc;

   OldX = -100;
   OldY = -100;
   WorldX = 3640;
   WorldY = 1560;
   iLevel = 0;

   frmMDI->btnNone->Down = true;
   bShowObj[item_ID] = frmMDI->btnItems->Down;
   bShowObj[critter_ID] = frmMDI->btnCritters->Down;
   bShowObj[scenery_ID] = frmMDI->btnScenery->Down;
   bShowObj[wall_ID] = frmMDI->btnWalls->Down;
   bShowObj[floor_ID] = frmMDI->btnFloor->Down;
   bShowObj[misc_ID] = frmMDI->btnMisc->Down;
   bShowObj[roof_ID] = frmMDI->btnRoof->Down;
   bShowObj[EG_blockID] = frmMDI->btnEGBlock->Down;
   bShowObj[SAI_blockID] = frmMDI->btnSaiBlock->Down;
   bShowObj[wall_blockID] = frmMDI->btnWallBlock->Down;
   bShowObj[obj_blockID] = frmMDI->btnObjBlock->Down;
   bShowObj[light_blockID] = frmMDI->btnLightBlock->Down;
   bShowObj[scroll_blockID] = frmMDI->btnScrollBlock->Down;
   bShowObj[obj_self_blockID] = frmMDI->btnObjSelfBlock->Down;
   bShowObj[hex_ID] = frmMDI->btnHEX->Down;
   SelectMode = SELECT_NONE;

   ULONG i;
   h_map = CreateFile(frmMDI->OpenDialog1->FileName.c_str(),
		GENERIC_READ,
		FILE_SHARE_READ,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL);
   if (h_map == INVALID_HANDLE_VALUE)
   {
      pLog->LogX("Cannot open file.");
      return;
   }

   frmMDI->OpenPBarForm();
   frmMDI->frmPBar->NewTask("Loading map data", "Load header ...",
                                                           0, 0, &frmMDI->iPos);
   pMap = new CMap(h_map); //passed
   frmMDI->frmPBar->NewTask(NULL, "Load tileset ...", 0, 0, &frmMDI->iPos);
   pTileSet = new CTileSet(h_map, pMap->TilesSizeX * pMap->TilesSizeY); //passed
   pObjSet = new CObjSet(h_map); //passed

   MapCaptionInfo();

   frmMDI->frmPBar->NewTask(NULL, "Load pro files ...", 0,
                                             pObjSet->nObjTotal, &frmMDI->iPos);
   Application->ProcessMessages();
   pProSet->ClearLocals();
   pProSet->LoadLocalPROs();

   frmMDI->frmPBar->NewTask(NULL, "Load frm files ...", 0,
       pMap->TilesSizeX * pMap->TilesSizeY + pObjSet->nObjTotal, &frmMDI->iPos);
   Application->ProcessMessages();
   pFrmSet->ClearLocals();
   pFrmSet->LoadLocalFRMs();

   frmMDI->DeletePBarForm();

   pBMPlocator = new Graphics::TBitmap();
   pBMPlocator->LoadFromResourceName((UINT)HInstance, "locator");
   DrawMiniMap();
   RedrawMap(true);

   frmMDI->tbPaint->Visible = true;
   frmMDI->tbcurs->Visible = true;
   frmMDI->tbVis->Visible = true;
   frmMDI->btnSaveAs->Enabled = true;
   btnWorldClick(this); //btnLocalClick(this);
   RedrawInventory();
   SetButtonSave(false);
}
//---------------------------------------------------------------------------
void TfrmEnv::MapCaptionInfo()
{
   this->Caption = frmMDI->OpenDialog1->FileName;
   this->lblInfo->Caption =
       (pMap->dwVer == VER_FALLOUT1 ? "[FALLOUT 1] " : "[FALLOUT 2] ") +
       (String)pMap->mapvars.mapname +
       " :: " + (String)(pUtil->GetDW(&pMap->mapvars.LocalVars)) + " local var(s)" +
       " :: " + (String)(pUtil->GetDW(&pMap->mapvars.GlobalVars)) + " global var(s)";
}
//---------------------------------------------------------------------------
void __fastcall TfrmEnv::FormDestroy(TObject *Sender)
{
   imgMap->WindowProc = OldMapWndProc;
   imgObj->WindowProc = OldNavWndProc;
   if (pBMPlocator) delete pBMPlocator;
   if (pObjSet) delete pObjSet;
   if (pTileSet) delete pTileSet;
   if (pMap) delete pMap;
   if (dds) dds->Release();
   if (ddc) ddc->Release();
   if (dds2Map) dds2Map->Release();
   if (dds2Nav) dds2Nav->Release();
   if (dds2Inv) dds2Inv->Release();
   frmMDI->tbcurs->Visible = false;
   frmMDI->tbVis->Visible = false;
   frmMDI->tbPaint->Visible = false;
   frmMDI->btnhand->Down = true;
   frmProp->Hide();
}
//---------------------------------------------------------------------------
void TfrmEnv::SetButtonSave(bool State, bool uState)
{
   undoAvailable = uState;
   frmMDI->btnSave->Enabled = State;
}
//---------------------------------------------------------------------------
void TfrmEnv::DrawMiniMap(void)
{
   Graphics::TBitmap *OffMiniMap = new Graphics::TBitmap;
   OffMiniMap->PixelFormat = pf32bit;
   OffMiniMap->IgnorePalette = true;
   OffMiniMap->Width = 100;
   OffMiniMap->Height = 300;
   PatBlt(OffMiniMap->Canvas->Handle, 0, 0, 100, 300, BLACKNESS);
   for (int y = 0; y < pMap->Levels * 100 ; y++)
   {
      unsigned int *LinePtr = (unsigned int *)OffMiniMap->ScanLine[y];
      for (int x = 0; x < 100; x++)
      {
         int FloorId = pTileSet->GetFloorID(x, y);
         if (FloorId != 1)
         {
            LinePtr[99 - x] =
             pPal->GetEntry(pFrmSet->pFRM[tile_ID][FloorId].GetCenterPix(0, 0));
         }
      }
   }
  imgMiniMap->Canvas->Draw(0, 0, OffMiniMap);
  delete OffMiniMap;
}
//---------------------------------------------------------------------------
void TfrmEnv::PrepareNavigator(BYTE nObjType)
{
   sb->Max = 1; // Обходим ошибку, когда sb->Min > sbMax 
   // Подготавливаем scrollbar в навигаторе для работы с объектами
   // Минимум равен 1, т.к. ПРО и ФРМ начинаются с индекса 1
   sb->Min = nObjType == tile_ID ? 0 : 1;
   sb->Position = sb->Min;

   // Проверяем режим показа объектов : локальный или глобальный
   switch (nObjType)
   {
      case tile_ID:
         if (Navigator.nShowMode == SHOW_WORLD)
            Navigator.nCount = pLstFiles->pFRMlst[nObjType]->Count - 1;
         else
            Navigator.nCount = pFrmSet->nFrmCount[nObjType];
         Navigator.nMaxID = pLstFiles->pFRMlst[nObjType]->Count;
         break;
      case inven_ID:
         if (Navigator.nShowMode == SHOW_WORLD)
            Navigator.nCount = pLstFiles->pPROlst[item_ID]->Count;
         else
            Navigator.nCount = pProSet->nProCount[item_ID];
         Navigator.nMaxID = pLstFiles->pPROlst[item_ID]->Count;
         break;
      default:
         if (Navigator.nShowMode == SHOW_WORLD)
            Navigator.nCount = pLstFiles->pPROlst[nObjType]->Count;
         else
            Navigator.nCount = pProSet->nProCount[nObjType];
         Navigator.nMaxID = pLstFiles->pPROlst[nObjType]->Count;
   }
   sb->Max = Navigator.nCount > objSeletorCount ? Navigator.nCount - (objSeletorCount - 1) : sb->Min;     // 8 / 7
   sb->Enabled = sb->Min != sb->Max;
   sb->Position = Navigator.cScrollPos[nObjType]; // Восстанавливаем пред. позицию

   // Устанавливаем тип отображаемых элементов для навигатора
   Navigator.nObjType = nObjType;
   // Сбрасываем селектирование элементов в навигаторе
   Navigator.nSelID = NONE_SELECTED;

   // Установка стиля фонта отображаемого в навигаторе
   imgObj->Font->Name = "Tahoma";
   imgObj->Font->Height = 11;
   imgObj->Font->Style = TFontStyles()<< fsBold;
   imgObj->Brush->Color = clBlack;
   imgObj->Font->Color = clWhite;
}
//---------------------------------------------------------------------------
void TfrmEnv::RedrawNavigator(int highlight)
{
   int navX, navY, nID, newWidth, newHeight, nPos, nTemp;
   WORD nFrmID;
   CFrame *frm;
   String frmfile;
   navX = 45;
   navY = 5;
   nPos = 0;

   nID = 0;
   if (Navigator.nShowMode == SHOW_LOCAL)
   {
      nTemp = Navigator.nObjType == tile_ID ? sb->Position + 1 : sb->Position;
      while (nTemp)
      {
         nID ++;
         switch (Navigator.nObjType)
         {
            case tile_ID:
               if (pFrmSet->pFRM[Navigator.nObjType][nID - 1].bLocal)
                  nTemp --;
               break;
            case inven_ID:
               if (pProSet->pPRO[item_ID][nID].bLocal)
                  nTemp --;
               break;
            default:
               if (pProSet->pPRO[Navigator.nObjType][nID].bLocal)
                  nTemp --;
         }
      }
   }
   else
      nID = sb->Position;

   memset(Navigator.nNavID, -1, 200); //32
   HDC dc;
   RECT rcs, rcd;
   dds2Nav->GetDC (&dc);
   PatBlt(dc, 0, 0, imgObj->Width,imgObj->Height, BLACKNESS);
   dds2Nav->ReleaseDC (dc);
   while (nID <= Navigator.nMaxID && ((navX + NAV_SIZE_X) < imgObj->Width))  //735
   {
      if (Navigator.nShowMode == SHOW_LOCAL)
      {
         if ((Navigator.nObjType == tile_ID &&
             !pFrmSet->pFRM[Navigator.nObjType][nID].bLocal) ||
             (Navigator.nObjType == inven_ID &&
             !pProSet->pPRO[item_ID][nID].bLocal) ||
             (Navigator.nObjType != tile_ID &&  Navigator.nObjType != inven_ID &&
             !pProSet->pPRO[Navigator.nObjType][nID].bLocal) )
         {
            nID++;
            continue;
         }
      }
      switch (Navigator.nObjType)
      {
         case tile_ID:
            nFrmID = nID;
            break;
         case inven_ID :
            pProSet->LoadPRO(item_ID, nID, false);
            nFrmID = pProSet->pPRO[item_ID][nID].GetInvFrmID();
            break;
         default :
            pProSet->LoadPRO(Navigator.nObjType, nID, false);
            nFrmID = GetFrmID(Navigator.nObjType, nID); //= pProSet->pPRO[Navigator.nObjType][nID].GetFrmID();
      }
      if (nFrmID != 0xFFFF)
      {
         frmfile = pUtil->GetFRMFileName(Navigator.nObjType,
                       pLstFiles->pFRMlst[Navigator.nObjType]->Strings[nFrmID]);
         if (Navigator.nObjType == critter_ID)
         {
            pFrmSet->GetCritterFName(&frmfile,
                  pProSet->pPRO[Navigator.nObjType][nID].GetFrmIDDW(), &nFrmID);
         }
         pFrmSet->LoadFRM(frmfile, Navigator.nObjType, nFrmID, false);
         frm = &pFrmSet->pFRM[Navigator.nObjType][nFrmID];
         if (frm->pBMP == NULL) // Неудалось получить FRM изображение из файла
            pFrmSet->LoadFRM("badimage.frm", Navigator.nObjType, nFrmID, false);
      }
      else
      {
         frmfile = pUtil->GetFRMFileName(item_ID,
                                       pLstFiles->pFRMlst[item_ID]->Strings[0]);
         pFrmSet->LoadFRM(frmfile, item_ID, 0, false);
         frm = &pFrmSet->pFRM[item_ID][0];
      }
      if (Navigator.nObjType == inven_ID)
      {
         pProSet->LoadPRO(item_ID, nID, false);
         nFrmID = pProSet->pPRO[item_ID][nID].GetFrmID();
         frmfile = pUtil->GetFRMFileName(item_ID,
                                  pLstFiles->pFRMlst[item_ID]->Strings[nFrmID]);
         pFrmSet->LoadFRM(frmfile, item_ID, nFrmID, false);
      }

      newWidth = frm->GetWi(0, 0);
      newWidth = newWidth > NAV_SIZE_X ? NAV_SIZE_X : newWidth;
      newHeight = frm->GetHe(0, 0);
      newHeight = newHeight > NAV_SIZE_Y ? NAV_SIZE_Y : newHeight;
      double ar; // aspect ratio of the frame
      if (newWidth && newHeight)
      {
         double ax = (double)frm->GetWi(0,0) / newWidth,
                ay = (double)frm->GetHe(0,0) / newHeight;
         ar = max (ax, ay);
         if (ar > .001)
         {
            newWidth = (int)frm->GetWi (0,0) / ar;
            newHeight = (int)frm->GetHe (0,0) / ar;
         }
      }

      int x = navX - (newWidth >> 1);
      int y = 35 - (newHeight >> 1);
      rcs = Rect (0, 0, frm->GetWi(0, 0), frm->GetHe(0, 0));
      rcd = Bounds (x, y, newWidth, newHeight);
      dds2Nav->Blt(&rcd, frm->GetBMP(), &rcs, DDBLT_WAIT | DDBLT_KEYSRC, NULL);

      Navigator.nNavID[nPos] = nID;
      if (Navigator.nSelID == nID)
         TransBltMask (frm, imgObj, 0, 0, x, y, dds2Nav, newWidth, newHeight);

      String id_str = "ID: " + String(nID);
      byte dir = frm->nDirTotal;
      if (dir > 1 && Navigator.nObjType != critter_ID && Navigator.nObjType != tile_ID && Navigator.nObjType != inven_ID)
         id_str += " [" + (String)dir + "]";

      dds2Nav->GetDC(&dc);
      SelectObject (dc, imgObj->Font->Handle);
      SelectObject (dc, Brush->Handle);
      SetBkMode (dc, TRANSPARENT);

      if (highlight == nID)
         SetTextColor (dc, ColorToRGB(clLime));
      else
         SetTextColor (dc, ColorToRGB((Navigator.nSelID == nID) ? clYellow : clWhite));

      SetBkColor (dc, ColorToRGB(clBlack));
      TextOutA (dc, navX - 40, 66, id_str.c_str(), id_str.Length());
      dds2Nav->ReleaseDC(dc);

      nID++;
      navX += SEL_NAV_SIZE_X; //85
      nPos++;
   }
   imgObj->Repaint();
}
//---------------------------------------------------------------------------
void TfrmEnv::ResetInventoryInfo(void)
{
   for (int i = 0; i < 25; i++)
      Inventory.pChildObj[i] = NULL;

   Inventory.pObj = NULL;
   Inventory.nItemNum = -1;
   Inventory.nInvStartItem = -1;
   Inventory.iLevel = iLevel;
   btnChange->Enabled = false;
//   btnAdd->Enabled = false;
   btnRemove->Enabled = false;
   piChange->Enabled = false;
   piRemove->Enabled = false;
   sbINV->Enabled = false;
   sbINV->Position = 0;
   sbINV->Min = 0;
   sbINV->Max = 0;
}
//---------------------------------------------------------------------------
void TfrmEnv::RedrawInventory(void)
{
   CFrame *frm;
   HDC dc;
   dds2Inv->GetDC (&dc);
   PatBlt(dc, 0, 0, imgInv->Width, imgInv->Height, BLACKNESS); //BLACKNESS
   dds2Inv->ReleaseDC (dc);
   imgInv->Repaint();

//   Inventory.pObj = NULL;

   if (pObjSet->nSelected != 1) return;

   if (objInventarCount == 0) {
      objInventarCount = imgInv->Height / NAV_SIZE_Y;
      if (objInventarCount < 3)
         objInventarCount = 3;
   }

   DWORD nObjNum, nChildCount = 0;
   WORD nID, nFrmID, hexX, hexY;
   BYTE nType, *pObj;
   String frmfile;
   int nPos, navY = 0;
   int newWidth, newHeight;
   RECT rcs, rcd;

   inventoryDraw = true;
   nPos = 0;
   pObj = pObjSet->GetFirstObj(&nObjNum, Inventory.iLevel);   //ilevel
   while (pObj)
   {
      if (pObjSet->ObjSelected())
      {
         Inventory.pObj = pObj;
         DWORD Count = pObjSet->ChildCount();
         if (!Count) break;
         sbINV->Max = Count > objInventarCount ? Count : objInventarCount;  //3
         sbINV->Min = objInventarCount;  //3
         sbINV->Enabled = (sbINV->Max > objInventarCount);  //3

         pObj = pObjSet->GetNextObj(&nObjNum, &nChildCount, Inventory.iLevel);
         for (int nP = 0; nP < Inventory.nInvStartItem; nP++)
            pObj = pObjSet->GetNextObj(&nObjNum, &nChildCount, Inventory.iLevel);

         while (pObjSet->ObjIsChild() && nPos < objInventarCount)  //3
         {
            pObjSet->GetObjType(&nType, &nID);
            pProSet->LoadPRO(item_ID, nID, false);
            nFrmID = pProSet->pPRO[item_ID][nID].GetInvFrmID();
            if (nFrmID != 0xFFFF)
            {
               frmfile = pUtil->GetFRMFileName(inven_ID,
                                 pLstFiles->pFRMlst[inven_ID]->Strings[nFrmID]);
               pFrmSet->LoadFRM(frmfile, inven_ID, nFrmID, true);
               frm = &pFrmSet->pFRM[inven_ID][nFrmID];
            }
            else
            {
               frmfile = pUtil->GetFRMFileName(item_ID,
                                       pLstFiles->pFRMlst[item_ID]->Strings[0]);
               pFrmSet->LoadFRM(frmfile, item_ID, 0, false);
               frm = &pFrmSet->pFRM[item_ID][0];
            }
            newWidth = frm->GetWi(0, 0);
            newWidth = newWidth > NAV_SIZE_X ? NAV_SIZE_X : newWidth;
            newHeight = frm->GetHe(0, 0);
            newHeight = newHeight > NAV_SIZE_Y ? NAV_SIZE_Y : newHeight;

            double ar; // aspect ratio of the frame
            if (newWidth && newHeight)
            {
               double ax = (double)frm->GetWi(0,0) / newWidth,
                      ay = (double)frm->GetHe(0,0) / newHeight;
               ar = max (ax, ay);
               if (ar > .001)
               {
                  newWidth = (int)frm->GetWi (0,0) / ar;
                  newHeight = (int)frm->GetHe (0,0) / ar;
               }
            }
            int x = 42 - (newWidth >> 1);
            int y = navY + (31 - (newHeight >> 1));
            rcs = Rect (0,0, frm->GetWi(0,0), frm->GetHe(0,0));
            rcd = Bounds (x, y, newWidth, newHeight);
            dds2Inv->Blt(&rcd, frm->GetBMP(), &rcs,
                                               DDBLT_WAIT | DDBLT_KEYSRC, NULL);
            if (Inventory.nItemNum == nPos)
            {
               dds2Inv->GetDC(&dc);
               Canvas->Brush->Color = clLime; //
               Canvas->Brush->Style = bsSolid; //
               RECT rc = Rect(0, navY, imgInv->Width, navY + NAV_SIZE_Y);
               FrameRect (dc, &rc, Canvas->Brush->Handle);
               dds2Inv->ReleaseDC (dc);
            }
            Inventory.pChildObj[nPos] = pObj;

            String count_str = "x" + String(pObjSet->GetCountInObj());
            dds2Inv->GetDC(&dc);
            SelectObject (dc, imgInv->Font->Handle);
            SelectObject (dc, Brush->Handle);
            SetBkMode (dc, TRANSPARENT);
            SetTextColor (dc, ColorToRGB(clLime));
            SetBkColor (dc, ColorToRGB(clBlack));
            TextOutA (dc, 5, navY + 5, count_str.c_str(), count_str.Length());
            dds2Inv->ReleaseDC(dc);


            navY += NAV_SIZE_Y;
            nPos ++;

            pObj = pObjSet->GetNextObj(&nObjNum, &nChildCount, Inventory.iLevel);
            if (pObj == NULL)
               break; // Выход если нет болше объектов (фикс бага)
         }
         break;
      }
      pObj = pObjSet->GetNextObj(&nObjNum, &nChildCount, Inventory.iLevel);
   }
   imgInv->Repaint();
   inventoryDraw = false;
}
//---------------------------------------------------------------------------
void __fastcall TfrmEnv::btnChangeClick(TObject *Sender)
{
   if (Inventory.nItemNum == -1) return;
   pObjSet->SetObj(Inventory.pChildObj[Inventory.nItemNum]);
   int nOld = pObjSet->GetCountInObj();
   DWORD nNew = fmChange->ChangeValue(nOld);
   if (nNew > 0 && nNew != nOld)
   {
      pObjSet->SetCountInObj(nNew);
      RedrawInventory();
      SetButtonSave(true);
   }
}
//---------------------------------------------------------------------------
void TfrmEnv::TransBltFrm(CFrame* frm, TControl* win, short nDir, short nFrame, int x, int y, LPDIRECTDRAWSURFACE7 dds2)
{
   RECT rcs, rcd;
   rcs = Bounds(frm->GetSprX(nDir, nFrame), frm->GetSprY(nDir, nFrame),
                            frm->GetWi(nDir, nFrame), frm->GetHe(nDir, nFrame));
   rcd = Bounds(x, y, frm->GetWi(nDir, nFrame), frm->GetHe(nDir, nFrame));
   HRESULT res = dds2->Blt(&rcd, frm->GetBMP(), &rcs, DDBLT_WAIT | DDBLT_KEYSRC, NULL);
}
//---------------------------------------------------------------------------
void TfrmEnv::TransBltMask(CFrame* frm, TControl* win, short nDir, short nFrame,
                 int x, int y, LPDIRECTDRAWSURFACE7 dds2, int width, int height)
{
   RECT rcs, rcd;
   rcs = Bounds(0, 0, width, height);
   rcd = Bounds(x, y, width, height);
   frm->InitBorder(nDir, nFrame, width, height);
   HRESULT res = dds2->Blt(&rcd, frm->GetBorder(), &rcs, DDBLT_WAIT | DDBLT_KEYSRC, NULL);
}
//---------------------------------------------------------------------------
void TfrmEnv::TransBltTileRegion(LPDIRECTDRAWSURFACE7 dds2,
                                 int TileX1, int TileY1, int TileX2, int TileY2, TColor clr, int wPen)
{
      int v1x, v1y, v2x, v2y, v3x, v3y, v4x, v4y;
      TileX1 = TileX1 % 100;
      TileY1 = TileY1 % 100;
      TileX2 = TileX2 % 100;
      TileY2 = TileY2 % 100;
      pUtil->GetTileWorldCoord(TileX1 , TileY1, &v1x, &v1y);
      v1x += TILE_UP_CORNER_X - WorldX;
      v1y += TILE_UP_CORNER_Y - WorldY;
      pUtil->GetTileWorldCoord(TileX1, TileY2, &v2x, &v2y);
      v2x += TILE_RIGHT_CORNER_X - WorldX;
      v2y += TILE_RIGHT_CORNER_Y - WorldY;
      pUtil->GetTileWorldCoord(TileX2, TileY2, &v3x, &v3y);
      v3x += TILE_DOWN_CORNER_X - WorldX;
      v3y += TILE_DOWN_CORNER_Y - WorldY;
      pUtil->GetTileWorldCoord(TileX2, TileY1, &v4x, &v4y);
      v4x += TILE_LEFT_CORNER_X - WorldX;
      v4y += TILE_LEFT_CORNER_Y - WorldY;

      Types::TPoint points[4];
//      Windows::TPoint points[4];
      points[0] = Point(v1x, v1y);
      points[1] = Point(v2x, v2y);
      points[2] = Point(v3x, v3y);
      points[3] = Point(v4x, v4y);

      Canvas->Brush->Style = bsClear; //
      Canvas->Pen->Color = clr; //clWhite
      Canvas->Pen->Width = wPen;
      HDC dc;
      dds2->GetDC(&dc);
      HPEN pen = Canvas->Pen->Handle;
      HPEN oldpen = SelectObject(dc, pen);
      HBRUSH brush = Canvas->Brush->Handle;
      HBRUSH oldbrush = SelectObject(dc, brush);
      Polygon(dc, points, 4);
      SelectObject(dc, oldpen);
      SelectObject(dc, oldbrush);
      dds2->ReleaseDC (dc);
}
//---------------------------------------------------------------------------
void TfrmEnv::AttachDDraw(TControl *win, LPDIRECTDRAWSURFACE7 *dds, int primary)
{
   HRESULT res;
   DDSURFACEDESC2 ddSurfaceDesc;
   if (primary)
   {
      ZeroMemory(&ddSurfaceDesc, sizeof(ddSurfaceDesc));
      ddSurfaceDesc.dwSize = sizeof(ddSurfaceDesc);
      ddSurfaceDesc.dwFlags = DDSD_CAPS;
      ddSurfaceDesc.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;
      res = frmMDI->pDD->CreateSurface (&ddSurfaceDesc, dds, NULL);
      res = frmMDI->pDD->CreateClipper (0, &ddcMap, NULL);
      res = ddcMap->SetHWnd (0, imgMap->Handle);
      res = frmMDI->pDD->CreateClipper (0, &ddcNav, NULL);
      res = ddcNav->SetHWnd (0, imgObj->Handle);
      res = frmMDI->pDD->CreateClipper (0, &ddcInv, NULL);
      res = ddcInv->SetHWnd (0, imgInv->Handle);
   }
   else
   {
      ZeroMemory(&ddSurfaceDesc, sizeof(ddSurfaceDesc));
      ddSurfaceDesc.dwSize = sizeof(ddSurfaceDesc);
      ddSurfaceDesc.dwFlags = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT;
      ddSurfaceDesc.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_SYSTEMMEMORY;
      ddSurfaceDesc.dwWidth = win->Width;
      ddSurfaceDesc.dwHeight = win->Height;
      res = frmMDI->pDD->CreateSurface(&ddSurfaceDesc, dds, NULL);

      struct
      {
	 RGNDATAHEADER rdh;
	 RECT rect;
      } clip_rgn;
      memset(&clip_rgn, 0, sizeof(clip_rgn));
      clip_rgn.rect = win->ClientRect;
      clip_rgn.rdh.dwSize = sizeof(RGNDATAHEADER);
      clip_rgn.rdh.iType = RDH_RECTANGLES;
      clip_rgn.rdh.nCount = 1;
      clip_rgn.rdh.nRgnSize = sizeof(RECT);
      clip_rgn.rdh.rcBound = clip_rgn.rect;

      res = frmMDI->pDD->CreateClipper (0, &ddc, NULL);
      res = ddc->SetClipList ((LPRGNDATA)&clip_rgn, 0);
      res = (*dds)->SetClipper (ddc);

      DDCOLORKEY ck;
      ck.dwColorSpaceLowValue = 0;
      ck.dwColorSpaceHighValue = 0;
      res = (*dds)->SetColorKey(DDCKEY_SRCBLT, &ck);
   }
}
//---------------------------------------------------------------------------
void TfrmEnv::RedrawMap(bool StaticRedraw)
{
   if ((OldWorldX == WorldX) && (OldWorldY == WorldY) && !StaticRedraw) return;
   int w1 = wform;  //640;
   int h1 = hform;  //480;
   HDC dc;
   dds2Map->GetDC (&dc);
   PatBlt(dc, 0, 0, w1, h1, BLACKNESS);
   dds2Map->ReleaseDC (dc);
   if (bShowObj[floor_ID]) RedrawFloor();

   if (bShowObj[hex_ID]) RedrawHex();

   if (blockOnTop) {
      RedrawObjects();
      ShowBlockers();
   } else {
      ShowBlockers();
      RedrawObjects();
   }

   if (bShowObj[roof_ID]) RedrawRoof();

   if (have_sel)
   {
      dds2Map->GetDC(&dc);
      Canvas->Brush->Color = clAqua; //
      Canvas->Brush->Style = bsSolid; //
      RECT rc = Rect(min(downX, cursorX), min(downY, cursorY),
                     max(downX, cursorX), max(downY, cursorY));
      FrameRect (dc, &rc, Canvas->Brush->Handle);
      dds2Map->ReleaseDC (dc);
   }
   imgMap->Repaint();
   RedrawLocator();
   OldWorldX = WorldX;
   OldWorldY = WorldY;
}
//---------------------------------------------------------------------------
void TfrmEnv::ShowBlockers()
{
   if (bShowObj[EG_blockID] || bShowObj[SAI_blockID] ||
       bShowObj[wall_blockID] || bShowObj[obj_blockID] ||
       bShowObj[light_blockID] || bShowObj[scroll_blockID] ||
       bShowObj[obj_self_blockID])
          PreRedrawBlockers();
}
//---------------------------------------------------------------------------
void TfrmEnv::RedrawFloor(void)
{
   int x, y, prev_xx, prev_yy;
   WORD TileId;
   int xx = 4752; //
   int yy = 0; //
   int w1 = wform;  //640;
   int h1 = hform;  //480;
   for (y = iLevel * 100; y < (iLevel + 1) * 100; y++)
   {
      prev_xx = xx;
      prev_yy = yy;
      for (x = 0; x < 100; x++)
      {
         TileId = pTileSet->GetFloorID(x, y);
         //TileId &= 0x0FFF; //отключаем ограничение на 4095 frm для тайлов
         if (TileId != 1) //несуществующие тайлы пола
         {
            if ((xx + pFrmSet->pFRM[tile_ID][TileId].GetWi(0, 0) > WorldX && xx < WorldX + w1) &&
               (yy + pFrmSet->pFRM[tile_ID][TileId].GetHe(0, 0) > WorldY && yy < WorldY + h1))
            {
               TransBltFrm(&pFrmSet->pFRM[tile_ID][TileId], imgMap, 0, 0,
                                             xx - WorldX, yy - WorldY, dds2Map);
            }
         }
         xx -= 48;
         yy += 12;
      }
      xx = prev_xx + 32;
      yy = prev_yy + 24;
   }
   if (pTileSet->dwSelection == floor_ID)
   {
      TransBltTileRegion(dds2Map,
                         pTileSet->SelectedFloorX1,
                         pTileSet->SelectedFloorY1,
                         pTileSet->SelectedFloorX2,
                         pTileSet->SelectedFloorY2,
                         clLime);
   }
   // Обозначить границу уровня
   TransBltTileRegion(dds2Map, 0, 0, 99, 99, clRed, 2);
}
//---------------------------------------------------------------------------
void TfrmEnv::RedrawObjects(void)
{
   int w1 = wform;  //640;
   int h1 = hform;  //480;
   int X, Y;
   BYTE *pObj, nObjType, nProObjType;
   WORD nX, nY, nXX, nYY, nFrmID, nProID;
   DWORD nDir, nObjNum, nChildCount = 0;
   int nMapX, nMapY, nLevel;
   short xx, yy;

   pObj = pObjSet->GetFirstObj(&nObjNum, iLevel);
   while (pObj)
   {
      pObjSet->GetObjFrm(&nObjType, &nFrmID);
      if (nObjType == critter_ID)
      {
         String filename = pUtil->GetFRMFileName(nObjType,
                                 pLstFiles->pFRMlst[nObjType]->Strings[nFrmID]);
         pFrmSet->GetCritterFName(&filename, pObjSet->GetObjFrmDW(), &nFrmID);
      }
      pObjSet->GetObjType(&nProObjType, &nProID);
      if (bShowObj[nObjType] && !pUtil->GetBlockType(nProObjType, nProID))
      {
         pObjSet->GetHexXY(&nX, &nY);
         pUtil->GetHexWorldCoord(nX, nY, &nMapX, &nMapY);
         pObjSet->GetDirection(&nDir);
         int nFrame = pObjSet->GetFrame();

//         pLog->LogX("Draw (" + String(nObjType) + ", " + String(nFrmID) + ") : (" +
//                     String(pFrmSet->pFRM[nObjType][nFrmID].nDirTotal) + ", " +
//                     String(pFrmSet->pFRM[nObjType][nFrmID].nFrames) + ") " +
//                     "dir:" + String(nDir) + " filename: " +
//                     pFrmSet->pFRM[nObjType][nFrmID].FileName);

         X = nMapX - WorldX -
             (pFrmSet->pFRM[nObjType][nFrmID].GetWi(nDir, nFrame) >> 1) +
             pFrmSet->pFRM[nObjType][nFrmID].GetDoffX(nDir) +
             pObjSet->GetXX();
         Y = nMapY - WorldY -
             pFrmSet->pFRM[nObjType][nFrmID].GetHe(nDir, nFrame) +
             pFrmSet->pFRM[nObjType][nFrmID].GetDoffY(nDir) +
             pObjSet->GetYY();
         if ((X + pFrmSet->pFRM[nObjType][nFrmID].GetWi(nDir, nFrame) > 0 && X < w1)
            && (Y + pFrmSet->pFRM[nObjType][nFrmID].GetHe(nDir, nFrame) > 0 && Y < h1))
         {
         TransBltFrm(&(pFrmSet->pFRM[nObjType][nFrmID]), imgMap, nDir, nFrame,
                                   X, Y, dds2Map);
             if (pObjSet->ObjSelected())
             {
		TransBltMask(&pFrmSet->pFRM[nObjType][nFrmID], imgMap, nDir, nFrame,
                                X, Y, dds2Map,
                                pFrmSet->pFRM[nObjType][nFrmID].GetWi(nDir, nFrame),
                                pFrmSet->pFRM[nObjType][nFrmID].GetHe(nDir, nFrame));
             }
         }
      }
      pObj = pObjSet->GetNextObj(&nObjNum, &nChildCount, iLevel);
   }
}
//---------------------------------------------------------------------------
void TfrmEnv::RedrawRoof(void)
{
   int x, y, prev_xx, prev_yy;
   WORD TileId;
   int xx = 4752; //4752              TileX = 0
   int yy = -96; //0                    TileY = 0
   int w1 = wform;  //640;
   int h1 = hform;  //480;
   for (y = iLevel * 100; y < (iLevel + 1) * 100; y++)
   {
      prev_xx = xx;
      prev_yy = yy;
      for (x = 0; x < 100; x++)
      {
         TileId = pTileSet->GetRoofID(x, y);
         //TileId &= 0x0FFF;  //отключаем ограничение на 4095 frm для тайлов
         if (TileId != 1) {//несуществующие тайлы потолка
            if ((xx + pFrmSet->pFRM[tile_ID][TileId].GetWi(0, 0) > WorldX && xx < WorldX + w1) &&
               (yy + pFrmSet->pFRM[tile_ID][TileId].GetHe(0, 0) > WorldY && yy < WorldY + h1))
            {
               TransBltFrm(&pFrmSet->pFRM[tile_ID][TileId], imgMap, 0, 0,
                                             xx - WorldX, yy - WorldY, dds2Map);
            }
         }
         xx -= 48;
         yy += 12;
      }
      xx = prev_xx + 32;
      yy = prev_yy + 24;
   }
   if (pTileSet->dwSelection == roof_ID)
   {
      TransBltTileRegion(dds2Map,
                         pTileSet->SelectedRoofX1 - 2,
                         pTileSet->SelectedRoofY1 - 3,
                         pTileSet->SelectedRoofX2 - 2,
                         pTileSet->SelectedRoofY2 - 3,
                         clYellow);
   }
}
//---------------------------------------------------------------------------
void TfrmEnv::PreRedrawBlockers(void)
{
   BYTE *pObj, nObjType, nProObjType;
   WORD  nFrmID, nProID;
   DWORD nObjNum, nChildCount = 0;

   pObj = pObjSet->GetFirstObj(&nObjNum, iLevel);
   while (pObj)
   {
      // отрисовка блокирующих гексов
      if (bShowObj[obj_self_blockID] && !(pObjSet->GetFlags() & 0x00000010))
         RedrawBlockers(obj_self_blockID, 0, 0);

      // отрисовка блокирующих маркеров
      pObjSet->GetObjType(&nProObjType, &nProID);
      int nBlockType = pUtil->GetBlockType(nProObjType, nProID);
      if (nBlockType && bShowObj[nBlockType]) {
         pObjSet->GetObjFrm(&nObjType, &nFrmID);  //нужно в случае если не переопределено изображение для маркера  
         RedrawBlockers(nBlockType, nObjType, nFrmID);
      }
      pObj = pObjSet->GetNextObj(&nObjNum, &nChildCount, iLevel);
   }
}
//---------------------------------------------------------------------------
void TfrmEnv::RedrawBlockers(int nBlockType, BYTE nObjType, WORD nFrmID)
{
   int w1 = wform;  //640;
   int h1 = hform;  //480;
   WORD nX, nY, nXX, nYY;
   int X, Y, nMapX, nMapY;
   short xx, yy;
   bool ThruFlags = false;

   if (nBlockType != obj_hexID && (pObjSet->GetFlags() & 0x80000000) || (pObjSet->GetFlags() & 0x20000000))
      ThruFlags = true;

   pUtil->GetBlockFrm(nBlockType, &nObjType, &nFrmID, ThruFlags);
   pObjSet->GetHexXY(&nX, &nY);
   pUtil->GetHexWorldCoord(nX, nY, &nMapX, &nMapY);
   X = nMapX - WorldX -
         (pFrmSet->pFRM[nObjType][nFrmID].GetWi(0, 0) >> 1) +
         pFrmSet->pFRM[nObjType][nFrmID].GetDoffX(0) +
         pObjSet->GetXX();
   Y = nMapY - WorldY -
         pFrmSet->pFRM[nObjType][nFrmID].GetHe(0, 0) +
         pFrmSet->pFRM[nObjType][nFrmID].GetDoffY(0) +
         pObjSet->GetYY();
   // смещаем координаты отрисовки относительно значений смещения из объекта
   X -= pObjSet->GetObjInfo(Xoffset);
   Y -= pObjSet->GetObjInfo(Yoffset);
   if ((X + pFrmSet->pFRM[nObjType][nFrmID].GetWi(0, 0) > 0 && X < w1) &&
      (Y + pFrmSet->pFRM[nObjType][nFrmID].GetHe(0, 0) > 0 && Y < h1)) {
      TransBltFrm(&pFrmSet->pFRM[nObjType][nFrmID], imgMap, 0, 0, X, Y, dds2Map);
      // выделение
      if (pObjSet->ObjSelected() && nBlockType != obj_hexID) {
         TransBltMask(&pFrmSet->pFRM[nObjType][nFrmID], imgMap, 0, 0, X, Y, dds2Map,
         pFrmSet->pFRM[nObjType][nFrmID].GetWi(0, 0),
         pFrmSet->pFRM[nObjType][nFrmID].GetHe(0, 0));
      }
   }
}
//---------------------------------------------------------------------------
void TfrmEnv::RedrawHex(void)
{
   int w1 = wform;  //640;
   int h1 = hform;  //480;
   int nMapX, nMapY;
   int x, y;

   for (y = 0; y < 200; y++)
   {
      for (x = 0; x < 200; x++)
      {
         pUtil->GetHexWorldCoord(x, y, &nMapX, &nMapY);
         if ((nMapX + 32 > WorldX && nMapX < WorldX + w1) &&
                                   (nMapY + 16 > WorldY && nMapY < WorldY + h1))
         {
	    TransBltFrm(&pFrmSet->pFRM[intrface_ID][hex_ID], imgMap, 0, 0,
                                  nMapX - WorldX - 16, nMapY - WorldY - 8, dds2Map);
         }
      }
   }

   DWORD nObjNum, nChildCount = 0;
   BYTE *pObj = pObjSet->GetFirstObj(&nObjNum, iLevel);
   while (pObj)
   {
      // отрисовка гекса для объекта
      if (!bShowObj[obj_self_blockID] || (pObjSet->GetFlags() & 0x00000010)) // _NoBlock
         RedrawBlockers(obj_hexID, 0, 0);

      pObj = pObjSet->GetNextObj(&nObjNum, &nChildCount, iLevel);
   }
}
//---------------------------------------------------------------------------
void TfrmEnv::RedrawLocator(void)
{
   int x, y;
   pUtil->GetCursorTile(0, 0, WorldX, WorldY, &x, &y);
   if (x == OldX && y == OldY) return;
   BitBlt(imgMiniMap->Canvas->Handle, OldX, OldY, 21, 21,
                                  pBMPlocator->Canvas->Handle, 0, 0, SRCINVERT);
   x = 89 - x;
   y = y + (100 * iLevel);
   BitBlt(imgMiniMap->Canvas->Handle, x, y, 21, 21,
                                  pBMPlocator->Canvas->Handle, 0, 0, SRCINVERT);
   OldX = x;
   OldY = y;
   imgMiniMap->Repaint();
}
//---------------------------------------------------------------------------
void __fastcall TfrmEnv::FormClose(TObject *Sender, TCloseAction &Action)
{
   Action = caNone;
}
//---------------------------------------------------------------------------
void __fastcall TfrmEnv::imgMapMouseDown(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)
{
   if (mouseBMiddle)
       return; // выход если зажата средняя кл.мышки

   downX = X;
   downY = Y;
   pUtil->GetCursorHex(X, Y, WorldX, WorldY, &downHexX, &downHexY);
   pUtil->GetCursorTile(X, Y, WorldX, WorldY, &downTileX, &downTileY);
   switch (Button)
   {
      case mbLeft:
         switch (SelectMode)
         {
            case SELECT_NONE:
               Screen->Cursor = (TCursor)crHandTakeCursor;
               break;
            case SELECT_OBJ:
               //if (!ctrlDown && !altDown)
               //   ClearObjSelection(true);
               break;
         }
         mouseBLeft = true;
         break;
      case mbRight:
         mouseBRight = true;
         if (SelectMode == DRAW_ROOF)
         {
             frmMDI->btnselect1->Click();
             frmMDI->btnselect1->Down = true;
         }
         if (SelectMode == DRAW_OBJ)
         {
             SelectMode = SELECT_NONE;
             frmMDI->btnselect2->Click();
             frmMDI->btnselect2->Down = true;
             if (drawBlocker) {
               frmMDI->btnNone->Down = true;
               DrawBlock(-1);
             }
         }
         if (SelectMode == DRAW_FLOOR)
         {
             frmMDI->btnselect3->Click();
             frmMDI->btnselect3->Down = true;
         }
         break;
      case mbMiddle:
         if (SelectMode == SELECT_NONE)
            Screen->Cursor = (TCursor)crHandTakeCursor;
         SelectModeMBM = SelectMode;
         SelectMode = SELECT_NONE;
         mouseBMiddle = true;
         break;
   }
}
//---------------------------------------------------------------------------
void __fastcall TfrmEnv::imgMapClick(TObject *Sender)
{
   int TileX, TileY;
   int HexX, HexY;
   switch (SelectMode)
   {
      case SELECT_OBJ:
         if (upX != downX || upY != downY) return;
         SelectObjXY(downX, downY);
         RedrawPickObject(true, NONE_SELECTED); //RedrawMap(true);
         break;
      case DRAW_FLOOR:
         if (Navigator.nObjType != tile_ID || Navigator.nSelID == NONE_SELECTED)
            return;
         pUtil->GetCursorTile(downX, downY, WorldX, WorldY, &TileX, &TileY);
         pTileSet->SetFloor(TileX, TileY, iLevel, Navigator.nSelID);
         RedrawMap(true);
         SetButtonSave(true);
         break;
      case DRAW_ROOF:
         if (Navigator.nObjType != tile_ID || Navigator.nSelID == NONE_SELECTED)
            return;
         pUtil->GetCursorTile(downX, downY, WorldX, WorldY, &TileX, &TileY);
         pTileSet->SetRoof(TileX, TileY, iLevel, Navigator.nSelID);
         RedrawMap(true);
         SetButtonSave(true);
         break;
      case DRAW_OBJ:
         if (Navigator.nObjType == tile_ID || Navigator.nSelID == NONE_SELECTED)
            return;
         pUtil->GetCursorHex(downX, downY, WorldX, WorldY, &HexX, &HexY);
         BYTE ObjType = Navigator.nObjType == inven_ID ? item_ID : Navigator.nObjType;

         bool blockFlag = ((!pUtil->AllowPlaced) && !(pProSet->pPRO[ObjType][Navigator.nSelID].GetFlags() & 0x00000010));
         // Определить заблокирован ли данный гекс.
         if (!ctrlDown && blockFlag && HexIsBlock(HexX, HexY))
            return;  // выходим, нельзя поставить еще один объект на заблокированный гекс

         pObjSet->AppendObject(HexX, HexY, iLevel, ObjType, Navigator.nSelID, globalObjDir);
         RedrawMap(true);
         SetButtonSave(true);

         // функция возвращает рандомные объекты из перечня пресета
         if (randomObject != NONE_SELECTED)
            Navigator.nSelID = ObjectRandomDraw(randomObject);

         break;
   }
}
//---------------------------------------------------------------------------
void __fastcall TfrmEnv::imgMapMouseUp(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)
{
   upX = X;
   upY = Y;
   switch (Button)
   {
      case mbLeft:
         mouseBLeft = false;
         break;
      case mbRight:
         mouseBRight = false;
         if (shiftDown) {
            SelectMode = SELECT_OBJ;
            Screen->Cursor = crDefault;
         }
         return; //break;
      case mbMiddle:
         MouseMiddleUp();
         return;
   }
   switch (SelectMode)
   {
      case SELECT_NONE:
         Screen->Cursor = crDefault;
         break;
      case SELECT_OBJ:
         if (upX - downX && upY - downY)
         {
            have_sel = 0;
            RedrawMap(true);
         }
         else
            imgMapClick(Sender);
         break;
      case SELECT_FLOOR:
      case SELECT_ROOF:
         if (upX - downX && upY - downY)
         {
            tile_sel = 0;
            RedrawMap(true);
         }
         else
         {
            SelectTileRegion(SelectMode, downX, downY, downX, downY);
            RedrawMap(true);
         }
   }
}
//---------------------------------------------------------------------------
void __fastcall TfrmEnv::imgMapMouseMove(TObject *Sender,
      TShiftState Shift, int X, int Y)
{
   if (mouseBRight)
       return; // выход если зажата правая кл.мышки

   pUtil->GetCursorTile(X, Y, WorldX, WorldY, &TileX, &TileY);
   panelTILE->Text = "TILE: (" + (String)TileX + ", " + TileY + ")";
   pUtil->GetCursorHex(X, Y, WorldX, WorldY, &HexX, &HexY);
   int hex = HexY * 200 + HexX;
   panelHEX->Text = "HEX: "+ (String)hex + " (" + (String)HexX + ", " + (String)HexY + ")";
   cursorX = X;
   cursorY = Y;
   switch (SelectMode)
   {
      case SELECT_NONE:
         if (mouseBLeft || mouseBMiddle)
         {
            int changeX = X - downX;
            int changeY = Y - downY;
            WorldX -= changeX;
            WorldY -= changeY;
            RedrawMap(false);
            downX = X;
            downY = Y;
         }
         break;
      case SELECT_FLOOR:
      case SELECT_ROOF:
         if ((X - downX) && (Y - downY) && (mouseBLeft || mouseBRight))
         {
            SelectTileRegion(SelectMode, downX, downY, X, Y);
            RedrawMap(true);

            if (!tile_sel || prevX != X || prevY != Y)
            {
               tile_sel = 1;
               prevX = X;
               prevY = Y;
            }
         }
         break;
      case SELECT_OBJ:
         if ((X - downX) && (Y - downY) &&
             (X - prevX) && (Y - prevY) && (mouseBLeft || mouseBRight))
         {
            SelectObjRegion(min(downX, X) , min(downY, Y),
                            max(downX, X) , max(downY, Y));
            RedrawMap(true);

            if (!have_sel || prevX != X || prevY != Y)
            {
               have_sel = 1;
               prevX = X;
               prevY = Y;
            }
         }
         if (!have_sel)
            RedrawPickObject(false, NONE_SELECTED);
         break;

      case DRAW_OBJ:
         //if (/*Navigator.nSelID != NONE_SELECTED &&*/ Navigator.nObjType != tile_ID)
            RedrawPickObject(false, Navigator.nSelID);
         break;
      case DRAW_FLOOR:
      case DRAW_ROOF:
         if (Navigator.nSelID != NONE_SELECTED && Navigator.nObjType == tile_ID)
         {
            int TileX, TileY, objX, objY;
            CFrame *frm = &pFrmSet->pFRM[tile_ID][Navigator.nSelID];
            pUtil->GetCursorTile(X, Y, WorldX, WorldY, &TileX, &TileY);
            pUtil->GetTileWorldCoord(TileX, TileY, &objX, &objY);

            if (!(objX == OldShowObjX && objY == OldShowObjY))
            {
               RedrawMap(true);
               TransBltFrm(frm, imgMap, 0, 0, objX - WorldX, objY - WorldY, dds2Map);
               // подсветить тайл
               TransBltTileRegion(dds2Map, TileX, TileY, TileX, TileY, clSilver, 1);
               imgMap->Repaint();
               OldShowObjX = objX;
               OldShowObjY = objY;
            }
         }
         break;
      case MOVE_OBJ:
         int offsetHexX = HexX - downHexX;
         int offsetHexY = HexY - downHexY;
         if ((offsetHexX || offsetHexY) && mouseBLeft && pObjSet->nSelected)
         {
            MoveSelectedObjects(offsetHexX, offsetHexY);
            RedrawMap(true);
            downHexX = HexX;
            downHexY = HexY;
         }
         break;
   }
}
//---------------------------------------------------------------------------
void __fastcall TfrmEnv::imgMiniMapMouseDown(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)
{
   if (Y / 100 >= pMap->Levels) return;
   pUtil->GetWorldCoord(X, Y, &WorldX, &WorldY, &iLevel);
   switch (Button)
   {
      case mbLeft:
         mouseBLeft2 = true;
         break;
      case mbRight:
         mouseBRight2 = true;
         break;
   }
   RedrawMap(false);
}
//---------------------------------------------------------------------------
void __fastcall TfrmEnv::imgMiniMapMouseUp(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)
{
   switch (Button)
   {
      case mbLeft:
         mouseBLeft2 = false;
         break;
      case mbRight:
         mouseBRight2 = false;
         break;
   }
}
//---------------------------------------------------------------------------
void __fastcall TfrmEnv::imgMiniMapMouseMove(TObject *Sender,
      TShiftState Shift, int X, int Y)
{
   X = (X <= 99 ? X : 99) * (X >= 0);
   Y = (Y <= 299 ? Y : 299) * (Y >= 0);
   panelTILE->Text =
                  "TILE: (" + (String)(99 - X) + ", " + (String)(Y % 100) + ")";
   if (mouseBLeft2)
      imgMiniMapMouseDown(Sender, mbRight, Shift, X, Y);
}
//---------------------------------------------------------------------------
void TfrmEnv::RedrawPickObject(bool force, int nID)
{
   int HexX, HexY, objX, objY;
   BYTE ObjType;
   WORD nFrmID;

   if (Navigator.nObjType == tile_ID)
      nID = NONE_SELECTED;

   if (nID == NONE_SELECTED && !bShowObj[hex_ID]) {
      if (force)
         RedrawMap(true);
      return;
   }

   if (nID == NONE_SELECTED) {
      ObjType = intrface_ID;  // для курсора гекса
   } else
      ObjType = Navigator.nObjType == inven_ID ? item_ID : Navigator.nObjType;

   nFrmID = GetFrmID(ObjType, nID); //pProSet->pPRO[ObjType][Navigator.nSelID].GetFrmID();
   CFrame *frm = &pFrmSet->pFRM[ObjType][nFrmID];

   pUtil->GetCursorHex(cursorX, cursorY, WorldX, WorldY, &HexX, &HexY);
   pUtil->GetHexWorldCoord(HexX, HexY, &objX, &objY);

   objX = objX - WorldX - (frm->GetWi(globalObjDir, 0) >> 1) + frm->GetDoffX(0);
   objY = objY - WorldY - frm->GetHe(globalObjDir, 0) + frm->GetDoffY(0);
   if (force || !(objX == OldShowObjX && objY == OldShowObjY)) {
      RedrawMap(true);
      TransBltFrm(frm, imgMap, globalObjDir, 0, objX, objY, dds2Map);
      imgMap->Repaint();
      OldShowObjX = objX;
      OldShowObjY = objY;
   }
}
//---------------------------------------------------------------------------
void TfrmEnv::SelectObjXY(int X, int Y)
{
   DWORD nObjNum, nChildCount = 0;
   WORD nFrmID, nProID, hexX, hexY;
   BYTE nObjType, nProObjType, *pObj, *pSelObj, tmpObjType;
   int nMapX, nMapY, objX, objY;
   DWORD nDir;

   bool pSelected = (pObjSet->nSelected > 0);

   if (!ctrlDown && !altDown)
      ClearObjSelection(false);

   pSelObj = NULL;
   pObj = pObjSet->GetFirstObj(&nObjNum, iLevel);
   while (pObj)
   {
      pObjSet->GetObjFrm(&nObjType, &nFrmID);
      if (nObjType == critter_ID)
      {
         String filename = pUtil->GetFRMFileName(nObjType,
                                 pLstFiles->pFRMlst[nObjType]->Strings[nFrmID]);
         pFrmSet->GetCritterFName(&filename, pObjSet->GetObjFrmDW(), &nFrmID);
      }
      pObjSet->GetObjType(&nProObjType, &nProID);
      pObjSet->GetDirection(&nDir);
      int nBlockType = pUtil->GetBlockType(nProObjType, nProID);
      if ((!nBlockType && bShowObj[nProObjType]) ||
          (nBlockType && bShowObj[nBlockType]))
      {
         if (nBlockType)
            tmpObjType = nObjType;
         pUtil->GetBlockFrm(nBlockType, &nObjType, &nFrmID, false);
         pObjSet->GetHexXY(&hexX, &hexY);
         pUtil->GetHexWorldCoord(hexX, hexY, &nMapX, &nMapY);
         objX = nMapX - WorldX -
             (pFrmSet->pFRM[nObjType][nFrmID].GetWi(nDir, 0) >> 1) +
             pFrmSet->pFRM[nObjType][nFrmID].GetDoffX(nDir) +
             pObjSet->GetXX();
         objY = nMapY - WorldY -
             pFrmSet->pFRM[nObjType][nFrmID].GetHe(nDir, 0) +
             pFrmSet->pFRM[nObjType][nFrmID].GetDoffY(nDir) +
             pObjSet->GetYY();
         if (X >= objX &&
             X <= objX + pFrmSet->pFRM[nObjType][nFrmID].GetWi(nDir, 0) &&
             Y >= objY &&
             Y <= objY + pFrmSet->pFRM[nObjType][nFrmID].GetHe(nDir, 0))
         {
            if (pFrmSet->pFRM[nObjType][nFrmID].GetPixel(X - objX, Y - objY, nDir, 0)) {
               pSelObj = pObj;
               CurMapObjSelect.nObjType = (nBlockType) ? tmpObjType : nObjType;
               CurMapObjSelect.nSelID = nProID;
               SetGlobalObjectDir(nDir);
            }
         }
      }
      pObj = pObjSet->GetNextObj(&nObjNum, &nChildCount, iLevel);
   }

   if (pSelObj)
   {
      pObjSet->SetObj(pSelObj);
      if (altDown) {
         pObjSet->DeselectObj();
         CurMapObjSelect.nObjType = -1;
      } else {
         pObjSet->SelectObj();
         CurMapObjSelect.pSelObj = pSelObj;
         if (CurMapObjSelect.nObjType != tile_ID && CurMapObjSelect.nObjType < intrface_ID) {
            DWORD nMsgID = pProSet->pPRO[CurMapObjSelect.nObjType][CurMapObjSelect.nSelID].GetMsgID();
            panelMSG->Text = pMsg->GetMSG(CurMapObjSelect.nObjType, nMsgID);
         }
      }
   }
   if (pSelected || pSelObj != NULL)
      SelectionChanged();
}
//---------------------------------------------------------------------------
bool TfrmEnv::SelectObjRegion(int X, int Y, int X2, int Y2)
{
   DWORD nObjNum, nChildCount = 0;
   WORD nFrmID, nProID, hexX, hexY;
   BYTE nObjType, nProObjType, *pObj;
   int nMapX, nMapY, objX, objY;
   DWORD nDir;

   int objSelCount = pObjSet->nSelected;

   pObj = pObjSet->GetFirstObj(&nObjNum, iLevel);
   while (pObj)
   {
      pObjSet->GetObjFrm(&nObjType, &nFrmID);
      if (nObjType == critter_ID)
      {
         String filename = pUtil->GetFRMFileName(nObjType,
                                 pLstFiles->pFRMlst[nObjType]->Strings[nFrmID]);
         pFrmSet->GetCritterFName(&filename, pObjSet->GetObjFrmDW(), &nFrmID);
      }
      pObjSet->GetObjType(&nProObjType, &nProID);
      pObjSet->GetDirection(&nDir);
      int nBlockType = pUtil->GetBlockType(nProObjType, nProID);
      if ((!nBlockType && bShowObj[nProObjType]) ||
          (nBlockType && bShowObj[nBlockType]))
      {
         pUtil->GetBlockFrm(nBlockType, &nObjType, &nFrmID, false);
         pObjSet->GetHexXY(&hexX, &hexY);
         pUtil->GetHexWorldCoord(hexX, hexY, &nMapX, &nMapY);
         objX = nMapX - WorldX -
             (pFrmSet->pFRM[nObjType][nFrmID].GetWi(nDir, 0) >> 1) +
             pFrmSet->pFRM[nObjType][nFrmID].GetDoffX(nDir) +
             pObjSet->GetXX();
         objY = nMapY - WorldY -
             pFrmSet->pFRM[nObjType][nFrmID].GetHe(nDir, 0) +
             pFrmSet->pFRM[nObjType][nFrmID].GetDoffY(nDir) +
             pObjSet->GetYY();
         if ((objX + pFrmSet->pFRM[nObjType][nFrmID].GetWi(nDir, 0) > X &&
               objX < X2) &&
             (objY + pFrmSet->pFRM[nObjType][nFrmID].GetHe(nDir, 0) > Y &&
               objY < Y2))
         {
            if (altDown)
               pObjSet->DeselectObj();
            else
               pObjSet->SelectObj();
         }
         else if (!ctrlDown && !altDown)
         {
            pObjSet->DeselectObj();
         }
      }
      pObj = pObjSet->GetNextObj(&nObjNum, &nChildCount, iLevel);
   }

   if (objSelCount != pObjSet->nSelected)
      SelectionChanged();

   CurMapObjSelect.pSelObj = NULL;
   CurMapObjSelect.nSelID = 0;
   CurMapObjSelect.nObjType = -1;

   return true;
}
//---------------------------------------------------------------------------
void TfrmEnv::SelectionChanged(void)
{
   frmProp->ListRefresh();
   ResetInventoryInfo();
   RedrawInventory();
   panelObjSelected->Text = "Selected: " + (String)pObjSet->nSelected;

   bool enabled = ((CurMapObjSelect.nObjType == item_ID || CurMapObjSelect.nObjType == critter_ID)
                  && (pObjSet->nSelected == 1 && Navigator.nSelID != NONE_SELECTED)
                  && (Navigator.nObjType == inven_ID || Navigator.nObjType == item_ID));
   if (enabled && CurMapObjSelect.nObjType == item_ID)
      enabled = (pProSet->GetSubType(item_ID, CurMapObjSelect.nSelID) == OContainer);

   btnAdd->Enabled = enabled;
   piAdd->Enabled = enabled;
}
//---------------------------------------------------------------------------
bool TfrmEnv::SelectTileRegion(int TileMode, int X, int Y, int X2, int Y2)
{
   int TileX1, TileY1, TileX2, TileY2;
   pUtil->GetCursorTile(X, Y, WorldX, WorldY, &TileX1, &TileY1);
   pUtil->GetCursorTile(X2, Y2, WorldX, WorldY, &TileX2, &TileY2);
   TileY1 = (iLevel * 100) + TileY1;
   TileY2 = (iLevel * 100) + TileY2;
   pTileSet->SelectTiles(TileMode, iLevel,
                         min(TileX1, TileX2), min(TileY1, TileY2),
                         max(TileX1, TileX2), max(TileY1, TileY2));
}
//---------------------------------------------------------------------------
void TfrmEnv::ClearSelection(void)
{
   ClearFloorSelection(false);
   ClearRoofSelection(false);
   ClearObjSelection(true); //with RedrawMap
   //RedrawMap(true);
}
//---------------------------------------------------------------------------
void TfrmEnv::ClearFloorSelection(bool NeedRedrawMap)
{
   pTileSet->ClearFloorSelection();
   if (NeedRedrawMap) RedrawMap(true);
}
//---------------------------------------------------------------------------
void TfrmEnv::ClearRoofSelection(bool NeedRedrawMap)
{
   pTileSet->ClearRoofSelection();
   if (NeedRedrawMap) RedrawMap(true);
}
//---------------------------------------------------------------------------
void TfrmEnv::ClearObjSelection(bool NeedRedrawMap)
{
   pObjSet->ClearSelection();
   CurMapObjSelect.nObjType = NONE_SELECTED;
//   frmProp->ListRefresh();
//   RedrawInventory();

   if (NeedRedrawMap) {
      SelectionChanged();
      RedrawMap(true);
   }
}
//---------------------------------------------------------------------------
void TfrmEnv::MoveSelectedObjects(int offsetHexX, int offsetHexY)
{
   DWORD nObjNum, nChildCount = 0;
   BYTE nObjType, nProObjType, *pObj;
   WORD objHexX, objHexY;
   pObj = pObjSet->GetFirstObj(&nObjNum, iLevel);
   while (pObj)
   {
      if (pObjSet->ObjSelected() == OBJ_SELECTED)
      {
         pObjSet->GetHexXY(&objHexX, &objHexY);
         objHexX += offsetHexX;
         objHexY += offsetHexY;
         pObjSet->SetHexXY(objHexX, objHexY);
      }
      pObj = pObjSet->GetNextObj(&nObjNum, &nChildCount, iLevel);
   }
   SetButtonSave(true);
}
//---------------------------------------------------------------------------
void TfrmEnv::RotateSelectedObjects(int direction)
{
   DWORD nDir, nObjNum, nChildCount = 0;
   WORD nProID, nFrmID, nMaxDir;
   BYTE nProObjType, nFrmType, *pObj;
   bool state = false;

   if (Navigator.nObjType == tile_ID)
      return;

   if (Navigator.nObjType > misc_ID)
       Navigator.nObjType = item_ID;

   if (pObjSet->nSelected < 1) {
      if (Navigator.nSelID != NONE_SELECTED) {
         nFrmID = pProSet->pPRO[Navigator.nObjType][Navigator.nSelID].GetFrmID();
         nMaxDir = pFrmSet->pFRM[Navigator.nObjType][nFrmID].nDirTotal;
      } else
           nMaxDir = 6;
      globalObjDir = CheckDirection(direction, globalObjDir, nMaxDir);

      if (SelectMode == DRAW_OBJ && Navigator.nSelID != NONE_SELECTED)
         RedrawPickObject(true, Navigator.nSelID);
   } else {
      pObj = pObjSet->GetFirstObj(&nObjNum, iLevel);
      while (pObj)
      {
         if (pObjSet->ObjSelected() == OBJ_SELECTED) {
            pObjSet->GetObjFrm(&nFrmType, &nFrmID);
            if (nFrmType == critter_ID) {
               pObjSet->GetObjType(&nProObjType, &nProID);
               String filename = pUtil->GetFRMFileName(nProObjType,
                               pLstFiles->pFRMlst[nProObjType]->Strings[nFrmID]);
               pFrmSet->GetCritterFName(&filename, pObjSet->GetObjFrmDW(), &nFrmID);
            }
            pObjSet->GetDirection(&nDir);
            nMaxDir = pFrmSet->pFRM[nFrmType][nFrmID].nDirTotal;
            globalObjDir = CheckDirection(direction, nDir, nMaxDir);
            state = (globalObjDir != nDir);
            if (state)
               pObjSet->SetDirection(globalObjDir);
         }
         pObj = pObjSet->GetNextObj(&nObjNum, &nChildCount, iLevel);
      }
      RedrawMap(true);
      SetButtonSave(state);
   }
   lblDir->Caption = (pObjSet->nSelected > 1) ? (String)"X" : IntToStr(globalObjDir);
}
//---------------------------------------------------------------------------
DWORD TfrmEnv::CheckDirection(int direction, DWORD nDir, WORD nMaxDir)
{
   if (nMaxDir) {
      if (direction == ROTATE_CW)
         nDir = (++nDir == nMaxDir) ? 0 : nDir;
      else
         nDir = ((int)--nDir < 0) ? (nMaxDir - 1) : nDir;
   }
   return nDir;
}
//---------------------------------------------------------------------------
void __fastcall TfrmEnv::popupMapPopup(TObject *Sender)
{
   ctrlDown = false;
   altDown = false;
   piTilesToNavigator->Enabled = (tabc->TabIndex == tile_ID);
   piChangeMode->Enabled = (pObjSet->nSelected && (SelectMode == SELECT_OBJ || SelectMode == MOVE_OBJ));
   piPickDrawObject->Enabled = (pObjSet->nSelected == 1);
   piUseObject->Enabled = (pObjSet->nSelected == 1 && CurMapObjSelect.nObjType != -1);
   piProperties->Enabled = (pObjSet->nSelected == 1);
   piDelete->Enabled = (pObjSet->nSelected | pTileSet->dwSelection != NONE_SELECTED);
}
//---------------------------------------------------------------------------
void __fastcall TfrmEnv::piDeleteClick(TObject *Sender)
{
   UndoDelete(false);

   if (pTileSet->dwSelection == floor_ID)
   {
      pTileSet->SetFloorRegion(pTileSet->SelectedFloorX1,
                               pTileSet->SelectedFloorY1,
                               pTileSet->SelectedFloorX2,
                               pTileSet->SelectedFloorY2, 1);
   }
   if (pTileSet->dwSelection == roof_ID)
   {
      pTileSet->SetRoofRegion(pTileSet->SelectedRoofX1,
                              pTileSet->SelectedRoofY1,
                              pTileSet->SelectedRoofX2,
                              pTileSet->SelectedRoofY2, 1);
   }
   if (pObjSet->nSelected)
   {
      pObjSet->DeleteSelected();
   }
   SelectionChanged();
   RedrawMap(true);
   SetButtonSave(true, true);
}
//---------------------------------------------------------------------------
void __fastcall TfrmEnv::piPropertiesClick(TObject *Sender)
{
   frmProp->ShowProperties();
}
//---------------------------------------------------------------------------
void __fastcall TfrmEnv::tabcChange(TObject *Sender)
{
   BYTE  nObjType = Navigator.nObjType;
   DWORD nSelID = Navigator.nSelID;
   Navigator.cSelectedID = 0;

   PrepareNavigator(tabc->TabIndex);
   RedrawNavigator();
   btnAdd->Enabled = false;
   piAdd->Enabled = false;

   if (randomObject != NONE_SELECTED) {
      Navigator.nObjType = nObjType;
      Navigator.nSelID = nSelID;
   }

   if (tabc->TabIndex == inven_ID)
      imgObj->ShowHint = true;
   else
      imgObj->ShowHint = false;
}
//---------------------------------------------------------------------------
void __fastcall TfrmEnv::imgObjMouseDown(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)
{
   int nPos = X / SEL_NAV_SIZE_X;
   //nPos = nPos > objSeletorCount - 1 ? objSeletorCount - 1 : nPos;       //7
   if (Navigator.nNavID[nPos] == -1 || nPos == objSeletorCount) return;

   if (Navigator.nObjType != critter_ID)
      SetGlobalObjectDir();

   Navigator.nObjType = tabc->TabIndex;
   Navigator.nSelID = Navigator.nNavID[nPos];
   Navigator.cSelectedID = Navigator.nSelID;

   if (Navigator.nObjType != tile_ID)
   {
      BYTE ObjType = Navigator.nObjType == inven_ID ? item_ID : Navigator.nObjType;
      DWORD nMsgID = pProSet->pPRO[ObjType][Navigator.nSelID].GetMsgID();
      String msg = pMsg->GetMSG(ObjType, nMsgID);
      String type;
      if (ObjType == scenery_ID || ObjType == item_ID)
         type = subType[GetIndexSubType(ObjType, Navigator.nSelID)] + " ";
      panelMSG->Text = type + msg;
   }
   else
   {
      panelMSG->Text = "";
      switch (pTileSet->dwSelection)
      {
         case floor_ID:
            pTileSet->SetFloorRegion(pTileSet->SelectedFloorX1,
                                     pTileSet->SelectedFloorY1,
                                     pTileSet->SelectedFloorX2,
                                     pTileSet->SelectedFloorY2,
                                     Navigator.nSelID);
            break;
         case roof_ID:
            pTileSet->SetRoofRegion(pTileSet->SelectedRoofX1,
                                    pTileSet->SelectedRoofY1,
                                    pTileSet->SelectedRoofX2,
                                    pTileSet->SelectedRoofY2,
                                    Navigator.nSelID);
            break;
      }
      RedrawMap(true);
      SetButtonSave(true);
   }
   // При выборе автоматически переключаемся в режим рисования
   if (Navigator.nObjType != inven_ID) {
      if (Navigator.nObjType != tile_ID) {
         if (drawBlocker)
            frmMDI->btnNone->Down = true;
         frmMDI->btnpen2->Down = true;
         frmMDI->btnpen2->Click();
      } else {
        if (pTileSet->dwSelection == -1 && !(SelectMode == DRAW_ROOF || SelectMode == DRAW_FLOOR)) {
          if (drawBlocker)
            frmMDI->btnNone->Down = true;
          if (SelectMode == SELECT_ROOF) {
               frmMDI->btnpen3->Down = true;
               frmMDI->btnpen3->Click();
          } else {
               frmMDI->btnpen1->Down = true;
               frmMDI->btnpen1->Click();
          }
        }
      }
   }

   if ((Navigator.nObjType == inven_ID || Navigator.nObjType == item_ID) &&
        pObjSet->nSelected == 1 && Navigator.nSelID != NONE_SELECTED &&
        (CurMapObjSelect.nObjType == item_ID || CurMapObjSelect.nObjType == critter_ID)) {
        int type = pProSet->GetSubType(CurMapObjSelect.nObjType, CurMapObjSelect.nSelID);
      btnAdd->Enabled = (CurMapObjSelect.nObjType == critter_ID || type == OContainer);
   } else
      btnAdd->Enabled = false;
   piAdd->Enabled = btnAdd->Enabled;
   RedrawNavigator();

}
//---------------------------------------------------------------------------
void __fastcall TfrmEnv::imgInvMouseDown(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)
{
   int nPos = Y / NAV_SIZE_Y;
   if (Inventory.pChildObj[nPos] == NULL)
        return;

   Inventory.nItemNum = nPos;
   RedrawInventory();

   //показать описание предмета
   BYTE nType;
   WORD nID;
   pObjSet->SetObj(Inventory.pChildObj[nPos]);
   pObjSet->GetObjType(&nType, &nID);
   DWORD nMsgID = pProSet->pPRO[nType][nID].GetMsgID();
   panelMSG->Text = pMsg->GetMSG(nType, ++nMsgID);

   bool enabled = (Inventory.pChildObj[nPos] != NULL);
   btnChange->Enabled = enabled;
   btnRemove->Enabled = enabled;
   piChange->Enabled = enabled;
   piRemove->Enabled = enabled;
}
//---------------------------------------------------------------------------
void __fastcall TfrmEnv::FormKeyDown(TObject *Sender, WORD &Key,
      TShiftState Shift)
{
   switch(Key)
   {
      case VK_UP:
         if (Shift.Contains(ssCtrl))
            WorldY -= 52;
         WorldY -= 12;
         RedrawMap(false);
         break;
      case VK_DOWN:
         if (Shift.Contains(ssCtrl))
            WorldY += 52;
         WorldY += 12;
         RedrawMap(false);
         break;
      case VK_LEFT:
         if (Shift.Contains(ssCtrl))
            WorldX -= 32;
         WorldX -= 32;
         RedrawMap(false);
         break;
      case VK_RIGHT:
         if (Shift.Contains(ssCtrl))
            WorldX += 32;
         WorldX += 32;
         RedrawMap(false);
         break;
      case VK_OEM_PERIOD:
         RotateSelectedObjects(ROTATE_CW);
         break;
      case VK_OEM_COMMA:
         RotateSelectedObjects(ROTATE_CCW);
         break;
      case VK_RETURN:
         if (pObjSet->nSelected) //Shift.Contains(ssAlt)
            frmProp->ShowProperties();
         break;
      case VK_DELETE:
         if (pObjSet->nSelected)
            piDeleteClick(Sender);
         break;
      case 'P':
         if (tabc->TabIndex == tile_ID)
            piTilesToNavigatorClick(Sender);
         break;
      case 'H':
         frmMDI->btnHEX->Down = !frmMDI->btnHEX->Down;
         frmMDI->btnHEXClick(Sender);
         break;
      case 'B':
         frmMDI->btnObjSelfBlock->Down = !frmMDI->btnObjSelfBlock->Down;
         frmMDI->btnObjSelfBlockClick(Sender);
         break;
      case VK_F5:
         if (pObjSet->nSelected)
            LogSelected();
         break;
      case VK_SHIFT:
         if (!mouseBLeft && SelectMode == SELECT_OBJ && pObjSet->nSelected) {
            Screen->Cursor = crMoveCursor;
            SelectMode = MOVE_OBJ;
            shiftDown = true;
            if (bShowObj[hex_ID])
               RedrawMap(true);
         }
         break;
      case VK_CONTROL:
         ctrlDown = true;
         break;
     case VK_MENU: //ALT key
         altDown = true;
         break;
     case VK_SPACE: // Swith to draw mode
         switch(SelectMode)
         {
            case SELECT_OBJ:
            case MOVE_OBJ:
               frmMDI->btnpen2->Down = true;
               frmMDI->btnpen2->Click();
               break;
            case SELECT_FLOOR:
               frmMDI->btnpen1->Down = true;
               frmMDI->btnpen1->Click();
               break;
            case SELECT_ROOF:
               frmMDI->btnpen3->Down = true;
               frmMDI->btnpen3->Click();
               break;
         }
         break;
   }
}
//---------------------------------------------------------------------------
void __fastcall TfrmEnv::FormKeyUp(TObject *Sender, WORD &Key,
      TShiftState Shift)
{
   switch(Key)
   {
      case VK_SHIFT:
         if (shiftDown && SelectMode == MOVE_OBJ) {
            SelectMode = SELECT_OBJ;
            Screen->Cursor = crDefault;
            if (bShowObj[hex_ID])
               RedrawPickObject(false, NONE_SELECTED);
         }
         shiftDown = false;
         break;
     case VK_CONTROL:
         ctrlDown = false;
         break;
      case VK_MENU: //ALT key
         altDown = false;
         break;
   }
}
//---------------------------------------------------------------------------
void __fastcall TfrmEnv::btnWorldClick(TObject *Sender)
{
   Navigator.nShowMode = SHOW_WORLD;
   PrepareNavigator(tabc->TabIndex);
   RedrawNavigator();
}
//---------------------------------------------------------------------------
void __fastcall TfrmEnv::btnLocalClick(TObject *Sender)
{
   Navigator.nShowMode = SHOW_LOCAL;
   PrepareNavigator(tabc->TabIndex);
   RedrawNavigator();
}
//---------------------------------------------------------------------------
void __fastcall TfrmEnv::sbScroll(TObject *Sender, TScrollCode ScrollCode,
      int &ScrollPos)
{
   Navigator.nObjType = tabc->TabIndex;
   Navigator.cScrollPos[Navigator.nObjType] = sb->Position;
   RedrawNavigator();
}
//---------------------------------------------------------------------------
void TfrmEnv::RepaintDDrawWindow(TWinControl *win, LPDIRECTDRAWSURFACE7 dds,
                           LPDIRECTDRAWSURFACE7 dds2, LPDIRECTDRAWCLIPPER ddc)
{
   RECT rcs, rcd;
//   GetUpdateRect (win->Handle, &rcs, false);
   rcs = win->ClientRect;
   rcd = rcs;
   OffsetRect(&rcd, win->ClientOrigin.x, win->ClientOrigin.y);
   dds->SetClipper(ddc);
   HRESULT res = dds->Blt(&rcd, dds2, &rcs, DDBLT_ASYNC, NULL);
   if (res == DDERR_SURFACELOST)
   {
      if (dds->IsLost())
         res = dds->Restore();
      if (dds2->IsLost())
         res = dds2->Restore();
      res = dds->Blt(&rcd, dds2, &rcs, DDBLT_ASYNC, NULL);
   }
   ValidateRect (win->Handle, &rcs);
}
//---------------------------------------------------------------------------
void __fastcall TfrmEnv::NewMapWndProc(TMessage &Msg)
{
   switch (Msg.Msg)
   {
      case WM_PAINT:
         RepaintDDrawWindow(imgMap, dds, dds2Map, ddcMap);
         Msg.Result = 0;
         break;
      case CM_MOUSELEAVE:
         if (mouseBMiddle)
            MouseMiddleUp();
         RedrawMap(true);
         //break;
      default:
         OldMapWndProc(Msg);
   }
}
//---------------------------------------------------------------------------
void __fastcall TfrmEnv::NewNavWndProc(TMessage &Msg)
{
   if (Msg.Msg == WM_PAINT)
   {
      RepaintDDrawWindow(imgObj, dds, dds2Nav, ddcNav);
      Msg.Result = 0;
   }
   else
   {
      OldNavWndProc(Msg);
   }
}
//---------------------------------------------------------------------------
void __fastcall TfrmEnv::NewInvWndProc(TMessage &Msg)
{
   if (Msg.Msg == WM_PAINT)
   {
      RepaintDDrawWindow(imgInv, dds, dds2Inv, ddcInv);
      Msg.Result = 0;
   }
   else
   {
      OldInvWndProc(Msg);
   }
}
//---------------------------------------------------------------------------
void __fastcall TfrmEnv::btnAddClick(TObject *Sender)
{
   BYTE ObjType = Navigator.nObjType == inven_ID ? item_ID : Navigator.nObjType;
   pObjSet->AppendChildObject(Inventory.pObj, iLevel, ObjType, Navigator.nSelID);
   ResetInventoryInfo();
   RedrawInventory();
   SetButtonSave(true);
}
//---------------------------------------------------------------------------
void __fastcall TfrmEnv::btnRemoveClick(TObject *Sender)
{
   UndoDelete(false);

   pObjSet->DeleteObject(Inventory.pChildObj[Inventory.nItemNum]);
   ResetInventoryInfo();
   RedrawInventory();
   SetButtonSave(true, true);
}
//---------------------------------------------------------------------------
void __fastcall TfrmEnv::sbINVChange(TObject *Sender)
{
   if (inventoryDraw)
      return;
   Inventory.nInvStartItem = sbINV->Position - objInventarCount; //3
   RedrawInventory();
}
//---------------------------------------------------------------------------
void TfrmEnv::LogSelected(void)
{
   DWORD nObjNum, nLevel, nChildCount;
   BYTE nObjType;
   WORD nObjID, nX, nY;
   BYTE *l_pObj;
   nLevel = 0;
   do
   {
      l_pObj = pObjSet->GetFirstObj(&nObjNum, nLevel);
      nChildCount = 0;
      while (l_pObj)
      {
         if (pObjSet->ObjSelected())
         {
            pObjSet->GetObjType(&nObjType, &nObjID);
            pObjSet->GetHexXY(&nX, &nY);
            pLog->LogX("Object type=" + String(nObjType) + ", ID=" +
                       String(nObjID) + "[" + String(nX) + ", " + String(nY) +
                       "]");
         }
         l_pObj = pObjSet->GetNextObj(&nObjNum, &nChildCount, nLevel);
      }
   } while (++nLevel < pMap->Levels);
}
//---------------------------------------------------------------------------
void __fastcall TfrmEnv::piChangeModeClick(TObject *Sender)
{
   if (SelectMode == SELECT_OBJ) {
      imgMap->Cursor = (TCursor)crMoveCursor;
      frmMDI->btnmove->Down = true;
      SelectMode = MOVE_OBJ;
   } else if (SelectMode == MOVE_OBJ) {
      imgMap->Cursor = (TCursor)crCrossCursor;
      frmMDI->btnselect2->Down = true;
      SelectMode = SELECT_OBJ;
   }
}
//---------------------------------------------------------------------------
void TfrmEnv::DrawBlock(int type)
{
      SetGlobalObjectDir();
      randomObject = NONE_SELECTED;

      switch (type) {
         case obj_blockID:
            Navigator.nSelID = OBJ_blockPID_1;
            Navigator.nObjType = scenery_ID;
            break;
         case obj_thru_blockID:
            Navigator.nSelID = OBJ_blockPID_2;
            Navigator.nObjType = scenery_ID;
            break;
         case SAI_blockID:
            Navigator.nSelID = SAI_blockPID;
            Navigator.nObjType = scenery_ID;
            break;
         case wall_blockID:
            Navigator.nSelID = WALL_blockPID_1;
            Navigator.nObjType = wall_ID;
            break;
         case wall_see_blockID:
            Navigator.nSelID = WALL_blockPID_2;
            Navigator.nObjType = wall_ID;
            break;
         default:
            Navigator.nSelID = NONE_SELECTED;
            drawBlocker = false;
            return;
      }

      LoadObjID(Navigator.nSelID, Navigator.nObjType);

      frmMDI->btnpen2->Down = true;
      frmMDI->btnpen2->Click();

      drawBlocker = true;
}
//---------------------------------------------------------------------------
WORD TfrmEnv::GetFrmID(BYTE nObjType, int nID)
{
   if (nObjType == scenery_ID ) {
      switch (nID) {
         case OBJ_blockPID_1:   // flags
            return 22;
         case OBJ_blockPID_2:
            return 23;
         case EG_blockPID:
            return 915;
         case LIGHT_blockPID:
            return 24;
         case SAI_blockPID:
            return 25;
      }
   } else if (nObjType == wall_ID) {
      switch (nID) {
         case WALL_blockPID_1:   // flags
            return 22;
         case WALL_blockPID_2:
            return 23;
       }
   }
   else if (nObjType == misc_ID && nID == SCROLL_blockPID_2)
            return 12;
   else if (nObjType == intrface_ID && nID == NONE_SELECTED)
            return cursor_hexID;

   return pProSet->pPRO[nObjType][nID].GetFrmID();
}
//---------------------------------------------------------------------------
void TfrmEnv::LoadObjID(int pID, BYTE typeID)
{
   pProSet->LoadPRO(typeID, pID, true);
   WORD nFrmID = GetFrmID(typeID, pID);

   String frmfile = pUtil->GetFRMFileName(typeID, pLstFiles->pFRMlst[typeID]->Strings[nFrmID]);
   pFrmSet->LoadFRM(frmfile, typeID, nFrmID, true);
}
//---------------------------------------------------------------------------
int TfrmEnv::ObjectRandomDraw(int index)
{
   CRandomObj *rndObj = (CRandomObj*)pRndObj->Items[index];
   int pID = rndObj->GetObjectID();

   if (pID == -1) return pID; //exit

   LoadObjID(pID, scenery_ID);
   Navigator.nObjType = scenery_ID;
   Navigator.nSelID = pID;

   return pID;
}
//---------------------------------------------------------------------------
void TfrmEnv::RandomObjState(bool enable)
{
   SetGlobalObjectDir();

   if (enable) {
      randomObject = frmMDI->PresetObj->ItemIndex;
      ObjectRandomDraw(frmMDI->PresetObj->ItemIndex);
      frmMDI->btnpen2->Down = true;
      frmMDI->btnpen2->Click();
   } else {
      if (randomObject == NONE_SELECTED)
         return;
      randomObject = NONE_SELECTED;
      Navigator.nObjType = tabc->TabIndex;
      if (Navigator.cSelectedID != 0)
         Navigator.nSelID = Navigator.cSelectedID;
      else
         Navigator.nSelID = NONE_SELECTED;
   }
}
//---------------------------------------------------------------------------
void __fastcall TfrmEnv::piPickDrawObjectClick(TObject *Sender)
{
   Navigator.nObjType = CurMapObjSelect.nObjType;
   Navigator.nSelID  =  CurMapObjSelect.nSelID;

   frmMDI->btnpen2->Down = true;
   frmMDI->btnpen2->Click();
}
//---------------------------------------------------------------------------
bool TfrmEnv::HexIsBlock(int hexX, int hexY)
{
   DWORD nObjNum, nChildCount = 0;
   WORD objHexX, objHexY;
   bool isBlock = false;

   BYTE *pObj = pObjSet->GetFirstObj(&nObjNum, iLevel);
   while (pObj)
   {
      if (!(pObjSet->GetFlags() & 0x00000010)) { // установлен флаг блокировки
         // проверяем принадлежит ли объект к проверяемым кординатам
         pObjSet->GetHexXY(&objHexX, &objHexY);
         if (objHexX == hexX && objHexY == hexY)
            isBlock = true;
      }
      pObj = pObjSet->GetNextObj(&nObjNum, &nChildCount, iLevel);
   }
   return isBlock;
}
//---------------------------------------------------------------------------
void __fastcall TfrmEnv::lblMMClick(TObject *Sender)
{
      bool visible = !imgMiniMap->Visible;
      if (!visible) {
         MinimapPanel->Height = 20;
         Shape4->Pen->Color = clMaroon;
      } else {

         MinimapPanel->Height = 323;
         Shape4->Pen->Color = 0x789DAF;
      }
      imgMiniMap->Visible = visible;
      Shape2->Visible = visible;

      InvenPanel->Top = 2 + MinimapPanel->BoundsRect.Bottom;
      InvenPanel->Height = shp->BoundsRect.Bottom - InvenPanel->Top;

      pLog->LogX("Attach DDraw inventory surface ...");
      AttachDDraw (imgInv, &dds2Inv, 0);
      objInventarCount = 0;
      ResetInventoryInfo();
      RedrawInventory();
}
//---------------------------------------------------------------------------
void __fastcall TfrmEnv::imgObjDblClick(TObject *Sender)
{
   if (tabc->TabIndex == inven_ID && btnAdd->Enabled)
      btnAddClick(NULL);
}
//---------------------------------------------------------------------------
void TfrmEnv::UndoDelete(bool undo)
{
   String undoFile = pUtil->MapperDir +"\\Undo\\undo.dat";

   if (!undo) {
      undoSelected = (pObjSet->nSelected > 0);

      HANDLE h_out = CreateFile(undoFile.c_str(),
		GENERIC_WRITE,
		FILE_SHARE_WRITE,
		NULL,
		CREATE_ALWAYS,
		FILE_ATTRIBUTE_NORMAL,
		NULL);

      pMap->SaveToFile(h_out);
      pTileSet->SaveToFile(h_out);
      pObjSet->SaveToFile(h_out);
      CloseHandle(h_out);

      undoAvailable = true;
   } else if (undoAvailable) { // undo function
      undoAvailable = false;

      h_map = CreateFile(undoFile.c_str(),
		GENERIC_READ,
		FILE_SHARE_READ,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL);
      if (h_map == INVALID_HANDLE_VALUE) {
         pLog->LogX("Cannot open undofile.");
         return;
      }

      pMap = new CMap(h_map); //passed
      pTileSet = new CTileSet(h_map, pMap->TilesSizeX * pMap->TilesSizeY); //passed
      pObjSet = new CObjSet(h_map); //passed

      pObjSet->nSelected = undoSelected;
      ClearSelection();
   }
}
//---------------------------------------------------------------------------
void __fastcall TfrmEnv::btnRotateLeftClick(TObject *Sender)
{
     RotateSelectedObjects(ROTATE_CCW);
}
//---------------------------------------------------------------------------
void __fastcall TfrmEnv::btnRotateRightClick(TObject *Sender)
{
     RotateSelectedObjects(ROTATE_CW);
}
//---------------------------------------------------------------------------
void TfrmEnv::MouseMiddleUp()
{
     mouseBMiddle = false;
     SelectMode = SelectModeMBM; // Возвращаем режим
     if (Screen->Cursor == crHandTakeCursor)
         Screen->Cursor = crDefault;
}
//---------------------------------------------------------------------------
// Передача управления клавишми
void __fastcall TfrmEnv::KeyControlKeyDown(TObject *Sender, WORD &Key,
      TShiftState Shift)
{
      FormKeyDown(Sender, Key, Shift);
      Key = 0;
}
//---------------------------------------------------------------------------
void __fastcall TfrmEnv::KeyControlKeyUp(TObject *Sender, WORD &Key,
      TShiftState Shift)
{
      FormKeyUp(Sender, Key, Shift);
}
//---------------------------------------------------------------------------
void __fastcall TfrmEnv::piTilesToNavigatorClick(TObject *Sender)
{
   int TileX, TileY;
   unsigned short pID;

   pUtil->GetCursorTile(cursorX, cursorY, WorldX, WorldY, &TileX, &TileY);
   if (SelectMode == DRAW_ROOF)
      pID = pTileSet->GetRoofID(TileX, TileY);
   else
      pID = pTileSet->GetFloorID(TileX, TileY);

   tabc->TabIndex = tile_ID;
   Navigator.nObjType = tile_ID;
   sb->Position = pID;
   Navigator.cScrollPos[Navigator.nObjType] = sb->Position;
   PrepareNavigator(tabc->TabIndex);
   RedrawNavigator(pID);
}
//---------------------------------------------------------------------------
String TfrmEnv::GetMapItemsInfo()
{
   DWORD nObjNum, nChildCount = 0;
   WORD nFrmID, nProID, hexX, hexY;
   BYTE nObjType, nProObjType, *pObj, *pSelObj;
   //int nMapX, nMapY, objX, objY;
   String iList;

   for (int mapLevel = 0; mapLevel < pMap->Levels; mapLevel++)
   {
      iList += "------ Map Level: " + IntToStr(mapLevel) + " ------\r\n";
      pObj = pObjSet->GetFirstObj(&nObjNum, mapLevel);
      while (pObj)
      {
         //pObjSet->GetObjFrm(&nObjType, &nFrmID);
         //DWORD Count = pObjSet->ChildCount();
         pObjSet->GetObjType(&nProObjType, &nProID);

         if (nProObjType == item_ID) {
            DWORD subType = pProSet->GetSubType(nProObjType, nProID);
            if (subType != OSContainer) {
               DWORD nMsgID = pProSet->pPRO[nProObjType][nProID].GetMsgID();
               DWORD count = pObjSet->GetCountInObj();
               iList += (count > 0) ? "x" + String(count) + "\t" : String("Ground\t");
               iList += pMsg->GetMSG(nProObjType, nMsgID) + "\r\n";
            }
         }

      //pObjSet->GetHexXY(&hexX, &hexY);
      //pUtil->GetHexWorldCoord(hexX, hexY, &nMapX, &nMapY);

         pObj = pObjSet->GetNextObj(&nObjNum, &nChildCount, mapLevel);
      }
      iList +=  "\r\n";
   }
   return iList;
}
//---------------------------------------------------------------------------
void __fastcall TfrmEnv::btnItemInfoClick(TObject *Sender)
{
   frmInfo->InfoShow(GetMapItemsInfo());
}
//---------------------------------------------------------------------------
void TfrmEnv::SetGlobalObjectDir(int dir)
{
   globalObjDir = dir;
   lblDir->Caption = dir;
}
//---------------------------------------------------------------------------
void __fastcall TfrmEnv::piUseObjectClick(TObject *Sender)
{
   WORD nFrmID;
   BYTE nObjType = CurMapObjSelect.nObjType;

   if (nObjType == scenery_ID || nObjType == item_ID) {

      pObjSet->SetObj(CurMapObjSelect.pSelObj);
      pObjSet->GetObjFrm(&nObjType, &nFrmID);
      WORD nFrames = pFrmSet->pFRM[nObjType][nFrmID].nFrames;
      if (nFrames == 1)
         return;

      DWORD nType = pObjSet->GetFullType();
      DWORD cFrame = pObjSet->GetObjInfo(CurrentFrame);

      if (cFrame != 0) {
         pObjSet->SetObjInfo(CurrentFrame, 0);
         pObjSet->SetObjInfo(Xoffset, 0);
         pObjSet->SetObjInfo(Yoffset, 0);
         if (nType == OPortal) {
            pObjSet->SetFlags(pProSet->pPRO[nObjType][CurMapObjSelect.nSelID].GetFlags());
            pObjSet->SetObjInfo(22, 0x0);
         }
      } else {
         pObjSet->SetObjInfo(CurrentFrame, nFrames - 1);
         int x = 0, y = 0;
         for (WORD f = 0; f < nFrames; f++)
         {
             x += pFrmSet->pFRM[nObjType][nFrmID].GetFoffX(0, f);
             y += pFrmSet->pFRM[nObjType][nFrmID].GetFoffY(0, f);
         }
         pObjSet->SetObjInfo(Xoffset, x);
         pObjSet->SetObjInfo(Yoffset, y);
         if (nType == OPortal) {
            pObjSet->SetFlags(pObjSet->GetFlags() | 0xA0000010);
            pObjSet->SetObjInfo(22, 0x1);
         }
      }
      RedrawMap(true);
      SetButtonSave(true);
   }
}
//---------------------------------------------------------------------------
int TfrmEnv::GetIndexSubType(BYTE nObjType, int nID)
{
   int nSubType = pProSet->GetSubType(nObjType, nID);

   switch (nObjType)
   {
      case item_ID:
         switch (nSubType)
         {
            case OSArmor:
               return OArmor;
            case OSContainer:
               return OContainer;
            case OSDrug:
               return ODrug;
            case OSWeapon:
               return OWeapon;
            case OSAmmo:
               return OAmmo;
            case OSMiscItem:
               return OMiscItem;
            case OSKey:
               return OKey;
         }
      case scenery_ID:
         switch (nSubType)
         {
            case OSPortal:
               return OPortal;
            case OSStairs:
               return OStairs;
            case OSElevator:
               return OElevator;
            case OSLadderBottom:
               return OLadderBottom;
            case OSLadderTop:
               return OLadderUp;
            case OSGenericScenery:
               return OGenericScenery;
         }
      case misc_ID:
         return (nID >= 16 && nID <= 23) ? OExitGrid : OGenericMisc;
      default:
         return -1;
   }
}
//---------------------------------------------------------------------------
