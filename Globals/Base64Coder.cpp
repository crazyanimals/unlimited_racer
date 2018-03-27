#include "stdafx.h"
#include "Base64Coder.h"



const char		CBase64Coder::EncodeTable[] = {	'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 
												'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 
												'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 
												'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f', 
												'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 
												'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 
												'w', 'x', 'y', 'z', '0', '1', '2', '3', 
												'4', '5', '6', '7', '8', '9', '+', '/' };

const char		CBase64Coder::DecodeTable[] = {	np,np,np,np,np,np,np,np,np,np,  // 0 - 9
												np,np,np,np,np,np,np,np,np,np,  //10 -19
												np,np,np,np,np,np,np,np,np,np,  //20 -29
												np,np,np,np,np,np,np,np,np,np,  //30 -39
												np,np,np,62,np,np,np,63,52,53,  //40 -49
												54,55,56,57,58,59,60,61,np,np,  //50 -59
												np,np,np,np,np, 0, 1, 2, 3, 4,  //60 -69
												 5, 6, 7, 8, 9,10,11,12,13,14,  //70 -79
												15,16,17,18,19,20,21,22,23,24,  //80 -89
												25,np,np,np,np,np,np,26,27,28,  //90 -99
												29,30,31,32,33,34,35,36,37,38,  //100 -109
												39,40,41,42,43,44,45,46,47,48,  //110 -119
												49,50,51,np,np,np,np,np,np,np,  //120 -129
												np,np,np,np,np,np,np,np,np,np,  //130 -139
												np,np,np,np,np,np,np,np,np,np,  //140 -149
												np,np,np,np,np,np,np,np,np,np,  //150 -159
												np,np,np,np,np,np,np,np,np,np,  //160 -169
												np,np,np,np,np,np,np,np,np,np,  //170 -179
												np,np,np,np,np,np,np,np,np,np,  //180 -189
												np,np,np,np,np,np,np,np,np,np,  //190 -199
												np,np,np,np,np,np,np,np,np,np,  //200 -209
												np,np,np,np,np,np,np,np,np,np,  //210 -219
												np,np,np,np,np,np,np,np,np,np,  //220 -229
												np,np,np,np,np,np,np,np,np,np,  //230 -239
												np,np,np,np,np,np,np,np,np,np,  //240 -249
												np,np,np,np,np,np            }; //250 -256*/


//////////////////////////////////////////////////////////////////////////////////////////////
//
// constructor & destructor
//
//////////////////////////////////////////////////////////////////////////////////////////////
CBase64Coder::CBase64Coder()
{
	uiSize = uiEncodedSize = uiDecodedSize = 0;
	pBuffer = pEncoded = pDecoded = NULL;
};

CBase64Coder::~CBase64Coder()
{
	Release();
};



//////////////////////////////////////////////////////////////////////////////////////////////
//
// creates a new buffer of a specified size and fills it with data pointed to by 'buffer' 
//  parameter; this automatically releases all previously associated buffers
//
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT CBase64Coder::InitBuffer( const BYTE * buffer, const UINT size )
{
	if ( !buffer || !size ) ERRORMSG( ERRINVALIDPARAMETER, "CBase64Coder::InitBuffer()", "" );

	Release();

	pBuffer = new BYTE[size];

	if ( !pBuffer ) ERRORMSG( ERROUTOFMEMORY, "CBase64Coder::InitBuffer()", "" );

	uiSize = size;

	memcpy( pBuffer, buffer, size );

	return ERRNOERROR;
};



//////////////////////////////////////////////////////////////////////////////////////////////
//
// creates a new buffer of a specified size; this automatically releases all previously 
//  associated buffers
//
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT CBase64Coder::InitBuffer( const UINT size )
{
	if ( !size ) ERRORMSG( ERRINVALIDPARAMETER, "CBase64Coder::InitBuffer()", "" );

	Release();

	pBuffer = new BYTE[size];

	if ( !pBuffer ) ERRORMSG( ERROUTOFMEMORY, "CBase64Coder::InitBuffer()", "" );

	uiSize = size;

	return ERRNOERROR;
};
	


