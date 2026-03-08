#include "light.h"
#include "parse.h"
#include "object.h"
#include "phong_shader.h"
#include "ray.h"
#include "render_world.h"

Phong_Shader::Phong_Shader(const Parse* parse,std::istream& in)
{
    in>>name;
    color_ambient=parse->Get_Color(in);
    color_diffuse=parse->Get_Color(in);
    color_specular=parse->Get_Color(in);
    in>>specular_power;
}

vec3 Phong_Shader::
Shade_Surface(const Render_World& render_world,const Ray& ray,const Hit& hit,
    const vec3& intersection_point,const vec3& normal,int recursion_depth) const
{
    vec3 color;
    if(render_world.ambient_color && render_world.ambient_intensity > 0)
    {
        color = color_ambient->Get_Color(hit.uv) * render_world.ambient_color->Get_Color(hit.uv) * render_world.ambient_intensity;
    }
    
    for(unsigned i = 0; i < render_world.lights.size(); i++)
    {
        const Light* light = render_world.lights[i];
        vec3 light_vec = light->position - intersection_point;
        vec3 light_dir = light_vec.normalized();
        double light_dist = light_vec.magnitude();
        bool in_shadow = false;

        if(render_world.enable_shadows)
        {
            Ray shadow_ray(intersection_point, light_dir);
            auto [shadow_obj, shadow_hit] = render_world.Closest_Intersection(shadow_ray);
            
            if(shadow_obj.object && shadow_hit.dist < light_dist)
            {
                in_shadow = true;
            }
        }
        
        if(in_shadow)
        {
            continue;
        }
        
        vec3 light_col = light->Emitted_Light(light_vec);
        double normal_dot_light = dot(normal, light_dir);
        
        if(normal_dot_light > 0)  
        {
            vec3 diffuse = color_diffuse->Get_Color(hit.uv) * light_col * normal_dot_light;
            color += diffuse;
        }
        
        vec3 reflection_dir = 2 * normal_dot_light * normal - light_dir;
        vec3 view_dir = -ray.direction;
        view_dir = view_dir.normalized();
        double reflection_dot_view = dot(reflection_dir, view_dir);
        
        if(reflection_dot_view > 0)  
        {
            double specular_intensity = pow(reflection_dot_view, specular_power);
            vec3 specular = color_specular->Get_Color(hit.uv) * light_col * specular_intensity;
            color += specular;
        }
    }
    
    return color;
}