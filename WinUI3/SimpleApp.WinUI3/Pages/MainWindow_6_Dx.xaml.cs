
namespace SimpleApp.WinUI3 {
    namespace Helpers {
        namespace Dx {
            public class Dx {
                static public void SaveTextureToFile(SharpDX.Direct3D11.Device d3dDevice, SharpDX.Direct3D11.Texture2D texture, string name) {
                    string filename = Windows.Storage.ApplicationData.Current.LocalFolder.Path + $"\\{name}.png";

                    using (var factory = new SharpDX.WIC.ImagingFactory2())
                    using (var stream = new SharpDX.WIC.WICStream(factory, filename, SharpDX.IO.NativeFileAccess.Write))
                    using (var encoder = new SharpDX.WIC.PngBitmapEncoder(factory)) {
                        encoder.Initialize(stream);

                        using (var frame = new SharpDX.WIC.BitmapFrameEncode(encoder)) {
                            frame.Initialize(); // ВАЖНО: инициализация кадра
                            frame.SetSize(texture.Description.Width, texture.Description.Height);

                            var pixelFormat = SharpDX.WIC.PixelFormat.Format32bppBGRA;
                            frame.SetPixelFormat(ref pixelFormat);

                            using (var stagingTexture = new SharpDX.Direct3D11.Texture2D(d3dDevice, new SharpDX.Direct3D11.Texture2DDescription {
                                Width = texture.Description.Width,
                                Height = texture.Description.Height,
                                MipLevels = 1,
                                ArraySize = 1,
                                Format = texture.Description.Format,
                                SampleDescription = new SharpDX.DXGI.SampleDescription(1, 0),
                                Usage = SharpDX.Direct3D11.ResourceUsage.Staging,
                                CpuAccessFlags = SharpDX.Direct3D11.CpuAccessFlags.Read,
                                BindFlags = SharpDX.Direct3D11.BindFlags.None,
                                OptionFlags = SharpDX.Direct3D11.ResourceOptionFlags.None
                            })) {
                                d3dDevice.ImmediateContext.CopyResource(texture, stagingTexture);

                                var dataBox = d3dDevice.ImmediateContext.MapSubresource(stagingTexture, 0, SharpDX.Direct3D11.MapMode.Read, SharpDX.Direct3D11.MapFlags.None);
                                using (var bitmap = new SharpDX.WIC.Bitmap(factory, texture.Description.Width, texture.Description.Height, pixelFormat, new SharpDX.DataRectangle(dataBox.DataPointer, dataBox.RowPitch))) {
                                    frame.WriteSource(bitmap); // Записываем данные
                                    frame.Commit(); // Завершаем кадр
                                    encoder.Commit(); // Завершаем запись в файл
                                }
                                d3dDevice.ImmediateContext.UnmapSubresource(stagingTexture, 0);
                            }
                        }
                    }
                }

                static public void SaveBytesToFile(SharpDX.Direct3D11.Device d3dDevice, byte[] byteArray, int width, int height, SharpDX.DXGI.Format format, string name) {
                    string filename = Windows.Storage.ApplicationData.Current.LocalFolder.Path + $"\\{name}.png";

                    using (var factory = new SharpDX.WIC.ImagingFactory2())
                    using (var stream = new SharpDX.WIC.WICStream(factory, filename, SharpDX.IO.NativeFileAccess.Write))
                    using (var encoder = new SharpDX.WIC.PngBitmapEncoder(factory)) {
                        encoder.Initialize(stream);

                        using (var frame = new SharpDX.WIC.BitmapFrameEncode(encoder)) {
                            frame.Initialize();
                            frame.SetSize(width, height);

                            var pixelFormat = SharpDX.WIC.PixelFormat.Format32bppBGRA;
                            frame.SetPixelFormat(ref pixelFormat);

                            using (var stagingTexture = new SharpDX.Direct3D11.Texture2D(d3dDevice, new SharpDX.Direct3D11.Texture2DDescription {
                                Width = width,
                                Height = height,
                                MipLevels = 1,
                                ArraySize = 1,
                                Format = SharpDX.DXGI.Format.B8G8R8A8_UNorm,
                                SampleDescription = new SharpDX.DXGI.SampleDescription(1, 0),
                                Usage = SharpDX.Direct3D11.ResourceUsage.Staging,
                                CpuAccessFlags = SharpDX.Direct3D11.CpuAccessFlags.Read,
                                BindFlags = SharpDX.Direct3D11.BindFlags.None,
                                OptionFlags = SharpDX.Direct3D11.ResourceOptionFlags.None
                            })) {
                                // Создаем bitmap с корректными данными
                                using (var bitmap = new SharpDX.WIC.Bitmap(factory, width, height, pixelFormat, new SharpDX.DataRectangle(System.Runtime.InteropServices.Marshal.UnsafeAddrOfPinnedArrayElement(byteArray, 0), width * 4))) {
                                    frame.WriteSource(bitmap);
                                    frame.Commit();
                                    encoder.Commit();
                                }
                            }
                        }
                    }
                }