//////////////////////////////////////////////////////////////////////////////////////////////
//
// encodes the stored initial buffer to base64; new memory is allocated for this buffer and
//  the old data are being kept so that they can be used later again
// if the encoded buffer already exists (previous unreleased conversion), it's released first
//
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT CBase64Coder::Encode()
{
	int		len, count;


	if ( !pBuffer ) ERRORMSG( ERRINVALIDDATA, "CBase64Coder::Encode()", "Buffer not initialized." );
	if ( !uiSize ) ERRORMSG( ERRINVALIDDATA, "CBase64Coder::Encode()", "Zero-sized buffer not allowed." );

	SAFE_DELETE_ARRAY( pEncoded );
	uiEncodedSize = 0;
	

	// count the size of output bufferand create it
	count = (int) (uiSize + 2) / 3;
	len = count * 4; // each started triplet will be converted into quad
	pEncoded = new BYTE[len];

	if ( !pEncoded ) ERRORMSG( ERROUTOFMEMORY, "CBase64Coder::Encode()", "" );

	uiEncodedSize = (UINT) len;

	
	// process each input character triplet by converting it to a 6bit quad
	for ( int i = 0; i < ((int)uiSize / 3); i++ )
	{
		pEncoded[i*4 + 0] = EncodeTable[   pBuffer[i*3  ] >> 2   ];
		pEncoded[i*4 + 1] = EncodeTable[ ((pBuffer[i*3  ] & 0x03) << 4) | ((pBuffer[i*3+1] & 0xF0) >> 4) ];
		pEncoded[i*4 + 2] = EncodeTable[ ((pBuffer[i*3+1] & 0x0F) << 2) | ((pBuffer[i*3+2] & 0xC0) >> 6) ];
		pEncoded[i*4 + 3] = EncodeTable[   pBuffer[i*3+2] & 0x3F ];
	}


	// if there are some characters left, we have to process them in a special way
	switch ( uiSize % 3 )
	{
	case 2:
		pEncoded[(count-1)*4 + 0] = EncodeTable[   pBuffer[(count-1)*3  ]         >> 2    ];
		pEncoded[(count-1)*4 + 1] = EncodeTable[ ((pBuffer[(count-1)*3  ] & 0x03) << 4) | 
												 ((pBuffer[(count-1)*3+1] & 0xF0) >> 4)   ];
		pEncoded[(count-1)*4 + 2] = EncodeTable[  (pBuffer[(count-1)*3+1] & 0x0F) << 2    ];
		pEncoded[(count-1)*4 + 3] = '='; // add one = character at the end
		break;
	case 1:
		pEncoded[(count-1)*4 + 0] = EncodeTable[  pBuffer[(count-1)*3  ]         >> 2 ];
		pEncoded[(count-1)*4 + 1] = EncodeTable[ (pBuffer[(count-1)*3  ] & 0x03) << 4 ];
		pEncoded[(count-1)*4 + 2] = '='; // add two = characters at the end
		pEncoded[(count-1)*4 + 3] = '=';
		break;
	}
	
	
	return ERRNOERROR;
};



