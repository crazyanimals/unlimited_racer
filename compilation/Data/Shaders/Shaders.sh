// Main program shaders - covers lighting, transformations, shadowing
// By Pavel Celba
// 26. 2. 2007

//////////////////////////////////////////////////////////////
// GLOBAL CONSTANTS
//////////////////////////////////////////////////////////////

// Matrixes
// World * View * Proj matrix
matrix AllMat;
// World * View matrix
matrix WorldViewMat;
// View matrix
matrix ViewMat;
// World matrix
matrix WorldMat;
// Shadow map matrix - from view space to texture space
matrix ShadowMat;
matrix ShadowMat2;
matrix ShadowMat3;
matrix ShadowMat4;
// Post projection matrix
matrix PostProjMat;


// Material power
float MaterialPower = 16.0f;

// Specular lighting on/off
bool Specular: register(b0) = false;

// Ambient color
float4 Ambient = {0.0, 0.0, 0.0, 1.0};

// for setting alpha member of color to 1 in lighting computation
float4 COLORBASE = {0.0, 0.0, 0.0, 1.0};

// single increasing unit for Shadow Volume rendering 
float4 SVIncrease = { 0.11, 0.11, 0.11, 0.11 };

// Stride for moving one pixel in a texture
float Stride = 0.00048828125;

// clamp vectors
// min x, min y, max x, max y
float4 ClampVector1 = {0.0, 0.0, 1.0, 1.0 };
float4 ClampVector2 = {0.0, 0.0, 1.0, 1.0 };
float4 ClampVector3 = {0.0, 0.0, 1.0, 1.0 };
float4 ClampVector4 = {0.0, 0.0, 1.0, 1.0 };

// normalization vectors
float2 NormalizationVector2 = {0.5, 0.5};
float3 NormalizationVector3 = {0.333, 0.333, 0.333};
float4 NormalizationVector4 = {0.25, 0.25, 0.25, 0.25};

// Light map color
float3 LightMapColor = {1.0, 1.0, 0.75};

// Fog properties
//   Restriction:  ZMax > ZMin > 0
float	FogZMin = 2000;
float	FogZMax = 20000;
float	MinFogIntensity = 0;
float	MaxFogIntensity = 0.9;
float3	FogColor;// = { 0.5, 0.5, 0.7 };
//float3	FogColor = { 0.7, 0.5, 0.5 };


// Grass properties
float	GrassTime;
float	GrassMaxDepth;
float3	GrassViewerPos; // camera position


// texture color modifier - matrix used for multiplication of RGB channels 
matrix	ColorModifier;


// Terrain normal mapping properties
float	MapWidth;
float	MapDepth;


// Material structure
struct MATERIAL
{
   float4 Diffuse;
   float4 Specular;
   float4 Ambient;
};

// material global
MATERIAL Material;

// Directional light structure
struct DIRECTIONAL_LIGHT
{
	float4 Ambient;
    float4 Diffuse;
    float4 Specular;
    float3 Direction;
};

struct SPOT_LIGHT
{
	float4 Ambient;
	float4 Diffuse;
	float4 Specular;
	float4 Position; // (x,y,z,1)
	float4 Direction; // (x,y,z,0)
	float4 Atten; //1, D, D^2,range;
	float4 Angles; //cos(theta/2), cos(phi/2), falloff, 0
};

SPOT_LIGHT SpotLight; // Spot light
SPOT_LIGHT SpotLights[9]; // Spot lights for night lighting

// Direction light global
DIRECTIONAL_LIGHT DirLight;

struct COLOR_PAIR
{
   float4 Diffuse;
   float4 Specular;     
};

//////////////////////////////////////////////////////////////
// TEXTURE SAMPLERS
/////////////////////////////////////////////////////////////
sampler TexSamp0: register(s0);
sampler TexSamp1: register(s1);
sampler TexSamp2: register(s2);
sampler TexSamp3: register(s3);
sampler TexSamp4: register(s4);
sampler TexSamp5: register(s5);
sampler TexSamp6: register(s6);
sampler TexSamp7: register(s7);



//////////////////////////////////////////////////////////////////////////////////////////
// STANDARD LIGHTING MODEL
///////////////////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
// Name: DoDirLight()
// Desc: Directional light computation
//-----------------------------------------------------------------------------
COLOR_PAIR DoDirLight(float3 Normal, float3 View, float3 Direction, DIRECTIONAL_LIGHT DirLight)
{
   COLOR_PAIR Out;
   Out.Specular =  COLORBASE;
   Out.Diffuse = COLORBASE;
   float NdotL = dot(Normal, Direction);
   if(NdotL > 0.f)
   {
      //compute diffuse color
      Out.Diffuse.xyz += NdotL * DirLight.Diffuse + DirLight.Ambient;
	/*
      //add specular component
      if(Specular)
      {
         float3 H = normalize(Direction + View);   //half vector
         Out.Specular.xyz += pow(max(0, dot(H, Normal)), MaterialPower) * DirLight.Specular;
      }
   */   
      
   }
   
   return Out;
}


//-----------------------------------------------------------------------------
// Name: DoDirLightWithSpecular()
// Desc: Directional light computation - both diffuse and specular illumination
//-----------------------------------------------------------------------------
COLOR_PAIR DoDirLightWithSpecular(float3 Normal, float3 NormalTrans, float3 View, float3 Direction, DIRECTIONAL_LIGHT DirLight)
{
   COLOR_PAIR Out;
   Out.Specular =  COLORBASE;
   Out.Diffuse = COLORBASE;
   float NdotL = dot( Normal, Direction );
   if ( NdotL > 0.f )
   {
      //compute diffuse color
      Out.Diffuse.xyz += NdotL * DirLight.Diffuse + DirLight.Ambient;

      //add specular component
      float3 H = normalize( Direction + View );   //half vector
      Out.Specular.xyz += pow( max( 0, dot( H, NormalTrans ) ), MaterialPower ) * DirLight.Specular;
   }
   
   return Out;
}


COLOR_PAIR DoSpotLight(float3 Normal, float3 View, float4 Position, SPOT_LIGHT SpotLight)
{
   float3 L;
   float AttenSpot = 1;
   COLOR_PAIR Out;
   Out.Specular = COLORBASE;
   Out.Diffuse = COLORBASE;
   L = normalize( mul((float3)SpotLight.Position - (float3)mul(Position, WorldMat), (float3x3)ViewMat) );
   
   float NDotL = dot(Normal, L);
   
  if (NDotL > 0.0f)
   {
      // compute basic diffuse color
      Out.Diffuse.xyz += NDotL * SpotLight.Diffuse + SpotLight.Ambient;
      
      
      // add basic specular component
      if (Specular)
      {
         float3 H = normalize(L + View); // half vector
         Out.Specular.xyz = pow(max(0, dot(H, Normal)), MaterialPower) * SpotLight.Specular;
      }
     
      float LD = length((float3) SpotLight.Position -(float3)mul(Position, WorldMat));
      if(LD > SpotLight.Atten.w) // SpotLight.Atten.w - Range
      {
         AttenSpot = 0.f;
      }
      else
      {
         AttenSpot *= 1.f/(SpotLight.Atten.x + SpotLight.Atten.y*LD + SpotLight.Atten.z*LD*LD);
      }
      
	  //spot cone computation
      float3 L2 = mul(-normalize((float3)SpotLight.Direction), (float3x3)ViewMat);
      float rho = dot(L, L2);
      AttenSpot *= pow(saturate((rho - SpotLight.Angles.y)/(SpotLight.Angles.x - SpotLight.Angles.y)), SpotLight.Angles.z);
		
	  
	  Out.Diffuse.xyz *= AttenSpot;
	  Out.Specular.xyz *= AttenSpot;
	 
   }
   
    return Out;
};

// Compute resulting color
float4 DoResultLight(COLOR_PAIR Color, MATERIAL Material, float4 Ambient)
{
    float4 Out;

  //  Out = Color.Diffuse * Material.Diffuse + Color.Specular * Material.Specular + Ambient * Material.Ambient;
    Out = Color.Diffuse * Material.Diffuse; // + Color.Specular * Material.Specular;
	Out = saturate(Out);
	
    return Out;
}




//////////////////////////////////////////////////////////////////////////////////
// FOG FUNCTIONS
//////////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
// Name: ComputeFogIntensity()
// Desc: Computes fog intensity for a vertex
//-----------------------------------------------------------------------------
float ComputeFogIntensity( float3 coord )
{
    float	FogFalloffZ;
    float	Intens;
    float	Dist;
    
	FogFalloffZ = 1 / ( FogZMax - FogZMin );

    Dist = sqrt( coord.x * coord.x + coord.z * coord.z );
    
	Intens = ( Dist - FogZMin ) * FogFalloffZ;
	
	return Intens;	
}


//-----------------------------------------------------------------------------
// Name: ApplyFogColor()
// Desc: Returns output color for pixel that is clouded by fog
//-----------------------------------------------------------------------------
float3 ApplyFogColor( float4 FogIntensity, float3 PixColor )
{
	float I;
	
	I = clamp( FogIntensity.w , MinFogIntensity, MaxFogIntensity );
	
	PixColor.rgb = I * FogColor.rgb + ( 1 - I ) * PixColor.rgb;
	

	return PixColor;
}


//-----------------------------------------------------------------------------
// Name: ApplyPixelFogColor()
// Desc: Returns output color for pixel that is clouded by fog
//-----------------------------------------------------------------------------
float3 ApplyPixelFogColor( float3 coord, float3 PixColor )
{
	float I;
	
	I = ComputeFogIntensity( coord );
	I = clamp( I, MinFogIntensity, MaxFogIntensity );

	PixColor.rgb = I * FogColor.rgb + ( 1 - I ) * PixColor.rgb;

	return PixColor;
}


//////////////////////////////////////////////////////////////////////////////////
// GENERAL TRANSFORMING SHADERS
//////////////////////////////////////////////////////////////////////////////////

// Inputs and outputs

struct SIMPLE_INPUT
{
	float4 Position: POSITION;
};

struct SIMPLE_OUTPUT
{
	float4 Position: POSITION;
};

struct SIMPLETEX_INPUT
{
	float4 Position: POSITION;
	float2 TexCoord: TEXCOORD;
};

struct SIMPLETEX_OUTPUT
{
	float4 Position: POSITION;
	float2 TexCoord: TEXCOORD;
};



// Simply transforms position coordinates and that's all
SIMPLE_OUTPUT TransformVS(SIMPLE_INPUT Input)
{
	SIMPLE_OUTPUT Output;
	Output.Position = mul(Input.Position, AllMat);
	return Output;
};


// Simply transforms position coordinated and copy texture coordinates
SIMPLETEX_OUTPUT TransformTexVS(SIMPLETEX_INPUT Input)
{
	SIMPLETEX_OUTPUT Output;
	Output.Position = mul(Input.Position, AllMat);
	Output.TexCoord = Input.TexCoord;
	
	return Output;
};





///////////////////////////////////////////////////////////////
// GENERAL PIXEL SHADERS
///////////////////////////////////////////////////////////////

struct LM_INPUT_PS
{
	float4 Light: COLOR0;
	float4 SMCoord: TEXCOORD;
};


// returns black color
float4 ZeroPS(): COLOR0
{
	float4 Color;
	Color.rgb = 0;
	Color.a = 1;
	return Color;
};

// returns ambient color
float4 SingleColorPS(): COLOR0
{
	float4 Color;
	Color.rgb = Ambient.rgb;
	Color.a = 1;
	return Color;
};

// Just texture color to output
float4 TexPS(float2 TexCoord: TEXCOORD) : COLOR0
{
	float4 TexCol;
	TexCol = tex2D(TexSamp0, TexCoord);
	
	return TexCol;	
};

// Light map pixel shader for light map rendering with projected shadow
float4 LightMapPS(LM_INPUT_PS Input): COLOR0
{
	float4 Shadow;
	float4 Color;
	
	Shadow = tex2Dproj(TexSamp3, Input.SMCoord);
	
	Color.rgb = Input.Light * Shadow;
	Color.a = 1;
	return Color;
};

////////////////////////////////////////////////////////////////
// NORMAL MAP GENERATION SHADERS
////////////////////////////////////////////////////////////////

struct NORMALMAP_INPUT
{
	float4 Position: POSITION;
	float3 Normal: NORMAL;
};

struct NORMALMAP_OUTPUT
{
	float4 Position: POSITION;
	float3 NormalInTexCoord: TEXCOORD0;
};

struct NORMALMAP_INPUT_PS
{
	float3 NormalInTexCoord: TEXCOORD0; 
};

// normal map stage 1 vertex shader
// vertex transformation for adding normals into floating point texture
NORMALMAP_OUTPUT NormalMapStage1VS(NORMALMAP_INPUT Input)
{
	NORMALMAP_OUTPUT Output = (NORMALMAP_OUTPUT) 0;
	
	Output.Position = mul(Input.Position, AllMat);
	Output.NormalInTexCoord = mul(Input.Normal, (float3x3) WorldMat);
	
	return Output;
};

// normal map stage 1 pixel shader
float4 NormalMapStage1PS(NORMALMAP_INPUT_PS Input): COLOR
{
	float4 Output;
	float NdotL;
	float3 N;
	
	N = normalize(Input.NormalInTexCoord);
	// convert normal to appropriate format
	DirLight.Direction = normalize( -DirLight.Direction );
	
	NdotL = dot( N, DirLight.Direction );
	
	if ( NdotL > 0.f ) Output.rgb = saturate( NdotL * DirLight.Diffuse.rgb );
	else Output.rgb = 0;
	
	Output.a = 1.0f;
	
	return Output;
}

// normal map stage 2 pixel shader
float4 NormalMapStage2PS(float2 TexCoord: TEXCOORD): COLOR
{
	float4 Output;
	float4 Normal;
	
	// get normal and number of added normals
	Normal = tex2D(TexSamp0, TexCoord);
	
	// Divide added normals with number of added normals
	//Normal.rgb /= Normal.a;
	
	// normalize
	Normal.rgb = normalize(Normal.rgb);
	
	// convert normal to appropriate format
	Output.rgb = Normal.rgb * 0.5f + 0.5f;
	Output.a = 1;
	
	return Output;
}

//////////////////////////////////////////////////////////////
// CHECKPOINT RENDERING SHADERS
////////////////////////////////////////////////////////////

struct CHECKPOINT_INPUT
{
	float4 Position: POSITION;
	float3 Normal: NORMAL;	
};

struct CHECKPOINT_OUTPUT
{
	float4 Position: POSITION;
	float4 Light: COLOR;
};

struct CHECKPOINT_INPUT_PS
{
	float4 Color: COLOR;
};

CHECKPOINT_OUTPUT CheckpointVS(CHECKPOINT_INPUT Input)
{
	float3 N;
	float3 V;
	float3 D;
	
    CHECKPOINT_OUTPUT Output = (CHECKPOINT_OUTPUT) 0;
    
    // compute position
    Output.Position = mul(Input.Position, AllMat);

    // compute lighting
    N = normalize(mul( Input.Normal, (float3x3) WorldViewMat ) );
    V = -normalize(mul(Input.Position, (float3x3) WorldViewMat) );
    D = -normalize(mul(DirLight.Direction, (float3x3) ViewMat) );

    COLOR_PAIR Lighting = (COLOR_PAIR) 0;
    Lighting = DoDirLight(N, V, D, DirLight);

    Output.Light = DoResultLight(Lighting, Material, Ambient);

    return Output;
};

float4 CheckpointPS(CHECKPOINT_INPUT_PS Input): COLOR
{
	return Input.Color;
};

//////////////////////////////////////////////////////////////////
// TERRAIN TRANSFORMATION SHADERS
//////////////////////////////////////////////////////////////////

// Input and output
struct TERRAIN_OPT_INPUT
{
	float4 Position: POSITION;
	float2 TexCoord: TEXCOORD0;
};

struct TERRAIN_OPT_NIGHT_INPUT
{
	float4 Position: POSITION;
	float4 Normal: NORMAL;
	float2 TexCoord: TEXCOORD0;
};

struct TERRAIN_OPT_OUTPUT
{
	float4 Position: POSITION0;
	float2 TexCoord: TEXCOORD0;
    float3 PixCoord: TEXCOORD1; // pixel coordinate stored for per pixel fog computation
};

struct TERRAIN_OPT_NIGHT_OUTPUT
{
	float4 Position: POSITION;
	float4 Light: COLOR;
	float2 TexCoord: TEXCOORD0;
	float3 PixCoord: TEXCOORD1;
};

struct TERRAIN_OUTPUT
{
    float4 Position: POSITION;
    float4 Light: COLOR0;
    float2 TerrainCoord: TEXCOORD1;
    float2 RoadCoord: TEXCOORD2;
    float  FogIntensity: TEXCOORD5; // fog intensity for vertex fog
    float2 NormalMapCoord: TEXCOORD7; // normal mapping
};

struct TERRAIN_SM_OUTPUT
{
	float4 Position: POSITION;
    float4 Light: COLOR0;
    float4 SMCoord: TEXCOORD3;
    float2 TerrainCoord: TEXCOORD1;
    float2 RoadCoord: TEXCOORD2;
    float  FogIntensity: TEXCOORD5; // fog intensity for vertex fog
};

