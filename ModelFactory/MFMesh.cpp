#include "StdAfx.h"
#include "MFMesh.h"
#include "MFFormat.h"
#include "FSUtility.h"
#include "MFSkeleton.h"
#include "MFClip.h"
#include "GlobalConfig.h"
#include "HashUtility.h"

MFMesh::MFMesh(void)
{
	m_numBonesPerVertex = 0;
}


MFMesh::~MFMesh(void)
{
}

void MFMesh::Serialize(std::fstream& fs)
{
	if( m_vecSubMesh.empty() )
		return;
	unsigned int tag = MF_MESH;		
	fs.write( (char*)&tag, sizeof(tag) );
	int nameSize = m_strName.size() + 1;
	unsigned int dataSize = 64 + nameSize + 4;
	fs.write( (char*)&dataSize, sizeof(int) );
	fs.write( (char*)&nameSize, sizeof(int) );
	fs.write( m_strName.data(), nameSize );

	float matFloat[16];
	for( int i = 0; i < 16; ++i )
	{
		matFloat[i] = (float)m_transform[i / 4][i % 4];
	}
	
	fs.write( (char*)matFloat, 16 * 4 );

	tag = MF_MESHTRANSFORM;
	fs.write( (char*)&tag, sizeof(tag) );
	dataSize = 40;
	fs.write( (char*)&dataSize, sizeof(int) );

	FbxVector4 T = m_transform.GetT();
	FbxQuaternion Q = m_transform.GetQ();
	FbxVector4 S = m_transform.GetS();

	for( int n = 0; n < 3; ++n )
	{
		float val = (float)T[n];
		fs.write((char*)&val, sizeof(float));
	}

	for( int n = 0; n < 4; ++n )
	{
		float val = (float)Q[n];
		fs.write((char*)&val, sizeof(float));
	}

	for( int n = 0; n < 3; ++n )
	{
		float val = (float)S[n];
		fs.write((char*)&val, sizeof(float));
	}

	if( m_numBonesPerVertex > 0 )
	{
		tag = MF_BONESPERVERTEX;
		fs.write( (char*)&tag, sizeof(tag) );
		dataSize = 4;
		fs.write( (char*)&dataSize, sizeof(int) );
		fs.write( (char*)&m_numBonesPerVertex, sizeof(int) );
	}

	size_t numSubMesh = m_vecSubMesh.size();
	for( int i = 0; i < (int)numSubMesh; ++i )
	{
		tag = MF_SUBMESH;		
		fs.write( (char*)&tag, sizeof(tag) );
		unsigned int dataSize = 8;
		fs.write( (char*)&dataSize, sizeof(int) );
		fs.write( (char*)&(m_vecSubMesh[i]->m_numTriangles), sizeof(int) );
		fs.write( (char*)&i, sizeof(int) );

		tag = MF_VERTEXINDEX;
		fs.write( (char*)&tag, sizeof(tag) );
		unsigned int numElements = m_vecSubMesh[i]->m_vecIndices.size();
		dataSize =  numElements * 4 + 4;
		fs.write( (char*)&dataSize, 4 );
		fs.write( (char*)&numElements, 4);
		fs.write( (char*)(m_vecSubMesh[i]->m_vecIndices.data()), dataSize - 4 );

		tag = MF_POSITION;
		fs.write( (char*)&tag, sizeof(tag) );
		numElements = m_vecSubMesh[i]->m_vecPos.size();
		dataSize =  numElements * 4 + 4;
		fs.write( (char*)&dataSize, 4 );
		fs.write( (char*)&numElements, 4);
		fs.write( (char*)m_vecSubMesh[i]->m_vecPos.data(), dataSize - 4 );

		tag = MF_NORMAL;
		fs.write( (char*)&tag, sizeof(tag) );
		numElements = m_vecSubMesh[i]->m_vecNormal.size();
		dataSize =  numElements * 4 + 4;
		fs.write( (char*)&dataSize, 4 );
		fs.write( (char*)&numElements, 4);
		fs.write( (char*)(m_vecSubMesh[i]->m_vecNormal.data()), dataSize - 4 );

		tag = MF_TANGENT;
		fs.write( (char*)&tag, sizeof(tag) );
		numElements = m_vecSubMesh[i]->m_vecTangent.size();
		dataSize =  numElements * 4 + 4;
		fs.write( (char*)&dataSize, 4 );
		fs.write( (char*)&numElements, 4);
		fs.write( (char*)m_vecSubMesh[i]->m_vecTangent.data(), dataSize - 4 );

		tag = MF_UV1;
		fs.write( (char*)&tag, sizeof(tag) );
		numElements = m_vecSubMesh[i]->m_vecUV.size();
		dataSize =  numElements * 4 + 4;
		fs.write( (char*)&dataSize, 4 );
		fs.write( (char*)&numElements, 4);
		fs.write( (char*)m_vecSubMesh[i]->m_vecUV.data(), dataSize - 4 );

		if( m_numBonesPerVertex > 0 )
		{ 
			tag = MF_BONEINDEX;
			fs.write( (char*)&tag, sizeof(tag) );
			numElements = m_vecSubMesh[i]->m_vecBoneIndex.size();
			dataSize =  numElements * 4 + 4;
			fs.write( (char*)&dataSize, 4 );
			fs.write( (char*)&numElements, 4);
			fs.write( (char*)m_vecSubMesh[i]->m_vecBoneIndex.data(), dataSize - 4 );

			tag = MF_BONEWEIGHT;
			fs.write( (char*)&tag, sizeof(tag) );
			numElements = m_vecSubMesh[i]->m_vecBoneWeight.size();
			dataSize =  numElements * 4 + 4;
			fs.write( (char*)&dataSize, 4 );
			fs.write( (char*)&numElements, 4);
			fs.write( (char*)m_vecSubMesh[i]->m_vecBoneWeight.data(), dataSize - 4 );
		}

		if( m_vecSubMesh[i]->m_strDiffuseMap.size() > 0 )
		{
			tag = MF_DIFFUSEMAP;
			fs.write( (char*)&tag, sizeof(tag) );
			FSUtility fph;
			fph.Initialize(m_vecSubMesh[i]->m_strDiffuseMap.c_str());
			std::string strRefPath = std::string("Textures/") + fph.GetFileName() + ".png";

			nameSize = strRefPath.size() + 1;
			dataSize = nameSize + 4;
			fs.write( (char*)&dataSize, 4 );
			fs.write( (char*)&nameSize, 4);
			fs.write( (char*)strRefPath.data(), nameSize );
		}

		if( m_vecSubMesh[i]->m_strNormalMap.size() > 0 )
		{
			tag = MF_NORMALMAP;
			fs.write( (char*)&tag, sizeof(tag) );
			FSUtility fph;
			fph.Initialize(m_vecSubMesh[i]->m_strNormalMap.c_str());
			std::string strRefPath = std::string("Textures/") + fph.GetFileName() + ".png";

			nameSize = strRefPath.size() + 1;
			dataSize = nameSize + 4;
			fs.write( (char*)&dataSize, 4 );
			fs.write( (char*)&nameSize, 4);
			fs.write( (char*)strRefPath.data(), nameSize );
		}

		if( m_vecSubMesh[i]->m_strSpecularMap.size() > 0 )
		{
			tag = MF_SPECULARMAP;
			fs.write( (char*)&tag, sizeof(tag) );
			FSUtility fph;
			fph.Initialize(m_vecSubMesh[i]->m_strSpecularMap.c_str());
			std::string strRefPath = std::string("Textures/") + fph.GetFileName() + ".png";

			nameSize = strRefPath.size() + 1;
			dataSize = nameSize + 4;
			fs.write( (char*)&dataSize, 4 );
			fs.write( (char*)&nameSize, 4);
			fs.write( (char*)strRefPath.data(), nameSize );
		}
	}

	if( m_numBonesPerVertex > 0 )
	{
		MFSkeleton skeleton(this);
		skeleton.Serialize(fs);

		MFClip clip(this);
		clip.Serialize(fs);
	}
}

