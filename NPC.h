#pragma once

#include "Client_Defines.h"
#include "GameObject.h"
#include "Transform.h"
#include "CollisionTrigger.h"
#include "Effect_Manager.h"

BEGIN(Engine)
class CModel;
class CShader;
class CRenderer;
class CTransform;
class CRigidBody;
END

BEGIN(Client)

class CNPC : public CGameObject
{
public:
	enum STATE_DISSOLVE { STATE_ON, STATE_ON_TO_OFF, STATE_OFF, STATE_OFF_TO_ON };

protected:
	CNPC(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CNPC(const CNPC& rhs);
	virtual ~CNPC() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Tick(_double TimeDelta) override;
	virtual void LateTick(_double TimeDelta) override;
	virtual HRESULT Render() override;	

public:
	CTransform* Get_Transform() {
		return m_pTransformCom;	}	
	_matrix	Get_World() {
		return m_pTransformCom->Get_WorldMatrix();	}
	INTERACTIONDESC Get_InteractionDesc() {
		return m_Interaction;
	}

protected:
	CRigidBody* m_pRigidBody = { nullptr };
	CTransform* m_pTransformCom = { nullptr };

protected:
	STATE_DISSOLVE	m_eState_Dissolve = { STATE_ON };
	INTERACTIONDESC	m_Interaction;	// Add 230907 Junseok

	_bool m_bDissolve = { false };	

	_float m_fPercent = { 0.f };
	_float m_fDistance_Dissolve = { 3.f };
	_float m_fRotationAngle = { 0.f };
	_float m_fDist = { 0.f };

	_float4 m_vTargetDir = { 0.f, 0.f, 0.f, 0.f };
	_bool	m_bCollision_OnOff = { false };	// Add 230907 Junseok

private:
	void Set_DissolveState(_double TimeDelta);
	void Set_Collision_OnOff(_bool _OnOff);	// Add 230907 Junseok


public:
	static CNPC* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

END