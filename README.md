# DX11
DX11 Render.

# Learn
## Draw A Triangle.
### Prepare Shaders.
1. Prepare Shader Source. \
Create `Triangle_FS.hlsl`,`Triangle_VS.hlsl`,`Triangle.hlsli` in HLSL dic.
2. Create Shader. \
   In `GameApp::InitEffect();`.
   - Create Vertex Shader
   - Create Input Layout
   - Create Pixel Shader
3. Create Resource. \
   In `GameApp::InitResource();`.
   - Create buffer_desc.
   - Create sub resource data.
   - Create VertexBuffer
4. Draw
   In `GameApp::InitResource() && GameApp::DrawScene()`.\
   - Set
     - VertexBuffer
     - Primitive Topology
     - InputLayout
     - Shaders
   - Draw - `Context->Draw(xx,xx);`

## Draw A Cube.
### Constant Buffer && Index Buffer
- Constant Buffer 
  - Desc
    - `cbd.Usage = D3D11_USAGE_DYNAMIC`;
    - `cbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE`;
  - First 0.
  - Update using Map && UnMap - memcpy_s.
- Index Buffer