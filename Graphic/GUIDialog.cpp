#include "stdafx.h"
#include "GUIDialog.h"


using namespace graphic;


//////////////////////////////////////////////////////////////////////////////////////////////
//
// simple function called from qsort as sorting criterion
// used for sorting ordered lists of control objects
//
//////////////////////////////////////////////////////////////////////////////////////////////
int SortingCriterion( const void * p1, const void * p2 )
{
	if ( ((CGUIDialog::LPSORTINGSTRUCTURE) p1)->iSortingKey < ((CGUIDialog::LPSORTINGSTRUCTURE) p2)->iSortingKey ) 
		return -1;
	if ( ((CGUIDialog::LPSORTINGSTRUCTURE) p1)->iSortingKey > ((CGUIDialog::LPSORTINGSTRUCTURE) p2)->iSortingKey ) 
		return 1;
	return 0;
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// loads dialog texture declarations from a file
// this method fills decl structure with data loaded from specified file
// it doesn't change all fields), but only those listed in file
//
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT CGUIDialog::LoadTextureDeclaration( LPCTSTR fileName, GUIDIALOGDECLARATION * decl )
{
	HRESULT		hr;

	hr = NAMEVALUEPAIR::Load( fileName, LoadTextureDeclCallBack, decl );
	if ( hr ) ERRORMSG( ERRGENERIC, "CGUIDialog::LoadTextureDeclaration()", "Can't load GUI texture assignment definitons." )
	
	
	return ERRNOERROR;
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// callback function, is called from within LoadTextureDeclaration() to process single texture
// declarant
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CALLBACK CGUIDialog::LoadTextureDeclCallBack( LPNAMEVALUEPAIR pPair )
{
	CString		name = pPair->GetName();
	
	//name.MakeLower();
	
	if ( name == "Dialog.Background.X" ) { ((GUIDIALOGDECLARATION *) pPair->ReservedPointer)->iFilePosX = pPair->GetInt(); return; }
	if ( name == "Dialog.Background.Y" ) { ((GUIDIALOGDECLARATION *) pPair->ReservedPointer)->iFilePosY = pPair->GetInt(); return; }
	if ( name == "Dialog.Background.Width" ) { ((GUIDIALOGDECLARATION *) pPair->ReservedPointer)->iFileWidth = pPair->GetInt(); return; }
	if ( name == "Dialog.Background.Height" ) { ((GUIDIALOGDECLARATION *) pPair->ReservedPointer)->iFileHeight = pPair->GetInt(); return; }

	if ( name == "ToolTip.TopLeft.X" ) { ((GUIDIALOGDECLARATION *) pPair->ReservedPointer)->tpToolTipTopLeft.iPosX = pPair->GetInt(); return; }
	if ( name == "ToolTip.TopLeft.Y" ) { ((GUIDIALOGDECLARATION *) pPair->ReservedPointer)->tpToolTipTopLeft.iPosY = pPair->GetInt(); return; }
	if ( name == "ToolTip.TopLeft.Width" ) { ((GUIDIALOGDECLARATION *) pPair->ReservedPointer)->tpToolTipTopLeft.iWidth = pPair->GetInt(); return; }
	if ( name == "ToolTip.TopLeft.Height" ) { ((GUIDIALOGDECLARATION *) pPair->ReservedPointer)->tpToolTipTopLeft.iHeight = pPair->GetInt(); return; }
	if ( name == "ToolTip.TopRight.X" ) { ((GUIDIALOGDECLARATION *) pPair->ReservedPointer)->tpToolTipTopRight.iPosX = pPair->GetInt(); return; }
	if ( name == "ToolTip.TopRight.Y" ) { ((GUIDIALOGDECLARATION *) pPair->ReservedPointer)->tpToolTipTopRight.iPosY = pPair->GetInt(); return; }
	if ( name == "ToolTip.TopRight.Width" ) { ((GUIDIALOGDECLARATION *) pPair->ReservedPointer)->tpToolTipTopRight.iWidth = pPair->GetInt(); return; }
	if ( name == "ToolTip.TopRight.Height" ) { ((GUIDIALOGDECLARATION *) pPair->ReservedPointer)->tpToolTipTopRight.iHeight = pPair->GetInt(); return; }
	if ( name == "ToolTip.BottomLeft.X" ) { ((GUIDIALOGDECLARATION *) pPair->ReservedPointer)->tpToolTipBottomLeft.iPosX = pPair->GetInt(); return; }
	if ( name == "ToolTip.BottomLeft.Y" ) { ((GUIDIALOGDECLARATION *) pPair->ReservedPointer)->tpToolTipBottomLeft.iPosY = pPair->GetInt(); return; }
	if ( name == "ToolTip.BottomLeft.Width" ) { ((GUIDIALOGDECLARATION *) pPair->ReservedPointer)->tpToolTipBottomLeft.iWidth = pPair->GetInt(); return; }
	if ( name == "ToolTip.BottomLeft.Height" ) { ((GUIDIALOGDECLARATION *) pPair->ReservedPointer)->tpToolTipBottomLeft.iHeight = pPair->GetInt(); return; }
	if ( name == "ToolTip.BottomRight.X" ) { ((GUIDIALOGDECLARATION *) pPair->ReservedPointer)->tpToolTipBottomRight.iPosX = pPair->GetInt(); return; }
	if ( name == "ToolTip.BottomRight.Y" ) { ((GUIDIALOGDECLARATION *) pPair->ReservedPointer)->tpToolTipBottomRight.iPosY = pPair->GetInt(); return; }
	if ( name == "ToolTip.BottomRight.Width" ) { ((GUIDIALOGDECLARATION *) pPair->ReservedPointer)->tpToolTipBottomRight.iWidth = pPair->GetInt(); return; }
	if ( name == "ToolTip.BottomRight.Height" ) { ((GUIDIALOGDECLARATION *) pPair->ReservedPointer)->tpToolTipBottomRight.iHeight = pPair->GetInt(); return; }
	if ( name == "ToolTip.HorizLine.X" ) { ((GUIDIALOGDECLARATION *) pPair->ReservedPointer)->tpToolTipHorizLine.iPosX = pPair->GetInt(); return; }
	if ( name == "ToolTip.HorizLine.Y" ) { ((GUIDIALOGDECLARATION *) pPair->ReservedPointer)->tpToolTipHorizLine.iPosY = pPair->GetInt(); return; }
	if ( name == "ToolTip.HorizLine.Width" ) { ((GUIDIALOGDECLARATION *) pPair->ReservedPointer)->tpToolTipHorizLine.iWidth = pPair->GetInt(); return; }
	if ( name == "ToolTip.HorizLine.Height" ) { ((GUIDIALOGDECLARATION *) pPair->ReservedPointer)->tpToolTipHorizLine.iHeight = pPair->GetInt(); return; }
	if ( name == "ToolTip.VertLine.X" ) { ((GUIDIALOGDECLARATION *) pPair->ReservedPointer)->tpToolTipVertLine.iPosX = pPair->GetInt(); return; }
	if ( name == "ToolTip.VertLine.Y" ) { ((GUIDIALOGDECLARATION *) pPair->ReservedPointer)->tpToolTipVertLine.iPosY = pPair->GetInt(); return; }
	if ( name == "ToolTip.VertLine.Width" ) { ((GUIDIALOGDECLARATION *) pPair->ReservedPointer)->tpToolTipVertLine.iWidth = pPair->GetInt(); return; }
	if ( name == "ToolTip.VertLine.Height" ) { ((GUIDIALOGDECLARATION *) pPair->ReservedPointer)->tpToolTipVertLine.iHeight = pPair->GetInt(); return; }
	if ( name == "ToolTip.Background.X" ) { ((GUIDIALOGDECLARATION *) pPair->ReservedPointer)->tpToolTipBackground.iPosX = pPair->GetInt(); return; }
	if ( name == "ToolTip.Background.Y" ) { ((GUIDIALOGDECLARATION *) pPair->ReservedPointer)->tpToolTipBackground.iPosY = pPair->GetInt(); return; }
	if ( name == "ToolTip.Background.Width" ) { ((GUIDIALOGDECLARATION *) pPair->ReservedPointer)->tpToolTipBackground.iWidth = pPair->GetInt(); return; }
	if ( name == "ToolTip.Background.Height" ) { ((GUIDIALOGDECLARATION *) pPair->ReservedPointer)->tpToolTipBackground.iHeight = pPair->GetInt(); return; }

	if ( name == "Image.Background.X" ) { ((GUIDIALOGDECLARATION *) pPair->ReservedPointer)->tpImageBackground.iPosX = pPair->GetInt(); return; }
	if ( name == "Image.Background.Y" ) { ((GUIDIALOGDECLARATION *) pPair->ReservedPointer)->tpImageBackground.iPosY = pPair->GetInt(); return; }
	if ( name == "Image.Background.Width" ) { ((GUIDIALOGDECLARATION *) pPair->ReservedPointer)->tpImageBackground.iWidth = pPair->GetInt(); return; }
	if ( name == "Image.Background.Height" ) { ((GUIDIALOGDECLARATION *) pPair->ReservedPointer)->tpImageBackground.iHeight = pPair->GetInt(); return; }

	if ( name == "Button.Up.X" ) { ((GUIDIALOGDECLARATION *) pPair->ReservedPointer)->tpButtonUp.iPosX = pPair->GetInt(); return; }
	if ( name == "Button.Up.Y" ) { ((GUIDIALOGDECLARATION *) pPair->ReservedPointer)->tpButtonUp.iPosY = pPair->GetInt(); return; }
	if ( name == "Button.Up.Width" ) { ((GUIDIALOGDECLARATION *) pPair->ReservedPointer)->tpButtonUp.iWidth = pPair->GetInt(); return; }
	if ( name == "Button.Up.Height" ) { ((GUIDIALOGDECLARATION *) pPair->ReservedPointer)->tpButtonUp.iHeight = pPair->GetInt(); return; }
	if ( name == "Button.Down.X" ) { ((GUIDIALOGDECLARATION *) pPair->ReservedPointer)->tpButtonDown.iPosX = pPair->GetInt(); return; }
	if ( name == "Button.Down.Y" ) { ((GUIDIALOGDECLARATION *) pPair->ReservedPointer)->tpButtonDown.iPosY = pPair->GetInt(); return; }
	if ( name == "Button.Down.Width" ) { ((GUIDIALOGDECLARATION *) pPair->ReservedPointer)->tpButtonDown.iWidth = pPair->GetInt(); return; }
	if ( name == "Button.Down.Height" ) { ((GUIDIALOGDECLARATION *) pPair->ReservedPointer)->tpButtonDown.iHeight = pPair->GetInt(); return; }

	if ( name == "CheckBox.Unchecked.X" ) { ((GUIDIALOGDECLARATION *) pPair->ReservedPointer)->tpCheckBoxUnchecked.iPosX = pPair->GetInt(); return; }
	if ( name == "CheckBox.Unchecked.Y" ) { ((GUIDIALOGDECLARATION *) pPair->ReservedPointer)->tpCheckBoxUnchecked.iPosY = pPair->GetInt(); return; }
	if ( name == "CheckBox.Unchecked.Width" ) { ((GUIDIALOGDECLARATION *) pPair->ReservedPointer)->tpCheckBoxUnchecked.iWidth = pPair->GetInt(); return; }
	if ( name == "CheckBox.Unchecked.Height" ) { ((GUIDIALOGDECLARATION *) pPair->ReservedPointer)->tpCheckBoxUnchecked.iHeight = pPair->GetInt(); return; }
	if ( name == "CheckBox.Checked.X" ) { ((GUIDIALOGDECLARATION *) pPair->ReservedPointer)->tpCheckBoxChecked.iPosX = pPair->GetInt(); return; }
	if ( name == "CheckBox.Checked.Y" ) { ((GUIDIALOGDECLARATION *) pPair->ReservedPointer)->tpCheckBoxChecked.iPosY = pPair->GetInt(); return; }
	if ( name == "CheckBox.Checked.Width" ) { ((GUIDIALOGDECLARATION *) pPair->ReservedPointer)->tpCheckBoxChecked.iWidth = pPair->GetInt(); return; }
	if ( name == "CheckBox.Checked.Height" ) { ((GUIDIALOGDECLARATION *) pPair->ReservedPointer)->tpCheckBoxChecked.iHeight = pPair->GetInt(); return; }

	if ( name == "TrackBar.Track.X" ) { ((GUIDIALOGDECLARATION *) pPair->ReservedPointer)->tpTrackBarTrack.iPosX = pPair->GetInt(); return; }
	if ( name == "TrackBar.Track.Y" ) { ((GUIDIALOGDECLARATION *) pPair->ReservedPointer)->tpTrackBarTrack.iPosY = pPair->GetInt(); return; }
	if ( name == "TrackBar.Track.Width" ) { ((GUIDIALOGDECLARATION *) pPair->ReservedPointer)->tpTrackBarTrack.iWidth = pPair->GetInt(); return; }
	if ( name == "TrackBar.Track.Height" ) { ((GUIDIALOGDECLARATION *) pPair->ReservedPointer)->tpTrackBarTrack.iHeight = pPair->GetInt(); return; }
	if ( name == "TrackBar.Slider.X" ) { ((GUIDIALOGDECLARATION *) pPair->ReservedPointer)->tpTrackBarSlider.iPosX = pPair->GetInt(); return; }
	if ( name == "TrackBar.Slider.Y" ) { ((GUIDIALOGDECLARATION *) pPair->ReservedPointer)->tpTrackBarSlider.iPosY = pPair->GetInt(); return; }
	if ( name == "TrackBar.Slider.Width" ) { ((GUIDIALOGDECLARATION *) pPair->ReservedPointer)->tpTrackBarSlider.iWidth = pPair->GetInt(); return; }
	if ( name == "TrackBar.Slider.Height" ) { ((GUIDIALOGDECLARATION *) pPair->ReservedPointer)->tpTrackBarSlider.iHeight = pPair->GetInt(); return; }

	if ( name == "RadioButton.Unselected.X" ) { ((GUIDIALOGDECLARATION *) pPair->ReservedPointer)->tpRadioButtonUnselected.iPosX = pPair->GetInt(); return; }
	if ( name == "RadioButton.Unselected.Y" ) { ((GUIDIALOGDECLARATION *) pPair->ReservedPointer)->tpRadioButtonUnselected.iPosY = pPair->GetInt(); return; }
	if ( name == "RadioButton.Unselected.Width" ) { ((GUIDIALOGDECLARATION *) pPair->ReservedPointer)->tpRadioButtonUnselected.iWidth = pPair->GetInt(); return; }
	if ( name == "RadioButton.Unselected.Height" ) { ((GUIDIALOGDECLARATION *) pPair->ReservedPointer)->tpRadioButtonUnselected.iHeight = pPair->GetInt(); return; }
	if ( name == "RadioButton.Selected.X" ) { ((GUIDIALOGDECLARATION *) pPair->ReservedPointer)->tpRadioButtonSelected.iPosX = pPair->GetInt(); return; }
	if ( name == "RadioButton.Selected.Y" ) { ((GUIDIALOGDECLARATION *) pPair->ReservedPointer)->tpRadioButtonSelected.iPosY = pPair->GetInt(); return; }
	if ( name == "RadioButton.Selected.Width" ) { ((GUIDIALOGDECLARATION *) pPair->ReservedPointer)->tpRadioButtonSelected.iWidth = pPair->GetInt(); return; }
	if ( name == "RadioButton.Selected.Height" ) { ((GUIDIALOGDECLARATION *) pPair->ReservedPointer)->tpRadioButtonSelected.iHeight = pPair->GetInt(); return; }

	if ( name == "Edit.TopLeft.X" ) { ((GUIDIALOGDECLARATION *) pPair->ReservedPointer)->tpEditTopLeft.iPosX = pPair->GetInt(); return; }
	if ( name == "Edit.TopLeft.Y" ) { ((GUIDIALOGDECLARATION *) pPair->ReservedPointer)->tpEditTopLeft.iPosY = pPair->GetInt(); return; }
	if ( name == "Edit.TopLeft.Width" ) { ((GUIDIALOGDECLARATION *) pPair->ReservedPointer)->tpEditTopLeft.iWidth = pPair->GetInt(); return; }
	if ( name == "Edit.TopLeft.Height" ) { ((GUIDIALOGDECLARATION *) pPair->ReservedPointer)->tpEditTopLeft.iHeight = pPair->GetInt(); return; }
	if ( name == "Edit.TopRight.X" ) { ((GUIDIALOGDECLARATION *) pPair->ReservedPointer)->tpEditTopRight.iPosX = pPair->GetInt(); return; }
	if ( name == "Edit.TopRight.Y" ) { ((GUIDIALOGDECLARATION *) pPair->ReservedPointer)->tpEditTopRight.iPosY = pPair->GetInt(); return; }
	if ( name == "Edit.TopRight.Width" ) { ((GUIDIALOGDECLARATION *) pPair->ReservedPointer)->tpEditTopRight.iWidth = pPair->GetInt(); return; }
	if ( name == "Edit.TopRight.Height" ) { ((GUIDIALOGDECLARATION *) pPair->ReservedPointer)->tpEditTopRight.iHeight = pPair->GetInt(); return; }
	if ( name == "Edit.BottomLeft.X" ) { ((GUIDIALOGDECLARATION *) pPair->ReservedPointer)->tpEditBottomLeft.iPosX = pPair->GetInt(); return; }
	if ( name == "Edit.BottomLeft.Y" ) { ((GUIDIALOGDECLARATION *) pPair->ReservedPointer)->tpEditBottomLeft.iPosY = pPair->GetInt(); return; }
	if ( name == "Edit.BottomLeft.Width" ) { ((GUIDIALOGDECLARATION *) pPair->ReservedPointer)->tpEditBottomLeft.iWidth = pPair->GetInt(); return; }
	if ( name == "Edit.BottomLeft.Height" ) { ((GUIDIALOGDECLARATION *) pPair->ReservedPointer)->tpEditBottomLeft.iHeight = pPair->GetInt(); return; }
	if ( name == "Edit.BottomRight.X" ) { ((GUIDIALOGDECLARATION *) pPair->ReservedPointer)->tpEditBottomRight.iPosX = pPair->GetInt(); return; }
	if ( name == "Edit.BottomRight.Y" ) { ((GUIDIALOGDECLARATION *) pPair->ReservedPointer)->tpEditBottomRight.iPosY = pPair->GetInt(); return; }
	if ( name == "Edit.BottomRight.Width" ) { ((GUIDIALOGDECLARATION *) pPair->ReservedPointer)->tpEditBottomRight.iWidth = pPair->GetInt(); return; }
	if ( name == "Edit.BottomRight.Height" ) { ((GUIDIALOGDECLARATION *) pPair->ReservedPointer)->tpEditBottomRight.iHeight = pPair->GetInt(); return; }
	if ( name == "Edit.HorizLine.X" ) { ((GUIDIALOGDECLARATION *) pPair->ReservedPointer)->tpEditHorizLine.iPosX = pPair->GetInt(); return; }
	if ( name == "Edit.HorizLine.Y" ) { ((GUIDIALOGDECLARATION *) pPair->ReservedPointer)->tpEditHorizLine.iPosY = pPair->GetInt(); return; }
	if ( name == "Edit.HorizLine.Width" ) { ((GUIDIALOGDECLARATION *) pPair->ReservedPointer)->tpEditHorizLine.iWidth = pPair->GetInt(); return; }
	if ( name == "Edit.HorizLine.Height" ) { ((GUIDIALOGDECLARATION *) pPair->ReservedPointer)->tpEditHorizLine.iHeight = pPair->GetInt(); return; }
	if ( name == "Edit.VertLine.X" ) { ((GUIDIALOGDECLARATION *) pPair->ReservedPointer)->tpEditVertLine.iPosX = pPair->GetInt(); return; }
	if ( name == "Edit.VertLine.Y" ) { ((GUIDIALOGDECLARATION *) pPair->ReservedPointer)->tpEditVertLine.iPosY = pPair->GetInt(); return; }
	if ( name == "Edit.VertLine.Width" ) { ((GUIDIALOGDECLARATION *) pPair->ReservedPointer)->tpEditVertLine.iWidth = pPair->GetInt(); return; }
	if ( name == "Edit.VertLine.Height" ) { ((GUIDIALOGDECLARATION *) pPair->ReservedPointer)->tpEditVertLine.iHeight = pPair->GetInt(); return; }
	if ( name == "Edit.Background.X" ) { ((GUIDIALOGDECLARATION *) pPair->ReservedPointer)->tpEditBackground.iPosX = pPair->GetInt(); return; }
	if ( name == "Edit.Background.Y" ) { ((GUIDIALOGDECLARATION *) pPair->ReservedPointer)->tpEditBackground.iPosY = pPair->GetInt(); return; }
	if ( name == "Edit.Background.Width" ) { ((GUIDIALOGDECLARATION *) pPair->ReservedPointer)->tpEditBackground.iWidth = pPair->GetInt(); return; }
	if ( name == "Edit.Background.Height" ) { ((GUIDIALOGDECLARATION *) pPair->ReservedPointer)->tpEditBackground.iHeight = pPair->GetInt(); return; }

	if ( name == "Combo.Button.X" ) { ((GUIDIALOGDECLARATION *) pPair->ReservedPointer)->tpComboButton.iPosX = pPair->GetInt(); return; }
	if ( name == "Combo.Button.Y" ) { ((GUIDIALOGDECLARATION *) pPair->ReservedPointer)->tpComboButton.iPosY = pPair->GetInt(); return; }
	if ( name == "Combo.Button.Width" ) { ((GUIDIALOGDECLARATION *) pPair->ReservedPointer)->tpComboButton.iWidth = pPair->GetInt(); return; }
	if ( name == "Combo.Button.Height" ) { ((GUIDIALOGDECLARATION *) pPair->ReservedPointer)->tpComboButton.iHeight = pPair->GetInt(); return; }
	if ( name == "Combo.Background.X" ) { ((GUIDIALOGDECLARATION *) pPair->ReservedPointer)->tpComboBackground.iPosX = pPair->GetInt(); return; }
	if ( name == "Combo.Background.Y" ) { ((GUIDIALOGDECLARATION *) pPair->ReservedPointer)->tpComboBackground.iPosY = pPair->GetInt(); return; }
	if ( name == "Combo.Background.Width" ) { ((GUIDIALOGDECLARATION *) pPair->ReservedPointer)->tpComboBackground.iWidth = pPair->GetInt(); return; }
	if ( name == "Combo.Background.Height" ) { ((GUIDIALOGDECLARATION *) pPair->ReservedPointer)->tpComboBackground.iHeight = pPair->GetInt(); return; }
	if ( name == "Combo.ActiveBackground.X" ) { ((GUIDIALOGDECLARATION *) pPair->ReservedPointer)->tpComboActiveBackground.iPosX = pPair->GetInt(); return; }
	if ( name == "Combo.ActiveBackground.Y" ) { ((GUIDIALOGDECLARATION *) pPair->ReservedPointer)->tpComboActiveBackground.iPosY = pPair->GetInt(); return; }
	if ( name == "Combo.ActiveBackground.Width" ) { ((GUIDIALOGDECLARATION *) pPair->ReservedPointer)->tpComboActiveBackground.iWidth = pPair->GetInt(); return; }
	if ( name == "Combo.ActiveBackground.Height" ) { ((GUIDIALOGDECLARATION *) pPair->ReservedPointer)->tpComboActiveBackground.iHeight = pPair->GetInt(); return; }
	if ( name == "Combo.TopLeft.X" ) { ((GUIDIALOGDECLARATION *) pPair->ReservedPointer)->tpComboTopLeft.iPosX = pPair->GetInt(); return; }
	if ( name == "Combo.TopLeft.Y" ) { ((GUIDIALOGDECLARATION *) pPair->ReservedPointer)->tpComboTopLeft.iPosY = pPair->GetInt(); return; }
	if ( name == "Combo.TopLeft.Width" ) { ((GUIDIALOGDECLARATION *) pPair->ReservedPointer)->tpComboTopLeft.iWidth = pPair->GetInt(); return; }
	if ( name == "Combo.TopLeft.Height" ) { ((GUIDIALOGDECLARATION *) pPair->ReservedPointer)->tpComboTopLeft.iHeight = pPair->GetInt(); return; }
	if ( name == "Combo.TopRight.X" ) { ((GUIDIALOGDECLARATION *) pPair->ReservedPointer)->tpComboTopRight.iPosX = pPair->GetInt(); return; }
	if ( name == "Combo.TopRight.Y" ) { ((GUIDIALOGDECLARATION *) pPair->ReservedPointer)->tpComboTopRight.iPosY = pPair->GetInt(); return; }
	if ( name == "Combo.TopRight.Width" ) { ((GUIDIALOGDECLARATION *) pPair->ReservedPointer)->tpComboTopRight.iWidth = pPair->GetInt(); return; }
	if ( name == "Combo.TopRight.Height" ) { ((GUIDIALOGDECLARATION *) pPair->ReservedPointer)->tpComboTopRight.iHeight = pPair->GetInt(); return; }
	if ( name == "Combo.BottomLeft.X" ) { ((GUIDIALOGDECLARATION *) pPair->ReservedPointer)->tpComboBottomLeft.iPosX = pPair->GetInt(); return; }
	if ( name == "Combo.BottomLeft.Y" ) { ((GUIDIALOGDECLARATION *) pPair->ReservedPointer)->tpComboBottomLeft.iPosY = pPair->GetInt(); return; }
	if ( name == "Combo.BottomLeft.Width" ) { ((GUIDIALOGDECLARATION *) pPair->ReservedPointer)->tpComboBottomLeft.iWidth = pPair->GetInt(); return; }
	if ( name == "Combo.BottomLeft.Height" ) { ((GUIDIALOGDECLARATION *) pPair->ReservedPointer)->tpComboBottomLeft.iHeight = pPair->GetInt(); return; }
	if ( name == "Combo.BottomRight.X" ) { ((GUIDIALOGDECLARATION *) pPair->ReservedPointer)->tpComboBottomRight.iPosX = pPair->GetInt(); return; }
	if ( name == "Combo.BottomRight.Y" ) { ((GUIDIALOGDECLARATION *) pPair->ReservedPointer)->tpComboBottomRight.iPosY = pPair->GetInt(); return; }
	if ( name == "Combo.BottomRight.Width" ) { ((GUIDIALOGDECLARATION *) pPair->ReservedPointer)->tpComboBottomRight.iWidth = pPair->GetInt(); return; }
	if ( name == "Combo.BottomRight.Height" ) { ((GUIDIALOGDECLARATION *) pPair->ReservedPointer)->tpComboBottomRight.iHeight = pPair->GetInt(); return; }
	if ( name == "Combo.HorizLine.X" ) { ((GUIDIALOGDECLARATION *) pPair->ReservedPointer)->tpComboHorizLine.iPosX = pPair->GetInt(); return; }
	if ( name == "Combo.HorizLine.Y" ) { ((GUIDIALOGDECLARATION *) pPair->ReservedPointer)->tpComboHorizLine.iPosY = pPair->GetInt(); return; }
	if ( name == "Combo.HorizLine.Width" ) { ((GUIDIALOGDECLARATION *) pPair->ReservedPointer)->tpComboHorizLine.iWidth = pPair->GetInt(); return; }
	if ( name == "Combo.HorizLine.Height" ) { ((GUIDIALOGDECLARATION *) pPair->ReservedPointer)->tpComboHorizLine.iHeight = pPair->GetInt(); return; }
	if ( name == "Combo.VertLine.X" ) { ((GUIDIALOGDECLARATION *) pPair->ReservedPointer)->tpComboVertLine.iPosX = pPair->GetInt(); return; }
	if ( name == "Combo.VertLine.Y" ) { ((GUIDIALOGDECLARATION *) pPair->ReservedPointer)->tpComboVertLine.iPosY = pPair->GetInt(); return; }
	if ( name == "Combo.VertLine.Width" ) { ((GUIDIALOGDECLARATION *) pPair->ReservedPointer)->tpComboVertLine.iWidth = pPair->GetInt(); return; }
	if ( name == "Combo.VertLine.Height" ) { ((GUIDIALOGDECLARATION *) pPair->ReservedPointer)->tpComboVertLine.iHeight = pPair->GetInt(); return; }

	if ( name == "Popup.Background.X" ) { ((GUIDIALOGDECLARATION *) pPair->ReservedPointer)->tpPopupBackground.iPosX = pPair->GetInt(); return; }
	if ( name == "Popup.Background.Y" ) { ((GUIDIALOGDECLARATION *) pPair->ReservedPointer)->tpPopupBackground.iPosY = pPair->GetInt(); return; }
	if ( name == "Popup.Background.Width" ) { ((GUIDIALOGDECLARATION *) pPair->ReservedPointer)->tpPopupBackground.iWidth = pPair->GetInt(); return; }
	if ( name == "Popup.Background.Height" ) { ((GUIDIALOGDECLARATION *) pPair->ReservedPointer)->tpPopupBackground.iHeight = pPair->GetInt(); return; }
	if ( name == "Popup.ActiveBackground.X" ) { ((GUIDIALOGDECLARATION *) pPair->ReservedPointer)->tpPopupActiveBackground.iPosX = pPair->GetInt(); return; }
	if ( name == "Popup.ActiveBackground.Y" ) { ((GUIDIALOGDECLARATION *) pPair->ReservedPointer)->tpPopupActiveBackground.iPosY = pPair->GetInt(); return; }
	if ( name == "Popup.ActiveBackground.Width" ) { ((GUIDIALOGDECLARATION *) pPair->ReservedPointer)->tpPopupActiveBackground.iWidth = pPair->GetInt(); return; }
	if ( name == "Popup.ActiveBackground.Height" ) { ((GUIDIALOGDECLARATION *) pPair->ReservedPointer)->tpPopupActiveBackground.iHeight = pPair->GetInt(); return; }
	if ( name == "Popup.TopLeft.X" ) { ((GUIDIALOGDECLARATION *) pPair->ReservedPointer)->tpPopupTopLeft.iPosX = pPair->GetInt(); return; }
	if ( name == "Popup.TopLeft.Y" ) { ((GUIDIALOGDECLARATION *) pPair->ReservedPointer)->tpPopupTopLeft.iPosY = pPair->GetInt(); return; }
	if ( name == "Popup.TopLeft.Width" ) { ((GUIDIALOGDECLARATION *) pPair->ReservedPointer)->tpPopupTopLeft.iWidth = pPair->GetInt(); return; }
	if ( name == "Popup.TopLeft.Height" ) { ((GUIDIALOGDECLARATION *) pPair->ReservedPointer)->tpPopupTopLeft.iHeight = pPair->GetInt(); return; }
	if ( name == "Popup.TopRight.X" ) { ((GUIDIALOGDECLARATION *) pPair->ReservedPointer)->tpPopupTopRight.iPosX = pPair->GetInt(); return; }
	if ( name == "Popup.TopRight.Y" ) { ((GUIDIALOGDECLARATION *) pPair->ReservedPointer)->tpPopupTopRight.iPosY = pPair->GetInt(); return; }
	if ( name == "Popup.TopRight.Width" ) { ((GUIDIALOGDECLARATION *) pPair->ReservedPointer)->tpPopupTopRight.iWidth = pPair->GetInt(); return; }
	if ( name == "Popup.TopRight.Height" ) { ((GUIDIALOGDECLARATION *) pPair->ReservedPointer)->tpPopupTopRight.iHeight = pPair->GetInt(); return; }
	if ( name == "Popup.BottomLeft.X" ) { ((GUIDIALOGDECLARATION *) pPair->ReservedPointer)->tpPopupBottomLeft.iPosX = pPair->GetInt(); return; }
	if ( name == "Popup.BottomLeft.Y" ) { ((GUIDIALOGDECLARATION *) pPair->ReservedPointer)->tpPopupBottomLeft.iPosY = pPair->GetInt(); return; }
	if ( name == "Popup.BottomLeft.Width" ) { ((GUIDIALOGDECLARATION *) pPair->ReservedPointer)->tpPopupBottomLeft.iWidth = pPair->GetInt(); return; }
	if ( name == "Popup.BottomLeft.Height" ) { ((GUIDIALOGDECLARATION *) pPair->ReservedPointer)->tpPopupBottomLeft.iHeight = pPair->GetInt(); return; }
	if ( name == "Popup.BottomRight.X" ) { ((GUIDIALOGDECLARATION *) pPair->ReservedPointer)->tpPopupBottomRight.iPosX = pPair->GetInt(); return; }
	if ( name == "Popup.BottomRight.Y" ) { ((GUIDIALOGDECLARATION *) pPair->ReservedPointer)->tpPopupBottomRight.iPosY = pPair->GetInt(); return; }
	if ( name == "Popup.BottomRight.Width" ) { ((GUIDIALOGDECLARATION *) pPair->ReservedPointer)->tpPopupBottomRight.iWidth = pPair->GetInt(); return; }
	if ( name == "Popup.BottomRight.Height" ) { ((GUIDIALOGDECLARATION *) pPair->ReservedPointer)->tpPopupBottomRight.iHeight = pPair->GetInt(); return; }
	if ( name == "Popup.HorizLine.X" ) { ((GUIDIALOGDECLARATION *) pPair->ReservedPointer)->tpPopupHorizLine.iPosX = pPair->GetInt(); return; }
	if ( name == "Popup.HorizLine.Y" ) { ((GUIDIALOGDECLARATION *) pPair->ReservedPointer)->tpPopupHorizLine.iPosY = pPair->GetInt(); return; }
	if ( name == "Popup.HorizLine.Width" ) { ((GUIDIALOGDECLARATION *) pPair->ReservedPointer)->tpPopupHorizLine.iWidth = pPair->GetInt(); return; }
	if ( name == "Popup.HorizLine.Height" ) { ((GUIDIALOGDECLARATION *) pPair->ReservedPointer)->tpPopupHorizLine.iHeight = pPair->GetInt(); return; }
	if ( name == "Popup.VertLine.X" ) { ((GUIDIALOGDECLARATION *) pPair->ReservedPointer)->tpPopupVertLine.iPosX = pPair->GetInt(); return; }
	if ( name == "Popup.VertLine.Y" ) { ((GUIDIALOGDECLARATION *) pPair->ReservedPointer)->tpPopupVertLine.iPosY = pPair->GetInt(); return; }
	if ( name == "Popup.VertLine.Width" ) { ((GUIDIALOGDECLARATION *) pPair->ReservedPointer)->tpPopupVertLine.iWidth = pPair->GetInt(); return; }
	if ( name == "Popup.VertLine.Height" ) { ((GUIDIALOGDECLARATION *) pPair->ReservedPointer)->tpPopupVertLine.iHeight = pPair->GetInt(); return; }

	if ( name == "ListBox.ButtonUp.X" ) { ((GUIDIALOGDECLARATION *) pPair->ReservedPointer)->tpListBoxButtonUp.iPosX = pPair->GetInt(); return; }
	if ( name == "ListBox.ButtonUp.Y" ) { ((GUIDIALOGDECLARATION *) pPair->ReservedPointer)->tpListBoxButtonUp.iPosY = pPair->GetInt(); return; }
	if ( name == "ListBox.ButtonUp.Width" ) { ((GUIDIALOGDECLARATION *) pPair->ReservedPointer)->tpListBoxButtonUp.iWidth = pPair->GetInt(); return; }
	if ( name == "ListBox.ButtonUp.Height" ) { ((GUIDIALOGDECLARATION *) pPair->ReservedPointer)->tpListBoxButtonUp.iHeight = pPair->GetInt(); return; }
	if ( name == "ListBox.ButtonDown.X" ) { ((GUIDIALOGDECLARATION *) pPair->ReservedPointer)->tpListBoxButtonDown.iPosX = pPair->GetInt(); return; }
	if ( name == "ListBox.ButtonDown.Y" ) { ((GUIDIALOGDECLARATION *) pPair->ReservedPointer)->tpListBoxButtonDown.iPosY = pPair->GetInt(); return; }
	if ( name == "ListBox.ButtonDown.Width" ) { ((GUIDIALOGDECLARATION *) pPair->ReservedPointer)->tpListBoxButtonDown.iWidth = pPair->GetInt(); return; }
	if ( name == "ListBox.ButtonDown.Height" ) { ((GUIDIALOGDECLARATION *) pPair->ReservedPointer)->tpListBoxButtonDown.iHeight = pPair->GetInt(); return; }
	if ( name == "ListBox.Background.X" ) { ((GUIDIALOGDECLARATION *) pPair->ReservedPointer)->tpListBoxBackground.iPosX = pPair->GetInt(); return; }
	if ( name == "ListBox.Background.Y" ) { ((GUIDIALOGDECLARATION *) pPair->ReservedPointer)->tpListBoxBackground.iPosY = pPair->GetInt(); return; }
	if ( name == "ListBox.Background.Width" ) { ((GUIDIALOGDECLARATION *) pPair->ReservedPointer)->tpListBoxBackground.iWidth = pPair->GetInt(); return; }
	if ( name == "ListBox.Background.Height" ) { ((GUIDIALOGDECLARATION *) pPair->ReservedPointer)->tpListBoxBackground.iHeight = pPair->GetInt(); return; }
	if ( name == "ListBox.ActiveBackground.X" ) { ((GUIDIALOGDECLARATION *) pPair->ReservedPointer)->tpListBoxActiveBackground.iPosX = pPair->GetInt(); return; }
	if ( name == "ListBox.ActiveBackground.Y" ) { ((GUIDIALOGDECLARATION *) pPair->ReservedPointer)->tpListBoxActiveBackground.iPosY = pPair->GetInt(); return; }
	if ( name == "ListBox.ActiveBackground.Width" ) { ((GUIDIALOGDECLARATION *) pPair->ReservedPointer)->tpListBoxActiveBackground.iWidth = pPair->GetInt(); return; }
	if ( name == "ListBox.ActiveBackground.Height" ) { ((GUIDIALOGDECLARATION *) pPair->ReservedPointer)->tpListBoxActiveBackground.iHeight = pPair->GetInt(); return; }
	if ( name == "ListBox.TopLeft.X" ) { ((GUIDIALOGDECLARATION *) pPair->ReservedPointer)->tpListBoxTopLeft.iPosX = pPair->GetInt(); return; }
	if ( name == "ListBox.TopLeft.Y" ) { ((GUIDIALOGDECLARATION *) pPair->ReservedPointer)->tpListBoxTopLeft.iPosY = pPair->GetInt(); return; }
	if ( name == "ListBox.TopLeft.Width" ) { ((GUIDIALOGDECLARATION *) pPair->ReservedPointer)->tpListBoxTopLeft.iWidth = pPair->GetInt(); return; }
	if ( name == "ListBox.TopLeft.Height" ) { ((GUIDIALOGDECLARATION *) pPair->ReservedPointer)->tpListBoxTopLeft.iHeight = pPair->GetInt(); return; }
	if ( name == "ListBox.TopRight.X" ) { ((GUIDIALOGDECLARATION *) pPair->ReservedPointer)->tpListBoxTopRight.iPosX = pPair->GetInt(); return; }
	if ( name == "ListBox.TopRight.Y" ) { ((GUIDIALOGDECLARATION *) pPair->ReservedPointer)->tpListBoxTopRight.iPosY = pPair->GetInt(); return; }
	if ( name == "ListBox.TopRight.Width" ) { ((GUIDIALOGDECLARATION *) pPair->ReservedPointer)->tpListBoxTopRight.iWidth = pPair->GetInt(); return; }
	if ( name == "ListBox.TopRight.Height" ) { ((GUIDIALOGDECLARATION *) pPair->ReservedPointer)->tpListBoxTopRight.iHeight = pPair->GetInt(); return; }
	if ( name == "ListBox.BottomLeft.X" ) { ((GUIDIALOGDECLARATION *) pPair->ReservedPointer)->tpListBoxBottomLeft.iPosX = pPair->GetInt(); return; }
	if ( name == "ListBox.BottomLeft.Y" ) { ((GUIDIALOGDECLARATION *) pPair->ReservedPointer)->tpListBoxBottomLeft.iPosY = pPair->GetInt(); return; }
	if ( name == "ListBox.BottomLeft.Width" ) { ((GUIDIALOGDECLARATION *) pPair->ReservedPointer)->tpListBoxBottomLeft.iWidth = pPair->GetInt(); return; }
	if ( name == "ListBox.BottomLeft.Height" ) { ((GUIDIALOGDECLARATION *) pPair->ReservedPointer)->tpListBoxBottomLeft.iHeight = pPair->GetInt(); return; }
	if ( name == "ListBox.BottomRight.X" ) { ((GUIDIALOGDECLARATION *) pPair->ReservedPointer)->tpListBoxBottomRight.iPosX = pPair->GetInt(); return; }
	if ( name == "ListBox.BottomRight.Y" ) { ((GUIDIALOGDECLARATION *) pPair->ReservedPointer)->tpListBoxBottomRight.iPosY = pPair->GetInt(); return; }
	if ( name == "ListBox.BottomRight.Width" ) { ((GUIDIALOGDECLARATION *) pPair->ReservedPointer)->tpListBoxBottomRight.iWidth = pPair->GetInt(); return; }
	if ( name == "ListBox.BottomRight.Height" ) { ((GUIDIALOGDECLARATION *) pPair->ReservedPointer)->tpListBoxBottomRight.iHeight = pPair->GetInt(); return; }
	if ( name == "ListBox.HorizLine.X" ) { ((GUIDIALOGDECLARATION *) pPair->ReservedPointer)->tpListBoxHorizLine.iPosX = pPair->GetInt(); return; }
	if ( name == "ListBox.HorizLine.Y" ) { ((GUIDIALOGDECLARATION *) pPair->ReservedPointer)->tpListBoxHorizLine.iPosY = pPair->GetInt(); return; }
	if ( name == "ListBox.HorizLine.Width" ) { ((GUIDIALOGDECLARATION *) pPair->ReservedPointer)->tpListBoxHorizLine.iWidth = pPair->GetInt(); return; }
	if ( name == "ListBox.HorizLine.Height" ) { ((GUIDIALOGDECLARATION *) pPair->ReservedPointer)->tpListBoxHorizLine.iHeight = pPair->GetInt(); return; }
	if ( name == "ListBox.VertLine.X" ) { ((GUIDIALOGDECLARATION *) pPair->ReservedPointer)->tpListBoxVertLine.iPosX = pPair->GetInt(); return; }
	if ( name == "ListBox.VertLine.Y" ) { ((GUIDIALOGDECLARATION *) pPair->ReservedPointer)->tpListBoxVertLine.iPosY = pPair->GetInt(); return; }
	if ( name == "ListBox.VertLine.Width" ) { ((GUIDIALOGDECLARATION *) pPair->ReservedPointer)->tpListBoxVertLine.iWidth = pPair->GetInt(); return; }
	if ( name == "ListBox.VertLine.Height" ) { ((GUIDIALOGDECLARATION *) pPair->ReservedPointer)->tpListBoxVertLine.iHeight = pPair->GetInt(); return; }

};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// process loading of a Dialog declaration
// is used in callback for loading dialog declaration (LoadDeclarationCallBack)
//
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT CGUIDialog::LoadDeclarationDialog( LPNAMEVALUEPAIR pPair )
{
	GUIDLGFILEDEFINITION *	pDecl = (GUIDLGFILEDEFINITION *) pPair->ReservedPointer;
	HRESULT					hr;
	CString					name = pPair->GetName();

	name.MakeLower();


	// opening tag
	if ( pPair->GetFlags() == NVPTYPE_TAGBEGIN )
	{
		// if already inside some block, the file is of a wrong format
		if ( pDecl->iDeepness ) ERRORMSG( ERRGENERIC, "CGUIDialog::LoadDeclarationDialog()", "Bad format." );
	
		pDecl->iDeepness++;
		pDecl->bInDlg = true;
	}
	else if ( pPair->GetFlags() == NVPTYPE_TAGEND ) // closing tag
	{
		// if inside another block or outside any blocks, the file is of a wrong format
		if ( pDecl->iDeepness != 1 || !pDecl->bInDlg ) ERRORMSG( ERRGENERIC, "CGUIDialog::LoadDeclarationDialog()", "Bad format." );

		// dialog needs to be initialized
		if ( !pDecl->bDialogInitialized )
		{
			// load texture definitions
			hr = CGUIDialog::LoadTextureDeclaration( CGUIControlBase::pResourceManager->GetResourcePath( RES_DlgDeclaration ) + pDecl->csDecl, &pDecl->dlgDecl );
			if ( hr ) ERRORMSG( ERRGENERIC, "CGUIDialog::LoadDeclarationDialog()", "Unsuccessful attempt to define dialog layout." );
			
			// initialize the dialog
			hr = pDecl->pDialog->Init( &pDecl->dlgDecl );
			if ( hr ) ERRORMSG( hr, "CGUIDialog::LoadDeclarationDialog()", "Dialog initialization failed." );

			// load possibly defined font object
			if ( pDecl->csFont.GetLength() )
			{
				hr = pDecl->pDialog->LoadBasicFont( pDecl->csFont );
				if ( hr ) ERRORMSG( ERRGENERIC, "CGUIDialog::LoadDeclarationDialog()", "Dialog font could not be created." );
				pDecl->dlgDecl.ridBasicFont = pDecl->pDialog->GetBasicFont();
			}

			// load possibly defined background texture
			if ( pDecl->csTex.GetLength() )
			{
				hr = pDecl->pDialog->LoadBackgroundTexture( pDecl->csTex );
				if ( hr ) ERRORMSG( ERRGENERIC, "CGUIDialog::LoadDeclarationDialog()", "Dialog background could not be created." );
				pDecl->dlgDecl.idBackgroundTexture = pDecl->pDialog->GetBackgroundTexture();
			}

			pDecl->bDialogInitialized = true;
		}

		pDecl->iDeepness--;
		pDecl->bInDlg = false;
	}
	else if ( pPair->GetFlags() & NVPTYPE_VARIABLE ) // just another property
	{
		if ( !pDecl->bDialogInitialized )
		{
			if ( name == "x" )			pDecl->dlgDecl.iPosX = pPair->GetInt();
			if ( name == "y" )			pDecl->dlgDecl.iPosY = pPair->GetInt();
			if ( name == "width" )		pDecl->dlgDecl.iWidth = pPair->GetInt();
			if ( name == "height" )		pDecl->dlgDecl.iHeight = pPair->GetInt();
			if ( name == "fontdeclaration" )		pDecl->csFont = pPair->GetString();
			if ( name == "backgroundtexture" )		pDecl->csTex = pPair->GetString();
			if ( name == "texturedeclarations" )	pDecl->csDecl = pPair->GetString();
			if ( name == "tooltipdelay" )			pDecl->dlgDecl.uiToolTipDelay = pPair->GetInt();
			if ( name == "transparency" )			pDecl->dlgDecl.fTransparency = pPair->GetFloat();
			if ( name == "tooltiptransparency" )	pDecl->dlgDecl.fControlsTransparency = pPair->GetFloat();
			if ( name == "controlstransparency" )	pDecl->dlgDecl.fToolTipTransparency = pPair->GetFloat();
			if ( name == "basictextcolor" )			pDecl->dlgDecl.uiBasicTextColor = pPair->GetInt();
			if ( name == "activetextcolor" )		pDecl->dlgDecl.uiActiveTextColor = pPair->GetInt();
			if ( name == "color" )			pDecl->dlgDecl.uiColor = pPair->GetInt();
			if ( name == "sliderposmin" )	pDecl->dlgDecl.iSliderPosMin = pPair->GetInt();
			if ( name == "sliderposmax" )	pDecl->dlgDecl.iSliderPosMax = pPair->GetInt();
		}
		else 
		{
			if ( name == "x" )			pDecl->pDialog->SetPositionX( pPair->GetInt() );
			if ( name == "y" )			pDecl->pDialog->SetPositionY( pPair->GetInt() );
			if ( name == "width" )		pDecl->pDialog->SetWidth( pPair->GetInt() );
			if ( name == "height" )		pDecl->pDialog->SetHeight( pPair->GetInt() );
			if ( name == "tooltipdelay" )			pDecl->pDialog->SetToolTipDelay( pPair->GetInt() );
			if ( name == "transparency" )			pDecl->pDialog->SetTransparency( pPair->GetFloat() );
			if ( name == "tooltiptransparency" )	pDecl->pDialog->SetControlsTransparency( pPair->GetFloat() );
			if ( name == "controlstransparency" )	pDecl->pDialog->SetToolTipTransparency( pPair->GetFloat() );
			// TODO: tady zkontroluj, zda se nastavi spravna hodnota, kdyz se tam predava int a chce to UINT
			if ( name == "basictextcolor" )			pDecl->pDialog->SetBasicTextColor( pPair->GetInt() );
			if ( name == "activetextcolor" )		pDecl->pDialog->SetActiveTextColor( pPair->GetInt() );
			if ( name == "color" )			pDecl->pDialog->SetColor( pPair->GetInt() );
			if ( name == "sliderposmin" )	pDecl->pDialog->SetSliderPosMin( pPair->GetInt() );
			if ( name == "sliderposmax" )	pDecl->pDialog->SetSliderPosMax( pPair->GetInt() );
			
			if ( name == "texturedeclarations" ) // load texture definitions
			{
				pDecl->csDecl = pPair->GetString();
				hr = CGUIDialog::LoadTextureDeclaration( CGUIControlBase::pResourceManager->GetResourcePath( RES_DlgDeclaration ) + pDecl->csDecl, &pDecl->dlgDecl );
				if ( hr ) ERRORMSG( ERRGENERIC, "CGUIDialog::LoadDeclarationDialog()", "Unsuccessful attempt to define dialog layout." );
			}
			
			if ( name == "fontdeclaration" ) // load possibly defined font object
			{
				pDecl->csFont = pPair->GetString();
				hr = pDecl->pDialog->LoadBasicFont( pDecl->csFont );
				if ( hr ) ERRORMSG( ERRGENERIC, "CGUIDialog::LoadDeclarationDialog()", "Dialog font could not be created." );
				pDecl->dlgDecl.ridBasicFont = pDecl->pDialog->GetBasicFont();
			}

			if ( name == "backgroundtexture" ) // load possibly defined background texture
			{
				pDecl->csTex = pPair->GetString();
				hr = pDecl->pDialog->LoadBackgroundTexture( pDecl->csTex );
				if ( hr ) ERRORMSG( ERRGENERIC, "CGUIDialog::LoadDeclarationDialog()", "Dialog background could not be created." );
				pDecl->dlgDecl.idBackgroundTexture = pDecl->pDialog->GetBackgroundTexture();
			}
		} // end of dialog property setting

	}
	else ERRORMSG( ERRGENERIC, "CGUIDialog::LoadDeclarationDialog()", "Bad format." );


	return ERRNOERROR;
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// process loading of a Control declaration
// is used in callback for loading dialog declaration (LoadDeclarationCallBack)
//
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT CGUIDialog::LoadDeclarationControl( LPNAMEVALUEPAIR pPair )
{
	GUIDLGFILEDEFINITION *	pDecl = (GUIDLGFILEDEFINITION *) pPair->ReservedPointer;
	GUICONTROLID			newID;
	HRESULT					hr;
	CGUIControlBase		*	pControl;
	CString					name = pPair->GetName();

	name.MakeLower();


	// opening tag
	if ( pPair->GetFlags() == NVPTYPE_TAGBEGIN )
	{
		// if already inside some block or dialog is not initialized yet, the file is of a wrong format
		if ( pDecl->iDeepness || !pDecl->bDialogInitialized ) ERRORMSG( ERRGENERIC, "CGUIDialog::LoadDeclarationControl()", "Bad format." );
	
		pDecl->iDeepness++;
		pDecl->bInControl = true;
			
		// init the control declaration structure
		ZeroMemory( &(pDecl->ctrlDecl), sizeof( pDecl->ctrlDecl ) );
		// set default values
		pDecl->uiControlType = GUICT_BASE;
		pDecl->csCtrlName = "";
		pDecl->csCtrlCaption = "";
		pDecl->cvValue.SetValue( "" );
		pDecl->bEnabled = pDecl->bVisible = true;
		pDecl->uiTextFormat = 0;
		pDecl->csFont = "";
		pDecl->csTex = "";
		// set derived dialog declaration values
		pDecl->ctrlDecl.pSharedDialogData = &(pDecl->pDialog->pSharedDialogData);
		pDecl->ctrlDecl.fTransparency = pDecl->dlgDecl.fTransparency;
		pDecl->ctrlDecl.idBackgroundTexture = pDecl->dlgDecl.idBackgroundTexture;
		pDecl->ctrlDecl.ridBasicFont = pDecl->dlgDecl.ridBasicFont;
		pDecl->ctrlDecl.uiBasicTextColor = pDecl->dlgDecl.uiBasicTextColor;
		pDecl->ctrlDecl.uiActiveTextColor = pDecl->dlgDecl.uiActiveTextColor;
		pDecl->ctrlDecl.uiColor = pDecl->dlgDecl.uiColor;
		pDecl->ctrlDecl.iSliderPosMin = pDecl->dlgDecl.iSliderPosMin;
		pDecl->ctrlDecl.iSliderPosMax = pDecl->dlgDecl.iSliderPosMax;
	}
	else if ( pPair->GetFlags() == NVPTYPE_TAGEND ) // closing tag
	{
		// if inside another block or outside any blocks, the file is of a wrong format
		if ( pDecl->iDeepness != 1 || !pDecl->bInControl ) ERRORMSG( ERRGENERIC, "CGUIDialog::LoadDeclarationControl()", "Bad format." );

		pDecl->ctrlDecl.bSelected = ( pDecl->cvValue.GetInt() != 0 );

		// create a new control, choose from selected type
		if ( pDecl->uiControlType == GUICT_LABEL )
		{
			hr = pDecl->pDialog->AddLabelEx( &pDecl->ctrlDecl, pDecl->cvValue.GetString(), &newID );
			if ( hr ) ERRORMSG( ERRGENERIC, "CGUIDialog::LoadDeclarationControl()", "Unable to add a control object to dialog." );
			pControl = pDecl->pDialog->GetControl( newID );
			if ( pDecl->uiTextFormat ) ((CGUIControlLabel *)pControl)->uiTextFormat = pDecl->uiTextFormat;
		}
		else if ( pDecl->uiControlType == GUICT_EDIT )
		{
			pDecl->ctrlDecl.TexturePos1 = pDecl->dlgDecl.tpEditBackground;
			pDecl->ctrlDecl.BottomLeftCornerPos = pDecl->dlgDecl.tpEditBottomLeft;
			pDecl->ctrlDecl.BottomRightCornerPos = pDecl->dlgDecl.tpEditBottomRight;
			pDecl->ctrlDecl.TopLeftCornerPos = pDecl->dlgDecl.tpEditTopLeft;
			pDecl->ctrlDecl.TopRightCornerPos = pDecl->dlgDecl.tpEditTopRight;
			pDecl->ctrlDecl.HorizontalLinePos = pDecl->dlgDecl.tpEditHorizLine;
			pDecl->ctrlDecl.VerticalLinePos = pDecl->dlgDecl.tpEditVertLine;
			
			hr = pDecl->pDialog->AddEditBoxEx( &pDecl->ctrlDecl, &newID );
			if ( hr ) ERRORMSG( ERRGENERIC, "CGUIDialog::LoadDeclarationControl()", "Unable to add a control object to dialog." );
			pControl = pDecl->pDialog->GetControl( newID );
			((CGUIControlEdit *)pControl)->SetValue( pDecl->cvValue.GetString() );
			if ( pDecl->uiTextFormat ) ((CGUIControlEdit *)pControl)->uiTextFormat = pDecl->uiTextFormat;
		}
		else if ( pDecl->uiControlType == GUICT_BUTTON )
		{
			pDecl->ctrlDecl.TexturePos1 = pDecl->dlgDecl.tpButtonUp;
			pDecl->ctrlDecl.TexturePos2 = pDecl->dlgDecl.tpButtonDown;
			
			hr = pDecl->pDialog->AddButtonEx( &pDecl->ctrlDecl, pDecl->cvValue.GetString(), &newID );
			if ( hr ) ERRORMSG( ERRGENERIC, "CGUIDialog::LoadDeclarationControl()", "Unable to add a control object to dialog." );
			pControl = pDecl->pDialog->GetControl( newID );
			if ( pDecl->uiTextFormat ) ((CGUIControlButton *)pControl)->uiTextFormat = pDecl->uiTextFormat;
		}
		else if ( pDecl->uiControlType == GUICT_CHECKBOX )
		{
			pDecl->ctrlDecl.TexturePos1 = pDecl->dlgDecl.tpCheckBoxUnchecked;
			pDecl->ctrlDecl.TexturePos2 = pDecl->dlgDecl.tpCheckBoxChecked;
			
			hr = pDecl->pDialog->AddCheckBoxEx( &pDecl->ctrlDecl, pDecl->csCtrlCaption, &newID );
			if ( hr ) ERRORMSG( ERRGENERIC, "CGUIDialog::LoadDeclarationControl()", "Unable to add a control object to dialog." );
			pControl = pDecl->pDialog->GetControl( newID );
			if ( pDecl->uiTextFormat ) ((CGUIControlCheckBox *)pControl)->uiTextFormat = pDecl->uiTextFormat;
		}
		else if ( pDecl->uiControlType == GUICT_RADIOBUTTON )
		{
			pDecl->ctrlDecl.TexturePos1 = pDecl->dlgDecl.tpRadioButtonUnselected;
			pDecl->ctrlDecl.TexturePos2 = pDecl->dlgDecl.tpRadioButtonSelected;
			
			hr = pDecl->pDialog->AddRadioButtonEx( &pDecl->ctrlDecl, pDecl->csCtrlCaption, &newID );
			if ( hr ) ERRORMSG( ERRGENERIC, "CGUIDialog::LoadDeclarationControl()", "Unable to add a control object to dialog." );
			pControl = pDecl->pDialog->GetControl( newID );
			if ( pDecl->uiTextFormat ) ((CGUIControlRadioButton *)pControl)->uiTextFormat = pDecl->uiTextFormat;
		}
		else if ( pDecl->uiControlType == GUICT_LISTBOX )
		{
			pDecl->ctrlDecl.TexturePos1 = pDecl->dlgDecl.tpListBoxBackground;
			pDecl->ctrlDecl.TexturePos2 = pDecl->dlgDecl.tpListBoxActiveBackground;
			pDecl->ctrlDecl.TexturePos3 = pDecl->dlgDecl.tpListBoxButtonUp;
			pDecl->ctrlDecl.TexturePos4 = pDecl->dlgDecl.tpListBoxButtonDown;
			pDecl->ctrlDecl.BottomLeftCornerPos = pDecl->dlgDecl.tpListBoxBottomLeft;
			pDecl->ctrlDecl.BottomRightCornerPos = pDecl->dlgDecl.tpListBoxBottomRight;
			pDecl->ctrlDecl.TopLeftCornerPos = pDecl->dlgDecl.tpListBoxTopLeft;
			pDecl->ctrlDecl.TopRightCornerPos = pDecl->dlgDecl.tpListBoxTopRight;
			pDecl->ctrlDecl.HorizontalLinePos = pDecl->dlgDecl.tpListBoxHorizLine;
			pDecl->ctrlDecl.VerticalLinePos = pDecl->dlgDecl.tpListBoxVertLine;
			
			hr = pDecl->pDialog->AddListBoxEx( &pDecl->ctrlDecl, &newID );
			if ( hr ) ERRORMSG( ERRGENERIC, "CGUIDialog::LoadDeclarationControl()", "Unable to add a control object to dialog." );
			pControl = pDecl->pDialog->GetControl( newID );
			if ( pDecl->uiTextFormat ) ((CGUIControlListBox *)pControl)->uiTextFormat = pDecl->uiTextFormat;
		}
		else if ( pDecl->uiControlType == GUICT_COMBOBOX )
		{
			pDecl->ctrlDecl.TexturePos1 = pDecl->dlgDecl.tpComboBackground;
			pDecl->ctrlDecl.TexturePos2 = pDecl->dlgDecl.tpComboActiveBackground;
			pDecl->ctrlDecl.TexturePos3 = pDecl->dlgDecl.tpComboButton;
			pDecl->ctrlDecl.BottomLeftCornerPos = pDecl->dlgDecl.tpComboBottomLeft;
			pDecl->ctrlDecl.BottomRightCornerPos = pDecl->dlgDecl.tpComboBottomRight;
			pDecl->ctrlDecl.TopLeftCornerPos = pDecl->dlgDecl.tpComboTopLeft;
			pDecl->ctrlDecl.TopRightCornerPos = pDecl->dlgDecl.tpComboTopRight;
			pDecl->ctrlDecl.HorizontalLinePos = pDecl->dlgDecl.tpComboHorizLine;
			pDecl->ctrlDecl.VerticalLinePos = pDecl->dlgDecl.tpComboVertLine;
			
			hr = pDecl->pDialog->AddComboBoxEx( &pDecl->ctrlDecl, &newID );
			if ( hr ) ERRORMSG( ERRGENERIC, "CGUIDialog::LoadDeclarationControl()", "Unable to add a control object to dialog." );
			pControl = pDecl->pDialog->GetControl( newID );
			if ( pDecl->uiTextFormat ) ((CGUIControlComboBox *)pControl)->uiTextFormat = pDecl->uiTextFormat;
		}
		else if ( pDecl->uiControlType == GUICT_TRACKBAR )
		{
			pDecl->ctrlDecl.TexturePos1 = pDecl->dlgDecl.tpTrackBarTrack;
			pDecl->ctrlDecl.TexturePos2 = pDecl->dlgDecl.tpTrackBarSlider;
			hr = pDecl->pDialog->AddTrackBarEx( &pDecl->ctrlDecl, pDecl->cvValue.GetFloat(), &newID );
			if ( hr ) ERRORMSG( ERRGENERIC, "CGUIDialog::LoadDeclarationControl()", "Unable to add a control object to dialog." );
		}
		else if ( pDecl->uiControlType == GUICT_POPUPMENU )
		{
			pDecl->ctrlDecl.TexturePos1 = pDecl->dlgDecl.tpPopupBackground;
			pDecl->ctrlDecl.TexturePos2 = pDecl->dlgDecl.tpPopupActiveBackground;
			pDecl->ctrlDecl.BottomLeftCornerPos = pDecl->dlgDecl.tpPopupBottomLeft;
			pDecl->ctrlDecl.BottomRightCornerPos = pDecl->dlgDecl.tpPopupBottomRight;
			pDecl->ctrlDecl.TopLeftCornerPos = pDecl->dlgDecl.tpPopupTopLeft;
			pDecl->ctrlDecl.TopRightCornerPos = pDecl->dlgDecl.tpPopupTopRight;
			pDecl->ctrlDecl.HorizontalLinePos = pDecl->dlgDecl.tpPopupHorizLine;
			pDecl->ctrlDecl.VerticalLinePos = pDecl->dlgDecl.tpPopupVertLine;
			
			hr = pDecl->pDialog->AddPopupMenuEx( &pDecl->ctrlDecl, &newID );
			if ( hr ) ERRORMSG( ERRGENERIC, "CGUIDialog::LoadDeclarationControl()", "Unable to add a control object to dialog." );
			pControl = pDecl->pDialog->GetControl( newID );
			if ( pDecl->uiTextFormat ) ((CGUIControlPopupMenu *)pControl)->uiTextFormat = pDecl->uiTextFormat;
		}
		else if ( pDecl->uiControlType == GUICT_ICONEDLIST )
		{
			pDecl->ctrlDecl.TexturePos1 = pDecl->dlgDecl.tpListBoxBackground;
			pDecl->ctrlDecl.TexturePos2 = pDecl->dlgDecl.tpListBoxActiveBackground;
			pDecl->ctrlDecl.TexturePos3 = pDecl->dlgDecl.tpListBoxButtonUp;
			pDecl->ctrlDecl.TexturePos4 = pDecl->dlgDecl.tpListBoxButtonDown;
			pDecl->ctrlDecl.BottomLeftCornerPos = pDecl->dlgDecl.tpListBoxBottomLeft;
			pDecl->ctrlDecl.BottomRightCornerPos = pDecl->dlgDecl.tpListBoxBottomRight;
			pDecl->ctrlDecl.TopLeftCornerPos = pDecl->dlgDecl.tpListBoxTopLeft;
			pDecl->ctrlDecl.TopRightCornerPos = pDecl->dlgDecl.tpListBoxTopRight;
			pDecl->ctrlDecl.HorizontalLinePos = pDecl->dlgDecl.tpListBoxHorizLine;
			pDecl->ctrlDecl.VerticalLinePos = pDecl->dlgDecl.tpListBoxVertLine;
			
			hr = pDecl->pDialog->AddIconedListEx( &pDecl->ctrlDecl, &newID );
			if ( hr ) ERRORMSG( ERRGENERIC, "CGUIDialog::LoadDeclarationControl()", "Unable to add a control object to dialog." );
			pControl = pDecl->pDialog->GetControl( newID );
			if ( pDecl->uiTextFormat ) ((CGUIControlIconedList *)pControl)->uiTextFormat = pDecl->uiTextFormat;
		}
		else if ( pDecl->uiControlType == GUICT_IMAGE )
		{
			pDecl->ctrlDecl.TexturePos1 = pDecl->dlgDecl.tpImageBackground;
			
			hr = pDecl->pDialog->AddImageEx( &pDecl->ctrlDecl, pDecl->cvValue.GetString(), &newID );
			if ( hr ) ERRORMSG( ERRGENERIC, "CGUIDialog::LoadDeclarationControl()", "Unable to add a control object to dialog." );
		}
		else ERRORMSG( ERRINVALIDPARAMETER, "CGUIDialog::LoadDeclarationControl()", "Unknown control type." );

		// set some control's properties
		pControl = pDecl->pDialog->GetControl( newID );
		pControl->Name = pDecl->csCtrlName;
		if ( pDecl->bEnabled ) pControl->Enable(); else pControl->Disable();
		if ( pDecl->bVisible ) pControl->SetVisible(); else pControl->SetInvisible();

		
		// load possibly defined font object
		if ( pDecl->csFont.GetLength() )
		{
			hr = pControl->LoadBasicFont( pDecl->csFont );
			if ( hr ) ERRORMSG( ERRGENERIC, "CGUIDialog::LoadDeclarationControl()", "GUI font could not be created." );
		}

		// load possibly defined background texture
		if ( pDecl->csTex.GetLength() )
		{
			hr = pControl->LoadBackgroundTexture( pDecl->csTex );
			if ( hr ) ERRORMSG( ERRGENERIC, "CGUIDialog::LoadDeclarationControl()", "Control object background could not be created." );
		}

		pDecl->iDeepness--;
		pDecl->bInControl = false;
	}
	else if ( pPair->GetFlags() & NVPTYPE_VARIABLE ) // just another property
	{
		if ( pDecl->iDeepness != 1 || !pDecl->bInControl ) ERRORMSG( ERRGENERIC, "CGUIDialog::LoadDeclarationControl()", "Bad format." );
		
		if ( name == "name" )		pDecl->csCtrlName = pPair->GetString();
		if ( name == "caption" )	pDecl->csCtrlCaption = pPair->GetString();
		if ( name == "type" )
		{
			CString		str = pPair->GetString();
			str.MakeLower();

			if ( str == "label" )			pDecl->uiControlType = GUICT_LABEL;
			else if ( str == "edit" )		pDecl->uiControlType = GUICT_EDIT;
			else if ( str == "button" )		pDecl->uiControlType = GUICT_BUTTON;
			else if ( str == "checkbox" )	pDecl->uiControlType = GUICT_CHECKBOX;
			else if ( str == "radiobutton" )pDecl->uiControlType = GUICT_RADIOBUTTON;
			else if ( str == "list" )		pDecl->uiControlType = GUICT_LISTBOX;
			else if ( str == "combo" )		pDecl->uiControlType = GUICT_COMBOBOX;
			else if ( str == "trackbar" )	pDecl->uiControlType = GUICT_TRACKBAR;
			else if ( str == "popupmenu" )	pDecl->uiControlType = GUICT_POPUPMENU;
			else if ( str == "iconedlist" )	pDecl->uiControlType = GUICT_ICONEDLIST;
			else if ( str == "image" )		pDecl->uiControlType = GUICT_IMAGE;
		}
		if ( name == "x" )			pDecl->ctrlDecl.iPosX = pPair->GetInt();
		if ( name == "y" )			pDecl->ctrlDecl.iPosY = pPair->GetInt();
		if ( name == "width" )		pDecl->ctrlDecl.iWidth = pPair->GetInt();
		if ( name == "height" )		pDecl->ctrlDecl.iHeight = pPair->GetInt();
		if ( name == "value" )		pDecl->cvValue.SetValue( pPair->GetString() );
		if ( name == "enabled" )	pDecl->bEnabled = ( pPair->GetInt() != 0 );
		if ( name == "visible" )	pDecl->bVisible = ( pPair->GetInt() != 0 );
		if ( name == "tabstop" )	pDecl->ctrlDecl.iTabStop = pPair->GetInt();
		if ( name == "zorder" )		pDecl->ctrlDecl.iZOrder = pPair->GetInt();
		if ( name == "absoluteposition" )	pDecl->ctrlDecl.bAbsolutePosition = ( pPair->GetInt() != 0 );
		if ( name == "transparency" )		pDecl->ctrlDecl.fTransparency = pPair->GetFloat();
		if ( name == "backgroundtexture" )	pDecl->csTex = pPair->GetString();
		if ( name == "fontdeclaration" )	pDecl->csFont = pPair->GetString();
		if ( name == "basictextcolor" )		pDecl->ctrlDecl.uiBasicTextColor = pPair->GetInt();
		if ( name == "activetextcolor" )	pDecl->ctrlDecl.uiActiveTextColor = pPair->GetInt();
		if ( name == "color" )				pDecl->ctrlDecl.uiColor = pPair->GetInt();
		if ( name == "textformat" )			pDecl->uiTextFormat = pPair->GetInt();
		if ( name == "tooltip" )
		{
			CString		str = pPair->GetString();
			strncpy( pDecl->ctrlDecl.cToolTip, str.GetString(), 510 );
		}
		if ( name == "sliderposmin" )		pDecl->ctrlDecl.iSliderPosMin = pPair->GetInt();
		if ( name == "sliderposmax" )		pDecl->ctrlDecl.iSliderPosMax = pPair->GetInt();
		if ( name == "maxtextlength" )		pDecl->ctrlDecl.iMaxEditTextLen = pPair->GetInt();
		if ( name == "radiogroup" )			pDecl->ctrlDecl.iRadioGroupIndex = pPair->GetInt();
		if ( name == "lineheight" )			pDecl->ctrlDecl.uiListLineHeight = pPair->GetInt();
		if ( name == "linescount" )			pDecl->ctrlDecl.uiLinesCount = pPair->GetInt();
		if ( name == "iconwidth" )			pDecl->ctrlDecl.uiListIconWidth = pPair->GetInt();
	}


	return ERRNOERROR;
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// callback for loading dialog declaration
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CALLBACK CGUIDialog::LoadDeclarationCallBack( LPNAMEVALUEPAIR pPair )
{
	GUIDLGFILEDEFINITION *	pDecl = (GUIDLGFILEDEFINITION *) pPair->ReservedPointer;
	CString					name = pPair->GetName();
	HRESULT					hr;
	

#define __GUIERROR(ch)	{ ErrorHandler.HandleError( ch, "CGUIDialog::LoadDeclarationCallBack()", \
									"Dialog declaration file has wrong syntax." ); \
						pPair->ThrowError(); return; }


	name.MakeLower();

	// new tag (file block) found
	if ( pPair->GetFlags() & NVPTYPE_TAG )
	{
		// dialog declaration is to be updated
		if ( name == "dialog" )	{ if ( hr = LoadDeclarationDialog( pPair ) ) __GUIERROR(hr); }
		// control declaration
		if ( name == "control" ) { if ( hr = LoadDeclarationControl( pPair ) ) __GUIERROR(hr); }
		//if ( name == "" )
	}
	// just a property inside some block
	else if ( pPair->GetFlags() & NVPTYPE_VARIABLE && pDecl->iDeepness )
	{
		if ( pDecl->bInDlg ) 		{ if ( hr = LoadDeclarationDialog( pPair ) ) __GUIERROR(hr); }
		if ( pDecl->bInControl )	{ if ( hr = LoadDeclarationControl( pPair ) ) __GUIERROR(hr); }
	}

#undef __GUIERROR
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// basic contructor
//
//////////////////////////////////////////////////////////////////////////////////////////////
CGUIDialog::CGUIDialog()
{
	pSharedDialogData.pEventTimer = NULL;
	
	Controls.clear();

	pBasicFont = NULL;
	ridBasicFont = -1;
	idBackgroundTexture = -1;

	pSharedDialogData.iPosX = pSharedDialogData.iPosY = 0;
	pSharedDialogData.iWidth = pSharedDialogData.iHeight = 0;
	fTransparency = fControlsTransparency = 0.0f;
	uiBasicTextColor = 0xffffffff;   // 0xargb format

	iSliderPosMin = iSliderPosMax = 0;

	idFocusedObject = -1;
	idMouseCapturingObject = -1;
	idToolTipObject = -1;
	iMaxTabStop = -1;
	iMaxZOrder = -1;
	iLastUID = 0;
	
	iBufferLenght = 0;
	pZOrderList = NULL;
	pTabStopList = NULL;

	SpriteBackground = NULL;

	lpOnRButtonDown = NULL;
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// basic destructor
//
//////////////////////////////////////////////////////////////////////////////////////////////
CGUIDialog::~CGUIDialog()
{
	CONTROLS_ITER	iter;

	for ( iter = Controls.begin(); iter != Controls.end(); iter++ )
	{
		SAFE_DELETE( iter->second );
	}
	Controls.clear();

	if ( idBackgroundTexture >= 0 ) CGUIControlBase::pResourceManager->ReleaseResource( idBackgroundTexture );
	if ( ridBasicFont >= 0 ) CGUIControlBase::pResourceManager->ReleaseResource( ridBasicFont );

	SAFE_FREE( pZOrderList );
	SAFE_FREE( pTabStopList );

	SAFE_DELETE( pSharedDialogData.pEventTimer );

	SAFE_RELEASE( SpriteBackground );
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// returns a pointer to a control specified by the given handler
// if the handler is not valid, this returns zero pointer
//
//////////////////////////////////////////////////////////////////////////////////////////////
CGUIControlBase * CGUIDialog::GetControl( GUICONTROLID UID )
{
	CONTROLS_ITER	iter;

	iter = Controls.find( UID );
	if ( iter == Controls.end() ) return NULL;
	
	return iter->second;
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// returns a pointer to a control specified by the given control name
// if there are more controls of the same name, first found (that is actually the first added)
// is returned
//
// if the name is not found, this returns zero pointer
//
//////////////////////////////////////////////////////////////////////////////////////////////
CGUIControlBase * CGUIDialog::GetControl( LPCTSTR csCtrlName )
{
	CONTROLS_ITER	iter;

	for ( iter = Controls.begin(); iter != Controls.end(); iter++ )
		if ( iter->second->GetName() == csCtrlName ) return iter->second;
	
	return NULL;
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// sets the actual background texture to the one specified by its handler to
// ResourceManager
//
// also performs releasing of the old texture and increasing reference count of the new one 
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CGUIDialog::SetBackgroundTexture( resManNS::RESOURCEID resID )
{
	// increase the reference count to the new texture
	CGUIControlBase::pResourceManager->DuplicateResource( resID );

	// release the old background texture if one is set
	if ( idBackgroundTexture >= 0 ) CGUIControlBase::pResourceManager->ReleaseResource( idBackgroundTexture );

	// set the new actual background texture
	idBackgroundTexture = resID;
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// releases actually attached texture and loads a new one
//
// returns a nonzero error value when unsuccessful
//
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT CGUIDialog::LoadBackgroundTexture( LPCTSTR fileName )
{
	resManNS::RESOURCEID		resID;
	
	resID = idBackgroundTexture; // save the ID

	// load new resource (if it is same, it is duplicated)
	idBackgroundTexture = CGUIControlBase::pResourceManager->LoadResource( fileName, RES_Texture );
	if ( idBackgroundTexture <= 0 ) ERRORMSG( ERRGENERIC, "CGUIDialog::LoadBackgroundTexture()", "Can't obtain a texture for this control." )

	// release the old background texture if one is set
	if ( resID > 0 ) CGUIControlBase::pResourceManager->ReleaseResource( resID );

	
	return ERRNOERROR;
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// releases previously attached font and sets the actual font used for standard text
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CGUIDialog::SetBasicFont( resManNS::RESOURCEID ridFont )
{
	// duplicate the resource
	CGUIControlBase::pResourceManager->DuplicateResource( ridFont );

	// release the old one
	if ( ridBasicFont >= 0 ) CGUIControlBase::pResourceManager->ReleaseResource( ridBasicFont );

	ridBasicFont = ridFont;

	// set font pointer
	resManNS::__FontDef * font = CGUIControlBase::pResourceManager->GetFontDef( ridBasicFont );
	if ( font ) pBasicFont = font->pFont;
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// loads font declaration and initializes the font object
// also releases previously attached font
//
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT CGUIDialog::LoadBasicFont( LPCTSTR fileName )
{
	resManNS::RESOURCEID		resID;
	
	resID = ridBasicFont; // save the ID

	// load new resource (if it is same, it is duplicated)
	ridBasicFont = CGUIControlBase::pResourceManager->LoadResource( fileName, RES_FontDef );
	if ( ridBasicFont < 0 ) ERRORMSG( ERRNOTFOUND, "CGUIDialog::LoadBasicFont()", "Font object could not be created." );

	// release old resource - this way it won't release the resource if it is not necessary
	if ( resID > 0 ) CGUIControlBase::pResourceManager->ReleaseResource( resID );

	// set font pointer
	resManNS::__FontDef * font = CGUIControlBase::pResourceManager->GetFontDef( ridBasicFont );
	if ( font ) pBasicFont = font->pFont;
	
	return ERRNOERROR;
};

//////////////////////////////////////////////////////////////////////////////////////////////
//
// sets the transparecny value; value outside of range <0,1> is clamped
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CGUIDialog::SetTransparency( float f )
{
	if ( f < 0.0f ) f = 0.0f;
	if ( f > 1.0f ) f = 1.0f;

	fTransparency = f;
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// sets the transparecny value for tooltips; value outside of range <0,1> is clamped
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CGUIDialog::SetToolTipTransparency( float f )
{
	if ( f < 0.0f ) f = 0.0f;
	if ( f > 1.0f ) f = 1.0f;

	fToolTipTransparency = f;
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// sets the transparecny for all controls; value outside of range <0,1> is clamped
// BEWARE!!!! This changes transparency value for all controls and this change is undoable!!!
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CGUIDialog::SetControlsTransparency( float f )
{
	int						UID = iLastUID + 1;
	CONTROLS_ITER			iterator;


	if ( f < 0.0f ) f = 0.0f;
	if ( f > 1.0f ) f = 1.0f;

	fControlsTransparency = f;

	for ( iterator = Controls.begin(); iterator != Controls.end(); iterator++ )	
		iterator->second->fTransparency = fControlsTransparency;
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// inits Dialog object
//
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT CGUIDialog::Init( GUIDIALOGDECLARATION * decl )
{
	HRESULT		hr;
	
	// set static variables shared among all control objects
	SetD3DDevice( decl->D3DDevice );
	SetResourceManager( decl->pResourceManager );
	SetHWND( decl->hWnd );

	if ( !decl->pTimer ) ERRORMSG( ERRINVALIDPARAMETER, "CGUIDialog::Init()", "Valid CTimeCounter object expected." );
	pSharedDialogData.pEventTimer = new CGUITimer( decl->pTimer );
	
	// set initial properties
	pSharedDialogData.iPosX = decl->iPosX;
	pSharedDialogData.iPosY = decl->iPosY;
	pSharedDialogData.iWidth = decl->iWidth;
	pSharedDialogData.iHeight = decl->iHeight;
	iFilePosX = decl->iFilePosX;
	iFilePosY = decl->iFilePosY;
	iFileWidth = decl->iFileWidth;
	iFileHeight = decl->iFileHeight;
	fTransparency = decl->fTransparency;
	uiBasicTextColor = decl->uiBasicTextColor;
	uiActiveTextColor = decl->uiActiveTextColor;
	fToolTipTransparency = decl->fToolTipTransparency;
	fControlsTransparency = decl->fControlsTransparency;
	uiColor = decl->uiColor;
	uiToolTipDelay = decl->uiToolTipDelay;
	iSliderPosMin = decl->iSliderPosMin;
	iSliderPosMax = decl->iSliderPosMax;

	// default values
	iToolTipRelX = -30;
	iToolTipRelY = 50;
	iLastMouseX = iLastMouseY = 0;
	uiToolTipDuration = 5000;
	uiLastMouseMoveTime = 0;
	bToolTipShowed = FALSE;
	bToolTipMouseMoved = TRUE;

	// textures position setting
	tpToolTipTopLeft = decl->tpToolTipTopLeft;
	tpToolTipTopRight = decl->tpToolTipTopRight;
	tpToolTipBottomLeft = decl->tpToolTipBottomLeft;
	tpToolTipBottomRight = decl->tpToolTipBottomRight;
	tpToolTipHorizLine = decl->tpToolTipHorizLine;
	tpToolTipVertLine = decl->tpToolTipVertLine;
	tpToolTipBackground = decl->tpToolTipBackground;
	tpButtonUp = decl->tpButtonUp;
	tpButtonDown = decl->tpButtonDown;
	tpCheckBoxChecked = decl->tpCheckBoxChecked;
	tpCheckBoxUnchecked = decl->tpCheckBoxUnchecked;
	tpRadioButtonSelected = decl->tpRadioButtonSelected;
	tpRadioButtonUnselected = decl->tpRadioButtonUnselected;
	tpTrackBarTrack = decl->tpTrackBarTrack;
	tpTrackBarSlider = decl->tpTrackBarSlider;
	tpEditTopLeft = decl->tpEditTopLeft;
	tpEditTopRight = decl->tpEditTopRight;
	tpEditBottomLeft = decl->tpEditBottomLeft;
	tpEditBottomRight = decl->tpEditBottomRight;
	tpEditHorizLine = decl->tpEditHorizLine;
	tpEditVertLine = decl->tpEditVertLine;
	tpEditBackground = decl->tpEditBackground;
	tpListBoxTopLeft = decl->tpListBoxTopLeft;
	tpListBoxTopRight = decl->tpListBoxTopRight;
	tpListBoxBottomLeft = decl->tpListBoxBottomLeft;
	tpListBoxBottomRight = decl->tpListBoxBottomRight;
	tpListBoxHorizLine = decl->tpListBoxHorizLine;
	tpListBoxVertLine = decl->tpListBoxVertLine;
	tpListBoxBackground = decl->tpListBoxBackground;
	tpListBoxActiveBackground = decl->tpListBoxActiveBackground;
	tpListBoxButtonUp = decl->tpListBoxButtonUp;
	tpListBoxButtonDown = decl->tpListBoxButtonDown;
	tpPopupTopLeft = decl->tpPopupTopLeft;
	tpPopupTopRight = decl->tpPopupTopRight;
	tpPopupBottomLeft = decl->tpPopupBottomLeft;
	tpPopupBottomRight = decl->tpPopupBottomRight;
	tpPopupHorizLine = decl->tpPopupHorizLine;
	tpPopupVertLine = decl->tpPopupVertLine;
	tpPopupBackground = decl->tpPopupBackground;
	tpPopupActiveBackground = decl->tpPopupActiveBackground;
	tpComboTopLeft = decl->tpComboTopLeft;
	tpComboTopRight = decl->tpComboTopRight;
	tpComboBottomLeft = decl->tpComboBottomLeft;
	tpComboBottomRight = decl->tpComboBottomRight;
	tpComboHorizLine = decl->tpComboHorizLine;
	tpComboVertLine = decl->tpComboVertLine;
	tpComboButton = decl->tpComboButton;
	tpComboBackground = decl->tpComboBackground;
	tpComboActiveBackground = decl->tpComboActiveBackground;
	tpScrollBarUp = decl->tpScrollBarUp;
	tpScrollBarDown = decl->tpScrollBarDown;
	tpScrollBarSlider = decl->tpScrollBarSlider;
	tpScrollBarBackground = decl->tpScrollBarBackground;


	SetBackgroundTexture( decl->idBackgroundTexture );
	SetBasicFont( decl->ridBasicFont );

	SAFE_RELEASE( SpriteBackground );
	hr = D3DXCreateSprite( decl->D3DDevice, &SpriteBackground );
	if ( hr ) ERRORMSG( hr, "CGUIDialog::Init()", "Unable to create background surface of the dialog." );


	return ERRNOERROR;	
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// inits the Dialog by configuration set in file
//
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT CGUIDialog::InitFromFile(	LPCTSTR				fileName,
									HWND				hWnd,
									LPDIRECT3DDEVICE9	D3DDevice,
									resManNS::CResourceManager *	pResourceManager,
									CTimeCounter	 *	pTimer )
{
	HRESULT					hr;
	GUIDLGFILEDEFINITION	decl;
	
	
	// set static variables shared among all control objects
	SetD3DDevice( D3DDevice );
	SetResourceManager( pResourceManager );
	SetHWND( hWnd );

	if ( !pTimer ) ERRORMSG( ERRINVALIDPARAMETER, "CGUIDialog::InitFromFile()", "Valid CTimeCounter object expected." );
	pSharedDialogData.pEventTimer = new CGUITimer( pTimer );
	
	
	// clear the declaration structure first and init some properties
	ZeroMemory( &(decl.ctrlDecl), sizeof( decl.ctrlDecl ) );
	ZeroMemory( &(decl.dlgDecl), sizeof( decl.dlgDecl ) );
	ZeroMemory( &(decl.dlgDeclTemp), sizeof( decl.dlgDeclTemp ) );

	decl.dlgDecl.D3DDevice = D3DDevice;
	decl.dlgDecl.hWnd = hWnd;
	decl.dlgDecl.pResourceManager = pResourceManager;
	decl.dlgDecl.pTimer = pTimer;

	decl.str = decl.csFont = decl.csDecl = decl.csTex = "";
	decl.bInControl = decl.bInDlg = decl.bInPosDecl = false;
	decl.bDialogInitialized = false;
	decl.iDeepness = 0;
	decl.pDialog = this;
	decl.pPosDecl = NULL;
	decl.uiControlType = GUICT_BASE;
	
	// now, load all properties from file
	hr = NAMEVALUEPAIR::Load( fileName, LoadDeclarationCallBack, &decl );
	if ( hr ) ERRORMSG( ERRGENERIC, "CGUIDialog::InitFromFile()", "Can't load dialog declaration data." )
	
	
	SAFE_RELEASE( SpriteBackground );
	hr = D3DXCreateSprite( D3DDevice, &SpriteBackground );
	if ( hr ) ERRORMSG( hr, "CGUIDialog::InitFromFile()", "Unable to create background surface of the dialog." );


	// default values
	iToolTipRelX = -30;
	iToolTipRelY = 50;
	iLastMouseX = iLastMouseY = 0;
	uiToolTipDuration = 5000;
	uiLastMouseMoveTime = 0;
	bToolTipShowed = FALSE;
	bToolTipMouseMoved = TRUE;


	return ERRNOERROR;
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// resets some properties of the dialog, like focused item and so
// this method should be used when displaying the dialog for second time so no control has
// focus nor the mouse is captured
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CGUIDialog::Reset()
{
	idFocusedObject = -1;
	idMouseCapturingObject = -1;
	idToolTipObject = -1;
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// adds a label to the dialog with standard dialog setting
//
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT CGUIDialog::AddLabel(	int x, int y, 
								int width, int height, 
								LPCTSTR caption, 
								GUICONTROLID * newID )
{
	GUICONTROLDECLARATION	decl;
	HRESULT					hr;
	CGUIControlLabel	*	pObject;
	int						UID = iLastUID + 1;
	CONTROLS_ITER			iterator;


	// just in case if anyone want to (re)create more than 2 billions objects - structures could crash in such situation
	if ( UID > 2000000000 ) ERRORMSG( ERRTOOMANYRESOURCES, "CGUIDialog::AddLabel()", "Can't add any more control objects." )

	// clear the UID
	*newID = 0;

	// fill the declaration structure with proper data
	ZeroMemory( &decl, sizeof( decl ) );

	decl.pSharedDialogData = &pSharedDialogData;
	decl.iPosX = x;
	decl.iPosY = y;
	decl.iHeight = height;
	decl.iWidth = width;
	decl.iTabStop = ++iMaxTabStop; // the new object will have the highest tabstop; increase the maximum
	decl.iZOrder = ++iMaxZOrder; // the new object will also have the highest ZOrder index (will be topmost)
	decl.bAbsolutePosition = false;
	decl.fTransparency = fControlsTransparency;
	decl.idBackgroundTexture = -1;//idBackgroundTexture;
	decl.ridBasicFont = ridBasicFont;
	decl.uiBasicTextColor = uiBasicTextColor;
	decl.uiActiveTextColor = uiActiveTextColor;
	decl.uiColor = uiColor;
	decl.cToolTip[0] = 0;


	// create and initialize new control object
	pObject = new CGUIControlLabel( caption );
	if ( !pObject )	ERRORMSG( ERROUTOFMEMORY, "CGUIDialog::AddLabel()", "Can't create new label object." )

	hr = pObject->Init( &decl );
	if ( hr ) ERRORMSG( hr, "CGUIDialog::AddLabel()", "Can't initialize label object." )
	pObject->UID = UID;

	
	// add the object to the binary tree (list of all control objects) and both ordered lists
	Controls[UID] = pObject;
	iterator = Controls.find( UID );
	if ( iterator == Controls.end() ) ERRORMSG( ERRGENERIC, "CGUIDialog::AddLabel()", "Unable to add object to searching structures.");
    hr = AddItemToOrderedLists( iterator );
	if ( hr ) ERRORMSG( hr, "CGUIDialog::AddLabel()", "Unable to add item to preordered structures." );


	*newID = UID; // set the output parameter
	iLastUID++; // increase UID counter

	return ERRNOERROR;
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// adds a label to the dialog with full customized setting
//
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT CGUIDialog::AddLabelEx(		GUICONTROLDECLARATION * decl, 
									LPCTSTR caption, 
									GUICONTROLID * newID )
{
	HRESULT					hr;
	CGUIControlLabel	*	pObject;
	int						UID = iLastUID + 1;
	CONTROLS_ITER			iterator;


	// just in case if anyone want to (re)create more than 2 billions objects - structures could crash in such situation
	if ( UID > 2000000000 ) ERRORMSG( ERRTOOMANYRESOURCES, "CGUIDialog::AddLabelEx()", "Can't add any more control objects." )

	// clear the UID
	*newID = 0;


	// create and initialize new control object
	pObject = new CGUIControlLabel( caption );
	if ( !pObject )	ERRORMSG( ERROUTOFMEMORY, "CGUIDialog::AddLabelEx()", "Can't create new label object." )

	hr = pObject->Init( decl );
	if ( hr ) ERRORMSG( hr, "CGUIDialog::AddLabelEx()", "Can't initialize label object." )
	pObject->UID = UID;

	
	// add the object to the binary tree (list of all control objects) and both ordered lists
	Controls[UID] = pObject;
	iterator = Controls.find( UID );
	if ( iterator == Controls.end() ) ERRORMSG( ERRGENERIC, "CGUIDialog::AddLabelEx()", "Unable to add object to searching structures.");
    hr = AddItemToOrderedLists( iterator );
	if ( hr ) ERRORMSG( hr, "CGUIDialog::AddLabelEx()", "Unable to add item to preordered structures." );


	*newID = UID; // set the output parameter
	iLastUID++; // increase UID counter

	return ERRNOERROR;
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// adds a button to the dialog with full customized setting
//
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT CGUIDialog::AddButton(	int x, int y, 
								int width, int height, 
								LPCTSTR caption, 
								ONEVENTCALLBACKPTR lpOnUse, 
								GUICONTROLID * newID )
{
	GUICONTROLDECLARATION	decl;
	HRESULT					hr;
	CGUIControlButton	*	pObject;
	int						UID = iLastUID + 1;
	CONTROLS_ITER			iterator;


	// just in case if anyone want to (re)create more than 2 billions objects - structures could crash in such situation
	if ( UID > 2000000000 ) ERRORMSG( ERRTOOMANYRESOURCES, "CGUIDialog::AddButton()", "Can't add any more control objects." )

	// clear the UID
	*newID = 0;

	// fill the declaration structure with proper data
	ZeroMemory( &decl, sizeof( decl ) );

	decl.pSharedDialogData = &pSharedDialogData;
	decl.iPosX = x;
	decl.iPosY = y;
	decl.iHeight = height;
	decl.iWidth = width;
	decl.iTabStop = ++iMaxTabStop; // the new object will have the highest tabstop; increase the maximum
	decl.iZOrder = ++iMaxZOrder; // the new object will also have the highest ZOrder index (will be topmost)
	decl.bAbsolutePosition = false;
	decl.fTransparency = fControlsTransparency;
	decl.idBackgroundTexture = idBackgroundTexture;
	decl.ridBasicFont = ridBasicFont;
	decl.uiBasicTextColor = uiBasicTextColor;
	decl.uiActiveTextColor = uiActiveTextColor;
	decl.uiColor = uiColor;
	decl.TexturePos1 = tpButtonUp;
	decl.TexturePos2 = tpButtonDown;
	decl.cToolTip[0] = 0;


	// create and initialize new control object
	pObject = new CGUIControlButton( caption );
	if ( !pObject )	ERRORMSG( ERROUTOFMEMORY, "CGUIDialog::AddButton()", "Can't create new button object." )

	hr = pObject->Init( &decl );
	if ( hr ) ERRORMSG( hr, "CGUIDialog::AddButton()", "Can't initialize button object." )
	pObject->UID = UID;

	
	// add the object to the binary tree (list of all control objects) and both ordered lists
	//Controls.insert( GUICONTROL_PAIR( UID, pObject ) );
	Controls[UID] = pObject;
	iterator = Controls.find( UID );
	if ( iterator == Controls.end() ) ERRORMSG( ERRGENERIC, "CGUIDialog::AddButton()", "Unable to add object to searching structures.");
    hr = AddItemToOrderedLists( iterator );
	if ( hr ) ERRORMSG( hr, "CGUIDialog::AddButton()", "Unable to add item to preordered structures." );


	pObject->lpOnMouseClick = lpOnUse;


	*newID = UID; // set the output parameter
	iLastUID++; // increase UID counter

	return ERRNOERROR;
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// adds a button to the dialog with full customized setting
//
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT CGUIDialog::AddButtonEx(	GUICONTROLDECLARATION * decl, 
									LPCTSTR caption, 
									GUICONTROLID * newID )

{
	HRESULT					hr;
	CGUIControlButton	*	pObject;
	int						UID = iLastUID + 1;
	CONTROLS_ITER			iterator;


	// just in case if anyone want to (re)create more than 2 billions objects - structures could crash in such situation
	if ( UID > 2000000000 ) ERRORMSG( ERRTOOMANYRESOURCES, "CGUIDialog::AddButtonEx()", "Can't add any more control objects." )

	// clear the UID
	*newID = 0;


	// create and initialize new control object
	pObject = new CGUIControlButton( caption );
	if ( !pObject )	ERRORMSG( ERROUTOFMEMORY, "CGUIDialog::AddButtonEx()", "Can't create new button object." )

	hr = pObject->Init( decl );
	if ( hr ) ERRORMSG( hr, "CGUIDialog::AddButtonEx()", "Can't initialize button object." )
	pObject->UID = UID;

	
	// add the object to the binary tree (list of all control objects) and both ordered lists
	Controls[UID] = pObject;
	iterator = Controls.find( UID );
	if ( iterator == Controls.end() ) ERRORMSG( ERRGENERIC, "CGUIDialog::AddButtonEx()", "Unable to add object to searching structures.");
    hr = AddItemToOrderedLists( iterator );
	if ( hr ) ERRORMSG( hr, "CGUIDialog::AddButtonEx()", "Unable to add item to preordered structures." );


	pObject->lpOnMouseClick = decl->lpOnUse;


	*newID = UID; // set the output parameter
	iLastUID++; // increase UID counter

	return ERRNOERROR;
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// adds a checkbox to the dialog with standard dialog setting
//
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT CGUIDialog::AddCheckBox(	int x, int y,
									int width, int height,
									LPCTSTR text,
									bool checked,
									GUICONTROLID * newID )
{
	GUICONTROLDECLARATION	decl;
	HRESULT					hr;
	CGUIControlCheckBox	*	pObject;
	int						UID = iLastUID + 1;
	CONTROLS_ITER			iterator;


	// just in case if anyone want to (re)create more than 2 billions objects - structures could crash in such situation
	if ( UID > 2000000000 ) ERRORMSG( ERRTOOMANYRESOURCES, "CGUIDialog::AddCheckBox()", "Can't add any more control objects." )

	// clear the UID
	*newID = 0;

	// fill the declaration structure with proper data
	ZeroMemory( &decl, sizeof( decl ) );

	decl.pSharedDialogData = &pSharedDialogData;
	decl.iPosX = x;
	decl.iPosY = y;
	decl.iHeight = height;
	decl.iWidth = width;
	decl.iTabStop = ++iMaxTabStop; // the new object will have the highest tabstop; increase the maximum
	decl.iZOrder = ++iMaxZOrder; // the new object will also have the highest ZOrder index (will be topmost)
	decl.bAbsolutePosition = false;
	decl.fTransparency = fControlsTransparency;
	decl.idBackgroundTexture = idBackgroundTexture;
	decl.ridBasicFont = ridBasicFont;
	decl.uiBasicTextColor = uiBasicTextColor;
	decl.uiActiveTextColor = uiActiveTextColor;
	decl.uiColor = uiColor;
	decl.TexturePos1 = tpCheckBoxUnchecked;
	decl.TexturePos2 = tpCheckBoxChecked;
	decl.bSelected = checked;
	decl.cToolTip[0] = 0;


	// create and initialize new control object
	pObject = new CGUIControlCheckBox( text );
	if ( !pObject )	ERRORMSG( ERROUTOFMEMORY, "CGUIDialog::AddCheckBox()", "Can't create new checkbox object." )

	hr = pObject->Init( &decl );
	if ( hr ) ERRORMSG( hr, "CGUIDialog::AddCheckBox()", "Can't initialize checkbox object." )
 	pObject->UID = UID;

	
	// add the object to the binary tree (list of all control objects) and both ordered lists
	Controls[UID] = pObject;
	iterator = Controls.find( UID );
	if ( iterator == Controls.end() ) ERRORMSG( ERRGENERIC, "CGUIDialog::AddCheckBox()", "Unable to add object to searching structures.");
    hr = AddItemToOrderedLists( iterator );
	if ( hr ) ERRORMSG( hr, "CGUIDialog::AddCheckBox()", "Unable to add item to preordered structures." );


	*newID = UID; // set the output parameter
	iLastUID++; // increase UID counter

	return ERRNOERROR;
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// adds a checkbox to the dialog with full customized setting
//
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT CGUIDialog::AddCheckBoxEx(	GUICONTROLDECLARATION * decl,
									LPCTSTR text,
									GUICONTROLID * newID )
{
	HRESULT					hr;
	CGUIControlCheckBox	*	pObject;
	int						UID = iLastUID + 1;
	CONTROLS_ITER			iterator;


	// just in case if anyone want to (re)create more than 2 billions objects - structures could crash in such situation
	if ( UID > 2000000000 ) ERRORMSG( ERRTOOMANYRESOURCES, "CGUIDialog::AddCheckBoxEx()", "Can't add any more control objects." )

	// clear the UID
	*newID = 0;


	// create and initialize new control object
	pObject = new CGUIControlCheckBox( text );
	if ( !pObject )	ERRORMSG( ERROUTOFMEMORY, "CGUIDialog::AddCheckBoxEx()", "Can't create new checkbox object." )

	hr = pObject->Init( decl );
	if ( hr ) ERRORMSG( hr, "CGUIDialog::AddCheckBoxEx()", "Can't initialize checkbox object." )
	pObject->UID = UID;

	
	// add the object to the binary tree (list of all control objects) and both ordered lists
	Controls[UID] = pObject;
	iterator = Controls.find( UID );
	if ( iterator == Controls.end() ) ERRORMSG( ERRGENERIC, "CGUIDialog::AddCheckBoxEx()", "Unable to add object to searching structures.");
    hr = AddItemToOrderedLists( iterator );
	if ( hr ) ERRORMSG( hr, "CGUIDialog::AddCheckBoxEx()", "Unable to add item to preordered structures." );


	*newID = UID; // set the output parameter
	iLastUID++; // increase UID counter

	return ERRNOERROR;
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// adds a trackbar to the dialog with standard dialog setting
//
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT CGUIDialog::AddTrackBar(	int x, int y,
									int width, int height,
									float value,
									GUICONTROLID * newID )
{
	GUICONTROLDECLARATION	decl;
	HRESULT					hr;
	CGUIControlTrackBar	*	pObject;
	int						UID = iLastUID + 1;
	CONTROLS_ITER			iterator;


	// just in case if anyone want to (re)create more than 2 billions objects - structures could crash in such situation
	if ( UID > 2000000000 ) ERRORMSG( ERRTOOMANYRESOURCES, "CGUIDialog::AddTrackBar()", "Can't add any more control objects." )

	// clear the UID
	*newID = 0;

	// fill the declaration structure with proper data
	ZeroMemory( &decl, sizeof( decl ) );

	decl.pSharedDialogData = &pSharedDialogData;
	decl.iPosX = x;
	decl.iPosY = y;
	decl.iHeight = height;
	decl.iWidth = width;
	decl.iTabStop = ++iMaxTabStop; // the new object will have the highest tabstop; increase the maximum
	decl.iZOrder = ++iMaxZOrder; // the new object will also have the highest ZOrder index (will be topmost)
	decl.bAbsolutePosition = false;
	decl.fTransparency = fControlsTransparency;
	decl.idBackgroundTexture = idBackgroundTexture;
	decl.ridBasicFont = ridBasicFont;
	decl.uiBasicTextColor = uiBasicTextColor;
	decl.uiActiveTextColor = uiActiveTextColor;
	decl.uiColor = uiColor;
	decl.TexturePos1 = tpTrackBarTrack;
	decl.TexturePos2 = tpTrackBarSlider;
	decl.iSliderPosMin = iSliderPosMin;
	decl.iSliderPosMax = iSliderPosMax;
	decl.cToolTip[0] = 0;


	// create and initialize new control object
	pObject = new CGUIControlTrackBar( value );
	if ( !pObject )	ERRORMSG( ERROUTOFMEMORY, "CGUIDialog::AddTrackBar()", "Can't create new track bar object." )

	hr = pObject->Init( &decl );
	if ( hr ) ERRORMSG( hr, "CGUIDialog::AddTrackBar()", "Can't initialize track bar object." )
	pObject->UID = UID;

	
	// add the object to the binary tree (list of all control objects) and both ordered lists
	Controls[UID] = pObject;
	iterator = Controls.find( UID );
	if ( iterator == Controls.end() ) ERRORMSG( ERRGENERIC, "CGUIDialog::AddTrackBarEx()", "Unable to add object to searching structures.");
    hr = AddItemToOrderedLists( iterator );
	if ( hr ) ERRORMSG( hr, "CGUIDialog::AddTrackBar()", "Unable to add item to preordered structures." );


	*newID = UID; // set the output parameter
	iLastUID++; // increase UID counter

	return ERRNOERROR;
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// adds a trackbar to the dialog with full customized setting
//
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT CGUIDialog::AddTrackBarEx(	GUICONTROLDECLARATION * decl,
									float value,
									GUICONTROLID * newID )
{
	HRESULT					hr;
	CGUIControlTrackBar	*	pObject;
	int						UID = iLastUID + 1;
	CONTROLS_ITER			iterator;


	// just in case if anyone want to (re)create more than 2 billions objects - structures could crash in such situation
	if ( UID > 2000000000 ) ERRORMSG( ERRTOOMANYRESOURCES, "CGUIDialog::AddTrackBarEx()", "Can't add any more control objects." )

	// clear the UID
	*newID = 0;


	// create and initialize new control object
	pObject = new CGUIControlTrackBar( value );
	if ( !pObject )	ERRORMSG( ERROUTOFMEMORY, "CGUIDialog::AddTrackBarEx()", "Can't create new track bar object." )

	hr = pObject->Init( decl );
	if ( hr ) ERRORMSG( hr, "CGUIDialog::AddTrackBarEx()", "Can't initialize track bar object." )
	pObject->UID = UID;

	
	// add the object to the binary tree (list of all control objects) and both ordered lists
	Controls[UID] = pObject;
	iterator = Controls.find( UID );
	if ( iterator == Controls.end() ) ERRORMSG( ERRGENERIC, "CGUIDialog::AddButtonEx()", "Unable to add object to searching structures.");
    hr = AddItemToOrderedLists( iterator );
	if ( hr ) ERRORMSG( hr, "CGUIDialog::AddTrackBarEx()", "Unable to add item to preordered structures." );


	*newID = UID; // set the output parameter
	iLastUID++; // increase UID counter

	return ERRNOERROR;
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// adds a radio button to the dialog with standard dialog setting
//
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT CGUIDialog::AddRadioButton(	int x, int y,
									int width, int height,
									LPCTSTR text,
									bool selected,
									int groupIndex, 
									GUICONTROLID * newID )
{
	GUICONTROLDECLARATION	decl;
	HRESULT					hr;
	CGUIControlRadioButton *pObject;
	int						UID = iLastUID + 1;
	CONTROLS_ITER			iterator;


	// just in case if anyone want to (re)create more than 2 billions objects - structures could crash in such situation
	if ( UID > 2000000000 ) ERRORMSG( ERRTOOMANYRESOURCES, "CGUIDialog::AddRadioButton()", "Can't add any more control objects." )

	// clear the UID
	*newID = 0;

	// fill the declaration structure with proper data
	ZeroMemory( &decl, sizeof( decl ) );

	decl.pSharedDialogData = &pSharedDialogData;
	decl.iPosX = x;
	decl.iPosY = y;
	decl.iHeight = height;
	decl.iWidth = width;
	decl.iTabStop = ++iMaxTabStop; // the new object will have the highest tabstop; increase the maximum
	decl.iZOrder = ++iMaxZOrder; // the new object will also have the highest ZOrder index (will be topmost)
	decl.bAbsolutePosition = false;
	decl.fTransparency = fControlsTransparency;
	decl.idBackgroundTexture = idBackgroundTexture;
	decl.ridBasicFont = ridBasicFont;
	decl.uiBasicTextColor = uiBasicTextColor;
	decl.uiActiveTextColor = uiActiveTextColor;
	decl.uiColor = uiColor;
	decl.TexturePos1 = tpRadioButtonUnselected;
	decl.TexturePos2 = tpRadioButtonSelected;
	decl.bSelected = selected;
	decl.iRadioGroupIndex = groupIndex;
	decl.cToolTip[0] = 0;


	// create and initialize new control object
	pObject = new CGUIControlRadioButton( text );
	if ( !pObject )	ERRORMSG( ERROUTOFMEMORY, "CGUIDialog::AddRadioButton()", "Can't create new radio button object." )

	hr = pObject->Init( &decl );
	if ( hr ) ERRORMSG( hr, "CGUIDialog::AddRadioButton()", "Can't initialize radio button object." )
	pObject->UID = UID;

	
	// add the object to the binary tree (list of all control objects) and both ordered lists
	Controls[UID] = pObject;
	iterator = Controls.find( UID );
	if ( iterator == Controls.end() ) ERRORMSG( ERRGENERIC, "CGUIDialog::AddRadioButton()", "Unable to add object to searching structures.");
    hr = AddItemToOrderedLists( iterator );
	if ( hr ) ERRORMSG( hr, "CGUIDialog::AddRadioButton()", "Unable to add item to preordered structures." );


	*newID = UID; // set the output parameter
	iLastUID++; // increase UID counter

	return ERRNOERROR;
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// adds a radio button to the dialog with full customized setting
//
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT CGUIDialog::AddRadioButtonEx(	GUICONTROLDECLARATION * decl,
										LPCTSTR text,
										GUICONTROLID * newID )
{
	HRESULT					hr;
	CGUIControlRadioButton *pObject;
	int						UID = iLastUID + 1;
	CONTROLS_ITER			iterator;


	// just in case if anyone want to (re)create more than 2 billions objects - structures could crash in such situation
	if ( UID > 2000000000 ) ERRORMSG( ERRTOOMANYRESOURCES, "CGUIDialog::AddRadioButtonEx()", "Can't add any more control objects." )

	// clear the UID
	*newID = 0;


	// create and initialize new control object
	pObject = new CGUIControlRadioButton( text );
	if ( !pObject )	ERRORMSG( ERROUTOFMEMORY, "CGUIDialog::AddRadioButtonEx()", "Can't create new radio button object." )

	hr = pObject->Init( decl );
	if ( hr ) ERRORMSG( hr, "CGUIDialog::AddRadioButtonEx()", "Can't initialize radio button object." )
	pObject->UID = UID;

	
	// add the object to the binary tree (list of all control objects) and both ordered lists
	Controls[UID] = pObject;
	iterator = Controls.find( UID );
	if ( iterator == Controls.end() ) ERRORMSG( ERRGENERIC, "CGUIDialog::AddRadioButtonEx()", "Unable to add object to searching structures.");
    hr = AddItemToOrderedLists( iterator );
	if ( hr ) ERRORMSG( hr, "CGUIDialog::AddRadioButtonEx()", "Unable to add item to preordered structures." );


	*newID = UID; // set the output parameter
	iLastUID++; // increase UID counter

	return ERRNOERROR;
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// adds an edit box to the dialog with standard dialog setting
//
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT CGUIDialog::AddEditBox(	int x, int y,
								int width, int height,
								int maxTextLen, 
								GUICONTROLID * newID )
{
	GUICONTROLDECLARATION	decl;
	HRESULT					hr;
	CGUIControlEdit		*	pObject;
	int						UID = iLastUID + 1;
	CONTROLS_ITER			iterator;


	// just in case if anyone want to (re)create more than 2 billions objects - structures could crash in such situation
	if ( UID > 2000000000 ) ERRORMSG( ERRTOOMANYRESOURCES, "CGUIDialog::AddEditBox()", "Can't add any more control objects." )

	// clear the UID
	*newID = 0;

	// fill the declaration structure with proper data
	ZeroMemory( &decl, sizeof( decl ) );

	decl.pSharedDialogData = &pSharedDialogData;
	decl.iPosX = x;
	decl.iPosY = y;
	decl.iHeight = height;
	decl.iWidth = width;
	decl.iTabStop = ++iMaxTabStop; // the new object will have the highest tabstop; increase the maximum
	decl.iZOrder = ++iMaxZOrder; // the new object will also have the highest ZOrder index (will be topmost)
	decl.bAbsolutePosition = false;
	decl.fTransparency = fControlsTransparency;
	decl.idBackgroundTexture = idBackgroundTexture;
	decl.ridBasicFont = ridBasicFont;
	decl.uiBasicTextColor = uiBasicTextColor;
	decl.uiActiveTextColor = uiActiveTextColor;
	decl.uiColor = uiColor;
	decl.TexturePos1 = tpEditBackground;
	decl.TopLeftCornerPos = tpEditTopLeft;
	decl.TopRightCornerPos = tpEditTopRight;
	decl.BottomLeftCornerPos = tpEditBottomLeft;
	decl.BottomRightCornerPos = tpEditBottomRight;
	decl.HorizontalLinePos = tpEditHorizLine;
	decl.VerticalLinePos = tpEditVertLine;
	decl.iMaxEditTextLen = maxTextLen;
	decl.cToolTip[0] = 0;


	// create and initialize new control object
	pObject = new CGUIControlEdit();
	if ( !pObject )	ERRORMSG( ERROUTOFMEMORY, "CGUIDialog::AddEditBox()", "Can't create new edit box object." )

	hr = pObject->Init( &decl );
	if ( hr ) ERRORMSG( hr, "CGUIDialog::AddEditBox()", "Can't initialize edit box object." )
	pObject->UID = UID;

	
	// add the object to the binary tree (list of all control objects) and both ordered lists
	Controls[UID] = pObject;
	iterator = Controls.find( UID );
	if ( iterator == Controls.end() ) ERRORMSG( ERRGENERIC, "CGUIDialog::AddEditBox()", "Unable to add object to searching structures.");
    hr = AddItemToOrderedLists( iterator );
	if ( hr ) ERRORMSG( hr, "CGUIDialog::AddEditBox()", "Unable to add item to preordered structures." );


	*newID = UID; // set the output parameter
	iLastUID++; // increase UID counter

	return ERRNOERROR;
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// adds an edit box to the dialog with full customized setting
//
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT CGUIDialog::AddEditBoxEx(	GUICONTROLDECLARATION * decl,
									GUICONTROLID * newID )
{
	HRESULT					hr;
	CGUIControlEdit		*	pObject;
	int						UID = iLastUID + 1;
	CONTROLS_ITER			iterator;


	// just in case if anyone want to (re)create more than 2 billions objects - structures could crash in such situation
	if ( UID > 2000000000 ) ERRORMSG( ERRTOOMANYRESOURCES, "CGUIDialog::AddEditBoxEx()", "Can't add any more control objects." )

	// clear the UID
	*newID = 0;


	// create and initialize new control object
	pObject = new CGUIControlEdit();
	if ( !pObject )	ERRORMSG( ERROUTOFMEMORY, "CGUIDialog::AddEditBoxEx()", "Can't create new edit box object." )

	hr = pObject->Init( decl );
	if ( hr ) ERRORMSG( hr, "CGUIDialog::AddEditBoxEx()", "Can't initialize edit box object." )
	pObject->UID = UID;

	
	// add the object to the binary tree (list of all control objects) and both ordered lists
	Controls[UID] = pObject;
	iterator = Controls.find( UID );
	if ( iterator == Controls.end() ) ERRORMSG( ERRGENERIC, "CGUIDialog::AddEditBoxEx()", "Unable to add object to searching structures.");
    hr = AddItemToOrderedLists( iterator );
	if ( hr ) ERRORMSG( hr, "CGUIDialog::AddEditBoxEx()", "Unable to add item to preordered structures." );


	*newID = UID; // set the output parameter
	iLastUID++; // increase UID counter

	return ERRNOERROR;
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// adds a combobox to the dialog with standard dialog setting
//
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT CGUIDialog::AddComboBox(	int x, int y,
									int width, int height,
									int lineHeight,
									GUICONTROLID * newID )
{
	GUICONTROLDECLARATION	decl;
	HRESULT					hr;
	CGUIControlComboBox	*	pObject;
	int						UID = iLastUID + 1;
	CONTROLS_ITER			iterator;


	// just in case if anyone want to (re)create more than 2 billions objects - structures could crash in such situation
	if ( UID > 2000000000 ) ERRORMSG( ERRTOOMANYRESOURCES, "CGUIDialog::AddComboBox()", "Can't add any more control objects." )

	// clear the UID
	*newID = 0;

	// fill the declaration structure with proper data
	ZeroMemory( &decl, sizeof( decl ) );

	decl.pSharedDialogData = &pSharedDialogData;
	decl.iPosX = x;
	decl.iPosY = y;
	decl.iHeight = height;
	decl.iWidth = width;
	decl.iTabStop = ++iMaxTabStop; // the new object will have the highest tabstop; increase the maximum
	decl.iZOrder = ++iMaxZOrder; // the new object will also have the highest ZOrder index (will be topmost)
	decl.bAbsolutePosition = false;
	decl.fTransparency = fControlsTransparency;
	decl.idBackgroundTexture = idBackgroundTexture;
	decl.ridBasicFont = ridBasicFont;
	decl.uiBasicTextColor = uiBasicTextColor;
	decl.uiActiveTextColor = uiActiveTextColor;
	decl.uiColor = uiColor;
	decl.TexturePos1 = tpComboBackground;
	decl.TexturePos2 = tpComboActiveBackground;
	decl.TexturePos3 = tpComboButton;
	decl.TopLeftCornerPos = tpComboTopLeft;
	decl.TopRightCornerPos = tpComboTopRight;
	decl.BottomLeftCornerPos = tpComboBottomLeft;
	decl.BottomRightCornerPos = tpComboBottomRight;
	decl.HorizontalLinePos = tpComboHorizLine;
	decl.VerticalLinePos = tpComboVertLine;
	decl.uiListLineHeight = lineHeight;
	decl.cToolTip[0] = 0;


	// create and initialize new control object
	pObject = new CGUIControlComboBox();
	if ( !pObject )	ERRORMSG( ERROUTOFMEMORY, "CGUIDialog::AddComboBox()", "Can't create new combobox object." )

	hr = pObject->Init( &decl );
	if ( hr ) ERRORMSG( hr, "CGUIDialog::AddComboBox()", "Can't initialize combobox object." )
	pObject->UID = UID;

	
	// add the object to the binary tree (list of all control objects) and both ordered lists
	Controls[UID] = pObject;
	iterator = Controls.find( UID );
	if ( iterator == Controls.end() ) ERRORMSG( ERRGENERIC, "CGUIDialog::AddComboBox()", "Unable to add object to searching structures.");
    hr = AddItemToOrderedLists( iterator );
	if ( hr ) ERRORMSG( hr, "CGUIDialog::AddComboBox()", "Unable to add item to preordered structures." );


	*newID = UID; // set the output parameter
	iLastUID++; // increase UID counter



	// add popup menu object and assign it to this combobox
	GUICONTROLID			popupID;

	hr = AddPopupMenu( decl.iPosX, decl.iPosY + decl.iHeight, decl.iWidth, decl.uiListLineHeight, &popupID );
	if ( hr ) ERRORMSG( hr, "CGUIDialog::AddComboBox()", "Can't create assigned popup menu." )
	pObject->pPopupMenu = (CGUIControlPopupMenu *) GetControl( popupID );
	pObject->pPopupMenu->SetZOrder( 500000000 );
	pObject->pPopupMenu->lpOnChange = GUIComboOnMenuChangeValue;
	pObject->pPopupMenu->lpOnLostFocus = GUIComboOnMenuLostFocus;
	RebuildStructures();


	return ERRNOERROR;
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// adds a combobox to the dialog with full customized setting
//
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT CGUIDialog::AddComboBoxEx(	GUICONTROLDECLARATION * decl,
									GUICONTROLID * newID )
{
	HRESULT					hr;
	CGUIControlComboBox	*	pObject;
	int						UID = iLastUID + 1;
	CONTROLS_ITER			iterator;


	// just in case if anyone want to (re)create more than 2 billions objects - structures could crash in such situation
	if ( UID > 2000000000 ) ERRORMSG( ERRTOOMANYRESOURCES, "CGUIDialog::AddComboBoxEx()", "Can't add any more control objects." )

	// clear the UID
	*newID = 0;


	// create and initialize new control object
	pObject = new CGUIControlComboBox();
	if ( !pObject )	ERRORMSG( ERROUTOFMEMORY, "CGUIDialog::AddComboBoxEx()", "Can't create new combobox object." )

	hr = pObject->Init( decl );
	if ( hr ) ERRORMSG( hr, "CGUIDialog::AddComboBoxEx()", "Can't initialize combobox object." )
	pObject->UID = UID;

	
	// add the object to the binary tree (list of all control objects) and both ordered lists
	Controls[UID] = pObject;
	iterator = Controls.find( UID );
	if ( iterator == Controls.end() ) ERRORMSG( ERRGENERIC, "CGUIDialog::AddComboBoxEx()", "Unable to add object to searching structures.");
    hr = AddItemToOrderedLists( iterator );
	if ( hr ) ERRORMSG( hr, "CGUIDialog::AddComboBoxEx()", "Unable to add item to preordered structures." );


	*newID = UID; // set the output parameter
	iLastUID++; // increase UID counter



	// add popup menu object and assign it to this combobox
	GUICONTROLID			popupID;

	hr = AddPopupMenu( decl->iPosX, decl->iPosY + decl->iHeight, decl->iWidth, decl->uiListLineHeight, &popupID );
	if ( hr ) ERRORMSG( hr, "CGUIDialog::AddComboBoxEx()", "Can't create assigned popup menu." )
	pObject->pPopupMenu = (CGUIControlPopupMenu *) GetControl( popupID );
	pObject->pPopupMenu->SetZOrder( 500000000 );
	pObject->pPopupMenu->lpOnChange = GUIComboOnMenuChangeValue;
	pObject->pPopupMenu->lpOnLostFocus = GUIComboOnMenuLostFocus;
	RebuildStructures();


	return ERRNOERROR;
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// adds a popup menu to the dialog with standard dialog setting
//
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT CGUIDialog::AddPopupMenu(	int x, int y,
									int width,
									int lineHeight,
									GUICONTROLID * newID )
{
	GUICONTROLDECLARATION	decl;
	HRESULT					hr;
	CGUIControlPopupMenu *	pObject;
	int						UID = iLastUID + 1;
	CONTROLS_ITER			iterator;


	// just in case if anyone want to (re)create more than 2 billions objects - structures could crash in such situation
	if ( UID > 2000000000 ) ERRORMSG( ERRTOOMANYRESOURCES, "CGUIDialog::AddPopupMenu()", "Can't add any more control objects." )

	// clear the UID
	*newID = 0;

	// fill the declaration structure with proper data
	ZeroMemory( &decl, sizeof( decl ) );

	decl.pSharedDialogData = &pSharedDialogData;
	decl.iPosX = x;
	decl.iPosY = y;
	decl.iHeight = 0;
	decl.iWidth = width;
	decl.iTabStop = ++iMaxTabStop; // the new object will have the highest tabstop; increase the maximum
	decl.iZOrder = ++iMaxZOrder; // the new object will also have the highest ZOrder index (will be topmost)
	decl.bAbsolutePosition = false;
	decl.fTransparency = fControlsTransparency;
	decl.idBackgroundTexture = idBackgroundTexture;
	decl.ridBasicFont = ridBasicFont;
	decl.uiBasicTextColor = uiBasicTextColor;
	decl.uiActiveTextColor = uiActiveTextColor;
	decl.uiColor = uiColor;
	decl.TexturePos1 = tpPopupBackground;
	decl.TexturePos2 = tpPopupActiveBackground;
	decl.TopLeftCornerPos = tpPopupTopLeft;
	decl.TopRightCornerPos = tpPopupTopRight;
	decl.BottomLeftCornerPos = tpPopupBottomLeft;
	decl.BottomRightCornerPos = tpPopupBottomRight;
	decl.HorizontalLinePos = tpPopupHorizLine;
	decl.VerticalLinePos = tpPopupVertLine;
	decl.uiListLineHeight = lineHeight;
	decl.cToolTip[0] = 0;


	// create and initialize new control object
	pObject = new CGUIControlPopupMenu();
	if ( !pObject )	ERRORMSG( ERROUTOFMEMORY, "CGUIDialog::AddPopupMenu()", "Can't create new popup menu object." )

	hr = pObject->Init( &decl );
	if ( hr ) ERRORMSG( hr, "CGUIDialog::AddPopupMenu()", "Can't initialize popup menu object." )
	pObject->UID = UID;

	
	// add the object to the binary tree (list of all control objects) and both ordered lists
	Controls[UID] = pObject;
	iterator = Controls.find( UID );
	if ( iterator == Controls.end() ) ERRORMSG( ERRGENERIC, "CGUIDialog::AddPopupMenu()", "Unable to add object to searching structures.");
    hr = AddItemToOrderedLists( iterator );
	if ( hr ) ERRORMSG( hr, "CGUIDialog::AddPopupMenu()", "Unable to add item to preordered structures." );


	*newID = UID; // set the output parameter
	iLastUID++; // increase UID counter

	return ERRNOERROR;
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// adds a popup menu to the dialog with full customized setting
//
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT CGUIDialog::AddPopupMenuEx(	GUICONTROLDECLARATION * decl,
									GUICONTROLID * newID )
{
	HRESULT					hr;
	CGUIControlPopupMenu *	pObject;
	int						UID = iLastUID + 1;
	CONTROLS_ITER			iterator;


	// just in case if anyone want to (re)create more than 2 billions objects - structures could crash in such situation
	if ( UID > 2000000000 ) ERRORMSG( ERRTOOMANYRESOURCES, "CGUIDialog::AddPopupMenuEx()", "Can't add any more control objects." )

	// clear the UID
	*newID = 0;


	// create and initialize new control object
	pObject = new CGUIControlPopupMenu();
	if ( !pObject )	ERRORMSG( ERROUTOFMEMORY, "CGUIDialog::AddPopupMenuEx()", "Can't create new popup menu object." )

	hr = pObject->Init( decl );
	if ( hr ) ERRORMSG( hr, "CGUIDialog::AddPopupMenuEx()", "Can't initialize popup menu object." )
	pObject->UID = UID;

	
	// add the object to the binary tree (list of all control objects) and both ordered lists
	Controls[UID] = pObject;
	iterator = Controls.find( UID );
	if ( iterator == Controls.end() ) ERRORMSG( ERRGENERIC, "CGUIDialog::AddPopupMenuEx()", "Unable to add object to searching structures.");
    hr = AddItemToOrderedLists( iterator );
	if ( hr ) ERRORMSG( hr, "CGUIDialog::AddPopupMenuEx()", "Unable to add item to preordered structures." );


	*newID = UID; // set the output parameter
	iLastUID++; // increase UID counter

	return ERRNOERROR;
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// adds a listbox to the dialog with standard dialog setting
//
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT	CGUIDialog::AddListBox(	int x, int y,
							   int width,
							   int linesCount,
							   int lineHeight,
							   GUICONTROLID * newID )
{
	GUICONTROLDECLARATION	decl;
	HRESULT					hr;
	CGUIControlListBox *	pObject;
	int						UID = iLastUID + 1;
	CONTROLS_ITER			iterator;


	// just in case if anyone want to (re)create more than 2 billions objects - structures could crash in such situation
	if ( UID > 2000000000 ) ERRORMSG( ERRTOOMANYRESOURCES, "CGUIDialog::AddListBox()", "Can't add any more control objects." )

	// clear the UID
	*newID = 0;

	// fill the declaration structure with proper data
	ZeroMemory( &decl, sizeof( decl ) );

	decl.pSharedDialogData = &pSharedDialogData;
	decl.iPosX = x;
	decl.iPosY = y;
	decl.iHeight = 1;
	decl.iWidth = width;
	decl.iTabStop = ++iMaxTabStop; // the new object will have the highest tabstop; increase the maximum
	decl.iZOrder = ++iMaxZOrder; // the new object will also have the highest ZOrder index (will be topmost)
	decl.bAbsolutePosition = false;
	decl.fTransparency = fControlsTransparency;
	decl.idBackgroundTexture = idBackgroundTexture;
	decl.ridBasicFont = ridBasicFont;
	decl.uiBasicTextColor = uiBasicTextColor;
	decl.uiActiveTextColor = uiActiveTextColor;
	decl.uiColor = uiColor;
	decl.TexturePos1 = tpListBoxBackground;
	decl.TexturePos2 = tpListBoxActiveBackground;
	decl.TexturePos3 = tpListBoxButtonUp;
	decl.TexturePos4 = tpListBoxButtonDown;
	decl.TopLeftCornerPos = tpListBoxTopLeft;
	decl.TopRightCornerPos = tpListBoxTopRight;
	decl.BottomLeftCornerPos = tpListBoxBottomLeft;
	decl.BottomRightCornerPos = tpListBoxBottomRight;
	decl.HorizontalLinePos = tpListBoxHorizLine;
	decl.VerticalLinePos = tpListBoxVertLine;
	decl.uiListLineHeight = lineHeight;
	decl.uiLinesCount = linesCount;
	decl.cToolTip[0] = 0;


	// create and initialize new control object
	pObject = new CGUIControlListBox();
	if ( !pObject )	ERRORMSG( ERROUTOFMEMORY, "CGUIDialog::AddListBox()", "Can't create new listbox object." )

	hr = pObject->Init( &decl );
	if ( hr ) ERRORMSG( hr, "CGUIDialog::AddListBox()", "Can't initialize listbox object." )
	pObject->UID = UID;

	
	// add the object to the binary tree (list of all control objects) and both ordered lists
	Controls[UID] = pObject;
	iterator = Controls.find( UID );
	if ( iterator == Controls.end() ) ERRORMSG( ERRGENERIC, "CGUIDialog::AddListBox()", "Unable to add object to searching structures.");
    hr = AddItemToOrderedLists( iterator );
	if ( hr ) ERRORMSG( hr, "CGUIDialog::AddListBox()", "Unable to add item to preordered structures." );


	*newID = UID; // set the output parameter
	iLastUID++; // increase UID counter

	return ERRNOERROR;
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// adds a listbox to the dialog with fully customized setting
//
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT	CGUIDialog::AddListBoxEx(	GUICONTROLDECLARATION * decl,
									GUICONTROLID * newID )
{
	HRESULT					hr;
	CGUIControlListBox *	pObject;
	int						UID = iLastUID + 1;
	CONTROLS_ITER			iterator;


	// just in case if anyone want to (re)create more than 2 billions of objects - structures could crash in such situation
	if ( UID > 2000000000 ) ERRORMSG( ERRTOOMANYRESOURCES, "CGUIDialog::AddListBoxEx()", "Can't add any more control objects." )

	// clear the UID
	*newID = 0;


	// create and initialize new control object
	pObject = new CGUIControlListBox();
	if ( !pObject )	ERRORMSG( ERROUTOFMEMORY, "CGUIDialog::AddListBoxEx()", "Can't create new listbox object." )

	hr = pObject->Init( decl );
	if ( hr ) ERRORMSG( hr, "CGUIDialog::AddListBoxEx()", "Can't initialize listbox object." )
	pObject->UID = UID;

	
	// add the object to the binary tree (list of all control objects) and both ordered lists
	Controls[UID] = pObject;
	iterator = Controls.find( UID );
	if ( iterator == Controls.end() ) ERRORMSG( ERRGENERIC, "CGUIDialog::AddListBoxEx()", "Unable to add object to searching structures.");
    hr = AddItemToOrderedLists( iterator );
	if ( hr ) ERRORMSG( hr, "CGUIDialog::AddListBoxEx()", "Unable to add item to preordered structures." );


	*newID = UID; // set the output parameter
	iLastUID++; // increase UID counter

	return ERRNOERROR;
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// adds an iconed listbox to the dialog with standard dialog setting
//
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT	CGUIDialog::AddIconedList(	int x, int y,
									int width,
									int iconWidth,
									int linesCount,
									int lineHeight,
									GUICONTROLID * newID )
{
	GUICONTROLDECLARATION	decl;
	HRESULT					hr;
	CGUIControlIconedList *	pObject;
	int						UID = iLastUID + 1;
	CONTROLS_ITER			iterator;


	// just in case if anyone want to (re)create more than 2 billions objects - structures could crash in such situation
	if ( UID > 2000000000 ) ERRORMSG( ERRTOOMANYRESOURCES, "CGUIDialog::AddIconedList()", "Can't add any more control objects." )

	// clear the UID
	*newID = 0;

	// fill the declaration structure with proper data
	ZeroMemory( &decl, sizeof( decl ) );

	decl.pSharedDialogData = &pSharedDialogData;
	decl.iPosX = x;
	decl.iPosY = y;
	decl.iHeight = 1;
	decl.iWidth = width;
	decl.iTabStop = ++iMaxTabStop; // the new object will have the highest tabstop; increase the maximum
	decl.iZOrder = ++iMaxZOrder; // the new object will also have the highest ZOrder index (will be topmost)
	decl.bAbsolutePosition = false;
	decl.fTransparency = fControlsTransparency;
	decl.idBackgroundTexture = idBackgroundTexture;
	decl.ridBasicFont = ridBasicFont;
	decl.uiBasicTextColor = uiBasicTextColor;
	decl.uiActiveTextColor = uiActiveTextColor;
	decl.uiColor = uiColor;
	decl.TexturePos1 = tpListBoxBackground;
	decl.TexturePos2 = tpListBoxActiveBackground;
	decl.TexturePos3 = tpListBoxButtonUp;
	decl.TexturePos4 = tpListBoxButtonDown;
	decl.TopLeftCornerPos = tpListBoxTopLeft;
	decl.TopRightCornerPos = tpListBoxTopRight;
	decl.BottomLeftCornerPos = tpListBoxBottomLeft;
	decl.BottomRightCornerPos = tpListBoxBottomRight;
	decl.HorizontalLinePos = tpListBoxHorizLine;
	decl.VerticalLinePos = tpListBoxVertLine;
	decl.uiListLineHeight = lineHeight;
	decl.uiLinesCount = linesCount;
	decl.uiListIconWidth = iconWidth;
	decl.cToolTip[0] = 0;


	// create and initialize new control object
	pObject = new CGUIControlIconedList();
	if ( !pObject )	ERRORMSG( ERROUTOFMEMORY, "CGUIDialog::AddIconedList()", "Can't create new listbox object." )

	hr = pObject->Init( &decl );
	if ( hr ) ERRORMSG( hr, "CGUIDialog::AddIconedList()", "Can't initialize listbox object." )
	pObject->UID = UID;

	
	// add the object to the binary tree (list of all control objects) and both ordered lists
	Controls[UID] = pObject;
	iterator = Controls.find( UID );
	if ( iterator == Controls.end() ) ERRORMSG( ERRGENERIC, "CGUIDialog::AddIconedList()", "Unable to add object to searching structures.");
    hr = AddItemToOrderedLists( iterator );
	if ( hr ) ERRORMSG( hr, "CGUIDialog::AddIconedList()", "Unable to add item to preordered structures." );


	*newID = UID; // set the output parameter
	iLastUID++; // increase UID counter

	return ERRNOERROR;
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// adds an iconed listbox to the dialog with full customized setting
//
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT	CGUIDialog::AddIconedListEx(	GUICONTROLDECLARATION * decl,
										GUICONTROLID * newID )
{
	HRESULT					hr;
	CGUIControlIconedList *	pObject;
	int						UID = iLastUID + 1;
	CONTROLS_ITER			iterator;


	// just in case if anyone want to (re)create more than 2 billions of objects - structures could crash in such situation
	if ( UID > 2000000000 ) ERRORMSG( ERRTOOMANYRESOURCES, "CGUIDialog::AddIconedListEx()", "Can't add any more control objects." )

	// clear the UID
	*newID = 0;


	// create and initialize new control object
	pObject = new CGUIControlIconedList();
	if ( !pObject )	ERRORMSG( ERROUTOFMEMORY, "CGUIDialog::AddIconedListEx()", "Can't create new listbox object." )

	hr = pObject->Init( decl );
	if ( hr ) ERRORMSG( hr, "CGUIDialog::AddIconedListEx()", "Can't initialize listbox object." )
	pObject->UID = UID;

	
	// add the object to the binary tree (list of all control objects) and both ordered lists
	Controls[UID] = pObject;
	iterator = Controls.find( UID );
	if ( iterator == Controls.end() ) ERRORMSG( ERRGENERIC, "CGUIDialog::AddIconedListEx()", "Unable to add object to searching structures.");
    hr = AddItemToOrderedLists( iterator );
	if ( hr ) ERRORMSG( hr, "CGUIDialog::AddIconedListEx()", "Unable to add item to preordered structures." );


	*newID = UID; // set the output parameter
	iLastUID++; // increase UID counter

	return ERRNOERROR;
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// adds an image to the dialog with standard dialog setting
//
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT CGUIDialog::AddImage(	int x, int y,
								int width, int height,
								LPCTSTR fileName,
								GUICONTROLID * newID )
{
	GUICONTROLDECLARATION	decl;
	HRESULT					hr;
	CGUIControlImage	*	pObject;
	int						UID = iLastUID + 1;
	CONTROLS_ITER			iterator;
	CString					str;


	// just in case if anyone want to (re)create more than 2 billions objects - structures could crash in such situation
	if ( UID > 2000000000 ) ERRORMSG( ERRTOOMANYRESOURCES, "CGUIDialog::AddImage()", "Can't add any more control objects." )

	// clear the UID
	*newID = 0;

	// fill the declaration structure with proper data
	ZeroMemory( &decl, sizeof( decl ) );

	decl.pSharedDialogData = &pSharedDialogData;
	decl.iPosX = x;
	decl.iPosY = y;
	decl.iHeight = height;
	decl.iWidth = width;
	decl.iTabStop = ++iMaxTabStop; // the new object will have the highest tabstop; increase the maximum
	decl.iZOrder = ++iMaxZOrder; // the new object will also have the highest ZOrder index (will be topmost)
	decl.bAbsolutePosition = false;
	decl.fTransparency = fControlsTransparency;
	decl.idBackgroundTexture = idBackgroundTexture;
	decl.ridBasicFont = ridBasicFont;
	decl.uiBasicTextColor = uiBasicTextColor;
	decl.uiActiveTextColor = uiActiveTextColor;
	decl.uiColor = uiColor;
	decl.TexturePos1 = tpImageBackground;
	decl.cToolTip[0] = 0;


	// create and initialize new control object
	pObject = new CGUIControlImage();
	if ( !pObject )	ERRORMSG( ERROUTOFMEMORY, "CGUIDialog::AddImage()", "Can't create new image object." )

	hr = pObject->Init( &decl );
	if ( hr ) ERRORMSG( hr, "CGUIDialog::AddImage()", "Can't initialize image object." )
	pObject->UID = UID;

	str = fileName;
	if ( str.GetLength() )
	{
		hr = pObject->LoadImage( fileName, NULL );
		if ( hr ) ERRORMSG( hr, "CGUIDialog::AddImage()", "Can't load image to GUIImage object." )
	}
	
	// add the object to the binary tree (list of all control objects) and both ordered lists
	//Controls.insert( GUICONTROL_PAIR( UID, pObject ) );
	Controls[UID] = pObject;
	iterator = Controls.find( UID );
	if ( iterator == Controls.end() ) ERRORMSG( ERRGENERIC, "CGUIDialog::AddImage()", "Unable to add object to searching structures.");
    hr = AddItemToOrderedLists( iterator );
	if ( hr ) ERRORMSG( hr, "CGUIDialog::AddImage()", "Unable to add item to preordered structures." );


	*newID = UID; // set the output parameter
	iLastUID++; // increase UID counter

	return ERRNOERROR;
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// adds an image to the dialog with full customized setting
//
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT CGUIDialog::AddImageEx(	GUICONTROLDECLARATION * decl,
								LPCTSTR fileName,
								GUICONTROLID * newID )
{
	HRESULT					hr;
	CGUIControlImage	*	pObject;
	int						UID = iLastUID + 1;
	CONTROLS_ITER			iterator;
	CString					str;


	// just in case if anyone want to (re)create more than 2 billions objects - structures could crash in such situation
	if ( UID > 2000000000 ) ERRORMSG( ERRTOOMANYRESOURCES, "CGUIDialog::AddImageEx()", "Can't add any more control objects." )

	// clear the UID
	*newID = 0;


	// create and initialize new control object
	pObject = new CGUIControlImage();
	if ( !pObject )	ERRORMSG( ERROUTOFMEMORY, "CGUIDialog::AddImageEx()", "Can't create new image object." )

	hr = pObject->Init( decl );
	if ( hr ) ERRORMSG( hr, "CGUIDialog::AddImageEx()", "Can't initialize image object." )
	pObject->UID = UID;

	str = fileName;
	if ( str.GetLength() )
	{
		hr = pObject->LoadImage( fileName, NULL );
		if ( hr ) ERRORMSG( hr, "CGUIDialog::AddImage()", "Can't load image to GUIImage object." )
	}
	
	// add the object to the binary tree (list of all control objects) and both ordered lists
	Controls[UID] = pObject;
	iterator = Controls.find( UID );
	if ( iterator == Controls.end() ) ERRORMSG( ERRGENERIC, "CGUIDialog::AddImageEx()", "Unable to add object to searching structures.");
    hr = AddItemToOrderedLists( iterator );
	if ( hr ) ERRORMSG( hr, "CGUIDialog::AddImageEx()", "Unable to add item to preordered structures." );


	*newID = UID; // set the output parameter
	iLastUID++; // increase UID counter

	return ERRNOERROR;
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// this adds new item to ordered lists - both of them ZOrder and TabStop
// iter parameter is an iterator used by binary tree structure containing all controls
// also enlrages and/or (re)creates both fileds if needed
// should be called whenever any control object is added to the controls binary tree
//
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT CGUIDialog::AddItemToOrderedLists( CONTROLS_ITER iter )
{
	int		size = (int) Controls.size();
	int		newBufferLen = iBufferLenght;

	iBufferLenght = 0;
		
	if ( !newBufferLen ) // buffers must be created first
	{
		newBufferLen = GUIDIALOG_BUFFERGRANULARITY;
		pZOrderList = (LPSORTINGSTRUCTURE) malloc( sizeof( SORTINGSTRUCTURE ) * newBufferLen );
		pTabStopList = (LPSORTINGSTRUCTURE) malloc( sizeof( SORTINGSTRUCTURE ) * newBufferLen );
		if ( !pZOrderList || !pTabStopList ) 
			ERRORMSG( ERROUTOFMEMORY, "CGUIDialog::AddItemToOrderedLists()", "Can't (re)create dialog sorting structure." )
	}
	else if ( newBufferLen < size ) // buffers must be recreated (are not long enough)
	{
		newBufferLen += GUIDIALOG_BUFFERGRANULARITY;
		pZOrderList = (LPSORTINGSTRUCTURE) realloc( pZOrderList, sizeof( SORTINGSTRUCTURE ) * newBufferLen );
		pTabStopList = (LPSORTINGSTRUCTURE) realloc( pTabStopList, sizeof( SORTINGSTRUCTURE ) * newBufferLen );
		if ( !pZOrderList || !pTabStopList ) 
			ERRORMSG( ERROUTOFMEMORY, "CGUIDialog::AddItemToOrderedLists()", "Can't (re)create dialog sorting structure." )
	}
	
	// set the new buffer size (doesn't have to be necessarily changed)
	iBufferLenght = newBufferLen;


	// set buffer values
	pZOrderList[size-1].iSortingKey = iter->second->iZOrder;
	pZOrderList[size-1].UID = iter->first;
	pZOrderList[size-1].pControl = iter->second;

	pTabStopList[size-1].iSortingKey = iter->second->iTabStop;
	pTabStopList[size-1].UID = iter->first;
	pTabStopList[size-1].pControl = iter->second;


	// resort both buffers
	qsort( pZOrderList, size, sizeof( SORTINGSTRUCTURE ), SortingCriterion );
	qsort( pTabStopList, size, sizeof( SORTINGSTRUCTURE ), SortingCriterion );


	return ERRNOERROR;	
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// this rebuilds preordered searching structures on the base of Controls tree
// must be called after changing ZOrder or TabStop value of any control object !!!!!
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CGUIDialog::RebuildStructures()
{
	CONTROLS_ITER	iter;
	int				ind = 0;

	// reset structure values
	for ( iter = Controls.begin(); iter != Controls.end(); iter++, ind++ )
	{
		pZOrderList[ind].iSortingKey = iter->second->iZOrder;
		pZOrderList[ind].UID = iter->first;
		pZOrderList[ind].pControl = iter->second;

		pTabStopList[ind].iSortingKey = iter->second->iTabStop;
		pTabStopList[ind].UID = iter->first;
		pTabStopList[ind].pControl = iter->second;
	}

	// sort both buffers
	qsort( pZOrderList, Controls.size(), sizeof( SORTINGSTRUCTURE ), SortingCriterion );
	qsort( pTabStopList, Controls.size(), sizeof( SORTINGSTRUCTURE ), SortingCriterion );
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// renders the dialog and all of its control objects
//
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT CGUIDialog::Render()
{
	HRESULT				hr;
	float				scaleX, scaleY;
	LPDIRECT3DTEXTURE9	texture;
	D3DXVECTOR3			position;
	D3DXMATRIX			mat;
	DWORD				dwTransparency;
	RECT				rect;
	CGUIControlBase	*	pObject;
	resManNS::__Texture		*	texStruct;


	CGUIControlBase::pD3DDevice->SetRenderState( D3DRS_ZENABLE, D3DZB_FALSE );


	// render the dialog's background
	if ( idBackgroundTexture > 0 )
	{
		// intitialize sprites rendering 
		hr = SpriteBackground->Begin( D3DXSPRITE_ALPHABLEND | D3DXSPRITE_SORT_DEPTH_BACKTOFRONT );
		if ( hr ) ERRORMSG( hr, "CGUIDialog::Render()", "Dialog surface initialization failed." );


		// obtain texture pointer
		texStruct = CGUIControlBase::pResourceManager->GetTexture( idBackgroundTexture );
		if ( !texStruct ) ERRORMSG( ERRNOTFOUND, "CGUIDialog::Render()", "Can't obtain GUI dialog's texture." );
		texture = texStruct->texture;

		scaleX = (float) pSharedDialogData.iWidth / iFileWidth;
		scaleY = (float) pSharedDialogData.iHeight / iFileHeight;

		D3DXMatrixScaling( &mat, scaleX, scaleY, 1.0f ); 
		SpriteBackground->SetTransform( &mat );
		
		position.x = (float) pSharedDialogData.iPosX / scaleX;
		position.y = (float) pSharedDialogData.iPosY / scaleY;
		position.z = 0.0f;

		rect.left = iFilePosX;
		rect.top = iFilePosY;
		rect.right = iFilePosX + iFileWidth;
		rect.bottom = iFilePosY + iFileHeight;

		dwTransparency = ( (DWORD) ( 255 * (1 - fTransparency) ) ) << 24;

		hr = SpriteBackground->Draw( texture, &rect, NULL, &position, dwTransparency + 0x00ffffff );
		if ( hr ) ERRORMSG( hr, "CGUIDialog::Render()", "Unable to draw GUI dialog's background." );


		// signalize finished rendering and draw all sprites to backbuffer
		hr = SpriteBackground->End();
		if ( hr ) ERRORMSG( hr, "CGUIDialog::Render()", "Dialog surface presentation failed." );
	}
	
  
	// render all controls ordered by ZOrder index
	for ( UINT i = 0; i < Controls.size(); i++ )
	{
		pZOrderList[i].pControl->Render();
	}


	// render actual tooltip
	pObject = GetControl( idToolTipObject );
	if ( pObject && pObject->csToolTip.GetLength() ) RenderToolTip( pObject, iToolTipPosX, iToolTipPosY );


	return ERRNOERROR;
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// draws object's tooltip at specified position
// returns a nonzero value when any error occurs
//
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT CGUIDialog::RenderToolTip( CGUIControlBase * pObject, int x, int y )
{
	UINT				format = 0;
	DWORD				dwTransparency;
	HRESULT				hr;
	RECT				textRect, rect, actRect;
	LPDIRECT3DTEXTURE9	texture;
	D3DXVECTOR3			position;
	float				scaleX, scaleY;
	D3DXMATRIX			mat;
	CString				csLine;
	int					indexStart, indexEnd;
	int					iWidth, iHeight;
	RECT				space;
	resManNS::__Texture		*	texStruct;


	
	// set "magic" constans here to make some spacing between text and tooltip frame
	space.left = 2;
	space.top = 2;
	space.right = 2;
	space.bottom = 4;




	// COMPUTE TOOLTIP PROPORTIONS

	// compute the tooltip rectangle size on the basis of text length
	actRect.left = rect.left = x;
	actRect.top = rect.top = y;
	actRect.right = rect.right = x;
	actRect.bottom = rect.bottom = y;
	textRect = actRect;
	format = DT_CALCRECT | DT_LEFT | DT_TOP | DT_NOCLIP | DT_EXPANDTABS;
	indexStart = 0;
	// compute the length of the longest line
	while ( true ) 
	{
		indexEnd = pObject->csToolTip.Find( "\n", indexStart ) + 1;
		if ( indexEnd > 0 ) csLine = pObject->csToolTip.Mid( indexStart, indexEnd - indexStart );
		else csLine = pObject->csToolTip.Mid( indexStart );

		if ( pBasicFont ) pBasicFont->DrawText( NULL, (LPCTSTR) csLine, -1, &rect, format, 0x00ffffff );
		if ( rect.right > actRect.right ) actRect.right = rect.right;

		indexStart = indexEnd;
		if ( indexEnd == 0 ) break;
	}
	
	// obtain text block height
	rect.right = actRect.right;
	if ( pBasicFont ) pBasicFont->DrawText( NULL, (LPCTSTR) pObject->csToolTip, -1, &rect, format, 0x00ffffff );
	textRect.right = rect.right;
	textRect.bottom = textRect.top + (rect.bottom - rect.top);

	// compute whole tooltip size
	iWidth = (rect.right - rect.left) + min( tpToolTipTopLeft.iWidth, tpToolTipBottomLeft.iWidth ) + 
		min( tpToolTipBottomRight.iWidth, tpToolTipTopRight.iWidth ) + space.left + space.right;
	iHeight = (rect.bottom - rect.top) + min( tpToolTipTopLeft.iHeight, tpToolTipTopRight.iHeight ) + 
		min( tpToolTipBottomRight.iHeight, tpToolTipBottomLeft.iHeight ) + space.top + space.bottom;

	actRect.right = actRect.left + iWidth - 1;
	actRect.bottom = actRect.top + iHeight - 1;

	textRect.left += space.left + min( tpToolTipTopLeft.iWidth, tpToolTipBottomLeft.iWidth );
	textRect.right += space.left + min( tpToolTipTopLeft.iWidth, tpToolTipBottomLeft.iWidth );
	textRect.top += space.top + min( tpToolTipTopLeft.iHeight, tpToolTipTopRight.iHeight );
	textRect.bottom += space.top + min( tpToolTipTopLeft.iHeight, tpToolTipTopRight.iHeight );





	// compute control's transparency
	dwTransparency = ( (DWORD) ( 255 * (1 - fToolTipTransparency) ) ) << 24;
	
	
	// obtain texture pointer
	texStruct = CGUIControlBase::pResourceManager->GetTexture( idBackgroundTexture );
	if ( !texStruct ) ERRORMSG( ERRNOTFOUND, "CGUIDialog::RenderToolTip()", "Can't obtain control's texture." )
	texture = texStruct->texture;


	// intitialize sprites rendering 
	hr = SpriteBackground->Begin( D3DXSPRITE_ALPHABLEND | D3DXSPRITE_SORT_DEPTH_BACKTOFRONT );
	if ( hr ) ERRORMSG( hr, "CGUIDialog::RenderToolTip()", "Control's surface initialization failed." );
	




	// DRAW THE TOOLTIP BACKGROUND
	
	// compute transformation
	scaleX = (float) ( iWidth - min( tpToolTipTopLeft.iWidth, tpToolTipBottomLeft.iWidth ) 
				- min( tpToolTipTopRight.iWidth, tpToolTipBottomRight.iWidth ) );
	scaleX /= tpToolTipBackground.iWidth;
	scaleY = (float) ( iHeight - min( tpToolTipTopLeft.iHeight, tpToolTipTopRight.iHeight )
				- min( tpToolTipBottomLeft.iHeight, tpToolTipBottomRight.iHeight ) );
	scaleY /= tpToolTipBackground.iHeight;

	D3DXMatrixScaling( &mat, scaleX, scaleY, 1.0f ); 
	SpriteBackground->SetTransform( &mat );

	position.x = (float) ( actRect.left + min( tpToolTipTopLeft.iWidth, tpToolTipBottomLeft.iWidth ) ) / scaleX;
	position.y = (float) ( actRect.top + min( tpToolTipTopLeft.iHeight, tpToolTipTopRight.iHeight ) ) / scaleY;
	position.z = 0.0f;

	rect.left = tpToolTipBackground.iPosX;
	rect.top = tpToolTipBackground.iPosY;
	rect.right = rect.left + tpToolTipBackground.iWidth;
	rect.bottom = rect.top + tpToolTipBackground.iHeight;

	hr = SpriteBackground->Draw( texture, &rect, NULL, &position, dwTransparency + 0x00ffffff );
	if ( hr ) ERRORMSG( hr, "CGUIDialog::RenderToolTip()", "Unable to draw control object's background1." );

	D3DXMatrixIdentity( &mat );
	SpriteBackground->SetTransform( &mat );





	// DRAW THE TOOLTIP CORNERS

	// top left corner
	position.x = (float) actRect.left;
	position.y = (float) actRect.top;
	position.z = 0.0f;

	rect.left = tpToolTipTopLeft.iPosX;
	rect.top = tpToolTipTopLeft.iPosY;
	rect.right = rect.left + tpToolTipTopLeft.iWidth;
	rect.bottom = rect.top + tpToolTipTopLeft.iHeight;

	hr = SpriteBackground->Draw( texture, &rect, NULL, &position, dwTransparency + 0x00ffffff );
	if ( hr ) ERRORMSG( hr, "CGUIDialog::RenderToolTip()", "Unable to draw control object's background2." );


 	// top right corner
	position.x = (float) ( actRect.right - tpToolTipTopRight.iWidth + 1 );
	position.y = (float) actRect.top;
	position.z = 0.0f;

	rect.left = tpToolTipTopRight.iPosX;
	rect.top = tpToolTipTopRight.iPosY;
	rect.right = rect.left + tpToolTipTopRight.iWidth;
	rect.bottom = rect.top + tpToolTipTopRight.iHeight;

	hr = SpriteBackground->Draw( texture, &rect, NULL, &position, dwTransparency + 0x00ffffff );
	if ( hr ) ERRORMSG( hr, "CGUIDialog::RenderToolTip()", "Unable to draw control object's background3." );


	// bottom left corner
	position.x = (float) actRect.left;
	position.y = (float) ( actRect.bottom - tpToolTipBottomLeft.iWidth + 1 );
	position.z = 0.0f;

	rect.left = tpToolTipBottomLeft.iPosX;
	rect.top = tpToolTipBottomLeft.iPosY;
	rect.right = rect.left + tpToolTipBottomLeft.iWidth;
	rect.bottom = rect.top + tpToolTipBottomLeft.iHeight;

	hr = SpriteBackground->Draw( texture, &rect, NULL, &position, dwTransparency + 0x00ffffff );
	if ( hr ) ERRORMSG( hr, "CGUIDialog::RenderToolTip()", "Unable to draw control object's background4." );


 	// bottom right corner
	position.x = (float) ( actRect.right - tpToolTipBottomRight.iWidth + 1 );
	position.y = (float) ( actRect.bottom - tpToolTipBottomRight.iWidth + 1 );
	position.z = 0.0f;

	rect.left = tpToolTipBottomRight.iPosX;
	rect.top = tpToolTipBottomRight.iPosY;
	rect.right = rect.left + tpToolTipBottomRight.iWidth;
	rect.bottom = rect.top + tpToolTipBottomRight.iHeight;

	hr = SpriteBackground->Draw( texture, &rect, NULL, &position, dwTransparency + 0x00ffffff );
	if ( hr ) ERRORMSG( hr, "CGUIDialog::RenderToolTip()", "Unable to draw control object's background5." );





	// DRAW THE TOOLTIP FRAME LINES

	// top horizontal line
	scaleX = (float) ( iWidth - tpToolTipTopLeft.iWidth - tpToolTipTopRight.iWidth ) / (float) tpToolTipHorizLine.iWidth;
	D3DXMatrixScaling( &mat, scaleX, 1.0f, 1.0f ); 
	SpriteBackground->SetTransform( &mat );

	position.x = (float) ( actRect.left + tpToolTipTopLeft.iWidth ) / scaleX;
	position.y = (float) actRect.top;
	position.z = 0.0f;

	rect.left = tpToolTipHorizLine.iPosX;
	rect.top = tpToolTipHorizLine.iPosY;
	rect.right = rect.left + tpToolTipHorizLine.iWidth;
	rect.bottom = rect.top + tpToolTipHorizLine.iHeight;

	hr = SpriteBackground->Draw( texture, &rect, NULL, &position, dwTransparency + 0x00ffffff );
	if ( hr ) ERRORMSG( hr, "CGUIDialog::RenderToolTip()", "Unable to draw control object's background6." );


	// bottom horizontal line
	scaleX = (float) ( iWidth - tpToolTipBottomLeft.iWidth - tpToolTipBottomRight.iWidth ) / (float) tpToolTipHorizLine.iWidth;
	D3DXMatrixScaling( &mat, scaleX, 1.0f, 1.0f ); 
	SpriteBackground->SetTransform( &mat );

	position.x = (float) ( actRect.left + tpToolTipBottomLeft.iWidth ) / scaleX;
	position.y = (float) ( actRect.bottom - tpToolTipHorizLine.iHeight + 1 );
	position.z = 0.0f;

	rect.left = tpToolTipHorizLine.iPosX;
	rect.top = tpToolTipHorizLine.iPosY;
	rect.right = rect.left + tpToolTipHorizLine.iWidth;
	rect.bottom = rect.top + tpToolTipHorizLine.iHeight;

	hr = SpriteBackground->Draw( texture, &rect, NULL, &position, dwTransparency + 0x00ffffff );
	if ( hr ) ERRORMSG( hr, "CGUIDialog::RenderToolTip()", "Unable to draw control object's background7." );


	// left vertical line
	scaleY = (float) ( iHeight - tpToolTipTopLeft.iHeight - tpToolTipBottomLeft.iHeight ) / (float) tpToolTipVertLine.iHeight;
	D3DXMatrixScaling( &mat, 1.0f, scaleY, 1.0f ); 
	SpriteBackground->SetTransform( &mat );

	position.x = (float) actRect.left;
	position.y = (float) ( actRect.top + tpToolTipTopLeft.iHeight ) / scaleY;
	position.z = 0.0f;

	rect.left = tpToolTipVertLine.iPosX;
	rect.top = tpToolTipVertLine.iPosY;
	rect.right = rect.left + tpToolTipVertLine.iWidth;
	rect.bottom = rect.top + tpToolTipVertLine.iHeight;

	hr = SpriteBackground->Draw( texture, &rect, NULL, &position, dwTransparency + 0x00ffffff );
	if ( hr ) ERRORMSG( hr, "CGUIDialog::RenderToolTip()", "Unable to draw control object's background8." );


	// right vertical line
	scaleY = (float) ( iHeight - tpToolTipTopRight.iHeight - tpToolTipBottomRight.iHeight ) / (float) tpToolTipVertLine.iHeight;
	D3DXMatrixScaling( &mat, 1.0f, scaleY, 1.0f ); 
	SpriteBackground->SetTransform( &mat );

	position.x = (float) ( actRect.right - tpToolTipVertLine.iWidth + 1 );
	position.y = (float) ( actRect.top + tpToolTipTopRight.iHeight ) / scaleY;
	position.z = 0.0f;

	rect.left = tpToolTipVertLine.iPosX;
	rect.top = tpToolTipVertLine.iPosY;
	rect.right = rect.left + tpToolTipVertLine.iWidth;
	rect.bottom = rect.top + tpToolTipVertLine.iHeight;

	hr = SpriteBackground->Draw( texture, &rect, NULL, &position, dwTransparency + 0x00ffffff );
	if ( hr ) ERRORMSG( hr, "CGUIDialog::RenderToolTip()", "Unable to draw control object's background9." );




	// signalize finished rendering and draw all sprites to backbuffer
	hr = SpriteBackground->End();
	if ( hr ) ERRORMSG( hr, "CGUIDialog::RenderToolTip()", "Control's surface presentation failed." );



	

	// DRAW TOOLTIP'S TEXT
	format = DT_LEFT | DT_TOP | DT_EXPANDTABS;
	if ( pBasicFont ) pBasicFont->DrawText( NULL, (LPCTSTR) pObject->csToolTip, -1, &textRect, format, uiBasicTextColor & (dwTransparency | 0x00ffffff) );




	return ERRNOERROR;

};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// default WndProc for dialog
//
// returns true if the message was processed
// regardless of the message type, timer is always being activated and his queue processed
//
//////////////////////////////////////////////////////////////////////////////////////////////
bool CGUIDialog::WndProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
	// process all timer events
	pSharedDialogData.pEventTimer->ProcessEvents();

	
	switch ( message ) 
	{
	case GUIMSG_TIMER:
		return HandleTimerMsg( hWnd, message, wParam, lParam );
	case WM_CHAR:
		return HandleCharMsg( hWnd, message, wParam, lParam );
	case WM_KEYDOWN:
		return HandleKeyDownMsg( hWnd, message, wParam, lParam );
	case WM_KEYUP:
		return HandleKeyUpMsg( hWnd, message, wParam, lParam );
	case WM_LBUTTONDOWN:
		return HandleLButtonDownMsg( hWnd, message, wParam, lParam );
	case WM_LBUTTONUP:
		return HandleLButtonUpMsg( hWnd, message, wParam, lParam );
	case WM_MOUSEMOVE:
		return HandleMouseMoveMsg( hWnd, message, wParam, lParam );
	case WM_MOUSEWHEEL:
		return HandleMouseWheelMsg( hWnd, message, wParam, lParam );
	case WM_RBUTTONDOWN:
		return HandleRButtonDownMsg( hWnd, message, wParam, lParam );
	case GUIMSG_RADIOBUTTONCHOOSEN:
		return HandleRadioButtonSelectedMsg( hWnd, message, wParam, lParam );
	case GUIMSG_COMBOBOXROLLDOWN:
		return HandleComboBoxRollDown( hWnd, message, wParam, lParam );
	default:
		return false;
	}
	
	return false;
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// handles the GUIMSG_TIMER message
//
// returns true if the message was processed
//
//////////////////////////////////////////////////////////////////////////////////////////////
bool CGUIDialog::HandleTimerMsg( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
	APPRUNTIME			uiTime = pSharedDialogData.pEventTimer->GetTime();
	CGUIControlBase *	pObject;

	// tooltip is not showed, but mouse cursor moved to new object and its time to display the tooltip
	if ( !bToolTipShowed && bToolTipMouseMoved && ( uiLastMouseMoveTime + uiToolTipDelay <= uiTime ) )
	{
		pObject = GetControlFromPoint( iLastMouseX, iLastMouseY );
		if ( pObject )
		{
			iToolTipPosX = iLastMouseX + iToolTipRelX;
			iToolTipPosY = iLastMouseY + iToolTipRelY;
			uiToolTipStartTime = uiTime;
			bToolTipMouseMoved = FALSE;
			bToolTipShowed = TRUE;
			idToolTipObject = pObject->UID;
		}
	}
	
	// tooltip is already visible, but its time expired, it has to be vanished
	if ( bToolTipShowed && ( uiToolTipStartTime + uiToolTipDuration <= uiTime ) )
	{
		bToolTipShowed = FALSE;
		idToolTipObject = -1;
	}


	return true;
}

	
//////////////////////////////////////////////////////////////////////////////////////////////
//
// handles the WM_KEYDOWN message
//
// returns true if the message was processed
//
//////////////////////////////////////////////////////////////////////////////////////////////
bool CGUIDialog::HandleKeyDownMsg( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
	CGUIControlBase	*	pObject;
	CGUIControlBase *	pNewObject;
	CGUIControlBase *	pHelpObject;
	int					index, first;
	GUICONTROLID		newID = -1;

	
	// return if there is no controls in the dialog
	if ( !Controls.size() ) return true;

	
	pObject = GetControl( idFocusedObject );
	pNewObject = NULL;

	// change focused object when TAB key pressed
	if ( wParam == VK_TAB )
	{
		// get the one when to start looking for next focusable
		first = 0;
		if ( pObject ) // one control is already focused
		{
			for ( int i = 0; i < (int) Controls.size(); i++ ) // find the one already focused
				if ( pTabStopList[i].UID == idFocusedObject )
				{
					first = i + 1;
					break;
				}
		}

		// find first focusable control
		for ( index = first; index < (int) Controls.size(); index++ ) 
		{
			pHelpObject = pTabStopList[index].pControl;
			if ( pHelpObject->bStopable && pHelpObject->bEnabled ) break;
		}

		if ( index >= (int) Controls.size() ) // focused control was the last one; have to continue from the beginning
		{
			// find first focusable control
			for ( index = 0; index < (int) Controls.size(); index++ ) 
			{
				pHelpObject = pTabStopList[index].pControl;
				if ( pHelpObject->bStopable && pHelpObject->bEnabled ) break;
			}
		}
		
		// get the pointer to the newly focused object
		if ( index < (int) Controls.size() )
		{
			pNewObject = GetControl( pTabStopList[index].UID );
			newID = pNewObject->GetUID();
		}

		if ( pObject )
		{
			pObject->bHasFocus = false;
			if ( pObject->lpOnLostFocus ) pObject->lpOnLostFocus( (CGUIControlBase *) pObject, (LPARAM) newID );
		}
		if ( pNewObject )
		{
			pNewObject->bHasFocus = true;
			if ( pNewObject->lpOnGetFocus ) pNewObject->lpOnGetFocus( (CGUIControlBase *) pNewObject, (LPARAM) idFocusedObject );
		}
		idFocusedObject = newID;

		return true;
	}

	// let the message be processed by a control that has focus
	if ( pObject )
	{
		return pObject->WndProc( hWnd, message, wParam, lParam );
	}

	return false;
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// handles the WM_KEYUP message
//
// returns true if the message was processed
//
//////////////////////////////////////////////////////////////////////////////////////////////
bool CGUIDialog::HandleKeyUpMsg( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
	CGUIControlBase	*	pObject;

	
	pObject = GetControl( idFocusedObject );
	// let the message be processed by a control that has focus
	if ( pObject )
	{
		return pObject->WndProc( hWnd, message, wParam, lParam );
	}

	return false;
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// handles the WM_CHAR message
//
// returns true if the message was processed
//
//////////////////////////////////////////////////////////////////////////////////////////////
bool CGUIDialog::HandleCharMsg( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
	CGUIControlBase	*	pObject;

	
	pObject = GetControl( idFocusedObject );
	// let the message be processed by a control that has focus
	if ( pObject )
	{
		return pObject->WndProc( hWnd, message, wParam, lParam );
	}

	return false;
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// handles the WM_MOUSEWHEEL message
//
// returns true if the message was processed
//
//////////////////////////////////////////////////////////////////////////////////////////////
bool CGUIDialog::HandleMouseWheelMsg( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
	CGUIControlBase	*	pObject;

	
	pObject = GetControl( idFocusedObject );
	// let the message be processed by a control that has focus
	if ( pObject )
	{
		return pObject->WndProc( hWnd, message, wParam, lParam );
	}

	return false;
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// handles the WM_MOUSEMOVE message
//
// returns true if the message was processed
//
//////////////////////////////////////////////////////////////////////////////////////////////
bool CGUIDialog::HandleMouseMoveMsg( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
	CGUIControlBase *	pCapturingObject;
	CGUIControlBase *	pActiveObject;
	int					xPos, yPos;
	static APPRUNTIME	StartTime = pSharedDialogData.pEventTimer->GetTime();
	static CGUIControlBase *	pLastActiveObject = NULL;

	
	xPos = LOWORD( lParam ); 
	yPos = HIWORD( lParam );

	pActiveObject = GetControlFromPoint( xPos, yPos );
	pCapturingObject = GetControl( idMouseCapturingObject );

	// mouse cursor points to a new object -> tooltip should be displayed
	if ( pActiveObject != pLastActiveObject )
	{
		bToolTipMouseMoved = TRUE;
		bToolTipShowed = FALSE;
		idToolTipObject = -1;
	}
		
	pLastActiveObject = pActiveObject;

	uiLastMouseMoveTime = pSharedDialogData.pEventTimer->GetTime();
	iLastMouseX = xPos;
	iLastMouseY = yPos;


	//  send the message to the object that captured the mouse input
	if ( pCapturingObject )	return pCapturingObject->WndProc( hWnd, message, wParam, lParam );


	if ( !pActiveObject ) return false;

	return pActiveObject->WndProc( hWnd, message, wParam, lParam );
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// handles the WM_LBUTTONDOWN message
//
// returns true if the message was processed
//
//////////////////////////////////////////////////////////////////////////////////////////////
bool CGUIDialog::HandleLButtonDownMsg( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{	
	CGUIControlBase *	pObject;
	bool				res;


	// send the message to the object that captured the mouse input
	pObject = GetControl( idMouseCapturingObject );
	if ( pObject )
	{
		idMouseCapturingObject = pObject->UID;
		return pObject->WndProc( hWnd, message, wParam, lParam );
	}


	iLastMouseDownX = LOWORD( lParam ); 
	iLastMouseDownY = HIWORD( lParam );

	pObject = GetControlFromPoint( iLastMouseDownX, iLastMouseDownY );

	if ( !pObject ) return false;

	res = pObject->WndProc( hWnd, message, wParam, lParam );

	idMouseCapturingObject = pObject->UID;

	return res;
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// handles the WM_RBUTTONDOWN message
//
// always returns true because the message is always considered as processed
//
//////////////////////////////////////////////////////////////////////////////////////////////
bool CGUIDialog::HandleRButtonDownMsg( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
	CGUIControlBase *	pObject;


	pObject = GetControlFromPoint( LOWORD( lParam ), HIWORD( lParam ) );

	if ( lpOnRButtonDown ) lpOnRButtonDown( pObject, lParam );
    
	return true;
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// handles the WM_LBUTTONUP message
// this checks whether the last LBUTTONDOWN message was invoked on the same object and
// if it was, the lpOnMouseClick event handler is called on that object
//
// returns true if the message was processed
//
//////////////////////////////////////////////////////////////////////////////////////////////
bool CGUIDialog::HandleLButtonUpMsg( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
	CGUIControlBase *	pObject;
	CGUIControlBase *	pOldObject;
	int					xPos, yPos;
	bool				res;


	xPos = LOWORD( lParam ); 
	yPos = HIWORD( lParam );


	//  send the message to the object that captured the mouse input
	pObject = GetControl( idMouseCapturingObject );
	if ( pObject ) 
	{
		idMouseCapturingObject = -1;
		res = pObject->WndProc( hWnd, message, wParam, lParam );

		// invoke onMouseClick message if DOWN and UP messages happen on the same object
		if ( GetControlFromPoint( xPos, yPos ) == pObject )
		{
			// change focus if the new object is focusable and enabled
			if ( pObject->IsStopable() && pObject->IsEnabled() )
			{
				pOldObject = GetControl( idFocusedObject );
				if ( pOldObject )
				{
					pOldObject->bHasFocus = false;
					if ( pOldObject->lpOnLostFocus ) pOldObject->lpOnLostFocus( (CGUIControlBase *) pOldObject, (LPARAM) pObject->GetUID() );
				}
				pObject->bHasFocus = true;
				if ( pObject->lpOnGetFocus ) pObject->lpOnGetFocus( (CGUIControlBase *) pObject, (LPARAM) idFocusedObject );
				idFocusedObject = pObject->GetUID();
			}

			// mouse click
			pObject->WndProc( hWnd, GUIMSG_MOUSECLICK, wParam, lParam );
		}


		return res;
	}


	pObject = GetControlFromPoint( xPos, yPos );

	if ( !pObject ) return false;

	
	// proceses the LButtonDown message on target control
	res = pObject->WndProc( hWnd, message, wParam, lParam );

	// invoke onMouseClick message if DOWN and UP messages happen on the same object
	if ( GetControlFromPoint( xPos, yPos ) == pObject )
	{
		pObject->WndProc( hWnd, GUIMSG_MOUSECLICK, wParam, lParam );
		
		// change focus if the new object is focusable and enabled
		if ( pObject->IsStopable() && pObject->IsEnabled() )
		{
			pOldObject = GetControl( idFocusedObject );
			if ( pOldObject )
			{
				pOldObject->bHasFocus = false;
				if ( pOldObject->lpOnLostFocus ) pOldObject->lpOnLostFocus( (CGUIControlBase *) pOldObject, (LPARAM) pObject->GetUID() );
			}
			pObject->bHasFocus = true;
			if ( pObject->lpOnGetFocus ) pObject->lpOnGetFocus( (CGUIControlBase *) pObject, (LPARAM) idFocusedObject );
			idFocusedObject = pObject->GetUID();
		}
	}


	return res;
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// handles the GUIMSG_RADIOBUTTONCHOOSEN message
// this goes thru all controls and unselects all radiobuttons that has the same RadioGroup 
// index as the one that invoked this message (that also stays untouched)
//
// returns true if the message was processed
//
//////////////////////////////////////////////////////////////////////////////////////////////
bool CGUIDialog::HandleRadioButtonSelectedMsg( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
	CGUIControlRadioButton	*	pObject;
	CONTROLS_ITER				iterator;



	pObject = (CGUIControlRadioButton *) GetControl( (GUICONTROLID) lParam );
	if ( !pObject ) return false;


	for ( iterator = Controls.begin(); iterator != Controls.end(); iterator++ )
	{
		// the control has to be radio button other then the one that invoked this message
		if ( iterator->second->GetControlType() == GUICT_RADIOBUTTON && 
			 iterator->second->GetUID() != pObject->GetUID() && 
			 ( (CGUIControlRadioButton *) iterator->second )->iRadioGroup == pObject->iRadioGroup )
		{
			( (CGUIControlRadioButton * ) iterator->second )->Value.SetValue( "0" );
		}
	}

	return true;
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// handles the GUIMSG_COMBOBOXROLLDOWN message
// this changes focus from combobox to its associated popup menu
// returns true
//
//////////////////////////////////////////////////////////////////////////////////////////////
bool CGUIDialog::HandleComboBoxRollDown( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
	CGUIControlComboBox *	pCombo;
	CGUIControlPopupMenu *	pMenu;

	
	pCombo = (CGUIControlComboBox *) lParam;
	pMenu = (CGUIControlPopupMenu *) lParam;

	// roll down (menu is just beeing opened)
	if ( LOWORD(wParam) == 0 )
	{
		pCombo->bHasFocus = false;
		if ( pCombo->pPopupMenu )
		{
			pCombo->pPopupMenu->bHasFocus = true;
			pCombo->pPopupMenu->SetVisible();
			pCombo->pPopupMenu->Enable();
			pCombo->pPopupMenu->pInvokingObject = pCombo;
			idFocusedObject = pCombo->pPopupMenu->UID;
		}
	}
	// roll up (menu is beeing closed)
	else if ( LOWORD(wParam) == 1 )
	{
		pCombo->bHasFocus = true;
		if ( pCombo->pPopupMenu )
		{
			pCombo->pPopupMenu->bHasFocus = false;
			pCombo->pPopupMenu->Disable();
			pCombo->pPopupMenu->SetInvisible();
		}
		idFocusedObject = pCombo->UID;
	}
	else if ( LOWORD(wParam) == 2 )
	{
		pMenu->Disable();
		pMenu->SetInvisible();
		pMenu->pInvokingObject = NULL;
	}


	return true;
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// returns a pointer to a control that is the top most in specified coordinates
//
// returns a zero when there is no object is target location
//
//////////////////////////////////////////////////////////////////////////////////////////////
CGUIControlBase * CGUIDialog::GetControlFromPoint( int x, int y )
{
	CGUIControlBase *	pObject;
	RECT				rect;
	
	// go thru all objects sorted by ZOrder in descendant order
	for ( int index = (int) Controls.size() - 1; index >= 0; index-- )
	{
		pObject = pZOrderList[index].pControl;
		if ( !pObject->bEnabled ) continue;
		pObject->ComputeAbsoluteLocation( &rect );		
		if ( ( x >= rect.left ) && ( x <= rect.right ) && ( y >= rect.top ) && ( y <= rect.bottom ) )
			 return pObject;
	}

	return NULL;
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// returns a pointer to the selected radio button from specified radio group
// if more then one button is selected, the first one found is returned, that is the first one
// added to the dialog control list
//
//////////////////////////////////////////////////////////////////////////////////////////////
CGUIControlRadioButton * CGUIDialog::GetSelectedRadioButton( int radioGroupIndex )
{
	CGUIControlRadioButton	*	pObject;
	CONTROLS_ITER				iterator;

	
	for ( iterator = Controls.begin(); iterator != Controls.end(); iterator++ )
	{
		if ( iterator->second->GetControlType() == GUICT_RADIOBUTTON )
		{
			pObject = (CGUIControlRadioButton *) iterator->second;
			if ( pObject->iRadioGroup == radioGroupIndex && pObject->IsSelected() ) return pObject;
		}
	}

	return NULL;
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// sets the focus to specified control object; if the new focused object can't get focus
// (is not focusable or is disabled), the focus is lost and no object will have focus then
// also calls lpOnGetFocus and lpOnSetFocus to those involved objects
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CGUIDialog::SetFocusTo( GUICONTROLID UID )
{
	CGUIControlBase *	pObject;
	CGUIControlBase *	pOldObject;

	// stop possible capturing of the mouse
	idMouseCapturingObject = -1;

	pObject = GetControl( UID );
	pOldObject = GetControl( idFocusedObject );

	if ( pOldObject )
	{
		pOldObject->bHasFocus = false;
		if ( pOldObject->lpOnLostFocus ) pOldObject->lpOnLostFocus( pOldObject, (LPARAM) UID );
	}
	
	// set the new focused object 
	if ( pObject && pObject->IsStopable() && pObject->IsEnabled() )
	{
		pObject->bHasFocus = true;
		if ( pObject->lpOnGetFocus ) pObject->lpOnGetFocus( pObject, (LPARAM) idFocusedObject );
		idFocusedObject = pObject->GetUID();
	}
	else idFocusedObject = -1;
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// this goes thru all control objects and for each of them calls the CallBack function
// use pReserved pointer to pass any data to and from the called callback routine
// the dwOptions parameter when used specify the orderd in which the dialog controls are listed
//   possible values are combinations of flags:
//			SO_Unsorted		= 0x0000, // unsorted
//			SO_Sorted		= 0x0001, // sorted in any way
//			SO_ZSort		= 0x0002, // ZSorted - if not set, then TabStop sorting used
//			SO_Ascending	= 0x0004  // if set, ascending ordering used, elsewhere descending ordering used
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CGUIDialog::EnumerateControls(	void (CALLBACK *CallBack)(CGUIControlBase * pControl, LPVOID pReserved),
									DWORD dwOptions, LPVOID pReserved )
{
	CONTROLS_ITER	iter;
	
	if ( !dwOptions ) // unsorted
	{
		for ( iter = Controls.begin(); iter != Controls.end(); iter++ )	CallBack( iter->second, pReserved );
	}
	else if ( dwOptions & SO_ZSort ) // sorted by "Z coordinate"
	{
		if ( dwOptions & SO_Ascending )
			for ( int i = 0; i < (int) Controls.size(); i++ )	CallBack( pZOrderList[i].pControl, pReserved );
		else for ( int i = (int) Controls.size(); i > 0; i-- ) CallBack( pZOrderList[i-1].pControl, pReserved );
	}
	else 
	{
		if ( dwOptions & SO_Ascending )
			for ( int i = 0; i < (int) Controls.size(); i++ )	CallBack( pTabStopList[i].pControl, pReserved );
		else for ( int i = (int) Controls.size(); i > 0; i-- ) CallBack( pTabStopList[i-1].pControl, pReserved );

	}
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// Must be called when D3D device is lost
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CGUIDialog::OnLostDevice()
{
	CONTROLS_ITER It;

	for (It = Controls.begin(); It != Controls.end(); It++)
	{
		(*It).second->OnLostDevice();
	}

	SpriteBackground->OnLostDevice();
	pBasicFont->OnLostDevice();
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// Must be called after D3D device has been reset
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CGUIDialog::OnResetDevice()
{
	CONTROLS_ITER It;

	for (It = Controls.begin(); It != Controls.end(); It++)
	{
		(*It).second->OnResetDevice();
	}

	SpriteBackground->OnResetDevice();
	pBasicFont->OnResetDevice();
};


//////////////////////////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////////////////////////
