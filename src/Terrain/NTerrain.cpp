#include "NTerrain.h"
#include "DREngine/DRLogging.h"
#include "DRCore2/Foundation/DRHash.h"
#include "DRCore2/Utils/DRProfiler.h"
#include "DRCore2/Foundation/DRFile.h"
#include "DRCore2/Foundation/DRVector2i.h"

using namespace std;


//Structs
CTerrain::SNode::SNode()
{
	cIndexCount = 10;
	for (int i = 0; i < 4; i++)
	{
		pChildsNode[i] = NULL;
		pNeighbourNode[i] = NULL;
	}
	pParentNode = NULL;
}

//********************************************************************************************************************++
//********************************************************************************************************************++

//Kon- und Deskonstruktor
CTerrain::CTerrain()
{
	m_pVertexBuffer = NULL;
	m_iMapAktiv = 0;
	m_iNumVertices = 0;
	m_fMaxHeightDifference = 0.0f;
	m_fTerrainSize = 0.0f;
	m_fTerrainHeight = 0.0f;
	m_fTerrainTiling = 0.0f;
	m_iNodeCounter = 0;
	m_iLeafNodeCounter = 0;
	m_iMaxDeep = 0;
	m_pRootNode = NULL;
	m_iActuellIndex = 0;
	m_pVertices = NULL;
	m_iCountIndice = 0;
	m_paNodePointer = NULL;
	m_iNumDeletedDoppelCracks = 0;
}

//----------------------------------------------------------------------------------------------------------------------

CTerrain::~CTerrain()
{
	if(m_pRootNode)
	{
		ReleaseNodes(m_pRootNode);
		DR_SAVE_DELETE(m_pRootNode);
	}
	if(m_pVertexBuffer) m_pVertexBuffer->Exit();
	DR_SAVE_DELETE(m_pVertexBuffer);
	DR_SAVE_DELETE_ARRAY(m_pVertices);
	DR_SAVE_DELETE_ARRAY(m_paNodePointer);
}
//********************************************************************************************************************++
void Percent(float fPercent)
{	
	printf("\rTerrain: %.0f%%", fPercent);
}

//----------------------------------------------------------------------------------------------------------------------
DRReturn CTerrain::Init(char* pcFileName)
{
	//Pointer Check
	if(!pcFileName) return DR_ZERO_POINTER;

	//Hilfsvariablen
	char acName[256];
	sprintf(acName, "%s.ter", pcFileName);
	//Versuche Binär Datei zu laden
	if(LoadTer(acName))
	{
		//Wenn es keine Binär Datei gibt
		//Versuchen die ttp Datei zu laden
		LOG_INFO("Terrain wird neu berechnet!");
		Percent(0.0f);
		sprintf(acName, "%s.ttp", pcFileName);
		if(!DRIsFileExist(acName))
		{
			DRLog.writeToLog("%s Konnte nicht geladen werden.", acName);
			LOG_ERROR("Es exestiert weder eine binär, noch eine Source Terrain Datei.", DR_ERROR);
		}
			//return DR_ERROR;
		DRProfiler timeUsed;
		if(LoadTTP(acName)) LOG_ERROR("Fehler beim Laden der Source Datei", DR_ERROR); 
		DRLog.writeToLog("CTerrain load TTP: %s\n", timeUsed.string().data());
		timeUsed.reset();
		Percent(2.0f);

		if(LoadTextures()) LOG_ERROR("Fehler beim Texturen Laden", DR_ERROR);
		DRLog.writeToLog("CTerrain load textures: %s\n", timeUsed.string().data());
		timeUsed.reset();
		Percent(4.0f);

		//in Ordnung, Terrain kann neu berechnet werden.
		
		//Max Hight Differense
		ComputeMaxHeightDifference();
		DRLog.writeToLog("CTerrain calculate %d x %d max height differences: %.4f (%s)", 
			m_AllMaps.iHeightMapSize, 
			m_AllMaps.iHeightMapSize, 
			m_fMaxHeightDifference, 
			timeUsed.string().data()
		);
		timeUsed.reset();
		Percent(10.0f);

		//Nodes
		m_iMapAktiv = 1;

		//Init Nodes
		//Memory for RootNode reserve 
		m_pRootNode = new SNode;
		m_TerrainVertices.reserve(1000000);
		m_TerrainIndices.reserve(5500000);
		//Rekursiv, Init Nodes
		if (InitNodes(m_pRootNode, 0, DRVector2(0.0f, 0.0f), m_fTerrainSize / 2)) {
			LOG_ERROR("Error in InitNodes", DR_ERROR);
		}
		DRLog.writeToLog("CTerrain init nodes: %s", timeUsed.string().data());
		timeUsed.reset();
		Percent(60.0f);
		DRLog.writeToLog("node count: %d, leaf node count: %d", m_iNodeCounter, m_iLeafNodeCounter);
		DRLog.writeToLog("m_iMaxDeep: %d", m_iMaxDeep);
		// return DR_ERROR;

		if(InitNeighbourPointer(m_pRootNode)) return DR_ERROR;

		DRLog.writeToLog("CTerrain init neighbour pointer: %s", timeUsed.string().data());
		timeUsed.reset();

		if (DeleteDoppelCracks(m_pRootNode, m_fTerrainSize / 2, DRVector2(0.0f, 0.0f)))
			LOG_ERROR("Fehler bei DeleteDoppelCracks in CTerrain::Init", DR_ERROR);

		while(m_iNumDeletedDoppelCracks)
		{
			m_iNumDeletedDoppelCracks = 0;
			InitNeighbourPointer(m_pRootNode);
			DeleteDoppelCracks(m_pRootNode, m_fTerrainSize/2, DRVector2(0.0f, 0.0f));
		}
		
		DRLog.writeToLog("CTerrain recursive delete double cracks: %s", timeUsed.string().data());
		timeUsed.reset();

		//Delete Cracks with T-Junctions 
		FillArrayT_JunctionsCompute(m_pRootNode);
        DeleteT_Junctions();
		DRLog.writeToLog("CTerrain recursive delete T-Junctions: %s", timeUsed.string().data());
		timeUsed.reset();

		Percent(90.0f);

		//Fill VertexBuffer and delete list and map
		DRLog.writeToLog("CTerrain reserved space for %d Vertices (%.3f MByte)", 
			m_TerrainVertices.size(),
			(float)(sizeof(DetailedVertex) * m_TerrainVertices.size()) / 1024.0f / 1024.0f
		);
		DRLog.writeToLog("CTerrain reserved space for %d Indices (%.3f MByte)",
			m_TerrainIndices.size(),
			(float)(sizeof(int) * m_TerrainIndices.size()) / 1024.0f / 1024.0f
		);
		if(FillVertexBuffer()) LOG_ERROR("Fehler bei FillVertexBuffer", DR_ERROR);
		DRLog.writeToLog("CTerrain fill vertex buffer: %s", timeUsed.string().data());
		timeUsed.reset();

		ReleaseNodes(m_pRootNode);
		DR_SAVE_DELETE(m_pRootNode);
		mVertexIndexMap.clear();
		m_VertexPosMap.clear();
		m_IndexPosRenderMap.clear();
		DR_SAVE_DELETE_ARRAY(m_paNodePointer);
		m_AllMaps.clear();
		DRLog.writeToLog("Cleanup nodes and maps: %s", timeUsed.string().data());

		DRLog.writeToLog("\r\nm_iMaxDeep: %d", m_iMaxDeep);
		DRLog.writeToLog("m_iNumVertices: %d", m_iNumVertices);

		Percent(100.0f);

		LOG_INFO("Terrain erfolgreich neu erstellt!");
		return DR_OK;
	}

	LOG_INFO("Terrain wurde erfolgreich geladen!");
	return DR_OK;
}

