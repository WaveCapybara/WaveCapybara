#include "..\Public\Mesh.h"
#include "Instance_Mesh.h"
#include "Instance_Model.h"
#include "Bone.h"
#include "Frustum.h"

CInstance_Mesh::CInstance_Mesh(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
	: CVIBuffer_Instance(pDevice, pContext)
{
}

CInstance_Mesh::CInstance_Mesh(const CInstance_Mesh & rhs)
	: CVIBuffer_Instance(rhs)
	, m_iMaterialIndex(rhs.m_iMaterialIndex)
{
	strcpy_s(m_szName, rhs.m_szName);
}

HRESULT CInstance_Mesh::Initialize_Prototype_Binary(_fmatrix PivotMatrix, CInstance_Model* pModel, BINARYINFO& eModel)
{
	memcpy(&m_szName, &eModel.Buffer[eModel.iIndex], sizeof(m_szName));
	eModel.iIndex += sizeof(m_szName);

	memcpy(&m_iMaterialIndex, &eModel.Buffer[eModel.iIndex], sizeof(_uint));
	eModel.iIndex += sizeof(_uint);

	m_iNumVertexBuffers = 2;

	memcpy(&m_iNumVertices, &eModel.Buffer[eModel.iIndex], sizeof(_uint));
	eModel.iIndex += sizeof(_uint);

	_uint iNumFaces;
	memcpy(&iNumFaces, &eModel.Buffer[eModel.iIndex], sizeof(_uint));
	eModel.iIndex += sizeof(_uint);
	m_iNumIndices = iNumFaces * 3;

	m_iIndexStride = sizeof(_ulong);
	m_eIndexFormat = DXGI_FORMAT_R32_UINT;
	m_eTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	m_iStride = sizeof(VTXMODEL);

	m_iNumIndicesPerInstance = m_iNumIndices;
	m_iInstanceStride = sizeof(MODELINSTANCE);

	ZeroMemory(&m_BufferDesc, sizeof m_BufferDesc);

	m_BufferDesc.ByteWidth = m_iStride * m_iNumVertices;
	m_BufferDesc.Usage = D3D11_USAGE_DEFAULT;
	m_BufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	m_BufferDesc.CPUAccessFlags = 0;
	m_BufferDesc.MiscFlags = 0;
	m_BufferDesc.StructureByteStride = m_iStride;

	HRESULT			hr = { 0 };
	
	hr = Ready_VertexBuffer_NonAnim_Binary(PivotMatrix, eModel);

	if (FAILED(hr))
		return E_FAIL;

	ZeroMemory(&m_BufferDesc, sizeof m_BufferDesc);

	m_BufferDesc.ByteWidth = m_iIndexStride * m_iNumIndices;
	m_BufferDesc.Usage = D3D11_USAGE_DEFAULT;
	m_BufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	m_BufferDesc.CPUAccessFlags = 0;
	m_BufferDesc.MiscFlags = 0;
	m_BufferDesc.StructureByteStride = 0;

	_ulong* pIndices = new _ulong[m_iNumIndices];
	ZeroMemory(pIndices, sizeof(_ulong) * m_iNumIndices);

	memcpy(&pIndices[0], &eModel.Buffer[eModel.iIndex], sizeof(_ulong) * m_iNumIndices);
	eModel.iIndex += sizeof(_ulong) * m_iNumIndices;

	ZeroMemory(&m_SubresourceData, sizeof m_SubresourceData);
	m_SubresourceData.pSysMem = pIndices;
	
	if (FAILED(m_pDevice->CreateBuffer(&m_BufferDesc, &m_SubresourceData, &m_pIB)))
		return E_FAIL;

	Safe_Delete_Array(pIndices);

	return S_OK;
}

HRESULT CInstance_Mesh::Initialize(void* pArg)
{
	return S_OK;
}

HRESULT CInstance_Mesh::Generate_Instance(void* pArg)
{
	MESHINSTANCEDESC InstanceDesc = *(MESHINSTANCEDESC*)pArg;

	ZeroMemory(&m_BufferDesc, sizeof m_BufferDesc);

	m_iNumInstance = InstanceDesc.iNumParticles;

	MODELINSTANCE* pInstanceVertices = new MODELINSTANCE[m_iNumInstance];
	m_pInstanceVertices = new MODELINSTANCE[m_iNumInstance];

	ZeroMemory(pInstanceVertices, sizeof(MODELINSTANCE) * m_iNumInstance);
	ZeroMemory(m_pInstanceVertices, sizeof(MODELINSTANCE) * m_iNumInstance);

	m_BufferDesc.ByteWidth = m_iInstanceStride * m_iNumInstance;
	m_BufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	m_BufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	m_BufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	m_BufferDesc.MiscFlags = 0;
	m_BufferDesc.StructureByteStride = m_iInstanceStride;

	for (_uint i = 0; i < m_iNumInstance; ++i)
	{
		_float fRandSize = RAND(InstanceDesc.fMinSize, InstanceDesc.fMaxSize);

		pInstanceVertices[i].vRight = _float4(fRandSize, 0.f, 0.f, 0.f);
		pInstanceVertices[i].vUp = _float4(0.f,fRandSize, 0.f, 0.f);
		pInstanceVertices[i].vLook = _float4(0.f, 0.f, fRandSize, 0.f);
		
		if (true == InstanceDesc.bOnePoint)
		{
			pInstanceVertices[i].vTranslation = _float4(0.f, 0.f, 0.f, 1.f);
		}
		else if (true == InstanceDesc.bBoundary)
		{
			pInstanceVertices[i].vTranslation = _float4(RAND(InstanceDesc.vRange.x * -0.5f, InstanceDesc.vRange.x * 0.5f),
				RAND(InstanceDesc.vRange.y * -0.5f, InstanceDesc.vRange.y * 0.5f),
				RAND(InstanceDesc.vRange.z * -0.5f, InstanceDesc.vRange.z * 0.5f),
				1.f);
		}
	}

	ZeroMemory(&m_SubresourceData, sizeof m_SubresourceData);
	m_SubresourceData.pSysMem = pInstanceVertices;

	if (FAILED(m_pDevice->CreateBuffer(&m_BufferDesc, &m_SubresourceData, &m_pVBInstance)))
		return E_FAIL;

	Safe_Delete_Array(pInstanceVertices);

	return S_OK;
}

HRESULT CInstance_Mesh::Load_InstanceInfo(_uint iNumInstance, MODELINSTANCE* pModleInstance)
{
	ZeroMemory(&m_BufferDesc, sizeof m_BufferDesc);

	m_iNumInstance = iNumInstance;

	MODELINSTANCE* pInstanceVertices = new MODELINSTANCE[m_iNumInstance];
	m_pInstanceVertices = new MODELINSTANCE[m_iNumInstance];

	ZeroMemory(pInstanceVertices, sizeof(MODELINSTANCE) * m_iNumInstance);
	ZeroMemory(m_pInstanceVertices, sizeof(MODELINSTANCE) * m_iNumInstance);

	m_BufferDesc.ByteWidth = m_iInstanceStride * m_iNumInstance;
	m_BufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	m_BufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	m_BufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	m_BufferDesc.MiscFlags = 0;
	m_BufferDesc.StructureByteStride = m_iInstanceStride;

	for (_uint i = 0; i < m_iNumInstance; ++i)
	{
		XMStoreFloat4(&m_pInstanceVertices[i].vRight, XMLoadFloat4(&pModleInstance[i].vRight));
		XMStoreFloat4(&m_pInstanceVertices[i].vUp, XMLoadFloat4(&pModleInstance[i].vUp));
		XMStoreFloat4(&m_pInstanceVertices[i].vLook, XMLoadFloat4(&pModleInstance[i].vLook));
		XMStoreFloat4(&m_pInstanceVertices[i].vTranslation, XMLoadFloat4(&pModleInstance[i].vTranslation));
	}

	ZeroMemory(&m_SubresourceData, sizeof m_SubresourceData);
	m_SubresourceData.pSysMem = pInstanceVertices;

	if (FAILED(m_pDevice->CreateBuffer(&m_BufferDesc, &m_SubresourceData, &m_pVBInstance)))
		return E_FAIL;

	Safe_Delete_Array(pInstanceVertices);

	return S_OK;		
}

HRESULT CInstance_Mesh::Generate_CullingInstance(list<_uint>* CullIndexList)
{
	ZeroMemory(&m_BufferDesc, sizeof m_BufferDesc);
	Safe_Release(m_pVBInstance);
	m_pVBInstance = { nullptr };

	m_iNumInstance_Culling = _uint(CullIndexList->size());
	
	if (0 == m_iNumInstance_Culling)
		return S_OK;
	
	m_BufferDesc.ByteWidth = m_iInstanceStride * m_iNumInstance_Culling;
	m_BufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	m_BufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	m_BufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	m_BufferDesc.MiscFlags = 0;
	m_BufferDesc.StructureByteStride = m_iInstanceStride;

	MODELINSTANCE* pInstanceVertices = new MODELINSTANCE[m_iNumInstance_Culling];
	ZeroMemory(pInstanceVertices, sizeof(MODELINSTANCE) * m_iNumInstance_Culling);	

	_uint index = 0;
	
	for (auto& iter : *CullIndexList)
	{
		XMStoreFloat4(&pInstanceVertices[index].vRight, XMLoadFloat4(&m_pInstanceVertices[iter].vRight));
		XMStoreFloat4(&pInstanceVertices[index].vUp, XMLoadFloat4(&m_pInstanceVertices[iter].vUp));
		XMStoreFloat4(&pInstanceVertices[index].vLook, XMLoadFloat4(&m_pInstanceVertices[iter].vLook));
		XMStoreFloat4(&pInstanceVertices[index].vTranslation, XMLoadFloat4(&m_pInstanceVertices[iter].vTranslation));

		++index;
	}

	ZeroMemory(&m_SubresourceData, sizeof m_SubresourceData);
	m_SubresourceData.pSysMem = pInstanceVertices;

	if (FAILED(m_pDevice->CreateBuffer(&m_BufferDesc, &m_SubresourceData, &m_pVBInstance)))
		return E_FAIL;

	Safe_Delete_Array(pInstanceVertices);

	return S_OK;
}

void CInstance_Mesh::Tick(_double TimeDelta)
{
}

HRESULT CInstance_Mesh::Ready_VertexBuffer_NonAnim_Binary(_fmatrix PivotMatrix, BINARYINFO& eModel)
{
	VTXMODEL* pVertices = new VTXMODEL[m_iNumVertices];
	ZeroMemory(pVertices, sizeof(VTXMODEL) * m_iNumVertices);

	memcpy(&pVertices[0], &eModel.Buffer[eModel.iIndex], sizeof(VTXMODEL) * m_iNumVertices);
	eModel.iIndex += sizeof(VTXMODEL) * m_iNumVertices;

	for (size_t i = 0; i < m_iNumVertices; i++)
	{
		XMStoreFloat3(&pVertices[i].vPosition,
			XMVector3TransformCoord(XMLoadFloat3(&pVertices[i].vPosition), PivotMatrix));

		XMStoreFloat3(&pVertices[i].vNormal,
			XMVector3TransformNormal(XMLoadFloat3(&pVertices[i].vNormal), PivotMatrix));
	}

	ZeroMemory(&m_SubresourceData, sizeof m_SubresourceData);
	m_SubresourceData.pSysMem = pVertices;

	if (FAILED(m_pDevice->CreateBuffer(&m_BufferDesc, &m_SubresourceData, &m_pVB)))
		return E_FAIL;

	Safe_Delete_Array(pVertices);

	return S_OK;
}

CInstance_Mesh* CInstance_Mesh::Create_Binary(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, _fmatrix PivotMatrix, CInstance_Model* pModel, BINARYINFO& eModel)
{
	CInstance_Mesh* pInstance = new CInstance_Mesh(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype_Binary(PivotMatrix, pModel, eModel)))
	{
		MSG_BOX("Failed to Created : CMesh");
		Safe_Release(pInstance);
	}

	return pInstance;
}
CInstance_Mesh* CInstance_Mesh::Clone(void * pArg)
{
	CInstance_Mesh*		pInstance = new CInstance_Mesh(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CInstance_Mesh");
		Safe_Release(pInstance);
	}

	return pInstance;
}


void CInstance_Mesh::Free()
{
	__super::Free();

	Safe_Delete_Array(m_pInstanceVertices);
}
