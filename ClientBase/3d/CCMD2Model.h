//------------------------------------------------------------------------
//
//	CCMD2Model - Quake 2 model (MD2) renderer for cocos2d-x
//
//	Licensed under the BSD license, see LICENSE in root for details.
// 
//	Copyright (c) 2012 James Hui (a.k.a. Dr.Watson)
// 
//	For latest updates, please visit http://cn.cocos2d-x.org/bbs
//
//------------------------------------------------------------------------

#ifndef _MD2MODEL_H
#define _MD2MODEL_H

#include "cocos2d.h"

using namespace cocos2d;


#define SMALLEST_FP		0.000001f

#define MAX_FRAMES		512

#pragma pack(push)
#pragma pack(1)   

//-------------------------------------------------------------------------------------------------
enum MD2AnimationStates
{
	STATE_IDLE,
	STATE_RUNNING,
	STATE_SHOT_NOT_FALLING_DOWN,
	STATE_SHOT_IN_SHOULDER,
	STATE_JUMP,
	STATE_IDLE2,
	STATE_SHOT_FALLING_DOWN,
	STATE_IDLE3,
	STATE_IDLE4,
	STATE_CROUCHING,
	STATE_CROUCHING_CRAWL,
	STATE_IDLE_CROUCHING,
	STATE_KNEELING_DYING,
	STATE_FALLING_BACK_DYING,
	STATE_FALING_FORWARD_DYING,
	STATE_FALLING_BACK_SLOWLY_DYING,
	MAX_ANIMATION
};


//-------------------------------------------------------------------------------------------------
class MD2Animation
{
public:
	int mStartFrame;
	int mEndFrame;

	MD2Animation(int start, int end);
	
};


//-------------------------------------------------------------------------------------------------
// texture coordinate
typedef struct
{
   float s;
   float t;
} texCoord_t;


//-------------------------------------------------------------------------------------------------
// texture coordinate index
typedef struct
{
   short s;
   short t;
} stIndex_t;


//-------------------------------------------------------------------------------------------------
// info for a single frame point
typedef struct
{
   unsigned char v[3];
   unsigned char normalIndex;		// not used
} framePoint_t;


//-------------------------------------------------------------------------------------------------
// information for a single frame
typedef struct
{
   float scale[3];
   float translate[3];
   char name[16];
   framePoint_t fp[1];
} frame_t;


//-------------------------------------------------------------------------------------------------
// data for a single triangle
typedef struct
{
   unsigned short meshIndex[3];		// vertex indices
   unsigned short stIndex[3];		// texture coordinate indices
} mesh_t;


//-------------------------------------------------------------------------------------------------
// the model data
typedef struct
{
	int numFrames;				// number of frames
	int numPoints;				// number of points
	int numTriangles;			// number of triangles
    int numST;					// number of skins
	int frameSize;				// size of each frame in bytes
	int texWidth, texHeight;	// texture width, height
	int currentFrame;			// current frame # in animation
	int nextFrame;				// next frame # in animation
	float interpol;				// percent through current frame
	mesh_t *triIndex;			// triangle list
	texCoord_t *st;				// texture coordinate list
	kmVec3 *pointList;		// vertex list
	CCTexture2D *modelTex;		// texture
} modelData_t;


//-------------------------------------------------------------------------------------------------
typedef struct
{
   int ident;			// identifies as MD2 file "IDP2"
   int version;			// 
   int skinwidth;		// width of texture
   int skinheight;		// height of texture
   int framesize;		// number of bytes per frame
   int numSkins;		// number of textures
   int numXYZ;			// number of points
   int numST;			// number of texture
   int numTris;			// number of triangles
   int numGLcmds;
   int numFrames;		// total number of frames
   int offsetSkins;		// offset to skin names (64 bytes each)
   int offsetST;		// offset of texture s-t values
   int offsetTris;		// offset of triangle mesh
   int offsetFrames;	// offset of frame data (points)
   int offsetGLcmds;	// type of OpenGL commands to use
   int offsetEnd;		// end of file
} modelHeader_t;


#pragma pack(pop)


class CCTexture;

//////////////////////////////////////////////////////////////////////////
/// Helper class to display Quake 2 MD2 model.
/// 
//////////////////////////////////////////////////////////////////////////
class CCMD2Model
{
public:

	//////////////////////////////////////////////////////////////////////////
	/// Constructor.
	//////////////////////////////////////////////////////////////////////////
	CCMD2Model();

	~CCMD2Model();

	//////////////////////////////////////////////////////////////////////////
	/// Set model state.
	/// 
	/// @param newState - Model state.
	///
	//////////////////////////////////////////////////////////////////////////
	void SetState(int newState);

	//////////////////////////////////////////////////////////////////////////
	/// Load MD2 model.
	/// 
	/// @param filename - Name of MD2 file.
	/// @param texturenName - Name of texture.
	/// 
	//////////////////////////////////////////////////////////////////////////
	bool Load(char *filename, char *textureName);

	//////////////////////////////////////////////////////////////////////////
	/// Render a single frame of the model.
	/// 
	/// @param frameNum - Frame to render.
	/// 
	//////////////////////////////////////////////////////////////////////////
	void Render(int frameNum);

	//////////////////////////////////////////////////////////////////////////
	/// Update animation.
	/// 
	/// @param dt - Time elpased since last update (in seconds).
	/// 
	//////////////////////////////////////////////////////////////////////////
	void Update(float dt);

	//////////////////////////////////////////////////////////////////////////
	/// Set speed of animation.
	/// 
	/// @param speed - Speed of animation.
	/// 
	//////////////////////////////////////////////////////////////////////////
	void SetAnimationSpeed(float speed);

	//////////////////////////////////////////////////////////////////////////
	/// Render the model.
	/// 
	/// @param percent - Interpolating percentage.
	/// 
	//////////////////////////////////////////////////////////////////////////
	void Render();

	//////////////////////////////////////////////////////////////////////////
	/// Get number of polygons of the model.
	/// 
	/// @return Number of polygons.
	/// 
	//////////////////////////////////////////////////////////////////////////
	int GetPolygonCount();
	
	void ReloadTexture(const char *textureName);


private:
	modelData_t *mModel;

	void CheckNextState();

//	void CalculateNormal(float *p1, float *p2, float *p3);
	void CalculateNormal(kmVec3* normal, float *p1, float *p2, float *p3);
	
	MD2Animation **mAnimations;
	int mState;
	int mNextState;

	float mAnimationSpeed;

	float *uv;
	float *vertices;
	
};

#endif