struct TERRAIN_TSM_OUTPUT
{
    float4 Position: POSITION;
    float4 Light: COLOR0;
    float4 TSMCoord: TEXCOORD3;
    float2 TerrainCoord: TEXCOORD1;
    float2 RoadCoord: TEXCOORD2;
    float  FogIntensity: TEXCOORD5; // fog intensity for vertex fog
    float2 NormalMapCoord: TEXCOORD7; // normal mapping
};

struct TERRAIN_LM_OUTPUT
{
	float4 Position: POSITION;
	float4 Light: COLOR0;
	float4 SMCoord: TEXCOORD0;
    float  FogIntensity: TEXCOORD5; // fog intensity for vertex fog
};

struct TERRAIN_INPUT
{
    float4 Position: POSITION;
    float3 Normal: NORMAL;
    float2 TexCoord: TEXCOORD;
};

struct TERRAIN_NIGHT_OUTPUT
{
	float4 Position: POSITION;
	float4 Light: COLOR0;
	float2 TerrainCoord: TEXCOORD1;
	float2 RoadCoord: TEXCOORD2;
	float FogIntensity: TEXCOORD5;
};

struct TERRAIN_NIGHT_OUTPUT_SM1
{
	float4 Position: POSITION;
	float4 Light: COLOR0;
	float2 TerrainCoord: TEXCOORD1;
	float2 RoadCoord: TEXCOORD2;
	float FogIntensity: TEXCOORD5;
	float4 SM1: TEXCOORD3;
};

struct TERRAIN_NIGHT_OUTPUT_SM2
{
	float4 Position: POSITION;
	float4 Light: COLOR0;
	float2 TerrainCoord: TEXCOORD1;
	float2 RoadCoord: TEXCOORD2;
	float FogIntensity: TEXCOORD5;
	float4 SM1: TEXCOORD3;
	float4 SM2: TEXCOORD4;
};

struct TERRAIN_NIGHT_OUTPUT_SM3
{
	float4 Position: POSITION;
	float4 Light: COLOR0;
	float2 TerrainCoord: TEXCOORD1;
	float2 RoadCoord: TEXCOORD2;
	float FogIntensity: TEXCOORD5;
	float4 SM1: TEXCOORD3;
	float4 SM2: TEXCOORD4;
	float4 SM3: TEXCOORD6;
};

struct TERRAIN_NIGHT_OUTPUT_SM4
{
	float4 Position: POSITION;
	float4 Light: COLOR0;
	float2 TerrainCoord: TEXCOORD1;
	float2 RoadCoord: TEXCOORD2;
	float FogIntensity: TEXCOORD5;
	float4 SM1: TEXCOORD3;
	float4 SM2: TEXCOORD4;
	float4 SM3: TEXCOORD6;
	float4 SM4: TEXCOORD7;
};


// Rotate terrain texture 
bool RotateTerrainTexture : register(b1) = false;
// Rotate road terrain texture
bool RotateRoadTexture : register(b2) = false;
// Terrain texture texcoord rotation matrix
matrix TerrainTextureMat; 
// Terrain road texture texcoord rotation matrix
matrix RoadTextureMat;

TERRAIN_OUTPUT TerrainNightVS(TERRAIN_INPUT Input)
{
	float3 N;
	float3 V;
	int i;
	float3 TexCoord;
	TERRAIN_OUTPUT Output = (TERRAIN_OUTPUT) 0;
	 
	// compute position
    Output.Position = mul(Input.Position, AllMat);
    
    // Terrain texture
    if ( RotateTerrainTexture )
    {
	    TexCoord.xy = Input.TexCoord.xy - 0.5;
		TexCoord.z = 1;
		Output.TerrainCoord.xy = mul( TexCoord, (float3x2) TerrainTextureMat ).xy + 0.5;
	}
    else Output.TerrainCoord = Input.TexCoord.xy;
    
    // Road texture
	if ( RotateRoadTexture )
    {
	    TexCoord.xy = Input.TexCoord.xy - 0.5;
		TexCoord.z = 1;
		Output.RoadCoord.xy = mul( TexCoord, (float3x2) RoadTextureMat ).xy + 0.5;
	}
	else Output.RoadCoord = Input.TexCoord;
	
	// compute lighting
	N = normalize(mul(Input.Normal, (float3x3) WorldViewMat) );
	V = -normalize(mul(Input.Position, (float3x3) WorldViewMat) );
	
	COLOR_PAIR ResultLighting = (COLOR_PAIR) 0;
	COLOR_PAIR Lighting;
	for (i = 0; i < 9; i++)
	{
		
		Lighting = (COLOR_PAIR) 0;
		Lighting = DoSpotLight(N, V, Input.Position, SpotLights[i]);
		ResultLighting.Diffuse += Lighting.Diffuse;
		ResultLighting.Specular += Lighting.Specular;
	}
	Output.Light = DoResultLight(ResultLighting, Material, Ambient);	
	
    
    // fog intensity calculation
    //Output.FogIntensity = ComputeFogIntensity( Output.Position.xyz );
    
    return Output;
};

TERRAIN_NIGHT_OUTPUT_SM1 TerrainNightSM1VS(TERRAIN_INPUT Input)
{
	float3 N;
	float3 V;
	int i;
	float3 TexCoord;
	float4 World;
	TERRAIN_NIGHT_OUTPUT_SM1 Output = (TERRAIN_NIGHT_OUTPUT_SM1) 0;
	 
	// compute position
    Output.Position = mul(Input.Position, AllMat);
    
    // Terrain texture
    if ( RotateTerrainTexture )
    {
	    TexCoord.xy = Input.TexCoord.xy - 0.5;
		TexCoord.z = 1;
		Output.TerrainCoord.xy = mul( TexCoord, (float3x2) TerrainTextureMat ).xy + 0.5;
	}
    else Output.TerrainCoord = Input.TexCoord.xy;
    
    // Road texture
	if ( RotateRoadTexture )
    {
	    TexCoord.xy = Input.TexCoord.xy - 0.5;
		TexCoord.z = 1;
		Output.RoadCoord.xy = mul( TexCoord, (float3x2) RoadTextureMat ).xy + 0.5;
	}
	else Output.RoadCoord = Input.TexCoord;
	
	// compute lighting
	N = normalize(mul(Input.Normal, (float3x3) WorldViewMat) );
	V = -normalize(mul(Input.Position, (float3x3) WorldViewMat) );
	
	COLOR_PAIR ResultLighting = (COLOR_PAIR) 0;
	COLOR_PAIR Lighting;
	for (i = 0; i < 9; i++)
	{
		
		Lighting = (COLOR_PAIR) 0;
		Lighting = DoSpotLight(N, V, Input.Position, SpotLights[i]);
		ResultLighting.Diffuse += Lighting.Diffuse;
		ResultLighting.Specular += Lighting.Specular;
	}
	Output.Light = DoResultLight(ResultLighting, Material, Ambient);	
	
    
    // fog intensity calculation
    //Output.FogIntensity = ComputeFogIntensity( Output.Position.xyz );
    
    // shadow map coords
    World = mul(Input.Position, WorldMat);
    Output.SM1 = mul(World, ShadowMat);
    Output.SM1 /= Output.SM1.w;
    Output.SM1.xy = clamp(Output.SM1.xy, ClampVector1.xy, ClampVector1.zw);
    
    if (Output.SM1.z > 1.0) Output.SM1.z = -1.0;
    
    return Output;
};

TERRAIN_NIGHT_OUTPUT_SM2 TerrainNightSM2VS(TERRAIN_INPUT Input)
{
	float3 N;
	float3 V;
	int i;
	float3 TexCoord;
	float4 World;
	TERRAIN_NIGHT_OUTPUT_SM2 Output = (TERRAIN_NIGHT_OUTPUT_SM2) 0;
	 
	// compute position
    Output.Position = mul(Input.Position, AllMat);
    
    // Terrain texture
    if ( RotateTerrainTexture )
    {
	    TexCoord.xy = Input.TexCoord.xy - 0.5;
		TexCoord.z = 1;
		Output.TerrainCoord.xy = mul( TexCoord, (float3x2) TerrainTextureMat ).xy + 0.5;
	}
    else Output.TerrainCoord = Input.TexCoord.xy;
    
    // Road texture
	if ( RotateRoadTexture )
    {
	    TexCoord.xy = Input.TexCoord.xy - 0.5;
		TexCoord.z = 1;
		Output.RoadCoord.xy = mul( TexCoord, (float3x2) RoadTextureMat ).xy + 0.5;
	}
	else Output.RoadCoord = Input.TexCoord;
	
	// compute lighting
	N = normalize(mul(Input.Normal, (float3x3) WorldViewMat) );
	V = -normalize(mul(Input.Position, (float3x3) WorldViewMat) );
	
	COLOR_PAIR ResultLighting = (COLOR_PAIR) 0;
	COLOR_PAIR Lighting;
	for (i = 0; i < 9; i++)
	{
		
		Lighting = (COLOR_PAIR) 0;
		Lighting = DoSpotLight(N, V, Input.Position, SpotLights[i]);
		ResultLighting.Diffuse += Lighting.Diffuse;
		ResultLighting.Specular += Lighting.Specular;
	}
	Output.Light = DoResultLight(ResultLighting, Material, Ambient);	
	
    
    // fog intensity calculation
    //Output.FogIntensity = ComputeFogIntensity( Output.Position.xyz );
    
    // shadow map coords
    World = mul(Input.Position, WorldMat);
    Output.SM1 = mul(World, ShadowMat);
    Output.SM1 /= Output.SM1.w;
    Output.SM1.xy = clamp(Output.SM1.xy, ClampVector1.xy, ClampVector1.zw);
    
    Output.SM2 = mul(World, ShadowMat2);
    Output.SM2 /= Output.SM2.w;
    Output.SM2.xy = clamp(Output.SM2.xy, ClampVector2.xy, ClampVector2.zw);
    
    if (Output.SM1.z > 1.0) Output.SM1.z = -1.0;
	if (Output.SM2.z > 1.0) Output.SM2.z = -1.0;
    
    return Output;
};


TERRAIN_NIGHT_OUTPUT_SM3 TerrainNightSM3VS(TERRAIN_INPUT Input)
{
	float3 N;
	float3 V;
	int i;
	float3 TexCoord;
	float4 World;
	TERRAIN_NIGHT_OUTPUT_SM3 Output = (TERRAIN_NIGHT_OUTPUT_SM3) 0;
	 
	// compute position
    Output.Position = mul(Input.Position, AllMat);
    
    // Terrain texture
    if ( RotateTerrainTexture )
    {
	    TexCoord.xy = Input.TexCoord.xy - 0.5;
		TexCoord.z = 1;
		Output.TerrainCoord.xy = mul( TexCoord, (float3x2) TerrainTextureMat ).xy + 0.5;
	}
    else Output.TerrainCoord = Input.TexCoord.xy;
    
    // Road texture
	if ( RotateRoadTexture )
    {
	    TexCoord.xy = Input.TexCoord.xy - 0.5;
		TexCoord.z = 1;
		Output.RoadCoord.xy = mul( TexCoord, (float3x2) RoadTextureMat ).xy + 0.5;
	}
	else Output.RoadCoord = Input.TexCoord;
	
	// compute lighting
	N = normalize(mul(Input.Normal, (float3x3) WorldViewMat) );
	V = -normalize(mul(Input.Position, (float3x3) WorldViewMat) );
	
	COLOR_PAIR ResultLighting = (COLOR_PAIR) 0;
	COLOR_PAIR Lighting;
	for (i = 0; i < 9; i++)
	{
		
		Lighting = (COLOR_PAIR) 0;
		Lighting = DoSpotLight(N, V, Input.Position, SpotLights[i]);
		ResultLighting.Diffuse += Lighting.Diffuse;
		ResultLighting.Specular += Lighting.Specular;
	}
	Output.Light = DoResultLight(ResultLighting, Material, Ambient);	
	
    
    // fog intensity calculation
    //Output.FogIntensity = ComputeFogIntensity( Output.Position.xyz );
    
    // shadow map coords
    World = mul(Input.Position, WorldMat);
    Output.SM1 = mul(World, ShadowMat);
    Output.SM1 /= Output.SM1.w;
    Output.SM1.xy = clamp(Output.SM1.xy, ClampVector1.xy, ClampVector1.zw);
    
    Output.SM2 = mul(World, ShadowMat2);
    Output.SM2 /= Output.SM2.w;
    Output.SM2.xy = clamp(Output.SM2.xy, ClampVector2.xy, ClampVector2.zw);
    
    Output.SM3 = mul(World, ShadowMat3);
    Output.SM3 /= Output.SM3.w;
    Output.SM3.xy = clamp(Output.SM3.xy, ClampVector3.xy, ClampVector3.zw);
    
    if (Output.SM1.z > 1.0) Output.SM1.z = -1.0;
	if (Output.SM2.z > 1.0) Output.SM2.z = -1.0;
	if (Output.SM3.z > 1.0) Output.SM3.z = -1.0;
    
    return Output;
};


TERRAIN_NIGHT_OUTPUT_SM4 TerrainNightSM4VS(TERRAIN_INPUT Input)
{
	float3 N;
	float3 V;
	int i;
	float3 TexCoord;
	float4 World;
	TERRAIN_NIGHT_OUTPUT_SM4 Output = (TERRAIN_NIGHT_OUTPUT_SM4) 0;
	 
	// compute position
    Output.Position = mul(Input.Position, AllMat);
    
    // Terrain texture
    if ( RotateTerrainTexture )
    {
	    TexCoord.xy = Input.TexCoord.xy - 0.5;
		TexCoord.z = 1;
		Output.TerrainCoord.xy = mul( TexCoord, (float3x2) TerrainTextureMat ).xy + 0.5;
	}
    else Output.TerrainCoord = Input.TexCoord.xy;
    
    // Road texture
	if ( RotateRoadTexture )
    {
	    TexCoord.xy = Input.TexCoord.xy - 0.5;
		TexCoord.z = 1;
		Output.RoadCoord.xy = mul( TexCoord, (float3x2) RoadTextureMat ).xy + 0.5;
	}
	else Output.RoadCoord = Input.TexCoord;
	
	// compute lighting
	N = normalize(mul(Input.Normal, (float3x3) WorldViewMat) );
	V = -normalize(mul(Input.Position, (float3x3) WorldViewMat) );
	
	COLOR_PAIR ResultLighting = (COLOR_PAIR) 0;
	COLOR_PAIR Lighting;
	for (i = 0; i < 9; i++)
	{
		
		Lighting = (COLOR_PAIR) 0;
		Lighting = DoSpotLight(N, V, Input.Position, SpotLights[i]);
		ResultLighting.Diffuse += Lighting.Diffuse;
		ResultLighting.Specular += Lighting.Specular;
	}
	Output.Light = DoResultLight(ResultLighting, Material, Ambient);	
	
    
    // fog intensity calculation
    //Output.FogIntensity = ComputeFogIntensity( Output.Position.xyz );
    
    // shadow map coords
    World = mul(Input.Position, WorldMat);
    Output.SM1 = mul(World, ShadowMat);
    Output.SM1 /= Output.SM1.w;
    Output.SM1.xy = clamp(Output.SM1.xy, ClampVector1.xy, ClampVector1.zw);
    
    Output.SM2 = mul(World, ShadowMat2);
    Output.SM2 /= Output.SM2.w;
    Output.SM2.xy = clamp(Output.SM2.xy, ClampVector2.xy, ClampVector2.zw);
    
    Output.SM3 = mul(World, ShadowMat3);
    Output.SM3 /= Output.SM3.w;
    Output.SM3.xy = clamp(Output.SM3.xy, ClampVector3.xy, ClampVector3.zw);
    
    Output.SM4 = mul(World, ShadowMat4);
    Output.SM4 /= Output.SM4.w;
    Output.SM4.xy = clamp(Output.SM4.xy, ClampVector4.xy, ClampVector4.zw);
    
    if (Output.SM1.z > 1.0) Output.SM1.z = -1.0;
	if (Output.SM2.z > 1.0) Output.SM2.z = -1.0;
	if (Output.SM3.z > 1.0) Output.SM3.z = -1.0;
	if (Output.SM4.z > 1.0) Output.SM4.z = -1.0;
    
    return Output;
};


