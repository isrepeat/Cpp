using System;
using System.Windows.Input;
using System.Collections.Generic;
using Windows.Foundation.Collections;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.Storage.Pickers;
using Windows.UI.Xaml.Input;
using Windows.System;
using Windows.Storage;
using Windows.Storage.Streams;
using Windows.UI.Core;
using Windows.UI.Xaml.Controls.Primitives;
using Windows.UI.ViewManagement;
using Windows.ApplicationModel.DataTransfer;
using Windows.UI.Xaml.Media.Imaging;
using Windows.UI.Popups;
using Windows.ApplicationModel.Activation;
using Windows.UI.Xaml.Navigation;
using System.Diagnostics.Tracing;
using System.IO;

using GalaSoft.MvvmLight.Command;
using MediaEngine = MediaEngineWinRT;


using System.Diagnostics;
using Windows.Foundation;
using Windows.UI.ViewManagement;

namespace VideoPlayerPrototype {
    public sealed partial class MainPage : Page {
        const string SchemeHandlerClassName = "AVSource.AVSchemeHandler";
        const string ByteStreamHandlerClassName = "AVSource.AVByteStreamHandler";

        AVSource.MediaStatus mediaStatus = new AVSource.MediaStatus();
        AVSource.DVDController dvdController = new AVSource.DVDController();
        AVSource.MediaController mediaController = new AVSource.MediaController();
        AVSource.RecordController recordController = new AVSource.RecordController();

        Windows.Media.MediaExtensionManager extensionManager = new Windows.Media.MediaExtensionManager();
        private void AddExtension(string extension) {
            extensionManager.RegisterByteStreamHandler(MainPage.ByteStreamHandlerClassName, extension, "", this.CreateProps());
        }

        private void AddExtension(string extension, string mimeType) {
            extensionManager.RegisterByteStreamHandler(MainPage.ByteStreamHandlerClassName, extension, mimeType, this.CreateProps());
        }

        private void AddScheme(string scheme) {
            extensionManager.RegisterSchemeHandler(MainPage.SchemeHandlerClassName, scheme, this.CreateProps());
        }

        private IPropertySet CreateProps() {
            IPropertySet props = new PropertySet();
            props.Add("MediaStatus", this.mediaStatus);
            props.Add("DVDController", this.dvdController);
            props.Add("MediaController", this.mediaController);
            props.Add("RecordController", this.recordController);
            return props;
        }


        private MediaEngine::Player player;
        private DispatcherTimer playbackTimer = new DispatcherTimer();
        private bool positionSliderChangeFromCode = false;

        private static readonly TimeSpan playbackUpdateRate = TimeSpan.FromMilliseconds(500); 
        private static readonly long minorSeekAmount = MediaEngine::MFTime.FromMsec(1000);

        public ICommand AddTrackCommand { get; protected set; }
        public ICommand ToggleFullscreenCommand { get; protected set; }

        public DynamicRenderResolutionScale DynamicRenderResolution { get; set; }


