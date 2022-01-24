struct TextBlock
{
    std::wstring        strText;
    D2D1_RECT_F         d2dLayoutRect;
    IDWriteTextFormat* pdwFormat;
};

class UI
{
public:
    UI(UINT nFrame, ID3D12Device* pd3dDevice, ID3D12CommandQueue* pd3dCommandQueue);
    ~UI();
    void Update(const std::vector<std::wstring>& strUIText);
    void Draw(UINT nFrame);
    void ReleaseResources();
    void PreDraw(ID3D12Resource** ppd3dRenderTargets, UINT width, UINT height);
    void BuildBrush(UINT UI_Cnt, D2D1::ColorF* ColorList);
    void BuildSolidBrush(UINT UI_Cnt, D2D1::ColorF* ColorList);
    void BuildLinearBrush();
    void Reset();
    void OnResize(ID3D12Resource** ppd3dRenderTargets, ID3D12Device* pd3dDevice, 
        ID3D12CommandQueue* pd3dCommandQueue, UINT nFrame, UINT width, UINT height);

    void SetVectorSize(UINT nFrame, UINT TextCnt);
    void SetTextRect();
    void SetDraftGage();

private:
    UINT GetRenderTargetsCount() { return static_cast<UINT>(m_vWrappedRenderTargets.size()); }
    void Initialize(ID3D12Device* pd3dDevice, ID3D12CommandQueue* pd3dCommandQueue);

    float m_fWidth = 0.0f;
    float m_fHeight = 0.0f;

    float GradientPoint = 0.0f;

    UINT uItemCnt = 0;
    float fDraftGage = 0.0f;

    POINT DraftUIStart;
    POINT DraftUIEnd;

    UINT TextCnt;

    ComPtr<ID3D11DeviceContext> m_pd3d11DeviceContext;
    ComPtr<ID3D11On12Device> m_pd3d11On12Device;
    ComPtr<ID2D1Factory3> m_pd2dFactory;
    ComPtr<IDWriteFactory> m_pd2dWriteFactory;
    ComPtr<ID2D1Device2> m_pd2dDevice;
    ComPtr<ID3D11Device> pd3d11Device;
    ComPtr<IDXGIDevice> pdxgiDevice;
    ComPtr<ID2D1DeviceContext2> m_pd2dDeviceContext;
    std::vector<ComPtr<IDWriteTextFormat>> m_vdwTextFormat;

    ComPtr<ID2D1LinearGradientBrush> m_d2dLinearGradientBrush;

    std::vector<ComPtr<ID2D1SolidColorBrush>> m_vd2dTextBrush;

    std::vector<ComPtr<ID3D11Resource>>    m_vWrappedRenderTargets;
    std::vector<ComPtr<ID2D1Bitmap1>>      m_vd2dRenderTargets;
    std::vector<TextBlock>          m_vTextBlocks;
};

