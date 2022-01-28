#pragma once

#include "scene.h"

class LoginScene : public Scene
{
public:
	LoginScene();
	virtual ~LoginScene() = default;

public:
	virtual void BuildObjects(
		ID3D12Device* device,
		ID3D12GraphicsCommandList* cmdList,
		float aspect,
		std::shared_ptr<btDiscreteDynamicsWorld>& dynamicWorld) override;

	virtual void Update(
		ID3D12Device* device,
		ID3D12GraphicsCommandList* cmdList,
		const GameTimer& timer,
		std::shared_ptr<btDiscreteDynamicsWorld>& dynamicWorld) override;

	virtual void Draw(ID3D12GraphicsCommandList* cmdList, ID3D12Resource* backBuffer) override;

private:

};