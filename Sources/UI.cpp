#include "UI.h"

#include "SDF.h"

#include "imgui.h"
#include "imgui-SFML.h"

#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Image.hpp>

#include <limits>

#include <Windows.h>
#include <string>
#include <shobjidl.h> 
#include <locale>
#include <codecvt>

bool openFile(char* selectedFile)
{
    //  CREATE FILE OBJECT INSTANCE
    HRESULT f_SysHr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    if (FAILED(f_SysHr))
        return FALSE;

    // CREATE FileOpenDialog OBJECT
    IFileOpenDialog* f_FileSystem;
    f_SysHr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL, IID_IFileOpenDialog, reinterpret_cast<void**>(&f_FileSystem));
    if (FAILED(f_SysHr)) {
        CoUninitialize();
        return FALSE;
    }
	
	// SET FILE TYPES
	COMDLG_FILTERSPEC rgSpec[] = {
		{L"PNG", L"*.png"},
	};
	f_SysHr = f_FileSystem->SetFileTypes(1, rgSpec);
	if (FAILED(f_SysHr)) {
        f_FileSystem->Release();
        CoUninitialize();
        return FALSE;
    }
	
	// SET DEFAULT EXTENSION
	f_SysHr = f_FileSystem->SetDefaultExtension(L"png");
	if (FAILED(f_SysHr)) {
        f_FileSystem->Release();
        CoUninitialize();
        return FALSE;
    }

    //  SHOW OPEN FILE DIALOG WINDOW
    f_SysHr = f_FileSystem->Show(NULL);
    if (FAILED(f_SysHr)) {
        f_FileSystem->Release();
        CoUninitialize();
        return FALSE;
    }

    //  RETRIEVE FILE NAME FROM THE SELECTED ITEM
    IShellItem* f_Files;
    f_SysHr = f_FileSystem->GetResult(&f_Files);
    if (FAILED(f_SysHr)) {
        f_FileSystem->Release();
        CoUninitialize();
        return FALSE;
    }

    //  STORE AND CONVERT THE FILE NAME
    PWSTR f_Path;
    f_SysHr = f_Files->GetDisplayName(SIGDN_FILESYSPATH, &f_Path);
    if (FAILED(f_SysHr)) {
        f_Files->Release();
        f_FileSystem->Release();
        CoUninitialize();
        return FALSE;
    }

    //  FORMAT AND STORE THE FILE PATH
    std::wstring path(f_Path);
	std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
    std::string s = converter.to_bytes(path);
	strcpy_s(selectedFile, s.length()+1, s.c_str());

    //  SUCCESS, CLEAN UP
    CoTaskMemFree(f_Path);
    f_Files->Release();
    f_FileSystem->Release();
    CoUninitialize();
    return TRUE;
}

bool saveFile(char* selectedFile)
{
    //  CREATE FILE OBJECT INSTANCE
    HRESULT f_SysHr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    if (FAILED(f_SysHr))
        return FALSE;

    // CREATE FileOpenDialog OBJECT
    IFileSaveDialog* f_FileSystem;
    f_SysHr = CoCreateInstance(CLSID_FileSaveDialog, NULL, CLSCTX_ALL, IID_IFileSaveDialog, reinterpret_cast<void**>(&f_FileSystem));
    if (FAILED(f_SysHr)) {
        CoUninitialize();
        return FALSE;
    }
	
	// SET FILE TYPES
	COMDLG_FILTERSPEC rgSpec[] = {
		{L"PNG", L"*.png"},
	};
	f_SysHr = f_FileSystem->SetFileTypes(1, rgSpec);
	if (FAILED(f_SysHr)) {
        f_FileSystem->Release();
        CoUninitialize();
        return FALSE;
    }
	
	// SET DEFAULT EXTENSION
	f_SysHr = f_FileSystem->SetDefaultExtension(L"png");
	if (FAILED(f_SysHr)) {
        f_FileSystem->Release();
        CoUninitialize();
        return FALSE;
    }

    //  SHOW OPEN FILE DIALOG WINDOW
    f_SysHr = f_FileSystem->Show(NULL);
    if (FAILED(f_SysHr)) {
        f_FileSystem->Release();
        CoUninitialize();
        return FALSE;
    }

    //  RETRIEVE FILE NAME FROM THE SELECTED ITEM
    IShellItem* f_Files;
    f_SysHr = f_FileSystem->GetResult(&f_Files);
    if (FAILED(f_SysHr)) {
        f_FileSystem->Release();
        CoUninitialize();
        return FALSE;
    }

    //  STORE AND CONVERT THE FILE NAME
    PWSTR f_Path;
    f_SysHr = f_Files->GetDisplayName(SIGDN_FILESYSPATH, &f_Path);
    if (FAILED(f_SysHr)) {
        f_Files->Release();
        f_FileSystem->Release();
        CoUninitialize();
        return FALSE;
    }

    //  FORMAT AND STORE THE FILE PATH
    std::wstring path(f_Path);
	std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
    std::string s = converter.to_bytes(path);
	strcpy_s(selectedFile, s.length()+1, s.c_str());

    //  SUCCESS, CLEAN UP
    CoTaskMemFree(f_Path);
    f_Files->Release();
    f_FileSystem->Release();
    CoUninitialize();
    return TRUE;
}

