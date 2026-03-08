#include <vector>
#include <string>
#include <iostream>
#include <limits>
#include "common.h"
#include <algorithm>

using namespace std;

void Rasterize(Pixel* pixels, int width, int height, const std::vector<Triangle>& tris)
{
    vector<double> z_buffer(width * height, numeric_limits<double>::infinity());

    for (const auto& tri : tris)
    {
        double wa = tri.A[3];
        double wb = tri.B[3];
        double wc = tri.C[3];
        // clip space to normalized DC
        vec3 a(tri.A[0]/wa, tri.A[1]/wa, tri.A[2]/wa);
        vec3 b(tri.B[0]/wb, tri.B[1]/wb, tri.B[2]/wb);
        vec3 c(tri.C[0]/wc, tri.C[1]/wc, tri.C[2]/wc);

        auto to_screen_coords = [&](const vec3& ndc) -> vec3 {
            return vec3((ndc[0] + 1.0) * 0.5 * width, (ndc[1] + 1.0) * 0.5 * height, ndc[2] );
        };

        vec3 pix_a = to_screen_coords(a);
        vec3 pix_b = to_screen_coords(b);
        vec3 pix_c = to_screen_coords(c);

        double minx = min({ pix_a[0], pix_b[0], pix_c[0]});
        double maxx = max({pix_a[0], pix_b[0], pix_c[0]});
        double miny = min({pix_a[1], pix_b[1], pix_c[1]});
        double maxy = max({pix_a[1], pix_b[1], pix_c[1]});

        int xmin = max(0, (int)floor(minx));
        int xmax = min(width - 1, (int)floor(maxx));
        int ymin = max(0, (int)floor(miny));
        int ymax = min(height - 1, (int)floor(maxy));

        auto edge = [](const vec3& p0, const vec3& p1, double px, double py) {
            return (p1[0] - p0[0]) * (py - p0[1]) - (p1[1] - p0[1]) * (px - p0[0]);
        };

        double area = edge(pix_a, pix_b, pix_c[0], pix_c[1]);
        if (area == 0.0) 
        {
            continue; 
        }

        for (int y = ymin; y <= ymax; y++)
        {
            for (int x = xmin; x <= xmax; x++)
            {
                double px = x + 0.5;
                double py = y + 0.5;

                // barycentric coordinates
                double alpha = edge(pix_b, pix_c, px, py ) / area;
                double beta  = edge(pix_c, pix_a, px, py) / area;
                double gamma = edge(pix_a, pix_b, px, py) / area;

                // outside edge cases
                if ((alpha < 0.0) || (beta  < 0.0 ) || (gamma < 0.0) )
                {
                     continue;
                }

                double q = alpha/wa + beta/wb + gamma/wc;
                double z = (alpha * a[2]/wa + beta * b[2]/wb + gamma * c[2]/wc) / q;

                // actually use z for the buffer test
                int idx = x + width * y;
                if (z >= z_buffer[idx]) 
                {
                    continue;
                }
                z_buffer[idx] = z;

                // finally interpolate color
                vec3 color = (tri.Ca * (alpha / wa ) + tri.Cb * (beta / wb) + tri.Cc * (gamma/ wc)) / q;
                set_pixel(pixels, width, height, x, y, color);
            }
        }
    }
}