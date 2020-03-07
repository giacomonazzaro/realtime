#include <stdio.h>

#include <vector>
using namespace std;

#include "../mesh_viewer.h"
#include "../yocto_modelio.h"
using namespace yocto;
using namespace opengl;

int main(int num_args, const char* args[]) {
  auto viewer = mesh_viewer{};
  auto mesh   = load_shape(args[1]);

  viewer.viewport = {500, 500};
  // viewer.vertex_filename   = "shaders/test.vert";
  viewer.fragment_filename = "shaders/test.frag";
  run(viewer, mesh);
}
