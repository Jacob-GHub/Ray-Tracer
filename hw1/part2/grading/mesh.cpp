#include "mesh.h"
#include "ray.h"
#include <fstream>
#include <limits>
#include <string>
#include <algorithm>

// Consider a triangle to intersect a ray if the ray intersects the plane of the
// triangle with barycentric weights in [-weight_tolerance, 1+weight_tolerance]
static const double weight_tolerance = 1e-4;

Mesh::Mesh(const Parse* parse, std::istream& in)
{
    std::string file;
    in>>name>>file;
    Read_Obj(file.c_str());
}

// Read in a mesh from an obj file.  Populates the bounding box and registers
// one part per triangle (by setting number_parts).
void Mesh::Read_Obj(const char* file)
{
    std::ifstream fin(file);
    if(!fin)
    {
        exit(EXIT_FAILURE);
    }
    std::string line;
    ivec3 e, t;
    vec3 v;
    vec2 u;
    while(fin)
    {
        getline(fin,line);

        if(sscanf(line.c_str(), "v %lg %lg %lg", &v[0], &v[1], &v[2]) == 3)
        {
            vertices.push_back(v);
        }

        if(sscanf(line.c_str(), "f %d %d %d", &e[0], &e[1], &e[2]) == 3)
        {
            for(int i=0;i<3;i++) e[i]--;
            triangles.push_back(e);
        }

        if(sscanf(line.c_str(), "vt %lg %lg", &u[0], &u[1]) == 2)
        {
            uvs.push_back(u);
        }

        if(sscanf(line.c_str(), "f %d/%d %d/%d %d/%d", &e[0], &t[0], &e[1], &t[1], &e[2], &t[2]) == 6)
        {
            for(int i=0;i<3;i++) e[i]--;
            triangles.push_back(e);
            for(int i=0;i<3;i++) t[i]--;
            triangle_texture_index.push_back(t);
        }
    }
    num_parts=triangles.size();
}

// Check for an intersection against the ray.  See the base class for details.
Hit Mesh::Intersection(const Ray& ray, int part) const
{
    if(part >= 0)
    {
        return Intersect_Triangle(ray, part);
    }

    Hit closest_hit;
    for(int i = 0; i < (int)triangles.size(); i++)
    {
        Hit cur_h = Intersect_Triangle(ray, i);
        if(cur_h.dist > 0 && (closest_hit.dist < 0 || cur_h.dist < closest_hit.dist))
        {
            closest_hit = cur_h;
        }
    }
    return closest_hit;
}

// Compute the normal direction for the triangle with index part.
vec3 Mesh::Normal(const Ray& ray, const Hit& hit) const
{
    assert(hit.triangle>=0);
    ivec3 triangleIndices = triangles[hit.triangle];
    vec3 vertex1 = vertices[triangleIndices[0]];
    vec3 vertex2= vertices[triangleIndices[1]];
    vec3 vertex3 = vertices[triangleIndices[2]];
    // Flat face normal via cross product of two edges
    return cross(vertex2 - vertex1, vertex3 - vertex1).normalized();
}

// This is a helper routine whose purpose is to simplify the implementation
// of the Intersection routine.  It should test for an intersection between
// the ray and the triangle with index tri.  If an intersection exists,
// record the distance and return true.  Otherwise, return false.
// This intersection should be computed by determining the intersection of
// the ray and the plane of the triangle.  From this, determine (1) where
// along the ray the intersection point occurs (dist) and (2) the barycentric
// coordinates within the triangle where the intersection occurs.  The
// triangle intersects the ray if dist>small_t and the barycentric weights are
// larger than -weight_tolerance.  The use of small_t avoid the self-shadowing
// bug, and the use of weight_tolerance prevents rays from passing in between
// two triangles.
Hit Mesh::Intersect_Triangle(const Ray& ray, int tri) const
{
    ivec3 e = triangles[tri];
    vec3 A = vertices[e[0]];
    vec3 B = vertices[e[1]];
    vec3 C = vertices[e[2]];

    vec3 AB = B - A;
    vec3 AC = C - A;
    vec3 N  = cross(AB, AC); // norm vec

    double denom = dot(N, ray.direction);
    if(std::fabs(denom) < 1e-10) return {};
    double dist = dot(N, A - ray.endpoint) / denom;
    if(dist <= small_t) {return {};};

    vec3 P = ray.endpoint + dist * ray.direction;

    // barycentric coordinates calculation
    vec3 AP  = P - A;
    double d00 = dot(AB, AB);
    double d01 = dot(AB, AC);
    double d11 = dot(AC, AC);
    double d20 = dot(AP, AB);
    double d21 = dot(AP, AC);
    double inv = 1.0 / (d00 * d11 - d01 * d01);

    double b_weight= (d11 * d20 - d01 * d21) * inv; 
    double c_weight = (d00 * d21 - d01 * d20) * inv; 
    double a_weight = 1.0 - b_weight - c_weight;               

    if(b_weight < -weight_tolerance || c_weight < -weight_tolerance || a_weight < -weight_tolerance)
        return {};

    Hit hit;
    hit.dist = dist;
    hit.triangle = tri;

    if(tri < (int)triangle_texture_index.size())
    {
        ivec3 ti = triangle_texture_index[tri];
        hit.uv = a_weight * uvs[ti[0]]
               + b_weight * uvs[ti[1]]
               + c_weight * uvs[ti[2]];
    }
    
    return hit;
}

std::pair<Box,bool> Mesh::Bounding_Box(int part) const
{
    if(part<0)
    {
        Box box;
        box.Make_Empty();
        for(const auto& v:vertices)
            box.Include_Point(v);
        return {box,false};
    }

    ivec3 e=triangles[part];
    vec3 A=vertices[e[0]];
    Box b={A,A};
    b.Include_Point(vertices[e[1]]);
    b.Include_Point(vertices[e[2]]);
    return {b,false};
}
