/*
HudCore.cpp
Implementation of HudCore class
by Pavel Celba
created: 3. 12. 2005
*/

#include "HudCore.h"

using namespace std;

// HUDElement constructor
CHUDCore::HUDElement::HUDElement(HUDElementType _Type)
{
	Type = _Type;
	Align = 0;
	AnchorsTo = 0;
	Visible = TRUE;

	ZeroMemory(&DestRect, sizeof(RECT) );
}

// HUDTextureElement constructor
CHUDCore::HUDTextureElement::HUDTextureElement()
: CHUDCore::HUDElement(HUDELEMENT_TEXTURE)
{
	AmountHeight = 1.0f;
	AmountWidth = 1.0f;
	PositionX = 0;
	PositionY = 0;
	Texture = NULL;
}

// copy constructor from other HUDTexture element in memory
CHUDCore::HUDTextureElement::HUDTextureElement(HUDTextureElement * Elem)
: CHUDCore::HUDElement(HUDELEMENT_TEXTURE)
{
	if (this != Elem)
	{
		this->Align = Elem->Align;
		this->AmountHeight = Elem->AmountHeight;
		this->AmountWidth = Elem->AmountWidth;
		this->AnchorsTo = Elem->AnchorsTo;
		this->DestRect = Elem->DestRect;
		this->PositionX = Elem->PositionX;
		this->PositionY = Elem->PositionY;
		this->Texture = Elem->Texture;
		this->Type = Elem->Type;
		this->Visible = Elem->Visible;
	}
}

// HUDTextElement constructor
CHUDCore::HUDTextElement::HUDTextElement()
: CHUDCore::HUDElement(HUDELEMENT_TEXT)
{
	Font = NULL;
	// Defaulting to left
	Format = 0;
	Text = "";
	Color = 0xFFFFFFFF;
}

// copy constructor from other HUDTexture element in memory
CHUDCore::HUDTextElement::HUDTextElement(HUDTextElement * Elem)
: CHUDCore::HUDElement(HUDELEMENT_TEXT)
{
	if (this != Elem)
	{
		this->Align = Elem->Align;
		this->AnchorsTo = Elem->AnchorsTo;
		this->Color = Elem->Color;
		this->DestRect = Elem->DestRect;
		this->Font = Elem->Font;
		this->Format = Elem->Format;
		this->Text = Elem->Text;
		this->Type = Elem->Type;
		this->Visible = Elem->Visible;
	}
}

// Constructor
CHUDCore::CHUDCore()
{
	Sprite = 0;
	PreparedToRender = FALSE;
}



// Destructor
CHUDCore::~CHUDCore()
{
	// Delete all elements;
	vector<HUDElement *>::iterator It;
	for ( It = Elements.begin(); It != Elements.end(); It++ )
		SAFE_DELETE(*It);
	Elements.clear();

	SAFE_RELEASE(Sprite);
}

// Deletes all elements, but the first
void CHUDCore::DeleteElements()
{
	vector<HUDElement *>::iterator It;
	if ( Elements.size() >= 1 )	{
		for ( It = Elements.begin() + 1; It != Elements.end(); It++ )
			SAFE_DELETE(*It);
		Elements.resize( 1 );
	}
}

// Adds element
// A copy of element is made when adding
// Rendering order is determined by order of insertion
// Added elements are deleted automatically, when CHUDCore is destroyed
// Or delete elements can be called
// Returns element ID
HRESULT CHUDCore::AddElement(HUDTextureElement * Element, UINT & ElementID)
{
	HUDTextureElement * Elem;
	Elem = new HUDTextureElement(Element);
	if (!Elem)
		ERRORMSG(ERROUTOFMEMORY, "CHUDCore::AddElement()", "Unable to allocate place for HUD texture element.");

	// Remember original destination rect
	Elem->OriginalDestRect = Elem->DestRect;

	Elements.push_back(Elem);

	ElementID = (UINT) Elements.size() - 1;
	return ERRNOERROR;
}

HRESULT CHUDCore::AddElement(HUDTextElement * Element, UINT & ElementID)
{
	HUDTextElement * Elem;
	Elem = new HUDTextElement(Element);
	if (!Elem)
		ERRORMSG(ERROUTOFMEMORY, "CHUDCore::AddElement()", "Unable to allocate place for HUD text element.");

	// Remember original destination rect
	Elem->OriginalDestRect = Elem->DestRect;

	Elements.push_back(Elem);

	ElementID = (UINT) Elements.size() - 1;
	return ERRNOERROR;
}

