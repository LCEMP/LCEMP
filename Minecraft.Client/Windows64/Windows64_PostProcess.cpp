#include "stdafx.h"
#include "Windows64_PostProcess.h"
#include <d3dcompiler.h>
#pragma comment(lib, "d3dcompiler.lib")

extern ID3D11Device*           g_pd3dDevice;
extern ID3D11DeviceContext*    g_pImmediateContext;
extern IDXGISwapChain*         g_pSwapChain;
extern ID3D11RenderTargetView* g_pRenderTargetView;

static ID3D11Texture2D*          g_pGammaOffscreenTex = NULL;
static ID3D11ShaderResourceView* g_pGammaOffscreenSRV = NULL;
static ID3D11RenderTargetView*   g_pGammaOffscreenRTV = NULL;
static ID3D11VertexShader*       g_pGammaVS = NULL;
static ID3D11PixelShader*        g_pGammaPS = NULL;
static ID3D11Buffer*             g_pGammaCB = NULL;
static ID3D11SamplerState*       g_pGammaSampler = NULL;
static ID3D11RasterizerState*    g_pGammaRastState = NULL;
static ID3D11DepthStencilState*  g_pGammaDepthState = NULL;
static ID3D11BlendState*         g_pGammaBlendState = NULL;
static bool                      g_gammaPostProcessReady = false;
static float                     g_gammaValue = 1.0f;

struct GammaCBData
{
	float gamma;
	float pad[3];
};

static const char* g_gammaVSCode =
	"void main(uint id : SV_VertexID, out float4 pos : SV_Position, out float2 uv : TEXCOORD0)\n"
	"{\n"
	"    uv = float2((id << 1) & 2, id & 2);\n"
	"    pos = float4(uv * float2(2.0, -2.0) + float2(-1.0, 1.0), 0.0, 1.0);\n"
	"}\n";

static const char* g_gammaPSCode =
	"cbuffer GammaCB : register(b0)\n"
	"{\n"
	"    float gamma;\n"
	"    float3 pad;\n"
	"};\n"
	"Texture2D sceneTex : register(t0);\n"
	"SamplerState sceneSampler : register(s0);\n"
	"float4 main(float4 pos : SV_Position, float2 uv : TEXCOORD0) : SV_Target\n"
	"{\n"
	"    float4 color = sceneTex.Sample(sceneSampler, uv);\n"
	"    color.rgb = pow(max(color.rgb, 0.0), 1.0 / gamma);\n"
	"    return color;\n"
	"}\n";


void SetGammaValue(float gamma)
{
	g_gammaValue = gamma;
}

bool InitGammaPostProcess()
{
	if (!g_pd3dDevice || !g_pSwapChain) return false;

	HRESULT hr;

	ID3D11Texture2D* pBackBuffer = NULL;
	hr = g_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);
	if (FAILED(hr)) return false;

	D3D11_TEXTURE2D_DESC bbDesc;
	pBackBuffer->GetDesc(&bbDesc);
	pBackBuffer->Release();

	D3D11_TEXTURE2D_DESC texDesc;
	ZeroMemory(&texDesc, sizeof(texDesc));
	texDesc.Width = bbDesc.Width;
	texDesc.Height = bbDesc.Height;
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 1;
	texDesc.Format = bbDesc.Format;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	hr = g_pd3dDevice->CreateTexture2D(&texDesc, NULL, &g_pGammaOffscreenTex);
	if (FAILED(hr)) return false;

	hr = g_pd3dDevice->CreateShaderResourceView(g_pGammaOffscreenTex, NULL, &g_pGammaOffscreenSRV);
	if (FAILED(hr)) return false;

	hr = g_pd3dDevice->CreateRenderTargetView(g_pGammaOffscreenTex, NULL, &g_pGammaOffscreenRTV);
	if (FAILED(hr)) return false;

	ID3DBlob* vsBlob = NULL;
	ID3DBlob* errBlob = NULL;
	hr = D3DCompile(g_gammaVSCode, strlen(g_gammaVSCode), "GammaVS", NULL, NULL, "main", "vs_4_0", 0, 0, &vsBlob, &errBlob);
	if (FAILED(hr))
	{
		if (errBlob) errBlob->Release();
		return false;
	}
	hr = g_pd3dDevice->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), NULL, &g_pGammaVS);
	vsBlob->Release();
	if (errBlob) errBlob->Release();
	if (FAILED(hr)) return false;

	errBlob = NULL;
	ID3DBlob* psBlob = NULL;
	hr = D3DCompile(g_gammaPSCode, strlen(g_gammaPSCode), "GammaPS", NULL, NULL, "main", "ps_4_0", 0, 0, &psBlob, &errBlob);
	if (FAILED(hr))
	{
		if (errBlob) errBlob->Release();
		return false;
	}
	hr = g_pd3dDevice->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), NULL, &g_pGammaPS);
	psBlob->Release();
	if (errBlob) errBlob->Release();
	if (FAILED(hr)) return false;

	D3D11_BUFFER_DESC cbDesc;
	ZeroMemory(&cbDesc, sizeof(cbDesc));
	cbDesc.ByteWidth = sizeof(GammaCBData);
	cbDesc.Usage = D3D11_USAGE_DYNAMIC;
	cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	GammaCBData initData = { 1.0f, {0, 0, 0} };
	D3D11_SUBRESOURCE_DATA srData;
	srData.pSysMem = &initData;
	srData.SysMemPitch = 0;
	srData.SysMemSlicePitch = 0;
	hr = g_pd3dDevice->CreateBuffer(&cbDesc, &srData, &g_pGammaCB);
	if (FAILED(hr)) return false;

	D3D11_SAMPLER_DESC sampDesc;
	ZeroMemory(&sampDesc, sizeof(sampDesc));
	sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	hr = g_pd3dDevice->CreateSamplerState(&sampDesc, &g_pGammaSampler);
	if (FAILED(hr)) return false;

	D3D11_RASTERIZER_DESC rasDesc;
	ZeroMemory(&rasDesc, sizeof(rasDesc));
	rasDesc.FillMode = D3D11_FILL_SOLID;
	rasDesc.CullMode = D3D11_CULL_NONE;
	rasDesc.DepthClipEnable = FALSE;
	hr = g_pd3dDevice->CreateRasterizerState(&rasDesc, &g_pGammaRastState);
	if (FAILED(hr)) return false;

	// Depth stencil state (disabled)
	D3D11_DEPTH_STENCIL_DESC dsDesc;
	ZeroMemory(&dsDesc, sizeof(dsDesc));
	dsDesc.DepthEnable = FALSE;
	dsDesc.StencilEnable = FALSE;
	hr = g_pd3dDevice->CreateDepthStencilState(&dsDesc, &g_pGammaDepthState);
	if (FAILED(hr)) return false;

	D3D11_BLEND_DESC blendDesc;
	ZeroMemory(&blendDesc, sizeof(blendDesc));
	blendDesc.RenderTarget[0].BlendEnable = FALSE;
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	hr = g_pd3dDevice->CreateBlendState(&blendDesc, &g_pGammaBlendState);
	if (FAILED(hr)) return false;

	g_gammaPostProcessReady = true;
	return true;
}