        public MainPage() {
            this.InitializeComponent();

            //this.AddExtension(".*");
            //this.AddExtension(".*", "video/3gpp");          // 3gpp
            //this.AddExtension(".*", "audio/3gpp");
            //this.AddExtension(".*", "video/3gpp2");         // 3gpp2
            //this.AddExtension(".*", "audio/3gpp2");
            //this.AddExtension(".*", "video/avi");           // avi
            //this.AddExtension(".*", "audio/avi");
            ////this.AddExtension(".*", "video/x-flv");       // flv
            //this.AddExtension(".*", "video/x-matroska");    // mkv
            //this.AddExtension(".*", "audio/x-matroska");
            //this.AddExtension(".*", "video/mpeg");          // mp2, mp3, mpa, mpeg, mpg
            //this.AddExtension(".*", "audio/mpeg");
            //this.AddExtension(".*", "audio/mp4");           // mp4
            //this.AddExtension(".*", "video/mp4");
            //this.AddExtension(".*", "audio/ogg");           // ogg
            //this.AddExtension(".*", "video/ogg");
            ////this.AddExtension(".*", "video/quicktime");   // mov, qt
            ////this.AddExtension(".*", "video/dvd");         // vob
            //this.AddExtension(".*", "video/MP2T");          // ts
            //this.AddExtension(".*", "video/x-ms-wmv");      // wmv
            //this.AddExtension(".*", "audio/x-ms-wmv");
            //this.AddExtension(".*", "video/webm");          // webm
            //this.AddExtension(".*", "audio/webm");


            //this.AddExtension(".asf");
            //this.AddExtension(".asx");
            //this.AddExtension(".f4v");
            //this.AddExtension(".flv");
            //this.AddExtension(".gtp");
            //this.AddExtension(".h264");
            //this.AddExtension(".mov");
            //this.AddExtension(".m4v");
            //this.AddExtension(".mod");
            //this.AddExtension(".mts");
            //this.AddExtension(".m2ts");
            //this.AddExtension(".ogg");
            //this.AddExtension(".rm");
            //this.AddExtension(".swf");
            //this.AddExtension(".srt");
            //this.AddExtension(".ts");
            //this.AddExtension(".vob");

            //AVSource.AV.RegisterFFVideoDecoder2();

            // Setup media player backend
            player = new MediaEngine::Player(this.videoPanel);
            player.OnPlaybackStarted += OnPlaybackStarted;
            player.OnPlaybackPaused += OnPlaybackPaused;
            player.OnPlaybackStopped += OnPlaybackStopped;

            Window.Current.CoreWindow.KeyDown += OnKeyDown;

            //// Setup playback position update timer
            //playbackTimer.Interval = playbackUpdateRate;
            //playbackTimer.Tick += PlaybackStatusUpdateTick;

            this.AddTrackCommand = new RelayCommand(this.PickMediaFile);
            this.ToggleFullscreenCommand = new RelayCommand(this.ToggleFullscreen);

            this.DynamicRenderResolution = new DynamicRenderResolutionScale();
            this.DynamicRenderResolution.DynamicRenderResolutionChangedEvent += Slider_DynamicRenderResolutionChanged;

            this.DataContext = this;
        }

        protected override void OnNavigatedTo(NavigationEventArgs e) {
            base.OnNavigatedTo(e);

            // On protocol launch
            if (e.Parameter is Uri)
                OpenMedia(e.Parameter as Uri);

            // On associated file launch
            else if (e.Parameter is StorageFile)
                OpenMedia(e.Parameter as StorageFile);
        }

        /**
         * Lets user pick a file and starts the playback.
         */
        private async void PickMediaFile() {
            var openPicker = new FileOpenPicker();
            openPicker.ViewMode = PickerViewMode.Thumbnail;
            openPicker.SuggestedStartLocation = PickerLocationId.VideosLibrary;
            openPicker.FileTypeFilter.AddRange(Assets.mediaTypes);

            var file = await openPicker.PickSingleFileAsync();
            if (file == null)
                return;

            OpenMedia(file);
        }


        private void ToggleFullscreen() {
            var view = ApplicationView.GetForCurrentView();
            if (view.IsFullScreenMode) {
                view.ExitFullScreenMode();
            }
            else {
                view.TryEnterFullScreenMode();
            }
        }

        private void Slider_DynamicRenderResolutionChanged(int resolutionPercent) {
            this.player.SetDynamicRenderResolution(resolutionPercent);
        }


        /**
         * Opens a user-selected file.
         */
        private async void OpenMedia(StorageFile file) {
            try {
                var stream = await file.OpenAsync(FileAccessMode.Read);
                player.OpenMedia(stream);
                UpdateVolumeControls();
                ApplicationView.GetForCurrentView().Title = file.Name;
                this.durationLabel.Text = MediaEngine::MFTime.ToTimeString(player.GetMediaDuration());
                this.positionSlider.Maximum = player.GetMediaDuration();
            }
            catch (Exception e) {
                FileOpenError(e);
            }
        }

