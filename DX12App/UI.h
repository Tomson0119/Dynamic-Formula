struct TextBlock
{
    std::wstring        strText;
    D2D1_RECT_F         d2dLayoutRect;
    IDWriteTextFormat* pdwFormat;
};

class UI
{
    UI() {};
    ~UI() {};

};

class InGameUI
{
public:
    InGameUI(UINT nFrame, ID3D12Device* pd3dDevice, ID3D12CommandQueue* pd3dCommandQueue);
    ~InGameUI();
    void Update(const std::vector<std::wstring>& strUIText);
    void Draw(UINT nFrame);
    void ReleaseResources();
    void PreDraw(ID3D12Resource** ppd3dRenderTargets, UINT width, UINT height);
    void BuildBrush(UINT UI_Cnt, D2D1::ColorF* ColorList);
    void BuildSolidBrush(UINT UI_Cnt, D2D1::ColorF* ColorList);
    void BuildLinearGradientBrush();
    void Reset();
    void OnResize(ID3D12Resource** ppd3dRenderTargets, ID3D12Device* pd3dDevice, 
        ID3D12CommandQueue* pd3dCommandQueue, UINT nFrame, UINT width, UINT height);

    void SetVectorSize(UINT nFrame, UINT TextCnt);
    void SetTextRect();
    void SetDraftGage();

private:
    UINT GetRenderTargetsCount() { return static_cast<UINT>(mvWrappedRenderTargets.size()); }
    void Initialize(ID3D12Device* pd3dDevice, ID3D12CommandQueue* pd3dCommandQueue);

    float m_fWidth = 0.0f;
    float m_fHeight = 0.0f;

    float GradientPoint = 0.0f;

    UINT uItemCnt = 0;
    float fDraftGage = 0.0f;

    POINT DraftUIStart;
    POINT DraftUIEnd;

    UINT TextCnt;
    UINT UICnt;

    ComPtr<ID3D11DeviceContext> mpd3d11DeviceContext;
    ComPtr<ID3D11On12Device> mpd3d11On12Device;
    ComPtr<ID2D1Factory3> mpd2dFactory;
    ComPtr<IDWriteFactory> mpd2dWriteFactory;
    ComPtr<ID2D1Device2> mpd2dDevice;
    ComPtr<ID3D11Device> pd3d11Device;
    ComPtr<IDXGIDevice> pdxgiDevice;
    ComPtr<ID2D1DeviceContext2> mpd2dDeviceContext;
    std::vector<ComPtr<IDWriteTextFormat>> mvdwTextFormat;

    ComPtr<ID2D1LinearGradientBrush> md2dLinearGradientBrush;

    std::vector<ComPtr<ID2D1SolidColorBrush>> mvd2dTextBrush;

    std::vector<ComPtr<ID3D11Resource>>    mvWrappedRenderTargets;
    std::vector<ComPtr<ID2D1Bitmap1>>      mvd2dRenderTargets;
    std::vector<TextBlock>          mvTextBlocks;
};

