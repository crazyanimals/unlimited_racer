/*
Sky system shaders
idea from ShaderX2 - Advanced Sky Dome Rendering by Marco Spoerl and Kurt Pelzer
by Pavel Celba
2. 5. 2005
*/

// Matrixes
// World * View * Proj matrix
matrix AllMat;
// Sun projection matrix
matrix SunMat;
// Cloud projection matrix
matrix CloudMat;

// Sky system constants
// Sun normal used to compute molecule and aerosol scattering
float3 SunNormal;
// Sun color
float3 SunColor;
// Molecule color
float3 MoleculeColor;
// Aerosol color
float3 AerosolColor;

// samplers
sampler TexSamp0: register(s0);
sampler TexSamp1: register(s1);

// Fog properties
float3 FogColor;// = { 0.5, 0.5, 0.7 };
//float3	FogColor = { 0.7, 0.5, 0.5 };


//////////////////////////////////////////////////////////////////////////////////
// VERTEX SHADERS
//////////////////////////////////////////////////////////////////////////////////

// Input and output
struct MOUNTAIN_INPUT
{
	float4 Position: POSITION;
	float2 TexCoord: TEXCOORD0;
	float  Alpha: TEXCOORD1;
};

struct SKY_SYSTEM_INPUT_VS
{
	float4 Position: POSITION;
	float4 Normal: NORMAL;
};

struct MOUNTAIN_OUTPUT
{
	float4 Position: POSITION0;
	float2 TexCoord: TEXCOORD0;
	float  Alpha: TEXCOORD1; // alpha channel stored for per pixel fog computation
};

struct SKY_SYSTEM_OUTPUT_VS
{
	float4 Position: POSITION;
	float4 SkyColor: COLOR;
	float4 SunTexCoord: TEXCOORD;
};

struct SKY_SYSTEM_CLOUD_OUTPUT_VS
{
	float4 Position: POSITION;
	float4 SkyColor: COLOR;
	float4 SunTexCoord: TEXCOORD;
	float4 CloudTexCoord: TEXCOORD1;
};

struct CLOUD_OUTPUT_VS
{
	float4 Position: POSITION;
	float4 CloudTexCoord: TEXCOORD0;
};



// Sky system vertex shader
SKY_SYSTEM_OUTPUT_VS SkySystemVS(SKY_SYSTEM_INPUT_VS Input)
{
	SKY_SYSTEM_OUTPUT_VS Output = (SKY_SYSTEM_OUTPUT_VS) 0;
	float Angle;
	float MoleculeScatter;
	float AerosolScatter;
	float4 SunCoords;
	
	// Compute position
	Output.Position = mul(Input.Position, AllMat);
	// force z to be 1
	Output.Position.z = Output.Position.w;
	
	// calculate cos of angle between sky and sun normal
	Angle = dot(Input.Normal, SunNormal);
	
	// calculate molecule scattering
	MoleculeScatter = 0.75 * (1 + sign(Angle) * Angle * Angle);
	
	// calculate aerosol scattering
	//if (Angle > 0.75)
	//	AerosolScatter = 1.75;
	//else
		AerosolScatter = 0.75;
		
	//AerosolScatter = lerp(0.75, 1.35, max(0, min(1,((Angle - 0.65) * 5))) );
	
	// calculate final color
	Output.SkyColor.rgb = (MoleculeScatter * MoleculeColor.rgb + AerosolScatter * AerosolColor.rgb) * SunColor.rgb;
	Output.SkyColor.a = 1;
	
	// compute sun texture coordinates
	SunCoords = mul(Input.Position, SunMat);
	SunCoords.z = SunCoords.w;
	if (SunCoords.z > 0) // cancels backward projection
		Output.SunTexCoord = SunCoords;
		
	return Output;
}


// Sky system vertex shader
SKY_SYSTEM_CLOUD_OUTPUT_VS SkySystemCloudVS(SKY_SYSTEM_INPUT_VS Input)
{
	SKY_SYSTEM_CLOUD_OUTPUT_VS Output = (SKY_SYSTEM_CLOUD_OUTPUT_VS) 0;
	float Angle;
	float MoleculeScatter;
	float AerosolScatter;
	float4 SunCoords;
	
	// Compute position
	Output.Position = mul(Input.Position, AllMat);
	// force z to be 1
	Output.Position.z = Output.Position.w;
	
	
	// calculate cos of angle between sky and sun normal
	Angle = dot(Input.Normal, SunNormal);
	
	// calculate molecule scattering
	MoleculeScatter = 0.75 * (1 + sign(Angle) * Angle * Angle);
	
	// calculate aerosol scattering
	//if (Angle > 0.75)
	//	AerosolScatter = 1.75;
	//else
		AerosolScatter = 0.75;
	
	// calculate final color
	Output.SkyColor.rgb = (MoleculeScatter * MoleculeColor.rgb + AerosolScatter * AerosolColor.rgb) * SunColor.rgb;
	Output.SkyColor.a = 1;
	
	// compute sun texture coordinates
	SunCoords = mul(Input.Position, SunMat);
	SunCoords.z = SunCoords.w;
	if (SunCoords.z > 0) // cancels backward projection
		Output.SunTexCoord = SunCoords;
		
	// compute cloud texture coordinates
	Output.CloudTexCoord = mul(Input.Position, CloudMat);
	Output.CloudTexCoord.z = Output.CloudTexCoord.w;
	
	return Output;
}


