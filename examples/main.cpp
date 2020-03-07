#include <stdio.h>

#include <vector>
using namespace std;

#include "../mesh_viewer.h"
#include "../yocto_modelio.h"
using namespace yocto;
using namespace opengl;

int main(int num_args, const char* args[]) {
  auto mesh    = load_shape(args[1]);
  auto options = mesh_viewer_options{};

  options.viewport          = {500, 500};
  options.fragment_filename = "shaders/test.frag";
  mesh_viewer(mesh, options);
}
