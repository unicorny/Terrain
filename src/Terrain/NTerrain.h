/*	
Meine Terrain Klasse, nun auch für OpenGL
*/
#ifndef __N_TERRAIN__
#define __N_TERRAIN__

#include "../GPU/DRVertexBuffer.h"
#include "../GPU/VertexGroups.h"
#include "DRCore2/Foundation/DRHashList.h"

#include <list>
#include <map>

//Versionsnummer
#define TERRAINVERSION 3.24f

class CTerrain
{
public:
	//Kon- und Deskonstruktoren
	CTerrain();
	~CTerrain();

	//Init und Exit
	DRReturn Init(char* pcFileName);
	void Exit();

	//Render
	DRReturn Render(float fTime);

	//GetHigh for a x,z Position
	inline float GetHight(DRVector2 vPos);
	float GetHeight(DRVector2 vPos);

private:
	//strukturen:
	struct SMaps
	{	
		SMaps()
		{memset(paMaps, 0, sizeof(DRColor*)*5);	};

		~SMaps()
		{
			clear();
		}
		void clear() {
			for (int i = 0; i < 5; i++)
			{
				DR_SAVE_DELETE_ARRAY(paMaps[i]);
				iaMapSize[i] = 0;
			}
		}
		union
		{	struct 
			{
				DRColor* pHeightMap;
				DRColor* pColorMap;
				DRColor* pNormalMap;
				DRColor* pDetailMap;
				DRColor* pLightMap;
			};
			DRColor* paMaps[5];
		};
		union
		{	struct 
			{
				int	iHeightMapSize;
				int	iColorMapSize;
				int iNormalMapSize;
				int	iDetailMapSize;
				int	iLightMapSize;
			};
			int	iaMapSize[5];	
		};
	};

	struct SLight
	{
		DRVector3	vPosition;
		DRColor		Color;
		float		fRange;
	};
	//struct for a HeighLayer
	struct SHeightLayer
	{
		float	fStartHeight;
		float	fEndHeight;
		int		iVariance;
		char	acTexture[256];
		float	wKoord;
		float	fTiling;
	};
	//struktur für Node
	//struct for a Node
	struct SNode
	{
		SNode();
		bool bLeaf; //Is true, when on end, no child nodes
		bool bRender; //Is true, when this node would be render
		SNode* pChildsNode[4]; //Pointer to child nodes
		SNode* pParentNode;		//Pointer to parent Node
		SNode* pNeighbourNode[4]; //Pointer to Neighbour Nodes
		char   iDeep;			//how far from root node
		u8     cIndexCount;
		float  fLayer;			//HightLayer, important for texture (sort)

		int aIndices[10];			//Indices Array, with all Indices		

		DRVector3 vEck1;
		DRVector3 vEck2;	
	};
	struct SVI //Vertex Index
	{
		DRVector2 vPos;
		int     iIndex;
	};


	//----------------------------------------------------------------------------------------------------------------------

	//Memberfunktionen
	DRReturn LoadTTP(char* pcTTPFileName);
	DRReturn LoadTer(char* pcTerFileName);

	//Compute the MaxHeight Difference
	DRReturn ComputeMaxHeightDifference();

	//create the node-tree, the first Node Pointer is the root node
	DRReturn InitNodes(SNode* pNode, char iDeep, DRVector2 vEck1, float fHalfLength);
	DRReturn CreateLeafNode(SNode* pNode, char iDeep, DRVector2 vEck1, float fHalfLength);

	//return true, if in the Square the HightDifferenz highter or same as MAXHIGHTDIFFERENZ
	bool IfMaxHightDifference(DRVector2 vEck1, float fHalfLength);

	//This function return a TerrainVertex (are computed)
	DetailedVertex GetTerrainVertex(DRVector2 vPos);

	//Compute Vertices in the Section between vEck1 and vEck2, do they in the map and fill
	//pOutIndices with the Indices in the FAN format, pOutIndices must be a array from 9 int variables
	//vEck1 must be the lower Value, z.B.: vEck1 = (120, 45), vEck2 = (300, 90)
	DRReturn GenerateVertices(DRVector2 vEck1, DRVector2 vEck2, int* pOutIndices, DRVector3* pOutEck1, DRVector3* pOutEck2);
	DRReturn GenerateVertices(DRVector2 vEck1, float fHalfLength, int* pOutIndices, DRVector3* pOutEck1, DRVector3* pOutEck2);

	//Check if the vVertex in the map, if not, they are write in the map and return the Index.
	int GetIndex(DetailedVertex vVertex);

	//Check if vVector in the map, if the return Index, if not than return -1.
	inline	int GetIndex(DRVector2 vVector);

	//Delete the Terrain
	void ReleaseNodes(SNode* pNode);

	//write the Vertcies from the VertexBuffer(if exist) in the map and list
	DRReturn FillListAndMap();

	//Fill the Vertex Buffer and delete or clear the map and the list
	DRReturn FillVertexBuffer();

	//Rekursiv Render Nodes
	DRReturn RenderNodes(SNode* pNode);

	//Lädt die Texturen fürs Terrain
	DRReturn LoadTextures();

	//Make the Neihbour Pointer
	DRReturn InitNeighbourPointer(SNode* pNode);

	//T-Junctions Compute
	//Fill a Array with Pointer to all Indice Arrays in the Nodes
	DRReturn FillArrayT_JunctionsCompute(SNode* pNode);

	//Delete all T-Junctions create Indices
	void DeleteT_Junctions();	
	DRReturn DeleteT_JunctionsNew(SNode* pNode);

	//Sorgt dafür, das benachbarte Nodes sich um maximal eine Ebene unterscheiden
	DRReturn DeleteDoppelCracks(SNode* pNode, float fHalfLength, DRVector2 vEck1);





	//----------------------------------------------------------------------------------------------------------------------

	//Membervariablen
	//maps
	SMaps			m_AllMaps;
	float			m_fTerrainSize;
	float			m_fTerrainHeight;
	float			m_fTerrainTiling;
	SLight			m_aLights[32];
	DRColor			m_AmbientLightColor;
	SHeightLayer	m_aHighLayer[32];		//gave information about the high layers

	//name
	char			m_acTTPFileName[256];

	//vertexDaten
	int				m_iNumVertices;			//anzahl Vertices
	DRVertexBuffer*	m_pVertexBuffer;
	DetailedVertex*	m_pVertices;

	//Daten zur Berechnung
	float			m_fMaxHeightDifference; //what the Name said, using for Height error Compute
	int				m_iMapAktiv;
	SNode*			m_pRootNode;
	std::map<DRVector2, int> mVertexIndexMap;
	std::vector<DetailedVertex> m_TerrainVertices;	//List with all TerrainVertices, will be delete, after Init
	std::vector<int> m_TerrainIndices;
	std::map<DRVector2, int> m_VertexPosMap;	//Map with all Vertices, for generate, then I have all Vertices only once, will be delete after Init

	//Informationsdaten
	int				m_iNodeCounter;
	int				m_iLeafNodeCounter;
	int				m_iMaxDeep;
	int				m_iActuellIndex;
	int				m_iCountIndexBufferList;
	int				m_iNumDeletedDoppelCracks; //Wie der Name schon sagt


	//For Move and Render
	std::map<int, DetailedVertex> m_IndexPosRenderMap;	//Map using for render and move

	//Texturen
	unsigned		m_uiColorMap;
	unsigned		m_uiLightMap;

	//T-Junctions Compute
	int				m_iCountIndice;
	SNode**			m_paNodePointer;

};

#endif //__N_TERRAIN__