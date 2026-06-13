#include "SDF.h"

#include "UI.h"

#include "imgui.h"
#include "imgui-SFML.h"

#include <SFML/Graphics.hpp>

#include <string>
#include <functional>
#include <cstdio>


static const std::string DATA_PATH = "./Data/";

//#define HIGH_DPI
#ifdef HIGH_DPI
static constexpr unsigned int WINDOW_SIZE_X = 2800;
static constexpr unsigned int WINDOW_SIZE_Y = 1900;
static constexpr float GLOBAL_SCALE = 2.0f;
#else
static constexpr unsigned int WINDOW_SIZE_X = 1280;
static constexpr unsigned int WINDOW_SIZE_Y = 720;
static constexpr float GLOBAL_SCALE = 1.0f;
#endif



void ShowSourceImage( int& _zoom, SDF& _sdf )
{
    ImGui::Begin( "Original Picture" );

    const float scale = ui::Zoom( _zoom, 10, 4000 );

    sf::Sprite& sourceSprite = _sdf.GetSourceSprite();
    sourceSprite.setScale( scale, scale );

    ui::Image( sourceSprite );

    // Restore default scale.
    sourceSprite.setScale( 1.0f, 1.0f );

    ImGui::End();
}

sf::Image convertChannelToGrayscale(const sf::Image& inputImage, int channel) {
	sf::Image outputImage;
    sf::Vector2u size = inputImage.getSize();
	
	outputImage.create(size.x, size.y, sf::Color(0, 0, 0, 255));

    for (unsigned int y = 0; y < size.y; ++y) {
        for (unsigned int x = 0; x < size.x; ++x) {
            sf::Color pixel = inputImage.getPixel(x, y);
			sf::Uint8 gray;
			switch(channel) {
				case 0:
					gray = pixel.r;
					break;
				case 1:
					gray = pixel.g;
					break;
				case 2:
					gray = pixel.b;
					break;
				case 3:
					gray = pixel.a;
					break;
			}
            outputImage.setPixel(x, y, sf::Color(gray, gray, gray, 255));
        }
    }
    return outputImage;
}

sf::Image combineChannels(const sf::Image& im_r, const sf::Image& im_g, const sf::Image& im_b, const sf::Image& im_a ) {
	sf::Image outputImage;
    sf::Vector2u size = im_r.getSize();
	
	outputImage.create(size.x, size.y, sf::Color(0, 0, 0, 255));

    for (unsigned int y = 0; y < size.y; ++y) {
        for (unsigned int x = 0; x < size.x; ++x) {
            outputImage.setPixel(x, y, sf::Color(im_r.getPixel(x, y).r, im_g.getPixel(x, y).r, im_b.getPixel(x, y).r, im_a.getPixel(x, y).r));
        }
    }
    return outputImage;
}

void ProcessImage(bool& _apply, SDF& _sdf, sf::Image& im)
{
	sf::Image orig_image;
	sf::Image combined;
    if( _apply )
    {
		orig_image = _sdf.GetSourceSprite().getTexture()->copyToImage();
		_sdf.imageType = ImageType::Grey;
		
		//_sdf.Process();
		//_apply = false;
		
		//red
		sf::Image im_r = convertChannelToGrayscale(orig_image, 0);
		_sdf.SetTexture(im_r);
        _sdf.Process();
		im_r = _sdf.GetSDFSprite().getTexture()->copyToImage();
		
		//green
		sf::Image im_g = convertChannelToGrayscale(orig_image, 1);
		_sdf.SetTexture(im_g);
        _sdf.Process();
		im_g = _sdf.GetSDFSprite().getTexture()->copyToImage();
		
		//blue
		sf::Image im_b = convertChannelToGrayscale(orig_image, 2);
		_sdf.SetTexture(im_b);
        _sdf.Process();
		im_b = _sdf.GetSDFSprite().getTexture()->copyToImage();
		
		//alpha
		sf::Image im_a = convertChannelToGrayscale(orig_image, 3);
		_sdf.SetTexture(im_a);
        _sdf.Process();
		im_a = _sdf.GetSDFSprite().getTexture()->copyToImage();
		
		combined = combineChannels(im_r, im_g, im_b, im_a);
		
		_sdf.SetTexture(orig_image);
		im = combined;
		
    }
}