//----------------------------------------------------------------------------------------------------------------------

void CTerrain::Exit()
{
	glDeleteTextures(1, &m_uiColorMap);
	glDeleteTextures(1, &m_uiLightMap);
}

//********************************************************************************************************************++

DRReturn CTerrain::Render(float fTime)
{
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, m_uiColorMap);
	glBindTexture(GL_TEXTURE_2D, m_uiLightMap);
	
// 	glColor3f(1.0f, 1.0f, 1.0f);
	if(DRGrafikError("Fehler in TerrainRender")) return DR_ERROR;

	m_pVertexBuffer->RenderIndex(GL_TRIANGLES);
	return DRGrafikError("Error by rendering Terrain");
}


//----------------------------------------------------------------------------------------------------------------------
DRReturn CTerrain::RenderNodes(CTerrain::SNode* pNode)
{
	if(pNode->bLeaf)
	{		
		m_pVertexBuffer->RenderIndex(GL_TRIANGLE_FAN, pNode->cIndexCount, (unsigned int*)pNode->aIndices);
		return DRGrafikError("Fehler in RenderNode");
	}
	else
	{
		for(int i = 0; i < 4; i++)
		{
			if(pNode->pChildsNode[i] != NULL)
				RenderNodes(pNode->pChildsNode[i]);
			else
				LOG_WARNING("DTerrain::RenderNodes bLeaf is false, but one Child is Zero!");
		}

	}
	//*/
	
	return DR_OK;
}

//********************************************************************************************************************++
//********************************************************************************************************************++
DRReturn CTerrain::LoadTTP(char* pcTTPFileName)
{
	if(!pcTTPFileName) return DR_ZERO_POINTER;

	DRFile file(pcTTPFileName, "rb");
	// Datei öffnen
	//Open the File
	
	if(!file.isOpen()) return DR_ERROR;
	const auto fileSize = file.getSize();
	const auto headerSize = sizeof(int) * 5 + sizeof(float) * 3;
	if (fileSize < headerSize) {
		LOG_ERROR("ttp file is to small to contain sizes", DR_ERROR);
	}

//	int g_iLightMapRes;
//	DRColor* g_pLightMap;
	// Terrain laden/load
	file.read(m_AllMaps.iaMapSize, sizeof(int), 5);
	file.read(&m_fTerrainSize, sizeof(float), 1);
	file.read(&m_fTerrainHeight, sizeof(float), 1);
	file.read(&m_fTerrainTiling, sizeof(float), 1);

	size_t readedSize = headerSize;

	// Speicher neu reservieren
	//  reserve memory again
	for (int i = 0; i < 5; i++)
	{
		if (m_AllMaps.paMaps[i] != NULL) {
			delete[] m_AllMaps.paMaps[i];
			m_AllMaps.paMaps[i] = NULL;
		}
		if (readedSize + m_AllMaps.iaMapSize[i] * m_AllMaps.iaMapSize[i] * sizeof(DRColor) > fileSize) {
			LOG_ERROR("ttp file to small for color map", DR_ERROR);
		}
		m_AllMaps.paMaps[i] = new DRColor[m_AllMaps.iaMapSize[i]*m_AllMaps.iaMapSize[i]];
		if(m_AllMaps.paMaps[i] == NULL) LOG_ERROR("Das Programm muss auf Grund eines schweren Fehlers geschlossen werden!", DR_ERROR);
		size_t localReadedSize;
		file.read(m_AllMaps.paMaps[i], sizeof(DRColor), m_AllMaps.iaMapSize[i]*m_AllMaps.iaMapSize[i], &localReadedSize);
		readedSize += localReadedSize;
	}

	
	bool bShadows;
	
	file.read(m_aLights, sizeof(SLight), 32);
	file.read(&m_AmbientLightColor, sizeof(DRColor), 1);
	file.read(&bShadows, sizeof(bool), 1);
	file.read(m_aHighLayer, sizeof(SHeightLayer), 32);
	
	// Datei schließen
	//Close the File
	file.close();
	LOG_INFO("TTP Datei wurde erfolgreich geladen!");
	return DR_OK;
}

