#include "yocto_opengl.h"

#include <algorithm>
#include <cstdarg>
#include <deque>
#include <mutex>

#include "yocto_common.h"
#include "yocto_commonio.h"
#include "yocto_opengl.h"

#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
#endif
#include "ext/glad/glad.h"

// -----------------------------------------------------------------------------
// LOW-LEVEL OPENGL FUNCTIONS
// -----------------------------------------------------------------------------
// namespace yocto {
namespace opengl {
using namespace yocto;

bool init_opengl() {
  // init gl extensions
  if (!gladLoadGL())
    throw std::runtime_error("cannot initialize OpenGL extensions");
  return true;
}

void check_glerror() {
  auto error = glGetError();
  if (error != GL_NO_ERROR) printf("gl error: %d (%x)\n", error, error);
  assert(error == GL_NO_ERROR);
}

void clear_glframebuffer(const vec4f& color, bool clear_depth) {
  glClearColor(color.x, color.y, color.z, color.w);
  if (clear_depth) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
  } else {
    glClear(GL_COLOR_BUFFER_BIT);
  }
}

void set_glviewport(const vec4i& viewport) {
  glViewport(viewport.x, viewport.y, viewport.z, viewport.w);
}

void set_glwireframe(bool enabled) {
  if (enabled)
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  else
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void set_glblending(bool enabled) {
  if (enabled) {
    glEnable(GL_BLEND);
    glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
    glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);
  } else {
    glDisable(GL_BLEND);
  }
}

// -----------------------------------------------------------------------------
// HIGH-LEVEL OPENGL IMAGE DRAWING
// -----------------------------------------------------------------------------

// init image program
void init_glimage_program(opengl_image& glimage) {
  if (glimage.program) return;
  auto vert =
      R"(
      #version 330
      in vec2 texcoord;
      out vec2 frag_texcoord;
      uniform vec2 window_size, image_size;
      uniform vec2 image_center;
      uniform float image_scale;
      void main() {
          vec2 pos = (texcoord - vec2(0.5,0.5)) * image_size * image_scale + image_center;
          gl_Position = vec4(2 * pos.x / window_size.x - 1, 1 - 2 * pos.y / window_size.y, 0, 1);
          frag_texcoord = texcoord;
      }
      )";
#if 0
  auto vert = R"(
          #version 330
          in vec2 texcoord;
          out vec2 frag_texcoord;
          uniform vec2 window_size, image_size, border_size;
          uniform vec2 image_center;
          uniform float image_scale;
          void main() {
              vec2 pos = (texcoord - vec2(0.5,0.5)) * (image_size + border_size*2) * image_scale + image_center;
              gl_Position = vec4(2 * pos.x / window_size.x - 1, 1 - 2 * pos.y / window_size.y, 0.1, 1);
              frag_texcoord = texcoord;
          }
      )";
#endif
  auto frag =
      R"(
      #version 330
      in vec2 frag_texcoord;
      out vec4 frag_color;
      uniform sampler2D txt;
      void main() {
          frag_color = texture(txt, frag_texcoord);
      }
      )";
#if 0
    auto frag = R"(
            #version 330
            in vec2 frag_texcoord;
            out vec4 frag_color;
            uniform vec2 image_size, border_size;
            uniform float image_scale;
            void main() {
                ivec2 imcoord = ivec2(frag_texcoord * (image_size + border_size*2) - border_size);
                ivec2 tilecoord = ivec2(frag_texcoord * (image_size + border_size*2) * image_scale - border_size);
                ivec2 tile = tilecoord / 16;
                if(imcoord.x <= 0 || imcoord.y <= 0 || 
                    imcoord.x >= image_size.x || imcoord.y >= image_size.y) frag_color = vec4(0,0,0,1);
                else if((tile.x + tile.y) % 2 == 0) frag_color = vec4(0.1,0.1,0.1,1);
                else frag_color = vec4(0.3,0.3,0.3,1);
            }
        )";
#endif

  init_glprogram(glimage.program, vert, frag);
  init_glshape(glimage.shape);
  add_vertex_attribute(
      glimage.shape, vector<vec2f>{{1, 0}, {1, 1}, {0, 0}, {0, 1}});
  glimage.shape.type = opengl_shape::type::triangles;
}

// update image data
void update_glimage(
    opengl_image& glimage, const image<vec4f>& img, bool linear, bool mipmap) {
  init_glimage_program(glimage);
  if (!glimage.texture) {
    init_gltexture(glimage.texture, img, false, linear, mipmap);
  } else if (glimage.texture.size != img.size() ||
             glimage.texture.linear != linear ||
             glimage.texture.mipmap != mipmap) {
    delete_gltexture(glimage.texture);
    init_gltexture(glimage.texture, img, false, linear, mipmap);
  } else {
    update_gltexture(glimage.texture, img, mipmap);
  }
}
void update_glimage(
    opengl_image& glimage, const image<vec4b>& img, bool linear, bool mipmap) {
  init_glimage_program(glimage);
  if (!glimage.texture) {
    init_gltexture(glimage.texture, img, false, linear, mipmap);
  } else if (glimage.texture.size != img.size() ||
             glimage.texture.linear != linear ||
             glimage.texture.mipmap != mipmap) {
    delete_gltexture(glimage.texture);
    init_gltexture(glimage.texture, img, false, linear, mipmap);
  } else {
    update_gltexture(glimage.texture, img, mipmap);
  }
}

void update_glimage_region(opengl_image& glimage, const image<vec4f>& img,
    const image_region& region) {
  if (!glimage) throw std::runtime_error("glimage is not initialized");
  update_gltexture_region(glimage.texture, img, region, glimage.texture.mipmap);
}
void update_glimage_region(opengl_image& glimage, const image<vec4b>& img,
    const image_region& region) {
  if (!glimage) throw std::runtime_error("glimage is not initialized");
  update_gltexture_region(glimage.texture, img, region, glimage.texture.mipmap);
}

// draw image
void draw_glimage(opengl_image& glimage, const draw_glimage_params& params) {
  check_glerror();
  set_glviewport(params.framebuffer);
  clear_glframebuffer(params.background);
  bind_glprogram(glimage.program);
  set_gluniform_texture(glimage.program, "txt", glimage.texture, 0);
  set_gluniform(glimage.program, "window_size",
      vec2f{(float)params.window.x, (float)params.window.y});
  set_gluniform(glimage.program, "image_size",
      vec2f{(float)glimage.texture.size.x, (float)glimage.texture.size.y});
  set_gluniform(glimage.program, "image_center", params.center);
  set_gluniform(glimage.program, "image_scale", params.scale);
  draw_glshape(glimage.shape);
  unbind_opengl_program();
  check_glerror();
}

// -----------------------------------------------------------------------------
// HIGH-LEVEL OPENGL FUNCTIONS
// -----------------------------------------------------------------------------

