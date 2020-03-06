#include <stdio.h>

#include <vector>
using namespace std;

#include "../apps.h"
#include "../yocto_modelio.h"
using namespace yocto;
using namespace opengl;

int main(int num_args, const char* args[]) {
  auto mesh = load_shape(args[1]);
  mesh_viewer(mesh);
}
