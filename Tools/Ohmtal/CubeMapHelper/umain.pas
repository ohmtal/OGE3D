//-----------------------------------------------------------------------------
// Copyright (c) 2023 huehn-software / Ohmtal Game Studio
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.
//-----------------------------------------------------------------------------
unit umain;

{$mode objfpc}{$H+}

interface

uses
  Classes, SysUtils, Forms, Controls, Graphics, Dialogs, ExtCtrls, StdCtrls,
  ExtDlgs, BGRABitmap, BGRABitmapTypes;

type

  { TfmCubeMapTool }

  TfmCubeMapTool = class(TForm)
    Bevel1: TBevel;
    Bevel10: TBevel;
    Bevel2: TBevel;
    Bevel3: TBevel;
    Bevel4: TBevel;
    Bevel5: TBevel;
    Bevel7: TBevel;
    Bevel8: TBevel;
    Bevel9: TBevel;
    btnLoadDML: TButton;
    btnMaterialsSave: TButton;
    btnRotate: TButton;
    btnExportImages: TButton;
    btnMaterials: TButton;
    dlgOpenFile: TOpenDialog;
    Label1: TLabel;
    Label2: TLabel;
    Label3: TLabel;
    Label4: TLabel;
    Label5: TLabel;
    Label6: TLabel;
    Label7: TLabel;
    Label8: TLabel;
    Label9: TLabel;
    memoMaterials: TMemo;
    memoDML: TMemo;
    dlgOpenPicture: TOpenPictureDialog;
    pbBack1: TPaintBox;
    pbFront1: TPaintBox;
    pbLeft: TPaintBox;
    pbBottom: TPaintBox;
    pbBack: TPaintBox;
    pbFront: TPaintBox;
    pbLeft1: TPaintBox;
    pbRight: TPaintBox;
    pbTop: TPaintBox;
    procedure btnExportImagesClick(Sender: TObject);
    procedure btnLoadDMLClick(Sender: TObject);
    procedure btnMaterialsClick(Sender: TObject);
    procedure btnMaterialsSaveClick(Sender: TObject);
    procedure btnRotateClick(Sender: TObject);
    procedure FormCreate(Sender: TObject);
    procedure FormDestroy(Sender: TObject);
    procedure pbDblClick(Sender: TObject);
    procedure pMouseDown(Sender: TObject; Button: TMouseButton;
      Shift: TShiftState; X, Y: Integer);
    procedure pbPaint(Sender: TObject);

  private
    imgArr : array[0..5] of TBGRABitmap;
    imgOK  : array[0..5] of Boolean;
    curPath:String;
    procedure initImgArr;
    procedure freeImgArr;
    function getImageFileName(id:integer):string;
    procedure log(str:string);
    function guessName:string;
  public

  end;

var
  fmCubeMapTool: TfmCubeMapTool;

implementation

{$R *.lfm}

{ TfmCubeMapTool }
procedure TfmCubeMapTool.log(str:string);
begin

//  if (MemDbg.Visible) then
//     MemDbg.Lines.Add(str);
end;



procedure TfmCubeMapTool.initImgArr;
var
  i:integer;
begin
  for  i := 0 to 5 do begin
      imgArr[i] := TBGRABitmap.Create;
      imgOK[i] := false;
  end;



end;
procedure TfmCubeMapTool.freeImgArr;
var
  i:integer;
begin
  for  i := 0 to 5 do
      imgArr[i].free;
end;

function TfmCubeMapTool.getImageFileName(id:integer):string;
var
  baseFile : String;
begin
  result:='';

  if not DirectoryExists(curPath) then
     log('DirectoryExists(curPath) failed!!!' + curPath);

  if DirectoryExists(curPath) and (memoDML.Lines.Count > id) then
  begin
    baseFile := memoDML.Lines[id];

    result := curPath+baseFile+'.png';
    log('looking for file:' + result);
    if FileExists(result) then exit;

    result := curPath+baseFile+'.jpg';
    log('looking for file:' + result);
    if FileExists(result) then exit;

    result := curPath+baseFile+'.bmp'; //?? bmp mhhh
    log('looking for file:' + result);
    if FileExists(result) then exit;

    result:='';
  end;
end;
procedure TfmCubeMapTool.btnLoadDMLClick(Sender: TObject);
var
  i:integer;
  sucCnt: Integer;
  imgFilename:string;
  notEmpty:Boolean;