// Initialize an OpenGL scene
void make_glscene(opengl_scene& glscene) {
#ifndef _WIN32
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Woverlength-strings"
#endif

  static const char* vertex =
      R"(
        #version 330

        layout(location = 0) in vec3 vert_pos;            // vertex position (in mesh coordinate frame)
        layout(location = 1) in vec3 vert_norm;           // vertex normal (in mesh coordinate frame)
        layout(location = 2) in vec2 vert_texcoord;       // vertex texcoords
        layout(location = 3) in vec4 vert_color;          // vertex color
        layout(location = 4) in vec4 vert_tangsp;         // vertex tangent space

        uniform mat4 shape_xform;                    // shape transform
        uniform mat4 shape_xform_invtranspose;       // shape transform
        uniform float shape_normal_offset;           // shape normal offset

        uniform mat4 cam_xform;          // camera xform
        uniform mat4 cam_xform_inv;      // inverse of the camera frame (as a matrix)
        uniform mat4 cam_proj;           // camera projection

        out vec3 pos;                   // [to fragment shader] vertex position (in world coordinate)
        out vec3 norm;                  // [to fragment shader] vertex normal (in world coordinate)
        out vec2 texcoord;              // [to fragment shader] vertex texture coordinates
        out vec4 color;                 // [to fragment shader] vertex color
        out vec4 tangsp;                // [to fragment shader] vertex tangent space

        // main function
        void main() {
            // copy values
            pos = vert_pos;
            norm = vert_norm;
            tangsp = vert_tangsp;

            // normal offset
            if(shape_normal_offset != 0) {
                pos += shape_normal_offset * norm;
            }

            // world projection
            pos = (shape_xform * vec4(pos,1)).xyz;
            norm = (shape_xform_invtranspose * vec4(norm,0)).xyz;
            tangsp.xyz = (shape_xform * vec4(tangsp.xyz,0)).xyz;

            // copy other vertex properties
            texcoord = vert_texcoord;
            color = vert_color;

            // clip
            gl_Position = cam_proj * cam_xform_inv * vec4(pos,1);
        }
        )";

  static const char* fragment =
      R"(
        #version 330

        float pif = 3.14159265;

        uniform bool eyelight;         // eyelight shading
        uniform vec3 lamb;             // ambient light
        uniform int lnum;              // number of lights
        uniform int ltype[16];         // light type (0 -> point, 1 -> directional)
        uniform vec3 lpos[16];         // light positions
        uniform vec3 lke[16];          // light intensities

        void evaluate_light(int lid, vec3 pos, out vec3 cl, out vec3 wi) {
            cl = vec3(0,0,0);
            wi = vec3(0,0,0);
            if(ltype[lid] == 0) {
                // compute point light color at pos
                cl = lke[lid] / pow(length(lpos[lid]-pos),2);
                // compute light direction at pos
                wi = normalize(lpos[lid]-pos);
            }
            else if(ltype[lid] == 1) {
                // compute light color
                cl = lke[lid];
                // compute light direction
                wi = normalize(lpos[lid]);
            }
        }

        vec3 brdfcos(int etype, vec3 ke, vec3 kd, vec3 ks, float rs, float op,
            vec3 n, vec3 wi, vec3 wo) {
            if(etype == 0) return vec3(0);
            vec3 wh = normalize(wi+wo);
            float ns = 2/(rs*rs)-2;
            float ndi = dot(wi,n), ndo = dot(wo,n), ndh = dot(wh,n);
            if(etype == 1) {
                return ((1+dot(wo,wi))/2) * kd/pif;
            } else if(etype == 2) {
                float si = sqrt(1-ndi*ndi);
                float so = sqrt(1-ndo*ndo);
                float sh = sqrt(1-ndh*ndh);
                if(si <= 0) return vec3(0);
                vec3 diff = si * kd / pif;
                if(sh<=0) return diff;
                float d = ((2+ns)/(2*pif)) * pow(si,ns);
                vec3 spec = si * ks * d / (4*si*so);
                return diff+spec;
            } else if(etype == 3 || etype == 4) {
                if(ndi<=0 || ndo <=0) return vec3(0);
                vec3 diff = ndi * kd / pif;
                if(ndh<=0) return diff;
                if(etype == 4) {
                    float d = ((2+ns)/(2*pif)) * pow(ndh,ns);
                    vec3 spec = ndi * ks * d / (4*ndi*ndo);
                    return diff+spec;
                } else {
                    float cos2 = ndh * ndh;
                    float tan2 = (1 - cos2) / cos2;
                    float alpha2 = rs * rs;
                    float d = alpha2 / (pif * cos2 * cos2 * (alpha2 + tan2) * (alpha2 + tan2));
                    float lambda_o = (-1 + sqrt(1 + (1 - ndo * ndo) / (ndo * ndo))) / 2;
                    float lambda_i = (-1 + sqrt(1 + (1 - ndi * ndi) / (ndi * ndi))) / 2;
                    float g = 1 / (1 + lambda_o + lambda_i);
                    vec3 spec = ndi * ks * d * g / (4*ndi*ndo);
                    return diff+spec;
                }
            }
        }

        uniform int elem_type;
        uniform bool elem_faceted;
        uniform vec4 highlight;   // highlighted color

        uniform int mat_type;          // material type
        uniform vec3 mat_ke;           // material ke
        uniform vec3 mat_kd;           // material kd
        uniform vec3 mat_ks;           // material ks
        uniform float mat_rs;          // material rs
        uniform float mat_op;          // material op

        uniform bool mat_ke_txt_on;    // material ke texture on
        uniform sampler2D mat_ke_txt;  // material ke texture
        uniform bool mat_kd_txt_on;    // material kd texture on
        uniform sampler2D mat_kd_txt;  // material kd texture
        uniform bool mat_ks_txt_on;    // material ks texture on
        uniform sampler2D mat_ks_txt;  // material ks texture
        uniform bool mat_rs_txt_on;    // material rs texture on
        uniform sampler2D mat_rs_txt;  // material rs texture
        uniform bool mat_op_txt_on;    // material op texture on
        uniform sampler2D mat_op_txt;  // material op texture

        uniform bool mat_norm_txt_on;    // material norm texture on
        uniform sampler2D mat_norm_txt;  // material norm texture

        uniform bool mat_double_sided;   // double sided rendering

        uniform mat4 shape_xform;              // shape transform
        uniform mat4 shape_xform_invtranspose; // shape transform

        bool evaluate_material(vec2 texcoord, vec4 color, out vec3 ke, 
                           out vec3 kd, out vec3 ks, out float rs, out float op) {
            if(mat_type == 0) {
                ke = mat_ke;
                kd = vec3(0,0,0);
                ks = vec3(0,0,0);
                op = 1;
                return false;
            }

            ke = color.xyz * mat_ke;
            kd = color.xyz * mat_kd;
            ks = color.xyz * mat_ks;
            rs = mat_rs;
            op = color.w * mat_op;

            vec4 ke_txt = (mat_ke_txt_on) ? texture(mat_ke_txt,texcoord) : vec4(1,1,1,1);
            vec4 kd_txt = (mat_kd_txt_on) ? texture(mat_kd_txt,texcoord) : vec4(1,1,1,1);
            vec4 ks_txt = (mat_ks_txt_on) ? texture(mat_ks_txt,texcoord) : vec4(1,1,1,1);
            vec4 rs_txt = (mat_rs_txt_on) ? texture(mat_rs_txt,texcoord) : vec4(1,1,1,1);
            vec4 op_txt = (mat_op_txt_on) ? texture(mat_op_txt,texcoord) : vec4(1,1,1,1);

            // get material color from textures and adjust values
            if(mat_type == 1) {
                ke *= ke_txt.xyz;
                kd *= kd_txt.xyz;
                ks *= ks_txt.xyz;
                rs *= rs_txt.y;
                rs = rs*rs;
                op *= op_txt.x * kd_txt.w;
            } else if(mat_type == 2) {
                ke *= ke_txt.xyz;
                vec3 kb = kd * kd_txt.xyz;
                float km = ks.x * ks_txt.z;
                kd = kb * (1 - km);
                ks = kb * km + vec3(0.04) * (1 - km);
                rs *= ks_txt.y;
                rs = rs*rs;
                op *= kd_txt.w;
            } else if(mat_type == 3) {
                ke *= ke_txt.xyz;
                kd *= kd_txt.xyz;
                ks *= ks_txt.xyz;
                float gs = (1 - rs) * ks_txt.w;
                rs = 1 - gs;
                rs = rs*rs;
                op *= kd_txt.w;
            }

            return true;
        }

        vec3 apply_normal_map(vec2 texcoord, vec3 norm, vec4 tangsp) {
            if(!mat_norm_txt_on) return norm;
            vec3 tangu = normalize((shape_xform * vec4(normalize(tangsp.xyz),0)).xyz);
            vec3 tangv = normalize(cross(norm, tangu));
            if(tangsp.w < 0) tangv = -tangv;
            vec3 texture = 2 * texture(mat_norm_txt,texcoord).xyz - 1;
            texture.y = -texture.y;
            return normalize( tangu * texture.x + tangv * texture.y + norm * texture.z );
        }

        in vec3 pos;                   // [from vertex shader] position in world space
        in vec3 norm;                  // [from vertex shader] normal in world space (need normalization)
        in vec2 texcoord;              // [from vertex shader] texcoord
        in vec4 color;                 // [from vertex shader] color
        in vec4 tangsp;                // [from vertex shader] tangent space

        uniform vec3 cam_pos;          // camera position
        uniform mat4 cam_xform_inv;      // inverse of the camera frame (as a matrix)
        uniform mat4 cam_proj;           // camera projection

        uniform float exposure; 
        uniform float gamma;

        out vec4 frag_color;      

        vec3 triangle_normal(vec3 pos) {
            vec3 fdx = dFdx(pos); 
            vec3 fdy = dFdy(pos); 
            return normalize((shape_xform * vec4(normalize(cross(fdx, fdy)), 0)).xyz);
        }

        // main
        void main() {
            // view vector
            vec3 wo = normalize(cam_pos - pos);

            // prepare normals
            vec3 n;
            if(elem_faceted) {
                n = triangle_normal(pos);
            } else {
                n = normalize(norm);
            }

            // apply normal map
            n = apply_normal_map(texcoord, n, tangsp);

            // use faceforward to ensure the normals points toward us
            if(mat_double_sided) n = faceforward(n,-wo,n);

            // get material color from textures
            vec3 brdf_ke, brdf_kd, brdf_ks; float brdf_rs, brdf_op;
            bool has_brdf = evaluate_material(texcoord, color, brdf_ke, brdf_kd, brdf_ks, brdf_rs, brdf_op);

            // exit if needed
            if(brdf_op < 0.005) discard;

            // check const color
            if(elem_type == 0) {
                frag_color = vec4(brdf_ke,brdf_op);
                return;
            }

            // emission
            vec3 c = brdf_ke;

            // check early exit
            if(brdf_kd != vec3(0,0,0) || brdf_ks != vec3(0,0,0)) {
                // eyelight shading
                if(eyelight) {
                    vec3 wi = wo;
                    c += pif * brdfcos((has_brdf) ? elem_type : 0, brdf_ke, brdf_kd, brdf_ks, brdf_rs, brdf_op, n,wi,wo);
                } else {
                    // accumulate ambient
                    c += lamb * brdf_kd;
                    // foreach light
                    for(int lid = 0; lid < lnum; lid ++) {
                        vec3 cl = vec3(0,0,0); vec3 wi = vec3(0,0,0);
                        evaluate_light(lid, pos, cl, wi);
                        c += cl * brdfcos((has_brdf) ? elem_type : 0, brdf_ke, brdf_kd, brdf_ks, brdf_rs, brdf_op, n,wi,wo);
                    }
                }
            }

            // final color correction
            c = pow(c * pow(2,exposure), vec3(1/gamma));

            // highlighting
            if(highlight.w > 0) {
                if(mod(int(gl_FragCoord.x)/4 + int(gl_FragCoord.y)/4, 2)  == 0)
                    c = highlight.xyz * highlight.w + c * (1-highlight.w);
            }

            // output final color by setting gl_FragColor
            frag_color = vec4(c,brdf_op);
        }
        )";