//----------------------------------------------------------------------------------------------------------------------
DRReturn CTerrain::LoadTer(char* pcTerFileName)
{
	DRProfiler timeUsed;
	if(!pcTerFileName) return DR_ZERO_POINTER;

	FILE*	pFile = NULL;

	// Datei öffnen
	//Open the File
	pFile = fopen(pcTerFileName, "rb");
	if(pFile == NULL) return DR_ERROR;

	float fVersion = TERRAINVERSION;

	//First Check Versionsnumber
	fread(&fVersion, sizeof(float), 1, pFile);
	if(TERRAINVERSION != fVersion)
	{
		fclose(pFile);
		LOG_ERROR("DTerrain::LoadTer Versionnumber isn't correct!", DR_ERROR);
	}

	//Versionsnumber is correct, now we save or load the Values
	fread(m_acTTPFileName, sizeof(char), 256, pFile);
	fread(&m_fTerrainHeight, sizeof(float), 1, pFile);
	fread(&m_fTerrainSize,   sizeof(float), 1, pFile);
	fread(&m_fTerrainTiling, sizeof(float), 1, pFile);
	fread(&m_AmbientLightColor, sizeof(DRColor), 1, pFile);
	fread(m_aLights, sizeof(SLight), 32, pFile);
	fread(m_aHighLayer, sizeof(SHeightLayer), 32, pFile);

	//Maps we need only the hightmap
	fread(&m_AllMaps.iHeightMapSize, sizeof(int), 1, pFile);

	//reserve memory
	if(m_AllMaps.pHeightMap != NULL) delete [] m_AllMaps.pHeightMap; m_AllMaps.pHeightMap = NULL;
	m_AllMaps.pHeightMap = new DRColor[m_AllMaps.iHeightMapSize * m_AllMaps.iHeightMapSize];

	if(m_AllMaps.pHeightMap == NULL)
	{
			// Schwerer Fehler!
			//Havy Exception
			//MessageBox(NULL, "Das Programm muss auf Grund eines schweren Fehlers geschlossen werden!",
			//	       "Fehler", MB_OK | MB_ICONEXCLAMATION);
			LOG_WARNING("Das Programm muss auf Grund eines schweren Fehlers geschlossen werden!");

			// Speicher freigeben
			//release Memory
			if(m_AllMaps.pHeightMap != NULL) delete[] m_AllMaps.pHeightMap; m_AllMaps.pHeightMap = NULL;
			//exit(0);
			return DR_ERROR;
	}
	fread(m_AllMaps.pHeightMap, sizeof(DRColor), m_AllMaps.iHeightMapSize* m_AllMaps.iHeightMapSize, pFile);
	
	//Vertex Daten
	//Vertex Buffer
	fread(&m_iNumVertices, sizeof(int), 1, pFile);
	DR_SAVE_DELETE_ARRAY(m_pVertices);
	m_pVertices = new DetailedVertex[m_iNumVertices];

	//if Load
	fread(m_pVertices, sizeof(DetailedVertex), m_iNumVertices, pFile);
	fclose(pFile); pFile = nullptr;
	DRLog.writeToLog("CTerrain time for loading from %s: %s", pcTerFileName, timeUsed.string().data());
	timeUsed.reset();
	if(m_pVertexBuffer != NULL)
	{
		//VertexBuffer Init and write Vertices into them
		m_pVertexBuffer->Exit();
		DR_SAVE_DELETE(m_pVertexBuffer);
	}
	m_pVertexBuffer = new DRVertexBuffer;

	DRVector3* vPos = new DRVector3[m_iNumVertices];
	DRVector3* vNormal = new DRVector3[m_iNumVertices];
	DRVector2* vTex = new DRVector2[m_iNumVertices];
	DRColor*	 vDiffuse = new DRColor[m_iNumVertices];

	float memorySumMByte = (float)(m_iNumVertices * 12 * 4) / 1024.0f / 1024.0f;
	DRLog.writeToLog("CTerrain time for Memory Allocation (%.0f MByte): %s", memorySumMByte, timeUsed.string().data());
	timeUsed.reset();

	for (int iVertex = 0; iVertex < m_iNumVertices; iVertex++)
	{
		vPos[iVertex] = m_pVertices[iVertex].vPos;
		vNormal[iVertex] = m_pVertices[iVertex].vNormal;
		vTex[iVertex] = m_pVertices[iVertex].vTex;
		vDiffuse[iVertex] = m_pVertices[iVertex].Color;
	}
	DRLog.writeToLog("CTerrain copy pixel data around: %s", timeUsed.string().data());
	if(m_pVertexBuffer->Init(vPos, m_iNumVertices, vDiffuse, vTex, vNormal, false))
		LOG_ERROR("Fehler bei VertexBuffer Init", DR_ERROR);

	DR_SAVE_DELETE_ARRAY(vPos);
	DR_SAVE_DELETE_ARRAY(vNormal);
	DR_SAVE_DELETE_ARRAY(vTex);
	DR_SAVE_DELETE_ARRAY(vDiffuse);
	
	return DR_OK;
}


//********************************************************************************************************************++
//Terrain Generier Funktionen
//********************************************************************************************************************++
DRReturn CTerrain::ComputeMaxHeightDifference()
{
	//helpers
	const int size = m_AllMaps.iHeightMapSize;

	float maxDiff = 0.0f;
	DRColor* pHeightMap = m_AllMaps.pHeightMap;
		
	//loop going to the whole Height Map
	for(int y = 0; y < size - 1; y++)
	{
		for (int x = 0; x < size - 1; x++)
		{
			//Get 3 Pixel
			//fO fO_x
			//.  .   .
			//fO_y
			//.  .   
			float h = pHeightMap[y * size + x].r;
			float hRight = pHeightMap[y * size + (x + 1)].r;
			float hDown = pHeightMap[(y + 1) * size + x].r;

			//Get the Max Height Difference between 2 Pixel in the Height Map
			float dx = fabsf(h - hRight);
			float dy = fabsf(h - hDown);

			if (dx > maxDiff) maxDiff = dx;
			if (dy > maxDiff) maxDiff = dy;
		}
	}
	m_fMaxHeightDifference = maxDiff;
	return DR_OK;	
}

//********************************************************************************************************************++
DRReturn CTerrain::InitNodes(CTerrain::SNode* pNode, char iDeep, DRVector2 vEck1, float fHalfLength)
{
	
	if(NULL == pNode)
	{
		pNode = new SNode;
		LOG_WARNING("DTerrain::InitNodes pNode is Zero, Memory are automatic reserve!");
		if(NULL == pNode)
			LOG_ERROR("DTerrain::InitNodes Error, cannot reserve Memory for node", DR_ERROR);
	}

	if (GenerateVertices(vEck1, fHalfLength, pNode->aIndices, &pNode->vEck1, &pNode->vEck2))
	{
		//Error
		LOG_ERROR("DTerrain::InitNodes Error by GenerateVertices!", DR_ERROR);
	}

	//printf("deep: %d, deep + 1: %d, deep + 1 < deep: %d, iDeep + 1 < 0: %d\n", (int)iDeep, (int)(iDeep+1), (int)(iDeep + 1 < iDeep), (int)(iDeep + 1 < 0));
	//Check if the End is reached (depently from the HightDifferenz in the Square)
	if (iDeep == 30) {
		LOG_ERROR("CTerrain::InitNodes nope you don't want a QuadTree with a deep of 30, that needs > 10 Years for calculating!, ", DR_ERROR);
	}
	if(!IfMaxHightDifference(vEck1, fHalfLength))
//	if(iDeep == 1)
	{
		pNode->bLeaf = true;
		m_iNodeCounter++;
		m_iLeafNodeCounter++;
		
		//write Layer
		pNode->iDeep = iDeep;
		if(iDeep > m_iMaxDeep) m_iMaxDeep = iDeep;
	/*	if(GenerateVertices(vEck1, fHalfLength, pNode->aIndices, &pNode->vEck1, &pNode->vEck2))
		{
			//Error
			TB_ERROR("DTerrain::InitNodes Error by GenerateVertices!", TB_ERROR);
		}
		*/

		return DR_OK;
	}
	else
	{
		pNode->bLeaf = false;
		m_iNodeCounter++;

		// printf("childs for %d, vEck: %f %f\n", (int)iDeep, vEck1.x, vEck1.y);
		printf("\rnodes: %d", m_iNodeCounter);

		//Loop to going throw the Child Nodes and create them
	/*	for (int y = 0; y < 2; y++)
		{
			for (int x = 0; x < 2; x++)
			{
				tbWriteToLog("DTerrain::InitNodes x+y: %d", x+y);
				pNode->pChildsNode[x+y] = new SNode;
				pNode->pChildsNode[x+y]->pParentNode = pNode;
				if(InitNodes(pNode->pChildsNode[x+y], iDeep+1, tbVector2(vEck1.x + x * fHalfLength, vEck1.y + y * fHalfLength), fHalfLength/2))
				{
					TB_ERROR("DTerrain::InitNodes Error by call for Childs!", TB_ERROR);
				}
			}
			
		}
		//*/
		//Childs
		for (int i = 0; i < 4; i++)
		{
			pNode->pChildsNode[i] = new SNode;
			pNode->pChildsNode[i]->pParentNode = pNode;

		}
		// Percent(10.0f + m_iNodeCounter/100.0f);
		if (InitNodes(pNode->pChildsNode[0], iDeep + 1, vEck1, fHalfLength / 2)) {
			LOG_ERROR("DTerrain::InitNodes Error by call for Child 0!", DR_ERROR);
		}
		if (InitNodes(pNode->pChildsNode[1], iDeep + 1, DRVector2(vEck1.x + fHalfLength, vEck1.y), fHalfLength / 2)) {
			LOG_ERROR("DTerrain::InitNodes Error by call for Child 1!", DR_ERROR);
		}
		if (InitNodes(pNode->pChildsNode[2], iDeep + 1, DRVector2(vEck1.x, vEck1.y + fHalfLength), fHalfLength / 2)) {
			LOG_ERROR("DTerrain::InitNodes Error by call for Child 2!", DR_ERROR);
		}
		if (InitNodes(pNode->pChildsNode[3], iDeep + 1, DRVector2(vEck1.x + fHalfLength, vEck1.y + fHalfLength), fHalfLength / 2)) {
			LOG_ERROR("DTerrain::InitNodes Error by call for Child 3!", DR_ERROR);
		}
	}
	return DR_OK;
}