namespace ui
{
    static constexpr char paperRef[] = "Chris Green. 2007. Improved alpha-tested magnification for vector textures and special effects."
        " In ACM SIGGRAPH 2007 courses (SIGGRAPH '07). ACM, New York, NY, USA, 9-18."
        " DOI: https://doi.org/10.1145/1281500.1281665";
    static constexpr char paperRefShort[] = "Chris Green Improved alpha-tested magnification for vector textures and special effects";

	static int resize_real = 0;

    void LoadImage2( SDF& _sdf )
    {
		char _fileName[1000] = "\0";
		bool result = openFile(_fileName);
		if (result) {
			_sdf.SetTexture(_fileName);
		}
    }

    void ImageType( int& _imageType )
    {
        ImGui::Text( "Image Type" );
        ImGui::RadioButton( "Grey (grey channel will be used)", &_imageType, ImageType::Grey );
        ImGui::RadioButton( "Grey + Alpha (Alpha channel will be used)", &_imageType, ImageType::Grey_Alpha );
        ImGui::RadioButton( "RGB (Average of the R, G and B channel wil be used)", &_imageType, ImageType::AverageRGB );
        ImGui::RadioButton( "RGBA (Alpha channel will be used)", &_imageType, ImageType::RGBA );
    }

    void SpreadResize( int& _spread, int& _resize )
    {
        ImGui::Text( "Signed Distance Field" );
        ImGui::SliderInt( "Spread", &_spread, 1, 100 );
        ImGui::SliderInt( "Resize Factor", &resize_real, 0, 10 );
		_resize = 1 << resize_real;
    }

    void SmoothOutlineGlow( SDF& _sdf )
    {
        ImGui::Text( "Alpha testing" );

        ImGui::SliderFloat( "Smoothing", &_sdf.smoothing, 2.0f, 128.0f );

        ImGui::Checkbox( "Outline", &_sdf.outline );
        sf::Glsl::Vec4 outlineColor( _sdf.outlineColor );

        ImGui::ColorEdit4( "Outline color", &outlineColor.x, ImGuiColorEditFlags_AlphaPreview | ImGuiColorEditFlags_Uint8 );
        ImGui::SliderFloat( "Outline depth ouside", &_sdf.outlineDepth.x, 0.0f, 0.5f );
        ImGui::SliderFloat( "Outline depth inside", &_sdf.outlineDepth.y, 0.0f, 0.5f );
                
        _sdf.outlineColor = sf::Color(
            ( sf::Uint8 )( outlineColor.x * 255.0f ),
            ( sf::Uint8 )( outlineColor.y * 255.0f ),
            ( sf::Uint8 )( outlineColor.z * 255.0f ),
            ( sf::Uint8 )( outlineColor.w * 255.0f ) );

        ImGui::Checkbox( "Glow", &_sdf.glow );
        sf::Glsl::Vec4 glowColor( _sdf.glowColor );
        ImGui::ColorEdit4( "Glow color", &glowColor.x , ImGuiColorEditFlags_AlphaPreview | ImGuiColorEditFlags_Uint8 );
        ImGui::SliderFloat2( "Glow offset", &_sdf.glowOffset.x, -0.5f, 0.5f, "%.5f" );
        ImGui::SliderFloat( "Glow strength", &_sdf.glowStrength, 0.0f, 2.0f );
        
        _sdf.glowColor = sf::Color(
            ( sf::Uint8 )( glowColor.x * 255.0f ),
            ( sf::Uint8 )( glowColor.y * 255.0f ),
            ( sf::Uint8 )( glowColor.z * 255.0f ),
            ( sf::Uint8 )( glowColor.w * 255.0f ) );
    }

