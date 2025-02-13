namespace SimpleApp.WinUI3 {
    using System.IO;
    using System;

    public sealed partial class MainWindow_6_Dx : Microsoft.UI.Xaml.Window {
        private SharpDX.Direct3D11.Device device;
        private SharpDX.Direct3D11.Texture2D renderTexture;
        private SharpDX.Direct3D11.RenderTargetView renderTargetView;
        private SharpDX.Direct3D11.VertexShader vertexShader;
        private SharpDX.Direct3D11.PixelShader pixelShader;
        private SharpDX.Direct3D11.InputLayout inputLayout;
        private SharpDX.Direct3D11.Buffer vertexBuffer;

        public MainWindow_6_Dx() {
            try {
                this.InitializeComponent();
                InitializeDirectX();
                Render();
                SaveTextureToFile();
            }
            catch (System.Exception ex) {
                System.Diagnostics.Debug.WriteLine($"Exception: {ex}");
                throw;
            }
        }

        private void InitializeDirectX() {
            device = new SharpDX.Direct3D11.Device(SharpDX.Direct3D.DriverType.Hardware, SharpDX.Direct3D11.DeviceCreationFlags.BgraSupport);

            // Создание текстуры
            SharpDX.Direct3D11.Texture2DDescription textureDesc = new SharpDX.Direct3D11.Texture2DDescription {
                Width = 512,
                Height = 512,
                MipLevels = 1,
                ArraySize = 1,
                Format = SharpDX.DXGI.Format.R8G8B8A8_UNorm,
                SampleDescription = new SharpDX.DXGI.SampleDescription(1, 0),
                Usage = SharpDX.Direct3D11.ResourceUsage.Default,
                BindFlags = SharpDX.Direct3D11.BindFlags.RenderTarget | SharpDX.Direct3D11.BindFlags.ShaderResource,
                CpuAccessFlags = SharpDX.Direct3D11.CpuAccessFlags.None,
                OptionFlags = SharpDX.Direct3D11.ResourceOptionFlags.None
            };

            renderTexture = new SharpDX.Direct3D11.Texture2D(device, textureDesc);
            renderTargetView = new SharpDX.Direct3D11.RenderTargetView(device, renderTexture);

            string defaultCommonShaderPath = Path.Combine(AppDomain.CurrentDomain.BaseDirectory, "DefaultCommonShader.hlsl");
            if (!System.IO.File.Exists(defaultCommonShaderPath)) {
                throw new FileNotFoundException($"Shader file not found: {defaultCommonShaderPath}");
            }

            // Компиляция шейдеров
            var vertexShaderByteCode = SharpDX.D3DCompiler.ShaderBytecode.CompileFromFile(defaultCommonShaderPath, "VSMain", "vs_5_0");
            var pixelShaderByteCode = SharpDX.D3DCompiler.ShaderBytecode.CompileFromFile(defaultCommonShaderPath, "PSMain", "ps_5_0");

            vertexShader = new SharpDX.Direct3D11.VertexShader(device, vertexShaderByteCode);
            pixelShader = new SharpDX.Direct3D11.PixelShader(device, pixelShaderByteCode);

            // Определение входного макета
            var layout = new[]
            {
                new SharpDX.Direct3D11.InputElement("POSITION", 0, SharpDX.DXGI.Format.R32G32B32_Float, 0, 0),
                new SharpDX.Direct3D11.InputElement("TEXCOORD", 0, SharpDX.DXGI.Format.R32G32_Float, 12, 0)
            };
            inputLayout = new SharpDX.Direct3D11.InputLayout(device, SharpDX.D3DCompiler.ShaderSignature.GetInputSignature(vertexShaderByteCode), layout);


            // Создание вершинного буфера
            var vertices = new[]
            {
                new Vertex { Position = new SharpDX.Mathematics.Interop.RawVector3(-1.0f, -1.0f, 0.0f), TexCoord = new SharpDX.Mathematics.Interop.RawVector2(0.0f, 1.0f) }, // 1
                new Vertex { Position = new SharpDX.Mathematics.Interop.RawVector3(-1.0f,  1.0f, 0.0f), TexCoord = new SharpDX.Mathematics.Interop.RawVector2(0.0f, 0.0f) }, // 2
                new Vertex { Position = new SharpDX.Mathematics.Interop.RawVector3( 1.0f, -1.0f, 0.0f), TexCoord = new SharpDX.Mathematics.Interop.RawVector2(1.0f, 1.0f) }, // 3

                new Vertex { Position = new SharpDX.Mathematics.Interop.RawVector3( 1.0f, -1.0f, 0.0f), TexCoord = new SharpDX.Mathematics.Interop.RawVector2(1.0f, 1.0f) }, // 3
                new Vertex { Position = new SharpDX.Mathematics.Interop.RawVector3(-1.0f,  1.0f, 0.0f), TexCoord = new SharpDX.Mathematics.Interop.RawVector2(0.0f, 0.0f) }, // 2
                new Vertex { Position = new SharpDX.Mathematics.Interop.RawVector3( 1.0f,  1.0f, 0.0f), TexCoord = new SharpDX.Mathematics.Interop.RawVector2(1.0f, 0.0f) }, // 4
            };
            vertexBuffer = SharpDX.Direct3D11.Buffer.Create(device, SharpDX.Direct3D11.BindFlags.VertexBuffer, vertices);

            if (vertexShaderByteCode.Bytecode == null) {
                throw new Exception("Ошибка компиляции вершинного шейдера!");
            }
            if (pixelShaderByteCode.Bytecode == null) {
                throw new Exception("Ошибка компиляции пиксельного шейдера!");
            }
        }

        private void Render() {
            var context = device.ImmediateContext;

            // Устанавливаем RenderTarget
            context.OutputMerger.SetRenderTargets(renderTargetView);
            context.ClearRenderTargetView(renderTargetView, new SharpDX.Mathematics.Interop.RawColor4(0, 0.5f, 0.5f, 1));

            // Устанавливаем Input Layout
            context.InputAssembler.InputLayout = inputLayout;
            context.InputAssembler.PrimitiveTopology = SharpDX.Direct3D.PrimitiveTopology.TriangleList;
            context.InputAssembler.SetVertexBuffers(0, new SharpDX.Direct3D11.VertexBufferBinding(vertexBuffer, SharpDX.Utilities.SizeOf<Vertex>(), 0));

            // Устанавливаем шейдеры
            context.VertexShader.Set(vertexShader);
            context.PixelShader.Set(pixelShader);

            // Устанавливаем Viewport
            SharpDX.Mathematics.Interop.RawViewportF viewport = new SharpDX.Mathematics.Interop.RawViewportF {
                X = 0,
                Y = 0,
                Width = 512,
                Height = 512,
                MinDepth = 0.0f,
                MaxDepth = 1.0f
            };
            device.ImmediateContext.Rasterizer.SetViewport(viewport);

            // Рендерим
            System.Diagnostics.Debug.WriteLine("Rendering...");
            context.Draw(6, 0);

            // Принудительно сбрасываем команды
            context.Flush();
        }

        private void SaveTextureToFile() {
            string filename = Windows.Storage.ApplicationData.Current.LocalFolder.Path + "\\output.png";

            using (var factory = new SharpDX.WIC.ImagingFactory2())
            using (var stream = new SharpDX.WIC.WICStream(factory, filename, SharpDX.IO.NativeFileAccess.Write))
            using (var encoder = new SharpDX.WIC.PngBitmapEncoder(factory)) {
                encoder.Initialize(stream);

                using (var frame = new SharpDX.WIC.BitmapFrameEncode(encoder)) {
                    frame.Initialize(); // ВАЖНО: инициализация кадра
                    frame.SetSize(512, 512);

                    var pixelFormat = SharpDX.WIC.PixelFormat.Format32bppBGRA;
                    frame.SetPixelFormat(ref pixelFormat);

                    using (var converter = new SharpDX.WIC.FormatConverter(factory))
                    using (var stagingTexture = new SharpDX.Direct3D11.Texture2D(device, new SharpDX.Direct3D11.Texture2DDescription {
                        Width = 512,
                        Height = 512,
                        MipLevels = 1,
                        ArraySize = 1,
                        Format = SharpDX.DXGI.Format.R8G8B8A8_UNorm,
                        SampleDescription = new SharpDX.DXGI.SampleDescription(1, 0),
                        Usage = SharpDX.Direct3D11.ResourceUsage.Staging,
                        CpuAccessFlags = SharpDX.Direct3D11.CpuAccessFlags.Read,
                        BindFlags = SharpDX.Direct3D11.BindFlags.None,
                        OptionFlags = SharpDX.Direct3D11.ResourceOptionFlags.None
                    })) {
                        device.ImmediateContext.CopyResource(renderTexture, stagingTexture);

                        var dataBox = device.ImmediateContext.MapSubresource(stagingTexture, 0, SharpDX.Direct3D11.MapMode.Read, SharpDX.Direct3D11.MapFlags.None);
                        using (var bitmap = new SharpDX.WIC.Bitmap(factory, 512, 512, SharpDX.WIC.PixelFormat.Format32bppBGRA, new SharpDX.DataRectangle(dataBox.DataPointer, dataBox.RowPitch))) {
                            converter.Initialize(bitmap, SharpDX.WIC.PixelFormat.Format32bppBGRA);
                            frame.WriteSource(converter); // Записываем данные
                            frame.Commit(); // Завершаем кадр
                            encoder.Commit(); // Завершаем запись в файл
                        }
                        device.ImmediateContext.UnmapSubresource(stagingTexture, 0);
                    }
                }
            }
        }

        private struct Vertex {
            public SharpDX.Mathematics.Interop.RawVector3 Position;
            public SharpDX.Mathematics.Interop.RawVector2 TexCoord;
        }
    }
}