DRReturn CTerrain::CreateLeafNode(SNode* pNode, char iDeep, DRVector2 vEck1, float fHalfLength)
{
	if (NULL == pNode)
	{
		pNode = new SNode;
		LOG_WARNING("DTerrain::InitNodes pNode is Zero, Memory are automatic reserve!");
		if (NULL == pNode)
			LOG_ERROR("DTerrain::InitNodes Error, cannot reserve Memory for node", DR_ERROR);
	}

	if (GenerateVertices(vEck1, fHalfLength, pNode->aIndices, &pNode->vEck1, &pNode->vEck2))
	{
		//Error
		LOG_ERROR("DTerrain::InitNodes Error by GenerateVertices!", DR_ERROR);
	}

	pNode->bLeaf = true;
	m_iNodeCounter++;
	m_iLeafNodeCounter++;

	//write Layer
	pNode->iDeep = iDeep;
	if (iDeep > m_iMaxDeep) m_iMaxDeep = iDeep;

	return DR_OK;
}
//********************************************************************************************************************++
//************************************************************************************************************
bool CTerrain::IfMaxHightDifference(DRVector2 vEck1, float fHalfLength)
{
	//Variablen
	float fMaxHight = 0;

	//Min must be init
	float fMinHeight = GetHight(vEck1);

	// distance between 2 height map points in vertex unit, so it is kind of the precision
	int distanceBetweenHeightMapPoints = (int)(m_fTerrainSize/m_AllMaps.iHeightMapSize);
	if(distanceBetweenHeightMapPoints < 1) distanceBetweenHeightMapPoints = 1;

	//loop to goint throw all Vertices in the range
	for (int y = 0; y < (int)(fHalfLength*2); y += distanceBetweenHeightMapPoints)
	{
		for (int x = 0; x < (int)(fHalfLength*2); x += distanceBetweenHeightMapPoints)
		{
			//Hight save in Temp
			float fTemp = GetHight(DRVector2(vEck1.x + x, vEck1.y + y));

			//Check if it gave a higher or lower value
			if(fTemp > fMaxHight) fMaxHight = fTemp;
			if(fTemp < fMinHeight) fMinHeight = fTemp;
			if(fMaxHight-fMinHeight > 1.0f) return true;
		}
	}

	//Check if the HightDifference of the range lower or same as the m_fMaxHeightDifference
	if(fMaxHight-fMinHeight <= 1.0f)
		return false; //Leaf
	else
		return true; //Childs 	
}

//********************************************************************************************************************++
DRReturn CTerrain::GenerateVertices(DRVector2 vEck1, float fHalfLength, int *pOutIndices, DRVector3* pOutEck1, DRVector3* pOutEck2)
{
		//Check pOutIndices
	if(NULL == pOutIndices)
		LOG_ERROR("DTerrain::GenerateVertices: pOut Indices are Zero", DR_ERROR);



	//Array for the vertices to generate 
	DetailedVertex aVertices[9];

	//loop to Get the 9 Vertices
	//begin by vEck1 

	//0     1   2
	//vEck1 -> ->
	//|  3  4  5
	//V -> ->  ->
	//| 6   7   8
	//V->  ->  vEck2

	//end by vEck2
/*
	for(int y = 0; y < 3; y++)
	{
		for (int x = 0; x < 3; x++)
		{
			aVertices[y* 3 + x] = GetTerrainVertex(tbVector2(vEck1.x + x * fHalfLength,
															 vEck1.y + y * fHalfLength));
		}
	}
	//*/
	// 1    2  3
	//vEck1 -> ->
	//| 8  0   4
	//V -> ->  ->
	//| 7  6    5
	//V -> ->  vEck2

	// 8    7  6
	//vEck1 -> ->
	//| 1/9  0   5
	//V -> ->  ->
	//| 2  3    4
	//V -> ->  vEck2

	aVertices[0] = GetTerrainVertex(DRVector2(vEck1.x + fHalfLength, vEck1.y + fHalfLength));
	aVertices[8] = GetTerrainVertex(vEck1);
	aVertices[7] = GetTerrainVertex(DRVector2(vEck1.x + fHalfLength, vEck1.y));
	aVertices[6] = GetTerrainVertex(DRVector2(vEck1.x + fHalfLength * 2, vEck1.y));
	aVertices[5] = GetTerrainVertex(DRVector2(vEck1.x + fHalfLength * 2, vEck1.y + fHalfLength));
	aVertices[4] = GetTerrainVertex(DRVector2(vEck1.x + fHalfLength * 2, vEck1.y + fHalfLength * 2));
	aVertices[3] = GetTerrainVertex(DRVector2(vEck1.x + fHalfLength, vEck1.y + fHalfLength * 2));
	aVertices[2] = GetTerrainVertex(DRVector2(vEck1.x, vEck1.y + fHalfLength * 2));
	aVertices[1] = GetTerrainVertex(DRVector2(vEck1.x, vEck1.y + fHalfLength));

	if(pOutEck1)
		*pOutEck1 = aVertices[2].vPos;
	if(pOutEck2)
		*pOutEck2 = aVertices[6].vPos;

	//4 is the middle, because we would have a FAN Geometry
/*	//and the middle Vertex must be the first
	pOutIndices[0] = GetIndex(aVertices[4]);
	if(-1 == pOutIndices[0]) TB_WARNING("DTerrain::GenerateVertices: GetIndex gave -1");

	//Help Variable, bool because we need not so much values
	bool bHelp = 0;

	//loop to get the another Indices
	for(int i = 0; i < 8; i++)
	{
		pOutIndices[i+1] = GetIndex(aVertices[i+bHelp]);
		if(-1 == pOutIndices[i+1]) TB_WARNING("DTerrain::GenerateVertices: GetIndex gave -1");

		if(3 == i) bHelp = 1;
	}
//*/
	//loop to get the Indices
	for(int i = 0; i < 9; i++)
	{
		int iTemp = GetIndex(aVertices[i]);
		if (iTemp < 0) {
			LOG_ERROR("index overflow, i32 isn't big enough", DR_ERROR);
		}
		//if(-1 == iTemp) LOG_WARNING("DTerrain::GenerateVertices: GetIndex gave -1");
		if(-1 == iTemp) LOG_ERROR("GetIndex lieferte -1", DR_ERROR);
		pOutIndices[i] = iTemp;
			
	}
	pOutIndices[9] = pOutIndices[1];
	//ready
	return DR_OK;

}