TERRAIN_OUTPUT TerrainVS(TERRAIN_INPUT Input)
{
	float3 N;
	float3 V;
	float3 D;
	float3 TexCoord;
    TERRAIN_OUTPUT Output = (TERRAIN_OUTPUT) 0;
    
    // compute position
    Output.Position = mul(Input.Position, AllMat);

    // compute texture coordinates
    // Terrain texture
    if ( RotateTerrainTexture )
    {
	    TexCoord.xy = Input.TexCoord.xy - 0.5;
		TexCoord.z = 1;
		Output.TerrainCoord.xy = mul( TexCoord, (float3x2) TerrainTextureMat ).xy + 0.5;
	}
    else Output.TerrainCoord = Input.TexCoord.xy;
		
	// Road texture
	if ( RotateRoadTexture )
    {
	    TexCoord.xy = Input.TexCoord.xy - 0.5;
		TexCoord.z = 1;
		Output.RoadCoord.xy = mul( TexCoord, (float3x2) RoadTextureMat ).xy + 0.5;
	}
	else Output.RoadCoord = Input.TexCoord;

/* NOT USED - NORMAL MAPPING PER PIXEL INSTEAD
	// compute lighting
    N = normalize(mul( Input.Normal, (float3x3) WorldViewMat ) );
    V = -normalize(mul(Input.Position, (float3x3) WorldViewMat) );
    D = -normalize(mul(DirLight.Direction, (float3x3) ViewMat) );

    COLOR_PAIR Lighting = (COLOR_PAIR) 0;
    Lighting = DoDirLight(N, V, D, DirLight);

    Output.Light = DoResultLight(Lighting, Material, Ambient);
*/    

	// normal mapping
	float4 vec;
	vec = mul( Input.Position, WorldMat );
	Output.NormalMapCoord.x = vec.x / MapWidth; 
	Output.NormalMapCoord.y = vec.z / MapDepth;


    // fog intensity calculation
    Output.FogIntensity = ComputeFogIntensity( Output.Position.xyz );
    
    return Output;
};


// VS for terrain LOD 0
TERRAIN_OPT_OUTPUT TerrainLOD0VS(TERRAIN_OPT_INPUT Input)
{
	TERRAIN_OPT_OUTPUT Output = (TERRAIN_OPT_OUTPUT) 0;
	Output.Position = mul(Input.Position, AllMat);
	Output.TexCoord = Input.TexCoord;
	
    // pixel coordinates for fog computation
    Output.PixCoord.xyz = Output.Position.xyz;
    
	return Output;
};


// Terrain LOD0 directional light vertex shader
TERRAIN_OPT_NIGHT_OUTPUT TerrainLOD0DLVS(TERRAIN_OPT_NIGHT_INPUT Input)
{
	float3 N;
	float3 V;
	float3 D;

	TERRAIN_OPT_NIGHT_OUTPUT Output = (TERRAIN_OPT_NIGHT_OUTPUT) 0;
	
	Output.Position = mul(Input.Position, AllMat);
	Output.TexCoord = Input.TexCoord;
	
    // pixel coordinates for fog computation
    Output.PixCoord.xyz = Output.Position.xyz;
/*    
     // compute lighting
    N = normalize(mul( Input.Normal, (float3x3) WorldViewMat ) );
    V = -normalize(mul(Input.Position, (float3x3) WorldViewMat) );
    D = -normalize(mul(DirLight.Direction, (float3x3) ViewMat) );

    COLOR_PAIR Lighting = (COLOR_PAIR) 0;
    Lighting = DoDirLight(N, V, D, DirLight);

    Output.Light = DoResultLight(Lighting, Material, Ambient);
*/	
	return Output;
};


TERRAIN_OPT_NIGHT_OUTPUT TerrainNightLOD0VS(TERRAIN_OPT_NIGHT_INPUT Input)
{
	float3 N, V;
	int i;

	TERRAIN_OPT_NIGHT_OUTPUT Output = (TERRAIN_OPT_NIGHT_OUTPUT) 0;
	Output.Position = mul(Input.Position, AllMat);
	Output.TexCoord = Input.TexCoord;
	
	// compute lighting
	N = normalize(mul(Input.Normal, (float3x3) WorldViewMat) );
	V = -normalize(mul(Input.Position, (float3x3) WorldViewMat) );
	
	COLOR_PAIR ResultLighting = (COLOR_PAIR) 0;
	COLOR_PAIR Lighting;
	for (i = 0; i < 9; i++)
	{
		Lighting = (COLOR_PAIR) 0;
		Lighting = DoSpotLight(N, V, Input.Position, SpotLights[i]);
		ResultLighting.Diffuse += Lighting.Diffuse;
		ResultLighting.Specular += Lighting.Specular;
	}
	Output.Light = DoResultLight(ResultLighting, Material, Ambient);
	
    // pixel coordinates for fog computation
    //Output.PixCoord.xyz = Output.Position.xyz;
    
    return Output;
}


// terrain VS without directional light
TERRAIN_OUTPUT TerrainNoDirLightVS(TERRAIN_INPUT Input)
{
	float3 TexCoord;
    TERRAIN_OUTPUT Output = (TERRAIN_OUTPUT) 0;
    
    // compute position
    Output.Position = mul(Input.Position, AllMat);

    // compute texture coordinates
    // Terrain texture
    if ( RotateTerrainTexture )
    {
	    TexCoord.xy = Input.TexCoord.xy - 0.5;
		TexCoord.z = 1;
		Output.TerrainCoord.xy = mul( TexCoord, (float3x2) TerrainTextureMat ).xy + 0.5;
	}
    else Output.TerrainCoord = Input.TexCoord.xy;
		
	// Road texture
	if ( RotateRoadTexture )
    {
	    TexCoord.xy = Input.TexCoord.xy - 0.5;
		TexCoord.z = 1;
		Output.RoadCoord.xy = mul( TexCoord, (float3x2) RoadTextureMat ).xy + 0.5;
	}
	else Output.RoadCoord = Input.TexCoord;

    // fog intensity calculation
    Output.FogIntensity = ComputeFogIntensity( Output.Position.xyz );

    return Output;
};


// Renders terrain lightened by spotlight
TERRAIN_OUTPUT TerrainSpotLightVS(TERRAIN_INPUT Input)
{
	float3 N;
	float3 V;
	float3 TexCoord;
	float4 World;
	
	TERRAIN_OUTPUT Output = (TERRAIN_OUTPUT) 0;
	
	// compute output position
	Output.Position = mul(Input.Position, AllMat);
	
    // Terrain texture
    if ( RotateTerrainTexture )
    {
	    TexCoord.xy = Input.TexCoord.xy - 0.5;
		TexCoord.z = 1;
		Output.TerrainCoord.xy = mul( TexCoord, (float3x2) TerrainTextureMat ).xy + 0.5;
	}
    else Output.TerrainCoord = Input.TexCoord.xy;
		
	// Road texture
	if ( RotateRoadTexture )
    {
	    TexCoord.xy = Input.TexCoord.xy - 0.5;
		TexCoord.z = 1;
		Output.RoadCoord.xy = mul( TexCoord, (float3x2) RoadTextureMat ).xy + 0.5;
	}
	else Output.RoadCoord = Input.TexCoord;
	
	// compute lighting
	N = normalize(mul(Input.Normal, (float3x3) WorldViewMat) );
	V = -normalize(mul(Input.Position, (float3x3) WorldViewMat) );
	
	COLOR_PAIR Lighting = (COLOR_PAIR) 0;
	Lighting = DoSpotLight(N, V, Input.Position, SpotLight);
	
	Output.Light = DoResultLight(Lighting, Material, Ambient);
	
    // fog intensity calculation
    Output.FogIntensity = ComputeFogIntensity( Output.Position.xyz );

	return Output;
};


// Renders terrain with shadow map for spot light
TERRAIN_SM_OUTPUT TerrainSpotLightWithSMVS(TERRAIN_INPUT Input)
{
	float3 N;
	float3 V;
	float3 TexCoord;
	float4 World;
	float4 SMCoord;
	
	TERRAIN_SM_OUTPUT Output = (TERRAIN_SM_OUTPUT) 0;
	
	// compute output position
	Output.Position = mul(Input.Position, AllMat);
	
	// compute shadow map texture coords
	World = mul(Input.Position, WorldMat);
	SMCoord = mul(World, ShadowMat);
	//Output.SMCoord = SMCoord / SMCoord.w;
	//Output.SMCoord = SMCoord / SMCoord.w;
	//Output.SMCoord.xy = clamp(Output.SMCoord.xy, ClampVector1.xy, ClampVector1.zw);
	Output.SMCoord = SMCoord;
	
    // Terrain texture
    if ( RotateTerrainTexture )
    {
	    TexCoord.xy = Input.TexCoord.xy - 0.5;
		TexCoord.z = 1;
		Output.TerrainCoord.xy = mul( TexCoord, (float3x2) TerrainTextureMat ).xy + 0.5;
	}
    else Output.TerrainCoord = Input.TexCoord.xy;
		
	// Road texture
	if ( RotateRoadTexture )
    {
	    TexCoord.xy = Input.TexCoord.xy - 0.5;
		TexCoord.z = 1;
		Output.RoadCoord.xy = mul( TexCoord, (float3x2) RoadTextureMat ).xy + 0.5;
	}
	else Output.RoadCoord = Input.TexCoord;
	
	// compute lighting
	N = normalize(mul(Input.Normal, (float3x3) WorldViewMat) );
	V = -normalize(mul(Input.Position, (float3x3) WorldViewMat) );
	
	COLOR_PAIR Lighting = (COLOR_PAIR) 0;
	Lighting = DoSpotLight(N, V, Input.Position, SpotLight);
	
	Output.Light = DoResultLight(Lighting, Material, Ambient);
	
    // fog intensity calculation
    Output.FogIntensity = ComputeFogIntensity( Output.Position.xyz );

	return Output;
};


// Terrain shader with TSM coord generation
TERRAIN_TSM_OUTPUT TerrainWithTSMVS(TERRAIN_INPUT Input)
{
	float3 N;
	float3 V;
	float3 D;
	float3 TexCoord;
	float4 World;
	float4 LightCoord;
	
    TERRAIN_TSM_OUTPUT Output = (TERRAIN_TSM_OUTPUT) 0;
    
    // compute position
    Output.Position = mul(Input.Position, AllMat);

    // compute texture coordinates
    // Terrain texture
    if ( RotateTerrainTexture )
    {
	    TexCoord.xy = Input.TexCoord.xy - 0.5;
		TexCoord.z = 1;
		Output.TerrainCoord.xy = mul( TexCoord, (float3x2) TerrainTextureMat ).xy + 0.5;
	}
    else Output.TerrainCoord = Input.TexCoord.xy;
		
	// Road texture
	if ( RotateRoadTexture )
    {
	    TexCoord.xy = Input.TexCoord.xy - 0.5;
		TexCoord.z = 1;
		Output.RoadCoord.xy = mul( TexCoord, (float3x2) RoadTextureMat ).xy + 0.5;
	}
	else Output.RoadCoord = Input.TexCoord;
		
	// compute texture coordinates for TSM
    World = mul(Input.Position, WorldMat);
    LightCoord = mul(World, ShadowMat);
    Output.TSMCoord = mul(LightCoord, PostProjMat);
    //Output.TSMCoord /= Output.TSMCoord.w;
    Output.TSMCoord.z = LightCoord.z * Output.TSMCoord.w;

/* NOT USED - NORMAL MAPPING PER PIXEL INSTEAD
    // compute lighting
    N = normalize(mul( Input.Normal, (float3x3) WorldViewMat ) );
    V = -normalize(mul(Input.Position, (float3x3) WorldViewMat) );
    D = -normalize(mul(DirLight.Direction, (float3x3) ViewMat) );

    COLOR_PAIR Lighting = (COLOR_PAIR) 0;
    Lighting = DoDirLight(N, V, D, DirLight);

    Output.Light = DoResultLight(Lighting, Material, Ambient);
*/    

	// normal mapping
	float4 vec;
	vec = mul( Input.Position, WorldMat );
	Output.NormalMapCoord.x = vec.x / MapWidth; 
	Output.NormalMapCoord.y = vec.z / MapDepth;

    
    // fog intensity calculation
    Output.FogIntensity = ComputeFogIntensity( Output.Position.xyz );

   return Output;
}


// Terrain spot light light map vertex shader
TERRAIN_LM_OUTPUT TerrainSpotLightMapVS(TERRAIN_INPUT Input)
{
	float3 N;
	float3 V;
	float4 View;
	
	TERRAIN_LM_OUTPUT Output = (TERRAIN_LM_OUTPUT) 0;
	
	// compute output position
	Output.Position = mul(Input.Position, AllMat);
	
	// compute shadow map texture coords and z-depth
	View = mul(Input.Position, WorldViewMat);
	Output.SMCoord = mul(View, ShadowMat);
	
	// compute lighting
	N = normalize(mul(Input.Normal, (float3x3) WorldViewMat) );
	V = -normalize( (float3) View);
	
	COLOR_PAIR Lighting = (COLOR_PAIR) 0;
	Lighting = DoSpotLight(N, V, Input.Position, SpotLight);
	
	Output.Light = DoResultLight(Lighting, Material, Ambient);
	
    // fog intensity calculation
    Output.FogIntensity = ComputeFogIntensity( Output.Position.xyz );

	return Output;
};




//////////////////////////////////////////////////////////////////////////
// TERRAIN PIXEL SHADERS
//////////////////////////////////////////////////////////////////////////

struct TERRAIN_NIGHT_INPUT_PS
{
	float4 Light: COLOR0;
	float2 TerrainCoord: TEXCOORD1;
	float2 RoadCoord: TEXCOORD2;
	float FogIntensity: TEXCOORD5;
};

struct TERRAIN_NIGHT_INPUT_SM1_PS
{
	float4 Light: COLOR0;
	float2 TerrainCoord: TEXCOORD1;
	float2 RoadCoord: TEXCOORD2;
	float FogIntensity: TEXCOORD5;
	float4 SM1: TEXCOORD3;
};

struct TERRAIN_NIGHT_INPUT_SM2_PS
{
	float4 Light: COLOR0;
	float2 TerrainCoord: TEXCOORD1;
	float2 RoadCoord: TEXCOORD2;
	float FogIntensity: TEXCOORD5;
	float4 SM1: TEXCOORD3;
	float4 SM2: TEXCOORD4;
};

struct TERRAIN_NIGHT_INPUT_SM3_PS
{
	float4 Light: COLOR0;
	float2 TerrainCoord: TEXCOORD1;
	float2 RoadCoord: TEXCOORD2;
	float FogIntensity: TEXCOORD5;
	float4 SM1: TEXCOORD3;
	float4 SM2: TEXCOORD4;
	float4 SM3: TEXCOORD6;
};

struct TERRAIN_NIGHT_INPUT_SM4_PS
{
	float4 Light: COLOR0;
	float2 TerrainCoord: TEXCOORD1;
	float2 RoadCoord: TEXCOORD2;
	float FogIntensity: TEXCOORD5;
	float4 SM1: TEXCOORD3;
	float4 SM2: TEXCOORD4;
	float4 SM3: TEXCOORD6;
	float4 SM4: TEXCOORD7;
};

struct TERRAIN_INPUT_PS
{
	float4 Light: COLOR0;
    float2 TerrainCoord: TEXCOORD1;
    float2 RoadCoord: TEXCOORD2;
	float  FogIntensity: TEXCOORD5; // fog intensity for vertex fog
    float2 NormalMapCoord: TEXCOORD7; // normal mapping
};

struct TERRAIN_OPT_INPUT_PS
{
    float2 TexCoord: TEXCOORD0;
	float3 PixCoord: TEXCOORD1; // pixel coordinate for per pixel fog computation
};

struct TERRAIN_OPT_NIGHT_INPUT_PS
{
	float4 Light: COLOR;
	float2 TexCoord: TEXCOORD0;
	float3 PixCoord: TEXCOORD1;
};

struct TERRAIN_SM_INPUT_PS
{
	float4 Light: COLOR0;
    float4 SMCoord: TEXCOORD3;
    float2 TerrainCoord: TEXCOORD1;
    float2 RoadCoord: TEXCOORD2;
    float  FogIntensity: TEXCOORD5; // fog intensity for vertex fog
};

struct TERRAIN_TSM_INPUT_PS
{
	float4 Light: COLOR0;
    float4 TSMCoord: TEXCOORD3;
    float2 TerrainCoord: TEXCOORD1;
    float2 RoadCoord: TEXCOORD2;
    float  FogIntensity: TEXCOORD5; // fog intensity for vertex fog
    float2 NormalMapCoord: TEXCOORD7; // normal mapping
};

float4 TerrainLOD0TexturePS(TERRAIN_INPUT_PS Input) : COLOR
{
	float4 TerrainCol;
	float4 RoadCol;
	float4 Color;
	float3 Light;
	float3 N;
	float NdotL;
	
	TerrainCol = tex2D( TexSamp1, Input.TerrainCoord );
	RoadCol = tex2D( TexSamp2, Input.RoadCoord );
	Light = tex2D( TexSamp3, Input.NormalMapCoord.xy );
	
	Color.rgb = saturate( Light.xyz + Ambient.rgb );	
	Color.rgb *= (TerrainCol.rgb * (1 - RoadCol.a) + RoadCol.rgb * RoadCol.a);
	Color.a = 1;
	
	return Color;
};