                static public void SaveTextureToImage(byte[] bgraData, int width, int height, string name, System.Drawing.Imaging.ImageFormat format) {
                    string filePath = Windows.Storage.ApplicationData.Current.LocalFolder.Path + $"\\{name}.png";

                    using (var bitmap = new System.Drawing.Bitmap(width, height, System.Drawing.Imaging.PixelFormat.Format32bppArgb)) {
                        var bitmapData = bitmap.LockBits(
                            new System.Drawing.Rectangle(0, 0, width, height),
                            System.Drawing.Imaging.ImageLockMode.WriteOnly,
                            System.Drawing.Imaging.PixelFormat.Format32bppArgb
                        );

                        // Копируем байты в bitmap
                        System.Runtime.InteropServices.Marshal.Copy(bgraData, 0, bitmapData.Scan0, bgraData.Length);
                        bitmap.UnlockBits(bitmapData);

                        // Сохраняем в нужном формате (PNG, JPG, BMP)
                        bitmap.Save(filePath, format);
                    }
                }
            } // class Dx


            public class Test {
                static public byte[] GenerateNV12Bytes(int width, int height) {
                    int ySize = width * height;
                    int uvSize = (width / 2) * (height / 2) * 2; // UV интерливированный
                    byte[] nv12Data = new byte[ySize + uvSize];

                    int squareSize = System.Math.Min(width, height) / 2;
                    int startX = (width - squareSize) / 2;
                    int startY = (height - squareSize) / 2;

                    // Заполняем Y-плоскость (яркость)
                    for (int y = 0; y < height; y++) {
                        for (int x = 0; x < width; x++) {
                            int index = y * width + x;
                            if (x >= startX && x < startX + squareSize && y >= startY && y < startY + squareSize) {
                                nv12Data[index] = 76; // Y для красного (примерно)
                            }
                            else {
                                nv12Data[index] = 128; // Серый фон
                            }
                        }
                    }

                    // Заполняем UV-плоскость (цвет)
                    for (int y = 0; y < height / 2; y++) {
                        for (int x = 0; x < width / 2; x++) {
                            int index = ySize + (y * (width / 2) + x) * 2;
                            if (x >= startX / 2 && x < (startX + squareSize) / 2 && y >= startY / 2 && y < (startY + squareSize) / 2) {
                                nv12Data[index] = 85;   // U для красного
                                nv12Data[index + 1] = 255; // V для красного
                            }
                            else {
                                nv12Data[index] = 128; // U (серый)
                                nv12Data[index + 1] = 128; // V (серый)
                            }
                        }
                    }
                    return nv12Data;
                }

