using Microsoft.UI.Xaml.Media.Imaging;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Security.Cryptography.X509Certificates;
using System.Text;
using System.Text.RegularExpressions;
using System.Threading.Tasks;
using System.Globalization;
using System.Threading;
using System.Runtime.CompilerServices;
using System.ComponentModel;
using Windows.Storage;


namespace SimpleApp.WinUI3 {
    namespace Helpers {
        public class ObservableObject : INotifyPropertyChanged {
            public event PropertyChangedEventHandler PropertyChanged;

            protected void OnPropertyChanged([CallerMemberName] string propertyName = null) {
                PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
            }

            protected bool SetProperty<T>(ref T field, T value, [CallerMemberName] string propertyName = null) {
                //if (Equals(field, value)) {
                //    return false;
                //}
                field = value;
                OnPropertyChanged(propertyName);
                return true;
            }
        }

        public static class FS {
            public static async Task<bool> SafeDeleteFile(FileInfo fileInfo) {
                if (fileInfo == null) {
                    Diagnostic.Logger.LogDebug($"passed 'fileInfo' is null");
                    return false;
                }

                try {
                    if (System.IO.File.Exists(fileInfo.Path)) {
                        Diagnostic.Logger.LogDebug($"Delete file '{fileInfo.Path}'");
                        StorageFile file = await StorageFile.GetFileFromPathAsync(fileInfo.Path);
                        await file.DeleteAsync();
                    }
                    return true;
                }
                catch (Exception ex) {
                    Diagnostic.Logger.LogDebug($"Exception [deletion file] = {ex.Message}");
                }
                return false;
            }


            public static async Task<bool> SafeDeleteStorageFileAsync(StorageFile storageFile) {
                if (storageFile == null) {
                    Diagnostic.Logger.LogDebug($"passed 'storageFile' is null");
                    return false;
                }

                try {
                    await storageFile.DeleteAsync();
                    return true;
                }
                catch (Exception ex) {
                    Diagnostic.Logger.LogDebug($"Exception [deletion file] = {ex}");
                }
                return false;
            }

            public static async Task<bool> SafeDeleteStorageFolderAsync(StorageFolder storageFolder) {
                if (storageFolder == null) {
                    Diagnostic.Logger.LogDebug($"passed 'storageFolder' is null");
                    return false;
                }

                try {
                    // Удаляем все файлы внутри папки
                    var files = await storageFolder.GetFilesAsync();
                    foreach (var file in files) {
                        await file.DeleteAsync();
                    }

                    // Удаляем все подпапки внутри папки
                    var subFolders = await storageFolder.GetFoldersAsync();
                    foreach (var subFolder in subFolders) {
                        await subFolder.DeleteAsync();
                    }

                    // Теперь удаляем саму папку
                    await storageFolder.DeleteAsync();

                    return true;
                }
                catch (Exception ex) {
                    Diagnostic.Logger.LogDebug($"Exception [deletion folder] = {ex}");
                }
                return false;
            }
        }
    }
}