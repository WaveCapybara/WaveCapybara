#pragma once

#include "VIBuffer_Instance.h"
#include "Instance_Model.h"

BEGIN(Engine)

class CInstance_Mesh final : public CVIBuffer_Instance
{
public:
	typedef struct tagMeshInstanceDesc
	{
		_bool		bOnePoint = { false };
		_bool		bBoundary = { false };

		_float3		vRange = { 1.f, 1.f, 1.f };

		_bool		bReduceSize = { false };
		_bool		bReduceAlpha = { false };

		_double		dLifeTimeMax = { 0.0 };
		_double		dLifeTimeAcc = { 0.0 };

		_uint		iTextureIndex = { 0 };
		_uint		iNumParticles = { 1 };

		_float		fMinSize = { 0.1f };
		_float		fMaxSize = { 0.1f };
		_float		fMinSpeed = { 0.1f };
		_float		fMaxSpeed = { 0.1f };
		_float3		vMinDir = { -1.f, -1.f, -1.f };
		_float3		vMaxDir = { 1.f, 1.f, 1.f };

		_bool		bGravity = { false };
		_float		fGravitySpeed = { 0.f };

		_bool		bRotation = { false };
		_float		fRotationSpeed = { 0.f };

		//CTransform* pTransform = { nullptr };

	}MESHINSTANCEDESC;
private:
	CInstance_Mesh(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CInstance_Mesh(const CInstance_Mesh& rhs);
	virtual ~CInstance_Mesh() = default;

public:
	_uint Get_MaterialIndex() const {
		return m_iMaterialIndex;
	}	

public:
	virtual HRESULT Initialize_Prototype_Binary(_fmatrix PivotMatrix, class CInstance_Model* pModel, BINARYINFO& eModel);

	virtual HRESULT Initialize(void* pArg) override;

	HRESULT Generate_Instance(void* pArg);
	HRESULT Load_InstanceInfo(_uint iNumInstance, MODELINSTANCE* pModleInstance);
	_float4 Get_InstPos(_uint iIndex) { return m_pInstanceVertices[iIndex].vTranslation; }

	HRESULT Generate_CullingInstance(list<_uint>* CullIndexList);

public:
	virtual void Tick(_double TimeDelta) override;

private:
	char					m_szName[MAX_PATH] = "";
	_uint					m_iMaterialIndex = { 0 };
	_double					m_iMovePos = { 0.0 };

	MODELINSTANCE*			m_pInstanceVertices = { nullptr };

private:		
	HRESULT Ready_VertexBuffer_NonAnim_Binary(_fmatrix PivotMatrix, BINARYINFO& eModel);

public:	
	static CInstance_Mesh* Create_Binary(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, _fmatrix PivotMatrix, class CInstance_Model* pModel, BINARYINFO& eModel);
	virtual CInstance_Mesh* Clone(void* pArg) override;
	virtual void Free() override;
};

END