float4 TerrainLOD0TextureNightPS(TERRAIN_INPUT_PS Input) : COLOR
{
	float4 TerrainCol;
	float4 RoadCol;
	float4 Color;
	float4 LightMap;
	
	LightMap = tex2D(TexSamp0, Input.TerrainCoord);
	TerrainCol = tex2D( TexSamp1, Input.TerrainCoord );
	RoadCol = tex2D( TexSamp2, Input.RoadCoord );
	
	Color.rgb = (TerrainCol.rgb * (1 - RoadCol.a) + RoadCol.rgb * RoadCol.a);
	Color.a = LightMap.r;
	
	return Color;
};


float4 TerrainPS(TERRAIN_INPUT_PS Input) : COLOR
{
	float4 TerrainCol;
	float4 RoadCol;	
	float4 Color;
	float3 Light;
	
	TerrainCol = tex2D( TexSamp1, Input.TerrainCoord );
	RoadCol = tex2D( TexSamp2, Input.RoadCoord );
	Light = tex2D( TexSamp3, Input.NormalMapCoord.xy );
	
	Color.rgb = saturate( Light.xyz + Ambient.rgb );
	Color.rgb *= (TerrainCol.rgb * (1 - RoadCol.a) + RoadCol.rgb * RoadCol.a);
	Color.a = 1;
	
	// apply fog
	Color.rgb = ApplyFogColor( Input.FogIntensity, Color.rgb );
		
	return Color;
};

// PS for terrain LOD 0
float4 TerrainLOD0PS( TERRAIN_OPT_INPUT_PS Input ) : COLOR0
{
	float4 TexCol;
	TexCol = tex2D( TexSamp0, Input.TexCoord );
	
    // fog intensity calculation
    TexCol.rgb = ApplyPixelFogColor( Input.PixCoord, TexCol.rgb );
    
	return TexCol;	
};


// Terrain LOD0 Optimalization pixel shader
float4 TerrainLOD0OptimalizationPS( TERRAIN_INPUT_PS Input) : COLOR
{
	float4 TerrainCol;
	float4 RoadCol;	
	float4 Color;
	
	TerrainCol = tex2D(TexSamp1, Input.TerrainCoord);
	RoadCol = tex2D(TexSamp2, Input.RoadCoord);
	
	Color.rgb = (TerrainCol.rgb * (1 - RoadCol.a) + RoadCol.rgb * RoadCol.a);
	Color.a = 1;

	return Color;	
};

float4 TerrainLOD0DLPS( TERRAIN_OPT_NIGHT_INPUT_PS Input) : COLOR0
{
	float4 TexCol;
	float4 Color;
	
	TexCol = tex2D( TexSamp0, Input.TexCoord );
	
	//Color.rgb = TexCol * saturate(Input.Light + Ambient);
	Color.rgb = TexCol.rgb;
	Color.a = 1.0f;
	
    // fog intensity calculation
	Color.rgb = ApplyPixelFogColor( Input.PixCoord, Color.rgb );
    
	return Color;	
}

float4 TerrainNightLOD0PS (TERRAIN_OPT_NIGHT_INPUT_PS Input) : COLOR0
{
	float4 TexCol;
	float4 Color;
	TexCol = tex2D( TexSamp0, Input.TexCoord );
	
	Color.rgb = TexCol.rgb * saturate(Input.Light.rgb + Ambient + TexCol.a * LightMapColor);
	Color.a = 1;
	
	// fog intensity calculation
	//Color.rgb = ApplyPixelFogColor( Input.PixCoord, Color.rgb );
    
    return Color;
}


// Terrain with shadow mapping pixel shader
float4 TerrainWithSMPS(TERRAIN_SM_INPUT_PS Input): COLOR
{
	float4 TerrainCol;
	float4 RoadCol;	
	float4 Color;
	float ShadowDepth;
	float Shadow;
	float4 LightMap;
	
	LightMap = tex2D(TexSamp0, Input.TerrainCoord);
	TerrainCol = tex2D(TexSamp1, Input.TerrainCoord);
	RoadCol = tex2D(TexSamp2, Input.RoadCoord);
	
	// Get shadow map depth to r
	Input.SMCoord /= Input.SMCoord.w;
	Input.SMCoord.z = min(Input.SMCoord.z, 1.0);
	ShadowDepth = tex2Dproj(TexSamp3, Input.SMCoord).r;
	
	// compare
	Shadow = (ShadowDepth >=  Input.SMCoord.z / Input.SMCoord.w);
	
	//Color.rgb = saturate(Input.Light.rgb * Shadow);
	Color.rgb = LightMap.r * LightMapColor * Shadow;
	Color.rgb *= (TerrainCol.rgb * (1 - RoadCol.a) + RoadCol.rgb * RoadCol.a);
	Color.a = 1;
	
	return Color;
};

float4 TerrainNightPS(TERRAIN_INPUT_PS Input): COLOR
{
	float4 TerrainCol;
	float4 RoadCol;	
	float4 Color;
	float4 LightMap;
	
	LightMap = tex2D(TexSamp0, Input.TerrainCoord);
	TerrainCol = tex2D(TexSamp1, Input.TerrainCoord);
	RoadCol = tex2D(TexSamp2, Input.RoadCoord);
	
	//Color.rgb = saturate(Input.Light.rgb + Ambient.rgb + LightMap);
	Color.rgb = Input.Light.rgb + Ambient.rgb + LightMap.r * LightMapColor;
	Color.rgb *= (TerrainCol.rgb * (1 - RoadCol.a) + RoadCol.rgb * RoadCol.a);
	Color.a = 1;
	
	// apply fog
	//Color.rgb = ApplyFogColor( Input.FogIntensity, Color.rgb );
	
	return Color;
};

float4 TerrainNightSM1PS(TERRAIN_NIGHT_INPUT_SM1_PS Input): COLOR
{
	float4 TerrainCol;
	float4 RoadCol;	
	float4 Color;
	float4 LightMap;
	float ShadowDepth;
	float Shadowed;
	
	LightMap = tex2D(TexSamp0, Input.TerrainCoord);
	TerrainCol = tex2D(TexSamp1, Input.TerrainCoord);
	RoadCol = tex2D(TexSamp2, Input.RoadCoord);
	
	ShadowDepth.r = tex2Dproj(TexSamp3, Input.SM1).r;
	Shadowed = (ShadowDepth.r >= Input.SM1.z);
	
	//Color.rgb = saturate(Input.Light.rgb * Shadowed + LightMap + Ambient.rgb);
	Color.rgb = Input.Light.rgb * Shadowed + LightMap.r * LightMapColor + Ambient.rgb;
	Color.rgb *= (TerrainCol.rgb * (1 - RoadCol.a) + RoadCol.rgb * RoadCol.a);
	Color.a = 1;
	
	// apply fog
	//Color.rgb = ApplyFogColor( Input.FogIntensity, Color.rgb );
	
	return Color;
};

float4 TerrainNightSM1HWPS(TERRAIN_NIGHT_INPUT_SM1_PS Input): COLOR
{
	float4 TerrainCol;
	float4 RoadCol;	
	float4 Color;
	float Shadowed;
	float4 LightMap;
	
	LightMap = tex2D(TexSamp0, Input.TerrainCoord);
	TerrainCol = tex2D(TexSamp1, Input.TerrainCoord);
	RoadCol = tex2D(TexSamp2, Input.RoadCoord);
	Shadowed = tex2Dproj(TexSamp3, Input.SM1).r;
	
	//Color.rgb = saturate(Input.Light.rgb * Shadowed + LightMap + Ambient.rgb);
	Color.rgb = Input.Light.rgb * Shadowed + LightMap.r * LightMapColor + Ambient.rgb;
	Color.rgb *= (TerrainCol.rgb * (1 - RoadCol.a) + RoadCol.rgb * RoadCol.a);
	Color.a = 1;
	
	// apply fog
	//Color.rgb = ApplyFogColor( Input.FogIntensity, Color.rgb );
	
	return Color;
};

float4 TerrainNightSM2PS(TERRAIN_NIGHT_INPUT_SM2_PS Input): COLOR
{
	float4 TerrainCol;
	float4 RoadCol;	
	float4 Color;
	float4 ShadowDepth;
	float4 Shadowed;
	float ResultShadow;
	float4 LightMap;
	
	LightMap = tex2D(TexSamp0, Input.TerrainCoord);
	TerrainCol = tex2D(TexSamp1, Input.TerrainCoord);
	RoadCol = tex2D(TexSamp2, Input.RoadCoord);
	
	ShadowDepth.r = tex2Dproj(TexSamp3, Input.SM1).r;
	ShadowDepth.g = tex2Dproj(TexSamp3, Input.SM2).r;
	Shadowed.r = (ShadowDepth.r >= Input.SM1.z);
	Shadowed.g = (ShadowDepth.g >= Input.SM2.z);
	//ResultShadow = dot(Shadowed.rg, NormalizationVector2.rg);
	//ResultShadow = Shadowed.r;
	ResultShadow = min(Shadowed.r, Shadowed.g);
	
	//Color.rgb = saturate(Input.Light.rgb * ResultShadow + LightMap + Ambient.rgb);
	Color.rgb = Input.Light.rgb * ResultShadow + LightMap.r * LightMapColor + Ambient.rgb;
	Color.rgb *= (TerrainCol.rgb * (1 - RoadCol.a) + RoadCol.rgb * RoadCol.a);
	Color.a = 1;
	
	// apply fog
	//Color.rgb = ApplyFogColor( Input.FogIntensity, Color.rgb );
	
	return Color;
};

float4 TerrainNightSM2HWPS(TERRAIN_NIGHT_INPUT_SM2_PS Input): COLOR
{
	float4 TerrainCol;
	float4 RoadCol;	
	float4 Color;
	float4 ShadowDepth;
	float4 Shadowed;
	float ResultShadow;
	float4 LightMap;
	
	LightMap = tex2D(TexSamp0, Input.TerrainCoord);
	TerrainCol = tex2D(TexSamp1, Input.TerrainCoord);
	RoadCol = tex2D(TexSamp2, Input.RoadCoord);
	
	ShadowDepth.r = tex2Dproj(TexSamp3, Input.SM1).r;
	ShadowDepth.g = tex2Dproj(TexSamp3, Input.SM2).r;
	//ResultShadow = dot(ShadowDepth.rg, NormalizationVector2.rg);
	ResultShadow = min(ShadowDepth.r, ShadowDepth.g);
	
	//Color.rgb = saturate(Input.Light.rgb * ResultShadow + LightMap + Ambient.rgb);
	Color.rgb = Input.Light.rgb * ResultShadow + LightMap.r * LightMapColor + Ambient.rgb;
	Color.rgb *= (TerrainCol.rgb * (1 - RoadCol.a) + RoadCol.rgb * RoadCol.a);
	Color.a = 1;
	
	// apply fog
	//Color.rgb = ApplyFogColor( Input.FogIntensity, Color.rgb );
	
	return Color;
};

float4 TerrainNightSM3PS(TERRAIN_NIGHT_INPUT_SM3_PS Input): COLOR
{
	float4 TerrainCol;
	float4 RoadCol;	
	float4 Color;
	float4 ShadowDepth;
	float4 Shadowed;
	float ResultShadow;
	float4 LightMap;
	
	LightMap = tex2D(TexSamp0, Input.TerrainCoord);
	TerrainCol = tex2D(TexSamp1, Input.TerrainCoord);
	RoadCol = tex2D(TexSamp2, Input.RoadCoord);
	
	ShadowDepth.r = tex2Dproj(TexSamp3, Input.SM1).r;
	ShadowDepth.g = tex2Dproj(TexSamp3, Input.SM2).r;
	ShadowDepth.b = tex2Dproj(TexSamp3, Input.SM3).r;
	Shadowed.r = (ShadowDepth.r >= Input.SM1.z);
	Shadowed.g = (ShadowDepth.g >= Input.SM2.z);
	Shadowed.b = (ShadowDepth.b >= Input.SM3.z);
	//ResultShadow = dot(Shadowed.rgb, NormalizationVector3.rgb);
	ResultShadow = min(Shadowed.r, Shadowed.g);
	ResultShadow = min(ResultShadow, Shadowed.b);
	
	//Color.rgb = saturate(Input.Light.rgb * ResultShadow + LightMap + Ambient.rgb);
	Color.rgb = Input.Light.rgb * ResultShadow + LightMap.r * LightMapColor + Ambient.rgb;
	Color.rgb *= (TerrainCol.rgb * (1 - RoadCol.a) + RoadCol.rgb * RoadCol.a);
	Color.a = 1;
	
	// apply fog
	//Color.rgb = ApplyFogColor( Input.FogIntensity, Color.rgb );
	
	return Color;
};

float4 TerrainNightSM3HWPS(TERRAIN_NIGHT_INPUT_SM3_PS Input): COLOR
{
	float4 TerrainCol;
	float4 RoadCol;	
	float4 Color;
	float4 ShadowDepth;
	float4 Shadowed;
	float ResultShadow;
	float4 LightMap;
	
	LightMap = tex2D(TexSamp0, Input.TerrainCoord);
	TerrainCol = tex2D(TexSamp1, Input.TerrainCoord);
	RoadCol = tex2D(TexSamp2, Input.RoadCoord);
	
	ShadowDepth.r = tex2Dproj(TexSamp3, Input.SM1).r;
	ShadowDepth.g = tex2Dproj(TexSamp3, Input.SM2).r;
	ShadowDepth.b = tex2Dproj(TexSamp3, Input.SM3).r;
	//ResultShadow = dot(ShadowDepth.rgb, NormalizationVector3.rgb);
	ResultShadow = min(ShadowDepth.r, ShadowDepth.g);
	ResultShadow = min(ResultShadow, ShadowDepth.b);
	
	//Color.rgb = saturate(Input.Light.rgb * ResultShadow + LightMap + Ambient.rgb);
	Color.rgb = Input.Light.rgb * ResultShadow + LightMap.r * LightMapColor + Ambient.rgb;
	Color.rgb *= (TerrainCol.rgb * (1 - RoadCol.a) + RoadCol.rgb * RoadCol.a);
	Color.a = 1;
	
	// apply fog
	//Color.rgb = ApplyFogColor( Input.FogIntensity, Color.rgb );
	
	return Color;
};

float4 TerrainNightSM4PS(TERRAIN_NIGHT_INPUT_SM4_PS Input): COLOR
{
	float4 TerrainCol;
	float4 RoadCol;	
	float4 Color;
	float4 ShadowDepth;
	float4 Shadowed;
	float ResultShadow;
	float Min1, Min2;
	float4 LightMap;
	
	LightMap = tex2D(TexSamp0, Input.TerrainCoord);
	TerrainCol = tex2D(TexSamp1, Input.TerrainCoord);
	RoadCol = tex2D(TexSamp2, Input.RoadCoord);
	
	ShadowDepth.r = tex2Dproj(TexSamp3, Input.SM1).r;
	ShadowDepth.g = tex2Dproj(TexSamp3, Input.SM2).r;
	ShadowDepth.b = tex2Dproj(TexSamp3, Input.SM3).r;
	ShadowDepth.a = tex2Dproj(TexSamp3, Input.SM4).r;
	Shadowed.r = (ShadowDepth.r >= Input.SM1.z);
	Shadowed.g = (ShadowDepth.g >= Input.SM2.z);
	Shadowed.b = (ShadowDepth.b >= Input.SM3.z);
	Shadowed.a = (ShadowDepth.a >= Input.SM4.z);
	//ResultShadow = dot(Shadowed, NormalizationVector4);
	Min1 = min(Shadowed.r, Shadowed.g);
	Min2 = min(Shadowed.b, Shadowed.a);
	ResultShadow = min(Min1, Min2);
	
	//Color.rgb = saturate(Input.Light.rgb * ResultShadow + LightMap + Ambient.rgb);
	Color.rgb = Input.Light.rgb * ResultShadow + LightMap.r * LightMapColor + Ambient.rgb;
	Color.rgb *= (TerrainCol.rgb * (1 - RoadCol.a) + RoadCol.rgb * RoadCol.a);
	Color.a = 1;
	
	// apply fog
	//Color.rgb = ApplyFogColor( Input.FogIntensity, Color.rgb );
	
	return Color;
};

float4 TerrainNightSM4HWPS(TERRAIN_NIGHT_INPUT_SM4_PS Input): COLOR
{
	float4 TerrainCol;
	float4 RoadCol;	
	float4 Color;
	float4 ShadowDepth;
	float4 Shadowed;
	float ResultShadow;
	float Min1, Min2;
	float4 LightMap;
	
	LightMap = tex2D(TexSamp0, Input.TerrainCoord);
	TerrainCol = tex2D(TexSamp1, Input.TerrainCoord);
	RoadCol = tex2D(TexSamp2, Input.RoadCoord);
	
	ShadowDepth.r = tex2Dproj(TexSamp3, Input.SM1).r;
	ShadowDepth.g = tex2Dproj(TexSamp3, Input.SM2).r;
	ShadowDepth.b = tex2Dproj(TexSamp3, Input.SM3).r;
	ShadowDepth.a = tex2Dproj(TexSamp3, Input.SM4).r;
	//ResultShadow = dot(ShadowDepth, NormalizationVector4);
	Min1 = min(ShadowDepth.r, ShadowDepth.g);
	Min2 = min(ShadowDepth.b, ShadowDepth.a);
	ResultShadow = min(Min1, Min2);
	
	//Color.rgb = saturate(Input.Light.rgb * ResultShadow + LightMap + Ambient.rgb);
	Color.rgb = Input.Light.rgb * ResultShadow + LightMap.r * LightMapColor + Ambient.rgb;
	Color.rgb *= (TerrainCol.rgb * (1 - RoadCol.a) + RoadCol.rgb * RoadCol.a);
	Color.a = 1;
	
	// apply fog
	//Color.rgb = ApplyFogColor( Input.FogIntensity, Color.rgb );
	
	return Color;
};

