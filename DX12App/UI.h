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
    //~UI();
    void UpdateLabels(const std::wstring& strUIText);
    void Draw(UINT nFrame);
    void ReleaseResources();
    void Resize(ID3D12Resource** ppd3dRenderTargets, UINT width, UINT height);

private:
    UINT GetRenderTargetsCount() { return static_cast<UINT>(m_vWrappedRenderTargets.size()); }
    void Initialize(ID3D12Device* pd3dDevice, ID3D12CommandQueue* pd3dCommandQueue);

    float m_fWidth;
    float m_fHeight;

    ComPtr<ID3D11DeviceContext> m_pd3d11DeviceContext;
    ComPtr<ID3D11On12Device> m_pd3d11On12Device;
    ComPtr<ID2D1Factory3> m_pd2dFactory;
    ComPtr<IDWriteFactory> m_pd2dWriteFactory;
    ComPtr<ID2D1Device2> m_pd2dDevice;
    ComPtr<ID3D11Device> pd3d11Device;
    ComPtr<IDXGIDevice> pdxgiDevice;
    //ID3D11On12Device*               m_pd3d11On12Device = NULL;
    //ID2D1Factory3*                  m_pd2dFactory = NULL;
    //IDWriteFactory*                 m_pd2dWriteFactory = NULL;
    //ID2D1Device2*                   m_pd2dDevice = NULL;
    ID2D1DeviceContext2* m_pd2dDeviceContext;
    ID2D1SolidColorBrush* m_pd2dTextBrush;
    IDWriteTextFormat* m_pdwTextFormat;

    std::vector<ID3D11Resource*>    m_vWrappedRenderTargets;
    std::vector<ID2D1Bitmap1*>      m_vd2dRenderTargets;
    std::vector<TextBlock>          m_vTextBlocks;
};

