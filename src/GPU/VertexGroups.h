#ifndef __S_TERRAIN_VERTEX_H
#define __S_TERRAIN_VERTEX_H

#include "DRCore2/Foundation/DRColor.h"
#include "DRCore2/Foundation/DRVector3.h"
#include "DRCore2/Foundation/DRVector2.h"

// struct for very detailed vertices (68 Bytes when DRReal is float), not cacheline optimzed 
// sorted for byte alignment
struct DetailedVertex
{
	DRVector3 vPos;       // 12 Byte
	DRVector3 vNormal;    // 12 Byte, normal for Light compute
	DRVector3 vTex3;      // 12 Byte, VolumenTexture Koordinaten
	DRVector2 vTex;       // 8 Byte
	DRVector2 vTexDetail; // 8 Byte, Texturkoordinaten für DetailMap
	DRColor   Color;	  // 16 Byte, Diffuse Color
};
#endif //__S_TERRAIN_VERTEX_H