// Terrain with HW shadow mapping pixel shader
float4 TerrainWithHWSMPS(TERRAIN_SM_INPUT_PS Input): COLOR
{
	float4 TerrainCol;
	float4 RoadCol;	
	float4 Color;
	// float Shadow;
	float4 Shadow;
	float4 LightMap;
	
	LightMap = tex2D(TexSamp0, Input.TerrainCoord);
	TerrainCol = tex2D(TexSamp1, Input.TerrainCoord);
	RoadCol = tex2D(TexSamp2, Input.RoadCoord);
	
	// Get shadow map depth to r
	//Shadow = tex2Dproj(TexSamp3, Input.SMCoord).r;
	Input.SMCoord /= Input.SMCoord.w;
	Input.SMCoord.z = min(Input.SMCoord.z, 1.0);
	
	Shadow = tex2Dproj(TexSamp3, Input.SMCoord);
	
	//Color.rgb = saturate(Input.Light.rgb * Shadow);
	Color.rgb = LightMap.r * LightMapColor * Shadow;
	Color.rgb *= (TerrainCol.rgb * (1 - RoadCol.a) + RoadCol.rgb * RoadCol.a);
	Color.a = 1;
	
	return Color;
};

// Terrain light map shader for rendering light map onto terrain
float4 TerrainLightMapPS(TERRAIN_INPUT_PS Input): COLOR
{
	float4 TerrainCol;
	float4 RoadCol;	
	float4 Color;
	float4 LightMap;
	
	LightMap = tex2D(TexSamp0, Input.TerrainCoord);
	TerrainCol = tex2D(TexSamp1, Input.TerrainCoord);
	RoadCol = tex2D(TexSamp2, Input.RoadCoord);
	
	Color.rgb = LightMap.rgb;
	Color.rgb *= (TerrainCol.rgb * (1 - RoadCol.a) + RoadCol.rgb * RoadCol.a);
	Color.a = 1;
	
	return Color;
}


// Terrain with TSM pixel shader
float4 TerrainWithTSMPS(TERRAIN_TSM_INPUT_PS Input): COLOR
{
	float4 TerrainCol;
	float4 RoadCol;	
	float4 Color;
	float ShadowDepth;
	float3 Light;
	
	TerrainCol = tex2D(TexSamp1, Input.TerrainCoord);
	RoadCol = tex2D(TexSamp2, Input.RoadCoord);
	
	// Get shadow map depth to r
	Input.TSMCoord /= Input.TSMCoord.w;
	ShadowDepth = tex2Dproj(TexSamp3, Input.TSMCoord).r;
	
	if ( ShadowDepth >=  Input.TSMCoord.z )
	{
		Light = tex2D( TexSamp4, Input.NormalMapCoord.xy );
	}
	else Light = 0;

	Color.rgb = saturate( Light.xyz + Ambient.rgb );	
	Color.rgb *= (TerrainCol.rgb * (1 - RoadCol.a) + RoadCol.rgb * RoadCol.a);
	Color.a = 1;
	
	// apply fog
	Color.rgb = ApplyFogColor( Input.FogIntensity, Color.rgb );

	return Color;
};


// Terrain with TSM and linear PCF pixel shader
float4 TerrainWithTSMLinearPCFPS(TERRAIN_TSM_INPUT_PS Input): COLOR
{
	float4 TerrainCol;
	float4 RoadCol;	
	float4 Color;
	float4 ShadowDepth;
	float4 Shadow;
	float2 ShadowCoord;
	float2 LerpPos;
	float r1, r2;
	float Amount;
	float3 Light;
	
	TerrainCol = tex2D(TexSamp1, Input.TerrainCoord);
	RoadCol = tex2D(TexSamp2, Input.RoadCoord);
	
	// Prepare shadow coord
	Input.TSMCoord /= Input.TSMCoord.w;
	ShadowCoord = Input.TSMCoord.xy;
	
	// Get shadow map depth from four corners of pixel
	ShadowDepth.r = tex2D(TexSamp3, ShadowCoord).r;
	ShadowCoord.x += Stride;
	ShadowDepth.g = tex2D(TexSamp3, ShadowCoord).r;
	ShadowCoord.y += Stride;
	ShadowDepth.b = tex2D(TexSamp3, ShadowCoord).r;
	ShadowCoord.x -= Stride;
	ShadowDepth.a = tex2D(TexSamp3, ShadowCoord).r;
	
	// compare
	Shadow = (ShadowDepth >=  Input.TSMCoord.z);
	
	// compute lerp position
	LerpPos = frac(Input.TSMCoord.xy * (1.0f / Stride));
	// do linear PCF
	r1 = lerp(Shadow.r, Shadow.g, LerpPos.x);
	r2 = lerp(Shadow.a, Shadow.b, LerpPos.x);
	Amount = lerp(r1, r2, LerpPos.y);
	
	// normal mapping
	Light = tex2D( TexSamp4, Input.NormalMapCoord.xy );
	
	Color.rgb = saturate( Light.xyz * Amount + Ambient.rgb );
	Color.rgb *= (TerrainCol.rgb * (1 - RoadCol.a) + RoadCol.rgb * RoadCol.a);
	Color.a = 1;

	// apply fog
	Color.rgb = ApplyFogColor( Input.FogIntensity, Color.rgb );

	return Color;
};


// Terrain with HW TSM pixel shader
float4 TerrainWithHWTSMPS(TERRAIN_TSM_INPUT_PS Input): COLOR
{
	float4 TerrainCol;
	float4 RoadCol;
	float4 Color;
	float Shadow;
	float3 Light;

	TerrainCol = tex2D(TexSamp1, Input.TerrainCoord);
	RoadCol = tex2D(TexSamp2, Input.RoadCoord);
	
	// Get shadow color to r
	Input.TSMCoord /= Input.TSMCoord.w;
	Shadow = tex2Dproj(TexSamp3, Input.TSMCoord).r;

	// normal mapping
	Light = tex2D( TexSamp4, Input.NormalMapCoord.xy );
	
	Color.rgb = saturate( Light.xyz * Shadow + Ambient.rgb );	
	Color.rgb *= (TerrainCol.rgb * (1 - RoadCol.a) + RoadCol.rgb * RoadCol.a);
	Color.a = 1;
	
	// apply fog
	Color.rgb = ApplyFogColor( Input.FogIntensity, Color.rgb );

	return Color;
};





//////////////////////////////////////////////////////////////////////////////
// OBJECT TRANSFORMATION SHADERS
//////////////////////////////////////////////////////////////////////////////

// Input and output
struct OBJECT_INPUT
{
    float4 Position: POSITION;
    float3 Normal: NORMAL;
    float2 TexCoord: TEXCOORD0;
};

struct OBJECT_OUTPUT
{
	float4 Position: POSITION;
	float4 Light: COLOR;
	float2 TexCoord: TEXCOORD0;
    float  FogIntensity: TEXCOORD5; // fog intensity for vertex fog
};

struct OBJECT_NIGHT_SM1_OUTPUT
{
	float4 Position: POSITION;
	float4 Light: COLOR;
	float2 TexCoord: TEXCOORD0;
    float  FogIntensity: TEXCOORD5; // fog intensity for vertex fog
    float4 SM1: TEXCOORD3;
};

struct OBJECT_NIGHT_SM2_OUTPUT
{
	float4 Position: POSITION;
	float4 Light: COLOR;
	float2 TexCoord: TEXCOORD0;
    float  FogIntensity: TEXCOORD5; // fog intensity for vertex fog
    float4 SM1: TEXCOORD3;
    float4 SM2: TEXCOORD4;
};

struct OBJECT_NIGHT_SM3_OUTPUT
{
	float4 Position: POSITION;
	float4 Light: COLOR;
	float2 TexCoord: TEXCOORD0;
    float  FogIntensity: TEXCOORD5; // fog intensity for vertex fog
    float4 SM1: TEXCOORD3;
    float4 SM2: TEXCOORD4;
    float4 SM3: TEXCOORD6;
};

struct OBJECT_NIGHT_SM4_OUTPUT
{
	float4 Position: POSITION;
	float4 Light: COLOR;
	float2 TexCoord: TEXCOORD0;
    float  FogIntensity: TEXCOORD5; // fog intensity for vertex fog
    float4 SM1: TEXCOORD3;
    float4 SM2: TEXCOORD4;
    float4 SM3: TEXCOORD6;
    float4 SM4: TEXCOORD7;
};


struct OBJECT_SM_OUTPUT
{
	float4 Position: POSITION;
	float4 Light: COLOR;
	float4 SMCoord: TEXCOORD1;
	float2 TexCoord: TEXCOORD0;
    float  FogIntensity: TEXCOORD5; // fog intensity for vertex fog
};

struct OBJECT_TSM_OUTPUT
{
	float4 Position: POSITION;
	float4 Light: COLOR;
	float4 TSMCoord: TEXCOORD1;
	float2 TexCoord: TEXCOORD0;
	float4 LightCoord: TEXCOORD6;
    float  FogIntensity: TEXCOORD5; // fog intensity for vertex fog
};



OBJECT_OUTPUT ObjectVS(OBJECT_INPUT Input)
{
	float3 N;
	float3 V;
	float3 D;
    OBJECT_OUTPUT Output = (OBJECT_OUTPUT) 0;
    
    // compute position
    Output.Position = mul(Input.Position, AllMat);

    // compute texture coordinates
    Output.TexCoord = Input.TexCoord;

    // compute lighting
    N = normalize(mul( Input.Normal, (float3x3) WorldViewMat ) );
    V = -normalize(mul(Input.Position, (float3x3) WorldViewMat) );
    D = -normalize(mul(DirLight.Direction, (float3x3) ViewMat) );

    COLOR_PAIR Lighting = (COLOR_PAIR) 0;
    Lighting = DoDirLight(N, V, D, DirLight);

    Output.Light = DoResultLight(Lighting, Material, Ambient);
    
    // fog intensity calculation
    Output.FogIntensity = ComputeFogIntensity( Output.Position.xyz );
    
    return Output;
};


// basic object vertex shader without fog computation - used during night for far objects
OBJECT_OUTPUT ObjectNoFogVS(OBJECT_INPUT Input)
{
	float3 N;
	float3 V;
	float3 D;
    OBJECT_OUTPUT Output = (OBJECT_OUTPUT) 0;
    
    // compute position
    Output.Position = mul(Input.Position, AllMat);

    // compute texture coordinates
    Output.TexCoord = Input.TexCoord;

    // compute lighting
    N = normalize(mul( Input.Normal, (float3x3) WorldViewMat ) );
    V = -normalize(mul(Input.Position, (float3x3) WorldViewMat) );
    D = -normalize(mul(DirLight.Direction, (float3x3) ViewMat) );

    COLOR_PAIR Lighting = (COLOR_PAIR) 0;
    Lighting = DoDirLight(N, V, D, DirLight);

    Output.Light = DoResultLight(Lighting, Material, Ambient);
    
    return Output;
};


// object vertex shader without directional light (just ambient)
OBJECT_OUTPUT ObjectNoDirLightVS(OBJECT_INPUT Input)
{
	float3 N;
	float3 V;
	float3 D;
    OBJECT_OUTPUT Output = (OBJECT_OUTPUT) 0;
    
    // compute position
    Output.Position = mul(Input.Position, AllMat);

    // compute texture coordinates
    Output.TexCoord = Input.TexCoord;

    Output.Light = 0;
    
    // fog intensity calculation
    Output.FogIntensity = ComputeFogIntensity( Output.Position.xyz );
    
    return Output;
};

// Renders objects lightened by spot light in night
OBJECT_OUTPUT ObjectNightVS(OBJECT_INPUT Input)
{
	float3 N;
	float3 V;
	float3 TexCoord;
	float4 World;
	int i;
	
	OBJECT_OUTPUT Output = (OBJECT_OUTPUT) 0;
	
	// compute output position
	Output.Position = mul(Input.Position, AllMat);
	
	// texture
	Output.TexCoord = Input.TexCoord;
	
	// compute lighting
	N = normalize(mul(Input.Normal, (float3x3) WorldViewMat) );
	V = -normalize(mul(Input.Position, (float3x3) WorldViewMat) );

	COLOR_PAIR ResultLighting = (COLOR_PAIR) 0;
	COLOR_PAIR Lighting;
	for (i = 0; i < 9; i++)
	{
		Lighting = (COLOR_PAIR) 0;
		Lighting = DoSpotLight(N, V, Input.Position, SpotLights[i]);
		ResultLighting.Diffuse += Lighting.Diffuse;
		ResultLighting.Specular += Lighting.Specular;
	}
	Output.Light = DoResultLight(ResultLighting, Material, Ambient);
	
    // fog intensity calculation
    //Output.FogIntensity = ComputeFogIntensity( Output.Position.xyz );
    
	return Output;
}

OBJECT_NIGHT_SM1_OUTPUT ObjectNightSM1VS(OBJECT_INPUT Input)
{
	float3 N;
	float3 V;
	float3 TexCoord;
	float4 World;
	int i;
	
	OBJECT_NIGHT_SM1_OUTPUT Output = (OBJECT_NIGHT_SM1_OUTPUT) 0;
	
	// compute output position
	Output.Position = mul(Input.Position, AllMat);
	
	// texture
	Output.TexCoord = Input.TexCoord;
	
	// compute lighting
	N = normalize(mul(Input.Normal, (float3x3) WorldViewMat) );
	V = -normalize(mul(Input.Position, (float3x3) WorldViewMat) );

	COLOR_PAIR ResultLighting = (COLOR_PAIR) 0;
	COLOR_PAIR Lighting;
	for (i = 0; i < 9; i++)
	{
		Lighting = (COLOR_PAIR) 0;
		Lighting = DoSpotLight(N, V, Input.Position, SpotLights[i]);
		ResultLighting.Diffuse += Lighting.Diffuse;
		ResultLighting.Specular += Lighting.Specular;
	}
	Output.Light = DoResultLight(ResultLighting, Material, Ambient);
	
    // fog intensity calculation
    //Output.FogIntensity = ComputeFogIntensity( Output.Position.xyz );
    
    // shadow map coords
    World = mul(Input.Position, WorldMat);
    Output.SM1 = mul(World, ShadowMat);
    Output.SM1 /= Output.SM1.w;
    Output.SM1.xy = clamp(Output.SM1.xy, ClampVector1.xy, ClampVector1.zw);
    
	return Output;
}

OBJECT_NIGHT_SM2_OUTPUT ObjectNightSM2VS(OBJECT_INPUT Input)
{
	float3 N;
	float3 V;
	float3 TexCoord;
	float4 World;
	int i;
	
	OBJECT_NIGHT_SM2_OUTPUT Output = (OBJECT_NIGHT_SM2_OUTPUT) 0;
	
	// compute output position
	Output.Position = mul(Input.Position, AllMat);
	
	// texture
	Output.TexCoord = Input.TexCoord;
	
	// compute lighting
	N = normalize(mul(Input.Normal, (float3x3) WorldViewMat) );
	V = -normalize(mul(Input.Position, (float3x3) WorldViewMat) );

	COLOR_PAIR ResultLighting = (COLOR_PAIR) 0;
	COLOR_PAIR Lighting;
	for (i = 0; i < 9; i++)
	{
		Lighting = (COLOR_PAIR) 0;
		Lighting = DoSpotLight(N, V, Input.Position, SpotLights[i]);
		ResultLighting.Diffuse += Lighting.Diffuse;
		ResultLighting.Specular += Lighting.Specular;
	}
	Output.Light = DoResultLight(ResultLighting, Material, Ambient);
	
    // fog intensity calculation
    //Output.FogIntensity = ComputeFogIntensity( Output.Position.xyz );
    
    // shadow map coords
    World = mul(Input.Position, WorldMat);
    Output.SM1 = mul(World, ShadowMat);
    Output.SM1 /= Output.SM1.w;
    Output.SM1.xy = clamp(Output.SM1.xy, ClampVector1.xy, ClampVector1.zw);
    
    Output.SM2 = mul(World, ShadowMat2);
    Output.SM2 /= Output.SM2.w;
    Output.SM2.xy = clamp(Output.SM2.xy, ClampVector2.xy, ClampVector2.zw);
    
	return Output;
}