//////////////////////////////////////////////////////////////////////////////////////////////
//
// decodes the stored initial buffer from base64; new memory is allocated for this buffer and
//  the old data are being kept so that they can be used later again
// if the decoded buffer already exists (previous unreleased conversion), it's released first
//
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT CBase64Coder::Decode()
{
	int		len, count, residue;


	if ( !pBuffer ) ERRORMSG( ERRINVALIDDATA, "CBase64Coder::Decode()", "Buffer not initialized." );
	if ( !uiSize ) ERRORMSG( ERRINVALIDDATA, "CBase64Coder::Encode()", "Zero-sized buffer not allowed." );
	if ( uiSize % 4 ) ERRORMSG( ERRINVALIDDATA, "CBase64Coder::Encode()", "Buffer must be alligned to 4 bytes." );

	SAFE_DELETE_ARRAY( pDecoded );
	uiDecodedSize = 0;
	

	// compute the output buffer size
	count = (int) uiSize / 4; // count of input quads -> output triplets
	residue = 0; // the residue says how many bytes have been added for alignment
	if ( pBuffer[uiSize-1] == '=' ) residue++;
	if ( pBuffer[uiSize-2] == '=' ) residue++;
	len = count * 3 - residue; // size of output buffer - count of triplets minus the alignment bytes
	
	// create the output buffer
	pDecoded = new BYTE[len];
	if ( !pDecoded ) ERRORMSG( ERROUTOFMEMORY, "CBase64Coder::Decode()", "" );

	uiDecodedSize = len;

	
	// decode each quad except the last one
	for ( int i = 0; i < count - 1; i++ )
	{
		pDecoded[i*3 + 0] = ((DecodeTable[pBuffer[i*4+0]] & 0x3F) << 2) | ((DecodeTable[pBuffer[i*4+1]] & 0x30) >> 4);
		pDecoded[i*3 + 1] = ((DecodeTable[pBuffer[i*4+1]] & 0x0F) << 4) | ((DecodeTable[pBuffer[i*4+2]] & 0x3C) >> 2);
		pDecoded[i*3 + 2] = ((DecodeTable[pBuffer[i*4+2]] & 0x03) << 6) | ((DecodeTable[pBuffer[i*4+3]] & 0x3F)     );
	}


	// process the last quad where some bytes can only be an alignment
	switch ( residue )
	{
	case 0: // no align bytes, whole quad used
		pDecoded[len - 3] = ((DecodeTable[pBuffer[(count-1)*4+0]] & 0x3F) << 2) | 
							((DecodeTable[pBuffer[(count-1)*4+1]] & 0x30) >> 4);
		pDecoded[len - 2] = ((DecodeTable[pBuffer[(count-1)*4+1]] & 0x0F) << 4) | 
							((DecodeTable[pBuffer[(count-1)*4+2]] & 0x3C) >> 2);
		pDecoded[len - 1] = ((DecodeTable[pBuffer[(count-1)*4+2]] & 0x03) << 6) | 
							((DecodeTable[pBuffer[(count-1)*4+3]] & 0x3F)     );
		break;
	case 1: // one align byte, three bytes carries the encoded data
		pDecoded[len - 2] = ((DecodeTable[pBuffer[(count-1)*4+0]] & 0x3F) << 2) | 
							((DecodeTable[pBuffer[(count-1)*4+1]] & 0x30) >> 4);
		pDecoded[len - 1] = ((DecodeTable[pBuffer[(count-1)*4+1]] & 0x0F) << 4) | 
							((DecodeTable[pBuffer[(count-1)*4+2]] & 0x3C) >> 2);
		break;
	case 2: // two align bytes, two bytes carries the encoded data
		pDecoded[len - 1] = ((DecodeTable[pBuffer[(count-1)*4+0]] & 0x3F) << 2) | 
							((DecodeTable[pBuffer[(count-1)*4+1]] & 0x30) >> 4);
		break;
	}


	return ERRNOERROR;
};


	
//////////////////////////////////////////////////////////////////////////////////////////////
//
// Saves the chosen buffer into a binary file. If the buffer points to encoded data,
//  the file content will be "readable" in text mode.
// Use 'buffer' parameter to choose which buffer to save. 
//   0 = original, 1 = encoded, 2 = decoded; or you can use the B64_*_BUFFER macros
//
//////////////////////////////////////////////////////////////////////////////////////////////
HRESULT CBase64Coder::SaveToFile( LPCTSTR fileName, const int buffer ) const
{
	FILE	*	f;
	UINT		size;
	BYTE	*	pBuf;

	// select appropriate buffer and its size
	switch ( buffer )
	{
	case B64_ENCODED_BUFFER: 
		size = uiEncodedSize;
		pBuf = pEncoded;
		break;
	case B64_DECODED_BUFFER:
		size = uiDecodedSize;
		pBuf = pDecoded;
		break;
	case B64_ORIGINAL_BUFFER:
		size = uiSize;
		pBuf = pBuffer;
		break;
	default:
		ERRORMSG( ERRINVALIDPARAMETER, "CBase64Coder::SaveToFile()", "Invalid buffer index chosen." );
	}

	// check for the buffer validity
	if ( !pBuf ) ERRORMSG( ERRINVALIDPARAMETER, "CBase64Coder::SaveToFile()", "Wrong buffer chosen." );

	// open the file and save the buffer 
	fopen_s( &f, fileName, "wb" );

	if ( !f ) ERRORMSG( ERRCANNOTOPENFORWRITE, "CBase64Coder::SaveToFile()", fileName );

	fwrite( pBuf, 1, size, f ); 

	fclose( f );

	return ERRNOERROR;
};



//////////////////////////////////////////////////////////////////////////////////////////////
//
// Releases all associated buffers
//
//////////////////////////////////////////////////////////////////////////////////////////////
void CBase64Coder::Release()
{
	SAFE_DELETE_ARRAY( pBuffer );
	SAFE_DELETE_ARRAY( pEncoded );
	SAFE_DELETE_ARRAY( pDecoded );
	uiSize = uiEncodedSize = uiDecodedSize = 0;
};



//////////////////////////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////////////////////////
