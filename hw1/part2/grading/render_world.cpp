// Student Name: Jacob Perez
// Student ID: Jpere508

#include "render_world.h"
#include "flat_shader.h"
#include "object.h"
#include "light.h"
#include "ray.h"

extern bool enable_acceleration;

Render_World::~Render_World()
{
    for(auto a:all_objects) delete a;
    for(auto a:all_shaders) delete a;
    for(auto a:all_colors) delete a;
    for(auto a:lights) delete a;
}

// Find and return the Hit structure for the closest intersection.  Be careful
// to ensure that hit.dist>=small_t.
std::pair<Shaded_Object,Hit> Render_World::Closest_Intersection(const Ray& ray) const
{
    Debug_Scope scope;

    double min_dist = std::numeric_limits<double>::infinity();
    Shaded_Object closest_obj;
    Hit closest_hit;
    
    for(const auto& obj : objects)
    {
        Hit hit = obj.object->Intersection(ray, -1);

        if(hit.dist < small_t) {
            Pixel_Print("no intersection with ", obj.object->name);
        }
        else {
            Pixel_Print("intersect test with ", obj.object->name,"; hit: (dist: ", hit.dist,"; triangle: ", hit.triangle,"; uv: ", hit.uv, ")");

            if(hit.dist < min_dist) {
                min_dist = hit.dist;
                closest_hit = hit;
                closest_obj = obj;
            }
        }
    }

    if(closest_obj.object){
        Pixel_Print("closest intersection; obj: ", closest_obj.object->name, "; hit: (dist: ", closest_hit.dist, "; triangle: ", closest_hit.triangle, "; uv: ", closest_hit.uv, ")");
        }
    else{
        Pixel_Print("no closest intersection");
    }
    return {closest_obj, closest_hit};
}

// set up the initial view ray and call
void Render_World::Render_Pixel(const ivec2& pixel_index)
{
    Debug_Scope scope;
    Pixel_Print("debug pixel: -x ", pixel_index[0], " -y ", pixel_index[1]);

    vec3 pos = camera.World_Position(pixel_index);
    vec3 dir = (pos - camera.position).normalized();

    Ray ray(camera.position, dir);
    Pixel_Print("cast ray (end: ", ray.endpoint, "; dir: ", ray.direction, ")");

    vec3 shade=Cast_Ray(ray,1);
    camera.Set_Pixel(pixel_index,Pixel_Color(shade));
}

void Render_World::Render()
{
    for(int j=0;j<camera.number_pixels[1];j++)
        for(int i=0;i<camera.number_pixels[0];i++)
            Render_Pixel(ivec2(i,j));
}

// cast ray and return the color of the closest intersected surface point,
// or the background color if there is no object intersection
vec3 Render_World::Cast_Ray(const Ray& ray,int recursion_depth) const
{

    Debug_Scope scope;
    vec3 color;
    auto [closest_obj, hit] = Closest_Intersection(ray);

    if (closest_obj.object != nullptr)
    {
        vec3 intersection_point = ray.Point(hit.dist);
        vec3 normal = closest_obj.object->Normal(ray, hit);


        Pixel_Print("call Shade_Surface with location ", intersection_point,"; normal: ", normal);

        color = closest_obj.shader->Shade_Surface(
            *this,
            ray,
            hit,
            intersection_point,
            normal,
            recursion_depth
        );
    }
    else
    {
        if (background_shader != nullptr)
        {
            color = background_shader->Shade_Surface(
                *this,
                ray,
                hit,
                vec3(0, 0, 0),
                vec3(0, 0, 0),
                recursion_depth
            );
        }
        else
        {
            color = vec3(0, 0, 0);
        }
    }

    return color;
}