OBJECT_NIGHT_SM3_OUTPUT ObjectNightSM3VS(OBJECT_INPUT Input)
{
	float3 N;
	float3 V;
	float3 TexCoord;
	float4 World;
	int i;
	
	OBJECT_NIGHT_SM3_OUTPUT Output = (OBJECT_NIGHT_SM3_OUTPUT) 0;
	
	// compute output position
	Output.Position = mul(Input.Position, AllMat);
	
	// texture
	Output.TexCoord = Input.TexCoord;
	
	// compute lighting
	N = normalize(mul(Input.Normal, (float3x3) WorldViewMat) );
	V = -normalize(mul(Input.Position, (float3x3) WorldViewMat) );

	COLOR_PAIR ResultLighting = (COLOR_PAIR) 0;
	COLOR_PAIR Lighting;
	for (i = 0; i < 9; i++)
	{
		Lighting = (COLOR_PAIR) 0;
		Lighting = DoSpotLight(N, V, Input.Position, SpotLights[i]);
		ResultLighting.Diffuse += Lighting.Diffuse;
		ResultLighting.Specular += Lighting.Specular;
	}
	Output.Light = DoResultLight(ResultLighting, Material, Ambient);
	
    // fog intensity calculation
    //Output.FogIntensity = ComputeFogIntensity( Output.Position.xyz );
    
    // shadow map coords
    World = mul(Input.Position, WorldMat);
    Output.SM1 = mul(World, ShadowMat);
    Output.SM1 /= Output.SM1.w;
    Output.SM1.xy = clamp(Output.SM1.xy, ClampVector1.xy, ClampVector1.zw);
    
    Output.SM2 = mul(World, ShadowMat2);
    Output.SM2 /= Output.SM2.w;
    Output.SM2.xy = clamp(Output.SM2.xy, ClampVector2.xy, ClampVector2.zw);
    
    Output.SM3 = mul(World, ShadowMat3);
    Output.SM3 /= Output.SM3.w;
    Output.SM3.xy = clamp(Output.SM3.xy, ClampVector3.xy, ClampVector3.zw);
    
	return Output;
}

OBJECT_NIGHT_SM4_OUTPUT ObjectNightSM4VS(OBJECT_INPUT Input)
{
	float3 N;
	float3 V;
	float3 TexCoord;
	float4 World;
	int i;
	
	OBJECT_NIGHT_SM4_OUTPUT Output = (OBJECT_NIGHT_SM4_OUTPUT) 0;
	
	// compute output position
	Output.Position = mul(Input.Position, AllMat);
	
	// texture
	Output.TexCoord = Input.TexCoord;
	
	// compute lighting
	N = normalize(mul(Input.Normal, (float3x3) WorldViewMat) );
	V = -normalize(mul(Input.Position, (float3x3) WorldViewMat) );

	COLOR_PAIR ResultLighting = (COLOR_PAIR) 0;
	COLOR_PAIR Lighting;
	for (i = 0; i < 9; i++)
	{
		Lighting = (COLOR_PAIR) 0;
		Lighting = DoSpotLight(N, V, Input.Position, SpotLights[i]);
		ResultLighting.Diffuse += Lighting.Diffuse;
		ResultLighting.Specular += Lighting.Specular;
	}
	Output.Light = DoResultLight(ResultLighting, Material, Ambient);
	
    // fog intensity calculation
    //Output.FogIntensity = ComputeFogIntensity( Output.Position.xyz );
    
    // shadow map coords
    World = mul(Input.Position, WorldMat);
    Output.SM1 = mul(World, ShadowMat);
    Output.SM1 /= Output.SM1.w;
    Output.SM1.xy = clamp(Output.SM1.xy, ClampVector1.xy, ClampVector1.zw);
    
    Output.SM2 = mul(World, ShadowMat2);
    Output.SM2 /= Output.SM2.w;
    Output.SM2.xy = clamp(Output.SM2.xy, ClampVector2.xy, ClampVector2.zw);
    
    Output.SM3 = mul(World, ShadowMat3);
    Output.SM3 /= Output.SM3.w;
    Output.SM3.xy = clamp(Output.SM3.xy, ClampVector3.xy, ClampVector3.zw);
    
    Output.SM4 = mul(World, ShadowMat4);
    Output.SM4 /= Output.SM4.w;
    Output.SM4.xy = clamp(Output.SM4.xy, ClampVector4.xy, ClampVector4.zw);
    
	return Output;
}

// Renders object lightened by spot light
OBJECT_OUTPUT ObjectSpotLightVS(OBJECT_INPUT Input)
{
	float3 N;
	float3 V;
	float3 TexCoord;
	float4 World;
	
	OBJECT_OUTPUT Output = (OBJECT_OUTPUT) 0;
	
	// compute output position
	Output.Position = mul(Input.Position, AllMat);
	
	// texture
	Output.TexCoord = Input.TexCoord;
	
	// compute lighting
	N = normalize(mul(Input.Normal, (float3x3) WorldViewMat) );
	V = -normalize(mul(Input.Position, (float3x3) WorldViewMat) );
	
	COLOR_PAIR Lighting = (COLOR_PAIR) 0;
	Lighting = DoSpotLight(N, V, Input.Position, SpotLight);
	
	Output.Light = DoResultLight(Lighting, Material, Ambient);	
	
    // fog intensity calculation
    Output.FogIntensity = ComputeFogIntensity( Output.Position.xyz );
    
	return Output;
};


// Renders object with shadow map for spot light
OBJECT_SM_OUTPUT ObjectSpotLightWithSMVS(OBJECT_INPUT Input)
{
	float3 N;
	float3 V;
	float3 TexCoord;
	float4 World;
	float4 SMCoord;
	
	OBJECT_SM_OUTPUT Output = (OBJECT_SM_OUTPUT) 0;
	
	// compute output position
	Output.Position = mul(Input.Position, AllMat);
	
	// texture
	Output.TexCoord = Input.TexCoord;
	
	// compute shadow map texture coords
	World = mul(Input.Position, WorldMat);
	SMCoord = mul(World, ShadowMat);
	//Output.SMCoord = SMCoord / SMCoord.w;
	//Output.SMCoord = SMCoord / SMCoord.w;
	//Output.SMCoord.xy = clamp(Output.SMCoord.xy, ClampVector1.xy, ClampVector1.zw);
	Output.SMCoord = SMCoord;
	
	// compute lighting
	N = normalize(mul(Input.Normal, (float3x3) WorldViewMat) );
	V = -normalize(mul(Input.Position, (float3x3) WorldViewMat) );
	
	COLOR_PAIR Lighting = (COLOR_PAIR) 0;
	Lighting = DoSpotLight(N, V, Input.Position, SpotLight);
	
	Output.Light = DoResultLight(Lighting, Material, Ambient);
	
    // fog intensity calculation
    Output.FogIntensity = ComputeFogIntensity( Output.Position.xyz );
    
	return Output;
};


OBJECT_TSM_OUTPUT ObjectWithTSMVS(OBJECT_INPUT Input)
{
	float3 N;
	float3 V;
	float3 D;
	float4 World;
	float4 LightCoord;
    OBJECT_TSM_OUTPUT Output = (OBJECT_TSM_OUTPUT) 0;
    
    // compute position
    Output.Position = mul(Input.Position, AllMat);

    // compute texture coordinates
    Output.TexCoord = Input.TexCoord;
    
    // compute texture coordinates for TSM
    World = mul(Input.Position, WorldMat);
    LightCoord = mul(World, ShadowMat);
    Output.TSMCoord = mul(LightCoord, PostProjMat);
    //Output.TSMCoord /= Output.TSMCoord.w;
    Output.TSMCoord.z = LightCoord.z * Output.TSMCoord.w;
    Output.LightCoord = LightCoord;
    
    //if (Output.TSMCoord.y < -1.0 || Output.TSMCoord.y > 1.0 || Output.TSMCoord.x < -1.0 || Output.TSMCoord.x > 1.0)
	//	Output.TSMCoord.z = 10;
    
    // compute lighting
    N = normalize(mul( Input.Normal, (float3x3) WorldViewMat ) );
    V = -normalize(mul(Input.Position, (float3x3) WorldViewMat) );
    D = -normalize(mul(DirLight.Direction, (float3x3) ViewMat) );

    COLOR_PAIR Lighting = (COLOR_PAIR) 0;
    Lighting = DoDirLight(N, V, D, DirLight);

    Output.Light = DoResultLight(Lighting, Material, Ambient);
    
    // fog intensity calculation
    Output.FogIntensity = ComputeFogIntensity( Output.Position.xyz );
    
    //Output.Position = Output.TSMCoord;
    
    return Output;
};




/////////////////////////////////////////////////////////////////////
// OBJECT PIXEL SHADERS
/////////////////////////////////////////////////////////////////////

// Input and output
struct OBJECT_INPUT_PS 
{
	float4 Light: COLOR;
	float2 TexCoord: TEXCOORD0;
    float  FogIntensity: TEXCOORD5; // fog intensity for vertex fog
};

struct OBJECT_NIGHT_SM1_PS
{
	float4 Light: COLOR;
	float2 TexCoord: TEXCOORD0;
    float  FogIntensity: TEXCOORD5; // fog intensity for vertex fog
	float4 SM1: TEXCOORD3;
};

struct OBJECT_NIGHT_SM2_PS
{
	float4 Light: COLOR;
	float2 TexCoord: TEXCOORD0;
    float  FogIntensity: TEXCOORD5; // fog intensity for vertex fog
	float4 SM1: TEXCOORD3;
	float4 SM2: TEXCOORD4;
};

struct OBJECT_NIGHT_SM3_PS
{
	float4 Light: COLOR;
	float2 TexCoord: TEXCOORD0;
    float  FogIntensity: TEXCOORD5; // fog intensity for vertex fog
	float4 SM1: TEXCOORD3;
	float4 SM2: TEXCOORD4;
	float4 SM3: TEXCOORD6;
};

struct OBJECT_NIGHT_SM4_PS
{
	float4 Light: COLOR;
	float2 TexCoord: TEXCOORD0;
    float  FogIntensity: TEXCOORD5; // fog intensity for vertex fog
	float4 SM1: TEXCOORD3;
	float4 SM2: TEXCOORD4;
	float4 SM3: TEXCOORD6;
	float4 SM4: TEXCOORD7;
};

struct OBJECT_SM_INPUT_PS
{
	float4 Light: COLOR;
	float4 SMCoord: TEXCOORD1;
	float2 TexCoord: TEXCOORD0;
    float  FogIntensity: TEXCOORD5; // fog intensity for vertex fog
};

struct OBJECT_TSM_INPUT_PS
{
	float4 Light: COLOR;
	float4 TSMCoord: TEXCOORD1;
	float2 TexCoord: TEXCOORD0;
	float4 LightCoord: TEXCOORD6;
    float  FogIntensity: TEXCOORD5; // fog intensity for vertex fog
};



// Basic object pixel shader
float4 ObjectPS(OBJECT_INPUT_PS Input) : COLOR
{
	float4 TexCol;
	float4 Color;
	
	TexCol = tex2D(TexSamp1, Input.TexCoord);
	
	Color.rgb = saturate(Input.Light.rgb + Ambient.rgb) * TexCol.rgb;
	Color.a = TexCol.a;
	
	// clip almost transparent pixels
	Color.a -= 0.01;
	clip( Color.a );

	// apply fog
	Color.rgb = ApplyFogColor( Input.FogIntensity, Color.rgb );

	return Color;
};

// Basic object pixel shader used during night - doesn't apply fog
float4 ObjectNightPS(OBJECT_INPUT_PS Input) : COLOR
{
	float4 TexCol;
	float4 Color;
	
	TexCol = tex2D(TexSamp1, Input.TexCoord);
	
	Color.rgb = saturate(Input.Light.rgb + Ambient.rgb) * TexCol.rgb;
	Color.a = TexCol.a;
	
	// clip almost transparent pixels
	Color.a -= 0.01;
	clip( Color.a );

	// apply fog
	//Color.rgb = ApplyFogColor( Input.FogIntensity, Color.rgb );

	return Color;
};


float4 ObjectNightSM1PS(OBJECT_NIGHT_SM1_PS Input) : COLOR
{
	float4 TexCol;
	float4 Color;
	float4 ShadowDepth;
	float Shadowed;
	
	TexCol = tex2D(TexSamp1, Input.TexCoord);
	ShadowDepth.r = tex2D(TexSamp3, Input.SM1.xy).r;
	Shadowed = (ShadowDepth.r >= Input.SM1.z);
	
	Color.rgb = saturate(Input.Light.rgb * Shadowed + Ambient.rgb) * TexCol.rgb;
	Color.a = TexCol.a;
	
	// clip almost transparent pixels
	Color.a -= 0.01;
	clip( Color.a );

	// apply fog
	//Color.rgb = ApplyFogColor( Input.FogIntensity, Color.rgb );

	return Color;
};

float4 ObjectNightHWSM1PS(OBJECT_NIGHT_SM1_PS Input) : COLOR
{
	float4 TexCol;
	float4 Color;
	float Shadowed;
	
	TexCol = tex2D(TexSamp1, Input.TexCoord);
	Shadowed = tex2D(TexSamp3, Input.SM1.xy).r;
	
	Color.rgb = saturate(Input.Light.rgb * Shadowed + Ambient.rgb) * TexCol.rgb;
	Color.a = TexCol.a;
	
	// clip almost transparent pixels
	Color.a -= 0.01;
	clip( Color.a );

	// apply fog
	//Color.rgb = ApplyFogColor( Input.FogIntensity, Color.rgb );

	return Color;
};

float4 ObjectNightSM2PS(OBJECT_NIGHT_SM2_PS Input) : COLOR
{
	float4 TexCol;
	float4 Color;
	float4 ShadowDepth;
	float4 Shadowed;
	float ResultShadow;
	
	TexCol = tex2D(TexSamp1, Input.TexCoord);
	
	Input.SM1.z = min(1.0, Input.SM1.z);
	Input.SM2.z = min(1.0, Input.SM2.z);
	ShadowDepth.r = tex2D(TexSamp3, Input.SM1.xy).r;
	ShadowDepth.g = tex2D(TexSamp3, Input.SM2.xy).r;
	Shadowed.r = (ShadowDepth.r >= Input.SM1.z);
	Shadowed.g = (ShadowDepth.g >= Input.SM2.z);
	
	ResultShadow = dot(Shadowed.rg, NormalizationVector2);
	
	Color.rgb = saturate(Input.Light.rgb * ResultShadow + Ambient.rgb) * TexCol.rgb;
	Color.a = TexCol.a;
	
	// clip almost transparent pixels
	Color.a -= 0.01;
	clip( Color.a );

	// apply fog
	//Color.rgb = ApplyFogColor( Input.FogIntensity, Color.rgb );

	return Color;
};

float4 ObjectNightHWSM2PS(OBJECT_NIGHT_SM2_PS Input) : COLOR
{
	float4 TexCol;
	float4 Color;
	float4 Shadowed;
	float ResultShadow;
	
	TexCol = tex2D(TexSamp1, Input.TexCoord);
	Input.SM1.z = min(1.0, Input.SM1.z);
	Input.SM2.z = min(1.0, Input.SM2.z);
	Shadowed.r = tex2D(TexSamp3, Input.SM1.xy).r;
	Shadowed.g = tex2D(TexSamp3, Input.SM2.xy).r;
	
	ResultShadow = dot(Shadowed.rg, NormalizationVector2);
	
	Color.rgb = saturate(Input.Light.rgb * ResultShadow + Ambient.rgb) * TexCol.rgb;
	Color.a = TexCol.a;
	
	// clip almost transparent pixels
	Color.a -= 0.01;
	clip( Color.a );

	// apply fog
	//Color.rgb = ApplyFogColor( Input.FogIntensity, Color.rgb );

	return Color;
};

float4 ObjectNightSM3PS(OBJECT_NIGHT_SM3_PS Input) : COLOR
{
	float4 TexCol;
	float4 Color;
	float4 ShadowDepth;
	float4 Shadowed;
	float ResultShadow;
	
	TexCol = tex2D(TexSamp1, Input.TexCoord);
	Input.SM1.z = min(1.0, Input.SM1.z);
	Input.SM2.z = min(1.0, Input.SM2.z);
	Input.SM3.z = min(1.0, Input.SM3.z);
	ShadowDepth.r = tex2D(TexSamp3, Input.SM1.xy).r;
	ShadowDepth.g = tex2D(TexSamp3, Input.SM2.xy).r;
	ShadowDepth.b = tex2D(TexSamp3, Input.SM3.xy).r;
	Shadowed.r = (ShadowDepth.r >= Input.SM1.z);
	Shadowed.g = (ShadowDepth.g >= Input.SM2.z);
	Shadowed.b = (ShadowDepth.b >= Input.SM3.z);
	
	ResultShadow = dot(Shadowed.rgb, NormalizationVector3);
	
	Color.rgb = saturate(Input.Light.rgb * ResultShadow + Ambient.rgb) * TexCol.rgb;
	Color.a = TexCol.a;
	
	// clip almost transparent pixels
	Color.a -= 0.01;
	clip( Color.a );

	// apply fog
	//Color.rgb = ApplyFogColor( Input.FogIntensity, Color.rgb );

	return Color;
};

