#include "stdafx.h"
#include "../Public/Object_Instance.h"
#include "GameInstance.h"
#include "Instance_Mesh.h"
#include "Instance_Model.h"
#include "Camera_Free.h"

CObject_Instance::CObject_Instance(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
	: CGameObject(pDevice, pContext)
{

}

CObject_Instance::CObject_Instance(const CObject_Instance & rhs)
	: CGameObject(rhs)
{

}

HRESULT CObject_Instance::Initialize_Prototype()
{
	if (FAILED(__super::Initialize_Prototype()))
		return E_FAIL;	

	return S_OK;
}

HRESULT CObject_Instance::Initialize(void * pArg)
{
	if (nullptr == pArg)
	{
		MSG_BOX("Create ObjectGuide_Instance Failed : Argument is nullptr!");
		return E_FAIL;
	}

	CObject_Instance::LOADWIDEINSTANCEINFO InvirormentInfo;
	ZeroMemory(&InvirormentInfo, sizeof CObject_Instance::LOADWIDEINSTANCEINFO);
	memcpy(&InvirormentInfo, pArg, sizeof CObject_Instance::LOADWIDEINSTANCEINFO);

	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Add_Components(InvirormentInfo)))
		return E_FAIL;

	m_pTransformCom->Set_State(CTransform::STATE_POSITION, InvirormentInfo.vPos);
	m_pTransformCom->Scaling(_float3(InvirormentInfo.fScale, InvirormentInfo.fScale, InvirormentInfo.fScale));

	m_isShaking = InvirormentInfo.is_Shaking;

	if (true == m_isShaking)
	{
		m_fShaking_Speed = InvirormentInfo.fShaking_Speed;
		m_fShaking_Range = InvirormentInfo.fShaking_Range;
	}

	m_isCullOff = InvirormentInfo.isCullOff;

	return S_OK;
}

void CObject_Instance::Tick(_double TimeDelta)
{	
	__super::Tick(TimeDelta);

	if (true == m_isShaking)
	{
		m_fTimeDelta = _float(TimeDelta);
		m_dTimeAcc += TimeDelta;
		m_fMoveX += cosf(_float(m_dTimeAcc * m_fShaking_Speed));
	}	
}

void CObject_Instance::LateTick(_double TimeDelta)
{
	__super::LateTick(TimeDelta);

	GET_GAME;

	if (false == m_isCullOff)
	{
		CCamera_Free* pCamera = static_cast<CCamera_Free*>(pGameInstance->Get_GameObject(TEXT("Layer_Camera"), TEXT("Prototype_GameObject_Camera_Free")));
		_vector vPlayerPos = pCamera->Get_TargetWorldPos();

		_vector vDir = GetPos - vPlayerPos;

		_float fDist = XMVectorGetX(XMVector3Length(vDir));

		if (128.f < fDist)
			return;
	}

	_bool isIn = { false };
	_uint iNumMeshes = m_pModelCom->Get_NumMeshes();	
	_matrix matWorld = m_pTransformCom->Get_WorldMatrix();

	for (_uint i = 0; i < iNumMeshes; ++i)
	{
		CInstance_Mesh* pInstanceMesh = m_pModelCom->Get_InstanceMesh(i);

		_uint iNumInstance = pInstanceMesh->Get_NumInstance();

		for (_uint j = 0; j < iNumInstance; ++j)
		{
			_vector vInstPos = XMVector3TransformCoord(XMLoadFloat4(&pInstanceMesh->Get_InstPos(j)), matWorld);
			isIn = pGameInstance->isIn_WorldSpace(vInstPos, 1.5f);
			
			if (true == isIn)
				m_isIn_IndexList.push_back(j);
		}

		if (m_isIn_IndexList.empty())
		{
			m_isIn_IndexList.clear();
			continue;
		}

		if(FAILED(m_pModelCom->Generate_CullingInstance(i, &m_isIn_IndexList)))
		{
			MSG_BOX("InstanceCulling Fail!!!");
			return;
		}	
	}	

	if (!m_isIn_IndexList.empty())
	{
		if (FAILED(m_pRendererCom->Add_RenderGroup(CRenderer::RENDER_NONBLEND, this)))
			return;
	}

	m_isIn_IndexList.clear();

}

