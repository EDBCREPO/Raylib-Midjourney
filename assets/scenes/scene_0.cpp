#pragma once

#include <nodepp/https.h>
#include <nodepp/url.h>

namespace rl { namespace scene {

    void scene_0( ptr_t<Scene> self ) {

        struct NODE {
            Texture img; float a1, a2;
            string_t bff1, bff2; 
            uint state=0, a3=0, a4=0; 
        };  ptr_t<NODE> obj = new NODE();

        event_t<> onGPT; onGPT([=](){
            if( obj->bff1.empty() ){ return; } console::log( "onGPT" );

            fetch_t args; ssl_t ssl;
                    args.method  = "POST"; 
                    args.url     = process::env::get("API");
                    
                    args.headers = header_t({
                        { "Content-Type", path::mimetype(".json") },
                        { "Host", url::hostname( args.url ) },
                    }); 
                    
                    args.body = json::stringify( object_t({
                        { "key", process::env::get("KEY") },
                        { "width", "512" }, { "height", "512" }, 
                        { "samples", "1" }, { "num_inference_steps", "20" },
                        { "prompt", regex::replace_all( obj->bff1, "\n", "" ) },
                        { "negative_prompt", regex::replace_all( obj->bff2, "\n", "" ) }
                    }) );

            UnloadTexture( obj->img ); obj->a4=0;
            obj->bff1.clear(); obj->bff2.clear(); 
            obj->a1=0.0f; obj->a2=0.0f;

            https::fetch( args, &ssl ).then([=]( https_t cli ){
            try {

                auto data = json::parse( regex::replace_all( stream::await( cli ), "\\\\", "" ) );
                
                fetch_t args; 
                        args.method  = "GET";
                        args.url     = data["output"][0].as<string_t>();
                        args.headers = header_t({
                            { "Host", url::hostname(args.url) }
                        });

                https::fetch( args, &ssl ).then([=]( https_t cli ){

                    auto data = stream::await( cli );
                    auto img  = LoadImageFromMemory( ".jpg", (uchar*) data.get(), data.size() ); 
                    
                    if( !IsImageReady( img ) ){ return; }
                    obj->img = LoadTextureFromImage(img); 
                    UnloadImage( img ); obj->a4=1;

                }).fail([]( except_t err ){ console::error( "something went wrong" ); });

            } catch( except_t err )    { console::error( "something went wrong" ); }
            }).fail([=]( except_t err ){ console::error( "something went wrong" ); });

        });

        self->onDraw([=](){ ClearBackground( RAYWHITE );

            auto w = type::cast<float>( GetRenderWidth()  );
            auto h = type::cast<float>( GetRenderHeight() );

            if( obj->a4 ){
                DrawTexturePro( obj->img, { 0, 0, 512, 512 }, 
                    { h*2/100, h*2/100, h*96/100, h*96/100 }, 
                    { 0, 0 }, 0.0f, WHITE 
                );
            } else {
                DrawCircleSectorLines( 
                    { h*50/100, h*50/100 }, 50, 
                    obj->a1, obj->a2, 10, BLACK 
                );
            }

            DrawRectangleLines( h*2/100, h*2/100, h*96/100, h*96/100, BLACK );

            DrawText( "Prompts:", h+3, h*2/100+3, 12, BLACK );
            DrawRectangleLines( h, h*2/100, w*35/100, h*45/100, BLACK );
            DrawText( obj->bff1.to_lower_case().get(), h+3, h*2/100+3, 12, BLACK );

            DrawText( "Negative Prompts:", h+3, h*50/100+3, 12, BLACK );
            DrawRectangleLines( h, h*50/100, w*35/100, h*40/100, BLACK );
            DrawText( obj->bff2.to_lower_case().get(), h+3, h*50/100+3, 12, BLACK );

            if( GuiButton( { h, h*93/100, w*35/100, h*5/100 }, "SEND" ) ){ onGPT.emit(); }

        });

        self->onLoop([=]( float delta ){
            
            auto mouse = GetMousePosition();
            auto w     = type::cast<float>( GetRenderWidth()  );
            auto h     = type::cast<float>( GetRenderHeight() );

            if( obj->a3 ){
                obj->a1 += delta * 100; obj->a2 += delta * 200;
            } else {
                obj->a2 += delta * 100; obj->a1 += delta * 200; 
            }

            [=](){
            coStart
                obj->a3 =! obj->a3; coDelay(3000);
            coStop
            }();

            if( IsMouseButtonReleased(0) ){

                if( CheckCollisionPointRec( mouse, { h, h*2/100, w*35/100, h*45/100 } ) ){
                    console::log( ">1<" ); obj->state = 1;
                }
                
                elif( CheckCollisionPointRec( mouse, { h, h*50/100, w*35/100, h*40/100 } ) ){
                    console::log( ">2<" ); obj->state = 2;
                }

                else {
                    console::log( ">0<" ); obj->state = 0;
                }

            }

            [=](){  
                static int key;
            coStart

                while( (key=GetKeyPressed())==0 ){ coNext; }
                
                    if( obj->state==1 && key==259 ){ obj->bff1.pop(); }
                    if( obj->state==2 && key==259 ){ obj->bff2.pop(); } 

                    if( !string::is_print(key) )  { coEnd;  }

                    if( obj->state==1 ){ 
                    if( obj->bff1.size()%37==0 ){ obj->bff1.push('\n'); }
                        obj->bff1.push( (char) key ); 
                    }

                    if( obj->state==2 ){ 
                    if( obj->bff2.size()%37==0 ){ obj->bff2.push('\n'); }
                        obj->bff2.push( (char) key ); 
                    }

            coStop
            }();

        });

        self->onRemove([=](){
            if( IsTextureReady( obj->img ) ){ UnloadTexture( obj->img ); }
        });

    }

}}