// Element setting
// Sets text to text element
HRESULT CHUDCore::SetText(UINT ElementID, CString & Text)
{
	HUDTextElement * Elem;

	if (ElementID >= Elements.size() ) 
		ERRORMSG(-1, "CHUDCore::SetText", "Bad element ID.");

	if (Elements[ElementID]->GetType() != HUDELEMENT_TEXT)
		ERRORMSG(-1, "CHUDCore::SetText", "Bad element type, cannot set text");

	Elem = (HUDTextElement *) Elements[ElementID];

	Elem->Text = Text;

	return 0;
}

// Sets width amount to texture element
HRESULT CHUDCore::SetWidthAmount(UINT ElementID, float Amount)
{
	HUDTextureElement * Elem;
	
	if (ElementID >= Elements.size() ) 
		ERRORMSG(-1, "CHUDCore::SetWidthAmount", "Bad element ID.");

	if (Elements[ElementID]->GetType() != HUDELEMENT_TEXTURE)
		ERRORMSG(-1, "CHUDCore::SetWidthAmount", "Bad element type, cannot set width amount");

	Elem = (HUDTextureElement *) Elements[ElementID];
	
	Amount = max(0.0f, min(1.0f, Amount) );
	Elem->AmountWidth = Amount;

	return 0;
}

// Sets height amount to texture element
HRESULT CHUDCore::SetHeightAmount(UINT ElementID, float Amount)
{
	HUDTextureElement * Elem;

	if (ElementID >= Elements.size() ) 
		ERRORMSG(-1, "CHUDCore::SetHeightAmount", "Bad element ID.");
	
	if (Elements[ElementID]->GetType() != HUDELEMENT_TEXTURE)
		ERRORMSG(-1, "CHUDCore::SetWidthAmount", "Bad element type, cannot set height amount");

	Elem = (HUDTextureElement *) Elements[ElementID];
	
	Amount = max(0.0f, min(1.0f, Amount) );
	Elem->AmountHeight = Amount;

	return 0;
}

// Set visibility to element
HRESULT CHUDCore::SetVisible(UINT ElementID, BOOL NewVisibility)
{
	if (ElementID >= Elements.size() ) 
		ERRORMSG(-1, "CHUDCore::SetVisible", "Bad element ID.");

	Elements[ElementID]->Visible = NewVisibility;

	return 0;
}

// Sets texture to texture element
HRESULT CHUDCore::SetTexture(UINT ElementID, LPDIRECT3DTEXTURE9 Texture)
{
	HUDTextureElement * Elem;
	
	if (ElementID >= Elements.size() ) 
		ERRORMSG(-1, "CHUDCore::SetTexture", "Bad element ID.");

	if (Elements[ElementID]->GetType() != HUDELEMENT_TEXTURE)
		ERRORMSG(-1, "CHUDCore::SetTexture()", "Bad element type, cannot set texture.");

	Elem = (HUDTextureElement *) Elements[ElementID];
	Elem->Texture = Texture;

	return 0;
}

// Sets font
HRESULT CHUDCore::SetFont(UINT ElementID, ID3DXFont * Font)
{
	HUDTextElement * Elem;

	if (ElementID >= Elements.size() ) 
		ERRORMSG(-1, "CHUDCore::SetFont", "Bad element ID.");

	if (Elements[ElementID]->GetType() != HUDELEMENT_TEXT)
		ERRORMSG(-1, "CHUDCore::SetFont()", "Bad element type, cannot set font.");

	Elem = (HUDTextElement *) Elements[ElementID];
	Elem->Font = Font;

	return 0;
}

// Sets text color to text element
HRESULT CHUDCore::SetColor(UINT ElementID, D3DCOLOR Color)
{
	HUDTextElement * Elem;
	
	if (ElementID >= Elements.size() ) 
		ERRORMSG(-1, "CHUDCore::SetColor", "Bad element ID.");

	if (Elements[ElementID]->GetType() != HUDELEMENT_TEXT)
		ERRORMSG(-1, "CHUDCore::SetColor()", "Bad element type, cannot set text color.");

	Elem = (HUDTextElement *) Elements[ElementID];
	Elem->Color = Color;

	return 0;
}

// Sets text format to text element
HRESULT CHUDCore::SetFormat(UINT ElementID, DWORD Format)
{
	HUDTextElement * Elem;

	if (ElementID >= Elements.size() ) 
		ERRORMSG(-1, "CHUDCore::SetFormat", "Bad element ID.");

	if (Elements[ElementID]->GetType() != HUDELEMENT_TEXT)
		ERRORMSG(-1, "CHUDCore::SetFormat()", "Bad element type, cannot set text format.");

	Elem = (HUDTextElement *) Elements[ElementID];
	Elem->Format = Format;

	return 0;
}

