#include "reflective_shader.h"
#include "parse.h"
#include "ray.h"
#include "render_world.h"

Reflective_Shader::Reflective_Shader(const Parse* parse,std::istream& in)
{
    in>>name;
    shader=parse->Get_Shader(in);
    in>>reflectivity;
    reflectivity=std::max(0.0,std::min(1.0,reflectivity));
}

vec3 Reflective_Shader::
Shade_Surface(const Render_World& render_world,const Ray& ray,const Hit& hit,
    const vec3& intersection_point,const vec3& normal,int recursion_depth) const
{
   vec3 base_col = shader->Shade_Surface(render_world, ray, hit, intersection_point, normal, recursion_depth);
    
    vec3 reflected_dir = ray.direction - 2 * dot(ray.direction, normal) * normal;
    reflected_dir = reflected_dir.normalized();
    
    Ray reflected_ray(intersection_point, reflected_dir);
    
    vec3 reflected_col;
    if(recursion_depth < render_world.recursion_depth_limit) {
        reflected_col = render_world.Cast_Ray(reflected_ray, recursion_depth + 1);
    } else {
        reflected_col = vec3(0, 0, 0);
    }
    
    vec3 res = (1 - reflectivity) * base_col + reflectivity * reflected_col;
    
    return res;
}