#ifndef _WIN32
#pragma GCC diagnostic pop
#endif

  // load program
  init_glprogram(glscene.program, vertex, fragment);
}

// Draw a shape
void draw_glinstance(opengl_scene& state, const opengl_instance& instance,
    const draw_glscene_params& params) {
  if (instance.shape < 0 || instance.shape > state.shapes.size()) return;
  if (instance.material < 0 || instance.material > state.materials.size())
    return;

  auto& shape    = state.shapes[instance.shape];
  auto& material = state.materials[instance.material];

  if (!shape.vao) return;

  set_gluniform(state.program, "shape_xform", mat4f(instance.frame));
  set_gluniform(state.program, "shape_xform_invtranspose",
      transpose(mat4f(inverse(instance.frame, params.non_rigid_frames))));
  set_gluniform(state.program, "shape_normal_offset", 0.0f);
  set_gluniform(state.program, "highlight",
      instance.highlighted ? vec4f{1, 1, 0, 1} : zero4f);

  auto mtype = 2;
  if (material.gltf_textures) mtype = 3;
  set_gluniform(state.program, "mat_type", mtype);
  set_gluniform(state.program, "mat_ke", material.emission);
  set_gluniform(state.program, "mat_kd", material.diffuse);
  set_gluniform(state.program, "mat_ks", vec3f{material.metallic});
  set_gluniform(state.program, "mat_rs", material.roughness);
  set_gluniform(state.program, "mat_op", material.opacity);
  set_gluniform(state.program, "mat_double_sided", (int)params.double_sided);
  if (material.emission_map >= 0) {
    set_gluniform_texture(state.program, "mat_ke_txt", "mat_ke_txt_on",
        state.textures.at(material.emission_map), 0);
  } else {
    set_gluniform_texture(
        state.program, "mat_ke_txt", "mat_ke_txt_on", opengl_texture{}, 0);
  }
  if (material.diffuse_map >= 0) {
    set_gluniform_texture(state.program, "mat_kd_txt", "mat_kd_txt_on",
        state.textures.at(material.diffuse_map), 1);
  } else {
    set_gluniform_texture(
        state.program, "mat_kd_txt", "mat_kd_txt_on", opengl_texture{}, 1);
  }
  if (material.metallic_map >= 0) {
    set_gluniform_texture(state.program, "mat_ks_txt", "mat_ks_txt_on",
        state.textures.at(material.metallic_map), 2);
  } else {
    set_gluniform_texture(
        state.program, "mat_ks_txt", "mat_ks_txt_on", opengl_texture{}, 2);
  }
  if (material.roughness_map >= 0) {
    set_gluniform_texture(state.program, "mat_rs_txt", "mat_rs_txt_on",
        state.textures.at(material.roughness_map), 3);
  } else {
    set_gluniform_texture(
        state.program, "mat_rs_txt", "mat_rs_txt_on", opengl_texture{}, 3);
  }
  if (material.normal_map >= 0) {
    set_gluniform_texture(state.program, "mat_norm_txt", "mat_norm_txt_on",
        state.textures.at(material.normal_map), 5);
  } else {
    set_gluniform_texture(
        state.program, "mat_norm_txt", "mat_norm_txt_on", opengl_texture{}, 5);
  }

  draw_glshape(shape);

#if 0
    if ((vbos.gl_edges && edges && !wireframe) || highlighted) {
        enable_glculling(false);
        check_glerror();
        set_gluniform(state.program, "mtype"), 0);
        glUniform3f(glGetUniformLocation(state.program, "ke"), 0, 0, 0);
        set_gluniform(state.program, "op"), material.op);
        set_gluniform(state.program, "shp_normal_offset"), 0.01f);
        check_glerror();
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbos.gl_edges);
        glDrawElements(GL_LINES, vbos.triangles.size() * 3, GL_UNSIGNED_INT, nullptr);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        check_glerror();
    }
