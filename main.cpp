#include <nodepp/nodepp.h>
#include <nodepp/https.h>
#include <nodepp/path.h>
#include <nodepp/url.h>

using namespace nodepp;

#include "assets/script/game.cpp"
#include "assets/scenes/scene_0.cpp"

void onMain() {

    process::env::init( ".env" );

    rl::Init( { 750, 500 }, 60, "midjourney" );

    rl::AppendScene( rl::scene::scene_0 );

    rl::onClose([](){
        console::log("Closed");
        rl::Close();
    });

}