                static public byte[] ConvertNV12ToBGRA_CPU(byte[] nv12Data, int width, int height) {
                    int ySize = width * height;
                    int uvSize = (width / 2) * (height / 2) * 2;
                    byte[] bgraData = new byte[width * height * 4];

                    for (int y = 0; y < height; y++) {
                        for (int x = 0; x < width; x++) {
                            int yIndex = y * width + x;
                            int uvIndex = ySize + (y / 2) * (width / 2) * 2 + (x / 2) * 2;

                            float Y = nv12Data[yIndex] / 255.0f;
                            float U = (nv12Data[uvIndex] - 128) / 255.0f;
                            float V = (nv12Data[uvIndex + 1] - 128) / 255.0f;

                            float R = Y + 1.402f * V;
                            float G = Y - 0.344136f * U - 0.714136f * V;
                            float B = Y + 1.772f * U;

                            int bgraIndex = (y * width + x) * 4;
                            bgraData[bgraIndex] = (byte)(255 * System.Math.Clamp(B, 0, 1));
                            bgraData[bgraIndex + 1] = (byte)(255 * System.Math.Clamp(G, 0, 1));
                            bgraData[bgraIndex + 2] = (byte)(255 * System.Math.Clamp(R, 0, 1));
                            bgraData[bgraIndex + 3] = 255;
                        }
                    }
                    return bgraData;
                }

                static public SharpDX.Direct3D11.Texture2D ConvertNV12TextureToBGRATexture(SharpDX.Direct3D11.Device d3dDevice, SharpDX.Direct3D11.Texture2D nv12Texture) {
                    int width = nv12Texture.Description.Width;
                    int height = nv12Texture.Description.Height;

                    int ySize = width * height;
                    int uvSize = (width / 2) * (height / 2) * 2; // UV интерливированный
                    byte[] nv12Data = new byte[ySize + uvSize];

                    // Создаем Staging-копию для CPU-доступа
                    var stagingDesc = new SharpDX.Direct3D11.Texture2DDescription {
                        Width = width,
                        Height = height,
                        MipLevels = 1,
                        ArraySize = 1,
                        Format = nv12Texture.Description.Format, // NV12
                        SampleDescription = new SharpDX.DXGI.SampleDescription(1, 0),
                        Usage = SharpDX.Direct3D11.ResourceUsage.Staging,
                        BindFlags = SharpDX.Direct3D11.BindFlags.None,
                        CpuAccessFlags = SharpDX.Direct3D11.CpuAccessFlags.Read,
                        OptionFlags = SharpDX.Direct3D11.ResourceOptionFlags.None
                    };

                    using (var stagingTexture = new SharpDX.Direct3D11.Texture2D(d3dDevice, stagingDesc)) {
                        d3dDevice.ImmediateContext.CopyResource(nv12Texture, stagingTexture);

                        var dataBox = d3dDevice.ImmediateContext.MapSubresource(stagingTexture, 0,
                            SharpDX.Direct3D11.MapMode.Read, SharpDX.Direct3D11.MapFlags.None);

                        int rowPitch = dataBox.RowPitch;

                        // Копируем Y-плоскость
                        for (int y = 0; y < height; y++) {
                            System.Runtime.InteropServices.Marshal.Copy(
                                dataBox.DataPointer + y * rowPitch,
                                nv12Data,
                                y * width,
                                width);
                        }

                        // Копируем UV-плоскость (половина высоты)
                        System.IntPtr uvPtr = dataBox.DataPointer + rowPitch * height;
                        for (int y = 0; y < height / 2; y++) {
                            System.Runtime.InteropServices.Marshal.Copy(
                                uvPtr + y * rowPitch,
                                nv12Data,
                                ySize + y * width,
                                width);
                        }

                        d3dDevice.ImmediateContext.UnmapSubresource(stagingTexture, 0);
                    }

                    byte[] bgraData = ConvertNV12ToBGRA_CPU(nv12Data, width, height);

                    // Создаем BGRA-текстуру
                    return MakeTextureFromBytes(d3dDevice, bgraData, width, height, SharpDX.DXGI.Format.B8G8R8A8_UNorm);
                }