#endif
  if (params.edges) throw std::runtime_error("edges are momentarily disabled");
}

// Display a scene
void draw_glscene(opengl_scene& state, const vec4i& viewport,
    const draw_glscene_params& params) {
  auto& glcamera      = state.cameras.at(params.camera);
  auto  camera_aspect = (float)viewport.z / (float)viewport.w;
  auto  camera_yfov =
      camera_aspect >= 0
          ? (2 * yocto::atan(
                     glcamera.film / (camera_aspect * 2 * glcamera.lens)))
          : (2 * yocto::atan(glcamera.film / (2 * glcamera.lens)));
  auto camera_view = mat4f(inverse(glcamera.frame));
  auto camera_proj = perspective_mat(
      camera_yfov, camera_aspect, params.near, params.far);

  clear_glframebuffer(params.background);
  set_glviewport(viewport);

  bind_glprogram(state.program);
  set_gluniform(state.program, "cam_pos", glcamera.frame.o);
  set_gluniform(state.program, "cam_xform_inv", camera_view);
  set_gluniform(state.program, "cam_proj", camera_proj);
  set_gluniform(state.program, "eyelight", (int)params.eyelight);
  set_gluniform(state.program, "exposure", params.exposure);
  set_gluniform(state.program, "gamma", params.gamma);

  if (!params.eyelight) {
    set_gluniform(state.program, "lamb", zero3f);
    set_gluniform(state.program, "lnum", (int)state.lights.size());
    for (auto i = 0; i < state.lights.size(); i++) {
      auto is = std::to_string(i);
      set_gluniform(state.program, ("lpos[" + is + "]").c_str(),
          state.lights[i].position);
      set_gluniform(
          state.program, ("lke[" + is + "]").c_str(), state.lights[i].emission);
      set_gluniform(state.program, ("ltype[" + is + "]").c_str(),
          (int)state.lights[i].type);
    }
  }

  if (params.wireframe) set_glwireframe(true);
  check_glerror();

  for (auto& instance : state.instances) {
    draw_glinstance(state, instance, params);
  }

  unbind_opengl_program();
  if (params.wireframe) set_glwireframe(false);
}

// -----------------------------------------------------------------------------
// LOW-LEVEL OPENGL FUNCTIONS
// -----------------------------------------------------------------------------

void reload_glprogram(opengl_program& program) {
  load_text(program.vertex_filename, program.vertex_code);
  load_text(program.fragment_filename, program.fragment_code);
}

void load_glprogram(opengl_program& program, const string& vertex_filename,
    const string& fragment_filename) {
  program.vertex_filename   = vertex_filename;
  program.fragment_filename = fragment_filename;
  reload_glprogram(program);
}

opengl_program create_glprogram(const string& vertex_filename,
    const string& fragment_filename, bool abort_on_error) {
  opengl_program program;
  load_glprogram(program, vertex_filename, fragment_filename);
  init_glprogram(program, abort_on_error);
  return program;
}

bool init_glprogram(opengl_program& program, bool abort_on_error) {
  return init_glprogram(program, program.vertex_code.c_str(),
      program.fragment_code.c_str(), abort_on_error);
}