//------------------------------------------------------------------------------------------

DRReturn CTerrain::GenerateVertices(DRVector2 vEck1, DRVector2 vEck2, int* pOutIndices, DRVector3* pOutEck1, DRVector3* pOutEck2)
{

	//Check if vEck1 < vEck2
	if(vEck1.x >= vEck2.x || vEck1.y >= vEck2.y)
	LOG_ERROR("DTerrain::GenerateVertices: vEck1.x or vEck1.y is higher or same as vEck2.x or vEck2.y!", DR_ERROR);

	//fLength = half side length:  
	//fLength = a/2
	//   a
	//  ____
 	//a |  | a
	//	|__|
	//    a
	float fHalfLength = (vEck2.x - vEck1.x)/2;
	
	//Check, if between vEck1 and vEck2 are Square
	if((vEck2.y - vEck1.y)/2 != fHalfLength)
		LOG_ERROR("DTerrain::GenerateVertices: vEck1 and vEck2 are not the Edges from a Square!", DR_ERROR);

	return GenerateVertices(vEck1, fHalfLength, pOutIndices, pOutEck1, pOutEck2);
}

//*********************************************************************************//ein hash aus einem Vector2D
int CTerrain::GetIndex(DRVector2 vVector)
{
	auto it = mVertexIndexMap.find(vVector);
	if (it == mVertexIndexMap.end()) {
		LOG_ERROR("index not found", -1);
	}
	return it->second;
}

//-------------------------------------------------------------------------------------

int CTerrain::GetIndex(DetailedVertex vVertex)
{
	//First Check if the Vertex Exist
	const auto planePosition = DRVector2(vVertex.vPos.x, vVertex.vPos.z);
	auto it = mVertexIndexMap.find(planePosition);
	if (it != mVertexIndexMap.end()) {
		return it->second;
	}

	//check, if the map and list aktivated, if not then try to activate
	if(!m_iMapAktiv) 
	{
		LOG_WARNING("DTerrain::GetIndex Map wasn't activated");
		if(FillListAndMap())
		{
			LOG_WARNING("DTerrain::GetIndex(STerrainVertex vVertex) but m_iMapAktiv = 0 and it given't a VertexBuffer");
			return -1;
		}
	}

	//Vertex not exist, we must write them in the map and in the list
	//map
	const auto& pos = vVertex.vPos;
	mVertexIndexMap.insert({ planePosition, m_iActuellIndex});
	m_iActuellIndex++;

	//list
	m_TerrainVertices.push_back(vVertex);
	m_iNumVertices++;

	return m_iActuellIndex-1;

}

//********************************************************************************************************************++

