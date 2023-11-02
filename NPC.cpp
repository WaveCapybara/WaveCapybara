#include "stdafx.h"
#include "NPC.h"

#include "GameInstance.h"
#include "Camera_Free.h"

CNPC::CNPC(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
	: CGameObject(pDevice, pContext)
{

}

CNPC::CNPC(const CNPC & rhs)
	: CGameObject(rhs)
{

}

HRESULT CNPC::Initialize_Prototype()
{
	if (FAILED(__super::Initialize_Prototype()))
		return E_FAIL;

	return S_OK;
}

HRESULT CNPC::Initialize(void * pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;	

	return S_OK;
}

void CNPC::Tick(_double TimeDelta)
{
	__super::Tick(TimeDelta);	

	Set_DissolveState(TimeDelta);
}

void CNPC::LateTick(_double TimeDelta)
{
	__super::LateTick(TimeDelta);	

	GET_GAME;

	m_fDist = 0.f;

	CCamera_Free* pCamera = static_cast<CCamera_Free*>(pGameInstance->Get_GameObject(TEXT("Layer_Camera"), TEXT("Prototype_GameObject_Camera_Free")));

	if(nullptr != pCamera)
		m_fDist = LENGTH(pCamera->Get_TargetWorldPos() - GetPos);
	
	if (m_fDistance_Dissolve < m_fDist)
		m_bDissolve = true;

	else
		m_bDissolve = false;
}

HRESULT CNPC::Render()
{
	return S_OK;
}

void CNPC::Set_DissolveState(_double TimeDelta)
{
	switch (m_eState_Dissolve)
	{
	case Client::CNPC::STATE_ON:
	{
		m_bHide = false;

		if (true == m_bDissolve)
			m_eState_Dissolve = STATE_ON_TO_OFF;
	}
	break;

	case Client::CNPC::STATE_ON_TO_OFF:
	{
		m_fPercent += _float(TimeDelta * 2.0);

		if (1.f < m_fPercent)
		{
			m_fPercent = 1.f;
			m_eState_Dissolve = STATE_OFF;
			m_bHide = true;
		}

	}
	break;

	case Client::CNPC::STATE_OFF:
	{
		if (false == m_bDissolve)
		{
			m_eState_Dissolve = STATE_OFF_TO_ON;
			m_bHide = false;
		}
	}
	break;

	case Client::CNPC::STATE_OFF_TO_ON:
	{
		m_fPercent -= _float(TimeDelta * 2.0);

		if (0.f > m_fPercent)
		{
			m_fPercent = 0.f;
			m_eState_Dissolve = STATE_ON;

		}
	}
	break;

	default:
		break;
	}
}

void CNPC::Set_Collision_OnOff(_bool _OnOff)
{
	m_bCollision_OnOff = _OnOff;
}

CNPC * CNPC::Create(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
{
	CNPC*		pInstance = new CNPC(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CNPC");
		Safe_Release(pInstance);
	}

	return pInstance;
}
CGameObject * CNPC::Clone(void * pArg)
{
	CNPC*		pInstance = new CNPC(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CNPC");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CNPC::Free()
{
	__super::Free();
}