begin
  if dlgOpenFile.Execute then
  begin
    memoMaterials.Lines.SetText('');
    Cursor:=crHourGlass;

    curPath:='';
    btnLoadDML.Enabled:=false;
    //btnRotate.Enabled:=false;
    //btnExportImages.Enabled:=false;

    if FileExists(dlgOpenFile.FileName) then
    begin
      curPath := dlgOpenFile.InitialDir;
      memoDML.lines.LoadFromFile(dlgOpenFile.FileName);
      caption := 'Working in ' + curPath + ExtractFileName(dlgOpenFile.FileName) + ' Lines:' + IntToStr(memoDML.Lines.Count);
      sucCnt := 0;
      for  i := 0 to 5 do begin
        imgFilename:=getImageFileName(i);
        notEmpty := imgFilename <> '';
        log('Loading image:'+imgFilename+' is empty:' + BoolToStr(notEmpty,true));
        if notEmpty then begin
           imgArr[i].LoadFromFile(imgFilename);
           sucCnt := sucCnt + 1;
           imgOK[i] := true;
        end;
      end;

    end;
    Cursor:=crDefault;
    if sucCnt < 6 then
       ShowMessage('Failed to load all images!');
    //else
    //    btnRotate.Enabled:=true;

    btnLoadDML.Enabled:=true;


    Invalidate;

  end;
end;

procedure TfmCubeMapTool.btnMaterialsClick(Sender: TObject);
var
  aList:TStringList;
  aName:string;
begin
  aName := guessName;
  if (aName = '') then
     aName:= 'ChangeThisName';


  {
  sky.dml 	Direction 	CubeFace-ID 	Rotation
  0 pos_x 	RIGHT 	0 	270
  1 neg_z 	BACK 	2 	180
  2 neg_x 	LEFT 	1 	90
  3 pos_z 	FRONT 	3
  4 neg_y 	UP 	4 	270
  5 pos_y 	DOWN 	5 	270
  cloud1
  cloud2
  cloud3
  }

  aList := TStringList.create();
  aList.Add('new CubemapData('+ aName + 'Cubemap)');
  aList.Add('  {');
  aList.Add('     cubeFace[0] = "./' + aName + '_sky_right.png";');
  aList.Add('     cubeFace[1] = "./' + aName + '_sky_left.png";');
  aList.Add('     cubeFace[2] = "./' + aName + '_sky_back.png";');
  aList.Add('     cubeFace[3] = "./' + aName + '_sky_front.png";');
  aList.Add('     cubeFace[4] = "./' + aName + '_sky_top.png";');
  aList.Add('     cubeFace[5] = "./' + aName + '_sky_bottom.png";');
  aList.Add('  };');
  aList.Add('');

  aList.Add('  singleton Material('+ aName +'SkyMat)');
  aList.Add('  {');
  aList.Add('     cubemap = "'+ aName +'Cubemap";');
  aList.Add('     materialTag0 = "Skies";');
  aList.Add('  };');


  memoMaterials.Lines := aList;

  aList.free;

end;

procedure TfmCubeMapTool.btnMaterialsSaveClick(Sender: TObject);
begin
  if  memoMaterials.Lines.GetText <> '' then
         memoMaterials.Lines.SaveToFile(curPath+'materials.cs')
  else
      ShowMessage('Materials definition is empty!');

end;

procedure TfmCubeMapTool.btnExportImagesClick(Sender: TObject);
var
  i:integer;
  aName:string;
begin
{
sky.dml 	Direction 	CubeFace-ID 	Rotation
0 pos_x 	RIGHT 	0 	270
1 neg_z 	BACK 	2 	180
2 neg_x 	LEFT 	1 	90
3 pos_z 	FRONT 	3
4 neg_y 	UP 	4 	270
5 pos_y 	DOWN 	5 	270
cloud1
cloud2
cloud3
}

  aName := guessName;
  if (aName = '') then
     aName:= 'ChangeThisName';


  for i:=0 to 5 do begin
    case i of
         0:  //RIGHT
             begin
                if (imgOK[i]) then
                   imgArr[i].SaveToFile(curPath+aName + '_sky_right.png');
              end;
         1:  //BACK
             begin
                if (imgOK[i]) then
                   imgArr[i].SaveToFile(curPath+aName + '_sky_back.png');
              end;
         2:  //LEFT
             begin
                if (imgOK[i]) then
                   imgArr[i].SaveToFile(curPath+aName + '_sky_left.png');
              end;
         3:  //FRONT
             begin
                if (imgOK[i]) then
                   imgArr[i].SaveToFile(curPath+aName + '_sky_front.png');
             end;
         4:  //UP | TOP
             begin
                if (imgOK[i]) then
                   imgArr[i].SaveToFile(curPath+aName + '_sky_top.png');
              end;
         5:  //DOWN | Bottom
             begin
                if (imgOK[i]) then
                   imgArr[i].SaveToFile(curPath+aName + '_sky_bottom.png');
              end;
    end;

  end;
 // btnExportImages.Enabled:=false;

end;

