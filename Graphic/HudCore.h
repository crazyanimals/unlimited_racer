#pragma once
/*
HudCore.h
Provides framework for rendering HUD (Head up display) - speedometer, ...
by Pavel Celba
created: 3. 12. 2005
*/

#include "..\globals\Globals.h"
#include "..\globals\ErrorHandler.h"
#include <vector>


/*
Class CHUDCore
	provides framework for rendering HUD (Head up display)

Following general rules apply for using this class:
- All HUD Elements added must be allocated using new operator
- All HUD Elements added are deallocated with destruction of CHUDCore class, user can call DeleteElements in order to deallocate
- OnScreenResolutionChange must be called after resolution is changed
- OnResetDevice must be called after device is reset
- OnLostDevice must be called after device is lost and before OnResetDevice is called
- Anchoring order must maintain tree structure, so that you can anchor only to elements with lower ElementID
- ID3DXFont in HUDTextElement are not released in any case
*/
class CHUDCore
{
public:

	enum HUDElementType
	{
		HUDELEMENT_TEXTURE,
		HUDELEMENT_TEXT
	};

	enum HUDAlign
	{
		HUDALIGN_INNERLEFT = 1,
		HUDALIGN_INNERRIGHT = 2,
		HUDALIGN_INNERTOP = 4,
		HUDALIGN_INNERBOTTOM = 8,
		HUDALIGN_OUTERLEFT = 16,
		HUDALIGN_OUTERRIGHT = 32,
		HUDALIGN_OUTERTOP = 64,
		HUDALIGN_OUTERBOTTOM = 128,
		HUDALIGN_HORIZONTALCENTER = 256,
		HUDALIGN_VERTICALCENTER = 512
	};

	class HUDElement
	{
	public:
		// Returns type of HUD element
		inline HUDElementType GetType()
		{
			return Type;
		}

		// Align - in relation to whole screen 
		// Allowed are flags from HUDAlign enum
		// Note: PositionX or/and PositionY member can be ignored,
		// which is determined by flag(s) specified
		int Align;

		// Visible
		BOOL Visible;

		// Destination rectangle to format text
		// Position depends on achoring
		RECT DestRect;
		
	protected:
		HUDElement(HUDElementType _Type);
		
		// Element type
		HUDElementType Type;
		
		// anchoring
		UINT AnchorsTo;

		// Original destination rect - is lost when prepare for rendering is called
		// and so it must be remembered
		RECT OriginalDestRect;

		friend class CHUDCore;
	};

	// Texture element of HUD
	class HUDTextureElement: public HUDElement
	{
	public:
		// constructor
		HUDTextureElement();

		// copy constructor from other HUDTexture element in memory
		HUDTextureElement(HUDTextureElement * Elem);

		// Texture
		LPDIRECT3DTEXTURE9 Texture;
		// Amount of width to take
		// Value between 0 and 1
		float AmountWidth;
		// Amount of height to take
		// Value between 0 and 1
		float AmountHeight;

		// Position
		// Starting position on texture
		UINT PositionX, PositionY;
	};

	// Text element of HUD 
	class HUDTextElement: public HUDElement
	{
	public:
		// constructor
		HUDTextElement();

		// copy constructor from other HUDTexture element in memory
		HUDTextElement(HUDTextElement * Elem);

		// Font to render - must be set from outside
		ID3DXFont * Font;
		// text
		CString Text;
		
		// Text format - same as in D3DXFont.Draw() function
		DWORD Format;

		// Text color
		D3DCOLOR Color;
	};

	// Constructor
	CHUDCore();

	// Destructor
	~CHUDCore();

	// Init
	HRESULT Init(IDirect3DDevice9 * Device, int ScreenWidth, int ScreenHeight);

	// Adds element
	// A copy of element is made when adding
	// Rendering order is determined by order of insertion
	// Added elements are deleted automatically, when CHUDCore is destroyed
	// Or delete elements can be called
	// Returns element ID
	HRESULT AddElement(HUDTextureElement * Element, UINT & ElementID);
	HRESULT AddElement(HUDTextElement * Element, UINT & ElementID);

	// Deletes all elements
	void DeleteElements();

	// Element setting
	// Sets text to text element
	HRESULT SetText(UINT ElementID, CString & Text);

	// Sets font
	HRESULT SetFont(UINT ElementID, ID3DXFont * Font);

	// Sets texture to texture element
	HRESULT SetTexture(UINT ElementID, LPDIRECT3DTEXTURE9 Texture);

	// Sets texture position to texture element
	HRESULT SetTexturePosition(UINT ElementID, UINT PositionX, UINT PositionY);

	// Sets width amount to texture element
	HRESULT SetWidthAmount(UINT ElementID, float Amount);

	// Sets height amount to texture element
	HRESULT SetHeightAmount(UINT ElementID, float Amount);

	// Sets text color to text element
	HRESULT SetColor(UINT ElementID, D3DCOLOR Color);

	// Sets text format to text element
	HRESULT SetFormat(UINT ElementID, DWORD Format);

	// Set visibility to element
	HRESULT SetVisible(UINT ElementID, BOOL NewVisibility);

	// Set anchor
	// Note: Newly Set anchors will be active only after calling PrepareForRendering() method
	HRESULT SetAnchor(UINT ElementID, UINT AnchorToElementID);

	// Prepares elements for rendering
	// Must be called before rendering is called
	HRESULT PrepareForRendering();

	// Is called, when screen resolution changes
	HRESULT OnScreenResolutionChange(int NewWidth, int NewHeight);

	// Is called after device reset
	void OnResetDevice();

	// Is called when device is lost
	void OnLostDevice();

	// Renders HUD
	HRESULT Render();
private:
	// Screen width and height
	int ScreenWidth, ScreenHeight;

	// Sprite for rendering sprites
	ID3DXSprite * Sprite;

	// Stores all elements
	std::vector<HUDElement *> Elements;

	// determines preparation to render
	BOOL PreparedToRender;
};