// Sets texture position to texture element
HRESULT CHUDCore::SetTexturePosition(UINT ElementID, UINT PositionX, UINT PositionY)
{
	HUDTextureElement * Elem;
	
	if (ElementID >= Elements.size() ) 
		ERRORMSG(-1, "CHUDCore::SetTexturePosition", "Bad element ID.");

	if (Elements[ElementID]->GetType() != HUDELEMENT_TEXTURE)
		ERRORMSG(-1, "CHUDCore::SetTexturePosition()", "Bad element type, cannot set texture position.");

	Elem = (HUDTextureElement *) Elements[ElementID];
	Elem->PositionX = PositionX;
	Elem->PositionY = PositionY;

	return 0;
}

// Set anchor
// Note: Newly Set anchors will be active only after calling PrepareForRendering() method
HRESULT CHUDCore::SetAnchor(UINT ElementID, UINT AnchorToElementID)
{
	if (ElementID >= Elements.size() ) 
		ERRORMSG(-1, "CHUDCore::SetAnchor", "Bad element ID.");
	if (AnchorToElementID >= Elements.size())
		ERRORMSG(-1, "CHUDCore::SetAnchor()", "Such element ID doesn't exists, cannot set anchor to non-existing element.");
	if (ElementID <= AnchorToElementID)
		ERRORMSG(-1, "CHUDCore::SetAnchor()", "Cannot set anchor to following or same element.");

	Elements[ElementID]->AnchorsTo = AnchorToElementID;

	return 0;
}

// Prepares elements for rendering
// Must be called before rendering is called
HRESULT CHUDCore::PrepareForRendering()
{
	UINT i;
	int Width, Height;
	HUDElement * Elem;
	HUDElement * AnchorElem;
	HUDTextureElement * TextureElem;

	// Process 
	for (i = 0; i < Elements.size(); i++)
	{
		Elem = Elements[i];
		AnchorElem = Elements[Elem->AnchorsTo];

		Width = Elem->DestRect.right - Elem->DestRect.left;
		Height = Elem->DestRect.bottom - Elem->DestRect.top;

		// Resolve anchoring
		Elem->DestRect.left = Elem->OriginalDestRect.left + AnchorElem->DestRect.left;
		Elem->DestRect.right = Elem->DestRect.left + Width;
		Elem->DestRect.top = Elem->OriginalDestRect.top + AnchorElem->DestRect.top;
		Elem->DestRect.bottom = Elem->DestRect.top + Height;
		
		// Resolve aligning
		if (Elem->Align & HUDALIGN_INNERLEFT)
		{
			Elem->DestRect.left = AnchorElem->DestRect.left;
			Elem->DestRect.right = Elem->DestRect.left + Width;
		}
		
		if (Elem->Align & HUDALIGN_INNERRIGHT)
		{
			Elem->DestRect.left = AnchorElem->DestRect.right - Width;
			Elem->DestRect.right = AnchorElem->DestRect.right;
		}

		if (Elem->Align & HUDALIGN_INNERTOP)
		{
			Elem->DestRect.top = AnchorElem->DestRect.top;
			Elem->DestRect.bottom = Elem->DestRect.top + Height;
		}

		if (Elem->Align & HUDALIGN_INNERBOTTOM)
		{
			Elem->DestRect.top = AnchorElem->DestRect.bottom - Height;
			Elem->DestRect.bottom = AnchorElem->DestRect.bottom;
		}

		if (Elem->Align & HUDALIGN_OUTERLEFT)
		{
			Elem->DestRect.left = AnchorElem->DestRect.left - Width;
			Elem->DestRect.right = AnchorElem->DestRect.left;
		}

		if (Elem->Align & HUDALIGN_OUTERRIGHT)
		{
			Elem->DestRect.left = AnchorElem->DestRect.right;
			Elem->DestRect.right = AnchorElem->DestRect.right + Width;
		}

		if (Elem->Align & HUDALIGN_OUTERTOP)
		{
			Elem->DestRect.top = AnchorElem->DestRect.top - Height;
			Elem->DestRect.bottom = AnchorElem->DestRect.top;
		}
		
		if (Elem->Align & HUDALIGN_OUTERBOTTOM)
		{
			Elem->DestRect.top = AnchorElem->DestRect.bottom;
			Elem->DestRect.bottom = AnchorElem->DestRect.bottom + Height;
		}

		if (Elem->Align & HUDALIGN_HORIZONTALCENTER)
		{
			Elem->DestRect.left = AnchorElem->DestRect.left + (AnchorElem->DestRect.right - AnchorElem->DestRect.left) / 2 - Width / 2;
			Elem->DestRect.right = Elem->DestRect.left + Width;
		}

		if (Elem->Align & HUDALIGN_VERTICALCENTER)
		{
			Elem->DestRect.top = AnchorElem->DestRect.top + (AnchorElem->DestRect.bottom - AnchorElem->DestRect.top) / 2 - Height / 2;
			Elem->DestRect.bottom = Elem->DestRect.top + Height;
		}

		// restrict variables
		if (Elem->GetType() == HUDELEMENT_TEXTURE)
		{
			TextureElem = (HUDTextureElement *) Elem;
			TextureElem->AmountWidth = max(0.0f, min(1.0f, TextureElem->AmountWidth) );
			TextureElem->AmountHeight = max(0.0f, min(1.0f, TextureElem->AmountHeight) );
		}
	}

	PreparedToRender = TRUE;

	return 0;
}