void ApplyGammaPostProcess()
{
	if (!g_gammaPostProcessReady) return;

	if (g_gammaValue > 0.99f && g_gammaValue < 1.01f) return;

	ID3D11DeviceContext* ctx = g_pImmediateContext;

	ID3D11Texture2D* pBackBuffer = NULL;
	HRESULT hr = g_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);
	if (FAILED(hr)) return;
	ctx->CopyResource(g_pGammaOffscreenTex, pBackBuffer);

	D3D11_MAPPED_SUBRESOURCE mapped;
	hr = ctx->Map(g_pGammaCB, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
	if (SUCCEEDED(hr))
	{
		GammaCBData* cb = (GammaCBData*)mapped.pData;
		cb->gamma = g_gammaValue;
		ctx->Unmap(g_pGammaCB, 0);
	}

	ID3D11RenderTargetView* oldRTV = NULL;
	ID3D11DepthStencilView* oldDSV = NULL;
	ctx->OMGetRenderTargets(1, &oldRTV, &oldDSV);

	UINT numViewports = 1;
	D3D11_VIEWPORT oldViewport;
	ctx->RSGetViewports(&numViewports, &oldViewport);

	ID3D11RenderTargetView* bbRTV = g_pRenderTargetView;
	ctx->OMSetRenderTargets(1, &bbRTV, NULL);

	// Set viewport to full screen
	D3D11_TEXTURE2D_DESC bbDesc;
	pBackBuffer->GetDesc(&bbDesc);
	pBackBuffer->Release();

	D3D11_VIEWPORT vp;
	vp.Width = (FLOAT)bbDesc.Width;
	vp.Height = (FLOAT)bbDesc.Height;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	ctx->RSSetViewports(1, &vp);

	ctx->IASetInputLayout(NULL);
	ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	ctx->VSSetShader(g_pGammaVS, NULL, 0);
	ctx->PSSetShader(g_pGammaPS, NULL, 0);
	ctx->PSSetShaderResources(0, 1, &g_pGammaOffscreenSRV);
	ctx->PSSetSamplers(0, 1, &g_pGammaSampler);
	ctx->PSSetConstantBuffers(0, 1, &g_pGammaCB);
	ctx->RSSetState(g_pGammaRastState);
	ctx->OMSetDepthStencilState(g_pGammaDepthState, 0);
	float blendFactor[4] = { 0, 0, 0, 0 };
	ctx->OMSetBlendState(g_pGammaBlendState, blendFactor, 0xFFFFFFFF);

	ctx->Draw(3, 0);

	ID3D11ShaderResourceView* nullSRV = NULL;
	ctx->PSSetShaderResources(0, 1, &nullSRV);

	ctx->OMSetRenderTargets(1, &oldRTV, oldDSV);
	ctx->RSSetViewports(1, &oldViewport);
	if (oldRTV) oldRTV->Release();
	if (oldDSV) oldDSV->Release();
}

void CleanupGammaPostProcess()
{
	if (g_pGammaBlendState)    { g_pGammaBlendState->Release();    g_pGammaBlendState = NULL; }
	if (g_pGammaDepthState)    { g_pGammaDepthState->Release();    g_pGammaDepthState = NULL; }
	if (g_pGammaRastState)     { g_pGammaRastState->Release();     g_pGammaRastState = NULL; }
	if (g_pGammaSampler)       { g_pGammaSampler->Release();       g_pGammaSampler = NULL; }
	if (g_pGammaCB)            { g_pGammaCB->Release();            g_pGammaCB = NULL; }
	if (g_pGammaPS)            { g_pGammaPS->Release();            g_pGammaPS = NULL; }
	if (g_pGammaVS)            { g_pGammaVS->Release();            g_pGammaVS = NULL; }
	if (g_pGammaOffscreenRTV)  { g_pGammaOffscreenRTV->Release();  g_pGammaOffscreenRTV = NULL; }
	if (g_pGammaOffscreenSRV)  { g_pGammaOffscreenSRV->Release();  g_pGammaOffscreenSRV = NULL; }
	if (g_pGammaOffscreenTex)  { g_pGammaOffscreenTex->Release();  g_pGammaOffscreenTex = NULL; }
	g_gammaPostProcessReady = false;
}
