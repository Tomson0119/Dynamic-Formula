#include "stdafx.h"
#include "scene.h"

Scene::Scene(SCENE_STAT stat)
	: mSceneState{ stat },
	  mFrameColor{ (XMFLOAT4)Colors::Aqua }
{
}

void Scene::OnProcessMouseDown(HWND hwnd, WPARAM btnState, int x, int y)
{
}

void Scene::OnProcessMouseUp(WPARAM btnState, int x, int y)
{
}

void Scene::OnProcessMouseMove(WPARAM btnState, int x, int y)
{
}

void Scene::OnProcessKeyInput(UINT msg, WPARAM wParam, LPARAM lParam)
{
}