                static public SharpDX.Direct3D11.Texture2D MakeTextureFromBytes(SharpDX.Direct3D11.Device d3dDevice, byte[] byteArray, int width, int height, SharpDX.DXGI.Format format) {
                    var desc = new SharpDX.Direct3D11.Texture2DDescription {
                        Width = width,
                        Height = height,
                        Format = format,
                        MipLevels = 1,
                        ArraySize = 1,
                        SampleDescription = new SharpDX.DXGI.SampleDescription(1, 0),
                        Usage = SharpDX.Direct3D11.ResourceUsage.Default,
                        BindFlags = SharpDX.Direct3D11.BindFlags.ShaderResource,
                        CpuAccessFlags = SharpDX.Direct3D11.CpuAccessFlags.None,
                        OptionFlags = SharpDX.Direct3D11.ResourceOptionFlags.None,
                    };

                    SharpDX.DataRectangle[] dataRectangles;

                    if (format == SharpDX.DXGI.Format.NV12) {
                        int ySize = width * height;
                        int uvSize = (width / 2) * (height / 2) * 2;

                        dataRectangles = new[] {
                            new SharpDX.DataRectangle(
                                System.Runtime.InteropServices.Marshal.UnsafeAddrOfPinnedArrayElement(byteArray, 0),
                                width), // Stride для Y-плоскости
                            new SharpDX.DataRectangle(
                                System.Runtime.InteropServices.Marshal.UnsafeAddrOfPinnedArrayElement(byteArray, ySize),
                                width) // Stride для UV-плоскости
                        };
                    }
                    else {
                        dataRectangles = new[] {
                            new SharpDX.DataRectangle(
                                System.Runtime.InteropServices.Marshal.UnsafeAddrOfPinnedArrayElement(byteArray, 0),
                                width * 4) // Stride для BGRA
                        };
                    }

                    return new SharpDX.Direct3D11.Texture2D(d3dDevice, desc, dataRectangles);
                }
            } // class Test
        } // namespace Dx
    } // namespace Helpers

    public struct Vertex {
        public SharpDX.Mathematics.Interop.RawVector3 Position;
        public SharpDX.Mathematics.Interop.RawVector2 TexCoord;
    }


    public sealed partial class MainWindow_6_Dx : Microsoft.UI.Xaml.Window {
        private SharpDX.Direct3D11.Device d3dDevice;
        private SharpDX.Direct3D11.Texture2D renderTexture;
        
        private SharpDX.Direct3D11.InputLayout inputLayout;
        private SharpDX.Direct3D11.Buffer vertexBuffer;
        private SharpDX.Direct3D11.VertexShader vertexShader;

        private SharpDX.Direct3D11.SamplerState samplerState;
        private SharpDX.Direct3D11.PixelShader pixelShader;

        private int targetWidth = 512;
        private int targetHeight = 512;
        private SharpDX.DXGI.Format targetFormat = SharpDX.DXGI.Format.B8G8R8A8_UNorm;

        public MainWindow_6_Dx() {
            try {
                this.InitializeComponent();
                this.InitializeDirectX();
                this.Render();
            }
            catch (System.Exception ex) {
                System.Diagnostics.Debug.WriteLine($"Exception: {ex}");
                throw;
            }
        }

