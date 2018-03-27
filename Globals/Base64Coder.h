/*
	Base64Coder:	Contains functions for simple encoding a decoding data to/from base64 format.
					Usage:
						CBase64Coder	B64Coder;
						B64Coder.InitBuffer( str.GetBuffer(), str.GetLength() );
						B64Coder.Encode();
						B64Coder.SaveToFile( "file.txt", B64_ENCODED_BUFFER );
						char a = B64Coder.GetEncodedBuffer()[3];
	Creation Date:	6.1.2007
	Last Update:	15.1.2007
	Author:			Roman Margold
*/	
	
#pragma once

#include "stdafx.h"
#include "Globals.h"
#include "ErrorHandler.h"
#include "DebugOutput.h"


#define B64_ORIGINAL_BUFFER		0
#define B64_ENCODED_BUFFER		1
#define B64_DECODED_BUFFER		2



class CBase64Coder
{
public:
					CBase64Coder();
					~CBase64Coder();

	HRESULT			InitBuffer( const BYTE * buffer, const UINT size );
	HRESULT			InitBuffer( const UINT size );
	
	BYTE		*	GetBuffer() const { return pBuffer; }; // the returned data can be freely manipulated
	BYTE		*	GetEncodedBuffer() const { return pEncoded; }; // the returned data can be freely manipulated
	BYTE		*	GetDecodedBuffer() const { return pDecoded; }; // the returned data can be freely manipulated
	UINT			GetBufferSize() const { return uiSize; };
	UINT			GetEncodedBufferSize() const { return uiEncodedSize; };
	UINT			GetDecodedBufferSize() const { return uiDecodedSize; };

	HRESULT			Encode();
	HRESULT			Decode();

	HRESULT			SaveToFile( LPCTSTR fileName, const int buffer ) const;

	void			Release();

private:
	BYTE		*	pBuffer;
	BYTE		*	pEncoded;
	BYTE		*	pDecoded;
	UINT			uiSize;
	UINT			uiEncodedSize;
	UINT			uiDecodedSize;

	static const char	EncodeTable[]; // see the definition in Base64Coder.cpp
	static const char	np = 0;
	static const char	DecodeTable[]; // see the definition in Base64Coder.cpp

};




