#pragma once

#include "Client_Defines.h"
#include "GameObject.h"

BEGIN(Engine)

class CInstance_Model;
class CShader;
class CRenderer;
class CTransform;

END

BEGIN(Client)

class CObject_Instance final : public CGameObject
{
public:
	typedef struct tagInstanceInfo
	{
		_tchar szModel_Name[MAX_PATH];
		_vector vPos;
		_float fScale;
		_float	fAngleX;
		_float	fAngleY;
		_float	fAngleZ;

		_int iNumInstance;
		_float3 vRange;
		_float3 vRandomScale;

		_bool is_Shaking;
		_float fShaking_Speed;
		_float fShaking_Range;

	}INSTANCEINFO;

	typedef struct tagLoadWideInstanceInfo
	{
		_tchar szModel_Name[MAX_PATH];
		_vector vPos;
		_float fScale;

		_int iNumInstance;
		MODELINSTANCE* tModelInstance;

		_bool is_Shaking;
		_float fShaking_Speed;
		_float fShaking_Range;

		_bool isCullOff = { false };

	}LOADWIDEINSTANCEINFO;

private:
	CObject_Instance(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CObject_Instance(const CObject_Instance& rhs);
	virtual ~CObject_Instance() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Tick(_double TimeDelta) override;
	virtual void LateTick(_double TimeDelta) override;
	virtual HRESULT Render() override;

private:
	CShader*				m_pShaderCom = { nullptr };	
	CRenderer*				m_pRendererCom = { nullptr };	
	CTransform*				m_pTransformCom = { nullptr };
	CInstance_Model*		m_pModelCom = { nullptr };
	
private:
	_float m_fMoveX = { 0.f };	
	_float m_fTimeDelta = { 0.f };
	_double m_dTimeAcc = { 0.0 };

	_bool m_isShaking = { false };
	_float m_fShaking_Speed = { 0.f };
	_float m_fShaking_Range = { 0.f };

	_bool m_isCullOff = { false };
private:
	list<_uint> m_isIn_IndexList;
	vector<_uint> m_vecCullingIndex;

private:
	HRESULT Add_Components(LOADWIDEINSTANCEINFO InstanceInfo);
	HRESULT SetUp_ShaderResources();

public:
	static CObject_Instance* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

END