        /**
         * Opens a video file by URL.
         * Will fail for files not in the `Videos` library.
         */
        private async void OpenMedia(Uri uri) {
            var path = Uri.UnescapeDataString(uri.AbsoluteUri);
            path = NormalizePath(path.Replace("play-video:", ""));
            await new MessageDialog(
                "You are about to play the following file: \"" + path + "\".",
                "Protocol Launch"
            ).ShowAsync();

            try {
                var file = await StorageFile.GetFileFromPathAsync(path);
                OpenMedia(file);
            }
            catch (Exception e) {
                FileOpenError(e);
            }
        }

        private static string NormalizePath(string path) {
            return Path.GetFullPath(new Uri(path).LocalPath)
                       .TrimEnd(Path.DirectorySeparatorChar, Path.AltDirectorySeparatorChar);
        }

        private async void FileOpenError(Exception e) {
            await new MessageDialog(e.Message, "Failed to open the file").ShowAsync();
        }

        private void SetCurrentPlaybackPosition(long pos) {
            if (this.positionSlider.Value != pos) {
                positionSliderChangeFromCode = true;
                this.positionSlider.Value = pos;
                positionSliderChangeFromCode = false;
                this.positionLabel.Text = MediaEngine::MFTime.ToTimeString(pos);
            }
        }

        private void UpdateVolumeControls(bool updateSlider = true) {
            //var volume = player.GetVolume() * 100.0f;
            //this.volumeLabel.Text = string.Format(Assets.volumeFormat, volume.ToString());
            //if (updateSlider)
            //    this.volumeSlider.Value = volume;
        }

        private void Seek(long position) {
            player.Seek(position);
            SetCurrentPlaybackPosition(position);
        }

        //private void PlaybackStatusUpdateTick(object sender, object e) {
        //    SetCurrentPlaybackPosition(player.GetCurrentPosition());
        //}

        private void OnPlaybackStarted() {
            this.DoLater(() => playbackTimer.Start());
        }

        private void OnPlaybackPaused() {
            this.DoLater(() => playbackTimer.Stop());
        }

        private void OnPlaybackStopped() {
            this.DoLater(() => OnPlaybackPaused());
        }

        private void OnVideoViewClicked(object sender, PointerRoutedEventArgs e) {
            //player.TogglePlayback();
        }

        private void OnKeyDown(CoreWindow window, KeyEventArgs e) {
            //switch (e.VirtualKey) {
            //    case VirtualKey.Space:
            //        player.TogglePlayback();
            //        break;

            //    case VirtualKey.Left:
            //        Seek(player.GetCurrentPosition() - minorSeekAmount);
            //        break;

            //    case VirtualKey.Right:
            //        Seek(player.GetCurrentPosition() + minorSeekAmount);
            //        break;

            //    default:
            //        return;
            //}
        }

        private void OnSeekSliderDragged(object sender, RangeBaseValueChangedEventArgs e) {
            if (!positionSliderChangeFromCode)
            {
                Seek((long)e.NewValue);
            }
        }

        private void OnSeekSliderDragStarted(object sender, ManipulationStartedRoutedEventArgs e) {
            //if (player.IsPlaying())
            //    playbackTimer.Stop();
        }

        private void OnSeekSliderDragEnded(object sender, ManipulationCompletedRoutedEventArgs e) {
            //if (player.IsPlaying())
            //    playbackTimer.Start();
        }

        private void OnSeekSliderTapped(object sender, TappedRoutedEventArgs e) {
            var slider = sender as Slider;
            //Seek((long)slider.Value);
        }

        private void OnVolumeSliderDragged(object sender, RangeBaseValueChangedEventArgs e) {
            if (player == null)
                return;

            player.SetVolume((float)e.NewValue / 100.0f);
            UpdateVolumeControls(false);
        }

        private void OnDragOver(object sender, DragEventArgs e) {
            e.AcceptedOperation = DataPackageOperation.Copy;
        }

        private async void OnDrop(object sender, DragEventArgs e) {
            if (!e.DataView.Contains(StandardDataFormats.StorageItems))
                return;

            var items = await e.DataView.GetStorageItemsAsync();
            if (items.Count == 0 || items.Count > 1)
                return;

            var file = items[0] as StorageFile;
            OpenMedia(file);
        }
    }
}