DetailedVertex CTerrain::GetTerrainVertex(DRVector2 vPos)
{
	//Helpers
	float fDiff;
	DRColor Diffuse;
	DetailedVertex Result; //Result Vertex
	fDiff = m_AllMaps.iHeightMapSize/m_fTerrainSize;
	// float fHigh = DRGetFilteredTexel(m_AllMaps.pHeightMap, m_AllMaps.iHeightMapSize, vPos.x*fDiff, vPos.y*fDiff).r * m_fTerrainHeight;
	float fHeight = GetHeight(vPos);

	//Get Normal Value
	/*float fNDiff = m_AllMaps.iNormalMapSize / m_fTerrainSize;
	DRColor TempC = DRGetFilteredTexel(m_AllMaps.pNormalMap, m_AllMaps.iNormalMapSize, vPos.x * fNDiff, vPos.y * fNDiff);
	Result.vNormal.x = TempC.r;
	Result.vNormal.y = TempC.b;
	Result.vNormal.z = TempC.g;
	*/
	//Set Position Vector
	Result.vPos.x = vPos.x;
	Result.vPos.y = fHeight;
	Result.vPos.z = vPos.y;
	
	/*
	//SetLightMap Texture Koords
	Result.vTex.x = vPos.x/m_fTerrainSize;
	Result.vTex.y = vPos.y/m_fTerrainSize;

	Result.vTexDetail.x = vPos.x/m_fTerrainTiling;
	Result.vTexDetail.y = vPos.y/m_fTerrainTiling;

	

	//Get Diffuse Color
	//LightMap in Diffuse Color schreiben
//	float fCDiff = m_AllMaps.iNormalMapSize/m_fTerrainSize;
//	float fCDiff = m_AllMaps.iLightMapSize/m_fTerrainSize;
	float fCDiff = m_AllMaps.iColorMapSize/m_fTerrainSize;
//	Result.dwColor = GetFilteredTexel(m_AllMaps.pNormalMap, m_AllMaps.iNormalMapSize, vPos.x*fCDiff, vPos.y*fCDiff);
//	Result.dwColor = GetFilteredTexel(m_AllMaps.pLightMap, m_AllMaps.iLightMapSize, vPos.x*fCDiff, vPos.y * fCDiff);
	Result.Color = DRGetFilteredTexel(m_AllMaps.pColorMap, m_AllMaps.iColorMapSize, vPos.x * fCDiff, vPos.y * fCDiff);
//	tbWriteColorToLog(Result.Color);
	

		float w = 0.0f;
		//compute relative Hight (interval [0;1])
		fHeight /= m_fTerrainHeight;

		bool bGo = true;
		//wKoordinate berechnen
		//compute w-koord. for 3D Textur (Cube-Textur)
		for (int i = 0; i < 32; i++)
		{
			if(!bGo) break;
			//x = Start - Variance/100*Start
			//y = End   + Variance/100*End
			if(fHeight >= m_aHighLayer[i].fStartHeight -m_aHighLayer[i].iVariance/100 * m_aHighLayer[i].fStartHeight &&
				fHeight <= m_aHighLayer[i].fEndHeight   +m_aHighLayer[i].iVariance/100 * m_aHighLayer[i].fEndHeight)
			{
				//Position/MapSize*Tiling
				//Texture Coords interval [0;1]
				Result.vTex3.x = vPos.x/m_fTerrainSize*m_aHighLayer[i].fTiling;
				Result.vTex3.y = vPos.y/m_fTerrainSize*m_aHighLayer[i].fTiling;

				//check if the High in the HighLayer i
				if(fHeight >= m_aHighLayer[i].fStartHeight &&
					fHeight <= m_aHighLayer[i].fEndHeight)
				{
				   w = m_aHighLayer[i].wKoord;
				   bGo = false;
				}
				//else it must be a overlay place
				//compute w with random
				else
				{
				//	if(fHigh < m_aHighLayer[i].fStartHeight)
				//	{
							//random Max = Variance/100*Start
				//		float fRandomValue = DRRand::Float(m_aHighLayer[i].iVariance/100 * m_aHighLayer[i].fStartHeight, 0.0f);
				//		w = m_aHighLayer[i].wKoord - fRandomValue;
				//		bGo = false;
				//	}
				//	else 
				//	{
						//random Max = Variance/100*End
				//		float fRandomValue = DRRand::Float(m_aHighLayer[i].iVariance/100 * m_aHighLayer[i].fEndHeight, 0.0f);
				//		w = m_aHighLayer[i].wKoord + fRandomValue;
				//		bGo = false;
				//	}
					bGo = false;
					w = m_aHighLayer[i].wKoord + 0.0f;
				}
			}
		}

		//Result.Color = tbColor(1.0f, 1.0f, 1.0f);
		Result.vTex3.z = w;
		*/
	return Result;
}

//---------------------------------------------------------------------------------------------------------------

float CTerrain::GetHeight(DRVector2 vPos)
{
	if (!m_AllMaps.pHeightMap) return 0.0f;

	const auto heightMap = m_AllMaps.pHeightMap;
	const auto size = m_AllMaps.iHeightMapSize;
	const float scale = (float)size / m_fTerrainSize;

	// position in terrain coords as float
	DRVector2 vMap(vPos * scale);
	// position in terrain coords only integer part
	DRVector2i vMapi((int)floor(vMap.x), (int)floor(vMap.y));
	if (vMapi.x < 0 || vMapi.y < 0 || vMapi.x + 1 >= size || vMapi.y + 1 >= size) return 0.0f;
	// diff from real position to integer position
	DRVector2 vMapFraction(vMap.x - vMapi.x, vMap.y - vMap.y);

	// four points for interpolation
	float h00 = heightMap[vMapi.x + vMapi.y * size].r;
	float h10 = heightMap[(vMapi.x + 1) + vMapi.y * size].r;
	float h01 = heightMap[vMapi.x + (vMapi.y + 1) * size].r;
	float h11 = heightMap[(vMapi.x + 1) + (vMapi.y + 1) * size].r;

	// billinear filtering
	float h0 = h00 * (1.0f - vMapFraction.x) + h10 * vMapFraction.x;
	float h1 = h01 * (1.0f - vMapFraction.x) + h11 * vMapFraction.x;
	float finalHeight = h0 * (1.0f - vMapFraction.y) + h1 * vMapFraction.y;

	auto scaledHeight = finalHeight * m_fTerrainHeight;
	return scaledHeight;
}

float CTerrain::GetHight(DRVector2 vPos)
{
	return GetHeight(vPos);
	// float fDiff = m_AllMaps.iHeightMapSize/m_fTerrainSize;
	// return (DRGetFilteredTexel(m_AllMaps.pHeightMap, m_AllMaps.iHeightMapSize, vPos.x*fDiff, vPos.y*fDiff).r * m_fTerrainHeight);
}


//********************************************************************************************************************++

void CTerrain::ReleaseNodes(CTerrain::SNode* pNode)
{
	char iErrorCounter = 0; //Count the errors
	//go throw the Childs and delete them
	if(!pNode->bLeaf)
	{
		for (int i = 0; i < 4; i++)
		{
			if(pNode->pChildsNode[i] != NULL)
			{
				ReleaseNodes(pNode->pChildsNode[i]);
				DR_SAVE_DELETE(pNode->pChildsNode[i]);
				pNode->pChildsNode[i] = NULL;
			}
			else
				iErrorCounter++;
			
		}
		 if(iErrorCounter > 0) LOG_WARNING("CTerrain::ReleaseNodes bLeaf is false, but one Childs are Zero");

	}

	//set the Pointer To Parent to Zero
	pNode->pParentNode = NULL;
}

//********************************************************************************************************************++
DRReturn CTerrain::FillListAndMap()
{
	if(m_iMapAktiv) return DR_OK;

	if(NULL == m_pVertexBuffer || 0 == m_iNumVertices) LOG_ERROR("DTerrain::FillListAndMap: m_pVertexBuffer is Zero ", DR_ERROR);

	m_iActuellIndex = 0;

	if(NULL == m_pVertices) LOG_ERROR("CTerrain::FillListAndMap m_pVertices is Zero", DR_ERROR);
	m_TerrainVertices.reserve(m_iNumVertices);
	//Loop to copy all value into the map and list
	for (int i = 0; i < m_iNumVertices; i++)
	{
		m_TerrainVertices.push_back(m_pVertices[i]);
		const auto& pos = m_pVertices[i].vPos;
		mVertexIndexMap.insert({{ pos.x, pos.z }, m_iActuellIndex });
		m_iActuellIndex++;
	}
	m_iMapAktiv = 1;

	return DR_OK;
}