HRESULT CObject_Instance::Render()
{
	if (FAILED(__super::Render()))
		return E_FAIL;

	if (FAILED(SetUp_ShaderResources()))
		return E_FAIL;

	_uint iNumMeshes = m_pModelCom->Get_NumMeshes();

	for (_uint i = 0; i < iNumMeshes; ++i)
	{		
		if (0 != m_pModelCom->Get_InstanceMesh(i)->Get_NumInstance_Cull())
		{
			if (FAILED(m_pModelCom->Bind_ShaderResource(m_pShaderCom, i, "g_DiffuseTexture", aiTextureType_DIFFUSE)))
				return E_FAIL;

			if (FAILED(m_pModelCom->Bind_ShaderResource(m_pShaderCom, i, "g_NormalTexture", aiTextureType_NORMALS)))
				return E_FAIL;

			if (true == m_isShaking)
				m_pShaderCom->Begin(1);

			else if (false == m_isShaking)
				m_pShaderCom->Begin(0);

			m_pModelCom->Render_Culling(i);
		}
	}

	return S_OK;
}

HRESULT CObject_Instance::Add_Components(CObject_Instance::LOADWIDEINSTANCEINFO InstanceInfo)
{
	/* For.Com_Transform */
	if (FAILED(__super::Add_Component(PROTO_STATIC, TEXT("Prototype_Component_Transform"),
		TEXT("Com_Transform"), (CComponent**)&m_pTransformCom)))
		return E_FAIL;

	/* For.Com_Renderer */
	if (FAILED(__super::Add_Component(PROTO_STATIC, TEXT("Prototype_Component_Renderer"),
		TEXT("Com_Renderer"), (CComponent**)&m_pRendererCom)))		
		return E_FAIL;

	/* For.Com_Shader */
	if (FAILED(__super::Add_Component(PROTO_STATIC, TEXT("Prototype_Component_Shader_Modelnstance"),
		TEXT("Com_Shader"), (CComponent**)&m_pShaderCom)))
		return E_FAIL;	

	/* For.Com_Model */
	if (FAILED(__super::Add_Component(PROTO_STATIC, InstanceInfo.szModel_Name,
		TEXT("Com_Instance_Model"), (CComponent**)&m_pModelCom)))
		return E_FAIL;

	m_pModelCom->Load_InstanceInfo(InstanceInfo.iNumInstance, InstanceInfo.tModelInstance);

	return S_OK;
}

HRESULT CObject_Instance::SetUp_ShaderResources()
{
	if (nullptr == m_pShaderCom)
		return E_FAIL;	

	if (FAILED(m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix")))
		return E_FAIL;

	CGameInstance*		pGameInstance = CGameInstance::GetInstance();
	Safe_AddRef(pGameInstance);

	_float4x4 matView, matProj;

	matView = pGameInstance->Get_TransformFloat4x4(CPipeLine::D3DTS_VIEW);
	matProj = pGameInstance->Get_TransformFloat4x4(CPipeLine::D3DTS_PROJ);

	if (FAILED(m_pShaderCom->SetUp_Matrix("g_ViewMatrix", &matView)))
		return E_FAIL;
	if (FAILED(m_pShaderCom->SetUp_Matrix("g_ProjMatrix", &matProj)))
		return E_FAIL;	
	
	if (true == m_isShaking)
	{
		if (FAILED(m_pShaderCom->SetUp_RawValue("g_MoveX", &m_fMoveX, sizeof(_float))))
			return E_FAIL;

		if (FAILED(m_pShaderCom->SetUp_RawValue("g_ShakingRange", &m_fShaking_Range, sizeof(_float))))
			return E_FAIL;

		if (FAILED(m_pShaderCom->SetUp_RawValue("g_TimeDelta", &m_fTimeDelta, sizeof(_float))))
			return E_FAIL;
	}

	Safe_Release(pGameInstance);

	return S_OK;
}

CObject_Instance * CObject_Instance::Create(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
{
	CObject_Instance*		pInstance = new CObject_Instance(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CObject_Instance");
		Safe_Release(pInstance);
	}

	return pInstance;
}
CGameObject * CObject_Instance::Clone(void * pArg)
{
	CObject_Instance*		pInstance = new CObject_Instance(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CObject_Instance");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CObject_Instance::Free()
{
	__super::Free();

	Safe_Release(m_pTransformCom);
	Safe_Release(m_pRendererCom);
	Safe_Release(m_pShaderCom);
	Safe_Release(m_pModelCom);
}