float4 ObjectNightHWSM3PS(OBJECT_NIGHT_SM3_PS Input) : COLOR
{
	float4 TexCol;
	float4 Color;
	float4 Shadowed;
	float ResultShadow;
	
	TexCol = tex2D(TexSamp1, Input.TexCoord);
	Input.SM1.z = min(1.0, Input.SM1.z);
	Input.SM2.z = min(1.0, Input.SM2.z);
	Input.SM3.z = min(1.0, Input.SM3.z);
	Shadowed.r = tex2D(TexSamp3, Input.SM1.xy).r;
	Shadowed.g = tex2D(TexSamp3, Input.SM2.xy).r;
	Shadowed.b = tex2D(TexSamp3, Input.SM3.xy).r;
	
	ResultShadow = dot(Shadowed.rgb, NormalizationVector3);
	
	Color.rgb = saturate(Input.Light.rgb * ResultShadow + Ambient.rgb) * TexCol.rgb;
	Color.a = TexCol.a;
	
	// clip almost transparent pixels
	Color.a -= 0.01;
	clip( Color.a );

	// apply fog
	//Color.rgb = ApplyFogColor( Input.FogIntensity, Color.rgb );

	return Color;
};

float4 ObjectNightSM4PS(OBJECT_NIGHT_SM4_PS Input) : COLOR
{
	float4 TexCol;
	float4 Color;
	float4 ShadowDepth;
	float4 Shadowed;
	float ResultShadow;
	
	TexCol = tex2D(TexSamp1, Input.TexCoord);
	Input.SM1.z = min(1.0, Input.SM1.z);
	Input.SM2.z = min(1.0, Input.SM2.z);
	Input.SM3.z = min(1.0, Input.SM3.z);
	Input.SM4.z = min(1.0, Input.SM4.z);
	ShadowDepth.r = tex2D(TexSamp3, Input.SM1.xy).r;
	ShadowDepth.g = tex2D(TexSamp3, Input.SM2.xy).r;
	ShadowDepth.b = tex2D(TexSamp3, Input.SM3.xy).r;
	ShadowDepth.a = tex2D(TexSamp3, Input.SM4.xy).r;
	Shadowed.r = (ShadowDepth.r >= Input.SM1.z);
	Shadowed.g = (ShadowDepth.g >= Input.SM2.z);
	Shadowed.b = (ShadowDepth.b >= Input.SM3.z);
	Shadowed.a = (ShadowDepth.a >= Input.SM4.z);
	
	ResultShadow = dot(Shadowed, NormalizationVector4);
	
	Color.rgb = saturate(Input.Light.rgb * ResultShadow + Ambient.rgb) * TexCol.rgb;
	Color.a = TexCol.a;
	
	// clip almost transparent pixels
	Color.a -= 0.01;
	clip( Color.a );

	// apply fog
	//Color.rgb = ApplyFogColor( Input.FogIntensity, Color.rgb );

	return Color;
};

float4 ObjectNightHWSM4PS(OBJECT_NIGHT_SM4_PS Input) : COLOR
{
	float4 TexCol;
	float4 Color;
	float4 Shadowed;
	float ResultShadow;
	
	TexCol = tex2D(TexSamp1, Input.TexCoord);
	Input.SM1.z = min(1.0, Input.SM1.z);
	Input.SM2.z = min(1.0, Input.SM2.z);
	Input.SM3.z = min(1.0, Input.SM3.z);
	Input.SM4.z = min(1.0, Input.SM4.z);
	Shadowed.r = tex2D(TexSamp3, Input.SM1.xy).r;
	Shadowed.g = tex2D(TexSamp3, Input.SM2.xy).r;
	Shadowed.b = tex2D(TexSamp3, Input.SM3.xy).r;
	Shadowed.a = tex2D(TexSamp3, Input.SM4.xy).r;
	
	ResultShadow = dot(Shadowed, NormalizationVector4);
	
	Color.rgb = saturate(Input.Light.rgb * ResultShadow + Ambient.rgb) * TexCol.rgb;
	Color.a = TexCol.a;
	
	// apply fog
	//Color.rgb = ApplyFogColor( Input.FogIntensity, Color.rgb );

	return Color;
};

float2 Jitter1 = {-0.000775525, -0.000745871};
float2 Jitter2 = {0.000154366, -0.000545861};
float2 Jitter3 = {0.000668442, -0.000523628};
float2 Jitter4 = {-0.00123327, 0.000159164};
float2 Jitter5 = {0.000399348, -0.000178894};
float2 Jitter6 = {0.000495255, 0.000137706};
float2 Jitter7 = {-0.000698871, 0.000723131};
float2 Jitter8 = {0.000147094, 0.000755408};
float2 Jitter9 = {0.00124817, 0.00131344};
float3 Weight3 = {0.111f, 0.111f, 0.111f};


// Object with shadow mapping pixel shader
float4 ObjectWithSMPS(OBJECT_SM_INPUT_PS Input) : COLOR
{
	float4 TexCol;
	float4 Color;
	float ShadowDepth;
	float Shadow;
	
	TexCol = tex2D(TexSamp1, Input.TexCoord);
	
	// Get shadow map depth to r
	ShadowDepth = tex2Dproj(TexSamp3, Input.SMCoord).r;
	
	// compare
	Shadow = (ShadowDepth >=  Input.SMCoord.z / Input.SMCoord.w);
	
	Color.rgb = saturate(Input.Light.rgb * Shadow) * TexCol.rgb;
	Color.a = TexCol.a;
	
	// clip almost transparent pixels
	Color.a -= 0.01;
	clip( Color.a );

	return Color;
	/*
	
	float3 u1 = {1,1,1}; // vectors for aquiring colors from texture
	
	float Amount;
	float4 Color;
	float3 CompRes;
	float4 TexCol;
	
	// Get standard texture
	TexCol = tex2D(TexSamp1, Input.TexCoord);
	
	Input.SMCoord /= Input.SMCoord.w;
	
	// extract texture colors from surrouding texture pixels
	u1.r = tex2D(TexSamp3, Input.SMCoord.xy + Jitter1).r;
	u1.g = tex2D(TexSamp3, Input.SMCoord.xy + Jitter2).g;
	u1.b = tex2D(TexSamp3, Input.SMCoord.xy + Jitter3).b; 
	
	// Compare
	CompRes = (Input.SMCoord.z <= u1);
	
	Amount = dot(CompRes, Weight3);
	
	// extract texture colors from surrouding texture pixels
	u1.r = tex2D(TexSamp3, Input.SMCoord.xy + Jitter4).r;
	u1.g = tex2D(TexSamp3, Input.SMCoord.xy + Jitter5).g;
	u1.b = tex2D(TexSamp3, Input.SMCoord.xy + Jitter6).b; 
	
	// Compare
	CompRes = (Input.SMCoord.z <= u1);
	
	Amount += dot(CompRes, Weight3);
	
	// extract texture colors from surrouding texture pixels
	u1.r = tex2D(TexSamp3, Input.SMCoord.xy + Jitter7).r;
	u1.g = tex2D(TexSamp3, Input.SMCoord.xy + Jitter8).g;
	u1.b = tex2D(TexSamp3, Input.SMCoord.xy + Jitter9).b; 
		
	// Compare
	CompRes = (Input.SMCoord.z <= u1);
	
	Amount += dot(CompRes, Weight3);
	//if (Amount > 0) Amount = 1;
	
	Color.rgb = saturate(Input.Light.rgb * Amount) * TexCol.rgb;
	Color.a = TexCol.a;
	
	return Color;
	*/
};


// Object with HW shadow mapping pixel shader
float4 ObjectWithHWSMPS(OBJECT_SM_INPUT_PS Input) : COLOR
{
	float4 TexCol;
	float4 Color;
	float Shadow;
	
	TexCol = tex2D(TexSamp1, Input.TexCoord);
	
	// Get shadow map depth to r
	Shadow = tex2Dproj(TexSamp3, Input.SMCoord);
	
	Color.rgb = saturate(Input.Light.rgb * Shadow) * TexCol.rgb;
	Color.a = TexCol.a;
	
	// clip almost transparent pixels
	Color.a -= 0.01;
	clip( Color.a );

	return Color;
};


// Object with TSM pixel shader
float4 ObjectWithTSMPS(OBJECT_TSM_INPUT_PS Input) : COLOR
{
	float4 TexCol;
	float4 Color;
	float ShadowDepth;
	float Shadow;
	
	TexCol = tex2D(TexSamp1, Input.TexCoord);
	
	// Get shadow map depth to r
	Input.TSMCoord /= Input.TSMCoord.w;
	ShadowDepth = tex2Dproj(TexSamp3, Input.TSMCoord).r;
	
	// compare
	Shadow = (ShadowDepth >=  Input.LightCoord.z);

	Color.rgb = saturate(Input.Light.rgb * Shadow + Ambient.rgb) * TexCol.rgb;
	Color.a = TexCol.a;
	
	// clip almost transparent pixels
	Color.a -= 0.01;
	clip( Color.a );

	// apply fog
	Color.rgb = ApplyFogColor( Input.FogIntensity, Color.rgb );

	return Color;
};


// Object with TSM with linear PCF pixel shader
float4 ObjectWithTSMLinearPCFPS(OBJECT_TSM_INPUT_PS Input) : COLOR
{
	float4 TexCol;
	float4 Color;
	float4 ShadowDepth;
	float4 Shadow;
	float2 ShadowCoord;
	float2 LerpPos;
	float r1, r2;
	float Amount;
	
	TexCol = tex2D(TexSamp1, Input.TexCoord);
	
	// Prepare shadow coord
	Input.TSMCoord /= Input.TSMCoord.w;
	ShadowCoord = Input.TSMCoord.xy;
	
	// Get shadow map depth from four corners of pixel
	ShadowDepth.r = tex2D(TexSamp3, ShadowCoord).r;
	ShadowCoord.x += Stride;
	ShadowDepth.g = tex2D(TexSamp3, ShadowCoord).r;
	ShadowCoord.y += Stride;
	ShadowDepth.b = tex2D(TexSamp3, ShadowCoord).r;
	ShadowCoord.x -= Stride;
	ShadowDepth.a = tex2D(TexSamp3, ShadowCoord).r;
	
	// compare
	Shadow = (ShadowDepth >=  Input.LightCoord.z);
	
	// compute lerp position
	LerpPos = frac(Input.TSMCoord.xy * (1.0f / Stride));
	// do linear PCF
	r1 = lerp(Shadow.r, Shadow.g, LerpPos.x);
	r2 = lerp(Shadow.a, Shadow.b, LerpPos.x);
	Amount = lerp(r1, r2, LerpPos.y);
	
	Color.rgb = saturate(Input.Light.rgb * Amount + Ambient.rgb) * TexCol.rgb;
	Color.a = TexCol.a;
	
	// clip almost transparent pixels
	Color.a -= 0.01;
	clip( Color.a );

	// apply fog
	Color.rgb = ApplyFogColor( Input.FogIntensity, Color.rgb );

	return Color;
};


// Object with HW TSM pixel shader
float4 ObjectWithHWTSMPS(OBJECT_TSM_INPUT_PS Input) : COLOR
{
	float4 TexCol;
	float4 Color;
	float Shadow;
	
	TexCol = tex2D(TexSamp1, Input.TexCoord);
	
	// Get shadow map color to r
	Input.TSMCoord /= Input.TSMCoord.w;
	Input.TSMCoord.z = Input.LightCoord.z;
	Shadow = tex2Dproj(TexSamp3, Input.TSMCoord).r;
	
	Color.rgb = saturate(Input.Light.rgb * Shadow + Ambient.rgb) * TexCol.rgb;
	Color.a = TexCol.a;
	
	// clip almost transparent pixels
	Color.a -= 0.01;
	clip( Color.a );

	// apply fog
	Color.rgb = ApplyFogColor( Input.FogIntensity, Color.rgb );

	return Color;
};













/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
////   R E S T R U C T U R E D    S H A D E R S
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////
// DEPTH SHADERS
/////////////////////////////////////////////////////////////////////

struct DEPTH_INPUT
{
	float4 Position: POSITION;
    float2 TextureCoord: TEXCOORD0;
};

struct DEPTH_OUTPUT
{
	float4 Position: POSITION;
	float4 TexCoord: TEXCOORD0;
    float2 TextureCoord: TEXCOORD1;
};


// Depth vertex shader
DEPTH_OUTPUT DepthVS (DEPTH_INPUT Input)
{
	DEPTH_OUTPUT Output = (DEPTH_OUTPUT) 0;

	// Transform to projection space
	Output.Position = mul(Input.Position, AllMat);
	
	// Set texcoord to position
	Output.TexCoord = Output.Position;
	Output.TexCoord /= Output.TexCoord.w;
	
    // copy texture coordinates
    Output.TextureCoord = Input.TextureCoord;

	return Output;
}


// TSM depth shader - addresses z value problem 
DEPTH_OUTPUT TSMDepthVS(DEPTH_INPUT Input)
{
	DEPTH_OUTPUT Output;
	
	// Texture coordinates are coordinates in light projection space
	Output.TexCoord = mul(Input.Position, AllMat);
	
	// Position is in post-projection TSM space
	Output.Position = mul(Output.TexCoord, PostProjMat);
	//Output.Position /= Output.Position.w;
	Output.Position.z = Output.TexCoord.z * Output.Position.w;
	
    // copy texture coordinates
    Output.TextureCoord = Input.TextureCoord;

	return Output;
};


// Depth pixel shader
float4 DepthPS( float4 TexCoord: TEXCOORD0, float2 TextureCoord: TEXCOORD1 ): COLOR
{
	float4 Color = 0;
	Color.r = TexCoord.z;
	
	// perform transparency dependent clipping
	float4 TexCol = tex2D( TexSamp1, TextureCoord );
	//Color.a = TexCol.a - 0.01;
	//clip( Color.a );

	return Color;
}


// TSM Depth pixel shader
float4 TSMDepthPS( float4 TexCoord: TEXCOORD0, float2 TextureCoord: TEXCOORD1, out float Depth: DEPTH) : COLOR
{
	float4 Color = (float4) 0;
	
	// Output depth of scene from projection light space
	Color.r = TexCoord.z / TexCoord.w;
	// Also replace depth in depth buffer
	Depth = Color.r;

	// perform transparency dependent clipping
	float4 TexCol = tex2D( TexSamp1, TextureCoord );
	Color.a = TexCol.a - 0.01;
	clip( Color.a );

	return Color;
};




/////////////////////////////////////////////////////////////////////
// SHADOW VOLUME SHADERS
/////////////////////////////////////////////////////////////////////

struct SV_COVERPOLY_INPUT_PS
{
	float2 ScreenCoord: TEXCOORD0;
};



float4 SVIncreaseCounterPS() : COLOR0
{
	return SVIncrease;
}


float4 SVCoveringPolygonPS( SV_COVERPOLY_INPUT_PS Input ) : COLOR0
{
	float4		Color;
	float4		Shadow;
	
	Shadow = tex2D( TexSamp3, Input.ScreenCoord );

	if ( Shadow.r > 0.495 ) Color.rgb = Ambient.rgb;
	else Color.rgb = 0;
	Color.a = 1;
	
	return Color;
}




/////////////////////////////////////////////////////////////////////
// GRASS SHADERS
/////////////////////////////////////////////////////////////////////

// Input and output structs
struct GRASS_INPUT
{
    float4 Position: POSITION;
    float2 TexCoord: TEXCOORD0; // texture coordinates
    float3 RotationSegment: TEXCOORD1; // RotationY in the first texcoord property, Segment <0,1> in the second property
    float3 RPCoord: TEXCOORD2; // relative point oordinates in the map field (plate)
};

struct GRASS_OUTPUT
{
	float4 Position: POSITION;
	float4 Light: COLOR;
    float2 TexCoord: TEXCOORD0; // texture coordinates
    float  FogIntensity: TEXCOORD5; // fog intensity for vertex fog
};

struct GRASS_INPUT_PS 
{
	float4 Light: COLOR;
	float2 TexCoord: TEXCOORD0;
    float  FogIntensity: TEXCOORD5; // fog intensity for vertex fog
};