    void ViewMode( int& _viewMode )
    {
        ImGui::Text( "View Mode" );
        ImGui::RadioButton( "Distance field without resize.", &_viewMode, ViewMode::NoResize );
        ImGui::RadioButton( "Distance field with resize.", &_viewMode, ViewMode::Resized );
        ImGui::RadioButton( "Distance field alpha tested.", &_viewMode, ViewMode::AlphaTested );
    }

    void PaperRef()
    {

        if( ImGui::Button( "Paper Reference" ) )
            ImGui::OpenPopup( "Reference" );

        if( ImGui::BeginPopupModal( "Reference" ) )
        {
            ImGui::BeginChild( "", ImVec2( 600, 300 ) );
            ImGui::TextWrapped( paperRef );

            if( ImGui::Button( "Copy" ) )
                ImGui::SetClipboardText( paperRef );

            ImGui::SameLine();

            if( ImGui::Button( "Copy author and title" ) )
                ImGui::SetClipboardText( paperRefShort );

            ImGui::SameLine();

            if( ImGui::Button( "Close" ) )
                ImGui::CloseCurrentPopup();

            ImGui::EndChild();
            ImGui::EndPopup();
        }
    }

    void Apply( bool& _apply, bool& _auto )
    {
        ImGui::Checkbox( "Auto Apply", &_auto );
        ImGui::SameLine();
        if( ImGui::Button( "Apply" ) || _auto )
            _apply = true;
    }


    void Image( const sf::Sprite& _sprite )
    {
        ImGui::BeginChild( "", ImVec2( 0, 0 ), false, ImGuiWindowFlags_HorizontalScrollbar );
        ImGui::Image( _sprite );
        ImGui::EndChild();
    }


    float Zoom( int& _zoom, int _min, int _max )
    {
        ImGui::SliderInt( "Zoom", &_zoom, _min, _max, "%d%%" );
        return static_cast<float>( _zoom ) / 100.0f;
    }


    void SaveImage( const std::string& _dataPath, char _prefix[100], SDF& _sdf )
    {
        ImGui::Begin( "Save to file" );

        ImGui::Text( "File prefix : " );
        ImGui::SameLine();
        ImGui::InputText( "", _prefix, 99 );
        if( ImGui::Button( "Save" ) )
        {
			
            sf::Image imageToSave = _sdf.GetSDFSprite().getTexture()->copyToImage();
            imageToSave.saveToFile( _dataPath + "Saved/" + _prefix + "_SDF.png" );

            imageToSave = _sdf.GetResizeSprite().getTexture()->copyToImage();
            imageToSave.saveToFile( _dataPath + "Saved/" + _prefix + "_Resized.png" );

            imageToSave = _sdf.GetAlphaSprite().getTexture()->copyToImage();
            imageToSave.saveToFile( _dataPath + "Saved/" + _prefix + "_AlphaTested.png" );
        }

        ImGui::End();
    }
	
	void SaveImage2( sf::Sprite& s )
    {
		char fileName[1000];
		bool result = saveFile(fileName);
		if (result) {
			sf::Image imageToSave = s.getTexture()->copyToImage();
			imageToSave.saveToFile( fileName );
		}
    }
	
	void FileMenu( SDF& _sdf, sf::Sprite& s, bool& _apply )
	{
		if (ImGui::BeginMainMenuBar())
		{
			if (ImGui::BeginMenu("File"))
			{
				if (ImGui::MenuItem("Open", "Ctrl+O")) 
				{
					char fileName[1000] = "\0";
					bool result = openFile(fileName);
					if (result) {
						_sdf.SetTexture(fileName);
						_apply = true;
					}
				}
				if (ImGui::MenuItem("Save", "Ctrl+S")) 
				{ 
					char fileName[1000];
					bool result = saveFile(fileName);
					if (result) {
						sf::Image imageToSave = s.getTexture()->copyToImage();
						imageToSave.saveToFile( fileName );
					}
				}
				ImGui::EndMenu();
			}
			ImGui::EndMainMenuBar();
		}
	}

}