// Renders HUD
HRESULT CHUDCore::Render()
{
	HRESULT hr;
	HUDElement * Elem;
	HUDTextureElement * TexElem;
	HUDTextElement * TextElem;

	RECT SrcRect;
	D3DXVECTOR3 Position = D3DXVECTOR3(0.0f,0.0f,0.0f);

	UINT i;

	hr = Sprite->Begin(D3DXSPRITE_ALPHABLEND);
	if (FAILED(hr) ) ERRORMSG(hr, "CHUDCore::Render()", "Error while calling Begin() of D3DXSprite.");

	for (i = 0; i < Elements.size(); i++)
	{
		Elem = Elements[i];
		if (!Elem->Visible) continue;
		if (Elem->GetType() == HUDELEMENT_TEXTURE)
		{
			// Texture rendering preparation and rendering
			TexElem = (HUDTextureElement *) Elem;
			
			SrcRect.left = TexElem->PositionX;
			SrcRect.top = TexElem->PositionY;
			SrcRect.right = (LONG) (SrcRect.left + (float) (TexElem->DestRect.right - TexElem->DestRect.left) * TexElem->AmountWidth);
			SrcRect.bottom = (LONG) (SrcRect.top + (float) (TexElem->DestRect.bottom - TexElem->DestRect.top) * TexElem->AmountHeight);

			Position.x = (float) TexElem->DestRect.left;
			Position.y = (float) TexElem->DestRect.top;

			// silently step those elements without texture
			if (TexElem->Texture)
			{
				hr = Sprite->Draw(TexElem->Texture, &SrcRect, NULL, &Position, 0xFFFFFFFF);
				if (FAILED(hr) ) ERRORMSG(hr, "CHUDCore::Render()", "Unable to render HUD texture element.");
			}
		}
		else if (Elem->GetType() == HUDELEMENT_TEXT)
		{
			// Text rendering preparation and rendering
			TextElem = (HUDTextElement *) Elem;

			if (TextElem->Font)
			{
                hr = TextElem->Font->DrawTextA(Sprite, TextElem->Text, 
											   TextElem->Text.GetLength(), &(TextElem->DestRect), 
											   TextElem->Format, TextElem->Color);
				if (FAILED(hr) ) ERRORMSG(hr, "CHUDCore::Render()", "Unable to render HUD text element.");
			}

		}
	}

	Sprite->End();

	return 0;
}

// Init
HRESULT CHUDCore::Init(IDirect3DDevice9 * Device, int Width, int Height)
{
	HRESULT hr;
	HUDTextureElement * Elem;

	SAFE_RELEASE(Sprite);
	hr = D3DXCreateSprite(Device, &Sprite);
	if (FAILED(hr) ) ERRORMSG(hr, "CHUDCore::Init()", "Unable to create sprite");

	ScreenWidth = Width;
	ScreenHeight = Height;

	DeleteElements();

	Elements.clear();

	// Create screen size element of no texture in order to simplyfy all alignings and anchoring resolving
	Elem = new HUDTextureElement();
	if (!Elem)
		ERRORMSG(ERROUTOFMEMORY, "CHUDCore::Init()", "Unable to allocate place for screen size hud texture element.");
	Elem->DestRect.left = 0;
	Elem->DestRect.top = 0;
	Elem->DestRect.right = ScreenWidth;
	Elem->DestRect.bottom = ScreenHeight;
	Elem->OriginalDestRect = Elem->DestRect;
	
	Elements.push_back(Elem);

	return 0;
}

// Is called, when screen resolution changes
HRESULT CHUDCore::OnScreenResolutionChange(int NewWidth, int NewHeight)
{
	HRESULT hr;

	ScreenWidth = NewWidth;
	ScreenHeight = NewHeight;

	// Change size of screen element
	Elements[0]->DestRect.right = ScreenWidth;
	Elements[0]->DestRect.bottom = ScreenHeight;
	Elements[0]->OriginalDestRect = Elements[0]->DestRect;

	hr = PrepareForRendering();
	if (FAILED(hr) ) ERRORMSG(hr, "CHUDCore::OnScreenResolutionChange()", "Unable to prepare HUD for rendering.");

	return 0;
}

// Is called after device reset
void CHUDCore::OnResetDevice()
{
	Sprite->OnResetDevice();
}

// Is called when device is lost
void CHUDCore::OnLostDevice()
{
	Sprite->OnLostDevice();
}
