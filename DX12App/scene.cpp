#include "stdafx.h"
#pragma once
#include "NetLib/NetModule.h"
#include "scene.h"
#include "UI.h"

Scene::Scene(SCENE_STAT stat, const XMFLOAT4& color, NetModule* netPtr)
	: mSceneState{ stat },
	  mFrameColor{ color },
	  mSceneChangeFlag{ SCENE_CHANGE_FLAG::NONE },
	  mNetPtr{ netPtr }
{
	mNetPtr->SetInterface(this);
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