void ShowProcessedImage( int& _zoom, bool& _apply, int _viewMode, SDF& _sdf, sf::Image& image )
{
    ImGui::Begin( "Processed Picture" );

    const float scale = ui::Zoom( _zoom, 10, 4000 );

    // Process the final render with the last processed signed distance field resized.
    // The scale will be applied while rendering to apply smooth.
    _sdf.ProcessAlphaTest( scale );//REMOVE

    // Retrieve the sprite that the user want to see.
    // If it is not the alpha tested result, we have to apply the scale.
	
	// REMOVE THIS SECTION --------------------
    std::reference_wrapper<sf::Sprite> processedSprite = std::ref( _sdf.GetAlphaSprite() );
    if( _viewMode == ui::ViewMode::NoResize )
    {
        processedSprite = std::ref( _sdf.GetSDFSprite() );
        processedSprite.get().setScale( scale, scale );
    }

    else if( _viewMode == ui::ViewMode::Resized )
    {
        processedSprite = std::ref( _sdf.GetResizeSprite() );
        processedSprite.get().setScale( scale, scale );
    }
	// ----------------------------------
	if (_apply) {
		_sdf.ResizeImage(image);
		_apply = false;
	}
	sf::Texture t;
	t.loadFromImage(image);
	sf::Sprite s;
	s.setTexture(t, true);
	s.setScale(scale, scale);

    ui::Image( s );
	
    // Resture default scale.
    processedSprite.get().setScale( 1.0f, 1.0f );

    ImGui::End();
}


int main(int argc, char* argv[])
{
	if (argc > 1) {
		/*
		usage:
		need input filepath, output filepath, spread, resize factor
		*/
		
		// get args
		std::string inFile(argv[1]);
		std::string outFile(argv[2]);
		std::string spread_str(argv[3]);
		std::string resizeFactor_str(argv[4]);
		int spread = std::stoi(spread_str);
		int resizeFactor = std::stoi(resizeFactor_str);
		
		// process SDF
		SDF sdf;
		sdf.Init(DATA_PATH);
		sdf.SetTexture(inFile);
		sdf.imageType = ImageType::Grey;
		sdf.spread = spread; //from argument
		sdf.resizeFactor = resizeFactor; //from argument
		sdf.Process();
		
		// save output image
		sf::Image imageToSave = sdf.GetResizeSprite().getTexture()->copyToImage();
		imageToSave.saveToFile(outFile);
		return 0;
	}
	
    sf::RenderWindow window( sf::VideoMode( WINDOW_SIZE_X, WINDOW_SIZE_Y ),
                             "SDF Processor For Helldivers 2" );
    window.setFramerateLimit( 60 );

    ImGui::SFML::Init( window );

    ImGui::GetIO().FontGlobalScale = GLOBAL_SCALE;
    ImGui::GetStyle().ScaleAllSizes( GLOBAL_SCALE );

    sf::Clock deltaClock;

    SDF sdf;
    sdf.Init( DATA_PATH );
    sdf.SetTexture( DATA_PATH + "Images/Circle1024.png" );

    int viewMode = ui::ViewMode::AlphaTested;
    int zoomOriginal = 100;
    int zoomProcessed = 100;
	sf::Image im;
    bool autoApply = false;
    bool apply = true; // True to process when oppening the app.


    while( window.isOpen() )
    {
        sf::Event event;
        while( window.pollEvent( event ) )
        {
            ImGui::SFML::ProcessEvent( event );

            if( event.type == sf::Event::Closed )
                window.close();
			if( event.type == sf::Event::LostFocus ) // reset key down values to false
			{
				for(int i = 0; i < 512; i++){
					ImGui::GetIO().KeysDown[i] = false;
				}
				ImGui::GetIO().KeyCtrl = false;
				ImGui::GetIO().KeyShift = false;
				ImGui::GetIO().KeyAlt = false;
				ImGui::GetIO().KeySuper = false;
			}
        }

        ImGui::SFML::Update( window, deltaClock.restart() );

        /********************************************************/
        /***             Retrieve user settings               ***/
        /********************************************************/

        ImGui::Begin( "SDF Settings" );

        ui::SpreadResize( sdf.spread, sdf.resizeFactor ); 
		sf::Vector2u imageSize = sdf.GetSourceSprite().getTexture()->getSize();
		
		char buf[100];
		sprintf_s(buf, 100, "(%dx%d) -> (%dx%d)", imageSize.x, imageSize.y, imageSize.x/sdf.resizeFactor, imageSize.y/sdf.resizeFactor);
		
		ImGui::Text(buf);
        ui::Apply( apply, autoApply ); ImGui::Separator();

        ImGui::End();

        /********************************************************/
        /***                   Show Results                   ***/
        /********************************************************/

        ShowSourceImage( zoomOriginal, sdf );
		
		ProcessImage(apply, sdf, im);

        ShowProcessedImage( zoomProcessed, apply, viewMode, sdf, im);

        /********************************************************/
        /***          Save to PNG functionnality              ***/
        /********************************************************/
		sf::Texture t;
		t.loadFromImage(im);
		sf::Sprite s;
		s.setTexture(t, true);
		ui::FileMenu( sdf, s, apply );
		
		if ((ImGui::GetIO().KeyCtrl)) { // 14 for O, 18 for S
			if (ImGui::GetIO().KeysDown[14]) {
				ui::LoadImage2(sdf);
			} else if (ImGui::GetIO().KeysDown[18]) {
				ui::SaveImage2(s);
			}
		}

        window.clear();
        ImGui::SFML::Render( window );
        window.display();
    }

    ImGui::SFML::Shutdown();

    return 0;
}