bool init_glprogram(opengl_program& program, const char* vertex,
    const char* fragment, bool abort_on_error) {
  check_glerror();
  int  errflags;
  char errbuf[10000];

  // create vertex
  check_glerror();
  program.vertex_shader_id = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(program.vertex_shader_id, 1, &vertex, NULL);
  glCompileShader(program.vertex_shader_id);
  glGetShaderiv(program.vertex_shader_id, GL_COMPILE_STATUS, &errflags);
  if (!errflags) {
    glGetShaderInfoLog(program.vertex_shader_id, 10000, 0, errbuf);
    errbuf[6] = '\n';
    printf("\n*** VERTEX SHADER COMPILATION %s\n", errbuf);
    if (abort_on_error) {
      throw std::runtime_error("shader compilation failed\n");
    }
    return false;
  }
  check_glerror();

  // create fragment
  check_glerror();
  program.fragment_shader_id = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(program.fragment_shader_id, 1, &fragment, NULL);
  glCompileShader(program.fragment_shader_id);
  glGetShaderiv(program.fragment_shader_id, GL_COMPILE_STATUS, &errflags);
  if (!errflags) {
    glGetShaderInfoLog(program.fragment_shader_id, 10000, 0, errbuf);
    errbuf[6] = '\n';
    printf("\n*** FRAGMENT SHADER COMPILATION %s\n", errbuf);
    if (abort_on_error) {
      throw std::runtime_error("shader compilation failed\n");
    }
    return false;
  }
  check_glerror();

  // create program
  check_glerror();
  program.program_id = glCreateProgram();
  glAttachShader(program.program_id, program.vertex_shader_id);
  glAttachShader(program.program_id, program.fragment_shader_id);
  glLinkProgram(program.program_id);
  glValidateProgram(program.program_id);
  glGetProgramiv(program.program_id, GL_LINK_STATUS, &errflags);
  if (!errflags) {
    glGetShaderInfoLog(program.fragment_shader_id, 10000, 0, errbuf);
    //    errbuf[6] = '\n';
    printf("\n*** SHADER LINKING %s\n", errbuf);
    if (abort_on_error) {
      throw std::runtime_error("shader linking failed\n");
    }
    return false;
  }
  check_glerror();
  return true;
}

void delete_glprogram(opengl_program& program) {
  if (!program) return;
  glDeleteProgram(program.program_id);
  glDeleteShader(program.vertex_shader_id);
  glDeleteShader(program.fragment_shader_id);
  program.program_id         = 0;
  program.vertex_shader_id   = 0;
  program.fragment_shader_id = 0;
}

void init_gltexture(opengl_texture& texture, const vec2i& size, bool as_float,
    bool as_srgb, bool linear, bool mipmap) {
  if (texture) delete_gltexture(texture);
  check_glerror();
  glGenTextures(1, &texture.texture_id);
  texture.size     = size;
  texture.mipmap   = mipmap;
  texture.is_srgb  = as_srgb;
  texture.is_float = as_float;
  glBindTexture(GL_TEXTURE_2D, texture.texture_id);
  if (as_float) {
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, size.x, size.y, 0, GL_RGBA,
        GL_FLOAT, nullptr);
  } else if (as_srgb) {
    glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB_ALPHA, size.x, size.y, 0, GL_RGBA,
        GL_UNSIGNED_BYTE, nullptr);
  } else {
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, size.x, size.y, 0, GL_RGBA,
        GL_FLOAT, nullptr);
  }
  if (mipmap) {
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
        (linear) ? GL_LINEAR_MIPMAP_LINEAR : GL_NEAREST_MIPMAP_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
        (linear) ? GL_LINEAR : GL_NEAREST);
  } else {
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
        (linear) ? GL_LINEAR : GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
        (linear) ? GL_LINEAR : GL_NEAREST);
  }
  check_glerror();
}

void update_gltexture(
    opengl_texture& texture, const image<vec4f>& img, bool mipmap) {
  check_glerror();
  glBindTexture(GL_TEXTURE_2D, texture.texture_id);
  glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, img.size().x, img.size().y, GL_RGBA,
      GL_FLOAT, img.data());
  if (mipmap) glGenerateMipmap(GL_TEXTURE_2D);
  check_glerror();
}

void update_gltexture_region(opengl_texture& texture, const image<vec4f>& img,
    const image_region& region, bool mipmap) {
  check_glerror();
  glBindTexture(GL_TEXTURE_2D, texture.texture_id);
  auto clipped = image<vec4f>{};
  get_region(clipped, img, region);
  glTexSubImage2D(GL_TEXTURE_2D, 0, region.min.x, region.min.y, region.size().x,
      region.size().y, GL_RGBA, GL_FLOAT, clipped.data());
  if (mipmap) glGenerateMipmap(GL_TEXTURE_2D);
  check_glerror();
}

void update_gltexture(
    opengl_texture& texture, const image<vec4b>& img, bool mipmap) {
  check_glerror();
  glBindTexture(GL_TEXTURE_2D, texture.texture_id);
  glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, img.size().x, img.size().y, GL_RGBA,
      GL_UNSIGNED_BYTE, img.data());
  if (mipmap) glGenerateMipmap(GL_TEXTURE_2D);
  check_glerror();
}

void update_gltexture_region(opengl_texture& texture, const image<vec4b>& img,
    const image_region& region, bool mipmap) {
  check_glerror();
  glBindTexture(GL_TEXTURE_2D, texture.texture_id);
  auto clipped = image<vec4b>{};
  get_region(clipped, img, region);
  glTexSubImage2D(GL_TEXTURE_2D, 0, region.min.x, region.min.y, region.size().x,
      region.size().y, GL_RGBA, GL_UNSIGNED_BYTE, clipped.data());
  if (mipmap) glGenerateMipmap(GL_TEXTURE_2D);
  check_glerror();
}

void delete_gltexture(opengl_texture& texture) {
  if (!texture) return;
  glDeleteTextures(1, &texture.texture_id);
  texture.texture_id = 0;
  texture.size       = zero2i;
}

template <typename T>
void init_glarray_buffer_impl(
    opengl_arraybuffer& buffer, const vector<T>& array, bool dynamic) {
  buffer           = opengl_arraybuffer{};
  buffer.num       = size(array);
  buffer.elem_size = sizeof(T);
  check_glerror();
  glGenBuffers(1, &buffer.buffer_id);
  glBindBuffer(GL_ARRAY_BUFFER, buffer.buffer_id);
  glBufferData(GL_ARRAY_BUFFER, size(array) * sizeof(T), array.data(),
      (dynamic) ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);
  check_glerror();
}

void init_glarraybuffer(
    opengl_arraybuffer& buffer, const vector<float>& data, bool dynamic) {
  init_glarray_buffer_impl(buffer, data, dynamic);
}
void init_glarraybuffer(
    opengl_arraybuffer& buffer, const vector<vec2f>& data, bool dynamic) {
  init_glarray_buffer_impl(buffer, data, dynamic);
}
void init_glarraybuffer(
    opengl_arraybuffer& buffer, const vector<vec3f>& data, bool dynamic) {
  init_glarray_buffer_impl(buffer, data, dynamic);
}
void init_glarraybuffer(
    opengl_arraybuffer& buffer, const vector<vec4f>& data, bool dynamic) {
  init_glarray_buffer_impl(buffer, data, dynamic);
}

void delete_glarraybuffer(opengl_arraybuffer& buffer) {
  if (!buffer) return;
  glDeleteBuffers(1, &buffer.buffer_id);
  buffer.buffer_id = 0;
  buffer.elem_size = 0;
  buffer.num       = 0;
}

template <typename T>
void init_glelementbuffer_impl(
    opengl_elementbuffer& buffer, const vector<T>& array, bool dynamic) {
  buffer           = opengl_elementbuffer{};
  buffer.num       = size(array);
  buffer.elem_size = sizeof(T);
  check_glerror();
  glGenBuffers(1, &buffer.buffer_id);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer.buffer_id);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, size(array) * sizeof(T), array.data(),
      (dynamic) ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);
  check_glerror();
}