        private void InitializeDirectX() {
            var deviceCreationFlags = SharpDX.Direct3D11.DeviceCreationFlags.BgraSupport;
#if DEBUG
            deviceCreationFlags |= SharpDX.Direct3D11.DeviceCreationFlags.Debug;
#endif
            this.d3dDevice = new SharpDX.Direct3D11.Device(SharpDX.Direct3D.DriverType.Hardware, deviceCreationFlags);

            // Создание текстуры
            SharpDX.Direct3D11.Texture2DDescription textureDesc = new SharpDX.Direct3D11.Texture2DDescription {
                Width = this.targetWidth,
                Height = this.targetHeight,
                MipLevels = 1,
                ArraySize = 1,
                Format = SharpDX.DXGI.Format.B8G8R8A8_UNorm,
                SampleDescription = new SharpDX.DXGI.SampleDescription(1, 0),
                Usage = SharpDX.Direct3D11.ResourceUsage.Default,
                BindFlags = SharpDX.Direct3D11.BindFlags.RenderTarget | SharpDX.Direct3D11.BindFlags.ShaderResource,
                CpuAccessFlags = SharpDX.Direct3D11.CpuAccessFlags.None,
                OptionFlags = SharpDX.Direct3D11.ResourceOptionFlags.None
            };

            this.renderTexture = new SharpDX.Direct3D11.Texture2D(this.d3dDevice, textureDesc);
            //renderTargetView = new SharpDX.Direct3D11.RenderTargetView(this.d3dDevice, this.renderTexture);

            string defaultCommonShaderPath = System.IO.Path.Combine(System.AppDomain.CurrentDomain.BaseDirectory, "DefaultCommonShader.hlsl");
            if (!System.IO.File.Exists(defaultCommonShaderPath)) {
                throw new System.IO.FileNotFoundException($"Shader file not found: {defaultCommonShaderPath}");
            }

            // Компиляция шейдеров
            var vertexShaderByteCode = SharpDX.D3DCompiler.ShaderBytecode.CompileFromFile(defaultCommonShaderPath, "VSMain", "vs_5_0");
            var pixelShaderByteCode = SharpDX.D3DCompiler.ShaderBytecode.CompileFromFile(defaultCommonShaderPath, "PSMain", "ps_5_0");

            this.vertexShader = new SharpDX.Direct3D11.VertexShader(this.d3dDevice, vertexShaderByteCode);
            this.pixelShader = new SharpDX.Direct3D11.PixelShader(this.d3dDevice, pixelShaderByteCode);

            // Определение входного макета
            var layout = new[]
            {
                new SharpDX.Direct3D11.InputElement("POSITION", 0, SharpDX.DXGI.Format.R32G32B32_Float, 0, 0),
                new SharpDX.Direct3D11.InputElement("TEXCOORD", 0, SharpDX.DXGI.Format.R32G32_Float, 12, 0)
            };
            this.inputLayout = new SharpDX.Direct3D11.InputLayout(this.d3dDevice, SharpDX.D3DCompiler.ShaderSignature.GetInputSignature(vertexShaderByteCode), layout);


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
            this.vertexBuffer = SharpDX.Direct3D11.Buffer.Create(this.d3dDevice, SharpDX.Direct3D11.BindFlags.VertexBuffer, vertices);

            if (vertexShaderByteCode.Bytecode == null) {
                throw new System.Exception("Ошибка компиляции вершинного шейдера!");
            }
            if (pixelShaderByteCode.Bytecode == null) {
                throw new System.Exception("Ошибка компиляции пиксельного шейдера!");
            }


            // Создаем самплер для выборки текстуры
            var samplerDesc = new SharpDX.Direct3D11.SamplerStateDescription {
                Filter = SharpDX.Direct3D11.Filter.MinMagMipLinear, // Линейная интерполяция
                AddressU = SharpDX.Direct3D11.TextureAddressMode.Clamp, // Обрезка текстуры по границам
                AddressV = SharpDX.Direct3D11.TextureAddressMode.Clamp,
                AddressW = SharpDX.Direct3D11.TextureAddressMode.Clamp,
                MipLodBias = 0,
                MaximumAnisotropy = 1,
                ComparisonFunction = SharpDX.Direct3D11.Comparison.Never,
                BorderColor = new SharpDX.Mathematics.Interop.RawColor4(0, 0, 0, 0),
                MinimumLod = 0,
                MaximumLod = float.MaxValue
            };

            // Создаем объект SamplerState
            this.samplerState = new SharpDX.Direct3D11.SamplerState(this.d3dDevice, samplerDesc);
        }