//********************************************************************************************************************++
DRReturn CTerrain::FillVertexBuffer()
{
//	return TB_ERROR;
	//load VertexBuffer

	if(m_iNumVertices == 0) return DR_ZERO_POINTER;

	if(m_pVertexBuffer != NULL)
	{
		//VertexBuffer Init and write Vertices into them
		m_pVertexBuffer->Exit();
		DR_SAVE_DELETE(m_pVertexBuffer);
	}
	m_pVertexBuffer = new DRVertexBuffer;
	
	//loop throw the list and fill the VertexBuffer and clear the list
	//and fill the render map
	
	if (m_pVertexBuffer->Init(m_TerrainVertices, m_TerrainIndices))
		LOG_ERROR("Fehler bei VertexBuffer Init", DR_ERROR);
	m_TerrainVertices.clear();
	m_TerrainVertices.shrink_to_fit();
	m_TerrainIndices.clear();
	m_TerrainIndices.shrink_to_fit();
	return DR_OK;
}

//********************************************************************************************************************++
DRReturn CTerrain::LoadTextures()
{
	//Color map
	glGenTextures(1, &m_uiColorMap);
    glBindTexture(GL_TEXTURE_2D, m_uiColorMap);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

	glTexImage2D	 (GL_TEXTURE_2D, 0, 4, m_AllMaps.iColorMapSize, m_AllMaps.iColorMapSize, 0, GL_RGBA, GL_FLOAT, m_AllMaps.pColorMap);
	//gluBuild2DMipmaps(GL_TEXTURE_2D,    format, conv->w, conv->h,    GL_RGBA, GL_UNSIGNED_BYTE, conv->pixels);
   // glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
	DRGrafikError("Fehler beim Color Texture laden!");

	//Light Map
	glGenTextures(1, &m_uiLightMap);
	glBindTexture(GL_TEXTURE_2D, m_uiLightMap);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

	glTexImage2D(GL_TEXTURE_2D, 0, 4, m_AllMaps.iLightMapSize, m_AllMaps.iLightMapSize, 0, GL_RGBA, GL_FLOAT, m_AllMaps.pLightMap);
	DRGrafikError("Fehler beim Light Texture laden!");
	return DR_OK;
}
//********************************************************************************************************************++


DRReturn CTerrain::InitNeighbourPointer(CTerrain::SNode* pNode)
{
	//Helper
	SNode* pTemp = NULL;

	if(NULL == pNode)
		LOG_ERROR("DTerrain::InitNeighbourPointer pNode is Zero", DR_ERROR);

	//Root
	if(pNode->iDeep == 0)
	{
		pTemp = pNode->pChildsNode[0];
		pTemp->pNeighbourNode[1] = pNode->pChildsNode[1];
		pTemp->pNeighbourNode[2] = pNode->pChildsNode[2];

		pTemp = pNode->pChildsNode[1];
		pTemp->pNeighbourNode[2] = pNode->pChildsNode[3];
		pTemp->pNeighbourNode[3] = pNode->pChildsNode[0];

		pTemp = pNode->pChildsNode[2];
		pTemp->pNeighbourNode[0] = pNode->pChildsNode[0];
		pTemp->pNeighbourNode[1] = pNode->pChildsNode[3];

		pTemp = pNode->pChildsNode[3];
		pTemp->pNeighbourNode[0] = pNode->pChildsNode[1];
		pTemp->pNeighbourNode[3] = pNode->pChildsNode[2];

	}
	else if(pNode->bLeaf)
		return DR_OK;
	else
	{
		//not a Root
		//Neighbours Set the Neighbours from the childrens,  pNode must be have his Neighbours
		//Child0
		pTemp = pNode->pChildsNode[0];
		if(pNode->pNeighbourNode[0])
			pTemp->pNeighbourNode[0] = pNode->pNeighbourNode[0]->pChildsNode[2];

		pTemp->pNeighbourNode[1] = pNode->pChildsNode[1];
		pTemp->pNeighbourNode[2] = pNode->pChildsNode[2];
		if(pNode->pNeighbourNode[3])
			pTemp->pNeighbourNode[3] = pNode->pNeighbourNode[3]->pChildsNode[1];

		//Child1
		pTemp = pNode->pChildsNode[1];
		if(pNode->pNeighbourNode[0])
			pTemp->pNeighbourNode[0] = pNode->pNeighbourNode[0]->pChildsNode[3];
		if(pNode->pNeighbourNode[1])
			pTemp->pNeighbourNode[1] = pNode->pNeighbourNode[1]->pChildsNode[0];
		pTemp->pNeighbourNode[2] = pNode->pChildsNode[3];
		pTemp->pNeighbourNode[3] = pNode->pChildsNode[0];

		//Child2
		pTemp = pNode->pChildsNode[2];
		pTemp->pNeighbourNode[0] = pNode->pChildsNode[0];
		pTemp->pNeighbourNode[1] = pNode->pChildsNode[3];
		if(pNode->pNeighbourNode[2])
			pTemp->pNeighbourNode[2] = pNode->pNeighbourNode[2]->pChildsNode[0];
		if(pNode->pNeighbourNode[3])
			pTemp->pNeighbourNode[3] = pNode->pNeighbourNode[3]->pChildsNode[3];

		//Child3
		pTemp = pNode->pChildsNode[3];
		pTemp->pNeighbourNode[0] = pNode->pChildsNode[1];
		if(pNode->pNeighbourNode[1])
			pTemp->pNeighbourNode[1] = pNode->pNeighbourNode[1]->pChildsNode[2];
		if(pNode->pNeighbourNode[2])
			pTemp->pNeighbourNode[2] = pNode->pNeighbourNode[2]->pChildsNode[1];
		pTemp->pNeighbourNode[3] = pNode->pChildsNode[2];

	}
	for(int i = 0; i < 4; i++)
		if(InitNeighbourPointer(pNode->pChildsNode[i]))
			LOG_ERROR("DTerrain::InitNeighbourPointer Error by call rekursiv", DR_ERROR);
		
	return DR_OK;
}

//********************************************************************************************************************++

DRReturn CTerrain::FillArrayT_JunctionsCompute(CTerrain::SNode* pNode)
{
	if(m_paNodePointer == NULL)
	{
		m_paNodePointer = new SNode*[m_iNodeCounter];
		m_iCountIndice = 0;
	}

	m_paNodePointer[m_iCountIndice] = pNode;
	m_iCountIndice++;

	if(pNode->bLeaf == false)
	{
		for (int i = 0; i < 4; i++)
		{
			if(pNode->pChildsNode[i])
				FillArrayT_JunctionsCompute(pNode->pChildsNode[i]);
			else
				LOG_WARNING("DTerrain::FillArrayT_JunctionsCompute bLeaf is false, but one Child is Zero");
		}
	}
	return DR_OK;	
}

//---------------------------------------------------------------------------