void init_glelementbuffer(
    opengl_elementbuffer& buffer, const vector<int>& data, bool dynamic) {
  init_glelementbuffer_impl(buffer, data, dynamic);
}
void init_glelementbuffer(
    opengl_elementbuffer& buffer, const vector<vec2i>& data, bool dynamic) {
  init_glelementbuffer_impl(buffer, data, dynamic);
}
void init_glelementbuffer(
    opengl_elementbuffer& buffer, const vector<vec3i>& data, bool dynamic) {
  init_glelementbuffer_impl(buffer, data, dynamic);
}

void delete_glelementbuffer(opengl_elementbuffer& buffer) {
  if (!buffer) return;
  glDeleteBuffers(1, &buffer.buffer_id);
  buffer.buffer_id = 0;
  buffer.elem_size = 0;
  buffer.num       = 0;
}

bool init_opengl_vertex_array_object(uint& vao) {
  check_glerror();
  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);
  check_glerror();
  return true;
}

bool delete_opengl_vertex_array_object(uint& vao) {
  check_glerror();
  glDeleteVertexArrays(1, &vao);
  check_glerror();
  return true;
}

bool bind_opengl_vertex_array_object(uint vao) {
  check_glerror();
  glBindVertexArray(vao);
  check_glerror();
  return true;
}

void bind_glprogram(const opengl_program& program) {
  glUseProgram(program.program_id);
}
void unbind_opengl_program() { glUseProgram(0); }

int get_gluniform_location(const opengl_program& program, const char* name) {
  return glGetUniformLocation(program.program_id, name);
}

void set_gluniform(int location, int value) {
  check_glerror();
  glUniform1i(location, value);
  check_glerror();
}

void set_gluniform(int location, const vec2i& value) {
  check_glerror();
  glUniform2i(location, value.x, value.y);
  check_glerror();
}

void set_gluniform(int location, const vec3i& value) {
  check_glerror();
  glUniform3i(location, value.x, value.y, value.z);
  check_glerror();
}

void set_gluniform(int location, const vec4i& value) {
  check_glerror();
  glUniform4i(location, value.x, value.y, value.z, value.w);
  check_glerror();
}

void set_gluniform(int location, float value) {
  check_glerror();
  glUniform1f(location, value);
  check_glerror();
}

void set_gluniform(int location, const vec2f& value) {
  check_glerror();
  glUniform2f(location, value.x, value.y);
  check_glerror();
}

void set_gluniform(int location, const vec3f& value) {
  check_glerror();
  glUniform3f(location, value.x, value.y, value.z);
  check_glerror();
}

void set_gluniform(int location, const vec4f& value) {
  check_glerror();
  glUniform4f(location, value.x, value.y, value.z, value.w);
  check_glerror();
}

void set_gluniform(int location, const mat2f& value) {
  check_glerror();
  glUniformMatrix2fv(location, 1, false, &value.x.x);
  check_glerror();
}

void set_gluniform(int location, const mat4f& value) {
  check_glerror();
  glUniformMatrix4fv(location, 1, false, &value.x.x);
  check_glerror();
}

void set_gluniform(int location, const frame3f& value) {
  check_glerror();
  glUniformMatrix4x3fv(location, 1, false, &value.x.x);
  check_glerror();
}

void set_gluniform(int location, const float* values, int num_values) {
  check_glerror();
  glUniform1fv(location, num_values, values);
  check_glerror();
}
void set_gluniform(int location, const vec3f* values, int num_values) {
  check_glerror();
  glUniform3fv(location, num_values, &values[0].x);
  check_glerror();
}

void set_gluniform_texture(
    int location, const opengl_texture& texture, int unit) {
  check_glerror();
  glActiveTexture(GL_TEXTURE0 + unit);
  glBindTexture(GL_TEXTURE_2D, texture.texture_id);
  glUniform1i(location, unit);
  check_glerror();
}

void set_gluniform_texture(const opengl_program& program, const char* name,
    const opengl_texture& texture, int unit) {
  set_gluniform_texture(get_gluniform_location(program, name), texture, unit);
}

void set_gluniform_texture(
    int location, int locatiom_on, const opengl_texture& texture, int unit) {
  check_glerror();
  if (texture.texture_id) {
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_2D, texture.texture_id);
    glUniform1i(location, unit);
    glUniform1i(locatiom_on, 1);
  } else {
    glUniform1i(locatiom_on, 0);
  }
  check_glerror();
}

void set_gluniform_texture(const opengl_program& program, const char* name,
    const char* name_on, const opengl_texture& texture, int unit) {
  set_gluniform_texture(get_gluniform_location(program, name),
      get_gluniform_location(program, name_on), texture, unit);
}

int get_glvertexattrib_location(
    const opengl_program& program, const char* name) {
  return glGetAttribLocation(program.program_id, name);
}

void set_glvertexattrib(
    int location, const opengl_arraybuffer& buffer, int elem_size) {
  check_glerror();
  assert(buffer.buffer_id);
  glBindBuffer(GL_ARRAY_BUFFER, buffer.buffer_id);
  glEnableVertexAttribArray(location);
  glVertexAttribPointer(location, elem_size, GL_FLOAT, false, 0, nullptr);
  check_glerror();
}

void set_glvertexattrib(
    int location, const opengl_arraybuffer& buffer, float value) {
  check_glerror();
  if (buffer.buffer_id) {
    glBindBuffer(GL_ARRAY_BUFFER, buffer.buffer_id);
    glEnableVertexAttribArray(location);
    glVertexAttribPointer(location, 1, GL_FLOAT, false, 0, nullptr);
  } else {
    glVertexAttrib1f(location, value);
  }
  check_glerror();
}

void set_glvertexattrib(
    int location, const opengl_arraybuffer& buffer, const vec2f& value) {
  check_glerror();
  if (buffer.buffer_id) {
    glBindBuffer(GL_ARRAY_BUFFER, buffer.buffer_id);
    glEnableVertexAttribArray(location);
    glVertexAttribPointer(location, 2, GL_FLOAT, false, 0, nullptr);
  } else {
    glVertexAttrib2f(location, value.x, value.y);
  }
  check_glerror();
}

void set_glvertexattrib(
    int location, const opengl_arraybuffer& buffer, const vec3f& value) {
  check_glerror();
  if (buffer.buffer_id) {
    glBindBuffer(GL_ARRAY_BUFFER, buffer.buffer_id);
    glEnableVertexAttribArray(location);
    glVertexAttribPointer(location, 3, GL_FLOAT, false, 0, nullptr);
  } else {
    glVertexAttrib3f(location, value.x, value.y, value.z);
  }
  check_glerror();
}