procedure TfmCubeMapTool.btnRotateClick(Sender: TObject);
var
  i:integer;
begin
{
sky.dml 	Direction 	CubeFace-ID 	Rotation
0 pos_x 	RIGHT 	0 	270
1 neg_z 	BACK 	2 	180
2 neg_x 	LEFT 	1 	90
3 pos_z 	FRONT 	3
4 neg_y 	UP 	4 	270
5 pos_y 	DOWN 	5 	270
cloud1
cloud2
cloud3
}

  for i:=0 to 5 do begin
    case i of
         0:  //RIGHT
             begin
                if (imgOK[i]) then
                   imgArr[i] := imgArr[i].RotateCCW; //270
              end;
         1:  //BACK
             begin
                if (imgOK[i]) then
                   imgArr[i] := imgArr[i].RotateUD; //180
              end;
         2:  //LEFT
             begin
                if (imgOK[i]) then
                   imgArr[i] := imgArr[i].RotateCW; //90
              end;
         4:  //UP | TOP
             begin
                if (imgOK[i]) then
                   imgArr[i] := imgArr[i].RotateCCW; //270
              end;
         5:  //DOWN | Bottom
             begin
                if (imgOK[i]) then
                   imgArr[i] := imgArr[i].RotateCCW; //270
              end;
    end;

  end;
  //btnRotate.Enabled:=false;
  //btnExportImages.Enabled:=true;
  Invalidate;

end;

procedure TfmCubeMapTool.FormCreate(Sender: TObject);
begin
  initImgArr();

  //tag is based on DML lines
  pbRight.Tag   := 0;
  pbBack.Tag    := 1;
  pbLeft.Tag    := 2;
  pbFront.Tag   := 3;
  pbTop.Tag     := 4;
  pbBottom.Tag  := 5;
  pbLeft1.Tag   := 2;
  pbBack1.Tag   := 1;
  pbFront1.Tag  := 3;

end;

procedure TfmCubeMapTool.FormDestroy(Sender: TObject);
begin
  freeImgArr;
end;


procedure TfmCubeMapTool.pbDblClick(Sender: TObject);
var
  i:integer;
begin
  if dlgOpenPicture.execute then
  begin
    i := (Sender as TPaintBox).Tag;
    curPath:=dlgOpenPicture.InitialDir; //FIXME?!
    if FileExists(dlgOpenPicture.FileName) then begin
       imgArr[i].LoadFromFile(dlgOpenPicture.FileName);
       imgOK[i] := true;
       caption := 'Working in ' + curPath;
       Invalidate;

    end;
  end;

end;

procedure TfmCubeMapTool.pMouseDown(Sender: TObject; Button: TMouseButton;
  Shift: TShiftState; X, Y: Integer);
var
  lTag : Integer;
begin
  lTag := (Sender as TPaintBox).Tag;
  if (Button = mbRight) and imgOK[lTag]  then
  begin
    imgArr[lTag] := imgArr[lTag].RotateCW;
    Invalidate;
  end;

end;

procedure TfmCubeMapTool.pbPaint(Sender: TObject);
var
  lTag : Integer;
  lPb  : TPaintBox;
  lRect: TRect;
  stretched: TBGRABitmap;
begin
  lPb  := (Sender as TPaintBox);
  lTag := lPb.Tag;
  lRect:= lPb.BoundsRect;
  log('painting no:' + inttostr(lTag) + 'OK:' + BoolToStr(imgOK[lTag],true));
  if (imgOK[lTag]) then begin
    // procedure TBGRAWinBitmap.Draw(ACanvas: TCanvas; Rect: TRect; Opaque: boolean);
    // imgArr[lTag].Draw(lPb.Canvas,lRect,True);
    stretched := imgArr[lTag].Resample(lRect.Width, lRect.Height)  as TBGRABitmap;
    if ( lPb =  pbBack1 ) or (lPb = pbFront1 ) then
    begin
      stretched := stretched.RotateUD;
      stretched.Draw(lPb.Canvas,0,0,True);
    end else begin
      stretched.Draw(lPb.Canvas,0,0,True);
    end;
  end;
end;

function TfmCubeMapTool.guessName:string;
var aList:TStringList;
    deli :char;
begin
deli := '/';
{$IFDEF WINDOWS}
deli := '\';
{$ENDIF}
  result := '';
  if (curPath <> '') then
  begin
    aList := TStringList.Create;
    aList.Delimiter := deli;
    aList.StrictDelimiter := false;
    aList.DelimitedText := curPath;

    //MemDbg.Lines := aList;
    log('NAME:' + aList[aList.Count - 2]);

    if (aList.Count > 2) then begin
      result := aList[aList.Count - 2];
    end;
    aList.free;
  end;

end;

end.

