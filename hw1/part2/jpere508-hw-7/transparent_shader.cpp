#include "transparent_shader.h"
#include "parse.h"
#include "ray.h"
#include "render_world.h"

Transparent_Shader::
Transparent_Shader(const Parse* parse,std::istream& in)
{
    in>>name>>index_of_refraction>>opacity;
    shader=parse->Get_Shader(in);
    assert(index_of_refraction>=1.0);
}

// Use opacity to determine the contribution of this->shader and the Schlick
// approximation to compute the reflectivity.  This routine shades transparent
// objects such as glass.  Note that the incoming and outgoing indices of
// refraction depend on whether the ray is entering the object or leaving it.
// You may assume that the object is surrounded by air with index of refraction
// 1.
vec3 Transparent_Shader::
Shade_Surface(const Render_World& render_world,const Ray& ray,const Hit& hit,
    const vec3& intersection_point,const vec3& normal,int recursion_depth) const
{
    vec3 norm_adj = normal;
    double n1 = 1.0;
    double n2 = index_of_refraction;
    
    if(dot(ray.direction, normal) > 0) {
        norm_adj = -normal;
        std::swap(n1, n2);
    }
    
    double ratio = n1 / n2;
    double cos_i = -dot(ray.direction, norm_adj);
    double sin_t_squared = ratio * ratio * (1.0 - cos_i * cos_i);
    
    vec3 refracted_col = vec3(0, 0, 0);
    vec3 reflected_col = vec3(0, 0, 0);
    double reflectance = 1.0;
    
    //checking lim
    if(recursion_depth < render_world.recursion_depth_limit) {
        if(sin_t_squared <= 1.0) {
            double cos_t = sqrt(1.0 - sin_t_squared);
            vec3 refracted_dir = ratio * ray.direction + (ratio * cos_i - cos_t) * norm_adj;
            refracted_dir = refracted_dir.normalized();
            
            Ray refracted_ray(intersection_point, refracted_dir);
            refracted_col = render_world.Cast_Ray(refracted_ray, recursion_depth + 1);
            
            double r0 = (n1 - n2) / (n1 + n2);
            r0 = r0 * r0;
            reflectance = r0 + (1.0 - r0) * pow(1.0 - cos_i, 5);
        }
        
        vec3 reflected_dir = ray.direction - 2 * dot(ray.direction, norm_adj) * norm_adj;
        reflected_dir = reflected_dir.normalized();
        
        Ray reflected_ray(intersection_point, reflected_dir);
        reflected_col = render_world.Cast_Ray(reflected_ray, recursion_depth + 1);
    }
    
    vec3 base_col = shader->Shade_Surface(render_world, ray, hit, intersection_point, normal, recursion_depth);
    vec3 transparent_col = reflectance * reflected_col + (1.0 - reflectance) * refracted_col;
    vec3 res = opacity * base_col + (1.0 - opacity) * transparent_col;
    
    return res;
}