// Sky system - only clouds vertex shader
CLOUD_OUTPUT_VS CloudVS(SKY_SYSTEM_INPUT_VS Input)
{
	CLOUD_OUTPUT_VS Output = (CLOUD_OUTPUT_VS) 0;
	
	// Compute position
	Output.Position = mul(Input.Position, AllMat);
	// force z to be 1
	Output.Position.z = Output.Position.w;
	
	
	// compute cloud texture coordinates
	Output.CloudTexCoord = mul(Input.Position, CloudMat);
	Output.CloudTexCoord.z = Output.CloudTexCoord.w;
	
	return Output;
}


// sky system - vertex shader for mountains - simple transformation and for intensity export
MOUNTAIN_OUTPUT MountainVS( MOUNTAIN_INPUT Input )
{
	MOUNTAIN_OUTPUT Output = (MOUNTAIN_OUTPUT) 0;

	Output.Position = mul( Input.Position, AllMat );
	Output.TexCoord = Input.TexCoord;
	
    // Fog opacity
    Output.Alpha = Input.Alpha;
    
	return Output;
};


// sky system - vertex shader for mountains - simple transformation
// doesn't export fog intensity
MOUNTAIN_OUTPUT MountainNightVS( MOUNTAIN_INPUT Input )
{
	MOUNTAIN_OUTPUT Output = (MOUNTAIN_OUTPUT) 0;

	Output.Position = mul( Input.Position, AllMat );
	Output.TexCoord = Input.TexCoord;
    
	return Output;
};






//////////////////////////////////////////////////////////////////////////////////
// PIXEL SHADERS
//////////////////////////////////////////////////////////////////////////////////


struct SKY_SYSTEM_INPUT_PS
{
	float4 SkyColor: COLOR;
	float4 SunTexCoord: TEXCOORD;
};

struct SKY_SYSTEM_CLOUD_INPUT_PS
{
	float4 SkyColor: COLOR;
	float4 SunTexCoord: TEXCOORD;
	float4 CloudTexCoord: TEXCOORD1;
};

struct MOUNTAIN_INPUT_PS
{
    float2 TexCoord: TEXCOORD0;
	float  Alpha: TEXCOORD1; // alpha value for per pixel fog 
};



// Sky system pixel shader
float4 SkySystemPS(SKY_SYSTEM_INPUT_PS Input): COLOR
{
	float4 SunTexColor;
	
	SunTexColor = tex2Dproj(TexSamp0, Input.SunTexCoord);
	
	return Input.SkyColor + ( (1 - Input.SkyColor) * SunTexColor);
	
};


float4 SkySystemCloudPS(SKY_SYSTEM_CLOUD_INPUT_PS Input): COLOR
{
	float4 SunTexColor;
	float4 CloudTexColor;
	
	SunTexColor = tex2Dproj(TexSamp0, Input.SunTexCoord);
	
	CloudTexColor = tex2Dproj(TexSamp1, Input.CloudTexCoord);
	
	return (Input.SkyColor + ( (1 - Input.SkyColor) * SunTexColor)) * (1 - CloudTexColor.a) + CloudTexColor * CloudTexColor.a;
	
};


// Only cloud PS
float4 CloudPS(float4 CloudTexCoord: TEXCOORD) : COLOR
{
	
	float4 CloudTexColor;
	
	CloudTexColor = tex2Dproj(TexSamp0, CloudTexCoord);
	
	return CloudTexColor;
};


// sky system - pixel shader for mountains - just a texture covered by fog
float4 MountainPS( MOUNTAIN_INPUT_PS Input ) : COLOR0
{
	float4 TexCol;
	
	TexCol = tex2D( TexSamp0, Input.TexCoord );
	
    // fog falloff application
 	TexCol.rgb = Input.Alpha * FogColor.rgb + ( 1 - Input.Alpha ) * TexCol.rgb;
    
	return TexCol;	
};

// sky system - pixel shader for mountains - just a texture
float4 MountainNightPS( MOUNTAIN_INPUT_PS Input ) : COLOR0
{
	float4 TexCol;
	
	TexCol = tex2D( TexSamp0, Input.TexCoord );
    
	return TexCol;	
};





//////////////////////////////////////////////////////////////////////////////////
// STARS SHADERS
//////////////////////////////////////////////////////////////////////////////////

struct STARS_INPUT_VS
{
	float4 Position: POSITION;
	float4 Color: COLOR0;
	float PointSize: PSIZE;
};

struct STARS_OUTPUT_VS
{
	float4 Position: POSITION;
	float4 Color: COLOR;
	float PointSize: PSIZE;
};



// Star field vertex shader
STARS_OUTPUT_VS StarsVS(STARS_INPUT_VS Input)
{
	STARS_OUTPUT_VS Output = (STARS_OUTPUT_VS) 0;
	
	// Compute position on the sky dome
	Output.Position = mul(Input.Position, AllMat);
	// Small value is substracted, because of star hiding and showing, when
	// rotating with camera
	Output.Position.z = Output.Position.w - 0.000001;
	
	// blend with SunColor, in order to make stars like appearing on the sky
	Output.Color.rgb = Input.Color.rgb * (1 - SunColor.rgb * 5);
	Output.Color.a = Input.Color.a;
	
	// Set point size in order to make stars bigger
	Output.PointSize = Input.PointSize;
	
	return Output;
}


// Star fiels pixel shader
float4 StarsPS(float4 Color: COLOR): COLOR
{	
	return Color;
}


