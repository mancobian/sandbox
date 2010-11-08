#include <Engine>

using namespace RSSD;

int main(int argc, char **argv)
{
  const std::string SANDBOX_SCENE_PATH("/home/arris/Code/rssd/sandbox/common/scenes/shipping/Shipping_Arris.osm");

  Engine engine;
  engine.loadScene(SANDBOX_SCENE_PATH);
  engine.start();
  return 0;
}