void CTerrain::DeleteT_Junctions()
{
	if(DeleteT_JunctionsNew(m_pRootNode)) return;
//	return;
	int tempIndices[10];
	//Init IndexBuffer
	for(int i = 0; i < m_iNodeCounter; i++)
	{		
		SNode* pNode = m_paNodePointer[i];
		if(!pNode->bLeaf) continue;
		int indexCount = 0;

		for(int j = 0; j < 9; j++)
		{
			if(pNode->aIndices[j] != -1) //(DWORD)-1
			{
				//pNode->pIndexBuffer->AddIndex(&pNode->aIndices[j]);
				//pNode->aIndices[pNode->cIndexCount++] = tempIndices[j];
				tempIndices[indexCount++] = pNode->aIndices[j];
			}
		}
		if (pNode->aIndices[1] != -1)
			// pNode->pIndexBuffer->AddIndex(&pNode->aIndices[1]);
			// pNode->aIndices[pNode->cIndexCount++] = pNode->aIndices[1];
			tempIndices[indexCount++] = pNode->aIndices[1];
		else
			// pNode->pIndexBuffer->AddIndex(&pNode->aIndices[2]);
			// pNode->aIndices[pNode->cIndexCount++] = pNode->aIndices[2];
			tempIndices[indexCount++] = pNode->aIndices[2];
		// build GL_TRIANGLES indices from GL_TRIANGLE_FAN
		for (int i = 1; i < indexCount - 1; ++i) {
			m_TerrainIndices.push_back(tempIndices[0]);
			m_TerrainIndices.push_back(tempIndices[i]);
			m_TerrainIndices.push_back(tempIndices[i + 1]);
		}

		m_iCountIndexBufferList++;
	}
}

//************************************************************************************************

DRReturn CTerrain::DeleteT_JunctionsNew(CTerrain::SNode* pNode)
{
	if(pNode->bLeaf)
	{
		for(int i = 0; i < 4; i++)
		{
	
			if(!pNode->pNeighbourNode[i])// || pNode->pNeighbourNode[i]->bLeaf == false)
			{/*
				//auf null setzen
				switch(i)
				{
				case 0:	pNode->aIndices[7] = -1; break;
				case 1: pNode->aIndices[5] = -1; break;
				case 2:	pNode->aIndices[3] = -1; break;
				case 3:	pNode->aIndices[1] = -1; break;
				}
				*/
				switch(i)
				{
				case 0:	pNode->aIndices[7] = -1; break;
				case 1: pNode->aIndices[5] = -1; break;
				case 2:	pNode->aIndices[3] = -1; break;
				case 3:	pNode->aIndices[1] = -1; break;
				}
			}
		}	
	}
	else
	{
		if(DeleteT_JunctionsNew(pNode->pChildsNode[0]) ||
		   DeleteT_JunctionsNew(pNode->pChildsNode[1]) ||
		   DeleteT_JunctionsNew(pNode->pChildsNode[2]) ||
		   DeleteT_JunctionsNew(pNode->pChildsNode[3]))
		   return DR_ERROR;

	}

	return DR_OK;
}
//-------------------------------------------------
//********************************************************************************************************************++

DRReturn CTerrain::DeleteDoppelCracks(CTerrain::SNode* pNode, float fHalfLength, DRVector2 vEck1)
{	
//	return TB_OK;
	//wenn Ende (Blatt)
	if(pNode->bLeaf)
	{
		for (int i = 0; i < 4; i++)
		{
			//NachbarKnoten überprüfen
			if(pNode->pNeighbourNode[i])
				if(!pNode->pNeighbourNode[i]->bLeaf)
				{
					for (int j = 0; j < 4; j++)
					{
						//Wenn ein Child Knoten von einem Nachbarknoten nicht Blatt ist, dann muss weiter unterteielt werden
						if(!pNode->pNeighbourNode[i]->pChildsNode[j]->bLeaf)
						{	
							
							m_iNumDeletedDoppelCracks++;
							pNode->bLeaf = false;
														
							for(int iC = 0; iC < 4; iC++)
							{
								pNode->pChildsNode[iC] = new SNode;
								pNode->pChildsNode[iC]->pParentNode = pNode;
							}
							

							//	ZeroMemory(pNode->aIndices, sizeof(DWORD)*10);
							DRReturn Result = DR_OK;
							if (CreateLeafNode(pNode->pChildsNode[0], pNode->iDeep + 1, vEck1, fHalfLength / 2)) {
								LOG_ERROR("Error creating LeafNode 0 in Delete DoubleCracks", DR_ERROR);
							}
							if (CreateLeafNode(pNode->pChildsNode[1], pNode->iDeep + 1, DRVector2(vEck1.x + fHalfLength, vEck1.y), fHalfLength / 2)) {
								LOG_ERROR("Error creating LeafNode 1 in Delete DoubleCracks", DR_ERROR);
							}
							if (CreateLeafNode(pNode->pChildsNode[2], pNode->iDeep + 1, DRVector2(vEck1.x, vEck1.y + fHalfLength), fHalfLength / 2)) {
								LOG_ERROR("Error creating LeafNode 2 in Delete DoubleCracks", DR_ERROR);
							}
							if (CreateLeafNode(pNode->pChildsNode[3], pNode->iDeep + 1, DRVector2(vEck1.x + fHalfLength, vEck1.y + fHalfLength), fHalfLength / 2)) {
								LOG_ERROR("Error creating LeafNode 3 in Delete DoubleCracks", DR_ERROR);
							}
				
							InitNeighbourPointer(pNode);
							
							if(Result)
								LOG_WARNING("Fehler in DTerrain::DeleteDoppelCracks, beim erstellen von neuen Nodes");
							if(pNode->bLeaf) LOG_WARNING("DTerrain::DeleteDoppelCracks bLeaf s true, after \"bLeaf = false\"");
							DeleteDoppelCracks(pNode->pChildsNode[0], fHalfLength/2, vEck1);
							DeleteDoppelCracks(pNode->pChildsNode[1], fHalfLength/2, DRVector2(vEck1.x + fHalfLength, vEck1.y));
							DeleteDoppelCracks(pNode->pChildsNode[2], fHalfLength/2, DRVector2(vEck1.x, vEck1.y + fHalfLength));
							DeleteDoppelCracks(pNode->pChildsNode[3], fHalfLength/2, DRVector2(vEck1.x + fHalfLength, vEck1.y + fHalfLength));

							return DR_OK;
						}
					}
				}
		}
		return DR_OK;
	}
	else
	{
		DeleteDoppelCracks(pNode->pChildsNode[0], fHalfLength/2, vEck1);
		DeleteDoppelCracks(pNode->pChildsNode[1], fHalfLength/2, DRVector2(vEck1.x + fHalfLength, vEck1.y));
		DeleteDoppelCracks(pNode->pChildsNode[2], fHalfLength/2, DRVector2(vEck1.x, vEck1.y + fHalfLength));
		DeleteDoppelCracks(pNode->pChildsNode[3], fHalfLength/2, DRVector2(vEck1.x + fHalfLength, vEck1.y + fHalfLength));
		return DR_OK;
	}
}