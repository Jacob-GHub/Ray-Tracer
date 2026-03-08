// Student Name: Jacob Perez
// Student ID: Jpere508

#include "sphere.h"
#include "ray.h"

Sphere::Sphere(const Parse* parse, std::istream& in)
{
    in>>name>>center>>radius;
}

// Determine if the ray intersects with the sphere
Hit Sphere::Intersection(const Ray& ray, int part) const
{
    Hit hit;
    vec3 oc = ray.endpoint - center;
    
    double a = dot(ray.direction, ray.direction);
    double b = 2.0 * dot(oc, ray.direction);
    double c = dot(oc, oc) - radius * radius;
    
    double discriminant = b * b - 4 * a * c;
    
    if(discriminant < 0)
    {
        return hit;
    }
    
    double sqrt_discriminant = sqrt(discriminant);
    double t1 = (-b - sqrt_discriminant) / (2.0 * a);
    double t2 = (-b + sqrt_discriminant) / (2.0 * a);
    
    double t = -1;
    
    if(t1 >= small_t)
    {
        t = t1;
    }
    else if(t2 >= small_t)
    {
        t = t2;
    }
    
    if(t >= small_t)
    {
        hit.dist = t;
    }
    
    return hit;
}

vec3 Sphere::Normal(const Ray& ray, const Hit& hit) const
{
    vec3 normal;
    vec3 intersection_point = ray.Point(hit.dist);
    
    normal = (intersection_point - center).normalized();
    
    return normal;
}

std::pair<Box,bool> Sphere::Bounding_Box(int part) const
{
    return {{center-radius,center+radius},false};
}