void MFMesh::Parse( FbxNode* pNode )
{
	FbxMesh* pMesh = pNode->GetMesh();
	m_strName = pNode->GetName();
	std::cout << "mesh name: " << m_strName << "\n";

	if( m_strName.find("_navigation") != std::string::npos ||
		m_strName.find("_airwall") != std::string::npos ||
		m_strName.find("_physics") != std::string::npos)
	{
		return;
	}

	FbxGeometryConverter converter( pNode->GetFbxManager() );
	pMesh = (FbxMesh*)converter.Triangulate(pMesh, true);
	pMesh->GenerateTangentsData(0, true);

	int numCtrlPoints = pMesh->GetControlPointsCount();

	std::vector<std::vector<std::pair<int, double> > > vecCtrlPoint2SkeletonJoints;
	vecCtrlPoint2SkeletonJoints.resize(numCtrlPoints);

	int numDeformers = pMesh->GetDeformerCount(FbxDeformer::eSkin);
	bool bHasAnimation = false;
	if( numDeformers > 0 )
		bHasAnimation = true;

	m_transform = pNode->EvaluateGlobalTransform();
	FbxVector4 t = pNode->GeometricTranslation.Get();
	FbxVector4 r = pNode->GeometricRotation.Get();
	FbxVector4 s = pNode->GeometricScaling.Get();
	FbxAMatrix matGeometric;
	FbxAMatrix matScale;
	matScale.SetIdentity();
	matScale[2][2] = -1.0;

	matGeometric.SetTRS(t, r, s);
	
	FbxAMatrix matNodeTransformInv = m_transform.Inverse();
	m_transform = matScale * m_transform * matGeometric;

	if( bHasAnimation )
	{
		int  numSkeletonJoints = 0;
		FbxCluster* pCluster = NULL;
		FbxAMatrix matIdentity;
		matIdentity.SetIdentity();

		std::string strPartName = m_strName;
		std::transform( strPartName.begin(), strPartName.end(), strPartName.begin(), std::tolower);
		int pos1 = strPartName.find("_");
		if( pos1 != std::string::npos )
		{
			int pos2 = strPartName.find("_", pos1 + 1);
			if( pos2 != std::string::npos )
				strPartName = strPartName.substr(0, pos2); 
		}
		std::string strSkeletonPartFile = GlobalConfig::GetSingleton()->m_strOutputDirectory + "/" + strPartName + ".skel";

		std::fstream fs(strSkeletonPartFile, std::ios::in | std::ios::binary);
		if( fs.is_open() )
		{
			fs.read( (char*)&numSkeletonJoints, sizeof(int) );
			for( int i = 0; i < numSkeletonJoints; ++i )
			{
				int nameSize = 0;
				fs.read( (char*)&nameSize, sizeof(int) );
				char* pName = new char[nameSize];
				fs.read( pName, nameSize );
				float bindPose[16];
				fs.read( (char*)bindPose, 64 );
				FbxAMatrix matFbx;
				for( size_t j = 0; j < 16; ++j )
					matFbx[j / 4][j % 4] = bindPose[j];
				float TPose[10];
				fs.read( (char*)TPose, 40);

				m_mapBoneUsed[pName] = m_vecBonePoseInverse.size();
				m_vecBonePoseInverse.push_back(std::make_tuple(pName, matFbx, pCluster));
				delete[] pName;
			}
		}
		else
		{
			m_mapBoneUsed.insert( std::make_pair("No Root Motion", m_vecBonePoseInverse.size()) );
			m_vecBonePoseInverse.push_back( std::make_tuple("No Root Motion", matIdentity, pCluster) );
			if( GlobalConfig::GetSingleton()->m_bNeedAttach )
			{
				m_mapBoneUsed.insert( std::make_pair("attach point for away3d", m_vecBonePoseInverse.size()) );
				m_vecBonePoseInverse.push_back( std::make_tuple("attach point for away3d", matIdentity, pCluster) );
			}
		}
		fs.close();

		for( int i = 0; i < std::min(1, numDeformers); ++i )
		{
			FbxSkin* pSkin = (FbxSkin*)pMesh->GetDeformer(i, FbxDeformer::eSkin);
			int numClusters = pSkin->GetClusterCount();
			for( int j = 0; j < numClusters; ++j )
			{
				FbxCluster* pCluster = pSkin->GetCluster(j);

				std::string strBoneName = pCluster->GetLink()->GetName();
				auto iter = m_mapBoneUsed.find( strBoneName );
				if( iter != m_mapBoneUsed.end() )
				{
					std::get<2>(m_vecBonePoseInverse[iter->second]) = pCluster;
				}
				else
				{
					m_mapBoneUsed.insert( std::make_pair(strBoneName, m_vecBonePoseInverse.size()) );
					m_vecBonePoseInverse.push_back( std::make_tuple( strBoneName, matIdentity, pCluster) );
				}
			}
		}

		size_t index = 0;
		std::for_each( m_vecBonePoseInverse.begin(),
			m_vecBonePoseInverse.end(), [&](std::tuple<std::string, FbxAMatrix, FbxCluster*> joint)
		{
			FbxCluster* pCluster = std::get<2>(joint);
			if( pCluster )
			{					
				std::string strJointName = pCluster->GetLink()->GetName();
				FbxAMatrix matModel, matLink;
				pCluster->GetTransformMatrix(matModel);
				pCluster->GetTransformLinkMatrix(matLink);

				std::get<1>(m_vecBonePoseInverse[index]) =
					matLink.Inverse() * matModel * matNodeTransformInv;

				int numPointInfluence = pCluster->GetControlPointIndicesCount();
				int* pIndices = pCluster->GetControlPointIndices();
				double* pWeight = pCluster->GetControlPointWeights();

				for( int k = 0; k < numPointInfluence; ++k )
				{
					vecCtrlPoint2SkeletonJoints[pIndices[k]].push_back( std::make_pair(index, pWeight[k]));
				}
			}
			++index;
		}
		);

		auto iter = vecCtrlPoint2SkeletonJoints.begin();
		auto iterEnd = vecCtrlPoint2SkeletonJoints.end();

		while( iter != iterEnd )
		{
			if( iter->size() > (unsigned int)m_numBonesPerVertex )
			{
				m_numBonesPerVertex = iter->size();
				if( m_numBonesPerVertex > 4 ) 
					m_numBonesPerVertex = 4;
			}
			++iter;
		}
	}

	FbxLayerElementArrayTemplate<int>* pMaterialIndices = NULL;
	pMesh->GetMaterialIndices( &pMaterialIndices );

	int numTriangles = pMesh->GetPolygonCount();

	int numMaterials = pNode->GetMaterialCount();
	int numSubMesh = numMaterials;

	FbxVector4* pCtrlPoints = pMesh->GetControlPoints();	

	for( int i = 0; i < numMaterials; ++i )
	{
		FbxSurfaceMaterial* pMaterial = pNode->GetMaterial(i);

		std::string strDiffuseMap = "";
		std::string strNormalMap = "";
		std::string strSpecularMap = "";

		if( pMaterial->GetClassId() == FbxSurfaceLambert::ClassId ||
			pMaterial->GetClassId() == FbxSurfacePhong::ClassId )
		{
			if( pMaterial->GetClassId() == FbxSurfacePhong::ClassId )
			{
				FbxSurfacePhong* pPhong = (FbxSurfacePhong*)pMaterial;

				FbxProperty& pDiffuseMap = pPhong->FindProperty( FbxSurfaceMaterial::sDiffuse );
				int numMaps = pDiffuseMap.GetSrcObjectCount<FbxFileTexture>();
				for( int j = 0; j < numMaps; ++j )
				{
					FbxFileTexture* pTexture = pDiffuseMap.GetSrcObject<FbxFileTexture>(j);
					strDiffuseMap = pTexture->GetFileName();

				}

				FbxProperty& pNormalMap = pPhong->FindProperty( FbxSurfaceMaterial::sNormalMap );
				numMaps = pNormalMap.GetSrcObjectCount<FbxFileTexture>();
				for( int j = 0; j < numMaps; ++j )
				{
					FbxFileTexture* pTexture = pNormalMap.GetSrcObject<FbxFileTexture>(j);
					strNormalMap = pTexture->GetFileName();
				}

				FbxProperty& pSpecularMap = pPhong->FindProperty( FbxSurfaceMaterial::sSpecularFactor );
				numMaps = pSpecularMap.GetSrcObjectCount<FbxFileTexture>();
				for( int j = 0; j < numMaps; ++j )
				{
					FbxFileTexture* pTexture = pSpecularMap.GetSrcObject<FbxFileTexture>(j);
					strSpecularMap = pTexture->GetFileName();
				}
			}
			else
			{
				FbxSurfaceLambert* pLambert = (FbxSurfaceLambert*)pMaterial;

				FbxProperty& pDiffuseMap = pLambert->FindProperty( FbxSurfaceMaterial::sDiffuse );
				int numMaps = pDiffuseMap.GetSrcObjectCount<FbxFileTexture>();
				for( int j = 0; j < numMaps; ++j )
				{
					FbxFileTexture* pTexture = pDiffuseMap.GetSrcObject<FbxFileTexture>(j);
					strDiffuseMap = pTexture->GetFileName();
				}

				FbxProperty& pNormalMap = pLambert->FindProperty( FbxSurfaceMaterial::sNormalMap );
				numMaps = pNormalMap.GetSrcObjectCount<FbxFileTexture>();
				for( int j = 0; j < numMaps; ++j )
				{
					FbxFileTexture* pTexture = pNormalMap.GetSrcObject<FbxFileTexture>(j);
					strNormalMap = pTexture->GetFileName();
				}
			}
		}
		else
		{
			std::cout << "unkown material type!\n";
		}

		FbxGeometryElementNormal* pNormalLayer = NULL;
		FbxGeometryElementTangent* pTangentLayer = NULL;
		FbxGeometryElementBinormal* pBinormalLayer = NULL;
		FbxGeometryElementUV* pUVLayer = NULL;

		int numNormalLayer = pMesh->GetElementNormalCount();
		if( numNormalLayer > 0 )
		{
			pNormalLayer = pMesh->GetElementNormal(0);
		}

		int numTangentLayer = pMesh->GetElementTangentCount();
		if( numTangentLayer > 0 )
		{
			pTangentLayer = pMesh->GetElementTangent(0);
		}

		int numBinormalLayer = pMesh->GetElementBinormalCount();
		if( numBinormalLayer > 0 )
		{
			pBinormalLayer = pMesh->GetElementBinormal(0);
		}

		int numUVLayer = pMesh->GetElementUVCount();
		if( numUVLayer > 0 )
		{
			pUVLayer = pMesh->GetElementUV(0);
		}

		int numRealTriangles = 0;
		for( int j = 0; j < numTriangles; ++j )
		{
			if( pMaterialIndices->GetAt(j) != i )
			{
				continue;
			}
			++numRealTriangles;
		}

		if( numRealTriangles < 1 )
			continue;

		std::vector<MFVertex> vecVertices;
		vecVertices.resize(numRealTriangles * 3);

		unsigned int vertCount = 0;
		for( int j = 0; j < numTriangles; ++j )
		{
			if( pMaterialIndices->GetAt(j) != i )
			{
				continue;
			}

			for( int k = 0; k < 3; ++k )
			{
				int index = pMesh->GetPolygonVertex(j, k);
				float x = (float)pCtrlPoints[index][0];
				float y = (float)pCtrlPoints[index][1];
				float z = (float)pCtrlPoints[index][2];

				vecVertices[vertCount].pos[0] = x;
				vecVertices[vertCount].pos[1] = y;
				vecVertices[vertCount].pos[2] = z;

				vecVertices[vertCount].vertexId = j * 3 + k;

				auto joint = vecCtrlPoint2SkeletonJoints[index];
				for( int m = 0; m < std::min((int)joint.size(), 4); ++m )
				{
					vecVertices[vertCount].boneIdx[m] = joint[m].first;
					vecVertices[vertCount].weight[m] = (float)joint[m].second;
				}

				if( pNormalLayer )
				{
					auto& pArray = pNormalLayer->GetDirectArray();
					auto& pIndexArray = pNormalLayer->GetIndexArray();

					auto mappingMode = pNormalLayer->GetMappingMode();
					float nx = 0.0;
					float ny = 0;
					float nz = 1.0;

					switch( mappingMode )
					{
					case FbxGeometryElement::eByControlPoint:
						{
							switch( pNormalLayer->GetReferenceMode())
							{
							case FbxGeometryElement::eDirect:
								{
									int normalIndex = index;
									nx = (float)pArray.GetAt(normalIndex)[0];
									ny = (float)pArray.GetAt(normalIndex)[1];
									nz = (float)pArray.GetAt(normalIndex)[2];
									break;
								}
							case FbxGeometryElement::eIndexToDirect:
								{
									int normalIndex = pIndexArray.GetAt(index);
									nx = (float)pArray.GetAt(normalIndex)[0];
									ny = (float)pArray.GetAt(normalIndex)[1];
									nz = (float)pArray.GetAt(normalIndex)[2];
									break;
								}
							default:
								break;
							}
						}
						break;
					case FbxGeometryElement::eByPolygonVertex:
						{
							switch( pNormalLayer->GetReferenceMode())
							{
							case FbxGeometryElement::eDirect:
								{
									int normalIndex = 3 * j + k;
									nx = (float)pArray.GetAt(normalIndex)[0];
									ny = (float)pArray.GetAt(normalIndex)[1];
									nz = (float)pArray.GetAt(normalIndex)[2];
									break;
								}
							case FbxGeometryElement::eIndexToDirect:								
								{
									int normalIndex = pIndexArray.GetAt(3 * j + k);
									nx = (float)pArray.GetAt(normalIndex)[0];
									ny = (float)pArray.GetAt(normalIndex)[1];
									nz = (float)pArray.GetAt(normalIndex)[2];
									break;
								}
							default:
								break;
							}
						}
						break;
					default:
						std::cout << "other mapping\n";
					}

					vecVertices[vertCount].normal[0] = nx;
					vecVertices[vertCount].normal[1] = ny;
					vecVertices[vertCount].normal[2] = nz;
				}

				if( pTangentLayer )
				{
					auto& pArray = pTangentLayer->GetDirectArray();
					auto& pIndexArray = pTangentLayer->GetIndexArray();

					auto mappingMode = pTangentLayer->GetMappingMode();
					float tx = 0.0;
					float ty = 0;
					float tz = 1.0;
					float tw = 1.0;

					switch( mappingMode )
					{
					case FbxGeometryElement::eByControlPoint:
						{
							switch( pTangentLayer->GetReferenceMode())
							{
							case FbxGeometryElement::eDirect:
								{
									int tangentIndex = index;
									tx = (float)pArray.GetAt(tangentIndex)[0];
									ty = (float)pArray.GetAt(tangentIndex)[1];
									tz = (float)pArray.GetAt(tangentIndex)[2];
									tw = (float)pArray.GetAt(tangentIndex)[3];
									break;
								}
							case FbxGeometryElement::eIndexToDirect:
								{
									int tangentIndex = pIndexArray.GetAt(index);
									tx = (float)pArray.GetAt(tangentIndex)[0];
									ty = (float)pArray.GetAt(tangentIndex)[1];
									tz = (float)pArray.GetAt(tangentIndex)[2];
									tw = (float)pArray.GetAt(tangentIndex)[3];
									break;
								}
							default:
								break;
							}
						}
						break;
					case FbxGeometryElement::eByPolygonVertex:
						{
							switch( pTangentLayer->GetReferenceMode())
							{
							case FbxGeometryElement::eDirect:
								{
									int tangentIndex = 3 * j + k;
									tx = (float)pArray.GetAt(tangentIndex)[0];
									ty = (float)pArray.GetAt(tangentIndex)[1];
									tz = (float)pArray.GetAt(tangentIndex)[2];
									tw = (float)pArray.GetAt(tangentIndex)[3];
									break;
								}
							case FbxGeometryElement::eIndexToDirect:								
								{
									int tangentIndex = pIndexArray.GetAt(3 * j + k);
									tx = (float)pArray.GetAt(tangentIndex)[0];
									ty = (float)pArray.GetAt(tangentIndex)[1];
									tz = (float)pArray.GetAt(tangentIndex)[2];
									tw = (float)pArray.GetAt(tangentIndex)[3];
									break;
								}
							default:
								break;
							}
						}
						break;
					default:
						std::cout << "other mapping\n";
					}

					if( tw < 0.0 )
					{
						tx = -tx;
						ty = -ty;
						tz = -tz;
					}
					vecVertices[vertCount].tangent[0] = tx;
					vecVertices[vertCount].tangent[1] = ty;
					vecVertices[vertCount].tangent[2] = tz;
				}

				if( pUVLayer )
				{
					auto& pArray = pUVLayer->GetDirectArray();
					auto& pIndexArray = pUVLayer->GetIndexArray();

					auto mappingMode = pUVLayer->GetMappingMode();
					float u = 0.0;
					float v = 0.0;

					switch( mappingMode )
					{
					case FbxGeometryElement::eByControlPoint:
						{
							switch( pUVLayer->GetReferenceMode())
							{
							case FbxGeometryElement::eDirect:
								{
									int uvIndex = index;
									u = (float)pArray.GetAt(uvIndex)[0];
									v = (float)pArray.GetAt(uvIndex)[1];
									break;
								}
							case FbxGeometryElement::eIndexToDirect:
								{
									int uvIndex = pIndexArray.GetAt(index);
									u = (float)pArray.GetAt(uvIndex)[0];
									v = (float)pArray.GetAt(uvIndex)[1];
									break;
								}
							default:
								break;
							}							
						}
						break;
					case FbxGeometryElement::eByPolygonVertex:
						{
							switch( pUVLayer->GetReferenceMode())
							{
							case FbxGeometryElement::eDirect:
								{
									int uvIndex = 3 * j + k;
									u = (float)pArray.GetAt(uvIndex)[0];
									v = (float)pArray.GetAt(uvIndex)[1];
									break;
								}
							case FbxGeometryElement::eIndexToDirect:								
								{
									int uvIndex = pIndexArray.GetAt(3 * j + k);
									u = (float)pArray.GetAt(uvIndex)[0];
									v = (float)pArray.GetAt(uvIndex)[1];
									break;
								}
							default:
								break;
							}
						}
						break;
					default:
						std::cout << "other mapping\n";
					}

					vecVertices[vertCount].uv[0] = u;
					vecVertices[vertCount].uv[1] = 1.0f - v;
				}

				++vertCount;
			}
		}

		int numRealVertices = vecVertices.size();
		std::map<std::string, int> mapVertices;
		std::vector<MFVertex> vecCompactVertices;
		std::vector<unsigned int> vecIndices;
		vecIndices.resize(numRealVertices);

		for( int j = 0; j < numRealVertices; ++j )
		{
			unsigned char result[16];
			HashUtility::md5( (const unsigned char*)&vecVertices[j], (size_t)(&(((MFVertex*)0)->vertexId)), result);
			std::string md5str = HashUtility::toString(result, 16);
			auto iter = mapVertices.find(md5str);
			if( iter != mapVertices.end())
				vecIndices[j] = vecCompactVertices[iter->second].vertexId;
			else
			{
				size_t idx = vecCompactVertices.size();
				vecVertices[j].vertexId = idx;
				mapVertices.insert( std::make_pair(md5str, idx));
				vecCompactVertices.push_back(vecVertices[j]);
				vecIndices[j] = idx;
			}
		}

		std::vector<float> vecPos;
		std::vector<float> vecNormal;		
		std::vector<float> vecTangent;
		std::vector<float> vecUV;		
		std::vector<int>   vecBoneIndex;
		std::vector<float> vecBoneWeight;

		numRealVertices = vecCompactVertices.size();
		vecPos.resize(numRealVertices * 3);
		vecNormal.resize(numRealVertices * 3);
		vecTangent.resize(numRealVertices * 3);
		vecUV.resize(numRealVertices * 2);
		vecBoneIndex.resize(numRealVertices * m_numBonesPerVertex);
		vecBoneWeight.resize(numRealVertices * m_numBonesPerVertex);

		for( int j = 0; j < numRealVertices; ++j )
		{
			FbxVector4 pos = FbxVector4(vecCompactVertices[j].pos[0], 
				vecCompactVertices[j].pos[1], vecCompactVertices[j].pos[2]);
			FbxVector4 normal = FbxVector4(FbxVector4(vecCompactVertices[j].normal[0], 
				vecCompactVertices[j].normal[1], vecCompactVertices[j].normal[2]));
			FbxVector4 tangent = FbxVector4(FbxVector4(vecCompactVertices[j].tangent[0], 
				vecCompactVertices[j].tangent[1], vecCompactVertices[j].tangent[2]));
			if( numDeformers > 0 )
			{
				pos = m_transform.MultT(pos);
				FbxAMatrix tmpMat = m_transform;
				tmpMat[3][0] = tmpMat[3][1] = tmpMat[3][2] = 0.0;
				tmpMat = tmpMat.Transpose().Inverse();
				normal = tmpMat.MultT(normal);
				tangent = tmpMat.MultT(tangent);
			}
			for( int k = 0; k < 3; ++k )
			{				
				vecPos[j * 3 + k] = (float)pos[k];
				vecNormal[j * 3 + k] = (float)normal[k];
				vecTangent[j * 3 + k] = (float)tangent[k];
				if( k < 2 )
					vecUV[j * 2 + k] = vecCompactVertices[j].uv[k];
			}
			for( int k = 0; k < m_numBonesPerVertex; ++k )
			{
				vecBoneIndex[j * m_numBonesPerVertex + k] = vecCompactVertices[j].boneIdx[k];
				vecBoneWeight[j * m_numBonesPerVertex + k] = vecCompactVertices[j].weight[k];
			}
		}

		for( int j = 0; j < (int)vecIndices.size(); j += 3 )
		{
			std::swap( vecIndices[j], vecIndices[j + 2] );
		}
		MFSubMesh* pSubMesh = new MFSubMesh();
		pSubMesh->m_numTriangles = numRealTriangles;
		pSubMesh->m_vecIndices = vecIndices;
		pSubMesh->m_vecPos = vecPos;
		pSubMesh->m_vecNormal = vecNormal;
		pSubMesh->m_vecTangent = vecTangent;
		pSubMesh->m_vecUV = vecUV;
		pSubMesh->m_vecBoneIndex = vecBoneIndex;
		pSubMesh->m_vecBoneWeight = vecBoneWeight;
		pSubMesh->m_strDiffuseMap = strDiffuseMap;
		pSubMesh->m_strNormalMap = strNormalMap;
		pSubMesh->m_strSpecularMap = strSpecularMap;

		m_vecSubMesh.push_back( pSubMesh );
	}

	if( numDeformers > 0 )
		m_transform.SetIdentity();
}