void set_glvertexattrib(
    int location, const opengl_arraybuffer& buffer, const vec4f& value) {
  check_glerror();
  if (buffer.buffer_id) {
    glBindBuffer(GL_ARRAY_BUFFER, buffer.buffer_id);
    glEnableVertexAttribArray(location);
    glVertexAttribPointer(location, 4, GL_FLOAT, false, 0, nullptr);
  } else {
    glVertexAttrib4f(location, value.x, value.y, value.z, value.w);
  }
  check_glerror();
}

void draw_glpoints(const opengl_elementbuffer& buffer, int num) {
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer.buffer_id);
  glDrawElements(GL_POINTS, num, GL_UNSIGNED_INT, nullptr);
}

void draw_gllines(const opengl_elementbuffer& buffer, int num) {
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer.buffer_id);
  glDrawElements(GL_LINES, num * 2, GL_UNSIGNED_INT, nullptr);
}

void draw_gltriangles(const opengl_elementbuffer& buffer, int num) {
  check_glerror();
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer.buffer_id);
  check_glerror();
  glDrawElements(GL_TRIANGLES, num * 3, GL_UNSIGNED_INT, nullptr);
  check_glerror();
}

void draw_glpoints(const opengl_arraybuffer& buffer, int num) {
  glDrawArrays(GL_POINTS, 0, num);
}

void draw_gllines(const opengl_arraybuffer& buffer, int num) {
  glDrawArrays(GL_LINE_STRIP, 0, num);
}

void draw_gltriangles(const opengl_arraybuffer& buffer, int num) {
  glDrawArrays(GL_TRIANGLE_STRIP, 0, num);
}

void draw_glimage(const opengl_texture& texture, int win_width, int win_height,
    const vec2f& image_center, float image_scale) {
  static opengl_program       gl_prog      = {};
  static opengl_arraybuffer   gl_texcoord  = {};
  static opengl_elementbuffer gl_triangles = {};

  // initialization
  if (!gl_prog) {
    auto vert = R"(
            #version 330
            in vec2 texcoord;
            out vec2 frag_texcoord;
            uniform vec2 window_size, image_size;
            uniform vec2 image_center;
            uniform float image_scale;
            void main() {
                vec2 pos = (texcoord - vec2(0.5,0.5)) * image_size * image_scale + image_center;
                gl_Position = vec4(2 * pos.x / window_size.x - 1, 1 - 2 * pos.y / window_size.y, 0, 1);
                frag_texcoord = texcoord;
            }
        )";
    auto frag = R"(
            #version 330
            in vec2 frag_texcoord;
            out vec4 frag_color;
            uniform sampler2D txt;
            void main() {
                // frag_color = texture(txt, frag_texcoord);
                frag_color = vec4(1,0,0,1);
            }
        )";
    init_glprogram(gl_prog, vert, frag);
    init_glarraybuffer(
        gl_texcoord, vector<vec2f>{{0, 0}, {0, 1}, {1, 1}, {1, 0}}, false);
    init_glelementbuffer(
        gl_triangles, vector<vec3i>{{0, 1, 2}, {0, 2, 3}}, false);
  }

  // draw
  check_glerror();
  bind_glprogram(gl_prog);
  set_gluniform_texture(gl_prog, "txt", texture, 0);
  set_gluniform(
      gl_prog, "window_size", vec2f{(float)win_width, (float)win_height});
  set_gluniform(gl_prog, "image_size",
      vec2f{(float)texture.size.x, (float)texture.size.y});
  set_gluniform(gl_prog, "image_center", image_center);
  set_gluniform(gl_prog, "image_scale", image_scale);
  set_glvertexattrib(gl_prog, "texcoord", gl_texcoord, zero2f);
  draw_gltriangles(gl_triangles, 2);
  unbind_opengl_program();
  check_glerror();
}

void draw_glimage_background(const opengl_texture& texture, int win_width,
    int win_height, const vec2f& image_center, float image_scale,
    float border_size) {
  static opengl_program       gl_prog      = {};
  static opengl_arraybuffer   gl_texcoord  = {};
  static opengl_elementbuffer gl_triangles = {};

  // initialization
  if (!gl_prog) {
    auto vert = R"(
            #version 330
            in vec2 texcoord;
            out vec2 frag_texcoord;
            uniform vec2 window_size, image_size, border_size;
            uniform vec2 image_center;
            uniform float image_scale;
            void main() {
                vec2 pos = (texcoord - vec2(0.5,0.5)) * (image_size + border_size*2) * image_scale + image_center;
                gl_Position = vec4(2 * pos.x / window_size.x - 1, 1 - 2 * pos.y / window_size.y, 0.1, 1);
                frag_texcoord = texcoord;
            }
        )";
    auto frag = R"(
            #version 330
            in vec2 frag_texcoord;
            out vec4 frag_color;
            uniform vec2 image_size, border_size;
            uniform float image_scale;
            void main() {
                ivec2 imcoord = ivec2(frag_texcoord * (image_size + border_size*2) - border_size);
                ivec2 tilecoord = ivec2(frag_texcoord * (image_size + border_size*2) * image_scale - border_size);
                ivec2 tile = tilecoord / 16;
                if(imcoord.x <= 0 || imcoord.y <= 0 || 
                    imcoord.x >= image_size.x || imcoord.y >= image_size.y) frag_color = vec4(0,0,0,1);
                else if((tile.x + tile.y) % 2 == 0) frag_color = vec4(0.1,0.1,0.1,1);
                else frag_color = vec4(0.3,0.3,0.3,1);
            }
        )";
    init_glprogram(gl_prog, vert, frag);
    init_glarraybuffer(
        gl_texcoord, vector<vec2f>{{0, 0}, {0, 1}, {1, 1}, {1, 0}}, false);
    init_glelementbuffer(
        gl_triangles, vector<vec3i>{{0, 1, 2}, {0, 2, 3}}, false);
  }

  // draw
  bind_glprogram(gl_prog);
  set_gluniform(
      gl_prog, "window_size", vec2f{(float)win_width, (float)win_height});
  set_gluniform(gl_prog, "image_size",
      vec2f{(float)texture.size.x, (float)texture.size.y});
  set_gluniform(
      gl_prog, "border_size", vec2f{(float)border_size, (float)border_size});
  set_gluniform(gl_prog, "image_center", image_center);
  set_gluniform(gl_prog, "image_scale", image_scale);
  set_glvertexattrib(gl_prog, "texcoord", gl_texcoord, zero2f);
  draw_gltriangles(gl_triangles, 2);
  unbind_opengl_program();
}

void init_glshape(opengl_shape& shape) {
  init_opengl_vertex_array_object(shape.vao);
}