// grass vertex shader - performs more complex computations on each vertex
GRASS_OUTPUT GrassVS( GRASS_INPUT Input )
{
    float	PlatePos, Depth;
    float	Sinus, Cosinus;
    float4	Pos, NewPos, RPCoord;
	float3	N, V, D;
    float3	Normal = { 0, 1, 0 };
    float3x3	RotMat;
 	float	NdotL;
 	GRASS_OUTPUT	Output = (GRASS_OUTPUT) 0;
    

    // TRANSFORM VERTEX LOCALLY
    
    // Compute rotation around Y axis
    Sinus = Input.RotationSegment.x;
    Cosinus = Input.RotationSegment.y;
    RotMat._m00 = Cosinus;
    RotMat._m02 = -Sinus;
    RotMat._m20 = Sinus;
    RotMat._m22 = Cosinus;
    RotMat._m11 = 1;
    RotMat._m01 = RotMat._m10 = RotMat._m21 = RotMat._m12 = 0;
    
	NewPos.xyz = mul( Input.Position.xyz, RotMat );


    // Compute deviation
	PlatePos = 4000 * (Input.RPCoord.x / 1000);
	GrassTime = fmod( GrassTime + PlatePos, 4000 );
	GrassTime = (GrassTime - 2000) / 2000; // <-1,1>
	GrassTime = ( abs(GrassTime) * 2 - 1 ) * 0.5 + 0.4;
    Sinus = sin( GrassTime ); 
    Cosinus = cos( GrassTime );
    RotMat._m00 = Cosinus;
    RotMat._m01 = Sinus;
    RotMat._m10 = -Sinus;
    RotMat._m11 = Cosinus;
    RotMat._m22 = 1;
    RotMat._m20 = RotMat._m02 = RotMat._m21 = RotMat._m12 = 0;
    
	Pos.xyz = mul( NewPos.xyz, (float3x3) RotMat );
	
    NewPos.zyx = lerp( NewPos.zyx, Pos.zyx, Input.RotationSegment.z );
    
	Pos.w = Input.Position.w;


    
    // COMPUTE VERTEX WORLD POSITION
    Pos.xyz = NewPos.xyz + Input.RPCoord;
    Pos.xz = Pos.xz - 500;
    Output.Position = mul( Pos, AllMat );
    

    // save texture coordinates
    Output.TexCoord = Input.TexCoord;

    
    
    // COMPUTE LIGHTING
	NdotL = dot( Normal, -DirLight.Direction );
	// illumine all grass under the same circrumstances; 1.15 multiplier is actually a magic constant obtained experimentally 
	if ( NdotL > 0.f ) Output.Light.xyz = saturate( 1.15 * NdotL * DirLight.Diffuse ); 


    // COMPUTE OPACITY (depth dependent)
    RPCoord.xyz = Input.RPCoord.xyz;
    RPCoord.w = 1;
    RPCoord = mul( RPCoord, WorldMat ); // get world position of the object
    Pos.xy = GrassViewerPos.xz - RPCoord.xz;
    Output.Light.a = 1 - saturate( sqrt( Pos.x * Pos.x + Pos.y * Pos.y ) / GrassMaxDepth - 0.0 );
    

    
    // fog intensity calculation
    Output.FogIntensity = ComputeFogIntensity( Output.Position.xyz );
    
    return Output;
};


// Grass pixel shader - just maps a texture on the geometry surface and applies opacity
float4 GrassPS( GRASS_INPUT_PS Input ) : COLOR0
{
	float4 TexCol;
	float4 Color;
	
	TexCol = tex2D( TexSamp1, Input.TexCoord );
	
	Color.rgb = saturate( Input.Light.rgb + Ambient.rgb ) * TexCol.rgb;
	Color.a = TexCol.a * Input.Light.a;
	
	Color.a -= 0.02;
	
	clip( Color.a );
	
	// apply fog
	//Color.rgb = ApplyFogColor( Input.FogIntensity, Color.rgb );

	return Color;
};


// Grass pixel shader - just maps a texture on the geometry surface and applies opacity
//  but unlike the day variant, this makes the grass two times more transparent
float4 GrassNightPS( GRASS_INPUT_PS Input ) : COLOR0
{
	float4 TexCol;
	float4 Color;
	
	TexCol = tex2D( TexSamp1, Input.TexCoord );
	
	Color.rgb = saturate( Input.Light.rgb + Ambient.rgb ) * TexCol.rgb;
	Color.a = (TexCol.a / 2) * Input.Light.a;
	
	Color.a -= 0.02;
	
	clip( Color.a );
	
	// apply fog
	//Color.rgb = ApplyFogColor( Input.FogIntensity, Color.rgb );

	return Color;
};




/////////////////////////////////////////////////////////////////////
//
// Shaders for alpha blended objects (full alpha channel)
//
/////////////////////////////////////////////////////////////////////
// Input and output structs
struct BILLBOARD_INPUT
{
    float4 Position: POSITION;
    float2 TexCoord: TEXCOORD0; // texture coordinates
};

struct BILLBOARD_OUTPUT
{
	float4 Position: POSITION;
    float2 TexCoord: TEXCOORD0; // texture coordinates
    float  FogIntensity: TEXCOORD5; // fog intensity for vertex fog
};

struct BILLBOARD_INPUT_PS 
{
	float2 TexCoord: TEXCOORD0;
    float  FogIntensity: TEXCOORD5; // fog intensity for vertex fog
};



// billboard vertex shader - only transforms the vertex coordinates and computes fog
BILLBOARD_OUTPUT BillboardVS( BILLBOARD_INPUT Input )
{
    BILLBOARD_OUTPUT	Output = (BILLBOARD_OUTPUT) 0;
    
    Output.Position = mul( Input.Position, AllMat );
    Output.TexCoord = Input.TexCoord;

    // fog intensity calculation
    Output.FogIntensity = ComputeFogIntensity( Output.Position.xyz );
    
    return Output;
};


// pixel shader used for alpha blended objects - just maps a texture on the geometry surface and applies opacity
// also applies fog onto it
float4 BillboardPS( BILLBOARD_INPUT_PS Input ) : COLOR0
{
	float4 TexCol;
	float4 Color;
	
	TexCol = tex2D( TexSamp1, Input.TexCoord );
	
	Color.rgba = TexCol.rgba;
	
	Color.a -= 0.005;
	
	clip( Color.a );
	
	// apply fog
	Color.rgb = ApplyFogColor( Input.FogIntensity, Color.rgb );

	return Color;
};


// billboard vertex shader for night - only transforms the vertex coordinates
// doesn't compute the fog
BILLBOARD_OUTPUT BillboardNightVS( BILLBOARD_INPUT Input )
{
    BILLBOARD_OUTPUT	Output = (BILLBOARD_OUTPUT) 0;
    
    Output.Position = mul( Input.Position, AllMat );
    Output.TexCoord = Input.TexCoord;

    // fog intensity calculation
    //Output.FogIntensity = ComputeFogIntensity( Output.Position.xyz );
    
    return Output;
};


// pixel shader used for alpha blended objects - just maps a texture on the geometry surface and applies opacity
// doesn't apply fog onto it
float4 BillboardNightPS( BILLBOARD_INPUT_PS Input ) : COLOR0
{
	float4 TexCol;
	float4 Color;
	
	TexCol = tex2D( TexSamp1, Input.TexCoord );
	
	Color.rgba = TexCol.rgba;
	
	Color.a -= 0.005;
	
	//clip( Color.a );
	
	// apply fog
	//Color.rgb = ApplyFogColor( Input.FogIntensity, Color.rgb );

	return Color;
};




/////////////////////////////////////////////////////////////////////
//
//	Shaders for rendering objects with specular per pixel lighting computation
//
/////////////////////////////////////////////////////////////////////

struct OBJECT_SPECULAR_OUTPUT
{
	float4 Position: POSITION;
	float2 TexCoord: TEXCOORD0;
	float3 Normal: TEXCOORD2; // vertex normal in View coordinates
	float3 View: TEXCOORD3; // precomputed View vector (vector from vertex to Camera)
	float3 Light: TEXCOORD4; // light vector (from vertex to light source)
    float  FogIntensity: TEXCOORD5; // fog intensity for vertex fog
};

struct OBJECT_SPECULAR_INPUT_PS 
{
	float2 TexCoord: TEXCOORD0;
	float3 Normal: TEXCOORD2; // vertex normal in View coordinates
	float3 View: TEXCOORD3; // precomputed View vector (vector from vertex to Camera)
	float3 Light: TEXCOORD4; // light vector (from vertex to light source)
    float  FogIntensity: TEXCOORD5; // fog intensity for vertex fog
};

// object vertex shader for per-pixel specular lighting
OBJECT_SPECULAR_OUTPUT ObjectSpecularVS(OBJECT_INPUT Input)
{
	float3 D;
	float4 P, NT, VT;
    OBJECT_SPECULAR_OUTPUT Output = (OBJECT_SPECULAR_OUTPUT) 0;
    
    // compute position
    Output.Position = mul(Input.Position, AllMat);

    // compute texture coordinates
    Output.TexCoord = Input.TexCoord;


    // prepare vectors for per-pixel lighting computation
    P.xyz = Input.Position;
    P.w = 1;
    VT = P;
    NT.xyz = Input.Normal + Input.Position;
    NT.w = 1;
    
    P = mul( P, WorldViewMat );
    NT = normalize( mul( NT, WorldViewMat ) - P );
    VT = -normalize( mul( VT, WorldViewMat ) );
    D = -normalize( mul( DirLight.Direction, (float3x3) ViewMat ) );

    Output.Normal = NT.xyz;
    Output.View = VT.xyz;
    Output.Light = D;
    

    
    // fog intensity calculation
    Output.FogIntensity = ComputeFogIntensity( Output.Position.xyz );
    
    return Output;
};


// object pixel shader computing per pixel specular lighting
float4 ObjectSpecularPS(OBJECT_SPECULAR_INPUT_PS Input) : COLOR
{
	float4 TexCol;
	float4 Color;
	float3 Specular;
	float3 Diffuse;
	
	TexCol = tex2D( TexSamp1, Input.TexCoord );
	

	float NdotL = dot( Input.Normal, Input.Light );
	
	if( NdotL > 0.f )
	{
		//compute diffuse color
		Diffuse.rgb = NdotL * DirLight.Diffuse.rgb;

		//add specular component
		float3 H = normalize( Input.Light + Input.View );   //half vector
		Specular.rgb = pow( max(0, dot(H, Input.Normal)), MaterialPower ) * DirLight.Specular;
	}
	else Diffuse.rgb = Specular.rgb = 0;


	// apply texture and common lighting 
	Color.rgb = saturate( Diffuse.rgb * Material.Diffuse.rgb + Ambient.rgb ) * TexCol.rgb;
	
	// apply color modifiers
	Color.a = 1; // just for easier multiplication - not an alpha value
	Color = mul(Color, ColorModifier);
	
	// apply specular lighting
	Color.rgb = saturate( Color.rgb + Specular.rgb * Material.Specular.rgb );
	
	Color.a = TexCol.a;
	
	// apply fog
	Color.rgb = ApplyFogColor( Input.FogIntensity, Color.rgb );

	return Color;
};



struct OBJECT_TSM_SPECULAR_OUTPUT
{
	float4 Position: POSITION;
	float4 TSMCoord: TEXCOORD1;
	float2 TexCoord: TEXCOORD0;
	float3 Normal: TEXCOORD2; // vertex normal in View coordinates
	float3 View: TEXCOORD3; // precomputed View vector (vector from vertex to Camera)
	float3 Light: TEXCOORD4; // light vector (from vertex to light source)
	float  FogIntensity: TEXCOORD5; // fog intensity for vertex fog
};

struct OBJECT_TSM_SPECULAR_INPUT_PS
{
	float4 TSMCoord: TEXCOORD1;
	float2 TexCoord: TEXCOORD0;
	float3 Normal: TEXCOORD2; // vertex normal in View coordinates
	float3 View: TEXCOORD3; // precomputed View vector (vector from vertex to Camera)
	float3 Light: TEXCOORD4; // light vector (from vertex to light source)
    float  FogIntensity: TEXCOORD5; // fog intensity for vertex fog
};



OBJECT_TSM_SPECULAR_OUTPUT ObjectWithTSMSpecularVS(OBJECT_INPUT Input)
{
	float3 D;
	float4 P, NT, VT;
	float4 World;
	float4 LightCoord;
    OBJECT_TSM_SPECULAR_OUTPUT Output = (OBJECT_TSM_SPECULAR_OUTPUT) 0;
    
    // compute position
    Output.Position = mul(Input.Position, AllMat);

    // compute texture coordinates
    Output.TexCoord = Input.TexCoord;
    
    // compute texture coordinates for TSM
    World = mul(Input.Position, WorldMat);
    LightCoord = mul(World, ShadowMat);
    Output.TSMCoord = mul(LightCoord, PostProjMat);
    Output.TSMCoord /= Output.TSMCoord.w;
    Output.TSMCoord.z = LightCoord.z;


    // prepare vectors for per-pixel lighting computation
    P.xyz = Input.Position;
    P.w = 1;
    VT = P;
    NT.xyz = Input.Normal + Input.Position;
    NT.w = 1;
    
    P = mul( P, WorldViewMat );
    NT = normalize( mul( NT, WorldViewMat ) - P );
    VT = -normalize( mul( VT, WorldViewMat ) );
    D = -normalize( mul( DirLight.Direction, (float3x3) ViewMat ) );

    Output.Normal = NT.xyz;
    Output.View = VT.xyz;
    Output.Light = D;

    
    // fog intensity calculation
    Output.FogIntensity = ComputeFogIntensity( Output.Position.xyz );
    
    return Output;
};


// Object with TSM with linear PCF pixel shader
float4 ObjectWithTSMLinearPCFSpecularPS(OBJECT_TSM_SPECULAR_INPUT_PS Input) : COLOR
{
	float4 TexCol;
	float4 Color;
	float3 Specular;
	float3 Diffuse;
	float4 ShadowDepth;
	float4 Shadow;
	float2 ShadowCoord;
	float2 LerpPos;
	float r1, r2;
	float Amount;
	
	TexCol = tex2D(TexSamp1, Input.TexCoord);
	
	// Prepare shadow coord
	Input.TSMCoord /= Input.TSMCoord.w;
	ShadowCoord = Input.TSMCoord.xy;
	
	// Get shadow map depth from four corners of pixel
	ShadowDepth.r = tex2D(TexSamp3, ShadowCoord).r;
	ShadowCoord.x += Stride;
	ShadowDepth.g = tex2D(TexSamp3, ShadowCoord).r;
	ShadowCoord.y += Stride;
	ShadowDepth.b = tex2D(TexSamp3, ShadowCoord).r;
	ShadowCoord.x -= Stride;
	ShadowDepth.a = tex2D(TexSamp3, ShadowCoord).r;
	
	// compare
	Shadow = (ShadowDepth >=  Input.TSMCoord.z);
	
	// compute lerp position
	LerpPos = frac(Input.TSMCoord.xy * (1.0f / Stride));
	// do linear PCF
	r1 = lerp(Shadow.r, Shadow.g, LerpPos.x);
	r2 = lerp(Shadow.a, Shadow.b, LerpPos.x);
	Amount = lerp(r1, r2, LerpPos.y);
	

	// lighting computation
	float NdotL = dot( Input.Normal, Input.Light );
	
	if( NdotL > 0.f )
	{
		//compute diffuse color
		Diffuse.rgb = NdotL * DirLight.Diffuse.rgb;

		//add specular component
		float3 H = normalize( Input.Light + Input.View );   //half vector
		Specular.rgb = pow( max(0, dot(H, Input.Normal)), MaterialPower ) * DirLight.Specular;
	}
	else Diffuse.rgb = Specular.rgb = 0;

	// apply common lighting on texture
	Color.rgb = saturate( Diffuse.rgb * Material.Diffuse.rgb * Amount + Ambient.rgb ) * TexCol.rgb;

	// apply color modifiers
	Color.a = 1; // just for easier multiplication - not an alpha value
	Color = mul(Color, ColorModifier);

	// apply specular lighting
	Color.rgb = saturate( Color.rgb + Specular.rgb * Material.Specular.rgb * Amount );
	Color.a = TexCol.a;
	
	// apply fog
	Color.rgb = ApplyFogColor( Input.FogIntensity, Color.rgb );

	return Color;
};


// Object with HW TSM pixel shader
float4 ObjectWithHWTSMSpecularPS(OBJECT_TSM_SPECULAR_INPUT_PS Input) : COLOR
{
	float4 TexCol;
	float4 Color;
	float3 Specular;
	float3 Diffuse;
	float Shadow;
	
	TexCol = tex2D(TexSamp1, Input.TexCoord);
	
	// Get shadow map color to r
	Input.TSMCoord /= Input.TSMCoord.w;
	Shadow = tex2Dproj(TexSamp3, Input.TSMCoord).r;
	
	
	// lighting computation
	float NdotL = dot( Input.Normal, Input.Light );
	
	if( NdotL > 0.f )
	{
		//compute diffuse color
		Diffuse.rgb = NdotL * DirLight.Diffuse.rgb;

		//add specular component
		float3 H = normalize( Input.Light + Input.View );   //half vector
		Specular.rgb = pow( max(0, dot(H, Input.Normal)), MaterialPower ) * DirLight.Specular;
	}
	else Diffuse.rgb = Specular.rgb = 0;

	
	// apply texture and common lighting
	Color.rgb = saturate( Diffuse.rgb * Material.Diffuse.rgb * Shadow + Ambient.rgb ) * TexCol.rgb;
	
	// apply color modifiers
	Color.a = 1; // just for easier multiplication - not an alpha value
	Color = mul(Color, ColorModifier);

	// apply specular lighting
	Color.rgb = saturate( Color.rgb + Specular.rgb * Material.Specular.rgb * Shadow );
	Color.a = TexCol.a;
	
	// apply fog
	Color.rgb = ApplyFogColor( Input.FogIntensity, Color.rgb );

	return Color;
};







