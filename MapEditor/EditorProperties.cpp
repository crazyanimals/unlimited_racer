#include "stdafx.h"
#include "EditorProperties.h"

globalProperties gProp;


int globalProperties::loadProp(){
	
	CString prefix="type";
	CString postfix="\\normal.grPlate";
	CString number;
	COLORREF _col[4];
	UINT i,i1,i2,i3,i4;
	CString path;
	pictureViewSizeX = 200;
	pictureViewSizeY = 200;

//	mapMaxSizeX = 500;	
//	mapMaxSizeY = 500;
	numberOfTerrainsPlates=13;

	for(i=1; i <= numberOfTerrainsPlates; i++){
		number.Format("%d",i);
		path = prefix + number + postfix;
		types.push_back(path);

	}
	
	for(i = 0; i < MAX_HEIGHT;i++){
		colors[i] = COLOR_GROUND + COLOR_ADD(i);	
	}

	for(i = 0; i < MAX_HEIGHT;i++){
		
		for(i1=0;i1<2;i1++)
			for(i2=0;i2<2;i2++)
				for(i3=0;i3<2;i3++)
					for(i4=0;i4<2;i4++){
						_col[0]= COLOR_GROUND + COLOR_ADD(i1+i);	
						_col[1]= COLOR_GROUND + COLOR_ADD(i2+i);	
						_col[2]= COLOR_GROUND + COLOR_ADD(i3+i);	
						_col[3]= COLOR_GROUND + COLOR_ADD(i4+i);	
						plates[i][i1][i2][i3][i4].Create(32,32,32);
						createInterpolatedImage(_col,plates[i][i1][i2][i3][i4]);

					}
	}

	mExts["cur"]=".cur";
	mExts["dlgdef"]=".dlgDef";
	mExts["dlgdec"]=".dlgDecl";
	mExts["icon"]=".*";
	mExts["iconbmp"]=".bmp";
	mExts["font"]=".font";
	mExts["light"]=".light";
	mExts["lodmap"]=".lodmap";
	mExts["urmap"]=".urmap";
	mExts["terrain"]=".terrain";
	mExts["x"]=".x";
	mExts["grass"]=".grass";
	mExts["shrub"]=".shrub";
	mExts["modeldef"]=".modelDef";
	mExts["object"]=".object";
	mExts["car"]=".car";
	mExts["phtexture"]=".phTexture";
	mExts["sk"]=".sk";
	mExts["textpng"]=".png";
	mExts["textjpg"]=".jpg";
	mExts["textbmp"]=".bmp";
	mExts["textset"]=".textset";
	mExts["dhlp"]=".dhlp";
	mExts["text"]=".*";

	mPaths["cur"]=CONFIG_CURSORSPATH;
	mPaths["dlgdef"]=CONFIG_DIALOGSPATH;
	mPaths["dlgdec"]=CONFIG_DIALOGSPATH;
	mPaths["icon"]=CONFIG_EDITORPATH;
	mPaths["iconbmp"]=CONFIG_EDITORPATH;
	mPaths["font"]=CONFIG_FONTSPATH;
	mPaths["light"]=CONFIG_LIGHTSPATH;
	mPaths["lodmap"]=CONFIG_LODDEFINITIONSPATH;
	mPaths["map"]=CONFIG_MAPSPATH;
	mPaths["terrain"]=CONFIG_MAPSPATH;
	mPaths["x"]=CONFIG_MODELSPATH;
	mPaths["grass"]=CONFIG_MODELSPATH;
	mPaths["shrub"]=CONFIG_MODELSPATH;
	mPaths["modeldef"]=CONFIG_MODELSPATH;
	mPaths["object"]=CONFIG_OBJECTSPATH;
	mPaths["car"]=CONFIG_OBJECTSPATH;
	mPaths["phtexture"]=CONFIG_PHTEXTURESPATH;
	mPaths["sk"]=CONFIG_SKYSYSTEMPATH;
	mPaths["textpng"]=CONFIG_TEXTURESPATH;
	mPaths["textjpg"]=CONFIG_TEXTURESPATH;
	mPaths["textbmp"]=CONFIG_TEXTURESPATH;
	mPaths["textset"]=CONFIG_TEXTURESETPATH;
	mPaths["dhlp"]=CONFIG_FILESFORMATPATH;
	mPaths["text"]=CONFIG_TEXTURESPATH;
	return 0;
}



void globalProperties::createInterpolatedImage(COLORREF colors[4],CImage &img){
	int i,j,c;
	BYTE col[3][4];
	BYTE vys[3];
	BYTE c1,c2;
	
	for(i=0;i<4;i++)
		for(j=0;j<3;j++){
			col[j][i]=getColor(colors[i],j);
		}

	for(i=0;i<img.GetHeight();i++){
		for(j=0;j<img.GetWidth();j++){
			for(c=0;c<3;c++){
				
				c1=col[c][0] + (BYTE)( (col[c][1] - col[c][0])*(1-(cos(3.14/img.GetWidth() * j)+1)/2 ));//img.GetWidth();
				c2=col[c][3] + (BYTE)( (col[c][2] - col[c][3])*(1-(cos(3.14/img.GetWidth() * j)+1)/2 ));//img.GetWidth();
				vys[c]=c1 + (BYTE)((c2 - c1)* (1-(cos(3.14/img.GetHeight()*i)+1)/2));
				
			}
			img.SetPixel(j,i,RGB(vys[0],vys[1],vys[2]));
		
		}

	}

}
BYTE globalProperties::getColor(COLORREF col, int barva){
	
	if(barva==MY_RED) return GetRValue(col);
	
	if(barva==MY_GREEN) return GetGValue(col);
	
	if(barva==MY_BLUE)return GetBValue(col);
	return 0;
}