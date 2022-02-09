#pragma once

#include "d3dFramework.h"
#include "scene.h"
#include "UI.h"

class Camera;

class GameFramework : public D3DFramework
{
public:
	GameFramework();
	GameFramework(const GameFramework& rhs) = delete;
	GameFramework& operator=(const GameFramework& rhs) = delete;
	virtual ~GameFramework();

	virtual bool InitFramework() override;

private:
	virtual void OnResize() override;
	virtual void OnProcessMouseDown(WPARAM buttonState, int x, int y) override;
	virtual void OnProcessMouseUp(WPARAM buttonState, int x, int y) override;
	virtual void OnProcessMouseMove(WPARAM buttonState, int x, int y) override;
	virtual void OnProcessKeyInput(UINT uMsg, WPARAM wParam, LPARAM lParam) override;

	virtual void Update() override;
	void UIUpdate();

	virtual void Draw() override;

private:
	void InitScene(SCENE_STAT state);
	void OnPreciseKeyInput();
	
	void CheckAndChangeScene();

private:
	std::stack<std::unique_ptr<Scene>> mScenes;
};