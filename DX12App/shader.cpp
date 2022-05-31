#include "stdafx.h"
#include "shader.h"


Shader::Shader()
{
}

ComPtr<ID3DBlob> Shader::CompileShader(
	const std::wstring& fileName,
	const std::string& entry,
	const std::string& target,
	const D3D_SHADER_MACRO* defines)
{
	UINT compileFlags = 0;
#ifdef _DEBUG
	compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

	ComPtr<ID3DBlob> codeBlob;
	ComPtr<ID3DBlob> errorBlob;
	HRESULT hr = D3DCompileFromFile(fileName.c_str(), defines,
		D3D_COMPILE_STANDARD_FILE_INCLUDE,
		entry.c_str(), target.c_str(), compileFlags, NULL,
		&codeBlob, &errorBlob);


	if (errorBlob)
		OutputDebugStringA((char*)errorBlob->GetBufferPointer());

	ThrowIfFailed(hr);

	return codeBlob;
}

ComPtr<ID3DBlob> Shader::ReadCSOShader(const std::wstring& filename)
{
	if (!filename.empty())
	{
		ComPtr<ID3DBlob> codeBlob;
		HRESULT hr = D3DReadFileToBlob(filename.c_str(), &codeBlob);
		return codeBlob;
	}
	else
		return NULL;
}


///////////////////////////////////////////////////////////////////////
//
DefaultShader::DefaultShader(const std::wstring& path)
	: Shader()
{
	Compile(path);
	BuildInputLayout();
}

DefaultShader::DefaultShader(const std::wstring& vsPath, const std::wstring& psPath)
	: Shader()
{
	VS = ReadCSOShader(vsPath);
	PS = ReadCSOShader(psPath);

	BuildInputLayout();
}

void DefaultShader::Compile(const std::wstring& path)
{
	VS = Shader::CompileShader(path, "VS", "vs_5_1");
	PS = Shader::CompileShader(path, "PS", "ps_5_1");
}

void DefaultShader::BuildInputLayout()
{
	mInputLayout =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },

		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },

		{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 24,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },

		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 36,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};
}


///////////////////////////////////////////////////////////////////////////////////////////
//
TerrainShader::TerrainShader(const std::wstring& path)
	: Shader()
{
	Compile(path);
	BuildInputLayout();
}

void TerrainShader::Compile(const std::wstring& path)
{
	VS = Shader::CompileShader(path, "VS", "vs_5_1");
	PS = Shader::CompileShader(path, "PS", "ps_5_1");
	DS = Shader::CompileShader(path, "DS", "ds_5_1");
	HS = Shader::CompileShader(path, "HS", "hs_5_1");
}

void TerrainShader::BuildInputLayout()
{
	mInputLayout =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },

		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },

		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },

		{ "TEXCOORD", 1, DXGI_FORMAT_R32G32_FLOAT, 0, 32,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};
}


///////////////////////////////////////////////////////////////////////////////////////////
//
BillboardShader::BillboardShader(const std::wstring& path, bool soActive)
	: Shader(), mSOActive(soActive)
{
	Compile(path);
	BuildInputLayout();
}

BillboardShader::BillboardShader(const std::wstring& vspath, const std::wstring& gspath, const std::wstring& pspath, bool soActive)
	: Shader(), mSOActive(soActive)
{
	VS = ReadCSOShader(vspath);
	GS = ReadCSOShader(gspath);
	PS = ReadCSOShader(pspath);

	BuildInputLayout();
}

void BillboardShader::Compile(const std::wstring& path)
{
	if (mSOActive) {
		VS = Shader::CompileShader(path, "VSStreamOutput", "vs_5_1");
		GS = Shader::CompileShader(path, "GSStreamOutput", "gs_5_1");
	}
	else {
		VS = Shader::CompileShader(path, "VSRender", "vs_5_1");
		GS = Shader::CompileShader(path, "GSRender", "gs_5_1");
		PS = Shader::CompileShader(path, "PSRender", "ps_5_1");
	}
}

void BillboardShader::BuildInputLayout()
{
	mInputLayout =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },

		{ "SIZE", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },

		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 20,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },

		{ "VELOCITY", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 36,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },

		{ "LIFETIME", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 48,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			
		{ "ACCELERATION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 56,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },

		{ "TYPE", 0, DXGI_FORMAT_R32_UINT, 0, 68,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};
}


///////////////////////////////////////////////////////////////////////////////////////////
//
CubeMapShader::CubeMapShader(const std::wstring& path)
	: Shader()
{
	Compile(path);
	BuildInputLayout();
}

void CubeMapShader::Compile(const std::wstring& path)
{
	VS = Shader::CompileShader(path, "VS", "vs_5_1");
	PS = Shader::CompileShader(path, "PS", "ps_5_1");
}

void CubeMapShader::BuildInputLayout()
{
	mInputLayout =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },

		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};
}


///////////////////////////////////////////////////////////////////////////////////////////
//
ComputeShader::ComputeShader(const std::wstring& path)
{
	Compile(path);
}

ComputeShader::ComputeShader(const std::wstring& path, bool readCSO)
{
	if (readCSO)
		CS = ReadCSOShader(path);
	else
		Compile(path);
}

void ComputeShader::Compile(const std::wstring& path)
{
	CS = Shader::CompileShader(path, "CS", "cs_5_1");
}

///////////////////////////////////////////////////////////////////////////////////////////
//
ShadowShader::ShadowShader(const std::wstring& path) : Shader()
{
	Compile(path);
	BuildInputLayout();
}

void ShadowShader::Compile(const std::wstring& path)
{
	VS = Shader::CompileShader(path, "VS", "vs_5_1");
}

void ShadowShader::BuildInputLayout()
{
	mInputLayout =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },

		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },

		{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 24,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },

		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 36,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};
}

///////////////////////////////////////////////////////////////////////////////////////////
//
ShadowTerrainShader::ShadowTerrainShader(const std::wstring& path)
{
	Compile(path);
	BuildInputLayout();
}

void ShadowTerrainShader::Compile(const std::wstring& path)
{
	VS = Shader::CompileShader(path, "VS", "vs_5_1");
	DS = Shader::CompileShader(path, "DS", "ds_5_1");
	HS = Shader::CompileShader(path, "HS", "hs_5_1");
}

void ShadowTerrainShader::BuildInputLayout()
{
	mInputLayout =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },

		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },

		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },

		{ "TEXCOORD", 1, DXGI_FORMAT_R32G32_FLOAT, 0, 32,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};
}