        private void Render() {
            var nv12Bytes = Helpers.Dx.Test.GenerateNV12Bytes(this.targetWidth, this.targetHeight);
            var nv12Texture = Helpers.Dx.Test.MakeTextureFromBytes(this.d3dDevice, nv12Bytes, this.targetWidth, this.targetHeight, SharpDX.DXGI.Format.NV12);

            //var bgraBytes = this.ConvertNV12ToBGRA_CPU(nv12Bytes, this.targetWidth, this.targetHeight);
            ////var bgraBytes = this.GenerateBGRABytes(this.targetWidth, this.targetHeight);
            ////this.SaveTextureToImage(bgraBytes, this.targetWidth, this.targetHeight, "outputTextureA", System.Drawing.Imaging.ImageFormat.Png);
            ////this.SaveBytesToFile(bgraBytes, this.targetWidth, this.targetHeight, this.targetFormat, "outputTextureB");
            //var bgraTexture = this.MakeTextureFromBytes(bgraBytes, this.targetWidth, this.targetHeight, this.targetFormat);

            var bgraTexture = Helpers.Dx.Test.ConvertNV12TextureToBGRATexture(this.d3dDevice, nv12Texture);
            Helpers.Dx.Dx.SaveTextureToFile(this.d3dDevice, bgraTexture, "outputTextureCPU");


            var d3dContext = this.d3dDevice.ImmediateContext;

            // Устанавливаем RenderTarget
            var renderTargetView = new SharpDX.Direct3D11.RenderTargetView(this.d3dDevice, this.renderTexture);
            d3dContext.OutputMerger.SetRenderTargets(renderTargetView);
            d3dContext.ClearRenderTargetView(renderTargetView, new SharpDX.Mathematics.Interop.RawColor4(0.1f, 0.0f, 0.1f, 1));

            // Устанавливаем Input Layout
            d3dContext.InputAssembler.InputLayout = this.inputLayout;
            d3dContext.InputAssembler.PrimitiveTopology = SharpDX.Direct3D.PrimitiveTopology.TriangleList;
            d3dContext.InputAssembler.SetVertexBuffers(0, new SharpDX.Direct3D11.VertexBufferBinding(this.vertexBuffer, SharpDX.Utilities.SizeOf<Vertex>(), 0));


            // Создаем ресурсные представления для Y и UV плоскостей
            var srvY = new SharpDX.Direct3D11.ShaderResourceView(this.d3dDevice, nv12Texture, new SharpDX.Direct3D11.ShaderResourceViewDescription {
                Format = SharpDX.DXGI.Format.R8_UNorm, // Y - 8 бит на пиксель
                Dimension = SharpDX.Direct3D.ShaderResourceViewDimension.Texture2D,
                Texture2D = { MipLevels = 1 }
            });

            var srvUV = new SharpDX.Direct3D11.ShaderResourceView(this.d3dDevice, nv12Texture, new SharpDX.Direct3D11.ShaderResourceViewDescription {
                Format = SharpDX.DXGI.Format.R8G8_UNorm, // UV - 8 бит на канал
                Dimension = SharpDX.Direct3D.ShaderResourceViewDimension.Texture2D,
                Texture2D = { MipLevels = 1 }
            });


            // Устанавливаем шейдеры
            d3dContext.VertexShader.Set(this.vertexShader);
            d3dContext.PixelShader.Set(this.pixelShader);
            d3dContext.PixelShader.SetShaderResources(0, srvY, srvUV);
            d3dContext.PixelShader.SetSamplers(0, this.samplerState);

            // Устанавливаем Viewport
            SharpDX.Mathematics.Interop.RawViewportF viewport = new SharpDX.Mathematics.Interop.RawViewportF {
                X = 0,
                Y = 0,
                Width = this.targetWidth,
                Height = this.targetHeight,
                MinDepth = 0.0f,
                MaxDepth = 1.0f
            };
            this.d3dDevice.ImmediateContext.Rasterizer.SetViewport(viewport);

            // Рендерим
            d3dContext.Draw(6, 0);

            // Принудительно сбрасываем команды
            d3dContext.Flush();

            Helpers.Dx.Dx.SaveTextureToFile(this.d3dDevice, this.renderTexture, "outputTextureGPU");

            // Очистка ресурсов
            d3dContext.VertexShader.Set(null);
            d3dContext.PixelShader.Set(null);
            d3dContext.PixelShader.SetShaderResource(0, null);
            d3dContext.PixelShader.SetShaderResource(1, null);
            renderTargetView.Dispose();
        }
    }
}