opengl_shape make_glpoints(const vector<vec3f>& positions) {
  auto shape = opengl_shape{};
  init_glshape(shape);
  add_vertex_attribute(shape, positions);
  shape.type = opengl_shape::type::points;
  return shape;
}

opengl_shape make_glpolyline(
    const vector<vec3f>& positions, const vector<vec3f>& normals, float eps) {
  auto shape = opengl_shape{};
  init_glshape(shape);
  add_vertex_attribute(shape, positions);
  if (normals.size()) {
    add_vertex_attribute(shape, normals);
  }
  shape.type = opengl_shape::type::lines;
  return shape;
}

opengl_shape make_glquad() {
  auto shape = opengl_shape{};
  init_glshape(shape);
  add_vertex_attribute(
      shape, vector<vec2f>{{-1, -1}, {1, -1}, {-1, 1}, {1, 1}});
  shape.type = opengl_shape::type::triangles;
  return shape;
}

opengl_shape make_glmesh(const vector<vec3i>& triangles,
    const vector<vec3f>& positions, const vector<vec3f>& normals) {
  auto shape = opengl_shape{};
  init_glshape(shape);
  add_vertex_attribute(shape, positions);
  add_vertex_attribute(shape, normals);
  init_elements(shape, triangles);
  return shape;
}

opengl_shape make_glvector_field(
    const vector<vec3f>& vector_field, const vector<vec3f>& from, float scale) {
  assert(vector_field.size() == from.size());
  auto shape = opengl_shape{};
  init_glshape(shape);
  auto size      = vector_field.size();
  auto positions = vector<vec3f>(size * 2);

  for (int i = 0; i < size; i++) {
    auto to              = from[i] + scale * vector_field[i];
    positions[i * 2]     = from[i];
    positions[i * 2 + 1] = to;
  }
  add_vertex_attribute(shape, positions);

  auto elements = vector<vec2i>(size);
  for (int i = 0; i < elements.size(); i++) {
    elements[i] = {2 * i, 2 * i + 1};
  }
  init_elements(shape, elements);
  return shape;
}

opengl_shape make_glvector_field(const vector<vec3f>& vector_field,
    const vector<vec3i>& triangles, const vector<vec3f>& positions,
    float scale) {
  assert(vector_field.size() == triangles.size());
  auto shape = opengl_shape{};
  init_glshape(shape);
  auto size = vector_field.size();
  auto pos  = vector<vec3f>(size * 2);

  for (int i = 0; i < triangles.size(); i++) {
    auto x      = positions[triangles[i].x];
    auto y      = positions[triangles[i].y];
    auto z      = positions[triangles[i].z];
    auto normal = triangle_normal(x, y, z);
    normal *= scale;
    auto center    = (x + y + z) / 3;
    auto from      = center + 0.001 * normal;
    auto to        = from + (scale * vector_field[i]) + 0.001 * normal;
    pos[i * 2]     = from;
    pos[i * 2 + 1] = to;
  }
  add_vertex_attribute(shape, positions);

  auto elements = vector<vec2i>(size);
  for (int i = 0; i < elements.size(); i++) {
    elements[i] = {2 * i, 2 * i + 1};
  }
  init_elements(shape, elements);
  return shape;
}

void draw_glshape(const opengl_shape& shape) {
  bind_opengl_vertex_array_object(shape.vao);

  // draw strip of points, lines or triangles
  if (!shape.elements) {
    auto& positions = shape.vertex_attributes[0];
    if (shape.type == opengl_shape::type::points) {
      draw_glpoints(positions, positions.num);
    } else if (shape.type == opengl_shape::type::lines) {
      draw_gllines(positions, positions.num);
    } else if (shape.type == opengl_shape::type::triangles) {
      draw_gltriangles(positions, positions.num);
    }
  }
  // draw points, lines or triangles
  else {
    auto& elements = shape.elements;
    // glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elements.buffer_id);
    if (shape.type == opengl_shape::type::points) {
      draw_glpoints(elements, elements.num);
    } else if (shape.type == opengl_shape::type::lines) {
      draw_gllines(elements, elements.num);
    } else if (shape.type == opengl_shape::type::triangles) {
      draw_gltriangles(elements, elements.num);
    }
  }
  check_glerror();
}

void delete_glshape(opengl_shape& glshape) {
  for (auto& attribute : glshape.vertex_attributes) {
    delete_glarraybuffer(attribute);
  }
  delete_glelementbuffer(glshape.elements);
  delete_opengl_vertex_array_object(glshape.vao);
}

opengl_camera make_lookat_camera(
    const vec3f& from, const vec3f& to, const vec3f& up) {
  auto camera  = opengl_camera{};
  camera.frame = lookat_frame(from, to);
  camera.focus = length(from - to);
  return camera;
}

mat4f make_view_matrix(const opengl_camera& camera) {
  return mat4f(inverse(camera.frame));
}

mat4f make_projection_matrix(
    const opengl_camera& camera, const vec2i& viewport, float near, float far) {
  auto camera_aspect = (float)viewport.x / (float)viewport.y;
  auto camera_yfov =
      camera_aspect >= 0
          ? (2 * yocto::atan(camera.film / (camera_aspect * 2 * camera.lens)))
          : (2 * yocto::atan(camera.film / (2 * camera.lens)));
  return perspective_mat(camera_yfov, camera_aspect, near, far);
}

opengl_render_target make_glrender_target(
    const vec2i& size, bool as_float, bool as_srgb, bool linear, bool mipmap) {
  auto target = opengl_render_target{};
  glGenFramebuffers(1, &target.framebuffer);
  glBindFramebuffer(GL_FRAMEBUFFER, target.framebuffer);

  // generate texture
  // create a color attachment texture
  auto& textureColorbuffer = target.texture.texture_id;
  glGenTextures(1, &textureColorbuffer);
  glBindTexture(GL_TEXTURE_2D, textureColorbuffer);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, size.x, size.y, 0, GL_RGB,
      GL_UNSIGNED_BYTE, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
      textureColorbuffer, 0);
  // create a renderbuffer object for depth and stencil attachment (we won't be
  // sampling these)

  // create render buffer for depth
  glGenRenderbuffers(1, &target.renderbuffer);
  glBindRenderbuffer(GL_RENDERBUFFER, target.renderbuffer);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, size.x, size.y);
  glBindRenderbuffer(GL_RENDERBUFFER, 0);

  // bind frame buffer and render buffer
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT,
      GL_RENDERBUFFER, target.renderbuffer);
  assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  // done
  return target;
}

void bind_glrender_target(const opengl_render_target& target) {
  glBindFramebuffer(GL_FRAMEBUFFER, target.framebuffer);
  // if (target.depth_testing) {
  //   glEnable(GL_DEPTH_TEST);
  // }
}

void unbind_glrender_target() { glBindFramebuffer(GL_FRAMEBUFFER, 0); }

}  // namespace opengl
